/***************************************************************************

  CWidget.cpp

  The Control class

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

#define __CWIDGET_CPP

#include "gambas.h"

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

#include <qnamespace.h>
#include <qapplication.h>
#if QT_VERSION >= 0x030200
#include <qobjectlist.h>
#else
#include <qobjcoll.h>
#endif
#include <qpalette.h>
#include <qtooltip.h>
#include <qpushbutton.h>
#include <qscrollview.h>
#include <qmap.h>

CWIDGET *CWIDGET_destroy_list = 0;
CWIDGET *CWIDGET_destroy_last = 0;

GB_CLASS CLASS_Control;
GB_CLASS CLASS_Container;
GB_CLASS CLASS_UserControl;
GB_CLASS CLASS_UserContainer;
GB_CLASS CLASS_Window;
GB_CLASS CLASS_Menu;
GB_CLASS CLASS_Picture;
GB_CLASS CLASS_Drawing;
GB_CLASS CLASS_DrawingArea;
GB_CLASS CLASS_Printer;

#ifndef NO_X_WINDOW
static QMap<int, int> _x11_to_qt_keycode;
#endif

/* Control class */


static void set_mouse(QWidget *w, int mouse, void *cursor)
{
  QObjectList *children;
  QObject *child;

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
    w->setCursor(mouse);

  children = (QObjectList *)(w->children());

  if (!children)
    return;

  child = children->first();
  while (child)
  {
    if (child->isWidgetType() && !CWidget::getReal(child))
      set_mouse((QWidget *)child, CMOUSE_DEFAULT, 0);

    child = children->next();
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
}

static void set_design_recursive(QWidget *w, bool set = false)
{
  QObjectList *children;
  QObject *child;
  CWIDGET *ob = CWidget::getReal(w);

	if (ob)
		set_design_object(ob);

  children = (QObjectList *)(w->children());

  if (!children)
    return;

  child = children->first();
  while (child)
  {
    if (child->isWidgetType())
      set_design_recursive((QWidget *)child, true);

    child = children->next();
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
}

static void set_name(CWIDGET *_object, const char *name)
{
	CWINDOW *window;
	MyMainWindow *win = 0;
	
	if (GB.Is(THIS, CLASS_Menu))
	{
		if (((CMENU *)THIS)->toplevel->isA("MyMainWindow"))
			win = (MyMainWindow *)((CMENU *)THIS)->toplevel;
	}
	else
	{
		window = CWidget::getWindow(THIS);
		if (window)
			win = (MyMainWindow *)QWIDGET(window);
	}
		
	if (win)
	{
		if (name)
			win->setName(name, THIS);
		else
			win->setName(THIS->name, 0);
	}
		
	GB.FreeString(&THIS->name);
	if (name)
		GB.NewString(&THIS->name, name, 0);
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
  CWidget::add(w, _object, no_filter);

  //qDebug("CWIDGET_new: %p: %p", object, w);

  THIS->widget = w;
  THIS->level = MAIN_loop_level;
  THIS->next = 0;

  if (!no_init)
  {
    THIS->tag.type = GB_T_NULL;
		CWIDGET_init_name(THIS);	
	}

	if (w->inherits("QScrollView"))
	{
		CWIDGET_set_flag(THIS, WF_SCROLLVIEW);
    ((QScrollView *)w)->setFrameStyle(QFrame::LineEditPanel + QFrame::Sunken);
	}

	THIS->flag.default_bg = true;
	THIS->flag.default_fg = true;
	
	if (!no_show)
	{
		THIS->flag.visible = true;
		w->show();
	}

	//WIDGET->setName(THIS->name);
}


int CCONTROL_check(void *object)
{
	return QWIDGET(object) == NULL || CWIDGET_test_flag(object, WF_DELETED);
}


QString CWIDGET_Utf8ToQString(GB_STRING *str)
{
  return QString::fromUtf8((const char *)(str->value.addr + str->value.start), str->value.len);
}


void CWIDGET_destroy(CWIDGET *object)
{
  if (!object->widget)
    return;

  if (CWIDGET_test_flag(object, WF_DELETED))
    return;

  //qDebug("CWIDGET_destroy: %p (%p) :%p:%ld", object, object->widget, object->ob.klass, object->ob.ref);

  if (!CWIDGET_destroy_list)
    CWIDGET_destroy_list = object;
  else
  {
    CWIDGET_destroy_last->next = object;
    object->prev = CWIDGET_destroy_last;
  }

  CWIDGET_destroy_last = object;

  CWIDGET_set_flag(object, WF_DELETED);

  //qDebug("destroy: %s %p", GB.GetClassName(object), object);

  object->widget->hide();
  //GB.Ref(object);
}


//#if QT_VERSION >= 0x030005
//  #define COORD(_c) (WIDGET->pos()._c())
//#else
#define COORD(_c) ((WIDGET->isA("MyMainWindow") && WIDGET->isTopLevel()) ? ((CWINDOW *)_object)->_c : WIDGET->pos()._c())
//#define WIDGET_POS(_c) ((WIDGET->isTopLevel()) ? ((CWINDOW *)_object)->_c : WIDGET->pos()._c())
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

static void move_widget(void *_object, int x, int y)
{
  QWidget *wid = get_widget(THIS);

	if (wid)
  	wid->move(x, y);

  if (GB.Is(THIS, CLASS_Window))
  {
    ((CWINDOW *)_object)->x = x;
    ((CWINDOW *)_object)->y = y;
  }
}


static void resize_widget(void *_object, int w, int h)
{
  QWidget *wid = get_widget_resize(THIS);

  if (w < 0 && h < 0)
    return;

  if (w < 0)
    w = wid->width();

  if (h < 0)
    h = wid->height();

	if (wid)
  	wid->resize(QMAX(0, w), QMAX(0, h));

  if (GB.Is(THIS, CLASS_Window))
  {
    //qDebug("resize_widget: %d %d", w, h);

    ((CWINDOW *)_object)->w = w;
    ((CWINDOW *)_object)->h = h;
    // menu bar height is ignored
    //((CWINDOW *)_object)->container->resize(w, h);
  }
}


static void move_resize_widget(void *_object, int x, int y, int w, int h)
{
  QWidget *wid = get_widget(THIS);

  if (wid->isA("QWorkspaceChild"))
  {
  	move_widget(THIS, x, y);
  	resize_widget(THIS, w, h);
  	return;
  }

	if (w < 0)
		w = wid->width();

	if (h < 0)
		h = wid->height();

	if (wid)
		wid->setGeometry(x, y, QMAX(0, w), QMAX(0, h));

  if (GB.Is(THIS, CLASS_Window))
  {
    ((CWINDOW *)_object)->x = x;
    ((CWINDOW *)_object)->y = y;
    ((CWINDOW *)_object)->w = w;
    ((CWINDOW *)_object)->h = h;
    //((CWINDOW *)_object)->container->resize(w, h);
  }
}


BEGIN_PROPERTY(CCONTROL_x)

  if (READ_PROPERTY)
    GB.ReturnInteger(COORD(x));
  else
  {
    move_widget(_object, VPROP(GB_INTEGER), COORD(y));
    /*if (WIDGET->isTopLevel())
      qDebug("X: %d ==> X = %d", PROPERTY(int), WIDGET->x());*/
  }

END_PROPERTY


BEGIN_PROPERTY(CCONTROL_screen_x)

  GB.ReturnInteger(WIDGET->mapToGlobal(QPoint(0, 0)).x());

END_PROPERTY


BEGIN_PROPERTY(CCONTROL_y)

  if (READ_PROPERTY)
    GB.ReturnInteger(COORD(y));
  else
    move_widget(_object, COORD(x), VPROP(GB_INTEGER));

END_PROPERTY


BEGIN_PROPERTY(CCONTROL_screen_y)

  GB.ReturnInteger(WIDGET->mapToGlobal(QPoint(0, 0)).y());

END_PROPERTY


BEGIN_PROPERTY(CCONTROL_w)

  if (READ_PROPERTY)
    GB.ReturnInteger(get_widget_resize(THIS)->width());
  else
    resize_widget(_object, VPROP(GB_INTEGER), -1);

END_PROPERTY


BEGIN_PROPERTY(CCONTROL_h)

  if (READ_PROPERTY)
    GB.ReturnInteger(get_widget_resize(THIS)->height());
  else
    resize_widget(_object, -1, VPROP(GB_INTEGER));

END_PROPERTY

BEGIN_PROPERTY(CCONTROL_font)

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
      WIDGET->unsetFont();
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

BEGIN_PROPERTY(CCONTROL_design)

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


BEGIN_PROPERTY(CCONTROL_enabled)

  if (READ_PROPERTY)
    GB.ReturnBoolean(QWIDGET(_object)->isEnabled());
  else
    QWIDGET(_object)->setEnabled(VPROP(GB_BOOLEAN));

END_PROPERTY


BEGIN_PROPERTY(CCONTROL_expand)

  if (READ_PROPERTY)
    GB.ReturnBoolean(THIS->flag.expand);
  else
  {
  	THIS->flag.expand = VPROP(GB_BOOLEAN);
    qApp->postEvent(WIDGET, new QEvent(EVENT_EXPAND));
  }

END_PROPERTY


BEGIN_PROPERTY(CCONTROL_ignore)

  if (READ_PROPERTY)
    GB.ReturnBoolean(THIS->flag.ignore);
  else
  {
  	THIS->flag.ignore = VPROP(GB_BOOLEAN);
    qApp->postEvent(WIDGET, new QEvent(EVENT_EXPAND));
  }

END_PROPERTY


BEGIN_METHOD(CCONTROL_move, GB_INTEGER x; GB_INTEGER y; GB_INTEGER w; GB_INTEGER h)

  move_resize_widget(_object, VARG(x), VARG(y), VARGOPT(w, -1), VARGOPT(h, -1));

END_METHOD


BEGIN_METHOD(CCONTROL_resize, GB_INTEGER w; GB_INTEGER h)

  resize_widget(_object, VARG(w), VARG(h));

END_METHOD


BEGIN_METHOD(CCONTROL_move_scaled, GB_FLOAT x; GB_FLOAT y; GB_FLOAT w; GB_FLOAT h)

  int x, y, w, h;

  x = (int)(VARG(x) * MAIN_scale);
  y = (int)(VARG(y) * MAIN_scale);
  w = (int)(MISSING(w) ? -1 : (VARG(w) * MAIN_scale));
  h = (int)(MISSING(h) ? -1 : (VARG(h) * MAIN_scale));

  move_resize_widget(_object, x, y, w, h);

END_METHOD


BEGIN_METHOD(CCONTROL_resize_scaled, GB_FLOAT w; GB_FLOAT h)

  int w, h;

  w = (int)(VARG(w) * MAIN_scale);
  h = (int)(VARG(h) * MAIN_scale);

  resize_widget(_object, w , h);

END_METHOD


BEGIN_METHOD_VOID(CCONTROL_delete)

  //if (WIDGET)
  //  qDebug("CWIDGET_delete: %p (%p)", THIS, WIDGET);

  CWIDGET_destroy(THIS);

END_METHOD


static bool is_visible(void *_object)
{
	return THIS->flag.visible || !QWIDGET(_object)->isHidden();
}


static void set_visible(void *_object, bool v)
{
	THIS->flag.visible = v;
	if (THIS->flag.visible)
		QWIDGET(_object)->show();
	else
		QWIDGET(_object)->hide();
}


BEGIN_PROPERTY(CCONTROL_visible)

  if (READ_PROPERTY)
    GB.ReturnBoolean(is_visible(THIS));
  else
  	set_visible(THIS, VPROP(GB_BOOLEAN));

END_PROPERTY


BEGIN_METHOD_VOID(CCONTROL_show)

	set_visible(THIS, true);

END_METHOD


BEGIN_METHOD_VOID(CCONTROL_hide)

	set_visible(THIS, false);

END_METHOD


BEGIN_METHOD_VOID(CCONTROL_raise)

  QWIDGET(_object)->raise();

END_METHOD


BEGIN_METHOD_VOID(CCONTROL_lower)

  QWIDGET(_object)->lower();

END_METHOD


BEGIN_METHOD(CCONTROL_move_under, GB_OBJECT control)

	CWIDGET *ob = (CWIDGET *)VARG(control);

	if (GB.CheckObject(ob))
		return;

	WIDGET->stackUnder(ob->widget);

END_METHOD


static QWidget *get_next(QWidget *w)
{
	QWidget *parent;
	QObjectList *children;
	QObject *current = NULL;

	parent = w->parentWidget();
	if (parent)
	{
		children = (QObjectList *)w->parentWidget()->children();
		if (children)
		{
			children->first();
			for(;;)
			{
				current = children->current();
				if (!current)
					break;
				children->next();
				if (current == w)
				{
					current = children->current();
					break;
				}
			}
		}
	}

	return (QWidget *)current;
}


static void arrange_parent(CWIDGET *_object)
{
	CWIDGET *parent = CWidget::get(WIDGET->parentWidget());
	if (!parent)
		return;
	CCONTAINER_arrange(parent);
}

BEGIN_PROPERTY(CCONTROL_next)

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


BEGIN_PROPERTY(CCONTROL_previous)

	if (READ_PROPERTY)
	{
		QWidget *parent;
		QObjectList *children;
		QObject *current = NULL;

		parent = WIDGET->parentWidget();
		if (parent)
		{
			children = (QObjectList *)WIDGET->parentWidget()->children();
			if (children)
			{
				children->first();
				for(;;)
				{
					current = children->current();
					if (!current)
						break;
					children->next();
					if (children->current() == WIDGET)
						break;
				}
			}
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


BEGIN_METHOD(CCONTROL_refresh, GB_INTEGER x; GB_INTEGER y; GB_INTEGER w; GB_INTEGER h)

  int x, y, w, h;

  if (!MISSING(x) && !MISSING(y))
  {
    x = VARG(x);
    y = VARG(y);
    w = VARGOPT(w, QWIDGET(_object)->width());
    h = VARGOPT(h, QWIDGET(_object)->height());
    QWIDGET(_object)->repaint(x, y, w, h);
  }
  else
    QWIDGET(_object)->repaint();

END_METHOD


BEGIN_METHOD_VOID(CCONTROL_set_focus)

  CWINDOW *win = CWidget::getWindow(THIS);

  if (QWIDGET(win)->isVisible())
    WIDGET->setFocus();
  else if ((CWIDGET *)win != THIS)
  {
    GB.Unref(POINTER(&win->focus));
    win->focus = THIS;
    GB.Ref(THIS);
  }

END_METHOD


BEGIN_PROPERTY(CCONTROL_tag)

  if (READ_PROPERTY)
    GB.ReturnPtr(GB_T_VARIANT, &OBJECT(CWIDGET)->tag);
  else
  {
    GB.StoreVariant(PROP(GB_VARIANT), (void *)&(OBJECT(CWIDGET)->tag));
    //printf("Set Tag %p : %i\n", _object, OBJECT(CWIDGET)->tag.type);
  }

END_METHOD


BEGIN_PROPERTY(CCONTROL_mouse)

  QWidget *wid = QWIDGET(_object);
  int shape;

  if (READ_PROPERTY)
  {
    if (wid->ownCursor())
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


BEGIN_PROPERTY(CCONTROL_cursor)

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

void CWIDGET_set_color(CWIDGET *_object, int bg, int fg)
{
	//qDebug("set_color: (%s %p) bg = %06X fg = %06X", GB.GetClassName(THIS), THIS, bg, fg);
	//if (bg == COLOR_DEFAULT && fg == COLOR_DEFAULT)
	WIDGET->unsetPalette();
  	
  if (bg != COLOR_DEFAULT)
    WIDGET->setPaletteBackgroundColor(QColor((QRgb)bg));
    
  if (fg != COLOR_DEFAULT)
    WIDGET->setPaletteForegroundColor(QColor((QRgb)fg));
	
	THIS->flag.default_bg = bg == COLOR_DEFAULT;
  THIS->flag.default_fg = fg == COLOR_DEFAULT;
}

int CWIDGET_get_background(CWIDGET *_object)
{
	if (THIS->flag.default_bg)
		return COLOR_DEFAULT;
	else
		return WIDGET->paletteBackgroundColor().rgb() & 0xFFFFFF;
}
	
int CWIDGET_get_foreground(CWIDGET *_object)
{
	if (THIS->flag.default_fg)
		return COLOR_DEFAULT;
	else
		return WIDGET->paletteForegroundColor().rgb() & 0xFFFFFF;
}
	
BEGIN_PROPERTY(CCONTROL_background)

  if (READ_PROPERTY)
    GB.ReturnInteger(CWIDGET_get_background(THIS));
  else
  {
  	int col = VPROP(GB_INTEGER);
  	if (col != CWIDGET_get_background(THIS))
  	{
			if (WIDGET->paletteBackgroundPixmap())
			{
				QPixmap p(*WIDGET->paletteBackgroundPixmap());
				CWIDGET_set_color(THIS, col, CWIDGET_get_foreground(THIS));
				WIDGET->setPaletteBackgroundPixmap(p);
			}
			else
				CWIDGET_set_color(THIS, col, CWIDGET_get_foreground(THIS));
		}
  }

END_PROPERTY


BEGIN_PROPERTY(CCONTROL_foreground)

  if (READ_PROPERTY)
    GB.ReturnInteger(CWIDGET_get_foreground(THIS));
  else
  {
  	int col = VPROP(GB_INTEGER);
  	if (col != CWIDGET_get_foreground(THIS))
  	{
			CWIDGET_set_color(THIS, CWIDGET_get_background(THIS), col);
    }
	}

END_PROPERTY


BEGIN_PROPERTY(CCONTROL_parent)

  QWidget *parent = QWIDGET(_object)->parentWidget();

  if (!parent || WIDGET->isTopLevel())
    GB.ReturnObject(NULL);
  else
    GB.ReturnObject(CWidget::get(parent));

END_PROPERTY


BEGIN_PROPERTY(CCONTROL_window)

  GB.ReturnObject(CWidget::getWindow(THIS));

END_PROPERTY


BEGIN_PROPERTY(CCONTROL_id)

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


BEGIN_PROPERTY(CCONTROL_tooltip)

  QWidget *w;

  if (READ_PROPERTY)
    GB.ReturnString(THIS->tooltip);
  else
  {
    GB.StoreString(PROP(GB_STRING), &(THIS->tooltip));
    w = WIDGET;
		if (CWIDGET_test_flag(THIS, WF_SCROLLVIEW))
			w = ((QScrollView *)w)->viewport();

		CWidget::resetTooltip(THIS);
  }

END_PROPERTY


BEGIN_PROPERTY(CCONTROL_name)

	if (READ_PROPERTY)
		GB.ReturnString(THIS->name);
	else
		set_name(THIS, GB.ToZeroString(PROP(GB_STRING)));

END_PROPERTY


BEGIN_PROPERTY(CCONTROL_action)

	if (READ_PROPERTY)
		CACTION_get(THIS);
	else
		CACTION_register(THIS, GB.ToZeroString(PROP(GB_STRING)));

END_PROPERTY


BEGIN_METHOD_VOID(CCONTROL_grab)

  GB.ReturnObject(CPICTURE_grab(QWIDGET(_object)));

END_METHOD


BEGIN_METHOD(CCONTROL_drag, GB_VARIANT data; GB_STRING format)

  CDRAG_drag(OBJECT(CWIDGET), &VARG(data), MISSING(format) ? NULL : ARG(format));

END_METHOD


BEGIN_METHOD(CCONTROL_reparent, GB_OBJECT container; GB_INTEGER x; GB_INTEGER y)

	QPoint p(WIDGET->pos());
	bool showIt = !WIDGET->isHidden();

	if (!MISSING(x) && !MISSING(y))
	{
		p.setX(VARG(x));
		p.setY(VARG(y));
	}

	if (GB.CheckObject(VARG(container)))
		return;

	WIDGET->reparent(QCONTAINER(VARG(container)), p, showIt);

END_METHOD


BEGIN_PROPERTY(CCONTROL_drop)

  if (READ_PROPERTY)
    GB.ReturnBoolean(WIDGET->acceptDrops());
  else
  {
    WIDGET->setAcceptDrops(VPROP(GB_BOOLEAN));
		if (CWIDGET_test_flag(THIS, WF_SCROLLVIEW))
    	((QScrollView *)WIDGET)->viewport()->setAcceptDrops(VPROP(GB_BOOLEAN));
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
    wid->repaint();
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
      wid->setFrameStyle(QFrame::LineEditPanel + QFrame::Sunken);
      //wid->setFrameStyle(QFrame::LineEditPanel);
      //wid->setLineWidth(2);
    }
    else
    {
      wid->setFrameStyle(QFrame::NoFrame);
    }

    //qDebug("--> %d", wid->frameStyle());

    wid->repaint();
  }

END_PROPERTY


BEGIN_PROPERTY(CWIDGET_scrollbar)

  QScrollView *wid = (QScrollView *)QWIDGET(_object);
  int scroll;

  if (READ_PROPERTY)
  {
    scroll = 0;
    if (wid->hScrollBarMode() == QScrollView::Auto)
      scroll += 1;
    if (wid->vScrollBarMode() == QScrollView::Auto)
      scroll += 2;

    GB.ReturnInteger(scroll);
  }
  else
  {
    scroll = VPROP(GB_INTEGER) & 3;
    wid->setHScrollBarMode( (scroll & 1) ? QScrollView::Auto : QScrollView::AlwaysOff);
    wid->setVScrollBarMode( (scroll & 2) ? QScrollView::Auto : QScrollView::AlwaysOff);
  }

END_PROPERTY



/* Classe CWidget */

CWidget CWidget::manager;
QPtrDict<CWIDGET> CWidget::dict;
bool CWidget::real;
CWIDGET *CWidget::enter = NULL;
//QPtrDict<char> CWidget::propDict;

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
  QObjectList *children;
  QObject *child;

  w->clearFocus();
  w->setFocusPolicy(QWidget::NoFocus);

  children = (QObjectList *)(w->children());

  if (!children)
    return;

  child = children->first();
  while (child)
  {
    if (child->isWidgetType())
      CWidget::removeFocusPolicy((QWidget *)child);

    child = children->next();
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
    if (ob != NULL)
      return ob;

    o = o->parent();
    real = false;
  }

  return NULL;
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

    o = o->parent();
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

void CWidget::resetTooltip(CWIDGET *_object)
{
  QToolTip::remove(WIDGET);
  if (THIS->tooltip)
    QToolTip::add(WIDGET, TO_QSTRING(THIS->tooltip));
}

void CWidget::destroy()
{
  QWidget *w = (QWidget *)sender();
  CWIDGET *ob = CWidget::get(w);

  if (ob == NULL)
    return;

  QEvent e(EVENT_DESTROY);
  
  //qDebug(">> CWidget::destroy %p (%p) :%p:%ld", ob, ob->widget, ob->ob.klass, ob->ob.ref);

  //w->blockSignals(TRUE);
  //w->disconnect();

  if (CWIDGET_destroy_list == ob)
    CWIDGET_destroy_list = ob->next;
  if (CWIDGET_destroy_last == ob)
    CWIDGET_destroy_last = ob->prev;
  if (ob->prev)
    ob->prev->next = ob->next;
  if (ob->next)
    ob->next->prev = ob->prev;

  if (enter == ob)
    enter = NULL;

	set_name(ob, 0);

  dict.remove(w);
  QWIDGET(ob) = NULL;

  GB.StoreVariant(NULL, &ob->tag);
  GB.FreeString(&ob->tooltip);
  GB.Unref(POINTER(&ob->cursor));
  GB.Unref(POINTER(&ob->font));

	CACTION_register(ob, NULL);
  //qDebug(">> CWidget::destroy %p (%p) :%p:%ld #2", ob, ob->widget, ob->ob.klass, ob->ob.ref);
  //if (!CWIDGET_test_flag(ob, WF_NODETACH))
  GB.Detach(ob);

  qApp->sendEvent(w, &e);
  //qDebug("<< CWidget::destroy %p (%p)", ob, ob->widget);

  GB.Unref(POINTER(&ob));
}

static void post_dblclick_event(void *control)
{
  GB.Raise(control, EVENT_DblClick, 0);
  GB.Unref(&control);
}

static void post_gotfocus_event(void *_object)
{
	if (WIDGET && WIDGET->hasFocus())
  	GB.Raise(THIS, EVENT_GotFocus, 0);
  GB.Unref(&_object);
}

static void post_lostfocus_event(void *control)
{
  GB.Raise(control, EVENT_LostFocus, 0);
  GB.Unref(&control);
}


bool CWidget::eventFilter(QObject *widget, QEvent *event)
{
  CWIDGET *control;
  int event_id;
  int type = event->type();
  bool real;
  bool design;
  bool original;
  int state;
  bool cancel;
  QPoint p;

	//if (widget->isA("MyMainWindow"))
	//	getDesignDebug(widget);

  //qDebug("eventFilter: DragEnter: (%s %p) %d", GB.GetClassName(control), control, event->type());

  control = CWidget::getDesign(widget);
  if (!control || GB.Is(control, CLASS_Menu))
    goto _STANDARD;

  real = CWidget::real;
  design = CWIDGET_test_flag(control, WF_DESIGN); // && !GB.Is(control, CLASS_Container);
  original = event->spontaneous();

  if (type == QEvent::Enter)
  {
    if (real)
      GB.Raise(control, EVENT_Enter, 0);
	}
  else if (type == QEvent::Leave)
  {
    if (real)
      GB.Raise(control, EVENT_Leave, 0);
  }
  else if (type == QEvent::FocusIn)
  {
  	if (GB.CanRaise(control, EVENT_GotFocus))
  	{
			GB.Ref(control);
			GB.Post((void (*)())post_gotfocus_event, (intptr_t)control);
		}

		CWINDOW_activate(control);
    //GB.Raise(control, EVENT_GotFocus, 0);
  }
  else if (type == QEvent::FocusOut)
  {
  	if (GB.CanRaise(control, EVENT_LostFocus))
  	{
			GB.Ref(control);
			GB.Post((void (*)())post_lostfocus_event, (intptr_t)control);
		}
    //GB.Raise(control, EVENT_LostFocus, 0);
  }
  else if (type == QEvent::ContextMenu)
  {
    // if (real && GB.CanRaise(control, EVENT_Menu))
    //qDebug("Menu event! %p %d", control, EVENT_Menu);
    if (GB.CanRaise(control, EVENT_Menu))
    {
      ((QContextMenuEvent *)event)->accept();
      GB.Raise(control, EVENT_Menu, 0);
			return true;
    }
  }
  else if ((type == QEvent::MouseButtonPress)
           || (type == QEvent::MouseButtonRelease)
           || (type == QEvent::MouseMove))
  {
    QMouseEvent *mevent = (QMouseEvent *)event;

    if (!original)
      goto _DESIGN;

		/*if (type == QEvent::MouseButtonPress)
		{
    	qDebug("mouse event on [%s %p] (%s %p) %s%s%s", widget->className(), widget, control ? GB.GetClassName(control) : "-", control, real ? "REAL " : "", 
    		design ? "DESIGN " : "", original ? "ORIGINAL ": "");
    	getDesignDebug(widget);
		}*/
    
		if (!real)
		{
			CCONTAINER *cont = (CCONTAINER *)CWidget::get(widget);
			if (CWIDGET_test_flag(cont, WF_SCROLLVIEW))
			{
				/*if (GB.Is(cont, CLASS_Container)
				{
					if (widget != cont->container)
						goto STANDARD;
				}
				else
				{*/
    			if (widget != ((QScrollView *)QWIDGET(cont))->viewport() && widget->name(0))
						goto _STANDARD;					
				//}
			}
		}
		
		p.setX(mevent->globalX());
		p.setY(mevent->globalY());
		p = QWIDGET(control)->mapFromGlobal(p);
    
    if (type == QEvent::MouseButtonPress)
    {
      //qDebug("MouseDown on %p (%s %p) %s%s", widget, control ? GB.GetClassName(control) : "-", control, real ? "REAL " : "", design ? "DESIGN " : "");

      event_id = EVENT_MouseDown;
      state = mevent->stateAfter();
      
      CMOUSE_info.sx = p.x();
      CMOUSE_info.sy = p.y();
      //qDebug("MouseEvent: %d %d", mevent->x(), mevent->y());
    }
    else
    {
      event_id = (type == QEvent::MouseButtonRelease) ? EVENT_MouseUp : EVENT_MouseMove;
      state = mevent->state();
    }

    if (event_id == EVENT_MouseMove && (state & QMouseEvent::MouseButtonMask) == 0 && !QWIDGET(control)->hasMouseTracking())
		  goto _DESIGN;


		/* GB.Raise() can free the control, so we must reference it as we may raise two successive events now */
		GB.Ref(control);
		cancel = false;

    if (GB.CanRaise(control, event_id))
    {
    	if (!design && CWIDGET_test_flag(control, WF_SCROLLVIEW))
    	{
    		/*if (widget != ((QScrollView *)QWIDGET(control))->viewport()
    		    && widget->name(0))
    		{
    			qDebug("cancel");
    			goto _DESIGN;
				}*/
    	}
    	
      CMOUSE_clear(true);
      CMOUSE_info.x = p.x();
      CMOUSE_info.y = p.y();
      CMOUSE_info.state = state;

      cancel = GB.Raise(control, event_id, 0); //, GB_T_INTEGER, p.x(), GB_T_INTEGER, p.y(), GB_T_INTEGER, state);

      CMOUSE_clear(false);
      
      /*if (CDRAG_dragging)
      	return true;*/
    }
    
    if (!cancel && event_id == EVENT_MouseMove && (state & QMouseEvent::MouseButtonMask)  && GB.CanRaise(control, EVENT_MouseDrag)
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
      CMOUSE_info.state = state;
		
			cancel = GB.Raise(control, EVENT_MouseDrag, 0);
      
      CMOUSE_clear(false);
		}
  
		GB.Unref(POINTER(&control));
		
		if (cancel)
			return true;
  }
  else if (type == QEvent::MouseButtonDblClick)
  {
    if (!original)
      goto _DESIGN;

    //GB.Raise(control, EVENT_DblClick, 0);
		if (GB.CanRaise(control, EVENT_DblClick))
		{
			GB.Ref(control);
			GB.Post((void (*)())post_dblclick_event, (intptr_t)control);
		}
  }
  else if ((type == QEvent::KeyPress)
           || (type == QEvent::KeyRelease))
  {
    QKeyEvent *kevent = (QKeyEvent *)event;

    #if QT_VERSION <= 0x030005
    if (!real || !original)
      goto _DESIGN;
    #endif

// 		qDebug("QKeyEvent: %s (%s %p) (%s %p) TL:%d -> %d %s",
// 			type == QEvent::KeyPress ? "KeyPress" : "KeyRelease",
// 			widget->className(), widget, GB.GetClassName(control), control,
// 			((QWidget *)widget)->isTopLevel(),  
// 			kevent->key(), (char *)kevent->text().latin1());

    event_id = (type == QEvent::KeyRelease) ? EVENT_KeyRelease : EVENT_KeyPress;

    #if QT_VERSION > 0x030005
    if (!original && type != QEvent::IMEnd)
      goto _DESIGN; //_ACCEL;
    #endif

    if (type == QEvent::KeyPress && GB.Is(control, CLASS_Window))
      goto _DESIGN; //_ACCEL;
      
    //qDebug("CWidget::eventFilter: KeyPress on %s %p", GB.GetClassName(control), control);

    if (GB.CanRaise(control, event_id))
    {
      CKEY_clear(true);

      GB.FreeString(&CKEY_info.text);
			GB.NewString(&CKEY_info.text, TO_UTF8(kevent->text()), 0);
			CKEY_info.state = kevent->state();
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
			
      cancel = GB.Raise(control, event_id, 0);

      CKEY_clear(false);

      if (cancel && (type != QEvent::KeyRelease))
        return true;
    }

/*_ACCEL:

    if (event_id == EVENT_KeyPress && CWINDOW_Main && ((QWidget *)widget)->isTopLevel())
    {
      //CWIDGET *top = CWidget::get(((QWidget *)widget)->topLevelWidget());
      CWIDGET *top = CWidget::get((QWidget *)widget);

      //qDebug("top = %p", top);

      if (!CWINDOW_Current && top && top != (CWIDGET *)CWINDOW_Main && QWIDGET(CWINDOW_Main))
      {
        //qDebug("post Accel to %p", CWINDOW_Main);
        //QMAINWINDOW(CWINDOW_Main)->setState(MyMainWindow::StateNormal);
        qApp->postEvent(QWIDGET(CWINDOW_Main),
          new QKeyEvent(QEvent::Accel, kevent->key(), kevent->ascii(), kevent->state(), kevent->text(), kevent->isAutoRepeat(), kevent->count()));
      }
    }*/
  }
  else if (type == QEvent::IMEnd)
  {
    QIMEvent *imevent = (QIMEvent *)event;

    #if QT_VERSION <= 0x030005
    if (!real || !original)
      goto _DESIGN;
    #endif

// 		qDebug("QIMEvent: IMEnd (%s %p) (%s %p) TL:%d",
// 			widget->className(), widget, GB.GetClassName(control), control,
// 			((QWidget *)widget)->isTopLevel());

    event_id = EVENT_KeyPress;

    if (GB.CanRaise(control, event_id))
    {
      CKEY_clear(true);

      GB.FreeString(&CKEY_info.text);
			//qDebug("IMEnd: %s", imevent->text().latin1());
			GB.NewString(&CKEY_info.text, TO_UTF8(imevent->text()), 0);
			CKEY_info.state = 0;
			CKEY_info.code = 0;

      cancel = GB.Raise(control, event_id, 0);

      CKEY_clear(false);

      if (cancel)
        return true;
    }
  }
  else if (type == QEvent::Wheel)
  {
    QWheelEvent *ev = (QWheelEvent *)event;

    //qDebug("Event on %p %s%s%s", widget,
    //  real ? "REAL " : "", design ? "DESIGN " : "", child ? "CHILD " : "");

    if (!original)
      goto _DESIGN;

    if (GB.CanRaise(control, EVENT_MouseWheel))
    {
      // Automatic focus for wheel events
    	((QWidget *)widget)->setFocus();
    	
      p.setX(ev->x());
      p.setY(ev->y());

      p = ((QWidget *)widget)->mapTo(QWIDGET(control), p);

      CMOUSE_clear(true);
      CMOUSE_info.x = p.x();
      CMOUSE_info.y = p.y();
      CMOUSE_info.state = ev->state();
      CMOUSE_info.orientation = ev->orientation();
      CMOUSE_info.delta = ev->delta();

      cancel = GB.Raise(control, EVENT_MouseWheel, 0);

      CMOUSE_clear(false);
    }
  }
	else if (type == QEvent::DragEnter)
	{
  	//if (!CWIDGET_test_flag(control, WF_NO_DRAG))
		if (CDRAG_drag_enter((QWidget *)widget, control, (QDropEvent *)event))
		{
			if (!((QDropEvent *)event)->isAccepted())
				CDRAG_hide_frame(control);
			return true;
		}
	}
	else if (type == QEvent::DragMove)
	{
  	//if (!CWIDGET_test_flag(control, WF_NO_DRAG))
		if (CDRAG_drag_move((QWidget *)widget, control, (QDropEvent *)event))
		{
			if (!((QDropEvent *)event)->isAccepted())
				CDRAG_hide_frame(control);
			return true;
		}
	}
	else if (type == QEvent::Drop)
	{
  	//if (!CWIDGET_test_flag(control, WF_NO_DRAG))
		CDRAG_drag_drop((QWidget *)widget, control, (QDropEvent *)event);
	}
	else if (type == QEvent::DragLeave)
	{
		CDRAG_hide_frame(control);
	}
    
  if (!control || CWIDGET_test_flag(control, WF_DELETED))
  {
    if (type != EVENT_DESTROY)
    {
      QObject::eventFilter(widget, event); 
      return true;
    }
  }
  
  /*if (CCONTROL_check(control))
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
        || (type == QEvent::IMStart)
        || (type == QEvent::IMCompose)
        || (type == QEvent::IMEnd)
        || (type == QEvent::Accel)
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

void CWIDGET_iconset(QIconSet &icon, QPixmap &pixmap, int size)
{
	QImage img;
	QPixmap disabled;
	QPixmap normal;

	img = pixmap.convertToImage().convertDepth(32);
	if (size > 0)
	{
		size = ((size + 1) & ~3);
		img = img.smoothScale(size, size, QImage::ScaleMax);
	}
	
	if (size <= 0)
		normal = pixmap;
	else
		normal.convertFromImage(img);
		
	icon = QIconSet(normal, QIconSet::Small);
	
	gray_image(img);
	
	disabled.convertFromImage(img);
	icon.setPixmap(disabled, QIconSet::Small, QIconSet::Disabled);
}


GB_DESC CControlDesc[] =
{
	GB_DECLARE("Control", sizeof(CCONTROL)), GB_NOT_CREATABLE(),

	GB_HOOK_CHECK(CCONTROL_check),

	GB_METHOD("_free", NULL, CCONTROL_delete, NULL),

	GB_METHOD("Move", NULL, CCONTROL_move, "(X)i(Y)i[(Width)i(Height)i]"),
	GB_METHOD("Resize", NULL, CCONTROL_resize, "(Width)i(Height)i"),

	GB_METHOD("MoveScaled", NULL, CCONTROL_move_scaled, "(X)f(Y)f[(Width)f(Height)f]"),
	GB_METHOD("ResizeScaled", NULL, CCONTROL_resize_scaled, "(Width)f(Height)f"),

	GB_METHOD("Delete", NULL, CCONTROL_delete, NULL),
	GB_METHOD("Show", NULL, CCONTROL_show, NULL),
	GB_METHOD("Hide", NULL, CCONTROL_hide, NULL),

	GB_METHOD("Raise", NULL, CCONTROL_raise, NULL),
	GB_METHOD("Lower", NULL, CCONTROL_lower, NULL),

	GB_PROPERTY("Next", "Control", CCONTROL_next),
	GB_PROPERTY("Previous", "Control", CCONTROL_previous),

	GB_METHOD("SetFocus", NULL, CCONTROL_set_focus, NULL),
	GB_METHOD("Refresh", NULL, CCONTROL_refresh, "[(X)i(Y)i(Width)i(Height)i]"),
	GB_METHOD("Grab", "Picture", CCONTROL_grab, NULL),
	GB_METHOD("Drag", NULL, CCONTROL_drag, "(Data)v[(Format)s]"),

	GB_METHOD("Reparent", NULL, CCONTROL_reparent, "(Parent)Container;[(X)i(Y)i]"),

	GB_PROPERTY("X", "i", CCONTROL_x),
	GB_PROPERTY("Y", "i", CCONTROL_y),
	GB_PROPERTY_READ("ScreenX", "i", CCONTROL_screen_x),
	GB_PROPERTY_READ("ScreenY", "i", CCONTROL_screen_y),
	GB_PROPERTY("W", "i", CCONTROL_w),
	GB_PROPERTY("H", "i", CCONTROL_h),
	GB_PROPERTY("Left", "i", CCONTROL_x),
	GB_PROPERTY("Top", "i", CCONTROL_y),
	GB_PROPERTY("Width", "i", CCONTROL_w),
	GB_PROPERTY("Height", "i", CCONTROL_h),

	GB_PROPERTY("Visible", "b", CCONTROL_visible),
	GB_PROPERTY("Enabled", "b", CCONTROL_enabled),
	
	GB_PROPERTY("Expand", "b", CCONTROL_expand),
	GB_PROPERTY("Ignore", "b", CCONTROL_ignore),

	GB_PROPERTY("Font", "Font", CCONTROL_font),
	GB_PROPERTY("Background", "i", CCONTROL_background),
	GB_PROPERTY("Foreground", "i", CCONTROL_foreground),

	GB_PROPERTY("Design", "b", CCONTROL_design),
	GB_PROPERTY("Name", "s", CCONTROL_name),
	GB_PROPERTY("Tag", "v", CCONTROL_tag),
	GB_PROPERTY("Mouse", "i", CCONTROL_mouse),
	GB_PROPERTY("Cursor", "Cursor", CCONTROL_cursor),
	GB_PROPERTY("ToolTip", "s", CCONTROL_tooltip),
	GB_PROPERTY("Drop", "b", CCONTROL_drop),
	GB_PROPERTY("Action", "s", CCONTROL_action),

	GB_PROPERTY_READ("Parent", "Container", CCONTROL_parent),
	GB_PROPERTY_READ("Window", "Window", CCONTROL_window),
	GB_PROPERTY_READ("Id", "i", CCONTROL_id),
	GB_PROPERTY_READ("Handle", "i", CCONTROL_id),

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



