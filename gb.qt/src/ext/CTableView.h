/***************************************************************************

  CTableView.h

  The TableView control

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

#ifndef __CTABLEVIEW_H
#define __CTABLEVIEW_H

#include "gambas.h"
#include "../gb.qt.h"

#include <qasciidict.h>
#include <qtable.h>
#include <qevent.h>

#ifndef __CTABLEVIEW_CPP
extern GB_DESC CTableViewDesc[];
extern GB_DESC CTableViewDataDesc[];
extern GB_DESC CTableItemDesc[];
extern GB_DESC CTableRowsDesc[];
extern GB_DESC CTableColumnsDesc[];
extern GB_DESC CTableRowDesc[];
extern GB_DESC CTableColumnDesc[];
#else

#define THIS      ((CTABLEVIEW *)_object)
#define WIDGET    ((MyTable *)((QT_WIDGET *)_object)->widget)
#define MANAGER   &CTableView::manager
#define ITEM      ((MyTableItem *)WIDGET->item(THIS->row, THIS->col))

#endif

class MyTableItem;

typedef
  struct {
    QT_WIDGET widget;
    int row;
    int col;
    QT_PICTURE picture;
    bool autoresize;
    }
  CTABLEVIEW;

class MyTableItem : public QTableItem
{
public:

  MyTableItem(QTable *table);
  ~MyTableItem();

  virtual int alignment() const;
  virtual QString text();
  virtual QPixmap pixmap();

  virtual void paint( QPainter *p, const QColorGroup &cg, const QRect &cr, bool selected);
  void setAlignment(int a) { _alignment = a; }
  void setBackground(long col) { _bg = col; }
  void setForeground(long col) { _fg = col; }
  int background() { return _bg; }
  int foreground() { return _fg; }
  void setPicture(GB_OBJECT *pict);
  int rtti() const { return 1000; }
  virtual QSize sizeHint() const;

	void invalidate();
  bool invalidate(int, int);

private:

  bool _valid;
  int _alignment;
  long _bg, _fg;
  CTABLEVIEW *_tableView;

  void getData();

};

class MyTable : public QTable
{
  Q_OBJECT

public:

  MyTable(QWidget *parent);
  ~MyTable();

  void paintFocus( QPainter *p, const QRect &r );

  virtual void setRowHeight(int row, long height);
  virtual void setColumnWidth(int col, long width);
  //virtual void adjustColumn( int col );

  void setHeaders(int);
  int headers() const { return _header; }

  //void setTableView(CTABLEVIEW *tv) { tableView = tv; }

  void resizeData( int ) { }
  QTableItem *item( int r, int c ) const;
  void setItem(int r, int c, QTableItem *i) { }
  void clearCell( int r, int c ) {  }
  void insertWidget( int r, int c, QWidget *w ) { }
  QWidget *cellWidget( int r, int c ) const { return 0; }
  void clearCellWidget( int r, int c ) { }
  MyTableItem *item() const { return _item; }
  void takeItem( QTableItem * ) { }

  void swapRows(int row1, int row2, bool swapHeader = FALSE) { }
  void swapColumns(int col1, int col2, bool swapHeader = FALSE) { }
  void swapCells(int row1, int col1, int row2, int col2) { }

  void updateRow(int row);
  void updateColumn(int col);

  void setCurrentCell(int row, int col);

  //QString definition() const;
  //void setDefinition(QString &def);

public slots:

  void setNumCols(int);
  void setNumRows(int);

protected:

  void fontChange(const QFont &old);

private:

  //QSize tableSize() const;
  void updateHeaders();

  int _header;
  //MyTableItem *_items[];
  MyTableItem *_item;
  int _rows;
  int _cols;
};

class CTableView : public QObject
{
  Q_OBJECT

public:

  static CTableView manager;
  static bool check(QTable *, long, long);
  static bool checkRow(QTable *, long);
  static bool checkCol(QTable *, long);

public slots:

  void changed(void);
  void selected(void);
  void clicked(void);
  void activated(void);
  void scrolled(void);
  void columnClicked(int);
  void rowClicked(int);
};

#endif
