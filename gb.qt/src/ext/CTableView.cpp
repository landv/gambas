/***************************************************************************

  CTableView.cpp

  The TableView control

  (c) 2000-2005 Beno� Minisini <gambas@users.sourceforge.net>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 1, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS F A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

***************************************************************************/


#define __CTABLEVIEW_CPP

#include "main.h"

#include <qapplication.h>
#include <qheader.h>
#include <qpainter.h>
#include <qpalette.h>

#include "CTableView.h"

DECLARE_EVENT(EVENT_Click);
DECLARE_EVENT(EVENT_Activate);
DECLARE_EVENT(EVENT_Scroll);
DECLARE_EVENT(EVENT_Data);
DECLARE_EVENT(EVENT_ColumnClick);
DECLARE_EVENT(EVENT_RowClick);
DECLARE_EVENT(EVENT_Change);
DECLARE_EVENT(EVENT_Select);


/***************************************************************************

  class MyTableItem

***************************************************************************/

MyTableItem::MyTableItem(QTable *table)
: QTableItem(table, QTableItem::Never, 0)
{
  _tableView = 0;
  _bg = -1;
  _fg = -1;
}

MyTableItem::~MyTableItem()
{
}

void MyTableItem::setPicture(GB_OBJECT *val)
{
  QT_PICTURE pict;

  pict = (QT_PICTURE)VALUE(val);

  if (pict)
    setPixmap(*QT.GetPixmap(pict));
  else
    setPixmap(0);
}

void MyTableItem::invalidate()
{
  _valid = false;

  _alignment = Qt::AlignLeft | Qt::AlignVCenter;
  _bg = -1;
  _fg = -1;
  setText(0);
  setPixmap(0);
}

bool MyTableItem::invalidate(int r, int c)
{
  if (r == row() && c == col())
    return true;

  setRow(r);
  setCol(c);
  
  invalidate();
  return false;
}


void MyTableItem::getData()
{
  if (_valid)
    return;

  //qDebug("Data(%d, %d)", row(), col());

  if (!_tableView)
    _tableView = (CTABLEVIEW *)QT.GetObject(table());

  if (!_tableView)
    return;

  _valid = true;

  GB.Raise(_tableView, EVENT_Data, 2,
    GB_T_INTEGER, row(),
    GB_T_INTEGER, col()
    );
}

QString MyTableItem::text()
{
  getData();
  return QTableItem::text();
}

QPixmap MyTableItem::pixmap()
{
  getData();
  return QTableItem::pixmap();
}

int MyTableItem::alignment() const
{
  ((MyTableItem *)this)->getData();
  return _alignment;
}

void MyTableItem::paint( QPainter *p, const QColorGroup &cg, const QRect &cr, bool selected )
{
  int w = cr.width();
  int h = cr.height();
  int x = 0;

  getData();

  QPixmap pix = pixmap();
  QString txt = text();

  p->fillRect( 0, 0, w, h,
    selected ?
      cg.brush(QColorGroup::Highlight)
      : (_bg < 0) ? cg.brush(QColorGroup::Base) : QColor((uint)_bg));

  if (!pix.isNull())
  {
    if (txt.length() == 0)
      p->drawPixmap((w - pix.width()) / 2, (h - pix.height() ) / 2, pix);
    else
    {
      p->drawPixmap(2, (h - pix.height() ) / 2, pix);
      x = pix.width() + 4;
    }
  }

  if (selected)
    p->setPen(cg.highlightedText());
  else if (_fg < 0)
    p->setPen(cg.text());
  else
    p->setPen(QColor((uint)_fg));

  p->drawText(x + 2, 0, w - x - 4, h,
    wordWrap() ? (_alignment | WordBreak) : _alignment, txt );
}


QSize MyTableItem::sizeHint() const
{
  QSize strutSize = QApplication::globalStrut();
  QSize s;
  int x = 0;
  QPixmap pix = ((MyTableItem *)this)->pixmap();
  QString t = ((MyTableItem *)this)->text();

  ((MyTableItem *)this)->getData();

  if (!pix.isNull() )
  {
	  s = pix.size();
	  s.setWidth( s.width() + 2 );
	  x = pix.width() + 2;
	}

  if ( !wordWrap() && t.find( '\n' ) == -1 )
		return QSize(s.width() + table()->fontMetrics().width(t) + 10,
		             QMAX( s.height(), table()->fontMetrics().height() ) ).expandedTo( strutSize );

	QRect r = table()->fontMetrics().boundingRect( x + 2, 0, table()->columnWidth( col() ) - x - 4, 0,
						   wordWrap() ? (alignment() | WordBreak) : alignment(),
						   t );
	r.setWidth( QMAX( r.width() + 10, table()->columnWidth(col())));

  return QSize( r.width(), QMAX( s.height(), r.height() ) ).expandedTo( strutSize );
}

/***************************************************************************

  class MyTable

***************************************************************************/

MyTable::MyTable(QWidget *parent) :
QTable(0, 0, parent)
{
  _item = new MyTableItem(this);
  //_items = (MyTableItem **)0;
  _header = 3;
  _rows = 0;
  _cols = 0;
  //setReadOnly(true);
  setSelectionMode(NoSelection);
  setFocusStyle(FollowStyle);
  verticalHeader()->setMovingEnabled(false);
  horizontalHeader()->setMovingEnabled(false);
  updateHeaders();
}

MyTable::~MyTable()
{
  //blockSignals(true);
  //setNumCols(0);
  //setNumRows(0);
  delete _item;
  //blockSignals(false);
}

void MyTable::paintFocus( QPainter *p, const QRect &r )
{
	//QTable::paintFocus(p, r);
}

/*QSize MyTable::tableSize() const
{
  return QSize( columnPos( numCols() - 1 ) + columnWidth( numCols() - 1 ),
    rowPos( numRows() - 1 ) + rowHeight( numRows() - 1 ) );
}*/


void MyTable::setRowHeight(int row, long height)
{
  //qDebug("MyTable::setRowHeight(%d, %ld)", row, height);
  if (height < 0)
    adjustRow(row);
  else
    QTable::setRowHeight(row, height);
}


void MyTable::setColumnWidth(int col, long width)
{
  //qDebug("MyTable::setColumnWidth(%d, %ld)", col, width);
  if (width < 0)
    adjustColumn(col);
  else
    QTable::setColumnWidth(col, width);
}

/*void MyTable::adjustColumn(int col)
{
  qDebug("MyTable::adjustColumn(%d)", col);
  QTable::adjustColumn(col);
}*/

void MyTable::updateHeaders()
{
  int dim = fontMetrics().height() + 4;

  if (_header & 1)
  {
    horizontalHeader()->show();
    setTopMargin(dim);
  }
  else
  {
    horizontalHeader()->hide();
    setTopMargin(0);
  }

  if (leftMargin() > dim)
    dim = leftMargin();

  if (_header & 2)
  {
    verticalHeader()->show();
    setLeftMargin(dim);
  }
  else
  {
    verticalHeader()->hide();
    setLeftMargin(0);
  }
}

void MyTable::setHeaders(int h)
{
  h &= 3;

  if (h == _header)
    return;

  _header = h;
  updateHeaders();
}

void MyTable::fontChange(const QFont &oldFont)
{
  QTable::fontChange(oldFont);
  updateHeaders();
}


QTableItem *MyTable::item( int row, int col ) const
{
  if ( row < 0 || col < 0 || row > _rows - 1 || col > _cols - 1)
    return 0;

  //_items[col]->invalidate(row, col);
  //return _items[col];
  _item->invalidate(row, col);
  return _item;
}

void MyTable::setNumCols(int newCols)
{
  int i;
  int col = numCols();

  if (newCols < 0)
    return;

  _cols = newCols;
  _item->invalidate();

  /*
  if (_items)
  {
    delete[] _items;
    _items = (MyTableItem **)0;
  }

  if (_cols > 0)
  {
    _items = new (MyTableItem *)[_cols];
    for (i = 0; i < _cols; i++)
      _items[i] = new MyTableItem(this);
  }
  */

  QTable::setNumCols(newCols);

  if (newCols > col)
  {
    bool upd = horizontalHeader()->isUpdatesEnabled();
    horizontalHeader()->setUpdatesEnabled(false);

    for (i = col; i < newCols; i++)
      horizontalHeader()->setLabel(i, "");

    horizontalHeader()->setUpdatesEnabled(upd);
  }
}

void MyTable::setNumRows(int newRows)
{
  //int i;
  //int row = numRows();

  //bool r = verticalHeader()->isResizeEnabled();

  //setLeftMargin(fontMetrics().width(QString::number(newRows) + "W"));
  //verticalHeader()->setResizeEnabled(false);

  if (newRows < 0)
    return;

  _rows = newRows;
  _item->invalidate();
  QTable::setNumRows(newRows);

  //verticalHeader()->setResizeEnabled(r);

  /*if (newRows > row)
  {
    bool upd = verticalHeader()->isUpdatesEnabled();
    verticalHeader()->setUpdatesEnabled(false);

    for (i = row; i < newRows; i++)
      verticalHeader()->setLabel(i, QString::number(i + 1));

    verticalHeader()->setUpdatesEnabled(upd);
  }*/
}


void MyTable::updateRow(int row)
{
  if (row < 0 || row >= numRows() || numCols() == 0)
    return;

  QRect cg = cellGeometry(row, 0);

  QRect r(contentsToViewport( QPoint( contentsX(), cg.y() - 2 ) ),
    QSize( contentsWidth(), cg.height() + 4 ) );

  QApplication::postEvent(viewport(), new QPaintEvent(r, FALSE ));
}


/*
void MyTable::swapRows(int row1, int row2, bool swapHeader)
{
  QTable::swapRows(row1, row2, swapHeader);
  updateRow(row1);
  updateRow(row2);
}
*/

void MyTable::updateColumn(int col)
{
  if (col < 0 || col >= numCols() || numRows() == 0)
    return;

  QRect cg = cellGeometry(0, col);

  QRect r(contentsToViewport( QPoint( cg.x() - 2, contentsY() ) ),
    QSize( cg.width() + 4, contentsHeight() ) );

  QApplication::postEvent( viewport(), new QPaintEvent( r, FALSE ) );
}

void MyTable::setCurrentCell(int row, int col)
{
	//if (row != currentRow() || col != currentColumn())
	//	clearSelection();

	QTable::setCurrentCell(row, col);
}

/*
void MyTable::swapColumns(int col1, int col2, bool swapHeader)
{
  QTable::swapColumns(col1, col2, swapHeader);
  updateColumn(col1);
  updateColumn(col2);
}
*/

/***************************************************************************

  TableViewItem

***************************************************************************/


BEGIN_PROPERTY(CTABLEITEM_x)

  GB.ReturnInteger(WIDGET->columnPos(THIS->col) - WIDGET->contentsX() + WIDGET->clipper()->x());

END_PROPERTY


BEGIN_PROPERTY(CTABLEITEM_y)

  /*
  int p;

  p = WIDGET->rowPos(THIS->row) - WIDGET->contentsY() + WIDGET->frameWidth();
  if (!WIDGET->horizontalHeader()->isHidden())
    p += WIDGET->horizontalHeader()->height();

  GB.ReturnInteger(p);
  */

  GB.ReturnInteger(WIDGET->rowPos(THIS->row) - WIDGET->contentsY() + WIDGET->clipper()->y());

END_PROPERTY


BEGIN_PROPERTY(CTABLEITEM_width)

  GB.ReturnInteger(WIDGET->columnWidth(THIS->col) - 1);

END_PROPERTY


BEGIN_PROPERTY(CTABLEITEM_height)

  GB.ReturnInteger(WIDGET->rowHeight(THIS->row) - 1);

END_PROPERTY

#define CHECK_ITEM() \
  if (!ITEM) \
  { \
    GB.Error("No current item"); \
    return; \
  }

BEGIN_PROPERTY(CTABLEITEM_text)

  CHECK_ITEM();

  GB.ReturnNewZeroString(TO_UTF8(ITEM->text()));

END_PROPERTY


BEGIN_PROPERTY(CTABLEITEM_alignment)

  CHECK_ITEM();

  GB.ReturnInteger(ITEM->alignment());

END_PROPERTY


BEGIN_PROPERTY(CTABLEITEM_picture)

  CHECK_ITEM();

  GB.ReturnObject(THIS->picture);

END_PROPERTY


BEGIN_METHOD_VOID(CTABLEITEM_ensure_visible)

  WIDGET->ensureCellVisible(THIS->row, THIS->col);

END_METHOD


BEGIN_METHOD_VOID(CTABLEITEM_refresh)

  WIDGET->item()->setRow(-1);
  WIDGET->item()->setCol(-1);
  WIDGET->updateCell(THIS->row, THIS->col);

END_METHOD


/***************************************************************************

  TableViewData

***************************************************************************/

BEGIN_PROPERTY(CTABLEVIEW_data_text)

  if (READ_PROPERTY)
    GB.ReturnNewZeroString(TO_UTF8(WIDGET->item()->text()));
  else
    WIDGET->item()->setText(QSTRING_PROP());

END_PROPERTY


BEGIN_PROPERTY(CTABLEVIEW_data_picture)

  if (READ_PROPERTY)
    GB.ReturnObject(THIS->picture);
  else
  {
    GB.StoreObject(PROP(GB_OBJECT), &THIS->picture);
    WIDGET->item()->setPicture(PROP(GB_OBJECT));
  }

END_PROPERTY


BEGIN_PROPERTY(CTABLEVIEW_data_alignment)

  if (READ_PROPERTY)
    GB.ReturnInteger(WIDGET->item()->alignment());
  else
    WIDGET->item()->setAlignment(VPROP(GB_INTEGER));

END_PROPERTY


BEGIN_PROPERTY(CTABLEVIEW_data_background)

  if (READ_PROPERTY)
    GB.ReturnInteger(WIDGET->item()->background());
  else
    WIDGET->item()->setBackground(VPROP(GB_INTEGER) & 0xFFFFFF);

END_PROPERTY


BEGIN_PROPERTY(CTABLEVIEW_data_foreground)

  if (READ_PROPERTY)
    GB.ReturnInteger(WIDGET->item()->foreground());
  else
    WIDGET->item()->setForeground(VPROP(GB_INTEGER) & 0xFFFFFF);

END_PROPERTY


/***************************************************************************

  TableViewRows

***************************************************************************/

BEGIN_METHOD(CTABLEROWS_get, GB_INTEGER row)

  CTableView::checkRow(WIDGET, VARG(row));
  THIS->row = VARG(row);

  RETURN_SELF();

END_METHOD


BEGIN_PROPERTY(CTABLEROWS_count)

  if (READ_PROPERTY)
    GB.ReturnInteger(WIDGET->numRows());
  else
  {
  	if (VPROP(GB_INTEGER) != WIDGET->numRows())
    	WIDGET->setNumRows(VPROP(GB_INTEGER));
    //CTableView::fillItems(WIDGET);
  }

END_PROPERTY


BEGIN_PROPERTY(CTABLEROWS_height)

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


BEGIN_PROPERTY(CTABLEROWS_width)

  if (WIDGET->verticalHeader()->isHidden())
    GB.ReturnInteger(0);
  else
    GB.ReturnInteger(WIDGET->verticalHeader()->width());

END_PROPERTY


BEGIN_PROPERTY(CTABLEROWS_resizable)

  if (READ_PROPERTY)
    GB.ReturnBoolean(WIDGET->verticalHeader()->isResizeEnabled());
  else
    WIDGET->verticalHeader()->setResizeEnabled(VPROP(GB_BOOLEAN));

END_PROPERTY


BEGIN_PROPERTY(CTABLEROWS_moveable)

  if (READ_PROPERTY)
    GB.ReturnBoolean(WIDGET->verticalHeader()->isMovingEnabled());
  else
    WIDGET->verticalHeader()->setMovingEnabled(VPROP(GB_BOOLEAN));

END_PROPERTY


BEGIN_METHOD_VOID(CTABLEROWS_unselect)

	WIDGET->clearSelection();

END_PROPERTY


static void select_rows(CTABLEVIEW *_object, int start, int length)
{
	length += start;

	for (int i = start; i < length; i++)
		WIDGET->selectRow(i);
}

BEGIN_METHOD(CTABLEROWS_select, GB_INTEGER start; GB_INTEGER length)

	int start, length;

	start = VARGOPT(start, 0);
	if (start < 0)
		start = 0;

	length = VARGOPT(length, WIDGET->numRows() - start);
	if (length < 0)
		length = 0;

	WIDGET->clearSelection();

	select_rows(THIS, start, length);

END_PROPERTY


BEGIN_METHOD_VOID(CTABLEROWS_select_all)

	select_rows(THIS, 0, WIDGET->numRows());

END_PROPERTY


BEGIN_PROPERTY(CTABLEROWS_text)

  if (READ_PROPERTY)
    GB.ReturnNewZeroString(TO_UTF8(WIDGET->verticalHeader()->label(THIS->row)));
  else
    WIDGET->verticalHeader()->setLabel(THIS->row, QSTRING_PROP());

END_PROPERTY


BEGIN_METHOD_VOID(CTABLEROWS_refresh)

  WIDGET->updateRow(THIS->row);

END_METHOD


BEGIN_METHOD_VOID(CTABLEROWS_selected)

	if (READ_PROPERTY)
  	GB.ReturnBoolean(WIDGET->isRowSelected(THIS->row, true));
	else
		WIDGET->selectRow(THIS->row);

END_METHOD


/***************************************************************************

  TableViewCols

***************************************************************************/

static void update_autoresize(CTABLEVIEW *_object)
{
	//for (i = 0; i < WIDGET->numCols(); i++)
  //	WIDGET->setColumnStretchable(i, THIS->autoresize);

  //WIDGET->horizontalHeader()->setResizeEnabled(!THIS->autoresize);

  //WIDGET->horizontalHeader()->setStretchEnabled(true);
  if (WIDGET->numCols())
  	WIDGET->setColumnStretchable(WIDGET->numCols() - 1, THIS->autoresize);
}

BEGIN_METHOD(CTABLECOLS_get, GB_INTEGER col)

  CTableView::checkCol(WIDGET, VARG(col));
  THIS->col = VARG(col);

  RETURN_SELF();

END_METHOD


BEGIN_PROPERTY(CTABLECOLS_count)

  if (READ_PROPERTY)
    GB.ReturnInteger(WIDGET->numCols());
  else
  {
  	if (VPROP(GB_INTEGER) != WIDGET->numCols())
  	{
	    WIDGET->setNumCols(VPROP(GB_INTEGER));
	    update_autoresize(THIS);
		}
    //CTableView::fillItems(WIDGET);
  }

END_PROPERTY


BEGIN_PROPERTY(CTABLECOLS_width)

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


BEGIN_PROPERTY(CTABLECOLS_height)

  if (WIDGET->horizontalHeader()->isHidden())
    GB.ReturnInteger(0);
  else
    GB.ReturnInteger(WIDGET->horizontalHeader()->height());

END_PROPERTY


BEGIN_PROPERTY(CTABLECOLS_resizable)

  if (READ_PROPERTY)
    GB.ReturnBoolean(WIDGET->horizontalHeader()->isResizeEnabled());
  else
  {
	  WIDGET->horizontalHeader()->setResizeEnabled(VPROP(GB_BOOLEAN));
  	//THIS->autoresize = !VPROP(GB_BOOLEAN);
  	//update_autoresize(THIS);
	}

END_PROPERTY


BEGIN_PROPERTY(CTABLECOLS_moveable)

  if (READ_PROPERTY)
    GB.ReturnBoolean(WIDGET->horizontalHeader()->isMovingEnabled());
  else
    WIDGET->horizontalHeader()->setMovingEnabled(VPROP(GB_BOOLEAN));

END_PROPERTY


BEGIN_PROPERTY(CTABLECOLS_text)

  if (READ_PROPERTY)
    GB.ReturnNewZeroString(TO_UTF8(WIDGET->horizontalHeader()->label(THIS->col)));
  else
    WIDGET->horizontalHeader()->setLabel(THIS->col, QSTRING_PROP());

END_PROPERTY


/*BEGIN_PROPERTY(CTABLECOLS_stretchable)

  if (READ_PROPERTY)
    GB.ReturnBoolean(WIDGET->isColumnStretchable(THIS->col));
  else
    WIDGET->setColumnStretchable(THIS->col, VPROP(GB_BOOLEAN));

END_PROPERTY*/


BEGIN_METHOD_VOID(CTABLECOLS_refresh)

  WIDGET->updateColumn(THIS->col);

END_METHOD


/***************************************************************************

  TableView

***************************************************************************/

BEGIN_METHOD(CTABLEVIEW_new, GB_OBJECT parent)

  MyTable *wid = new MyTable(QT.GetContainer(VARG(parent)));

  QObject::connect(wid, SIGNAL(currentChanged(int, int)), MANAGER, SLOT(changed()));
  QObject::connect(wid, SIGNAL(selectionChanged()), MANAGER, SLOT(selected()));
  QObject::connect(wid, SIGNAL(doubleClicked(int, int, int, const QPoint &)), MANAGER, SLOT(activated()));
  QObject::connect(wid, SIGNAL(clicked(int, int, int, const QPoint &)), MANAGER, SLOT(clicked()));
  QObject::connect(wid, SIGNAL(contentsMoving(int, int)), MANAGER, SLOT(scrolled()));
  QObject::connect(wid->horizontalHeader(), SIGNAL(sectionClicked(int)), MANAGER, SLOT(columnClicked(int)));
  QObject::connect(wid->verticalHeader(), SIGNAL(sectionClicked(int)), MANAGER, SLOT(rowClicked(int)));

  QT.InitWidget(wid, _object);

  //QT.SetBackgroundRole(THIS, QColorGroup::Base);

  THIS->row = -1;
  THIS->col = -1;

  //wid->setTableView(THIS);

  wid->show();

END_METHOD


BEGIN_METHOD_VOID(CTABLEVIEW_free)

  GB.StoreObject(NULL, &THIS->picture);

END_METHOD


static void set_current_cell(CTABLEVIEW *_object, int row, int col)
{
	if (row < 0 || col < 0 || row >= WIDGET->numRows() || col >= WIDGET->numCols())
	{
		//QTable::SelectionMode mode = WIDGET->selectionMode();
		//WIDGET->setSelectionMode(QTable::NoSelection);
		//WIDGET->setSelectionMode(mode);
		WIDGET->clearSelection();
	}
	else
	{
		if (row == WIDGET->currentRow() && WIDGET->selectionMode() == QTable::SingleRow)
		{
			WIDGET->blockSignals(true);
	    WIDGET->setCurrentCell(row == 0 ? 1 : row - 1, col);
	    WIDGET->blockSignals(false);
		}
    WIDGET->setCurrentCell(row, col);
	}
}

BEGIN_PROPERTY(CTABLEVIEW_row)

  if (READ_PROPERTY)
    GB.ReturnInteger(WIDGET->currentRow());
  else
    set_current_cell(THIS, VPROP(GB_INTEGER), WIDGET->currentColumn());

END_PROPERTY


BEGIN_PROPERTY(CTABLEVIEW_column)

  if (READ_PROPERTY)
    GB.ReturnInteger(WIDGET->currentColumn());
  else
    set_current_cell(THIS, WIDGET->currentRow(), VPROP(GB_INTEGER));

END_PROPERTY


BEGIN_METHOD(CTABLEVIEW_move_to, GB_INTEGER row; GB_INTEGER col)

  long row = VARG(row);
  long col = VARG(col);

  //if (CTableView::check(WIDGET, row, col))
  //  return;

  set_current_cell(THIS, row, col);

END_METHOD



#if 0
BEGIN_METHOD_VOID(CTABLEVIEW_clear)

  long rows = WIDGET->numRows();
  WIDGET->setNumRows(0);
  WIDGET->setNumRows(rows);

END_METHOD
#endif


BEGIN_METHOD(CTABLEVIEW_get, GB_INTEGER row; GB_INTEGER col)

  long row = VARG(row);
  long col = VARG(col);

  if (CTableView::check(WIDGET, row, col))
    return;

  THIS->row = row;
  THIS->col = col;

  RETURN_SELF();

END_METHOD


BEGIN_PROPERTY(CTABLEVIEW_current)

  THIS->row = WIDGET->currentRow();
  THIS->col = WIDGET->currentColumn();

  if (CTableView::check(WIDGET, THIS->row, THIS->col))
    return;
  else
    RETURN_SELF();

END_PROPERTY


BEGIN_PROPERTY(CTABLEVIEW_self)

  THIS->row = -1;
  THIS->col = -1;

  RETURN_SELF();

END_PROPERTY


BEGIN_PROPERTY(CTABLEVIEW_grid)

  if (READ_PROPERTY)
    GB.ReturnBoolean(WIDGET->showGrid());
  else
    WIDGET->setShowGrid(VPROP(GB_BOOLEAN));

END_PROPERTY


BEGIN_PROPERTY(CTABLEVIEW_scrollbars)

  long scroll;

  if (READ_PROPERTY)
  {
    scroll = 0;
    if (WIDGET->hScrollBarMode() == QScrollView::Auto)
      scroll += 1;
    if (WIDGET->vScrollBarMode() == QScrollView::Auto)
      scroll += 2;

    GB.ReturnInteger(scroll);
  }
  else
  {
    scroll = VPROP(GB_INTEGER) & 3;
    WIDGET->setHScrollBarMode( (scroll & 1) ? QScrollView::Auto : QScrollView::AlwaysOff);
    WIDGET->setVScrollBarMode( (scroll & 2) ? QScrollView::Auto : QScrollView::AlwaysOff);
  }

END_PROPERTY


BEGIN_PROPERTY(CTABLEVIEW_header)

  if (READ_PROPERTY)
    GB.ReturnInteger(WIDGET->headers());
  else
    WIDGET->setHeaders(VPROP(GB_INTEGER));

END_PROPERTY


BEGIN_PROPERTY(CTABLEVIEW_mode)

  if (READ_PROPERTY)
  {
    switch (WIDGET->selectionMode())
    {
      case QTable::NoSelection: GB.ReturnInteger(0); break;
      case QTable::SingleRow: GB.ReturnInteger(1); break;
      case QTable::MultiRow: GB.ReturnInteger(2); break;
      default: GB.ReturnInteger(0); break;
    }
  }
  else
  {
    switch(VPROP(GB_INTEGER))
    {
      case 0: WIDGET->setSelectionMode(QTable::NoSelection); break;
      case 1: WIDGET->setSelectionMode(QTable::SingleRow); break;
      case 2: WIDGET->setSelectionMode(QTable::MultiRow); break;
      case 3: WIDGET->setSelectionMode(QTable::Single); break;
      case 4: WIDGET->setSelectionMode(QTable::Multi); break;
    }
  }

END_PROPERTY


BEGIN_PROPERTY(CTABLEVIEW_client_width)

  WIDGET->updateScrollBars();
  GB.ReturnInteger(WIDGET->visibleWidth());

END_PROPERTY


BEGIN_PROPERTY(CTABLEVIEW_client_height)

  WIDGET->updateScrollBars();
  GB.ReturnInteger(WIDGET->clipper()->height());

END_PROPERTY

BEGIN_PROPERTY(CTABLEVIEW_border)

  QT.BorderProperty(_object, _param);

END_PROPERTY

// CWIDGET_refresh does not work for this widget ??
// Should do viewport->refresh() !

BEGIN_METHOD(CTABLEVIEW_refresh, GB_INTEGER x; GB_INTEGER y; GB_INTEGER w; GB_INTEGER h)

  int x, y, w, h;

  if (!MISSING(x) && !MISSING(y))
  {
    x = VARG(x);
    y = VARG(y);
    w = VARGOPT(w, WIDGET->width());
    h = VARGOPT(h, WIDGET->height());
    WIDGET->viewport()->repaint(x, y, w, h);
  }
  else
    WIDGET->viewport()->repaint();

END_METHOD

BEGIN_PROPERTY(CTABLEVIEW_scroll_x)

  if (READ_PROPERTY)
    GB.ReturnInteger(WIDGET->contentsX());
  else
    WIDGET->setContentsPos(VPROP(GB_INTEGER), WIDGET->contentsY());

END_PROPERTY


BEGIN_PROPERTY(CTABLEVIEW_scroll_y)

  if (READ_PROPERTY)
    GB.ReturnInteger(WIDGET->contentsY());
  else
    WIDGET->setContentsPos(WIDGET->contentsX(), VPROP(GB_INTEGER));

END_PROPERTY

BEGIN_METHOD(CTABLEVIEW_row_at, GB_INTEGER ypos)

  int ypos = VARG(ypos);

  ypos = ypos + WIDGET->contentsY();

  if (!WIDGET->horizontalHeader()->isHidden())
    ypos = ypos - WIDGET->horizontalHeader()->height();

  GB.ReturnInteger(WIDGET->rowAt(ypos));

END_PROPERTY

BEGIN_METHOD(CTABLEVIEW_column_at, GB_INTEGER xpos)

  int xpos = VARG(xpos);

  xpos = xpos + WIDGET->contentsX();

  if (!WIDGET->verticalHeader()->isHidden())
    xpos = xpos - WIDGET->verticalHeader()->width();

  GB.ReturnInteger(WIDGET->columnAt(xpos));

END_PROPERTY


BEGIN_PROPERTY(CTABLEVIEW_autoresize)

  if (READ_PROPERTY)
    GB.ReturnBoolean(THIS->autoresize);
  else
  {
  	THIS->autoresize = VPROP(GB_BOOLEAN);
  	update_autoresize(THIS);
	}

END_PROPERTY

#if 0
BEGIN_PROPERTY(CTABLEVIEW_table)

  if (READ_PROPERTY)
    GB.ReturnNewZeroString(TO_UTF8(WIDGET->definition()));
  else
    WIDGET->setDefinition(QSTRING_PROP());

END_PROPERTY
#endif


/***************************************************************************

  Descriptions

***************************************************************************/

GB_DESC CTableViewDataDesc[] =
{
  GB_DECLARE(".TableViewData", 0), GB_VIRTUAL_CLASS(),

  //GB_PROPERTY_READ("Row", "i", CTABLEITEM_row),
  //GB_PROPERTY_READ("Column", "i", CTABLEITEM_column),

  //GB_PROPERTY("Picture", "Picture", CTABLEITEM_picture),
  //GB_PROPERTY("Text", "s", CTABLEITEM_text),
  //GB_PROPERTY("Alignment", "i", CTABLEITEM_alignment),

  GB_PROPERTY("Text", "s", CTABLEVIEW_data_text),
  GB_PROPERTY("Picture", "Picture", CTABLEVIEW_data_picture),
  GB_PROPERTY("Alignment", "i", CTABLEVIEW_data_alignment),
  GB_PROPERTY("Background", "i", CTABLEVIEW_data_background),
  GB_PROPERTY("BackColor", "i", CTABLEVIEW_data_background),
  GB_PROPERTY("Foreground", "i", CTABLEVIEW_data_foreground),
  GB_PROPERTY("ForeColor", "i", CTABLEVIEW_data_foreground),

  GB_END_DECLARE
};

GB_DESC CTableItemDesc[] =
{
  GB_DECLARE(".TableViewCell", 0), GB_VIRTUAL_CLASS(),

  //GB_PROPERTY_READ("Row", "i", CTABLEITEM_row),
  //GB_PROPERTY_READ("Column", "i", CTABLEITEM_column),

  GB_PROPERTY_READ("Picture", "Picture", CTABLEITEM_picture),
  GB_PROPERTY_READ("Text", "s", CTABLEITEM_text),
  GB_PROPERTY_READ("Alignment", "i", CTABLEITEM_alignment),

  GB_PROPERTY_READ("X", "i", CTABLEITEM_x),
  GB_PROPERTY_READ("Y", "i", CTABLEITEM_y),
  GB_PROPERTY_READ("Left", "i", CTABLEITEM_x),
  GB_PROPERTY_READ("Top", "i", CTABLEITEM_y),
  GB_PROPERTY_READ("Width", "i", CTABLEITEM_width),
  GB_PROPERTY_READ("Height", "i", CTABLEITEM_height),
  GB_PROPERTY_READ("W", "i", CTABLEITEM_width),
  GB_PROPERTY_READ("H", "i", CTABLEITEM_height),

  //GB_METHOD("Clear", NULL, CTABLEITEM_clear, NULL),
  GB_METHOD("EnsureVisible", NULL, CTABLEITEM_ensure_visible, NULL),
  GB_METHOD("Refresh", NULL, CTABLEITEM_refresh, NULL),

  GB_END_DECLARE
};


GB_DESC CTableRowDesc[] =
{
  GB_DECLARE(".TableViewRow", 0), GB_VIRTUAL_CLASS(),

  GB_PROPERTY_READ("X", "i", CTABLEITEM_x),
  GB_PROPERTY("Height", "i", CTABLEROWS_height),
  GB_PROPERTY("H", "i", CTABLEROWS_height),
  GB_PROPERTY("Text", "s", CTABLEROWS_text),
  GB_PROPERTY("Title", "s", CTABLEROWS_text),
  GB_PROPERTY("Selected", "b", CTABLEROWS_selected),
  GB_METHOD("Refresh", NULL, CTABLEROWS_refresh, NULL),

  GB_END_DECLARE
};


GB_DESC CTableColumnDesc[] =
{
  GB_DECLARE(".TableViewColumn", 0), GB_VIRTUAL_CLASS(),

  GB_PROPERTY_READ("Y", "i", CTABLEITEM_y),
  GB_PROPERTY("Width", "i", CTABLECOLS_width),
  GB_PROPERTY("W", "i", CTABLECOLS_width),
  GB_PROPERTY("Text", "s", CTABLECOLS_text),
  GB_PROPERTY("Title", "s", CTABLECOLS_text),
  GB_METHOD("Refresh", NULL, CTABLECOLS_refresh, NULL),

  GB_END_DECLARE
};


GB_DESC CTableRowsDesc[] =
{
  GB_DECLARE(".TableViewRows", 0), GB_VIRTUAL_CLASS(),

  GB_METHOD("_get", ".TableViewRow", CTABLEROWS_get, "(Row)i"),
  GB_PROPERTY("Count", "i", CTABLEROWS_count),
  GB_PROPERTY("Height", "i", CTABLEROWS_height),
  GB_PROPERTY("H", "i", CTABLEROWS_height),
  GB_PROPERTY_READ("Width", "i", CTABLEROWS_width),
  GB_PROPERTY_READ("W", "i", CTABLEROWS_width),
  GB_PROPERTY("Resizable", "b", CTABLEROWS_resizable),
  GB_PROPERTY("Moveable", "b", CTABLEROWS_moveable),
  GB_METHOD("Select", NULL, CTABLEROWS_select, "[(Start)i(Length)i]"),
  GB_METHOD("SelectAll", NULL, CTABLEROWS_select_all, NULL),
  GB_METHOD("UnSelect", NULL, CTABLEROWS_unselect, NULL),

  GB_END_DECLARE
};


GB_DESC CTableColumnsDesc[] =
{
  GB_DECLARE(".TableViewColumns", 0), GB_VIRTUAL_CLASS(),

  GB_METHOD("_get", ".TableViewColumn", CTABLECOLS_get, "(Column)i"),
  GB_PROPERTY("Count", "i", CTABLECOLS_count),
  GB_PROPERTY("Width", "i", CTABLECOLS_width),
  GB_PROPERTY("W", "i", CTABLECOLS_width),
  GB_PROPERTY_READ("Height", "i", CTABLECOLS_height),
  GB_PROPERTY_READ("H", "i", CTABLECOLS_height),
  GB_PROPERTY("Resizable", "b", CTABLECOLS_resizable),
  GB_PROPERTY("Moveable", "b", CTABLECOLS_moveable),

  GB_END_DECLARE
};


GB_DESC CTableViewDesc[] =
{
  GB_DECLARE("TableView", sizeof(CTABLEVIEW)), GB_INHERITS("Control"),

  GB_CONSTANT("None", "i", 0),
  GB_CONSTANT("Horizontal", "i", 1),
  GB_CONSTANT("Vertical", "i", 2),
  GB_CONSTANT("Both", "i", 3),

  GB_CONSTANT("Single", "i", 1),
  GB_CONSTANT("Multi", "i", 2),

  GB_METHOD("_new", NULL, CTABLEVIEW_new, "(Parent)Container;"),
  GB_METHOD("_free", NULL, CTABLEVIEW_free, NULL),

  GB_PROPERTY_READ("Rows", ".TableViewRows", CTABLEVIEW_self),
  GB_PROPERTY_READ("Columns", ".TableViewColumns", CTABLEVIEW_self),

  GB_METHOD("_get", ".TableViewCell", CTABLEVIEW_get, "(Row)i(Column)i"),

  //GB_METHOD("Clear", NULL, CTABLEVIEW_clear, NULL),

  GB_PROPERTY("Row", "i", CTABLEVIEW_row),
  GB_PROPERTY("Column", "i", CTABLEVIEW_column),
  GB_METHOD("MoveTo", NULL, CTABLEVIEW_move_to, "(Row)i(Column)i"),
  GB_PROPERTY_READ("Current", ".TableViewCell", CTABLEVIEW_current),
  GB_PROPERTY("Grid", "b", CTABLEVIEW_grid),
  GB_PROPERTY("Border", "b", CTABLEVIEW_border),
  GB_PROPERTY("ScrollBar", "i", CTABLEVIEW_scrollbars),
  GB_PROPERTY("Header", "i", CTABLEVIEW_header),
  GB_PROPERTY("Mode", "i", CTABLEVIEW_mode),
  GB_PROPERTY("AutoResize", "b", CTABLEVIEW_autoresize),
  GB_PROPERTY("Resizable", "b", CTABLECOLS_resizable),

  GB_METHOD("RowAt", "i", CTABLEVIEW_row_at, "(Y)i"),
  GB_METHOD("ColumnAt", "i", CTABLEVIEW_column_at,"(X)i"),

  GB_PROPERTY("ScrollX", "i", CTABLEVIEW_scroll_x),
  GB_PROPERTY("ScrollY", "i", CTABLEVIEW_scroll_y),
  GB_PROPERTY_READ("ClientWidth", "i", CTABLEVIEW_client_width),
  GB_PROPERTY_READ("ClientW", "i", CTABLEVIEW_client_width),
  GB_PROPERTY_READ("ClientHeight", "i", CTABLEVIEW_client_height),
  GB_PROPERTY_READ("ClientH", "i", CTABLEVIEW_client_height),

  GB_PROPERTY_SELF("Data", ".TableViewData"),

  GB_METHOD("Refresh", NULL, CTABLEVIEW_refresh, "[(X)i(Y)i(Width)i(Height)i]"),

  GB_CONSTANT("_Properties", "s", 
  	"*,Mode{TableView.None;Single;Multi},Grid=True,Header{TableView.None;Vertical;Horizontal;Both}=Both,"
  	"Scrollbar{Scroll.*}=Both,Border=True,Resizable=True,AutoResize"),
  GB_CONSTANT("_DefaultEvent", "s", "Click"),

  GB_EVENT("Change", NULL, NULL, &EVENT_Change),
  GB_EVENT("Select", NULL, NULL, &EVENT_Select),
  GB_EVENT("Activate", NULL, NULL, &EVENT_Activate),
  GB_EVENT("Click", NULL, NULL, &EVENT_Click),
  GB_EVENT("Scroll", NULL, NULL, &EVENT_Scroll),
  GB_EVENT("Data", NULL, "(Row)i(Column)i", &EVENT_Data),
  GB_EVENT("ColumnClick", NULL, "(Column)i", &EVENT_ColumnClick),
  GB_EVENT("RowClick", NULL, "(Row)i", &EVENT_RowClick),

  GB_END_DECLARE
};


/***************************************************************************

  class CTableView

***************************************************************************/

CTableView CTableView::manager;

bool CTableView::checkRow(QTable *table, long row)
{
  if (row < -1 || row >= table->numRows())
  {
    GB.Error("Bad row index");
    return true;
  }
  else
    return false;
}

bool CTableView::checkCol(QTable *table, long col)
{
  if (col < -1 || col >= table->numCols())
  {
    GB.Error("Bad column index");
    return true;
  }
  else
    return false;
}

bool CTableView::check(QTable *table, long row, long col)
{
  if (checkRow(table, row))
    return true;
  else if (checkCol(table, col))
    return true;
  else
    return false;
}

void CTableView::changed(void)
{
  MyTable *w = (MyTable *)sender();
  void *object = QT.GetObject((QWidget *)w);

  if (w->currentRow() < 0 || w->currentColumn() < 0)
    return;

  GB.Raise(object, EVENT_Change, 0);
}

void CTableView::activated(void)
{
  void *object = QT.GetObject((QWidget *)sender());
  GB.Raise(object, EVENT_Activate, 0);
}

void CTableView::selected(void)
{
  void *_object = QT.GetObject((QWidget *)sender());
  if (WIDGET->selectionMode() == QTable::SingleRow)
  	GB.Raise(THIS, EVENT_Select, 0);
  else
  	GB.RaiseLater(THIS, EVENT_Select);
}

void CTableView::clicked(void)
{
  void *object = QT.GetObject((QWidget *)sender());
  GB.Raise(object, EVENT_Click, 0);
}

static void send_scroll(void *param)
{
  GB.Raise(param, EVENT_Scroll, 0);
  GB.Unref(&param);
}

void CTableView::scrolled(void)
{
  void *object = QT.GetObject((QWidget *)sender());

  GB.Ref(object);
  GB.Post((void (*)())send_scroll, (long)object);
}

void CTableView::columnClicked(int col)
{
  void *object = QT.GetObject((QWidget *)sender());
  GB.Raise(object, EVENT_ColumnClick, 1, GB_T_INTEGER, col);
}

void CTableView::rowClicked(int row)
{
  void *object = QT.GetObject((QWidget *)sender());
  GB.Raise(object, EVENT_RowClick, 1, GB_T_INTEGER, row);
}

