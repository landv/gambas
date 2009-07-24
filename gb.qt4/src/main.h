/***************************************************************************

  main.h

  The interface between the QT plug-in and the Gambas interpreter

  (c) 2000-2007 Benoit Minisini <gambas@users.sourceforge.net>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 1, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

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
extern int MAIN_loop_level;
extern int MAIN_scale;
#ifndef NO_X_WINDOW
extern int MAIN_x11_last_key_code;
#endif
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
