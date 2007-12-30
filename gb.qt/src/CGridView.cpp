/***************************************************************************

  CGridView.cpp

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


#define __CGRIDVIEW_CPP

#include <qapplication.h>
#include <qheader.h>

#include "gambas.h"

#include "CWidget.h"
#include "CPicture.h"
#include "CGridView.h"

//static long EVENT_Change;    /* selection change */
static long EVENT_Click;     /* simple click */
static long EVENT_Activate;  /* double click */
static long EVENT_Scroll;


/***************************************************************************

  class MyTableItem

***************************************************************************/

MyTableItem::MyTableItem(QTable *table)
: QTableItem(table, QTableItem::Never, 0)
{
  picture = NULL;
  key = NULL;
  align = AlignLeft | SingleLine;
  //container = (CGRIDVIEW *)CWidget::get(table);
}

MyTableItem::~MyTableItem()
{
  GB.Unref((void **)&picture);
  /*if (container->item == this)
  	container->item = 0;*/
}

void MyTableItem::setPicture(GB_OBJECT *pict)
{
  SET_PIXMAP(setPixmap, &picture, pict);
}

int MyTableItem::alignment() const
{
  return align | AlignVCenter;
}


/***************************************************************************

  class MyTable

***************************************************************************/

MyTable::MyTable(QWidget *parent) :
QTable(0, 0, parent)
{
}

void MyTable::paintFocus( QPainter *p, const QRect &r )
{
}

QSize MyTable::tableSize() const
{
  return QSize( columnPos( numCols() - 1 ) + columnWidth( numCols() - 1 ),
    rowPos( numRows() - 1 ) + rowHeight( numRows() - 1 ) );
}

/*
void MyTable::columnWidthChanged( int col )
{
  updateContents( columnPos( col ), 0, contentsWidth(), contentsHeight() );
  QSize s(tableSize());
  int w = contentsWidth();
  resizeContents( s.width(), s.height() );


  if ( contentsWidth() < w )
	  repaintContents( s.width(), 0, w - s.width() + 1, contentsHeight(), TRUE );
  else
	  repaintContents( w, 0, s.width() - w + 1, contentsHeight(), FALSE );


  setLeftMargin(0); // updateGeometries();

  for ( int j = col; j < numCols(); ++j )
  {
  	for ( int i = 0; i < numRows(); ++i )
    {
	    QWidget *w = cellWidget( i, j );
	    if ( !w )
		    continue;
	    moveChild( w, columnPos( j ), rowPos( i ) );
	    w->resize( columnWidth( j ) - 1, rowHeight( i ) - 1 );
	  }
  }
}

void MyTable::rowHeightChanged( int row )
{
  updateContents( 0, rowPos( row ), contentsWidth(), contentsHeight() );
  QSize s( tableSize() );
  int h = contentsHeight();
  resizeContents( s.width(), s.height() );
  if ( contentsHeight() < h )
  	repaintContents( 0, contentsHeight(), contentsWidth(), h - s.height() + 1, TRUE );
  else
	  repaintContents( 0, h, contentsWidth(), s.height() - h + 1, FALSE );

  setLeftMargin(0); // updateGeometries();

  for ( int j = row; j < numRows(); ++j )
  {
	  for ( int i = 0; i < numCols(); ++i )
    {
	    QWidget *w = cellWidget( j, i );
	    if ( !w )
		    continue;
	    moveChild( w, columnPos( i ), rowPos( j ) );
	    w->resize( columnWidth( i ) - 1, rowHeight( j ) - 1 );
	  }
  }
}
*/

void MyTable::setRowHeight(int row, long height)
{
  if (height < 0)
    adjustRow(row);
  else
    QTable::setRowHeight(row, height);
}


void MyTable::setColumnWidth(int col, long width)
{
  if (width < 0)
    adjustColumn(col);
  else
    QTable::setColumnWidth(col, width);
}

void MyTable::fontChange(const QFont &f)
{
  int i;
  int h;

  //qDebug("font change");

  h = fontMetrics().lineSpacing() + 2;
  for (i = 0; i < numRows(); i++)
    QTable::setRowHeight(i, h);

  QTable::fontChange(f);
}


/***************************************************************************

  GridView

***************************************************************************/

BEGIN_METHOD(CGRIDVIEW_new, GB_OBJECT parent)

  MyTable *wid = new MyTable(QCONTAINER(VARG(parent)));

  //QObject::connect(wid, SIGNAL(currentChanged(int, int)), MANAGER, SLOT(changed()));
  QObject::connect(wid, SIGNAL(doubleClicked(int, int, int, const QPoint &)), MANAGER, SLOT(activated()));
  QObject::connect(wid, SIGNAL(clicked(int, int, int, const QPoint &)), MANAGER, SLOT(clicked()));
  QObject::connect(wid, SIGNAL(contentsMoving(int, int)), MANAGER, SLOT(scrolled()));

  CWIDGET_new(wid, (void *)_object, "GridView");

  THIS->row = -1;
  THIS->col = -1;

  wid->setSelectionMode(QTable::NoSelection);
  wid->setTopMargin(0);
  wid->verticalHeader()->hide();
  wid->setLeftMargin(0);
  wid->horizontalHeader()->hide();
  wid->show();

END_METHOD


BEGIN_PROPERTY(CGRIDVIEW_row)

  if (READ_PROPERTY)
    GB.ReturnInteger(WIDGET->currentRow());
  else
    WIDGET->setCurrentCell(VPROP(GB_INTEGER), WIDGET->currentColumn());

END_PROPERTY


BEGIN_PROPERTY(CGRIDVIEW_column)

  if (READ_PROPERTY)
    GB.ReturnInteger(WIDGET->currentColumn());
  else
    WIDGET->setCurrentCell(WIDGET->currentRow(), VPROP(GB_INTEGER));

END_PROPERTY


BEGIN_METHOD_VOID(CGRIDVIEW_clear)

  long rows = WIDGET->numRows();
  WIDGET->setNumRows(0);
  WIDGET->setNumRows(rows);
  CGridView::fillItems(WIDGET);

END_METHOD


BEGIN_METHOD(CGRIDVIEW_get, GB_INTEGER row; GB_INTEGER col)

  long row = VARG(row);
  long col = VARG(col);

  if (CGridView::check(WIDGET, row, col))
    return;

  THIS->row = row;
  THIS->col = col;

  RETURN_SELF();

END_METHOD


BEGIN_PROPERTY(CGRIDVIEW_current)

  THIS->row = WIDGET->currentRow();
  THIS->col = WIDGET->currentColumn();

  RETURN_SELF();

END_PROPERTY


BEGIN_PROPERTY(CGRIDVIEW_item)

	if (THIS->row_item < 0 || THIS->col_item < 0)
	{
		GB.ReturnNull();
	}
	else
	{
		THIS->row = THIS->row_item;
		THIS->col = THIS->col_item;

		RETURN_SELF();
	}

END_PROPERTY


BEGIN_PROPERTY(CGRIDVIEW_self)

  THIS->row = -1;
  THIS->col = -1;

  RETURN_SELF();

END_PROPERTY


BEGIN_PROPERTY(CGRIDVIEW_grid)

  if (READ_PROPERTY)
    GB.ReturnBoolean(WIDGET->showGrid());
  else
    WIDGET->setShowGrid(VPROP(GB_BOOLEAN));

END_PROPERTY


BEGIN_PROPERTY(CGRIDVIEW_client_width)

  WIDGET->updateScrollBars();
  GB.ReturnInteger(WIDGET->clipper()->width());

END_PROPERTY


BEGIN_PROPERTY(CGRIDVIEW_client_height)

  WIDGET->updateScrollBars();
  GB.ReturnInteger(WIDGET->clipper()->height());

END_PROPERTY


BEGIN_METHOD(CGRIDVIEW_find, GB_INTEGER x; GB_INTEGER y)

	int x = VARG(x) + WIDGET->contentsX();
	int y = VARG(y) + WIDGET->contentsY();
	int row = WIDGET->rowAt(y);
	int col = WIDGET->columnAt(x);

	THIS->row_item = row;
	THIS->col_item = col;
	GB.ReturnBoolean(row < 0 || col < 0);

	//THIS->item = CGridView::getItem(WIDGET, row, col, false);
	//GB.ReturnBoolean(THIS->item != 0);

END_METHOD

/***************************************************************************

  GridItem

***************************************************************************/


BEGIN_PROPERTY(CGRIDITEM_row)

  GB.ReturnInteger(THIS->row);

END_PROPERTY


BEGIN_PROPERTY(CGRIDITEM_column)

  GB.ReturnInteger(THIS->col);

END_PROPERTY


BEGIN_PROPERTY(CGRIDITEM_x)

  GB.ReturnInteger(WIDGET->columnPos(THIS->col) - WIDGET->contentsX());

END_PROPERTY


BEGIN_PROPERTY(CGRIDITEM_y)

  GB.ReturnInteger(WIDGET->rowPos(THIS->row) - WIDGET->contentsY());

END_PROPERTY


BEGIN_PROPERTY(CGRIDITEM_width)

  GB.ReturnInteger(WIDGET->columnWidth(THIS->col));

END_PROPERTY


BEGIN_PROPERTY(CGRIDITEM_height)

  GB.ReturnInteger(WIDGET->rowHeight(THIS->row));

END_PROPERTY


BEGIN_PROPERTY(CGRIDITEM_text)

  if (READ_PROPERTY)
    GB.ReturnNewZeroString(TO_UTF8(ITEM->text()));
  else
  {
    ITEM->setText(QSTRING_PROP());
    WIDGET->updateCell(THIS->row, THIS->col);
  }

END_PROPERTY


BEGIN_PROPERTY(CGRIDITEM_alignment)

  if (READ_PROPERTY)
    GB.ReturnInteger(ITEM->align);
  else
  {
    ITEM->align = VPROP(GB_INTEGER);
    WIDGET->updateCell(THIS->row, THIS->col);
  }

END_PROPERTY


BEGIN_PROPERTY(CGRIDITEM_picture)

  if (READ_PROPERTY)
    GB.ReturnObject(ITEM->picture);
  else
  {
    ITEM->setPicture(PROP(GB_OBJECT));
    WIDGET->updateCell(THIS->row, THIS->col);
  }

END_PROPERTY


BEGIN_METHOD_VOID(CGRIDITEM_ensure_visible)

  WIDGET->ensureCellVisible(THIS->row, THIS->col);

END_METHOD

BEGIN_METHOD_VOID(CGRIDITEM_clear)

  WIDGET->clearCell(THIS->row, THIS->col);
  CGridView::getItem(WIDGET, THIS->row, THIS->col, false);
  WIDGET->updateCell(THIS->row, THIS->col);

END_METHOD


/***************************************************************************

  GridRows

***************************************************************************/

BEGIN_METHOD(CGRIDROWS_get, GB_INTEGER row)

  CGridView::checkRow(WIDGET, VARG(row));
  THIS->row = VARG(row);

  RETURN_SELF();

END_METHOD


BEGIN_PROPERTY(CGRIDROWS_count)

  if (READ_PROPERTY)
    GB.ReturnInteger(WIDGET->numRows());
  else
  {
    WIDGET->setNumRows(VPROP(GB_INTEGER));
    CGridView::fillItems(WIDGET);
  }

END_PROPERTY


BEGIN_PROPERTY(CGRIDROWS_height)

  long row = THIS->row;

  if (READ_PROPERTY)
  {
    if (row < 0)
      row = 0;

    GB.ReturnInteger(WIDGET->rowHeight(row));
  }
  else
  {
    if (row < 0)
    {
      for (row = 0; row < WIDGET->numRows(); row++)
        WIDGET->setRowHeight(row, VPROP(GB_INTEGER));
    }
    else
      WIDGET->setRowHeight(row, VPROP(GB_INTEGER));
  }

END_PROPERTY


BEGIN_PROPERTY(CGRIDROWS_resizable)

  if (READ_PROPERTY)
    GB.ReturnBoolean(WIDGET->rowMovingEnabled());
  else
    WIDGET->setRowMovingEnabled(VPROP(GB_BOOLEAN));

END_PROPERTY


/***************************************************************************

  GridCols

***************************************************************************/

BEGIN_METHOD(CGRIDCOLS_get, GB_INTEGER col)

  CGridView::checkCol(WIDGET, VARG(col));
  THIS->col = VARG(col);

  RETURN_SELF();

END_METHOD


BEGIN_PROPERTY(CGRIDCOLS_count)

  if (READ_PROPERTY)
    GB.ReturnInteger(WIDGET->numCols());
  else
  {
    WIDGET->setNumCols(VPROP(GB_INTEGER));
    CGridView::fillItems(WIDGET);
  }

END_PROPERTY


BEGIN_PROPERTY(CGRIDCOLS_width)

  long col = THIS->col;

  if (READ_PROPERTY)
  {
    if (col < 0)
      col = 0;

    GB.ReturnInteger(WIDGET->columnWidth(col));
  }
  else
  {
    if (col < 0)
    {
      for (col = 0; col < WIDGET->numCols(); col++)
        WIDGET->setColumnWidth(col, VPROP(GB_INTEGER));
    }
    else
      WIDGET->setColumnWidth(col, VPROP(GB_INTEGER));
  }

END_PROPERTY


BEGIN_PROPERTY(CGRIDCOLS_resizable)

  if (READ_PROPERTY)
    GB.ReturnBoolean(WIDGET->columnMovingEnabled());
  else
    WIDGET->setColumnMovingEnabled(VPROP(GB_BOOLEAN));

END_PROPERTY


/***************************************************************************

  Descriptions

***************************************************************************/

GB_DESC CGridItemDesc[] =
{
  GB_DECLARE(".GridCell", 0), GB_VIRTUAL_CLASS(),

  //GB_PROPERTY_READ("Row", "i", CGRIDITEM_row),
  //GB_PROPERTY_READ("Column", "i", CGRIDITEM_column),
  GB_PROPERTY("Picture", "Picture", CGRIDITEM_picture),
  GB_PROPERTY("Text", "s", CGRIDITEM_text),
  GB_PROPERTY("Alignment", "i", CGRIDITEM_alignment),

  GB_PROPERTY_READ("Row", "i", CGRIDITEM_row),
  GB_PROPERTY_READ("Column", "i", CGRIDITEM_column),
  GB_PROPERTY_READ("X", "i", CGRIDITEM_x),
  GB_PROPERTY_READ("Y", "i", CGRIDITEM_y),
  GB_PROPERTY_READ("Left", "i", CGRIDITEM_x),
  GB_PROPERTY_READ("Top", "i", CGRIDITEM_y),
  GB_PROPERTY_READ("Width", "i", CGRIDITEM_width),
  GB_PROPERTY_READ("Height", "i", CGRIDITEM_height),
  GB_PROPERTY_READ("W", "i", CGRIDITEM_width),
  GB_PROPERTY_READ("H", "i", CGRIDITEM_height),

  GB_METHOD("Clear", NULL, CGRIDITEM_clear, NULL),
  GB_METHOD("EnsureVisible", NULL, CGRIDITEM_ensure_visible, NULL),

  GB_END_DECLARE
};


GB_DESC CGridRowDesc[] =
{
  GB_DECLARE(".GridRow", 0), GB_VIRTUAL_CLASS(),

  GB_PROPERTY("Height", "i", CGRIDROWS_height),
  GB_PROPERTY("H", "i", CGRIDROWS_height),

  GB_END_DECLARE
};


GB_DESC CGridColumnDesc[] =
{
  GB_DECLARE(".GridColumn", 0), GB_VIRTUAL_CLASS(),

  GB_PROPERTY("Width", "i", CGRIDCOLS_width),
  GB_PROPERTY("W", "i", CGRIDCOLS_width),

  GB_END_DECLARE
};


GB_DESC CGridRowsDesc[] =
{
  GB_DECLARE(".GridRows", 0), GB_VIRTUAL_CLASS(),

  GB_METHOD("_get", ".GridRow", CGRIDROWS_get, "(Row)i"),
  GB_PROPERTY("Count", "i", CGRIDROWS_count),
  GB_PROPERTY("Height", "i", CGRIDROWS_height),
  GB_PROPERTY("Resizable", "b", CGRIDROWS_resizable),

  GB_END_DECLARE
};


GB_DESC CGridColumnsDesc[] =
{
  GB_DECLARE(".GridColumns", 0), GB_VIRTUAL_CLASS(),

  GB_METHOD("_get", ".GridColumn", CGRIDCOLS_get, "(Column)i"),
  GB_PROPERTY("Count", "i", CGRIDCOLS_count),
  GB_PROPERTY("Width", "i", CGRIDCOLS_width),
  GB_PROPERTY("Resizable", "b", CGRIDCOLS_resizable),

  GB_END_DECLARE
};


GB_DESC CGridViewDesc[] =
{
  GB_DECLARE("GridView", sizeof(CGRIDVIEW)), GB_INHERITS("Control"),

  GB_METHOD("_new", NULL, CGRIDVIEW_new, "(Parent)Container;"),

  GB_PROPERTY_READ("Rows", ".GridRows", CGRIDVIEW_self),
  GB_PROPERTY_READ("Columns", ".GridColumns", CGRIDVIEW_self),

  GB_METHOD("_get", ".GridCell", CGRIDVIEW_get, "(Row)i(Column)i"),

  GB_METHOD("Clear", NULL, CGRIDVIEW_clear, NULL),

  GB_PROPERTY("Row", "i", CGRIDVIEW_row),
  GB_PROPERTY("Column", "i", CGRIDVIEW_column),
  GB_PROPERTY_READ("Current", ".GridCell", CGRIDVIEW_current),
  GB_PROPERTY_READ("Item", ".GridCell", CGRIDVIEW_item),
  GB_PROPERTY("Grid", "b", CGRIDVIEW_grid),
  GB_PROPERTY("ScrollBar", "i", CWIDGET_scrollbar),
  GB_PROPERTY("Border", "b", CWIDGET_border_simple),
  GB_PROPERTY_READ("ClientWidth", "i", CGRIDVIEW_client_width),
  GB_PROPERTY_READ("ClientW", "i", CGRIDVIEW_client_width),
  GB_PROPERTY_READ("ClientHeight", "i", CGRIDVIEW_client_height),
  GB_PROPERTY_READ("ClientH", "i", CGRIDVIEW_client_height),
  GB_METHOD("Find", "b", CGRIDVIEW_find, "(X)i(Y)i"),

  GB_CONSTANT("_Properties", "s", "*,Grid=True,Border=True,Scrollbar{Scroll.*}=Both"),
  GB_CONSTANT("_DefaultEvent", "s", "Click"),

  //GB_EVENT("Change", NULL, NULL, &EVENT_Change),
  GB_EVENT("Activate", NULL, NULL, &EVENT_Activate),
  GB_EVENT("Click", NULL, NULL, &EVENT_Click),
  GB_EVENT("Scroll", NULL, NULL, &EVENT_Scroll),

  GB_END_DECLARE
};


/***************************************************************************

  class CGridView

***************************************************************************/

CGridView CGridView::manager;
bool CGridView::created;

bool CGridView::checkRow(QTable *table, long row)
{
  if (row < 0 || row >= table->numRows())
  {
    GB.Error("Bad row index %d", row);
    return true;
  }
  else
    return false;
}

bool CGridView::checkCol(QTable *table, long col)
{
  if (col < 0 || col >= table->numCols())
  {
    GB.Error("Bad column index %d", col);
    return true;
  }
  else
    return false;
}

bool CGridView::check(QTable *table, long row, long col)
{
  if (checkRow(table, row))
    return true;
  else if (checkCol(table, col))
    return true;
  else
    return false;
}

MyTableItem *CGridView::getItem(QTable *table, long row, long col, bool error)
{
  MyTableItem *item;

  if (error)
    check(table, row, col);

  //if (row < 0 || row >= table->numRows())
  //  return 0;
  //if (col < 0 || col >= table->numCols())
  //  return 0;

  item = (MyTableItem *)table->item(row, col);

  if (item == 0)
  {
    item = new MyTableItem(table);
    table->setItem(row, col, item);
    created = true;
  }
  else
    created = false;

  return item;
}

void CGridView::fillItems(QTable *table)
{
  long row, col;
  long cols;
  MyTableItem *item;

  row = table->numRows() - 1;
  cols = table->numCols();
  col =  cols - 1;

  if (col < 0)
    return;

  for(;;)
  {
    if (row < 0)
      return;

    if (col < 0)
      created = false;
    else
      item = getItem(table, row, col, false);

    if (!created)
    {
      if (col == (cols - 1))
        return;
      col = cols;
      row--;
    }
    else if (col == 0)
      table->setRowHeight(row, table->fontMetrics().lineSpacing() + 2);
    //item->setText("Gambas");

    col--;
  }
}

/*
void CGridView::changed(void)
{
  RAISE_EVENT(EVENT_Change, NULL);
}
*/

void CGridView::activated(void)
{
  RAISE_EVENT(EVENT_Activate);
}

void CGridView::clicked(void)
{
  RAISE_EVENT(EVENT_Click);
}

static void send_scroll(void *param)
{
  GB.Raise(param, EVENT_Scroll, 0);
  GB.Unref(&param);
}

void CGridView::scrolled(void)
{
  GET_SENDER(_object);

  GB.Ref(THIS);
  GB.Post((void (*)())send_scroll, (long)THIS);
}
