/***************************************************************************

	CTreeView.h

	(c) 2000-2012 Benoît Minisini <gambas@users.sourceforge.net>

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

#ifndef __CTREEVIEW_H
#define __CTREEVIEW_H

#include "gambas.h"

#include <q3asciidict.h>
#include <q3listview.h>
#include <qevent.h>
//Added by qt3to4:
#include <QPixmap>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QDragMoveEvent>
#include <QDragLeaveEvent>

#include "CWidget.h"
#include "CPicture.h"

#ifndef __CTREEVIEW_CPP
extern GB_DESC CTreeViewItemDesc[];
extern GB_DESC CListViewItemDesc[];
extern GB_DESC CColumnViewItemDesc[];

extern GB_DESC CTreeViewDesc[];
extern GB_DESC CListViewDesc[];

extern GB_DESC CColumnViewColumnDesc[];
extern GB_DESC CColumnViewColumnsDesc[];

extern GB_DESC CColumnViewDesc[];
#else

#define QLISTVIEW(object) ((MyListView *)((CWIDGET *)object)->widget)
#define WIDGET QLISTVIEW(_object)
#define THIS ((CTREEVIEW *)_object)

#endif

class MyListViewItem;

typedef
	struct {
		CWIDGET widget;
		Q3AsciiDict<MyListViewItem> *dict;
		MyListViewItem *item;
		MyListViewItem *save;
		char *before;
		short sorted;
		char compare;
		unsigned asc : 1;
		unsigned rename : 1;
		}
	CTREEVIEW;

/*
typedef
	struct {
		GB_BASE ob;
		MyListViewItem *item;
		CPICTURE *picture;
		char *key;
		}
	CTREEVIEWITEM;
*/


class MyListView : public Q3ListView
{
	Q_OBJECT

public:

	MyListView(QWidget *parent);
	MyListViewItem *last;

	int _column;
	bool _auto_resize;

	virtual void contentsDragEnterEvent(QDragEnterEvent *);
	virtual void contentsDragMoveEvent( QDragMoveEvent *);
	virtual void contentsDropEvent(QDropEvent *);
	virtual void contentsDragLeaveEvent(QDragLeaveEvent *);	

	virtual void setColumnText(int col, const QString &s);

	bool isAutoResize() { return _auto_resize; }
	void setAutoResize(bool a);
	void setAutoResize(int col, bool a);
	int minimumWidth(int col);

public slots:
	virtual void clear();
};

class MyListViewItem : public Q3ListViewItem
{
public:

	MyListViewItem(CTREEVIEW *cont, MyListView *parent);
	MyListViewItem(CTREEVIEW *cont, MyListView *parent, MyListViewItem *after);
	MyListViewItem(CTREEVIEW *cont, MyListViewItem *parent);
	MyListViewItem(CTREEVIEW *cont, MyListViewItem *parent, MyListViewItem *after);
	virtual ~MyListViewItem();

	CPICTURE *picture;
	char *key;
	//double *sortKey;
	//int nSortKey;
	CTREEVIEW *container;
	MyListViewItem *last;
	MyListViewItem *prev;

	void setPixmap(const QPixmap & pixmap) { Q3ListViewItem::setPixmap(0, pixmap); }
	void setPicture(GB_OBJECT *pict);
	//void setSortKey(int col, int k);

	virtual int compare(Q3ListViewItem *i, int col, bool ascending) const;
	virtual void startRename(int col);
	virtual void cancelRename(int col);
	//virtual void setSelected(bool s);
	//virtual void paintCell(QPainter *p, const QColorGroup &cg, int column, int width, int align);
	
private:

	void initData(CTREEVIEW *);
};


class CTreeView : public QObject
{
	Q_OBJECT

public:

	static CTreeView manager;
	static MyListViewItem *getItem(CTREEVIEW *listview, char *key);
	//static QListViewItem *removeItem(CLISTVIEW *listview, char *key);

public slots:

	void selected(void);
	void activated(Q3ListViewItem *);
	void clicked(Q3ListViewItem *);
	void renamed(Q3ListViewItem *, int);
	//void columnClicked(QListViewItem *, const QPoint &, int);
	void expanded(Q3ListViewItem *);
	void collapsed(Q3ListViewItem *);
	void headerClicked(int);
	void headerSizeChange(int, int, int);

private:

	void raiseEvent(int, Q3ListViewItem *);
};

#endif
