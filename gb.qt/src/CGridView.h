/***************************************************************************

  CGridView.h

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

#ifndef __CGRIDVIEW_H
#define __CGRIDVIEW_H

#include "gambas.h"
#include "../gb.qt.h"

#include <qasciidict.h>
#include <qtable.h>
#include <qevent.h>

#include "CWidget.h"
#include "CPicture.h"
#include "CFont.h"

#ifndef __CGRIDVIEW_CPP
extern GB_DESC CGridViewDesc[];
extern GB_DESC CGridViewDataDesc[];
extern GB_DESC CGridItemDesc[];
extern GB_DESC CGridRowsDesc[];
extern GB_DESC CGridColumnsDesc[];
extern GB_DESC CGridRowDesc[];
extern GB_DESC CGridColumnDesc[];
#else

#define THIS      ((CGRIDVIEW *)_object)
#define WIDGET    ((MyTable *)((CWIDGET *)_object)->widget)
#define MANAGER   &CGridView::manager

#endif

typedef
  struct {
    CWIDGET widget;
    int row;
    int col;
    bool autoresize;
    }
  CGRIDVIEW;

struct MyTableData
{
	MyTableData();
	~MyTableData();
	void init();
	void clear();
	
  short alignment;
  short padding;
  int bg;
  int fg;
  CFONT *font;
  char *text;
  CPICTURE *pict;
};

class MyTableItem : public QTableItem
{
public:

  MyTableItem(QTable *table, CGRIDVIEW *view);
  ~MyTableItem();

  virtual int alignment() const;
  virtual QString text();
  virtual QPixmap pixmap();

  virtual void paint( QPainter *p, const QColorGroup &cg, const QRect &cr, bool selected);
  int rtti() const { return 1000; }
  virtual QSize sizeHint() const;

	void invalidate();
  bool invalidate(int, int);
  
  MyTableData *data(bool create = false);
  void clear();
  void clearAll() { invalidate(); _dict.clear(); }
  
private:

	static int getKey(int row, int col);

  CGRIDVIEW *_view;
	MyTableData _default;
  QIntDict<MyTableData> _dict;
	MyTableData *_data;
};

class MyTable : public QTable
{
  Q_OBJECT

public:

  MyTable(QWidget *parent, CGRIDVIEW *view);
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

  virtual void setCurrentCell(int row, int col);
  
  void clear() { _item->clearAll(); }
  
  void unselectRow(int row);
  void selectRow(int row, bool update = true);
  void selectRows(int row, int count);
  bool isRowReallySelected(int row);
  int findSelection(int row);

public slots:

  virtual void setNumCols(int);
  virtual void setNumRows(int);
  virtual void columnWidthChanged(int col);

protected:

  virtual void fontChange(const QFont &old);
  virtual void resizeEvent(QResizeEvent *e);

private:

	void drawContents(QPainter *p, int clipx, int clipy, int clipw, int cliph);
  void updateHeaders();
  void updateLastColumn();
  int _header;
  MyTableItem *_item;
  int _rows;
  int _cols;
  int _last_col_width;
};

class CGridView : public QObject
{
  Q_OBJECT

public:

  static CGridView manager;
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
  void columnResized(int);
  void rowResized(int);
};

#endif
