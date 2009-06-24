/***************************************************************************

  CListBox.cpp

  The ListBox class

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
  along with WIDGET program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

***************************************************************************/


#define __CLISTBOX_CPP

#include <qapplication.h>
#include <qpainter.h>
//Added by qt3to4:
#include <QResizeEvent>
#include <QMouseEvent>
#include <QListWidget>

#include "gambas.h"

#include "CWidget.h"
#include "CPicture.h"
#include "CDraw.h"
#include "CListBox.h"
#include "CConst.h"


DECLARE_EVENT(EVENT_Select);    /* selection change */
DECLARE_EVENT(EVENT_Click);     /* simple click */
DECLARE_EVENT(EVENT_Activate);  /* double click */
//DECLARE_EVENT(EVENT_Draw);

static int _selection_mode[] = 
{
	SELECT_NONE, QAbstractItemView::NoSelection, 
	SELECT_SINGLE, QAbstractItemView::SingleSelection, 
	SELECT_MULTIPLE, QAbstractItemView::ExtendedSelection, 
	CONST_MAGIC
};


BEGIN_METHOD(CLISTBOX_new, GB_OBJECT parent)

  QListWidget *wid = new QListWidget(QCONTAINER(VARG(parent)));

  QObject::connect(wid, SIGNAL(itemSelectionChanged()), &CListBox::manager, SLOT(selected()));
  QObject::connect(wid, SIGNAL(itemActivated(QListWidgetItem *)), &CListBox::manager, SLOT(activated(QListWidgetItem *)));
  //QObject::connect(wid, SIGNAL(highlighted(int)), &CListBox::manager, SLOT(highlighted(int)));
  QObject::connect(wid, SIGNAL(currentItemChanged(QListWidgetItem *, QListWidgetItem *)), &CListBox::manager, SLOT(clicked(QListWidgetItem *)));

	THIS->widget.flag.fillBackground = true;
  CWIDGET_new(wid, (void *)_object);

  THIS->last = -1;

END_METHOD


BEGIN_METHOD_VOID(CLISTBOX_clear)

  WIDGET->clear();

END_METHOD


BEGIN_METHOD(CLISTBOX_get, GB_INTEGER index)

  int index = VARG(index);

  if (index < 0 || index >= (int)WIDGET->count())
  {
    //qDebug("index = %ld", index);
    GB.Error("Bad index");
    return;
  }

  THIS->index = index;

  RETURN_SELF();

END_METHOD

/*
BEGIN_METHOD(CLISTBOX_set, GB_STRING item; int index)

  int index = PARAM(index);
  int current;
  bool selected;

  if (index < 0 || index >= (int)WIDGET->count())
    return;

  selected = WIDGET->isSelected(PARAM(index));

  WIDGET->changeItem(QString(GB.ToZeroString(PARAM(item))), index);
  WIDGET->setSelected(index, selected);

  current = WIDGET->currentItem();
  WIDGET->setCurrentItem(current);

END_METHOD
*/

BEGIN_METHOD(CLISTBOX_add, GB_STRING item; GB_INTEGER pos)

  int pos = VARGOPT(pos, -1);

  if (pos < 0)
  	WIDGET->addItem(QSTRING_ARG(item));
  else
  	WIDGET->insertItem(pos, QSTRING_ARG(item));
  
END_METHOD


/*
BEGIN_METHOD(CLISTBOX_add_custom, GB_INTEGER width; GB_INTEGER height; GB_STRING text; GB_INTEGER pos)

  int pos = VARGOPT(pos, -1);
  QString text(GB.ToZeroString(ARG(text)));

  WIDGET->insertItem(new MyListBoxItem(WIDGET, text, VARG(width), VARG(height)), pos);
  if (THIS->sorted)
    WIDGET->sort();

END_METHOD
*/

BEGIN_METHOD(CLISTBOX_remove, GB_INTEGER pos)

  //WIDGET->blockSignals(true);
	//qDebug("count = %d pos = %d item = %p", WIDGET->count(), VARG(pos), WIDGET->item(VARG(pos)));
  delete WIDGET->takeItem(VARG(pos));
	//qDebug("--> count = %d", WIDGET->count());
  //WIDGET->blockSignals(false);

END_METHOD


BEGIN_PROPERTY(CLISTBOX_sorted)

  if (READ_PROPERTY)
    GB.ReturnBoolean(WIDGET->isSortingEnabled());
  else
  	WIDGET->setSortingEnabled(VPROP(GB_BOOLEAN));

END_METHOD


BEGIN_PROPERTY(CLISTBOX_count)

  GB.ReturnInteger(WIDGET->count());

END_PROPERTY


BEGIN_PROPERTY(CLISTBOX_index)

  int index;

  if (READ_PROPERTY)
    GB.ReturnInteger(WIDGET->currentRow());
  else
  {
    index = VPROP(GB_INTEGER);
    if (index < 0 || index >= (int)WIDGET->count())
    {
      WIDGET->clearSelection();
      return;
    }

    //if (WIDGET->selectionMode() == QListWidget::Multi || WIDGET->selectionMode() == QListWidget::Extended)
    //{
      //WIDGET->clearSelection();
      //WIDGET->setSelected(index, true);
    //}

    WIDGET->setCurrentRow(index);

    //if (WIDGET->selectionMode() == QAbstractItemView::SingleSelection)
    //  WIDGET->setSelected(WIDGET->currentItem(), true);

    //WIDGET->ensureCurrentVisible();
  }

END_PROPERTY


BEGIN_PROPERTY(CLISTBOX_current)

  THIS->index = WIDGET->currentRow();

  if (THIS->index < 0)
    GB.ReturnNull();
  else
    RETURN_SELF();

END_PROPERTY


BEGIN_PROPERTY(CLISTBOX_text)

  QListWidgetItem *item = WIDGET->currentItem();

  if (!item)
    GB.ReturnNull();
  else
    GB.ReturnNewZeroString(TO_UTF8(item->text()));

END_PROPERTY


BEGIN_PROPERTY(CLISTBOX_mode)

  if (READ_PROPERTY)
    GB.ReturnInteger(CCONST_convert(_selection_mode, WIDGET->selectionMode(), SELECT_NONE, false));
  else
    WIDGET->setSelectionMode((QListWidget::SelectionMode)CCONST_convert(_selection_mode, VPROP(GB_INTEGER), SELECT_NONE, true));

END_PROPERTY


BEGIN_PROPERTY(CLISTBOX_item_selected)

  if (READ_PROPERTY)
    GB.ReturnBoolean(WIDGET->item(THIS->index)->isSelected());
  else
    WIDGET->item(THIS->index)->setSelected(VPROP(GB_BOOLEAN));

END_PROPERTY


BEGIN_PROPERTY(CLISTBOX_item_text)

  if (READ_PROPERTY)
    GB.ReturnNewZeroString(TO_UTF8(WIDGET->item(THIS->index)->text()));
  else
  	WIDGET->item(THIS->index)->setText(QSTRING_PROP());

END_PROPERTY


/*
BEGIN_PROPERTY(CLISTBOX_item_custom)

  GB.ReturnBoolean(WIDGET->item(WIDGET->currentItem())->rtti() == CUSTOM_RTTI);

END_PROPERTY
*/

BEGIN_METHOD(CLISTBOX_find, GB_STRING item)

  GB.ReturnInteger(CListBox::find(WIDGET, QSTRING_ARG(item)));

END_METHOD


BEGIN_PROPERTY(CLISTBOX_list)

 	GB_ARRAY array;
 	
  if (READ_PROPERTY)
  {
  	GB.Array.New(&array, GB_T_STRING, WIDGET->count());
    CListBox::getAll(WIDGET, array);
    GB.ReturnObject(array);
	}
  else
  {
    CListBox::setAll(WIDGET, (GB_ARRAY)VPROP(GB_OBJECT));
  }

END_PROPERTY



GB_DESC CListBoxItemDesc[] =
{
  GB_DECLARE(".ListBoxItem", 0), GB_VIRTUAL_CLASS(),

  GB_PROPERTY("Text", "s", CLISTBOX_item_text),
  GB_PROPERTY("Selected", "b", CLISTBOX_item_selected),
  //GB_PROPERTY_READ("Custom", "b", CLISTBOX_item_custom),

  GB_END_DECLARE
};


GB_DESC CListBoxDesc[] =
{
  GB_DECLARE("ListBox", sizeof(CLISTBOX)), GB_INHERITS("Control"),

  //GB_CONSTANT("None", "i", SELECT_NONE),
  //GB_CONSTANT("Single", "i", SELECT_SINGLE),
  //GB_CONSTANT("Multi", "i", SELECT_MULTIPLE), // REMOVE
  //GB_CONSTANT("Multiple", "i", SELECT_MULTIPLE),
  //GB_CONSTANT("Extended", "i", QListWidget::Extended),

  GB_METHOD("_new", NULL, CLISTBOX_new, "(Parent)Container;"),

  GB_METHOD("_get", ".ListBoxItem", CLISTBOX_get, "(Index)i"),

  GB_METHOD("Clear", NULL, CLISTBOX_clear, NULL),
  GB_METHOD("Add", NULL, CLISTBOX_add, "(Text)s[(Index)i]"),
  //GB_METHOD("AddCustom", NULL, CLISTBOX_add_custom, "(Width)i(Height)i[(Text)s(After)i]"),
  GB_METHOD("Remove", NULL, CLISTBOX_remove, "(Index)i"),

  GB_PROPERTY("Sorted", "b", CLISTBOX_sorted),

  GB_PROPERTY("List", "String[]", CLISTBOX_list),
  //GB_PROPERTY("Contents", "s", CLISTBOX_list),

  GB_PROPERTY_READ("Count", "i", CLISTBOX_count),
  GB_PROPERTY_READ("Current", ".ListBoxItem", CLISTBOX_current),
  GB_PROPERTY_READ("Text", "s", CLISTBOX_text),
  GB_PROPERTY("Index", "i", CLISTBOX_index),

  GB_PROPERTY("Mode", "i", CLISTBOX_mode),

  GB_METHOD("Find", "i", CLISTBOX_find, "(Item)s"),

  /*
  GB_METHOD("Select", NULL, CLISTBOX_select, 1, 1, "i"),
  GB_METHOD("Invert", NULL, CLISTBOX_invert, 0, 0, NULL),
  */

  GB_EVENT("Select", NULL, NULL, &EVENT_Select),
  GB_EVENT("Activate", NULL, NULL, &EVENT_Activate),
  GB_EVENT("Click", NULL, NULL, &EVENT_Click),
  //GB_EVENT("Draw", NULL, "(Index)i", &EVENT_Draw),

	LISTBOX_DESCRIPTION,

  GB_END_DECLARE
};


/** MyListBox ***************************************************************/

#if 0
MyListBox::MyListBox(QWidget *parent) :
  QListWidget(parent)
{
}

void MyListBox::mousePressEvent( QMouseEvent *e )
{
  if (e->button() == Qt::LeftButton)
    QListWidget::mousePressEvent(e);
}

void MyListBox::resizeEvent( QResizeEvent *e )
{
  //qDebug("MyListBox::resizeEvent");
  Q3ScrollView::resizeEvent(e);
  triggerUpdate(true);
  doLayout();
  viewport()->repaint();
  ensureCurrentVisible();
}

void MyListBox::setCurrentItem(QListWidgetItem *item)
{
	//void *_object = CWidget::get(this);
	//int current = currentItem();
	
	//qDebug("setCurrentItem");
	QListWidget::setCurrentItem(item);
	//if (current == currentItem())
	//  GB.Raise(THIS, EVENT_Click, 0);	
}
#endif


#if 0
/** MyListBoxItem ***********************************************************/

MyListBoxItem::MyListBoxItem(QListWidget *listbox, QString& text, int width, int height)
  :QListWidgetItem(listbox)
{
  setText(text);
  w = width;
  h = height;
}

MyListBoxItem::~MyListBoxItem()
{
}

void MyListBoxItem::paint(QPainter *painter)
{
  int status = DRAW_status();
  DRAW_begin(DRAW_ON_ITEM, painter);

  //qDebug("MyDrawingArea::paintEvent %p", CWidget::get(this));
  GB.Raise(CWidget::get(listBox()), EVENT_Draw, 1,
    GB_T_INTEGER, listBox()->index(this));

  DRAW_restore(status);
}

int MyListBoxItem::height( const QListWidget* lb ) const
{
  return QMAX(h, QApplication::globalStrut().height());
}

int MyListBoxItem::width( const QListWidget* lb ) const
{
  return QMAX(w, QApplication::globalStrut().width());
}

int MyListBoxItem::RTTI = CUSTOM_RTTI;

int MyListBoxItem::rtti() const
{
  return RTTI;
}
#endif


/** CListBox ****************************************************************/

CListBox CListBox::manager;

static void post_select_event(void *_object)
{
	GB.Raise(_object, EVENT_Select, 0);
	GB.Unref(&_object);
}

void CListBox::selected(void)
{
	GET_SENDER(_object);
	if (WIDGET->selectionMode() == QAbstractItemView::SingleSelection)
		GB.Raise(_object, EVENT_Select, 0);
	else
	{
		GB.Ref(THIS);
		GB.Post((GB_POST_FUNC)post_select_event, (intptr_t)THIS);
	}
  //RAISE_EVENT(EVENT_Select);
}

void CListBox::activated(QListWidgetItem *)
{
  RAISE_EVENT(EVENT_Activate);
}

#if 0
void CListBox::highlighted(int index)
{
  GET_SENDER(_object);

  /*if (currentItem(WIDGET) < 0)
    return;

	if (!WIDGET->isSelected(index))
		return;*/

	THIS->last = index;
  GB.Raise(_object, EVENT_Click, 0);
}
#endif

void CListBox::clicked(QListWidgetItem *i)
{
	int current;
  GET_SENDER(_object);

  if (!i)
    return;

	current = WIDGET->row(i);

	if (THIS->last == current)
	{
		THIS->last = -1;
		return;
	}

  GB.Raise(_object, EVENT_Click, 0);
	THIS->last = -1;
}

#if 0
void CListBox::clicked(QListWidgetItem *item)
{
  int current;

  GET_SENDER(_object);

  if (!item)
    return;

  current = currentItem(WIDGET);

  if (current < 0 || WIDGET->item(current) != item)
    return;

  GB.Raise(_object, EVENT_Click, 0);
}
#endif

int CListBox::find(QListWidget *list, const QString& s)
{
  for (int i = 0; i < (int)list->count(); i++)
  {
    if (list->item(i)->text() == s)
      return i;
  }

  return (-1);
}


void CListBox::getAll(QListWidget *list, GB_ARRAY array)
{
	int i;
	char *str;
	
	for (i = 0; i < list->count(); i++)
	{
		GB.NewString(&str, TO_UTF8(list->item(i)->text()), 0);
		*((char **)GB.Array.Get(array, i)) = str;
	}
}


void CListBox::setAll(QListWidget *list, GB_ARRAY array)
{
	int i;
	
  list->clear();
  list->blockSignals(true);

	if (array)
	{
		for (i = 0; i < GB.Array.Count(array); i++)
		{
			list->addItem(TO_QSTRING(*((char **)GB.Array.Get(array, i))));
		}
	}

  list->blockSignals(false);
}


/*
int CListBox::currentItem(QListWidget *list)
{
  int mode = list->selectionMode();
  int index;

  if (mode == QAbstractItemView::NoSelection)
    return -1;

  index = list->currentRow();

  if (mode == QAbstractItemView::SingleSelection)
  {
    if (!list->isSelected(index))
      index = -1;
  }

  return index;
}

*/

