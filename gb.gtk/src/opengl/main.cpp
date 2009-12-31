/***************************************************************************

  main.cpp

  (c) 2009-2010 Laurent Carlier <lordheavy@users.sourceforge.net>

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

#include "main.h"

//#include "CGLarea.h"

static void my_main(int *argc, char **argv);

extern "C" {

GB_INTERFACE GB EXPORT;
GTK_INTERFACE GTK;
GL_INTERFACE GL;

GB_DESC *GB_CLASSES[] EXPORT =
{
//  CGlareaDesc,
  NULL
};

int EXPORT GB_INIT(void)
{
	GB.Hook(GB_HOOK_MAIN, (void *)my_main);

	GB.GetInterface("gb.gtk", GTK_INTERFACE_VERSION, &GTK);
	GB.GetInterface("gb.opengl", GL_INTERFACE_VERSION, &GL);

	return FALSE;
}

void EXPORT GB_EXIT()
{
}

}

static void my_main(int *argc, char **argv)
{
	GTK.Init(argc, argv);
	
	if (!gdk_gl_init_check(argc, &argv))
		GB.Error("Failed to init GTKglext !");
}
