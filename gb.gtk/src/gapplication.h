/***************************************************************************

  gapplication.h

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

#ifndef __GAPPLICATION_H
#define __GAPPLICATION_H

typedef
	void (*X11_EVENT_FILTER)(XEvent *);

class gControl;
class gMainWindow;

class gApplication
{
public:
	static void init(int *argc, char ***argv);
	static void quit();
	static void exit();
	static bool mustQuit() { return _must_quit; }

	static int controlCount();

	static gControl* controlItem(int index);
	static gControl* controlItem(GtkWidget *wid);

  static void setBusy(bool b);
  static bool isBusy() { return _busy; }

	static gControl* activeControl() { return _active_control; }
	static gControl* previousControl() { return _previous_control; }
	static void setActiveControl(gControl *control, bool on);
	
	static void suspendEvents(bool vl);
	static void enableEvents();
	static bool userEvents();
	static bool allEvents();

	static void enableTooltips(bool vl);
	static bool areTooltipsEnabled();

	static void setDefaultTitle(const char *title);
	static char *defaultTitle() { return _title; }

	static void setDirty();
	static int loopLevel() { return _loopLevel; }
	static void enterLoop(void *owner, bool showIt = false, GtkWindow *modal = NULL);
	static void enterPopup(gMainWindow *owner);
	static void exitLoop(void *owner);
	static bool hasLoop(void *owner) { return _loop_owner == owner; }
	static GtkWindowGroup *enterGroup();
	static void exitGroup(GtkWindowGroup *oldGroup);
	static guint32 lastEventTime() { return _event_time; }
	static GdkEvent *lastEvent() { return _event; }
	static void updateLastEvent(GdkEvent *e);

	static bool (*onKeyEvent)(int type);
	
	static int getScrollbarSize();
	static int getScrollbarSpacing();
	static int getFrameWidth();
	static int getInnerWidth();
	static void getBoxFrame(int *w, int *h);
	static char *getStyleName();
	
	static void grabPopup();
	static void ungrabPopup();
	
	static void setMainWindow(gMainWindow *win);
	static gMainWindow *mainWindow() { return _main_window; }
	
	static void checkHoveredControl(gControl *control);

	static void setEventFilter(X11_EVENT_FILTER filter);

	static void setButtonGrab(gControl *grab) { _button_grab = grab; }

	//"Private"
	static bool _busy;
	static bool _must_quit;
	static char *_title;
	static int _loopLevel;
	static int _in_popup;
	static GtkWidget *_popup_grab;
	static void *_loop_owner;
	static GtkWindowGroup *_group;
	static GtkWindowGroup *currentGroup();
	//static void dispatchEnterLeave(gControl *enter);
	static gControl *_enter;
	static gControl *_leave;
	static gControl *_ignore_until_next_enter;
	static gControl *_active_control;
	static gControl *_previous_control;
	static gControl *_old_active_control;
	static gControl *_button_grab;
	static gControl *_control_grab;
	static guint32 _event_time;
	static GdkEvent *_event;
	static gMainWindow *_main_window;
	static bool _close_next_window;
	static bool _fix_printer_dialog;
	static void (*onEnterEventLoop)();
	static void (*onLeaveEventLoop)();
};

#endif
