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

#define USE_NOTHING  0
#define USE_GB_QT4   1
#define USE_GB_GTK   2
#define USE_GB_GTK3  3

GB_INTERFACE GB EXPORT;

// Prevents gbi3 from complaining

GB_DESC *GB_CLASSES[] EXPORT =
{
  NULL
};

char *GB_INCLUDE EXPORT = "gb.qt4";

int EXPORT GB_INIT(void)
{
	int use = USE_NOTHING;
	char *env;
	const char *comp;
	const char *comp2;

	env = getenv("GB_GUI");
	if (env)
	{
		if (strcmp(env, "gb.qt4") == 0)
			use = USE_GB_QT4;
		else if (strcmp(env, "gb.gtk") == 0)
			use = USE_GB_GTK;
		else if (strcmp(env, "gb.gtk3") == 0)
			use = USE_GB_GTK3;
	}
	
	if (use == USE_NOTHING)
	{
		use = USE_GB_GTK;
		
		env = getenv("KDE_FULL_SESSION");
		
		if (env && !strcmp(env, "true"))
		{
			env = getenv("KDE_SESSION_VERSION");
			if (env && !strcmp(env, "4"))
				use = USE_GB_QT4;
		}
	}

	comp = use == USE_GB_QT4 ? "gb.qt4" : (use == USE_GB_GTK3 ? "gb.gtk3" : "gb.gtk");

	if (GB.Component.Load(comp))
	{
		comp2 = use == USE_GB_QT4 ? "gb.gtk" : "gb.qt4";

		if (GB.Component.Load(comp2))
		{
			fprintf(stderr, "gb.gui: error: unable to find any GUI component. Please install the 'gb.qt4' or 'gb.gtk' component\n");
			exit(1);
		}

		fprintf(stderr, "gb.gui: warning: '%s' component not found, using '%s' instead\n", comp, comp2);
		comp = comp2;
	}
  
  setenv("GB_GUI", comp, TRUE);

  return 0;
}

void EXPORT GB_EXIT()
{
}


