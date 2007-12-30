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
};

#endif
