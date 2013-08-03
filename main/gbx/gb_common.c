/***************************************************************************

	gb_common.c

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

#define __COMMON_C
#define __COMMON_CHECK_C

#include "gb_common.h"

#include "gb_common_check.h"

#include "gb_common_case_temp.h"
#include "gb_common_buffer_temp.h"
#include "gb_common_swap_temp.h"

#include "gbx_signal.h"

sigjmp_buf CHECK_jump;

static SIGNAL_HANDLER _SIGSEGV_handler;
static SIGNAL_HANDLER _SIGBUS_handler;

static int _dummy;
static volatile int _got_error;


void COMMON_init(void)
{
}


static void signal_error(int sig, siginfo_t *info, void *context)
{
	SIGNAL_previous(sig == SIGSEGV ? &_SIGSEGV_handler : &_SIGBUS_handler, sig, info, context);
	_got_error = TRUE;
	siglongjmp(CHECK_jump, 1);
}

void CHECK_enter(void)
{
	_got_error = FALSE;
	SIGNAL_install(&_SIGSEGV_handler, SIGSEGV, signal_error);
	SIGNAL_install(&_SIGBUS_handler, SIGBUS, signal_error);
}

void CHECK_leave(void)
{
	SIGNAL_uninstall(&_SIGSEGV_handler, SIGSEGV);
	SIGNAL_uninstall(&_SIGBUS_handler, SIGBUS);
}

bool CHECK_got_error(void)
{
	return _got_error;
}

bool CHECK_address(void *ptr, ssize_t len)
{
	offset_t i;

	if (len < 0)
		return TRUE;
	
	CHECK_enter();
	if (sigsetjmp(CHECK_jump, TRUE) == 0)
	{
		for (i = 0; i < len; i += 1024)
			_dummy = ((int *)ptr)[i];
		
		_dummy = ((int *)ptr)[len >> 2];
	}
	CHECK_leave();
	return _got_error;
}

bool CHECK_strlen(char *ptr, ssize_t *len)
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

