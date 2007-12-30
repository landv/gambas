/***************************************************************************

  CGridView.cpp

  (c) 2000-2007 Benoit Minisini <gambas@users.sourceforge.net>

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

#define __CGRIDVIEW_CPP

#include "main.h"

#include <qapplication.h>
#include <qheader.h>
#include <qpainter.h>
#include <qpalette.h>

#include "CConst.h"
#include "CGridView.h"

DECLARE_EVENT(EVENT_Click);
DECLARE_EVENT(EVENT_Activate);
DECLARE_EVENT(EVENT_Scroll);
DECLARE_EVENT(EVENT_Data);
DECLARE_EVENT(EVENT_ColumnClick);
DECLARE_EVENT(EVENT_RowClick);
DECLARE_EVENT(EVENT_ColumnResize);
DECLARE_EVENT(EVENT_RowResize);
DECLARE_EVENT(EVENT_Change);
DECLARE_EVENT(EVENT_Select);

/** MyTableData **************************************************************/

void MyTableData::init()
{
  alignment = ALIGN_NORMAL;
  bg = fg = -1; font = 0; text = 0; pict = 0;
}

void MyTableData::clear()
{
  GB.Unref(POINTER(&pict));
  GB.Unref(POINTER(&font));
  GB.FreeString(&text);
  init();
}

MyTableData::MyTableData()
{
	init();
}

MyTableData::~MyTableData()
{
	clear();
}


/** MyTableItem **************************************************************/

MyTableItem::MyTableItem(QTable *table, CGRIDVIEW *view)
: QTableItem(table, QTableItem::Never, 0)
{
  _view = view;
  _data = &_default;
  _dict.setAutoDelete(true);
}

MyTableItem::~MyTableItem()
{
}

int MyTableItem::getKey(int row, int col)
{
	int n;
	 
 	if (row >= col)
 		n = row * row + col * 2;
	else
		n = col * col + row * 2 + 1;
	
	return n;
}

void MyTableItem::invalidate()
{
	_data = 0;
}

bool MyTableItem::invalidate(int r, int c)
{
  if (r == row() && c == col() && _data != &_default)
    return true;

  setRow(r);
  setCol(c);
  
  invalidate();
  return false;
}


MyTableData *MyTableItem::data(bool create)
{
	int key;
	
  if (!_data)
  {
  	key = getKey(row(), col());
  	_data = _dict[key];
  	if (!_data)
  	{
  		if (create)
  		{
  			_data = new MyTableData;
  			_dict.insert(key, _data);
  			//qDebug("new [%d, %d] / %d", row(), col(), key);
			}
			else
			{
				_data = &_default;
				_default.clear();
			
				GB.Raise(_view, EVENT_Data, 2,
					GB_T_INTEGER, row(),
					GB_T_INTEGER, col()
					);
			}
		}
	}
	
	return _data;
}

void MyTableItem::clear()
{
	int key = getKey(row(), col());
	_dict.remove(key);
}

QString MyTableItem::text()
{
	return TO_QSTRING(data()->text);
}

QPixmap MyTableItem::pixmap()
{
	CPICTURE *pict = data()->pict;
	
	if (pict)
		return *(pict->pixmap);
	else
		return QTableItem::pixmap();
}

int MyTableItem::alignment() const
{
  return ((MyTableItem *)this)->data()->alignment;
}

void MyTableItem::paint( QPainter *p, const QColorGroup &cg, const QRect &cr, bool selected )
{
	MyTableData *d = data();
  int w = cr.width();
  int h = cr.height();
  int x = 0;

  QPixmap pix = pixmap();
  QString txt = text();
  int _bg = d->bg;
  int _fg = d->fg;
  int _alignment = d->alignment;

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

	if (d->font)
		p->setFont(*(d->font->font));

  p->drawText(x + 1, 0, w - x - 4, h,
    wordWrap() ? (_alignment | WordBreak) : _alignment, txt );
    
	if (d->font)
		p->setFont(table()->font());
}


QSize MyTableItem::sizeHint() const
{
  QSize strutSize = QApplication::globalStrut();
  QSize s;
  int x = 0;
  MyTableItem *item = (MyTableItem *)this;
  
  QPixmap pix = item->pixmap();
  QString t = item->text();

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

/** MyTable ******************************************************************/

MyTable::MyTable(QWidget *parent, CGRIDVIEW *view) :
QTable(0, 0, parent)
{
  _item = new MyTableItem(this, view);
  _header = 0;
  _rows = 0;
  _cols = 0;
  _no_selection = 0;
  
  setSelectionMode(NoSelection);
  setFocusStyle(FollowStyle);
  verticalHeader()->setMovingEnabled(false);
  horizontalHeader()->setMovingEnabled(false);
  updateHeaders();
}

MyTable::~MyTable()
{
  delete _item;
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

  clearSelection();
  emit currentChanged(-1, -1); 
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
  clearSelection();
  emit currentChanged(-1, -1); 
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
	clearSelection();
	QTable::setCurrentCell(row, col);
}

/*void MyTable::createCell(int row, int col)
{
	int key = getKey(row, col);
	
	if (_dict[key])
		return;
		
	_dict.insert(key, new MyTableItem);
}

void MyTable::removeCell(int row, int col)
{
	int key = getKey(row, col);
	MyTableItem *item = _dict[key];
	
	if (!item)
		return;
		
	_dict.remove(key);
	delete item;
}*/


/*
void MyTable::swapColumns(int col1, int col2, bool swapHeader)
{
  QTable::swapColumns(col1, col2, swapHeader);
  updateColumn(col1);
  updateColumn(col2);
}
*/

static MyTableItem *get_item(void *_object, bool create)
{
	return (MyTableItem *)(!create ? WIDGET->item() : WIDGET->item(THIS->row, THIS->col));
}

static void text_property(void *_object, void *_param, bool create)
{
	MyTableItem *item = get_item(_object, create);
	
  if (READ_PROPERTY)
    GB.ReturnString(item->data()->text);
  else
  {
    GB.StoreString(PROP(GB_STRING), &item->data(create)->text);
    if (create)
			WIDGET->updateCell(THIS->row, THIS->col);
	}
}

static void picture_property(void *_object, void *_param, bool create)
{
	MyTableItem *item = get_item(_object, create);	
		
  if (READ_PROPERTY)
    GB.ReturnObject(item->data()->pict);
  else
  {
    GB.StoreObject(PROP(GB_OBJECT), POINTER(&item->data(create))->pict);
    if (create)
			WIDGET->updateCell(THIS->row, THIS->col);
	}
}

static void font_property(void *_object, void *_param, bool create)
{
	MyTableItem *item = get_item(_object, create);	
  CFONT *font;
  
  if (READ_PROPERTY)
  {
  	font = item->data()->font;
  	
  	if (!font)
  	{
  		font = item->data()->font = CFONT_create(WIDGET->font());
  		GB.Ref(font);
		}
  	
    GB.ReturnObject(font);
	}
  else
  {
    if (VPROP(GB_OBJECT))
    {
      font = CFONT_create(*(((CFONT *)VPROP(GB_OBJECT))->font));
      GB.Ref(font);
    }
    else
      font = 0;
      
    GB.Unref(POINTER(&item->data(create)->font));
    item->data(create)->font = font;
    
    if (create)
			WIDGET->updateCell(THIS->row, THIS->col);
	}
}

static void alignment_property(void *_object, void *_param, bool create)
{
	MyTableItem *item = get_item(_object, create);
		
  if (READ_PROPERTY)
    GB.ReturnInteger(item->data()->alignment);
  else
  {
    item->data(create)->alignment = VPROP(GB_INTEGER);
    if (create)
			WIDGET->updateCell(THIS->row, THIS->col);
	}
}

static void background_property(void *_object, void *_param, bool create)
{
	MyTableItem *item = get_item(_object, create);
	
  if (READ_PROPERTY)
    GB.ReturnInteger(item->data()->bg);
  else
  {
    item->data(create)->bg = VPROP(GB_INTEGER) & 0xFFFFFF;
    if (create)
			WIDGET->updateCell(THIS->row, THIS->col);
	}
}

static void foreground_property(void *_object, void *_param, bool create)
{
	MyTableItem *item = get_item(_object, create);
	
  if (READ_PROPERTY)
    GB.ReturnInteger(item->data()->fg);
  else
  {
    item->data(create)->fg = VPROP(GB_INTEGER) & 0xFFFFFF;
    if (create)
			WIDGET->updateCell(THIS->row, THIS->col);
	}
}


void MyTable::drawContents(QPainter *p, int clipx, int clipy, int clipw, int cliph)
{
	if (isVisible())
		QTable::drawContents(p, clipx, clipy, clipw, cliph);
}

/***************************************************************************

  GridViewData

***************************************************************************/

BEGIN_PROPERTY(CGRIDVIEW_data_text)

	text_property(_object, _param, false);

END_PROPERTY


BEGIN_PROPERTY(CGRIDVIEW_data_picture)

	picture_property(_object, _param, false);

END_PROPERTY


BEGIN_PROPERTY(CGRIDVIEW_data_font)

	font_property(_object, _param, false);

END_PROPERTY


BEGIN_PROPERTY(CGRIDVIEW_data_alignment)

	alignment_property(_object, _param, false);

END_PROPERTY


BEGIN_PROPERTY(CGRIDVIEW_data_background)

	background_property(_object, _param, false);

END_PROPERTY


BEGIN_PROPERTY(CGRIDVIEW_data_foreground)

	foreground_property(_object, _param, false);

END_PROPERTY


/***************************************************************************

  GridViewItem

***************************************************************************/


BEGIN_PROPERTY(CGRIDITEM_x)

  GB.ReturnInteger(WIDGET->columnPos(THIS->col) - WIDGET->contentsX() + WIDGET->clipper()->x());

END_PROPERTY


BEGIN_PROPERTY(CGRIDITEM_y)

  /*
  int p;

  p = WIDGET->rowPos(THIS->row) - WIDGET->contentsY() + WIDGET->frameWidth();
  if (!WIDGET->horizontalHeader()->isHidden())
    p += WIDGET->horizontalHeader()->height();

  GB.ReturnInteger(p);
  
  */

  GB.ReturnInteger(WIDGET->rowPos(THIS->row) - WIDGET->contentsY() + WIDGET->clipper()->y());

END_PROPERTY


BEGIN_PROPERTY(CGRIDITEM_width)

  GB.ReturnInteger(WIDGET->columnWidth(THIS->col) - 1);

END_PROPERTY


BEGIN_PROPERTY(CGRIDITEM_height)

  GB.ReturnInteger(WIDGET->rowHeight(THIS->row) - 1);

END_PROPERTY


BEGIN_PROPERTY(CGRIDITEM_text)

	text_property(_object, _param, true);

END_PROPERTY


BEGIN_PROPERTY(CGRIDITEM_picture)

	picture_property(_object, _param, true);

END_PROPERTY


BEGIN_PROPERTY(CGRIDITEM_font)

	font_property(_object, _param, true);

END_PROPERTY


BEGIN_PROPERTY(CGRIDITEM_alignment)

	alignment_property(_object, _param, true);

END_PROPERTY


BEGIN_PROPERTY(CGRIDITEM_background)

	background_property(_object, _param, true);

END_PROPERTY


BEGIN_PROPERTY(CGRIDITEM_foreground)

	foreground_property(_object, _param, true);

END_PROPERTY



BEGIN_METHOD_VOID(CGRIDITEM_ensure_visible)

  WIDGET->ensureCellVisible(THIS->row, THIS->col);

END_METHOD


BEGIN_METHOD_VOID(CGRIDITEM_refresh)

  WIDGET->item()->setRow(-1);
  WIDGET->item()->setCol(-1);
  WIDGET->updateCell(THIS->row, THIS->col);

END_METHOD


/***************************************************************************

  GridViewRows

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
  	if (VPROP(GB_INTEGER) != WIDGET->numRows())
    	WIDGET->setNumRows(VPROP(GB_INTEGER));
    //CGridView::fillItems(WIDGET);
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


BEGIN_PROPERTY(CGRIDROWS_width)

  if (WIDGET->verticalHeader()->isHidden())
    GB.ReturnInteger(0);
  else
    GB.ReturnInteger(WIDGET->verticalHeader()->width());

END_PROPERTY


BEGIN_PROPERTY(CGRIDROWS_resizable)

  if (READ_PROPERTY)
    GB.ReturnBoolean(WIDGET->verticalHeader()->isResizeEnabled());
  else
    WIDGET->verticalHeader()->setResizeEnabled(VPROP(GB_BOOLEAN));

END_PROPERTY


BEGIN_PROPERTY(CGRIDROWS_moveable)

  if (READ_PROPERTY)
    GB.ReturnBoolean(WIDGET->verticalHeader()->isMovingEnabled());
  else
    WIDGET->verticalHeader()->setMovingEnabled(VPROP(GB_BOOLEAN));

END_PROPERTY


static void select_rows(CGRIDVIEW *_object, int start, int length)
{
	length += start;

	for (int i = start; i < length; i++)
		WIDGET->selectRow(i);
}

BEGIN_METHOD(CGRIDROWS_select, GB_INTEGER start; GB_INTEGER length)

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


BEGIN_METHOD(CGRIDROWS_select_all, GB_BOOLEAN sel)

	if (VARGOPT(sel, TRUE))
		select_rows(THIS, 0, WIDGET->numRows());
	else
		WIDGET->clearSelection();

END_PROPERTY


BEGIN_METHOD_VOID(CGRIDROWS_unselect)

	WIDGET->clearSelection();

END_METHOD


BEGIN_PROPERTY(CGRIDROWS_text)

  if (READ_PROPERTY)
    GB.ReturnNewZeroString(TO_UTF8(WIDGET->verticalHeader()->label(THIS->row)));
  else
    WIDGET->verticalHeader()->setLabel(THIS->row, QSTRING_PROP());

END_PROPERTY


BEGIN_METHOD_VOID(CGRIDROWS_refresh)

  WIDGET->updateRow(THIS->row);

END_METHOD


BEGIN_METHOD_VOID(CGRIDROWS_selected)

	if (READ_PROPERTY)
  	GB.ReturnBoolean(WIDGET->isRowSelected(THIS->row, true));
	else
		WIDGET->selectRow(THIS->row);

END_METHOD


/***************************************************************************

  GridViewCols

***************************************************************************/

static void update_autoresize(CGRIDVIEW *_object)
{
	//for (i = 0; i < WIDGET->numCols(); i++)
  //	WIDGET->setColumnStretchable(i, THIS->autoresize);

  //WIDGET->horizontalHeader()->setResizeEnabled(!THIS->autoresize);

  //WIDGET->horizontalHeader()->setStretchEnabled(true);
  if (WIDGET->numCols())
  	WIDGET->setColumnStretchable(WIDGET->numCols() - 1, THIS->autoresize);
}

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
  	if (VPROP(GB_INTEGER) != WIDGET->numCols())
  	{
	    WIDGET->setNumCols(VPROP(GB_INTEGER));
	    update_autoresize(THIS);
		}
    //CGridView::fillItems(WIDGET);
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


BEGIN_PROPERTY(CGRIDCOLS_height)

  if (WIDGET->horizontalHeader()->isHidden())
    GB.ReturnInteger(0);
  else
    GB.ReturnInteger(WIDGET->horizontalHeader()->height());

END_PROPERTY


BEGIN_PROPERTY(CGRIDCOLS_resizable)

  if (READ_PROPERTY)
    GB.ReturnBoolean(WIDGET->horizontalHeader()->isResizeEnabled());
  else
  {
	  WIDGET->horizontalHeader()->setResizeEnabled(VPROP(GB_BOOLEAN));
  	//THIS->autoresize = !VPROP(GB_BOOLEAN);
  	//update_autoresize(THIS);
	}

END_PROPERTY


BEGIN_PROPERTY(CGRIDCOLS_moveable)

  if (READ_PROPERTY)
    GB.ReturnBoolean(WIDGET->horizontalHeader()->isMovingEnabled());
  else
    WIDGET->horizontalHeader()->setMovingEnabled(VPROP(GB_BOOLEAN));

END_PROPERTY


BEGIN_PROPERTY(CGRIDCOLS_text)

  if (READ_PROPERTY)
    GB.ReturnNewZeroString(TO_UTF8(WIDGET->horizontalHeader()->label(THIS->col)));
  else
    WIDGET->horizontalHeader()->setLabel(THIS->col, QSTRING_PROP());

END_PROPERTY


/*BEGIN_PROPERTY(CGRIDCOLS_stretchable)

  if (READ_PROPERTY)
    GB.ReturnBoolean(WIDGET->isColumnStretchable(THIS->col));
  else
    WIDGET->setColumnStretchable(THIS->col, VPROP(GB_BOOLEAN));

END_PROPERTY*/


BEGIN_METHOD_VOID(CGRIDCOLS_refresh)

  WIDGET->updateColumn(THIS->col);

END_METHOD


/***************************************************************************

  GridView

***************************************************************************/

BEGIN_METHOD(CGRIDVIEW_new, GB_OBJECT parent)

  MyTable *wid = new MyTable(QCONTAINER(VARG(parent)), THIS);

  QObject::connect(wid, SIGNAL(currentChanged(int, int)), MANAGER, SLOT(changed()));
  QObject::connect(wid, SIGNAL(selectionChanged()), MANAGER, SLOT(selected()));
  QObject::connect(wid, SIGNAL(doubleClicked(int, int, int, const QPoint &)), MANAGER, SLOT(activated()));
  QObject::connect(wid, SIGNAL(clicked(int, int, int, const QPoint &)), MANAGER, SLOT(clicked()));
  QObject::connect(wid, SIGNAL(contentsMoving(int, int)), MANAGER, SLOT(scrolled()));
  QObject::connect(wid->horizontalHeader(), SIGNAL(clicked(int)), MANAGER, SLOT(columnClicked(int)));
  QObject::connect(wid->verticalHeader(), SIGNAL(clicked(int)), MANAGER, SLOT(rowClicked(int)));
  QObject::connect(wid->horizontalHeader(), SIGNAL(sizeChange(int, int, int)), MANAGER, SLOT(columnResized(int)));
  QObject::connect(wid->verticalHeader(), SIGNAL(sizeChange(int, int, int)), MANAGER, SLOT(rowResized(int)));

  CWIDGET_new(wid, _object);

  THIS->row = -1;
  THIS->col = -1;

  wid->show();

END_METHOD


BEGIN_METHOD_VOID(CGRIDVIEW_clear)

	WIDGET->clear();

END_METHOD


static void set_current_cell(CGRIDVIEW *_object, int row, int col)
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
		/*if (row == WIDGET->currentRow() && WIDGET->selectionMode() == QTable::SingleRow)
		{
			WIDGET->blockSignals(true);
	    WIDGET->setCurrentCell(row == 0 ? 1 : row - 1, col);
	    WIDGET->blockSignals(false);
		}*/
    WIDGET->setCurrentCell(row, col);
	}
}

BEGIN_PROPERTY(CGRIDVIEW_row)

  if (READ_PROPERTY)
    GB.ReturnInteger(WIDGET->currentRow());
  else
    set_current_cell(THIS, VPROP(GB_INTEGER), WIDGET->currentColumn());

END_PROPERTY


BEGIN_PROPERTY(CGRIDVIEW_column)

  if (READ_PROPERTY)
    GB.ReturnInteger(WIDGET->currentColumn());
  else
    set_current_cell(THIS, WIDGET->currentRow(), VPROP(GB_INTEGER));

END_PROPERTY


BEGIN_METHOD(CGRIDVIEW_move_to, GB_INTEGER row; GB_INTEGER col)

  long row = VARG(row);
  long col = VARG(col);

  //if (CGridView::check(WIDGET, row, col))
  //  return;

  set_current_cell(THIS, row, col);

END_METHOD



#if 0
BEGIN_METHOD_VOID(CGRIDVIEW_clear)

  long rows = WIDGET->numRows();
  WIDGET->setNumRows(0);
  WIDGET->setNumRows(rows);

END_METHOD
#endif


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

  if (CGridView::check(WIDGET, THIS->row, THIS->col))
    return;
  else
    RETURN_SELF();

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


BEGIN_PROPERTY(CGRIDVIEW_scrollbars)

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


BEGIN_PROPERTY(CGRIDVIEW_header)

  if (READ_PROPERTY)
    GB.ReturnInteger(WIDGET->headers());
  else
    WIDGET->setHeaders(VPROP(GB_INTEGER));

END_PROPERTY


BEGIN_PROPERTY(CGRIDVIEW_mode)

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


BEGIN_PROPERTY(CGRIDVIEW_client_width)

  WIDGET->updateScrollBars();
  GB.ReturnInteger(WIDGET->visibleWidth());

END_PROPERTY


BEGIN_PROPERTY(CGRIDVIEW_client_height)

  WIDGET->updateScrollBars();
  GB.ReturnInteger(WIDGET->clipper()->height());

END_PROPERTY

// CWIDGET_refresh does not work for this widget ??
// Should do viewport->refresh() !

BEGIN_METHOD(CGRIDVIEW_refresh, GB_INTEGER x; GB_INTEGER y; GB_INTEGER w; GB_INTEGER h)

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

BEGIN_PROPERTY(CGRIDVIEW_scroll_x)

  if (READ_PROPERTY)
    GB.ReturnInteger(WIDGET->contentsX());
  else
    WIDGET->setContentsPos(VPROP(GB_INTEGER), WIDGET->contentsY());

END_PROPERTY


BEGIN_PROPERTY(CGRIDVIEW_scroll_y)

  if (READ_PROPERTY)
    GB.ReturnInteger(WIDGET->contentsY());
  else
    WIDGET->setContentsPos(WIDGET->contentsX(), VPROP(GB_INTEGER));

END_PROPERTY

BEGIN_METHOD(CGRIDVIEW_row_at, GB_INTEGER ypos)

  int ypos = VARG(ypos);

  ypos = ypos + WIDGET->contentsY();

  if (!WIDGET->horizontalHeader()->isHidden())
    ypos = ypos - WIDGET->horizontalHeader()->height();

  GB.ReturnInteger(WIDGET->rowAt(ypos));

END_PROPERTY

BEGIN_METHOD(CGRIDVIEW_column_at, GB_INTEGER xpos)

  int xpos = VARG(xpos);

  xpos = xpos + WIDGET->contentsX();

  if (!WIDGET->verticalHeader()->isHidden())
    xpos = xpos - WIDGET->verticalHeader()->width();

  GB.ReturnInteger(WIDGET->columnAt(xpos));

END_PROPERTY


BEGIN_PROPERTY(CGRIDVIEW_autoresize)

  if (READ_PROPERTY)
    GB.ReturnBoolean(THIS->autoresize);
  else
  {
  	THIS->autoresize = VPROP(GB_BOOLEAN);
  	update_autoresize(THIS);
	}

END_PROPERTY

#if 0
BEGIN_PROPERTY(CGRIDVIEW_table)

  if (READ_PROPERTY)
    GB.ReturnNewZeroString(TO_UTF8(WIDGET->definition()));
  else
    WIDGET->setDefinition(QSTRING_PROP());

END_PROPERTY
#endif


/***************************************************************************

  Descriptions

***************************************************************************/

GB_DESC CGridViewDataDesc[] =
{
  GB_DECLARE(".GridViewData", 0), GB_VIRTUAL_CLASS(),

  //GB_PROPERTY_READ("Row", "i", CGRIDITEM_row),
  //GB_PROPERTY_READ("Column", "i", CGRIDITEM_column),

  //GB_PROPERTY("Picture", "Picture", CGRIDITEM_picture),
  //GB_PROPERTY("Text", "s", CGRIDITEM_text),
  //GB_PROPERTY("Alignment", "i", CGRIDITEM_alignment),

  GB_PROPERTY("Text", "s", CGRIDVIEW_data_text),
  GB_PROPERTY("Picture", "Picture", CGRIDVIEW_data_picture),
  GB_PROPERTY("Font", "Font", CGRIDVIEW_data_font),
  GB_PROPERTY("Alignment", "i", CGRIDVIEW_data_alignment),
  GB_PROPERTY("Background", "i", CGRIDVIEW_data_background),
  GB_PROPERTY("BackColor", "i", CGRIDVIEW_data_background),
  GB_PROPERTY("Foreground", "i", CGRIDVIEW_data_foreground),
  GB_PROPERTY("ForeColor", "i", CGRIDVIEW_data_foreground),

  GB_END_DECLARE
};

GB_DESC CGridItemDesc[] =
{
  GB_DECLARE(".GridViewCell", 0), GB_VIRTUAL_CLASS(),

  //GB_PROPERTY_READ("Row", "i", CGRIDITEM_row),
  //GB_PROPERTY_READ("Column", "i", CGRIDITEM_column),

  GB_PROPERTY("Picture", "Picture", CGRIDITEM_picture),
  GB_PROPERTY("Font", "Font", CGRIDITEM_font),
  GB_PROPERTY("Text", "s", CGRIDITEM_text),
  GB_PROPERTY("Alignment", "i", CGRIDITEM_alignment),
  GB_PROPERTY("Background", "i", CGRIDITEM_background),
  GB_PROPERTY("BackColor", "i", CGRIDITEM_background),
  GB_PROPERTY("Foreground", "i", CGRIDITEM_foreground),
  GB_PROPERTY("ForeColor", "i", CGRIDITEM_foreground),

  GB_PROPERTY_READ("X", "i", CGRIDITEM_x),
  GB_PROPERTY_READ("Y", "i", CGRIDITEM_y),
  GB_PROPERTY_READ("Left", "i", CGRIDITEM_x),
  GB_PROPERTY_READ("Top", "i", CGRIDITEM_y),
  GB_PROPERTY_READ("Width", "i", CGRIDITEM_width),
  GB_PROPERTY_READ("Height", "i", CGRIDITEM_height),
  GB_PROPERTY_READ("W", "i", CGRIDITEM_width),
  GB_PROPERTY_READ("H", "i", CGRIDITEM_height),

  //GB_METHOD("Clear", NULL, CGRIDITEM_clear, NULL),
  GB_METHOD("EnsureVisible", NULL, CGRIDITEM_ensure_visible, NULL),
  GB_METHOD("Refresh", NULL, CGRIDITEM_refresh, NULL),

  GB_END_DECLARE
};


GB_DESC CGridRowDesc[] =
{
  GB_DECLARE(".GridViewRow", 0), GB_VIRTUAL_CLASS(),

  GB_PROPERTY_READ("X", "i", CGRIDITEM_x),
  GB_PROPERTY("Height", "i", CGRIDROWS_height),
  GB_PROPERTY("H", "i", CGRIDROWS_height),
  GB_PROPERTY("Text", "s", CGRIDROWS_text),
  GB_PROPERTY("Title", "s", CGRIDROWS_text),
  GB_PROPERTY("Selected", "b", CGRIDROWS_selected),
  GB_METHOD("Refresh", NULL, CGRIDROWS_refresh, NULL),

  GB_END_DECLARE
};


GB_DESC CGridColumnDesc[] =
{
  GB_DECLARE(".GridViewColumn", 0), GB_VIRTUAL_CLASS(),

  GB_PROPERTY_READ("Y", "i", CGRIDITEM_y),
  GB_PROPERTY("Width", "i", CGRIDCOLS_width),
  GB_PROPERTY("W", "i", CGRIDCOLS_width),
  GB_PROPERTY("Text", "s", CGRIDCOLS_text),
  GB_PROPERTY("Title", "s", CGRIDCOLS_text),
  GB_METHOD("Refresh", NULL, CGRIDCOLS_refresh, NULL),

  GB_END_DECLARE
};


GB_DESC CGridRowsDesc[] =
{
  GB_DECLARE(".GridViewRows", 0), GB_VIRTUAL_CLASS(),

  GB_METHOD("_get", ".GridViewRow", CGRIDROWS_get, "(Row)i"),
  GB_PROPERTY("Count", "i", CGRIDROWS_count),
  GB_PROPERTY("Height", "i", CGRIDROWS_height),
  GB_PROPERTY("H", "i", CGRIDROWS_height),
  GB_PROPERTY_READ("Width", "i", CGRIDROWS_width),
  GB_PROPERTY_READ("W", "i", CGRIDROWS_width),
  GB_PROPERTY("Resizable", "b", CGRIDROWS_resizable),
  GB_PROPERTY("Moveable", "b", CGRIDROWS_moveable),
  GB_METHOD("Select", NULL, CGRIDROWS_select, "[(Start)i(Length)i]"),
  GB_METHOD("SelectAll", NULL, CGRIDROWS_select_all, "[(Selected)b]"),
  GB_METHOD("UnSelect", NULL, CGRIDROWS_unselect, NULL),
  GB_END_DECLARE
};


GB_DESC CGridColumnsDesc[] =
{
  GB_DECLARE(".GridViewColumns", 0), GB_VIRTUAL_CLASS(),

  GB_METHOD("_get", ".GridViewColumn", CGRIDCOLS_get, "(Column)i"),
  GB_PROPERTY("Count", "i", CGRIDCOLS_count),
  GB_PROPERTY("Width", "i", CGRIDCOLS_width),
  GB_PROPERTY("W", "i", CGRIDCOLS_width),
  GB_PROPERTY_READ("Height", "i", CGRIDCOLS_height),
  GB_PROPERTY_READ("H", "i", CGRIDCOLS_height),
  GB_PROPERTY("Resizable", "b", CGRIDCOLS_resizable),
  GB_PROPERTY("Moveable", "b", CGRIDCOLS_moveable),

  GB_END_DECLARE
};


GB_DESC CGridViewDesc[] =
{
  GB_DECLARE("GridView", sizeof(CGRIDVIEW)), GB_INHERITS("Control"),

  GB_CONSTANT("None", "i", 0),
  GB_CONSTANT("Horizontal", "i", 1),
  GB_CONSTANT("Vertical", "i", 2),
  GB_CONSTANT("Both", "i", 3),

  GB_METHOD("_new", NULL, CGRIDVIEW_new, "(Parent)Container;"),
  //GB_METHOD("_free", NULL, CGRIDVIEW_free, NULL),

  GB_PROPERTY_READ("Rows", ".GridViewRows", CGRIDVIEW_self),
  GB_PROPERTY_READ("Columns", ".GridViewColumns", CGRIDVIEW_self),

  GB_METHOD("_get", ".GridViewCell", CGRIDVIEW_get, "(Row)i(Column)i"),

  GB_METHOD("Clear", NULL, CGRIDVIEW_clear, NULL),

  GB_PROPERTY("Row", "i", CGRIDVIEW_row),
  GB_PROPERTY("Column", "i", CGRIDVIEW_column),
  GB_METHOD("MoveTo", NULL, CGRIDVIEW_move_to, "(Row)i(Column)i"),
  GB_PROPERTY_READ("Current", ".GridViewCell", CGRIDVIEW_current),
  GB_PROPERTY("Grid", "b", CGRIDVIEW_grid),
  GB_PROPERTY("Border", "b", CWIDGET_border_simple),
  GB_PROPERTY("ScrollBar", "i", CGRIDVIEW_scrollbars),
  GB_PROPERTY("Header", "i", CGRIDVIEW_header),
  GB_PROPERTY("Mode", "i", CGRIDVIEW_mode),
  GB_PROPERTY("AutoResize", "b", CGRIDVIEW_autoresize),
  GB_PROPERTY("Resizable", "b", CGRIDCOLS_resizable),

  GB_METHOD("RowAt", "i", CGRIDVIEW_row_at, "(Y)i"),
  GB_METHOD("ColumnAt", "i", CGRIDVIEW_column_at,"(X)i"),

  GB_PROPERTY("ScrollX", "i", CGRIDVIEW_scroll_x),
  GB_PROPERTY("ScrollY", "i", CGRIDVIEW_scroll_y),
  GB_PROPERTY_READ("ClientWidth", "i", CGRIDVIEW_client_width),
  GB_PROPERTY_READ("ClientW", "i", CGRIDVIEW_client_width),
  GB_PROPERTY_READ("ClientHeight", "i", CGRIDVIEW_client_height),
  GB_PROPERTY_READ("ClientH", "i", CGRIDVIEW_client_height),

  GB_PROPERTY_SELF("Data", ".GridViewData"),

  GB_METHOD("Refresh", NULL, CGRIDVIEW_refresh, "[(X)i(Y)i(Width)i(Height)i]"),

  GB_CONSTANT("_Properties", "s", 
  	"*,Mode{Select.None;Single;Multiple}=None,Grid=True,Header{GridView.None;Vertical;Horizontal;Both}=None,"
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
  GB_EVENT("ColumnResize", NULL, "(Column)i", &EVENT_ColumnResize),
  GB_EVENT("RowResize", NULL, "(Row)i", &EVENT_RowResize),

  GB_END_DECLARE
};


/***************************************************************************

  class CGridView

***************************************************************************/

CGridView CGridView::manager;

bool CGridView::checkRow(QTable *table, long row)
{
  if (row < 0 || row >= table->numRows())
  {
    GB.Error("Bad row index");
    return true;
  }
  else
    return false;
}

bool CGridView::checkCol(QTable *table, long col)
{
  if (col < 0 || col >= table->numCols())
  {
    GB.Error("Bad column index");
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

void CGridView::changed(void)
{
	GET_SENDER(_object);
  MyTable *w = (MyTable *)sender();

  if (w->currentRow() < 0 || w->currentColumn() < 0)
    return;

  GB.Raise(THIS, EVENT_Change, 0);
}

void CGridView::activated(void)
{
	GET_SENDER(_object);
  GB.Raise(THIS, EVENT_Activate, 0);
}

void CGridView::selected(void)
{
	GET_SENDER(_object);
  if (WIDGET->selectionMode() == QTable::SingleRow)
  	GB.Raise(THIS, EVENT_Select, 0);
  else
  	GB.RaiseLater(THIS, EVENT_Select);
}

void CGridView::clicked(void)
{
	GET_SENDER(_object);
  GB.Raise(THIS, EVENT_Click, 0);
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

void CGridView::columnClicked(int col)
{
	GET_SENDER(_object);
  GB.Raise(THIS, EVENT_ColumnClick, 1, GB_T_INTEGER, col);
}

void CGridView::rowClicked(int row)
{
	GET_SENDER(_object);
  GB.Raise(THIS, EVENT_RowClick, 1, GB_T_INTEGER, row);
}

void CGridView::columnResized(int col)
{
	GET_SENDER(_object);
  GB.Raise(THIS, EVENT_ColumnResize, 1, GB_T_INTEGER, col);
}

void CGridView::rowResized(int row)
{
	GET_SENDER(_object);
  GB.Raise(THIS, EVENT_RowResize, 1, GB_T_INTEGER, row);
}

