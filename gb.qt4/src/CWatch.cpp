/***************************************************************************

  CWatch.cpp

  (c) 2000-2009 Benoît Minisini <gambas@users.sourceforge.net>

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

#define __CWATCH_CPP

#include "main.h"
#include "CWatch.h"

QHash<int, CWatch *> CWatch::readDict;
QHash<int, CWatch *> CWatch::writeDict;
int CWatch::count = 0;

void CWatch::watch(int fd, int type, GB_WATCH_CALLBACK callback, intptr_t param)
{
  CWatch *watch;

  switch (type)
  {
    case GB_WATCH_NONE:

      watch = readDict[fd];
      if (watch) delete watch;

      watch = writeDict[fd];
      if (watch) delete watch;

      break;

    case GB_WATCH_READ:

			if (callback)
				new CWatch(fd, QSocketNotifier::Read, callback, param);
			else
			{
				watch = readDict[fd];
				if (watch) delete watch;
			}
      break;

    case GB_WATCH_WRITE:

			if (callback)
				new CWatch(fd, QSocketNotifier::Write, callback, param);
			else
			{
				watch = writeDict[fd];
				if (watch) delete watch;
			}
      break;
  }
}

void CWatch::stop()
{
  int fd;
  
  for (fd = 0; count > 0; fd++)
    watch(fd, GB_WATCH_NONE, 0, 0);
}

CWatch::CWatch(int fd, QSocketNotifier::Type type, GB_WATCH_CALLBACK callback, intptr_t param)
{
  count++;
   
  notifier = new QSocketNotifier(fd, type);
  this->callback = callback;
  this->param = param;

  if (type == QSocketNotifier::Read)
  {
    //qDebug("CWatch: %d (read)", fd);
    
    if (readDict[fd])
      delete readDict[fd];

    readDict.insert(fd, this);
    QObject::connect(notifier, SIGNAL(activated(int)), this, SLOT(read(int)));
  }
  else if (type == QSocketNotifier::Write)
  {
    //qDebug("CWatch: %d (write)", fd);
    
    if (writeDict[fd])
      delete writeDict[fd];

    writeDict.insert(fd, this);
    QObject::connect(notifier, SIGNAL(activated(int)), this, SLOT(write(int)));
  }
}


CWatch::~CWatch()
{
  if (notifier->type() == QSocketNotifier::Read)
  {
    //qDebug("~CWatch: %d (read)", notifier->socket());
    readDict.remove(notifier->socket());
  }
  else if (notifier->type() == QSocketNotifier::Write)
  {
    //qDebug("~CWatch: %d (write)", notifier->socket());
    writeDict.remove(notifier->socket());
  }
    
  delete notifier;

  count--;
  MAIN_check_quit();
}

void CWatch::read(int fd)
{
  //qDebug("CWatch::read: fd = %d readDict[fd] = %p", fd, readDict[fd]);
  if (readDict[fd])
    (*callback)(fd, GB_WATCH_READ, param);
}

void CWatch::write(int fd)
{
  if (writeDict[fd])
    (*callback)(fd, GB_WATCH_WRITE, param);
}
