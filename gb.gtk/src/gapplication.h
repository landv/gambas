/***************************************************************************

  gapplication.h

  (c) 2000-2011 Benoît Minisini <gambas@users.sourceforge.net>

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

	static gControl* activeControl() { return _active_control; }
	static void setActiveControl(gControl *control, bool on);
	
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
	static void enterLoop(void *owner, bool showIt = false);
	static void enterPopup(gMainWindow *owner);
	static void exitLoop(void *owner);
	static bool hasLoop(void *owner) { return _loop_owner == owner; }
	static GtkWindowGroup *enterGroup();
	static void exitGroup(GtkWindowGroup *oldGroup);
	static guint32 lastEventTime() { return _event_time; }
	static void updateLastEventTime(GdkEvent *e);

	static bool (*onKeyEvent)(int type);
	
	static int getScrollbarSize();
	static int getScrollbarSpacing();
	static int getFrameWidth();
	static int getTextBoxFrameWidth();

	//"Private"
	static GtkTooltips *tipHandle();
	static bool _busy;
	static char *_title;
	static int _loopLevel;
	static int _tooltip_delay;
	static void *_loop_owner;
	static GtkWindowGroup *_group;
	static GtkWindowGroup *currentGroup();
	//static void dispatchEnterLeave(gControl *enter);
	static gControl *_enter;
	static gControl *_leave;
	static gControl *_active_control;
	static gControl *_old_active_control;
	static guint32 _event_time;
	static bool _close_next_window;
};

#endif
