/***************************************************************************

  CTreeView.cpp

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

#define __CTREEVIEW_CPP

#include "CWidget.h"
#include "CContainer.h"
#include "CPicture.h"
#include "CTreeView.h"

DECLARE_EVENT(EVENT_Select);
DECLARE_EVENT(EVENT_Click);
DECLARE_EVENT(EVENT_Activate);
DECLARE_EVENT(EVENT_Rename);
DECLARE_EVENT(EVENT_Cancel);
DECLARE_EVENT(EVENT_Compare);
DECLARE_EVENT(EVENT_Expand);
DECLARE_EVENT(EVENT_Collapse);
//DECLARE_EVENT(EVENT_ColumnClick);

static int check_valid_item(void *_object)
{
	return THIS->item == 0 || CWIDGET_check(_object);
}


static void set_item(CTREEVIEW *_object, char *key)
{
	//if (THIS->item != key)
	//	fprintf(stderr, "set_item: %p: (%p) '%s' -> (%p) '%s'\n", THIS, THIS->item, THIS->item, key, key);
  
  //if (key && WIDGET->intern(key) != key)
  //	fprintf(stderr, "key is not intern!!\n");
  
  THIS->item = key;
}

static void set_save_item(CTREEVIEW *_object, char *key)
{
	//if (THIS->save != key)
	//	fprintf(stderr, "set_save_item: %p: (%p) '%s' -> (%p) '%s'\n", THIS, THIS->item, THIS->item, key, key);
  
  THIS->save = key;
}

static bool check_item(CTREEVIEW *_object, char *key)
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

static bool set_check_item(CTREEVIEW *_object, char *key)
{
	if (check_item(THIS, key))
		return true;
		
  set_item(THIS, key);
  return false;
}

static void return_item(CTREEVIEW *_object, char *key)
{
	if (!WIDGET->exists(key))
		key = NULL;
		
  if (!key)
    set_save_item(THIS, THIS->item);
  
	set_item(THIS, key);

  GB.ReturnBoolean(key == NULL);
}

static void raise_event(gTreeView *sender, int event, char *key)
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

/*static void post_expand(gTreeView *sender, char *key)
{
	CWIDGET *_object = GetObject(sender);
	raise_event(sender, EVENT_Expand, key);
	GB.Unref(POINTER(&_object));
	g_free(key);
}*/

static void raise_select(gTreeView *sender)
{
  raise_event(sender, EVENT_Select, NULL);
}

static void raise_activate(gTreeView *sender, char *key)
{
  raise_event(sender, EVENT_Activate, key);
}

static void raise_expand(gTreeView *sender, char *key)
{
	/*CWIDGET *_object = GetObject(sender);
	GB.Ref(THIS);
	GB.Post2((GB_POST_FUNC)post_expand, (long)sender, (long)g_strdup(key)); */
	raise_event(sender, EVENT_Expand, key);
}

static void raise_collapse(gTreeView *sender, char *key)
{
  raise_event(sender, EVENT_Collapse, key);
}

static void raise_rename(gTreeView *sender, char *key)
{
  raise_event(sender, EVENT_Rename, key);
}

static void raise_cancel(gTreeView *sender, char *key)
{
  raise_event(sender, EVENT_Cancel, key);
}

/*static void post_click(CTREEVIEW *_object)
{
  raise_event(WIDGET, EVENT_Click, WIDGET->current()); 
	GB.Unref(POINTER(&_object));
}*/

static void raise_click(gTreeView *sender)
{
  raise_event(sender, EVENT_Click, NULL); 
}

static bool raise_compare(gTreeView *sender, char *keya, char *keyb, int *comp)
{
	CWIDGET *_object = GetObject(sender);
	
  if (!GB.CanRaise(THIS, EVENT_Compare))
  	return true;
  	
  THIS->compare = 0;
  GB.Raise(THIS, EVENT_Compare, 2, GB_T_STRING, keya, 0, GB_T_STRING, keyb, 0);
  *comp = THIS->compare;
  return false;
}

static void cb_remove(gTreeView *sender, char *key)
{
	CWIDGET *_object = GetObject(sender);
	
  if (THIS->item == key)
    THIS->item = NULL;
  if (THIS->save == key)
    THIS->save = NULL;
}

static void create_control(CTREEVIEW *_object, void *parent, bool list)
{
	InitControl(new gTreeView(CONTAINER(parent), list), (CWIDGET*)THIS);

	WIDGET->onSelect = raise_select;
	WIDGET->onActivate = raise_activate;
	WIDGET->onClick = raise_click;
	WIDGET->onExpand = raise_expand;
	WIDGET->onCollapse = raise_collapse;
	WIDGET->onRename = raise_rename;
	WIDGET->onCancel = raise_cancel;
	WIDGET->onRemove = cb_remove;
	WIDGET->onCompare = raise_compare;
}

BEGIN_METHOD(CTREEVIEW_new, GB_OBJECT parent)

	create_control(THIS, VARG(parent), false);
	
END_METHOD


BEGIN_METHOD(CLISTVIEW_new, GB_OBJECT parent)

	create_control(THIS, VARG(parent), true);
	
END_METHOD


BEGIN_METHOD(CCOLUMNVIEW_new, GB_OBJECT parent)

	create_control(THIS, VARG(parent), false);
	WIDGET->setHeaders(true);
	
END_METHOD


BEGIN_METHOD_VOID(CTREEVIEW_free)

  set_item(THIS, NULL);
  set_save_item(THIS, NULL);
  
END_METHOD


BEGIN_METHOD_VOID(CTREEVIEW_clear)

	WIDGET->clear();
  set_item(THIS, NULL);
  set_save_item(THIS, NULL);

END_METHOD


static void add_item(CTREEVIEW *_object, char *key, char *text, gPicture *pic, char *parent, char *after)
{
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
	
	if (parent && *parent)
	{
		if (!WIDGET->exists(parent))
		{
			GB.Error("Parent item does not exist");
			return;
		}
	}
	else
		parent = NULL;
	
	// BM: The parameters are temporary string than can be freed 
	// when the Compare event is called by add()
	
	key = g_strdup(key);
	text = g_strdup(text);
	
	if (!WIDGET->add(key, text, pic, after, parent))
	{
		GB.Error("Key already used: '&1'", key);
	}
	else
	{
		set_item(THIS, WIDGET->intern(key));
		RETURN_SELF();
	}
	
	g_free(key);
	g_free(text);
}


BEGIN_METHOD(CTREEVIEW_add, GB_STRING key; GB_STRING text; GB_OBJECT picture; GB_STRING parent; GB_STRING after)

  char *key = GB.ToZeroString(ARG(key));
	char *text = GB.ToZeroString(ARG(text));
	char *after = MISSING(after) ? NULL : GB.ToZeroString(ARG(after));
	char *parent = MISSING(parent) ? NULL : GB.ToZeroString(ARG(parent));
	gPicture *pic = NULL;
	
	if (!MISSING(picture) && VARG(picture))
    pic = ((CPICTURE*)VARG(picture))->picture;
	else
		pic = NULL;

	add_item(THIS, key, text, pic, parent, after);

END_METHOD


BEGIN_METHOD(CLISTVIEW_add, GB_STRING key; GB_STRING text; GB_OBJECT picture; GB_STRING after)

  char *key = GB.ToZeroString(ARG(key));
	char *text = GB.ToZeroString(ARG(text));
	char *after = MISSING(after) ? NULL : GB.ToZeroString(ARG(after));
	gPicture *pic = NULL;
	
	if (!MISSING(picture) && VARG(picture))
    pic = ((CPICTURE*)VARG(picture))->picture;
	else
		pic = NULL;

	add_item(THIS, key, text, pic, 0, after);

END_METHOD


BEGIN_METHOD(CTREEVIEW_remove, GB_STRING key)

  char *key = GB.ToZeroString(ARG(key));
  
  if (check_item(THIS, key))
  	return;
  	
  key = WIDGET->intern(key);
  WIDGET->remove(key);

END_METHOD


BEGIN_METHOD(CTREEVIEW_exist, GB_STRING key)

  GB.ReturnBoolean(WIDGET->exists(GB.ToZeroString(ARG(key))));

END_METHOD


BEGIN_METHOD(CTREEVIEW_find, GB_INTEGER x; GB_INTEGER y)

	char *key = WIDGET->find(VARG(x), VARG(y));
		
	set_item(THIS, key);
	GB.ReturnBoolean(!key);

END_METHOD


BEGIN_METHOD(CTREEVIEW_get, GB_STRING key)

  if (set_check_item(THIS, WIDGET->intern(GB.ToZeroString(ARG(key)))))
    return;

	RETURN_SELF();

END_METHOD


BEGIN_PROPERTY(CTREEVIEW_mode)

	if (READ_PROPERTY)
    GB.ReturnInteger(WIDGET->mode());
  else
    WIDGET->setMode(VPROP(GB_INTEGER));

END_PROPERTY


BEGIN_PROPERTY(CTREEVIEW_current)

	set_item(THIS, WIDGET->current());
		  
  if (THIS->item)
    RETURN_SELF();
  else
    GB.ReturnNull();

END_PROPERTY


BEGIN_PROPERTY(CTREEVIEW_key)

	//set_item(THIS, WIDGET->current());
	GB.ReturnNewZeroString(WIDGET->current());

END_PROPERTY


BEGIN_PROPERTY(CTREEVIEW_item)

  if (THIS->item)
    RETURN_SELF();
  else
    GB.ReturnNull();

END_PROPERTY


BEGIN_PROPERTY(CTREEVIEW_available)

  GB.ReturnBoolean(THIS->item != NULL);

END_PROPERTY


BEGIN_PROPERTY(CTREEVIEW_header)

	if (READ_PROPERTY)
		GB.ReturnBoolean(WIDGET->headers());
	else
		WIDGET->setHeaders(VPROP(GB_BOOLEAN));

END_PROPERTY


BEGIN_PROPERTY(CTREEVIEW_sorted)

	if (READ_PROPERTY)
		GB.ReturnBoolean(WIDGET->isSorted());
	else
		WIDGET->setSorted(VPROP(GB_BOOLEAN));

END_PROPERTY


BEGIN_PROPERTY(CTREEVIEW_editable)

	if (READ_PROPERTY)
		GB.ReturnBoolean(WIDGET->isEditable());
	else
		WIDGET->setEditable(VPROP(GB_BOOLEAN));

END_PROPERTY


BEGIN_PROPERTY(CTREEVIEW_count)

  GB.ReturnInteger(WIDGET->count());

END_PROPERTY


BEGIN_METHOD_VOID(CTREEVIEW_back)

  set_item(THIS, THIS->save);
  set_save_item(THIS, NULL);
  GB.ReturnBoolean(THIS->item == NULL);

END_METHOD


BEGIN_METHOD(CTREEVIEW_move_to, GB_STRING key)

	return_item(THIS, WIDGET->intern(GB.ToZeroString(ARG(key))));

END_METHOD


BEGIN_METHOD_VOID(CTREEVIEW_move_current)

	return_item(THIS, WIDGET->current());

END_PROPERTY


BEGIN_METHOD_VOID(CTREEVIEW_first)

	return_item(THIS, WIDGET->firstItem());

END_METHOD

BEGIN_METHOD_VOID(CTREEVIEW_last)

	return_item(THIS, WIDGET->lastItem(THIS->item));

END_METHOD


BEGIN_METHOD_VOID(CTREEVIEW_previous)

	return_item(THIS, WIDGET->prevItem(THIS->item));

END_METHOD


BEGIN_METHOD_VOID(CTREEVIEW_next)

	return_item(THIS, WIDGET->nextItem(THIS->item));

END_METHOD


BEGIN_METHOD_VOID(CTREEVIEW_parent)

	return_item(THIS, WIDGET->parentItem(THIS->item));

END_METHOD


BEGIN_METHOD_VOID(CTREEVIEW_child)

	return_item(THIS, WIDGET->firstItem(THIS->item));

END_METHOD


BEGIN_METHOD_VOID(CTREEVIEW_above)

	return_item(THIS, WIDGET->aboveItem(THIS->item));

END_METHOD


BEGIN_METHOD_VOID(CTREEVIEW_below)

	return_item(THIS, WIDGET->belowItem(THIS->item));

END_METHOD


BEGIN_PROPERTY(CTREEVIEWITEM_key)

	GB.ReturnNewZeroString(THIS->item);

END_PROPERTY

BEGIN_PROPERTY(CTREEVIEWITEM_parent_key)

	GB.ReturnNewZeroString(WIDGET->parentItem(THIS->item));

END_PROPERTY


BEGIN_PROPERTY(CTREEVIEWITEM_picture)

	if (READ_PROPERTY)
	{
		gPicture *pic = WIDGET->itemPicture(THIS->item);
		GB.ReturnObject(pic ? pic->getTagValue() : 0);
	}
	else
	{
		CPICTURE *pic = (CPICTURE *)VPROP(GB_OBJECT);
		WIDGET->setItemPicture(THIS->item, pic ? pic->picture : 0);
	}

END_PROPERTY


BEGIN_PROPERTY(CTREEVIEWITEM_selected)

	if (READ_PROPERTY)
		GB.ReturnBoolean(WIDGET->isItemSelected(THIS->item));
	else
		WIDGET->setItemSelected(THIS->item, VPROP(GB_BOOLEAN));
	
END_PROPERTY


BEGIN_PROPERTY(CTREEVIEWITEM_text)

	if (READ_PROPERTY)
		GB.ReturnNewZeroString(WIDGET->itemText(THIS->item));
	else	
		WIDGET->setItemText(THIS->item, GB.ToZeroString(PROP(GB_STRING)));
	
END_PROPERTY


BEGIN_PROPERTY(CTREEVIEWITEM_expanded)

  if (WIDGET->itemChildren(THIS->item) == 0)
  {
    if (READ_PROPERTY)
      GB.ReturnBoolean(false);
    return;
  }

  if (READ_PROPERTY)
    GB.ReturnBoolean(WIDGET->isItemExpanded(THIS->item));
  else
    WIDGET->setItemExpanded(THIS->item, VPROP(GB_BOOLEAN));

END_PROPERTY



BEGIN_PROPERTY(CTREEVIEWITEM_count)

	GB.ReturnInteger( WIDGET->itemChildren(THIS->item) );

END_PROPERTY


BEGIN_METHOD_VOID(CTREEVIEWITEM_ensure_visible)

  WIDGET->ensureItemVisible(THIS->item);

END_METHOD


BEGIN_METHOD_VOID(CTREEVIEWITEM_delete)

	WIDGET->remove(THIS->item);

END_METHOD


BEGIN_METHOD_VOID(CTREEVIEWITEM_clear)

	WIDGET->clear(THIS->item);

END_METHOD


BEGIN_PROPERTY(CTREEVIEWITEM_editable)

	if (READ_PROPERTY)
		GB.ReturnBoolean(WIDGET->isItemEditable(THIS->item));
	else
		WIDGET->setItemEditable(THIS->item, VPROP(GB_BOOLEAN));

END_PROPERTY


BEGIN_METHOD_VOID(CTREEVIEWITEM_rename)

	WIDGET->startRename(THIS->item);

END_METHOD


BEGIN_PROPERTY(CTREEVIEW_scrollbar)

	if (READ_PROPERTY)
	 GB.ReturnInteger(WIDGET->scrollBar());
  else
	 WIDGET->setScrollBar(VPROP(GB_INTEGER));
	
END_PROPERTY

BEGIN_METHOD(CTREEVIEW_select_all, GB_BOOLEAN select)

	if (VARGOPT(select, TRUE))
		WIDGET->selectAll();
	else
		WIDGET->unselectAll();

END_METHOD

BEGIN_METHOD_VOID(CTREEVIEWITEM_move_first)

	WIDGET->moveItemFirst(THIS->item);

END_METHOD

BEGIN_METHOD(CTREEVIEWITEM_move_after, GB_STRING key)
	
	WIDGET->moveItemAfter(THIS->item, GB.ToZeroString(ARG(key)));

END_METHOD

BEGIN_METHOD(CTREEVIEWITEM_move_before, GB_STRING key)
	
	WIDGET->moveItemBefore(THIS->item, GB.ToZeroString(ARG(key)));

END_METHOD

BEGIN_METHOD_VOID(CTREEVIEWITEM_move_last)
	
	WIDGET->moveItemLast(THIS->item);

END_METHOD

BEGIN_PROPERTY(CTREEVIEWITEM_x)

	int x, y, w, h;
	WIDGET->rectItem(THIS->item, &x, &y, &w, &h);
	GB.ReturnInteger(x);

END_PROPERTY

BEGIN_PROPERTY(CTREEVIEWITEM_y)

	int x, y, w, h;
	WIDGET->rectItem(THIS->item, &x, &y, &w, &h);
	GB.ReturnInteger(y);

END_PROPERTY

BEGIN_PROPERTY(CTREEVIEWITEM_w)

	int x, y, w, h;
	WIDGET->rectItem(THIS->item, &x, &y, &w, &h);
	GB.ReturnInteger(w);

END_PROPERTY

BEGIN_PROPERTY(CTREEVIEWITEM_h)

	int x, y, w, h;
	WIDGET->rectItem(THIS->item, &x, &y, &w, &h);
	GB.ReturnInteger(h);

END_PROPERTY

BEGIN_METHOD(CTREEVIEWITEM_get, GB_INTEGER col)

  GB.ReturnNewZeroString(WIDGET->itemText(THIS->item, VARG(col)));

END_METHOD

BEGIN_METHOD(CTREEVIEWITEM_put, GB_STRING text; GB_INTEGER col)

	WIDGET->setItemText(THIS->item, VARG(col), GB.ToZeroString(ARG(text)));

END_METHOD

BEGIN_PROPERTY(CLISTVIEW_auto_resize)

	if (READ_PROPERTY)
		GB.ReturnBoolean(WIDGET->isAutoResize());
	else
		WIDGET->setAutoResize(VPROP(GB_BOOLEAN));

END_PROPERTY

BEGIN_PROPERTY(CCOLUMNVIEW_resizable)

	if (READ_PROPERTY)
		GB.ReturnBoolean(WIDGET->isResizable());
	else
		WIDGET->setResizable(VPROP(GB_BOOLEAN));

END_PROPERTY

BEGIN_PROPERTY(CLISTVIEW_columns_count)

  long col;

  if (READ_PROPERTY)
  {
    GB.ReturnInteger(WIDGET->columnCount());
    return;
  }

  col = VPROP(GB_INTEGER);

  if (col < 1 || col > 255)
  {
    GB.Error("Bad number of columns");
    return;
  }
  
  WIDGET->setColumnCount(col);
  
END_PROPERTY

BEGIN_METHOD(CLISTVIEW_columns_get, GB_INTEGER col)

  long col = VARG(col);

  if (col < 0 || col >= WIDGET->columnCount())
  {
    GB.Error("Bad column index");
    return;
  }

  THIS->column = col;

  RETURN_SELF();

END_PROPERTY

BEGIN_METHOD_VOID(CLISTVIEW_columns_adjust)
END_METHOD

BEGIN_PROPERTY(CLISTVIEW_column_text)

  if (READ_PROPERTY)
    GB.ReturnNewZeroString(WIDGET->columnText(THIS->column));
  else
    WIDGET->setColumnText(THIS->column, GB.ToZeroString(PROP(GB_STRING)));

END_PROPERTY

BEGIN_PROPERTY(CCOLUMNVIEW_column_width)

	if (READ_PROPERTY)
		GB.ReturnInteger(WIDGET->columnWidth(THIS->column));
	else
		WIDGET->setColumnWidth(THIS->column, VPROP(GB_INTEGER));

END_PROPERTY

BEGIN_PROPERTY(CLISTVIEW_column_alignment)

	if (READ_PROPERTY)
		GB.ReturnInteger(WIDGET->columnAlignment(THIS->column));
	else
		WIDGET->setColumnAlignment(THIS->column, VPROP(GB_INTEGER));

END_PROPERTY

BEGIN_PROPERTY(CLISTVIEW_columns_sort)

	if (READ_PROPERTY)
		GB.ReturnInteger(WIDGET->getSortColumn());
	else
		WIDGET->setSortColumn(VPROP(GB_INTEGER));

END_PROPERTY

BEGIN_PROPERTY(CLISTVIEW_columns_ascending)

	if (READ_PROPERTY)
		GB.ReturnBoolean(WIDGET->isSortAscending());
	else
		WIDGET->setSortAscending(VPROP(GB_BOOLEAN));

END_PROPERTY

BEGIN_PROPERTY(CTREEVIEW_compare)

  if (READ_PROPERTY)
    GB.ReturnInteger(THIS->compare);
  else
    THIS->compare = VPROP(GB_INTEGER);

END_PROPERTY

static GB_FUNCTION _get_settings_func;
static GB_FUNCTION _set_settings_func;

static void init_settings()
{
  static bool init = false;
  
  if (init)
  	return;
  	
	GB.GetFunction(&_get_settings_func, (void *)GB.FindClass("_Gtk"), "_GetColumnViewSettings", "ColumnView;", "s");
	GB.GetFunction(&_set_settings_func, (void *)GB.FindClass("_Gtk"), "_SetColumnViewSettings", "ColumnView;s", "");
	
	init = true;
}

BEGIN_PROPERTY(CCOLUMNVIEW_settings)

	init_settings();

	if (READ_PROPERTY)
	{
		GB.Push(1, GB_T_OBJECT, THIS);
		GB.Call(&_get_settings_func, 1, false);
	}
	else
	{
		GB.Push(2,
			GB_T_OBJECT, THIS,
			GB_T_STRING, PSTRING(), PLENGTH());

		GB.Call(&_set_settings_func, 2, true);
	}

END_PROPERTY


/***************************************************************************

  Columns

***************************************************************************/




BEGIN_PROPERTY(CTREEVIEW_client_width)

	GB.ReturnInteger(WIDGET->clientWidth());

END_PROPERTY


BEGIN_PROPERTY(CTREEVIEW_client_height)

	GB.ReturnInteger(WIDGET->clientHeight());

END_PROPERTY

BEGIN_PROPERTY(CTREEVIEW_border)

	if (READ_PROPERTY)
		GB.ReturnBoolean(WIDGET->hasBorder());
	else
		WIDGET->setBorder(VPROP(GB_BOOLEAN));

END_PROPERTY



/***************************************************************************

  ListViewItem

***************************************************************************/

GB_DESC CListViewItemDesc[] =
{
  GB_DECLARE(".ListViewItem", 0), GB_VIRTUAL_CLASS(), 
  GB_HOOK_CHECK(check_valid_item),

  GB_PROPERTY_READ("Key", "s", CTREEVIEWITEM_key),
  GB_PROPERTY("Picture", "Picture", CTREEVIEWITEM_picture),
  GB_PROPERTY("Selected", "b", CTREEVIEWITEM_selected),
  GB_PROPERTY("Text", "s", CTREEVIEWITEM_text),
  GB_METHOD("EnsureVisible", 0, CTREEVIEWITEM_ensure_visible, 0),
  GB_METHOD("Delete", 0, CTREEVIEWITEM_delete, 0),

  GB_PROPERTY_READ("X", "i", CTREEVIEWITEM_x),
  GB_PROPERTY_READ("Y", "i", CTREEVIEWITEM_y),
  GB_PROPERTY_READ("W", "i", CTREEVIEWITEM_w),
  GB_PROPERTY_READ("Width", "i", CTREEVIEWITEM_w),
  GB_PROPERTY_READ("H", "i", CTREEVIEWITEM_h),
  GB_PROPERTY_READ("Height", "i", CTREEVIEWITEM_h),
  
  GB_METHOD("MoveAfter", 0, CTREEVIEWITEM_move_after, "[(Key)s]"),
  GB_METHOD("MoveBefore", 0, CTREEVIEWITEM_move_before, "[(Key)s]"),
  GB_METHOD("MoveFirst", 0, CTREEVIEWITEM_move_first, 0),
  GB_METHOD("MoveLast", 0, CTREEVIEWITEM_move_last, 0),
  
  GB_PROPERTY("Editable", "b", CTREEVIEWITEM_editable),
  GB_METHOD("Rename", 0, CTREEVIEWITEM_rename, 0),

  GB_END_DECLARE
};


/***************************************************************************

  TreeViewItem

***************************************************************************/

GB_DESC CTreeViewItemDesc[] =
{
  GB_DECLARE(".TreeViewItem", 0), GB_VIRTUAL_CLASS(), GB_HOOK_CHECK(check_valid_item),

  GB_PROPERTY_READ("Key", "s", CTREEVIEWITEM_key),
  GB_PROPERTY_READ("ParentKey", "s", CTREEVIEWITEM_parent_key),
  GB_PROPERTY("Picture", "Picture", CTREEVIEWITEM_picture),
  GB_PROPERTY("Selected", "b", CTREEVIEWITEM_selected),
  GB_PROPERTY("Text", "s", CTREEVIEWITEM_text),
  GB_METHOD("EnsureVisible", 0, CTREEVIEWITEM_ensure_visible, 0),
  GB_METHOD("Delete", 0, CTREEVIEWITEM_delete, 0),
  GB_METHOD("Clear", 0, CTREEVIEWITEM_clear, 0),

  GB_PROPERTY("Expanded", "b", CTREEVIEWITEM_expanded),
  GB_PROPERTY_READ("Children", "i", CTREEVIEWITEM_count),
  GB_PROPERTY_READ("Count", "i", CTREEVIEWITEM_count),

  GB_PROPERTY_READ("X", "i", CTREEVIEWITEM_x),
  GB_PROPERTY_READ("Y", "i", CTREEVIEWITEM_y),
  GB_PROPERTY_READ("W", "i", CTREEVIEWITEM_w),
  GB_PROPERTY_READ("Width", "i", CTREEVIEWITEM_w),
  GB_PROPERTY_READ("H", "i", CTREEVIEWITEM_h),
  GB_PROPERTY_READ("Height", "i", CTREEVIEWITEM_h),
  
  /*GB_METHOD("MoveNext", "b", CTREEVIEW_next, 0),
  GB_METHOD("MovePrevious", "b", CTREEVIEW_previous, 0),
  GB_METHOD("MoveChild", "b", CTREEVIEW_child, 0),
  GB_METHOD("MoveParent", "b", CTREEVIEW_parent, 0),
  GB_METHOD("MoveAbove", "b", CTREEVIEW_above, 0),
  GB_METHOD("MoveBelow", "b", CTREEVIEW_below, 0),*/

  GB_METHOD("MoveAfter", 0, CTREEVIEWITEM_move_after, "[(Key)s]"),
  GB_METHOD("MoveBefore", 0, CTREEVIEWITEM_move_before, "[(Key)s]"),
  GB_METHOD("MoveFirst", 0, CTREEVIEWITEM_move_first, 0),
  GB_METHOD("MoveLast", 0, CTREEVIEWITEM_move_last, 0),
  
  GB_PROPERTY("Editable", "b", CTREEVIEWITEM_editable),
  GB_METHOD("Rename", 0, CTREEVIEWITEM_rename, 0),

  GB_END_DECLARE
};


/***************************************************************************

  ColumnViewItem

***************************************************************************/

GB_DESC CColumnViewItemDesc[] =
{
  GB_DECLARE(".ColumnViewItem", 0), GB_VIRTUAL_CLASS(), GB_HOOK_CHECK(check_valid_item),

  GB_PROPERTY_READ("Key", "s", CTREEVIEWITEM_key),
  GB_PROPERTY_READ("ParentKey", "s", CTREEVIEWITEM_parent_key),
  GB_PROPERTY("Picture", "Picture", CTREEVIEWITEM_picture),
  GB_PROPERTY("Selected", "b", CTREEVIEWITEM_selected),
  GB_PROPERTY("Text", "s", CTREEVIEWITEM_text),
  GB_METHOD("EnsureVisible", 0, CTREEVIEWITEM_ensure_visible, 0),
  GB_METHOD("Delete", 0, CTREEVIEWITEM_delete, 0),
  GB_METHOD("Clear", 0, CTREEVIEWITEM_clear, 0),

  GB_PROPERTY("Expanded", "b", CTREEVIEWITEM_expanded),
  GB_PROPERTY_READ("Children", "i", CTREEVIEWITEM_count),
  GB_PROPERTY_READ("Count", "i", CTREEVIEWITEM_count),

  GB_PROPERTY_READ("X", "i", CTREEVIEWITEM_x),
  GB_PROPERTY_READ("Y", "i", CTREEVIEWITEM_y),
  GB_PROPERTY_READ("W", "i", CTREEVIEWITEM_w),
  GB_PROPERTY_READ("Width", "i", CTREEVIEWITEM_w),
  GB_PROPERTY_READ("H", "i", CTREEVIEWITEM_h),
  GB_PROPERTY_READ("Height", "i", CTREEVIEWITEM_h),
  /*GB_METHOD("MoveNext", "b", CTREEVIEW_next, 0),
  GB_METHOD("MovePrevious", "b", CTREEVIEW_previous, 0),
  GB_METHOD("MoveChild", "b", CTREEVIEW_child, 0),
  GB_METHOD("MoveParent", "b", CTREEVIEW_parent, 0),
  GB_METHOD("MoveAbove", "b", CTREEVIEW_above, 0),
  GB_METHOD("MoveBelow", "b", CTREEVIEW_below, 0),*/

  GB_METHOD("MoveAfter", 0, CTREEVIEWITEM_move_after, "[(Key)s]"),
  GB_METHOD("MoveBefore", 0, CTREEVIEWITEM_move_before, "[(Key)s]"),
  GB_METHOD("MoveFirst", 0, CTREEVIEWITEM_move_first, 0),
  GB_METHOD("MoveLast", 0, CTREEVIEWITEM_move_last, 0),
  
  GB_METHOD("_get", "s", CTREEVIEWITEM_get, "(Column)i"),
  GB_METHOD("_put", 0, CTREEVIEWITEM_put, "(Text)s(Column)i"),

  GB_PROPERTY("Editable", "b", CTREEVIEWITEM_editable),
  GB_METHOD("Rename", 0, CTREEVIEWITEM_rename, 0),

  //GB_PROPERTY("SortKey", "f", CTREEVIEWITEM_sort_key),

  GB_END_DECLARE
};


/***************************************************************************

  ListView

***************************************************************************/

GB_DESC CListViewDesc[] =
{
  GB_DECLARE("ListView", sizeof(CTREEVIEW)), GB_INHERITS("Control"),

  GB_METHOD("_new", 0, CLISTVIEW_new, "(Parent)Container;"),
  GB_METHOD("_free", 0, CTREEVIEW_free, 0),

  //GB_PROPERTY("Drop", "b", CTREEVIEW_drop),
  GB_PROPERTY("Mode", "i", CTREEVIEW_mode),
  GB_PROPERTY("Sorted", "b", CTREEVIEW_sorted),
  GB_PROPERTY("Editable", "b", CTREEVIEW_editable),
  GB_PROPERTY("ScrollBar", "i", CTREEVIEW_scrollbar),
  GB_PROPERTY("Border", "b", CTREEVIEW_border),
  GB_PROPERTY("Compare", "i", CTREEVIEW_compare),

  GB_PROPERTY_READ("Count", "i", CTREEVIEW_count),

  GB_PROPERTY_READ("Available", "b", CTREEVIEW_available),
  GB_METHOD("MoveTo", "b", CTREEVIEW_move_to, "(Key)s"),
  GB_METHOD("MoveCurrent", "b", CTREEVIEW_move_current, 0),
  GB_METHOD("MoveFirst", "b", CTREEVIEW_first, 0),
  GB_METHOD("MoveLast", "b", CTREEVIEW_last, 0),
  GB_METHOD("MovePrevious", "b", CTREEVIEW_previous, 0),
  GB_METHOD("MoveNext", "b", CTREEVIEW_next, 0),
  GB_METHOD("MoveAbove", "b", CTREEVIEW_above, 0),
  GB_METHOD("MoveBelow", "b", CTREEVIEW_below, 0),
  GB_METHOD("MoveBack", "b", CTREEVIEW_back, 0),

  GB_METHOD("_get", ".ListViewItem", CTREEVIEW_get, "(Key)s"),

  GB_METHOD("Clear", 0, CTREEVIEW_clear, 0),
  GB_METHOD("Add", ".ListViewItem", CLISTVIEW_add, "(Key)s(Text)s[(Picture)Picture;(After)s]"),
  GB_METHOD("Remove", 0, CTREEVIEW_remove, "(Key)s"),
  GB_METHOD("Exist", "b", CTREEVIEW_exist, "(Key)s"),
  GB_METHOD("Find", "b", CTREEVIEW_find, "(X)i(Y)i"),
  //GB_METHOD("FindText", "b", CTREEVIEW_find, "(X)i(Y)i"),
  GB_METHOD("SelectAll", 0, CTREEVIEW_select_all, "[(Selected)b]"),

  GB_PROPERTY_READ("Current", ".ListViewItem", CTREEVIEW_current),
  GB_PROPERTY_READ("Key", "s", CTREEVIEW_key),
  GB_PROPERTY_READ("Item", ".ListViewItem", CTREEVIEW_item),

  GB_PROPERTY_READ("ClientWidth", "i", CTREEVIEW_client_width),
  GB_PROPERTY_READ("ClientW", "i",  CTREEVIEW_client_width),
  GB_PROPERTY_READ("ClientHeight", "i", CTREEVIEW_client_height),
  GB_PROPERTY_READ("ClientH", "i", CTREEVIEW_client_height),

  GB_EVENT("Select", 0, 0, &EVENT_Select),
  GB_EVENT("Activate", 0, 0, &EVENT_Activate),
  GB_EVENT("Click", 0, 0, &EVENT_Click),
  GB_EVENT("Rename", 0, 0, &EVENT_Rename),
  GB_EVENT("Cancel", 0, 0, &EVENT_Cancel),
  GB_EVENT("Compare", 0, "(Key)s(OtherKey)s", &EVENT_Compare),

	LISTVIEW_DESCRIPTION,

  GB_END_DECLARE
};



/***************************************************************************

  TreeView

***************************************************************************/

GB_DESC CTreeViewDesc[] =
{
  GB_DECLARE("TreeView", sizeof(CTREEVIEW)), GB_INHERITS("Control"),

  GB_METHOD("_new", 0, CTREEVIEW_new, "(Parent)Container;"),
  GB_METHOD("_free", 0, CTREEVIEW_free, 0),

  //GB_PROPERTY("Drop", "b", CTREEVIEW_drop),
  GB_PROPERTY("Mode", "i", CTREEVIEW_mode),
  //GB_PROPERTY("Root", "b", CTREEVIEW_show_root),
  GB_PROPERTY("Sorted", "b", CTREEVIEW_sorted),
  GB_PROPERTY("Editable", "b", CTREEVIEW_editable),
  GB_PROPERTY("ScrollBar", "i", CTREEVIEW_scrollbar),
  GB_PROPERTY("Border", "b", CTREEVIEW_border),
  GB_PROPERTY("Compare", "i", CTREEVIEW_compare),

  GB_PROPERTY_READ("Count", "i", CTREEVIEW_count),

  GB_PROPERTY_READ("Available", "b", CTREEVIEW_available),
  
  GB_METHOD("MoveCurrent", "b", CTREEVIEW_move_current, 0),
  GB_METHOD("MoveTo", "b", CTREEVIEW_move_to, "(Key)s"),
  GB_METHOD("MoveFirst", "b", CTREEVIEW_first, 0),
  GB_METHOD("MoveLast", "b", CTREEVIEW_last, 0),
  GB_METHOD("MoveNext", "b", CTREEVIEW_next, 0),
  GB_METHOD("MovePrevious", "b", CTREEVIEW_previous, 0),
  GB_METHOD("MoveChild", "b", CTREEVIEW_child, 0),
  GB_METHOD("MoveParent", "b", CTREEVIEW_parent, 0),
  GB_METHOD("MoveAbove", "b", CTREEVIEW_above, 0),
  GB_METHOD("MoveBelow", "b", CTREEVIEW_below, 0),
  GB_METHOD("MoveBack", "b", CTREEVIEW_back, 0),

  GB_METHOD("_get", ".TreeViewItem", CTREEVIEW_get, "(Key)s"),

  GB_METHOD("Clear", 0, CTREEVIEW_clear, 0),
  GB_METHOD("Add", ".TreeViewItem", CTREEVIEW_add, "(Key)s(Text)s[(Picture)Picture;(Parent)s(After)s]"),
  GB_METHOD("Remove", 0, CTREEVIEW_remove, "(Key)s"),
  GB_METHOD("Exist", "b", CTREEVIEW_exist, "(Key)s"),
  GB_METHOD("Find", "b", CTREEVIEW_find, "(X)i(Y)i"),
  GB_METHOD("SelectAll", 0, CTREEVIEW_select_all, "[(Selected)b]"),

  GB_PROPERTY_READ("Current", ".TreeViewItem", CTREEVIEW_current),
  GB_PROPERTY_READ("Key", "s", CTREEVIEW_key),
  GB_PROPERTY_READ("Item", ".TreeViewItem", CTREEVIEW_item),

  GB_PROPERTY_READ("ClientWidth", "i", CTREEVIEW_client_width),
  GB_PROPERTY_READ("ClientW", "i",  CTREEVIEW_client_width),
  GB_PROPERTY_READ("ClientHeight", "i", CTREEVIEW_client_height),
  GB_PROPERTY_READ("ClientH", "i", CTREEVIEW_client_height),

  GB_EVENT("Select", 0, 0, &EVENT_Select),
  GB_EVENT("Activate", 0, 0, &EVENT_Activate),
  GB_EVENT("Click", 0, 0, &EVENT_Click),
  GB_EVENT("Rename", 0, 0, &EVENT_Rename),
  GB_EVENT("Cancel", 0, 0, &EVENT_Cancel),
  GB_EVENT("Compare", 0, "(Key)s(OtherKey)s", &EVENT_Compare),
  GB_EVENT("Expand", 0, 0, &EVENT_Expand),
  GB_EVENT("Collapse", 0, 0, &EVENT_Collapse),

	TREEVIEW_DESCRIPTION,

  GB_END_DECLARE
};


/***************************************************************************

  ColumnView

***************************************************************************/

GB_DESC CColumnViewColumnDesc[] =
{
  GB_DECLARE(".ColumnViewColumn", 0), GB_VIRTUAL_CLASS(),

  GB_PROPERTY("Text", "s", CLISTVIEW_column_text),
  GB_PROPERTY("Alignment", "i", CLISTVIEW_column_alignment),
  GB_PROPERTY("W", "i", CCOLUMNVIEW_column_width),
  GB_PROPERTY("Width", "i", CCOLUMNVIEW_column_width),
  //GB_PROPERTY("AutoResize", "b", CLISTVIEW_column_auto_resize),

  GB_END_DECLARE
};

GB_DESC CColumnViewColumnsDesc[] =
{
  GB_DECLARE(".ColumnViewColumns", 0), GB_VIRTUAL_CLASS(),

  GB_METHOD("_get", ".ColumnViewColumn", CLISTVIEW_columns_get, "(Column)i"),

  GB_PROPERTY("Count", "i", CLISTVIEW_columns_count),
  GB_PROPERTY("Sort", "i", CLISTVIEW_columns_sort),
  GB_PROPERTY("Ascending", "b", CLISTVIEW_columns_ascending),

  GB_END_DECLARE
};

GB_DESC CColumnViewDesc[] =
{
  GB_DECLARE("ColumnView", sizeof(CTREEVIEW)), GB_INHERITS("Control"),

  GB_METHOD("_new", 0, CCOLUMNVIEW_new, "(Parent)Container;"),
  GB_METHOD("_free", 0, CTREEVIEW_free, 0),

  GB_PROPERTY("Mode", "i", CTREEVIEW_mode),
  GB_PROPERTY("Sorted", "b", CTREEVIEW_sorted),
  GB_PROPERTY("Editable", "b", CTREEVIEW_editable),
  GB_PROPERTY("ScrollBar", "i", CTREEVIEW_scrollbar),
  GB_PROPERTY("Border", "b", CTREEVIEW_border),
  GB_PROPERTY("Compare", "i", CTREEVIEW_compare),

  GB_PROPERTY_READ("Count", "i", CTREEVIEW_count),

  GB_PROPERTY_READ("Available", "b", CTREEVIEW_available),
  GB_METHOD("MoveCurrent", "b", CTREEVIEW_move_current, 0),
  GB_METHOD("MoveTo", "b", CTREEVIEW_move_to, "(Key)s"),
  GB_METHOD("MoveFirst", "b", CTREEVIEW_first, 0),
  GB_METHOD("MoveLast", "b", CTREEVIEW_last, 0),
  GB_METHOD("MoveNext", "b", CTREEVIEW_next, 0),
  GB_METHOD("MovePrevious", "b", CTREEVIEW_previous, 0),
  GB_METHOD("MoveChild", "b", CTREEVIEW_child, 0),
  GB_METHOD("MoveParent", "b", CTREEVIEW_parent, 0),
  GB_METHOD("MoveAbove", "b", CTREEVIEW_above, 0),
  GB_METHOD("MoveBelow", "b", CTREEVIEW_below, 0),
  GB_METHOD("MoveBack", "b", CTREEVIEW_back, 0),

  GB_METHOD("_get", ".ColumnViewItem", CTREEVIEW_get, "(Key)s"),

  GB_METHOD("Clear", 0, CTREEVIEW_clear, 0),
  GB_METHOD("Add", ".ColumnViewItem", CTREEVIEW_add, "(Key)s(Text)s[(Picture)Picture;(Parent)s(After)s]"),
  GB_METHOD("Remove", 0, CTREEVIEW_remove, "(Key)s"),
  GB_METHOD("Exist", "b", CTREEVIEW_exist, "(Key)s"),
  GB_METHOD("Find", "b", CTREEVIEW_find, "(X)i(Y)i"),
  GB_METHOD("SelectAll", 0, CTREEVIEW_select_all, "[(Selected)b]"),

  GB_PROPERTY_READ("Current", ".ColumnViewItem", CTREEVIEW_current),
  GB_PROPERTY_READ("Key", "s", CTREEVIEW_key),
  GB_PROPERTY_READ("Item", ".ColumnViewItem", CTREEVIEW_item),

  GB_EVENT("Select", 0, 0, &EVENT_Select),
  GB_EVENT("Activate", 0, 0, &EVENT_Activate),
  GB_EVENT("Click", 0, 0, &EVENT_Click),
  GB_EVENT("Rename", 0, 0, &EVENT_Rename),
  GB_EVENT("Cancel", 0, 0, &EVENT_Cancel),
  GB_EVENT("Compare", 0, "(Key)s(OtherKey)s", &EVENT_Compare),
  GB_EVENT("Expand", 0, 0, &EVENT_Expand),
  GB_EVENT("Collapse", 0, 0, &EVENT_Collapse),
  //GB_EVENT("ColumnClick", 0, "(Column)i", &EVENT_ColumnClick),

  GB_PROPERTY_SELF("Columns", ".ColumnViewColumns"),

  GB_PROPERTY("Resizable", "b", CCOLUMNVIEW_resizable),
  GB_PROPERTY("Header", "b", CTREEVIEW_header),
  GB_PROPERTY("AutoResize", "b", CLISTVIEW_auto_resize),

  GB_PROPERTY_READ("ClientWidth", "i", CTREEVIEW_client_width),
  GB_PROPERTY_READ("ClientW", "i",  CTREEVIEW_client_width),
  GB_PROPERTY_READ("ClientHeight", "i", CTREEVIEW_client_height),
  GB_PROPERTY_READ("ClientH", "i", CTREEVIEW_client_height),

  GB_PROPERTY("Settings", "s", CCOLUMNVIEW_settings),

	COLUMNVIEW_DESCRIPTION,

  GB_END_DECLARE
};
