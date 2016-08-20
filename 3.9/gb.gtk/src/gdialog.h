/***************************************************************************

  gdialog.h

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

#ifndef __GDIALOG_H
#define __GDIALOG_H

#include "gcolor.h"

class gDialog
{
public:
	static void exit();

	static gColor color();
	static char** filter(int *nfilter);
	static char** paths();
	static char* path();
	static char* title();
	static gFont* font();
	static bool showHidden();

	static void setColor(gColor col);
	static void setFilter(char **filter, int nfilter);
	static void setPath(char *vl);
	static void setTitle(char *title);
	static void setFont(gFont *ft);
	static void setShowHidden(bool v);

	static bool selectColor();
	static bool selectFolder();
	static bool selectFont();
	static bool openFile(bool multi=false);
	static bool saveFile();

private:
  static GPtrArray *_filter;
};

#endif
