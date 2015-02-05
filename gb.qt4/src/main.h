/***************************************************************************

  main.h

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

#ifndef __MAIN_H
#define __MAIN_H

#include "gambas.h"

#include <QEvent>
#include <QApplication>
#include <QTimerEvent>
#include <QEventLoop>
#include <QSessionManager>

#define DO_NOT_USE_QT_INTERFACE
#include "gb.qt.h"
#include "gb.image.h"

#ifndef __MAIN_CPP
extern "C" const GB_INTERFACE *GB_PTR;
extern "C" IMAGE_INTERFACE IMAGE;
extern int MAIN_in_wait;
extern int MAIN_in_message_box;
extern int MAIN_loop_level;
extern int MAIN_scale;
extern bool MAIN_debug_busy;
extern bool MAIN_init;
#ifndef NO_X_WINDOW
extern int MAIN_x11_last_key_code;
#endif

extern GB_CLASS CLASS_Control;
extern GB_CLASS CLASS_Container;
extern GB_CLASS CLASS_UserControl;
extern GB_CLASS CLASS_UserContainer;
extern GB_CLASS CLASS_TabStrip;
extern GB_CLASS CLASS_Window;
extern GB_CLASS CLASS_Menu;
extern GB_CLASS CLASS_Picture;
extern GB_CLASS CLASS_Drawing;
extern GB_CLASS CLASS_DrawingArea;
extern GB_CLASS CLASS_ScrollArea;
extern GB_CLASS CLASS_Printer;
extern GB_CLASS CLASS_ScrollView;
extern GB_CLASS CLASS_Image;
extern GB_CLASS CLASS_SvgImage;
extern GB_CLASS CLASS_TextArea;

#endif

#define GB (*GB_PTR)

class MyPostCheck: public QObject
{
  Q_OBJECT

public:

  static bool in_check;

public slots:

  void check(void);
};

class MyApplication: public QApplication
{
  Q_OBJECT

public:

  MyApplication(int &argc, char **argv);
  #ifndef NO_X_WINDOW
  virtual bool x11EventFilter(XEvent *e);
 	#endif
  virtual bool eventFilter(QObject *o, QEvent *e);
  //virtual bool notify(QObject *o, QEvent *e);
  
  static void setEventFilter(bool set);
  
  static bool isTooltipEnabled() { return !_tooltip_disable; }
  static void setTooltipEnabled(bool b);
	
	static void initClipboard();
  
  static QEventLoop *eventLoop;
	

public slots:
	
	void linkDestroyed(QObject *);
	void clipboardHasChanged();
	void commitDataRequested(QSessionManager &);
  
private:
  static bool _tooltip_disable;
  static int _event_filter;
};

class MyTimer : public QObject
{
public:

  MyTimer(GB_TIMER *timer);
  ~MyTimer();
	void clearTimer() { timer = 0; }

protected:

  void timerEvent(QTimerEvent *);

private:

	GB_TIMER *timer;
	intptr_t id;
};


#define UTF8_NBUF 4

void MAIN_check_quit(void);
void MAIN_update_scale(void);
void MAIN_process_events(void);
void MAIN_init_error(void);

#define MAIN_CHECK_INIT() (MAIN_init ? 0 : (MAIN_init_error(), 1))

const char *QT_ToUTF8(const QString &str);
void QT_RegisterAction(void *object, const char *key, int on);
void QT_RaiseAction(const char *key);
void *QT_GetObject(QWidget *);
void QT_Link(QObject *, void *);
void *QT_GetLink(QObject *);

#endif
