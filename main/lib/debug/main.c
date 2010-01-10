/***************************************************************************

  main.c

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

#define __MAIN_C

#include "gambas.h"
#include "debug.h"
#include "CDebug.h"
#include "main.h"

GB_INTERFACE GB EXPORT;

GB_DESC *GB_CLASSES[] EXPORT =
{
  CDebugDesc,
  NULL
};

void *GB_DEBUG_1[] EXPORT =
{
	(void *)1,
	(void *)DEBUG_init,
	(void *)DEBUG_exit,
	(void *)DEBUG_welcome,
	(void *)DEBUG_main,
	(void *)DEBUG_where,
	(void *)DEBUG_backtrace,
	(void *)DEBUG_breakpoint,
	(void *)DEBUG_break_on_next_line,
	(void *)DEBUG_get_position,
	(void *)DEBUG_get_current_position,
	(void *)DEBUG_init_breakpoints,
	NULL
};

int EXPORT GB_INIT(void)
{
  return 0;
}

void EXPORT GB_EXIT()
{
}


