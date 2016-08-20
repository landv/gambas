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

#include "main.h"
#include "gb_common.h"
#include <stdarg.h>
#include "eval.h"
#include "gb_error.h"

ERROR_CONTEXT *ERROR_current = NULL;

void ERROR_clear(void)
{
  errno = 0;
}

void ERROR_reset(ERROR_INFO *info)
{
	if (!info->code)
		return;

	info->code = 0;
	if (info->free)
	{
		GB.FreeString(&info->msg);
		info->free = FALSE;
	}

	info->msg = NULL;
}


void ERROR_propagate()
{
	if (ERROR_in_catch(ERROR_current))
		ERROR_leave(ERROR_current);
  longjmp(ERROR_current->env, 1);
}



PUBLIC char *ERROR_get(void)
{
  /*
  if (code > 0 && code < 256)
    return strerror(code);
  else
    return ERROR_Message[code - 256];
  */
  return strerror(errno);
}

PUBLIC void THROW(const char *msg)
{
	GB.FreeString(&EVAL->error);
	EVAL->error = GB.NewZeroString(msg);
  ERROR_propagate();
}

static const char *_error_arg;

static void get_error_arg(int index, char **str, int *len)
{
	*str = (char *)_error_arg;
	*len = strlen(_error_arg);
}

PUBLIC void THROW2(const char *pattern, const char *msg)
{
	GB.FreeString(&EVAL->error);
	_error_arg = msg;
	EVAL->error = GB.NewZeroString(GB.SubstString(pattern, strlen(pattern), get_error_arg));
  ERROR_propagate();
}

void ERROR_panic(const char *error, ...)
{
  va_list args;

  va_start(args, error);

  fflush(NULL);

  fprintf(stderr, "\n** INTERNAL ERROR **\n**");
  vfprintf(stderr, error, args);
  putc('\n', stderr);
  /*if (ERROR_current->info.code)
  {
    ERROR_print();
  }*/
  fprintf(stderr, "** Program aborting. Sorry! :-(\n");
  /*abort();*/
  _exit(1);
}
