/***************************************************************************

  CGridView.h

  (c) 2000-2005 Beno� Minisini <gambas@users.sourceforge.net>

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

#ifndef __CGRIDVIEW_H
#define __CGRIDVIEW_H

#include "gambas.h"

#include <qasciidict.h>
#include <qtable.h>
#include <qevent.h>

#include "CWidget.h"
#include "CContainer.h"
#include "CPicture.h"

#ifndef __CGRIDVIEW_CPP
extern GB_DESC CGridViewDesc[];
extern GB_DESC CGridItemDesc[];
extern GB_DESC CGridRowsDesc[];
extern GB_DESC CGridColumnsDesc[];
extern GB_DESC CGridRowDesc[];
extern GB_DESC CGridColumnDesc[];
#else

#define THIS      ((CGRIDVIEW *)_object)
#define WIDGET    ((QTable *)((CWIDGET *)_object)->widget)
#define MANAGER   &CGridView::manager
#define ITEM      ((MyTableItem *)WIDGET->item(THIS->row, THIS->col))

#endif

class MyTableItem;

typedef
  struct {
    CWIDGET widget;
    int row;
    int col;
    int row_item;
    int col_item;
    //QAsciiDict<MyListViewItem> *dict;
    //long lock;
    }
  CGRIDVIEW;

class MyTableItem : public QTableItem
{
public:

  MyTableItem(QTable *table);
  ~MyTableItem();

  CPICTURE *picture;
  CGRIDVIEW *container;
  char *key;
  int align;
  int bg;
  int fg;

  void setPicture(GB_OBJECT *pict);
  int alignment() const;

private:

  void initData(void);
};

class MyTable : public QTable
{
  Q_OBJECT

public:

  MyTable(QWidget *parent);

  void paintFocus( QPainter *p, const QRect &r );

  void setRowHeight(int row, long height);
  void setColumnWidth(int col, long width);

protected:

  virtual void fontChange(const QFont &);

private:

  QSize tableSize() const;
};

class CGridView : public QObject
{
  Q_OBJECT

public:

  static CGridView manager;
  static MyTableItem *getItem(QTable *, long, long, bool);
  static void fillItems(QTable *);
  static bool check(QTable *, long, long);
  static bool checkRow(QTable *, long);
  static bool checkCol(QTable *, long);

public slots:

  //void changed(void);
  void clicked(void);
  void activated(void);
  void scrolled(void);

private:

  static bool created;
};

#endif
