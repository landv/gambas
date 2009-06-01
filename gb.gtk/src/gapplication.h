#ifndef __GAPPLICATION_H
#define __GAPPLICATION_H

class gControl;
class gMainWindow;

class gApplication
{
public:
	static void init(int *argc, char ***argv);
	static void quit();
	static void exit();

	static int controlCount();

	static gControl* controlItem(int index);
	static gControl* controlItem(GtkWidget *wid);

  static void setBusy(bool b);
  static bool isBusy() { return _busy; }

	static void suspendEvents(bool vl);
	static void enableEvents();
	static bool userEvents();
	static bool allEvents();

	static void enableTooltips(bool vl);
	static bool toolTips();
	static gFont* toolTipsFont();
	static void setToolTipsFont(gFont *ft);
	static int toolTipsDelay() { return _tooltip_delay; }
	static void setToolTipsDelay(int v);

	static void setDefaultTitle(const char *title);
	static char *defaultTitle() { return _title; }

	static void setDirty();
	static int loopLevel() { return _loopLevel; }
	static void enterLoop();
	static void exitLoop();

//"Private"
	static GtkTooltips *tipHandle();
	static bool _busy;
	static char *_title;
	static int _loopLevel;
	static int _tooltip_delay;
};

#endif
