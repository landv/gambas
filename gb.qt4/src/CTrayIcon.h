/***************************************************************************

  CTrayIcon.h

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

#ifndef __CTRAYICON_H
#define __CTRAYICON_H

#include "gambas.h"

#include <qobject.h>
#include <qwidget.h>
//Added by qt3to4:
#include <QEvent>

#include "systemtrayicon.h"

#include "CPicture.h"

#ifndef __CTRAYICON_CPP
extern GB_DESC CTrayIconDesc[];
extern GB_DESC CTrayIconsDesc[];
extern int TRAYICON_count;
#else

#define THIS  ((CTRAYICON *)_object)
#define WIDGET (THIS->widget)

#endif

void CTRAYICON_close_all(void);

class MyTrayIcon: public SystemTrayIcon
{
public:
	MyTrayIcon();
	QPixmap icon() const { return _icon; }
	void setIcon(QPixmap &icon);
protected:
	virtual void drawContents(QPainter *p);
private:
	QPixmap _icon;
};

typedef
  struct {
    GB_BASE ob;
    MyTrayIcon *widget;
    GB_VARIANT_VALUE tag;
    CPICTURE *icon;
    char *tooltip;
		char *popup;
    }
  CTRAYICON;

class CTrayIcon : public QObject
{
  Q_OBJECT

public:

  static CTrayIcon manager;

public slots:

  void error(void);
  void embedded(void);
  void closed(void);

protected:

  bool eventFilter(QObject *, QEvent *);
};

#endif
