/***************************************************************************

  gkey.h

  (c) 2000-2009 Benoît Minisini <gambas@users.sourceforge.net>

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
	static bool enable(GtkWidget *w, GdkEventKey *e);
	static void init();
	static void exit();
	
	static void setActiveControl(gControl *control);

private:
	static GdkEventKey _event;
	static bool _valid;
	static GtkIMContext *_im_context;
	static GtkWidget *_im_widget;
	static bool _no_input_method;
};

#endif
