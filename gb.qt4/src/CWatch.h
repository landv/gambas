/***************************************************************************

  CWatch.h

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

#ifndef __CWATCH_H
#define __CWATCH_H

#include <qobject.h>
#include <qsocketnotifier.h>
#include <QHash>

#include "gambas.h"

class CWatch: public QObject
{
  Q_OBJECT

public:

  static void watch(int fd, int type, GB_WATCH_CALLBACK callback, intptr_t param);
  static void stop();
  static int count;

  CWatch(int fd, QSocketNotifier::Type type, GB_WATCH_CALLBACK callback, intptr_t param);
  ~CWatch();

private:

  static QHash<int, CWatch *> readDict;
  static QHash<int, CWatch *> writeDict;

  QSocketNotifier *notifier;
  GB_WATCH_CALLBACK callback;
  intptr_t param;

public slots:

  void read(int);
  void write(int);
};


#endif
