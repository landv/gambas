/***************************************************************************

  CGridView.cpp

  (c) 2004-2006 - Daniel Campos Fernández <dcamposf@gmail.com>
  
  GTK+ component
  
  Realizado para la Junta de Extremadura. 
  Consejería de Educación Ciencia y Tecnología. 
  Proyecto gnuLinEx

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
		GB.ReturnObject(THIS->data->picture ? THIS->data->picture->tag->get() : 0);
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
	{
		GB.ReturnNewString(THIS->data->text,0);
		return;
	}

	THIS->data->setText(GB.ToZeroString(PROP(GB_STRING)));

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
		GB.ReturnObject(GRIDVIEW->itemPicture(THIS->row, THIS->col));
	else
	{
		CPICTURE *pict = (CPICTURE *)VPROP(GB_OBJECT);
		GRIDVIEW->setItemPicture(THIS->row, THIS->col, pict ? pict->picture : 0);
	}

END_PROPERTY

BEGIN_PROPERTY(CGRIDVIEWITEM_font)

	if (READ_PROPERTY)
		GB.ReturnObject(GRIDVIEW->itemFont(THIS->row, THIS->col));
	else
	{
		CFONT *font = (CFONT *)VPROP(GB_OBJECT);
		GRIDVIEW->setItemFont(THIS->row, THIS->col, font ? font->font : 0);
	}

END_PROPERTY

BEGIN_PROPERTY(CGRIDVIEWITEM_alignment)

	if (READ_PROPERTY) 
		GB.ReturnInteger(GRIDVIEW->itemAlignment(THIS->row,THIS->col));
	else
		GRIDVIEW->setItemAlignment( THIS->row, THIS->col,VPROP(GB_INTEGER) );

END_PROPERTY

/*BEGIN_PROPERTY(CGRIDVIEWITEM_key)

	GB.ReturnInteger(THIS->row);

END_PROPERTY

BEGIN_PROPERTY(CGRIDVIEWITEM_column)

	GB.ReturnInteger(THIS->col);

END_PROPERTY*/

BEGIN_PROPERTY(CGRIDVIEWITEM_x)

	GB.ReturnInteger(GRIDVIEW->itemX(THIS->col));

END_PROPERTY

BEGIN_PROPERTY(CGRIDVIEWITEM_y)

	GB.ReturnInteger(GRIDVIEW->itemY(THIS->row));

END_PROPERTY

BEGIN_PROPERTY(CGRIDVIEWITEM_w)

	GB.ReturnInteger(GRIDVIEW->itemW(THIS->col));

END_PROPERTY

BEGIN_PROPERTY(CGRIDVIEWITEM_h)

	GB.ReturnInteger(GRIDVIEW->itemH(THIS->row));

END_PROPERTY

BEGIN_PROPERTY(CGRIDVIEWITEM_text)

	if (READ_PROPERTY) 
		GB.ReturnNewZeroString(GRIDVIEW->itemText(THIS->row,THIS->col)); 
	else
		GRIDVIEW->setItemText(THIS->row, THIS->col, GB.ToZeroString(PROP(GB_STRING)));

END_PROPERTY

BEGIN_PROPERTY(CGRIDVIEWITEM_bg)

	if (READ_PROPERTY) 
	{ 
		GB.ReturnInteger(GRIDVIEW->itemBg(THIS->row,THIS->col)); 
		return; 
	}

	GRIDVIEW->setItemBg( THIS->row, THIS->col,VPROP(GB_INTEGER) );

END_PROPERTY

BEGIN_PROPERTY(CGRIDVIEWITEM_fg)

	if (READ_PROPERTY) 
	{ 
		GB.ReturnInteger(GRIDVIEW->itemFg(THIS->row,THIS->col)); 
		return; 
	}

	GRIDVIEW->setItemFg( THIS->row, THIS->col,VPROP(GB_INTEGER) );

END_PROPERTY

BEGIN_PROPERTY(CGRIDVIEWITEM_padding)

	if (READ_PROPERTY) 
		GB.ReturnInteger(GRIDVIEW->itemPadding(THIS->row,THIS->col));
	else
		GRIDVIEW->setItemPadding( THIS->row, THIS->col,VPROP(GB_INTEGER) );

END_PROPERTY

/*BEGIN_PROPERTY(CGRIDVIEWITEM_selected)

	if (READ_PROPERTY) 
	{ 
		GB.ReturnBoolean(GRIDVIEW->itemSelected(THIS->row,THIS->col)); 
		return; 
	}

	GRIDVIEW->setItemSelected( THIS->row, THIS->col,VPROP(GB_BOOLEAN) );

END_PROPERTY*/

BEGIN_METHOD_VOID(CGRIDVIEWITEM_clear)

	GRIDVIEW->clearItem(THIS->row,THIS->col);

END_METHOD

BEGIN_METHOD_VOID(CGRIDVIEWITEM_refresh)

	GRIDVIEW->queryUpdate(THIS->row,THIS->col);

END_METHOD

BEGIN_METHOD_VOID(CGRIDVIEWITEM_ensure_visible)

	GRIDVIEW->ensureVisible(THIS->row,THIS->col);

END_METHOD

/*************************************************

 GridViewColumns
 
**************************************************/

BEGIN_METHOD(CGRIDVIEW_columns_get,GB_INTEGER Column;)

	if ( (VARG(Column)<0) || (VARG(Column)>=GRIDVIEW->columnCount() ) )
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
		if (!GRIDVIEW->columnCount())
			GB.ReturnBoolean(true);
		else
			GB.ReturnBoolean(GRIDVIEW->columnResizable(0)); 
		return; 
	}

	for (bc=0; bc<GRIDVIEW->columnCount(); bc++)
		GRIDVIEW->setColumnResizable(bc,VPROP(GB_BOOLEAN));

END_PROPERTY

BEGIN_PROPERTY(CGRIDVIEW_columns_count)

	if (READ_PROPERTY) { GB.ReturnInteger(GRIDVIEW->columnCount()); return; }
	GRIDVIEW->setColumnCount(VPROP(GB_INTEGER));

	if ( (GRIDVIEW->rowCount()==0) || (GRIDVIEW->columnCount()==0) )
	{
		THIS->row=-1;
		THIS->col=-1;
		return;
	}

	if (GRIDVIEW->columnCount()>=THIS->col) THIS->col=GRIDVIEW->columnCount()-1;

END_PROPERTY

BEGIN_PROPERTY(CGRIDVIEW_column_width)

	if (READ_PROPERTY) { GB.ReturnInteger(GRIDVIEW->columnWidth(THIS->col)); return; }	
	GRIDVIEW->setColumnWidth(THIS->col,VPROP(GB_INTEGER));

END_PROPERTY

BEGIN_PROPERTY(CGRIDCOLS_width)

	int bc;

	if (READ_PROPERTY) { GB.ReturnInteger(GRIDVIEW->columnWidth(0)); return; }	
	for (bc=0;bc<GRIDVIEW->columnCount(); bc++)
		GRIDVIEW->setColumnWidth(bc,VPROP(GB_INTEGER));

END_PROPERTY

BEGIN_PROPERTY(CGRIDVIEW_column_resizable)

	if (READ_PROPERTY) 
	{ 
		GB.ReturnBoolean(GRIDVIEW->columnResizable(THIS->col)); 
		return;

	}

	GRIDVIEW->setColumnResizable(THIS->col,VPROP(GB_BOOLEAN));

END_PROPERTY

BEGIN_PROPERTY(CGRIDCOLS_height)

	GB.ReturnInteger(WIDGET->headerHeight());

END_PROPERTY

BEGIN_METHOD_VOID(CGRIDCOL_refresh)

	GRIDVIEW->queryUpdate(-1, THIS->col);

END_METHOD


/*************************************************

 GridViewRows
 
**************************************************/

BEGIN_PROPERTY(CGRIDVIEW_count)

	if (READ_PROPERTY) { GB.ReturnInteger(GRIDVIEW->rowCount()); return; }
	GRIDVIEW->setRowCount(VPROP(GB_INTEGER));
	
	if ( (GRIDVIEW->rowCount()==0) || (GRIDVIEW->columnCount()==0) )
	{
		THIS->row=-1;
		THIS->col=-1;
		return;
	}

	if (GRIDVIEW->rowCount()>=THIS->row) THIS->row=GRIDVIEW->rowCount()-1;

END_PROPERTY

BEGIN_METHOD(CGRIDVIEW_rows_get,GB_INTEGER Row;)

	if ( (VARG(Row)<0) || (VARG(Row)>=GRIDVIEW->rowCount() ) )
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

	if (READ_PROPERTY) { GB.ReturnInteger(GRIDVIEW->rowHeight(0)); return; }	
	for (bc=0;bc<GRIDVIEW->rowCount(); bc++)
		GRIDVIEW->setRowHeight(bc,VPROP(GB_INTEGER));

END_PROPERTY

BEGIN_PROPERTY(CGRIDROWS_width)

	GB.ReturnInteger(WIDGET->rowWidth());

END_PROPERTY

BEGIN_METHOD_VOID(CGRIDROWS_unselect)

	GRIDVIEW->clearSelection();

END_METHOD

BEGIN_PROPERTY(CGRIDVIEW_rows_resizable)

	int bc;

	if (READ_PROPERTY) { GB.ReturnBoolean(GRIDVIEW->rowResizable(0)); return; }	
	for (bc=0;bc<GRIDVIEW->rowCount(); bc++)
		GRIDVIEW->setRowResizable(bc,VPROP(GB_BOOLEAN));

END_PROPERTY

BEGIN_PROPERTY(CGRIDVIEW_row_height)

	if (READ_PROPERTY) { GB.ReturnInteger(GRIDVIEW->rowHeight(THIS->row)); return; }	
	GRIDVIEW->setRowHeight(THIS->row,VPROP(GB_INTEGER));

END_PROPERTY

BEGIN_PROPERTY(CGRIDVIEW_row_resizable)

	if (READ_PROPERTY) { GB.ReturnBoolean(GRIDVIEW->rowResizable(THIS->row)); return; }	
	GRIDVIEW->setRowResizable(THIS->row,VPROP(GB_BOOLEAN));

END_PROPERTY

BEGIN_PROPERTY(CGRIDVIEW_row_selected)

	if (READ_PROPERTY) { GB.ReturnBoolean(GRIDVIEW->rowSelected(THIS->row)); return; }
	GRIDVIEW->setRowSelected(THIS->row,VPROP(GB_BOOLEAN));

END_PROPERTY

BEGIN_METHOD_VOID(CGRIDROW_refresh)

	GRIDVIEW->queryUpdate(THIS->row, -1);

END_METHOD

/******************************************************

 GridView
 
*******************************************************/

BEGIN_PROPERTY(CGRIDVIEW_scrollbar)

	if (READ_PROPERTY) { GB.ReturnInteger(GRIDVIEW->scrollBar()); return; }
	GRIDVIEW->setScrollBar(VPROP(GB_INTEGER));

END_PROPERTY

BEGIN_PROPERTY(CGRIDVIEW_scrollX)

	if (READ_PROPERTY) { GB.ReturnInteger(GRIDVIEW->scrollX()); return; }
	GRIDVIEW->setScrollX(VPROP(GB_INTEGER));

END_PROPERTY

BEGIN_PROPERTY(CGRIDVIEW_scrollY)

	if (READ_PROPERTY) { GB.ReturnInteger(GRIDVIEW->scrollY()); return; }
	GRIDVIEW->setScrollY(VPROP(GB_INTEGER));

END_PROPERTY

BEGIN_PROPERTY(CGRIDVIEW_grid)

	if (READ_PROPERTY) { GB.ReturnBoolean(GRIDVIEW->drawGrid()); return; }
	GRIDVIEW->setDrawGrid(VPROP(GB_BOOLEAN));

END_PROPERTY

BEGIN_PROPERTY(CGRIDVIEW_client_x)

	GB.ReturnInteger(GRIDVIEW->visibleLeft());

END_PROPERTY

BEGIN_PROPERTY(CGRIDVIEW_client_y)

	GB.ReturnInteger(GRIDVIEW->visibleTop());

END_PROPERTY

BEGIN_PROPERTY(CGRIDVIEW_client_width)

	GB.ReturnInteger(GRIDVIEW->visibleWidth());

END_PROPERTY

BEGIN_PROPERTY(CGRIDVIEW_client_height)

	GB.ReturnInteger(GRIDVIEW->visibleHeight());

END_PROPERTY

BEGIN_PROPERTY(CGRIDVIEW_current)
	
	GRIDVIEW->getCursor(&THIS->row, &THIS->col);
	
	if (THIS->row < 0 || THIS->col < 0)
		GB.ReturnNull(); 
	else
		RETURN_SELF();
	
END_PROPERTY

BEGIN_METHOD(CGRIDVIEW_new, GB_OBJECT parent)

	InitControl(new gGridView(CONTAINER(VARG(parent))),(CWIDGET*)THIS);

	THIS->row=-1;
	THIS->col=-1;

	GRIDVIEW->setDataFunc((void*)raise_data, _object);
	GRIDVIEW->onActivate = raise_activate;
	GRIDVIEW->onClick = raise_click;
	GRIDVIEW->onChange = raise_change;
	GRIDVIEW->onRowClick = raise_row_click;
	GRIDVIEW->onColumnClick = raise_col_click;
	GRIDVIEW->onFooterClick = raise_foot_click;
	GRIDVIEW->onScroll = raise_scroll;
	GRIDVIEW->onRowResize = raise_row_resize;
	GRIDVIEW->onColumnResize = raise_col_resize;

END_METHOD

BEGIN_METHOD(CGRIDVIEW_find, GB_INTEGER X; GB_INTEGER Y;)

	int px,py;

	px=GRIDVIEW->rowAt(VARG(Y));
	py=GRIDVIEW->columnAt(VARG(X));
	
	if ( (px==-1) || (py==-1) ) { GB.ReturnBoolean(true); return; }

	THIS->row=px;
	THIS->col=py;

	GB.ReturnBoolean(false);

END_METHOD

BEGIN_METHOD(CGRIDVIEW_rowat, GB_INTEGER X;)

	GB.ReturnInteger(GRIDVIEW->rowAt(VARG(X)));

END_METHOD

BEGIN_METHOD(CGRIDVIEW_colat, GB_INTEGER X;)

	GB.ReturnInteger(GRIDVIEW->columnAt(VARG(X)));

END_METHOD

BEGIN_METHOD_VOID(CGRIDVIEW_clear)

	int bx,by;

	for (bx=0; bx<GRIDVIEW->rowCount(); bx++)
		for (by=0; by<GRIDVIEW->columnCount(); by++)
			GRIDVIEW->setItemText(by,bx,"");

END_METHOD

BEGIN_METHOD(CGRIDVIEW_get, GB_INTEGER Key; GB_INTEGER Column;)

	if ( (VARG(Key)<0) || (VARG(Key)>=GRIDVIEW->rowCount()) )
	{
		GB.Error("Bad row index");
		GB.ReturnNull();
		return;
	}

	if ( (VARG(Column)<0) || (VARG(Column)>=GRIDVIEW->columnCount()) )
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

	if (READ_PROPERTY) { GB.ReturnBoolean(GRIDVIEW->getBorder()); return; }
	GRIDVIEW->setBorder(VPROP(GB_BOOLEAN));

END_PROPERTY

BEGIN_PROPERTY(CGRIDVIEW_mode)

	int mode;

	if (READ_PROPERTY) { GB.ReturnInteger(GRIDVIEW->selectionMode()); return; }

	mode=VPROP(GB_INTEGER);
	if (mode<0) mode=0;
	if (mode>2) mode=2;
	GRIDVIEW->setSelectionMode(mode);

END_PROPERTY

BEGIN_PROPERTY(CGRIDVIEW_item)

	WIDGET->getCursor(&THIS->row, &THIS->col);
	RETURN_SELF();

END_PROPERTY

/***************************************************************************

Headers, Footers, and Row Separators

****************************************************************************/
BEGIN_PROPERTY(CGRIDVIEWHEADER_visible)

	if (READ_PROPERTY) { GB.ReturnInteger(GRIDVIEW->headersVisible()); return; }
	GRIDVIEW->setHeadersVisible(VPROP(GB_INTEGER));

END_PROPERTY

BEGIN_PROPERTY(CGRIDVIEWFOOTER_visible)

	if (READ_PROPERTY) { GB.ReturnBoolean(GRIDVIEW->footersVisible()); return; }
	GRIDVIEW->setFootersVisible(VPROP(GB_BOOLEAN));

END_PROPERTY

BEGIN_PROPERTY(CGRIDVIEW_column_headertext)

	if (READ_PROPERTY)
	{
		GB.ReturnNewString(GRIDVIEW->headerText(THIS->col),0);
		return;
	}
	GRIDVIEW->setHeaderText(THIS->col,PROP(GB_STRING)->value.addr);

END_PROPERTY

BEGIN_PROPERTY(CGRIDVIEW_column_footertext)

	if (READ_PROPERTY)
	{
		GB.ReturnNewString(GRIDVIEW->footerText(THIS->col),0);
		return;
	}
	GRIDVIEW->setFooterText(THIS->col,PROP(GB_STRING)->value.addr);

END_PROPERTY

BEGIN_PROPERTY(CGRIDVIEW_row_text)

	if (READ_PROPERTY)
	{
		GB.ReturnNewString(GRIDVIEW->rowText(THIS->row),0);
		return;
	}
	GRIDVIEW->setRowText(THIS->row,PROP(GB_STRING)->value.addr);

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
  GB_PROPERTY("Background", "i", CGRIDVIEWITEM_bg),
  GB_PROPERTY("BackColor", "i", CGRIDVIEWITEM_bg),
  GB_PROPERTY("Foreground", "i", CGRIDVIEWITEM_fg),
  GB_PROPERTY("ForeColor", "i", CGRIDVIEWITEM_fg),
  GB_PROPERTY("Padding", "i", CGRIDVIEWITEM_padding),
  GB_PROPERTY("Alignment", "i", CGRIDVIEWITEM_alignment),
  GB_PROPERTY("Font", "Font", CGRIDVIEWITEM_font),
  //GB_PROPERTY("Selected","b",CGRIDVIEWITEM_selected),

  GB_METHOD("Clear", NULL, CGRIDVIEWITEM_clear, NULL),
  GB_METHOD("Refresh",NULL,CGRIDVIEWITEM_refresh,NULL),
  GB_METHOD("EnsureVisible", NULL, CGRIDVIEWITEM_ensure_visible, NULL),
  GB_END_DECLARE
};

GB_DESC CGridViewDataDesc[] =
{
  GB_DECLARE(".GridViewData", 0), GB_VIRTUAL_CLASS(),

  GB_PROPERTY("Picture", "Picture", CGRIDVIEWDATA_picture),
  GB_PROPERTY("Text", "s", CGRIDVIEWDATA_text),
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
  GB_PROPERTY("Width", "i", CGRIDVIEW_column_width),
  GB_PROPERTY("W", "i", CGRIDVIEW_column_width),
  GB_PROPERTY("Text","s",CGRIDVIEW_column_headertext),
  //GB_PROPERTY("HeaderText","s",CGRIDVIEW_column_headertext),
  //GB_PROPERTY("FooterText","s",CGRIDVIEW_column_footertext),
  GB_METHOD("Refresh", NULL, CGRIDCOL_refresh, NULL),
  GB_END_DECLARE
};

GB_DESC CGridViewColumnsDesc[] =
{
  GB_DECLARE(".GridViewColumns", 0), GB_VIRTUAL_CLASS(),
  GB_METHOD("_get", ".GridViewColumn", CGRIDVIEW_columns_get, "(Column)i"),
  GB_PROPERTY("Resizable","b",CGRIDVIEW_columns_resizable),
  GB_PROPERTY("Count", "i", CGRIDVIEW_columns_count),
  GB_PROPERTY("Width", "i", CGRIDCOLS_width),
  GB_PROPERTY("W", "i", CGRIDCOLS_width),
  GB_PROPERTY_READ("Height", "i", CGRIDCOLS_height),
  GB_PROPERTY_READ("H", "i", CGRIDCOLS_height),
  GB_END_DECLARE
};

GB_DESC CGridViewRowDesc[] =
{
  GB_DECLARE(".GridViewRow", 0), GB_VIRTUAL_CLASS(),
  GB_PROPERTY("Height", "i", CGRIDVIEW_row_height),
  GB_PROPERTY("H", "i", CGRIDVIEW_row_height),
  GB_PROPERTY("Resizable","b",CGRIDVIEW_row_resizable),
  GB_PROPERTY("Text","s",CGRIDVIEW_row_text),
  GB_PROPERTY("Selected","b",CGRIDVIEW_row_selected),
  GB_METHOD("Refresh", NULL, CGRIDROW_refresh, NULL),
  GB_END_DECLARE
};

GB_DESC CGridViewRowsDesc[] =
{
  GB_DECLARE(".GridViewRows", 0), GB_VIRTUAL_CLASS(),
  GB_METHOD("_get", ".GridViewRow", CGRIDVIEW_rows_get, "(Row)i"),
  GB_PROPERTY("Count", "i", CGRIDVIEW_count),
  GB_PROPERTY("Resizable","b",CGRIDVIEW_rows_resizable),
  GB_PROPERTY("Height", "i", CGRIDROWS_height),
  GB_PROPERTY("H", "i", CGRIDROWS_height),
  GB_PROPERTY_READ("Width", "i", CGRIDROWS_width),
  GB_PROPERTY_READ("W", "i", CGRIDROWS_width),
  GB_METHOD("Unselect", NULL, CGRIDROWS_unselect, NULL),
  GB_END_DECLARE
};

GB_DESC CGridViewDesc[] =
{
  GB_DECLARE("GridView", sizeof(CGRIDVIEW)), GB_INHERITS("Control"),

  GB_CONSTANT("None", "i", 0),
  GB_CONSTANT("Horizontal", "i", 1),
  GB_CONSTANT("Vertical", "i", 2),
  GB_CONSTANT("Both", "i", 3),

  GB_CONSTANT("_Properties", "s", CGRIDVIEW_PROPERTIES),
  GB_CONSTANT("_DefaultEvent", "s", "Click"),

  GB_METHOD("_new", NULL, CGRIDVIEW_new, "(Parent)Container;"),
  GB_METHOD("_get", ".GridViewCell", CGRIDVIEW_get, "(Row)i(Column)i"),
  GB_METHOD("Clear", NULL, CGRIDVIEW_clear, NULL),
  GB_METHOD("Find", "b", CGRIDVIEW_find, "(X)i(Y)i"),
  GB_METHOD("RowAt","i",CGRIDVIEW_rowat,"(Y)i"),
  GB_METHOD("ColumnAt","i",CGRIDVIEW_colat,"(X)i"),

  GB_PROPERTY("Border", "b", CGRIDVIEW_border),
  GB_PROPERTY("Mode","i",CGRIDVIEW_mode),
  GB_PROPERTY("ScrollBar", "i", CGRIDVIEW_scrollbar),
  GB_PROPERTY("Grid", "b", CGRIDVIEW_grid),
  GB_PROPERTY("Resizable","b",CGRIDVIEW_columns_resizable),
  GB_PROPERTY("Header", "i", CGRIDVIEWHEADER_visible),
  //GB_PROPERTY("Footer", "b", CGRIDVIEWFOOTER_visible),
    
  GB_PROPERTY("ScrollX", "i", CGRIDVIEW_scrollX),
  GB_PROPERTY("ScrollY", "i", CGRIDVIEW_scrollY),
  
  GB_PROPERTY("Row", "i", CGRIDVIEW_row),
  GB_PROPERTY("Column", "i", CGRIDVIEW_column),
  GB_METHOD("MoveTo", NULL, CGRIDVIEW_move_to, "(Row)i(Column)i"),
  
  GB_PROPERTY_READ("Data", ".GridViewData",CGRIDVIEW_data),
  
  GB_PROPERTY_SELF("Columns", ".GridViewColumns"),
  GB_PROPERTY_SELF("Rows", ".GridViewRows"),
  GB_PROPERTY_READ("Current", ".GridViewCell", CGRIDVIEW_current),
  GB_PROPERTY_READ("Item", ".GridViewCell", CGRIDVIEW_item),
  GB_PROPERTY_READ("ClientX", "i", CGRIDVIEW_client_x),
  GB_PROPERTY_READ("ClientY", "i",  CGRIDVIEW_client_y),
  GB_PROPERTY_READ("ClientWidth", "i", CGRIDVIEW_client_width),
  GB_PROPERTY_READ("ClientW", "i",  CGRIDVIEW_client_width),
  GB_PROPERTY_READ("ClientHeight", "i", CGRIDVIEW_client_height),
  GB_PROPERTY_READ("ClientH", "i", CGRIDVIEW_client_height),

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
