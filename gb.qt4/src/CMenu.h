/***************************************************************************

	CMenu.h

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

#ifndef __CMENU_H
#define __CMENU_H

#include <QAction>
#include <QMenu>
#include <QMenuBar>
#include <QKeySequence>
#include <QList>
#include <QPoint>

#include "gambas.h"

#include "CWidget.h"
#include "CPicture.h"

#ifndef __CMENU_CPP

extern GB_DESC CMenuDesc[];
extern GB_DESC CMenuChildrenDesc[];
extern int MENU_popup_count;

#else

#define THIS  OBJECT(CMENU)
#define THIS_EXT ((CMENU_EXT *)(THIS->widget.ext))
#define ACTION ((QAction *)((CWIDGET *)_object)->widget)
#define PARENT_ACTION ((QAction *)((CWIDGET *)(THIS->parent))->widget)

#define CMENU_is_toplevel(_menu) (GB.Is((_menu)->parent, CLASS_Window))

#define GET_MENU_SENDER(_menu) CMENU *_menu = CMenu::dict[((QMenu *)sender())->menuAction()]

#endif

typedef 
	struct {
		GB_VARIANT_VALUE tag;
		void *proxy;
		char *action;
	}
	CMENU_EXT;

typedef
	struct _CMENU {
		CWIDGET widget;
		void *parent;
		QWidget *toplevel;
		QMenu *menu;
		QKeySequence *accel;
		CPICTURE *picture;
		char *save_text;
		unsigned deleted : 1;
		unsigned toggle : 1;
		unsigned radio : 1;
		unsigned exec : 1;
		unsigned checked : 1;
		unsigned disabled : 1;
		unsigned visible : 1;
		unsigned init_shortcut : 1;
		unsigned opened : 1;
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

	void slotTriggered();
	void slotToggled(bool);
	void slotDestroyed();
	void slotShown();
	void slotHidden();
};

#if QT_VERSION >= QT_VERSION_CHECK(5,0,0) && QT_VERSION < QT_VERSION_CHECK(5,5,0)
class MyMenu: public QMenu
{
public:
	
	virtual void setVisible(bool visible);
};
#endif

void CMENU_popup(CMENU *_object, const QPoint &pos);
void CMENU_update_menubar(CWINDOW *window);

#endif
