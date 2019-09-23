/***************************************************************************

	ctrayicon.h

	(c) 2000-2017 Benoît Minisini <g4mba5@gmail.com>

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

#ifndef __CTRAYICON_H
#define __CTRAYICON_H

#include <QSystemTrayIcon>

#include "gambas.h"
#include "main.h"

#include "CPicture.h"

#ifndef __CTRAYICON_CPP
extern GB_DESC TrayIconsDesc[];
extern GB_DESC TrayIconDesc[];
#else

#define THIS  ((CTRAYICON *)_object)
#define TRAYICON (THIS->indicator)

#endif

void TRAYICON_close_all(void);

typedef
	struct {
		GB_BASE ob;
		QSystemTrayIcon *indicator;
		GB_VARIANT_VALUE tag;
		CPICTURE *icon;
		char *tooltip;
		char *popup;
		}
	CTRAYICON;

class TrayIconManager : public QObject
{
	Q_OBJECT

public:

	static TrayIconManager manager;

	virtual bool eventFilter(QObject *o, QEvent *e);
	
public slots:

	void activated(QSystemTrayIcon::ActivationReason);
};
#endif
