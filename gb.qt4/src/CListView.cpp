/***************************************************************************

  CListView.cpp

  (c) 2000-2011 Benoît Minisini <gambas@users.sourceforge.net>

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

#define __CLISTVIEW_CPP

#include <QDragLeaveEvent>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QHeaderView>

#include "gambas.h"

#include "CWidget.h"
#include "CPicture.h"
#include "CClipboard.h"
#include "CListView.h"
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
	SELECT_NONE, QAbstractItemView::NoSelection, 
	SELECT_SINGLE, QAbstractItemView::SingleSelection,
	SELECT_MULTIPLE, QAbstractItemView::ExtendedSelection,
	CONST_MAGIC
};

/***************************************************************************

  class MyTreeWidgetItem

***************************************************************************/

void MyTreeWidgetItem::initData(CLISTVIEW *cont)
{
  picture = 0;
  key = 0;
  container = cont;
  //sortKey = NULL;
  //setDropEnabled(true);
	setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
}


MyTreeWidgetItem::MyTreeWidgetItem(CLISTVIEW *cont, MyTreeWidget *parent)
: QTreeWidgetItem(parent)
{
  initData(cont);
}

MyTreeWidgetItem::MyTreeWidgetItem(CLISTVIEW *cont, MyTreeWidget *parent, MyTreeWidgetItem *after)
: QTreeWidgetItem(parent, after)
{
  initData(cont);
}

MyTreeWidgetItem::MyTreeWidgetItem(CLISTVIEW *cont, MyTreeWidgetItem *parent)
: QTreeWidgetItem(parent)
{
  initData(cont);
}

MyTreeWidgetItem::MyTreeWidgetItem(CLISTVIEW *cont, MyTreeWidgetItem *parent, MyTreeWidgetItem *after)
: QTreeWidgetItem(parent, after)
{
  initData(cont);
}

MyTreeWidgetItem::~MyTreeWidgetItem()
{
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

MyTreeWidgetItem *MyTreeWidgetItem::parent() const
{
	return (MyTreeWidgetItem *)(QTreeWidgetItem::parent() ? QTreeWidgetItem::parent() : ((MyTreeWidget *)container->widget.widget)->invisibleRootItem()); 
}

void MyTreeWidgetItem::setPixmap(const QPixmap &pixmap)
{
	QIcon icon;
	CWIDGET_iconset(icon, pixmap);
	setIcon(0, icon);
}

void MyTreeWidgetItem::setPicture(GB_OBJECT *pict)
{
  SET_PIXMAP(setPixmap, &picture, pict);
}

bool MyTreeWidgetItem::operator< (const QTreeWidgetItem &other) const
{
  //if (!GB.CanRaise(container, EVENT_Compare) || col != container->sorted)
    return QTreeWidgetItem::operator<(other); //i, col, ascending);

  //container->compare = 0;
  //GB.Raise(container, EVENT_Compare, 2, GB_T_STRING, key, 0, GB_T_STRING, ((MyTreeWidgetItem *)i)->key, 0);
  //return container->compare;
}

#if 0
bool MyTreeWidgetItem::acceptDrop(const QMimeSource *mime) const
{
	/*QObject *e = (QObject *)e;

	qDebug("MyTreeWidgetItem::acceptDrop");

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

void MyTreeWidgetItem::dropped(QDropEvent *e)
{
	/*qDebug("MyTreeWidgetItem::dropped");

	if (QWIDGET(container)->acceptDrops())
		CDRAG_drag_drop(QWIDGET(container), (CWIDGET *)container, e);*/
}
#endif

#if 0
void MyTreeWidgetItem::startRename(int col)
{
	//qDebug("before: %s", text(col).latin1());
	//GB.FreeString(&container->before);
	//container->before = GB.NewZeroString(TO_UTF8(text(col)));
	QTreeWidgetItem::startRename(col);
}
#endif

static void post_cancel_event(void *_object)
{
  GB.Raise(THIS, EVENT_Cancel, 0);
  GB.Unref(&_object);
}

/*
void MyTreeWidgetItem::cancelRename(int col)
{
	QTreeWidgetItem::cancelRename(col);
	if (GB.CanRaise(container, EVENT_Cancel))
	{
		GB.Ref(container);
		GB.Post((GB_POST_FUNC)post_cancel_event, (intptr_t)container);
	}
}
*/

/*void MyTreeWidgetItem::setSelected(bool s)
{
	if (CDRAG_dragging)
		return;
	qDebug("setSelected: %s %d", key, s);
	QListViewItem::setSelected(s);
}*/

void MyTreeWidgetItem::setEditable(bool e)
{
	if (e)
		setFlags(flags() | Qt::ItemIsEditable);
	else
		setFlags(flags() & ~Qt::ItemIsEditable);
}

/***************************************************************************

  class MyTreeWidget

***************************************************************************/

MyTreeWidget::MyTreeWidget(QWidget *parent)
: QTreeWidget(parent)
{
  _auto_resize = true;
  _column = -1;
}

void MyTreeWidget::dragEnterEvent(QDragEnterEvent *e)
{
	//qDebug("contentsDragEnterEvent");
	//QListView::contentsDragEnterEvent(e);
}

void MyTreeWidget::dragMoveEvent(QDragMoveEvent *e)
{
	//qDebug("contentsDragMoveEvent");
	//QListView::contentsDragMoveEvent(e);
}

void MyTreeWidget::dropEvent(QDropEvent *e)
{
	//qDebug("contentsDropEvent");
	//QListView::contentsDropEvent(e);
}

void MyTreeWidget::dragLeaveEvent(QDragLeaveEvent *e)
{
	//QListView::contentsDragLeaveEvent(e);
}

void MyTreeWidget::setAutoResize(bool a)
{
	/*_auto_resize = a;
	
	for (int i = 0; i < columns(); i++)
		setAutoResize(i, a);*/
}

void MyTreeWidget::setAutoResize(int col, bool a)
{
	/*setColumnWidthMode(col, QTreeWidget::Manual);
	if (a)
	{
		QFontMetrics fm(font());
		QString s = columnText(col);
		setColumnWidth(col, fm.width(s) + 16);
		setColumnWidthMode(col, QTreeWidget::Maximum);
	}*/
}

int MyTreeWidget::minimumWidth(int col)
{
	/*QFontMetrics fm(font());
	QString s = columnText(col);
	return fm.width(s) + 16;*/
	return 16;
}

/*
void MyTreeWidget::setColumnText(int col, const QString &s)
{
	QTreeWidget::setColumnText(col, s);
	if (_auto_resize)
	{
		int w = minimumWidth(col);
		if (columnWidth(col) < w)
			setColumnWidth(col, w);
	}
}
*/

/***************************************************************************

  TreeView

***************************************************************************/

static int check_item(void *_object)
{
	return THIS->item == 0 || CWIDGET_check(_object);
}

static void return_item(void *_object, MyTreeWidgetItem *item)
{
	if (item == (MyTreeWidgetItem *)WIDGET->invisibleRootItem())
		item = 0;
	
  if (!item)
    THIS->save = THIS->item;
  THIS->item = item;

  GB.ReturnBoolean(item == 0);
}


static MyTreeWidget *init_treewidget(void *_object, void *parent)
{
  MyTreeWidget *wid;

  wid = new MyTreeWidget(QCONTAINER(parent));

  QObject::connect(wid, SIGNAL(itemSelectionChanged()), &CListView::manager, SLOT(selected()));
  QObject::connect(wid, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)), &CListView::manager, SLOT(activated(QTreeWidgetItem *)));
  QObject::connect(wid, SIGNAL(itemPressed(QTreeWidgetItem *, int)), &CListView::manager, SLOT(clicked(QTreeWidgetItem *)));
  // TODO QObject::connect(wid, SIGNAL(itemRenamed(QTreeWidgetItem *, int)), &CListView::manager, SLOT(renamed(QTreeWidgetItem *, int)));

  wid->setSortingEnabled(false);
  wid->setSelectionMode(QAbstractItemView::SingleSelection);
	//wid->setItemMargin(2);

	THIS->widget.flag.fillBackground = true;
  CWIDGET_new(wid, (void *)_object);

  THIS->dict = new QHash<QByteArray, MyTreeWidgetItem *>;
  THIS->sorted = -1;
  THIS->asc = true;
  THIS->item = 0;
  THIS->save = 0;
  THIS->rename = false;

  return wid;
}


BEGIN_METHOD(TreeView_new, GB_OBJECT parent)

  MyTreeWidget *wid = init_treewidget(_object, VARG(parent));

  QObject::connect(wid, SIGNAL(itemExpanded(QTreeWidgetItem *)), &CListView::manager, SLOT(expanded(QTreeWidgetItem *)));
  QObject::connect(wid, SIGNAL(itemCollapsed(QTreeWidgetItem *)), &CListView::manager, SLOT(collapsed(QTreeWidgetItem *)));

  wid->setColumnCount(1);
  wid->setHeaderHidden(true);
  wid->setRootIsDecorated(true);

END_METHOD


BEGIN_METHOD(ListView_new, GB_OBJECT parent)

  MyTreeWidget *wid = init_treewidget(_object, VARG(parent));

  wid->setColumnCount(1);
  wid->setHeaderHidden(true);
	wid->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  wid->setAllColumnsShowFocus(true);
	
  //wid->setColumnWidthMode(0, QTreeWidget::Manual);
  //wid->setResizeMode(QTreeWidget::LastColumn);

END_METHOD


BEGIN_METHOD(ColumnView_new, GB_OBJECT parent)

  MyTreeWidget *wid = init_treewidget(_object, VARG(parent));

  //QObject::connect(wid, SIGNAL(pressed(QListViewItem *, const QPoint &, int)), &CListView::manager, SLOT(columnClicked(QListViewItem *, const QPoint &, int)));
  QObject::connect(wid->header(), SIGNAL(sectionClicked(int)), &CListView::manager, SLOT(headerClicked(int)));
  //QObject::connect(wid->header(), SIGNAL(sizeChange(int, int, int)), &CListView::manager, SLOT(headerSizeChange(int, int, int)));
  QObject::connect(wid, SIGNAL(itemExpanded(QTreeWidgetItem *)), &CListView::manager, SLOT(expanded(QTreeWidgetItem *)));
  QObject::connect(wid, SIGNAL(itemCollapsed(QTreeWidgetItem *)), &CListView::manager, SLOT(collapsed(QTreeWidgetItem *)));

  wid->setColumnCount(1);
  //wid->setColumnAlignment(0, Qt::AlignLeft);
  //wid->setColumnWidthMode(0, QListView::Manual);
  wid->header()->setMovable(false);
  //wid->header()->setResizeEnabled(false);
  wid->setAllColumnsShowFocus(true);
  
  //wid->setResizeMode(QTreeWidget::LastColumn);

END_METHOD


BEGIN_METHOD_VOID(TreeView_free)

  delete THIS->dict;
  //if (THIS->numeric)
  //  delete[] THIS->numeric;

END_METHOD


/*BEGIN_PROPERTY(TreeView_drop)

  if (READ_PROPERTY)
    GB.ReturnBoolean(WIDGET->acceptDrops());
  else
  {
    WIDGET->setAcceptDrops(VPROP(GB_BOOLEAN));
    WIDGET->viewport()->setAcceptDrops(VPROP(GB_BOOLEAN));
	}

END_PROPERTY*/


BEGIN_METHOD_VOID(TreeView_clear)

  WIDGET->clear();
  THIS->dict->clear();

END_METHOD

BEGIN_METHOD_VOID(ColumnView_clear)

  WIDGET->clear();
  THIS->dict->clear();

#if 0
  if (WIDGET->isAutoResize())
  {
		//QListView::ResizeMode mode = WIDGET->resizeMode();
		WIDGET->setResizeMode(QTreeWidget::NoColumn);
		
		for (int i = 0; i < WIDGET->columns(); i++)
		{
			WIDGET->setColumnWidthMode(i, QTreeWidget::Manual);
			WIDGET->setColumnText(i, WIDGET->columnText(i));
			WIDGET->setColumnWidth(i, WIDGET->minimumWidth(i));
			WIDGET->setColumnWidthMode(i, QTreeWidget::Maximum);  	
		}
		
		WIDGET->setAutoResize(WIDGET->isAutoResize());
		WIDGET->setResizeMode(QTreeWidget::LastColumn);
	}
#endif

END_METHOD


BEGIN_METHOD(TreeView_add, GB_STRING key; GB_STRING text; GB_OBJECT picture; GB_STRING parent; GB_STRING after)

  MyTreeWidgetItem *item;
  MyTreeWidget *wid = WIDGET;
  char *key = GB.ToZeroString(ARG(key));
  MyTreeWidgetItem *parent = NULL;
  MyTreeWidgetItem *after = NULL;
  char *akey;

  /*
  if (GB.IsMissing(4))
    qDebug("TreeView_add: key = %s  parent = NULL", key);
  else
    qDebug("TreeView_add: key = %s  parent = %s", key, GB.ToZeroString(PARAM(parent)));
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
      item = new MyTreeWidgetItem(THIS, wid);
    else
      item = new MyTreeWidgetItem(THIS, wid, after);
  }
  else
  {
    if (after == NULL)
      item = new MyTreeWidgetItem(THIS, parent);
    else
      item = new MyTreeWidgetItem(THIS, parent, after);
  }

  item->setText(0, QSTRING_ARG(text));

  GB.StoreString(ARG(key), &item->key);
  THIS->dict->insert(item->key, item);

  if (!MISSING(picture))
    item->setPicture(ARG(picture));

	item->setEditable(THIS->rename);

  THIS->item = item;

  RETURN_SELF();

END_METHOD


BEGIN_METHOD(ListView_add, GB_STRING key; GB_STRING text; GB_OBJECT picture; GB_STRING after)

  MyTreeWidgetItem *item;
  MyTreeWidget *wid = WIDGET;
  char *key = GB.ToZeroString(ARG(key));
  MyTreeWidgetItem *after = NULL;
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
    item = new MyTreeWidgetItem(THIS, wid);
  else
    item = new MyTreeWidgetItem(THIS, wid, after);

  item->setText(0, QSTRING_ARG(text));

  GB.StoreString(ARG(key), &item->key);
  THIS->dict->insert(item->key, item);

  if (!MISSING(picture))
    item->setPicture(ARG(picture));

  item->setEditable(THIS->rename);

  THIS->item = item;

  RETURN_SELF();

END_METHOD


BEGIN_METHOD(TreeView_remove, GB_STRING key)

  MyTreeWidgetItem *lvi;
  char *key = GB.ToZeroString(ARG(key));
  //QListView *wid = QLISTVIEW(_object);

  lvi = CListView::getItem(THIS, key);
  if (!lvi)
    return;

  //THIS->dict->remove(key);
  delete lvi;

END_METHOD


BEGIN_METHOD(TreeView_exist, GB_STRING key)

  GB.ReturnBoolean((*THIS->dict)[GB.ToZeroString(ARG(key))] != 0);

END_METHOD


BEGIN_METHOD(TreeView_find, GB_INTEGER x; GB_INTEGER y)

  QPoint p(VARG(x), VARG(y));
  MyTreeWidgetItem *lvi;

	if (!WIDGET->header()->isHidden())
		p.setY(p.y() - WIDGET->header()->height());

  //qDebug("before: %d %d", p.x(), p.y());
  WIDGET->viewport()->mapFrom(WIDGET, p);
  //qDebug("after: %d %d", p.x(), p.y());
  lvi = (MyTreeWidgetItem *)WIDGET->itemAt(p);

  return_item(THIS, lvi);

END_METHOD


BEGIN_METHOD(TreeView_get, GB_STRING key)

  MyTreeWidgetItem *item;
  char *key = GB.ToZeroString(ARG(key));

  item = CListView::getItem(THIS, key);
  if (!item)
    return;

  THIS->item = item;
  RETURN_SELF();

END_METHOD


BEGIN_PROPERTY(TreeView_mode)

  if (READ_PROPERTY)
    GB.ReturnInteger(CCONST_convert(_selection_mode, WIDGET->selectionMode(), SELECT_NONE, false));
  else
    WIDGET->setSelectionMode((QTreeWidget::SelectionMode)CCONST_convert(_selection_mode, VPROP(GB_INTEGER), SELECT_NONE, true));

END_PROPERTY


BEGIN_PROPERTY(TreeView_current)

  MyTreeWidgetItem *item = (MyTreeWidgetItem *)(WIDGET->currentItem());

  //if (item && (WIDGET->selectionMode() == QListView::Single) && !item->isSelected())
  //  item = 0;

  THIS->item = item;

  if (item == 0)
    GB.ReturnNull();
  else
    RETURN_SELF();

END_PROPERTY


BEGIN_PROPERTY(TreeView_key)

  MyTreeWidgetItem *item = (MyTreeWidgetItem *)(WIDGET->currentItem());

  //THIS->item = item;

  if (item == 0)
    GB.ReturnNull();
  else
    GB.ReturnString(item->key);

END_PROPERTY


BEGIN_PROPERTY(TreeView_item)

  if (THIS->item == 0)
    GB.ReturnNull();
  else
    RETURN_SELF();

END_PROPERTY


BEGIN_PROPERTY(TreeView_available)

  GB.ReturnBoolean(THIS->item != 0);

END_PROPERTY


/*BEGIN_PROPERTY(TreeView_show_root)

  if (READ_PROPERTY)
    GB.ReturnBoolean(WIDGET->rootIsDecorated());
  else
    WIDGET->setRootIsDecorated(VPROP(GB_BOOLEAN));

END_PROPERTY*/


BEGIN_PROPERTY(TreeView_sorted)

  if (READ_PROPERTY)
    GB.ReturnBoolean(OBJECT(CLISTVIEW)->sorted != -1);
  else
  {
    OBJECT(CLISTVIEW)->sorted = VPROP(GB_BOOLEAN) ? 0 : -1;
    WIDGET->setSortingEnabled(OBJECT(CLISTVIEW)->sorted);
    WIDGET->header()->setSortIndicatorShown(VPROP(GB_BOOLEAN));
  }

END_PROPERTY


BEGIN_PROPERTY(TreeView_editable)

  if (READ_PROPERTY)
    GB.ReturnBoolean(THIS->rename);
  else
    THIS->rename = VPROP(GB_BOOLEAN);

END_PROPERTY


BEGIN_PROPERTY(TreeView_count)

  GB.ReturnInteger(THIS->dict->count());

END_PROPERTY


BEGIN_METHOD_VOID(TreeView_back)

  MyTreeWidgetItem *item = THIS->save;
  THIS->save = 0;
  return_item(THIS, item);

END_METHOD

BEGIN_METHOD(TreeView_move_to, GB_STRING key)

  char *key = GB.ToZeroString(ARG(key));
  MyTreeWidgetItem *item = (*THIS->dict)[key];

  return_item(THIS, item);

END_METHOD


BEGIN_METHOD_VOID(TreeView_move_current)

  return_item(THIS, (MyTreeWidgetItem *)(WIDGET->currentItem()));

END_PROPERTY


BEGIN_METHOD_VOID(TreeView_first)

	//qDebug("TreeView_first: %d", WIDGET->childCount());
  return_item(THIS, (MyTreeWidgetItem *)(WIDGET->firstChild()));

END_METHOD

BEGIN_METHOD_VOID(TreeView_last)

	MyTreeWidgetItem *lwi, *lwn = 0;
	
	if (THIS->item)
		lwi = THIS->item; //(MyTreeWidgetItem *)WIDGET->firstChild(); 
	else
		lwi = WIDGET->firstChild();
		
	for (;;)
	{
		if (!lwi)
			break;
		lwn = lwi;
		lwi = lwi->nextSibling();
	}

  return_item(THIS, (MyTreeWidgetItem *)lwn);

END_METHOD


BEGIN_METHOD_VOID(TreeView_previous)

	MyTreeWidgetItem *lwi, *lwn = 0;
	
	if (THIS->item)
	{
		lwi = ((MyTreeWidgetItem *)THIS->item->parent())->firstChild(); 
		for (;;)
		{
			if (!lwi || lwi == THIS->item)
				break;
			lwn = lwi;
			lwi = lwi->nextSibling();
		}
	}

  return_item(THIS, lwn);

  //return_item(THIS, THIS->item ? (MyTreeWidgetItem *)(THIS->item->prev) : 0);

END_METHOD

BEGIN_METHOD_VOID(TreeView_next)

  return_item(THIS, THIS->item ? (MyTreeWidgetItem *)(THIS->item->nextSibling()) : 0);

END_METHOD


BEGIN_METHOD_VOID(TreeView_parent)

  return_item(THIS, THIS->item ? (MyTreeWidgetItem *)(THIS->item->parent()) : 0);

END_METHOD


BEGIN_METHOD_VOID(TreeView_child)

  return_item(THIS, THIS->item ? (MyTreeWidgetItem *)(THIS->item->firstChild()) : 0);

END_METHOD


BEGIN_METHOD_VOID(TreeView_above)

  return_item(THIS, THIS->item ? (MyTreeWidgetItem *)(WIDGET->itemAbove(THIS->item)) : 0);

END_METHOD


BEGIN_METHOD_VOID(TreeView_below)

  return_item(THIS, THIS->item ? (MyTreeWidgetItem *)(WIDGET->itemBelow(THIS->item)) : 0);

END_METHOD


BEGIN_METHOD_VOID(TreeView_SelectAll)

	WIDGET->selectAll();

END_METHOD


BEGIN_METHOD_VOID(TreeView_UnselectAll)

	WIDGET->clearSelection();

END_METHOD


BEGIN_PROPERTY(TreeViewItem_key)

	if (!THIS->item)
		GB.ReturnNull();
	else
		GB.ReturnString(THIS->item->key);

END_PROPERTY

BEGIN_PROPERTY(TreeViewItem_parent_key)

	MyTreeWidgetItem *parent = (MyTreeWidgetItem *)THIS->item->parent();

  GB.ReturnString(parent ? parent->key : NULL);

END_PROPERTY


BEGIN_PROPERTY(TreeViewItem_picture)

  if (READ_PROPERTY)
    GB.ReturnObject(THIS->item->picture);
  else
    THIS->item->setPicture(PROP(GB_OBJECT));

END_PROPERTY


BEGIN_PROPERTY(TreeViewItem_selected)

  MyTreeWidgetItem *it = THIS->item;

  if (READ_PROPERTY)
    GB.ReturnBoolean(it->isSelected());
  else
    it->setSelected(VPROP(GB_BOOLEAN));

END_PROPERTY


BEGIN_PROPERTY(TreeViewItem_expanded)

  MyTreeWidgetItem *it = THIS->item;

  if (it->childCount() == 0)
  {
    if (READ_PROPERTY)
      GB.ReturnBoolean(false);
    return;
  }

  if (READ_PROPERTY)
    GB.ReturnBoolean(it->isExpanded());
  else
    it->setExpanded(VPROP(GB_BOOLEAN));

END_PROPERTY


BEGIN_PROPERTY(TreeViewItem_text)

  MyTreeWidgetItem *it = THIS->item;

  if (READ_PROPERTY)
    GB.ReturnNewZeroString(TO_UTF8(it->text(0)));
  else
    it->setText(0, QSTRING_PROP());

END_PROPERTY


BEGIN_PROPERTY(TreeViewItem_count)

  GB.ReturnInteger(THIS->item->childCount());

END_PROPERTY


BEGIN_METHOD_VOID(TreeViewItem_ensure_visible)

  //qDebug("ensure visible: %p", THIS->item);

  WIDGET->scrollToItem(THIS->item);

END_METHOD


BEGIN_METHOD_VOID(TreeViewItem_delete)

  //THIS->dict->remove(THIS->item->key);

  //qDebug("TreeViewItem_delete < %p %p", THIS, THIS->item);
  delete THIS->item;
  //qDebug("TreeViewItem_delete > %p", THIS->item);

END_METHOD


BEGIN_METHOD_VOID(TreeViewItem_clear)

  MyTreeWidgetItem *item = THIS->item->firstChild();
  MyTreeWidgetItem *next;

  //qDebug("< TreeViewItem_clear");

  while (item)
  {
    next = item->nextSibling();
    //THIS->dict->remove(((MyTreeWidgetItem *)item)->key);
    delete item;
    item = next;
  }

  //qDebug("TreeViewItem_clear >");

END_METHOD


/*
BEGIN_METHOD_VOID(TreeViewItem_init)

  CLASS_Item = GB.FindClass("Item");

END_METHOD
*/


BEGIN_PROPERTY(TreeViewItem_editable)

  if (READ_PROPERTY)
    GB.ReturnBoolean(THIS->item->isEditable());
  else
    THIS->item->setEditable(VPROP(GB_BOOLEAN));

END_PROPERTY


BEGIN_METHOD_VOID(TreeViewItem_rename)

	WIDGET->editItem(THIS->item, 0);

END_METHOD

/*
BEGIN_PROPERTY(TreeViewItem_sort_key)

  MyTreeWidgetItem *it = THIS->item;

  if (READ_PROPERTY)
    GB.ReturnInteger(it->sortKey);
  else
    it->sortKey = VPROP(GB_INTEGER);

END_PROPERTY
*/

static void move_item(void *_object, QTreeWidgetItem *item, int index)
{
	QTreeWidgetItem *parent = item->parent();
	int current;
	
	current = parent->indexOfChild(item);
	parent->takeChild(current);
	
	if (index < 0)
		index = parent->childCount();
	
	parent->insertChild(index, item);
}

BEGIN_METHOD_VOID(TreeViewItem_move_first)

	move_item(THIS, THIS->item, 0);

END_METHOD

BEGIN_METHOD(TreeViewItem_move_after, GB_STRING key)

	MyTreeWidgetItem *after;

	if (MISSING(key) || LENGTH(key) == 0)
	{
		move_item(THIS, THIS->item, 0);
		return;
	}
		
	after = CListView::getItem(THIS, GB.ToZeroString(ARG(key)));
	if (!after)
		return;
	
	if (THIS->item->parent() != after->parent())
		return;
	
	move_item(THIS, THIS->item, after->index() + 1);
	
END_METHOD

BEGIN_METHOD(TreeViewItem_move_before, GB_STRING key)

	MyTreeWidgetItem *after;

	if (MISSING(key) || LENGTH(key) == 0)
	{
		move_item(THIS, THIS->item, -1);
		return;
	}
		
	after = CListView::getItem(THIS, GB.ToZeroString(ARG(key)));
	if (!after)
		return;
	
	if (THIS->item->parent() != after->parent())
		return;
	
	move_item(THIS, THIS->item, after->index());
	
END_METHOD


BEGIN_METHOD_VOID(TreeViewItem_move_last)

		move_item(THIS, THIS->item, -1);

END_METHOD


BEGIN_PROPERTY(TreeViewItem_x)

	int x; //, m;
  QRect r = WIDGET->visualItemRect(THIS->item);
	
	if (r.isValid())
	{
		//m = WIDGET->treeStepSize() * THIS->item->depth();
		x = WIDGET->viewport()->mapTo(WIDGET, r.topLeft()).x();
	}
	else
		x = 0;
	
	GB.ReturnInteger(x);

END_PROPERTY

BEGIN_PROPERTY(TreeViewItem_y)

	int y;
  QRect r = WIDGET->visualItemRect(THIS->item);
	
	if (r.isValid())
		y = WIDGET->viewport()->mapTo(WIDGET, r.topLeft()).y();
	else
		y = 0;
	
	GB.ReturnInteger(y);

END_PROPERTY

BEGIN_PROPERTY(TreeViewItem_w)

	int w;
  //QRect r = WIDGET->itemRect(THIS->item);
	
	/*w = WIDGET->header()->cellPos(WIDGET->header()->count() - 1) 
	    + WIDGET->header()->cellSize(WIDGET->header()->count() - 1)
	    - WIDGET->treeStepSize() * THIS->item->depth();*/

  QRect r = WIDGET->visualItemRect(THIS->item);
	
	if (r.isValid())
		w = r.width();
	else
		w = 0;
	
	GB.ReturnInteger(w);

END_PROPERTY

BEGIN_PROPERTY(TreeViewItem_h)

	int h;
  QRect r = WIDGET->visualItemRect(THIS->item);
	
	if (r.isValid())
		h = r.height();
	else
		h = 0;
	
	GB.ReturnInteger(h);

END_PROPERTY




/***************************************************************************

  Columns

***************************************************************************/

BEGIN_PROPERTY(ListView_resizable)

  if (READ_PROPERTY)
    GB.ReturnBoolean(WIDGET->header()->resizeMode(0) == QHeaderView::Interactive);
  else
    WIDGET->header()->setResizeMode(VPROP(GB_BOOLEAN) ? QHeaderView::Interactive : QHeaderView::Fixed);

END_PROPERTY


/*BEGIN_PROPERTY(ListView_moveable)

  if (READ_PROPERTY)
    GB.ReturnBoolean(WIDGET->header()->isMovingEnabled());
  else
    WIDGET->header()->setMovingEnabled(VPROP(GB_BOOLEAN));

END_PROPERTY*/


BEGIN_PROPERTY(ListView_columns_count)

  int col;
  //QTreeWidget::ResizeMode mode;
  //bool *numeric;
  //int i, n;

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

  //mode = WIDGET->resizeMode();
  //WIDGET->setResizeMode(QTreeWidget::NoColumn);
  
  WIDGET->setColumnCount(col);

  //WIDGET->setResizeMode(mode);

END_PROPERTY


BEGIN_METHOD(ListView_columns_get, GB_INTEGER col)

  int col = VARG(col);

  if (col < 0 || col >= WIDGET->columnCount())
  {
    GB.Error("Bad column index");
    return;
  }

  WIDGET->_column = col;

  RETURN_SELF();

END_PROPERTY


#if 0
BEGIN_METHOD_VOID(ListView_columns_adjust)

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
#endif

BEGIN_PROPERTY(ListView_column_text)

  if (READ_PROPERTY)
    GB.ReturnNewZeroString(TO_UTF8(WIDGET->headerItem()->text(WIDGET->_column)));
  else
    WIDGET->headerItem()->setText(WIDGET->_column, QSTRING_PROP());

END_PROPERTY


BEGIN_PROPERTY(ListView_column_width)

  int w;

  if (READ_PROPERTY)
    GB.ReturnInteger(WIDGET->columnWidth(WIDGET->_column));
  else
  {
    w = VPROP(GB_INTEGER);
		
		if (w < 0)
			WIDGET->resizeColumnToContents(WIDGET->_column);
		else
			WIDGET->setColumnWidth(WIDGET->_column, w);
  }

END_PROPERTY


BEGIN_PROPERTY(ListView_column_alignment)

  if (READ_PROPERTY)
    GB.ReturnInteger(CCONST_alignment((WIDGET->headerItem()->textAlignment(WIDGET->_column) | Qt::AlignVCenter) & ALIGN_MASK, ALIGN_NORMAL, false));
  else
    WIDGET->headerItem()->setTextAlignment(WIDGET->_column, CCONST_alignment(VPROP(GB_INTEGER), ALIGN_NORMAL, true)  & Qt::AlignHorizontal_Mask);

END_PROPERTY


BEGIN_PROPERTY(ListView_columns_sort)

  int sort;

  if (READ_PROPERTY)
    GB.ReturnInteger(THIS->sorted);
  else
  {
    sort = VPROP(GB_INTEGER);
    if (sort < -1 || sort >= WIDGET->columnCount())
      return;
    THIS->sorted = sort;
    THIS->asc = true;
    WIDGET->setSortingEnabled(THIS->sorted);
  }

END_PROPERTY


BEGIN_PROPERTY(ListView_columns_ascending)

  if (READ_PROPERTY)
    GB.ReturnBoolean(THIS->asc);
  else
  {
    THIS->asc = VPROP(GB_BOOLEAN);
    WIDGET->setSortingEnabled(THIS->sorted);
  }

END_PROPERTY

/*
BEGIN_PROPERTY(ListView_column_numeric)

  if (READ_PROPERTY)
    GB.ReturnNewZeroString(TO_UTF8(WIDGET->columnText(_column)));
  else
    WIDGET->setColumnText(_column, QSTRING_PROP());

END_PROPERTY
*/


BEGIN_PROPERTY(ListView_auto_resize)

  if (READ_PROPERTY)
    GB.ReturnBoolean(WIDGET->isAutoResize());
  else
    WIDGET->setAutoResize(VPROP(GB_BOOLEAN));

END_PROPERTY


BEGIN_METHOD(TreeViewItem_get, GB_INTEGER col)

  const MyTreeWidgetItem* item = THIS->item;

  GB.ReturnNewZeroString(TO_UTF8(item->text(VARG(col))));

END_METHOD


BEGIN_METHOD(TreeViewItem_put, GB_STRING text; GB_INTEGER col)

  MyTreeWidgetItem* item = THIS->item;

  item->setText(VARG(col), QSTRING_ARG(text));

END_METHOD


BEGIN_PROPERTY(TreeView_client_width)

  //int w = WIDGET->width() - WIDGET->frameWidth() * 2;
  //int sw = WIDGET->verticalScrollBar()->width();
  //int width = w;

  //if (WIDGET->verticalScrollBar()->isHidden())
  //  width -= sw;

  //qDebug("client_width = %ld (%ld) -> %ld", w, sw, width);


  GB.ReturnInteger(WIDGET->width() - WIDGET->frameWidth() * 2);
  //GB.ReturnInteger(width);

END_PROPERTY


BEGIN_PROPERTY(TreeView_client_height)

  GB.ReturnInteger(WIDGET->height() - WIDGET->frameWidth() * 2);

END_PROPERTY


BEGIN_PROPERTY(TreeView_header)

  if (READ_PROPERTY)
    GB.ReturnBoolean(!WIDGET->isHeaderHidden());
  else
		WIDGET->setHeaderHidden(!VPROP(GB_BOOLEAN));

END_PROPERTY


BEGIN_PROPERTY(ListView_compare)

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

BEGIN_PROPERTY(ColumnView_settings)

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

BEGIN_PROPERTY(TreeView_Renaming)

	GB.ReturnBoolean(WIDGET->isRenaming());

END_PROPERTY

/***************************************************************************

  ListViewItem

***************************************************************************/

GB_DESC CListView2ItemDesc[] =
{
  GB_DECLARE(".ListView2.Item", 0), GB_VIRTUAL_CLASS(), GB_HOOK_CHECK(check_item),

  GB_PROPERTY_READ("Key", "s", TreeViewItem_key),
  GB_PROPERTY("Picture", "Picture", TreeViewItem_picture),
  GB_PROPERTY("Selected", "b", TreeViewItem_selected),
  GB_PROPERTY("Text", "s", TreeViewItem_text),
  GB_METHOD("EnsureVisible", NULL, TreeViewItem_ensure_visible, NULL),
  GB_METHOD("Delete", NULL, TreeViewItem_delete, NULL),

  GB_PROPERTY_READ("X", "i", TreeViewItem_x),
  GB_PROPERTY_READ("Y", "i", TreeViewItem_y),
  GB_PROPERTY_READ("W", "i", TreeViewItem_w),
  GB_PROPERTY_READ("Width", "i", TreeViewItem_w),
  GB_PROPERTY_READ("H", "i", TreeViewItem_h),
  GB_PROPERTY_READ("Height", "i", TreeViewItem_h),
  
  GB_METHOD("MoveAfter", NULL, TreeViewItem_move_after, "[(Key)s]"),
  GB_METHOD("MoveBefore", NULL, TreeViewItem_move_before, "[(Key)s]"),
  GB_METHOD("MoveFirst", NULL, TreeViewItem_move_first, NULL),
  GB_METHOD("MoveLast", NULL, TreeViewItem_move_last, NULL),
  
  GB_PROPERTY("Editable", "b", TreeViewItem_editable),
  GB_METHOD("Rename", NULL, TreeViewItem_rename, NULL),

  GB_END_DECLARE
};


/***************************************************************************

  TreeViewItem

***************************************************************************/

GB_DESC CTreeView2ItemDesc[] =
{
  GB_DECLARE(".TreeView2.Item", 0), GB_VIRTUAL_CLASS(), GB_HOOK_CHECK(check_item),

  GB_PROPERTY_READ("Key", "s", TreeViewItem_key),
  GB_PROPERTY_READ("ParentKey", "s", TreeViewItem_parent_key),
  GB_PROPERTY("Picture", "Picture", TreeViewItem_picture),
  GB_PROPERTY("Selected", "b", TreeViewItem_selected),
  GB_PROPERTY("Text", "s", TreeViewItem_text),
  GB_METHOD("EnsureVisible", NULL, TreeViewItem_ensure_visible, NULL),
  GB_METHOD("Delete", NULL, TreeViewItem_delete, NULL),
  GB_METHOD("Clear", NULL, TreeViewItem_clear, NULL),

  GB_PROPERTY("Expanded", "b", TreeViewItem_expanded),
  GB_PROPERTY_READ("Children", "i", TreeViewItem_count),
  GB_PROPERTY_READ("Count", "i", TreeViewItem_count),

  GB_PROPERTY_READ("X", "i", TreeViewItem_x),
  GB_PROPERTY_READ("Y", "i", TreeViewItem_y),
  GB_PROPERTY_READ("W", "i", TreeViewItem_w),
  GB_PROPERTY_READ("Width", "i", TreeViewItem_w),
  GB_PROPERTY_READ("H", "i", TreeViewItem_h),
  GB_PROPERTY_READ("Height", "i", TreeViewItem_h),
  
  /*GB_METHOD("MoveNext", "b", TreeView_next, NULL),
  GB_METHOD("MovePrevious", "b", TreeView_previous, NULL),
  GB_METHOD("MoveChild", "b", TreeView_child, NULL),
  GB_METHOD("MoveParent", "b", TreeView_parent, NULL),
  GB_METHOD("MoveAbove", "b", TreeView_above, NULL),
  GB_METHOD("MoveBelow", "b", TreeView_below, NULL),*/

  GB_METHOD("MoveAfter", NULL, TreeViewItem_move_after, "[(Key)s]"),
  GB_METHOD("MoveBefore", NULL, TreeViewItem_move_before, "[(Key)s]"),
  GB_METHOD("MoveFirst", NULL, TreeViewItem_move_first, NULL),
  GB_METHOD("MoveLast", NULL, TreeViewItem_move_last, NULL),
  
  GB_PROPERTY("Editable", "b", TreeViewItem_editable),
  GB_METHOD("Rename", NULL, TreeViewItem_rename, NULL),

  GB_END_DECLARE
};


/***************************************************************************

  ColumnViewItem

***************************************************************************/

GB_DESC CColumnView2ItemDesc[] =
{
  GB_DECLARE(".ColumnView2.Item", 0), GB_VIRTUAL_CLASS(), GB_HOOK_CHECK(check_item),

  GB_PROPERTY_READ("Key", "s", TreeViewItem_key),
  GB_PROPERTY_READ("ParentKey", "s", TreeViewItem_parent_key),
  GB_PROPERTY("Picture", "Picture", TreeViewItem_picture),
  GB_PROPERTY("Selected", "b", TreeViewItem_selected),
  GB_PROPERTY("Text", "s", TreeViewItem_text),
  GB_METHOD("EnsureVisible", NULL, TreeViewItem_ensure_visible, NULL),
  GB_METHOD("Delete", NULL, TreeViewItem_delete, NULL),
  GB_METHOD("Clear", NULL, TreeViewItem_clear, NULL),

  GB_PROPERTY("Expanded", "b", TreeViewItem_expanded),
  GB_PROPERTY_READ("Children", "i", TreeViewItem_count),
  GB_PROPERTY_READ("Count", "i", TreeViewItem_count),

  GB_PROPERTY_READ("X", "i", TreeViewItem_x),
  GB_PROPERTY_READ("Y", "i", TreeViewItem_y),
  GB_PROPERTY_READ("W", "i", TreeViewItem_w),
  GB_PROPERTY_READ("Width", "i", TreeViewItem_w),
  GB_PROPERTY_READ("H", "i", TreeViewItem_h),
  GB_PROPERTY_READ("Height", "i", TreeViewItem_h),
  /*GB_METHOD("MoveNext", "b", TreeView_next, NULL),
  GB_METHOD("MovePrevious", "b", TreeView_previous, NULL),
  GB_METHOD("MoveChild", "b", TreeView_child, NULL),
  GB_METHOD("MoveParent", "b", TreeView_parent, NULL),
  GB_METHOD("MoveAbove", "b", TreeView_above, NULL),
  GB_METHOD("MoveBelow", "b", TreeView_below, NULL),*/

  GB_METHOD("MoveAfter", NULL, TreeViewItem_move_after, "[(Key)s]"),
  GB_METHOD("MoveBefore", NULL, TreeViewItem_move_before, "[(Key)s]"),
  GB_METHOD("MoveFirst", NULL, TreeViewItem_move_first, NULL),
  GB_METHOD("MoveLast", NULL, TreeViewItem_move_last, NULL),
  
  GB_METHOD("_get", "s", TreeViewItem_get, "(Column)i"),
  GB_METHOD("_put", NULL, TreeViewItem_put, "(Text)s(Column)i"),

  GB_PROPERTY("Editable", "b", TreeViewItem_editable),
  GB_METHOD("Rename", NULL, TreeViewItem_rename, NULL),

  //GB_PROPERTY("SortKey", "f", TreeViewItem_sort_key),

  GB_END_DECLARE
};


/***************************************************************************

  ListView

***************************************************************************/

GB_DESC CListView2Desc[] =
{
  GB_DECLARE("ListView2", sizeof(CLISTVIEW)), GB_INHERITS("Control"),

  GB_METHOD("_new", NULL, ListView_new, "(Parent)Container;"),
  GB_METHOD("_free", NULL, TreeView_free, NULL),

  //GB_PROPERTY("Drop", "b", TreeView_drop),
  GB_PROPERTY("Mode", "i", TreeView_mode),
  GB_PROPERTY("Sorted", "b", TreeView_sorted),
  GB_PROPERTY("Editable", "b", TreeView_editable),
  GB_PROPERTY("ScrollBar", "i", CWIDGET_scrollbar),
  GB_PROPERTY("Border", "b", CWIDGET_border_simple),
  GB_PROPERTY("Compare", "i", ListView_compare),

  GB_PROPERTY_READ("Count", "i", TreeView_count),

  GB_PROPERTY_READ("Available", "b", TreeView_available),
  GB_METHOD("MoveTo", "b", TreeView_move_to, "(Key)s"),
  GB_METHOD("MoveCurrent", "b", TreeView_move_current, NULL),
  GB_METHOD("MoveFirst", "b", TreeView_first, NULL),
  GB_METHOD("MoveLast", "b", TreeView_last, NULL),
  GB_METHOD("MovePrevious", "b", TreeView_previous, NULL),
  GB_METHOD("MoveNext", "b", TreeView_next, NULL),
  GB_METHOD("MoveAbove", "b", TreeView_above, NULL),
  GB_METHOD("MoveBelow", "b", TreeView_below, NULL),
  GB_METHOD("MoveBack", "b", TreeView_back, NULL),

  GB_METHOD("_get", ".ListView2.Item", TreeView_get, "(Key)s"),

  GB_METHOD("Clear", NULL, TreeView_clear, NULL),
  GB_METHOD("Add", ".ListView2.Item", ListView_add, "(Key)s(Text)s[(Picture)Picture;(After)s]"),
  GB_METHOD("Remove", NULL, TreeView_remove, "(Key)s"),
  GB_METHOD("Exist", "b", TreeView_exist, "(Key)s"),
  GB_METHOD("FindAt", "b", TreeView_find, "(X)i(Y)i"),
  //GB_METHOD("FindText", "b", TreeView_find, "(X)i(Y)i"),
  GB_METHOD("SelectAll", NULL, TreeView_SelectAll, NULL),
  GB_METHOD("UnselectAll", NULL, TreeView_UnselectAll, NULL),

  GB_PROPERTY_READ("Current", ".ListView2.Item", TreeView_current),
  GB_PROPERTY_READ("Key", "s", TreeView_key),
  GB_PROPERTY_READ("Item", ".ListView2.Item", TreeView_item),
  GB_PROPERTY_READ("Renaming", "b", TreeView_Renaming),

  GB_PROPERTY_READ("ClientWidth", "i", TreeView_client_width),
  GB_PROPERTY_READ("ClientW", "i",  TreeView_client_width),
  GB_PROPERTY_READ("ClientHeight", "i", TreeView_client_height),
  GB_PROPERTY_READ("ClientH", "i", TreeView_client_height),

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

GB_DESC CTreeView2Desc[] =
{
  GB_DECLARE("TreeView2", sizeof(CLISTVIEW)), GB_INHERITS("Control"),

  GB_METHOD("_new", NULL, TreeView_new, "(Parent)Container;"),
  GB_METHOD("_free", NULL, TreeView_free, NULL),

  //GB_PROPERTY("Drop", "b", TreeView_drop),
  GB_PROPERTY("Mode", "i", TreeView_mode),
  //GB_PROPERTY("Root", "b", TreeView_show_root),
  GB_PROPERTY("Sorted", "b", TreeView_sorted),
  GB_PROPERTY("Editable", "b", TreeView_editable),
  GB_PROPERTY("ScrollBar", "i", CWIDGET_scrollbar),
  GB_PROPERTY("Border", "b", CWIDGET_border_simple),
  GB_PROPERTY("Compare", "i", ListView_compare),

  GB_PROPERTY_READ("Count", "i", TreeView_count),

  GB_PROPERTY_READ("Available", "b", TreeView_available),
  
  GB_METHOD("MoveCurrent", "b", TreeView_move_current, NULL),
  GB_METHOD("MoveTo", "b", TreeView_move_to, "(Key)s"),
  GB_METHOD("MoveFirst", "b", TreeView_first, NULL),
  GB_METHOD("MoveLast", "b", TreeView_last, NULL),
  GB_METHOD("MoveNext", "b", TreeView_next, NULL),
  GB_METHOD("MovePrevious", "b", TreeView_previous, NULL),
  GB_METHOD("MoveChild", "b", TreeView_child, NULL),
  GB_METHOD("MoveParent", "b", TreeView_parent, NULL),
  GB_METHOD("MoveAbove", "b", TreeView_above, NULL),
  GB_METHOD("MoveBelow", "b", TreeView_below, NULL),
  GB_METHOD("MoveBack", "b", TreeView_back, NULL),

  /*GB_METHOD("GoToCurrent", "b", TreeView_move_current, NULL),
  GB_METHOD("GoTo", "b", TreeView_move_to, "(Key)s"),
  GB_METHOD("GoToFirst", "b", TreeView_first, NULL),
  GB_METHOD("GoToLast", "b", TreeView_last, NULL),
  GB_METHOD("GoToNext", "b", TreeView_next, NULL),
  GB_METHOD("GoToPrevious", "b", TreeView_previous, NULL),
  GB_METHOD("GoToChild", "b", TreeView_child, NULL),
  GB_METHOD("GoToParent", "b", TreeView_parent, NULL),
  GB_METHOD("GoAbove", "b", TreeView_above, NULL),
  GB_METHOD("GoBelow", "b", TreeView_below, NULL),
  GB_METHOD("GoBack", "b", TreeView_back, NULL),*/

  GB_METHOD("_get", ".TreeView2.Item", TreeView_get, "(Key)s"),

  GB_METHOD("Clear", NULL, TreeView_clear, NULL),
  GB_METHOD("Add", ".TreeView2.Item", TreeView_add, "(Key)s(Text)s[(Picture)Picture;(Parent)s(After)s]"),
  GB_METHOD("Remove", NULL, TreeView_remove, "(Key)s"),
  GB_METHOD("Exist", "b", TreeView_exist, "(Key)s"),
  GB_METHOD("FindAt", "b", TreeView_find, "(X)i(Y)i"),
  GB_METHOD("SelectAll", NULL, TreeView_SelectAll, NULL),
  GB_METHOD("UnselectAll", NULL, TreeView_UnselectAll, NULL),

  GB_PROPERTY_READ("Current", ".TreeView2.Item", TreeView_current),
  GB_PROPERTY_READ("Key", "s", TreeView_key),
  GB_PROPERTY_READ("Item", ".TreeView2.Item", TreeView_item),
  GB_PROPERTY_READ("Renaming", "b", TreeView_Renaming),

  GB_PROPERTY_READ("ClientWidth", "i", TreeView_client_width),
  GB_PROPERTY_READ("ClientW", "i",  TreeView_client_width),
  GB_PROPERTY_READ("ClientHeight", "i", TreeView_client_height),
  GB_PROPERTY_READ("ClientH", "i", TreeView_client_height),

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

GB_DESC CColumnView2ColumnDesc[] =
{
  GB_DECLARE(".ColumnView2.Column", 0), GB_VIRTUAL_CLASS(),

  GB_PROPERTY("Text", "s", ListView_column_text),
  GB_PROPERTY("Alignment", "i", ListView_column_alignment),
  GB_PROPERTY("W", "i", ListView_column_width),
  GB_PROPERTY("Width", "i", ListView_column_width),

  GB_END_DECLARE
};

GB_DESC CColumnView2ColumnsDesc[] =
{
  GB_DECLARE(".ColumnView2.Columns", 0), GB_VIRTUAL_CLASS(),

  GB_METHOD("_get", ".ColumnView2.Column", ListView_columns_get, "(Column)i"),

  GB_PROPERTY("Count", "i", ListView_columns_count),
  GB_PROPERTY("Sort", "i", ListView_columns_sort),
  GB_PROPERTY("Ascending", "b", ListView_columns_ascending),

  GB_END_DECLARE
};

GB_DESC CColumnView2Desc[] =
{
  GB_DECLARE("ColumnView2", sizeof(CLISTVIEW)), GB_INHERITS("Control"),

  GB_METHOD("_new", NULL, ColumnView_new, "(Parent)Container;"),
  GB_METHOD("_free", NULL, TreeView_free, NULL),

  GB_PROPERTY("Mode", "i", TreeView_mode),
  GB_PROPERTY("Sorted", "b", TreeView_sorted),
  GB_PROPERTY("Editable", "b", TreeView_editable),
  GB_PROPERTY("ScrollBar", "i", CWIDGET_scrollbar),
  GB_PROPERTY("Border", "b", CWIDGET_border_simple),
  GB_PROPERTY("Compare", "i", ListView_compare),

  GB_PROPERTY_READ("Count", "i", TreeView_count),

  GB_PROPERTY_READ("Available", "b", TreeView_available),
  GB_METHOD("MoveCurrent", "b", TreeView_move_current, NULL),
  GB_METHOD("MoveTo", "b", TreeView_move_to, "(Key)s"),
  GB_METHOD("MoveFirst", "b", TreeView_first, NULL),
  GB_METHOD("MoveLast", "b", TreeView_last, NULL),
  GB_METHOD("MoveNext", "b", TreeView_next, NULL),
  GB_METHOD("MovePrevious", "b", TreeView_previous, NULL),
  GB_METHOD("MoveChild", "b", TreeView_child, NULL),
  GB_METHOD("MoveParent", "b", TreeView_parent, NULL),
  GB_METHOD("MoveAbove", "b", TreeView_above, NULL),
  GB_METHOD("MoveBelow", "b", TreeView_below, NULL),
  GB_METHOD("MoveBack", "b", TreeView_back, NULL),

  GB_METHOD("_get", ".ColumnView2.Item", TreeView_get, "(Key)s"),

  GB_METHOD("Clear", NULL, ColumnView_clear, NULL),
  GB_METHOD("Add", ".ColumnView2.Item", TreeView_add, "(Key)s(Text)s[(Picture)Picture;(Parent)s(After)s]"),
  GB_METHOD("Remove", NULL, TreeView_remove, "(Key)s"),
  GB_METHOD("Exist", "b", TreeView_exist, "(Key)s"),
  GB_METHOD("FindAt", "b", TreeView_find, "(X)i(Y)i"),
  GB_METHOD("SelectAll", NULL, TreeView_SelectAll, NULL),
  GB_METHOD("UnselectAll", NULL, TreeView_UnselectAll, NULL),

  GB_PROPERTY_READ("Current", ".ColumnView2.Item", TreeView_current),
  GB_PROPERTY_READ("Key", "s", TreeView_key),
  GB_PROPERTY_READ("Item", ".ColumnView2.Item", TreeView_item),
  GB_PROPERTY_READ("Renaming", "b", TreeView_Renaming),

  GB_EVENT("Select", NULL, NULL, &EVENT_Select),
  GB_EVENT("Activate", NULL, NULL, &EVENT_Activate),
  GB_EVENT("Click", NULL, NULL, &EVENT_Click),
  GB_EVENT("Rename", NULL, NULL, &EVENT_Rename),
  GB_EVENT("Cancel", NULL, NULL, &EVENT_Cancel),
  GB_EVENT("Compare", NULL, "(Key)s(OtherKey)s", &EVENT_Compare),
  GB_EVENT("Expand", NULL, NULL, &EVENT_Expand),
  GB_EVENT("Collapse", NULL, NULL, &EVENT_Collapse),
  //GB_EVENT("ColumnClick", NULL, "(Column)i", &EVENT_ColumnClick),

  GB_PROPERTY_SELF("Columns", ".ColumnView2.Columns"),

  GB_PROPERTY("Resizable", "b", ListView_resizable),
  GB_PROPERTY("Header", "b", TreeView_header),
  GB_PROPERTY("AutoResize", "b", ListView_auto_resize),

  GB_PROPERTY_READ("ClientWidth", "i", TreeView_client_width),
  GB_PROPERTY_READ("ClientW", "i",  TreeView_client_width),
  GB_PROPERTY_READ("ClientHeight", "i", TreeView_client_height),
  GB_PROPERTY_READ("ClientH", "i", TreeView_client_height),

  GB_PROPERTY("Settings", "s", ColumnView_settings),

	COLUMNVIEW_DESCRIPTION,

  GB_END_DECLARE
};


/***************************************************************************

  class CListView

***************************************************************************/

CListView CListView::manager;


MyTreeWidgetItem *CListView::getItem(CLISTVIEW *treeview, char *key)
{
  MyTreeWidgetItem *item = (*treeview->dict)[key];

  if (item == 0)
  {
    GB.Error("Unknown item: '&1'", key);
    return NULL;
  }

  return item;
}

static void raise_event(void *_object, int ev, QTreeWidgetItem *it)
{
  MyTreeWidgetItem *old = THIS->item;

  if (!it)
    it = WIDGET->currentItem();
  THIS->item = (MyTreeWidgetItem *)it;
  GB.Raise(THIS, ev, 0);
  // TODO: what happens if old is destroyed during the event?
  THIS->item = old;
}

void CListView::raiseEvent(int ev, QTreeWidgetItem *it)
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

void CListView::selected(void)
{
  GET_SENDER();
  if (WIDGET->selectionMode() == QAbstractItemView::SingleSelection)
	  raise_event(_object, EVENT_Select, 0);
	else
	{
  	GB.Ref(_object);
  	GB.Post((GB_POST_FUNC)post_select_event, (intptr_t)THIS);
	}
}

void CListView::activated(QTreeWidgetItem *it)
{
  if (!it)
    return;
  raiseEvent(EVENT_Activate, it);
}

void CListView::clicked(QTreeWidgetItem *it)
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

void CListView::renamed(QTreeWidgetItem *it, int col)
{
  GET_SENDER();

  if (it == 0)
    return;

  THIS->item = (MyTreeWidgetItem *)it;
  GB.Ref(THIS);
  GB.Post((void (*)())post_rename_event, (intptr_t)THIS);
}

// void CListView::columnClicked(QListViewItem *it, const QPoint &p, int c)
// {
//   GET_SENDER();
// 
//   if (it == 0)
//     return;
// 
//   THIS->item = (MyTreeWidgetItem *)it;
// 
//   GB.Raise(_object, EVENT_ColumnClick, 1, GB_T_INTEGER, c);
// }

void CListView::expanded(QTreeWidgetItem *it)
{
  raiseEvent(EVENT_Expand, it);
}

void CListView::collapsed(QTreeWidgetItem *it)
{
  raiseEvent(EVENT_Collapse, it);
}

void CListView::headerClicked(int c)
{
  CLISTVIEW *_object = (CLISTVIEW *)CWidget::get((QObject *)sender());

  if (_object->sorted == c)
    _object->asc = !_object->asc;
  else
    _object->asc = true;

  _object->sorted = c;

  //WIDGET->setSorted(_object->sorted, _object->asc);
}


void CListView::headerSizeChange(int c, int os, int ns)
{
//  CLISTVIEW *_object = (CLISTVIEW *)CWidget::get((QObject *)sender());
//  updateLastColumn(WIDGET);
}

