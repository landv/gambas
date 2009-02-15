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
#include "CScrollView.h"

#include "CContainer.h"

// #define DEBUG_ME

DECLARE_EVENT(EVENT_Insert);
//DECLARE_EVENT(EVENT_Remove);
DECLARE_EVENT(EVENT_BeforeArrange);
DECLARE_EVENT(EVENT_Arrange);

#if DEBUG_CONTAINER
static int _count_move, _count_resize, _count_set_geom;
#endif

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

#if 0
static void move_widget(QWidget *wid, int x, int y)
{
	if (wid->x() != x || wid->y() != y)
	{
		#if DEBUG_CONTAINER
		_count_move++;
		#endif
		wid->move(x, y);
	}
}

static void resize_widget(QWidget *wid, int w, int h)
{
	if (wid->width() != w || wid->height() != h)
	{
		#if DEBUG_CONTAINER
		_count_resize++;
		#endif
		wid->resize(w, h);
	}
}

static void move_resize_widget(QWidget *wid, int x, int y, int w, int h)
{
	if (wid->x() != x || wid->y() != y || wid->width() != w || wid->height() != h)
	{
		#if DEBUG_CONTAINER
		_count_set_geom++;
		#endif
		wid->setGeometry(x, y, w, h);
	}
}
#endif

static void resize_container(void *_object, QWidget *cont, int w, int h)
{
	QWidget *wid = ((CWIDGET *)_object)->widget;
	CWIDGET_resize(_object, w + wid->width() - cont->width(), h + wid->height() - cont->height());
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

#define CAN_ARRANGE(_object) ((_object) && (IS_WIDGET_VISIBLE(GET_CONTAINER(_object)) || IS_WIDGET_VISIBLE(GET_WIDGET(_object))))

#define GET_WIDGET_CONTENTS(_widget, _x, _y, _w, _h) \
	_x = (_widget)->contentsRect().x(); \
	_y = (_widget)->contentsRect().y(); \
	_w = (_widget)->contentsRect().width(); \
	_h = (_widget)->contentsRect().height();
#define GET_WIDGET_X(_widget) (_widget)->x()
#define GET_WIDGET_Y(_widget) (_widget)->y()
#define GET_WIDGET_W(_widget) (_widget)->width()
#define GET_WIDGET_H(_widget) (_widget)->height()
#define MOVE_WIDGET(_object, _widget, _x, _y) CWIDGET_move((_object), (_x), (_y))
#define RESIZE_WIDGET(_object, _widget, _w, _h) CWIDGET_resize((_object), (_w), (_h))
#define MOVE_RESIZE_WIDGET(_object, _widget, _x, _y, _w, _h) CWIDGET_move_resize((_object), (_x), (_y), (_w), (_h))
#define RESIZE_CONTAINER(_object, _cont, _w, _h) resize_container((_object), (_cont), (_w), (_h))

#define INIT_CHECK_CHILDREN_LIST(_widget) \
	QObjectList *list = (QObjectList *)(_widget)->children(); \
	if (!list || list->count() == 0) \
		return;

#define RESET_CHILDREN_LIST() list->first()
#define GET_NEXT_CHILD_WIDGET() get_widget(list)

#define GET_OBJECT_FROM_WIDGET(_widget) CWidget::getValid(CWidget::getReal(_widget))

#define GET_OBJECT_NAME(_object) (((CWIDGET *)_object)->name)

#define RAISE_ARRANGE_EVENT(_object) \
{ \
	GB.Raise(_object, EVENT_Arrange, 0); \
}

#define RAISE_BEFORE_ARRANGE_EVENT(_object) \
{ \
	GB.Raise(_object, EVENT_BeforeArrange, 0); \
}

#define DESKTOP_SCALE MAIN_scale

//THIS_ARRANGEMENT->dirty = FALSE;

#define FUNCTION_NAME CCONTAINER_arrange_real

#include "gb.form.arrangement.h"

void CCONTAINER_arrange(void *_object)
{
	#if DEBUG_CONTAINER
	static int level = 0;
	
	if (!level)
		_count_move = _count_resize = _count_set_geom = 0;
	level++;
	#endif

	CCONTAINER_arrange_real(_object);

	QWidget *cont = GET_CONTAINER(_object);
	if (cont->isA("MyContents"))
		((MyContents *)cont)->afterArrange();

	#if DEBUG_CONTAINER
	level--;
	if (!level)
	{
		if (_count_move || _count_resize || _count_set_geom)
			qDebug("CCONTAINER_arrange: (%s %s): move = %d  resize = %d  setGeometry = %d", GB.GetClassName(THIS), THIS->widget.name, _count_move, _count_resize, _count_set_geom);
	}
	#endif
}

static int max_w, max_h;

static void gms_move_widget(QWidget *wid, int x, int y)
{
	int w = x + wid->width();
	int h = y + wid->height();
	
	if (w > max_w) max_w = w;
	if (h > max_h) max_h = h;
}

static void gms_move_resize_widget(QWidget *wid, int x, int y, int w, int h)
{
	w += x;
	h += y;

	if (w > max_w) max_w = w;
	if (h > max_h) max_h = h;
}

#undef MOVE_WIDGET
#define MOVE_WIDGET(_object, _widget, _x, _y) gms_move_widget(_widget, _x, _y)
#undef RESIZE_WIDGET
#define RESIZE_WIDGET(_object, _widget, _w, _h) (0)
#undef MOVE_RESIZE_WIDGET
#define MOVE_RESIZE_WIDGET(_object, _widget, _x, _y, _w, _h) gms_move_resize_widget(_widget, _x, _y, _w, _h)
#undef RAISE_BEFORE_ARRANGE_EVENT
#define RAISE_BEFORE_ARRANGE_EVENT(_object) (0)
#undef RAISE_ARRANGE_EVENT
#define RAISE_ARRANGE_EVENT(_object) (0)
#undef FUNCTION_NAME
#define FUNCTION_NAME get_max_size
#undef RESIZE_CONTAINER
#define RESIZE_CONTAINER(_object, _cont, _w, _h) (0)
//#undef IS_WIDGET_VISIBLE
//#define IS_WIDGET_VISIBLE(_cont) (1)

#undef INIT_CHECK_CHILDREN_LIST
#define INIT_CHECK_CHILDREN_LIST(_widget) \
	QObjectList *list = (QObjectList *)(_widget)->children(); \
	if (!list || list->count() == 0) \
		return;


#include "gb.form.arrangement.h"

void CCONTAINER_get_max_size(void *_object, int *w, int *h)
{
	bool locked = THIS_ARRANGEMENT->locked;
	THIS_ARRANGEMENT->locked = false;
	
	max_w = 0;
	max_h = 0;
	get_max_size(THIS);
	//qDebug("get_max_size: %d %d", max_w, max_h);
	*w = max_w + THIS_ARRANGEMENT->padding + (THIS_ARRANGEMENT->margin ? MAIN_scale : 0);
	*h = max_h + THIS_ARRANGEMENT->padding + (THIS_ARRANGEMENT->margin ? MAIN_scale : 0);
	
	THIS_ARRANGEMENT->locked = locked;
}

void CCONTAINER_insert_child(void *child)
{
	void *_object = CWIDGET_get_parent(child);
	if (THIS)
		GB.Raise(THIS, EVENT_Insert, 1, GB_T_OBJECT, child);
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
	GB.Post((void (*)())post_arrange_later, (intptr_t)THIS);
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

void MyContainer::showEvent(QShowEvent *e)
{
	//qDebug("MyContainer::showEvent %p %s", CWidget::get(this), GB.GetClassName(CWidget::get(this)));
	QFrame::showEvent(e);
	arrange_now(this);
}

#if 0
void MyContainer::childEvent(QChildEvent *e)
{
	void *_object = CWidget::get(this);
	void *child;
	//qDebug("MyContainer::childEvent %p", CWidget::get(this));
	
	QFrame::childEvent(e);

	if (!e->child()->isWidgetType())
		return;

	child = CWidget::get((QWidget *)e->child());

	if (e->inserted())
	{
		e->child()->installEventFilter(this);
		GB.Raise(THIS, EVENT_Insert, 1, GB_T_OBJECT, child);
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
#endif

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

		widget = CWidget::getValid(CWidget::getReal(list->at(index)));
		if (widget)
		{
			GB.ReturnObject(widget);
			return;
		}
	}

END_METHOD


BEGIN_METHOD(CCONTAINER_children_get, GB_INTEGER index)

	QObjectList *list = (QObjectList *)CONTAINER->children();
	int index = VARG(index), i;
	CWIDGET *widget;

	if (index >= 0)
	{
		i = 0;
		for(i = 0; i < (int)list->count(); i++)
		{
			widget = CWidget::getValid(CWidget::getReal(list->at(i)));
			if (!widget)
				continue;
			if (index == 0)
			{
				GB.ReturnObject(widget);
				return;
			}
			index--;
		}
	}

	GB.Error(GB_ERR_BOUND);

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
		if (ob->isWidgetType() && CWidget::getValid(CWidget::getReal(ob)))
			n++;
	}

__DONE:
	GB.ReturnInteger(n);

END_PROPERTY


BEGIN_METHOD_VOID(CCONTAINER_children_clear)

	QWidget *wid = CONTAINER;
	QObjectList *list;
	QObject *ob;

	if (!wid)
		return;

	list = (QObjectList *)wid->children();
	if (!list || !list->count())
		return;

	list->first();
	for(;;)
	{
		ob = list->current();
		if (!ob)
			break;
		list->next();
		if (ob->isWidgetType())
			CWIDGET_destroy(CWidget::getReal(ob));
	}

END_METHOD


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


BEGIN_PROPERTY(CCONTAINER_margin)

	if (READ_PROPERTY)
		GB.ReturnBoolean(THIS_ARRANGEMENT->margin);
	else
	{
		bool val = VPROP(GB_BOOLEAN);
		if (val != THIS_ARRANGEMENT->margin)
		{
			THIS_ARRANGEMENT->margin = val;
			arrange_now(CONTAINER);
		}
	}

END_PROPERTY

BEGIN_PROPERTY(CUSERCONTAINER_margin)

	CCONTAINER *cont = (CCONTAINER *)CWidget::get(CONTAINER);
	CCONTAINER_margin(cont, _param);
	if (!READ_PROPERTY)
	{
		THIS_USERCONTAINER->save = cont->arrangement;
		//qDebug("(%s %p): save = %08X (padding)", GB.GetClassName(THIS), THIS, THIS_USERCONTAINER->save);
	}

END_PROPERTY


BEGIN_PROPERTY(CCONTAINER_spacing)

	if (READ_PROPERTY)
		GB.ReturnBoolean(THIS_ARRANGEMENT->spacing);
	else
	{
		THIS_ARRANGEMENT->spacing = VPROP(GB_BOOLEAN) ? MAIN_scale : 0;
		arrange_now(CONTAINER);
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


BEGIN_PROPERTY(CCONTAINER_padding)

	if (READ_PROPERTY)
		GB.ReturnInteger(THIS_ARRANGEMENT->padding);
	else
	{
		int val = VPROP(GB_INTEGER);
		if (val >= 0 && val < 256)
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
		//qDebug("(%s %p): save = %08X (spacing)", GB.GetClassName(THIS), THIS, THIS_USERCONTAINER->save);
	}

END_PROPERTY


BEGIN_METHOD(CUSERCONTROL_new, GB_OBJECT parent)

	MyContainer *wid = new MyContainer(QCONTAINER(VARG(parent)));

	THIS->container = wid;
	THIS_ARRANGEMENT->mode = ARRANGE_FILL;
	THIS_ARRANGEMENT->user = true;

	CWIDGET_new(wid, (void *)_object);

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


GB_DESC CChildrenDesc[] =
{
	GB_DECLARE(".ContainerChildren", sizeof(CCONTAINER)), GB_VIRTUAL_CLASS(),

	GB_METHOD("_next", "Control", CCONTAINER_children_next, NULL),
	GB_METHOD("_get", "Control", CCONTAINER_children_get, "(Index)i"),
	GB_PROPERTY_READ("Count", "i", CCONTAINER_children_count),
	GB_METHOD("Clear", NULL, CCONTAINER_children_clear, NULL),

	GB_END_DECLARE
};


GB_DESC CContainerDesc[] =
{
	GB_DECLARE("Container", sizeof(CCONTAINER)), GB_INHERITS("Control"),
	GB_NOT_CREATABLE(),

	GB_PROPERTY_SELF("Children", ".ContainerChildren"),

	GB_PROPERTY_READ("ClientX", "i", CCONTAINER_x),
	GB_PROPERTY_READ("ClientY", "i", CCONTAINER_y),
	GB_PROPERTY_READ("ClientW", "i", CCONTAINER_width),
	GB_PROPERTY_READ("ClientWidth", "i", CCONTAINER_width),
	GB_PROPERTY_READ("ClientH", "i", CCONTAINER_height),
	GB_PROPERTY_READ("ClientHeight", "i", CCONTAINER_height),
	
	GB_METHOD("Find", "Control", CCONTAINER_find, "(X)i(Y)i"),

	GB_EVENT("BeforeArrange", NULL, NULL, &EVENT_BeforeArrange),
	GB_EVENT("Arrange", NULL, NULL, &EVENT_Arrange),
	GB_EVENT("Insert", NULL, "(Control)Control", &EVENT_Insert),

	GB_END_DECLARE
};


GB_DESC CUserControlDesc[] =
{
	GB_DECLARE("UserControl", sizeof(CCONTAINER)), GB_INHERITS("Container"),
	GB_NOT_CREATABLE(),

	GB_METHOD("_new", NULL, CUSERCONTROL_new, "(Parent)Container;"),

	GB_PROPERTY("_Container", "Container", CUSERCONTROL_container),
	GB_PROPERTY("_AutoResize", "b", CCONTAINER_auto_resize),

	USERCONTROL_DESCRIPTION,
	
	GB_END_DECLARE
};


GB_DESC CUserContainerDesc[] =
{
	GB_DECLARE("UserContainer", sizeof(CUSERCONTAINER)), GB_INHERITS("Container"),
	GB_NOT_CREATABLE(),

	GB_METHOD("_new", NULL, CUSERCONTROL_new, "(Parent)Container;"),

	//GB_PROPERTY("Container", "Container", CUSERCONTAINER_container),
	GB_PROPERTY("_Container", "Container", CUSERCONTAINER_container),

	GB_PROPERTY("Arrangement", "i", CUSERCONTAINER_arrangement),
	GB_PROPERTY("AutoResize", "b", CUSERCONTAINER_auto_resize),
	GB_PROPERTY("Margin", "b", CUSERCONTAINER_margin),
	GB_PROPERTY("Spacing", "b", CUSERCONTAINER_spacing),
	GB_PROPERTY("Padding", "i", CUSERCONTAINER_padding),
	
	GB_PROPERTY("Design", "b", CUSERCONTAINER_design),

	USERCONTAINER_DESCRIPTION,
	
	GB_END_DECLARE
};

