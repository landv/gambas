/***************************************************************************

  CIconView.cpp

  The IconView class

  (c) 2000-2007 Benoit Minisini <gambas@users.sourceforge.net>

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

#include <qapplication.h>
#include <qheader.h>
#include <qpalette.h>
#include <qscrollview.h>
#include <qiconview.h>
#if QT_VERSION >= 0x030200
#include <qobjectlist.h>
#else
#include <qobjcoll.h>
#endif

#include "gambas.h"

#include "CWidget.h"
#include "CPicture.h"
#include "CIconView.h"
#include "CConst.h"

DECLARE_EVENT(EVENT_Select);    /* selection change */
DECLARE_EVENT(EVENT_Click);     /* simple click */
DECLARE_EVENT(EVENT_Activate);  /* double click */
DECLARE_EVENT(EVENT_Rename);
DECLARE_EVENT(EVENT_Cancel);
DECLARE_EVENT(EVENT_Compare);

static int _selection_mode[] = 
{
	SELECT_NONE, QIconView::NoSelection, 
	SELECT_SINGLE, QIconView::Single, 
	SELECT_MULTIPLE, QIconView::Extended, 
	CONST_MAGIC
};

/***************************************************************************

  class MyIconViewItem

***************************************************************************/

void MyIconViewItem::initData(void)
{
  picture = NULL;
  key = NULL;
}


MyIconViewItem::MyIconViewItem(QIconView *parent)
: QIconViewItem(parent, " ")
{
  initData();
}

MyIconViewItem::MyIconViewItem(QIconView *parent, MyIconViewItem *after)
: QIconViewItem(parent, after)
{
  initData();
}

MyIconViewItem::~MyIconViewItem()
{
  //CIconView *ob = (CIconView *)CWidget::get(listView());

  //if (ob->item == this)
  //  ob->item = NULL;

  if (container->item == this)
    container->item = NULL;

  if (container->save == this)
    container->save = NULL;

  container->dict->remove(key);

  GB.Unref(POINTER(&picture));
  GB.FreeString(&key);
}


void MyIconViewItem::setPicture(GB_OBJECT *pict)
{
  SET_PIXMAP(setPixmap, &picture, pict);
}


int MyIconViewItem::compare(QIconViewItem *i) const
{
	int ret;
	
  if (!GB.CanRaise(container, EVENT_Compare))
    ret = QIconViewItem::compare(i);
	else
	{
  	container->compare = 0;
  	GB.Raise(container, EVENT_Compare, 2, GB_T_STRING, key, 0, GB_T_STRING, ((MyIconViewItem *)i)->key, 0);
  	ret = container->compare;
  }
  
  if (!container->asc)
  	ret = (-ret);
  	
  return ret;
}


/***************************************************************************

  class MyIconView

***************************************************************************/

MyIconView::MyIconView(QWidget *parent)
: QIconView(parent)
{
  setMouseTracking(false);
  viewport()->setMouseTracking(false);
  setArrangementMode(QIconView::LeftToRight);
  setWordWrapIconText(true);
  setGridY(64);
  setSpacing(8);
  setGridWidth(0);
}

void MyIconView::startDrag()
{
}


int MyIconView::getArrangementMode()
{
	if (itemsMovable())
		return ARRANGEMENT_FREE;
	else
		return arrangement();
}

void MyIconView::setArrangementMode(int arr)
{
	if (arr == ARRANGEMENT_FREE)
	{
		setItemsMovable(true);
		setAutoArrange(false);
		setResizeMode(QIconView::Fixed);
	}
	else
	{
		void *_object = CWidget::get(this);
		setAutoArrange(true);
		setItemsMovable(false);
		setArrangement((QIconView::Arrangement)arr);
		if (THIS->sorted)
			sort(THIS->asc);
		setResizeMode(QIconView::Adjust);
		arrangeItemsInGrid(true);
	}
}

void MyIconView::setGridWidth(int w)
{
	if (w <= 0) w = 0;
	_grid_width = w;
	
	if (w == 0)
		setGridX(192);
	else
		setGridX(w * MAIN_scale);
		
	if (!itemsMovable())
		arrangeItemsInGrid(true);
}


/***************************************************************************

  IconView

***************************************************************************/

BEGIN_METHOD(CICONVIEW_new, GB_OBJECT parent)

  MyIconView *wid = new MyIconView(QCONTAINER(VARG(parent)));

  QObject::connect(wid, SIGNAL(selectionChanged()), &CIconView::manager, SLOT(selected()));
  QObject::connect(wid, SIGNAL(doubleClicked(QIconViewItem *)), &CIconView::manager, SLOT(activated(QIconViewItem *)));
  QObject::connect(wid, SIGNAL(clicked(QIconViewItem *)), &CIconView::manager, SLOT(clicked(QIconViewItem *)));
  QObject::connect(wid, SIGNAL(itemRenamed(QIconViewItem *)), &CIconView::manager, SLOT(renamed(QIconViewItem *)));

  CWIDGET_new(wid, (void *)_object, "IconView");

  THIS->dict = new QAsciiDict<MyIconViewItem>;
  THIS->sorted = false;
  THIS->asc = true;
  THIS->item = NULL;
  THIS->save = NULL;
  THIS->editable = false;
  
  wid->show();

END_METHOD


BEGIN_METHOD_VOID(CICONVIEW_free)

  delete THIS->dict;

  //CALL_METHOD_VOID(CWIDGET_delete);

END_METHOD


BEGIN_METHOD_VOID(CICONVIEW_clear)

  THIS->dict->clear();
  WIDGET->clear();

END_METHOD


BEGIN_METHOD(CICONVIEW_add, GB_STRING key; GB_STRING text; GB_OBJECT picture; GB_STRING after)

  MyIconViewItem *item;
  QIconView *wid = WIDGET;
  char *key = GB.ToZeroString(ARG(key));
  MyIconViewItem *after = NULL;
  char *akey;

  if (*key == 0)
  {
    GB.Error("Null key");
    return;
  }

  item = (*THIS->dict)[key];
  if (item != NULL)
  {
    GB.Error("Key already used");
    return;
  }

  if (!MISSING(after))
  {
    akey = GB.ToZeroString(ARG(after));
    if (*akey)
    {
      after = (*THIS->dict)[akey];
      if (after == NULL)
      {
        GB.Error("After item does not exist");
        return;
      }
    }
  }

  if (after == NULL)
    item = new MyIconViewItem(wid);
  else
    item = new MyIconViewItem(wid, after);

  item->setText(QSTRING_ARG(text));

  GB.StoreString(ARG(key), &item->key);
  THIS->dict->insert(item->key, item);

  if (!MISSING(picture))
    item->setPicture(ARG(picture));

  item->container = THIS;

  item->setRenameEnabled(THIS->editable);

  THIS->item = item;
  RETURN_SELF();

END_METHOD


BEGIN_METHOD(CICONVIEW_remove, GB_STRING key)

  MyIconViewItem *item;
  char *key = GB.ToZeroString(ARG(key));

  item = CIconView::getItem(THIS, key);
  if (!item)
    return;

  delete item;

END_METHOD


BEGIN_METHOD(CICONVIEW_exist, GB_STRING key)

  GB.ReturnBoolean((*THIS->dict)[GB.ToZeroString(ARG(key))] != 0);

END_METHOD


BEGIN_METHOD(CICONVIEW_get, GB_STRING key)

  MyIconViewItem *item;
  char *key = GB.ToZeroString(ARG(key));

  item = CIconView::getItem(THIS, key);
  if (!item)
    return;

  THIS->item = item;
  RETURN_SELF();

END_METHOD


BEGIN_PROPERTY(CICONVIEW_mode)

  if (READ_PROPERTY)
    GB.ReturnInteger(CCONST_convert(_selection_mode, WIDGET->selectionMode(), SELECT_NONE, false));
  else
    WIDGET->setSelectionMode((QIconView::SelectionMode)CCONST_convert(_selection_mode, VPROP(GB_INTEGER), SELECT_NONE, true));

END_PROPERTY


BEGIN_PROPERTY(CICONVIEW_current)

  MyIconViewItem *item = (MyIconViewItem *)(WIDGET->currentItem());

  THIS->item = item;

  if (item == 0)
    GB.ReturnNull();
  else
    RETURN_SELF();

END_PROPERTY


BEGIN_PROPERTY(CICONVIEW_key)

  MyIconViewItem *item = (MyIconViewItem *)(WIDGET->currentItem());

  THIS->item = item;

  if (item == 0)
    GB.ReturnNull();
  else
    GB.ReturnString(item->key);

END_PROPERTY


BEGIN_PROPERTY(CICONVIEW_item)

  if (THIS->item == 0)
    GB.ReturnNull();
  else
    RETURN_SELF();

END_PROPERTY


BEGIN_PROPERTY(CICONVIEW_available)

  GB.ReturnBoolean(THIS->item != 0);

END_PROPERTY


BEGIN_METHOD(CICONVIEW_find, GB_INTEGER x; GB_INTEGER y)

  QPoint p(VARG(x), VARG(y));
  MyIconViewItem *item;

  WIDGET->viewport()->mapFrom(WIDGET, p);
  item = (MyIconViewItem *)WIDGET->findItem(p);

  THIS->item = item;

  GB.ReturnBoolean(THIS->item == 0);

END_METHOD





static void set_sorting(void *_object, bool sorted, bool asc)
{
  if (sorted == THIS->sorted && asc == THIS->asc)
    return;

  THIS->sorted = sorted;
  THIS->asc = asc;

  //qDebug("setSorting(%s, %s)", THIS->sorted ? "true" : "false", THIS->asc ? "true" : "false");

  WIDGET->setSorting(THIS->sorted, THIS->asc);
  if (THIS->sorted)
  {
    //qDebug("sort(%s)", THIS->asc ? "true" : "false");
    WIDGET->sort();
  }
}


BEGIN_PROPERTY(CICONVIEW_sorted)

  if (READ_PROPERTY)
    GB.ReturnBoolean(THIS->sorted);
  else
    set_sorting(THIS, VPROP(GB_BOOLEAN), THIS->asc);

END_PROPERTY


BEGIN_PROPERTY(CICONVIEW_editable)

  if (READ_PROPERTY)
    GB.ReturnBoolean(THIS->editable);
  else
    THIS->editable = VPROP(GB_BOOLEAN);

END_PROPERTY


BEGIN_PROPERTY(CICONVIEW_ascending)

  if (READ_PROPERTY)
    GB.ReturnBoolean(THIS->asc);
  else
    set_sorting(_object, THIS->sorted, VPROP(GB_BOOLEAN));

END_PROPERTY


BEGIN_PROPERTY(CICONVIEW_count)

  GB.ReturnInteger(WIDGET->count());

END_PROPERTY


static void return_item(void *_object, MyIconViewItem *item)
{
  if (!item)
    THIS->save = THIS->item;
  THIS->item = item;

  GB.ReturnBoolean(item == 0);
}


BEGIN_METHOD_VOID(CICONVIEW_back)

  MyIconViewItem *item = THIS->save;
  THIS->save = 0;
  return_item(THIS, item);

END_METHOD

BEGIN_METHOD(CICONVIEW_move_to, GB_STRING key)

  char *key = GB.ToZeroString(ARG(key));
  MyIconViewItem *item = (*THIS->dict)[key];

  return_item(THIS, item);

END_METHOD


BEGIN_METHOD_VOID(CICONVIEW_first)

  return_item(THIS, (MyIconViewItem *)(WIDGET->firstItem()));

END_METHOD


BEGIN_METHOD_VOID(CICONVIEW_next)

  return_item(THIS, (MyIconViewItem *)(THIS->item->nextItem()));

END_METHOD


BEGIN_PROPERTY(CICONVIEW_scrollbar)

  //QScrollView *wid = (QScrollView *)QWIDGET(_object);
  int scroll;

  if (READ_PROPERTY)
  {
    scroll = 0;
    if (WIDGET->hScrollBarMode() == QScrollView::Auto)
      scroll += 1;
    if (WIDGET->vScrollBarMode() == QScrollView::Auto)
      scroll += 2;

    GB.ReturnInteger(scroll);
  }
  else
  {
    scroll = VPROP(GB_INTEGER) & 3;
    WIDGET->setHScrollBarMode( (scroll & 1) ? QScrollView::Auto : QScrollView::AlwaysOff);
    WIDGET->setVScrollBarMode( (scroll & 2) ? QScrollView::Auto : QScrollView::AlwaysOff);
  }

END_PROPERTY




BEGIN_PROPERTY(CICONVIEWITEM_key)

  GB.ReturnString(THIS->item->key);

END_PROPERTY


BEGIN_PROPERTY(CICONVIEWITEM_picture)

  if (READ_PROPERTY)
    GB.ReturnObject(THIS->item->picture);
  else
    THIS->item->setPicture(PROP(GB_OBJECT));

END_PROPERTY


BEGIN_PROPERTY(CICONVIEWITEM_selected)

  MyIconViewItem *it = THIS->item;

  if (READ_PROPERTY)
    GB.ReturnBoolean(it->isSelected());
  else
    it->iconView()->setSelected(it, VPROP(GB_BOOLEAN), true);

END_PROPERTY


BEGIN_PROPERTY(CICONVIEWITEM_text)

  MyIconViewItem *it = THIS->item;

  if (READ_PROPERTY)
    GB.ReturnNewZeroString(TO_UTF8(it->text()));
  else
    it->setText(QSTRING_PROP());

END_PROPERTY


BEGIN_METHOD_VOID(CICONVIEWITEM_ensure_visible)

  //qDebug("ensure visible: %p", THIS->item);

  WIDGET->ensureItemVisible(THIS->item);

END_METHOD


BEGIN_METHOD_VOID(CICONVIEWITEM_delete)

  THIS->dict->remove(THIS->item->key);

  delete THIS->item;

END_PROPERTY


/*
BEGIN_METHOD_VOID(CICONVIEWITEM_init)

  CLASS_Item = GB.FindClass("Item");

END_METHOD
*/


BEGIN_PROPERTY(CICONVIEW_client_width)

  GB.ReturnInteger(WIDGET->width() - WIDGET->frameWidth() * 2);

END_PROPERTY


BEGIN_PROPERTY(CICONVIEW_client_height)

  GB.ReturnInteger(WIDGET->height() - WIDGET->frameWidth() * 2);

END_PROPERTY


BEGIN_METHOD(CICONVIEW_select_all, GB_BOOLEAN on)

  if (MISSING(on) || VARG(on))
    WIDGET->selectAll(true);
  else
    WIDGET->selectAll(false);

END_METHOD


BEGIN_PROPERTY(CICONVIEW_grid_w)

  if (READ_PROPERTY)
  	GB.ReturnInteger(WIDGET->gridWidth());
  else
  	WIDGET->setGridWidth(VPROP(GB_INTEGER));

END_PROPERTY


/*BEGIN_PROPERTY(CICONVIEW_grid_h)

  if (READ_PROPERTY)
    GB.ReturnInteger(WIDGET->gridY() / MAIN_scale);
  else
  {
    WIDGET->setGridY(VPROP(GB_INTEGER) * MAIN_scale);
    if (!WIDGET->itemsMovable())
      WIDGET->arrangeItemsInGrid(true);
  }

END_PROPERTY*/


/*BEGIN_PROPERTY(CICONVIEW_arrangement)

  if (READ_PROPERTY)
    GB.ReturnInteger(WIDGET->getArrangementMode());
  else
	  WIDGET->setArrangementMode(VPROP(GB_INTEGER));

END_PROPERTY*/


/*BEGIN_PROPERTY(CICONVIEW_word_wrap)

  if (READ_PROPERTY)
    GB.ReturnBoolean(WIDGET->wordWrapIconText());
  else
    WIDGET->setWordWrapIconText(VPROP(GB_BOOLEAN));

END_PROPERTY*/


BEGIN_PROPERTY(CICONVIEW_compare)

  if (READ_PROPERTY)
    GB.ReturnInteger(THIS->compare);
  else
    THIS->compare = VPROP(GB_INTEGER);

END_PROPERTY


BEGIN_PROPERTY(CICONVIEWITEM_x)

  if (READ_PROPERTY)
    GB.ReturnInteger(THIS->item->x());
  else
    THIS->item->move(VPROP(GB_INTEGER), THIS->item->y());

END_PROPERTY


BEGIN_PROPERTY(CICONVIEWITEM_y)

  if (READ_PROPERTY)
    GB.ReturnInteger(THIS->item->y());
  else
    THIS->item->move(THIS->item->x(), VPROP(GB_INTEGER));

END_PROPERTY


BEGIN_PROPERTY(CICONVIEWITEM_w)

  GB.ReturnInteger(THIS->item->width());

END_PROPERTY


BEGIN_PROPERTY(CICONVIEWITEM_h)

  GB.ReturnInteger(THIS->item->height());

END_PROPERTY


BEGIN_METHOD(CICONVIEWITEM_move, GB_INTEGER x; GB_INTEGER y)

  THIS->item->move(VARG(x), VARG(y));

END_METHOD


BEGIN_PROPERTY(CICONVIEWITEM_editable)

  if (READ_PROPERTY)
    GB.ReturnBoolean(THIS->item->renameEnabled());
  else
    THIS->item->setRenameEnabled(VPROP(GB_BOOLEAN));

END_PROPERTY


BEGIN_METHOD_VOID(CICONVIEWITEM_rename)

  THIS->item->rename();

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

  GB_CONSTANT("Free", "i", ARRANGEMENT_FREE),
  GB_CONSTANT("Row", "i", QIconView::LeftToRight),
  GB_CONSTANT("Column", "i", QIconView::TopToBottom),
  GB_CONSTANT("LeftRight", "i", QIconView::LeftToRight),
  GB_CONSTANT("TopBottom", "i", QIconView::TopToBottom),

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
  GB_PROPERTY("Border", "b", CWIDGET_border_simple),
  GB_PROPERTY("ScrollBar", "i", CWIDGET_scrollbar),
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

	ICONVIEW_DESCRIPTION,

  GB_END_DECLARE
};


/***************************************************************************

  class CIconView

***************************************************************************/

CIconView CIconView::manager;


MyIconViewItem *CIconView::getItem(CICONVIEW *treeview, char *key)
{
  MyIconViewItem *item = (*treeview->dict)[key];

  if (item == 0)
  {
    GB.Error("Unknown item '&1'", key);
    return NULL;
  }

  return item;
}

void CIconView::raiseEvent(int ev, QIconViewItem *it)
{
  GET_SENDER(_object);
  MyIconViewItem *old = THIS->item;

  if (!it)
    it = WIDGET->currentItem();
  THIS->item = (MyIconViewItem *)it;
  RAISE_EVENT(ev);

  THIS->item = old;
}

void CIconView::selected(void)
{
  raiseEvent(EVENT_Select, 0);
  #if QT_VERSION < 0x030200
  raiseEvent(EVENT_Click, 0);
  #endif
}

void CIconView::activated(QIconViewItem *it)
{
  if (!it)
    return;
  raiseEvent(EVENT_Activate, it);
}

void CIconView::clicked(QIconViewItem *it)
{
  if (!it)
    return;
  raiseEvent(EVENT_Click, it);
}

void CIconView::renamed(QIconViewItem *it)
{
  raiseEvent(EVENT_Rename, it);
}

