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

#include <qobject.h>
#include <qevent.h>
#include <qapplication.h>

#define DO_NOT_USE_QT_INTERFACE
#include "gb.qt.h"

#ifndef __MAIN_CPP
extern "C" GB_INTERFACE GB;
extern int MAIN_in_wait;
extern int MAIN_loop_level;
extern int MAIN_scale;
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
  virtual bool eventFilter(QObject *o, QEvent *e);
  virtual bool notify(QObject *o, QEvent *e);
};

class MyTimer : public QObject
{
public:

  MyTimer(GB_TIMER *timer);
  ~MyTimer();

protected:

  void timerEvent(QTimerEvent *);

private:

	GB_TIMER *timer;
	long id;
};


#define UTF8_NBUF 4

void MAIN_check_quit(void);
void MAIN_update_scale(void);

const char *QT_ToUTF8(const QString &str);
void QT_RegisterAction(void *object, const char *key, int on);
void QT_RaiseAction(const char *key);
QMimeSourceFactory *QT_MimeSourceFactory(void);

#endif
