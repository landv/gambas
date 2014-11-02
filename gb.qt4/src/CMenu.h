/***************************************************************************

  CMenu.h

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

#ifndef __CMENU_H
#define __CMENU_H

#include "gambas.h"

#include <QAction>
#include <QMenu>
#include <QMenuBar>
#include <QKeySequence>
#include <QList>
#include <QPoint>

#include "CWidget.h"
#include "CPicture.h"

#ifndef __CMENU_CPP
extern GB_DESC CMenuDesc[];
extern GB_DESC CMenuChildrenDesc[];
extern int MENU_popup_count;
#else

#define THIS  OBJECT(CMENU)
//#define CONTROL  OBJECT(CWIDGET)
#define ACTION ((QAction *)((CWIDGET *)_object)->widget)
#define PARENT_ACTION ((QAction *)((CWIDGET *)(THIS->parent))->widget)

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
		QKeySequence *accel;
    CPICTURE *picture;
		char *action;
		char *save_text;
    unsigned deleted : 1;
    unsigned toggle : 1;
		unsigned radio : 1;
    unsigned exec : 1;
		unsigned checked : 1;
		unsigned disabled : 1;
		unsigned visible : 1;
		unsigned init_shortcut : 1;
    }
  CMENU;

typedef
  QList<CMENU *> CMenuList;

class MyAction : public QAction
{
	Q_OBJECT

public:

	MyAction(QObject *parent);

protected:

	virtual bool event(QEvent *);
};

class CMenu : public QObject
{
  Q_OBJECT

public:

  static CMenu manager;
	static QHash<QAction *, CMENU *> dict;

  //static void unrefChildren(QWidget *wid);
  //static void enableAccel(CMENU *item, bool enable, bool rec = false);
  static void hideSeparators(CMENU *item);

public slots:

  void slotTriggered(QAction *);
  void slotToggled(bool);
  void slotDestroyed();
  void slotShown();
  void slotHidden();
};

void CMENU_popup(CMENU *_object, const QPoint &pos);

#endif
