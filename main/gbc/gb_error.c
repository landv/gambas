/***************************************************************************

  gb_error.c

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

#define __GB_ERROR_C

#include "gb_common.h"
#include <stdarg.h>

#include "gb_buffer.h"
#include "gb_error.h"


ERROR_INFO ERROR_info;
bool ERROR_translate = FALSE;

static const char *_message[] =
{
  /*  0 E_UNKNOWN */ "Unknown error",
  /*  1 E_MEMORY */ "Out of memory",
  /*  2 E_OPEN */ "Cannot open file: &1",
  /*  3 E_READ */ "Cannot read file: &1",
  /*  4 E_SYNTAX */ "Syntax error",
  /*  5 E_UNEXPECTED */ "Unexpected &1",
	/*  6 E_EXPECTED */ "&1 expected",
	/*  7 E_MISSING */ "Missing &1",
	/*  8 E_SYNTAX_MISSING */ "Syntax error. Missing &1",
	/*  9 E_TOOLONG */ "File name is too long",
  NULL
};

ERROR_CONTEXT *ERROR_current = NULL;


#if 0
void ERROR_enter(ERROR_CONTEXT *err)
{
  CLEAR(err);
  err->prev = ERROR_current;
  ERROR_current = err;
}


void ERROR_leave(ERROR_CONTEXT *err)
{
  if (err->prev != ERROR_LEAVE_DONE)
  {
    /*ERROR_panic("ERROR_leave already done");*/

    ERROR_current = err->prev;
    err->prev = ERROR_LEAVE_DONE;
  }
}
#endif

char *ERROR_get(void)
{
  /*
  if (code > 0 && code < 256)
    return strerror(code);
  else
    return ERROR_Message[code - 256];
  */
  return strerror(errno);
}


void ERROR_define(const char *pattern, const char *arg[])
{
  int i, n;
  uchar c;
  bool subst;

  void _add_char(uchar c)
  {
    if (n >= MAX_ERROR_MSG)
      return;

    ERROR_info.msg[n++] = c;
  }

  void _add_string(const char *s)
  {
    while (*s)
    {
      _add_char(*s);
      s++;
    }
  }

  if ((intptr_t)pattern > 0 && (intptr_t)pattern < 256)
  {
    ERROR_info.code = (int)(intptr_t)pattern;
    pattern = _message[(int)(intptr_t)pattern];
  }
  else
    ERROR_info.code = -1;

  n = 0;

	subst = FALSE;

	if (ERROR_translate)
	{
		int nsubst = 0;
		
		_add_string(pattern);

		for (;;)
		{
			c = *pattern++;
			if (c == 0)
				break;

			if (subst)
			{
				if (c >= '1' && c <= '4')
				{
					c -= '0';
					if (c > nsubst) nsubst = c;
				}
				subst = FALSE;
			}
			else
			{
				if (c == '&')
					subst = TRUE;
			}
		}
		
		for (i = 0; i < nsubst; i++)
		{
			_add_char('\t');
			_add_string(arg[i]);
		}
	}
	else
	{
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

  _add_char(0);
}

void PROPAGATE()
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

  if (ERROR_current == NULL)
    ERROR_panic("Cannot propagate error. No error handler.");

  err = ERROR_current;
  ERROR_leave(ERROR_current);
  longjmp(err->env, 1);
}

void THROW(const char *code, ...)
{
  va_list args;
  int i;
  const char *arg[4];

  va_start(args, code);

  for (i = 0; i < 4; i++)
    arg[i] = va_arg(args, const char *);

  ERROR_define(code, arg);
  PROPAGATE();
}

void ERROR_panic(const char *error, ...)
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
  abort();
}


void ERROR_print_at(FILE *where)
{
  fprintf(where, "%s\n", ERROR_info.msg);
}

void ERROR_print(void)
{
  ERROR_print_at(stderr);
}

void TRACE_where(void)
{
}

void ERROR_warning(const char *warning, ...)
{
  va_list args;

  va_start(args, warning);

  fflush(NULL);

  fprintf(stderr, "gbc: warning: ");
  vfprintf(stderr, warning, args);
  putc('\n', stderr);
}
