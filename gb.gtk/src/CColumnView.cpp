/***************************************************************************

  CColumnView.cpp

  (c) 2004-2005 - Daniel Campos Fernández <dcamposf@gmail.com>
  
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


#define __CCOLUMNVIEW_CPP

#include <stdlib.h>

#include <stdio.h>

#include "gambas.h"
#include "main.h"
#include "widgets.h"

#include "CWidget.h"
#include "CContainer.h"
#include "CPicture.h"
#include "CColumnView.h"

DECLARE_EVENT(EVENT_Select);    /* selection change */
DECLARE_EVENT(EVENT_Click);     /* simple click */
DECLARE_EVENT(EVENT_Activate);  /* double click */
DECLARE_EVENT(EVENT_Rename);    /* double click */
DECLARE_EVENT(EVENT_Expand);
DECLARE_EVENT(EVENT_Collapse);
DECLARE_EVENT(EVENT_Compare);
DECLARE_EVENT(EVENT_ColumnClick);     /* simple click */


void gb_raise_colview_Click(gControl *sender)
{
	CWIDGET *_ob=GetObject(sender);
	CCOLUMNVIEW *_object=(CCOLUMNVIEW*)_ob;
	
	if (!_ob) return;
	THIS->cursor=COLUMNVIEW->getCurrentCursor();
	GB.Raise((void*)_ob,EVENT_Click,0);
}

void gb_raise_colview_Activate(gControl *sender)
{
	CWIDGET *_ob=GetObject(sender);
	
	if (!_ob) return;
	GB.Raise((void*)_ob,EVENT_Activate,0);
}


/*******************************************

 ColumnViewItem
 
 *******************************************/
BEGIN_PROPERTY(CCOLUMNVIEWITEM_key)

	char *buf;
	
	buf=COLUMNVIEW->key(THIS->index);
	GB.ReturnNewString(buf,0);
	if (buf) g_free(buf);

END_PROPERTY


BEGIN_PROPERTY(CCOLUMNVIEWITEM_picture)

	CPICTURE *pic=NULL;
	gPicture *hPic=NULL;

	if (READ_PROPERTY)
	{
		hPic=COLUMNVIEW->itemPicture(THIS->index);
		if (hPic)
		{
			GB.New((void **)&pic, GB.FindClass("Picture"), 0, 0);
			if (pic->picture) pic->picture->unref();
			pic->picture=hPic;
			GB.ReturnObject((void*)pic);
		}
		GB.ReturnObject(pic);
		return;
	}
	
	pic=(CPICTURE*)VPROP(GB_OBJECT);
	if (!pic)
		COLUMNVIEW->setItemPicture(THIS->index,NULL);
	else
		COLUMNVIEW->setItemPicture(THIS->index,pic->picture);

END_PROPERTY

BEGIN_PROPERTY(CCOLUMNVIEWITEM_selected)

	stub("CCOLUMNVIEWITEM_selected");
	if (READ_PROPERTY) GB.ReturnBoolean(false);

END_PROPERTY

BEGIN_PROPERTY(CCOLUMNVIEWITEM_text)

	char *buf;
	
	buf=COLUMNVIEW->itemText(THIS->index,0);
	GB.ReturnNewString(buf,0);
	if (buf) g_free(buf);

END_PROPERTY

BEGIN_PROPERTY(CCOLUMNVIEWITEM_expanded)

	if (READ_PROPERTY) { GB.ReturnBoolean(COLUMNVIEW->expanded(THIS->index)); return; }
	COLUMNVIEW->setExpanded(THIS->index,VPROP(GB_BOOLEAN));

END_PROPERTY

BEGIN_PROPERTY(CCOLUMNVIEWITEM_count)

	GB.ReturnInteger(COLUMNVIEW->itemCount(THIS->index));

END_PROPERTY


BEGIN_PROPERTY(CCOLUMNVIEWITEM_editable)

	stub("CCOLUMNVIEWITEM_editable");
	if (READ_PROPERTY) GB.ReturnBoolean(false);

END_PROPERTY

BEGIN_METHOD_VOID(CCOLUMNVIEWITEM_ensure_visible)

	stub("CCOLUMNVIEWITEM_ensure_visible");

END_METHOD

BEGIN_METHOD_VOID(CCOLUMNVIEWITEM_delete)

	COLUMNVIEW->remove(THIS->index);

END_METHOD

BEGIN_METHOD_VOID(CCOLUMNVIEWITEM_clear)

	COLUMNVIEW->itemClear(THIS->index);

END_METHOD

BEGIN_METHOD(CCOLUMNVIEWITEM_get, GB_INTEGER Column;)

	char *buf;
	
	buf=COLUMNVIEW->itemText(THIS->index,VARG(Column));
	GB.ReturnNewString(buf,0);
	if (buf) g_free(buf);

END_METHOD

BEGIN_METHOD(CCOLUMNVIEWITEM_put, GB_STRING Text; GB_INTEGER Column;)

	char *buf=GB.ToZeroString(ARG(Text));
	
	COLUMNVIEW->setItemText(THIS->index,VARG(Column),buf);

END_METHOD
  
BEGIN_METHOD_VOID(CCOLUMNVIEWITEM_rename)

	stub("CCOLUMNVIEWITEM_rename");

END_METHOD

/*************************************************

 ColumnViewColumn
 
**************************************************/
BEGIN_PROPERTY(CCOLUMNVIEW_column_text)

	char *buf;
	
	if (READ_PROPERTY)
	{
		buf=COLUMNVIEW->columnText(THIS->colIndex);
		GB.ReturnNewString(buf,0);
		if (buf) g_free(buf);
		return;
	}
	
	COLUMNVIEW->setColumnText(THIS->colIndex,GB.ToZeroString(PROP(GB_STRING)));

END_PROPERTY

BEGIN_PROPERTY(CCOLUMNVIEW_column_alignment)

	if (READ_PROPERTY) { GB.ReturnInteger(COLUMNVIEW->columnAlignment(THIS->colIndex)); return; }
	COLUMNVIEW->setColumnAlignment(THIS->colIndex,VPROP(GB_INTEGER));

END_PROPERTY

BEGIN_PROPERTY(CCOLUMNVIEW_column_width)

	if (READ_PROPERTY) { GB.ReturnInteger(COLUMNVIEW->columnWidth(THIS->colIndex)); return; }
	COLUMNVIEW->setColumnWidth(THIS->colIndex,VPROP(GB_INTEGER));

END_PROPERTY

BEGIN_PROPERTY(CCOLUMNVIEW_column_auto_resize)

	stub("CCOLUMNVIEW_column_auto_resize");
	if (READ_PROPERTY) GB.ReturnBoolean(false);

END_PROPERTY

/*************************************************

 ColumnViewColumns
 
**************************************************/

BEGIN_METHOD(CCOLUMNVIEW_columns_get,GB_INTEGER Column;)

	if ( (VARG(Column)<0) || (VARG(Column)>=COLUMNVIEW->columnsCount()) )
	{
		GB.Error("Bad column index");
		GB.ReturnNull();
	}
	
	THIS->colIndex=VARG(Column);
	RETURN_SELF();

END_METHOD

BEGIN_PROPERTY(CCOLUMNVIEW_columns_count)

	if (READ_PROPERTY) { GB.ReturnInteger(COLUMNVIEW->columnsCount()); return; }
	
	if ( (VPROP(GB_INTEGER)<1) || (VPROP(GB_INTEGER)>64) ) 
	{ 
		GB.Error("Bad number of columns"); 
		return; 
	}
	
	COLUMNVIEW->setColumnsCount(VPROP(GB_INTEGER));

END_PROPERTY

BEGIN_PROPERTY(CCOLUMNVIEW_columns_sort)

END_PROPERTY

BEGIN_PROPERTY(CCOLUMNVIEW_columns_ascending)

END_PROPERTY


/******************************************************

 ColumnView
 
*******************************************************/
BEGIN_PROPERTY(CCOLUMNVIEW_mode)

END_PROPERTY

BEGIN_PROPERTY(CCOLUMNVIEW_show_root)

END_PROPERTY

BEGIN_PROPERTY(CCOLUMNVIEW_sorted)

END_PROPERTY

BEGIN_PROPERTY(CCOLUMNVIEW_editable)

END_PROPERTY

BEGIN_PROPERTY(CCOLUMNVIEW_scrollbar)

	if (READ_PROPERTY) { GB.ReturnInteger(COLUMNVIEW->scrollBars()); return; }
	COLUMNVIEW->setScrollBars(VPROP(GB_INTEGER));

END_PROPERTY

BEGIN_PROPERTY(CCOLUMNVIEW_border_simple)

END_PROPERTY

BEGIN_PROPERTY(CCOLUMNVIEW_count)

	GB.ReturnInteger(COLUMNVIEW->count());

END_PROPERTY

BEGIN_PROPERTY(CCOLUMNVIEW_available)

	GB.ReturnBoolean( (THIS->cursor!=-1) );

END_PROPERTY

BEGIN_PROPERTY(CCOLUMNVIEW_resizable)

	if (READ_PROPERTY) { GB.ReturnBoolean(COLUMNVIEW->resizable()); return; }
	COLUMNVIEW->setResizable(VPROP(GB_BOOLEAN));

END_PROPERTY

BEGIN_PROPERTY(CCOLUMNVIEW_auto_resize)

END_PROPERTY

BEGIN_PROPERTY(CCOLUMNVIEW_header)

	if (READ_PROPERTY) { GB.ReturnBoolean(COLUMNVIEW->header()); return; }
	COLUMNVIEW->setHeader(VPROP(GB_BOOLEAN));

END_PROPERTY

BEGIN_PROPERTY(CCOLUMNVIEW_client_width)

END_PROPERTY

BEGIN_PROPERTY(CCOLUMNVIEW_client_height)

END_PROPERTY

BEGIN_PROPERTY(CCOLUMNVIEW_columns)

	RETURN_SELF();

END_PROPERTY

BEGIN_PROPERTY(CCOLUMNVIEW_current)

	THIS->index=COLUMNVIEW->getCurrentCursor();
	if (THIS->index==-1) 
		GB.ReturnNull();
	else
		RETURN_SELF();

END_PROPERTY

BEGIN_PROPERTY(CCOLUMNVIEW_key)

END_PROPERTY

BEGIN_PROPERTY(CCOLUMNVIEW_item)

	if (THIS->cursor==-1) { GB.ReturnNull(); return; }
	THIS->index=THIS->cursor;
	RETURN_SELF();

END_PROPERTY

BEGIN_METHOD_VOID(CCOLUMNVIEW_move_current)

	long vl;

	vl=COLUMNVIEW->getCurrentCursor();
	if (vl==-1) { GB.ReturnBoolean(TRUE); return; }
	THIS->cursor=vl;
	GB.ReturnBoolean(FALSE);

END_METHOD

BEGIN_METHOD_VOID(CCOLUMNVIEW_first)

	THIS->cursor=COLUMNVIEW->getFirstIndex();
	GB.ReturnBoolean( (THIS->cursor == -1) );

END_METHOD

BEGIN_METHOD_VOID(CCOLUMNVIEW_last)  

	THIS->cursor=COLUMNVIEW->getLastIndex();
	GB.ReturnBoolean( (THIS->cursor == -1) );

END_METHOD

BEGIN_METHOD_VOID(CCOLUMNVIEW_next)  

	THIS->cursor=COLUMNVIEW->getNextIndex(THIS->cursor);
	GB.ReturnBoolean( (THIS->cursor == -1) );

END_METHOD

BEGIN_METHOD_VOID(CCOLUMNVIEW_previous)  

	THIS->cursor=COLUMNVIEW->getPreviousIndex(THIS->cursor);
	GB.ReturnBoolean( (THIS->cursor == -1) );

END_METHOD

BEGIN_METHOD_VOID(CCOLUMNVIEW_child)  

	THIS->cursor=COLUMNVIEW->getChildIndex(THIS->cursor);
	GB.ReturnBoolean( (THIS->cursor == -1) );

END_METHOD

BEGIN_METHOD_VOID(CCOLUMNVIEW_parent)  

	THIS->cursor=COLUMNVIEW->getParentIndex(THIS->cursor);
	GB.ReturnBoolean( (THIS->cursor == -1) );

END_METHOD

BEGIN_METHOD_VOID(CCOLUMNVIEW_above)  

END_METHOD

BEGIN_METHOD_VOID(CCOLUMNVIEW_below)  

END_METHOD

BEGIN_METHOD_VOID(CCOLUMNVIEW_back)  

END_METHOD

BEGIN_METHOD_VOID(CCOLUMNVIEW_clear)

	while (COLUMNVIEW->count()) COLUMNVIEW->remove(0);

END_METHOD


BEGIN_METHOD(CCOLUMNVIEW_new, GB_OBJECT Parent;)

	CCONTAINER *Parent=(CCONTAINER*)VPROP(GB_OBJECT);
	Parent=(CCONTAINER*)GetContainer ((CWIDGET*)Parent);

	THIS->cursor=-1;
	THIS->widget=new gColumnView(Parent->widget);
	InitControl(THIS->widget,(CWIDGET*)THIS);
	COLUMNVIEW->onClick=gb_raise_colview_Click;
	COLUMNVIEW->onActivate=gb_raise_colview_Activate;

END_METHOD
    
BEGIN_METHOD(CCOLUMNVIEW_move_to, GB_STRING Key;)

	char *key=GB.ToZeroString(ARG(Key));
	THIS->cursor=COLUMNVIEW->keyIndex(key);

	if (THIS->cursor == -1) GB.ReturnBoolean(TRUE);
	else                    GB.ReturnBoolean(FALSE);

END_METHOD

BEGIN_METHOD(CCOLUMNVIEW_get, GB_STRING Key;)

	char *key=GB.ToZeroString(ARG(Key));
	THIS->index=COLUMNVIEW->keyIndex(key);
	if (THIS->index != -1) 
	{
		RETURN_SELF();
		return;
	}
	
	GB.Error("Unknown item : &1",key);

END_METHOD

BEGIN_METHOD(CCOLUMNVIEW_add,GB_STRING key;GB_STRING text;GB_OBJECT picture;GB_STRING parent;GB_STRING after;)

	gPicture *pic=NULL;
	char *after=NULL;
	char *parent=NULL;
	
	if (!LENGTH(key)) { GB.Error("Null key"); return; }
	
	if (!MISSING(picture))
	{
		if ( VARG(picture) )
			pic=((CPICTURE*)VARG(picture))->picture;
	}
	if (!MISSING(parent)) parent=GB.ToZeroString(ARG(parent));
	if (!MISSING(after)) after=GB.ToZeroString(ARG(after));
	
	switch (COLUMNVIEW->add(GB.ToZeroString(ARG(key)),GB.ToZeroString(ARG(text)),pic,after,parent))
	{
		case  0: break;
		case -1:
			GB.Error ("Key already used: &1",GB.ToZeroString(ARG(key))); break;
		case -2:
			GB.Error ("Parent does not exist");break;
	}
	
END_METHOD

BEGIN_METHOD(CCOLUMNVIEW_remove, GB_STRING Key;)

	char *key=GB.ToZeroString(ARG(Key));
	long index=COLUMNVIEW->keyIndex(key);

	if (index != -1) 
	{
		COLUMNVIEW->remove(index);
		return;
	}
	
	GB.Error("Unknown item : &1",key);

END_METHOD

BEGIN_METHOD(CCOLUMNVIEW_exist, GB_STRING Key;)

	char *key=GB.ToZeroString(ARG(Key));
	long index=COLUMNVIEW->keyIndex(key);

	if (index == -1) GB.ReturnBoolean(FALSE);
	else             GB.ReturnBoolean(TRUE);

END_METHOD

BEGIN_METHOD(CCOLUMNVIEW_find, GB_INTEGER X; GB_INTEGER Y;)

	THIS->cursor=COLUMNVIEW->findAt(VARG(X),VARG(Y));
	GB.ReturnBoolean( (THIS->cursor == -1) );

END_METHOD

BEGIN_PROPERTY(CCOLUMNVIEW_compare)

  stub("CCOLUMNVIEW_compare");
  if (READ_PROPERTY) GB.ReturnInteger(0);

END_PROPERTY

/***************************************************************************

  Gambas Interfaces

***************************************************************************/

GB_DESC CColumnViewColumnDesc[] =
{
  GB_DECLARE(".ColumnViewColumn", 0), GB_VIRTUAL_CLASS(),

  GB_PROPERTY("Text", "s", CCOLUMNVIEW_column_text),
  GB_PROPERTY("Alignment", "i<Align,Left,Center,Right>", CCOLUMNVIEW_column_alignment),
  GB_PROPERTY("Width", "i", CCOLUMNVIEW_column_width),
  GB_PROPERTY("AutoResize", "b", CCOLUMNVIEW_column_auto_resize),

  GB_END_DECLARE
};

GB_DESC CColumnViewColumnsDesc[] =
{
  GB_DECLARE(".ColumnViewColumns", 0), GB_VIRTUAL_CLASS(),

  GB_METHOD("_get", ".ColumnViewColumn", CCOLUMNVIEW_columns_get, "(Column)i"),

  GB_PROPERTY("Count", "i", CCOLUMNVIEW_columns_count),
  GB_PROPERTY("Sort", "i", CCOLUMNVIEW_columns_sort),
  GB_PROPERTY("Ascending", "b", CCOLUMNVIEW_columns_ascending),


  GB_END_DECLARE
};

GB_DESC CColumnViewItemDesc[] =
{
  GB_DECLARE(".ColumnViewItem", 0), GB_VIRTUAL_CLASS(),

  GB_PROPERTY_READ("Key", "s", CCOLUMNVIEWITEM_key),
  GB_PROPERTY("Picture", "Picture", CCOLUMNVIEWITEM_picture),
  GB_PROPERTY("Selected", "b", CCOLUMNVIEWITEM_selected),
  GB_PROPERTY("Text", "s", CCOLUMNVIEWITEM_text),
  GB_METHOD("EnsureVisible", NULL, CCOLUMNVIEWITEM_ensure_visible, NULL),
  GB_METHOD("Delete", NULL, CCOLUMNVIEWITEM_delete, NULL),
  GB_METHOD("Clear", NULL, CCOLUMNVIEWITEM_clear, NULL),

  GB_PROPERTY("Expanded", "b", CCOLUMNVIEWITEM_expanded),
  GB_PROPERTY_READ("Children", "i", CCOLUMNVIEWITEM_count),
  GB_PROPERTY_READ("Count", "i", CCOLUMNVIEWITEM_count),

  GB_METHOD("MoveNext", "b", CCOLUMNVIEW_next, NULL),
  GB_METHOD("MovePrevious", "b", CCOLUMNVIEW_previous, NULL),
  GB_METHOD("MoveChild", "b", CCOLUMNVIEW_child, NULL),
  GB_METHOD("MoveParent", "b", CCOLUMNVIEW_parent, NULL),
  GB_METHOD("MoveAbove", "b", CCOLUMNVIEW_above, NULL),
  GB_METHOD("MoveBelow", "b", CCOLUMNVIEW_below, NULL),

  GB_METHOD("_get", "s", CCOLUMNVIEWITEM_get, "(Column)i"),
  GB_METHOD("_put", NULL, CCOLUMNVIEWITEM_put, "(Text)s(Column)i"),

  GB_PROPERTY("Editable", "b", CCOLUMNVIEWITEM_editable),
  GB_METHOD("Rename", NULL, CCOLUMNVIEWITEM_rename, NULL),
  
  GB_END_DECLARE
};

GB_DESC CColumnViewDesc[] =
{
  GB_DECLARE("ColumnView", sizeof(CCOLUMNVIEW)), GB_INHERITS("Control"),

  GB_METHOD("_new", NULL, CCOLUMNVIEW_new, "(Parent)Container;"),

  GB_PROPERTY("Mode", "i", CCOLUMNVIEW_mode),
  GB_PROPERTY("Root", "b", CCOLUMNVIEW_show_root),
  GB_PROPERTY("Sorted", "b", CCOLUMNVIEW_sorted),
  GB_PROPERTY("Editable", "b", CCOLUMNVIEW_editable),
  GB_PROPERTY("ScrollBar", "i<Scroll>", CCOLUMNVIEW_scrollbar),
  GB_PROPERTY("Border", "b", CCOLUMNVIEW_border_simple),

  GB_PROPERTY_READ("Count", "i", CCOLUMNVIEW_count),

  GB_PROPERTY_READ("Available", "b", CCOLUMNVIEW_available),
  GB_METHOD("MoveCurrent", "b", CCOLUMNVIEW_move_current, NULL),
  GB_METHOD("MoveTo", "b", CCOLUMNVIEW_move_to, "(Key)s"),
  GB_METHOD("MoveFirst", "b", CCOLUMNVIEW_first, NULL),
  GB_METHOD("MoveLast", "b", CCOLUMNVIEW_last, NULL),
  GB_METHOD("MoveNext", "b", CCOLUMNVIEW_next, NULL),
  GB_METHOD("MovePrevious", "b", CCOLUMNVIEW_previous, NULL),
  GB_METHOD("MoveChild", "b", CCOLUMNVIEW_child, NULL),
  GB_METHOD("MoveParent", "b", CCOLUMNVIEW_parent, NULL),
  GB_METHOD("MoveAbove", "b", CCOLUMNVIEW_above, NULL),
  GB_METHOD("MoveBelow", "b", CCOLUMNVIEW_below, NULL),
  GB_METHOD("MoveBack", "b", CCOLUMNVIEW_back, NULL),

  GB_METHOD("_get", ".ColumnViewItem", CCOLUMNVIEW_get, "(Key)s"),

  GB_METHOD("Clear", NULL, CCOLUMNVIEW_clear, NULL),
  GB_METHOD("Add", ".ColumnViewItem", CCOLUMNVIEW_add, "(Key)s(Text)s[(Picture)Picture;(Parent)s(After)s]"),
  GB_METHOD("Remove", NULL, CCOLUMNVIEW_remove, "(Key)s"),
  GB_METHOD("Exist", "b", CCOLUMNVIEW_exist, "(Key)s"),
  GB_METHOD("Find", "b", CCOLUMNVIEW_find, "(X)i(Y)i"),

  GB_PROPERTY_READ("Current", ".ColumnViewItem", CCOLUMNVIEW_current),
  GB_PROPERTY_READ("Key", "s", CCOLUMNVIEW_key),
  GB_PROPERTY_READ("Item", ".ColumnViewItem", CCOLUMNVIEW_item),

  GB_EVENT("Select", NULL, NULL, &EVENT_Select),
  GB_EVENT("Activate", NULL, NULL, &EVENT_Activate),
  GB_EVENT("Click", NULL, NULL, &EVENT_Click),
  GB_EVENT("Rename", NULL, NULL, &EVENT_Rename),
  GB_EVENT("Expand", NULL, NULL, &EVENT_Expand),
  GB_EVENT("Collapse", NULL, NULL, &EVENT_Collapse),
  GB_EVENT("ColumnClick", NULL, "(Column)i", &EVENT_ColumnClick),
  GB_EVENT("Compare", NULL, "(Key)s(OtherKey)s", &EVENT_Compare),

  GB_PROPERTY_READ("Columns", ".ColumnViewColumns", CCOLUMNVIEW_columns),

  GB_PROPERTY("Resizable", "b", CCOLUMNVIEW_resizable),
  GB_PROPERTY("AutoResize", "b", CCOLUMNVIEW_auto_resize),
  GB_PROPERTY("Header", "b", CCOLUMNVIEW_header),
  GB_PROPERTY("Compare", "i", CCOLUMNVIEW_compare),

  GB_PROPERTY_READ("ClientWidth", "i", CCOLUMNVIEW_client_width),
  GB_PROPERTY_READ("ClientW", "i",  CCOLUMNVIEW_client_width),
  GB_PROPERTY_READ("ClientHeight", "i", CCOLUMNVIEW_client_height),
  GB_PROPERTY_READ("ClientH", "i", CCOLUMNVIEW_client_height),

  GB_CONSTANT("_Properties", "s", CCOLUMNVIEW_PROPERTIES),
  GB_CONSTANT("_DefaultEvent", "s", "Click"),

  GB_END_DECLARE
};


