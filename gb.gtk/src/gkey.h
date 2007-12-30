#ifndef __GKEY_H
#define __GKEY_H

class gKey
{
public:
	static bool valid() { return _valid; }
	static char *text();
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
	static void enable(GdkEventKey *e);
	static void exit();

private:
	static GdkEventKey _event;
	static bool _valid;
};

#endif
