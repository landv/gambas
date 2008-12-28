/***************************************************************************

  CTrayIcon.cpp

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

#define __CTRAYICON_CPP

#include <qnamespace.h>
#include <qapplication.h>
#include <qtooltip.h>
#include <qpixmap.h>
#include <qbitmap.h>
#include <qptrlist.h>
#include <qevent.h>

#if QT_VERSION >= 0x030100
  #include <qeventloop.h>
#endif

#include "gambas.h"
#include "main.h"
#include "x11.h"

#include "CMouse.h"
#define DO_NOT_DECLARE_EVENTS
#include "CWidget.h"
#include "CTrayIcon.h"

DECLARE_METHOD(CCONTROL_screen_x);
DECLARE_METHOD(CCONTROL_screen_y);
DECLARE_METHOD(CCONTROL_w);
DECLARE_METHOD(CCONTROL_h);

DECLARE_EVENT(EVENT_MouseDown);
DECLARE_EVENT(EVENT_MouseUp);
DECLARE_EVENT(EVENT_MouseMove);
DECLARE_EVENT(EVENT_MouseWheel);
DECLARE_EVENT(EVENT_DblClick);
DECLARE_EVENT(EVENT_Enter);
DECLARE_EVENT(EVENT_Leave);
DECLARE_EVENT(EVENT_GotFocus);
DECLARE_EVENT(EVENT_LostFocus);
DECLARE_EVENT(EVENT_Menu);

static int _state;
static QList<CTRAYICON> _list;

#include "gb.form.trayicon.h"

/** MyTrayIcon ***********************************************************/

MyTrayIcon::MyTrayIcon() : QtXEmbedClient()
{
	setBackgroundMode(X11ParentRelative);
	_icon = QPixmap(_default_trayicon);
}

void MyTrayIcon::setIcon(QPixmap &icon)
{ 
	if (icon.isNull())
		_icon = QPixmap(_default_trayicon);
	else
		_icon = icon; 
	update();
}

void MyTrayIcon::paintEvent(QPaintEvent *e)
{
	QPainter p(this);
	p.drawPixmap((width() - _icon.width()) / 2, (height() - _icon.height()) / 2, _icon);
}


static CTRAYICON *find_object(const QObject *o)
{
  QListIterator<CTRAYICON> it(_list);
  CTRAYICON *_object;

  while ((_object = it.current()))
  {
    ++it;
    if (THIS->widget == o)
      return THIS;
  }

  return NULL;
}

static void destroy_widget(CTRAYICON *_object)
{
  if (WIDGET)
  {
    delete WIDGET;
    THIS->widget = NULL;
  }
}

void CTRAYICON_close_all(void)
{
  //qDebug("CTRAYICON_close_all");

  QListIterator<CTRAYICON> it(_list);
  CTRAYICON *_object;

  GB.StopAllEnum(GB.FindClass("TrayIcons"));
  
  while ((_object = it.current()))
  {
    ++it;
    destroy_widget(THIS);
    GB.Unref(POINTER(&_object));
  }
}


BEGIN_METHOD_VOID(CTRAYICON_new)

  THIS->tag.type = GB_T_NULL;
  _list.append((const CTRAYICON *)THIS);
  GB.Ref(THIS);

END_METHOD


BEGIN_METHOD_VOID(CTRAYICON_free)

  //qDebug("CTRAYICON_free");

  _list.removeRef((const CTRAYICON *)THIS);

  GB.StoreObject(NULL, POINTER(&THIS->icon));
  GB.FreeString(&THIS->tooltip);
  GB.StoreVariant(NULL, &THIS->tag);

  destroy_widget(THIS);

END_METHOD


static void define_mask(CTRAYICON *_object)
{
  QPixmap *p;
  XSizeHints hints;

  if (!WIDGET)
    return;

  if (THIS->icon)
    p = THIS->icon->pixmap;
  else
    p = new QPixmap(_default_trayicon);

  /*WIDGET->clearMask();
  if (p->hasAlpha())
    WIDGET->setMask(*(p->mask()));*/

  WIDGET->setIcon(*p);
  //WIDGET->setPaletteBackgroundColor(QColor(255, 0, 0));
  WIDGET->resize(p->width(), p->height());

  if (!THIS->icon)
    delete p;
 
  // Needed, otherwise the icon does not appear in Gnome of XFCE notification area!
	hints.flags = PMinSize;
	hints.min_width = WIDGET->width();
	hints.min_height = WIDGET->height();
  XSetWMNormalHints(WIDGET->x11Display(), WIDGET->winId(), &hints);
}


static void define_tooltip(CTRAYICON *_object)
{
  char *tooltip;

  if (!WIDGET)
    return;

  tooltip = THIS->tooltip;
  if (tooltip)
    QToolTip::add(WIDGET, TO_QSTRING(tooltip));
  else
    QToolTip::remove(WIDGET);
}


BEGIN_PROPERTY(CTRAYICON_picture)

  if (READ_PROPERTY)
  {
    GB.ReturnObject(THIS->icon);
    return;
  }

  GB.StoreObject(PROP(GB_OBJECT), POINTER(&THIS->icon));
  define_mask(THIS);

END_PROPERTY


BEGIN_PROPERTY(CTRAYICON_tooltip)

  if (READ_PROPERTY)
    GB.ReturnString(THIS->tooltip);
  else
  {
    GB.StoreString(PROP(GB_STRING), &(THIS->tooltip));
    define_tooltip(THIS);
  }

END_PROPERTY


BEGIN_METHOD_VOID(CTRAYICON_show)

	int i;

  if (!WIDGET)
  {
    MyTrayIcon *wid = new MyTrayIcon();
    wid->setFocusPolicy(QWidget::NoFocus);
    wid->installEventFilter(&CTrayIcon::manager);

    THIS->widget = wid;
    
    QObject::connect(WIDGET, SIGNAL(embedded()), &CTrayIcon::manager, SLOT(embedded()));
    //QObject::connect(WIDGET, SIGNAL(containerClosed()), &CTrayIcon::manager, SLOT(closed()));
    QObject::connect(WIDGET, SIGNAL(error(int)), &CTrayIcon::manager, SLOT(error()));

    define_mask(THIS);
    define_tooltip(THIS);
    
    X11_window_dock(WIDGET->winId());

    _state = EMBED_WAIT;
    for(i = 0; i < 500; i++)
    {
      MAIN_process_events();
      if (_state)
        break;
      usleep(10000);
    }
    
    if (_state != EMBED_OK)
    {
      GB.Error("Embedding has failed");
      destroy_widget(THIS);
      return;
		}

    wid->show();
  }

END_METHOD


BEGIN_METHOD_VOID(CTRAYICON_hide)

  destroy_widget(THIS);

END_METHOD


BEGIN_PROPERTY(CTRAYICON_visible)

  if (READ_PROPERTY)
  {
    GB.ReturnBoolean(WIDGET != NULL);
    return;
  }

  if (VPROP(GB_BOOLEAN))
    CTRAYICON_show(_object, _param);
  else
    CTRAYICON_hide(_object, _param);

END_PROPERTY


BEGIN_PROPERTY(CTRAYICON_tag)

  if (READ_PROPERTY)
    GB.ReturnPtr(GB_T_VARIANT, &THIS->tag);
  else
    GB.StoreVariant(PROP(GB_VARIANT), (void *)&THIS->tag);

END_METHOD


BEGIN_METHOD_VOID(CTRAYICON_next)

  unsigned int index;

  index = ENUM(int);

  if (index >= _list.count())
  {
    GB.StopEnum();
    return;
  }

  ENUM(int) = index + 1;

  GB.ReturnObject(_list.at(index));

END_METHOD


BEGIN_METHOD(CTRAYICON_get, GB_INTEGER index)

  uint index = (uint)VARG(index);

  if (index >= _list.count())
	{
		GB.Error("Bad index");
		return;
	}
	
  GB.ReturnObject(_list.at(index));

END_METHOD


BEGIN_PROPERTY(CTRAYICON_count)

  GB.ReturnInteger(_list.count());

END_PROPERTY

BEGIN_PROPERTY(CTRAYICON_screen_x)

	if (WIDGET)
		CCONTROL_screen_x(_object, _param);
	else
		GB.ReturnInteger(0);

END_PROPERTY


BEGIN_PROPERTY(CTRAYICON_screen_y)

	if (WIDGET)
		CCONTROL_screen_y(_object, _param);
	else
		GB.ReturnInteger(0);

END_PROPERTY


BEGIN_PROPERTY(CTRAYICON_w)

	if (WIDGET)
		CCONTROL_w(_object, _param);
	else
		GB.ReturnInteger(0);

END_PROPERTY


BEGIN_PROPERTY(CTRAYICON_h)

	if (WIDGET)
		CCONTROL_h(_object, _param);
	else
		GB.ReturnInteger(0);

END_PROPERTY




GB_DESC CTrayIconsDesc[] =
{
  GB_DECLARE("TrayIcons", 0), GB_NOT_CREATABLE(),

  GB_STATIC_METHOD("_next", "TrayIcon", CTRAYICON_next, NULL),
  GB_STATIC_METHOD("_get", "TrayIcon", CTRAYICON_get, "(Index)i"),
  GB_STATIC_PROPERTY_READ("Count", "i", CTRAYICON_count),

  GB_END_DECLARE
};


GB_DESC CTrayIconDesc[] =
{
  GB_DECLARE("TrayIcon", sizeof(CTRAYICON)),

  GB_METHOD("_new", NULL, CTRAYICON_new, NULL),
  GB_METHOD("_free", NULL, CTRAYICON_free, NULL),

  GB_METHOD("Show", NULL, CTRAYICON_show, NULL),
  GB_METHOD("Hide", NULL, CTRAYICON_hide, NULL),
  GB_METHOD("Delete", NULL, CTRAYICON_hide, NULL),

  GB_PROPERTY("Picture", "Picture", CTRAYICON_picture),
  GB_PROPERTY("Icon", "Picture", CTRAYICON_picture),
  GB_PROPERTY("Visible", "b", CTRAYICON_visible),

  GB_PROPERTY("Text", "s", CTRAYICON_tooltip),
  GB_PROPERTY("Tooltip", "s", CTRAYICON_tooltip),
  GB_PROPERTY("Tag", "v", CTRAYICON_tag),

  GB_PROPERTY_READ("ScreenX", "i", CTRAYICON_screen_x),
  GB_PROPERTY_READ("ScreenY", "i", CTRAYICON_screen_y),
  GB_PROPERTY_READ("Width", "i", CTRAYICON_w),
  GB_PROPERTY_READ("Height", "i", CTRAYICON_h),
  GB_PROPERTY_READ("W", "i", CTRAYICON_w),
  GB_PROPERTY_READ("H", "i", CTRAYICON_h),

  GB_EVENT("Enter", NULL, NULL, &EVENT_Enter),
  GB_EVENT("GotFocus", NULL, NULL, &EVENT_GotFocus),
  GB_EVENT("LostFocus", NULL, NULL, &EVENT_LostFocus),
  GB_EVENT("Leave", NULL, NULL, &EVENT_Leave),
  GB_EVENT("MouseDown", NULL, NULL, &EVENT_MouseDown),
  GB_EVENT("MouseMove", NULL, NULL, &EVENT_MouseMove),
  GB_EVENT("MouseUp", NULL, NULL, &EVENT_MouseUp),
  GB_EVENT("MouseWheel", NULL, NULL, &EVENT_MouseWheel),
  GB_EVENT("DblClick", NULL, NULL, &EVENT_DblClick),
  GB_EVENT("Menu", NULL, NULL, &EVENT_Menu),

	TRAYICON_DESCRIPTION,

  GB_END_DECLARE
};


/*--- CTrayIcon -----------------------------------------------------------------------------------------*/

CTrayIcon CTrayIcon::manager;

void CTrayIcon::error(void)
{
  //CWINDOW *_object = (CWINDOW *)CWidget::getReal((QObject *)sender());
  //qDebug("XEMBED: CWindow::error %p -> %p", sender(), THIS);
  _state = EMBED_ERROR;
}

void CTrayIcon::embedded(void)
{
  //CWINDOW *_object = (CWINDOW *)CWidget::getReal((QObject *)sender());
  //qDebug("XEMBED: CWindow::embedded %p -> %p", sender(), THIS);
  _state = EMBED_OK;
}

void CTrayIcon::closed(void)
{
  //CWINDOW *_object = (CWINDOW *)CWidget::getReal((QObject *)sender());
  //qDebug("XEMBED: CWindow::closed %p -> %p", sender(), THIS);
  //CWIDGET_destroy(CWidget::getReal((QObject *)sender()));
  destroy_widget(find_object(sender()));
}


bool CTrayIcon::eventFilter(QObject *widget, QEvent *event)
{
  CTRAYICON *_object;
  int event_id;
  int type = event->type();
  bool original;
  int state;
  QPoint p;

  _object = find_object(widget);

  if (!_object)
		goto _STANDARD;

  original = event->spontaneous();

  if (type == QEvent::Enter)
  {
    GB.Raise(THIS, EVENT_Enter, 0);
  }
  else if (type == QEvent::Leave)
  {
    GB.Raise(THIS, EVENT_Leave, 0);
  }
  else if (type == QEvent::FocusIn)
  {
    GB.Raise(THIS, EVENT_GotFocus, 0);
  }
  else if (type == QEvent::FocusOut)
  {
    GB.Raise(THIS, EVENT_LostFocus, 0);
  }
  else if (type == QEvent::ContextMenu)
  {
    if (GB.CanRaise(THIS, EVENT_Menu))
    {
      ((QContextMenuEvent *)event)->accept();
      GB.Raise(THIS, EVENT_Menu, 0);
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

    if (type == QEvent::MouseButtonPress)
    {
      event_id = EVENT_MouseDown;
      state = mevent->stateAfter();
    }
    else
    {
      event_id = (type == QEvent::MouseButtonRelease) ? EVENT_MouseUp : EVENT_MouseMove;
      state = mevent->state();
    }

    if (GB.CanRaise(THIS, event_id))
    {
      p.setX(mevent->globalX());
      p.setY(mevent->globalY());

      p = WIDGET->mapFromGlobal(p);

      CMOUSE_clear(true);
      CMOUSE_info.x = p.x();
      CMOUSE_info.y = p.y();
      CMOUSE_info.state = state;

      GB.Raise(THIS, event_id, 0);

      CMOUSE_clear(false);
    }
  }
  else if (type == QEvent::MouseButtonDblClick)
  {
    if (!original)
      goto _DESIGN;

    GB.Raise(THIS, EVENT_DblClick, 0);
  }
  else if (type == QEvent::Wheel)
  {
    QWheelEvent *ev = (QWheelEvent *)event;

    //qDebug("Event on %p %s%s%s", widget,
    //  real ? "REAL " : "", design ? "DESIGN " : "", child ? "CHILD " : "");

    if (!original)
      goto _DESIGN;

    if (GB.CanRaise(THIS, EVENT_MouseWheel))
    {
      p.setX(ev->x());
      p.setY(ev->y());

      //p = WIDGET->mapTo(QWIDGET(control), p);

      CMOUSE_clear(true);
      CMOUSE_info.x = p.x();
      CMOUSE_info.y = p.y();
      CMOUSE_info.state = ev->state();
      CMOUSE_info.orientation = ev->orientation();
      CMOUSE_info.delta = ev->delta();

      GB.Raise(THIS, EVENT_MouseWheel, 0);

      CMOUSE_clear(false);
    }
  }

_DESIGN:

	if (!find_object(widget))
		return true;

_STANDARD:
		
  return QObject::eventFilter(widget, event);    // standard event processing
}
