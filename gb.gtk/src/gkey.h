/***************************************************************************

  gkey.h

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

#ifndef __GKEY_H
#define __GKEY_H

class gKey
{
public:
	static bool valid() { return _valid; }
	static const char *text();
	static int code();
	static int state();

	static bool alt();
	static bool control();
	static bool meta();
	static bool normal();
	static bool shift();

	static int fromString(char* str);

//"Private"
	static void disable();
	static bool enable(gControl *control, GdkEventKey *e);
	static bool canceled() { return _canceled; }
	static void init();
	static void exit();
	
	static void setActiveControl(gControl *control);

	static bool raiseEvent(int type, gControl *control, const char *text);

	static bool mustIgnoreEvent(GdkEventKey *e);

	static bool _canceled;
	static GdkEventKey _event;

private:
	static bool _valid;
};

#endif
