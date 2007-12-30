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

#include <stdlib.h>
#include <stdio.h>
#include <gtk/gtk.h>

#include "gambas.h"
#include "main.h"
#include "widgets.h"
#include "tablerender.h"
#include "CWidget.h"
#include "CContainer.h"
#include "CGridView.h"
#include "CPicture.h"

DECLARE_EVENT(EVENT_Click);     /* simple click */
DECLARE_EVENT(EVENT_RowClick);
DECLARE_EVENT(EVENT_ColumnClick);
DECLARE_EVENT(EVENT_FooterClick);
DECLARE_EVENT(EVENT_Activate);  /* double click */
DECLARE_EVENT(EVENT_Scroll);
DECLARE_EVENT(EVENT_Data);

void grid_scroll(gGridView *sender)
{
	CWIDGET *_ob=GetObject(sender);

	if (!_ob) return;
	if (GB.CanRaise(_ob,EVENT_Scroll)) 
		GB.Raise((void*)_ob,EVENT_Scroll,0);
}

void grid_col_click(gGridView *sender,long col)
{
	CWIDGET *_ob=GetObject(sender);

	if (!_ob) return;
	if (GB.CanRaise(_ob,EVENT_ColumnClick)) 
		GB.Raise((void*)_ob,EVENT_ColumnClick,1,GB_T_INTEGER,col);
}

void grid_foot_click(gGridView *sender,long col)
{
	CWIDGET *_ob=GetObject(sender);
	
	if (!_ob) return;
	if (GB.CanRaise(_ob,EVENT_FooterClick)) 
		GB.Raise((void*)_ob,EVENT_FooterClick,1,GB_T_INTEGER,col);
}

void grid_row_click(gGridView *sender,long row)
{
	CWIDGET *_ob=GetObject(sender);
	
	if (!_ob) return;
	if (GB.CanRaise(_ob,EVENT_RowClick)) 
		GB.Raise((void*)_ob,EVENT_RowClick,1,GB_T_INTEGER,row);
}

void grid_click(gGridView *sender,long row,long col)
{
	CWIDGET *_ob=GetObject(sender);
	
	if (!_ob) return;
	if (GB.CanRaise(_ob,EVENT_Click)) 
	{
		((CGRIDVIEW*)_ob)->curr_row=row;
		((CGRIDVIEW*)_ob)->curr_col=col;
		GB.Raise((void*)_ob,EVENT_Click,0);
	}
}

void grid_activate(gGridView *sender,long row,long col)
{
	CWIDGET *_ob=GetObject(sender);
	
	if (!_ob) return;
	if (GB.CanRaise(_ob,EVENT_Activate)) 
	{
		((CGRIDVIEW*)_ob)->curr_row=row;
		((CGRIDVIEW*)_ob)->curr_col=col;
		GB.Raise((void*)_ob,EVENT_Activate,0);
	}
}

void grid_data_func(gTableData **fill,long row, long col,void *_object)
{
	if (GB.CanRaise(THIS,EVENT_Data))
	{
		THIS->data=(*fill);
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

	stub("CGRIDVIEWDATA_picture");
	if (READ_PROPERTY) { GB.ReturnNull(); }

END_PROPERTY

BEGIN_PROPERTY(CGRIDVIEWDATA_alignment)

	stub("CGRIDVIEWDATA_alignment");
	if (READ_PROPERTY) { GB.ReturnInteger(0); }

END_PROPERTY

BEGIN_PROPERTY(CGRIDVIEWDATA_text)

	if (READ_PROPERTY)
	{
		GB.ReturnNewString(THIS->data->text,0);
		return;
	}

	THIS->data->setText(PROP(GB_STRING)->value.addr);

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


BEGIN_PROPERTY(CGRIDVIEWDATA_xpad)

	if (READ_PROPERTY)
	{
		GB.ReturnInteger(THIS->data->xpad);
		return;
	}

	THIS->data->xpad=VPROP(GB_INTEGER);
	if (THIS->data->xpad<0) THIS->data->xpad=0;

END_PROPERTY

BEGIN_PROPERTY(CGRIDVIEWDATA_ypad)

	if (READ_PROPERTY)
	{
		GB.ReturnInteger(THIS->data->ypad);
		return;
	}

	THIS->data->xpad=VPROP(GB_INTEGER);
	if (THIS->data->xpad<0) THIS->data->ypad=0;

END_PROPERTY




BEGIN_PROPERTY(CGRIDVIEWITEM_picture)

	stub("CGRIDVIEWITEM_picture");
	if (READ_PROPERTY) { GB.ReturnNull(); }

END_PROPERTY

BEGIN_PROPERTY(CGRIDVIEWITEM_key)

	GB.ReturnInteger(THIS->row);

END_PROPERTY

BEGIN_PROPERTY(CGRIDVIEWITEM_column)

	GB.ReturnInteger(THIS->col);

END_PROPERTY

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

BEGIN_PROPERTY(CGRIDVIEWITEM_alignment)



END_PROPERTY

BEGIN_PROPERTY(CGRIDVIEWITEM_text)

	if (READ_PROPERTY) 
	{ 
		GB.ReturnNewString(GRIDVIEW->itemText(THIS->row,THIS->col),0); 
		return; 
	}

	GRIDVIEW->setItemText( THIS->row, THIS->col,PROP(GB_STRING)->value.addr );

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

BEGIN_PROPERTY(CGRIDVIEWITEM_xpad)

	if (READ_PROPERTY) 
	{ 
		GB.ReturnInteger(GRIDVIEW->itemXPad(THIS->row,THIS->col)); 
		return; 
	}

	GRIDVIEW->setItemXPad( THIS->row, THIS->col,VPROP(GB_INTEGER) );

END_PROPERTY

BEGIN_PROPERTY(CGRIDVIEWITEM_ypad)

	if (READ_PROPERTY) 
	{ 
		GB.ReturnInteger(GRIDVIEW->itemYPad(THIS->row,THIS->col)); 
		return; 
	}

	GRIDVIEW->setItemYPad( THIS->row, THIS->col,VPROP(GB_INTEGER) );

END_PROPERTY

BEGIN_PROPERTY(CGRIDVIEWITEM_selected)

	if (READ_PROPERTY) 
	{ 
		GB.ReturnBoolean(GRIDVIEW->itemSelected(THIS->row,THIS->col)); 
		return; 
	}

	GRIDVIEW->setItemSelected( THIS->row, THIS->col,VPROP(GB_BOOLEAN) );

END_PROPERTY

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

	long bc;

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

BEGIN_PROPERTY(CGRIDVIEW_columns_width)

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

	if ( (VARG(Row)<0) || (VARG(Row)>=GRIDVIEW->columnCount() ) )
	{
		GB.Error("Bad row index");
		GB.ReturnNull();
		return;
	}

	THIS->row=VARG(Row);
	RETURN_SELF();

END_METHOD

BEGIN_PROPERTY(CGRIDVIEW_rows_height)

	int bc;

	if (READ_PROPERTY) { GB.ReturnInteger(GRIDVIEW->rowHeight(0)); return; }	
	for (bc=0;bc<GRIDVIEW->rowCount(); bc++)
		GRIDVIEW->setRowHeight(bc,VPROP(GB_INTEGER));

END_PROPERTY

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
	
	long row,col;

	GRIDVIEW->getGridCursor(&row,&col);
	if ( (col==-1) || (row==-1) ) { GB.ReturnNull(); return; }
	THIS->row=row;
	THIS->col=col;
	RETURN_SELF();
	
END_PROPERTY

BEGIN_METHOD(CGRIDVIEW_new, GB_OBJECT Parent;)

	CCONTAINER *Parent=(CCONTAINER*)VPROP(GB_OBJECT);
	Parent=(CCONTAINER*)GetContainer ((CWIDGET*)Parent);

	THIS->row=-1;
	THIS->col=-1;

	THIS->widget=new gGridView(Parent->widget);
	InitControl(THIS->widget,(CWIDGET*)THIS);

	GRIDVIEW->setDataFunc((void*)grid_data_func,_object);
	GRIDVIEW->onActivate=grid_activate;
	GRIDVIEW->onClick=grid_click;
	GRIDVIEW->onRowClick=grid_row_click;
	GRIDVIEW->onColumnClick=grid_col_click;
	GRIDVIEW->onFooterClick=grid_foot_click;
	GRIDVIEW->onScroll=grid_scroll;

END_METHOD

BEGIN_METHOD(CGRIDVIEW_find, GB_INTEGER X; GB_INTEGER Y;)

	long px,py;

	px=GRIDVIEW->rowAt(VARG(Y));
	py=GRIDVIEW->columnAt(VARG(X));
	
	if ( (px==-1) || (py==-1) ) { GB.ReturnBoolean(true); return; }

	THIS->row=px;
	THIS->col=py;

	GB.ReturnBoolean(false);

END_METHOD

BEGIN_METHOD(CGRIDVIEW_rowat, GB_INTEGER X;)

	long px;

	GB.ReturnInteger(GRIDVIEW->rowAt(VARG(X)));


END_METHOD

BEGIN_METHOD(CGRIDVIEW_colat, GB_INTEGER X;)

	long px;

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

	int row;

	if (READ_PROPERTY) { GB.ReturnInteger(THIS->row); return; }

	if ( (!GRIDVIEW->rowCount()) || (!GRIDVIEW->columnCount()) ) return;
	
	row=VPROP(GB_INTEGER);
	if (row<0) row=-1;
	if (row>=GRIDVIEW->rowCount()) row=GRIDVIEW->rowCount()-1;
	THIS->row=row;
	

END_PROPERTY

BEGIN_PROPERTY(CGRIDVIEW_column)

	int col;

	if (READ_PROPERTY) { GB.ReturnInteger(THIS->col); return; }

	if ( (!GRIDVIEW->rowCount()) || (!GRIDVIEW->columnCount()) ) return;
	
	col=VPROP(GB_INTEGER);
	if (col<0) col=-1;
	if (col>=GRIDVIEW->columnCount()) col=GRIDVIEW->columnCount()-1;
	THIS->col=col;
	

END_PROPERTY

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

	THIS->row=THIS->curr_row;
	THIS->col=THIS->curr_col;
	RETURN_SELF();

END_PROPERTY

/***************************************************************************

Headers, Footers, and Row Separators

****************************************************************************/
BEGIN_PROPERTY(CGRIDVIEWHEADER_visible)

	if (READ_PROPERTY) { GB.ReturnBoolean(GRIDVIEW->headersVisible()); return; }
	GRIDVIEW->setHeadersVisible(VPROP(GB_BOOLEAN));

END_PROPERTY

BEGIN_PROPERTY(CGRIDVIEWFOOTER_visible)

	if (READ_PROPERTY) { GB.ReturnBoolean(GRIDVIEW->footersVisible()); return; }
	GRIDVIEW->setFootersVisible(VPROP(GB_BOOLEAN));

END_PROPERTY

BEGIN_PROPERTY(CGRIDVIEWROWSEP_visible)

	if (READ_PROPERTY) { GB.ReturnBoolean(GRIDVIEW->rowSeparatorVisible()); return; }
	GRIDVIEW->setRowSeparatorVisible(VPROP(GB_BOOLEAN));

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
  GB_DECLARE(".GridCell", 0), GB_VIRTUAL_CLASS(),
  
  GB_PROPERTY_READ("Row", "i", CGRIDVIEWITEM_key),
  GB_PROPERTY_READ("Column", "i", CGRIDVIEWITEM_column),  
  GB_PROPERTY_READ("X", "i", CGRIDVIEWITEM_x),
  GB_PROPERTY_READ("Y", "i", CGRIDVIEWITEM_y),
  GB_PROPERTY_READ("Left", "i", CGRIDVIEWITEM_x),
  GB_PROPERTY_READ("Top", "i", CGRIDVIEWITEM_y),
  GB_PROPERTY_READ("Width", "i", CGRIDVIEWITEM_w),
  GB_PROPERTY_READ("Height", "i", CGRIDVIEWITEM_h),
  GB_PROPERTY_READ("W", "i", CGRIDVIEWITEM_w),
  GB_PROPERTY_READ("H", "i", CGRIDVIEWITEM_h),
  
  GB_PROPERTY("Picture", "Picture", CGRIDVIEWDATA_picture),
  GB_PROPERTY("Text", "s", CGRIDVIEWITEM_text),
  GB_PROPERTY("Background", "i", CGRIDVIEWITEM_bg),
  GB_PROPERTY("BackColor", "i", CGRIDVIEWITEM_bg),
  GB_PROPERTY("Foreground", "i", CGRIDVIEWITEM_fg),
  GB_PROPERTY("ForeColor", "i", CGRIDVIEWITEM_fg),
  GB_PROPERTY("XPad", "i", CGRIDVIEWITEM_xpad),
  GB_PROPERTY("YPad", "i", CGRIDVIEWITEM_ypad),
  GB_PROPERTY("Alignment", "i", CGRIDVIEWITEM_alignment),
  GB_PROPERTY("Selected","b",CGRIDVIEWITEM_selected),

  GB_METHOD("Clear", NULL, CGRIDVIEWITEM_clear, NULL),
  GB_METHOD("Refresh",NULL,CGRIDVIEWITEM_refresh,NULL),
  GB_METHOD("EnsureVisible", NULL, CGRIDVIEWITEM_ensure_visible, NULL),
  GB_END_DECLARE
};

GB_DESC CGridViewDataDesc[] =
{
  GB_DECLARE(".GridData", 0), GB_VIRTUAL_CLASS(),

  GB_PROPERTY("Picture", "Picture", CGRIDVIEWITEM_picture),
  GB_PROPERTY("Text", "s", CGRIDVIEWDATA_text),
  GB_PROPERTY("Background", "i", CGRIDVIEWDATA_bg),
  GB_PROPERTY("BackColor", "i", CGRIDVIEWDATA_bg),
  GB_PROPERTY("Foreground", "i", CGRIDVIEWDATA_fg),
  GB_PROPERTY("ForeColor", "i", CGRIDVIEWDATA_fg),
  GB_PROPERTY("XPad", "i", CGRIDVIEWDATA_xpad),
  GB_PROPERTY("YPad", "i", CGRIDVIEWDATA_ypad),
  GB_PROPERTY("Alignment", "i", CGRIDVIEWDATA_alignment),

  GB_END_DECLARE
};

GB_DESC CGridViewColumnDesc[] =
{
  GB_DECLARE(".GridViewColumn", 0), GB_VIRTUAL_CLASS(),
  GB_PROPERTY("Resizable","b",CGRIDVIEW_column_resizable),
  GB_PROPERTY("Width", "i", CGRIDVIEW_column_width),
  GB_PROPERTY("W", "i", CGRIDVIEW_column_width),
  GB_PROPERTY("HeaderText","s",CGRIDVIEW_column_headertext),
  GB_PROPERTY("FooterText","s",CGRIDVIEW_column_footertext),
  GB_END_DECLARE
};

GB_DESC CGridViewColumnsDesc[] =
{
  GB_DECLARE(".GridColumns", 0), GB_VIRTUAL_CLASS(),
  GB_METHOD("_get", ".GridViewColumn", CGRIDVIEW_columns_get, "(Column)i"),
  GB_PROPERTY("Resizable","b",CGRIDVIEW_columns_resizable),
  GB_PROPERTY("Count", "i", CGRIDVIEW_columns_count),
  GB_PROPERTY("Width", "i", CGRIDVIEW_columns_width),
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
  GB_END_DECLARE
};

GB_DESC CGridViewRowsDesc[] =
{
  GB_DECLARE(".GridRows", 0), GB_VIRTUAL_CLASS(),
  GB_METHOD("_get", ".GridViewRow", CGRIDVIEW_rows_get, "(Row)i"),
  GB_PROPERTY("Count", "i", CGRIDVIEW_count),
  GB_PROPERTY("Height", "i", CGRIDVIEW_rows_height),
  GB_PROPERTY("Resizable","b",CGRIDVIEW_rows_resizable),
  GB_END_DECLARE
};

GB_DESC CGridViewDesc[] =
{
  GB_DECLARE("GridView", sizeof(CGRIDVIEW)), GB_INHERITS("Control"),

  GB_CONSTANT("_Properties", "s", CGRIDVIEW_PROPERTIES),
  GB_CONSTANT("_DefaultEvent", "s", "Click"),

  GB_METHOD("_new", NULL, CGRIDVIEW_new, "(Parent)Container;"),
  GB_METHOD("_get", ".GridCell", CGRIDVIEW_get, "(Row)i(Column)i"),
  GB_METHOD("Clear", NULL, CGRIDVIEW_clear, NULL),
  GB_METHOD("Find", "b", CGRIDVIEW_find, "(X)i(Y)i"),
  GB_METHOD("RowAt","i",CGRIDVIEW_rowat,"(Y)i"),
  GB_METHOD("ColumnAt","i",CGRIDVIEW_colat,"(X)i"),

  GB_PROPERTY("Border", "b", CGRIDVIEW_border),
  GB_PROPERTY("Mode","i",CGRIDVIEW_mode),
  GB_PROPERTY("ScrollBar", "i<Scroll>", CGRIDVIEW_scrollbar),
  GB_PROPERTY("ScrollX", "i", CGRIDVIEW_scrollX),
  GB_PROPERTY("ScrollY", "i", CGRIDVIEW_scrollY),
  GB_PROPERTY("Grid", "b", CGRIDVIEW_grid),
  GB_PROPERTY("Row", "i", CGRIDVIEW_row),
  GB_PROPERTY("Column", "i", CGRIDVIEW_column),
  GB_PROPERTY("Header", "b", CGRIDVIEWHEADER_visible),
  GB_PROPERTY("Footer", "b", CGRIDVIEWFOOTER_visible),
  GB_PROPERTY("RowSeparator", "b", CGRIDVIEWROWSEP_visible),
  GB_PROPERTY("Data", ".GridData",CGRIDVIEW_data),
  GB_PROPERTY_SELF("Columns", ".GridColumns"),
  GB_PROPERTY_SELF("Rows", ".GridRows"),
  GB_PROPERTY_READ("Current", ".GridCell", CGRIDVIEW_current),
  GB_PROPERTY_READ("Item", ".GridCell", CGRIDVIEW_item),
  GB_PROPERTY_READ("ClientX", "i", CGRIDVIEW_client_x),
  GB_PROPERTY_READ("ClientY", "i",  CGRIDVIEW_client_y),
  GB_PROPERTY_READ("ClientWidth", "i", CGRIDVIEW_client_width),
  GB_PROPERTY_READ("ClientW", "i",  CGRIDVIEW_client_width),
  GB_PROPERTY_READ("ClientHeight", "i", CGRIDVIEW_client_height),
  GB_PROPERTY_READ("ClientH", "i", CGRIDVIEW_client_height),

  GB_EVENT("Activate", NULL, NULL, &EVENT_Activate),
  GB_EVENT("Click", NULL, NULL, &EVENT_Click),
  GB_EVENT("RowClick",NULL,"(Row)i",&EVENT_RowClick),
  GB_EVENT("ColumnClick",NULL,"(Column)i",&EVENT_ColumnClick),
  GB_EVENT("FooterClick",NULL,"(Column)i",&EVENT_FooterClick),
  GB_EVENT("Scroll",NULL,NULL,&EVENT_Scroll),
  GB_EVENT("Data",NULL,"(Row)i(Column)i",&EVENT_Data),

  GB_END_DECLARE
};
