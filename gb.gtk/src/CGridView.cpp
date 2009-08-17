/***************************************************************************

  CGridView.cpp

  (c) 2004-2006 - Daniel Campos Fernández <dcamposf@gmail.com>

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

#include "CWidget.h"
#include "CContainer.h"
#include "CGridView.h"
#include "CPicture.h"
#include "CFont.h"

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
DECLARE_EVENT(EVENT_FooterClick);

static void raise_event(gGridView *sender, int event)
{
	CWIDGET *_object = GetObject(sender);
	GB.Raise(THIS, event, 0);
}

static void raise_event_int(gGridView *sender, int event, int arg)
{
	CWIDGET *_object = GetObject(sender);
	GB.Raise(THIS, event, 1, GB_T_INTEGER, arg);
}

static void raise_col_click(gGridView *sender, int col)
{
	raise_event_int(sender, EVENT_ColumnClick, col);
}

static void raise_row_click(gGridView *sender,int row)
{
	raise_event_int(sender, EVENT_RowClick, row);
}

static void raise_col_resize(gGridView *sender, int col)
{
	raise_event_int(sender, EVENT_ColumnResize, col);
}

static void raise_row_resize(gGridView *sender,int row)
{
	raise_event_int(sender, EVENT_RowResize, row);
}

static void raise_foot_click(gGridView *sender, int col)
{
	raise_event_int(sender, EVENT_FooterClick, col);
}

static void raise_click(gGridView *sender, int row, int col)
{
	raise_event(sender, EVENT_Click);
}

static void raise_activate(gGridView *sender, int row, int col)
{
	raise_event(sender, EVENT_Activate);
}

static void raise_change(gGridView *sender)
{
	raise_event(sender, EVENT_Change);
}

static void raise_select(gGridView *sender)
{
	raise_event(sender, EVENT_Select);
}

static void raise_scroll(gGridView *sender)
{
	raise_event(sender, EVENT_Scroll);
}

static void raise_data(gTableData *fill,int row, int col,void *_object)
{
	if (GB.CanRaise(THIS,EVENT_Data))
	{
		THIS->data = fill;
		GB.Raise(_object,EVENT_Data,2,GB_T_INTEGER,row,GB_T_INTEGER,col);
		THIS->data=NULL;
	}
}

BEGIN_PROPERTY(CGRIDVIEW_data)

	if (!THIS->data)
	{
		GB.Error("No data event");
		GB.ReturnNull();
		return;
	}

	RETURN_SELF();

END_PROPERTY

BEGIN_PROPERTY(CGRIDVIEWDATA_picture)

	if (READ_PROPERTY)
		GB.ReturnObject(THIS->data->picture ? THIS->data->picture->getTagValue() : 0);
	else
	{
		CPICTURE *pict = (CPICTURE *)VPROP(GB_OBJECT);
		THIS->data->setPicture(pict ? pict->picture : 0);
	}

END_PROPERTY

BEGIN_PROPERTY(CGRIDVIEWDATA_alignment)

	if (READ_PROPERTY)
		GB.ReturnInteger(THIS->data->alignment);
	else
		THIS->data->alignment = VPROP(GB_INTEGER);

END_PROPERTY

BEGIN_PROPERTY(CGRIDVIEWDATA_font)

	CFONT *font;

	if (READ_PROPERTY)
		GB.ReturnObject(CFONT_create(THIS->data->font));
	else
	{
		font = (CFONT *)VPROP(GB_OBJECT);	
		THIS->data->setFont(font ? font->font : 0);
	}

END_PROPERTY

BEGIN_PROPERTY(CGRIDVIEWDATA_text)

	if (READ_PROPERTY)
		GB.ReturnNewString(THIS->data->text,0);
	else
		THIS->data->setText(GB.ToZeroString(PROP(GB_STRING)));

END_PROPERTY

BEGIN_PROPERTY(CGRIDVIEWDATA_rich_text)

	if (READ_PROPERTY)
		GB.ReturnNewString(THIS->data->richText,0);
	else
		THIS->data->setRichText(GB.ToZeroString(PROP(GB_STRING)));

END_PROPERTY

BEGIN_PROPERTY(CGRIDVIEWDATA_bg)

	if (READ_PROPERTY)
	{
		GB.ReturnInteger(THIS->data->bg);
		return;
	}

	THIS->data->bg=VPROP(GB_INTEGER);

END_PROPERTY

BEGIN_PROPERTY(CGRIDVIEWDATA_fg)

	if (READ_PROPERTY)
	{
		GB.ReturnInteger(THIS->data->fg);
		return;
	}

	THIS->data->fg=VPROP(GB_INTEGER);

END_PROPERTY


BEGIN_PROPERTY(CGRIDVIEWDATA_padding)

	if (READ_PROPERTY)
		GB.ReturnInteger(THIS->data->padding);
	else
	{
		THIS->data->padding = VPROP(GB_INTEGER);
		if (THIS->data->padding < 0) 
			THIS->data->padding = 0;
	}

END_PROPERTY


BEGIN_PROPERTY(CGRIDVIEWITEM_picture)

	if (READ_PROPERTY)
	{
		gPicture *pic = WIDGET->itemPicture(THIS->row, THIS->col);
		GB.ReturnObject(pic ? pic->getTagValue() : 0);
	}
	else
	{
		CPICTURE *pict = (CPICTURE *)VPROP(GB_OBJECT);
		WIDGET->setItemPicture(THIS->row, THIS->col, pict ? pict->picture : 0);
	}

END_PROPERTY

BEGIN_PROPERTY(CGRIDVIEWITEM_font)

	if (READ_PROPERTY)
	{
		gFont *f = WIDGET->itemFont(THIS->row, THIS->col);
		if (!f)
		{
			CFONT *font = CFONT_create(WIDGET->font()->copy());
			WIDGET->setItemFont(THIS->row, THIS->col, font->font);
			f = WIDGET->itemFont(THIS->row, THIS->col);
		}
		
		GB.ReturnObject(f ? f->getTagValue() : 0);
	}
	else
	{
		CFONT *font = (CFONT *)VPROP(GB_OBJECT);
		//fprintf(stderr, "%s\n", font ? font->font->toString() : 0);
		WIDGET->setItemFont(THIS->row, THIS->col, font ? font->font : 0);
	}

END_PROPERTY

BEGIN_PROPERTY(CGRIDVIEWITEM_alignment)

	if (READ_PROPERTY) 
		GB.ReturnInteger(WIDGET->itemAlignment(THIS->row,THIS->col));
	else
		WIDGET->setItemAlignment( THIS->row, THIS->col,VPROP(GB_INTEGER) );

END_PROPERTY

/*BEGIN_PROPERTY(CGRIDVIEWITEM_key)

	GB.ReturnInteger(THIS->row);

END_PROPERTY

BEGIN_PROPERTY(CGRIDVIEWITEM_column)

	GB.ReturnInteger(THIS->col);

END_PROPERTY*/

BEGIN_PROPERTY(CGRIDVIEWITEM_x)

	GB.ReturnInteger(WIDGET->itemX(THIS->col));

END_PROPERTY

BEGIN_PROPERTY(CGRIDVIEWITEM_y)

	GB.ReturnInteger(WIDGET->itemY(THIS->row));

END_PROPERTY

BEGIN_PROPERTY(CGRIDVIEWITEM_w)

	GB.ReturnInteger(WIDGET->itemW(THIS->col));

END_PROPERTY

BEGIN_PROPERTY(CGRIDVIEWITEM_h)

	GB.ReturnInteger(WIDGET->itemH(THIS->row));

END_PROPERTY

BEGIN_PROPERTY(CGRIDVIEWITEM_text)

	if (READ_PROPERTY) 
		GB.ReturnNewZeroString(WIDGET->itemText(THIS->row,THIS->col)); 
	else
		WIDGET->setItemText(THIS->row, THIS->col, GB.ToZeroString(PROP(GB_STRING)));

END_PROPERTY

BEGIN_PROPERTY(CGRIDVIEWITEM_rich_text)

	if (READ_PROPERTY) 
		GB.ReturnNewZeroString(WIDGET->itemRichText(THIS->row,THIS->col)); 
	else
		WIDGET->setItemRichText(THIS->row, THIS->col, GB.ToZeroString(PROP(GB_STRING)));

END_PROPERTY

BEGIN_PROPERTY(CGRIDVIEWITEM_bg)

	if (READ_PROPERTY) 
	{ 
		GB.ReturnInteger(WIDGET->itemBg(THIS->row,THIS->col)); 
		return; 
	}

	WIDGET->setItemBg( THIS->row, THIS->col,VPROP(GB_INTEGER) );

END_PROPERTY

BEGIN_PROPERTY(CGRIDVIEWITEM_fg)

	if (READ_PROPERTY) 
	{ 
		GB.ReturnInteger(WIDGET->itemFg(THIS->row,THIS->col)); 
		return; 
	}

	WIDGET->setItemFg( THIS->row, THIS->col,VPROP(GB_INTEGER) );

END_PROPERTY

BEGIN_PROPERTY(CGRIDVIEWITEM_padding)

	if (READ_PROPERTY) 
		GB.ReturnInteger(WIDGET->itemPadding(THIS->row,THIS->col));
	else
		WIDGET->setItemPadding( THIS->row, THIS->col,VPROP(GB_INTEGER) );

END_PROPERTY

/*BEGIN_PROPERTY(CGRIDVIEWITEM_selected)

	if (READ_PROPERTY) 
	{ 
		GB.ReturnBoolean(WIDGET->itemSelected(THIS->row,THIS->col)); 
		return; 
	}

	WIDGET->setItemSelected( THIS->row, THIS->col,VPROP(GB_BOOLEAN) );

END_PROPERTY*/

BEGIN_METHOD_VOID(CGRIDVIEWITEM_clear)

	WIDGET->clearItem(THIS->row, THIS->col);

END_METHOD

BEGIN_METHOD_VOID(CGRIDVIEWITEM_refresh)

	WIDGET->queryUpdate(THIS->row, THIS->col);

END_METHOD

BEGIN_METHOD_VOID(CGRIDVIEWITEM_ensure_visible)

	WIDGET->ensureVisible(THIS->row, THIS->col);

END_METHOD

BEGIN_PROPERTY(CGRIDVIEWITEM_row_span)

	int rowspan, colspan;
	
	WIDGET->getItemSpan(THIS->row, THIS->col, &rowspan, &colspan);

	if (READ_PROPERTY)
	{
		if (rowspan >= 0)
			GB.ReturnInteger(rowspan + 1);
		else
			GB.ReturnInteger(rowspan);
	}
	else
	{
		WIDGET->setItemSpan(THIS->row, THIS->col, VPROP(GB_INTEGER) - 1, colspan);
	}

END_PROPERTY

BEGIN_PROPERTY(CGRIDVIEWITEM_column_span)

	int rowspan, colspan;
	
	WIDGET->getItemSpan(THIS->row, THIS->col, &rowspan, &colspan);

	if (READ_PROPERTY)
	{
		if (colspan >= 0)
			GB.ReturnInteger(colspan + 1);
		else
			GB.ReturnInteger(colspan);
	}
	else
	{
		WIDGET->setItemSpan(THIS->row, THIS->col, rowspan, VPROP(GB_INTEGER) - 1);
	}

END_PROPERTY

/*************************************************

 GridViewColumns
 
**************************************************/

BEGIN_METHOD(CGRIDVIEW_columns_get,GB_INTEGER Column;)

	if ( (VARG(Column)<0) || (VARG(Column)>=WIDGET->columnCount() ) )
	{
		GB.Error("Bad column index");
 		GB.ReturnNull();
		return;
	}

	THIS->col=VARG(Column);
	RETURN_SELF();

END_METHOD

BEGIN_PROPERTY(CGRIDVIEW_columns_resizable)

	int bc;

	if (READ_PROPERTY) 
	{ 
		if (!WIDGET->columnCount())
			GB.ReturnBoolean(true);
		else
			GB.ReturnBoolean(WIDGET->columnResizable(0)); 
		return; 
	}

	for (bc=0; bc<WIDGET->columnCount(); bc++)
		WIDGET->setColumnResizable(bc,VPROP(GB_BOOLEAN));

END_PROPERTY

BEGIN_PROPERTY(CGRIDVIEW_columns_count)

	if (READ_PROPERTY) { GB.ReturnInteger(WIDGET->columnCount()); return; }
	WIDGET->setColumnCount(VPROP(GB_INTEGER));

	if ( (WIDGET->rowCount()==0) || (WIDGET->columnCount()==0) )
	{
		THIS->row=-1;
		THIS->col=-1;
		return;
	}

	if (WIDGET->columnCount()>=THIS->col) THIS->col=WIDGET->columnCount()-1;

END_PROPERTY

BEGIN_PROPERTY(CGRIDVIEW_column_width)

	if (READ_PROPERTY) { GB.ReturnInteger(WIDGET->columnWidth(THIS->col)); return; }	
	WIDGET->setColumnWidth(THIS->col,VPROP(GB_INTEGER));

END_PROPERTY

BEGIN_PROPERTY(CGRIDCOLS_width)

	int bc;

	if (READ_PROPERTY) { GB.ReturnInteger(WIDGET->columnWidth(0)); return; }	
	for (bc=0;bc<WIDGET->columnCount(); bc++)
		WIDGET->setColumnWidth(bc,VPROP(GB_INTEGER));

END_PROPERTY

BEGIN_PROPERTY(CGRIDVIEW_column_resizable)

	if (READ_PROPERTY) 
	{ 
		GB.ReturnBoolean(WIDGET->columnResizable(THIS->col)); 
		return;

	}

	WIDGET->setColumnResizable(THIS->col,VPROP(GB_BOOLEAN));

END_PROPERTY

BEGIN_PROPERTY(CGRIDCOLS_height)

	GB.ReturnInteger(WIDGET->headerHeight());

END_PROPERTY

BEGIN_METHOD_VOID(CGRIDCOL_refresh)

	WIDGET->queryUpdate(-1, THIS->col);

END_METHOD


/*************************************************

 GridViewRows
 
**************************************************/

BEGIN_PROPERTY(CGRIDVIEW_count)

	if (READ_PROPERTY) { GB.ReturnInteger(WIDGET->rowCount()); return; }
	WIDGET->setRowCount(VPROP(GB_INTEGER));
	
	if ( (WIDGET->rowCount()==0) || (WIDGET->columnCount()==0) )
	{
		THIS->row=-1;
		THIS->col=-1;
		return;
	}

	if (WIDGET->rowCount()>=THIS->row) THIS->row=WIDGET->rowCount()-1;

END_PROPERTY

BEGIN_METHOD(CGRIDVIEW_rows_get,GB_INTEGER Row;)

	if ( (VARG(Row)<0) || (VARG(Row)>=WIDGET->rowCount() ) )
	{
		GB.Error("Bad row index");
		GB.ReturnNull();
		return;
	}

	THIS->row=VARG(Row);
	RETURN_SELF();

END_METHOD

BEGIN_PROPERTY(CGRIDROWS_height)

	int bc;

	if (READ_PROPERTY) { GB.ReturnInteger(WIDGET->rowHeight(0)); return; }	
	for (bc=0;bc<WIDGET->rowCount(); bc++)
		WIDGET->setRowHeight(bc,VPROP(GB_INTEGER));

END_PROPERTY

BEGIN_PROPERTY(CGRIDROWS_width)

	GB.ReturnInteger(WIDGET->rowWidth());

END_PROPERTY

BEGIN_METHOD(CGRIDROWS_select, GB_INTEGER start; GB_INTEGER length)

	int start, length;

	start = VARGOPT(start, 0);
	if (start < 0)
		start = 0;

	length = VARGOPT(length, WIDGET->rowCount() - start);
	if (length < 0)
		length = 0;

	WIDGET->clearSelection();
	WIDGET->selectRows(start, length);

END_PROPERTY


BEGIN_METHOD(CGRIDROWS_select_all, GB_BOOLEAN sel)

	WIDGET->clearSelection();
	if (VARGOPT(sel, TRUE))
		WIDGET->selectRows(0, WIDGET->rowCount());

END_PROPERTY

BEGIN_METHOD_VOID(CGRIDROWS_unselect)

	WIDGET->clearSelection();

END_METHOD

BEGIN_METHOD(CGRIDROWS_remove, GB_INTEGER start; GB_INTEGER length)

	int start = VARG(start);
	int length = VARGOPT(length, 1);

	if (start < 0 || start >= WIDGET->rowCount() || length <= 0 || (start + length) > WIDGET->rowCount())
	{
		GB.Error(GB_ERR_ARG);
		return;
	}
	
	WIDGET->removeRows(start, length);
	
END_METHOD


BEGIN_METHOD(CGRIDROWS_insert, GB_INTEGER start; GB_INTEGER length)

	int start = VARG(start);
	int length = VARGOPT(length, 1);

	if (start < 0 || length <= 0 || start > WIDGET->rowCount())
	{
		GB.Error(GB_ERR_ARG);
		return;
	}
	
	WIDGET->insertRows(start, length);

END_METHOD

BEGIN_PROPERTY(CGRIDVIEW_rows_resizable)

	int bc;

	if (READ_PROPERTY) { GB.ReturnBoolean(WIDGET->rowResizable(0)); return; }	
	for (bc=0;bc<WIDGET->rowCount(); bc++)
		WIDGET->setRowResizable(bc,VPROP(GB_BOOLEAN));

END_PROPERTY

BEGIN_PROPERTY(CGRIDVIEW_row_height)

	if (READ_PROPERTY) { GB.ReturnInteger(WIDGET->rowHeight(THIS->row)); return; }	
	WIDGET->setRowHeight(THIS->row,VPROP(GB_INTEGER));

END_PROPERTY

BEGIN_PROPERTY(CGRIDVIEW_row_resizable)

	if (READ_PROPERTY) { GB.ReturnBoolean(WIDGET->rowResizable(THIS->row)); return; }	
	WIDGET->setRowResizable(THIS->row,VPROP(GB_BOOLEAN));

END_PROPERTY

BEGIN_PROPERTY(CGRIDVIEW_row_selected)

	if (READ_PROPERTY) 
		GB.ReturnBoolean(WIDGET->rowSelected(THIS->row));
	else
		WIDGET->setRowSelected(THIS->row,VPROP(GB_BOOLEAN));

END_PROPERTY

BEGIN_METHOD_VOID(CGRIDROW_refresh)

	WIDGET->queryUpdate(THIS->row, -1);

END_METHOD

/******************************************************

 GridView
 
*******************************************************/

BEGIN_PROPERTY(CGRIDVIEW_scrollbar)

	if (READ_PROPERTY) { GB.ReturnInteger(WIDGET->scrollBar()); return; }
	WIDGET->setScrollBar(VPROP(GB_INTEGER));

END_PROPERTY

BEGIN_PROPERTY(CGRIDVIEW_scrollX)

	if (READ_PROPERTY) { GB.ReturnInteger(WIDGET->scrollX()); return; }
	WIDGET->setScrollX(VPROP(GB_INTEGER));

END_PROPERTY

BEGIN_PROPERTY(CGRIDVIEW_scrollY)

	if (READ_PROPERTY) { GB.ReturnInteger(WIDGET->scrollY()); return; }
	WIDGET->setScrollY(VPROP(GB_INTEGER));

END_PROPERTY

BEGIN_PROPERTY(CGRIDVIEW_grid)

	if (READ_PROPERTY) { GB.ReturnBoolean(WIDGET->drawGrid()); return; }
	WIDGET->setDrawGrid(VPROP(GB_BOOLEAN));

END_PROPERTY

BEGIN_PROPERTY(CGRIDVIEW_client_x)

	GB.ReturnInteger(WIDGET->visibleLeft());

END_PROPERTY

BEGIN_PROPERTY(CGRIDVIEW_client_y)

	GB.ReturnInteger(WIDGET->visibleTop());

END_PROPERTY

BEGIN_PROPERTY(CGRIDVIEW_client_width)

	GB.ReturnInteger(WIDGET->visibleWidth());

END_PROPERTY

BEGIN_PROPERTY(CGRIDVIEW_client_height)

	GB.ReturnInteger(WIDGET->visibleHeight());

END_PROPERTY

BEGIN_PROPERTY(CGRIDVIEW_current)
	
	WIDGET->getCursor(&THIS->row, &THIS->col);
	
	if (THIS->row < 0 || THIS->col < 0)
		GB.ReturnNull(); 
	else
		RETURN_SELF();
	
END_PROPERTY

BEGIN_METHOD(CGRIDVIEW_new, GB_OBJECT parent)

	InitControl(new gGridView(CONTAINER(VARG(parent))),(CWIDGET*)THIS);

	THIS->row=-1;
	THIS->col=-1;

	WIDGET->setDataFunc((void*)raise_data, _object);
	WIDGET->onActivate = raise_activate;
	WIDGET->onClick = raise_click;
	WIDGET->onChange = raise_change;
	WIDGET->onSelect = raise_select;
	WIDGET->onRowClick = raise_row_click;
	WIDGET->onColumnClick = raise_col_click;
	WIDGET->onFooterClick = raise_foot_click;
	WIDGET->onScroll = raise_scroll;
	WIDGET->onRowResize = raise_row_resize;
	WIDGET->onColumnResize = raise_col_resize;

END_METHOD

BEGIN_METHOD(CGRIDVIEW_find, GB_INTEGER X; GB_INTEGER Y;)

	int px,py;

	px=WIDGET->rowAt(VARG(Y));
	py=WIDGET->columnAt(VARG(X));
	
	if ( (px==-1) || (py==-1) ) { GB.ReturnBoolean(true); return; }

	THIS->row=px;
	THIS->col=py;

	GB.ReturnBoolean(false);

END_METHOD

BEGIN_METHOD(CGRIDVIEW_rowat, GB_INTEGER X;)

	GB.ReturnInteger(WIDGET->rowAt(VARG(X)));

END_METHOD

BEGIN_METHOD(CGRIDVIEW_colat, GB_INTEGER X;)

	GB.ReturnInteger(WIDGET->columnAt(VARG(X)));

END_METHOD

BEGIN_METHOD_VOID(CGRIDVIEW_clear)

	int bx,by;

	for (bx=0; bx<WIDGET->rowCount(); bx++)
		for (by=0; by<WIDGET->columnCount(); by++)
			WIDGET->setItemText(by,bx,"");

END_METHOD

BEGIN_METHOD(CGRIDVIEW_get, GB_INTEGER Key; GB_INTEGER Column;)

	if ( (VARG(Key)<0) || (VARG(Key)>=WIDGET->rowCount()) )
	{
		GB.Error("Bad row index");
		GB.ReturnNull();
		return;
	}

	if ( (VARG(Column)<0) || (VARG(Column)>=WIDGET->columnCount()) )
	{
		GB.Error("Bad column index");
		GB.ReturnNull();
		return;
	}

	THIS->row=VARG(Key);
	THIS->col=VARG(Column);
	RETURN_SELF();

END_METHOD


BEGIN_PROPERTY(CGRIDVIEW_row)

	int row, col;
	
	WIDGET->getCursor(&row, &col);

	if (READ_PROPERTY)
		GB.ReturnInteger(row);
	else
		WIDGET->setCursor(VPROP(GB_INTEGER), col);

END_PROPERTY

BEGIN_PROPERTY(CGRIDVIEW_column)

	int row, col;
	
	WIDGET->getCursor(&row, &col);

	if (READ_PROPERTY)
		GB.ReturnInteger(col);
	else
		WIDGET->setCursor(row, VPROP(GB_INTEGER));

END_PROPERTY

BEGIN_METHOD(CGRIDVIEW_move_to, GB_INTEGER row; GB_INTEGER col)

  WIDGET->setCursor(VARG(row), VARG(col));

END_METHOD

BEGIN_PROPERTY(CGRIDVIEW_border)

	if (READ_PROPERTY) { GB.ReturnBoolean(WIDGET->getBorder()); return; }
	WIDGET->setBorder(VPROP(GB_BOOLEAN));

END_PROPERTY

BEGIN_PROPERTY(CGRIDVIEW_mode)

	int mode;

	if (READ_PROPERTY) { GB.ReturnInteger(WIDGET->selectionMode()); return; }

	mode=VPROP(GB_INTEGER);
	if (mode<0) mode=0;
	if (mode>2) mode=2;
	WIDGET->setSelectionMode(mode);

END_PROPERTY

BEGIN_PROPERTY(CGRIDVIEW_autoresize)

  if (READ_PROPERTY)
    GB.ReturnBoolean(WIDGET->isAutoResize());
  else
		WIDGET->setAutoResize(VPROP(GB_BOOLEAN));

END_PROPERTY


/***************************************************************************

Headers, Footers, and Row Separators

****************************************************************************/

BEGIN_PROPERTY(CGRIDVIEWHEADER_visible)

	if (READ_PROPERTY) { GB.ReturnInteger(WIDGET->headersVisible()); return; }
	WIDGET->setHeadersVisible(VPROP(GB_INTEGER));

END_PROPERTY

BEGIN_PROPERTY(CGRIDVIEW_footer)

	if (READ_PROPERTY) { GB.ReturnBoolean(WIDGET->footersVisible()); return; }
	WIDGET->setFootersVisible(VPROP(GB_BOOLEAN));

END_PROPERTY

BEGIN_PROPERTY(CGRIDVIEW_column_headertext)

	if (READ_PROPERTY)
	{
		GB.ReturnNewString(WIDGET->headerText(THIS->col),0);
		return;
	}
	WIDGET->setHeaderText(THIS->col,PROP(GB_STRING)->value.addr);

END_PROPERTY

BEGIN_PROPERTY(CGRIDVIEW_column_footer_text)

	if (READ_PROPERTY)
	{
		GB.ReturnNewString(WIDGET->footerText(THIS->col),0);
		return;
	}
	WIDGET->setFooterText(THIS->col,PROP(GB_STRING)->value.addr);

END_PROPERTY

BEGIN_PROPERTY(CGRIDVIEW_row_text)

	if (READ_PROPERTY)
	{
		GB.ReturnNewString(WIDGET->rowText(THIS->row),0);
		return;
	}
	WIDGET->setRowText(THIS->row,PROP(GB_STRING)->value.addr);

END_PROPERTY

/***************************************************************************

  Gambas Interfaces

***************************************************************************/

GB_DESC CGridViewItemDesc[] =
{
  GB_DECLARE(".GridViewCell", 0), GB_VIRTUAL_CLASS(),
  
  //GB_PROPERTY_READ("Row", "i", CGRIDVIEWITEM_key),
  //GB_PROPERTY_READ("Column", "i", CGRIDVIEWITEM_column),  
  GB_PROPERTY_READ("X", "i", CGRIDVIEWITEM_x),
  GB_PROPERTY_READ("Y", "i", CGRIDVIEWITEM_y),
  GB_PROPERTY_READ("Left", "i", CGRIDVIEWITEM_x),
  GB_PROPERTY_READ("Top", "i", CGRIDVIEWITEM_y),
  GB_PROPERTY_READ("Width", "i", CGRIDVIEWITEM_w),
  GB_PROPERTY_READ("Height", "i", CGRIDVIEWITEM_h),
  GB_PROPERTY_READ("W", "i", CGRIDVIEWITEM_w),
  GB_PROPERTY_READ("H", "i", CGRIDVIEWITEM_h),
  
  GB_PROPERTY("Picture", "Picture", CGRIDVIEWITEM_picture),
  GB_PROPERTY("Text", "s", CGRIDVIEWITEM_text),
  GB_PROPERTY("RichText", "s", CGRIDVIEWITEM_rich_text),
  GB_PROPERTY("Background", "i", CGRIDVIEWITEM_bg),
  GB_PROPERTY("BackColor", "i", CGRIDVIEWITEM_bg),
  GB_PROPERTY("Foreground", "i", CGRIDVIEWITEM_fg),
  GB_PROPERTY("ForeColor", "i", CGRIDVIEWITEM_fg),
  GB_PROPERTY("Padding", "i", CGRIDVIEWITEM_padding),
  GB_PROPERTY("Alignment", "i", CGRIDVIEWITEM_alignment),
  GB_PROPERTY("Font", "Font", CGRIDVIEWITEM_font),
  //GB_PROPERTY("Selected","b",CGRIDVIEWITEM_selected),

  GB_PROPERTY("RowSpan", "i", CGRIDVIEWITEM_row_span),
  GB_PROPERTY("ColumnSpan", "i", CGRIDVIEWITEM_column_span),

	GB_METHOD("Clear", 0, CGRIDVIEWITEM_clear, 0),
  GB_METHOD("Refresh",0,CGRIDVIEWITEM_refresh,0),
  GB_METHOD("EnsureVisible", 0, CGRIDVIEWITEM_ensure_visible, 0),
  GB_END_DECLARE
};

GB_DESC CGridViewDataDesc[] =
{
  GB_DECLARE(".GridViewData", 0), GB_VIRTUAL_CLASS(),

  GB_PROPERTY("Picture", "Picture", CGRIDVIEWDATA_picture),
  GB_PROPERTY("Text", "s", CGRIDVIEWDATA_text),
  GB_PROPERTY("RichText", "s", CGRIDVIEWDATA_rich_text),
  GB_PROPERTY("Background", "i", CGRIDVIEWDATA_bg),
  GB_PROPERTY("BackColor", "i", CGRIDVIEWDATA_bg),
  GB_PROPERTY("Foreground", "i", CGRIDVIEWDATA_fg),
  GB_PROPERTY("ForeColor", "i", CGRIDVIEWDATA_fg),
  GB_PROPERTY("Padding", "i", CGRIDVIEWDATA_padding),
  GB_PROPERTY("Alignment", "i", CGRIDVIEWDATA_alignment),
  GB_PROPERTY("Font", "Font", CGRIDVIEWDATA_font),

  GB_END_DECLARE
};

GB_DESC CGridViewColumnDesc[] =
{
  GB_DECLARE(".GridViewColumn", 0), GB_VIRTUAL_CLASS(),
  GB_PROPERTY("Resizable","b",CGRIDVIEW_column_resizable),
  GB_PROPERTY_READ("X", "i", CGRIDVIEWITEM_x),
  GB_PROPERTY_READ("Left", "i", CGRIDVIEWITEM_x),
  GB_PROPERTY("Width", "i", CGRIDVIEW_column_width),
  GB_PROPERTY("W", "i", CGRIDVIEW_column_width),
  GB_PROPERTY("Text","s",CGRIDVIEW_column_headertext),
  GB_PROPERTY("Title","s",CGRIDVIEW_column_headertext),
  //GB_PROPERTY("HeaderText","s",CGRIDVIEW_column_headertext),
  //GB_PROPERTY("FooterText","s",CGRIDVIEW_column_footertext),
  GB_METHOD("Refresh", 0, CGRIDCOL_refresh, 0),
  GB_END_DECLARE
};

GB_DESC CGridViewColumnsDesc[] =
{
  GB_DECLARE(".GridViewColumns", 0), GB_VIRTUAL_CLASS(),
  GB_METHOD("_get", ".GridViewColumn", CGRIDVIEW_columns_get, "(Column)i"),
  GB_PROPERTY("Resizable","b",CGRIDVIEW_columns_resizable),
  GB_PROPERTY("Count", "i", CGRIDVIEW_columns_count),
  GB_PROPERTY("Width", "i", CGRIDCOLS_width),
  GB_PROPERTY_READ("HeaderWidth", "i", CGRIDROWS_width),
  GB_PROPERTY("W", "i", CGRIDCOLS_width),
  GB_PROPERTY_READ("Height", "i", CGRIDCOLS_height),
  GB_PROPERTY_READ("H", "i", CGRIDCOLS_height),
  GB_END_DECLARE
};

GB_DESC CGridViewRowDesc[] =
{
  GB_DECLARE(".GridViewRow", 0), GB_VIRTUAL_CLASS(),
  GB_PROPERTY_READ("Y", "i", CGRIDVIEWITEM_y),
  GB_PROPERTY_READ("Top", "i", CGRIDVIEWITEM_y),
  GB_PROPERTY("Height", "i", CGRIDVIEW_row_height),
  GB_PROPERTY("H", "i", CGRIDVIEW_row_height),
  GB_PROPERTY("Resizable","b",CGRIDVIEW_row_resizable),
  GB_PROPERTY("Text","s",CGRIDVIEW_row_text),
  GB_PROPERTY("Title","s",CGRIDVIEW_row_text),
  GB_PROPERTY("Selected","b",CGRIDVIEW_row_selected),
  GB_METHOD("Refresh", 0, CGRIDROW_refresh, 0),
  GB_END_DECLARE
};

GB_DESC CGridViewRowsDesc[] =
{
  GB_DECLARE(".GridViewRows", 0), GB_VIRTUAL_CLASS(),
  GB_METHOD("_get", ".GridViewRow", CGRIDVIEW_rows_get, "(Row)i"),
  GB_PROPERTY("Count", "i", CGRIDVIEW_count),
  GB_PROPERTY("Resizable","b",CGRIDVIEW_rows_resizable),
  GB_PROPERTY("Height", "i", CGRIDROWS_height),
  GB_PROPERTY_READ("HeaderHeight", "i", CGRIDCOLS_height),
  GB_PROPERTY("H", "i", CGRIDROWS_height),
  GB_PROPERTY_READ("Width", "i", CGRIDROWS_width),
  GB_PROPERTY_READ("W", "i", CGRIDROWS_width),
  GB_METHOD("Select", 0, CGRIDROWS_select, "[(Start)i(Length)i]"),
  GB_METHOD("SelectAll", 0, CGRIDROWS_select_all, "[(Selected)b]"),
  GB_METHOD("Unselect", 0, CGRIDROWS_unselect, 0),
  GB_METHOD("Remove", 0, CGRIDROWS_remove, "(Start)i[(Length)i]"),
  GB_METHOD("Insert", 0, CGRIDROWS_insert, "(Start)i[(Length)i]"),
  GB_END_DECLARE
};

GB_DESC CGridViewDesc[] =
{
  GB_DECLARE("GridView", sizeof(CGRIDVIEW)), GB_INHERITS("Control"),

  GB_CONSTANT("None", "i", 0),
  GB_CONSTANT("Horizontal", "i", 1),
  GB_CONSTANT("Vertical", "i", 2),
  GB_CONSTANT("Both", "i", 3),

  GB_METHOD("_new", 0, CGRIDVIEW_new, "(Parent)Container;"),
  GB_METHOD("_get", ".GridViewCell", CGRIDVIEW_get, "(Row)i(Column)i"),
  GB_METHOD("Clear", 0, CGRIDVIEW_clear, 0),
  GB_METHOD("Find", "b", CGRIDVIEW_find, "(X)i(Y)i"),
  GB_METHOD("RowAt","i",CGRIDVIEW_rowat,"(Y)i"),
  GB_METHOD("ColumnAt","i",CGRIDVIEW_colat,"(X)i"),

  GB_PROPERTY("Border", "b", CGRIDVIEW_border),
  GB_PROPERTY("Mode","i",CGRIDVIEW_mode),
  GB_PROPERTY("ScrollBar", "i", CGRIDVIEW_scrollbar),
  GB_PROPERTY("Grid", "b", CGRIDVIEW_grid),
  GB_PROPERTY("AutoResize", "b", CGRIDVIEW_autoresize),
  GB_PROPERTY("Resizable","b",CGRIDVIEW_columns_resizable),
  GB_PROPERTY("Header", "i", CGRIDVIEWHEADER_visible),
  //GB_PROPERTY("Footer", "b", CGRIDVIEWFOOTER_visible),
    
  GB_PROPERTY("ScrollX", "i", CGRIDVIEW_scrollX),
  GB_PROPERTY("ScrollY", "i", CGRIDVIEW_scrollY),
  
  GB_PROPERTY("Row", "i", CGRIDVIEW_row),
  GB_PROPERTY("Column", "i", CGRIDVIEW_column),
  GB_METHOD("MoveTo", 0, CGRIDVIEW_move_to, "(Row)i(Column)i"),
  
  GB_PROPERTY_READ("Data", ".GridViewData",CGRIDVIEW_data),
  
  GB_PROPERTY_SELF("Columns", ".GridViewColumns"),
  GB_PROPERTY_SELF("Rows", ".GridViewRows"),
  GB_PROPERTY_READ("Current", ".GridViewCell", CGRIDVIEW_current),
  GB_PROPERTY_READ("ClientX", "i", CGRIDVIEW_client_x),
  GB_PROPERTY_READ("ClientY", "i",  CGRIDVIEW_client_y),
  GB_PROPERTY_READ("ClientWidth", "i", CGRIDVIEW_client_width),
  GB_PROPERTY_READ("ClientW", "i",  CGRIDVIEW_client_width),
  GB_PROPERTY_READ("ClientHeight", "i", CGRIDVIEW_client_height),
  GB_PROPERTY_READ("ClientH", "i", CGRIDVIEW_client_height),

  GB_EVENT("Change", 0, 0, &EVENT_Change),
  GB_EVENT("Select", 0, 0, &EVENT_Select),
  GB_EVENT("Activate", 0, 0, &EVENT_Activate),
  GB_EVENT("Click", 0, 0, &EVENT_Click),
  GB_EVENT("Scroll", 0, 0, &EVENT_Scroll),
  GB_EVENT("Data", 0, "(Row)i(Column)i", &EVENT_Data),
  GB_EVENT("ColumnClick", 0, "(Column)i", &EVENT_ColumnClick),
  GB_EVENT("RowClick", 0, "(Row)i", &EVENT_RowClick),
  GB_EVENT("ColumnResize", 0, "(Column)i", &EVENT_ColumnResize),
  GB_EVENT("RowResize", 0, "(Row)i", &EVENT_RowResize),

	GRIDVIEW_DESCRIPTION,
	
  GB_END_DECLARE
};
