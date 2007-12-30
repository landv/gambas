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
