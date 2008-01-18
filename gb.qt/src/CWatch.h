/***************************************************************************

  CWatch.h

  Watching for file descriptors

  (c) 2000-2005 Beno�t Minisini <gambas@users.sourceforge.net>

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

#ifndef __CWATCH_H
#define __CWATCH_H

#include <qobject.h>
#include <qintdict.h>
#include <qsocketnotifier.h>

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

  static QIntDict<CWatch> readDict;
  static QIntDict<CWatch> writeDict;

  QSocketNotifier *notifier;
  GB_WATCH_CALLBACK callback;
  intptr_t param;

public slots:

  void read(int);
  void write(int);
};


#endif
