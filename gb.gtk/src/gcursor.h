/***************************************************************************

  gcursor.h

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

#ifndef __GCURSOR_H
#define __GCURSOR_H

#define CURSOR_DEFAULT GDK_X_CURSOR
#define CURSOR_CUSTOM GDK_CURSOR_IS_PIXMAP

class gPicture;

class gCursor
{
public:
	gCursor(gPicture *pic,int x,int y);
	gCursor(gCursor *cursor);
	~gCursor();

//"Properties"
	int left() const { return x; }
	int top() const { return y; }

//"Private"
	GdkCursor *cur;
	int x;
	int y;
};

#endif
