/***************************************************************************

  CIconView.cpp

  (c) 2004-2005 - Daniel Campos Fernández <danielcampos@netcourrier.com>
  
  GTK+ component

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


#define __CICONVIEW_CPP

#include <stdlib.h>

#include "gambas.h"
#include "main.h"
#include "widgets.h"

#include "CWidget.h"
#include "CContainer.h"
#include "CPicture.h"
#include "CIconView.h"

#include <stdio.h>
#include <string.h>

DECLARE_EVENT(EVENT_Select);    /* selection change */
DECLARE_EVENT(EVENT_Click);     /* simple click */
DECLARE_EVENT(EVENT_Activate);  /* double click */
DECLARE_EVENT(EVENT_Rename);
DECLARE_EVENT(EVENT_Compare);

void gb_posted_iconview_Rename(void *_ob)
{
	GB.Raise(_ob,EVENT_Rename,0);
	GB.Unref(&_ob);
}

void gb_posted_iconview_Activate(void *_ob)
{
	GB.Raise(_ob,EVENT_Activate,0);
	GB.Unref(&_ob);
}

void gb_posted_iconview_Select(void *_ob)
{
	GB.Raise(_ob,EVENT_Select,0);
	GB.Unref(&_ob);
}

void gb_posted_iconview_Click(void *_ob)
{
	GB.Raise(_ob,EVENT_Click,0);
	GB.Unref(&_ob);
}

void gb_raise_iconview_Activate(gIconView *sender)
{
	CWIDGET *_ob=GetObject((gControl*)sender);
	
	if (!_ob) return;
	if (GB.CanRaise((void*)_ob,EVENT_Activate) )
	{
		GB.Ref((void*)_ob);
		GB.Post ( (void(*)())gb_posted_iconview_Activate, (long)_ob ); 	
	}
}

void gb_raise_iconview_Select(gIconView *sender)
{
	CWIDGET *_ob=GetObject((gControl*)sender);
	
	if (!_ob) return;

	if (GB.CanRaise((void*)_ob,EVENT_Select) )
	{
		GB.Ref((void*)_ob);
		GB.Post ( (void(*)())gb_posted_iconview_Select, (long)_ob ); 	
	} 
	
		
}

void gb_raise_iconview_Rename(gIconView *sender)
{
	CWIDGET *_ob=GetObject((gControl*)sender);
	
	if (!_ob) return;
	if (GB.CanRaise((void*)_ob,EVENT_Rename) )
	{
		GB.Ref((void*)_ob);
		GB.Post ( (void(*)())gb_posted_iconview_Rename, (long)_ob ); 	
	}	
}

void gb_raise_iconview_Click(gIconView *sender)
{
	CWIDGET *_ob=GetObject((gControl*)sender);
	
	if (!_ob) return;
	if (GB.CanRaise((void*)_ob,EVENT_Click) )
	{
		GB.Ref((void*)_ob);
		GB.Post ( (void(*)())gb_posted_iconview_Click, (long)_ob ); 	
	}	
}

/***************************************************************************

  IconView

***************************************************************************/

BEGIN_METHOD(CICONVIEW_new, GB_OBJECT parent)

	CCONTAINER *Parent=(CCONTAINER*)VPROP(GB_OBJECT);
	Parent=(CCONTAINER*)GetContainer ((CWIDGET*)Parent);

	THIS->widget=new gIconView(Parent->widget);
	InitControl(THIS->widget,(CWIDGET*)THIS);

	ICONVIEW->onActivate=gb_raise_iconview_Activate;
	ICONVIEW->onSelect=gb_raise_iconview_Select;
	ICONVIEW->onRename=gb_raise_iconview_Rename;
	ICONVIEW->onClick=gb_raise_iconview_Click;
	
END_METHOD


BEGIN_METHOD_VOID(CICONVIEW_free)

	if (THIS->icursor) GB.FreeString(&THIS->icursor);
	if (THIS->item) GB.FreeString(&THIS->item);

END_METHOD


BEGIN_METHOD_VOID(CICONVIEW_clear)

	ICONVIEW->clear();

END_METHOD


BEGIN_METHOD(CICONVIEW_add, GB_STRING key; GB_STRING text; GB_OBJECT picture; GB_STRING after)

	CPICTURE *pic=NULL;
	char *after=NULL;
	
	if (!LENGTH(key))
	{
		GB.Error("Null key");
		return;
	}
	
	if (!MISSING(after)) after=STRING(after);
	
	if (!MISSING(picture)) pic=(CPICTURE*)VARG(picture);
	
	switch (ICONVIEW->add(STRING(key),after))
	{
		case 0: 
			ICONVIEW->setItemText(STRING(key),GB.ToZeroString(ARG(text)));
			if (pic) ICONVIEW->setItemPicture(STRING(key),pic->picture);
			if (THIS->icursor) GB.FreeString(&THIS->icursor);
			GB.NewString(&THIS->icursor,STRING(key),0);
			ICONVIEW->sort();
			break;
		case -1: GB.Error ("After item does not exist"); break;
		case -2: GB.Error ("Key already used"); break;
	}

END_METHOD


BEGIN_METHOD(CICONVIEW_remove, GB_STRING key)

	ICONVIEW->remove(STRING(key));

END_METHOD


BEGIN_METHOD(CICONVIEW_exist, GB_STRING key)

	GB.ReturnBoolean ( ICONVIEW->exists(STRING(key)) );

END_METHOD


BEGIN_METHOD(CICONVIEW_get, GB_STRING key)

	if ( !ICONVIEW->exists(STRING(key)) ) 
	{
		GB.Error("Unknown item '&1'",STRING(key)); 
		GB.ReturnNull(); 
		return; 
	}
	
	if (THIS->item) GB.FreeString(&THIS->item);
	GB.NewString(&THIS->item,STRING(key),0);
	RETURN_SELF();


END_METHOD

BEGIN_PROPERTY(CICONVIEW_border_simple)

	if (READ_PROPERTY) { GB.ReturnBoolean(ICONVIEW->getBorder()); return; }
	ICONVIEW->setBorder(VPROP(GB_BOOLEAN));

END_PROPERTY


BEGIN_PROPERTY(CICONVIEW_mode)

	if (READ_PROPERTY) { GB.ReturnInteger ( ICONVIEW->selMode() ); return; }
	ICONVIEW->setSelMode(VPROP(GB_INTEGER));

END_PROPERTY


BEGIN_PROPERTY(CICONVIEW_current)

	char *vl;
	
	if (THIS->item) GB.FreeString(&THIS->item);
	vl=ICONVIEW->current();
	if (!vl) GB.ReturnNull();
	GB.NewString(&THIS->item,vl,0);
	RETURN_SELF();


END_PROPERTY


BEGIN_PROPERTY(CICONVIEW_key)

	GB.ReturnNewString(ICONVIEW->current(),0);

END_PROPERTY


BEGIN_PROPERTY(CICONVIEW_item)

	if (!THIS->icursor) { GB.ReturnNull(); return; }
	RETURN_SELF();

END_PROPERTY


BEGIN_PROPERTY(CICONVIEW_available)

	GB.ReturnBoolean ( (bool)THIS->icursor );

END_PROPERTY


BEGIN_METHOD(CICONVIEW_find, GB_INTEGER x; GB_INTEGER y)



END_METHOD


BEGIN_PROPERTY(CICONVIEW_sorted)



END_PROPERTY


BEGIN_PROPERTY(CICONVIEW_ascending)


END_PROPERTY

BEGIN_PROPERTY(CICONVIEW_scrollbar)

	if (READ_PROPERTY) { GB.ReturnInteger(ICONVIEW->scrollBar()); return; }
	ICONVIEW->setScrollBar(VPROP(GB_INTEGER));

END_PROPERTY

BEGIN_PROPERTY(CICONVIEW_count)

	GB.ReturnInteger(ICONVIEW->count());

END_PROPERTY



BEGIN_METHOD(CICONVIEW_move_to, GB_STRING key)

	if (THIS->icursor) GB.FreeString(&THIS->icursor);
	
	if (!ICONVIEW->exists(STRING(key))) { GB.ReturnBoolean(true); return; }
	GB.NewString(&THIS->icursor,STRING(key),LENGTH(key));
	
END_METHOD


BEGIN_METHOD_VOID(CICONVIEW_first)

	char *buf;

	if (THIS->icursor) GB.FreeString(&THIS->icursor);

	buf=ICONVIEW->firstKey();
	if (!buf) { GB.ReturnBoolean(true); return; }
	GB.NewString(&THIS->icursor,buf,0);
	free(buf);
	GB.ReturnBoolean(false);
	
	
END_METHOD


BEGIN_METHOD_VOID(CICONVIEW_next)

	char *buf;

	if (!THIS->icursor) { GB.ReturnBoolean(true); return; }

	buf=ICONVIEW->nextKey(THIS->icursor);
	GB.FreeString(&THIS->icursor);
	if (!buf) 
	{ 
		GB.ReturnBoolean(true); 
	}
	else
	{
		GB.NewString(&THIS->icursor,buf,0);
		free(buf);
		GB.ReturnBoolean(false);
	}


END_METHOD



BEGIN_PROPERTY(CICONVIEWITEM_key)

	GB.ReturnNewString( THIS->icursor,0 );

END_PROPERTY


BEGIN_PROPERTY(CICONVIEWITEM_picture)

	CPICTURE *pic;
	
	if (READ_PROPERTY)
	{
		return;
	}
	
	pic=(CPICTURE*)VPROP(GB_OBJECT);
	ICONVIEW->setItemPicture(THIS->item,pic->picture);
	

END_PROPERTY


BEGIN_PROPERTY(CICONVIEWITEM_selected)

	if (READ_PROPERTY) { GB.ReturnBoolean(ICONVIEW->itemSelected(THIS->item)); return; }
	
	ICONVIEW->setItemSelected(THIS->item,VPROP(GB_BOOLEAN));

END_PROPERTY


BEGIN_PROPERTY(CICONVIEWITEM_text)

	char *buf;
	
	if (READ_PROPERTY)
	{
		buf=ICONVIEW->itemText(THIS->item);
		GB.ReturnNewString(buf,0);
		if (buf) free(buf);
		return;
	}
	
	ICONVIEW->setItemText(THIS->item,GB.ToZeroString(PROP(GB_STRING)));

END_PROPERTY


BEGIN_METHOD_VOID(CICONVIEWITEM_ensure_visible)

	ICONVIEW->ensureVisible(THIS->item);

END_METHOD


BEGIN_METHOD_VOID(CICONVIEWITEM_delete)


END_PROPERTY




BEGIN_PROPERTY(CICONVIEW_client_width)

	GB.ReturnInteger (ICONVIEW->clientWidth());

END_PROPERTY


BEGIN_PROPERTY(CICONVIEW_client_height)

	GB.ReturnInteger (ICONVIEW->clientHeight());

END_PROPERTY


BEGIN_METHOD(CICONVIEW_select_all, GB_BOOLEAN on)

	ICONVIEW->selectAll(VARG(on));

END_METHOD


BEGIN_PROPERTY(CICONVIEW_grid_x)

	if (READ_PROPERTY) { GB.ReturnInteger(ICONVIEW->gridX()); return; }
	ICONVIEW->setGrid(VPROP(GB_INTEGER),ICONVIEW->gridY());
	
END_PROPERTY


BEGIN_PROPERTY(CICONVIEW_grid_y)

	if (READ_PROPERTY) { GB.ReturnInteger(ICONVIEW->gridY()); return; }
	ICONVIEW->setGrid(ICONVIEW->gridX(),VPROP(GB_INTEGER));

END_PROPERTY


BEGIN_PROPERTY(CICONVIEW_arrangement)

	if (READ_PROPERTY) { GB.ReturnInteger(ICONVIEW->arrange()); return; }
	ICONVIEW->setArrange(VPROP(GB_INTEGER));

END_PROPERTY


BEGIN_PROPERTY(CICONVIEW_word_wrap)



END_PROPERTY


BEGIN_PROPERTY(CICONVIEW_compare)



END_PROPERTY


BEGIN_PROPERTY(CICONVIEWITEM_x)

	if (READ_PROPERTY)
	{
		GB.ReturnInteger( ICONVIEW->itemLeft(THIS->item) );
		return;
	}
	
	ICONVIEW->moveItem(THIS->item,VPROP(GB_INTEGER),ICONVIEW->itemTop(THIS->item));

END_PROPERTY


BEGIN_PROPERTY(CICONVIEWITEM_y)

	if (READ_PROPERTY)
	{
		GB.ReturnInteger( ICONVIEW->itemTop(THIS->item) );
		return;
	}
	
	ICONVIEW->moveItem(THIS->item,ICONVIEW->itemLeft(THIS->item),VPROP(GB_INTEGER));

END_PROPERTY


BEGIN_PROPERTY(CICONVIEWITEM_w)

	GB.ReturnInteger ( ICONVIEW->itemWidth(THIS->item) );

END_PROPERTY


BEGIN_PROPERTY(CICONVIEWITEM_h)

	GB.ReturnInteger ( ICONVIEW->itemHeight(THIS->item) ); 

END_PROPERTY


BEGIN_METHOD(CICONVIEWITEM_move, GB_INTEGER x; GB_INTEGER y)

	ICONVIEW->moveItem(THIS->item,VARG(x),VARG(y));

END_METHOD


BEGIN_PROPERTY(CICONVIEWITEM_editable)

	if (READ_PROPERTY) { GB.ReturnInteger(ICONVIEW->itemEditable(THIS->item)); return; }
	ICONVIEW->setItemEditable(THIS->item,VPROP(GB_BOOLEAN));

END_PROPERTY


BEGIN_METHOD_VOID(CICONVIEWITEM_rename)

	ICONVIEW->renameItem(THIS->item);

END_METHOD



/***************************************************************************

  IconViewItem

***************************************************************************/

GB_DESC CIconViewItemDesc[] =
{
  GB_DECLARE(".IconViewItem", 0), GB_VIRTUAL_CLASS(),

  GB_PROPERTY_READ("Key", "s", CICONVIEWITEM_key),
  GB_PROPERTY("Picture", "Picture", CICONVIEWITEM_picture),
  GB_PROPERTY("Selected", "b", CICONVIEWITEM_selected),
  GB_PROPERTY("Text", "s", CICONVIEWITEM_text),
  GB_METHOD("EnsureVisible", NULL, CICONVIEWITEM_ensure_visible, NULL),
  GB_METHOD("Delete", NULL, CICONVIEWITEM_delete, NULL),

  GB_PROPERTY("Editable", "b", CICONVIEWITEM_editable),
  GB_METHOD("Rename", NULL, CICONVIEWITEM_rename, NULL),

  GB_METHOD("MoveNext", "b", CICONVIEW_next, NULL),

  GB_PROPERTY("X", "i", CICONVIEWITEM_x),
  GB_PROPERTY("Left", "i", CICONVIEWITEM_x),
  GB_PROPERTY("Y", "i", CICONVIEWITEM_y),
  GB_PROPERTY("Top", "i", CICONVIEWITEM_y),
  GB_PROPERTY_READ("W", "i", CICONVIEWITEM_w),
  GB_PROPERTY_READ("Width", "i", CICONVIEWITEM_w),
  GB_PROPERTY_READ("H", "i", CICONVIEWITEM_h),
  GB_PROPERTY_READ("Height", "i", CICONVIEWITEM_h),

  GB_METHOD("Move", NULL, CICONVIEWITEM_move, "(X)i(Y)i"),

  GB_END_DECLARE
};


/***************************************************************************

  IconView

***************************************************************************/

GB_DESC CIconViewDesc[] =
{
  GB_DECLARE("IconView", sizeof(CICONVIEW)), GB_INHERITS("Control"),

  GB_CONSTANT("None", "i", 3),
  GB_CONSTANT("Single", "i", 0),
  GB_CONSTANT("Multi", "i", 1),
  GB_CONSTANT("Extended", "i", 2),

  GB_CONSTANT("Free", "i", -1),
  GB_CONSTANT("LeftRight", "i", 0),
  GB_CONSTANT("TopBottom", "i", 1),

  GB_METHOD("_new", NULL, CICONVIEW_new, "(Parent)Container;"),
  GB_METHOD("_free", NULL, CICONVIEW_free, NULL),

  GB_PROPERTY("Mode", "i<IconView,None,Single,Multi,Extended>", CICONVIEW_mode),
  GB_PROPERTY("Sorted", "b", CICONVIEW_sorted),
  GB_PROPERTY("Ascending", "b", CICONVIEW_ascending),
  GB_PROPERTY("Arrangement", "i<IconView,Free,LeftRight,TopBottom>", CICONVIEW_arrangement),
  GB_PROPERTY("GridX", "i", CICONVIEW_grid_x),
  GB_PROPERTY("GridY", "i", CICONVIEW_grid_y),
  GB_PROPERTY("WordWrap", "b", CICONVIEW_word_wrap),
  GB_PROPERTY("Border", "b", CICONVIEW_border_simple),
  GB_PROPERTY("ScrollBar", "i<Scroll>", CICONVIEW_scrollbar),
  GB_PROPERTY("Compare", "i", CICONVIEW_compare),

  GB_PROPERTY_READ("Count", "i", CICONVIEW_count),

  GB_PROPERTY_READ("Available", "b", CICONVIEW_available),
  GB_METHOD("MoveTo", "b", CICONVIEW_move_to, "(Key)s"),
  GB_METHOD("MoveFirst", "b", CICONVIEW_first, NULL),
  GB_METHOD("MoveNext", "b", CICONVIEW_next, NULL),

  GB_METHOD("_get", ".IconViewItem", CICONVIEW_get, "(Key)s"),

  GB_METHOD("Clear", NULL, CICONVIEW_clear, NULL),
  GB_METHOD("Add", ".IconViewItem", CICONVIEW_add, "(Key)s(Text)s[(Picture)Picture;(After)s]"),
  GB_METHOD("Remove", NULL, CICONVIEW_remove, "(Key)s"),
  GB_METHOD("Exist", "b", CICONVIEW_exist, "(Key)s"),
  GB_METHOD("Find", "b", CICONVIEW_find, "(X)i(Y)i"),

  GB_PROPERTY_READ("Current", ".IconViewItem", CICONVIEW_current),
  GB_PROPERTY_READ("Key", "s", CICONVIEW_key),
  GB_PROPERTY_READ("Item", ".IconViewItem", CICONVIEW_item),

  GB_METHOD("SelectAll", NULL, CICONVIEW_select_all, "[(Select)b]"),

  GB_CONSTANT("_Properties", "s", CICONVIEW_PROPERTIES),
  GB_CONSTANT("_DefaultEvent", "s", "Click"),

  GB_PROPERTY_READ("ClientWidth", "i", CICONVIEW_client_width),
  GB_PROPERTY_READ("ClientW", "i",  CICONVIEW_client_width),
  GB_PROPERTY_READ("ClientHeight", "i", CICONVIEW_client_height),
  GB_PROPERTY_READ("ClientH", "i", CICONVIEW_client_height),

  GB_EVENT("Select", NULL, NULL, &EVENT_Select),
  GB_EVENT("Activate", NULL, NULL, &EVENT_Activate),
  GB_EVENT("Click", NULL, NULL, &EVENT_Click),
  GB_EVENT("Rename", NULL, NULL, &EVENT_Rename),
  GB_EVENT("Compare", NULL, "(Key)s(OtherKey)s", &EVENT_Compare),

  GB_END_DECLARE
};


