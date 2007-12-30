/***************************************************************************

  CContainer.cpp

  The Container class

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

#define __CCONTAINER_CPP

#include <qnamespace.h>
#include <qapplication.h>
#include <qmainwindow.h>
#include <qlayout.h>
#include <qevent.h>

#include "gambas.h"

#include "CWidget.h"
#include "CWindow.h"
#include "CConst.h"

#include "CContainer.h"

// #define DEBUG_ME

//DECLARE_EVENT(EVENT_Insert);
//DECLARE_EVENT(EVENT_Remove);
DECLARE_EVENT(EVENT_Arrange);

static QWidget *get_widget(QObjectList *list)
{
  for(;;)
  {
    QObject *ob = list->current();
    if (!ob)
      return NULL;
    list->next();
    if (ob->isWidgetType())
    {
      if (!((QWidget *)ob)->isHidden() && !((QWidget *)ob)->isA("QSizeGrip"))
        return (QWidget *)ob;
    }
  }
}

#define WIDGET_TYPE QWidget *
#define CONTAINER_TYPE QFrame *
#define ARRANGEMENT_TYPE CCONTAINER_ARRANGEMENT *

#define IS_RIGHT_TO_LEFT() qApp->reverseLayout()

#define GET_WIDGET(_object) ((CWIDGET *)_object)->widget
#define GET_CONTAINER(_object) ((CCONTAINER *)_object)->container
#define GET_ARRANGEMENT(_object) ((CCONTAINER_ARRANGEMENT *)_object)
#define IS_EXPAND(_object) (((CWIDGET *)_object)->flag.expand)
#define IS_IGNORE(_object) (((CWIDGET *)_object)->flag.ignore)
#define IS_DESIGN(_object) (CWIDGET_test_flag(_object, WF_DESIGN) && CWIDGET_test_flag(_object, WF_DESIGN_LEADER))

#define IS_WIDGET_VISIBLE(_widget) (_widget)->isVisible()
#define GET_WIDGET_CONTENTS(_widget, _x, _y, _w, _h) \
	_x = (_widget)->contentsRect().x(); \
	_y = (_widget)->contentsRect().y(); \
	_w = (_widget)->contentsRect().width(); \
	_h = (_widget)->contentsRect().height();
#define GET_WIDGET_X(_widget) (_widget)->x()
#define GET_WIDGET_Y(_widget) (_widget)->y()
#define GET_WIDGET_W(_widget) (_widget)->width()
#define GET_WIDGET_H(_widget) (_widget)->height()
#define MOVE_WIDGET(_widget, _x, _y) (_widget)->move(_x, _y)
#define RESIZE_WIDGET(_widget, _w, _h) (_widget)->resize(_w, _h)
#define MOVE_RESIZE_WIDGET(_widget, _x, _y, _w, _h) (_widget)->setGeometry(_x, _y, _w, _h)

#define INIT_CHECK_CHILDREN_LIST(_widget) \
  QObjectList *list = (QObjectList *)(_widget)->children(); \
  if (!list || list->count() == 0) \
    return;

#define RESET_CHILDREN_LIST() list->first()
#define GET_NEXT_CHILD_WIDGET() get_widget(list)

#define GET_OBJECT_FROM_WIDGET(_widget) CWidget::get(_widget)

#define RAISE_ARRANGE_EVENT(_object) GB.Raise(_object, EVENT_Arrange, 0); THIS_ARRANGEMENT->dirty = FALSE;

#define FUNCTION_NAME CCONTAINER_arrange

#include "gb.form.arrangement.h"


static int max_w, max_h;

static void move_widget(QWidget *wid, int x, int y)
{
	int w = x + wid->width();
	int h = y + wid->height();
	
	if (w > max_w) max_w = w;
	if (h > max_h) max_h = h;
}

static void move_resize_widget(QWidget *wid, int x, int y, int w, int h)
{
	w += x;
	h += y;

	if (w > max_w) max_w = w;
	if (h > max_h) max_h = h;
}

#undef MOVE_WIDGET
#define MOVE_WIDGET(_widget, _x, _y) move_widget(_widget, _x, _y)
#undef RESIZE_WIDGET
#define RESIZE_WIDGET(_widget, _w, _h) (0)
#undef MOVE_RESIZE_WIDGET
#define MOVE_RESIZE_WIDGET(_widget, _x, _y, _w, _h) move_resize_widget(_widget, _x, _y, _w, _h)
#undef RAISE_ARRANGE_EVENT
#define RAISE_ARRANGE_EVENT(_object) (0)
#undef FUNCTION_NAME
#define FUNCTION_NAME get_max_size
#undef IS_WIDGET_VISIBLE
#define IS_WIDGET_VISIBLE(_cont) (1)

#include "gb.form.arrangement.h"

void CCONTAINER_get_max_size(void *_object, int *w, int *h)
{
	bool locked = THIS_ARRANGEMENT->locked;
	THIS_ARRANGEMENT->locked = false;
	
	max_w = 0;
	max_h = 0;
	get_max_size(THIS);
	*w = max_w;
	*h = max_h;
	
	THIS_ARRANGEMENT->locked = locked;
}


#define arrange_later arrange_now
#define arrange_now(_widget) CCONTAINER_arrange(CWidget::get(_widget))

#if 0
static void post_arrange_later(void *_object)
{
  if (WIDGET && THIS_ARRANGEMENT->dirty)
    CCONTAINER_arrange(THIS);

  GB.Unref(&_object);
}

static void arrange_later(QWidget *cont)
{
  void *_object = CWidget::get(cont);

  if (THIS_ARRANGEMENT->dirty || THIS_ARRANGEMENT->mode == ARRANGE_NONE)
    return;

  GB.Ref(_object);
  //qDebug("later: %p: dirty = TRUE", THIS);
  THIS_ARRANGEMENT->dirty = TRUE;
  GB.Post((void (*)())post_arrange_later, (long)THIS);
}
#endif

/***************************************************************************

  class MyContainer

***************************************************************************/

MyContainer::MyContainer(QWidget *parent)
: QFrame(parent)
{
}


void MyContainer::frameChanged(void)
{
  //qDebug("MyContainer::frameChanged %p", CWidget::get(this));
  QFrame::frameChanged();
  //CCONTAINER_arrange(this);
  arrange_now(this);
}

void MyContainer::resizeEvent(QResizeEvent *e)
{
  //qDebug("MyContainer::resizeEvent %s %p", GB.GetClassName(CWidget::get(this)), CWidget::get(this));
  QFrame::resizeEvent(e);
  arrange_now(this);
}

void MyContainer::showEvent(QShowEvent *e)
{
  //qDebug("MyContainer::showEvent %p %s", CWidget::get(this), GB.GetClassName(CWidget::get(this)));
  QFrame::showEvent(e);
  arrange_now(this);
}

void MyContainer::childEvent(QChildEvent *e)
{
  //void *_object = CWidget::get(this);
  void *child;
  //qDebug("MyContainer::childEvent %p", CWidget::get(this));
  
  QFrame::childEvent(e);

  if (!e->child()->isWidgetType())
    return;

	child = CWidget::get((QWidget *)e->child());

  if (e->inserted())
  {
    e->child()->installEventFilter(this);
		//qApp->sendEvent(WIDGET, new QEvent(EVENT_INSERT));
    //if (THIS_ARRANGEMENT->user)
    //	GB.Raise(THIS, EVENT_Insert, 1, GB_T_OBJECT, child);    
  }
  else if (e->removed())
  {
    e->child()->removeEventFilter(this);
    //if (THIS_ARRANGEMENT->user)
    //	GB.Raise(THIS, EVENT_Remove, 1, GB_T_OBJECT, child);
  }

  arrange_later(this);
}

bool MyContainer::eventFilter(QObject *o, QEvent *e)
{
  int type = e->type();

  if (type == QEvent::Move || type == QEvent::Resize || type == QEvent::Show || type == QEvent::Hide || type == EVENT_EXPAND)
  {
  	CWIDGET *ob = CWidget::getReal(o);
  	if (ob && (type == EVENT_EXPAND || !ob->flag.ignore))
    	arrange_now(this);
  }

  return QObject::eventFilter(o, e);
}


/***************************************************************************

  CContainer

***************************************************************************/

static QRect getRect(void *_object)
{
	QWidget *w = CONTAINER;

	if (WIDGET->isA("MyMainWindow"))
		((MyMainWindow *)WIDGET)->configure();

	if (w->inherits("QFrame"))
		return ((QFrame *)w)->contentsRect();
	else
		return QRect(0, 0, w->width(), w->height());
}

BEGIN_METHOD_VOID(CCONTAINER_children_next)

  #ifdef DEBUG
  if (!CONTAINER)
    qDebug("Null container");
  #endif

  QObjectList *list = (QObjectList *)CONTAINER->children();
  unsigned int index;
  CWIDGET *widget;

  for(;;)
  {
    index = ENUM(int);

    if (list == NULL || index >= list->count())
    {
      GB.StopEnum();
      return;
    }

    ENUM(int) = index + 1;

    widget = CWidget::getReal(list->at(index));
    if (widget)
    {
      GB.ReturnObject(widget);
      return;
    }
  }

END_METHOD


BEGIN_PROPERTY(CCONTAINER_children_count)

  QWidget *wid = CONTAINER;
  QObjectList *list;
  QObject *ob;
  int n = 0;

  if (!wid)
  	goto __DONE;

	list = (QObjectList *)wid->children();
	if (!list || !list->count())
		goto __DONE;

	list->first();
  for(;;)
  {
    ob = list->current();
    if (!ob)
      break;
    list->next();
    if (ob->isWidgetType() && CWidget::getReal(ob))
    	n++;
  }

__DONE:
  GB.ReturnInteger(n);

END_PROPERTY


BEGIN_PROPERTY(CCONTAINER_x)

  #ifdef DEBUG
  if (!CONTAINER)
    qDebug("Null container");
  #endif

	QRect r = getRect(THIS); //_CONTAINER);
  QPoint p(r.x(), r.y());

  GB.ReturnInteger(CONTAINER->mapTo(WIDGET, p).x());

END_PROPERTY


BEGIN_PROPERTY(CCONTAINER_y)

  #ifdef DEBUG
  if (!CONTAINER)
    qDebug("Null container");
  #endif

	QRect r = getRect(THIS); // _CONTAINER);
  QPoint p(r.x(), r.y());

  GB.ReturnInteger(CONTAINER->mapTo(WIDGET, p).y());

END_PROPERTY


BEGIN_PROPERTY(CCONTAINER_width)

  #ifdef DEBUG
  if (!CONTAINER)
    qDebug("Null container");
  #endif

  GB.ReturnInteger(getRect(THIS).width());

END_PROPERTY


BEGIN_PROPERTY(CCONTAINER_height)

  #ifdef DEBUG
  if (!CONTAINER)
    qDebug("Null container");
  #endif

  GB.ReturnInteger(getRect(THIS).height());

END_PROPERTY


BEGIN_PROPERTY(CCONTAINER_arrangement)

  int arr;

  if (READ_PROPERTY)
    GB.ReturnInteger(THIS_ARRANGEMENT->mode);
  else
  {
    arr = VPROP(GB_INTEGER);
    if (arr < 0 || arr > 8)
      return;
    THIS_ARRANGEMENT->mode = arr;
    arrange_now(CONTAINER);
  }

END_PROPERTY

BEGIN_PROPERTY(CUSERCONTAINER_arrangement)

	CCONTAINER *cont = (CCONTAINER *)CWidget::get(CONTAINER);
	CCONTAINER_arrangement(cont, _param);
	if (!READ_PROPERTY)
	{
		THIS_USERCONTAINER->save = cont->arrangement;
	  //qDebug("(%s %p): save = %08X (arrangement %d)", GB.GetClassName(THIS), THIS, THIS_USERCONTAINER->save, val);
	}

END_PROPERTY


BEGIN_PROPERTY(CCONTAINER_auto_resize)

  if (READ_PROPERTY)
    GB.ReturnBoolean(THIS_ARRANGEMENT->autoresize);
  else
  {
    THIS_ARRANGEMENT->autoresize = VPROP(GB_BOOLEAN);
    arrange_now(CONTAINER);
  }

END_PROPERTY

BEGIN_PROPERTY(CUSERCONTAINER_auto_resize)

	CCONTAINER *cont = (CCONTAINER *)CWidget::get(CONTAINER);
	CCONTAINER_auto_resize(cont, _param);
	if (!READ_PROPERTY)
	{
		THIS_USERCONTAINER->save = cont->arrangement;
	  //qDebug("(%s %p): save = %08X (autoresize)", GB.GetClassName(THIS), THIS, THIS_USERCONTAINER->save);
	}

END_PROPERTY


BEGIN_PROPERTY(CCONTAINER_padding)

  if (READ_PROPERTY)
    GB.ReturnInteger(THIS_ARRANGEMENT->padding);
  else
  {
    int val = VPROP(GB_INTEGER);

    if (val >= 0 && val < 32768)
    {
      THIS_ARRANGEMENT->padding = val;
      arrange_now(CONTAINER);
    }
  }

END_PROPERTY

BEGIN_PROPERTY(CUSERCONTAINER_padding)

	CCONTAINER *cont = (CCONTAINER *)CWidget::get(CONTAINER);
	CCONTAINER_padding(cont, _param);
	if (!READ_PROPERTY)
	{
		THIS_USERCONTAINER->save = cont->arrangement;
	  //qDebug("(%s %p): save = %08X (padding)", GB.GetClassName(THIS), THIS, THIS_USERCONTAINER->save);
	}

END_PROPERTY


BEGIN_PROPERTY(CCONTAINER_spacing)

  if (READ_PROPERTY)
    GB.ReturnInteger(THIS_ARRANGEMENT->spacing);
  else
  {
    int val = VPROP(GB_INTEGER);

    if (val >= 0 && val < 32768)
    {
      THIS_ARRANGEMENT->spacing = val;
      arrange_now(CONTAINER);
    }
  }

END_PROPERTY

BEGIN_PROPERTY(CUSERCONTAINER_spacing)

	CCONTAINER *cont = (CCONTAINER *)CWidget::get(CONTAINER);
	CCONTAINER_spacing(cont, _param);
	if (!READ_PROPERTY)
	{
		THIS_USERCONTAINER->save = cont->arrangement;
	  //qDebug("(%s %p): save = %08X (spacing)", GB.GetClassName(THIS), THIS, THIS_USERCONTAINER->save);
	}

END_PROPERTY


BEGIN_METHOD(CUSERCONTROL_new, GB_OBJECT parent)

  MyContainer *wid = new MyContainer(QCONTAINER(VARG(parent)));

  CWIDGET_new(wid, (void *)_object);

  THIS->container = wid;
  THIS_ARRANGEMENT->mode = ARRANGE_FILL;
  THIS_ARRANGEMENT->user = true;

  wid->show();

END_METHOD


BEGIN_PROPERTY(CUSERCONTROL_container)

	CCONTAINER *current = (CCONTAINER *)CWidget::get(CONTAINER);

	if (READ_PROPERTY)
		GB.ReturnObject(current);
	else
	{
		CCONTAINER *cont = (CCONTAINER *)VPROP(GB_OBJECT);
		QWidget *w;
		QWidget *p;

		// sanity checks

		if (!cont)
		{
			THIS->container = WIDGET;
			return;
		}

		if (GB.CheckObject(cont))
			return;

		w = cont->container;
		if (w == THIS->container)
			return;

		for (p = w; p; p = p->parentWidget())
		{
			if (p == WIDGET)
				break;
		}

		if (!p)
			GB.Error("Container must be a child control");
		else
		{
			THIS->container = w;

			CWIDGET_update_design((CWIDGET *)THIS);
			CCONTAINER_arrange(THIS);
		}
	}

END_PROPERTY


BEGIN_PROPERTY(CUSERCONTAINER_container)

	//CCONTAINER *before;
	CCONTAINER *after;

	if (READ_PROPERTY)
		CUSERCONTROL_container(_object, _param);
	else
	{
		CUSERCONTROL_container(_object, _param);

		after = (CCONTAINER *)CWidget::get(THIS->container);
		after->arrangement = THIS_USERCONTAINER->save;
		//qDebug("(%s %p): arrangement = %08X", GB.GetClassName(THIS), THIS, after->arrangement);
		CCONTAINER_arrange(after);
	}

END_PROPERTY

DECLARE_METHOD(CCONTROL_design);

BEGIN_PROPERTY(CUSERCONTAINER_design)

	CCONTROL_design(_object, _param);
	
	if (!READ_PROPERTY && VPROP(GB_BOOLEAN))
	{
		CCONTAINER *cont = (CCONTAINER *)CWidget::get(CONTAINER);
		
    cont->arrangement = 0;
		THIS_USERCONTAINER->save = cont->arrangement;
	}

END_PROPERTY


BEGIN_METHOD(CCONTAINER_find, GB_INTEGER x; GB_INTEGER y)

	QWidget *w;
	void *control;
	
	w = CONTAINER->childAt(VARG(x), VARG(y));
	control = CWidget::get(w);
	if (control == THIS)
		control = NULL;
	
	GB.ReturnObject(control);

END_METHOD


#include "CContainer_desc.h"
