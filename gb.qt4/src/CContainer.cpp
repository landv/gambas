/***************************************************************************

  CContainer.cpp

  (c) 2000-2013 Benoît Minisini <gambas@users.sourceforge.net>

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

#define __CCONTAINER_CPP

#include <QApplication>
#include <QLayout>
#include <QEvent>
#include <QShowEvent>
#include <QResizeEvent>
#include <QChildEvent>
#include <QFrame>
#include <QHash>
#include <QStyleOptionFrameV3>
#include <QGroupBox>

#include "gambas.h"
#include "gb_common.h"

#include "CWidget.h"
#include "CWindow.h"
#include "CConst.h"
#include "CScrollView.h"
#include "CTabStrip.h"
#include "CColor.h"

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

#else
#if 0

static void get_widget_contents(QWidget *wid, int &x, int &y, int &w, int &h)
{
	QRect wc;
	
	wc = wid->contentsRect();
	
	x = wc.x();
	y = wc.y();
	w = wc.width();
	h = wc.height();
	
}
#endif
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

//#define CAN_ARRANGE(_object) ((_object) && !CWIDGET_test_flag(_object, WF_DELETED) && (!GB.Is(_object, CLASS_Window) || (((CWINDOW *)_object)->opened)))
#define CAN_ARRANGE(_object) ((_object) && ((CWIDGET *)(_object))->flag.shown && !CWIDGET_test_flag(_object, WF_DELETED))

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

//#define GET_WIDGET_CONTENTS(_widget, _x, _y, _w, _h) get_widget_contents(_widget, _x, _y, _w, _h)
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
	int list_index = 0;
	
#define HAS_CHILDREN() (list.count() != 0)

#define RESET_CHILDREN_LIST() list_index = 0
#define GET_NEXT_CHILD_WIDGET() get_next_widget(list, list_index)

#define GET_OBJECT_FROM_WIDGET(_widget) CWidget::getRealExisting(_widget)

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

	#if USE_CACHE
	qDebug("CCONTAINER_arrange: %s %p", GB.GetClassName(THIS), THIS->widget.widget);
	_cache_level++;
	#endif

	if (GB.Is(THIS, CLASS_TabStrip))
		CTABSTRIP_arrange(THIS);
	else if (GB.Is(THIS, CLASS_ScrollView))
		CSCROLLVIEW_arrange(THIS);

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
static int _gms_x, _gms_y, _gms_w, _gms_h;

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
#undef GET_WIDGET_CONTENTS
#define GET_WIDGET_CONTENTS(_widget, _x, _y, _w, _h) \
	_x = _gms_x; \
	_y = _gms_y; \
	_w = _gms_w; \
	_h = _gms_h;

//#undef IS_WIDGET_VISIBLE
//#define IS_WIDGET_VISIBLE(_cont) (1)
#define GET_MAX_SIZE

#include "gb.form.arrangement.h"

void CCONTAINER_get_max_size(void *_object, int xc, int yc, int wc, int hc, int *w, int *h)
{
	int add;
	CCONTAINER_ARRANGEMENT *arr = THIS_ARRANGEMENT;
	bool locked;

	locked = arr->locked;
	arr->locked = false;
	
	max_w = 0;
	max_h = 0;
	_gms_x = xc;
	_gms_y = yc;
	_gms_w = wc;
	_gms_h = hc;
	get_max_size(THIS);

	if (arr->margin)
		add = arr->padding ? arr->padding : MAIN_scale;
	else if (!arr->spacing)
		add = arr->padding;
	else
		add = 0;

	*w = max_w + add;
	*h = max_h + add;
	
	arr->locked = locked;
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

void CCONTAINER_decide(CWIDGET *control, bool *width, bool *height)
{
	void *_object = CWIDGET_get_parent(control);

	*width = *height = FALSE;
	
	if (!THIS || control->flag.ignore || THIS_ARRANGEMENT->autoresize)
		return;
	
	if ((THIS_ARRANGEMENT->mode == ARRANGE_VERTICAL)
	    || (THIS_ARRANGEMENT->mode == ARRANGE_HORIZONTAL && control->flag.expand)
	    || (THIS_ARRANGEMENT->mode == ARRANGE_ROW && control->flag.expand))
		*width = TRUE;
	
	if ((THIS_ARRANGEMENT->mode == ARRANGE_HORIZONTAL)
	    || (THIS_ARRANGEMENT->mode == ARRANGE_VERTICAL && control->flag.expand)
	    || (THIS_ARRANGEMENT->mode == ARRANGE_COLUMN && control->flag.expand))
		*height = TRUE;
}


/***************************************************************************

	class MyFrame

***************************************************************************/

MyFrame::MyFrame(QWidget *parent)
: QWidget(parent),_frame(0),_pixmap(0),_bg(false)
{
}

void MyFrame::setStaticContents(bool on)
{
	void *_object = CWidget::get(this);
	setAttribute(Qt::WA_StaticContents, on && _frame == BORDER_NONE && (_pixmap || THIS->widget.flag.fillBackground));
}

void MyFrame::setFrameStyle(int frame)
{
	int margin;

	_frame = frame;

	setStaticContents(true);
	
	margin = frameWidth();
	setContentsMargins(margin, margin, margin, margin);
	
	update();
}

static void _draw_border(QPainter *p, int frame, QWidget *w, QStyleOptionFrame &opt)
{
	QStyle *style;
	QStyleOptionFrameV3 optv3;
	bool a;
	QBrush save_brush;
	//QRect rect = opt.rect;
	
	if (frame == BORDER_NONE)
		return;
	
	if (w)
		style = w->style();
	else
		style = QApplication::style();

	switch (frame)
	{
		case BORDER_PLAIN:
			//qDrawPlainRect(p, opt.rect, CCOLOR_light_foreground());
			a = p->testRenderHint(QPainter::Antialiasing);
			if (a)
				p->setRenderHint(QPainter::Antialiasing, false);
			p->setPen(CCOLOR_light_foreground());
			p->setBrush(Qt::NoBrush);
			opt.rect = QRect(opt.rect.x(), opt.rect.y(), opt.rect.width() - 1, opt.rect.height() - 1);
			p->drawRect(opt.rect);
			if (a)
				p->setRenderHint(QPainter::Antialiasing, true);
			break;
			
		case BORDER_SUNKEN:
			optv3.rect = opt.rect;
			optv3.state = opt.state | QStyle::State_Sunken;
			optv3.frameShape = QFrame::StyledPanel;
			
			save_brush = p->brush();
			p->setBrush(QBrush());
			style->drawPrimitive(QStyle::PE_Frame, &optv3, p, w);
			p->setBrush(save_brush);
			//style->drawControl(QStyle::CE_ShapedFrame, &optv3, p, w);
			break;
			
		case BORDER_RAISED:
			optv3.rect = opt.rect;
			optv3.state = opt.state | QStyle::State_Raised;
			optv3.frameShape = QFrame::StyledPanel;
			style->drawPrimitive(QStyle::PE_Frame, &optv3, p, w);
			/*opt.lineWidth = 2;
			opt.midLineWidth = 2;
			opt.state |= QStyle::State_Raised;
			style->drawPrimitive(QStyle::PE_Frame, &opt, p, w);*/
			break;
			
		case BORDER_ETCHED:
			optv3.rect = opt.rect;
			//optv3.state = opt.state | QStyle::State_Raised;
			optv3.frameShape = QFrame::StyledPanel;
			style->drawPrimitive(QStyle::PE_FrameGroupBox, &optv3, p, w);
			//qDrawShadeRect(p, opt.rect, opt.palette, true, 1, 0);
			break;
			
		default:
			return;
	}
}

void CCONTAINER_draw_border(QPainter *p, char frame, QWidget *wid)
{
	QStyleOptionFrame opt;
	opt.init(wid);
	opt.rect = QRect(0, 0, wid->width(), wid->height());
	_draw_border(p, frame, wid, opt);
}

void CCONTAINER_draw_border_without_widget(QPainter *p, char frame, QStyleOptionFrame &opt)
{
	_draw_border(p, frame, NULL, opt);
}


int CCONTAINER_get_border_width(char border)
{
	switch (border)
	{
		case BORDER_PLAIN:
			return 1;
			break;

		case BORDER_SUNKEN:
		case BORDER_RAISED:
		case BORDER_ETCHED:

			return qApp->style()->pixelMetric(QStyle::QStyle::PM_ComboBoxFrameWidth);
			break;

		default:
			return 0;
	}
}

void CCONTAINER_set_border(char *border, char new_border, QWidget *wid)
{
	int m;

	if (new_border < BORDER_NONE || new_border > BORDER_ETCHED || new_border == *border)
		return;

	*border = new_border;
	m = CCONTAINER_get_border_width(new_border);

	wid->setContentsMargins(m, m, m, m);
	wid->update();
}

void MyFrame::drawFrame(QPainter *p)
{
	CCONTAINER_draw_border(p, _frame, this);
}

int MyFrame::frameWidth()
{
	switch (_frame)
	{
		case BORDER_PLAIN:
			return 1;

		case BORDER_SUNKEN:
		case BORDER_RAISED:
			return style()->pixelMetric(QStyle::PM_ComboBoxFrameWidth);

		case BORDER_ETCHED:
			return 2;

		default:
			return 0;
	}
}

void MyFrame::setPixmap(QPixmap *pixmap)
{
	if (_pixmap != pixmap)
	{
		_pixmap = pixmap;
		setAttribute(Qt::WA_OpaquePaintEvent, _pixmap != 0);
		setStaticContents(_pixmap != 0);
	}
}

void MyFrame::paintEvent(QPaintEvent *e)
{
	QPainter painter(this);

	if (_bg)
	{
		CWIDGET *window = CWidget::get(parentWidget());
		GB_COLOR col = CWIDGET_get_background(window);
		if (col != COLOR_DEFAULT)
			painter.fillRect(e->rect(), CCOLOR_make(col));
	}

	if (_pixmap)
		painter.drawTiledPixmap(0, 0, width(), height(), *_pixmap);

	drawFrame(&painter);
}


/***************************************************************************

	class MyContainer

***************************************************************************/

MyContainer::MyContainer(QWidget *parent)
: MyFrame(parent)
{
	//setStaticContents(true);
	//setAttribute(Qt::WA_StaticContents);
	//setAttribute(Qt::WA_OpaquePaintEvent); //, _pixmap != 0);
}

MyContainer::~MyContainer()
{
	CWIDGET *_object = CWidget::getReal(this);
	if (THIS)
		CWIDGET_set_flag(THIS, WF_DELETED);
}

void MyContainer::showEvent(QShowEvent *e)
{
	void *_object = CWidget::get(this);
	QWidget::showEvent(e);
	THIS->widget.flag.shown = TRUE;
	/*if (!qstrcmp(GB.GetClassName(THIS), "HBox"))
	{
		qDebug("MyContainer::showEvent: %s %p: shown = 1 (%d %d)", THIS->widget.name, THIS, THIS->widget.widget->isVisible() , !THIS->widget.widget->isHidden());
		//BREAKPOINT();
	}*/
	CCONTAINER_arrange(THIS);
}

void MyContainer::hideEvent(QHideEvent *e)
{
	void *_object = CWidget::get(this);
	QWidget::hideEvent(e);
	THIS->widget.flag.shown = FALSE;
	/*if (!qstrcmp(GB.GetClassName(THIS), "HBox"))
	{
		qDebug("MyContainer::hideEvent: %s %p: shown = 0", THIS->widget.name, THIS);
		//BREAKPOINT();
	}*/
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

	if (qobject_cast<QGroupBox *>(WIDGET))
		return QRect(0, 0, w->width(), w->height());

	return w->contentsRect();
}

BEGIN_METHOD_VOID(Container_Children_next)

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


BEGIN_METHOD(Container_Children_get, GB_INTEGER index)

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


BEGIN_PROPERTY(Container_Children_Count)

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


BEGIN_METHOD_VOID(Container_Children_Clear)

	QWidget *wid = CONTAINER;
	QObjectList list;
	QObject *ob;
	int i;
	bool locked;

	if (!wid)
		return;

	locked = THIS_ARRANGEMENT->locked;
	THIS_ARRANGEMENT->locked = true;

	list = wid->children();

	for(i = 0; i < list.count(); i++)
	{
		ob = list.at(i);
		if (ob->isWidgetType())
			CWIDGET_destroy(CWidget::getRealExisting(ob));
	}

	THIS_ARRANGEMENT->locked = locked;
	CCONTAINER_arrange(THIS);

END_METHOD


BEGIN_PROPERTY(Container_X)

	#ifdef DEBUG
	if (!CONTAINER)
		qDebug("Null container");
	#endif

	QRect r = getRect(THIS); //_CONTAINER);
	QPoint p(r.x(), r.y());

	GB.ReturnInteger(CONTAINER->mapTo(WIDGET, p).x());

END_PROPERTY


BEGIN_PROPERTY(Container_Y)

	#ifdef DEBUG
	if (!CONTAINER)
		qDebug("Null container");
	#endif

	QRect r = getRect(THIS); // _CONTAINER);
	QPoint p(r.x(), r.y());
	
	GB.ReturnInteger(CONTAINER->mapTo(WIDGET, p).y());

END_PROPERTY


BEGIN_PROPERTY(Container_Width)

	#ifdef DEBUG
	if (!CONTAINER)
		qDebug("Null container");
	#endif

	GB.ReturnInteger(getRect(THIS).width());

END_PROPERTY


BEGIN_PROPERTY(Container_Height)

	#ifdef DEBUG
	if (!CONTAINER)
		qDebug("Null container");
	#endif

	GB.ReturnInteger(getRect(THIS).height());

END_PROPERTY

BEGIN_PROPERTY(Container_Border)

	MyContainer *w = qobject_cast<MyContainer *>(THIS->container);

	if (!w)
		return;

	if (READ_PROPERTY)
		GB.ReturnInteger(w->frameStyle());
	else
	{
		w->setFrameStyle(VPROP(GB_INTEGER));
		arrange_now(CONTAINER);
	}

END_PROPERTY

BEGIN_PROPERTY(Container_SimpleBorder)

	MyContainer *w = qobject_cast<MyContainer *>(THIS->container);

	if (!w)
		return;

	if (READ_PROPERTY)
		GB.ReturnBoolean(w->frameStyle());
	else
	{
		w->setFrameStyle(VPROP(GB_BOOLEAN) ? BORDER_PLAIN : BORDER_NONE);
		arrange_now(CONTAINER);
	}

END_PROPERTY

BEGIN_PROPERTY(Container_Arrangement)

	if (READ_PROPERTY)
		GB.ReturnInteger(THIS_ARRANGEMENT->mode);
	else
	{
		int arr = VPROP(GB_INTEGER);
		if (arr < 0 || arr > 8 || arr == THIS_ARRANGEMENT->mode)
			return;
		THIS_ARRANGEMENT->mode = arr;
		arrange_now(CONTAINER);
	}

END_PROPERTY

BEGIN_PROPERTY(UserContainer_Arrangement)

	CCONTAINER *cont = (CCONTAINER *)CWidget::get(CONTAINER);
	Container_Arrangement(cont, _param);
	if (!READ_PROPERTY)
	{
		THIS_USERCONTAINER->save = cont->arrangement;
		//qDebug("(%s %p): save = %08X (arrangement %d)", GB.GetClassName(THIS), THIS, THIS_USERCONTAINER->save, val);
	}

END_PROPERTY

BEGIN_PROPERTY(UserContainer_Focus)

	if (READ_PROPERTY)
		GB.ReturnBoolean(CWIDGET_get_allow_focus(THIS));
	else
		CWIDGET_set_allow_focus(THIS, VPROP(GB_BOOLEAN));

END_PROPERTY

BEGIN_PROPERTY(Container_AutoResize)

	if (READ_PROPERTY)
		GB.ReturnBoolean(THIS_ARRANGEMENT->autoresize);
	else
	{
		bool v = VPROP(GB_BOOLEAN);
		if (v == THIS_ARRANGEMENT->autoresize)
			return;
		
		THIS_ARRANGEMENT->autoresize = v;
		arrange_now(CONTAINER);
	}

END_PROPERTY

BEGIN_PROPERTY(UserContainer_AutoResize)

	CCONTAINER *cont = (CCONTAINER *)CWidget::get(CONTAINER);
	Container_AutoResize(cont, _param);
	if (!READ_PROPERTY)
	{
		THIS_USERCONTAINER->save = cont->arrangement;
		//qDebug("(%s %p): save = %08X (autoresize)", GB.GetClassName(THIS), THIS, THIS_USERCONTAINER->save);
	}

END_PROPERTY


BEGIN_PROPERTY(Container_Margin)

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

BEGIN_PROPERTY(UserContainer_Margin)

	CCONTAINER *cont = (CCONTAINER *)CWidget::get(CONTAINER);
	Container_Margin(cont, _param);
	if (!READ_PROPERTY)
	{
		THIS_USERCONTAINER->save = cont->arrangement;
	  //qDebug("(%s %p): save = %08X (padding)", GB.GetClassName(THIS), THIS, THIS_USERCONTAINER->save);
	}

END_PROPERTY


BEGIN_PROPERTY(Container_Spacing)

  if (READ_PROPERTY)
    GB.ReturnBoolean(THIS_ARRANGEMENT->spacing);
  else
  {
  	bool val = VPROP(GB_BOOLEAN);
  	if (val != THIS_ARRANGEMENT->spacing)
  	{
    	THIS_ARRANGEMENT->spacing = val;
			arrange_now(CONTAINER);
		}
  }

END_PROPERTY

BEGIN_PROPERTY(UserContainer_Spacing)

	CCONTAINER *cont = (CCONTAINER *)CWidget::get(CONTAINER);
	Container_Spacing(cont, _param);
	if (!READ_PROPERTY)
	{
		THIS_USERCONTAINER->save = cont->arrangement;
	  //qDebug("(%s %p): save = %08X (spacing)", GB.GetClassName(THIS), THIS, THIS_USERCONTAINER->save);
	}

END_PROPERTY


BEGIN_PROPERTY(Container_Invert)

  if (READ_PROPERTY)
    GB.ReturnBoolean(THIS_ARRANGEMENT->invert);
  else
  {
  	bool val = VPROP(GB_BOOLEAN);
  	if (val != THIS_ARRANGEMENT->invert)
  	{
    	THIS_ARRANGEMENT->invert = val;
			arrange_now(CONTAINER);
		}
  }

END_PROPERTY

BEGIN_PROPERTY(UserContainer_Invert)

	CCONTAINER *cont = (CCONTAINER *)CWidget::get(CONTAINER);
	Container_Invert(cont, _param);
	if (!READ_PROPERTY)
		THIS_USERCONTAINER->save = cont->arrangement;

END_PROPERTY


BEGIN_PROPERTY(Container_Padding)

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

BEGIN_PROPERTY(UserContainer_Padding)

	CCONTAINER *cont = (CCONTAINER *)CWidget::get(CONTAINER);
	Container_Padding(cont, _param);
	if (!READ_PROPERTY)
	{
		THIS_USERCONTAINER->save = cont->arrangement;
	  //qDebug("(%s %p): save = %08X (spacing)", GB.GetClassName(THIS), THIS, THIS_USERCONTAINER->save);
	}

END_PROPERTY

BEGIN_PROPERTY(Container_Indent)

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

BEGIN_METHOD(UserControl_new, GB_OBJECT parent)

	MyContainer *wid = new MyContainer(QCONTAINER(VARG(parent)));

	THIS->container = wid;
	THIS_ARRANGEMENT->mode = ARRANGE_FILL;
	THIS_ARRANGEMENT->user = true;

	CWIDGET_new(wid, (void *)_object);

END_METHOD


BEGIN_PROPERTY(UserControl_Container)

	void *current = (CCONTAINER *)CWidget::get(CONTAINER);

	if (READ_PROPERTY)
	{
		GB.ReturnObject(current);
	}
	else
	{
		CCONTAINER *cont = (CCONTAINER *)VPROP(GB_OBJECT);
		QWidget *w;
		QWidget *p;

		// sanity checks
		
		if (!cont)
		{
			if (current)
				CWIDGET_container_for(current, NULL);
			THIS->container = WIDGET;
			CWIDGET_register_proxy(THIS, NULL);
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
			GB_COLOR bg = CWIDGET_get_background((CWIDGET *)current, true);
			GB_COLOR fg = CWIDGET_get_foreground((CWIDGET *)current, true);

			if (current)
				CWIDGET_container_for(current, NULL);
			CWIDGET_container_for(cont, THIS);
			
			THIS->container = w;

			CWIDGET_update_design((CWIDGET *)THIS);
			CCONTAINER_arrange(THIS);

			CWIDGET_set_color((CWIDGET *)cont, bg, fg, true);

			CWIDGET_register_proxy(THIS, cont);
		}
	}

END_PROPERTY


BEGIN_PROPERTY(UserContainer_Indent)

	CCONTAINER *cont = (CCONTAINER *)CWidget::get(CONTAINER);
	Container_Indent(cont, _param);
	if (!READ_PROPERTY)
	{
		THIS_USERCONTAINER->save = cont->arrangement;
	}

END_PROPERTY


BEGIN_PROPERTY(UserContainer_Container)

	//CCONTAINER *before;
	CCONTAINER *after;

	if (READ_PROPERTY)
		UserControl_Container(_object, _param);
	else
	{
		UserControl_Container(_object, _param);

		after = (CCONTAINER *)CWidget::get(THIS->container);
		bool old_locked = ((CCONTAINER_ARRANGEMENT *)after)->locked;
		after->arrangement = THIS_USERCONTAINER->save;
		((CCONTAINER_ARRANGEMENT *)after)->locked = old_locked;
		//qDebug("(%s %p): arrangement = %08X", GB.GetClassName(THIS), THIS, after->arrangement);
		CCONTAINER_arrange(after);
	}

END_PROPERTY


BEGIN_PROPERTY(UserContainer_Design)

	Control_Design(_object, _param);
	
	if (!READ_PROPERTY && VPROP(GB_BOOLEAN))
	{
		CCONTAINER *cont = (CCONTAINER *)CWidget::get(CONTAINER);
		
		cont->arrangement = 0;
		((CCONTAINER_ARRANGEMENT *)cont)->user = true;
		THIS_USERCONTAINER->save = cont->arrangement;
	}

END_PROPERTY


BEGIN_METHOD(Container_FindChild, GB_INTEGER x; GB_INTEGER y)

	QObjectList list = CONTAINER->children();
	int index = 0;
	QWidget *w;
	void *control;
	
	for(;;)
	{
		w = get_next_widget(list, index);
		if (!w)
			break;
		if (w->geometry().contains(VARG(x), VARG(y)))
		{
			control = CWidget::get(w);
			if (control)
			{
				GB.ReturnObject(control);
				return;
			}
		}
	}

	GB.ReturnNull();

END_METHOD


BEGIN_METHOD(Container_unknown, GB_VALUE x; GB_VALUE y)

	char *name = GB.GetUnknown();
	int nparam = GB.NParam();
	
	if (strcasecmp(name, "Find"))
	{
		GB.Error(GB_ERR_NSYMBOL, GB.GetClassName(NULL), name);
		return;
	}
	
	if (nparam < 2)
	{
		GB.Error("Not enough argument");
		return;
	}
	else if (nparam > 2)
	{
		GB.Error("Too many argument");
		return;
	}
	
	GB.Deprecated("gb.qt4", "Container.Find", "Container.FindChild");
	
	if (GB.Conv(ARG(x), GB_T_INTEGER) || GB.Conv(ARG(y), GB_T_INTEGER))
		return;
	
	Container_FindChild(_object, _param);
	
	GB.ReturnConvVariant();

END_METHOD


//---------------------------------------------------------------------------

GB_DESC CChildrenDesc[] =
{
	GB_DECLARE(".Container.Children", sizeof(CCONTAINER)), GB_VIRTUAL_CLASS(),

	GB_METHOD("_next", "Control", Container_Children_next, NULL),
	GB_METHOD("_get", "Control", Container_Children_get, "(Index)i"),
	GB_PROPERTY_READ("Count", "i", Container_Children_Count),
	GB_METHOD("Clear", NULL, Container_Children_Clear, NULL),

	GB_END_DECLARE
};


GB_DESC CContainerDesc[] =
{
	GB_DECLARE("Container", sizeof(CCONTAINER)), GB_INHERITS("Control"),
	GB_NOT_CREATABLE(),

	GB_PROPERTY_SELF("Children", ".Container.Children"),

	GB_PROPERTY_READ("ClientX", "i", Container_X),
	GB_PROPERTY_READ("ClientY", "i", Container_Y),
	GB_PROPERTY_READ("ClientW", "i", Container_Width),
	GB_PROPERTY_READ("ClientWidth", "i", Container_Width),
	GB_PROPERTY_READ("ClientH", "i", Container_Height),
	GB_PROPERTY_READ("ClientHeight", "i", Container_Height),
	
	GB_METHOD("_unknown", "v", Container_unknown, "."),
	GB_METHOD("FindChild", "Control", Container_FindChild, "(X)i(Y)i"),
	
	CONTAINER_DESCRIPTION,

	GB_EVENT("BeforeArrange", NULL, NULL, &EVENT_BeforeArrange),
	GB_EVENT("Arrange", NULL, NULL, &EVENT_Arrange),
	GB_EVENT("NewChild", NULL, "(Control)Control", &EVENT_Insert),

	GB_END_DECLARE
};


GB_DESC CUserControlDesc[] =
{
	GB_DECLARE("UserControl", sizeof(CCONTAINER)), GB_INHERITS("Container"),
	GB_NOT_CREATABLE(),

	GB_METHOD("_new", NULL, UserControl_new, "(Parent)Container;"),

	GB_PROPERTY("_Container", "Container", UserControl_Container),
	GB_PROPERTY("_AutoResize", "b", Container_AutoResize),
	GB_PROPERTY("_Arrangement", "i", Container_Arrangement),
	GB_PROPERTY("_Padding", "i", Container_Padding),
	GB_PROPERTY("_Spacing", "b", Container_Spacing),
	GB_PROPERTY("_Margin", "b", Container_Margin),
	GB_PROPERTY("_Indent", "b", Container_Indent),
	GB_PROPERTY("_Invert", "b", Container_Invert),

	USERCONTROL_DESCRIPTION,
	
	GB_END_DECLARE
};


GB_DESC CUserContainerDesc[] =
{
	GB_DECLARE("UserContainer", sizeof(CUSERCONTAINER)), GB_INHERITS("Container"),
	GB_NOT_CREATABLE(),

	GB_METHOD("_new", NULL, UserControl_new, "(Parent)Container;"),

	GB_PROPERTY("_Container", "Container", UserContainer_Container),
	GB_PROPERTY("_Arrangement", "i", Container_Arrangement),

	GB_PROPERTY("Arrangement", "i", UserContainer_Arrangement),
	GB_PROPERTY("AutoResize", "b", UserContainer_AutoResize),
	GB_PROPERTY("Padding", "i", UserContainer_Padding),
	GB_PROPERTY("Spacing", "b", UserContainer_Spacing),
	GB_PROPERTY("Margin", "b", UserContainer_Margin),
	GB_PROPERTY("Indent", "b", UserContainer_Indent),
	GB_PROPERTY("Invert", "b", UserContainer_Invert),
	
	GB_PROPERTY("Design", "b", UserContainer_Design),

	//GB_PROPERTY("Focus", "b", UserContainer_Focus),

	USERCONTAINER_DESCRIPTION,
	
	GB_END_DECLARE
};


