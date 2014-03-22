/***************************************************************************

	CWatch.cpp

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

#define __CWATCH_CPP

//#include "gb_common.h"
#include "main.h"
#include "CWatch.h"

QHash<int, CWatch *> CWatch::readDict;
QHash<int, CWatch *> CWatch::writeDict;
int CWatch::count = 0;

void CWatch::watch(int fd, int type, GB_WATCH_CALLBACK callback, intptr_t param)
{
	CWatch *watch;

	//qDebug("CWatch::watch: %d %d", fd, type);

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

	if (type == QSocketNotifier::Read)
	{
		if (readDict[fd])
			delete readDict[fd];
	}
	else if (type == QSocketNotifier::Write)
	{
		if (writeDict[fd])
			delete writeDict[fd];
	}

	notifier = new QSocketNotifier(fd, type);
	this->callback = callback;
	this->param = param;

	if (type == QSocketNotifier::Read)
	{
		//qDebug("CWatch: %d (read)", fd);

		readDict.insert(fd, this);
		QObject::connect(notifier, SIGNAL(activated(int)), this, SLOT(read(int)));
	}
	else if (type == QSocketNotifier::Write)
	{
		//qDebug("CWatch: %d (write)", fd);

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

	//BREAKPOINT();

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
	//qDebug("CWatch::write: fd = %d writeDict[fd] = %p", fd, writeDict[fd]);
	if (writeDict[fd])
		(*callback)(fd, GB_WATCH_WRITE, param);
}
