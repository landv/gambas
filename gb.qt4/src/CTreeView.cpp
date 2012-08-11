/***************************************************************************

  CTreeView.cpp

  (c) 2000-2012 Benoît Minisini <gambas@users.sourceforge.net>

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
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
  MA 02110-1301, USA.

***************************************************************************/

#define __CTREEVIEW_CPP

#include <qapplication.h>
#include <q3header.h>
#include <qpalette.h>
#include <q3scrollview.h>
#include <q3listview.h>
#include <qobject.h>
//Added by qt3to4:
#include <QDragLeaveEvent>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>

#include "gambas.h"

#include "CWidget.h"
#include "CPicture.h"
#include "CClipboard.h"
#include "CTreeView.h"
#include "CConst.h"

DECLARE_EVENT(EVENT_Select);
DECLARE_EVENT(EVENT_Click);
DECLARE_EVENT(EVENT_Activate);
DECLARE_EVENT(EVENT_Rename);
DECLARE_EVENT(EVENT_Cancel);
DECLARE_EVENT(EVENT_Compare);
DECLARE_EVENT(EVENT_Expand);
DECLARE_EVENT(EVENT_Collapse);
//DECLARE_EVENT(EVENT_ColumnClick);


static int _selection_mode[] = 
{
	SELECT_NONE, Q3ListView::NoSelection, 
	SELECT_SINGLE, Q3ListView::Single, 
	SELECT_MULTIPLE, Q3ListView::Extended, 
	CONST_MAGIC
};

/***************************************************************************

  class MyListViewItem

***************************************************************************/

void MyListViewItem::initData(CTREEVIEW *cont)
{
  picture = 0;
  key = 0;
  last = 0;
  prev = 0;
  container = cont;
  //sortKey = NULL;
  setDropEnabled(true);
}


MyListViewItem::MyListViewItem(CTREEVIEW *cont, MyListView *parent)
: Q3ListViewItem(parent, parent->last)
{
  initData(cont);
  prev = parent->last;
  parent->last = this;
}

MyListViewItem::MyListViewItem(CTREEVIEW *cont, MyListView *parent, MyListViewItem *after)
: Q3ListViewItem(parent, after)
{
  initData(cont);
  prev = after;

  if (after == parent->last)
    parent->last = this;
  else
  {
    MyListViewItem *n = ((MyListViewItem *)nextSibling());
    n->prev = this;
  }
}

MyListViewItem::MyListViewItem(CTREEVIEW *cont, MyListViewItem *parent)
: Q3ListViewItem(parent, parent->last)
{
  initData(cont);
  prev = parent->last;
  parent->last = this;
}

MyListViewItem::MyListViewItem(CTREEVIEW *cont, MyListViewItem *parent, MyListViewItem *after)
: Q3ListViewItem(parent, after)
{
  initData(cont);

  if (after == parent->last)
    parent->last = this;
  else
  {
    MyListViewItem *n = ((MyListViewItem *)nextSibling());
    n->prev = this;
  }
}

MyListViewItem::~MyListViewItem()
{
  MyListView *lw = (MyListView *)container->widget.widget;
  MyListViewItem *par = (MyListViewItem *)parent();
  MyListViewItem *next = (MyListViewItem *)nextSibling();

  //qDebug("< delete item %s %p", key, this);

  if (par)
  {
    if (par->last == this)
      par->last = prev;
    // Why that ?
    //par->takeItem(this);
  }
  else
  {
    if (lw->last == this)
      lw->last = prev;
    // Why that ?
    //lw->takeItem(this);
  }

  if (next)
    next->prev = prev;

  //qDebug("container = %p ->item = %p", container, container->item);

  if (container->item == this)
    container->item = 0;
  if (container->save == this)
    container->save = 0;

  //if (container->last == this)
  //  container->last = NULL;

  container->dict->remove(key);

  GB.Unref(POINTER(&picture));
  GB.FreeString(&key);

  //qDebug("container = %p ->item = %p", container, container->item);
  //qDebug(">");
}


void MyListViewItem::setPicture(GB_OBJECT *pict)
{
  SET_PIXMAP(setPixmap, &picture, pict);
}

int MyListViewItem::compare(Q3ListViewItem *i, int col, bool ascending) const
{
  if (!GB.CanRaise(container, EVENT_Compare) || col != container->sorted)
    return Q3ListViewItem::compare(i, col, ascending);

  container->compare = 0;
  GB.Raise(container, EVENT_Compare, 2, GB_T_STRING, key, 0, GB_T_STRING, ((MyListViewItem *)i)->key, 0);
  return container->compare;
}

#if 0
bool MyListViewItem::acceptDrop(const QMimeSource *mime) const
{
	/*QObject *e = (QObject *)e;

	qDebug("MyListViewItem::acceptDrop");

	if (!QWIDGET(container)->acceptDrops())
		return false;

	if (e->isA("QDragEnterEvent"))
		return CDRAG_drag_enter(QWIDGET(container), (CWIDGET *)container, (QDropEvent *)e);
	else if (e->isA("QDragMoveEvent"))
		return CDRAG_drag_move(QWIDGET(container), (CWIDGET *)container, (QDropEvent *)e);
	else
		return false;*/
	return true;
}

void MyListViewItem::dropped(QDropEvent *e)
{
	/*qDebug("MyListViewItem::dropped");

	if (QWIDGET(container)->acceptDrops())
		CDRAG_drag_drop(QWIDGET(container), (CWIDGET *)container, e);*/
}
#endif

void MyListViewItem::startRename(int col)
{
	//qDebug("before: %s", text(col).latin1());
	GB.FreeString(&container->before);
	container->before = GB.NewZeroString(TO_UTF8(text(col)));
	Q3ListViewItem::startRename(col);
}

static void post_cancel_event(void *_object)
{
  GB.Raise(THIS, EVENT_Cancel, 0);
  GB.Unref(&_object);
}

void MyListViewItem::cancelRename(int col)
{
	Q3ListViewItem::cancelRename(col);
	container->widget.flag.no_keyboard = FALSE;
	if (GB.CanRaise(container, EVENT_Cancel))
	{
		GB.Ref(container);
		GB.Post((GB_POST_FUNC)post_cancel_event, (intptr_t)container);
	}
}

/***************************************************************************

  class MyListView

***************************************************************************/

MyListView::MyListView(QWidget *parent)
: Q3ListView(parent)
{
  last = 0;
  _auto_resize = true;
  _column = -1;
}

void MyListView::contentsDragEnterEvent(QDragEnterEvent *e)
{
	//qDebug("contentsDragEnterEvent");
	//QListView::contentsDragEnterEvent(e);
}

void MyListView::contentsDragMoveEvent(QDragMoveEvent *e)
{
	//qDebug("contentsDragMoveEvent");
	//QListView::contentsDragMoveEvent(e);
}

void MyListView::contentsDropEvent(QDropEvent *e)
{
	//qDebug("contentsDropEvent");
	//QListView::contentsDropEvent(e);
}

void MyListView::contentsDragLeaveEvent(QDragLeaveEvent *e)
{
	//QListView::contentsDragLeaveEvent(e);
}

void MyListView::setAutoResize(bool a)
{
	_auto_resize = a;
	
	for (int i = 0; i < columns(); i++)
		setAutoResize(i, a);
}

void MyListView::setAutoResize(int col, bool a)
{
	setColumnWidthMode(col, Q3ListView::Manual);
	if (a)
	{
		QFontMetrics fm(font());
		QString s = columnText(col);
		setColumnWidth(col, fm.width(s) + 16);
		setColumnWidthMode(col, Q3ListView::Maximum);
	}
}

int MyListView::minimumWidth(int col)
{
	QFontMetrics fm(font());
	QString s = columnText(col);
	return fm.width(s) + 16;
}

void MyListView::setColumnText(int col, const QString &s)
{
	Q3ListView::setColumnText(col, s);
	if (_auto_resize)
	{
		int w = minimumWidth(col);
		if (columnWidth(col) < w)
			setColumnWidth(col, w);
	}
}

void MyListView::clear()
{
	Q3ListView::clear();
	last = 0;
}

/***************************************************************************

  TreeView

***************************************************************************/

static int check_item(void *_object)
{
	return THIS->item == 0 || CWIDGET_check(_object);
}

static void return_item(void *_object, MyListViewItem *item)
{
  if (!item)
    THIS->save = THIS->item;
  THIS->item = item;

  GB.ReturnBoolean(item == 0);
}


static MyListView *listview_init(void *_object, void *parent)
{
  MyListView *wid;

  wid = new MyListView(QCONTAINER(parent));

  QObject::connect(wid, SIGNAL(selectionChanged()), &CTreeView::manager, SLOT(selected()));
  QObject::connect(wid, SIGNAL(doubleClicked(Q3ListViewItem *)), &CTreeView::manager, SLOT(activated(Q3ListViewItem *)));
  QObject::connect(wid, SIGNAL(pressed(Q3ListViewItem *)), &CTreeView::manager, SLOT(clicked(Q3ListViewItem *)));
  QObject::connect(wid, SIGNAL(itemRenamed(Q3ListViewItem *, int)), &CTreeView::manager, SLOT(renamed(Q3ListViewItem *, int)));

  wid->setSorting(-1);
  wid->setSelectionMode(Q3ListView::Single);
	//wid->setItemMargin(2);

  CWIDGET_new(wid, (void *)_object);

  THIS->dict = new Q3AsciiDict<MyListViewItem>;
  THIS->sorted = -1;
  THIS->asc = true;
  THIS->item = 0;
  THIS->save = 0;
  THIS->rename = false;

  return wid;
}


BEGIN_METHOD(CTREEVIEW_new, GB_OBJECT parent)

  MyListView *wid = listview_init(_object, VARG(parent));

  QObject::connect(wid, SIGNAL(expanded(Q3ListViewItem *)), &CTreeView::manager, SLOT(expanded(Q3ListViewItem *)));
  QObject::connect(wid, SIGNAL(collapsed(Q3ListViewItem *)), &CTreeView::manager, SLOT(collapsed(Q3ListViewItem *)));

  wid->addColumn("-");
  wid->header()->hide();
  wid->setRootIsDecorated(true);

END_METHOD


BEGIN_METHOD(CLISTVIEW_new, GB_OBJECT parent)

  MyListView *wid = listview_init(_object, VARG(parent));

  wid->addColumn("-");
  wid->setColumnWidthMode(0, Q3ListView::Manual);
  wid->setAllColumnsShowFocus(true);
  wid->setHScrollBarMode(Q3ScrollView::AlwaysOff);
  wid->header()->hide();
  wid->setResizeMode(Q3ListView::LastColumn);

END_METHOD


BEGIN_METHOD(CCOLUMNVIEW_new, GB_OBJECT parent)

  MyListView *wid = listview_init(_object, VARG(parent));

  //QObject::connect(wid, SIGNAL(pressed(QListViewItem *, const QPoint &, int)), &CTreeView::manager, SLOT(columnClicked(QListViewItem *, const QPoint &, int)));
  QObject::connect(wid->header(), SIGNAL(clicked(int)), &CTreeView::manager, SLOT(headerClicked(int)));
  //QObject::connect(wid->header(), SIGNAL(sizeChange(int, int, int)), &CTreeView::manager, SLOT(headerSizeChange(int, int, int)));
  QObject::connect(wid, SIGNAL(expanded(Q3ListViewItem *)), &CTreeView::manager, SLOT(expanded(Q3ListViewItem *)));
  QObject::connect(wid, SIGNAL(collapsed(Q3ListViewItem *)), &CTreeView::manager, SLOT(collapsed(Q3ListViewItem *)));

  wid->addColumn(" ");
  //wid->setColumnAlignment(0, Qt::AlignLeft);
  //wid->setColumnWidthMode(0, QListView::Manual);
  wid->header()->setMovingEnabled(false);
  wid->header()->setResizeEnabled(false);
  wid->setAllColumnsShowFocus(true);
  
  wid->setResizeMode(Q3ListView::LastColumn);

END_METHOD


BEGIN_METHOD_VOID(CTREEVIEW_free)

  delete THIS->dict;
  GB.FreeString(&THIS->before);
  //if (THIS->numeric)
  //  delete[] THIS->numeric;

END_METHOD


/*BEGIN_PROPERTY(CTREEVIEW_drop)

  if (READ_PROPERTY)
    GB.ReturnBoolean(WIDGET->acceptDrops());
  else
  {
    WIDGET->setAcceptDrops(VPROP(GB_BOOLEAN));
    WIDGET->viewport()->setAcceptDrops(VPROP(GB_BOOLEAN));
	}

END_PROPERTY*/


BEGIN_METHOD_VOID(CTREEVIEW_clear)

  WIDGET->clear();
  THIS->dict->clear();

END_METHOD

BEGIN_METHOD_VOID(CCOLUMNVIEW_clear)

  WIDGET->clear();
  THIS->dict->clear();
  
  if (WIDGET->isAutoResize())
  {
		//QListView::ResizeMode mode = WIDGET->resizeMode();
		WIDGET->setResizeMode(Q3ListView::NoColumn);
		
		for (int i = 0; i < WIDGET->columns(); i++)
		{
			WIDGET->setColumnWidthMode(i, Q3ListView::Manual);
			WIDGET->setColumnText(i, WIDGET->columnText(i));
			WIDGET->setColumnWidth(i, WIDGET->minimumWidth(i));
			WIDGET->setColumnWidthMode(i, Q3ListView::Maximum);  	
		}
		
		WIDGET->setAutoResize(WIDGET->isAutoResize());
		WIDGET->setResizeMode(Q3ListView::LastColumn);
	}

END_METHOD


BEGIN_METHOD(CTREEVIEW_add, GB_STRING key; GB_STRING text; GB_OBJECT picture; GB_STRING parent; GB_STRING after)

  MyListViewItem *item;
  MyListView *wid = WIDGET;
  char *key = GB.ToZeroString(ARG(key));
  MyListViewItem *parent = NULL;
  MyListViewItem *after = NULL;
  char *akey;

  /*
  if (GB.IsMissing(4))
    qDebug("CTREEVIEW_add: key = %s  parent = NULL", key);
  else
    qDebug("CTREEVIEW_add: key = %s  parent = %s", key, GB.ToZeroString(PARAM(parent)));
  */

  if (*key == 0)
  {
    GB.Error("Null key");
    return;
  }

  item = (*THIS->dict)[key];
  if (item != NULL)
  {
    GB.Error("Key already used: &1", key);
    return;
  }

  if (!MISSING(parent))
  {
    akey = GB.ToZeroString(ARG(parent));
    parent = NULL;
    if (*akey)
    {
      parent = (*THIS->dict)[akey];
      if (parent == NULL)
      {
        GB.Error("Parent item does not exist");
        return;
      }
    }
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
  else
    after = NULL;

  if (parent == NULL)
  {
    if (after == NULL)
      item = new MyListViewItem(THIS, wid);
    else
      item = new MyListViewItem(THIS, wid, after);
  }
  else
  {
    if (after == NULL)
      item = new MyListViewItem(THIS, parent);
    else
      item = new MyListViewItem(THIS, parent, after);
  }

  item->setText(0, QSTRING_ARG(text));

  GB.StoreString(ARG(key), &item->key);
  THIS->dict->insert(item->key, item);

  if (!MISSING(picture))
    item->setPicture(ARG(picture));

  item->setRenameEnabled(0, THIS->rename);

  THIS->item = item;

  RETURN_SELF();

END_METHOD


BEGIN_METHOD(CLISTVIEW_add, GB_STRING key; GB_STRING text; GB_OBJECT picture; GB_STRING after)

  MyListViewItem *item;
  MyListView *wid = QLISTVIEW(_object);
  char *key = GB.ToZeroString(ARG(key));
  MyListViewItem *after = NULL;
  char *akey;

  if (*key == 0)
  {
    GB.Error("Null key");
    return;
  }

  item = (*THIS->dict)[key];
  if (item != NULL)
  {
    GB.Error("Key already used: &1", key);
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
  else
    after = 0; //WIDGET->last;

  if (after == NULL)
    item = new MyListViewItem(THIS, wid);
  else
    item = new MyListViewItem(THIS, wid, after);

  item->setText(0, QSTRING_ARG(text));

  GB.StoreString(ARG(key), &item->key);
  THIS->dict->insert(item->key, item);

  if (!MISSING(picture))
    item->setPicture(ARG(picture));

  item->setRenameEnabled(0, THIS->rename);

  THIS->item = item;

  RETURN_SELF();

END_METHOD


BEGIN_METHOD(CTREEVIEW_remove, GB_STRING key)

  MyListViewItem *lvi;
  char *key = GB.ToZeroString(ARG(key));
  //QListView *wid = QLISTVIEW(_object);

  lvi = CTreeView::getItem(THIS, key);
  if (!lvi)
    return;

  //THIS->dict->remove(key);
  delete lvi;

END_METHOD


BEGIN_METHOD(CTREEVIEW_exist, GB_STRING key)

  GB.ReturnBoolean((*THIS->dict)[GB.ToZeroString(ARG(key))] != 0);

END_METHOD


BEGIN_METHOD(CTREEVIEW_find, GB_INTEGER x; GB_INTEGER y)

  QPoint p(VARG(x), VARG(y));
  MyListViewItem *lvi;

	if (!WIDGET->header()->isHidden())
		p.setY(p.y() - WIDGET->header()->height());

  //qDebug("before: %d %d", p.x(), p.y());
  WIDGET->viewport()->mapFrom(WIDGET, p);
  //qDebug("after: %d %d", p.x(), p.y());
  lvi = (MyListViewItem *)WIDGET->itemAt(p);

  return_item(THIS, lvi);

END_METHOD


BEGIN_METHOD(CTREEVIEW_get, GB_STRING key)

  MyListViewItem *item;
  char *key = GB.ToZeroString(ARG(key));

  item = CTreeView::getItem(THIS, key);
  if (!item)
    return;

  THIS->item = item;
  RETURN_SELF();

END_METHOD


BEGIN_PROPERTY(CTREEVIEW_mode)

  if (READ_PROPERTY)
    GB.ReturnInteger(CCONST_convert(_selection_mode, WIDGET->selectionMode(), SELECT_NONE, false));
  else
    WIDGET->setSelectionMode((Q3ListView::SelectionMode)CCONST_convert(_selection_mode, VPROP(GB_INTEGER), SELECT_NONE, true));

END_PROPERTY


BEGIN_PROPERTY(CTREEVIEW_current)

  MyListViewItem *item = (MyListViewItem *)(WIDGET->currentItem());

  //if (item && (WIDGET->selectionMode() == QListView::Single) && !item->isSelected())
  //  item = 0;

  THIS->item = item;

  if (item == 0)
    GB.ReturnNull();
  else
    RETURN_SELF();

END_PROPERTY


BEGIN_PROPERTY(CTREEVIEW_key)

  MyListViewItem *item = (MyListViewItem *)(WIDGET->currentItem());

  //THIS->item = item;

  if (item == 0)
    GB.ReturnVoidString();
  else
    GB.ReturnString(item->key);

END_PROPERTY


BEGIN_PROPERTY(CTREEVIEW_item)

  if (THIS->item == 0)
    GB.ReturnNull();
  else
    RETURN_SELF();

END_PROPERTY


BEGIN_PROPERTY(CTREEVIEW_available)

  GB.ReturnBoolean(THIS->item != 0);

END_PROPERTY


/*BEGIN_PROPERTY(CTREEVIEW_show_root)

  if (READ_PROPERTY)
    GB.ReturnBoolean(WIDGET->rootIsDecorated());
  else
    WIDGET->setRootIsDecorated(VPROP(GB_BOOLEAN));

END_PROPERTY*/


BEGIN_PROPERTY(CTREEVIEW_sorted)

  if (READ_PROPERTY)
    GB.ReturnBoolean(OBJECT(CTREEVIEW)->sorted != -1);
  else
  {
    OBJECT(CTREEVIEW)->sorted = VPROP(GB_BOOLEAN) ? 0 : -1;
    WIDGET->setSorting(OBJECT(CTREEVIEW)->sorted);
    WIDGET->setShowSortIndicator(VPROP(GB_BOOLEAN));
  }

END_PROPERTY


BEGIN_PROPERTY(CTREEVIEW_editable)

  if (READ_PROPERTY)
    GB.ReturnBoolean(THIS->rename);
  else
    THIS->rename = VPROP(GB_BOOLEAN);

END_PROPERTY


BEGIN_PROPERTY(CTREEVIEW_count)

  GB.ReturnInteger(THIS->dict->count());

END_PROPERTY


BEGIN_METHOD_VOID(CTREEVIEW_back)

  MyListViewItem *item = THIS->save;
  THIS->save = 0;
  return_item(THIS, item);

END_METHOD

BEGIN_METHOD(CTREEVIEW_move_to, GB_STRING key)

  char *key = GB.ToZeroString(ARG(key));
  MyListViewItem *item = (*THIS->dict)[key];

  return_item(THIS, item);

END_METHOD


BEGIN_METHOD_VOID(CTREEVIEW_move_current)

  return_item(THIS, (MyListViewItem *)(WIDGET->currentItem()));

END_PROPERTY


BEGIN_METHOD_VOID(CTREEVIEW_first)

	//qDebug("CTREEVIEW_first: %d", WIDGET->childCount());
  return_item(THIS, (MyListViewItem *)(WIDGET->firstChild()));

END_METHOD

BEGIN_METHOD_VOID(CTREEVIEW_last)

	Q3ListViewItem *lwi, *lwn = 0;
	
	if (THIS->item)
		lwi = THIS->item; //(MyListViewItem *)WIDGET->firstChild(); 
	else
		lwi = WIDGET->firstChild();
		
	for (;;)
	{
		if (!lwi)
			break;
		lwn = lwi;
		lwi = lwi->nextSibling();
	}

  return_item(THIS, (MyListViewItem *)lwn);

END_METHOD


BEGIN_METHOD_VOID(CTREEVIEW_previous)

	MyListViewItem *lwi, *lwn = 0;
	
	if (THIS->item)
	{
		if (THIS->item->parent())
			lwi = (MyListViewItem *)THIS->item->parent()->firstChild(); 
		else
			lwi = (MyListViewItem *)WIDGET->firstChild();
		for (;;)
		{
			if (!lwi || lwi == THIS->item)
				break;
			lwn = lwi;
			lwi = (MyListViewItem *)lwi->nextSibling();
		}
	}

  return_item(THIS, lwn);

  //return_item(THIS, THIS->item ? (MyListViewItem *)(THIS->item->prev) : 0);

END_METHOD

BEGIN_METHOD_VOID(CTREEVIEW_next)

  return_item(THIS, THIS->item ? (MyListViewItem *)(THIS->item->nextSibling()) : 0);

END_METHOD


BEGIN_METHOD_VOID(CTREEVIEW_parent)

  return_item(THIS, THIS->item ? (MyListViewItem *)(THIS->item->parent()) : 0);

END_METHOD


BEGIN_METHOD_VOID(CTREEVIEW_child)

  return_item(THIS, THIS->item ? (MyListViewItem *)(THIS->item->firstChild()) : 0);

END_METHOD


BEGIN_METHOD_VOID(CTREEVIEW_above)

  return_item(THIS, THIS->item ? (MyListViewItem *)(THIS->item->itemAbove()) : 0);

END_METHOD


BEGIN_METHOD_VOID(CTREEVIEW_below)

  return_item(THIS, THIS->item ? (MyListViewItem *)(THIS->item->itemBelow()) : 0);

END_METHOD


BEGIN_METHOD_VOID(CTREEVIEW_SelectAll)

	WIDGET->selectAll(true);

END_METHOD


BEGIN_METHOD_VOID(CTREEVIEW_UnselectAll)

	WIDGET->selectAll(false);

END_METHOD


BEGIN_PROPERTY(CTREEVIEWITEM_key)

	if (!THIS->item)
		GB.ReturnVoidString();
	else
		GB.ReturnString(THIS->item->key);

END_PROPERTY

BEGIN_PROPERTY(CTREEVIEWITEM_parent_key)

	MyListViewItem *parent = (MyListViewItem *)THIS->item->parent();

  GB.ReturnString(parent ? parent->key : NULL);

END_PROPERTY


BEGIN_PROPERTY(CTREEVIEWITEM_picture)

  if (READ_PROPERTY)
    GB.ReturnObject(THIS->item->picture);
  else
    THIS->item->setPicture(PROP(GB_OBJECT));

END_PROPERTY


BEGIN_PROPERTY(CTREEVIEWITEM_selected)

  MyListViewItem *it = THIS->item;

  if (READ_PROPERTY)
    GB.ReturnBoolean(it->isSelected());
  else
    it->listView()->setSelected(it, VPROP(GB_BOOLEAN));

END_PROPERTY


BEGIN_PROPERTY(CTREEVIEWITEM_expanded)

  MyListViewItem *it = THIS->item;

  if (it->childCount() == 0)
  {
    if (READ_PROPERTY)
      GB.ReturnBoolean(false);
    return;
  }

  if (READ_PROPERTY)
    GB.ReturnBoolean(it->isOpen());
  else
    WIDGET->setOpen(it, VPROP(GB_BOOLEAN));

END_PROPERTY


BEGIN_PROPERTY(CTREEVIEWITEM_text)

  MyListViewItem *it = THIS->item;

  if (READ_PROPERTY)
    GB.ReturnNewZeroString(TO_UTF8(it->text(0)));
  else
    it->setText(0, QSTRING_PROP());

END_PROPERTY


BEGIN_PROPERTY(CTREEVIEWITEM_count)

  GB.ReturnInteger(THIS->item->childCount());

END_PROPERTY


BEGIN_METHOD_VOID(CTREEVIEWITEM_ensure_visible)

  //qDebug("ensure visible: %p", THIS->item);

  WIDGET->ensureItemVisible(THIS->item);

END_METHOD


BEGIN_METHOD_VOID(CTREEVIEWITEM_delete)

  //THIS->dict->remove(THIS->item->key);

  //qDebug("CTREEVIEWITEM_delete < %p %p", THIS, THIS->item);
  delete THIS->item;
  //qDebug("CTREEVIEWITEM_delete > %p", THIS->item);

END_METHOD


BEGIN_METHOD_VOID(CTREEVIEWITEM_clear)

  const MyListViewItem *item = (const MyListViewItem *)THIS->item->firstChild();
  const MyListViewItem *next;

  //qDebug("< CTREEVIEWITEM_clear");

  while (item)
  {
    next = (const MyListViewItem *)item->nextSibling();
    //THIS->dict->remove(((MyListViewItem *)item)->key);
    delete item;
    item = next;
  }

  //qDebug("CTREEVIEWITEM_clear >");

END_METHOD


/*
BEGIN_METHOD_VOID(CTREEVIEWITEM_init)

  CLASS_Item = GB.FindClass("Item");

END_METHOD
*/


BEGIN_PROPERTY(CTREEVIEWITEM_editable)

  if (READ_PROPERTY)
    GB.ReturnBoolean(THIS->item->renameEnabled(0));
  else
    THIS->item->setRenameEnabled(0, VPROP(GB_BOOLEAN));

END_PROPERTY


BEGIN_METHOD_VOID(CTREEVIEWITEM_rename)

	THIS->widget.flag.no_keyboard = TRUE;
  THIS->item->startRename(0);

END_METHOD

/*
BEGIN_PROPERTY(CTREEVIEWITEM_sort_key)

  MyListViewItem *it = THIS->item;

  if (READ_PROPERTY)
    GB.ReturnInteger(it->sortKey);
  else
    it->sortKey = VPROP(GB_INTEGER);

END_PROPERTY
*/

BEGIN_METHOD_VOID(CTREEVIEWITEM_move_first)

	Q3ListViewItem *parent;

	parent = THIS->item->parent();
	if (parent)
	{
		parent->takeItem(THIS->item);
		parent->insertItem(THIS->item);
	}
	else
	{
		WIDGET->takeItem(THIS->item);
		WIDGET->insertItem(THIS->item);
	}

END_METHOD

BEGIN_METHOD(CTREEVIEWITEM_move_after, GB_STRING key)

	Q3ListViewItem *after;

	if (MISSING(key) || LENGTH(key) == 0)
	{
		CTREEVIEWITEM_move_first(_object, _param);
		return;
	}
		
	after = CTreeView::getItem(THIS, GB.ToZeroString(ARG(key)));
	if (!after)
		return;
		
	THIS->item->moveItem(after);

END_METHOD

BEGIN_METHOD(CTREEVIEWITEM_move_before, GB_STRING key)

	Q3ListViewItem *before;
	Q3ListViewItem *child, *after, *parent;

	if (!MISSING(key))
	{
		before = CTreeView::getItem(THIS, GB.ToZeroString(ARG(key)));
		if (!before)
			return;
	}
	else
		before = 0;
		
	parent = THIS->item->parent();
	if (parent)
	{
		child = parent->firstChild();
		after = 0;
		while (child)
		{
			if (child == before)
				break;
			after = child;
			child = child->nextSibling();
		}

		if (after)
			THIS->item->moveItem(after);
		else
		{
			parent->takeItem(THIS->item);
			parent->insertItem(THIS->item);
		}
	}
	else
	{
		child = WIDGET->firstChild();
		after = 0;
		while (child)
		{
			if (child == before)
				break;
			after = child;
			child = child->nextSibling();
		}

		if (after)		
			THIS->item->moveItem(after);
		else
		{
			WIDGET->takeItem(THIS->item);
			WIDGET->insertItem(THIS->item);
		}
	}

END_METHOD

BEGIN_METHOD_VOID(CTREEVIEWITEM_move_last)

	CTREEVIEWITEM_move_before(_object, _param);

END_METHOD


BEGIN_PROPERTY(CTREEVIEWITEM_x)

	int x, m;
  QRect r = WIDGET->itemRect(THIS->item);
	
	if (r.isValid())
	{
		m = WIDGET->treeStepSize() * THIS->item->depth();
		x = WIDGET->viewport()->mapTo(WIDGET, r.topLeft()).x() + m;
	}
	else
		x = 0;
	
	GB.ReturnInteger(x);

END_PROPERTY

BEGIN_PROPERTY(CTREEVIEWITEM_y)

	int y;
  QRect r = WIDGET->itemRect(THIS->item);
	
	if (r.isValid())
		y = WIDGET->viewport()->mapTo(WIDGET, r.topLeft()).y();
	else
		y = 0;
	
	GB.ReturnInteger(y);

END_PROPERTY

BEGIN_PROPERTY(CTREEVIEWITEM_w)

	int w;
  //QRect r = WIDGET->itemRect(THIS->item);
	
	w = WIDGET->header()->cellPos(WIDGET->header()->count() - 1) 
	    + WIDGET->header()->cellSize(WIDGET->header()->count() - 1)
	    - WIDGET->treeStepSize() * THIS->item->depth();

	GB.ReturnInteger(w);

END_PROPERTY

BEGIN_PROPERTY(CTREEVIEWITEM_h)

	GB.ReturnInteger(THIS->item->height());

END_PROPERTY




/***************************************************************************

  Columns

***************************************************************************/

BEGIN_PROPERTY(CLISTVIEW_resizable)

  if (READ_PROPERTY)
    GB.ReturnBoolean(WIDGET->header()->isResizeEnabled());
  else
  {
    WIDGET->header()->setResizeEnabled(VPROP(GB_BOOLEAN));
    //WIDGET->setResizeMode(PROPERTY(GB_BOOLEAN) ? QListView::NoColumn : QListView::LastColumn);
  }

END_PROPERTY


/*BEGIN_PROPERTY(CLISTVIEW_moveable)

  if (READ_PROPERTY)
    GB.ReturnBoolean(WIDGET->header()->isMovingEnabled());
  else
    WIDGET->header()->setMovingEnabled(VPROP(GB_BOOLEAN));

END_PROPERTY*/


BEGIN_PROPERTY(CLISTVIEW_columns_count)

  int col;
  Q3ListView::ResizeMode mode;
  //bool *numeric;
  //int i, n;

  if (READ_PROPERTY)
  {
    GB.ReturnInteger(WIDGET->columns());
    return;
  }

  col = VPROP(GB_INTEGER);

  if (col < 1 || col > 255)
  {
    GB.Error("Bad number of columns");
    return;
  }

  mode = WIDGET->resizeMode();
  WIDGET->setResizeMode(Q3ListView::NoColumn);

  if (col < WIDGET->columns())
  {
    while (col < WIDGET->columns())
      WIDGET->removeColumn(WIDGET->columns() - 1);
  }
  else if (col > WIDGET->columns())
  {
    while (col > WIDGET->columns())
    {
      WIDGET->addColumn("", 16);
      //if (mode == QListView::NoColumn)
      WIDGET->setAutoResize(WIDGET->columns() - 1, WIDGET->isAutoResize());
    }
  }

  WIDGET->setResizeMode(mode);

END_PROPERTY


BEGIN_METHOD(CLISTVIEW_columns_get, GB_INTEGER col)

  int col = VARG(col);

  if (col < 0 || col >= WIDGET->columns())
  {
    GB.Error("Bad column index");
    return;
  }

  WIDGET->_column = col;

  RETURN_SELF();

END_PROPERTY


BEGIN_METHOD_VOID(CLISTVIEW_columns_adjust)

  Q3Header *header = WIDGET->header();
  int col = WIDGET->columns() - 1;
  int w;

  w = WIDGET->width() - WIDGET->frameWidth() * 2 - header->sectionPos(col);

  WIDGET->setColumnWidth(col, w);
  //updateScrollBars();
  //qDebug("setColumnWidth %d = %d", col, w);
  //header->resize(w, header->height());

  /*header->resizeSection(header->count() - 1,
    WIDGET->frameSize().width() - WIDGET );*/

END_METHOD


BEGIN_PROPERTY(CLISTVIEW_column_text)

  if (READ_PROPERTY)
    GB.ReturnNewZeroString(TO_UTF8(WIDGET->columnText(WIDGET->_column)));
  else
    WIDGET->setColumnText(WIDGET->_column, QSTRING_PROP());

END_PROPERTY


BEGIN_PROPERTY(CLISTVIEW_column_width)

  int w;
  Q3ListView::ResizeMode mode;

  if (READ_PROPERTY)
    GB.ReturnInteger(WIDGET->columnWidth(WIDGET->_column));
  else
  {
    w = VPROP(GB_INTEGER);
    //if ((_column == (WIDGET->columns() - 1)) && w < 0)
    //  updateLastColumn(WIDGET);
    //else
    //WIDGET->setColumnWidthMode(_column, QListView::Manual);
		mode = WIDGET->resizeMode();
		WIDGET->setResizeMode(Q3ListView::NoColumn);
		    
		WIDGET->setColumnWidthMode(WIDGET->_column, Q3ListView::Manual);
    if (w < 0)
    {
			WIDGET->adjustColumn(WIDGET->_column);
    	WIDGET->setColumnWidth(WIDGET->_column, QMAX(WIDGET->minimumWidth(WIDGET->_column), WIDGET->columnWidth(WIDGET->_column)) + 16);
    }
    else
    {
    	WIDGET->setColumnWidth(WIDGET->_column, w);
    }
    if (WIDGET->isAutoResize())
			WIDGET->setColumnWidthMode(WIDGET->_column, Q3ListView::Maximum);
    
    WIDGET->setResizeMode(mode);
  }

END_PROPERTY


BEGIN_PROPERTY(CLISTVIEW_column_alignment)

  if (READ_PROPERTY)
    GB.ReturnInteger(CCONST_alignment((WIDGET->columnAlignment(WIDGET->_column) | Qt::AlignVCenter) & ALIGN_MASK, ALIGN_NORMAL, false));
  else
    WIDGET->setColumnAlignment(WIDGET->_column, CCONST_alignment(VPROP(GB_INTEGER), ALIGN_NORMAL, true)  & Qt::AlignHorizontal_Mask);

END_PROPERTY



BEGIN_PROPERTY(CLISTVIEW_columns_sort)

  int sort;

  if (READ_PROPERTY)
    GB.ReturnInteger(THIS->sorted);
  else
  {
    sort = VPROP(GB_INTEGER);
    if (sort < -1 || sort >= WIDGET->columns())
      return;
    THIS->sorted = sort;
    THIS->asc = true;
    WIDGET->setSorting(THIS->sorted);
  }

END_PROPERTY


BEGIN_PROPERTY(CLISTVIEW_columns_ascending)

  if (READ_PROPERTY)
    GB.ReturnBoolean(THIS->asc);
  else
  {
    THIS->asc = VPROP(GB_BOOLEAN);
    WIDGET->setSorting(THIS->sorted, THIS->asc);
  }

END_PROPERTY

/*
BEGIN_PROPERTY(CLISTVIEW_column_numeric)

  if (READ_PROPERTY)
    GB.ReturnNewZeroString(TO_UTF8(WIDGET->columnText(_column)));
  else
    WIDGET->setColumnText(_column, QSTRING_PROP());

END_PROPERTY
*/


BEGIN_PROPERTY(CLISTVIEW_auto_resize)

  if (READ_PROPERTY)
    GB.ReturnBoolean(WIDGET->isAutoResize());
  else
    WIDGET->setAutoResize(VPROP(GB_BOOLEAN));

END_PROPERTY


BEGIN_METHOD(CTREEVIEWITEM_get, GB_INTEGER col)

  const MyListViewItem* item = THIS->item;

  GB.ReturnNewZeroString(TO_UTF8(item->text(VARG(col))));

END_METHOD


BEGIN_METHOD(CTREEVIEWITEM_put, GB_STRING text; GB_INTEGER col)

  MyListViewItem* item = THIS->item;

  item->setText(VARG(col), QSTRING_ARG(text));

END_METHOD


BEGIN_PROPERTY(CTREEVIEW_client_width)

  //int w = WIDGET->width() - WIDGET->frameWidth() * 2;
  //int sw = WIDGET->verticalScrollBar()->width();
  //int width = w;

  //if (WIDGET->verticalScrollBar()->isHidden())
  //  width -= sw;

  //qDebug("client_width = %ld (%ld) -> %ld", w, sw, width);


  GB.ReturnInteger(WIDGET->width() - WIDGET->frameWidth() * 2);
  //GB.ReturnInteger(width);

END_PROPERTY


BEGIN_PROPERTY(CTREEVIEW_client_height)

  GB.ReturnInteger(WIDGET->height() - WIDGET->frameWidth() * 2);

END_PROPERTY


BEGIN_PROPERTY(CTREEVIEW_header)

  if (READ_PROPERTY)
    GB.ReturnBoolean(!WIDGET->header()->isHidden());
  else
  {
    if (VPROP(GB_BOOLEAN))
      WIDGET->header()->show();
    else
      WIDGET->header()->hide();

    WIDGET->triggerUpdate();
  }


END_PROPERTY


BEGIN_PROPERTY(CLISTVIEW_compare)

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
  	
	GB.GetFunction(&_get_settings_func, (void *)GB.FindClass("_Qt"), "_GetColumnViewSettings", "ColumnView;", "s");
	GB.GetFunction(&_set_settings_func, (void *)GB.FindClass("_Qt"), "_SetColumnViewSettings", "ColumnView;s", "");
	
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

BEGIN_PROPERTY(CTREEVIEW_Renaming)

	GB.ReturnBoolean(WIDGET->isRenaming());

END_PROPERTY

/***************************************************************************

  ListViewItem

***************************************************************************/

GB_DESC CListViewItemDesc[] =
{
  GB_DECLARE(".ListView.Item", 0), GB_VIRTUAL_CLASS(), GB_HOOK_CHECK(check_item),

  GB_PROPERTY_READ("Key", "s", CTREEVIEWITEM_key),
  GB_PROPERTY("Picture", "Picture", CTREEVIEWITEM_picture),
  GB_PROPERTY("Selected", "b", CTREEVIEWITEM_selected),
  GB_PROPERTY("Text", "s", CTREEVIEWITEM_text),
  GB_METHOD("EnsureVisible", NULL, CTREEVIEWITEM_ensure_visible, NULL),
  GB_METHOD("Delete", NULL, CTREEVIEWITEM_delete, NULL),

  GB_PROPERTY_READ("X", "i", CTREEVIEWITEM_x),
  GB_PROPERTY_READ("Y", "i", CTREEVIEWITEM_y),
  GB_PROPERTY_READ("W", "i", CTREEVIEWITEM_w),
  GB_PROPERTY_READ("Width", "i", CTREEVIEWITEM_w),
  GB_PROPERTY_READ("H", "i", CTREEVIEWITEM_h),
  GB_PROPERTY_READ("Height", "i", CTREEVIEWITEM_h),
  
  GB_METHOD("MoveAfter", NULL, CTREEVIEWITEM_move_after, "[(Key)s]"),
  GB_METHOD("MoveBefore", NULL, CTREEVIEWITEM_move_before, "[(Key)s]"),
  GB_METHOD("MoveFirst", NULL, CTREEVIEWITEM_move_first, NULL),
  GB_METHOD("MoveLast", NULL, CTREEVIEWITEM_move_last, NULL),
  
  GB_PROPERTY("Editable", "b", CTREEVIEWITEM_editable),
  GB_METHOD("Rename", NULL, CTREEVIEWITEM_rename, NULL),

  GB_END_DECLARE
};


/***************************************************************************

  TreeViewItem

***************************************************************************/

GB_DESC CTreeViewItemDesc[] =
{
  GB_DECLARE(".TreeView.Item", 0), GB_VIRTUAL_CLASS(), GB_HOOK_CHECK(check_item),

  GB_PROPERTY_READ("Key", "s", CTREEVIEWITEM_key),
  GB_PROPERTY_READ("ParentKey", "s", CTREEVIEWITEM_parent_key),
  GB_PROPERTY("Picture", "Picture", CTREEVIEWITEM_picture),
  GB_PROPERTY("Selected", "b", CTREEVIEWITEM_selected),
  GB_PROPERTY("Text", "s", CTREEVIEWITEM_text),
  GB_METHOD("EnsureVisible", NULL, CTREEVIEWITEM_ensure_visible, NULL),
  GB_METHOD("Delete", NULL, CTREEVIEWITEM_delete, NULL),
  GB_METHOD("Clear", NULL, CTREEVIEWITEM_clear, NULL),

  GB_PROPERTY("Expanded", "b", CTREEVIEWITEM_expanded),
  GB_PROPERTY_READ("Children", "i", CTREEVIEWITEM_count),
  GB_PROPERTY_READ("Count", "i", CTREEVIEWITEM_count),

  GB_PROPERTY_READ("X", "i", CTREEVIEWITEM_x),
  GB_PROPERTY_READ("Y", "i", CTREEVIEWITEM_y),
  GB_PROPERTY_READ("W", "i", CTREEVIEWITEM_w),
  GB_PROPERTY_READ("Width", "i", CTREEVIEWITEM_w),
  GB_PROPERTY_READ("H", "i", CTREEVIEWITEM_h),
  GB_PROPERTY_READ("Height", "i", CTREEVIEWITEM_h),
  
  /*GB_METHOD("MoveNext", "b", CTREEVIEW_next, NULL),
  GB_METHOD("MovePrevious", "b", CTREEVIEW_previous, NULL),
  GB_METHOD("MoveChild", "b", CTREEVIEW_child, NULL),
  GB_METHOD("MoveParent", "b", CTREEVIEW_parent, NULL),
  GB_METHOD("MoveAbove", "b", CTREEVIEW_above, NULL),
  GB_METHOD("MoveBelow", "b", CTREEVIEW_below, NULL),*/

  GB_METHOD("MoveAfter", NULL, CTREEVIEWITEM_move_after, "[(Key)s]"),
  GB_METHOD("MoveBefore", NULL, CTREEVIEWITEM_move_before, "[(Key)s]"),
  GB_METHOD("MoveFirst", NULL, CTREEVIEWITEM_move_first, NULL),
  GB_METHOD("MoveLast", NULL, CTREEVIEWITEM_move_last, NULL),
  
  GB_PROPERTY("Editable", "b", CTREEVIEWITEM_editable),
  GB_METHOD("Rename", NULL, CTREEVIEWITEM_rename, NULL),

  GB_END_DECLARE
};


/***************************************************************************

  ColumnViewItem

***************************************************************************/

GB_DESC CColumnViewItemDesc[] =
{
  GB_DECLARE(".ColumnView.Item", 0), GB_VIRTUAL_CLASS(), GB_HOOK_CHECK(check_item),

  GB_PROPERTY_READ("Key", "s", CTREEVIEWITEM_key),
  GB_PROPERTY_READ("ParentKey", "s", CTREEVIEWITEM_parent_key),
  GB_PROPERTY("Picture", "Picture", CTREEVIEWITEM_picture),
  GB_PROPERTY("Selected", "b", CTREEVIEWITEM_selected),
  GB_PROPERTY("Text", "s", CTREEVIEWITEM_text),
  GB_METHOD("EnsureVisible", NULL, CTREEVIEWITEM_ensure_visible, NULL),
  GB_METHOD("Delete", NULL, CTREEVIEWITEM_delete, NULL),
  GB_METHOD("Clear", NULL, CTREEVIEWITEM_clear, NULL),

  GB_PROPERTY("Expanded", "b", CTREEVIEWITEM_expanded),
  GB_PROPERTY_READ("Children", "i", CTREEVIEWITEM_count),
  GB_PROPERTY_READ("Count", "i", CTREEVIEWITEM_count),

  GB_PROPERTY_READ("X", "i", CTREEVIEWITEM_x),
  GB_PROPERTY_READ("Y", "i", CTREEVIEWITEM_y),
  GB_PROPERTY_READ("W", "i", CTREEVIEWITEM_w),
  GB_PROPERTY_READ("Width", "i", CTREEVIEWITEM_w),
  GB_PROPERTY_READ("H", "i", CTREEVIEWITEM_h),
  GB_PROPERTY_READ("Height", "i", CTREEVIEWITEM_h),
  /*GB_METHOD("MoveNext", "b", CTREEVIEW_next, NULL),
  GB_METHOD("MovePrevious", "b", CTREEVIEW_previous, NULL),
  GB_METHOD("MoveChild", "b", CTREEVIEW_child, NULL),
  GB_METHOD("MoveParent", "b", CTREEVIEW_parent, NULL),
  GB_METHOD("MoveAbove", "b", CTREEVIEW_above, NULL),
  GB_METHOD("MoveBelow", "b", CTREEVIEW_below, NULL),*/

  GB_METHOD("MoveAfter", NULL, CTREEVIEWITEM_move_after, "[(Key)s]"),
  GB_METHOD("MoveBefore", NULL, CTREEVIEWITEM_move_before, "[(Key)s]"),
  GB_METHOD("MoveFirst", NULL, CTREEVIEWITEM_move_first, NULL),
  GB_METHOD("MoveLast", NULL, CTREEVIEWITEM_move_last, NULL),
  
  GB_METHOD("_get", "s", CTREEVIEWITEM_get, "(Column)i"),
  GB_METHOD("_put", NULL, CTREEVIEWITEM_put, "(Text)s(Column)i"),

  GB_PROPERTY("Editable", "b", CTREEVIEWITEM_editable),
  GB_METHOD("Rename", NULL, CTREEVIEWITEM_rename, NULL),

  //GB_PROPERTY("SortKey", "f", CTREEVIEWITEM_sort_key),

  GB_END_DECLARE
};


/***************************************************************************

  ListView

***************************************************************************/

GB_DESC CListViewDesc[] =
{
  GB_DECLARE("ListView", sizeof(CTREEVIEW)), GB_INHERITS("Control"),

  GB_METHOD("_new", NULL, CLISTVIEW_new, "(Parent)Container;"),
  GB_METHOD("_free", NULL, CTREEVIEW_free, NULL),

  //GB_PROPERTY("Drop", "b", CTREEVIEW_drop),
  GB_PROPERTY("Mode", "i", CTREEVIEW_mode),
  GB_PROPERTY("Sorted", "b", CTREEVIEW_sorted),
  GB_PROPERTY("Editable", "b", CTREEVIEW_editable),
  GB_PROPERTY("ScrollBar", "i", CWIDGET_scrollbar),
  GB_PROPERTY("Border", "b", CWIDGET_border_simple),
  GB_PROPERTY("Compare", "i", CLISTVIEW_compare),

  GB_PROPERTY_READ("Count", "i", CTREEVIEW_count),

  GB_PROPERTY_READ("Available", "b", CTREEVIEW_available),
  GB_METHOD("MoveTo", "b", CTREEVIEW_move_to, "(Key)s"),
  GB_METHOD("MoveCurrent", "b", CTREEVIEW_move_current, NULL),
  GB_METHOD("MoveFirst", "b", CTREEVIEW_first, NULL),
  GB_METHOD("MoveLast", "b", CTREEVIEW_last, NULL),
  GB_METHOD("MovePrevious", "b", CTREEVIEW_previous, NULL),
  GB_METHOD("MoveNext", "b", CTREEVIEW_next, NULL),
  GB_METHOD("MoveAbove", "b", CTREEVIEW_above, NULL),
  GB_METHOD("MoveBelow", "b", CTREEVIEW_below, NULL),
  GB_METHOD("MoveBack", "b", CTREEVIEW_back, NULL),

  GB_METHOD("_get", ".ListView.Item", CTREEVIEW_get, "(Key)s"),

  GB_METHOD("Clear", NULL, CTREEVIEW_clear, NULL),
  GB_METHOD("Add", ".ListView.Item", CLISTVIEW_add, "(Key)s(Text)s[(Picture)Picture;(After)s]"),
  GB_METHOD("Remove", NULL, CTREEVIEW_remove, "(Key)s"),
  GB_METHOD("Exist", "b", CTREEVIEW_exist, "(Key)s"),
  GB_METHOD("FindAt", "b", CTREEVIEW_find, "(X)i(Y)i"),
  //GB_METHOD("FindText", "b", CTREEVIEW_find, "(X)i(Y)i"),
  GB_METHOD("SelectAll", NULL, CTREEVIEW_SelectAll, NULL),
  GB_METHOD("UnselectAll", NULL, CTREEVIEW_UnselectAll, NULL),

  GB_PROPERTY_READ("Current", ".ListView.Item", CTREEVIEW_current),
  GB_PROPERTY_READ("Key", "s", CTREEVIEW_key),
  GB_PROPERTY_READ("Item", ".ListView.Item", CTREEVIEW_item),
  GB_PROPERTY_READ("Renaming", "b", CTREEVIEW_Renaming),

  GB_PROPERTY_READ("ClientWidth", "i", CTREEVIEW_client_width),
  GB_PROPERTY_READ("ClientW", "i",  CTREEVIEW_client_width),
  GB_PROPERTY_READ("ClientHeight", "i", CTREEVIEW_client_height),
  GB_PROPERTY_READ("ClientH", "i", CTREEVIEW_client_height),

  GB_EVENT("Select", NULL, NULL, &EVENT_Select),
  GB_EVENT("Activate", NULL, NULL, &EVENT_Activate),
  GB_EVENT("Click", NULL, NULL, &EVENT_Click),
  GB_EVENT("Rename", NULL, NULL, &EVENT_Rename),
  GB_EVENT("Cancel", NULL, NULL, &EVENT_Cancel),
  GB_EVENT("Compare", NULL, "(Key)s(OtherKey)s", &EVENT_Compare),

	LISTVIEW_DESCRIPTION,

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

  //GB_PROPERTY("Drop", "b", CTREEVIEW_drop),
  GB_PROPERTY("Mode", "i", CTREEVIEW_mode),
  //GB_PROPERTY("Root", "b", CTREEVIEW_show_root),
  GB_PROPERTY("Sorted", "b", CTREEVIEW_sorted),
  GB_PROPERTY("Editable", "b", CTREEVIEW_editable),
  GB_PROPERTY("ScrollBar", "i", CWIDGET_scrollbar),
  GB_PROPERTY("Border", "b", CWIDGET_border_simple),
  GB_PROPERTY("Compare", "i", CLISTVIEW_compare),

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

  /*GB_METHOD("GoToCurrent", "b", CTREEVIEW_move_current, NULL),
  GB_METHOD("GoTo", "b", CTREEVIEW_move_to, "(Key)s"),
  GB_METHOD("GoToFirst", "b", CTREEVIEW_first, NULL),
  GB_METHOD("GoToLast", "b", CTREEVIEW_last, NULL),
  GB_METHOD("GoToNext", "b", CTREEVIEW_next, NULL),
  GB_METHOD("GoToPrevious", "b", CTREEVIEW_previous, NULL),
  GB_METHOD("GoToChild", "b", CTREEVIEW_child, NULL),
  GB_METHOD("GoToParent", "b", CTREEVIEW_parent, NULL),
  GB_METHOD("GoAbove", "b", CTREEVIEW_above, NULL),
  GB_METHOD("GoBelow", "b", CTREEVIEW_below, NULL),
  GB_METHOD("GoBack", "b", CTREEVIEW_back, NULL),*/

  GB_METHOD("_get", ".TreeView.Item", CTREEVIEW_get, "(Key)s"),

  GB_METHOD("Clear", NULL, CTREEVIEW_clear, NULL),
  GB_METHOD("Add", ".TreeView.Item", CTREEVIEW_add, "(Key)s(Text)s[(Picture)Picture;(Parent)s(After)s]"),
  GB_METHOD("Remove", NULL, CTREEVIEW_remove, "(Key)s"),
  GB_METHOD("Exist", "b", CTREEVIEW_exist, "(Key)s"),
  GB_METHOD("FindAt", "b", CTREEVIEW_find, "(X)i(Y)i"),
  GB_METHOD("SelectAll", NULL, CTREEVIEW_SelectAll, NULL),
  GB_METHOD("UnselectAll", NULL, CTREEVIEW_UnselectAll, NULL),

  GB_PROPERTY_READ("Current", ".TreeView.Item", CTREEVIEW_current),
  GB_PROPERTY_READ("Key", "s", CTREEVIEW_key),
  GB_PROPERTY_READ("Item", ".TreeView.Item", CTREEVIEW_item),
  GB_PROPERTY_READ("Renaming", "b", CTREEVIEW_Renaming),

  GB_PROPERTY_READ("ClientWidth", "i", CTREEVIEW_client_width),
  GB_PROPERTY_READ("ClientW", "i",  CTREEVIEW_client_width),
  GB_PROPERTY_READ("ClientHeight", "i", CTREEVIEW_client_height),
  GB_PROPERTY_READ("ClientH", "i", CTREEVIEW_client_height),

  GB_EVENT("Select", NULL, NULL, &EVENT_Select),
  GB_EVENT("Activate", NULL, NULL, &EVENT_Activate),
  GB_EVENT("Click", NULL, NULL, &EVENT_Click),
  GB_EVENT("Rename", NULL, NULL, &EVENT_Rename),
  GB_EVENT("Cancel", NULL, NULL, &EVENT_Cancel),
  GB_EVENT("Compare", NULL, "(Key)s(OtherKey)s", &EVENT_Compare),
  GB_EVENT("Expand", NULL, NULL, &EVENT_Expand),
  GB_EVENT("Collapse", NULL, NULL, &EVENT_Collapse),

	TREEVIEW_DESCRIPTION,

  GB_END_DECLARE
};


/***************************************************************************

  ColumnView

***************************************************************************/

GB_DESC CColumnViewColumnDesc[] =
{
  GB_DECLARE(".ColumnView.Column", 0), GB_VIRTUAL_CLASS(),

  GB_PROPERTY("Text", "s", CLISTVIEW_column_text),
  GB_PROPERTY("Alignment", "i", CLISTVIEW_column_alignment),
  GB_PROPERTY("W", "i", CLISTVIEW_column_width),
  GB_PROPERTY("Width", "i", CLISTVIEW_column_width),

  GB_END_DECLARE
};

GB_DESC CColumnViewColumnsDesc[] =
{
  GB_DECLARE(".ColumnView.Columns", 0), GB_VIRTUAL_CLASS(),

  GB_METHOD("_get", ".ColumnView.Column", CLISTVIEW_columns_get, "(Column)i"),

  GB_PROPERTY("Count", "i", CLISTVIEW_columns_count),
  GB_PROPERTY("Sort", "i", CLISTVIEW_columns_sort),
  GB_PROPERTY("Ascending", "b", CLISTVIEW_columns_ascending),

  GB_END_DECLARE
};

GB_DESC CColumnViewDesc[] =
{
  GB_DECLARE("ColumnView", sizeof(CTREEVIEW)), GB_INHERITS("Control"),

  GB_METHOD("_new", NULL, CCOLUMNVIEW_new, "(Parent)Container;"),
  GB_METHOD("_free", NULL, CTREEVIEW_free, NULL),

  GB_PROPERTY("Mode", "i", CTREEVIEW_mode),
  GB_PROPERTY("Sorted", "b", CTREEVIEW_sorted),
  GB_PROPERTY("Editable", "b", CTREEVIEW_editable),
  GB_PROPERTY("ScrollBar", "i", CWIDGET_scrollbar),
  GB_PROPERTY("Border", "b", CWIDGET_border_simple),
  GB_PROPERTY("Compare", "i", CLISTVIEW_compare),

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

  GB_METHOD("_get", ".ColumnView.Item", CTREEVIEW_get, "(Key)s"),

  GB_METHOD("Clear", NULL, CCOLUMNVIEW_clear, NULL),
  GB_METHOD("Add", ".ColumnView.Item", CTREEVIEW_add, "(Key)s(Text)s[(Picture)Picture;(Parent)s(After)s]"),
  GB_METHOD("Remove", NULL, CTREEVIEW_remove, "(Key)s"),
  GB_METHOD("Exist", "b", CTREEVIEW_exist, "(Key)s"),
  GB_METHOD("FindAt", "b", CTREEVIEW_find, "(X)i(Y)i"),
  GB_METHOD("SelectAll", NULL, CTREEVIEW_SelectAll, NULL),
  GB_METHOD("UnselectAll", NULL, CTREEVIEW_UnselectAll, NULL),

  GB_PROPERTY_READ("Current", ".ColumnView.Item", CTREEVIEW_current),
  GB_PROPERTY_READ("Key", "s", CTREEVIEW_key),
  GB_PROPERTY_READ("Item", ".ColumnView.Item", CTREEVIEW_item),
  GB_PROPERTY_READ("Renaming", "b", CTREEVIEW_Renaming),

  GB_EVENT("Select", NULL, NULL, &EVENT_Select),
  GB_EVENT("Activate", NULL, NULL, &EVENT_Activate),
  GB_EVENT("Click", NULL, NULL, &EVENT_Click),
  GB_EVENT("Rename", NULL, NULL, &EVENT_Rename),
  GB_EVENT("Cancel", NULL, NULL, &EVENT_Cancel),
  GB_EVENT("Compare", NULL, "(Key)s(OtherKey)s", &EVENT_Compare),
  GB_EVENT("Expand", NULL, NULL, &EVENT_Expand),
  GB_EVENT("Collapse", NULL, NULL, &EVENT_Collapse),
  //GB_EVENT("ColumnClick", NULL, "(Column)i", &EVENT_ColumnClick),

  GB_PROPERTY_SELF("Columns", ".ColumnView.Columns"),

  GB_PROPERTY("Resizable", "b", CLISTVIEW_resizable),
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


/***************************************************************************

  class CTreeView

***************************************************************************/

CTreeView CTreeView::manager;


MyListViewItem *CTreeView::getItem(CTREEVIEW *treeview, char *key)
{
  MyListViewItem *item = (*treeview->dict)[key];

  if (item == 0)
  {
    GB.Error("Unknown item: '&1'", key);
    return NULL;
  }

  return item;
}

static void raise_event(void *_object, int ev, Q3ListViewItem *it)
{
  MyListViewItem *old = THIS->item;

  if (!it)
    it = WIDGET->currentItem();
  THIS->item = (MyListViewItem *)it;
  GB.Raise(THIS, ev, 0);
  // TODO: what happens if old is destroyed during the event?
  THIS->item = old;
}

void CTreeView::raiseEvent(int ev, Q3ListViewItem *it)
{
	GET_SENDER();
	raise_event(_object, ev, it);
}

static void post_select_event(void *_object)
{
	if (WIDGET) // Control can become invalid!
		raise_event(_object, EVENT_Select, 0);
  GB.Unref(&_object);
}

void CTreeView::selected(void)
{
  GET_SENDER();
  if (WIDGET->selectionMode() == Q3ListView::Single)
	  raise_event(_object, EVENT_Select, 0);
	else
	{
  	GB.Ref(_object);
  	GB.Post((GB_POST_FUNC)post_select_event, (intptr_t)THIS);
	}
}

void CTreeView::activated(Q3ListViewItem *it)
{
  if (!it)
    return;
  raiseEvent(EVENT_Activate, it);
}

void CTreeView::clicked(Q3ListViewItem *it)
{
  if (!it)
    return;
  raiseEvent(EVENT_Click, it);
}

static void post_rename_event(void *_object)
{
  GB.Raise(THIS, EVENT_Rename, 0); //1, GB_T_STRING, THIS->before, GB.StringLength(THIS->before));
  GB.Unref(&_object);
}

void CTreeView::renamed(Q3ListViewItem *it, int col)
{
  GET_SENDER();

	THIS->widget.flag.no_keyboard = FALSE;
	
  if (it == 0)
    return;

  THIS->item = (MyListViewItem *)it;
  GB.Ref(THIS);
  GB.Post((void (*)())post_rename_event, (intptr_t)THIS);
}

// void CTreeView::columnClicked(QListViewItem *it, const QPoint &p, int c)
// {
//   GET_SENDER();
// 
//   if (it == 0)
//     return;
// 
//   THIS->item = (MyListViewItem *)it;
// 
//   GB.Raise(_object, EVENT_ColumnClick, 1, GB_T_INTEGER, c);
// }

void CTreeView::expanded(Q3ListViewItem *it)
{
  raiseEvent(EVENT_Expand, it);
}

void CTreeView::collapsed(Q3ListViewItem *it)
{
  raiseEvent(EVENT_Collapse, it);
}

void CTreeView::headerClicked(int c)
{
  CTREEVIEW *_object = (CTREEVIEW *)CWidget::get((QObject *)sender());

  if (_object->sorted == c)
    _object->asc = !_object->asc;
  else
    _object->asc = true;

  _object->sorted = c;

  //WIDGET->setSorted(_object->sorted, _object->asc);
}


void CTreeView::headerSizeChange(int c, int os, int ns)
{
//  CTREEVIEW *_object = (CTREEVIEW *)CWidget::get((QObject *)sender());
//  updateLastColumn(WIDGET);
}

