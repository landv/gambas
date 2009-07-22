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

#include <QApplication>
#include <QLayout>
#include <QEvent>
#include <QShowEvent>
#include <QResizeEvent>
#include <QChildEvent>
#include <QFrame>
#include <QHash>

#include "gambas.h"
#include "gb_common.h"

#include "CWidget.h"
#include "CWindow.h"
#include "CConst.h"
#include "CScrollView.h"

#include "CContainer.h"

//#define DEBUG_ME
//#define USE_CACHE 1

DECLARE_EVENT(EVENT_Insert);
//DECLARE_EVENT(EVENT_Remove);
DECLARE_EVENT(EVENT_BeforeArrange);
DECLARE_EVENT(EVENT_Arrange);

#if 0
static int _count_move, _count_resize, _count_set_geom;

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

#if USE_CACHE
static QHash<QWidget *, QRect *> _cache;
static int _cache_level = 0;
#endif

static QWidget *get_next_widget(QObjectList &list, int &index)
{
	QObject *ob;
	
	for(;;)
	{
		if (index >= list.count())
			return NULL;
	
		ob = list.at(index); // ob might be null if we are inside the QWidget destructor
		index++;
		
		if (ob && ob->isWidgetType())
		{
			QWidget *w = (QWidget *)ob;
			if (!w->isHidden() && !qobject_cast<QSizeGrip *>(w))
				return w;
		}
	}
}

#if USE_CACHE

static QRect *ensure_widget_geometry(QWidget *w)
{
	if (_cache.contains(w))
		return _cache.value(w);
	else
	{
		QRect *r = new QRect(w->geometry());
		_cache.insert(w, r);
		return r;
	}
}

static QRect *get_widget_geometry(QWidget *w)
{
	static QRect wg;
	
	if (_cache.contains(w))
		return _cache.value(w);
	else
	{
		wg = w->geometry();
		return &wg;
	}
}

static void move_widget(void *_object, int x, int y)
{
	QRect *r = ensure_widget_geometry(WIDGET);
	if (x == r->x() && y == r->y())
		return;
	r->setX(x);
	r->setY(y);
	CWIDGET_move_cached(THIS, x, y);
}

static void resize_widget(void *_object, int w, int h)
{
	QRect *r = ensure_widget_geometry(WIDGET);
	if (w == r->width() && h == r->height())
		return;
	r->setWidth(w);
	r->setHeight(h);
	CWIDGET_resize_cached(THIS, w, h);
}

static void move_resize_widget(void *_object, int x, int y, int w, int h)
{
	QRect *r = ensure_widget_geometry(WIDGET);
	if (x == r->x() && y == r->y() && w == r->width() && h == r->height())
		return;
	r->setRect(x, y, w, h);
	CWIDGET_move_resize_cached(THIS, x, y, w, h);
}

static void get_widget_contents(QWidget *wid, int &x, int &y, int &w, int &h)
{
	QRect wc = wid->contentsRect();
	QRect wg = wid->geometry();
	QRect *g = get_widget_geometry(wid);
	
	x = wc.x();
	y = wc.y();
	w = wc.width() + g->width() - wg.width();
	h = wc.height() + g->height() - wg.height();
}

static void flush_cache()
{
	QHash<QWidget *, QRect *> hash;
	QWidget *w;
	QRect *r;
	
	hash = _cache;
	_cache.clear();
	
	QHashIterator<QWidget *, QRect *> it(_cache);
	while (it.hasNext()) 
	{
		it.next();
		w = it.key();
		r = it.value();
		w->setGeometry(*r);
		delete r;
		//CWIDGET_move_resize(CWidget::getReal(w), r->x(), r->y(), r->width(), r->height());
	}
	
	_cache.clear();
}


#endif

static void resize_container(void *_object, QWidget *cont, int w, int h)
{
	QWidget *wid = ((CWIDGET *)_object)->widget;
	#if USE_CACHE
	resize_widget(_object, w + wid->width() - cont->width(), h + wid->height() - cont->height());
	#else
	CWIDGET_resize(_object, w + wid->width() - cont->width(), h + wid->height() - cont->height());
	#endif
}


#define WIDGET_TYPE QWidget *
#define CONTAINER_TYPE QWidget *
#define ARRANGEMENT_TYPE CCONTAINER_ARRANGEMENT *

#define IS_RIGHT_TO_LEFT() qApp->isRightToLeft()

#define GET_WIDGET(_object) ((CWIDGET *)_object)->widget
#define GET_CONTAINER(_object) ((CCONTAINER *)_object)->container
#define GET_ARRANGEMENT(_object) ((CCONTAINER_ARRANGEMENT *)_object)
#define IS_EXPAND(_object) (((CWIDGET *)_object)->flag.expand)
#define IS_IGNORE(_object) (((CWIDGET *)_object)->flag.ignore)
#define IS_DESIGN(_object) (CWIDGET_test_flag(_object, WF_DESIGN) && CWIDGET_test_flag(_object, WF_DESIGN_LEADER))
#define IS_WIDGET_VISIBLE(_widget) (_widget)->isVisible()

#define CAN_ARRANGE(_object) ((_object) && ((CWIDGET *)(_object))->flag.shown && !CWIDGET_test_flag(_object, WF_DELETED))
//(IS_WIDGET_VISIBLE(GET_CONTAINER(_object)) || IS_WIDGET_VISIBLE(GET_WIDGET(_object))))

#if USE_CACHE

#define GET_WIDGET_CONTENTS(_widget, _x, _y, _w, _h) get_widget_contents(_widget, _x, _y, _w, _h)
#define GET_WIDGET_X(_widget) get_widget_geometry(_widget)->x()
#define GET_WIDGET_Y(_widget) get_widget_geometry(_widget)->y()
#define GET_WIDGET_W(_widget) get_widget_geometry(_widget)->width()
#define GET_WIDGET_H(_widget) get_widget_geometry(_widget)->height()
#define MOVE_WIDGET(_object, _widget, _x, _y) move_widget((_object), (_x), (_y))
#define RESIZE_WIDGET(_object, _widget, _w, _h) resize_widget((_object), (_w), (_h))
#define MOVE_RESIZE_WIDGET(_object, _widget, _x, _y, _w, _h) move_resize_widget((_object), (_x), (_y), (_w), (_h))
#define RESIZE_CONTAINER(_object, _cont, _w, _h) resize_container((_object), (_cont), (_w), (_h))

#else

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

#endif

#define INIT_CHECK_CHILDREN_LIST(_widget) \
	QObjectList list = (_widget)->children(); \
	int list_index = 0; \
	if (list.count() == 0) \
		return;

#define RESET_CHILDREN_LIST() list_index = 0
#define GET_NEXT_CHILD_WIDGET() get_next_widget(list, list_index)

#define GET_OBJECT_FROM_WIDGET(_widget) CWidget::getRealExisting(_widget)

#define RAISE_ARRANGE_EVENT(_object) \
{ \
	GB.Raise(_object, EVENT_Arrange, 0); \
}

#define RAISE_BEFORE_ARRANGE_EVENT(_object) \
{ \
	GB.Raise(_object, EVENT_BeforeArrange, 0); \
}

#define DESKTOP_SCALE MAIN_scale

#define FUNCTION_NAME CCONTAINER_arrange_real

#include "gb.form.arrangement.h"

void CCONTAINER_arrange(void *_object)
{
	if (GB.Is(THIS, CLASS_ScrollView))
		CSCROLLVIEW_arrange(THIS);

	#if DEBUG_CONTAINER
	static int level = 0;
	
	if (!level)
		_count_move = _count_resize = _count_set_geom = 0;
	level++;
	#endif

	#if USE_CACHE
	qDebug("CCONTAINER_arrange: %s %p", GB.GetClassName(THIS), THIS->widget.widget);
	_cache_level++;
	#endif

	CCONTAINER_arrange_real(_object);
	
	#if USE_CACHE
	_cache_level--;
	
	if (!_cache_level)
	{
		flush_cache();
	}
	#endif

	//QWidget *cont = GET_CONTAINER(_object);
	//if (cont->isA("MyContents"))
	//	((MyContents *)cont)->afterArrange();

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

#include "gb.form.arrangement.h"

void CCONTAINER_get_max_size(void *_object, int *w, int *h)
{
	bool locked = THIS_ARRANGEMENT->locked;
	THIS_ARRANGEMENT->locked = false;
	
	max_w = 0;
	max_h = 0;
	get_max_size(THIS);
	*w = max_w + THIS_ARRANGEMENT->padding + (THIS_ARRANGEMENT->margin ? MAIN_scale : 0);
	*h = max_h + THIS_ARRANGEMENT->padding + (THIS_ARRANGEMENT->margin ? MAIN_scale : 0);
	
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
	GB.Post((void (*)())post_arrange_later, (intptr_t)THIS);
}
#endif

void CCONTAINER_insert_child(void *_object)
{
	CWIDGET *parent = CWidget::get(WIDGET->parentWidget());
	if (parent)
		GB.Raise(parent, EVENT_Insert, 1, GB_T_OBJECT, THIS);
}

/***************************************************************************

	class MyContainer

***************************************************************************/

MyContainer::MyContainer(QWidget *parent)
: QWidget(parent),_frame(0)
{
}

void MyContainer::showEvent(QShowEvent *e)
{
	void *_object = CWidget::get(this);
	QWidget::showEvent(e);
	THIS->widget.flag.shown = TRUE;
	CCONTAINER_arrange(THIS);
	//if (!qstrcmp(GB.GetClassName(THIS), "ListContainer"))
	//	qDebug("MyContainer::showEvent: %s %p: SHOWN = 1 (%d %d)", THIS->widget.name, THIS, THIS->widget.widget->isVisible() , !THIS->widget.widget->isHidden());
}

void MyContainer::hideEvent(QHideEvent *e)
{
	void *_object = CWidget::get(this);
	QWidget::hideEvent(e);
	THIS->widget.flag.shown = FALSE;
	/*if (!qstrcmp(GB.GetClassName(THIS), "ListContainer"))
	{
		qDebug("MyContainer::hideEvent: %s %p: SHOWN = 0", THIS->widget.name, THIS);
		//BREAKPOINT();
	}*/
}

void MyContainer::setFrameStyle(int frame)
{
	int margin;
	_frame = frame;
	
	if (_frame == BORDER_NONE)
		margin = 0;
	else if (_frame == BORDER_PLAIN)
		margin = 1;
	else
		margin = 2;
	
	setContentsMargins(margin, margin, margin, margin);
	update();
}

void CCONTAINER_draw_frame(QPainter *p, int frame, QStyleOptionFrame &opt, QWidget *w)
{
	QStyle *style;
	
	if (frame == 0)
		return;
	
	if (w)
		style = w->style();
	else
		style = QApplication::style();
	
	switch (frame)
	{
		case BORDER_PLAIN:
			qDrawPlainRect(p, opt.rect, opt.palette.foreground().color(), 1);
			break;
			
		case BORDER_SUNKEN:
			opt.lineWidth = 2;
			opt.midLineWidth = 2;
			opt.state |= QStyle::State_Sunken;
			style->drawPrimitive(QStyle::PE_Frame, &opt, p, w);
			break;
			
		case BORDER_RAISED:
			opt.lineWidth = 2;
			opt.midLineWidth = 2;
			opt.state |= QStyle::State_Raised;
			style->drawPrimitive(QStyle::PE_Frame, &opt, p, w);
			break;
			
		case BORDER_ETCHED:
			qDrawShadeRect(p, opt.rect, opt.palette, true, 1, 0);
			break;
			
		default:
			break;
	}
}

void MyContainer::drawFrame(QPainter *p)
{
	QStyleOptionFrame opt;
	opt.init(this);
	opt.rect = QRect(0, 0, width(), height());

	CCONTAINER_draw_frame(p, _frame, opt, this);
}

#if 0
	Q_D(QFrame);
	QPoint      p1, p2;
	QStyleOptionFrame opt;
	opt.init(this);
	int frameShape  = d->frameStyle & QFrame::Shape_Mask;
	int frameShadow = d->frameStyle & QFrame::Shadow_Mask;

	int lw = 0;
	int mlw = 0;
	opt.rect = frameRect();
	switch (frameShape) {
	case QFrame::Box:
	case QFrame::HLine:
	case QFrame::VLine:
	case QFrame::StyledPanel:
			lw = d->lineWidth;
			mlw = d->midLineWidth;
			break;
	default:
			// most frame styles do not handle customized line and midline widths
			// (see updateFrameWidth()).
			lw = d->frameWidth;
			break;
	}
	opt.lineWidth = lw;
	opt.midLineWidth = mlw;
	if (frameShadow == Sunken)
			opt.state |= QStyle::State_Sunken;
	else if (frameShadow == Raised)
			opt.state |= QStyle::State_Raised;

	switch (frameShape) {
	case Box:
			if (frameShadow == Plain)
					qDrawPlainRect(p, opt.rect, opt.palette.foreground().color(), lw);
			else
					qDrawShadeRect(p, opt.rect, opt.palette, frameShadow == Sunken, lw, mlw);
			break;


	case StyledPanel:
			style()->drawPrimitive(QStyle::PE_Frame, &opt, p, this);
			break;

	case Panel:
			if (frameShadow == Plain)
					qDrawPlainRect(p, opt.rect, opt.palette.foreground().color(), lw);
			else
					qDrawShadePanel(p, opt.rect, opt.palette, frameShadow == Sunken, lw);
			break;

	case WinPanel:
			if (frameShadow == Plain)
					qDrawPlainRect(p, opt.rect, opt.palette.foreground().color(), lw);
			else
					qDrawWinPanel(p, opt.rect, opt.palette, frameShadow == Sunken);
			break;
	case HLine:
	case VLine:
			if (frameShape == HLine) {
					p1 = QPoint(opt.rect.x(), opt.rect.height() / 2);
					p2 = QPoint(opt.rect.x() + opt.rect.width(), p1.y());
			} else {
					p1 = QPoint(opt.rect.x()+opt.rect.width() / 2, 0);
					p2 = QPoint(p1.x(), opt.rect.height());
			}
			if (frameShadow == Plain) {
					QPen oldPen = p->pen();
					p->setPen(QPen(opt.palette.foreground().color(), lw));
					p->drawLine(p1, p2);
					p->setPen(oldPen);
			} else {
					qDrawShadeLine(p, p1, p2, opt.palette, frameShadow == Sunken, lw, mlw);
			}
			break;
	}
}
#endif

void MyContainer::paintEvent(QPaintEvent *)
{
	QPainter paint(this);
	drawFrame(&paint);
}

/*void MyContainer::childEvent(QChildEvent *e)
{
	//void *_object = CWidget::get(this);
	void *child;
	//qDebug("MyContainer::childEvent %p", CWidget::get(this));
	
	QFrame::childEvent(e);

	if (!e->child()->isWidgetType())
		return;

	child = CWidget::get((QWidget *)e->child());

	if (e->added())
	{
		//e->child()->installEventFilter(this);
		//qApp->sendEvent(WIDGET, new QEvent(EVENT_INSERT));
		//if (THIS_ARRANGEMENT->user)
		//	GB.Raise(THIS, EVENT_Insert, 1, GB_T_OBJECT, child);    
	}
	else if (e->removed())
	{
		//e->child()->removeEventFilter(this);
		//if (THIS_ARRANGEMENT->user)
		//	GB.Raise(THIS, EVENT_Remove, 1, GB_T_OBJECT, child);
	}

	arrange_later(this);
}*/

/*bool MyContainer::eventFilter(QObject *o, QEvent *e)
{
	int type = e->type();

	if (type == QEvent::Move || type == QEvent::Resize || type == QEvent::Show || type == QEvent::Hide || type == EVENT_EXPAND)
	{
		CWIDGET *ob = CWidget::getReal(o);
		if (ob && (type == EVENT_EXPAND || !ob->flag.ignore))
			arrange_now(this);
	}

	return QObject::eventFilter(o, e);
}*/


/***************************************************************************

	CContainer

***************************************************************************/

static QRect getRect(void *_object)
{
	QWidget *w = CONTAINER;

	if (qobject_cast<MyMainWindow *>(WIDGET))
		((MyMainWindow *)WIDGET)->configure();

	return w->contentsRect();
}

BEGIN_METHOD_VOID(CCONTAINER_children_next)

	#ifdef DEBUG
	if (!CONTAINER)
		qDebug("Null container");
	#endif

	QObjectList list = CONTAINER->children();
	int index;
	CWIDGET *widget;

	for(;;)
	{
		index = ENUM(int);

		if (index >= list.count())
		{
			GB.StopEnum();
			return;
		}

		ENUM(int) = index + 1;

		widget = CWidget::getRealExisting(list.at(index));
		if (widget)
		{
			GB.ReturnObject(widget);
			return;
		}
	}

END_METHOD


BEGIN_METHOD(CCONTAINER_children_get, GB_INTEGER index)

	QObjectList list = CONTAINER->children();
	int index = VARG(index);
	int i;
	CWIDGET *widget;

	if (index >= 0)
	{
		i = 0;
		for(i = 0; i < list.count(); i++)
		{
			widget = CWidget::getRealExisting(list.at(i));
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
	QObjectList list;
	QObject *ob;
	int n = 0;
	int i;

	if (wid)
	{
		list = wid->children();
	
		for(i = 0; i < list.count(); i++)
		{
			ob = list.at(i);
			if (ob->isWidgetType() && CWidget::getRealExisting(ob))
				n++;
		}
	}

	GB.ReturnInteger(n);

END_PROPERTY


BEGIN_METHOD_VOID(CCONTAINER_children_clear)

	QWidget *wid = CONTAINER;
	QObjectList list;
	QObject *ob;
	int i;

	if (!wid)
		return;

	list = wid->children();

	for(i = 0; i < list.count(); i++)
	{
		ob = list.at(i);
		if (ob->isWidgetType())
			CWIDGET_destroy(CWidget::getRealExisting(ob));
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

BEGIN_PROPERTY(CCONTAINER_border)

	MyContainer *w = qobject_cast<MyContainer *>(THIS->container);
	
	if (!w)
		return;

	if (READ_PROPERTY)
		GB.ReturnInteger(w->frameStyle());
	else
		w->setFrameStyle(VPROP(GB_INTEGER));

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
  	if (val != THIS_ARRANGEMENT->padding && val >= 0 && val <= 255)
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

BEGIN_PROPERTY(CCONTAINER_indent)

  if (READ_PROPERTY)
    GB.ReturnInteger(THIS_ARRANGEMENT->indent);
  else
  {
  	int val = VPROP(GB_INTEGER);
		if (val < 0) val = 1;
  	if (val != THIS_ARRANGEMENT->indent && val >= 0 && val <= 7)
  	{
    	THIS_ARRANGEMENT->indent = val;
			arrange_now(CONTAINER);
		}
  }

END_PROPERTY

BEGIN_PROPERTY(CUSERCONTAINER_indent)

	CCONTAINER *cont = (CCONTAINER *)CWidget::get(CONTAINER);
	CCONTAINER_indent(cont, _param);
	if (!READ_PROPERTY)
	{
		THIS_USERCONTAINER->save = cont->arrangement;
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
		((CCONTAINER_ARRANGEMENT *)cont)->user = true;
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
	GB_PROPERTY("Padding", "i", CUSERCONTAINER_padding),
	GB_PROPERTY("Spacing", "b", CUSERCONTAINER_spacing),
	GB_PROPERTY("Margin", "b", CUSERCONTAINER_margin),
	GB_PROPERTY("Indent", "b", CUSERCONTAINER_indent),
	
	GB_PROPERTY("Design", "b", CUSERCONTAINER_design),

	USERCONTAINER_DESCRIPTION,
	
	GB_END_DECLARE
};


