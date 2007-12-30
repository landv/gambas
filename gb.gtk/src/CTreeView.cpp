/***************************************************************************

  CTreeView.cpp

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


#define __CTREEVIEW_CPP

#include <stdlib.h>

#include "gambas.h"
#include "main.h"
#include "widgets.h"

#include "CWidget.h"
#include "CContainer.h"
#include "CPicture.h"
#include "CTreeView.h"

#include <stdio.h>
#include <string.h>

DECLARE_EVENT(EVENT_Select);    /* selection change */
DECLARE_EVENT(EVENT_Click);     /* simple click */
DECLARE_EVENT(EVENT_Activate);  /* double click */
DECLARE_EVENT(EVENT_Rename);  /* double click */
DECLARE_EVENT(EVENT_Compare); 
DECLARE_EVENT(EVENT_Expand);
DECLARE_EVENT(EVENT_Collapse);


void TREEVIEW_raise_select(gTreeView *sender)
{
	CWIDGET *_ob=GetObject(sender);
	
	if (!_ob) return;
	GB.Raise((void*)_ob,EVENT_Select,0);
}

void TREEVIEW_raise_activate(gTreeView *sender)
{
	CWIDGET *_ob=GetObject(sender);
	
	if (!_ob) return;
	GB.Raise((void*)_ob,EVENT_Activate,0);
}

void TREEVIEW_raise_click(CTREEVIEW *sender)
{
	if (!sender) return;
	
	GB.Raise((void*)sender,EVENT_Click,0);
	GB.Unref((void**)&sender);
}

void TREEVIEW_post_click(gTreeView *sender)
{
	CWIDGET *_ob=GetObject(sender);
	
	if (!_ob) return;

	if (GB.CanRaise((void*)_ob,EVENT_Click) )
	{
		GB.Ref((void*)_ob);
		GB.Post ( (void(*)())TREEVIEW_raise_click, (long)_ob ); 	
	}
}

BEGIN_METHOD(CTREEVIEW_new, GB_OBJECT parent)

	CCONTAINER *Parent=(CCONTAINER*)VPROP(GB_OBJECT);
	Parent=(CCONTAINER*)GetContainer ((CWIDGET*)Parent);

	THIS->widget=new gTreeView(Parent->widget);
	InitControl(THIS->widget,(CWIDGET*)THIS);

	TREEVIEW->onSelect=TREEVIEW_raise_select;
	TREEVIEW->onActivate=TREEVIEW_raise_activate;
	TREEVIEW->onClick=TREEVIEW_post_click;
	
END_METHOD



BEGIN_METHOD_VOID(CTREEVIEW_free)

	if (THIS->icursor) GB.FreeString(&THIS->icursor);
	if (THIS->item) GB.FreeString(&THIS->item);

END_METHOD


BEGIN_METHOD_VOID(CTREEVIEW_clear)

	TREEVIEW->clear();

END_METHOD


BEGIN_METHOD(CTREEVIEW_add, GB_STRING key; GB_STRING text; GB_OBJECT picture; GB_STRING parent; GB_STRING after)

	char *text="";
	char *after=NULL;
	char *parent=NULL;
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
			if (!TREEVIEW->exists(after))
			{
				GB.Error("After item does not exist");
				return;
			}
		}
	}
	
	if (!MISSING(parent))
	{
		if (LENGTH(parent))
		{
			parent=STRING(parent);
			if (!TREEVIEW->exists(parent))
			{
				GB.Error("Parent item does not exist");
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

	if (!TREEVIEW->add (STRING(key),text,pic,after,parent))
	{
		GB.Error("Key already used: '&1'",STRING(key));
		return;
	}

	if (THIS->item) GB.FreeString(&THIS->item);
	GB.NewString(&THIS->item,STRING(key),LENGTH(key));
	



END_METHOD



BEGIN_METHOD(CTREEVIEW_remove, GB_STRING key)



END_METHOD


BEGIN_METHOD(CTREEVIEW_exist, GB_STRING key)



END_METHOD


BEGIN_METHOD(CTREEVIEW_find, GB_INTEGER x; GB_INTEGER y)



END_METHOD


BEGIN_METHOD(CTREEVIEW_get, GB_STRING key)

	if (!LENGTH(key))
	{
		GB.Error ("Unknown item: ''");
		GB.ReturnNull();
		return;
	}
	
	if (!TREEVIEW->exists(STRING(key)) )
	{
		GB.Error ("Unknown item: '&1'",STRING(key));
		GB.ReturnNull();
		return;
	}
	
	if (THIS->icursor) GB.FreeString(&THIS->icursor);
	GB.NewString (&THIS->icursor,STRING(key),LENGTH(key));
	RETURN_SELF();



END_METHOD


BEGIN_PROPERTY(CTREEVIEW_mode)



END_PROPERTY


BEGIN_PROPERTY(CTREEVIEW_current)

	char *key=TREEVIEW->current();
	
	if (THIS->icursor) GB.FreeString(&THIS->icursor); 
	if (!key) { GB.ReturnNull(); return; }
	GB.NewString(&THIS->icursor,key,0);
	free(key);
	RETURN_SELF();

END_PROPERTY


BEGIN_PROPERTY(CTREEVIEW_key)



END_PROPERTY


BEGIN_PROPERTY(CTREEVIEW_item)

	if (THIS->item)
	{
		if (THIS->icursor) GB.FreeString(&THIS->icursor);
		GB.NewString(&THIS->icursor,THIS->item,0);
		RETURN_SELF();
	}
	else
		GB.ReturnNull();

END_PROPERTY


BEGIN_PROPERTY(CTREEVIEW_available)


END_PROPERTY



BEGIN_PROPERTY(CTREEVIEW_sorted)



END_PROPERTY


BEGIN_PROPERTY(CTREEVIEW_editable)


END_PROPERTY


BEGIN_PROPERTY(CTREEVIEW_count)


END_PROPERTY


BEGIN_METHOD_VOID(CTREEVIEW_back)



END_METHOD

BEGIN_METHOD(CTREEVIEW_move_to, GB_STRING key)


END_METHOD


BEGIN_METHOD_VOID(CTREEVIEW_move_current)



END_PROPERTY


BEGIN_METHOD_VOID(CTREEVIEW_first)


END_METHOD

BEGIN_METHOD_VOID(CTREEVIEW_last)



END_METHOD


BEGIN_METHOD_VOID(CTREEVIEW_previous)



END_METHOD

BEGIN_METHOD_VOID(CTREEVIEW_next)



END_METHOD


BEGIN_METHOD_VOID(CTREEVIEW_parent)


END_METHOD


BEGIN_METHOD_VOID(CTREEVIEW_child)



END_METHOD


BEGIN_METHOD_VOID(CTREEVIEW_above)



END_METHOD


BEGIN_METHOD_VOID(CTREEVIEW_below)



END_METHOD


BEGIN_PROPERTY(CTREEVIEWITEM_key)

	GB.ReturnNewString(THIS->icursor,0);

END_PROPERTY


BEGIN_PROPERTY(CTREEVIEWITEM_picture)

	CPICTURE *Pic=NULL;
	gPicture *pic=NULL;
	
	
	if (READ_PROPERTY)
	{
		pic=TREEVIEW->itemPicture(THIS->icursor);
		if (pic)
		{
			GB.New((void **)&Pic, GB.FindClass("Picture"), 0, 0);
			if (Pic->picture) delete Pic->picture;
			Pic->picture=pic;
		}
		GB.ReturnObject((void*)Pic);
	}
	
	Pic=(CPICTURE*)VPROP(GB_OBJECT);
	if (Pic) pic=Pic->picture;
	
	TREEVIEW->itemSetPicture(THIS->icursor,pic);

END_PROPERTY


BEGIN_PROPERTY(CTREEVIEWITEM_selected)

	if (READ_PROPERTY)
	{
		GB.ReturnBoolean(TREEVIEW->itemSelected(THIS->icursor));
		return;
	}
	
END_PROPERTY


BEGIN_PROPERTY(CTREEVIEWITEM_text)

	if (READ_PROPERTY)
	{
		GB.ReturnNewString(TREEVIEW->itemText(THIS->icursor),0);
		return;
	}
	
	TREEVIEW->itemSetText(THIS->icursor,GB.ToZeroString(PROP(GB_STRING)));
	
END_PROPERTY



BEGIN_PROPERTY(CTREEVIEWITEM_expanded)



END_PROPERTY



BEGIN_PROPERTY(CTREEVIEWITEM_count)

	GB.ReturnInteger( TREEVIEW->itemChildren(THIS->icursor) );

END_PROPERTY


BEGIN_METHOD_VOID(CTREEVIEWITEM_ensure_visible)


END_METHOD


BEGIN_METHOD_VOID(CTREEVIEWITEM_delete)



END_METHOD


BEGIN_METHOD_VOID(CTREEVIEWITEM_clear)



END_METHOD



BEGIN_PROPERTY(CTREEVIEWITEM_editable)



END_PROPERTY


BEGIN_METHOD_VOID(CTREEVIEWITEM_rename)


END_METHOD

BEGIN_PROPERTY(CTREEVIEW_scrollbar)

	
END_PROPERTY

/***************************************************************************

  Columns

***************************************************************************/




BEGIN_PROPERTY(CTREEVIEW_client_width)



END_PROPERTY


BEGIN_PROPERTY(CTREEVIEW_client_height)



END_PROPERTY

BEGIN_PROPERTY(CTREEVIEW_border_simple)


END_PROPERTY

BEGIN_PROPERTY(CTREEVIEW_compare)



END_PROPERTY

GB_DESC CTreeViewItemDesc[] =
{
  GB_DECLARE(".TreeViewItem", 0), GB_VIRTUAL_CLASS(),

  GB_PROPERTY_READ("Key", "s", CTREEVIEWITEM_key),
  GB_PROPERTY("Picture", "Picture", CTREEVIEWITEM_picture),
  GB_PROPERTY("Selected", "b", CTREEVIEWITEM_selected),
  GB_PROPERTY("Text", "s", CTREEVIEWITEM_text),
  GB_METHOD("EnsureVisible", NULL, CTREEVIEWITEM_ensure_visible, NULL),
  GB_METHOD("Delete", NULL, CTREEVIEWITEM_delete, NULL),
  GB_METHOD("Clear", NULL, CTREEVIEWITEM_clear, NULL),

  GB_PROPERTY("Expanded", "b", CTREEVIEWITEM_expanded),
  GB_PROPERTY_READ("Children", "i", CTREEVIEWITEM_count),
  GB_PROPERTY_READ("Count", "i", CTREEVIEWITEM_count),

  GB_METHOD("MoveNext", "b", CTREEVIEW_next, NULL),
  GB_METHOD("MovePrevious", "b", CTREEVIEW_previous, NULL),
  GB_METHOD("MoveChild", "b", CTREEVIEW_child, NULL),
  GB_METHOD("MoveParent", "b", CTREEVIEW_parent, NULL),
  GB_METHOD("MoveAbove", "b", CTREEVIEW_above, NULL),
  GB_METHOD("MoveBelow", "b", CTREEVIEW_below, NULL),

  GB_PROPERTY("Editable", "b", CTREEVIEWITEM_editable),
  GB_METHOD("Rename", NULL, CTREEVIEWITEM_rename, NULL),

  GB_END_DECLARE
};


/***************************************************************************

  TreeView

***************************************************************************/

GB_DESC CTreeViewDesc[] =
{
  GB_DECLARE("TreeView", sizeof(CTREEVIEW)), GB_INHERITS("Control"),

  GB_METHOD("_new", NULL, CTREEVIEW_new, "(Parent)Container;"),
  GB_METHOD("_free", NULL, CTREEVIEW_free, NULL),

  GB_PROPERTY("Mode", "i", CTREEVIEW_mode),
  GB_PROPERTY("Sorted", "b", CTREEVIEW_sorted),
  GB_PROPERTY("Editable", "b", CTREEVIEW_editable),
  GB_PROPERTY("ScrollBar", "i<Scroll>", CTREEVIEW_scrollbar),
  GB_PROPERTY("Border", "b", CTREEVIEW_border_simple),
  GB_PROPERTY("Compare", "i", CTREEVIEW_compare),

  GB_PROPERTY_READ("Count", "i", CTREEVIEW_count),

  GB_PROPERTY_READ("Available", "b", CTREEVIEW_available),
  GB_METHOD("MoveCurrent", "b", CTREEVIEW_move_current, NULL),
  GB_METHOD("MoveTo", "b", CTREEVIEW_move_to, "(Key)s"),
  GB_METHOD("MoveFirst", "b", CTREEVIEW_first, NULL),
  GB_METHOD("MoveLast", "b", CTREEVIEW_last, NULL),
  GB_METHOD("MoveNext", "b", CTREEVIEW_next, NULL),
  GB_METHOD("MovePrevious", "b", CTREEVIEW_previous, NULL),
  GB_METHOD("MoveChild", "b", CTREEVIEW_child, NULL),
  GB_METHOD("MoveParent", "b", CTREEVIEW_parent, NULL),
  GB_METHOD("MoveAbove", "b", CTREEVIEW_above, NULL),
  GB_METHOD("MoveBelow", "b", CTREEVIEW_below, NULL),
  GB_METHOD("MoveBack", "b", CTREEVIEW_back, NULL),

  GB_METHOD("_get", ".TreeViewItem", CTREEVIEW_get, "(Key)s"),

  GB_METHOD("Clear", NULL, CTREEVIEW_clear, NULL),
  GB_METHOD("Add", ".TreeViewItem", CTREEVIEW_add, "(Key)s(Text)s[(Picture)Picture;(Parent)s(After)s]"),
  GB_METHOD("Remove", NULL, CTREEVIEW_remove, "(Key)s"),
  GB_METHOD("Exist", "b", CTREEVIEW_exist, "(Key)s"),
  GB_METHOD("Find", "b", CTREEVIEW_find, "(X)i(Y)i"),

  GB_PROPERTY_READ("Current", ".TreeViewItem", CTREEVIEW_current),
  GB_PROPERTY_READ("Key", "s", CTREEVIEW_key),
  GB_PROPERTY_READ("Item", ".TreeViewItem", CTREEVIEW_item),

  GB_CONSTANT("_Properties", "s", CTREEVIEW_PROPERTIES),
  GB_CONSTANT("_DefaultEvent", "s", "Click"),

  GB_PROPERTY_READ("ClientWidth", "i", CTREEVIEW_client_width),
  GB_PROPERTY_READ("ClientW", "i",  CTREEVIEW_client_width),
  GB_PROPERTY_READ("ClientHeight", "i", CTREEVIEW_client_height),
  GB_PROPERTY_READ("ClientH", "i", CTREEVIEW_client_height),

  GB_EVENT("Select", NULL, NULL, &EVENT_Select),
  GB_EVENT("Activate", NULL, NULL, &EVENT_Activate),
  GB_EVENT("Click", NULL, NULL, &EVENT_Click),
  GB_EVENT("Rename", NULL, NULL, &EVENT_Rename),
  GB_EVENT("Compare", NULL, "(Key)s(OtherKey)s", &EVENT_Compare),
  GB_EVENT("Expand", NULL, NULL, &EVENT_Expand),
  GB_EVENT("Collapse", NULL, NULL, &EVENT_Collapse),

  GB_END_DECLARE
};
