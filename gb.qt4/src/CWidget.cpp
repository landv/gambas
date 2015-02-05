/***************************************************************************

  CWidget.cpp

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
#include <QAbstractEventDispatcher>
#include <QListWidget>
#include <QComboBox>
#include <QSpinBox>
#include <QSet>
#include <QScrollBar>
#include <QLineEdit>
 
#ifndef NO_X_WINDOW
static QMap<int, int> _x11_to_qt_keycode;
#endif

CWIDGET *CWIDGET_active_control = 0;
CWIDGET *CWIDGET_previous_control = 0;
static bool _focus_change = false;
static CWIDGET *_old_active_control = 0;
int CCONTROL_last_event_type = 0;

static CWIDGET *_hovered = 0;
static CWIDGET *_official_hovered = 0;

QSet<CWIDGET *> *_enter_leave_set = NULL;

static QT_COLOR_FUNC _after_set_color = NULL;

#define EXT(_ob) (((CWIDGET *)_ob)->ext)

#define ENSURE_EXT(_ob) (EXT(_ob) ? EXT(_ob) : alloc_ext((CWIDGET *)(_ob)))

#define HANDLE_PROXY(_ob) \
	while (EXT(_ob) && EXT(_ob)->proxy) \
		_ob = (typeof _ob)(EXT(_ob)->proxy);

static CWIDGET_EXT *alloc_ext(CWIDGET *_object)
{
	GB.Alloc(POINTER(&(THIS->ext)), sizeof(CWIDGET_EXT));
	CLEAR(THIS->ext);
	THIS_EXT->bg = COLOR_DEFAULT;
	THIS_EXT->fg = COLOR_DEFAULT;
	THIS_EXT->tag.type = GB_T_NULL;
	return THIS->ext;
}
	
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

void CWIDGET_set_name(CWIDGET *_object, const char *name)
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

int CWIDGET_get_handle(void *_object)
{
	return (int)WIDGET->winId();
}

bool CWIDGET_is_visible(void *_object)
{
	return THIS->flag.visible; // || !QWIDGET(_object)->isHidden();
}

void CWIDGET_register_proxy(void *_object, void *proxy)
{
	void *check = proxy;

	while (check)
	{
		if (check == THIS)
		{
			GB.Error("Circular proxy chain");	
			return;
		}
		check = EXT(check) ? EXT(check)->proxy : NULL;
	}
	
	if (THIS_EXT && THIS_EXT->proxy && EXT(THIS_EXT->proxy))
		EXT(THIS_EXT->proxy)->proxy_for = NULL;
	
	if (proxy)
		ENSURE_EXT(THIS)->proxy = proxy;
	else if (EXT(THIS))
		EXT(THIS)->proxy = NULL;
	
	if (proxy)
		ENSURE_EXT(proxy)->proxy_for = THIS;
}

int CWIDGET_check(void *_object)
{
	return WIDGET == NULL || CWIDGET_test_flag(THIS, WF_DELETED);
}

static QWidget *get_viewport(QWidget *w)
{
	if (qobject_cast<QAbstractScrollArea *>(w))
		return ((QAbstractScrollArea *)w)->viewport();
	//else if (qobject_cast<Q3ScrollView *>(w))
	//	return ((Q3ScrollView *)w)->viewport();
	//else if (qobject_cast<Q3ListView *>(w))
	//	return ((Q3ListView *)w)->viewport();
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
	static int n = 0;
	char *name = GB.GetLastEventName();
	
	if (!name)
	{
		char buffer[16];
		n++;
		sprintf(buffer, "#%d", n);
		CWIDGET_set_name(THIS, buffer);
	}
	else
		CWIDGET_set_name(THIS, name);
}

bool CWIDGET_container_for(void *_object, void *container_for)
{
	if (THIS_EXT)
	{
		if (container_for)
		{
			if (!THIS_EXT->container_for)
			{
				THIS_EXT->container_for = container_for;
				return false;
			}
		}
		else
		{
			THIS_EXT->container_for = NULL;
			return false;
		}
	}
	else
	{
		if (container_for)
			ENSURE_EXT(THIS)->container_for = container_for;
		return false;
	}
	
	return true;
}

static void CWIDGET_enter(void *_object)
{
	if (!THIS->flag.inside)
	{
		THIS->flag.inside = true;
		GB.Raise(THIS, EVENT_Enter, 0);
	}
}

static void CWIDGET_leave(void *_object)
{
	if (THIS->flag.inside)
	{
		THIS->flag.inside = false;
		GB.Raise(THIS, EVENT_Leave, 0);
	}
}

bool CWIDGET_get_allow_focus(void *_object)
{
	return WIDGET->focusPolicy() != Qt::NoFocus;
}

void CWIDGET_set_allow_focus(void *_object, bool f)
{
	if (f)
	{
		WIDGET->setFocusPolicy(GB.CanRaise(THIS, EVENT_MouseWheel) ? Qt::WheelFocus : Qt::StrongFocus);
		WIDGET->setAttribute(Qt::WA_InputMethodEnabled, true);
	}
	else
	{
		WIDGET->setFocusPolicy(Qt::NoFocus);
	}
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
		CWIDGET_init_name(THIS);	

	if (qobject_cast<QAbstractScrollArea *>(w)) // || qobject_cast<Q3ScrollView *>(w))
		CWIDGET_set_flag(THIS, WF_SCROLLVIEW);

	//w->setAttribute(Qt::WA_PaintOnScreen, true);
	
	CWIDGET_reset_color(THIS); //w->setPalette(QApplication::palette());
	
	//THIS->flag.fillBackground = GB.Is(THIS, CLASS_Container);
	//w->setAutoFillBackground(THIS->flag.fillBackground);
	
	if (!no_show)
	{
		w->setGeometry(-16, -16, 8, 8);
		CWIDGET_set_visible(THIS, true);
		w->raise();
	}
	
	CCONTAINER_insert_child(THIS);
}


QString CWIDGET_Utf8ToQString(GB_STRING *str)
{
	return QString::fromUtf8((const char *)(str->value.addr + str->value.start), str->value.len);
}

void *CWIDGET_enter_popup()
{
	void *save = _enter_leave_set;
	
	_enter_leave_set = new QSet<CWIDGET *>;
	return save;
}

void CWIDGET_leave_popup(void *save)
{
	CWIDGET *_object;
	QSetIterator<CWIDGET *> i(*_enter_leave_set);
	
	while (i.hasNext())
	{
		_object = i.next();
		GB.Unref(POINTER(&_object));
		if (_object)
		{
			if (THIS->flag.inside_later != THIS->flag.inside)
			{
				if (THIS->flag.inside_later)
					CWIDGET_enter(THIS);
				else
					CWIDGET_leave(THIS);
			}
		}
	}
	
	delete _enter_leave_set;
	_enter_leave_set = (QSet<CWIDGET *>*) save;
}

static void insert_enter_leave_event(CWIDGET *control, bool in)
{
	control->flag.inside_later = in;
	
	if (_enter_leave_set->contains(control))
		return;
	
	_enter_leave_set->insert(control);
	GB.Ref(control);
}

static bool _post_check_hovered = false;
static CWIDGET *_post_check_hovered_window = NULL;

static void post_check_hovered(intptr_t)
{
	CWIDGET *_object = _post_check_hovered_window;
	
	if (THIS && WIDGET)
	{
		//qDebug("post_check_hovered");
		const QPoint globalPos(QCursor::pos());
		QPoint pos = WIDGET->mapFromGlobal(globalPos);
		_hovered = CWidget::getRealExisting(WIDGET->childAt(pos));
		if (_hovered)
			CWIDGET_enter(_hovered);
	}
	
	_post_check_hovered = false;
}

void CWIDGET_destroy(CWIDGET *_object)
{
	if (!THIS || !WIDGET)
		return;

	if (CWIDGET_test_flag(THIS, WF_DELETED))
		return;
	
	if (THIS->flag.dragging)
	{
		GB.Error("Control is being dragged");
		return;
	}

	//qDebug("CWIDGET_destroy: %s %p", GB.GetClassName(THIS), THIS);

	CWIDGET_set_visible(THIS, false);
	CWIDGET_set_flag(THIS, WF_DELETED);

	WIDGET->deleteLater();
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

void CWIDGET_check_visibility(CWIDGET *_object)
{
	if (!THIS->flag.resized)
	{
		THIS->flag.resized = TRUE;
		//qDebug("CWIDGET_check_visibility: %s %s %d", GB.GetClassName(THIS), THIS->name, THIS->flag.visible);
		CWIDGET_set_visible(THIS, THIS->flag.visible);
	}
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
	
	if (!THIS->flag.ignore)
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

	CWIDGET_check_visibility(THIS);

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

	CWIDGET_check_visibility(THIS);

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

#if 0
void CWIDGET_check_hovered()
{
	//qDebug("CWIDGET_check_hovered: %p %s -> %p %s", _hovered, _hovered ? _hovered->name : 0, _official_hovered, _official_hovered ? _official_hovered->name : 0);
	
	if (_official_hovered != _hovered)
	{
		if (_official_hovered)
			CWIDGET_leave(_official_hovered);
		
		if (_hovered)
			CWIDGET_enter(_hovered);
		
		_official_hovered = _hovered;
	}
}
#endif

bool CWIDGET_is_design(CWIDGET *_object)
{
	return CWIDGET_test_flag(THIS, WF_DESIGN) || CWIDGET_test_flag(THIS, WF_DESIGN_LEADER);
}

//---------------------------------------------------------------------------

BEGIN_METHOD_VOID(Control_new)

	MAIN_CHECK_INIT();

END_METHOD

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

void *CWIDGET_get_real_font(CWIDGET *_object)
{
	if (THIS->font)
		return CFONT_create(*((CFONT *)THIS->font)->font);

	CWIDGET *parent = (CWIDGET *)CWIDGET_get_parent(THIS);
	if (parent)
		return CWIDGET_get_real_font(parent);
	else
		return CFONT_create(qApp->font());
}

BEGIN_PROPERTY(Control_Font)

	CFONT *font;
	
	if (!THIS->font)
	{
		THIS->font = CFONT_create(WIDGET->font(), 0, THIS);
		GB.Ref(THIS->font);
	}
	
	if (READ_PROPERTY)
	{
		*(((CFONT *)THIS->font)->font) = WIDGET->font();
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
		GB.ReturnBoolean(CWIDGET_is_design(THIS));
		return;
	}

	if (VPROP(GB_BOOLEAN))
	{
		set_design(THIS);
		//CWIDGET_set_flag(THIS, WF_DESIGN);
	}
	else if (CWIDGET_test_flag(_object, WF_DESIGN) || CWIDGET_test_flag(_object, WF_DESIGN_LEADER))
		GB.Error("Design property cannot be reset");

END_PROPERTY


BEGIN_PROPERTY(Control_Enabled)

	if (READ_PROPERTY)
		GB.ReturnBoolean(WIDGET->isEnabled());
	else
		WIDGET->setEnabled(VPROP(GB_BOOLEAN));

END_PROPERTY


BEGIN_PROPERTY(Control_HasFocus)

	HANDLE_PROXY(_object);
	
	GB.ReturnBoolean(WIDGET->hasFocus());

END_PROPERTY

BEGIN_PROPERTY(Control_Hovered)

	if (!CWIDGET_is_visible(THIS))
		GB.ReturnBoolean(false);
	else
		GB.ReturnBoolean(THIS->flag.inside);

END_PROPERTY

BEGIN_PROPERTY(Control_Expand)

	if (READ_PROPERTY)
		GB.ReturnBoolean(THIS->flag.expand);
	else if (THIS->flag.expand != VPROP(GB_BOOLEAN))
	{
		THIS->flag.expand = VPROP(GB_BOOLEAN);
		CWIDGET_check_visibility(THIS);
		arrange_parent(THIS);
	}

END_PROPERTY


BEGIN_PROPERTY(Control_Ignore)

	if (READ_PROPERTY)
		GB.ReturnBoolean(THIS->flag.ignore);
	else if (THIS->flag.ignore != VPROP(GB_BOOLEAN))
	{
		THIS->flag.ignore = VPROP(GB_BOOLEAN);
		arrange_parent(THIS);
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

	w = (int)(VARG(w) * MAIN_scale + 0.5);
	h = (int)(VARG(h) * MAIN_scale + 0.5);
	
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

	if (!THIS->flag.resized)
		return;

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
		GB.ReturnBoolean(CWIDGET_is_visible(THIS));
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
		//qDebug("set focus on %s for %s", THIS->name, ((CWIDGET *)win)->name);
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
	{
		if (THIS_EXT)
			GB.ReturnVariant(&THIS_EXT->tag);
		else
		{
			GB.ReturnNull();
			GB.ReturnConvVariant();
		}
	}
	else
		GB.StoreVariant(PROP(GB_VARIANT), POINTER(&(ENSURE_EXT(THIS)->tag)));

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
		set_mouse(wid, VPROP(GB_INTEGER), THIS_EXT ? THIS_EXT->cursor : NULL);

END_METHOD


BEGIN_PROPERTY(Control_Cursor)

	HANDLE_PROXY(_object);
	
	if (READ_PROPERTY)
		GB.ReturnObject(THIS_EXT ? THIS_EXT->cursor : NULL);
	else
	{
		GB.StoreObject(PROP(GB_OBJECT), &(ENSURE_EXT(THIS)->cursor));
		set_mouse(WIDGET, CMOUSE_CUSTOM, THIS_EXT->cursor);
	}

END_PROPERTY


BEGIN_PROPERTY(Control_NoTabFocus)

	HANDLE_PROXY(_object);
	
	if (READ_PROPERTY)
		GB.ReturnBoolean(THIS->flag.noTabFocus);
	else
	{
		bool v = VPROP(GB_BOOLEAN);
		Qt::FocusPolicy policy;
			
		
		if (THIS->flag.noTabFocus == v)
			return;

		THIS->flag.noTabFocus = v;
		
		if (v)
		{
			policy = WIDGET->focusPolicy();
			
			ENSURE_EXT(THIS)->focusPolicy = (int)policy;
		
			switch (policy)
			{
				case Qt::TabFocus: policy = Qt::NoFocus; break;
				case Qt::StrongFocus: policy = Qt::ClickFocus; break;
				case Qt::WheelFocus: policy = Qt::ClickFocus; break;
				default: break;
			}
		}
		else
		{
			policy = (Qt::FocusPolicy)THIS_EXT->focusPolicy;
		}

		WIDGET->setFocusPolicy(policy);
	}

END_PROPERTY


static QWidget *get_color_widget(CWIDGET *_object)
{
	if (qobject_cast<MyScrollView *>(WIDGET))
		return ((CSCROLLVIEW *)THIS)->container;
	
	QWidget *view = get_viewport(WIDGET);
	if (view)
		return view;

	return WIDGET;
}

QT_COLOR_FUNC CWIDGET_after_set_color(QT_COLOR_FUNC func)
{
	QT_COLOR_FUNC old = _after_set_color;
	_after_set_color = func;
	return old;
}


void CWIDGET_reset_color(CWIDGET *_object)
{
	GB_COLOR fg, bg;
	QPalette palette;
	QWidget *w;
	
	HANDLE_PROXY(_object);
	//qDebug("reset_color: %s", THIS->name);
	//qDebug("set_color: (%s %p) bg = %08X (%d) fg = %08X (%d)", GB.GetClassName(THIS), THIS, THIS->bg, w->backgroundRole(), THIS->fg, w->foregroundRole());
	
	w = get_color_widget(THIS);
	
	if (!THIS_EXT || (THIS_EXT->bg == COLOR_DEFAULT && THIS_EXT->fg == COLOR_DEFAULT))
	{
		w->setPalette(QPalette());
		w->setAutoFillBackground(!THIS->flag.noBackground && THIS->flag.fillBackground);
	}
	else
	{
		bg = THIS_EXT->bg;
		fg = THIS_EXT->fg;
		
		if (qobject_cast<QComboBox *>(w))
		{
			QComboBox *cb = (QComboBox *)w;
			palette = QPalette();

			if (bg != COLOR_DEFAULT)
			{
				if (cb->isEditable())
					palette.setColor(QPalette::Base, TO_QCOLOR(bg));
				else
					palette.setColor(QPalette::Button, TO_QCOLOR(bg));
			}

			if (fg != COLOR_DEFAULT)
			{
				if (cb->isEditable())
					palette.setColor(QPalette::Text, TO_QCOLOR(fg));
				else
					palette.setColor(QPalette::ButtonText, TO_QCOLOR(fg));
			}

			w->setPalette(palette);
		}
		else if (qobject_cast<QSpinBox *>(w))
		{
			palette = QPalette();

			if (bg != COLOR_DEFAULT)
			{
				palette.setColor(QPalette::Base, TO_QCOLOR(bg));
			}

			if (fg != COLOR_DEFAULT)
			{
				palette.setColor(QPalette::Text, TO_QCOLOR(fg));
			}

			w->setPalette(palette);
		}
		else
		{
			palette = QPalette();
		
			if (bg != COLOR_DEFAULT)
				palette.setColor(w->backgroundRole(), TO_QCOLOR(bg));
			
			if (fg != COLOR_DEFAULT)
				palette.setColor(w->foregroundRole(), TO_QCOLOR(fg));
		
			w->setPalette(palette);

			w->setAutoFillBackground(!THIS->flag.noBackground && (THIS->flag.fillBackground || ((THIS_EXT && THIS_EXT->bg != COLOR_DEFAULT) && w->backgroundRole() == QPalette::Window)));
		}

	}
	
	//w->setAutoFillBackground(THIS->bg != COLOR_DEFAULT);
	
	if (GB.Is(THIS, CLASS_TextArea))
		CTEXTAREA_set_foreground(THIS);
	
	if (_after_set_color)
		(*_after_set_color)(THIS);

	if (!GB.Is(THIS, CLASS_Container))
		return;
	
	if (GB.Is(THIS, CLASS_Window))
		CWINDOW_define_mask((CWINDOW *)THIS);
}


void CWIDGET_set_color(CWIDGET *_object, int bg, int fg, bool handle_proxy)
{
	if (handle_proxy) { HANDLE_PROXY(_object); }

	ENSURE_EXT(THIS);
	THIS_EXT->bg = bg;
	THIS_EXT->fg = fg;
	
	CWIDGET_reset_color(THIS);
}


GB_COLOR CWIDGET_get_background(CWIDGET *_object, bool handle_proxy)
{
	if (handle_proxy) { HANDLE_PROXY(_object); }

	return THIS_EXT ? THIS_EXT->bg : COLOR_DEFAULT;
}


GB_COLOR CWIDGET_get_real_background(CWIDGET *_object)
{
	GB_COLOR bg = CWIDGET_get_background(THIS);
	
	if (bg != COLOR_DEFAULT)
		return bg;

	CWIDGET *parent = (CWIDGET *)CWIDGET_get_parent(THIS);
	
	if (parent)
		return CWIDGET_get_real_background(parent);
	else
		return QApplication::palette().color(QPalette::Window).rgb() & 0xFFFFFF;
}


GB_COLOR CWIDGET_get_foreground(CWIDGET *_object, bool handle_proxy)
{
	if (handle_proxy) { HANDLE_PROXY(_object); }

	return THIS_EXT ? THIS_EXT->fg : COLOR_DEFAULT;
}


GB_COLOR CWIDGET_get_real_foreground(CWIDGET *_object)
{
	GB_COLOR fg = CWIDGET_get_foreground(THIS);
	
	if (fg != COLOR_DEFAULT)
		return fg;

	CWIDGET *parent = (CWIDGET *)CWIDGET_get_parent(THIS);
	
	if (parent)
		return CWIDGET_get_real_foreground(parent);
	else
		return QApplication::palette().color(QPalette::WindowText).rgb() & 0xFFFFFF;
}


BEGIN_PROPERTY(Control_Background)

	if (THIS_EXT && THIS_EXT->proxy)
	{
		if (READ_PROPERTY)
			GB.GetProperty(THIS_EXT->proxy, "Background");
		else
		{
			GB_VALUE value;
			value.type = GB_T_INTEGER;
			value._integer.value = VPROP(GB_INTEGER);
			GB.SetProperty(THIS_EXT->proxy, "Background", &value);
		}
		
		return;
	}

	if (READ_PROPERTY)
		GB.ReturnInteger(CWIDGET_get_background(THIS));
	else
	{
		GB_COLOR col = VPROP(GB_INTEGER);
		if (col != CWIDGET_get_background(THIS))
			CWIDGET_set_color(THIS, col, CWIDGET_get_foreground(THIS));
	}

END_PROPERTY


BEGIN_PROPERTY(Control_Foreground)

	if (THIS_EXT && THIS_EXT->proxy)
	{
		if (READ_PROPERTY)
			GB.GetProperty(THIS_EXT->proxy, "Foreground");
		else
		{
			GB_VALUE value;
			value.type = GB_T_INTEGER;
			value._integer.value = VPROP(GB_INTEGER);
			GB.SetProperty(THIS_EXT->proxy, "Foreground", &value);
		}

		return;
	}

	if (READ_PROPERTY)
		GB.ReturnInteger(CWIDGET_get_foreground(THIS));
	else
	{
		GB_COLOR col = VPROP(GB_INTEGER);
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
	{
		QString tip = QSTRING_PROP();
		if (THIS->flag.inside)
		{
			if (tip.isEmpty())
				QToolTip::hideText();
			else if (QToolTip::isVisible())
			{
				QToolTip::hideText();
				QToolTip::showText(QCursor::pos(), tip, WIDGET);
			}
		}
		WIDGET->setToolTip(tip);
	}

END_PROPERTY


BEGIN_PROPERTY(Control_Name)

	if (READ_PROPERTY)
		GB.ReturnString(THIS->name);
	else
		CWIDGET_set_name(THIS, GB.ToZeroString(PROP(GB_STRING)));

END_PROPERTY


BEGIN_PROPERTY(Control_Action)

	char *current = THIS_EXT ? THIS_EXT->action : NULL;

	if (READ_PROPERTY)
		GB.ReturnString(current);
	else
	{
		char *action = PLENGTH() ? GB.NewString(PSTRING(), PLENGTH()) : NULL;
		
		CACTION_register(THIS, current, action);
		
		if (THIS_EXT)
			GB.FreeString(&THIS_EXT->action);
		
		if (action)
			ENSURE_EXT(THIS)->action = action;
	}

END_PROPERTY


BEGIN_PROPERTY(Control_Proxy)

	if (READ_PROPERTY)
		GB.ReturnObject(THIS_EXT ? THIS_EXT->proxy : NULL);
	else
		CWIDGET_register_proxy(THIS, VPROP(GB_OBJECT));

END_PROPERTY


BEGIN_PROPERTY(Control_PopupMenu)

	if (READ_PROPERTY)
		GB.ReturnString(THIS_EXT ? THIS_EXT->popup : NULL);
	else
		GB.StoreString(PROP(GB_STRING), &(ENSURE_EXT(THIS)->popup));

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

	show = CWIDGET_is_visible(THIS);
	CWIDGET_set_visible(THIS, false);
	WIDGET->setParent(QCONTAINER(VARG(container)));
	WIDGET->move(p);
	CCONTAINER_insert_child(THIS);
	CWIDGET_set_visible(THIS, show);

END_METHOD


BEGIN_PROPERTY(Control_Drop)

	if (READ_PROPERTY)
		GB.ReturnBoolean(THIS->flag.drop);
	else
	{
		THIS->flag.drop = VPROP(GB_BOOLEAN);
		if (CWIDGET_test_flag(THIS, WF_SCROLLVIEW))
			get_viewport(WIDGET)->setAcceptDrops(VPROP(GB_BOOLEAN));
		else
			WIDGET->setAcceptDrops(VPROP(GB_BOOLEAN));
	}

END_PROPERTY

static bool has_tracking(CWIDGET *_object)
{
	HANDLE_PROXY(_object);
	return THIS->flag.tracking;
}

BEGIN_PROPERTY(Control_Tracking)

	HANDLE_PROXY(_object);

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
		if (wid->frameStyle() == (QFrame::Box + QFrame::Plain) && wid->lineWidth() == 1)
			border = BORDER_PLAIN;
		else if (wid->frameStyle() == (QFrame::StyledPanel + QFrame::Sunken))
			border = BORDER_SUNKEN;
		else if (wid->frameStyle() == (QFrame::StyledPanel + QFrame::Raised))
			border = BORDER_RAISED;
		else if (wid->frameStyle() == (QFrame::StyledPanel + QFrame::Plain))
			border = BORDER_ETCHED;
		else
			border = BORDER_NONE;

		GB.ReturnInteger(border);
	}
	else
	{
		switch (VPROP(GB_INTEGER))
		{
			case BORDER_PLAIN: border = QFrame::Box + QFrame::Plain; lw = 1; break;
			case BORDER_SUNKEN: border = QFrame::StyledPanel + QFrame::Sunken; lw = 2; break;
			case BORDER_RAISED: border = QFrame::StyledPanel + QFrame::Raised; lw = 2; break;
			case BORDER_ETCHED: border = QFrame::StyledPanel + QFrame::Plain; lw = 2; break;
			default: border = QFrame::NoFrame; lw = 0; break;
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
			wid->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
			//wid->setFrameStyle(QFrame::LineEditPanel);
			wid->setLineWidth(2);
		}
		else
		{
			wid->setFrameStyle(QFrame::NoFrame);
			wid->setLineWidth(0);
		}

		//qDebug("--> %s %d %d %d %d", THIS->name, wid->contentsRect().x(), wid->contentsRect().y(), wid->contentsRect().width(), wid->contentsRect().height());
		//wid->style()->polish(wid);
		wid->update();
	}

END_PROPERTY


BEGIN_PROPERTY(CWIDGET_scrollbar)

	QAbstractScrollArea *wid = qobject_cast<QAbstractScrollArea *>(WIDGET);
	//Q3ScrollView *sw = qobject_cast<Q3ScrollView *>(WIDGET);
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
	/*else if (sw)
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
	}*/

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

#define CLEAN_POINTER(_ptr) if ((_ptr) == THIS) _ptr = NULL

void CWidget::destroy()
{
	QWidget *w = (QWidget *)sender();
	CWIDGET *_object = CWidget::get(w);

	if (!THIS)
		return;

	//qDebug("CWidget::destroy: (%s %p) %s [%p]", GB.GetClassName(THIS), THIS, THIS->name, _hovered);

	if (!_post_check_hovered)
	{
		CWIDGET *top = (CWIDGET *)CWidget::getTopLevel(THIS);
		if (top != THIS)
		{
			_post_check_hovered = true;
			_post_check_hovered_window = top;
			GB.Post((void (*)())post_check_hovered, NULL);
		}
	}
	
	CLEAN_POINTER(_hovered);
	CLEAN_POINTER(_official_hovered);
	CLEAN_POINTER(_post_check_hovered_window);
	CLEAN_POINTER(CWIDGET_active_control);
	CLEAN_POINTER(CWIDGET_previous_control);
	CLEAN_POINTER(_old_active_control);

	if (THIS_EXT)
	{
		CACTION_register(THIS, THIS_EXT->action, NULL);
		GB.FreeString(&THIS_EXT->action);
	
		if (THIS_EXT->proxy)
			EXT(THIS_EXT->proxy)->proxy_for = NULL;
		if (THIS_EXT->proxy_for)
			EXT(THIS_EXT->proxy_for)->proxy = NULL;
		
		if (THIS_EXT->container_for)
		{
			((CCONTAINER *)THIS_EXT->container_for)->container = ((CWIDGET *)THIS_EXT->container_for)->widget;
			THIS_EXT->container_for = NULL;
		}
	
		GB.Unref(POINTER(&THIS_EXT->cursor));
		GB.FreeString(&THIS_EXT->popup);
		GB.StoreVariant(NULL, &THIS_EXT->tag);
		GB.Free(POINTER(&THIS->ext));
	}
	
	CWIDGET_set_name(THIS, 0);

	dict.remove(w);

	QWIDGET(THIS) = NULL;
	GB.Unref(POINTER(&THIS->font));
	
	//qDebug(">> CWidget::destroy %p (%p) :%p:%ld #2", ob, ob->widget, ob->ob.klass, ob->ob.ref);
	//if (!CWIDGET_test_flag(ob, WF_NODETACH))
	GB.Detach(THIS);
	
	GB.Unref(POINTER(&_object));
}

/*static void post_dblclick_event(void *control)
{
	GB.Raise(control, EVENT_DblClick, 0);
	GB.Unref(&control);
}*/

static void post_focus_change(void *)
{
	CWIDGET *current, *control;
	
	//qDebug("post_focus_change");

	for(;;)
	{
		current = CWIDGET_active_control;
		if (current == _old_active_control)
			break;

		control = _old_active_control;
		while (control)
		{
			GB.Raise(control, EVENT_LostFocus, 0);
			control = (CWIDGET *)(EXT(control) ? EXT(control)->proxy_for : NULL);
		}
		
		_old_active_control = current;
		CWINDOW_activate(current);
		
		control = current;
		while (control)
		{
			GB.Raise(control, EVENT_GotFocus, 0);
			control = (CWIDGET *)(EXT(control) ? EXT(control)->proxy_for : NULL);
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
	
	if (CWIDGET_active_control && !_focus_change)
		CWIDGET_previous_control = CWIDGET_active_control;
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
	bool parent_got_it;

	CCONTROL_last_event_type = type;

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
		case QEvent::TabletMove:
		case QEvent::TabletPress:
		case QEvent::TabletRelease:
			jump = &&__TABLET; break;
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

	__ENTER:
	{
		QWidget *popup = qApp->activePopupWidget();
		
		//qDebug("enter %s real = %d popup = %p inside = %d", control->name, real, popup, control->flag.inside);
		
		if (real)
		{
			if (!popup || CWidget::getReal(popup))
				CWIDGET_enter(control);
			else if (_enter_leave_set)
				insert_enter_leave_event(control, true);
		}
		
		goto __NEXT;
	}

	__LEAVE:
	{
		QWidget *popup = qApp->activePopupWidget();
		
		//qDebug("leave %s real = %d popup = %p inside = %d", control->name, real, popup, control->flag.inside);
		
		if (real)
		{
			if (!popup || CWidget::getReal(popup))
				CWIDGET_leave(control);
			else if (_enter_leave_set)
				insert_enter_leave_event(control, false);
		}
		
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
		while (EXT(control) && EXT(control)->proxy_for)
			control = (CWIDGET *)(EXT(control)->proxy_for);

	__MENU_TRY_PROXY:
	
		// if (real && GB.CanRaise(control, EVENT_Menu))
		//qDebug("Menu event! %p %d", control, EVENT_Menu);
		if (GB.CanRaise(control, EVENT_Menu))
		{
			int old = MENU_popup_count;

			((QContextMenuEvent *)event)->accept();

			if (GB.Raise(control, EVENT_Menu, 0) || MENU_popup_count != old)
				return true;
			//else
			//	goto __NEXT;
		}

		if (EXT(control) && EXT(control)->popup)
		{
			CWINDOW *window = CWidget::getWindow(control);
			CMENU *menu = CWindow::findMenu(window, EXT(control)->popup);
			if (menu)
				CMENU_popup(menu, QCursor::pos());
			return true;
		}

		if (EXT(control) && EXT(control)->proxy)
		{
			control = (CWIDGET *)(EXT(control)->proxy);
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
			
			MOUSE_info.sx = p.x();
			MOUSE_info.sy = p.y();
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

		if (event_id == EVENT_MouseMove && mevent->buttons() == Qt::NoButton && !has_tracking(control))
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
			MOUSE_info.x = p.x();
			MOUSE_info.y = p.y();
			MOUSE_info.screenX = mevent->globalX();
			MOUSE_info.screenY = mevent->globalY();
			MOUSE_info.button = mevent->button();
			MOUSE_info.state = mevent->buttons();
			MOUSE_info.modifier = mevent->modifiers();

			cancel = GB.Raise(control, event_id, 0); //, GB_T_INTEGER, p.x(), GB_T_INTEGER, p.y(), GB_T_INTEGER, state);

			CMOUSE_clear(false);
			
			/*if (CDRAG_dragging)
				return true;*/
		}
		
		if (event_id == EVENT_MouseMove && !cancel && (mevent->buttons() != Qt::NoButton) && GB.CanRaise(control, EVENT_MouseDrag) && !CDRAG_dragging
				&& ((abs(p.x() - MOUSE_info.sx) + abs(p.y() - MOUSE_info.sy)) > 8)) // QApplication::startDragDistance()))
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
			MOUSE_info.x = p.x();
			MOUSE_info.y = p.y();
			MOUSE_info.screenX = mevent->globalX();
			MOUSE_info.screenY = mevent->globalY();
			MOUSE_info.button = mevent->button();
			MOUSE_info.state = mevent->buttons();
			MOUSE_info.modifier = mevent->modifiers();
		
			//qDebug("MouseDrag: %s", control->name);
			cancel = GB.Raise(control, EVENT_MouseDrag, 0);
			
			CMOUSE_clear(false);
		}
	
		GB.Unref(POINTER(&control));
		
		if (!control)
			goto __MOUSE_RETURN_TRUE;

		if (control->flag.grab && event_id == EVENT_MouseUp)
			MyApplication::eventLoop->exit();
		
		if (cancel)
			goto __MOUSE_RETURN_TRUE;
		
		if (EXT(control) && EXT(control)->proxy_for)
		{
			control = (CWIDGET *)(EXT(control)->proxy_for);
			goto __MOUSE_TRY_PROXY;
		}
		
		CMOUSE_reset_translate();
		goto __NEXT;

	__MOUSE_RETURN_TRUE:

		CMOUSE_reset_translate();
		return true;
	}
	
	__TABLET:
	{
		QTabletEvent *tevent = (QTabletEvent *)event;

		if (!original)
			goto _DESIGN;

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
		
		if (!control->flag.use_tablet)
			goto __NEXT;
		
	__TABLET_TRY_PROXY:
	
		p.setX(tevent->globalX());
		p.setY(tevent->globalY());
		p = QWIDGET(control)->mapFromGlobal(p);
		
		if (type == QEvent::TabletPress)
		{
			//qDebug("MouseDown on %p (%s %p) %s%s", widget, control ? GB.GetClassName(control) : "-", control, real ? "REAL " : "", design ? "DESIGN " : "");

			event_id = EVENT_MouseDown;
			//state = mevent->buttons();
			
			//MOUSE_info.sx = p.x();
			//MOUSE_info.sy = p.y();
			
			control->flag.tablet_pressed = true;
			//qDebug("MouseEvent: %d %d", mevent->x(), mevent->y());
		}
		else if (type == QEvent::TabletMove)
		{
			//if (!control->flag.tracking && !control->flag.tablet_pressed)
			//	return false;
			
			event_id = EVENT_MouseMove;
		}
		else //if (type == QEvent::TabletRelease)
		{
			event_id = EVENT_MouseUp;
			//state = mevent->buttons();
		}

		//if (event_id == EVENT_MouseMove && mevent->buttons() == Qt::NoButton && !QWIDGET(control)->hasMouseTracking())
		//	goto _DESIGN;


		cancel = false;

		if (GB.CanRaise(control, event_id))
		{
			//MOUSE_info.x = p.x();
			//MOUSE_info.y = p.y();
			//POINTER_info.screenX = tevent->globalX();
			//POINTER_info.screenY = tevent->globalY();
			//MOUSE_info.modifier = tevent->modifiers();
			POINTER_info.tx = tevent->hiResGlobalX();
			POINTER_info.ty = tevent->hiResGlobalY();
			POINTER_info.pressure = tevent->pressure();
			POINTER_info.rotation = tevent->rotation();
			POINTER_info.xtilt = tevent->xTilt();
			POINTER_info.ytilt = tevent->yTilt();
			
			switch(tevent->pointerType())
			{
				case QTabletEvent::Pen: POINTER_info.type = POINTER_PEN; break;
				case QTabletEvent::Eraser: POINTER_info.type = POINTER_ERASER; break;
				case QTabletEvent::Cursor: POINTER_info.type = POINTER_CURSOR; break;
				default: POINTER_info.type = POINTER_MOUSE;
			}

			//cancel = GB.Raise(control, event_id, 0);

			//CMOUSE_clear(false);
		}
		
		//if (control->flag.grab && event_id == EVENT_MouseUp)
		//	MyApplication::eventLoop->exit();
		
		if (event_id == EVENT_MouseUp)
			control->flag.tablet_pressed = false;
		
		//if (cancel)
		//	return true;
		
		if (EXT(control) && EXT(control)->proxy_for)
		{
			control = (CWIDGET *)(EXT(control)->proxy_for);
			goto __TABLET_TRY_PROXY;
		}
		
		CMOUSE_reset_translate();
		return false; // We fill the information, and then expect Qt to generate a Mouse event from the Tablet event
	}
	
	__KEY:
	{
		QKeyEvent *kevent = (QKeyEvent *)event;

		#if QT_VERSION <= 0x030005
		if (!real || !original)
			goto _DESIGN;
		#endif

		if (control->flag.no_keyboard)
			goto _DESIGN;

		event_id = (type == QEvent::KeyRelease) ? EVENT_KeyRelease : EVENT_KeyPress;
		cancel = false;
		parent_got_it = false;

		#if QT_VERSION > 0x030005
		if (!original && type != QEvent::InputMethod)
			goto _DESIGN; //_ACCEL;
		#endif

		if (type == QEvent::KeyRelease && kevent->isAutoRepeat())
			goto __NEXT;
			
		/*qDebug("QKeyEvent: %s: (%s %s) -> %d %s %s",
			type == QEvent::KeyPress ? "KeyPress" : "KeyRelease",
			GB.GetClassName(control), control->name,
			kevent->key(), (const char *)kevent->text().toLatin1(), kevent->isAutoRepeat() ? "AR" : "--");*/

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
		
		GB.Ref(control);
		
		if (!parent_got_it)
		{
			parent_got_it = true;
			if (!cancel)
				cancel = raise_key_event_to_parent_window(control, event_id);
		}
		
		if (!cancel)
			cancel = GB.Raise(control, event_id, 0);

		GB.Unref(POINTER(&control));
		
		CKEY_clear(false);

		if ((cancel && (type != QEvent::KeyRelease)) || !control)
			return true;

		if (EXT(control) && EXT(control)->proxy_for)
		{
			control = (CWIDGET *)(EXT(control)->proxy_for);
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
	
				if (EXT(control) && EXT(control)->proxy_for)
					cancel = GB.Raise(EXT(control)->proxy_for, event_id, 0);
				if (!cancel)
					cancel = GB.Raise(control, event_id, 0);
	
				CKEY_clear(false);
	
				if (cancel)
					return true;
			}

			if (EXT(control) && EXT(control)->proxy_for)
			{
				control = (CWIDGET *)(EXT(control)->proxy_for);
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
			MOUSE_info.x = p.x();
			MOUSE_info.y = p.y();
			MOUSE_info.screenX = ev->globalX();
			MOUSE_info.screenY = ev->globalY();
			MOUSE_info.state = ev->buttons();
			MOUSE_info.modifier = ev->modifiers();
			MOUSE_info.orientation = ev->orientation();
			MOUSE_info.delta = ev->delta();

			cancel = GB.Raise(control, EVENT_MouseWheel, 0);

			CMOUSE_clear(false);
			
			if (cancel)
				return true;
		}
		
		if (EXT(control) && EXT(control)->proxy_for)
		{
			control = (CWIDGET *)(EXT(control)->proxy_for);
			goto __MOUSE_WHEEL_TRY_PROXY;
		}
		
		goto __NEXT;
	}
	
	__DRAG_ENTER:
	{
		if (!control->flag.drop)
			goto __NEXT;

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
		if (!control->flag.drop)
			goto __NEXT;

		for(;;)
		{
			if (GB.CanRaise(control, EVENT_DragMove))
			{
				if (CDRAG_drag_move(QWIDGET(control), control, (QDropEvent *)event))
				{
					if (!((QDropEvent *)event)->isAccepted())
						CDRAG_hide_frame(control);
					break;
				}
			}

			if (EXT(control) && EXT(control)->proxy)
			{
				control = (CWIDGET *)(EXT(control)->proxy);
				continue;
			}
			else
				break;
		}
		
		goto __NEXT;
	}
	
	__DRAG_LEAVE:
	{
		if (!control->flag.drop)
			goto __NEXT;

		CDRAG_drag_leave(control);
		goto __NEXT;
	}
	
	__DROP:
	{
		if (!control->flag.drop)
			goto __NEXT;

		//if (!CWIDGET_test_flag(control, WF_NO_DRAG))
		CDRAG_drag_leave(control);
		if (CDRAG_drag_drop((QWidget *)widget, control, (QDropEvent *)event))
			return true;
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

#define HAS_ACTION(_control) ((CWIDGET *)(_control))->flag.has_action
#define SET_ACTION(_control, _flag) (((CWIDGET *)(_control))->flag.has_action = (_flag))

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
		img = pixmap.toImage().convertToFormat(QImage::Format_ARGB32_Premultiplied);
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

	GB_METHOD("_new", NULL, Control_new, NULL),
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
	GB_PROPERTY("NoTabFocus", "b", Control_NoTabFocus),

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
	GB_EVENT("DragLeave", NULL, NULL, &EVENT_DragLeave),

	CONTROL_DESCRIPTION,

	GB_END_DECLARE
};



