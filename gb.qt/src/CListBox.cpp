/***************************************************************************

  CListBox.cpp

  The ListBox class

  (c) 2000-2005 Beno� Minisini <gambas@users.sourceforge.net>

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
#include <qlistbox.h>
#include <qpainter.h>

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


BEGIN_METHOD(CLISTBOX_new, GB_OBJECT parent)

  QListBox *wid = new MyListBox(QCONTAINER(VARG(parent)));

  QObject::connect(wid, SIGNAL(selectionChanged()), &CListBox::manager, SLOT(selected()));
  QObject::connect(wid, SIGNAL(selected(int)), &CListBox::manager, SLOT(activated(int)));
  QObject::connect(wid, SIGNAL(highlighted(int)), &CListBox::manager, SLOT(clicked(int)));
  //QObject::connect(wid, SIGNAL(clicked(QListBoxItem *)), &CListBox::manager, SLOT(clicked(QListBoxItem *)));

  CWIDGET_new(wid, (void *)_object, "ListBox");

  THIS->sorted = false;

  wid->show();

END_METHOD


BEGIN_METHOD_VOID(CLISTBOX_clear)

  WIDGET->clear();

END_METHOD


BEGIN_METHOD(CLISTBOX_get, GB_INTEGER index)

  long index = VARG(index);

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
BEGIN_METHOD(CLISTBOX_set, GB_STRING item; long index)

  long index = PARAM(index);
  long current;
  bool selected;

  if (index < 0 || index >= (long)WIDGET->count())
    return;

  selected = WIDGET->isSelected(PARAM(index));

  WIDGET->changeItem(QString(GB.ToZeroString(PARAM(item))), index);
  WIDGET->setSelected(index, selected);

  current = WIDGET->currentItem();
  WIDGET->setCurrentItem(current);

END_METHOD
*/

BEGIN_METHOD(CLISTBOX_add, GB_STRING item; GB_INTEGER pos)

  long pos = VARGOPT(pos, -1);

  //if (GB.IsMissing(2))
  //WIDGET->blockSignals(true);
  WIDGET->insertItem(QSTRING_ARG(item), pos);
  if (THIS->sorted)
    WIDGET->sort();
  //WIDGET->blockSignals(false);
  //else
  //{
    //QLISTBOX(_object)->insertItem(PIXMAP_get(GB.ToZeroString(PARAM(picture))),
    //  GB.ToZeroString(PARAM(item)), pos);
  //}

  //QLISTBOX(_object)->updateGeometry();

END_METHOD


/*
BEGIN_METHOD(CLISTBOX_add_custom, GB_INTEGER width; GB_INTEGER height; GB_STRING text; GB_INTEGER pos)

  long pos = VARGOPT(pos, -1);
  QString text(GB.ToZeroString(ARG(text)));

  WIDGET->insertItem(new MyListBoxItem(WIDGET, text, VARG(width), VARG(height)), pos);
  if (THIS->sorted)
    WIDGET->sort();

END_METHOD
*/

BEGIN_METHOD(CLISTBOX_remove, GB_INTEGER pos)

  //WIDGET->blockSignals(true);
  WIDGET->removeItem(VARG(pos));
  //WIDGET->blockSignals(false);

END_METHOD


BEGIN_PROPERTY(CLISTBOX_sorted)

  if (READ_PROPERTY)
    GB.ReturnBoolean(THIS->sorted);
  else
  {
    THIS->sorted = VPROP(GB_BOOLEAN);
    if (THIS->sorted)
      WIDGET->sort();
  }

END_METHOD


BEGIN_PROPERTY(CLISTBOX_count)

  GB.ReturnInteger(WIDGET->count());

END_PROPERTY


BEGIN_PROPERTY(CLISTBOX_index)

  long index;

  if (READ_PROPERTY)
    GB.ReturnInteger(CListBox::currentItem(WIDGET));
  else
  {
    index = VPROP(GB_INTEGER);
    if (index < 0 || index >= (long)WIDGET->count())
    {
      WIDGET->clearSelection();
      return;
    }

    //if (WIDGET->selectionMode() == QListBox::Multi || WIDGET->selectionMode() == QListBox::Extended)
    //{
      //WIDGET->clearSelection();
      //WIDGET->setSelected(index, true);
    //}

    WIDGET->setCurrentItem(index);

    if (WIDGET->selectionMode() == QListBox::Single)
      WIDGET->setSelected(WIDGET->currentItem(), true);

    WIDGET->ensureCurrentVisible();
  }

END_PROPERTY


BEGIN_PROPERTY(CLISTBOX_current)

  THIS->index = CListBox::currentItem(WIDGET);

  if (THIS->index < 0)
    GB.ReturnNull();
  else
    RETURN_SELF();

END_PROPERTY


BEGIN_PROPERTY(CLISTBOX_text)

  int index = CListBox::currentItem(WIDGET);

  if (index < 0)
    GB.ReturnNull();
  else
    GB.ReturnNewZeroString(TO_UTF8(WIDGET->text(index)));

END_PROPERTY


BEGIN_PROPERTY(CLISTBOX_mode)

  if (READ_PROPERTY)
    GB.ReturnInteger(
    	CCONST_convert(WIDGET->selectionMode(), 3, 
    		QListBox::NoSelection, SELECT_NONE, 
    		QListBox::Single, SELECT_SINGLE, 
    		QListBox::Extended, SELECT_MULTIPLE, 
    		SELECT_NONE));
  else
    WIDGET->setSelectionMode((QListBox::SelectionMode)
    	CCONST_convert(VPROP(GB_INTEGER), -3, 
    		QListBox::NoSelection, SELECT_NONE, 
    		QListBox::Single, SELECT_SINGLE, 
    		QListBox::Extended, SELECT_MULTIPLE, 
    		QListBox::NoSelection));

END_PROPERTY


BEGIN_PROPERTY(CLISTBOX_item_selected)

  if (READ_PROPERTY)
    GB.ReturnBoolean(WIDGET->isSelected(THIS->index));
  else
    WIDGET->setSelected(THIS->index, VPROP(GB_BOOLEAN));

END_PROPERTY


BEGIN_PROPERTY(CLISTBOX_item_text)

  bool selected;
  int current;

  if (READ_PROPERTY)
    GB.ReturnNewZeroString(TO_UTF8(WIDGET->text(THIS->index)));
  else
  {
    current = WIDGET->currentItem();
    WIDGET->blockSignals(true);

    selected = WIDGET->isSelected(THIS->index);

    WIDGET->changeItem(QSTRING_PROP(), THIS->index);

    if (selected)
      WIDGET->setSelected(THIS->index, selected);

    WIDGET->setCurrentItem(current);

    WIDGET->blockSignals(false);
  }

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
    if (THIS->sorted)
      WIDGET->sort();
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
  //GB_CONSTANT("Extended", "i", QListBox::Extended),

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

  GB_CONSTANT("_Properties", "s", CLISTBOX_PROPERTIES),
  GB_CONSTANT("_DefaultEvent", "s", "Click"),

  GB_EVENT("Select", NULL, NULL, &EVENT_Select),
  GB_EVENT("Activate", NULL, NULL, &EVENT_Activate),
  GB_EVENT("Click", NULL, NULL, &EVENT_Click),
  //GB_EVENT("Draw", NULL, "(Index)i", &EVENT_Draw),

  GB_END_DECLARE
};


/** MyListBox ***************************************************************/

MyListBox::MyListBox(QWidget *parent) :
  QListBox(parent)
{
}

void MyListBox::mousePressEvent( QMouseEvent *e )
{
  if (e->button() == LeftButton)
    QListBox::mousePressEvent(e);
}

void MyListBox::resizeEvent( QResizeEvent *e )
{
  //qDebug("MyListBox::resizeEvent");
  QScrollView::resizeEvent(e);
  triggerUpdate(true);
  doLayout();
  viewport()->repaint( FALSE );
  ensureCurrentVisible();
}


#if 0
/** MyListBoxItem ***********************************************************/

MyListBoxItem::MyListBoxItem(QListBox *listbox, QString& text, int width, int height)
  :QListBoxItem(listbox)
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

int MyListBoxItem::height( const QListBox* lb ) const
{
  return QMAX(h, QApplication::globalStrut().height());
}

int MyListBoxItem::width( const QListBox* lb ) const
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
	if (WIDGET->selectionMode() == QListBox::Single)
		GB.Raise(_object, EVENT_Select, 0);
	else
	{
		GB.Ref(THIS);
		GB.Post((GB_POST_FUNC)post_select_event, (long)THIS);
	}
  //RAISE_EVENT(EVENT_Select);
}

void CListBox::activated(int index)
{
  RAISE_EVENT(EVENT_Activate);
}

void CListBox::clicked(int index)
{
  GET_SENDER(_object);

  if (currentItem(WIDGET) < 0)
    return;

  GB.Raise(_object, EVENT_Click, 0);
}

#if 0
void CListBox::clicked(QListBoxItem *item)
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

int CListBox::find(QListBox *list, const QString& s)
{
  for (int i = 0; i < (int)list->count(); i++)
  {
    if (list->text(i) == s)
      return i;
  }

  return (-1);
}


void CListBox::getAll(QListBox *list, GB_ARRAY array)
{
	uint i;
	char *str;
	
	for (i = 0; i < list->count(); i++)
	{
		GB.NewString(&str, TO_UTF8(list->text(i)), 0);
		*((char **)GB.Array.Get(array, i)) = str;
	}
}


void CListBox::setAll(QListBox *list, GB_ARRAY array)
{
	int i;
	
  list->clear();
  list->blockSignals(true);

	if (array)
	{
		for (i = 0; i < GB.Array.Count(array); i++)
		{
			list->insertItem(TO_QSTRING(*((char **)GB.Array.Get(array, i))));
		}
	}

  list->blockSignals(false);
}


int CListBox::currentItem(QListBox *list)
{
  int mode = list->selectionMode();
  int index;

  if (mode == QListBox::NoSelection)
    return -1;

  index = list->currentItem();

  if (mode == QListBox::Single)
  {
    if (!list->isSelected(index))
      index = -1;
  }

  return index;
}

