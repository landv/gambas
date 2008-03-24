/***************************************************************************

  CTreeView.h

  The TreeView class

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

#ifndef __CTREEVIEW_H
#define __CTREEVIEW_H

#include "gambas.h"

#include <qasciidict.h>
#include <qlistview.h>
#include <qevent.h>

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
    QAsciiDict<MyListViewItem> *dict;
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


class MyListView : public QListView
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

class MyListViewItem : public QListViewItem
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

  void setPixmap(const QPixmap & pixmap) { QListViewItem::setPixmap(0, pixmap); }
  void setPicture(GB_OBJECT *pict);
  //void setSortKey(int col, int k);

  virtual int compare(QListViewItem *i, int col, bool ascending) const;
  virtual void startRename(int col);
  virtual void cancelRename(int col);
  //virtual void setSelected(bool s);
  
  MyListViewItem *previousSibling();
  MyListViewItem *findAbove();
  MyListViewItem *findBelow();
  
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
  void activated(QListViewItem *);
  void clicked(QListViewItem *);
  void renamed(QListViewItem *, int);
  //void columnClicked(QListViewItem *, const QPoint &, int);
  void expanded(QListViewItem *);
  void collapsed(QListViewItem *);
  void headerClicked(int);
  void headerSizeChange(int, int, int);

private:

  void raiseEvent(int, QListViewItem *);
};

#endif
