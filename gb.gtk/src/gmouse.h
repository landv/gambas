/***************************************************************************

  gmouse.h

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
#ifndef __GMOUSE_H
#define __GMOUSE_H

class gMouse
{
public:

//"Properties"
	static int button();
	static bool left();
	static bool right();
	static bool middle();
	static bool shift();
	static bool control();
	static bool alt();
	static bool meta();
	static bool normal();
	static int x();
	static int y();
	static int screenX();
	static int screenY();
	static void getScreenPos(int *x, int *y);
	static int delta();
	static int orientation();
	static bool isValid() { return _isValid; }
	static int startX() { return _start_x; }
	static int startY() { return _start_y; }

//"Methods"
	static void move(int x,int y);

//"Private"
	static void setWheel(int dt, int orn);
	static void setStart(int sx, int sy);
	static void setMouse(int x, int y, int button, int state);
	static void validate() { _isValid++; }
	static void invalidate() { _isValid--; }

private:
	static int _isValid;
	static int _x;
	static int _y;
	static int _button;
	static int _state;
	static int _delta;
	static int _orientation;
	static int _start_x;
	static int _start_y;
};

#endif
