/***************************************************************************

  error.c

  Errors management routines

  (c) 2000-2006 Benoît Minisini <gambas@users.sourceforge.net>

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

#define _ERROR_C

#include "gb_common.h"
#include <stdarg.h>
#include <ctype.h>

#include "gb_buffer.h"
#include "gb_error.h"
#include "gbx_debug.h"

#include "gbx_exec.h"
#include "gbx_api.h"


PUBLIC ERROR_INFO ERROR_info;


static const char *_message[128] =
{
  /*  0 E_UNKNOWN */ "Unknown error",
  /*  1 E_MEMORY */ "Out of memory",
  /*  2 E_CLASS */ "Cannot load class '&1': &2&3",
  /*  3 E_STACK */ "Stack overflow",
  /*  4 E_NEPARAM */ "Not enough arguments",
  /*  5 E_TMPARAM */ "Too many arguments",
  /*  6 E_TYPE */ "Type mismatch: wanted &1, got &2 instead",
  /*  7 E_OVERFLOW */ "Overflow",
  /*  8 E_ILLEGAL */ "Illegal instruction",
  /*  9 E_NFUNC */ "Not a function",
  /* 10 E_CSTATIC */ "Class '&1' is static",
  /* 11 E_NSYMBOL */ "Unknown symbol '&1' in class '&2'",
  /* 12 E_NOBJECT */ "Not an object",
  /* 13 E_NULL */ "Null object",
  /* 14 E_STATIC */ "'&1.&2' is static",
  /* 15 E_NREAD */ "'&1.&2' is write only",
  /* 16 E_NWRITE */ "'&1.&2' is read only",
  /* 17 E_NPROPERTY */ "'&1.&2' is not a property",
  /* 18 E_NRETURN */ "No return value",
  /* 19 E_MATH*/ "Mathematic error",
  /* 20 E_ARG */ "Bad argument",
  /* 21 E_BOUND */ "Out of bounds",
  /* 22 E_NDIM */ "Bad number of dimensions",
  /* 23 E_NARRAY */ "Not an array",
  /* 24 E_MAIN */ "No startup method",
  /* 25 E_NNEW */ "No instanciation method",
  /* 26 E_ZERO */ "Division by zero",
  /* 27 E_LIBRARY */ "Cannot load component '&1': &2",
  /* 28 E_EVENT */ "Bad event handler in &1.&2(): &3",
  /* 29 E_IOBJECT */ "Invalid object",
  /* 30 E_ENUM */ "Not an enumeration",
  /* 31 E_UCONV */ "Unsupported string conversion",
  /* 32 E_CONV */ "Bad string conversion",
  /* 33 E_DATE */ "Invalid date",
  /* 34 E_BADPATH */ "Invalid path",
  /* 35 E_OPEN */ "Cannot open file '&1': &2",
  /* 36 E_PROJECT */ "Bad project file: line &1: &2",
  /* 37 E_FULL */ "Device is full",
  /* 38 E_EXIST */ "File already exists",  /* &1 */
  /* 39 E_EOF */ "End of file",
  /* 40 E_FORMAT */ "Bad format string",
  /* 41 E_DYNAMIC */ "'&1.&2' is not static",
  /* 42 E_SYSTEM */ "System error. &1",
  /* 43 E_ACCESS */ "Access forbidden",
  /* 44 E_TOOLONG */ "File name is too long",
  /* 45 E_NEXIST */ "File or directory does not exist", /* &1 */
  /* 46 E_DIR */ "File is a directory", /* &1 */
  /* 47 E_READ */ "Read error",
  /* 48 E_WRITE */ "Write error",
  /* 49 E_NDIR */ "Not a directory: &1",
  /* 50 E_REGEXP */ "Bad regular expression: &1",
  /* 51 E_ARCH */ "Bad archive: &1",
  /* 52 E_REGISTER */ "Cannot register class '&1'",
  /* 53 E_CLOSED */ "File is closed",
  /* 54 E_VIRTUAL */ "Bad use of virtual class",
  /* 55 E_STOP */ "STOP instruction encountered",
  /* 56 E_STRING */ "Too many simultaneous new strings",
  /* 57 E_EVAL */ "Bad expression: &1",
  /* 58 E_LOCK */ "File is locked",
  /* 59 E_PARENT */ "No parent class",
  /* 60 E_EXTLIB */ "Cannot find dynamic library '&1': &2",
  /* 61 E_EXTSYM */ "Cannot find symbol '&2' in dynamic library '&1'",
  NULL
};

static ERROR_CONTEXT *_current = NULL;

/*
static void *_must_free[MAX_MUST_FREE] = { 0 };
static int _must_free_index = 0;
*/

/*PUBLIC void ERROR_clear(void)
{
  errno = 0;
  CLEAR(&ERROR_info);
}*/


PUBLIC void ERROR_enter(ERROR_CONTEXT *err)
{
  /*if (!err)
    ERROR_panic("ERROR_enter: alloca() has failed!");*/
  CLEAR(err);
  err->prev = _current;
  _current = err;
}


PUBLIC void ERROR_leave(ERROR_CONTEXT *err)
{
  if (err->prev != ERROR_LEAVE_DONE)
  {
    /*ERROR_panic("ERROR_leave already done");*/

    _current = err->prev;
    err->prev = ERROR_LEAVE_DONE;
  }
}


PUBLIC const char *ERROR_get(void)
{
  /*
  if (code > 0 && code < 256)
    return strerror(code);
  else
    return ERROR_Message[code - 256];
  */
  return strerror(errno);
}


PUBLIC void ERROR_define(const char *pattern, char *arg[])
{
  int n;
  uchar c;
  boolean subst;

  void _add_char(uchar c)
  {
    if (n >= MAX_ERROR_MSG)
      return;

    if (isspace(c))
      c = ' ';

    ERROR_info.msg[n++] = c;
  }

  void _add_string(const char *s)
  {
    if (!s)
      s = "";

    while (*s)
    {
      _add_char(*s);
      s++;
    }
  }

  if ((long)pattern >= 0 && (long)pattern < 256)
  {
    ERROR_info.code = (long)pattern;
    pattern = _message[(long)pattern];
  }
  else if ((long)pattern == E_ABORT)
  {
    ERROR_info.code = E_ABORT;
  	pattern = "";
	}
	else
    ERROR_info.code = E_CUSTOM;

  n = 0;

  if (arg)
  {
    subst = FALSE;

    for (;;)
    {
      c = *pattern++;
      if (c == 0)
        break;

      if (subst)
      {
        if (c >= '1' && c <= '4')
          _add_string(arg[c - '1']);
        else
        {
          _add_char('&');
          _add_char(c);
        }
        subst = FALSE;
      }
      else
      {
        if (c == '&')
          subst = TRUE;
        else
          _add_char(c);
      }
    }
  }
  else
  {
    _add_string(pattern);
  }

  _add_char(0);

  ERROR_info.cp = CP;
  ERROR_info.fp = FP;
  ERROR_info.pc = PC;
}

PUBLIC void PROPAGATE()
{
  ERROR_CONTEXT *err;

  /*
  if (_must_free_index)
  {
    for (i = 0; i < _must_free_index; i++)
      OBJECT_UNREF(&_must_free[i], "PROPAGATE");

    _must_free_index = 0;
  }
  */

  if (_current == NULL)
    ERROR_panic("Cannot propagate error. No error handler.");

  err = _current;
  ERROR_leave(_current);
  longjmp(err->env, 1);
}

/*
PUBLIC void ERROR(long code, ...)
{
  va_list args;
  int i;
  char *arg[4];

  va_start(args, code);

  for (i = 0; i < 4; i++)
    arg[i] = va_arg(args, char *);

  ERROR_define((char *)code, arg);
}
*/

/*
static void throw(long code, va_list args)
{
  int i;
  char *arg[4];

  for (i = 0; i < 4; i++)
    arg[i] = va_arg(args, char *);

  ERROR_define((char *)code, arg);
  PROPAGATE();
}
*/

PUBLIC void THROW(long code, ...)
{
  va_list args;
  int i;
  char *arg[4];

  va_start(args, code);

  for (i = 0; i < 4; i++)
    arg[i] = va_arg(args, char *);

  ERROR_define((char *)code, arg);
  PROPAGATE();
}


PUBLIC void THROW_SYSTEM(int err, const char *path)
{
  switch(err)
  {
    case ENOENT:
      THROW(E_NEXIST, path);

    case EISDIR:
      THROW(E_DIR, path);

    case ENOTDIR:
      THROW(E_NDIR, path);

    case ENOMEM:
      THROW(E_MEMORY);

    case EACCES:
      THROW(E_ACCESS);

    case ENAMETOOLONG:
      THROW(E_TOOLONG);

    case ENOSPC:
      THROW(E_FULL);

    case EEXIST:
      THROW(E_EXIST, path);

    default:
      THROW(E_SYSTEM, strerror(err));
  }
}

/*
PUBLIC void THROW_LIBRARY()
{
  int n;
  char buf[512];
  char *msg = GAMBAS_Error;

  if (FP != NULL)
  {
    sprintf(buf, "[%s] ", TRACE_get_current_position());
    add_error_message(buf, FALSE);
  }

  sprintf(buf, "#%d: ", E_FROMLIB);
  add_error_message(buf, FALSE);

  if ((long)msg > 0 && (long)msg < 255)
    n = snprintf(buf, sizeof(buf), ERROR_Message[(long)msg]);
  else
    n = snprintf(buf, sizeof(buf), msg);

  ERROR_code = E_FROMLIB;
  throw_error(msg);
}
*/

PUBLIC void ERROR_panic(const char *error, ...)
{
  va_list args;

  va_start(args, error);

  fflush(NULL);

  fprintf(stderr, "\n"
                  "** INTERNAL ERROR **\n"
                  );
  vfprintf(stderr, error, args);
  fprintf(stderr, "\n");
  if (ERROR_info.code)
  {
    fprintf(stderr, "\n");
    ERROR_print();
    fprintf(stderr, "\n");
  }
  fprintf(stderr, "** Program aborting... Sorry... :-(\n\n");
  /*abort();*/
  _exit(1);
}


PUBLIC void ERROR_print_at(FILE *where, bool msgonly, bool newline)
{
  if (!ERROR_info.code)
    return;

	if (!msgonly)
	{
		if (ERROR_info.cp && ERROR_info.fp && ERROR_info.pc)
			fprintf(where, "%s: ", DEBUG_get_position(ERROR_info.cp, ERROR_info.fp, ERROR_info.pc));
		else
			fprintf(where, "ERROR: ");
		/*if (ERROR_info.code > 0 && ERROR_info.code < 256)
			fprintf(where, "%ld:", ERROR_info.code);*/
		if (ERROR_info.code > 0)
			fprintf(where, "#%ld: ", ERROR_info.code);
	}

  fprintf(where, "%s", ERROR_info.msg);
  if (newline)
  	fputc('\n', where);
}

PUBLIC void ERROR_print(void)
{
  static bool lock = FALSE;

  ERROR_print_at(stderr, FALSE, TRUE);

  if (EXEC_main_hook_done && !EXEC_debug && EXEC_Hook.error && !lock)
  {
    lock = TRUE;
    GAMBAS_DoNotRaiseEvent = TRUE;
    HOOK(error)(ERROR_info.code, ERROR_info.msg, DEBUG_get_position(ERROR_info.cp, ERROR_info.fp, ERROR_info.pc));
    lock = FALSE;
  }
}

PUBLIC void ERROR_save(ERROR_INFO *save)
{
  save->code = ERROR_info.code;
  save->cp = ERROR_info.cp;
  save->fp = ERROR_info.fp;
  save->pc = ERROR_info.pc;
  strcpy(save->msg, ERROR_info.msg);
}

PUBLIC void ERROR_restore(ERROR_INFO *save)
{
  ERROR_info.code = save->code;
  ERROR_info.cp = save->cp;
  ERROR_info.cp = save->fp;
  ERROR_info.pc = save->pc;
  strcpy(ERROR_info.msg, save->msg);
}
