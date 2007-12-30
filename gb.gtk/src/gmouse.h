#ifndef __GMOUSE_H
#define __GMOUSE_H

class gMouse
{
public:

//"Properties"
	static long button();
	static bool left();
	static bool right();
	static bool middle();
	static bool shift();
	static bool control();
	static bool alt();
	static bool meta();
	static bool normal();
	static long x();
	static long y();
	static long screenX();
	static long screenY();
	static long delta();
	static long orientation();
	static bool valid();

//"Methods"
	static void move(long x,long y);

//"Private"
	static void setWheel(long dt,long orn);
	static void setValid(int vl,long xv,long yv,long bt_val,long state_val,long sx,long sy);
};

#endif
