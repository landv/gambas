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
#include <qmenudata.h>
#include <qpopupmenu.h>
#include <qintdict.h>
#include <qptrlist.h>
#include <qkeysequence.h>

#include "CWidget.h"
#include "CPicture.h"

#ifndef __CMENU_CPP
extern GB_DESC CMenuDesc[];
extern GB_DESC CMenuChildrenDesc[];
#else

#define THIS  OBJECT(CMENU)
#define CONTROL  OBJECT(CWIDGET)

#define QPOPUPMENU(object) ((QPopupMenu *)((CWIDGET *)object)->widget)
#define QMENUDATA(object) ((QMenuData *)((CWIDGET *)object)->widget)

#define CMENU_is_popup(_menu) (((CMENU *)_menu)->children != NULL)
#define CMENU_is_top(_menu) (((CMENU *)_menu)->parent == NULL)
#define CMENU_is_visible(_menu) (CWIDGET_test_flag(_menu, WF_VISIBLE))
#define CMENU_is_separator(_menu) (((CMENU *)_menu)->text == 0 || *((CMENU *)_menu)->text == 0)

#endif

/*
typedef
  struct {
    GB_BASE ob;
    struct _CMENU *menu;
    CPICTURE *picture;
    int id;
    //char *key;
    }
  CMENUITEM;
*/

typedef
  struct _CMENU {
    CWIDGET widget;
    char *text;
    CPICTURE *picture;
    QMenuData *container;
    struct _CMENU *parent;
    QWidget *toplevel;
    QList<struct _CMENU> *children;
    int id;
    int pos;
    QKeySequence *accel;
    unsigned enabled : 1;
    unsigned checked : 1;
    unsigned deleted : 1;
    unsigned toggle : 1;
    unsigned noshortcut : 1;
    unsigned stretch : 1;
    unsigned exec : 1;
    }
  CMENU;

typedef
  QIntDict<CMENU> CMenuDict;

typedef
  QPtrList<CMENU> CMenuList;


class CMenu : public QObject
{
  Q_OBJECT

public:

  static CMenu manager;
  static CMenuDict dict;

  static void unrefChildren(CMenuList *list);
  static void enableAccel(CMENU *item, bool enable);
  static void hideSeparators(CMENU *item);

public slots:

  void activated(int);
  void shown();
  void hidden();
  void destroy();
};

#endif
