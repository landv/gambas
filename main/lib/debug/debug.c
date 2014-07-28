/***************************************************************************

	debug.c

	(c) 2000-2013 Benoît Minisini <gambas@users.sourceforge.net>

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2, or (at your option)
	any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
	MA 02110-1301, USA.

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

//#define DEBUG_ME

DEBUG_INFO DEBUG_info = { 0 };
GB_DEBUG_INTERFACE *DEBUG_interface;
char DEBUG_buffer[DEBUG_BUFFER_MAX + 1];

static DEBUG_BREAK *Breakpoint;
static bool Error;

static EVAL_INTERFACE EVAL;

static int _fdr;
static int _fdw;
static FILE *_out;
static FILE *_in;
static bool _fifo;

#define EXEC_current (*(STACK_CONTEXT *)GB_DEBUG.GetExec())

#ifdef DEBUG_ME
#define WARNING(_msg, ...) fprintf(stderr, "W\t" _msg "\n", ##__VA_ARGS__)
#define INFO(_msg, ...) fprintf(stderr, "I\t" _msg "\n", ##__VA_ARGS__)
#else
#define WARNING(_msg, ...) fprintf(_out, "W\t" _msg "\n", ##__VA_ARGS__)
#define INFO(_msg, ...) fprintf(_out, "I\t" _msg "\n", ##__VA_ARGS__)
#endif

static void init_eval_interface()
{
	static bool init = FALSE;

	if (!init)
	{
		GB.GetInterface("gb.eval", EVAL_INTERFACE_VERSION, &EVAL);
		init = TRUE;
	}
}

void DEBUG_break_on_next_line(void)
{
	DEBUG_info.stop = TRUE;
	DEBUG_info.leave = FALSE;
	DEBUG_info.fp = NULL;
	DEBUG_info.bp = NULL;
	DEBUG_info.pp = NULL;
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


bool DEBUG_calc_line_from_position(CLASS *class, FUNCTION *func, PCODE *addr, ushort *line)
{
	int lo, hi;
	int mid;
	ushort pos = addr - func->code;
	ushort *post;

	if (func->debug)
	{
		post = func->debug->pos;
		
		lo = 0;
		hi = func->debug->nline - 1;
		
		while (lo < hi)
		{
			mid = (lo + hi) >> 1;
			if (pos < post[mid])
			{
				hi = mid;
			}
			else if (pos >= post[mid + 1])
			{
				lo = mid + 1;
			}
			else
			{
				*line = mid + func->debug->line;
				return FALSE;
			}
		}
	}

	return TRUE;
}


static bool calc_position_from_line(CLASS *class, ushort line, FUNCTION **function, PCODE **addr)
{
	int i;
	ushort pos, pos_after;
	FUNCTION *func = NULL;
	FUNC_DEBUG *debug = NULL;

	for (i = 0; i < class->load->n_func; i++)
	{
		func = &class->load->func[i];
		debug = func->debug;
		//fprintf(stderr, "calc_position_from_line: %s (%d -> %d) / %d\n", debug->name, debug->line, debug->line + debug->nline - 1, line);
		if (debug && line >= debug->line && line < (debug->line + debug->nline))
			break;
	}

	if (i >= class->load->n_func)
		return TRUE;

	//fprintf(stderr, "calc_position_from_line: %s OK\n", debug->name);

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


DEBUG_INFO *DEBUG_init(GB_DEBUG_INTERFACE *debug, bool fifo, const char *fifo_name)
{
	char path[DEBUG_FIFO_PATH_MAX];
	char name[16];
	//int i;

	//if (!EXEC_debug)
	//  return;

	DEBUG_interface = debug;
	_fifo = fifo;

	if (_fifo)
	{
		if (!fifo_name)
		{
			sprintf(name, "%d", getppid());
			fifo_name = name;
		}
		
		snprintf(path, sizeof(path), "/tmp/gambas.%d/%s.out", getuid(), fifo_name);
		
		/*for (i = 0; i < 20; i++)
		{
			_fdr = open(path, O_RDONLY | O_NONBLOCK);
			if (_fdr >= 0)
				break;
			usleep(10000);
		}
		if (_fdr < 0)
			return NULL;*/
		
		_fdr = open(path, O_RDONLY | O_CLOEXEC);
		if (_fdr < 0)
			return NULL;
		
		snprintf(path, sizeof(path), "/tmp/gambas.%d/%s.in", getuid(), fifo_name);
		
		_fdw = open(path, O_WRONLY | O_CLOEXEC);
		if (_fdw < 0)
			return NULL;
		
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

void DEBUG_exit(void)
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

	if (brk->addr || !CLASS_is_loaded(brk->class))
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
		WARNING("Cannot set breakpoint: cannot calculate position");
		//fprintf(_out, "Cannot calc position from line number\n");
		return TRUE;
	}

	if (!PCODE_is_breakpoint(*addr))
	{
		//WARNING("Cannot set breakpoint: Not a line beginning: %04d: %04X", addr - func->code, *addr);
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


static bool set_breakpoint(CLASS *class, ushort line)
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


static bool unset_breakpoint(CLASS *class, ushort line)
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


void DEBUG_init_breakpoints(CLASS *class)
{
	int i;
	DEBUG_BREAK *brk;

	#ifdef DEBUG_ME
	fprintf(stderr, "DEBUG_init_breakpoints: %p %s\n", class, class->name);
	#endif
	
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

static void print_local()
{
	int i;
	FUNCTION *fp;
	LOCAL_SYMBOL *lp;

	fp = DEBUG_info.fp;
	
	if (!fp || !fp->debug)
		return;
		
	for (i = 0; i < fp->debug->n_local; i++)
	{
		lp = &fp->debug->local[i];
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
	CLASS *cp = DEBUG_info.cp;
	void *op = DEBUG_info.op;

	if (!cp || !cp->load)
		return;

	fprintf(_out, "S: ");

	for (i = 0; i < cp->load->n_global; i++)
	{
		gp = &cp->load->global[i];
		print_symbol(gp, TRUE, TRUE);
	}

	fprintf(_out, "s: ");

	for (i = 0; i < cp->load->n_global; i++)
	{
		gp = &cp->load->global[i];
		print_symbol(gp, TRUE, FALSE);
	}

	if (op)
	{
		fprintf(_out, "D: ");

		for (i = 0; i < cp->load->n_global; i++)
		{
			gp = &cp->load->global[i];
			print_symbol(gp, FALSE, TRUE);
		}

		fprintf(_out, "d: ");

		for (i = 0; i < cp->load->n_global; i++)
		{
			gp = &cp->load->global[i];
			print_symbol(gp, FALSE, FALSE);
		}
	}
}

static void command_quit(const char *cmd)
{
	exit(1);
}

static void command_go(const char *cmd)
{
	GB.Component.Signal(GB_SIGNAL_DEBUG_CONTINUE, 0);

	DEBUG_info.stop = FALSE;
	DEBUG_info.leave = FALSE;
	DEBUG_info.fp = NULL;
	DEBUG_info.bp = NULL;
	DEBUG_info.pp = NULL;
}

static void command_step(const char *cmd)
{
	GB.Component.Signal(GB_SIGNAL_DEBUG_FORWARD, 0);
	DEBUG_break_on_next_line();
}

static void command_next(const char *cmd)
{
	GB.Component.Signal(GB_SIGNAL_DEBUG_FORWARD, 0);
	DEBUG_info.stop = TRUE;
	DEBUG_info.leave = FALSE;
	DEBUG_info.fp = FP;
	DEBUG_info.bp = BP;
	DEBUG_info.pp = PP;
}

static void command_from(const char *cmd)
{
	STACK_CONTEXT *sc = GB_DEBUG.GetStack(0); //STACK_get_current();

	if (sc && sc->pc)
	{
		GB.Component.Signal(GB_SIGNAL_DEBUG_FORWARD, 0);
		DEBUG_info.stop = TRUE;
		DEBUG_info.leave = FALSE;
		DEBUG_info.fp = sc->fp;
		DEBUG_info.bp = sc->bp;
		DEBUG_info.pp = sc->pp;
	}
	else
	{
		GB.Component.Signal(GB_SIGNAL_DEBUG_FORWARD, 0);
		DEBUG_info.stop = TRUE;
		DEBUG_info.leave = TRUE;
		DEBUG_info.fp = FP;
		DEBUG_info.bp = BP;
		DEBUG_info.pp = PP;
	}
}


static void command_set_breakpoint(const char *cmd)
{
	char class_name[64];
	ushort line;
	//CLASS *class;

	if (sscanf(cmd, "+%64[^.].%hu", class_name, &line) != 2)
		WARNING("Cannot set breakpoint: syntax error");
	else
	{
		//class = (CLASS *)GB.FindClassLocal(class_name);
		//CLASS_load_without_init(class);
		//fprintf(stderr, "command_set_breakpoint: %s %s\n", class->name, class->component ? class->component->name : "?");
		set_breakpoint((CLASS *)GB_DEBUG.FindClass(class_name), line);
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
		unset_breakpoint((CLASS *)GB_DEBUG.FindClass(class_name), line);
	}
}


void DEBUG_backtrace(FILE *out)
{
	int i, n;
	STACK_CONTEXT *context;
	ushort line;

	if (CP)
		fprintf(out, "%s", DEBUG_get_current_position());
	else
		fprintf(out, "?");

	//for (i = 0; i < (STACK_frame_count - 1); i++)
	n = 0;
	for (i = 0;; i++)
	{
		context = GB_DEBUG.GetStack(i); //&STACK_frame[i];
		if (!context)
			break;

		if (context->pc)
		{
			line = 0;
			if (DEBUG_calc_line_from_position(context->cp, context->fp, context->pc, &line))
				n += fprintf(out, " %s.?.?", context->cp->name);
			else
				n += fprintf(out, " %s.%s.%d", context->cp->name, context->fp->debug->name, line);
		}
		else if (context->cp)
			n += fprintf(out, " ?");
		
		if (n >= (DEBUG_OUTPUT_MAX_SIZE / 2))
		{
			fprintf(out, " ...");
			break;
		}
	}
}

static void debug_info()
{
	fprintf(_out, "*[%d]\t", getpid());
	
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

static void command_frame(const char *cmd)
{
	int i;
	int frame;
	STACK_CONTEXT *context = NULL;
	
	if (cmd) 
	{
		frame = atoi(&cmd[1]);
		//fprintf(_out, "switching to frame %d\n", frame);
	
		if (frame > 0)
		{
			for (i = 0;; i++)
			{
				context = GB_DEBUG.GetStack(i);
				if (!context)
					break;
				if (!context->pc && !context->cp)
					continue;
				
				frame--;
				if (!frame)
				{
					DEBUG_info.bp = context->bp;
					DEBUG_info.pp = context->pp;
					DEBUG_info.fp = context->fp;
					DEBUG_info.op = context->op;
					DEBUG_info.cp = context->cp;
					break;
				}
			}
		}
	}

	if (!context)
	{
		DEBUG_info.bp = BP;
		DEBUG_info.pp = PP;
		DEBUG_info.fp = FP;
		DEBUG_info.op = OP;
		DEBUG_info.cp = CP;
	}

	debug_info();
}

static void command_eval(const char *cmd)
{
	EXPRESSION *expr;
	ERROR_INFO save_error = { 0 };
	ERROR_INFO save_last = { 0 };
	DEBUG_INFO save_debug;
	VALUE *val;
	int start, len;
	FILE *out;
	const char *name;
	int ret;

	init_eval_interface();

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

	GB_DEBUG.SaveError(&save_error, &save_last);
	save_debug = DEBUG_info;

	start++;
	EVAL.New(POINTER(&expr), &cmd[start], len - start);

	if (EVAL.Compile(expr, *cmd == '='))
	{
		if (*cmd != '!')
			fprintf(_out, "!");
		fputs(expr->error, out);
		goto __END;
	}
	
	GB_DEBUG.EnterEval();
	val = (VALUE *)EVAL.Run(expr, GB_DEBUG.GetValue);
	GB_DEBUG.LeaveEval();
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
			
		case '=':
			if (!EVAL.GetAssignmentSymbol(expr, &name, &len))
			{
				ret = GB_DEBUG.SetValue(name, len, val);
				if (ret == GB_DEBUG_SET_ERROR)
					goto __ERROR;
				else if (ret == GB_DEBUG_SET_READ_ONLY)
				{
					fprintf(out, "!%.*s is read-only", len, name);
					goto __END;
				}
			}
			fprintf(out, "OK");
			break;
	}

	goto __END;

__ERROR:

	if (*cmd != '!')
		fprintf(out, "!");
	GB_DEBUG.PrintError(out, TRUE, FALSE);

__END:

	EVAL.Free(POINTER(&expr));
	DEBUG_info = save_debug; //.cp = NULL;
	GB_DEBUG.RestoreError(&save_error, &save_last);
	
	fprintf(out, "\n");
	fflush(out);
}


static void command_symbol(const char *cmd)
{
	int start, len;
	DEBUG_INFO save_debug = DEBUG_info;

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

	/*DEBUG_info.bp = BP;
	DEBUG_info.fp = FP;
	DEBUG_info.op = OP;
	DEBUG_info.cp = CP;*/
	
	start++;
	PRINT_symbol(_out, &cmd[start], len - start);
	
	fprintf(_out, "\n");
	fflush(_out);
	
	DEBUG_info = save_debug;
}


static void command_break_on_error(const char *cmd)
{
	GB_DEBUG.BreakOnError(cmd[1] == '+');
}

void DEBUG_main(bool error)
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
		{ "&", TC_NONE, command_symbol, TRUE },
		{ "?", TC_NONE, command_eval, TRUE },
		{ "!", TC_NONE, command_eval, TRUE },
		{ "#", TC_NONE, command_eval, TRUE },
		{ "=", TC_NONE, command_eval, TRUE },
		{ "@", TC_NONE, command_frame, TRUE },
		{ "b", TC_NONE, command_break_on_error, TRUE },

		{ NULL }
	};

	static bool first = TRUE;
	char *cmd = NULL;
	char cmdbuf[64];
	int len;
	DEBUG_COMMAND *tc = NULL;
	/*static int cpt = 0;*/

	Error = error;

	fflush(_out);

	#ifdef DEBUG_ME
	fprintf(stderr, "DEBUG_main {\n");
	#endif

	if (_fifo)
	{
		fprintf(_out, first ? "!!\n" : "!\n");
		first = FALSE;
	}

	command_frame(NULL);

	do
	{
		/*if (CP == NULL)
			printf("[]:");
		else
			printf("[%s%s]:", DEBUG_get_current_position(), Error ? "*" : "");*/

		GB.Component.Signal(GB_SIGNAL_DEBUG_BREAK, 0);

		if (!_fifo)
		{
			fprintf(_out, "> ");
			fflush(_out);
		}

		GB.FreeString(&cmd);

		for(;;)
		{
			*cmdbuf = 0;
			errno = 0;
			if (fgets(cmdbuf, sizeof(cmdbuf), _in) == NULL && errno != EINTR)
				break;
			if (!*cmdbuf)
				continue;
			cmd = GB.AddString(cmd, cmdbuf, 0);
			if (cmd[GB.StringLength(cmd) - 1] == '\n')
				break;
		}

		len = GB.StringLength(cmd);
		
		// A null string command means an I/O error
		if (len == 0)
		{
			fprintf(stderr, "warning: debugger I/O error: %s\n", strerror(errno));
			exit(1);
		}
		
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



void DEBUG_breakpoint(int id)
{
	DEBUG_main(FALSE);
}


const char *DEBUG_get_position(CLASS *cp, FUNCTION *fp, PCODE *pc)
{
	if (pc)
	{
		ushort line = 0;

		if (fp != NULL && fp->debug)
			DEBUG_calc_line_from_position(cp, fp, pc, &line);

		snprintf(DEBUG_buffer, sizeof(DEBUG_buffer), "%.64s.%.64s.%d",
			cp ? cp->name : "?",
			(fp && fp->debug) ? fp->debug->name : "?",
			line);
	}
	else
	{
		snprintf(DEBUG_buffer, sizeof(DEBUG_buffer), "%.64s.%.64s",
			cp ? cp->name : "?",
			(fp && fp->debug) ? fp->debug->name : "?");
	}

	return DEBUG_buffer;
}

const char *DEBUG_get_profile_position(CLASS *cp, FUNCTION *fp, PCODE *pc)
{
	static uint prof_index = 0;
	const char *name;
	const char *func;
	char buffer[16], buffer2[16];
	
	func = "?";
	
	if (cp)
	{
		if (cp->load && cp->load->prof)
		{
			if (cp->load->prof[0] == 0)
			{
				prof_index++;
				cp->load->prof[0] = prof_index;
				name = cp->name;
			}
			else
			{
				sprintf(buffer, "%u", cp->load->prof[0]);
				name = buffer;
			}

			if (fp && fp->debug)
			{
				int i = fp->debug->index + 1;
				if (cp->load->prof[i] == 0)
				{
					prof_index++;
					cp->load->prof[i] = prof_index;
					func = fp->debug->name;
				}
				else
				{
					sprintf(buffer2, "%u", cp->load->prof[i]);
					func = buffer2;
				}
			}
			else
				func = "?";
		}
		else
			name = cp->name;
	}
	else
		name = "?";
	
	if (pc)
	{
		ushort line = 0;

		if (fp != NULL && fp->debug)
			DEBUG_calc_line_from_position(cp, fp, pc, &line);

		snprintf(DEBUG_buffer, sizeof(DEBUG_buffer), "%.64s.%.64s.%d", name, func, line);
	}
	else
	{
		snprintf(DEBUG_buffer, sizeof(DEBUG_buffer), "%.64s.%.64s", name, func);
	}

	return DEBUG_buffer;
}


const char *DEBUG_get_current_position(void)
{
	return DEBUG_get_position(CP, FP, PC);
}


void DEBUG_where(void)
{
	fprintf(_out ? _out : stderr, "%s: ", DEBUG_get_current_position());
}


void DEBUG_welcome(void)
{
	if (!_fifo)
		fprintf(_out, DEBUG_WELCOME);
}
