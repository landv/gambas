/***************************************************************************

  CListView.cpp

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


#define __CLISTVIEW_CPP

#include <stdlib.h>

#include "gambas.h"
#include "main.h"
#include "widgets.h"

#include "CWidget.h"
#include "CContainer.h"
#include "CPicture.h"
#include "CListView.h"

#include <stdio.h>
#include <string.h>

DECLARE_EVENT(EVENT_Select);    /* selection change */
DECLARE_EVENT(EVENT_Click);     /* simple click */
DECLARE_EVENT(EVENT_Activate);  /* double click */
DECLARE_EVENT(EVENT_Rename);  /* double click */
DECLARE_EVENT(EVENT_Compare);  


/***************************************************************************

  TreeView

***************************************************************************/
void gb_raise_listview_Activate(gListView *sender)
{
	CWIDGET *_ob=GetObject((gControl*)sender);
	
	if (!_ob) return;
	GB.Raise((void*)_ob,EVENT_Activate,0);
}

void gb_raise_listview_Select(gListView *sender)
{
	CWIDGET *_ob=GetObject((gControl*)sender);
	
	if (!_ob) return;
	GB.Raise((void*)_ob,EVENT_Select,0);
}


BEGIN_METHOD(CLISTVIEW_new, GB_OBJECT parent)

	CCONTAINER *Parent=(CCONTAINER*)VPROP(GB_OBJECT);
	Parent=(CCONTAINER*)GetContainer ((CWIDGET*)Parent);

	THIS->widget=new gListView(Parent->widget);
	InitControl(THIS->widget,(CWIDGET*)THIS);
	LISTVIEW->onActivate=gb_raise_listview_Activate;
	LISTVIEW->onSelect=gb_raise_listview_Select;
	
END_METHOD

BEGIN_METHOD_VOID(CLISTVIEW_free)

	if (THIS->icursor) GB.FreeString(&THIS->icursor);
	if (THIS->item) GB.FreeString(&THIS->item);
	
END_METHOD

BEGIN_METHOD_VOID(CLISTVIEW_clear)

	LISTVIEW->clear();

END_METHOD



BEGIN_METHOD(CLISTVIEW_add, GB_STRING key; GB_STRING text; GB_OBJECT picture; GB_STRING after;)

	char *text="";
	char *after=NULL;
	gPicture *pic=NULL;
	
	if (LENGTH(text)) text=STRING(text);

	if (!LENGTH(key))
	{
		GB.Error("Null key");
		return;
	}
	
	if (!MISSING(after))
	{
		if (LENGTH(after))
		{
			after=STRING(after);
			if (!LISTVIEW->exists(after))
			{
				GB.Error("After item does not exist");
				return;
			}
		}
	}
	
	if (!MISSING(picture))
	{
		if (VARG(picture))
		{
			pic=((CPICTURE*)VARG(picture))->picture;
		}
	}

	if (!LISTVIEW->add (STRING(key),text,pic,after))
	{
		GB.Error("Key already used: '&1'",STRING(key));
		return;
	}

	if (THIS->item) GB.FreeString(&THIS->item);
	GB.NewString(&THIS->item,STRING(key),LENGTH(key));
	

END_METHOD


BEGIN_METHOD(CLISTVIEW_remove, GB_STRING key)

	char *key="";
	
	if (LENGTH(key)) key=STRING(key);
	if (!LISTVIEW->remove(key))
	{
		GB.Error("Unknown item: '&1'" , key);
		return;
	}
	
	if (THIS->item)
		if (!strcasecmp(STRING(key),THIS->item) )
			GB.FreeString(&THIS->item);
	
END_METHOD


BEGIN_METHOD(CLISTVIEW_exist, GB_STRING key)

	if (!LENGTH(key)) { GB.ReturnBoolean(false); return; }
	GB.ReturnBoolean( LISTVIEW->exists(STRING(key)) );

END_METHOD


BEGIN_METHOD(CLISTVIEW_find, GB_INTEGER x; GB_INTEGER y)

	char *vl;	

	if (THIS->item) GB.FreeString(&THIS->item);
	vl=LISTVIEW->find(VARG(x),VARG(y));
	if (!vl) { GB.ReturnBoolean(true); return; }
	GB.ReturnBoolean(false);
	GB.NewString(&THIS->item,vl,0);
	free(vl);

END_METHOD


BEGIN_METHOD(CLISTVIEW_get, GB_STRING key)

	if (!LENGTH(key))
	{
		GB.Error ("Unknown item: ''");
		GB.ReturnNull();
		return;
	}
	
	if (!LISTVIEW->exists(STRING(key)) )
	{
		GB.Error ("Unknown item: '&1'",STRING(key));
		GB.ReturnNull();
		return;
	}
	
	if (THIS->icursor) GB.FreeString(&THIS->icursor);
	GB.NewString (&THIS->icursor,STRING(key),LENGTH(key));
	RETURN_SELF();

END_METHOD


BEGIN_PROPERTY(CLISTVIEW_mode)

	if (READ_PROPERTY) { GB.ReturnInteger(LISTVIEW->mode()); return; }
	LISTVIEW->setMode(VPROP(GB_INTEGER));

END_PROPERTY


BEGIN_PROPERTY(CLISTVIEW_current)

	char *key=LISTVIEW->current();
	
	if (THIS->item) GB.FreeString(&THIS->item); 
	if (!key) { GB.ReturnNull(); return; }
	GB.NewString(&THIS->item,key,0);
	free(key);
	RETURN_SELF();

END_PROPERTY


BEGIN_PROPERTY(CLISTVIEW_key)

	GB.ReturnNewString(THIS->item,0);

END_PROPERTY


BEGIN_PROPERTY(CLISTVIEW_item)

	if (!THIS->item) GB.ReturnNull();
	else {
		if (THIS->icursor) GB.FreeString(&THIS->icursor);
		GB.NewString(&THIS->icursor,THIS->item,0);
		RETURN_SELF();
	}

END_PROPERTY


BEGIN_PROPERTY(CLISTVIEW_available)

	GB.ReturnBoolean((bool)THIS->item);

END_PROPERTY



BEGIN_PROPERTY(CLISTVIEW_count)

	GB.ReturnInteger((long)LISTVIEW->count());

END_PROPERTY


BEGIN_METHOD(CLISTVIEW_move_to, GB_STRING key)

	if (LENGTH(key))
	{
		if (LISTVIEW->exists(STRING(key)))
		{
			if (THIS->item) GB.FreeString(&THIS->item);
			GB.NewString(&THIS->item,STRING(key),LENGTH(key));
			GB.ReturnBoolean(false);
		}
	}
	
	if (THIS->item) GB.FreeString(&THIS->item);
	GB.ReturnBoolean(true);
		

END_METHOD


BEGIN_METHOD_VOID(CLISTVIEW_move_current)

	char *key=LISTVIEW->current();
	
	if (THIS->item) GB.FreeString(&THIS->item); 
	if (!key) { GB.ReturnBoolean(true); return; }
	GB.NewString(&THIS->item,key,0);
	free(key);
	GB.ReturnBoolean(false);
	
END_PROPERTY


BEGIN_METHOD_VOID(CLISTVIEW_first)

	char *vl;
	
	if (THIS->item) GB.FreeString(&THIS->item);
	vl=LISTVIEW->firstItem();
	if (!vl)
	{
		GB.ReturnBoolean(true);
		return;
	}
	GB.NewString(&THIS->item,vl,0);
	GB.ReturnBoolean(false);
	free(vl);	

END_METHOD

BEGIN_METHOD_VOID(CLISTVIEW_last)

	char *vl;
	
	if (THIS->item) GB.FreeString(&THIS->item);
	vl=LISTVIEW->lastItem();
	if (!vl)
	{
		GB.ReturnBoolean(true);
		return;
	}
	GB.NewString(&THIS->item,vl,0);
	GB.ReturnBoolean(false);
	free(vl);

END_METHOD


BEGIN_METHOD_VOID(CLISTVIEWITEM_previous)

	char *vl;
	
	if (THIS->item)
	{
		vl=LISTVIEW->prevItem(THIS->item);
		if (vl)
		{
			if (THIS->item) GB.FreeString(&THIS->item);
			GB.NewString(&THIS->item,vl,0);
			GB.ReturnBoolean(false);
			free(vl);
			return;
		}
		GB.FreeString(&THIS->item);
	}
	
	GB.ReturnBoolean(true);
	

END_METHOD

BEGIN_METHOD_VOID(CLISTVIEWITEM_next)

	char *vl;
	
	if (THIS->item)
	{
		vl=LISTVIEW->nextItem(THIS->item);
		if (vl)
		{
			if (THIS->item) GB.FreeString(&THIS->item);
			GB.NewString(&THIS->item,vl,0);
			GB.ReturnBoolean(false);
			free(vl);
			return;
		}
		GB.FreeString(&THIS->item);
	}
	
	GB.ReturnBoolean(true);

END_METHOD


BEGIN_PROPERTY(CLISTVIEWITEM_key)

	GB.ReturnNewString(THIS->icursor,0);

END_PROPERTY


BEGIN_PROPERTY(CLISTVIEWITEM_picture)

	CPICTURE *Pic=NULL;
	gPicture *pic=NULL;
	
	
	if (READ_PROPERTY)
	{
		pic=LISTVIEW->itemPicture(THIS->icursor);
		if (pic)
		{
			GB.New((void **)&Pic, GB.FindClass("Picture"), 0, 0);
			if (Pic->picture) Pic->picture->unref();
			Pic->picture=pic;
		}
		GB.ReturnObject((void*)Pic);
	}
	
	Pic=(CPICTURE*)VPROP(GB_OBJECT);
	if (Pic) pic=Pic->picture;
	
	LISTVIEW->itemSetPicture(THIS->icursor,pic);

END_PROPERTY


BEGIN_PROPERTY(CLISTVIEWITEM_selected)

	if (READ_PROPERTY)
	{
		GB.ReturnBoolean(LISTVIEW->itemSelected(THIS->icursor));
		return;
	}
	
	LISTVIEW->itemSetSelected(THIS->icursor,VPROP(GB_BOOLEAN));
	
END_PROPERTY


BEGIN_PROPERTY(CLISTVIEWITEM_text)

	if (READ_PROPERTY)
	{
		GB.ReturnNewString(LISTVIEW->itemText(THIS->icursor),0);
		return;
	}
	
	LISTVIEW->itemSetText(THIS->icursor,GB.ToZeroString(PROP(GB_STRING)));
	
END_PROPERTY


BEGIN_METHOD_VOID(CLISTVIEWITEM_ensure_visible)



END_METHOD


BEGIN_METHOD_VOID(CLISTVIEWITEM_delete)

	LISTVIEW->remove(THIS->icursor);
	if (THIS->item)
		if (!strcasecmp(THIS->icursor,THIS->item) )
			GB.FreeString(&THIS->item);

END_METHOD


BEGIN_PROPERTY(CLISTVIEWITEM_editable)



END_PROPERTY


BEGIN_PROPERTY(CLISTVIEW_client_width)

	GB.ReturnInteger(LISTVIEW->visibleWidth());

END_PROPERTY


BEGIN_PROPERTY(CLISTVIEW_client_height)

	GB.ReturnInteger(LISTVIEW->visibleHeight());

END_PROPERTY

BEGIN_PROPERTY(CLISTVIEW_border_simple)

	if (READ_PROPERTY) { GB.ReturnBoolean(LISTVIEW->getBorder()); return; }
	LISTVIEW->setBorder(VPROP(GB_BOOLEAN));

END_PROPERTY

BEGIN_PROPERTY(CLISTVIEW_scrollbar)

	if (READ_PROPERTY) { GB.ReturnInteger(LISTVIEW->scrollBar()); return; }
	LISTVIEW->setScrollBar(VPROP(GB_INTEGER));

END_PROPERTY


BEGIN_PROPERTY(CLISTVIEW_compare)


END_PROPERTY

BEGIN_PROPERTY(CLISTVIEW_sorted)

	if (READ_PROPERTY) { GB.ReturnBoolean(LISTVIEW->sorted()); return; }
	LISTVIEW->setSorted(VPROP(GB_BOOLEAN));

END_PROPERTY


BEGIN_PROPERTY(CLISTVIEW_editable)



END_PROPERTY

/***************************************************************************

  ListViewItem

***************************************************************************/

GB_DESC CListViewItemDesc[] =
{
  GB_DECLARE(".ListViewItem", 0), GB_VIRTUAL_CLASS(),

  GB_PROPERTY_READ("Key", "s", CLISTVIEWITEM_key),
  GB_PROPERTY("Picture", "Picture", CLISTVIEWITEM_picture),
  GB_PROPERTY("Selected", "b", CLISTVIEWITEM_selected),
  GB_PROPERTY("Text", "s", CLISTVIEWITEM_text),
  GB_METHOD("EnsureVisible", NULL, CLISTVIEWITEM_ensure_visible, NULL),
  GB_METHOD("Delete", NULL, CLISTVIEWITEM_delete, NULL),
  GB_PROPERTY("Editable", "b", CLISTVIEWITEM_editable),

  GB_METHOD("MoveNext", "b", CLISTVIEWITEM_next, NULL),
  GB_METHOD("MovePrevious", "b", CLISTVIEWITEM_previous, NULL),
  GB_METHOD("MoveAbove", "b", CLISTVIEWITEM_previous, NULL),
  GB_METHOD("MoveBelow", "b", CLISTVIEWITEM_next, NULL),

  GB_END_DECLARE
};

/***************************************************************************

  ListView

***************************************************************************/

GB_DESC CListViewDesc[] =
{
  GB_DECLARE("ListView", sizeof(CLISTVIEW)), GB_INHERITS("Control"),


  GB_METHOD("_new", NULL, CLISTVIEW_new, "(Parent)Container;"),
  GB_METHOD("_free",NULL,CLISTVIEW_free,NULL),

  GB_PROPERTY("Mode", "i", CLISTVIEW_mode),
  GB_PROPERTY("Sorted", "b", CLISTVIEW_sorted),
  GB_PROPERTY("Editable", "b", CLISTVIEW_editable),
  GB_PROPERTY("ScrollBar", "i<Scroll>", CLISTVIEW_scrollbar),
  GB_PROPERTY("Border", "b", CLISTVIEW_border_simple),
  GB_PROPERTY("Compare", "i", CLISTVIEW_compare),

  GB_PROPERTY_READ("Count", "i", CLISTVIEW_count),

  GB_PROPERTY_READ("Available", "b", CLISTVIEW_available),
  GB_METHOD("MoveTo", "b", CLISTVIEW_move_to, "(Key)s"),
  GB_METHOD("MoveCurrent", "b", CLISTVIEW_move_current, NULL),
  GB_METHOD("MoveFirst", "b", CLISTVIEW_first, NULL),
  GB_METHOD("MoveLast", "b", CLISTVIEW_last, NULL),
  GB_METHOD("MovePrevious", "b", CLISTVIEWITEM_previous, NULL),
  GB_METHOD("MoveNext", "b", CLISTVIEWITEM_next, NULL),
  GB_METHOD("MoveAbove", "b", CLISTVIEWITEM_previous, NULL),
  GB_METHOD("MoveBelow", "b", CLISTVIEWITEM_next, NULL),

  GB_METHOD("_get", ".ListViewItem", CLISTVIEW_get, "(Key)s"),

  GB_METHOD("Clear", NULL, CLISTVIEW_clear, NULL),
  GB_METHOD("Add", ".ListViewItem", CLISTVIEW_add, "(Key)s(Text)s[(Picture)Picture;(After)s]"),
  GB_METHOD("Remove", NULL, CLISTVIEW_remove, "(Key)s"),
  GB_METHOD("Exist", "b", CLISTVIEW_exist, "(Key)s"),
  GB_METHOD("Find", "b", CLISTVIEW_find, "(X)i(Y)i"),

  GB_PROPERTY_READ("Current", ".ListViewItem", CLISTVIEW_current),
  GB_PROPERTY_READ("Key", "s", CLISTVIEW_key),
  GB_PROPERTY_READ("Item", ".ListViewItem", CLISTVIEW_item),

  GB_CONSTANT("_Properties", "s", CLISTVIEW_PROPERTIES),
  GB_CONSTANT("_DefaultEvent", "s", "Click"),

  GB_PROPERTY_READ("ClientWidth", "i", CLISTVIEW_client_width),
  GB_PROPERTY_READ("ClientW", "i",  CLISTVIEW_client_width),
  GB_PROPERTY_READ("ClientHeight", "i", CLISTVIEW_client_height),
  GB_PROPERTY_READ("ClientH", "i", CLISTVIEW_client_height),

  GB_EVENT("Select", NULL, NULL, &EVENT_Select),
  GB_EVENT("Activate", NULL, NULL, &EVENT_Activate),
  GB_EVENT("Click", NULL, NULL, &EVENT_Click),
  GB_EVENT("Rename", NULL, NULL, &EVENT_Rename),
  GB_EVENT("Compare", NULL, "(Key)s(OtherKey)s", &EVENT_Compare),

  GB_END_DECLARE
};






