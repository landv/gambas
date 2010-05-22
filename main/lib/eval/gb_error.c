/***************************************************************************

  gb_error.c

  (c) 2000-2009 Benoît Minisini <gambas@users.sourceforge.net>

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
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

***************************************************************************/

#define __GB_ERROR_C

#include "main.h"
#include "gb_common.h"
#include <stdarg.h>
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
	else
		info->msg = NULL;
	//DEBUG_free_backtrace(&info->backtrace);
}

/*
void ERROR_enter(ERROR_CONTEXT *err)
{
  CLEAR(err);
  err->prev = ERROR_current;
  ERROR_current = err;
}


void ERROR_leave(ERROR_CONTEXT *err)
{
  if (err->prev == ERROR_LEAVE_DONE)
  	return;
  	
	ERROR_current = err->prev;
	
	if (ERROR_current)
	{
		ERROR_reset(&ERROR_current->info);
		ERROR_current->info = err->info;
	}

	err->prev = ERROR_LEAVE_DONE;
}
*/

void ERROR_propagate()
{
	if (ERROR_current->ret)
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
  GB.Error("&1", (char *)msg);
  ERROR_propagate();
}

PUBLIC void THROW2(const char *pattern, const char *msg)
{
  GB.Error((char *)pattern, (char *)msg);
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
