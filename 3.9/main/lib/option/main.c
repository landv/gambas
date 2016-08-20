/***************************************************************************

  main.c

  (c) 2000-2009 Chintan Rao <chintanraoh@gmail.com>

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

#define __MAIN_C

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "getoptions.h"

#include "main.h"

static void *_old_hook_main;

char** cmd_arg;
int arg_count;


GB_INTERFACE GB EXPORT;



GB_DESC *GB_CLASSES[] EXPORT =
{
  GetOptionsDesc,
  NULL
};

static void hook_main(int *argc, char ***argv)
{
	int i;
	char **tmp;

	CALL_HOOK_MAIN(_old_hook_main, argc, argv);

	arg_count = *argc;
			
	GB.NewArray(POINTER(&cmd_arg), sizeof(*cmd_arg), 0);

	for(i=0; i<*argc; i++)
	{
		tmp = (char **)GB.Add((void*)(&cmd_arg));
		*tmp = GB.NewZeroString((*argv)[i]);
	}
	
	*argc = 1;
}

int EXPORT GB_INIT(void)
{
	_old_hook_main = GB.Hook(GB_HOOK_MAIN, (void *)hook_main);
  return 0;
}


void EXPORT GB_EXIT()
{
	int i;
	for(i=0;i<arg_count;i++)
	{
		GB.FreeString( (void *) &cmd_arg[i]);
//		tmp=(char **)GB.Array.Get((void*)(cmd_arg),i);
//		GB.FreeString(tmp);
	}
	GB.FreeArray( (void *) &cmd_arg );

}

