/***************************************************************************

  CTrayIcon.cpp

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

#define __CTRAYICON_CPP

#include <qnamespace.h>
#include <qapplication.h>
#include <qtooltip.h>
#include <qpixmap.h>
#include <qbitmap.h>
#include <qevent.h>
#include <qeventloop.h>

//Added by qt3to4:
#include <QContextMenuEvent>
#include <QWheelEvent>
#include <QMouseEvent>

#include "gambas.h"
#include "main.h"

#include "CMouse.h"
#define DO_NOT_DECLARE_EVENTS
#include "CWidget.h"
#include "CMenu.h"
#include "CWindow.h"
#include "x11.h"
#include "CTrayIcon.h"

DECLARE_METHOD(Control_ScreenX);
DECLARE_METHOD(Control_ScreenY);
DECLARE_METHOD(Control_Width);
DECLARE_METHOD(Control_Height);

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

int TRAYICON_count = 0;

static int _state;
static QList<CTRAYICON *> _list;
static QImage _default_trayicon;

#include "gb.form.trayicon.h"

/** MyTrayIcon ***********************************************************/

MyTrayIcon::MyTrayIcon() : SystemTrayIcon()
{
	if (_default_trayicon.isNull())
		_default_trayicon = QImage(_default_trayicon_data, DEFAULT_TRAYICON_WIDTH, DEFAULT_TRAYICON_HEIGHT, QImage::Format_ARGB32_Premultiplied);

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

void MyTrayIcon::drawContents(QPainter *p)
{
	//p->drawPixmap((width() - _icon.width()) / 2, (height() - _icon.height()) / 2, _icon);
	p->drawPixmap(0, 0, width(), height(), _icon);
}


static CTRAYICON *find_object(const QObject *o)
{
	int i;
	CTRAYICON *_object;

	for (i = 0; i < _list.count(); i++)
	{
		_object = _list.at(i);
		if (THIS->widget == o)
			return THIS;
	}

	return NULL;
}

static void destroy_widget(CTRAYICON *_object)
{
	if (WIDGET)
	{
		WIDGET->deleteLater();
		THIS->widget = NULL;
		TRAYICON_count--;
		MAIN_check_quit();
	}
}

void CTRAYICON_close_all(void)
{
	CTRAYICON *_object, *last = 0;
	int i;

	GB.StopAllEnum((void *)GB.FindClass("TrayIcons"));
	
	i = 0;
	for (;;)
	{
		if (i >= _list.count())
			break;
		
		_object = _list.at(i);
		
		if (_object == last)
		{
			i++;
			continue;
		}
		
		last = _object;
		destroy_widget(THIS);
		GB.Unref(POINTER(&_object));
	}
	
	_list.clear();
}


BEGIN_METHOD_VOID(CTRAYICON_new)

	THIS->tag.type = GB_T_NULL;
	_list.append(THIS);
	GB.Ref(THIS);

END_METHOD


BEGIN_METHOD_VOID(CTRAYICON_free)

	//qDebug("CTRAYICON_free");

	_list.removeAt(_list.indexOf(THIS));

	GB.StoreObject(NULL, POINTER(&THIS->icon));
	GB.FreeString(&THIS->tooltip);
	GB.FreeString(&THIS->popup);
	GB.StoreVariant(NULL, &THIS->tag);

	destroy_widget(THIS);

END_METHOD


static void define_mask(CTRAYICON *_object)
{
	QPixmap *p;

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
	//qDebug("resize: %d %d", p->width(), p->height());

	if (!THIS->icon)
		delete p;
	
	/*#ifndef NO_X_WINDOW
	// Needed, otherwise the icon does not appear in Gnome or XFCE notification area!
	XSizeHints hints;
	hints.flags = PMinSize;
	hints.min_width = WIDGET->width();
	hints.min_height = WIDGET->height();
	XSetWMNormalHints(WIDGET->x11Display(), WIDGET->winId(), &hints);
	qDebug("set hints: %ld %d %d", WIDGET->winId(), WIDGET->width(), WIDGET->height());
	#endif*/
}

static void define_tooltip(CTRAYICON *_object)
{
	if (!WIDGET)
		return;

	WIDGET->setToolTip(TO_QSTRING(THIS->tooltip));
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

BEGIN_PROPERTY(TrayIcon_PopupMenu)

	if (READ_PROPERTY)
		GB.ReturnString(THIS->popup);
	else
		GB.StoreString(PROP(GB_STRING), &(THIS->popup));

END_PROPERTY


BEGIN_METHOD_VOID(CTRAYICON_show)

	if (!WIDGET)
	{
		MyTrayIcon *wid = new MyTrayIcon();
		//wid->setFocusPolicy(Qt::NoFocus);
		wid->installEventFilter(&CTrayIcon::manager);

		THIS->widget = wid;
		TRAYICON_count++;

		//QObject::connect(WIDGET, SIGNAL(embedded()), &CTrayIcon::manager, SLOT(embedded()));
		//QObject::connect(WIDGET, SIGNAL(containerClosed()), &CTrayIcon::manager, SLOT(closed()));
		//QObject::connect(WIDGET, SIGNAL(error(QX11EmbedWidget::Error)), &CTrayIcon::manager, SLOT(error()));

		//qDebug("XEMBED: EmbedState: %d", CWINDOW_EmbedState);
		define_mask(THIS);
		define_tooltip(THIS);
		//X11_window_dock(WIDGET->winId());

		/*_state = EMBED_WAIT;
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
		}*/
		
		#ifndef NO_X_WINDOW
		if (WIDGET->locateSystemTray() == None)
		{
			destroy_widget(THIS);
			GB.Error("Unable to find system tray");
			return;
		}
		WIDGET->addToTray();
		#else
		WIDGET->show();
		#endif
		//define_mask(THIS);
		//define_tooltip(THIS);
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
		GB.ReturnVariant(&THIS->tag);
	else
		GB.StoreVariant(PROP(GB_VARIANT), (void *)&THIS->tag);

END_METHOD


BEGIN_METHOD_VOID(CTRAYICON_next)

	int index;

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

	int index = (uint)VARG(index);

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
		Control_ScreenX(_object, _param);
	else
		GB.ReturnInteger(0);

END_PROPERTY


BEGIN_PROPERTY(CTRAYICON_screen_y)

	if (WIDGET)
		Control_ScreenY(_object, _param);
	else
		GB.ReturnInteger(0);

END_PROPERTY


BEGIN_PROPERTY(CTRAYICON_w)

	if (WIDGET)
		Control_Width(_object, _param);
	else
		GB.ReturnInteger(0);

END_PROPERTY


BEGIN_PROPERTY(CTRAYICON_h)

	if (WIDGET)
		Control_Height(_object, _param);
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
	GB_PROPERTY("PopupMenu", "s", TrayIcon_PopupMenu),
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
	QPoint p;

	_object = find_object(widget);

	if (!_object)
		goto __STANDARD;

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
		if (THIS->popup)
		{
			void *parent = GB.Parent(THIS);
			if (parent && GB.Is(parent, CLASS_Control))
			{
				CWINDOW *window = CWidget::getWindow((CWIDGET *)parent);
				CMENU *menu = CWindow::findMenu(window, THIS->popup);
				if (menu)
					CMENU_popup(menu, QCursor::pos());
				return true;
			}
		}
	}
	else if ((type == QEvent::MouseButtonPress)
					|| (type == QEvent::MouseButtonRelease)
					|| (type == QEvent::MouseMove))
	{
		QMouseEvent *mevent = (QMouseEvent *)event;

		if (!original)
			goto __DESIGN;

		if (type == QEvent::MouseButtonPress)
		{
			event_id = EVENT_MouseDown;
		}
		else
		{
			event_id = (type == QEvent::MouseButtonRelease) ? EVENT_MouseUp : EVENT_MouseMove;
		}

		if (GB.CanRaise(THIS, event_id))
		{
			p.setX(mevent->globalX());
			p.setY(mevent->globalY());

			p = WIDGET->mapFromGlobal(p);

			CMOUSE_clear(true);
			MOUSE_info.x = p.x();
			MOUSE_info.y = p.y();
			MOUSE_info.screenX = mevent->globalX();
			MOUSE_info.screenY = mevent->globalY();
			MOUSE_info.button = mevent->button();
			MOUSE_info.state = mevent->buttons();
			MOUSE_info.modifier = mevent->modifiers();

			GB.Raise(THIS, event_id, 0);

			CMOUSE_clear(false);
		}
	}
	else if (type == QEvent::MouseButtonDblClick)
	{
		if (!original)
			goto __DESIGN;

		GB.Raise(THIS, EVENT_DblClick, 0);
	}
	else if (type == QEvent::Wheel)
	{
		QWheelEvent *ev = (QWheelEvent *)event;

		//qDebug("Event on %p %s%s%s", widget,
		//  real ? "REAL " : "", design ? "DESIGN " : "", child ? "CHILD " : "");

		if (!original)
			goto __DESIGN;

		if (GB.CanRaise(THIS, EVENT_MouseWheel))
		{
			p.setX(ev->x());
			p.setY(ev->y());

			//p = WIDGET->mapTo(QWIDGET(control), p);

			CMOUSE_clear(true);
			MOUSE_info.x = p.x();
			MOUSE_info.y = p.y();
			MOUSE_info.screenX = ev->globalX();
			MOUSE_info.screenY = ev->globalY();
			MOUSE_info.state = ev->buttons();
			MOUSE_info.modifier = ev->modifiers();
			MOUSE_info.orientation = ev->orientation();
			MOUSE_info.delta = ev->delta();

			GB.Raise(THIS, EVENT_MouseWheel, 0);

			CMOUSE_clear(false);
		}
	}

__DESIGN:

	if (!find_object(widget))
		return true;
		
__STANDARD:
	
	return QObject::eventFilter(widget, event);    // standard event processing
}
