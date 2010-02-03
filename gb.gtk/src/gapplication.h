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

	static void setDefaultTitle(const char *title);
	static char *defaultTitle() { return _title; }

	static void setDirty();
	static int loopLevel() { return _loopLevel; }
	static void enterLoop(void *owner, bool showIt = false);
	static void exitLoop(void *owner);
	static bool hasLoop(void *owner) { return _loop_owner == owner; }
	static GtkWindowGroup *enterGroup();
	static void exitGroup(GtkWindowGroup *oldGroup);
	static guint32 lastEventTime() { return _event_time; }

	static bool (*onKeyEvent)(int type);

//"Private"
	static GtkTooltips *tipHandle();
	static bool _busy;
	static char *_title;
	static int _loopLevel;
	static void *_loop_owner;
	static GtkWindowGroup *_group;
	static GtkWindowGroup *currentGroup();
	//static void dispatchEnterLeave(gControl *enter);
	static gControl *_enter;
	static gControl *_leave;
	static guint32 _event_time;
};

#endif
