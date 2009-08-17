/***************************************************************************

  gb_common.c

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

#define __COMMON_C
#define __COMMON_CHECK_C

#include "gb_common.h"

#include "gb_common_check.h"

#include "gb_common_case_temp.h"
#include "gb_common_buffer_temp.h"
#include "gb_common_swap_temp.h"

sigjmp_buf CHECK_jump;

static sighandler_t _oldsegv;
static sighandler_t _oldbus;
static int _dummy;
static volatile int _got_error;


void COMMON_init(void)
{
}


static void signal_error()
{
  _got_error = TRUE;
  siglongjmp(CHECK_jump, 1);
}

void CHECK_enter(void)
{
  _got_error = FALSE;
  _oldsegv = signal(SIGSEGV, signal_error);
  _oldbus = signal(SIGBUS, signal_error);
}

void CHECK_leave(void)
{
  signal(SIGSEGV, _oldsegv);
  signal(SIGBUS, _oldbus);
}

bool CHECK_got_error(void)
{
  return _got_error;
}

bool CHECK_address(void *ptr, size_t len)
{
  offset_t i;

  CHECK_enter();
  if (sigsetjmp(CHECK_jump, TRUE) == 0)
  {
    for (i = 0; i < len; i += 1024)
      _dummy = ((int *)ptr)[i];
  }
  CHECK_leave();
  return _got_error;
}

bool CHECK_strlen(char *ptr, size_t *len)
{
  size_t l = 0;

  CHECK_enter();
  if (sigsetjmp(CHECK_jump, TRUE) == 0)
  {
    l = strlen(ptr);
    *len = l;
  }
  CHECK_leave();
  if (l != (size_t)(int)l)
  	_got_error = TRUE;
  return _got_error;
}

