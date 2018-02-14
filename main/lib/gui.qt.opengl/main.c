/***************************************************************************

  main.c

  (c) 2000-2017 Benoît Minisini <g4mba5@gmail.com>

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

GB_INTERFACE GB EXPORT;

GB_DESC *GB_CLASSES[] EXPORT =
{
  NULL
};

char *GB_INCLUDE EXPORT = "gb.qt4.opengl|gb.qt5.opengl";

int EXPORT GB_INIT(void)
{
	const char *comp = NULL;
	char *env;
	
	env = getenv("GB_GUI");
	if (env)
	{
		if (strcmp(env, "gb.qt4") == 0)
			comp = "gb.qt4.opengl";
		else if (strcmp(env, "gb.qt5") == 0)
			comp = "gb.qt5.opengl";
	}
	
	if (!comp)
	{
		// GB_GUI should be set by gb.gui
		if (!env)
			fprintf(stderr, "gb.gui.qt.opengl: error: no component specified in GB_GUI environment variable");
		else
			fprintf(stderr, "gb.gui.qt.opengl: error: unsupported component specified in GB_GUI environment variable");
		exit(1);
	}
		
	if (GB.Component.Load(comp))
		fprintf(stderr, "gb.gui.qt.opengl: unable to load '%s' component\n", comp);
  
  return 0;
}

void EXPORT GB_EXIT()
{
}


