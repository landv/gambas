/***************************************************************************

  main.cpp

  (c) 2005 Carlo Sorda <gambas@users.sourceforge.net><csorda@libero.it>

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

#define __MAIN_CPP

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <fcntl.h>

#include "gambas.h"
#include "gb_common.h"

#include "main.h"
#include "CCorbaApplication.h"

extern "C" {

GB_INTERFACE GB EXPORT;

} /* extern "C" */

static void cl_wait(long duration)
{

}

static void cl_init(void)
{

}

static void cl_exit(void)
{

}

static int cl_loop(void)
{
	while(1)
	{

	}

	return 0;
}
extern "C" {

	GB_DESC *GB_CLASSES[] EXPORT =
	{
		CCORBAApplicationDesc,
		CCORBAObjectDesc,
		CCORBACosNamingNamesDesc,
		CCORBACosNamingNameDesc,


		NULL
	};

	int EXPORT GB_INIT(void)
	{
		GB.Hook(GB_HOOK_WAIT, (void *)cl_wait);
		GB.Hook(GB_HOOK_LOOP, (void *)cl_loop);



		return -1;
	}

	void EXPORT GB_EXIT()
	{
		cl_exit();
	}

} //extern "C"
