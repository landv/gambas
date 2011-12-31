/***************************************************************************

  CWidget.cpp

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

#define __CWIDGET_CPP

#undef QT3_SUPPORT

#include "gambas.h"
#include "gb_common.h"

#include <stdio.h>
#include <stdlib.h>

#include "CWidget.h"
#include "CFont.h"
#include "CMouse.h"
#include "CKey.h"
#include "CWindow.h"
#include "CConst.h"
#include "CColor.h"
#include "CClipboard.h"
#include "CMenu.h"
#include "CScrollView.h"
#include "CProgress.h"
#include "CDrawingArea.h"
#include "CTextArea.h"

#include <QApplication>
#include <QObject>
#include <QPalette>
#include <QToolTip>
#include <QPushButton>
#include <QMap>
#include <QContextMenuEvent>
#include <QKeyEvent>
#include <QPixmap>
#include <QFrame>
#include <QDropEvent>
#include <QMouseEvent>
#include <QEvent>
#include <QWheelEvent>
#include <QHash>
#include <QAbstractScrollArea>
#include <Q3ScrollView>
#include <QProgressBar>
#include <QAbstractEventDispatcher>
 
#ifndef NO_X_WINDOW
static QMap<int, int> _x11_to_qt_keycode;
#endif

/* Control class */

CWIDGET *CWIDGET_active_control = 0;
static bool _focus_change = false;
static CWIDGET *_old_active_control = 0;

static CWIDGET *_hovered = 0;
static CWIDGET *_official_hovered = 0;

#define HANDLE_PROXY(_ob) \
	while (((CWIDGET *)_ob)->proxy) \
		_ob = (typeof _ob)((CWIDGET *)_ob)->proxy;

static void set_mouse(QWidget *w, int mouse, void *cursor)
{
	QObjectList children;
	QObject *child;
	int i;

	if (mouse == CMOUSE_DEFAULT)
		w->unsetCursor();
	else if (mouse == CMOUSE_CUSTOM)
	{
		if (cursor)
			w->setCursor(*((CCURSOR *)cursor)->cursor);
		else
			w->unsetCursor();
	}
	else
		w->setCursor(QCursor((Qt::CursorShape)mouse));

	children = w->children();

	for (i = 0; i < children.count(); i++)
	{
		child = children.at(i);
		
		if (child->isWidgetType() && !CWidget::getReal(child))
			set_mouse((QWidget *)child, CMOUSE_DEFAULT, 0);
	}
}

static void set_design_object(CWIDGET *_object)
{
	if (CWIDGET_test_flag(THIS, WF_DESIGN))
		return;

	//qDebug("%s %p (%p): DESIGN", GB.GetClassName(THIS), THIS, WIDGET);
	CWIDGET_set_flag(THIS, WF_DESIGN);

	CWidget::removeFocusPolicy(WIDGET);
	set_mouse(WIDGET, CMOUSE_DEFAULT, 0);
	//THIS->flag.fillBackground = true;
}

static void set_design_recursive(QWidget *w, bool set = false)
{
	QObjectList children;
	int i;
	QObject *child;
	CWIDGET *ob = CWidget::getReal(w);

	if (ob)
		set_design_object(ob);

	children = w->children();

	for (i = 0; i < children.count(); i++)
	{
		child = children.at(i);
		
		if (child->isWidgetType())
			set_design_recursive((QWidget *)child, true);
	}
}

static void set_design(CWIDGET *_object)
{
	CWIDGET *cont;

	if (GB.Is(THIS, CLASS_UserControl))
		set_design_recursive(WIDGET);
	else if (!GB.Is(THIS, CLASS_Container))
		set_design_object(THIS);

	CWIDGET_set_flag(THIS, WF_DESIGN_LEADER);
	
	if (GB.Is(THIS, CLASS_Container))
	{
		//qDebug("(%s %p - %p): LEADER / %p %p", GB.GetClassName(THIS), THIS, WIDGET, QCONTAINER(THIS), CWidget::getReal(QCONTAINER(THIS)));

		cont = CWidget::get(QCONTAINER(THIS));
		//debugObject(cont);		
		if (cont && cont != THIS)
			set_design_object(cont);
	}

	if (GB.Is(THIS, CLASS_TabStrip))
	{
		THIS->flag.fillBackground = TRUE;
		CWIDGET_reset_color(THIS);
	}
}

static void set_name(CWIDGET *_object, const char *name)
{
	CWINDOW *window;
	MyMainWindow *win = 0;
	
	if (GB.Is(THIS, CLASS_Menu))
	{
		if (qobject_cast<MyMainWindow *>(((CMENU *)THIS)->toplevel))
			win = (MyMainWindow *)((CMENU *)THIS)->toplevel;
	}
	else
	{
		window = CWidget::getWindow(THIS);
		if (window)
			win = (MyMainWindow *)QWIDGET(window);

		if (win)
		{
			if (name)
				win->setName(name, THIS);
			else
				win->setName(THIS->name, 0);
		}
	}
		
	GB.FreeString(&THIS->name);
	
	if (name)
		THIS->name = GB.NewZeroString(name);
}

void *CWIDGET_get_parent(void *_object)
{
  QWidget *parent = WIDGET->parentWidget();

  if (!parent || (GB.Is(THIS, CLASS_Window) && ((CWINDOW *)_object)->toplevel))
    return NULL;
  else
    return CWidget::get(parent);
}

static bool is_visible(void *_object)
{
	return THIS->flag.visible; // || !QWIDGET(_object)->isHidden();
}

static void register_proxy(void *_object, void *proxy)
{
	void *check = proxy;

	while (check)
	{
		if (check == THIS)
		{
			GB.Error("Circular proxy chain");	
			return;
		}
		check = ((CWIDGET *)check)->proxy;
	}
	
	if (THIS->proxy)
		((CWIDGET *)THIS->proxy)->proxy_for = NULL;
	
	THIS->proxy = proxy;
	
	if (THIS->proxy)
		((CWIDGET *)THIS->proxy)->proxy_for = THIS;
}

int CWIDGET_check(void *_object)
{
	return WIDGET == NULL || CWIDGET_test_flag(THIS, WF_DELETED);
}

static QWidget *get_viewport(QWidget *w)
{
	if (qobject_cast<QAbstractScrollArea *>(w))
		return ((QAbstractScrollArea *)w)->viewport();
	else if (qobject_cast<Q3ScrollView *>(w))
		return ((Q3ScrollView *)w)->viewport();
	else
		return 0;
}

void CWIDGET_update_design(CWIDGET *_object)
{
	if (!CWIDGET_test_flag(THIS, WF_DESIGN) && !CWIDGET_test_flag(THIS, WF_DESIGN_LEADER))
		return;

	//qDebug("CWIDGET_update_design: %s %p", GB.GetClassName(THIS), THIS);
	set_design(THIS);
}

void CWIDGET_init_name(CWIDGET *_object)
{
	char *name = GB.GetLastEventName();
	if (!name)
		name = GB.GetClassName(THIS);
	//qDebug("name: %p: %s", THIS, name);
	set_name(THIS, name);
}

void CWIDGET_new(QWidget *w, void *_object, bool no_show, bool no_filter, bool no_init)
{
	//QAbstractScrollArea *sa;
	
	CWidget::add(w, _object, no_filter);

	//QWidget *p = w->parentWidget();
	//qDebug("CWIDGET_new: %s %p: %p in (%s %p)", GB.GetClassName(THIS), THIS, w, p ? GB.GetClassName(CWidget::get(p)) : "", CWidget::get(p));

	THIS->widget = w;
	THIS->level = MAIN_loop_level;

	if (!no_init)
	{
		THIS->tag.type = GB_T_NULL;
		CWIDGET_init_name(THIS);	
	}

	THIS->bg = COLOR_DEFAULT;
	THIS->fg = COLOR_DEFAULT;
	
	if (qobject_cast<QAbstractScrollArea *>(w) || qobject_cast<Q3ScrollView *>(w))
		CWIDGET_set_flag(THIS, WF_SCROLLVIEW);

	//w->setAttribute(Qt::WA_PaintOnScreen, true);
	
	CWIDGET_reset_color(THIS); //w->setPalette(QApplication::palette());
	
	//THIS->flag.fillBackground = GB.Is(THIS, CLASS_Container);
	//w->setAutoFillBackground(THIS->flag.fillBackground);
	
	CCONTAINER_insert_child(THIS);

	if (!no_show)
	{
		w->setGeometry(-16, -16, 8, 8);
		CWIDGET_set_visible(THIS, true);
		w->raise();
	}
}


QString CWIDGET_Utf8ToQString(GB_STRING *str)
{
	return QString::fromUtf8((const char *)(str->value.addr + str->value.start), str->value.len);
}


void CWIDGET_destroy(CWIDGET *object)
{
	if (!object || !object->widget)
		return;

	if (CWIDGET_test_flag(object, WF_DELETED))
		return;
	
	if (object->flag.dragging)
	{
		GB.Error("Control is being dragged");
		return;
	}

	//qDebug("CWIDGET_destroy: %p (%p) :%p:%ld", object, object->widget, object->ob.klass, object->ob.ref);
	//qDebug("CWIDGET_destroy: %s %p", GB.GetClassName(object), object);


	CWIDGET_set_flag(object, WF_DELETED);
	CWIDGET_set_visible(object, false);

	if (qobject_cast<QProgressBar *>(object->widget))
		CPROGRESS_style_hack(object);
	
	object->widget->deleteLater();
}


//#if QT_VERSION >= 0x030005
//  #define COORD(_c) (WIDGET->pos()._c())
//#else
#define COORD(_c) ((qobject_cast<MyMainWindow *>(WIDGET) && WIDGET->isWindow()) ? ((CWINDOW *)_object)->_c : WIDGET->pos()._c())
//#define WIDGET_POS(_c) ((WIDGET->isWindow()) ? ((CWINDOW *)_object)->_c : WIDGET->pos()._c())
//#define WIDGET_SIZE(_c) ((WIDGET->isA("MyMainWindow")) ? ((CWINDOW *)_object)->_c : WIDGET->pos()._c())
//#endif

#if 0
static QWidget *get_widget(void *_object)
{
	QWidget *w = THIS->widget;
	//if (w->isVisible() && CWIDGET_test_flag(THIS, WF_PARENT_GEOMETRY))
	//  w = w->parentWidget();

	if (WIDGET->isA("MyMainWindow"))
	{
		CWINDOW *win = (CWINDOW *)THIS;
		if (win->toplevel && win->embedded)
		{
			QWidget *p = w->parentWidget();
			if (p && p->isA("QWorkspaceChild"))
				w = p;
		}
	}

	return w;
}

static QWidget *get_widget_resize(void *_object)
{
	QWidget *w = THIS->widget;
	return w;
}
#endif

#define get_widget(_object) QWIDGET(_object)
#define get_widget_resize(_object) QWIDGET(_object)

static void arrange_parent(CWIDGET *_object)
{
	void *parent = CWIDGET_get_parent(THIS);
	if (!parent)
		return;
	if (CWIDGET_check(parent))
		return;
	CCONTAINER_arrange(parent);
}

static void CWIDGET_after_geometry_change(void *_object, bool arrange)
{
	if (arrange)
	{
		if (GB.Is(THIS, CLASS_Container))
			CCONTAINER_arrange(THIS);
		if (GB.Is(THIS, CLASS_DrawingArea))
			((MyDrawingArea *)((CWIDGET *)_object)->widget)->updateBackground();
	}
	
  arrange_parent(THIS);
}

void CWIDGET_move(void *_object, int x, int y)
{
  QWidget *wid = get_widget(THIS);

  if (GB.Is(THIS, CLASS_Window))
  {
		CWINDOW *win = (CWINDOW *)_object;
    win->x = x;
    win->y = y;
		win->mustCenter = false;
  }
  
	if (wid)
	{
		if (x == wid->x() && y == wid->y())
			return;

  	wid->move(x, y);
	}

	CWIDGET_after_geometry_change(THIS, false);
}

/*
void CWIDGET_move_cached(void *_object, int x, int y)
{
  if (GB.Is(THIS, CLASS_Window))
  {
    ((CWINDOW *)_object)->x = x;
    ((CWINDOW *)_object)->y = y;
  }
  
	CWIDGET_after_geometry_change(THIS, false);
}
*/

void CWIDGET_resize(void *_object, int w, int h)
{
  QWidget *wid = get_widget_resize(THIS);
	bool window;
	bool resizable = true;
	bool decide_w, decide_h;

	if (!wid)
		return;
	
	window = wid->isTopLevel();
	
	if (w < 0 && h < 0)
		return;

	CCONTAINER_decide(THIS, &decide_w, &decide_h);

	if (w < 0 || decide_w)
		w = wid->width();

	if (h < 0 || decide_h)
		h = wid->height();

	if (w == wid->width() && h == wid->height())
		return;

	if (window)
	{
		resizable = ((MyMainWindow *)wid)->isResizable();
		if (!resizable)
			((MyMainWindow *)wid)->setResizable(true);
	}
  
	wid->resize(qMax(0, w), qMax(0, h));

  if (window)
  {
		((MyMainWindow *)wid)->setResizable(resizable);
    ((CWINDOW *)_object)->w = w;
    ((CWINDOW *)_object)->h = h;
    // menu bar height is ignored
    //((CWINDOW *)_object)->container->resize(w, h);
  }

	CWIDGET_after_geometry_change(THIS, true);
}

/*
void CWIDGET_resize_cached(void *_object, int w, int h)
{
  if (GB.Is(THIS, CLASS_Window))
  {
    ((CWINDOW *)_object)->w = w;
    ((CWINDOW *)_object)->h = h;
  }

	CWIDGET_after_geometry_change(THIS, true);
}
*/

void CWIDGET_move_resize(void *_object, int x, int y, int w, int h)
{
  QWidget *wid = get_widget(THIS);

	if (wid)
	{
		if (w < 0)
			w = wid->width();

		if (h < 0)
			h = wid->height();
	}

  if (GB.Is(THIS, CLASS_Window))
  {
		CWINDOW *win = (CWINDOW *)_object;
		win->x = x;
    win->y = y;
    win->w = w;
    win->h = h;
		win->mustCenter = false;
  }

	if (wid)
	{
		if (w < 0)
			w = wid->width();

		if (h < 0)
			h = wid->height();

		if (x == wid->x() && y == wid->y() && w == wid->width() && h == wid->height())
			return;
		
		if (wid->isTopLevel())
		{
			wid->move(x, y);
			wid->resize(qMax(0, w), qMax(0, h));
		}
		else
			wid->setGeometry(x, y, qMax(0, w), qMax(0, h));
	}

	CWIDGET_after_geometry_change(THIS, true);
}

#if 0
void CWIDGET_move_resize(void *_object, int x, int y, int w, int h)
{
  QWidget *wid = get_widget(THIS);

	if (wid)
	{
// 		if (wid->isA("QWorkspaceChild"))
// 		{
// 			CWIDGET_move(THIS, x, y);
// 			CWIDGET_resize(THIS, w, h);
// 			return;
// 		}

		if (w < 0)
			w = wid->width();

		if (h < 0)
			h = wid->height();

		if (x == wid->x() && y == wid->y() && w == wid->width() && h == wid->height())
			return;
		wid->setGeometry(x, y, qMax(0, w), qMax(0, h));
	}

  if (GB.Is(THIS, CLASS_Window))
  {
    ((CWINDOW *)_object)->x = x;
    ((CWINDOW *)_object)->y = y;
    ((CWINDOW *)_object)->w = w;
    ((CWINDOW *)_object)->h = h;
    //((CWINDOW *)_object)->container->resize(w, h);
  }

	CWIDGET_after_geometry_change(THIS, true);
}
#endif

/*
void CWIDGET_move_resize_cached(void *_object, int x, int y, int w, int h)
{
  if (GB.Is(THIS, CLASS_Window))
  {
    ((CWINDOW *)_object)->x = x;
    ((CWINDOW *)_object)->y = y;
    ((CWINDOW *)_object)->w = w;
    ((CWINDOW *)_object)->h = h;
  }

	CWIDGET_after_geometry_change(THIS, true);
}
*/

void CWIDGET_check_hovered()
{
	if (_official_hovered != _hovered)
	{
		if (_official_hovered) GB.Raise(_official_hovered, EVENT_Leave, NULL);
		if (_hovered) GB.Raise(_hovered, EVENT_Enter, NULL);
		_official_hovered = _hovered;
	}
}

BEGIN_PROPERTY(Control_X)

	if (READ_PROPERTY)
		GB.ReturnInteger(COORD(x));
	else
	{
		CWIDGET_move(_object, VPROP(GB_INTEGER), COORD(y));
		/*if (WIDGET->isWindow())
			qDebug("X: %d ==> X = %d", PROPERTY(int), WIDGET->x());*/
	}

END_PROPERTY


BEGIN_PROPERTY(Control_ScreenX)

	GB.ReturnInteger(WIDGET->mapToGlobal(QPoint(0, 0)).x());

END_PROPERTY


BEGIN_PROPERTY(Control_Y)

	if (READ_PROPERTY)
		GB.ReturnInteger(COORD(y));
	else
		CWIDGET_move(_object, COORD(x), VPROP(GB_INTEGER));

END_PROPERTY


BEGIN_PROPERTY(Control_ScreenY)

	GB.ReturnInteger(WIDGET->mapToGlobal(QPoint(0, 0)).y());

END_PROPERTY


BEGIN_PROPERTY(Control_Width)

	if (READ_PROPERTY)
		GB.ReturnInteger(get_widget_resize(THIS)->width());
	else
		CWIDGET_resize(_object, VPROP(GB_INTEGER), -1);

END_PROPERTY


BEGIN_PROPERTY(Control_Height)

	if (READ_PROPERTY)
		GB.ReturnInteger(get_widget_resize(THIS)->height());
	else
		CWIDGET_resize(_object, -1, VPROP(GB_INTEGER));

END_PROPERTY

BEGIN_PROPERTY(Control_Font)

	CFONT *font;
	
	if (!THIS->font)
	{
		THIS->font = CFONT_create(WIDGET->font(), 0, THIS);
		GB.Ref(THIS->font);
	}

	if (READ_PROPERTY)
	{
		GB.ReturnObject(THIS->font);
	}
	else
	{
		font = (CFONT *)VPROP(GB_OBJECT);

		if (!font)
		{
			WIDGET->setFont(QFont());
			GB.Unref(POINTER(&THIS->font));
			THIS->font = NULL;
		}
		else
		{
			WIDGET->setFont(*(font->font));
			*(((CFONT *)THIS->font)->font) = WIDGET->font();
		}
	}

END_PROPERTY


BEGIN_PROPERTY(Control_Design)

	if (READ_PROPERTY)
	{
		GB.ReturnBoolean(CWIDGET_test_flag(_object, WF_DESIGN) || CWIDGET_test_flag(_object, WF_DESIGN_LEADER));
		return;
	}

	if (VPROP(GB_BOOLEAN))
	{
		set_design(THIS);
		//CWIDGET_set_flag(THIS, WF_DESIGN);
	}
	else if (CWIDGET_test_flag(_object, WF_DESIGN) || CWIDGET_test_flag(_object, WF_DESIGN_LEADER))
		GB.Error("Cannot reset Design property");

END_PROPERTY


BEGIN_PROPERTY(Control_Enabled)

	if (READ_PROPERTY)
		GB.ReturnBoolean(QWIDGET(_object)->isEnabled());
	else
		QWIDGET(_object)->setEnabled(VPROP(GB_BOOLEAN));

END_PROPERTY


BEGIN_PROPERTY(Control_HasFocus)

	while (THIS->proxy)
		_object = THIS->proxy;
	
	GB.ReturnBoolean(WIDGET->hasFocus());

END_PROPERTY

BEGIN_PROPERTY(Control_Hovered)

	if (!is_visible(THIS))
	{
		GB.ReturnBoolean(false);
		return;
	}

	QPoint m = QCursor::pos();
	int x = WIDGET->mapToGlobal(QPoint(0, 0)).x(); 
	int y = WIDGET->mapToGlobal(QPoint(0, 0)).y(); 

	GB.ReturnBoolean(m.x() >= x && m.y() >= y && m.x() < (x + WIDGET->width()) && m.y() < (y + WIDGET->height()));

END_PROPERTY

BEGIN_PROPERTY(Control_Expand)

	if (READ_PROPERTY)
		GB.ReturnBoolean(THIS->flag.expand);
	else
	{
		THIS->flag.expand = VPROP(GB_BOOLEAN);
		arrange_parent(THIS);
		//qApp->postEvent(WIDGET, new QEvent(EVENT_EXPAND));
	}

END_PROPERTY


BEGIN_PROPERTY(Control_Ignore)

	if (READ_PROPERTY)
		GB.ReturnBoolean(THIS->flag.ignore);
	else
	{
		THIS->flag.ignore = VPROP(GB_BOOLEAN);
		arrange_parent(THIS);
		//qApp->postEvent(WIDGET, new QEvent(EVENT_EXPAND));
	}

END_PROPERTY


BEGIN_METHOD(Control_Move, GB_INTEGER x; GB_INTEGER y; GB_INTEGER w; GB_INTEGER h)

	CWIDGET_move_resize(_object, VARG(x), VARG(y), VARGOPT(w, -1), VARGOPT(h, -1));

END_METHOD


BEGIN_METHOD(Control_Resize, GB_INTEGER w; GB_INTEGER h)

	CWIDGET_resize(_object, VARG(w), VARG(h));

END_METHOD


BEGIN_METHOD(Control_MoveScaled, GB_FLOAT x; GB_FLOAT y; GB_FLOAT w; GB_FLOAT h)

	int x, y, w, h;

	x = (int)(VARG(x) * MAIN_scale + 0.5);
	y = (int)(VARG(y) * MAIN_scale + 0.5);
	w = (MISSING(w) ? -1 : (VARG(w) * MAIN_scale + 0.5));
	h = (MISSING(h) ? -1 : (VARG(h) * MAIN_scale + 0.5));
	
	if (w == 0) w = 1;
	if (h == 0) h = 1;

	CWIDGET_move_resize(_object, x, y, w, h);

END_METHOD


BEGIN_METHOD(Control_ResizeScaled, GB_FLOAT w; GB_FLOAT h)

	int w, h;

	w = (int)(VARG(w) * MAIN_scale);
	h = (int)(VARG(h) * MAIN_scale);
	
	if (w == 0) w = 1;
	if (h == 0) h = 1;

	CWIDGET_resize(_object, w , h);

END_METHOD


BEGIN_METHOD_VOID(Control_Delete)

	//if (WIDGET)
	//  qDebug("CWIDGET_delete: %p (%p)", THIS, WIDGET);

	CWIDGET_destroy(THIS);

END_METHOD


void CWIDGET_set_visible(CWIDGET *_object, bool v)
{
	bool arrange = false;
	
	THIS->flag.visible = v;
	if (THIS->flag.visible)
	{
		arrange = !WIDGET->isVisible();
		QWIDGET(_object)->show();
		if (GB.Is(THIS, CLASS_Container))
			CCONTAINER_arrange(THIS);
	}
	else
	{
		arrange = !WIDGET->isHidden();
		QWIDGET(_object)->hide();
	}
	
	if (arrange)
		arrange_parent(THIS);
}



BEGIN_PROPERTY(Control_Visible)

	if (READ_PROPERTY)
		GB.ReturnBoolean(is_visible(THIS));
	else
		CWIDGET_set_visible(THIS, VPROP(GB_BOOLEAN));

END_PROPERTY


BEGIN_METHOD_VOID(Control_Show)

	CWIDGET_set_visible(THIS, true);

END_METHOD


BEGIN_METHOD_VOID(Control_Hide)

	CWIDGET_set_visible(THIS, false);

END_METHOD


BEGIN_METHOD_VOID(Control_Raise)

	QWIDGET(_object)->raise();
	arrange_parent(THIS);

END_METHOD


BEGIN_METHOD_VOID(Control_Lower)

	QWIDGET(_object)->lower();
	arrange_parent(THIS);

END_METHOD


BEGIN_METHOD(Control_Move_under, GB_OBJECT control)

	CWIDGET *ob = (CWIDGET *)VARG(control);

	if (GB.CheckObject(ob))
		return;

	WIDGET->stackUnder(ob->widget);

END_METHOD


static QWidget *get_next(QWidget *w)
{
	QWidget *parent;
	QObjectList children;
	int i;
	QObject *current = NULL;

	parent = w->parentWidget();
	if (parent)
	{
		children = w->parentWidget()->children();
		i = children.indexOf(w) + 1;
		if (i > 0 && i < children.count())
			current = children.at(i);
	}

	return (QWidget *)current;
}

BEGIN_PROPERTY(Control_Next)

	if (READ_PROPERTY)
	{
		QWidget *next = get_next(WIDGET);

		if (next)
			GB.ReturnObject(CWidget::get(next));
		else
			GB.ReturnNull();
	}
	else
	{
		CWIDGET *ob = (CWIDGET *)VPROP(GB_OBJECT);
		
		if (!ob)
			WIDGET->raise();
		else
		{
			if (GB.CheckObject(ob))
				return;
			
			WIDGET->stackUnder(ob->widget);
		}
		arrange_parent(THIS);
	}

END_PROPERTY


BEGIN_PROPERTY(Control_Previous)

	if (READ_PROPERTY)
	{
		QWidget *parent;
		QObjectList children;
		int i;
		QObject *current = NULL;

		parent = WIDGET->parentWidget();
		if (parent)
		{
			children = WIDGET->parentWidget()->children();
			i = children.indexOf(WIDGET);
			if (i > 0)
				current = children.at(i - 1);
		}

		if (current)
			GB.ReturnObject(CWidget::get(current));
		else
			GB.ReturnNull();
	}
	else
	{
		CWIDGET *ob = (CWIDGET *)VPROP(GB_OBJECT);
		QWidget *w;

		if (!ob)
		{
			WIDGET->lower();
		}
		else
		{
			if (GB.CheckObject(ob))
				return;

			w = get_next(ob->widget);
			if (w)
			{
				//w = get_next(w);
				//if (w)
					WIDGET->stackUnder(w);
			}
		}
		arrange_parent(THIS);
	}

END_PROPERTY


BEGIN_METHOD_VOID(Control_Refresh) //, GB_INTEGER x; GB_INTEGER y; GB_INTEGER w; GB_INTEGER h)

	QWIDGET(_object)->update();
	if (CWIDGET_test_flag(THIS, WF_SCROLLVIEW))
		get_viewport(WIDGET)->update();

END_METHOD

static void set_focus(void *_object)
{
	CWINDOW *win;

	HANDLE_PROXY(_object);
	
	win = CWidget::getTopLevel(THIS);

	if (win->opened && QWIDGET(win)->isVisible())
	{
		WIDGET->setFocus();
	}
	else if ((CWIDGET *)win != THIS)
	{
		//qDebug("delayed focus on %s for %s", THIS->name, ((CWIDGET *)win)->name);
		GB.Unref(POINTER(&win->focus));
		win->focus = THIS;
		GB.Ref(THIS);
	}
}

BEGIN_METHOD_VOID(Control_SetFocus)

	set_focus(THIS);

END_METHOD


BEGIN_PROPERTY(Control_Tag)

	if (READ_PROPERTY)
		GB.ReturnVariant(&OBJECT(CWIDGET)->tag);
	else
		GB.StoreVariant(PROP(GB_VARIANT), (void *)&(OBJECT(CWIDGET)->tag));

END_METHOD


BEGIN_PROPERTY(Control_Mouse)

	QWidget *wid;
	int shape;

	HANDLE_PROXY(_object);
	
	wid = QWIDGET(_object);
	
	if (READ_PROPERTY)
	{
		if (wid->testAttribute(Qt::WA_SetCursor))
		{
			shape = wid->cursor().shape();
			if (shape == Qt::BitmapCursor)
				GB.ReturnInteger(CMOUSE_CUSTOM);
			else
				GB.ReturnInteger(shape);
		}
		else
			GB.ReturnInteger(CMOUSE_DEFAULT);
	}
	else
		set_mouse(wid, VPROP(GB_INTEGER), THIS->cursor);

END_METHOD


BEGIN_PROPERTY(Control_Cursor)

	HANDLE_PROXY(_object);
	
	if (READ_PROPERTY)
		GB.ReturnObject(THIS->cursor);
	else
	{
		GB.StoreObject(PROP(GB_OBJECT), &THIS->cursor);
		set_mouse(WIDGET, CMOUSE_CUSTOM, THIS->cursor);
	}

END_PROPERTY


#if 0

/*
static QColor get_background(CWIDGET *_object, QWidget *wid)
{
	QPalette pal(wid->palette());
	QColorGroup::ColorRole role = (QColorGroup::ColorRole)OBJECT(CWIDGET)->background;

	return pal.color(QPalette::Active, role);
}

static void test_color(CWIDGET *_object, QWidget *wid)
{
	QColor b, f, bp, fp;

	if (!wid->ownPalette() || !wid->parentWidget())
		return;

	f = wid->paletteForegroundColor();
	fp = wid->parentWidget()->paletteForegroundColor();

	if (f != fp)
		return;

	b = get_background(_object, wid);
	bp = get_background(CWidget::get(wid->parentWidget()), wid->parentWidget());

	if (b != bp)
		return;

	wid->unsetPalette();
}
*/

BEGIN_PROPERTY(CWIDGET_background)

	QWidget *wid = QWIDGET(_object);
	QPalette pal(wid->palette());
	QColorGroup::ColorRole role = (QColorGroup::ColorRole)OBJECT(CWIDGET)->background;

	//qDebug("bm = %d (%d %d)", wid->backgroundMode(), QWidget::PaletteButton, QWidget::PaletteBase);

	if (READ_PROPERTY)
		GB.ReturnInteger(pal.color(QPalette::Active, role).rgb() & 0xFFFFFF);
	else
	{
		pal.setColor(role, QColor((QRgb)VPROP(GB_INTEGER)));
		wid->setPalette(pal);
		//test_color((CWIDGET *)_object, wid);
	}

#if 0
	if (READ_PROPERTY)
		GB.ReturnInteger((int)(WIDGET->paletteBackgroundColor().rgb() & 0xFFFFFF));
	else
		WIDGET->setPaletteBackgroundColor(QColor((QRgb)PROPERTY(int)));
#endif

END_PROPERTY


BEGIN_PROPERTY(CWIDGET_foreground)

	QWidget *wid;

	if (GB.Is(_object, GB.FindClass("Container")))
		wid = ((CCONTAINER *)_object)->container;
	else
		wid = QWIDGET(_object);

	if (READ_PROPERTY)
	{
		GB.ReturnInteger((int)(wid->paletteForegroundColor().rgb() & 0xFFFFFF));
		return;
	}
	else
	{
		QPalette pal(QWIDGET(_object)->palette());

		pal.setColor(QColorGroup::Foreground, QColor((QRgb)VPROP(GB_INTEGER)));
		pal.setColor(QColorGroup::Text, QColor((QRgb)VPROP(GB_INTEGER)));
		wid->setPalette(pal);
		//test_color((CWIDGET *)_object, wid);
	}

END_PROPERTY
#endif

static QWidget *get_color_widget(QWidget *w)
{
	QWidget *view = get_viewport(w);
	if (view)
		return view;
	else
		return w;
}

/*int get_real_background(CWIDGET *_object)
{
	CWIDGET *parent = (CWIDGET *)CWIDGET_get_parent(THIS);
	if (THIS->bg == COLOR_DEFAULT && parent)
		return get_real_background(parent);
	else
		return THIS->bg;
}

int get_real_foreground(CWIDGET *_object)
{
	CWIDGET *parent = (CWIDGET *)CWIDGET_get_parent(THIS);
	if (THIS->fg == COLOR_DEFAULT && parent)
		return get_real_foreground(parent);
	else
		return THIS->fg;
}*/

void CWIDGET_reset_color(CWIDGET *_object)
{
	int fg, bg;
	QPalette palette;
	QWidget *w;
	
	HANDLE_PROXY(_object);
	//qDebug("reset_color: %s", THIS->name);
	//qDebug("set_color: (%s %p) bg = %08X (%d) fg = %08X (%d)", GB.GetClassName(THIS), THIS, THIS->bg, w->backgroundRole(), THIS->fg, w->foregroundRole());
	
	w = get_color_widget(WIDGET);
	
	if (THIS->bg == COLOR_DEFAULT && THIS->fg == COLOR_DEFAULT)
	{
		//CWIDGET *parent = (CWIDGET *)CWIDGET_get_parent(THIS);
		//if (parent)
		//	w->setPalette(parent->widget->palette());
		//else
		w->setPalette(QPalette());
		//WIDGET->setPalette(QPalette());
	}
	else
	{
		palette = QPalette(); //w->palette());
		bg = THIS->bg;
		fg = THIS->fg;
		
		if (bg != COLOR_DEFAULT)
			palette.setColor(w->backgroundRole(), QColor((QRgb)bg));
		
		if (fg != COLOR_DEFAULT)
		{
			palette.setColor(w->foregroundRole(), QColor((QRgb)fg));
			//palette.setColor(QPalette::Text, QColor((QRgb)fg));
			//palette.setColor(QPalette::WindowText, QColor((QRgb)fg));
			//palette.setColor(QPalette::ButtonText, QColor((QRgb)fg));
			/*palette.setColor(QPalette::WindowText, QColor((QRgb)fg));
			palette.setColor(QPalette::Text, QColor((QRgb)fg));
			palette.setColor(QPalette::ButtonText, QColor((QRgb)fg));*/
		}
			
		w->setPalette(palette);
		//WIDGET->setPalette(palette);
	}	
	
	w->setAutoFillBackground(!THIS->flag.noBackground && (THIS->flag.fillBackground || (THIS->bg != COLOR_DEFAULT && w->backgroundRole() == QPalette::Window)));
	//w->setAutoFillBackground(THIS->bg != COLOR_DEFAULT);
	
	if (GB.Is(THIS, CLASS_TextArea))
		CTEXTAREA_set_foreground(THIS);
	
	if (!GB.Is(THIS, CLASS_Container))
		return;
	
	if (GB.Is(THIS, CLASS_Window))
		CWINDOW_define_mask((CWINDOW *)THIS);
}

void CWIDGET_set_color(CWIDGET *_object, int bg, int fg)
{
	THIS->bg = bg;
	THIS->fg = fg;
	CWIDGET_reset_color(THIS);
}


int CWIDGET_get_background(CWIDGET *_object)
{
	return THIS->bg;
	/*
	QWidget *w = get_color_widget(WIDGET);
	
	if (THIS->flag.default_bg)
		return COLOR_DEFAULT;
	else
		return w->palette().color(w->backgroundRole()).rgb() & 0xFFFFFF;
	*/
}
	
int CWIDGET_get_foreground(CWIDGET *_object)
{
	return THIS->fg;
	/*
	QWidget *w = get_color_widget(WIDGET);
	
	if (THIS->flag.default_fg)
		return COLOR_DEFAULT;
	else
		return w->palette().color(w->foregroundRole()).rgb() & 0xFFFFFF;
	*/
}
	
BEGIN_PROPERTY(Control_Background)

	if (THIS->proxy)
	{
		if (READ_PROPERTY)
			GB.GetProperty(THIS->proxy, "Background");
		else
			GB.SetProperty(THIS->proxy, "Background", GB_T_INTEGER, VPROP(GB_INTEGER));
		
		return;
	}

	if (READ_PROPERTY)
		GB.ReturnInteger(CWIDGET_get_background(THIS));
	else
	{
		int col = VPROP(GB_INTEGER);
		if (col != CWIDGET_get_background(THIS))
			CWIDGET_set_color(THIS, col, CWIDGET_get_foreground(THIS));
	}

END_PROPERTY


BEGIN_PROPERTY(Control_Foreground)

	if (THIS->proxy)
	{
		if (READ_PROPERTY)
			GB.GetProperty(THIS->proxy, "Foreground");
		else
			GB.SetProperty(THIS->proxy, "Foreground", GB_T_INTEGER, VPROP(GB_INTEGER));
		
		return;
	}

	if (READ_PROPERTY)
		GB.ReturnInteger(CWIDGET_get_foreground(THIS));
	else
	{
		int col = VPROP(GB_INTEGER);
		if (col != CWIDGET_get_foreground(THIS))
			CWIDGET_set_color(THIS, CWIDGET_get_background(THIS), col);
	}

END_PROPERTY

BEGIN_PROPERTY(Control_Parent)

	GB.ReturnObject(CWIDGET_get_parent(THIS));

END_PROPERTY


BEGIN_PROPERTY(Control_Window)

	GB.ReturnObject(CWidget::getWindow(THIS));

END_PROPERTY


BEGIN_PROPERTY(Control_Id)

	GB.ReturnInteger((int)WIDGET->winId());

END_PROPERTY


/*static QString remove_ampersand(const QString &s)
{
	QString r;
	uint i;

	for (i = 0; i < s.length(); i++)
	{
		if (s[i] == '&')
		{
			i++;
			if (i < s.length())
				r += s[i];
		}
		else
		{
			r += s[i];
		}
	}

	return r;
}*/


BEGIN_PROPERTY(Control_Tooltip)

	//QWidget *w;

	if (READ_PROPERTY)
		GB.ReturnNewZeroString(TO_UTF8(WIDGET->toolTip()));
	else
		WIDGET->setToolTip(QSTRING_PROP());

END_PROPERTY


BEGIN_PROPERTY(Control_Name)

	if (READ_PROPERTY)
		GB.ReturnString(THIS->name);
	else
		set_name(THIS, GB.ToZeroString(PROP(GB_STRING)));

END_PROPERTY


BEGIN_PROPERTY(Control_Action)

	if (READ_PROPERTY)
		GB.ReturnString(THIS->action);
	else
	{
		char *action = PLENGTH() ? GB.NewString(PSTRING(), PLENGTH()) : NULL;
		CACTION_register(THIS, THIS->action, action);
		GB.FreeString(&THIS->action);
		THIS->action = action;
	}

END_PROPERTY


BEGIN_PROPERTY(Control_Proxy)

	if (READ_PROPERTY)
		GB.ReturnObject(THIS->proxy);
	else
		register_proxy(THIS, VPROP(GB_OBJECT));

END_PROPERTY


BEGIN_PROPERTY(Control_PopupMenu)

	if (READ_PROPERTY)
		GB.ReturnString(THIS->popup);
	else
		GB.StoreString(PROP(GB_STRING), &(THIS->popup));

END_PROPERTY


/*BEGIN_METHOD_VOID(Control_Screenshot)

	GB.ReturnObject(CPICTURE_grab(QWIDGET(_object)));

END_METHOD*/


BEGIN_METHOD(Control_Drag, GB_VARIANT data; GB_STRING format)

	GB.ReturnObject(CDRAG_drag(OBJECT(CWIDGET), &VARG(data), MISSING(format) ? NULL : ARG(format)));

END_METHOD


BEGIN_METHOD(Control_Reparent, GB_OBJECT container; GB_INTEGER x; GB_INTEGER y)

	QPoint p(WIDGET->pos());
	bool show;

	if (!MISSING(x) && !MISSING(y))
	{
		p.setX(VARG(x));
		p.setY(VARG(y));
	}

	if (GB.CheckObject(VARG(container)))
		return;

	show = is_visible(THIS);
	CWIDGET_set_visible(THIS, false);
	WIDGET->setParent(QCONTAINER(VARG(container)));
	WIDGET->move(p);
	CCONTAINER_insert_child(THIS);
	CWIDGET_set_visible(THIS, show);

END_METHOD


BEGIN_PROPERTY(Control_Drop)

	if (READ_PROPERTY)
		GB.ReturnBoolean(WIDGET->acceptDrops());
	else
	{
		WIDGET->setAcceptDrops(VPROP(GB_BOOLEAN));
		if (CWIDGET_test_flag(THIS, WF_SCROLLVIEW))
			get_viewport(WIDGET)->setAcceptDrops(VPROP(GB_BOOLEAN));
	}

END_PROPERTY


BEGIN_PROPERTY(Control_Tracking)

  if (READ_PROPERTY)
    GB.ReturnBoolean(THIS->flag.tracking);
  else
	{
		if (VPROP(GB_BOOLEAN) != THIS->flag.tracking)
		{
			THIS->flag.tracking = VPROP(GB_BOOLEAN);
			if (THIS->flag.tracking)
			{
				THIS->flag.old_tracking = WIDGET->hasMouseTracking();
				WIDGET->setMouseTracking(true);
			}
			else
			{
				WIDGET->setMouseTracking(THIS->flag.old_tracking);
			}
		}
	}
	
END_PROPERTY


BEGIN_PROPERTY(CWIDGET_border_full)

	QFrame *wid = (QFrame *)QWIDGET(_object);
	int border, lw;

	if (READ_PROPERTY)
	{
		if (wid->frameStyle() == (QFrame::Box + QFrame::Plain))
			border = BORDER_PLAIN;
		else if (wid->frameStyle() == (QFrame::StyledPanel + QFrame::Sunken))
			border = BORDER_SUNKEN;
		else if (wid->frameStyle() == (QFrame::StyledPanel + QFrame::Raised))
			border = BORDER_RAISED;
		else if (wid->frameStyle() == (QFrame::Box + QFrame::Sunken))
			border = BORDER_ETCHED;
		else
			border = BORDER_NONE;

		GB.ReturnInteger(border);
	}
	else
	{
		lw = 1;

		switch (VPROP(GB_INTEGER))
		{
			case BORDER_PLAIN: border = QFrame::Box + QFrame::Plain; break;
			case BORDER_SUNKEN: border = QFrame::StyledPanel + QFrame::Sunken; lw = 2; break;
			case BORDER_RAISED: border = QFrame::StyledPanel + QFrame::Raised; lw = 2; break;
			case BORDER_ETCHED: border = QFrame::Box + QFrame::Sunken; break;
			default: border = QFrame::NoFrame; break;
		}

		wid->setFrameStyle(border);
		wid->setLineWidth(lw);
		wid->update();
	}

END_PROPERTY


BEGIN_PROPERTY(CWIDGET_border_simple)

	QFrame *wid = (QFrame *)QWIDGET(_object);

	if (READ_PROPERTY)
	{
		GB.ReturnBoolean(wid->frameStyle() != QFrame::NoFrame);
	}
	else
	{
		//qDebug("frameStyle = %d", wid->frameStyle());

		if (VPROP(GB_BOOLEAN))
		{
			wid->setFrameStyle(QFrame::StyledPanel + QFrame::Sunken);
			//wid->setFrameStyle(QFrame::LineEditPanel);
			//wid->setLineWidth(2);
		}
		else
		{
			wid->setFrameStyle(QFrame::NoFrame);
		}

		//qDebug("--> %d", wid->frameStyle());

		wid->update();
	}

END_PROPERTY


BEGIN_PROPERTY(CWIDGET_scrollbar)

	QAbstractScrollArea *wid = qobject_cast<QAbstractScrollArea *>(WIDGET);
	Q3ScrollView *sw = qobject_cast<Q3ScrollView *>(WIDGET);
	int scroll;

	if (wid)
	{
		if (READ_PROPERTY)
		{
			scroll = 0;
			if (wid->horizontalScrollBarPolicy() == Qt::ScrollBarAsNeeded)
				scroll += 1;
			if (wid->verticalScrollBarPolicy() == Qt::ScrollBarAsNeeded)
				scroll += 2;
	
			GB.ReturnInteger(scroll);
		}
		else
		{
			scroll = VPROP(GB_INTEGER) & 3;
			wid->setHorizontalScrollBarPolicy( (scroll & 1) ? Qt::ScrollBarAsNeeded : Qt::ScrollBarAlwaysOff);
			wid->setVerticalScrollBarPolicy( (scroll & 2) ? Qt::ScrollBarAsNeeded : Qt::ScrollBarAlwaysOff);
		}
	}
	else if (sw)
	{
		if (READ_PROPERTY)
		{
			scroll = 0;
			if (sw->hScrollBarMode() == Q3ScrollView::Auto)
				scroll += 1;
			if (sw->vScrollBarMode() == Q3ScrollView::Auto)
				scroll += 2;
	
			GB.ReturnInteger(scroll);
		}
		else
		{
			scroll = VPROP(GB_INTEGER) & 3;
			sw->setHScrollBarMode( (scroll & 1) ? Q3ScrollView::Auto : Q3ScrollView::AlwaysOff);
			sw->setVScrollBarMode( (scroll & 2) ? Q3ScrollView::Auto : Q3ScrollView::AlwaysOff);
		}
	}

END_PROPERTY

void CWIDGET_grab(CWIDGET *_object)
{
	QEventLoop eventLoop;
	QEventLoop *old;
	
	if (THIS->flag.grab)
		return;
	
	THIS->flag.grab = true;
	WIDGET->grabMouse(WIDGET->cursor());
	WIDGET->grabKeyboard();

	old = MyApplication::eventLoop;
	MyApplication::eventLoop = &eventLoop;
	eventLoop.exec();
	MyApplication::eventLoop = old;
	
	WIDGET->releaseMouse();
	WIDGET->releaseKeyboard();
	THIS->flag.grab = false;

}

BEGIN_METHOD_VOID(Control_Grab)

	CWIDGET_grab(THIS);

END_METHOD


/* Classe CWidget */

CWidget CWidget::manager;
QHash<QObject *, CWIDGET *> CWidget::dict;
bool CWidget::real;

#if 0
bool haveChildren;

void CWidget::installFilter(QObject *o)
{
	QObjectList *children;
	QObject *child;

	children = (QObjectList *)(o->children());

	o->installEventFilter(&manager);

	if (!children)
		return;

	child = children->first();
	while (child)
	{
		if (child->isWidgetType())
		{
			haveChildren = true;
			CWidget::installFilter(child);
		}

		child = children->next();
	}
}

void CWidget::removeFilter(QObject *o)
{
	QObjectList *children = (QObjectList *)(o->children());
	QObject *child;

	if (!o->isWidgetType())
		return;

	o->removeEventFilter(&manager);

	if (!children)
		return;

	child = children->first();
	while (child)
	{
		CWidget::removeFilter(child);
		child = children->next();
	}
}
#endif

void CWidget::removeFocusPolicy(QWidget *w)
{
	QObjectList children;
	int i;
	QObject *child;

	w->clearFocus();
	w->setFocusPolicy(Qt::NoFocus);

	children = w->children();

	for (i = 0; i < children.count(); i++)
	{
		child = children.at(i);
		
		if (child->isWidgetType())
			CWidget::removeFocusPolicy((QWidget *)child);
	}
}


void CWidget::add(QObject *o, void *object, bool no_filter)
{
	//if (!no_filter)
	QObject::connect(o, SIGNAL(destroyed()), &manager, SLOT(destroy()));

	dict.insert(o, (CWIDGET *)object);

	/*
	if (!no_filter)
	{
		haveChildren = false;
		CWidget::installFilter(o);
		if (haveChildren)
			CWIDGET_set_flag(object, WF_NO_EVENT);
	}
	*/

	GB.Ref(object);
}

CWIDGET *CWidget::get(QObject *o)
{
	CWIDGET *ob;

	real = true;

	while (o)
	{
		ob = dict[o];
		if (ob)
			return ob;
		if (((QWidget *)o)->isWindow())
			return NULL;

		o = o->parent();
		real = false;
	}

	return NULL;
}

CWIDGET *CWidget::getRealExisting(QObject *o)
{
	CWIDGET *_object = dict[o];
	
	if (THIS && CWIDGET_test_flag(THIS, WF_DELETED))
		_object = 0;
	
	return _object;
}


CWIDGET *CWidget::getDesign(QObject *o)
{
	CWIDGET *ob;

	if (!o->isWidgetType())
		return NULL;

	real = true;

	while (o)
	{
		ob = dict[o];
		if (ob)
			break;
		if (((QWidget *)o)->isWindow())
			return NULL;

		o = o->parent();
		real = false;
	}

	if (!o)
		return NULL;

	if (!CWIDGET_test_flag(ob, WF_DESIGN))
		return ob;

	while (o)
	{
		ob = dict[o];
		if (ob && CWIDGET_test_flag(ob, WF_DESIGN_LEADER))
			return ob;
		if (((QWidget *)o)->isWindow())
			return NULL;

		o = o->parent();
		real = false;
	}

	return NULL;
}

/*
static void debugObject(void *ob)
{
	if (!ob)
		return;
	qDebug("  (%s %p) %s%s", ob ? GB.GetClassName(ob) : "", ob, CWIDGET_test_flag(ob, WF_DESIGN) ? "D" : "", CWIDGET_test_flag(ob, WF_DESIGN_LEADER) ? "L" : "");
}
*/

#if 0
static CWIDGET *getDesignDebug(QObject *o)
{
	CWIDGET *ob;

	if (!o->isWidgetType())
		return NULL;

	while (o)
	{
		ob = CWidget::getReal(o);
		debugObject(ob);
		if (ob)
			break;

		o = o->parent();
	}

	if (!o)
		return NULL;

	if (!CWIDGET_test_flag(ob, WF_DESIGN))
		return ob;

	while (o)
	{
		ob = CWidget::getReal(o);
		debugObject(ob);
		if (ob && CWIDGET_test_flag(ob, WF_DESIGN_LEADER))
			return ob;

		o = o->parent();
	}

	return NULL;
}
#endif

QWidget *CWidget::getContainerWidget(CCONTAINER *object)
{
	if (GB.CheckObject(object))
		GB.Propagate();

	if (object->container == NULL)
	{
		GB.Error("Null container");
		GB.Propagate();
	}

	//qDebug("Container = %p", object->container);

	return (object->container);
}

CWINDOW *CWidget::getWindow(CWIDGET *ob)
{
	//QWidget *p = w->parentWidget();
	for(;;)
	{
		if (GB.Is(ob, CLASS_Window)) // && ((CWINDOW *)ob)->window)
			break;

		ob = CWidget::get(QWIDGET(ob)->parentWidget());
		if (!ob)
			break;
	}

	return (CWINDOW *)ob;
}


CWINDOW *CWidget::getTopLevel(CWIDGET *ob)
{
	//QWidget *p = w->parentWidget();
	for(;;)
	{
		if (GB.Is(ob, CLASS_Window) && ((CWINDOW *)ob)->toplevel)
			break;

		ob = CWidget::get(QWIDGET(ob)->parentWidget());
		if (!ob)
			break;
	}

	return (CWINDOW *)ob;
}


#if 0
void CWidget::setName(CWIDGET *object, const char *name)
{
	QWidget *w = QWIDGET(object);
	CTOPLEVEL *top = (CTOPLEVEL *)CWidget::get(w->topLevelWidget());

	if (QWIDGET(top) == w)
		return;

	if (w->name() != NULL)
	{
		/*qDebug("- %s", w->name());*/
		top->dict->remove(w->name());
	}

	if (name != NULL)
	{
		top->dict->insert((const char *)name, object);
		w->setName(name);
		/*qDebug("+ %s", w->name());*/
	}
}
#endif

void CWidget::destroy()
{
	QWidget *w = (QWidget *)sender();
	CWIDGET *ob = CWidget::get(w);

	if (ob == NULL)
		return;

	//qDebug("CWidget::destroy: (%s %p) %s", GB.GetClassName(ob), ob, ob->name);
	
	if (CWIDGET_active_control == ob)
		CWIDGET_active_control = NULL;
	if (_old_active_control == ob)
		_old_active_control = NULL;
	if (_hovered == ob)
		_hovered = NULL;
	if (_official_hovered == ob)
		_official_hovered = NULL;
	
	CACTION_register(ob, ob->action, NULL);
	GB.FreeString(&ob->action);
	
	if (ob->proxy)
		((CWIDGET *)ob->proxy)->proxy_for = NULL;
	if (ob->proxy_for)
		((CWIDGET *)ob->proxy_for)->proxy = NULL;
	
	set_name(ob, 0);

	dict.remove(w);

	QWIDGET(ob) = NULL;
	GB.StoreVariant(NULL, &ob->tag);
	GB.Unref(POINTER(&ob->cursor));
	GB.Unref(POINTER(&ob->font));
	GB.FreeString(&ob->popup);
	
	//qDebug(">> CWidget::destroy %p (%p) :%p:%ld #2", ob, ob->widget, ob->ob.klass, ob->ob.ref);
	//if (!CWIDGET_test_flag(ob, WF_NODETACH))
	GB.Detach(ob);

	GB.Unref(POINTER(&ob));
}

/*static void post_dblclick_event(void *control)
{
	GB.Raise(control, EVENT_DblClick, 0);
	GB.Unref(&control);
}*/

static void post_focus_change(void *)
{
	CWIDGET *current;
	
	//qDebug("post_focus_change");

	for(;;)
	{
		current = CWIDGET_active_control;
		if (current == _old_active_control)
			break;

		if (_old_active_control)
		{
			GB.Raise(_old_active_control, EVENT_LostFocus, 0);
			// TODO: follow proxy chain
			if (_old_active_control->proxy_for)
				GB.Raise(_old_active_control->proxy_for, EVENT_LostFocus, 0);
		}
		
		_old_active_control = current;
		CWINDOW_activate(_old_active_control);
		
		if (current)
		{
			GB.Raise(current, EVENT_GotFocus, 0);
			// TODO: follow proxy chain
			if (current->proxy_for)
				GB.Raise(current->proxy_for, EVENT_GotFocus, 0);
		}
}
	
	_focus_change = FALSE;
}

static void handle_focus_change()
{
	if (_focus_change)
		return;
	
	_focus_change = TRUE;
	GB.Post((void (*)())post_focus_change, NULL);
}

void CWIDGET_handle_focus(CWIDGET *control, bool on) 
{
	if (on == (CWIDGET_active_control == control))
		return;
	
	//qDebug("CWIDGET_handle_focus: %p %s %d", control, control->name, on);
	CWIDGET_active_control = on ? control : NULL;
	handle_focus_change();
}

static bool raise_key_event_to_parent_window(void *control, int event)
{
	for(;;)
	{
		control = CWIDGET_get_parent(control);
		if (!control)
			break;
		control = CWidget::getWindow((CWIDGET *)control);
		if (GB.Raise(control, event, 0))
			return true;
	}
	
	return false;
}

bool CWidget::eventFilter(QObject *widget, QEvent *event)
{
	CWIDGET *control;
	int event_id;
	int type = event->type();
	bool real;
	bool design;
	bool original;
	bool cancel;
	QPoint p;
	void *jump;

	//if (widget->isA("MyMainWindow"))
	//	getDesignDebug(widget);
	switch (type)
	{
		case QEvent::Enter: 
			jump = &&__ENTER; break;
		case QEvent::Leave: 
			jump = &&__LEAVE; break;
		case QEvent::FocusIn:
			jump = &&__FOCUS_IN; break;
		case QEvent::FocusOut:
			jump = &&__FOCUS_OUT; break;
		case QEvent::ContextMenu:
			jump = &&__CONTEXT_MENU; break;
		case QEvent::MouseButtonPress:
		case QEvent::MouseButtonRelease:
		case QEvent::MouseMove:
		case QEvent::MouseButtonDblClick:
			jump = &&__MOUSE; break;
			//jump = &&__DBL_CLICK; break;
		case QEvent::KeyPress:
		case QEvent::KeyRelease:
			jump = &&__KEY; break;
		case QEvent::Shortcut:
			jump = &&_DESIGN; break;
		case QEvent::InputMethod:
			jump = &&__INPUT_METHOD; break;
		case QEvent::Wheel:
			jump = &&__MOUSE_WHEEL; break;
		case QEvent::DragEnter:
			jump = &&__DRAG_ENTER; break;
		case QEvent::DragMove:
			jump = &&__DRAG_MOVE; break;
		case QEvent::Drop:
			jump = &&__DROP; break;
		case QEvent::DragLeave:
			jump = &&__DRAG_LEAVE; break;
		case QEvent::DeferredDelete:
			control = CWidget::getDesign(widget);
			if (!control || CWIDGET_test_flag(control, WF_DELETED))
			{
				QObject::eventFilter(widget, event); 
				return false;
			}
			else
				goto _STANDARD;
		default:
			goto _STANDARD;
	}
	
	control = CWidget::getDesign(widget);
	//for(;;)
	//{
		if (!control || GB.Is(control, CLASS_Menu))
			goto _STANDARD;
	//	if (control->widget->isEnabled())
	//		break;
	//	control = (CWIDGET *)CWIDGET_get_parent(control);
	//}

	real = CWidget::real;
	design = CWIDGET_test_flag(control, WF_DESIGN); // && !GB.Is(control, CLASS_Container);
	original = event->spontaneous();
	
	goto *jump;

	/*if (type != QEvent::Enter)
	{
		if (_control_leave)
		{
			save = _control_leave;
			_control_leave = NULL;
			
			if (type != QEvent::Leave || save != control)
				GB.Raise(save, EVENT_Leave, 0);
		}
	}*/
	
	__ENTER:
	{
		_hovered = control;
		if (real && !qApp->activePopupWidget())
		{
			_official_hovered = control;
			GB.Raise(control, EVENT_Enter, 0);
		}
		goto __NEXT;
	}

	__LEAVE:
	{
		if (real && !qApp->activePopupWidget())
			GB.Raise(control, EVENT_Leave, 0);
		goto __NEXT;
	}
	
  __FOCUS_IN:
  {
		CWIDGET_handle_focus(control, true);
		goto __NEXT;
  }
  
  __FOCUS_OUT:
  {
		CWIDGET_handle_focus(control, false);
		goto __NEXT;
  }
  
	__CONTEXT_MENU:
	{
		while (control->proxy_for)
			control = (CWIDGET *)control->proxy_for;

	__MENU_TRY_PROXY:
	
		// if (real && GB.CanRaise(control, EVENT_Menu))
		//qDebug("Menu event! %p %d", control, EVENT_Menu);
		if (GB.CanRaise(control, EVENT_Menu))
		{
			((QContextMenuEvent *)event)->accept();
			GB.Raise(control, EVENT_Menu, 0);
			return true;
		}
		if (control->popup)
		{
			CWINDOW *window = CWidget::getWindow(control);
			CMENU *menu = CWindow::findMenu(window, control->popup);
			if (menu)
				CMENU_popup(menu, QCursor::pos());
			return true;
		}

		if (control->proxy)
		{
			control = (CWIDGET *)control->proxy;
			goto __MENU_TRY_PROXY;
		}
		
		goto __NEXT;		
	}
	
	__MOUSE:
	{
		QMouseEvent *mevent = (QMouseEvent *)event;

		if (!original)
			goto _DESIGN;

		/*if (type == QEvent::MouseButtonPress)
		{
			qDebug("mouse event on [%s %s %p] (%s %p) %s%s%s", widget->metaObject()->className(), qPrintable(widget->objectName()), widget, 
						 control ? GB.GetClassName(control) : "-", control, real ? "REAL " : "", design ? "DESIGN " : "", original ? "ORIGINAL ": "");
			//getDesignDebug(widget);
		}*/
		
		if (!real)
		{
			CWIDGET *cont = CWidget::get(widget);
			if (CWIDGET_test_flag(cont, WF_SCROLLVIEW))
			{
				if (qobject_cast<QScrollBar *>(widget))
					goto _STANDARD;
				/*if (widget != get_viewport(QWIDGET(cont)))
				{
					if (!widget->objectName().isNull())
						goto _STANDARD;
				}*/
			}
		}
		
		//while (control->proxy_for)
		//	control = (CWIDGET *)control->proxy_for;

	__MOUSE_TRY_PROXY:
	
		p.setX(mevent->globalX());
		p.setY(mevent->globalY());
		p = QWIDGET(control)->mapFromGlobal(p);
		
		if (type == QEvent::MouseButtonPress)
		{
			//qDebug("MouseDown on %p (%s %p) %s%s", widget, control ? GB.GetClassName(control) : "-", control, real ? "REAL " : "", design ? "DESIGN " : "");

			event_id = EVENT_MouseDown;
			//state = mevent->buttons();
			
			CMOUSE_info.sx = p.x();
			CMOUSE_info.sy = p.y();
			//qDebug("MouseEvent: %d %d", mevent->x(), mevent->y());
		}
		else if (type == QEvent::MouseButtonDblClick)
		{
			event_id = EVENT_DblClick;
		}
		else
		{
			event_id = (type == QEvent::MouseButtonRelease) ? EVENT_MouseUp : EVENT_MouseMove;
			//state = mevent->buttons();
		}

		if (event_id == EVENT_MouseMove && mevent->buttons() == Qt::NoButton && !QWIDGET(control)->hasMouseTracking())
			goto _DESIGN;


		/* GB.Raise() can free the control, so we must reference it as we may raise two successive events now */
		GB.Ref(control);
		cancel = false;

		if (GB.CanRaise(control, event_id))
		{
			/*if (!design && CWIDGET_test_flag(control, WF_SCROLLVIEW))
			{
				if (widget != ((QScrollView *)QWIDGET(control))->viewport()
						&& widget->name(0))
				{
					qDebug("cancel");
					goto _DESIGN;
				}
			}*/
			
			CMOUSE_clear(true);
			CMOUSE_info.x = p.x();
			CMOUSE_info.y = p.y();
			CMOUSE_info.button = mevent->buttons() | mevent->button();
			CMOUSE_info.modifier = mevent->modifiers();

			cancel = GB.Raise(control, event_id, 0); //, GB_T_INTEGER, p.x(), GB_T_INTEGER, p.y(), GB_T_INTEGER, state);

			CMOUSE_clear(false);
			
			/*if (CDRAG_dragging)
				return true;*/
		}
		
		if (event_id == EVENT_MouseMove && !cancel && (mevent->buttons() != Qt::NoButton) && GB.CanRaise(control, EVENT_MouseDrag) && !CDRAG_dragging
				&& ((abs(p.x() - CMOUSE_info.sx) + abs(p.y() - CMOUSE_info.sy)) > 8)) // QApplication::startDragDistance()))
		{		
			/*if (!design && CWIDGET_test_flag(control, WF_SCROLLVIEW))
			{
				if (widget != ((QScrollView *)QWIDGET(control))->viewport()
						&& widget->name(0))
				{
					goto _DESIGN;
				}
			}*/
			
			CMOUSE_clear(true);
			CMOUSE_info.x = p.x();
			CMOUSE_info.y = p.y();
			CMOUSE_info.button = mevent->buttons();
			CMOUSE_info.modifier = mevent->modifiers();
		
			cancel = GB.Raise(control, EVENT_MouseDrag, 0);
			
			CMOUSE_clear(false);
		}
	
		GB.Unref(POINTER(&control));
		
		if (control->flag.grab && event_id == EVENT_MouseUp)
			MyApplication::eventLoop->exit();
		
		if (cancel)
			return true;
		
		if (control->proxy_for)
		{
			control = (CWIDGET *)control->proxy_for;
			goto __MOUSE_TRY_PROXY;
		}
		
		goto __NEXT;
	}
	
	/*
	__DBL_CLICK:
	{
		if (!original)
			goto _DESIGN;

		//GB.Raise(control, EVENT_DblClick, 0);
		if (GB.CanRaise(control, EVENT_DblClick))
		{
			GB.Ref(control);
			GB.Post((void (*)())post_dblclick_event, (intptr_t)control);
		}
		goto __NEXT;
	}
	*/
	
	__KEY:
	{
		QKeyEvent *kevent = (QKeyEvent *)event;

		#if QT_VERSION <= 0x030005
		if (!real || !original)
			goto _DESIGN;
		#endif

		event_id = (type == QEvent::KeyRelease) ? EVENT_KeyRelease : EVENT_KeyPress;
		cancel = false;

		#if QT_VERSION > 0x030005
		if (!original && type != QEvent::InputMethod)
			goto _DESIGN; //_ACCEL;
		#endif

		/*qDebug("QKeyEvent: %s: (%s %s) window:%d -> %d %s",
			type == QEvent::KeyPress ? "KeyPress" : "KeyRelease",
			GB.GetClassName(control), control->name,
			((QWidget *)widget)->isWindow(),  
			kevent->key(), (const char *)kevent->text().toLatin1());*/

		//qDebug("CWidget::eventFilter: KeyPress on %s %p", GB.GetClassName(control), control);
			
	__KEY_TRY_PROXY:
			
		CKEY_clear(true);

		GB.FreeString(&CKEY_info.text);
		CKEY_info.text = GB.NewZeroString(TO_UTF8(kevent->text()));
		CKEY_info.state = kevent->modifiers();
		CKEY_info.code = kevent->key();
		CKEY_info.release = type == QEvent::KeyRelease;
		
		#ifndef NO_X_WINDOW
		if (type == QEvent::KeyPress && CKEY_info.code)
			_x11_to_qt_keycode.insert(MAIN_x11_last_key_code, CKEY_info.code);
		else if (type == QEvent::KeyRelease && CKEY_info.code == 0)
		{
			if (_x11_to_qt_keycode.contains(MAIN_x11_last_key_code))
			{
				CKEY_info.code = _x11_to_qt_keycode[MAIN_x11_last_key_code];
				_x11_to_qt_keycode.remove(MAIN_x11_last_key_code);
			}
		}
		#endif
		
		if (!cancel)
			cancel = raise_key_event_to_parent_window(control, event_id);
		
		if (!cancel)
			cancel = GB.Raise(control, event_id, 0);

		CKEY_clear(false);

		if (cancel && (type != QEvent::KeyRelease))
			return true;

		if (control->proxy_for)
		{
			control = (CWIDGET *)control->proxy_for;
			goto __KEY_TRY_PROXY;
		}
		
		if (control->flag.grab && event_id == EVENT_KeyPress && kevent->key() == Qt::Key_Escape)
			MyApplication::eventLoop->exit();

		goto __NEXT;
	}
	
	__INPUT_METHOD:
	{
		QInputMethodEvent *imevent = (QInputMethodEvent *)event;

		#if QT_VERSION <= 0x030005
		if (!real || !original)
			goto _DESIGN;
		#endif

		if (!imevent->commitString().isEmpty())
		{

			// 		qDebug("QIMEvent: IMEnd (%s %p) (%s %p) TL:%d",
			// 			widget->className(), widget, GB.GetClassName(control), control,
			// 			((QWidget *)widget)->isWindow());
	
			event_id = EVENT_KeyPress;
			cancel = false;
			
		__IM_TRY_PROXY:
	
			if (GB.CanRaise(control, event_id))
			{
				CKEY_clear(true);
	
				GB.FreeString(&CKEY_info.text);
				//qDebug("IMEnd: %s", imevent->text().latin1());
				CKEY_info.text = GB.NewZeroString(TO_UTF8(imevent->commitString()));
				CKEY_info.state = 0;
				CKEY_info.code = 0;
	
				if (control->proxy_for)
					cancel = GB.Raise(control->proxy_for, event_id, 0);
				if (!cancel)
					cancel = GB.Raise(control, event_id, 0);
	
				CKEY_clear(false);
	
				if (cancel)
					return true;
			}

			if (control->proxy_for)
			{
				control = (CWIDGET *)control->proxy_for;
				goto __IM_TRY_PROXY;
			}
		}
		
		goto __NEXT;
	}
	
	__MOUSE_WHEEL:
	{
		QWheelEvent *ev = (QWheelEvent *)event;

		//qDebug("Event on %p %s%s%s", widget,
		//  real ? "REAL " : "", design ? "DESIGN " : "", child ? "CHILD " : "");

		if (!original)
			goto _DESIGN;

	__MOUSE_WHEEL_TRY_PROXY:
		
		if (GB.CanRaise(control, EVENT_MouseWheel))
		{
			// Automatic focus for wheel events
			set_focus(control);
			
			p.setX(ev->x());
			p.setY(ev->y());

			p = ((QWidget *)widget)->mapTo(QWIDGET(control), p);

			CMOUSE_clear(true);
			CMOUSE_info.x = p.x();
			CMOUSE_info.y = p.y();
			CMOUSE_info.button = ev->buttons();
			CMOUSE_info.modifier = ev->modifiers();
			CMOUSE_info.orientation = ev->orientation();
			CMOUSE_info.delta = ev->delta();

			cancel = GB.Raise(control, EVENT_MouseWheel, 0);

			CMOUSE_clear(false);
			
			if (cancel)
				return true;
		}
		
		if (control->proxy_for)
		{
			control = (CWIDGET *)control->proxy_for;
			goto __MOUSE_WHEEL_TRY_PROXY;
		}
		
		goto __NEXT;
	}
	
	__DRAG_ENTER:
	{
		//if (!CWIDGET_test_flag(control, WF_NO_DRAG))
		if (CDRAG_drag_enter((QWidget *)widget, control, (QDropEvent *)event))
		{
			if (!((QDropEvent *)event)->isAccepted())
				CDRAG_hide_frame(control);
			return true;
		}
		goto __NEXT;
	}
	
	__DRAG_MOVE:
	{
		//if (!CWIDGET_test_flag(control, WF_NO_DRAG))
		if (CDRAG_drag_move((QWidget *)widget, control, (QDropEvent *)event))
		{
			if (!((QDropEvent *)event)->isAccepted())
				CDRAG_hide_frame(control);
			return true;
		}
		goto __NEXT;
	}
	
	__DROP:
	{
		//if (!CWIDGET_test_flag(control, WF_NO_DRAG))
		if (CDRAG_drag_drop((QWidget *)widget, control, (QDropEvent *)event))
			return true;
		goto __NEXT;
	}
	
	__DRAG_LEAVE:
	{
		CDRAG_hide_frame(control);
		goto __NEXT;
	}
	
	__NEXT:
	
	if (!control || CWIDGET_test_flag(control, WF_DELETED))
	{
		QObject::eventFilter(widget, event); 
		return (type != QEvent::DeferredDelete);
	}
	
	/*if (CWIDGET_check(control))
	{
		qDebug("CWidget::eventFilter: %p was destroyed", control);
		return true;
	}*/

_DESIGN:

	if (design)
	{
		if ((type == QEvent::MouseButtonPress)
				|| (type == QEvent::MouseButtonRelease)
				|| (type == QEvent::MouseButtonDblClick)
				|| (type == QEvent::MouseMove)
				|| (type == QEvent::Wheel)
				|| (type == QEvent::ContextMenu)
				|| (type == QEvent::KeyPress)
				|| (type == QEvent::KeyRelease)
				|| (type == QEvent::InputMethod)
				|| (type == QEvent::Shortcut)
				|| (type == QEvent::Enter)
				|| (type == QEvent::Leave)
				|| (type == QEvent::FocusIn)
				|| (type == QEvent::FocusOut)
				|| (type == QEvent::DragEnter)
				|| (type == QEvent::DragMove)
				|| (type == QEvent::DragLeave)
				|| (type == QEvent::Drop)
				)
		return true;
	}

_STANDARD:

	return QObject::eventFilter(widget, event);    // standard event processing
}

/** Action *****************************************************************/

#define HAS_ACTION(_control) CWIDGET_test_flag((CWIDGET *)(_control), WF_ACTION)
#define SET_ACTION(_control, _flag) \
	if (_flag) \
		CWIDGET_set_flag((CWIDGET *)(_control), WF_ACTION); \
	else \
		CWIDGET_clear_flag((CWIDGET *)(_control), WF_ACTION);

#include "gb.form.action.h"

#if 0
static void gray_image(QImage &img)
{
	register uchar *b(img.bits());
	register uchar *g(img.bits() + 1);
	register uchar *r(img.bits() + 2);

	uchar * end(img.bits() + img.numBytes());

	while (b != end) {

			*b = *g = *r = 0x80 | (((*r + *b) >> 1) + *g) >> 2; // (r + b + g) / 3

			b += 4;
			g += 4;
			r += 4;
	}
}
#endif

void CWIDGET_iconset(QIcon &icon, const QPixmap &pixmap, int size)
{
	QImage img;
	//QPixmap disabled;
	QPixmap normal;

	if (pixmap.isNull())
		return;
	
	if (size > 0)
	{
		img = pixmap.toImage().convertToFormat(QImage::Format_ARGB32);
		size = ((size + 1) & ~3);
		img = img.scaled(size, size, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
		normal = QPixmap::fromImage(img);
	}
	else
		normal = pixmap;
		
	icon = QIcon(normal);
	
	/*gray_image(img);
	
	disabled.convertFromImage(img);
	icon.setPixmap(disabled, QIcon::Small, QIcon::Disabled);*/
}


GB_DESC CControlDesc[] =
{
	GB_DECLARE("Control", sizeof(CCONTROL)), GB_NOT_CREATABLE(),

	GB_HOOK_CHECK(CWIDGET_check),

	GB_METHOD("_free", NULL, Control_Delete, NULL),

	GB_METHOD("Move", NULL, Control_Move, "(X)i(Y)i[(Width)i(Height)i]"),
	GB_METHOD("Resize", NULL, Control_Resize, "(Width)i(Height)i"),

	GB_METHOD("MoveScaled", NULL, Control_MoveScaled, "(X)f(Y)f[(Width)f(Height)f]"),
	GB_METHOD("ResizeScaled", NULL, Control_ResizeScaled, "(Width)f(Height)f"),

	GB_METHOD("Delete", NULL, Control_Delete, NULL),
	GB_METHOD("Show", NULL, Control_Show, NULL),
	GB_METHOD("Hide", NULL, Control_Hide, NULL),

	GB_METHOD("Raise", NULL, Control_Raise, NULL),
	GB_METHOD("Lower", NULL, Control_Lower, NULL),

	GB_PROPERTY("Next", "Control", Control_Next),
	GB_PROPERTY("Previous", "Control", Control_Previous),

	GB_METHOD("SetFocus", NULL, Control_SetFocus, NULL),
	GB_METHOD("Refresh", NULL, Control_Refresh, NULL),
	//GB_METHOD("Screenshot", "Picture", Control_Screenshot, NULL),
	GB_METHOD("Drag", "Control", Control_Drag, "(Data)v[(Format)s]"),
	GB_METHOD("Grab", NULL, Control_Grab, NULL),

	GB_METHOD("Reparent", NULL, Control_Reparent, "(Parent)Container;[(X)i(Y)i]"),

	GB_PROPERTY("X", "i", Control_X),
	GB_PROPERTY("Y", "i", Control_Y),
	GB_PROPERTY_READ("ScreenX", "i", Control_ScreenX),
	GB_PROPERTY_READ("ScreenY", "i", Control_ScreenY),
	GB_PROPERTY("W", "i", Control_Width),
	GB_PROPERTY("H", "i", Control_Height),
	GB_PROPERTY("Left", "i", Control_X),
	GB_PROPERTY("Top", "i", Control_Y),
	GB_PROPERTY("Width", "i", Control_Width),
	GB_PROPERTY("Height", "i", Control_Height),

	GB_PROPERTY("Visible", "b", Control_Visible),
	GB_PROPERTY("Enabled", "b", Control_Enabled),
	GB_PROPERTY_READ("HasFocus", "b", Control_HasFocus),
	GB_PROPERTY_READ("Hovered", "b", Control_Hovered),
	
	GB_PROPERTY("Expand", "b", Control_Expand),
	GB_PROPERTY("Ignore", "b", Control_Ignore),

	GB_PROPERTY("Font", "Font", Control_Font),
	GB_PROPERTY("Background", "i", Control_Background),
	GB_PROPERTY("Foreground", "i", Control_Foreground),

	GB_PROPERTY("Design", "b", Control_Design),
	GB_PROPERTY("Name", "s", Control_Name),
	GB_PROPERTY("Tag", "v", Control_Tag),
  GB_PROPERTY("Tracking", "b", Control_Tracking),
	GB_PROPERTY("Mouse", "i", Control_Mouse),
	GB_PROPERTY("Cursor", "Cursor", Control_Cursor),
	GB_PROPERTY("Tooltip", "s", Control_Tooltip),
	GB_PROPERTY("Drop", "b", Control_Drop),
	GB_PROPERTY("Action", "s", Control_Action),
	GB_PROPERTY("PopupMenu", "s", Control_PopupMenu),
	GB_PROPERTY("Proxy", "Control", Control_Proxy),

	GB_PROPERTY_READ("Parent", "Container", Control_Parent),
	GB_PROPERTY_READ("Window", "Window", Control_Window),
	GB_PROPERTY_READ("Id", "i", Control_Id),
	GB_PROPERTY_READ("Handle", "i", Control_Id),

	GB_EVENT("Enter", NULL, NULL, &EVENT_Enter),
	GB_EVENT("GotFocus", NULL, NULL, &EVENT_GotFocus),
	GB_EVENT("LostFocus", NULL, NULL, &EVENT_LostFocus),
	GB_EVENT("KeyPress", NULL, NULL, &EVENT_KeyPress),
	GB_EVENT("KeyRelease", NULL, NULL, &EVENT_KeyRelease),
	GB_EVENT("Leave", NULL, NULL, &EVENT_Leave),
	GB_EVENT("MouseDown", NULL, NULL, &EVENT_MouseDown),
	GB_EVENT("MouseMove", NULL, NULL, &EVENT_MouseMove),
	GB_EVENT("MouseDrag", NULL, NULL, &EVENT_MouseDrag),
	GB_EVENT("MouseUp", NULL, NULL, &EVENT_MouseUp),
	GB_EVENT("MouseWheel", NULL, NULL, &EVENT_MouseWheel),
	GB_EVENT("DblClick", NULL, NULL, &EVENT_DblClick),
	GB_EVENT("Menu", NULL, NULL, &EVENT_Menu),
	GB_EVENT("Drag", NULL, NULL, &EVENT_Drag),
	GB_EVENT("DragMove", NULL, NULL, &EVENT_DragMove),
	GB_EVENT("Drop", NULL, NULL, &EVENT_Drop),

	CONTROL_DESCRIPTION,

	GB_END_DECLARE
};



