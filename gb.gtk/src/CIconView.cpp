/***************************************************************************

  CIconView.cpp

  (c) 2004-2006 - Daniel Campos Fernández <dcamposf@gmail.com>
  
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

#include "gambas.h"
#include "main.h"
#include "widgets.h"

#include "CWidget.h"
#include "CContainer.h"
#include "CPicture.h"
#include "CIconView.h"

#include "gdesktop.h"

DECLARE_EVENT(EVENT_Select);    /* selection change */
DECLARE_EVENT(EVENT_Click);     /* simple click */
DECLARE_EVENT(EVENT_Activate);  /* double click */
DECLARE_EVENT(EVENT_Rename);
DECLARE_EVENT(EVENT_Cancel);
DECLARE_EVENT(EVENT_Compare);


static void set_item(CICONVIEW *_object, char *key)
{
	//if (THIS->item != key)
	//	fprintf(stderr, "set_item: %p: (%p) '%s' -> (%p) '%s'\n", THIS, THIS->item, THIS->item, key, key);
  
  //if (key && WIDGET->intern(key) != key)
  //	fprintf(stderr, "key is not intern!!\n");
  
  THIS->item = key;
}
 
static void set_save_item(CICONVIEW *_object, char *key)
{
	//if (THIS->save != key)
	//	fprintf(stderr, "set_save_item: %p: (%p) '%s' -> (%p) '%s'\n", THIS, THIS->item, THIS->item, key, key);
  
  THIS->save = key;
}

static bool check_item(CICONVIEW *_object, char *key)
{
  if (!key || !*key)
  {
    GB.Error("Null key");
    return true;
  }
  
  if (!WIDGET->exists(key))
  {
    GB.Error("Unknown item: '&1'", key);
    return true;
  }
  
  return false;
}

static bool set_check_item(CICONVIEW *_object, char *key)
{
	if (check_item(THIS, key))
		return true;
		
  set_item(THIS, key);
  return false;
}

static void return_item(CICONVIEW *_object, char *key)
{
	if (!WIDGET->exists(key))
		key = NULL;
		
  if (!key)
    set_save_item(THIS, THIS->item);
  
	set_item(THIS, key);

  GB.ReturnBoolean(key == NULL);
}

static void raise_event(gIconView *sender, int event, char *key)
{
	CWIDGET *_object = GetObject(sender);
  char *save;

  if (!key)
    key = sender->current();
  
  save = g_strdup(THIS->item);
  
  set_item(THIS, key);
  
  GB.Raise(THIS, event, 0);
  
  if (sender->exists(save))
  	set_item(THIS, WIDGET->intern(save)); 
  else
  	set_item(THIS, NULL);
  	
  g_free(save);
}

static void raise_select(gIconView *sender)
{
  raise_event(sender, EVENT_Select, NULL);
}

static void raise_activate(gIconView *sender, char *key)
{
  raise_event(sender, EVENT_Activate, key);
}

static void raise_click(gIconView *sender)
{
  raise_event(sender, EVENT_Click, NULL); 
}

static void raise_rename(gIconView *sender, char *key)
{
  raise_event(sender, EVENT_Rename, key);
}

static void raise_cancel(gIconView *sender, char *key)
{
  raise_event(sender, EVENT_Cancel, key);
}

static void cb_remove(gIconView *sender, char *key)
{
	CWIDGET *_object = GetObject(sender);
	
  if (THIS->item == key)
    THIS->item = NULL;
  if (THIS->save == key)
    THIS->save = NULL;
}


/***************************************************************************

  IconView

***************************************************************************/

BEGIN_METHOD(CICONVIEW_new, GB_OBJECT parent)

	InitControl(new gIconView(CONTAINER(VARG(parent))), (CWIDGET*)THIS);

	WIDGET->onActivate = raise_activate;
	WIDGET->onSelect = raise_select;
	//WIDGET->onRename=gb_raise_iconview_Rename;
	WIDGET->onClick = raise_click;
	WIDGET->onRemove = cb_remove;
	WIDGET->onRename = raise_rename;
	WIDGET->onCancel = raise_cancel;

END_METHOD


BEGIN_METHOD_VOID(CICONVIEW_free)

	THIS->item = NULL;
	THIS->save = NULL;

END_METHOD


BEGIN_METHOD_VOID(CICONVIEW_clear)

	WIDGET->clear();

END_METHOD


BEGIN_METHOD(CICONVIEW_add, GB_STRING key; GB_STRING text; GB_OBJECT picture; GB_STRING after)

  char *key = GB.ToZeroString(ARG(key));
	char *text = GB.ToZeroString(ARG(text));
	char *after = MISSING(after) ? NULL : GB.ToZeroString(ARG(after));
	gPicture *pic = NULL;
	
	if (!MISSING(picture) && VARG(picture))
    pic = ((CPICTURE*)VARG(picture))->picture;
	else
		pic = NULL;

	if (!*key)
	{
		GB.Error("Null key");
		return;
	}
	
	if (after && *after)
	{
		if (!WIDGET->exists(after))
		{
			GB.Error("After item does not exist");
			return;
		}
	}
	else
		after = NULL;
	
	if (!WIDGET->add(key, text, pic, after))
	{
		GB.Error("Key already used: '&1'", key);
		return;
	}

  set_item(THIS, WIDGET->intern(key));
  RETURN_SELF();

END_METHOD


BEGIN_METHOD(CICONVIEW_remove, GB_STRING key)

  char *key = GB.ToZeroString(ARG(key));
  
  if (check_item(THIS, key))
  	return;
  	
  key = WIDGET->intern(key);
  WIDGET->remove(key);

END_METHOD


BEGIN_METHOD(CICONVIEW_exist, GB_STRING key)

  GB.ReturnBoolean(WIDGET->exists(GB.ToZeroString(ARG(key))));

END_METHOD


BEGIN_METHOD(CICONVIEW_get, GB_STRING key)

  if (set_check_item(THIS, WIDGET->intern(GB.ToZeroString(ARG(key)))))
    return;

	RETURN_SELF();

END_METHOD


BEGIN_PROPERTY(CICONVIEW_border)

	if (READ_PROPERTY)
		GB.ReturnBoolean(WIDGET->getBorder());
	else
		WIDGET->setBorder(VPROP(GB_BOOLEAN));

END_PROPERTY


BEGIN_PROPERTY(CICONVIEW_mode)

	if (READ_PROPERTY)
    GB.ReturnInteger(WIDGET->mode());
  else
    WIDGET->setMode(VPROP(GB_INTEGER));

END_PROPERTY


BEGIN_PROPERTY(CICONVIEW_current)

	set_item(THIS, WIDGET->current());
		  
  if (THIS->item)
    RETURN_SELF();
  else
    GB.ReturnNull();

END_PROPERTY


BEGIN_PROPERTY(CICONVIEW_key)

	GB.ReturnNewZeroString(WIDGET->current());

END_PROPERTY


BEGIN_PROPERTY(CICONVIEW_item)

  if (THIS->item)
    RETURN_SELF();
  else
    GB.ReturnNull();

END_PROPERTY


BEGIN_PROPERTY(CICONVIEW_available)

  GB.ReturnBoolean(THIS->item != NULL);

END_PROPERTY


BEGIN_METHOD(CICONVIEW_find, GB_INTEGER x; GB_INTEGER y)

	char *key = WIDGET->find(VARG(x), VARG(y));
		
	set_item(THIS, key);
	GB.ReturnBoolean(!key);

END_METHOD


BEGIN_PROPERTY(CICONVIEW_sorted)



END_PROPERTY


BEGIN_PROPERTY(CICONVIEW_editable)

  if (READ_PROPERTY)
    GB.ReturnBoolean(WIDGET->isEditable());
  else
    WIDGET->setEditable(VPROP(GB_BOOLEAN));

END_PROPERTY


BEGIN_PROPERTY(CICONVIEW_ascending)


END_PROPERTY

BEGIN_PROPERTY(CICONVIEW_scrollbar)

	if (READ_PROPERTY) { GB.ReturnInteger(WIDGET->scrollBar()); return; }
	WIDGET->setScrollBar(VPROP(GB_INTEGER));

END_PROPERTY

BEGIN_PROPERTY(CICONVIEW_count)

	GB.ReturnInteger(WIDGET->count());

END_PROPERTY


BEGIN_METHOD(CICONVIEW_move_to, GB_STRING key)

	return_item(THIS, WIDGET->intern(GB.ToZeroString(ARG(key))));
	
END_METHOD


BEGIN_METHOD_VOID(CICONVIEW_first)

	return_item(THIS, WIDGET->firstItem());
	
END_METHOD


BEGIN_METHOD_VOID(CICONVIEW_next)

	return_item(THIS, WIDGET->nextItem(THIS->item));

END_METHOD


BEGIN_PROPERTY(CICONVIEWITEM_key)

	GB.ReturnNewZeroString(THIS->item);

END_PROPERTY


BEGIN_METHOD_VOID(CICONVIEW_back)

  set_item(THIS, THIS->save);
  set_save_item(THIS, NULL);
  GB.ReturnBoolean(THIS->item == NULL);

END_METHOD


BEGIN_PROPERTY(CICONVIEWITEM_picture)

	CPICTURE *picture = NULL;
	gPicture *pic = NULL;
	
	if (READ_PROPERTY)
	{
		pic = WIDGET->itemPicture(THIS->item);
		if (pic)
		{
			picture = CPICTURE_create(pic);
			pic->ref();
		}
		GB.ReturnObject((void*)picture);
	}
	else
	{
		picture = (CPICTURE *)VPROP(GB_OBJECT);
		WIDGET->setItemPicture(THIS->item, picture ? picture->picture : 0);
	}
	

END_PROPERTY


BEGIN_PROPERTY(CICONVIEWITEM_selected)

	if (READ_PROPERTY)
		GB.ReturnBoolean(WIDGET->isItemSelected(THIS->item));
	else
		WIDGET->setItemSelected(THIS->item, VPROP(GB_BOOLEAN));

END_PROPERTY


BEGIN_PROPERTY(CICONVIEWITEM_text)

	if (READ_PROPERTY)
		GB.ReturnNewString(WIDGET->itemText(THIS->item), 0);
	else	
		WIDGET->setItemText(THIS->item, GB.ToZeroString(PROP(GB_STRING)));
	
END_PROPERTY


BEGIN_METHOD_VOID(CICONVIEWITEM_ensure_visible)

	WIDGET->ensureItemVisible(THIS->item);

END_METHOD


BEGIN_METHOD_VOID(CICONVIEWITEM_delete)

	WIDGET->remove(THIS->item);

END_PROPERTY


BEGIN_PROPERTY(CICONVIEW_client_width)

	GB.ReturnInteger(WIDGET->clientWidth());

END_PROPERTY


BEGIN_PROPERTY(CICONVIEW_client_height)

	GB.ReturnInteger(WIDGET->clientHeight());

END_PROPERTY


BEGIN_METHOD(CICONVIEW_select_all, GB_BOOLEAN select)

	if (VARGOPT(select, TRUE))
		WIDGET->selectAll();
	else
		WIDGET->unselectAll();

END_METHOD


BEGIN_PROPERTY(CICONVIEW_grid_w)

	if (READ_PROPERTY)
	{
		if (WIDGET->gridWidth() <= 0)
			GB.ReturnInteger(0);
		else
			GB.ReturnInteger(WIDGET->gridWidth() / gDesktop::scale());
	}
	else
	{
		if (VPROP(GB_INTEGER) <= 0)
			WIDGET->setGridWidth(-1);
		else
			WIDGET->setGridWidth(VPROP(GB_INTEGER) * gDesktop::scale());
	}
	
END_PROPERTY



BEGIN_PROPERTY(CICONVIEW_compare)



END_PROPERTY


BEGIN_PROPERTY(CICONVIEWITEM_x)

END_PROPERTY


BEGIN_PROPERTY(CICONVIEWITEM_y)

END_PROPERTY


BEGIN_PROPERTY(CICONVIEWITEM_w)

END_PROPERTY


BEGIN_PROPERTY(CICONVIEWITEM_h)

END_PROPERTY


BEGIN_METHOD(CICONVIEWITEM_move, GB_INTEGER x; GB_INTEGER y)

END_METHOD


BEGIN_PROPERTY(CICONVIEWITEM_editable)

	if (READ_PROPERTY)
		GB.ReturnBoolean(WIDGET->isItemEditable(THIS->item));
	else
		WIDGET->setItemEditable(THIS->item, VPROP(GB_BOOLEAN));

END_PROPERTY


BEGIN_METHOD_VOID(CICONVIEWITEM_rename)

	WIDGET->startRename(THIS->item);

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

  GB_CONSTANT("Free", "i", -1),
  GB_CONSTANT("Row", "i", 0),
  GB_CONSTANT("Column", "i", 1),
  GB_CONSTANT("LeftRight", "i", 0),
  GB_CONSTANT("TopBottom", "i", 1),

  GB_METHOD("_new", NULL, CICONVIEW_new, "(Parent)Container;"),
  GB_METHOD("_free", NULL, CICONVIEW_free, NULL),

  GB_PROPERTY("Mode", "i", CICONVIEW_mode),
  GB_PROPERTY("Sorted", "b", CICONVIEW_sorted),
  GB_PROPERTY("Editable", "b", CICONVIEW_editable),
  GB_PROPERTY("Ascending", "b", CICONVIEW_ascending),
  //GB_PROPERTY("Arrangement", "i", CICONVIEW_arrangement),
  GB_PROPERTY("GridWidth", "i", CICONVIEW_grid_w),
  //GB_PROPERTY("GridHeight", "i", CICONVIEW_grid_h),
  //GB_PROPERTY("WordWrap", "b", CICONVIEW_word_wrap),
  GB_PROPERTY("Border", "b", CICONVIEW_border),
  GB_PROPERTY("ScrollBar", "i", CICONVIEW_scrollbar),
  GB_PROPERTY("Compare", "i", CICONVIEW_compare),

  GB_PROPERTY_READ("Count", "i", CICONVIEW_count),

  GB_PROPERTY_READ("Available", "b", CICONVIEW_available),
  GB_METHOD("MoveTo", "b", CICONVIEW_move_to, "(Key)s"),
  GB_METHOD("MoveFirst", "b", CICONVIEW_first, NULL),
  GB_METHOD("MoveNext", "b", CICONVIEW_next, NULL),
  GB_METHOD("MoveBack", "b", CICONVIEW_back, NULL),

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
  GB_EVENT("Cancel", NULL, NULL, &EVENT_Cancel),
  GB_EVENT("Compare", NULL, "(Key)s(OtherKey)s", &EVENT_Compare),

  GB_END_DECLARE
};

