/***************************************************************************

  CListView.h

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

#ifndef __CLISTVIEW_H
#define __CLISTVIEW_H

#include "gambas.h"

#include <QTreeWidgetItem>
#include <QTreeWidget>
#include <QEvent>
#include <QPixmap>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QDragMoveEvent>
#include <QDragLeaveEvent>

#include "CWidget.h"
#include "CPicture.h"

#ifndef __CLISTVIEW_CPP
extern GB_DESC CTreeView2ItemDesc[];
extern GB_DESC CTreeView2Desc[];

extern GB_DESC CListView2ItemDesc[];
extern GB_DESC CListView2Desc[];

extern GB_DESC CColumnView2ColumnDesc[];
extern GB_DESC CColumnView2ColumnsDesc[];
extern GB_DESC CColumnView2ItemDesc[];
extern GB_DESC CColumnView2Desc[];
#else

#define QTREEWIDGET(object) ((MyTreeWidget *)((CWIDGET *)object)->widget)
#define WIDGET QTREEWIDGET(_object)
#define THIS ((CLISTVIEW *)_object)

#endif

class MyTreeWidgetItem;

typedef
  struct {
    CWIDGET widget;
    QHash<QByteArray, MyTreeWidgetItem*> *dict;
    MyTreeWidgetItem *item;
    MyTreeWidgetItem *save;
    short sorted;
    char compare;
    unsigned asc : 1;
    unsigned rename : 1;
    }
  CLISTVIEW;

class MyTreeWidget : public QTreeWidget
{
  Q_OBJECT

public:

  MyTreeWidget(QWidget *parent);

	int _column;
	bool _auto_resize;

	virtual void dragEnterEvent(QDragEnterEvent *);
	virtual void dragMoveEvent( QDragMoveEvent *);
	virtual void dragLeaveEvent(QDragLeaveEvent *);	
	virtual void dropEvent(QDropEvent *);

	//virtual void setColumnText(int col, const QString &s);

  bool isAutoResize() { return _auto_resize; }
  void setAutoResize(bool a);
  void setAutoResize(int col, bool a);
  int minimumWidth(int col);
	
	bool isRenaming() const { return state() == EditingState; }
	
	MyTreeWidgetItem *firstChild() { return (MyTreeWidgetItem *)(invisibleRootItem()->child(0)); }
};

class MyTreeWidgetItem : public QTreeWidgetItem
{
public:

  MyTreeWidgetItem(CLISTVIEW *cont, MyTreeWidget *parent);
  MyTreeWidgetItem(CLISTVIEW *cont, MyTreeWidget *parent, MyTreeWidgetItem *after);
  MyTreeWidgetItem(CLISTVIEW *cont, MyTreeWidgetItem *parent);
  MyTreeWidgetItem(CLISTVIEW *cont, MyTreeWidgetItem *parent, MyTreeWidgetItem *after);
  virtual ~MyTreeWidgetItem();

  CPICTURE *picture;
  char *key;
  CLISTVIEW *container;

  void setPixmap(const QPixmap & pixmap);
  void setPicture(GB_OBJECT *pict);

  //virtual int compare(QTreeWidgetItem *i, int col, bool ascending) const;
	bool operator< (const QTreeWidgetItem &other) const;
  //virtual void startRename(int col);
  //virtual void cancelRename(int col);
  //virtual void setSelected(bool s);
	
	void setEditable(bool e);
	bool isEditable() const { return flags() & Qt::ItemIsEditable; }
	
	int index() { return parent()->indexOfChild(this); }
	
	MyTreeWidgetItem *parent() const;
	
	MyTreeWidgetItem *firstChild() { return (MyTreeWidgetItem *)(child(0)); }
	MyTreeWidgetItem *nextSibling() { return (MyTreeWidgetItem *)(parent()->child(parent()->indexOfChild(this) + 1)); }
  
private:

  void initData(CLISTVIEW *);
};


class CListView : public QObject
{
  Q_OBJECT

public:

  static CListView manager;
  static MyTreeWidgetItem *getItem(CLISTVIEW *listview, char *key);
  //static QTreeWidgetItem *removeItem(CLISTVIEW *listview, char *key);

public slots:

  void selected(void);
  void activated(QTreeWidgetItem *);
  void clicked(QTreeWidgetItem *);
  void renamed(QTreeWidgetItem *, int);
  //void columnClicked(QTreeWidgetItem *, const QPoint &, int);
  void expanded(QTreeWidgetItem *);
  void collapsed(QTreeWidgetItem *);
  void headerClicked(int);
  void headerSizeChange(int, int, int);

private:

  void raiseEvent(int, QTreeWidgetItem *);
};

#endif
