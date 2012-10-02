/***************************************************************************

  main.c

  gb.httpd component

  (c) 2012 Benoît Minisini <gambas@users.sourceforge.net>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 1, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
  MA 02110-1301, USA.

***************************************************************************/

#define __MAIN_C

#include "gb_common.h"

#include <setjmp.h>

#include "main.h"

int thttpd_main(int argc, char **argv);

const GB_INTERFACE *GB_PTR EXPORT;

static jmp_buf _setjmp_env;

void syslog(int priority, const char *format, ...)
{
	va_list args;

	va_start(args, format);

	fprintf(stderr, "gb.httpd: ");
	vfprintf(stderr, format, args);
	putc('\n', stderr);
}

void run_cgi(void)
{
	longjmp(_setjmp_env, 1);
}

void EXPORT GB_MAIN(int argc, char **argv)
{
	if (setjmp(_setjmp_env) == 0)
		thttpd_main(argc, argv);
	else
		GB.System.HasForked();
}

int EXPORT GB_INIT()
{
	return 0;
}

void EXPORT GB_EXIT()
{
}
