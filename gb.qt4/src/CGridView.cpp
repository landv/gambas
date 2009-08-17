/***************************************************************************

  CGridView.cpp

  (c) 2000-2009 Benoît Minisini <gambas@users.sourceforge.net>

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
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

***************************************************************************/

#define __CGRIDVIEW_CPP

#include "main.h"

#include <QApplication>
#include <Q3Header>
#include <QPainter>
#include <QPalette>
#include <QPixmap>
#include <QResizeEvent>
#include <QPaintEvent>
#include <QColor>
#include <QTimer>

#include "CConst.h"
#include "CColor.h"
#include "CDraw.h"
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
	padding = 0;
	richText = 0;
}

void MyTableData::clear()
{
	GB.Unref(POINTER(&pict));
	GB.Unref(POINTER(&font));
	GB.FreeString(&text);
	GB.FreeString(&richText);
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

#define GET_ROW_SPAN(_span) ((int)(short)(((int)_span) & 0xFFFF))
#define GET_COL_SPAN(_span) ((int)(short)((((int)_span) >> 16) & 0xFFFF))
#define MAKE_SPAN(_rowspan, _colspan) (((uint)(unsigned short)(_rowspan)) | (((uint)(unsigned short)(_colspan)) << 16))

MyTableItem::MyTableItem(Q3Table *table, CGRIDVIEW *view)
: Q3TableItem(table, Q3TableItem::Never, 0)
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

QString MyTableItem::richText()
{
	return TO_QSTRING(data()->richText);
}

QPixmap MyTableItem::pixmap()
{
	CPICTURE *pict = data()->pict;
	
	if (pict)
		return *(pict->pixmap);
	else
		return Q3TableItem::pixmap();
}

int MyTableItem::alignment() const
{
	return CCONST_alignment(((MyTableItem *)this)->data()->alignment, ALIGN_NORMAL, false);
}

void MyTableItem::paint( QPainter *p, const QColorGroup &cg, const QRect &cr, bool selected )
{
	MyTableData *d = data();
	
	if (!d) // May happen, do not why!
		return;
	
	int w = cr.width();
	int h = cr.height();
	int x = 0;
	int y = 0;

	QPixmap pix = pixmap();
	QString txt;
	bool rich;
	int _bg = d->bg;
	int _fg = d->fg;
	int _alignment = CCONST_alignment(d->alignment, ALIGN_NORMAL, true);
	int _padding = QMAX(1, (int)d->padding);
	
	/*if (((MyTable *)table())->noSelection())
		selected = false;
	else*/ 
	/*if (row() == table()->currentRow() && col() == table()->currentColumn())
		selected = ((MyTable *)table())->isRowReallySelected(row());*/
	
	p->fillRect(x, y, w, h,
		selected ?
			cg.brush(QColorGroup::Highlight).color()
			: (_bg == COLOR_DEFAULT) ? cg.brush(QColorGroup::Base).color() : QColor((uint)_bg));

	if (_padding)
	{
		x += _padding;
		y += _padding;
		w -= _padding * 2;
		h -= _padding * 2;
		
		if (w < 1 || h < 1)
			return;
	}

	txt = richText();
	rich = txt.length() > 0;
	if (!rich)
		txt = text();

	if (!pix.isNull())
	{
		if (txt.length() == 0)
		{
			DRAW_aligned_pixmap(p, pix, x, y, w, h, _alignment);
		}
		else
		{
			p->drawPixmap(x, y + (h - pix.height() ) / 2, pix);
			x += pix.width() + _padding;
			w -= pix.width() + _padding;
		}
	}

	if (selected)
		p->setPen(cg.highlightedText());
	else if (_fg == COLOR_DEFAULT)
		p->setPen(cg.text());
	else
		p->setPen(QColor(_fg));

	if (d->font)
		p->setFont(*(d->font->font));

	if (rich)
		DRAW_rich_text(p, x, y, w, h, wordWrap() ? (_alignment | Qt::WordBreak) : _alignment, txt );
	else
		p->drawText(x, y, w, h, wordWrap() ? (_alignment | Qt::WordBreak) : _alignment, txt );
		
	if (d->font)
		p->setFont(table()->font());
}


QSize MyTableItem::sizeHint() const
{
	QSize strutSize = QApplication::globalStrut();
	QSize s;
	MyTableItem *item = (MyTableItem *)this;
	MyTableData *d = item->data();
	int padding;
	
	QPixmap pix = item->pixmap();
	QString t = item->text();
	QFontMetrics fm = d->font ? QFontMetrics(*(d->font->font)) : table()->fontMetrics();

	padding = QMAX(1, (int)d->padding);
	
	if (!pix.isNull() )
	{
		s = pix.size();
	  s.setWidth(s.width());
	  if (t.length())
		  s.setWidth(s.width() + padding);
	}

	if (t.length())
	{
		if ( !wordWrap() && t.find( '\n' ) == -1 )
		{
			s = QSize(s.width() + fm.width(t) + 10,
								QMAX( s.height(), fm.height() ) ).expandedTo( strutSize );
		}
		else
		{
			QRect r = fm.boundingRect(0, 0, table()->columnWidth( col() ) - padding, 0,
									wordWrap() ? (alignment() | Qt::WordBreak) : alignment(),
									t );
			r.setWidth( QMAX( r.width() + 10, table()->columnWidth(col())));
	
			s = QSize( r.width(), QMAX( s.height(), r.height() ) ).expandedTo( strutSize );
		}
	}
  
	s.setWidth(s.width() + padding * 2);
	s.setHeight(s.height() + padding * 2);
  
	s.setWidth(s.width() + padding);
	s.setHeight(s.height() + padding);

	return s;
}

void MyTableItem::getSpan(int row, int col, int *rowspan, int *colspan)
{
	int span;
	
	*colspan = 0;
	*rowspan = 0;

	if ((col < 0) || (col >= table()->numCols())) return;
	if ((row < 0) || (row >= table()->numRows())) return;

	span = _span.value(getKey(row, col), 0);
	if (!span)
		return;

	*colspan = qMin(GET_COL_SPAN(span), table()->numCols() - col - 1);
	*rowspan = qMin(GET_ROW_SPAN(span), table()->numRows() - row - 1);
}

void MyTableItem::setSpan(int row, int col, int rowspan, int colspan)
{
	int i, j, key;
	int oldcs, oldrs;
	int span;

	if ((col < 0) || (col >= table()->numCols())) return;
	if ((row < 0) || (row >= table()->numRows())) return;

	if (rowspan < -32768 || rowspan > 32767)
		return;
	if (colspan < -32768 || colspan > 32767)
		return;
	
	if (rowspan > 0 && colspan < 0)
		colspan = 0;
	if (colspan > 0 && rowspan < 0)
		rowspan = 0;

	key = getKey(row, col);
	span = _span.value(key, 0);
	
	oldcs = GET_COL_SPAN(span);
	oldrs = GET_ROW_SPAN(span);
	if ((oldcs < 0 || oldrs < 0) && (colspan != 0 || rowspan != 0))
		return;
	
	_span.remove(key);
	
	if (oldcs > 0 || oldrs > 0)
	{
		i = 1; j = 0;
		for(;;)
		{
			if (i > oldcs)
			{
				i = 0;
				j++;
				if (j > oldrs)
					break;
			}
			setSpan(col + i, row + j, 0, 0);
			i++;
		}
	}

	if (rowspan == 0 && colspan == 0)
		return;
	
	_span.insert(key, MAKE_SPAN(rowspan, colspan));
	
	if (rowspan >= 0 && colspan >= 0)
	{
		i = 1; j = 0;
		for(;;)
		{
			if (i > colspan)
			{
				i = 0;
				j++;
				if (j > rowspan)
					break;
			}
			setSpan(row + j, col + i, -j, -i);
			i++;
		}
	}
}

void MyTableItem::move(int srow, int scol, int drow, int dcol)
{
	MyTableData *data;
	int skey = getKey(srow, scol);
	int dkey = getKey(drow, dcol);
	
	_dict.remove(dkey);
	
	data = _dict.take(skey);
	if (data)
		_dict.insert(dkey, data);
}

/** MyTable ******************************************************************/

MyTable::MyTable(QWidget *parent, CGRIDVIEW *view) :
Q3Table(0, 0, parent)
{
	_item = new MyTableItem(this, view);
	_header = 0;
	_rows = 0;
	_cols = 0;
	_last_col_width = 0;
	_no_row = true;
	_no_col = true;
	_updating_last_column = false;
	_autoresize = true;
	_updateLastColumn = false;
	_enableUpdates = false;
	_min_row = -1;
	_show_grid = true;
	
	setSelectionMode(NoSelection);
	setFocusStyle(FollowStyle);
	verticalHeader()->setMovingEnabled(false);
	horizontalHeader()->setMovingEnabled(false);
	setDragEnabled(false);
	setShowGrid(false);
	updateHeaders();
}

MyTable::~MyTable()
{
	delete _item;
}

void MyTable::paintFocus( QPainter *p, const QRect &r )
{
	//Q3Table::paintFocus(p, r);
}

void MyTable::paintCell(QPainter *p, int row, int col, const QRect &cr, bool selected, const QColorGroup &cg)
{
	Q3Table::paintCell(p, row, col, cr, selected, cg);
	if (_show_grid)
	{
    QPalette pal = cg;
    int w = cr.width();
    int h = cr.height();
    int x2 = w - 1;
    int y2 = h - 1;
		QPen pen(p->pen());
		p->setPen(pal.mid().color());
		p->drawLine(x2, 0, x2, y2);
		p->drawLine(0, y2, x2, y2);
		p->setPen(pen);
	}
}

void MyTable::drawContents(QPainter *p, int cx, int cy, int cw, int ch)
{
	int colspan, rowspan, rs, cs;
	int colfirst = columnAt(cx);
	int collast = columnAt(cx + cw);
	int rowfirst = rowAt(cy);
	int rowlast = rowAt(cy + ch);

	if (rowfirst == -1 || colfirst == -1) 
	{
		paintEmptyArea(p, cx, cy, cw, ch);
		return;
	}

	/*drawActiveSelection = hasFocus() || viewport()->hasFocus() || d->inMenuMode
											|| is_child_of(qApp->focusWidget(), viewport())
											|| !style()->styleHint(QStyle::SH_ItemView_ChangeHighlightOnFocus, 0, this);*/
	if (rowlast == -1)
			rowlast = numRows() - 1;
	if (collast == -1)
			collast = numCols() - 1;

	// Go through the rows
	for (int r = rowfirst; r <= rowlast; ++r) {
		// get row position and height
		int rowp = rowPos(r);
		int rowh = rowHeight(r);

		// Go through the columns in row r
		// if we know from where to where, go through [colfirst, collast],
		// else go through all of them
		for (int c = colfirst; c <= collast; ++c) 
		{
			// get position and width of column c
			int colp, colw;
			colp = columnPos(c);
			colw = columnWidth(c);
			int oldrp = rowp;
			int oldrh = rowh;

			MyTableItem *itm = (MyTableItem *)item(r, c);
			itm->getSpan(r, c, &rowspan, &colspan);
			if (rowspan >= 0 && colspan >= 0)
			{
				rs = r;
				cs = c;
			}
			else if ((c == colfirst && colspan < 0) || (r == rowfirst && rowspan < 0))
			{
				rs = r + rowspan;
				cs = c + colspan;
				itm->getSpan(rs, cs, &rowspan, &colspan);
				if (rowspan < 0 && colspan < 0)
				{
					rs = r;
					cs = c;
					rowspan = 0;
					colspan = 0;
				}
				else
				{
					rowp = rowPos(rs);
					colp = columnPos(cs);
				}
			}
			else
				continue;
			
			if (rowspan > 0 || colspan > 0)
			{
				rowh = 0;
				int i;
				for (i = 0; i <= rowspan; ++i)
					rowh += rowHeight(i + rs);
				colw = 0;
				for (i = 0; i <= colspan; ++i)
					colw += columnWidth(i + cs);
			}

			// Translate painter and draw the cell
			p->translate(colp, rowp);
			//bool selected = isSelected(rs, cs);
			/*if (focusStl != FollowStyle && selected && !currentInSelection &&
						r == curRow && c == curCol )
					selected = false;*/
			paintCell(p, rs, cs, QRect(colp, rowp, colw, rowh), isRowReallySelected(rs), colorGroup());
			p->translate(-colp, -rowp);

			rowp = oldrp;
			rowh = oldrh;

			/*QWidget *w = cellWidget(r, c);
			QRect cg(cellGeometry(r, c));
			if (w && w->geometry() != QRect(contentsToViewport(cg.topLeft()), cg.size() - QSize(1, 1))) {
					moveChild(w, colp, rowp);
					w->resize(cg.size() - QSize(1, 1));
			}*/
		}
	}
	
	//d->lastVisCol = collast;
	//d->lastVisRow = rowlast;

	// draw indication of current cell
	/*QRect focusRect = cellGeometry(currentRow(), currentColumn());
	p->translate(focusRect.x(), focusRect.y());
	paintFocus(p, focusRect);
	p->translate(-focusRect.x(), -focusRect.y());*/

	// Paint empty rects
	paintEmptyArea(p, cx, cy, cw, ch);
}


void MyTable::enableUpdates()
{
	horizontalHeader()->setUpdatesEnabled(true);
	verticalHeader()->setUpdatesEnabled(true);
	setUpdatesEnabled(true);
	_enableUpdates = false;
	rowHeightChanged(_min_row);
}

void MyTable::rowHeightChanged(int row)
{
	if (_enableUpdates)
	{
		_min_row = QMIN(_min_row, row);
		return;
	}
	Q3Table::rowHeightChanged(row);
}

void MyTable::setRowHeight(int row, int height)
{
	/*verticalHeader()->setUpdatesEnabled(false);
	horizontalHeader()->setUpdatesEnabled(false);
	setUpdatesEnabled(false);
	if (!_enableUpdates)
	{
		_enableUpdates = true;
		_min_row = numRows();
		QTimer::singleShot(0, this, SLOT(enableUpdates()));
	}*/
	
	if (height < 0)
		adjustRow(row);
	else if (height != rowHeight(row))
		Q3Table::setRowHeight(row, height);
}


void MyTable::setColumnWidth(int col, int width)
{
	//qDebug("MyTable::setColumnWidth(%d, %ld): %d", col, width, columnWidth(col));
	if (width < 0)
		adjustColumn(col);
	else if (width != columnWidth(width))
		Q3Table::setColumnWidth(col, width);
		
	if (col == (numCols() - 1) && !_updating_last_column)
		_last_col_width = columnWidth(numCols() - 1);
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

void MyTable::changeEvent(QEvent *e)
{
	Q3Table::changeEvent(e);
	if (e->type() == QEvent::FontChange || e->type() == QEvent::StyleChange)
		updateHeaders();
}

Q3TableItem *MyTable::item( int row, int col ) const
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
	bool b;

	if (newCols < 0)
		return;

	_cols = newCols;
	_item->invalidate();

	b = signalsBlocked();
	blockSignals(true);
	Q3Table::setNumCols(newCols);
	blockSignals(b);

	_last_col_width = 0;

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
	bool b;
	
	if (newRows < 0)
		return;

	_rows = newRows;
	_item->invalidate();
	
	b = signalsBlocked();
	blockSignals(true);
	Q3Table::setNumRows(newRows);
	blockSignals(b);

	clearSelection();
	emit currentChanged(-1, -1); 
}


void MyTable::updateRow(int row)
{
	if (row < 0 || row >= numRows() || numCols() == 0)
		return;
	
	QRect cg = cellGeometry(row, 0);

	QRect r(contentsToViewport( QPoint( contentsX(), cg.y() - 2 ) ),
		QSize( contentsWidth(), cg.height()) );

	QApplication::postEvent(viewport(), new QPaintEvent(r));
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
		QSize( cg.width(), contentsHeight() ) );

	QApplication::postEvent(viewport(), new QPaintEvent(r));
}

void MyTable::setCurrentCell(int row, int col)
{
	int rowspan, colspan;
	
	_no_row = row < 0 || row >= numRows();
	_no_col = col < 0 || col >= numCols();
	
	if (_no_row || _no_col)
	{
		clearSelection();
		return;
	}
	
	if (selectionMode() != MultiRow)
	{
		_item->getSpan(row, col, &rowspan, &colspan);
	
		if (rowspan < 0)
			row += rowspan;
		if (colspan < 0)
			col += colspan;
	}

	Q3Table::setCurrentCell(row, col);
}

void MyTable::updateCurrentCell()
{
	setCurrentCell(currentRow(), currentColumn());
}

void MyTable::getCurrentCell(int *row, int *col)
{
	if (row)
	{
		if (_no_row)
			*row = -1;
		else
			*row = currentRow();
	}
	
	if (col)
	{
		if (_no_col)
			*col = -1;
		else
			*col = currentColumn();
	}
}

int MyTable::findSelection(int row)
{
	int i;
	Q3TableSelection s;
	
	for (i = 0; i < numSelections(); i++)
	{
		s = selection(i);	
		if (row >= s.topRow() && row <= s.bottomRow())
			return i;
	}
	
	return -1;
}

bool MyTable::isRowReallySelected(int row)
{
	return findSelection(row) >= 0;
}

void MyTable::selectRow(int row, bool update)
{
	if (selectionMode() == NoSelection)
		return;
		
	if (selectionMode() == SingleRow)
	{
		if (row == currentRow() && !isRowReallySelected(row))
		{
			setSelectionMode(NoSelection);
			setSelectionMode(SingleRow);
		}
		else
			setCurrentCell(row, currentColumn());
		return;
	}
	
	if (isRowReallySelected(row))
		return;
	
	Q3TableSelection s(row, 0, row, numCols() - 1);
	addSelection(s);
	
	if (update)
		updateHeaderStates();
}

void MyTable::unselectRow(int row)
{
	int i;
	Q3TableSelection s;
	bool u = false;
	
	if (selectionMode() != MultiRow)
		return;
	
	for(;;)
	{
		i = findSelection(row);
		if (i < 0)
			break;
		
		s = selection(i);
		removeSelection(i);
		u = true;
		
		for (i = s.topRow(); i <= s.bottomRow(); i++)
			if (i != row)
				selectRow(i);
	}
				
	if (u) 
	{
		updateHeaderStates();
	}
}


void MyTable::selectRows(int start, int length)
{
	if (selectionMode() == NoSelection)
		return;

	length += start;

	if (selectionMode() == SingleRow)
	{
		selectRow(length - 1);
		return;
	}

	for (int i = start; i < length; i++)
		selectRow(i, false);
		
	updateHeaderStates();
}

void MyTable::updateLastColumn()
{
	int n = numCols() - 1;
	
	if (n < 0)
		return;
	
	if (_updating_last_column || !_autoresize)
		return;
		
	_updating_last_column = true;
	
	if (!_last_col_width)
		_last_col_width = columnWidth(n);
	
	if (((columnPos(n) + _last_col_width) < visibleWidth()) && (columnWidth(n) != visibleWidth() - columnPos(n)))
		setColumnWidth(n, visibleWidth() - columnPos(n));

	_updating_last_column = false;
	_updateLastColumn = false;
}

void MyTable::updateLastColumnLater()
{
	if (_updateLastColumn)
		return;
	_updateLastColumn = true;
	QTimer::singleShot(0, this, SLOT(updateLastColumn()));	
}

void MyTable::resizeEvent(QResizeEvent *e)
{
	Q3Table::resizeEvent(e);
	updateLastColumn();
}

void MyTable::columnWidthChanged(int col)
{
	//qDebug("MyTable::columnWidthChanged");
	Q3Table::columnWidthChanged(col);
	//if (col != (numCols() - 1))
	updateLastColumnLater();
}

QRect MyTable::cellGeometry(int row, int col) const
{
	QRect r;
	int rowspan, colspan, i;
	
	_item->getSpan(row, col, &rowspan, &colspan);
	r = Q3Table::cellGeometry(row, col);
	
	if (rowspan == 0 && colspan == 0)
		return r;
	
	if (rowspan <= 0 && colspan <= 0)
		return cellGeometry(row + rowspan, col + colspan);
	
	for (i = 1; i <= rowspan; i++)
		r.setHeight(r.height() + rowHeight(row + i));
	for (i = 1; i <= colspan; i++)
		r.setWidth(r.width() + columnWidth(col + i));
	
	return r;
}

/*QRect MyTable::cellRect(int row, int col)
{
}*/




/** GridView *****************************************************************/

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

static void rich_text_property(void *_object, void *_param, bool create)
{
	MyTableItem *item = get_item(_object, create);
	
	if (READ_PROPERTY)
		GB.ReturnString(item->data()->richText);
	else
	{
		GB.StoreString(PROP(GB_STRING), &item->data(create)->richText);
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


static void padding_property(void *_object, void *_param, bool create)
{
	MyTableItem *item = get_item(_object, create);
		
	if (READ_PROPERTY)
		GB.ReturnInteger(item->data()->padding);
	else
	{
		item->data(create)->padding = QMAX(0, VPROP(GB_INTEGER));
		if (create)
			WIDGET->updateCell(THIS->row, THIS->col);
	}
}


/***************************************************************************

	GridViewData

***************************************************************************/

BEGIN_PROPERTY(CGRIDVIEW_data_text)

	text_property(_object, _param, false);

END_PROPERTY


BEGIN_PROPERTY(CGRIDVIEW_data_rich_text)

	rich_text_property(_object, _param, false);

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


BEGIN_PROPERTY(CGRIDVIEW_data_padding)

	padding_property(_object, _param, false);

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


BEGIN_PROPERTY(CGRIDITEM_rich_text)

	rich_text_property(_object, _param, true);

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


BEGIN_PROPERTY(CGRIDITEM_padding)

	padding_property(_object, _param, true);

END_PROPERTY


BEGIN_METHOD_VOID(CGRIDITEM_ensure_visible)

	WIDGET->ensureCellVisible(THIS->row, THIS->col);

END_METHOD


BEGIN_METHOD_VOID(CGRIDITEM_refresh)

	WIDGET->item()->setRow(-1);
	WIDGET->item()->setCol(-1);
	WIDGET->updateCell(THIS->row, THIS->col);

END_METHOD


BEGIN_METHOD_VOID(CGRIDITEM_clear)

	MyTableItem *item = get_item(THIS, false);
	item->clear();
	
	WIDGET->item()->setRow(-1);
	WIDGET->item()->setCol(-1);
	WIDGET->updateCell(THIS->row, THIS->col);

END_METHOD

BEGIN_PROPERTY(CGRIDITEM_row_span)

	int rowspan, colspan;
	
	WIDGET->item()->getSpan(THIS->row, THIS->col, &rowspan, &colspan);

	if (READ_PROPERTY)
	{
		if (rowspan >= 0)
			GB.ReturnInteger(rowspan + 1);
		else
			GB.ReturnInteger(rowspan);
	}
	else
	{
		WIDGET->item()->setSpan(THIS->row, THIS->col, VPROP(GB_INTEGER) - 1, colspan);
	}

END_PROPERTY

BEGIN_PROPERTY(CGRIDITEM_column_span)

	int rowspan, colspan;
	
	WIDGET->item()->getSpan(THIS->row, THIS->col, &rowspan, &colspan);

	if (READ_PROPERTY)
	{
		if (colspan >= 0)
			GB.ReturnInteger(colspan + 1);
		else
			GB.ReturnInteger(colspan);
	}
	else
	{
		WIDGET->item()->setSpan(THIS->row, THIS->col, rowspan, VPROP(GB_INTEGER) - 1);
	}

END_PROPERTY

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

	int row = THIS->row;

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

	int row = THIS->row;

	if (READ_PROPERTY)
		GB.ReturnBoolean(WIDGET->verticalHeader()->isResizeEnabled(row));
	else
		WIDGET->verticalHeader()->setResizeEnabled(VPROP(GB_BOOLEAN), row);

END_PROPERTY


// BEGIN_PROPERTY(CGRIDROWS_moveable)
// 
//   if (READ_PROPERTY)
//     GB.ReturnBoolean(WIDGET->verticalHeader()->isMovingEnabled());
//   else
//     WIDGET->verticalHeader()->setMovingEnabled(VPROP(GB_BOOLEAN));
// 
// END_PROPERTY


BEGIN_METHOD(CGRIDROWS_select, GB_INTEGER start; GB_INTEGER length)

	int start, length;

	start = VARGOPT(start, 0);
	if (start < 0)
		start = 0;

	length = VARGOPT(length, WIDGET->numRows() - start);
	if (length < 0)
		length = 0;

	WIDGET->clearSelection();
	WIDGET->selectRows(start, length);

END_PROPERTY


BEGIN_METHOD(CGRIDROWS_select_all, GB_BOOLEAN sel)

	WIDGET->clearSelection();
	if (VARGOPT(sel, TRUE))
		WIDGET->selectRows(0, WIDGET->numRows());

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
		GB.ReturnBoolean(WIDGET->isRowReallySelected(THIS->row));
	else
	{
		if (VPROP(GB_BOOLEAN))
		{
			if (!WIDGET->isRowReallySelected(THIS->row))
				WIDGET->selectRow(THIS->row);
		}
		else
			WIDGET->unselectRow(THIS->row);
	}
	
END_METHOD


BEGIN_METHOD(CGRIDROWS_remove, GB_INTEGER start; GB_INTEGER length)

	int start = VARG(start);
	int length = VARGOPT(length, 1);
	int i, j, dst;

	if (start < 0 || start >= WIDGET->numRows() || length <= 0 || (start + length) > WIDGET->numRows())
	{
		GB.Error(GB_ERR_ARG);
		return;
	}
	
	dst = start;
	
	for (i = start + length; i < WIDGET->numRows(); i++)
	{
		for (j = 0; j < WIDGET->numCols(); j++)
			WIDGET->moveItem(i, j, dst, j);
		dst++;
	}
		
	WIDGET->setNumRows(WIDGET->numRows() - length);

END_METHOD


BEGIN_METHOD(CGRIDROWS_insert, GB_INTEGER start; GB_INTEGER length)

	int start = VARG(start);
	int length = VARGOPT(length, 1);
	int i, j;

	if (start < 0 || length <= 0 || start > WIDGET->numRows())
	{
		GB.Error(GB_ERR_ARG);
		return;
	}
	
	for (i = WIDGET->numRows() - 1; i >= start; i--)
	{
		for (j = 0; j < WIDGET->numCols(); j++)
			WIDGET->moveItem(i, j, i + length, j);
	}

	WIDGET->setNumRows(WIDGET->numRows() + length);
	
END_METHOD

/***************************************************************************

	GridViewCols

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
		if (VPROP(GB_INTEGER) != WIDGET->numCols())
			WIDGET->setNumCols(VPROP(GB_INTEGER));
	}

END_PROPERTY


BEGIN_PROPERTY(CGRIDCOLS_width)

	int col = THIS->col;

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
		
		WIDGET->updateLastColumn();
	}

END_PROPERTY


BEGIN_PROPERTY(CGRIDCOLS_height)

	if (WIDGET->horizontalHeader()->isHidden())
		GB.ReturnInteger(0);
	else
		GB.ReturnInteger(WIDGET->horizontalHeader()->height());

END_PROPERTY


BEGIN_PROPERTY(CGRIDCOLS_resizable)

	int col = THIS->col;

	if (READ_PROPERTY)
		GB.ReturnBoolean(WIDGET->horizontalHeader()->isResizeEnabled(col));
	else
		WIDGET->horizontalHeader()->setResizeEnabled(VPROP(GB_BOOLEAN), col);

END_PROPERTY


// BEGIN_PROPERTY(CGRIDCOLS_moveable)
// 
//   if (READ_PROPERTY)
//     GB.ReturnBoolean(WIDGET->horizontalHeader()->isMovingEnabled());
//   else
//     WIDGET->horizontalHeader()->setMovingEnabled(VPROP(GB_BOOLEAN));
// 
// END_PROPERTY


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
	
END_METHOD


BEGIN_METHOD_VOID(CGRIDVIEW_clear)

	WIDGET->clear();

END_METHOD


static void set_current_cell(CGRIDVIEW *_object, int row, int col)
{
	WIDGET->clearSelection();
	WIDGET->setCurrentCell(row, col);
}

BEGIN_PROPERTY(CGRIDVIEW_row)

	int row, col;
	
	WIDGET->getCurrentCell(&row, &col);

	if (READ_PROPERTY)
		GB.ReturnInteger(row);
	else
		set_current_cell(THIS, VPROP(GB_INTEGER), col);

END_PROPERTY


BEGIN_PROPERTY(CGRIDVIEW_column)

	int row, col;
	
	WIDGET->getCurrentCell(&row, &col);

	if (READ_PROPERTY)
		GB.ReturnInteger(col);
	else
		set_current_cell(THIS, row, VPROP(GB_INTEGER));

END_PROPERTY


BEGIN_METHOD(CGRIDVIEW_move_to, GB_INTEGER row; GB_INTEGER col)

	int row = VARG(row);
	int col = VARG(col);

	//if (CGridView::check(WIDGET, row, col))
	//  return;

	set_current_cell(THIS, row, col);

END_METHOD



#if 0
BEGIN_METHOD_VOID(CGRIDVIEW_clear)

	int rows = WIDGET->numRows();
	WIDGET->setNumRows(0);
	WIDGET->setNumRows(rows);

END_METHOD
#endif


BEGIN_METHOD(CGRIDVIEW_get, GB_INTEGER row; GB_INTEGER col)

	int row = VARG(row);
	int col = VARG(col);

	if (CGridView::check(WIDGET, row, col))
		return;

	THIS->row = row;
	THIS->col = col;

	RETURN_SELF();

END_METHOD


BEGIN_PROPERTY(CGRIDVIEW_current)

	WIDGET->getCurrentCell(&THIS->row, &THIS->col);

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
		GB.ReturnBoolean(WIDGET->hasGrid());
	else
		WIDGET->setGrid(VPROP(GB_BOOLEAN));

END_PROPERTY


BEGIN_PROPERTY(CGRIDVIEW_scrollbars)

	int scroll;

	if (READ_PROPERTY)
	{
		scroll = 0;
		if (WIDGET->hScrollBarMode() == Q3ScrollView::Auto)
			scroll += 1;
		if (WIDGET->vScrollBarMode() == Q3ScrollView::Auto)
			scroll += 2;

		GB.ReturnInteger(scroll);
	}
	else
	{
		scroll = VPROP(GB_INTEGER) & 3;
		WIDGET->setHScrollBarMode( (scroll & 1) ? Q3ScrollView::Auto : Q3ScrollView::AlwaysOff);
		WIDGET->setVScrollBarMode( (scroll & 2) ? Q3ScrollView::Auto : Q3ScrollView::AlwaysOff);
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
			case Q3Table::NoSelection: GB.ReturnInteger(0); break;
			case Q3Table::SingleRow: GB.ReturnInteger(1); break;
			case Q3Table::MultiRow: GB.ReturnInteger(2); break;
			default: GB.ReturnInteger(0); break;
		}
	}
	else
	{
		int mode = VPROP(GB_INTEGER);
		
		if (mode == WIDGET->selectionMode())
			return;
		
		switch(mode)
		{
			case 0: WIDGET->setSelectionMode(Q3Table::NoSelection); break;
			case 1: WIDGET->setSelectionMode(Q3Table::SingleRow); break;
			case 2: WIDGET->setSelectionMode(Q3Table::MultiRow); break;
			//case 3: WIDGET->setSelectionMode(QTable::Single); break;
			//case 4: WIDGET->setSelectionMode(QTable::Multi); break;
		}
	}

END_PROPERTY


BEGIN_PROPERTY(CGRIDVIEW_client_x)

	WIDGET->updateScrollBars();
	GB.ReturnInteger(WIDGET->contentsRect().x());

END_PROPERTY


BEGIN_PROPERTY(CGRIDVIEW_client_y)

	WIDGET->updateScrollBars();
	GB.ReturnInteger(WIDGET->contentsRect().y());

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

static int row_at(CGRIDVIEW *_object, int y)
{
	y += WIDGET->contentsY();

	if (!WIDGET->horizontalHeader()->isHidden())
		y -= WIDGET->horizontalHeader()->height();

	return WIDGET->rowAt(y);
}

static int column_at(CGRIDVIEW *_object, int x)
{
	x += WIDGET->contentsX();

	if (!WIDGET->verticalHeader()->isHidden())
		x -= WIDGET->verticalHeader()->width();

	return WIDGET->columnAt(x);
}

BEGIN_METHOD(CGRIDVIEW_row_at, GB_INTEGER ypos)

	GB.ReturnInteger(row_at(THIS, VARG(ypos)));

END_PROPERTY

BEGIN_METHOD(CGRIDVIEW_column_at, GB_INTEGER xpos)

	GB.ReturnInteger(column_at(THIS, VARG(xpos)));

END_PROPERTY

BEGIN_METHOD(CGRIDVIEW_find, GB_INTEGER x; GB_INTEGER y)

	int px, py;

	px = row_at(THIS, VARG(y));
	py = column_at(THIS, VARG(x));
	
	if (px < 0 || py < 0)
	{
		GB.ReturnBoolean(TRUE);
		return;
	}
	
	THIS->row = px;
	THIS->col = py;

	GB.ReturnBoolean(FALSE);

END_METHOD


BEGIN_PROPERTY(CGRIDVIEW_autoresize)

	if (READ_PROPERTY)
		GB.ReturnBoolean(WIDGET->isAutoResize());
	else
		WIDGET->setAutoResize(VPROP(GB_BOOLEAN));

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
	GB_PROPERTY("RichText", "s", CGRIDVIEW_data_rich_text),
	GB_PROPERTY("Picture", "Picture", CGRIDVIEW_data_picture),
	GB_PROPERTY("Font", "Font", CGRIDVIEW_data_font),
	GB_PROPERTY("Alignment", "i", CGRIDVIEW_data_alignment),
	GB_PROPERTY("Background", "i", CGRIDVIEW_data_background),
	GB_PROPERTY("BackColor", "i", CGRIDVIEW_data_background),
	GB_PROPERTY("Foreground", "i", CGRIDVIEW_data_foreground),
	GB_PROPERTY("ForeColor", "i", CGRIDVIEW_data_foreground),
	GB_PROPERTY("Padding", "i", CGRIDVIEW_data_padding),

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
	GB_PROPERTY("RichText", "s", CGRIDITEM_rich_text),
	GB_PROPERTY("Alignment", "i", CGRIDITEM_alignment),
	GB_PROPERTY("Background", "i", CGRIDITEM_background),
	GB_PROPERTY("BackColor", "i", CGRIDITEM_background),
	GB_PROPERTY("Foreground", "i", CGRIDITEM_foreground),
	GB_PROPERTY("ForeColor", "i", CGRIDITEM_foreground),
	GB_PROPERTY("Padding", "i", CGRIDITEM_padding),

	GB_PROPERTY_READ("X", "i", CGRIDITEM_x),
	GB_PROPERTY_READ("Y", "i", CGRIDITEM_y),
	GB_PROPERTY_READ("Left", "i", CGRIDITEM_x),
	GB_PROPERTY_READ("Top", "i", CGRIDITEM_y),
	GB_PROPERTY_READ("Width", "i", CGRIDITEM_width),
	GB_PROPERTY_READ("Height", "i", CGRIDITEM_height),
	GB_PROPERTY_READ("W", "i", CGRIDITEM_width),
	GB_PROPERTY_READ("H", "i", CGRIDITEM_height),

  GB_PROPERTY("RowSpan", "i", CGRIDITEM_row_span),
  GB_PROPERTY("ColumnSpan", "i", CGRIDITEM_column_span),

	GB_METHOD("Clear", NULL, CGRIDITEM_clear, NULL),
	GB_METHOD("EnsureVisible", NULL, CGRIDITEM_ensure_visible, NULL),
	GB_METHOD("Refresh", NULL, CGRIDITEM_refresh, NULL),

	GB_END_DECLARE
};


GB_DESC CGridRowDesc[] =
{
	GB_DECLARE(".GridViewRow", 0), GB_VIRTUAL_CLASS(),

	GB_PROPERTY_READ("Y", "i", CGRIDITEM_y),
	GB_PROPERTY_READ("Top", "i", CGRIDITEM_y),
	GB_PROPERTY("Height", "i", CGRIDROWS_height),
	GB_PROPERTY("H", "i", CGRIDROWS_height),
	GB_PROPERTY("Text", "s", CGRIDROWS_text),
	GB_PROPERTY("Title", "s", CGRIDROWS_text),
	GB_PROPERTY("Selected", "b", CGRIDROWS_selected),
	GB_METHOD("Refresh", NULL, CGRIDROWS_refresh, NULL),
	GB_PROPERTY("Resizable", "b", CGRIDROWS_resizable),

	GB_END_DECLARE
};


GB_DESC CGridColumnDesc[] =
{
	GB_DECLARE(".GridViewColumn", 0), GB_VIRTUAL_CLASS(),

	GB_PROPERTY_READ("X", "i", CGRIDITEM_x),
	GB_PROPERTY_READ("Left", "i", CGRIDITEM_x),
	GB_PROPERTY("Width", "i", CGRIDCOLS_width),
	GB_PROPERTY("W", "i", CGRIDCOLS_width),
	GB_PROPERTY("Text", "s", CGRIDCOLS_text),
	GB_PROPERTY("Title", "s", CGRIDCOLS_text),
	GB_METHOD("Refresh", NULL, CGRIDCOLS_refresh, NULL),
	GB_PROPERTY("Resizable", "b", CGRIDCOLS_resizable),

	GB_END_DECLARE
};


GB_DESC CGridRowsDesc[] =
{
	GB_DECLARE(".GridViewRows", 0), GB_VIRTUAL_CLASS(),

	GB_METHOD("_get", ".GridViewRow", CGRIDROWS_get, "(Row)i"),
	GB_PROPERTY("Count", "i", CGRIDROWS_count),
	GB_PROPERTY("Height", "i", CGRIDROWS_height),
	GB_PROPERTY_READ("HeaderHeight", "i", CGRIDCOLS_height),
	GB_PROPERTY("H", "i", CGRIDROWS_height),
	GB_PROPERTY_READ("Width", "i", CGRIDROWS_width),
	GB_PROPERTY_READ("W", "i", CGRIDROWS_width),
	GB_PROPERTY("Resizable", "b", CGRIDROWS_resizable),
	//GB_PROPERTY("Moveable", "b", CGRIDROWS_moveable),
	GB_METHOD("Select", NULL, CGRIDROWS_select, "[(Start)i(Length)i]"),
	GB_METHOD("SelectAll", NULL, CGRIDROWS_select_all, "[(Selected)b]"),
	GB_METHOD("Unselect", NULL, CGRIDROWS_unselect, NULL),
	GB_METHOD("Remove", NULL, CGRIDROWS_remove, "(Start)i[(Length)i]"),
	GB_METHOD("Insert", NULL, CGRIDROWS_insert, "(Start)i[(Length)i]"),
	GB_END_DECLARE
};


GB_DESC CGridColumnsDesc[] =
{
	GB_DECLARE(".GridViewColumns", 0), GB_VIRTUAL_CLASS(),

	GB_METHOD("_get", ".GridViewColumn", CGRIDCOLS_get, "(Column)i"),
	GB_PROPERTY("Count", "i", CGRIDCOLS_count),
	GB_PROPERTY("Width", "i", CGRIDCOLS_width),
	GB_PROPERTY_READ("HeaderWidth", "i", CGRIDROWS_width),
	GB_PROPERTY("W", "i", CGRIDCOLS_width),
	GB_PROPERTY_READ("Height", "i", CGRIDCOLS_height),
	GB_PROPERTY_READ("H", "i", CGRIDCOLS_height),
	GB_PROPERTY("Resizable", "b", CGRIDCOLS_resizable),
	//GB_PROPERTY("Moveable", "b", CGRIDCOLS_moveable),
	//GB_METHOD("Remove", NULL, CGRIDCOLS_remove, "(Start)i[(Length)i]"),
	//GB_METHOD("Insert", NULL, CGRIDCOLS_insert, "(Start)i[(Length)i]"),

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
	GB_METHOD("Find", "b", CGRIDVIEW_find, "(X)i(Y)i"),

	GB_PROPERTY("ScrollX", "i", CGRIDVIEW_scroll_x),
	GB_PROPERTY("ScrollY", "i", CGRIDVIEW_scroll_y),
	GB_PROPERTY_READ("ClientX", "i", CGRIDVIEW_client_x),
	GB_PROPERTY_READ("ClientY", "i", CGRIDVIEW_client_y),
	GB_PROPERTY_READ("ClientWidth", "i", CGRIDVIEW_client_width),
	GB_PROPERTY_READ("ClientW", "i", CGRIDVIEW_client_width),
	GB_PROPERTY_READ("ClientHeight", "i", CGRIDVIEW_client_height),
	GB_PROPERTY_READ("ClientH", "i", CGRIDVIEW_client_height),

	GB_PROPERTY_SELF("Data", ".GridViewData"),

	GB_METHOD("Refresh", NULL, CGRIDVIEW_refresh, "[(X)i(Y)i(Width)i(Height)i]"),

	GRIDVIEW_DESCRIPTION,

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

bool CGridView::checkRow(Q3Table *table, int row)
{
	if (row < 0 || row >= table->numRows())
	{
		GB.Error("Bad row index");
		return true;
	}
	else
		return false;
}

bool CGridView::checkCol(Q3Table *table, int col)
{
	if (col < 0 || col >= table->numCols())
	{
		GB.Error("Bad column index");
		return true;
	}
	else
		return false;
}

bool CGridView::check(Q3Table *table, int row, int col)
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
	int row, col;
	
	GET_SENDER();
	MyTable *w = (MyTable *)sender();

	w->updateCurrentCell();
	w->getCurrentCell(&row, &col);

	if (row < 0 || col < 0)
		return;

	GB.Raise(THIS, EVENT_Change, 0);
}

void CGridView::activated(void)
{
	GET_SENDER();
	GB.Raise(THIS, EVENT_Activate, 0);
}

void CGridView::selected(void)
{
	GET_SENDER();
	if (WIDGET->selectionMode() == Q3Table::SingleRow)
		GB.Raise(THIS, EVENT_Select, 0);
	else
	{
		//QRect r(WIDGET->contentsToViewport(QPoint(WIDGET->contentsX(), WIDGET->contentsY())), QSize(WIDGET->contentsWidth(), WIDGET->contentsHeight()));
		WIDGET->viewport()->update();
		GB.RaiseLater(THIS, EVENT_Select);
	}
}

void CGridView::clicked(void)
{
	GET_SENDER();
	GB.Raise(THIS, EVENT_Click, 0);
}

static void send_scroll(void *param)
{
	GB.Raise(param, EVENT_Scroll, 0);
	GB.Unref(&param);
}

void CGridView::scrolled(void)
{
	GET_SENDER();

	GB.Ref(THIS);
	GB.Post((void (*)())send_scroll, (intptr_t)THIS);
}

void CGridView::columnClicked(int col)
{
	GET_SENDER();
	GB.Raise(THIS, EVENT_ColumnClick, 1, GB_T_INTEGER, col);
}

void CGridView::rowClicked(int row)
{
	GET_SENDER();
	GB.Raise(THIS, EVENT_RowClick, 1, GB_T_INTEGER, row);
}

void CGridView::columnResized(int col)
{
	GET_SENDER();
	GB.Raise(THIS, EVENT_ColumnResize, 1, GB_T_INTEGER, col);
}

void CGridView::rowResized(int row)
{
	GET_SENDER();
	GB.Raise(THIS, EVENT_RowResize, 1, GB_T_INTEGER, row);
}

