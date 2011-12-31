/***************************************************************************

  main.h

  (c) 2000-2012 Benoît Minisini <gambas@users.sourceforge.net>

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

#define DO_NOT_USE_QT_INTERFACE
#include "gb.qt.h"
#include "gb.image.h"

#ifndef __MAIN_CPP
extern "C" GB_INTERFACE GB;
extern "C" IMAGE_INTERFACE IMAGE;
extern int MAIN_in_wait;
extern int MAIN_in_message_box;
extern int MAIN_loop_level;
extern int MAIN_scale;
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
  
  static QEventLoop *eventLoop;
	
public slots:
	
	void linkDestroyed(QObject *);
  
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

const char *QT_ToUTF8(const QString &str);
void QT_RegisterAction(void *object, const char *key, int on);
void QT_RaiseAction(const char *key);
void *QT_GetObject(QWidget *);
void QT_Link(QObject *, void *);
void *QT_GetLink(QObject *);

#endif
