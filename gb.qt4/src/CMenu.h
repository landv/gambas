/***************************************************************************

  CMenu.h

  The Menu class

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

#ifndef __CMENU_H
#define __CMENU_H

#include "gambas.h"

#include <QAction>
#include <QMenu>
#include <QMenuBar>
#include <QKeySequence>
#include <QList>

#include "CWidget.h"
#include "CPicture.h"

#ifndef __CMENU_CPP
extern GB_DESC CMenuDesc[];
extern GB_DESC CMenuChildrenDesc[];
#else

#define THIS  OBJECT(CMENU)
//#define CONTROL  OBJECT(CWIDGET)
#define ACTION ((QAction *)((CWIDGET *)_object)->widget)

#define CMENU_is_toplevel(_menu) (GB.Is((_menu)->parent, CLASS_Window))

#define GET_MENU_SENDER(_menu) CMENU *_menu = CMenu::dict[((QMenu *)sender())->menuAction()]

#endif

typedef
  struct _CMENU {
    CWIDGET widget;
    void *parent;
    //QList<struct _CMENU *> children;
    //QMenu *parentMenu;
    //QMenuBar *parentMenuBar;
    QWidget *toplevel;
    QMenu *menu;
    CPICTURE *picture;
    unsigned deleted : 1;
    unsigned toggle : 1;
    unsigned noshortcut : 1;
    unsigned exec : 1;
    }
  CMENU;

typedef
  QList<CMENU *> CMenuList;


class CMenu : public QObject
{
  Q_OBJECT

public:

  static CMenu manager;
	static QHash<QAction *, CMENU *> dict;

  //static void unrefChildren(QWidget *wid);
  static void enableAccel(CMENU *item, bool enable);
  static void hideSeparators(CMENU *item);

public slots:

  void slotTriggered(QAction *);
  void slotDestroyed();
  void slotShown();
  void slotHidden();
};

#endif
