/***************************************************************************

	main.c

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

#define __MAIN_C

#include "main.h"

#include "c_glarea.h"

GB_INTERFACE GB EXPORT;
GTK_INTERFACE GTK;
GL_INTERFACE GL;

GB_DESC *GB_CLASSES[] EXPORT =
{
	GLAreaDesc,
	NULL
};

static void *_old_hook_main;

static void hook_main(int *argc, char ***argv)
{
	gtk_init(argc, argv);
	gtk_gl_init(argc, argv);
	
	CALL_HOOK_MAIN(_old_hook_main, argc, argv);
}

int EXPORT GB_INIT(void)
{
	GB.GetInterface("gb.gtk", GTK_INTERFACE_VERSION, &GTK);
	GB.GetInterface("gb.opengl", GL_INTERFACE_VERSION, &GL);

	_old_hook_main = GB.Hook(GB_HOOK_MAIN, (void *)hook_main);
	return 0;
}

void EXPORT GB_EXIT()
{
}
