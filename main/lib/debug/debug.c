/***************************************************************************

  debug.c

  (c) 2000-2007 Benoit Minisini <gambas@freesurf.fr>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 1, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

***************************************************************************/

#define __DEBUG_C

// Do not include gbx_debug.h
#define __GBX_DEBUG_H

#include "gb_common.h"
#include "gambas.h"


#include <unistd.h>
#include <signal.h>
#include <fcntl.h>

#include "gb_error.h"
#include "gbx_type.h"
#include "gb_limit.h"
#include "gbx_stack.h"
#include "gbx_class.h"
#include "gbx_exec.h"
#include "gbx_local.h"
#include "gbx_object.h"

#include "gbx_eval.h"

#include "print.h"

#include "debug.h"

/*#define DEBUG_ME*/

PUBLIC DEBUG_INFO DEBUG_info = { 0 };
PUBLIC GB_DEBUG_INTERFACE *DEBUG_interface;
PUBLIC char DEBUG_buffer[DEBUG_BUFFER_MAX + 1];

static DEBUG_BREAK *Breakpoint;
static bool Error;

static EVAL_INTERFACE EVAL;

static int _fdr;
static int _fdw;
static FILE *_out;
static FILE *_in;
static bool _fifo;

#define EXEC_current (*(STACK_CONTEXT *)GB_DEBUG.GetExec())


#define WARNING(_msg, ...) fprintf(_out, "W\t" _msg "\n", ##__VA_ARGS__)
#define INFO(_msg, ...) fprintf(_out, "I\t" _msg "\n", ##__VA_ARGS__)

PUBLIC void DEBUG_break_on_next_line(void)
{
  DEBUG_info.stop = TRUE;
  DEBUG_info.fp = NULL;
  DEBUG_info.bp = NULL;
}


static void signal_user(int sig)
{
  signal(SIGUSR1, signal_user);

  #ifdef DEBUG_ME
  fprintf(stderr, "Got SIGUSR1\n");
  #endif

  /*CAPP_got_signal();*/
  DEBUG_break_on_next_line();
}


static boolean calc_line_from_position(CLASS *class, FUNCTION *func, PCODE *addr, ushort *line)
{
  int i;
  ushort pos = addr - func->code;
  ushort *post;

  if (func->debug)
  {
    post =  func->debug->pos;
    for (i = 0; i < (func->debug->nline - 1); i++)
    {
      if (pos >= post[i] && pos < post[i + 1])
      {
        *line = i + func->debug->line;
        return FALSE;
      }
    }

    /*printf("pos = %d addr=%p func->code=%p\n", pos, addr, func->code);*/
  }

  return TRUE;
}


static boolean calc_position_from_line(CLASS *class, ushort line, FUNCTION **function, PCODE **addr)
{
  int i;
  ushort pos, pos_after;
  FUNCTION *func = NULL;
  FUNC_DEBUG *debug = NULL;

  for (i = 0; i < class->load->n_func; i++)
  {
    func = &class->load->func[i];
    debug = func->debug;
    if (debug && line >= debug->line && line < (debug->line + debug->nline))
      break;
  }

  if (i >= class->load->n_func)
    return TRUE;

  line -= debug->line;

  for(;;)
  {
    pos = debug->pos[line];
    pos_after = debug->pos[line + 1];
    if (pos != pos_after)
      break;

    line++;
    if (line >= debug->nline)
      return TRUE;
  }

  *function = func;
  *addr = &func->code[pos];

  /*printf("%s.%d -> %04X\n", class->name, line + debug->line, **addr);*/

  return FALSE;
}


PUBLIC DEBUG_INFO *DEBUG_init(GB_DEBUG_INTERFACE *debug, bool fifo)
{
  char path[MAX_PATH];

  //if (!EXEC_debug)
  //  return;

	DEBUG_interface = debug;
	_fifo = fifo;

  if (_fifo)
  {
    sprintf(path, "/tmp/gambas.%d/%d.out", getuid(), getppid());
    _fdr = open(path, O_RDONLY);
		fcntl(_fdr, F_SETFD, FD_CLOEXEC);
    sprintf(path, "/tmp/gambas.%d/%d.in", getuid(), getppid());
    _fdw = open(path, O_WRONLY);
		fcntl(_fdw, F_SETFD, FD_CLOEXEC);

    _in = fdopen(_fdr, "r");
    _out = fdopen(_fdw, "w");

    if (!_in || !_out)
    	return NULL;
      //ERROR_panic("Cannot open fifos");

    setlinebuf(_in);
    //setvbuf(_in, NULL, _IONBF, 0);
    setlinebuf(_out);
    //setvbuf(_out, NULL, _IONBF, 0);
  }
  else
  {
    _in = stdin;
    _out = stdout;
  }

  //ARRAY_create(&Breakpoint);
  GB.NewArray(&Breakpoint, sizeof(DEBUG_BREAK), 16);
  signal(SIGUSR1, signal_user);
  signal(SIGPIPE, SIG_IGN);

  setlinebuf(_out);

  return &DEBUG_info;
}

PUBLIC void DEBUG_exit(void)
{
  GB.FreeArray(&Breakpoint);

  /* Don't do it, it blocks!

  if (EXEC_fifo)
  {
    fclose(_in);
    fclose(_out);
  }

  */
}


static int find_free_breakpoint(void)
{
  int i;
  char used[MAX_BREAKPOINT];

  memset(used, FALSE, MAX_BREAKPOINT);

  for (i = 0; i < ARRAY_count(Breakpoint); i++)
    used[Breakpoint[i].id - 1] = TRUE;

  for (i = 0; i < MAX_BREAKPOINT; i++)
    if (!used[i])
      return (i + 1);

  return 0;
}

static bool init_breakpoint(DEBUG_BREAK *brk)
{
  PCODE *addr = NULL;
  FUNCTION *func;

  //fprintf(stderr, "init_breakpoint: id = %d\n", brk->id);

	if (brk->addr || brk->class->state == CS_NULL)
	{
		WARNING("Breakpoint is pending");
		return TRUE;
	}

  if (CLASS_is_native(brk->class) || !brk->class->debug)
  {
    WARNING("Cannot set breakpoint: no debugging information");
    return TRUE;
  }

  if (calc_position_from_line(brk->class, brk->line, &func, &addr))
  {
  	WARNING("Cannot set breakpoint: cannot calc position");
    //fprintf(_out, "Cannot calc position from line number\n");
    return TRUE;
  }

  if (!PCODE_is_breakpoint(*addr))
  {
  	WARNING("Cannot set breakpoint: Not a line beginning");
    //fprintf(_out, "Not a line beginning ?\n");
    return TRUE;
  }

  if (*addr & 0xFF)
  {
  	WARNING("Breakpoint already set");
    //fprintf(_out, "Breakpoint already set\n");
    return FALSE;
  }

	brk->addr = addr;
  *addr = PCODE_BREAKPOINT(brk->id);

  //fprintf(stderr, "init_breakpoint: OK\n");

  #ifdef DEBUG_ME
  fprintf(stderr, "init_breakpoint: %s.%d\n", brk->class->name, brk->line);
  #endif

 	INFO("Breakpoint set: %s.%d", brk->class->name, brk->line);
  return FALSE;
}


static boolean set_breakpoint(CLASS *class, ushort line)
{
  DEBUG_BREAK *brk;
  int id;

  if (GB.Count(Breakpoint) >= MAX_BREAKPOINT)
  {
    WARNING("Too many breakpoints");
    return TRUE;
  }

  id = find_free_breakpoint();
  if (id == 0)
  {
    WARNING("Cannot create breakpoint");
    return TRUE;
  }

  brk = (DEBUG_BREAK *)GB.Add(&Breakpoint);

  brk->id = id;
  brk->addr = NULL;
  brk->class = class;
  brk->line = line;

  #ifdef DEBUG_ME
  fprintf(stderr, "set_breakpoint: %s.%d\n", class->name, line);
  #endif

  init_breakpoint(brk);

  return FALSE;
}


static boolean unset_breakpoint(CLASS *class, ushort line)
{
  int i;
  DEBUG_BREAK *brk;

  for (i = 0; i < GB.Count(Breakpoint); i++)
  {
  	brk = &Breakpoint[i];
    if (brk->class == class && brk->line == line)
    {
    	if (brk->addr)
      	*(brk->addr) = PCODE_BREAKPOINT(0);
      GB.Remove(&Breakpoint, i, 1);

      #ifdef DEBUG_ME
      fprintf(stderr, "unset_breakpoint: %s.%d\n", class->name, line);
      #endif

		 	INFO("Breakpoint removed");
      return FALSE;
    }
  }

  WARNING("Unknown breakpoint");
  return TRUE;
}


PUBLIC void DEBUG_init_breakpoints(CLASS *class)
{
  int i;
  DEBUG_BREAK *brk;

  for (i = 0; i < GB.Count(Breakpoint); i++)
  {
  	brk = &Breakpoint[i];
    if (brk->class == class)
    {
		  //fprintf(stderr, "DEBUG_init_breakpoints: %s\n", class->name);
    	init_breakpoint(brk);
		}
  }
}

static void command_quit(const char *cmd)
{
  exit(1);
}

static void command_go(const char *cmd)
{
  GB.Signal(GB_SIGNAL_DEBUG_CONTINUE, 0);

  DEBUG_info.stop = FALSE;
  DEBUG_info.fp = NULL;
  DEBUG_info.bp = NULL;
}

static void command_step(const char *cmd)
{
  GB.Signal(GB_SIGNAL_DEBUG_FORWARD, 0);
  DEBUG_break_on_next_line();
}

static void command_next(const char *cmd)
{
  GB.Signal(GB_SIGNAL_DEBUG_FORWARD, 0);
  DEBUG_info.stop = TRUE;
  DEBUG_info.fp = FP;
  DEBUG_info.bp = BP;
}

static void command_from(const char *cmd)
{
  STACK_CONTEXT *sc = GB_DEBUG.GetStack(0); //STACK_get_current();

  if (sc)
  {
	  GB.Signal(GB_SIGNAL_DEBUG_FORWARD, 0);
    DEBUG_info.stop = TRUE;
    DEBUG_info.fp = sc->fp;
    DEBUG_info.bp = sc->bp;
  }
  else
    command_go(cmd);
}


static void command_set_breakpoint(const char *cmd)
{
  char class_name[64];
  ushort line;
  CLASS *class;

  if (sscanf(cmd, "+%64[^.].%hu", class_name, &line) != 2)
    WARNING("Cannot set breakpoint: syntax error");
  else
  {
    class = (CLASS *)GB.FindClass(class_name);
    //CLASS_load_without_init(class);
		//fprintf(stderr, "command_set_breakpoint: %s %s\n", class->name, class->component ? class->component->name : "?");
    set_breakpoint(GB.FindClass(class_name), line);
  }
}


static void command_unset_breakpoint(const char *cmd)
{
  char class_name[64];
  ushort line;

  if (sscanf(cmd, "-%64[^.].%hu", class_name, &line) != 2)
    WARNING("Cannot remove breakpoint: Syntax error");
  else
  {
    //class = CLASS_find(class_name);
    //CLASS_load_without_init(class);
    unset_breakpoint(GB.FindClass(class_name), line);
  }
}


PUBLIC void DEBUG_backtrace(FILE *out)
{
  int i;
  STACK_CONTEXT *context;
  ushort line;

  if (CP)
    fprintf(out, "%s", DEBUG_get_current_position());
  else
    fprintf(out, "?");

  //for (i = 0; i < (STACK_frame_count - 1); i++)
  for (i = 0;; i++)
  {
    context = GB_DEBUG.GetStack(i); //&STACK_frame[i];
    if (!context)
    	break;

    if (context->pc)
    {
      line = 0;
      if (calc_line_from_position(context->cp, context->fp, context->pc, &line))
        fprintf(out, " %s.?.?", context->cp->name);
      else
        fprintf(out, " %s.%s.%d", context->cp->name, context->fp->debug->name, line);
    }
    else if (context->cp)
      fprintf(out, " ?");
  }
}

/*static void command_where(const char *cmd)
{
	DEBUG_backtrace(_out);
}*/

static void print_local()
{
  int i;
  LOCAL_SYMBOL *lp;

  if (!FP || !FP->debug)
  	return;
  	
	for (i = 0; i < FP->debug->n_local; i++)
	{
		lp = &FP->debug->local[i];
		fprintf(_out, "%.*s ", lp->sym.len, lp->sym.name);
	}
	
	/*else
	{
		cmd++;

		for (i = 0; i < FP->debug->n_local; i++)
		{
			lp = &FP->debug->local[i];
			if (lp->sym.len == strlen(cmd) && strncasecmp(lp->sym.name, cmd, lp->sym.len) == 0)
			{
				fprintf(_out, "=");
				//fprintf(_out, "TUT\n");
				PRINT_value(_out, &BP[lp->value], TRUE);
				nl = FALSE;
				break;
			}
		}
	}*/
}


static void print_symbol(GLOBAL_SYMBOL *gp, bool is_static, bool is_public)
{
  if (CTYPE_get_kind(gp->ctype) != TK_VARIABLE && CTYPE_get_kind(gp->ctype) != TK_CONST)
    return;

  if (CTYPE_is_static(gp->ctype) && !is_static)
    return;

  if (!CTYPE_is_static(gp->ctype) && is_static)
    return;

  if (CTYPE_is_public(gp->ctype) && !is_public)
    return;

  if (!CTYPE_is_public(gp->ctype) && is_public)
    return;

  fprintf(_out, "%.*s ", gp->sym.len, gp->sym.name);
}


static void print_object()
{
  int i;
  GLOBAL_SYMBOL *gp;

	if (!CP || !CP->load)
		return;

	fprintf(_out, "S: ");

	for (i = 0; i < CP->load->n_global; i++)
	{
		gp = &CP->load->global[i];
		print_symbol(gp, TRUE, TRUE);
	}

	fprintf(_out, "s: ");

	for (i = 0; i < CP->load->n_global; i++)
	{
		gp = &CP->load->global[i];
		print_symbol(gp, TRUE, FALSE);
	}

  if (OP)
  {
    fprintf(_out, "D: ");

    for (i = 0; i < CP->load->n_global; i++)
    {
      gp = &CP->load->global[i];
      print_symbol(gp, FALSE, TRUE);
    }

    fprintf(_out, "d: ");

    for (i = 0; i < CP->load->n_global; i++)
    {
      gp = &CP->load->global[i];
      print_symbol(gp, FALSE, FALSE);
    }
  }
}


/*static void command_error(const char *cmd)
{
  if (Error)
  	GB_DEBUG.PrintError(_out, FALSE, TRUE);
    //ERROR_print_at(stdout);
  else
    fprintf(_out, "OK\n");
}*/

static void command_frame(const char *cmd)
{
}


static void command_eval(const char *cmd)
{
	static bool init = FALSE;
  EXPRESSION *expr;
  ERROR_INFO save;
  VALUE *val;
  int start, len;
  FILE *out;

	if (!init)
	{
  	GB.GetInterface("gb.eval", EVAL_INTERFACE_VERSION, &EVAL);
  	init = TRUE;
	}

	out = *cmd == '!' ? stdout : _out;

	len = strlen(cmd);
	for (start = 0; start < len; start++)
	{
		if (cmd[start] == '\t')
			break;
		if (*cmd != '!')
			fputc(cmd[start], _out);
	}
	
	if (start >= len)
		return;

	if (*cmd != '!')
		fprintf(_out, "\t");

  //ERROR_save(&save);
  GB_DEBUG.SaveError(&save);

	start++;
	EVAL.New(POINTER(&expr), &cmd[start], len - start);

  if (EVAL.Compile(expr))
    goto __ERROR;

  DEBUG_info.bp = BP;
  DEBUG_info.fp = FP;
  DEBUG_info.op = OP;
  DEBUG_info.cp = CP;

  val = (VALUE *)EVAL.Run(expr, GB_DEBUG.GetValue);
  if (!val)
    goto __ERROR;

	switch(*cmd)
	{		  
		case '?':
		  PRINT_value(out, val, TRUE);
		  break;
		  
		case '!':
		  PRINT_value(out, val, FALSE);
		  break;
		  
		case '#':
		  PRINT_object(out, val);
		  break;
	}

  goto __FREE;

__ERROR:
  //fprintf(_out, "%s\n", ERROR_info.msg);
  if (*cmd != '!')
  	fprintf(out, "!");
  GB_DEBUG.PrintError(out, TRUE, FALSE);

__FREE:
  EVAL.Free(POINTER(&expr));

  DEBUG_info.cp = NULL;

  //ERROR_restore(&save);
  GB_DEBUG.RestoreError(&save);
  
 	fprintf(out, "\n");
 	fflush(out);
}

static void command_symbol(const char *cmd)
{
  int start, len;

	len = strlen(cmd);
	for (start = 0; start < len; start++)
	{
		if (cmd[start] == '\t')
			break;
  	fputc(cmd[start], _out);
	}
	
	if (start >= len)
		return;

	fprintf(_out, "\t");

  DEBUG_info.bp = BP;
  DEBUG_info.fp = FP;
  DEBUG_info.op = OP;
  DEBUG_info.cp = CP;
  
  start++;
  PRINT_symbol(_out, &cmd[start], len - start);
 	
 	fprintf(_out, "\n");
 	fflush(_out);
}


static void debug_info()
{
	fprintf(_out, "*\t");
  
  if (Error)
    GB_DEBUG.PrintError(_out, TRUE, FALSE);
	
	fprintf(_out, "\t");
	
	DEBUG_backtrace(_out);
	fprintf(_out, "\t");
	
	print_local();
	fprintf(_out, "\t");
	
	print_object();
	fprintf(_out, "\n");
}

PUBLIC void DEBUG_main(boolean error)
{
  static DEBUG_TYPE last_command = TC_NONE;

  static DEBUG_COMMAND Command[] =
  {
    { "q", TC_NONE, command_quit, FALSE },
    { "n", TC_NEXT, command_next, FALSE },
    { "s", TC_STEP, command_step, FALSE },
    { "f", TC_FROM, command_from, FALSE },
    { "g", TC_GO, command_go, FALSE },
    { "+", TC_NONE, command_set_breakpoint, TRUE },
    { "-", TC_NONE, command_unset_breakpoint, TRUE },
    //{ "w", TC_NONE, command_where, TRUE },
    //{ "l", TC_NONE, command_local, TRUE },
    { "&", TC_NONE, command_symbol, TRUE },
    { "?", TC_NONE, command_eval, TRUE },
    { "!", TC_NONE, command_eval, TRUE },
    { "#", TC_NONE, command_eval, TRUE },
    //{ "e", TC_NONE, command_error, TRUE },
    { "@", TC_NONE, command_frame, TRUE },
    
    { NULL }
  };


  char *cmd = NULL;
  char cmdbuf[64];
  int len;
  DEBUG_COMMAND *tc = NULL;
  /*static int cpt = 0;*/

  Error = error;

  fflush(NULL);

  #ifdef DEBUG_ME
  fprintf(stderr, "DEBUG_main {\n");
  #endif

  if (_fifo)
    fprintf(_out, "!\n");

	debug_info();

  do
  {
    /*if (CP == NULL)
      printf("[]:");
    else
      printf("[%s%s]:", DEBUG_get_current_position(), Error ? "*" : "");*/

		GB.Signal(GB_SIGNAL_DEBUG_BREAK, 0);

    if (!_fifo)
    {
      fprintf(_out, "> ");
      fflush(NULL);
    }

		GB.FreeString(&cmd);

		for(;;)
		{
			*cmdbuf = 0;
	    if (fgets(cmdbuf, sizeof(cmdbuf), _in) == NULL)
	    	break;
      if (!*cmdbuf)
        continue;
			GB.AddString(&cmd, cmdbuf, 0);
			if (cmd[GB.StringLength(cmd) - 1] == '\n')
				break;
		}

    len = GB.StringLength(cmd);
    if (len > 0 && cmd[len - 1] == '\n')
    {
      len--;
      cmd[len] = 0;
    }
    
    #ifdef DEBUG_ME
    fprintf(stderr, "--> %s\n", cmd);
    #endif

    if (len == 0)
    {
      if (last_command == TC_NONE)
        continue;

      for (tc = Command; tc->pattern; tc++)
      {
        if (tc->type == last_command)
        {
          (*tc->func)(cmd);
          break;
        }
      }
    }
    else
    {
      for (tc = Command; tc->pattern; tc++)
      {
        if (strncasecmp(tc->pattern, cmd, strlen(tc->pattern)) == 0)
        {
          if (tc->type != TC_NONE)
            last_command = tc->type;
          (*tc->func)(cmd);
          break;
        }
      }
    }

    if (tc->pattern == NULL)
      WARNING("Unknown command: %s", cmd);

    fflush(_out);
  }
  while (last_command == TC_NONE || tc->pattern == NULL || tc->loop);

  GB.FreeString(&cmd);

  #ifdef DEBUG_ME
  fprintf(stderr, "} DEBUG_main\n");
  #endif
}



PUBLIC void DEBUG_breakpoint(int id)
{
  DEBUG_main(FALSE);
}


PUBLIC const char *DEBUG_get_position(CLASS *cp, FUNCTION *fp, PCODE *pc)
{
  ushort line = 0;

  if (fp != NULL && fp->debug)
    calc_line_from_position(cp, fp, pc, &line);

  sprintf(DEBUG_buffer, "%.64s.%.64s.%d",
    cp ? cp->name : "?",
    (fp && fp->debug) ? fp->debug->name : "?",
    line);

  return DEBUG_buffer;
}


PUBLIC const char *DEBUG_get_current_position(void)
{
  return DEBUG_get_position(CP, FP, PC);
}


PUBLIC void DEBUG_where(void)
{
  fprintf(_out ? _out : stderr, "%s: ", DEBUG_get_current_position());
}


PUBLIC void DEBUG_welcome(void)
{
  if (!_fifo)
    fprintf(_out, DEBUG_WELCOME);
}
