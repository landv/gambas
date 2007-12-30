/***************************************************************************

  common.c

  common useful routines

  (c) 2000-2007 Benoit Minisini <gambas@users.sourceforge.net>

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

#define __COMMON_C
#define __COMMON_CHECK_C

#include "gb_common.h"

#include "gb_common_check.h"

#include "gb_common_case_temp.h"
#include "gb_common_buffer_temp.h"
#include "gb_common_swap_temp.h"

PUBLIC sigjmp_buf CHECK_jump;

static sighandler_t _oldsegv;
static sighandler_t _oldbus;
static int _dummy;
static volatile int _got_error;


PUBLIC void COMMON_init(void)
{
  COMMON_case_init();
}


static void signal_error()
{
  _got_error = TRUE;
  siglongjmp(CHECK_jump, 1);
}

PUBLIC void CHECK_enter(void)
{
  _got_error = FALSE;
  _oldsegv = signal(SIGSEGV, signal_error);
  _oldbus = signal(SIGBUS, signal_error);
}

PUBLIC void CHECK_leave(void)
{
  signal(SIGSEGV, _oldsegv);
  signal(SIGBUS, _oldbus);
}

PUBLIC bool CHECK_got_error(void)
{
  return _got_error;
}

PUBLIC bool CHECK_address(void *ptr, int len)
{
  long i;

  CHECK_enter();
  if (sigsetjmp(CHECK_jump, TRUE) == 0)
  {
    for (i = 0; i < len; i += 1024)
      _dummy = ((int *)ptr)[i];
  }
  CHECK_leave();
  return _got_error;
}

PUBLIC bool CHECK_strlen(char *ptr, int *len)
{
  long l;

  CHECK_enter();
  if (sigsetjmp(CHECK_jump, TRUE) == 0)
  {
    l = strlen(ptr);
    //while (!*ptr++)
    // l++;
    *len = l;
  }
  CHECK_leave();
  return _got_error;
}

