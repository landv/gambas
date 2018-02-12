/***************************************************************************

  ctrayicon.cpp

  (c) 2000-2017 Benoît Minisini <g4mba5@gmail.com>

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

#include "gambas.h"
#include "main.h"

#include <QMenu>
#include <QWheelEvent>

#include "gb.form.properties.h"
#include "ctrayicon.h"

DECLARE_EVENT(EVENT_Click);
DECLARE_EVENT(EVENT_MiddleClick);
DECLARE_EVENT(EVENT_Scroll);

static QList<CTRAYICON *> _list;
static QPixmap *_default_trayicon = NULL;

#include "gb.form.trayicon.large.h"

//---------------------------------------------------------------------------

static void destroy_trayicon(CTRAYICON *_object)
{
	if (TRAYICON)
	{
		TRAYICON->deleteLater();
		THIS->indicator = NULL;
		QT_PreventQuit(false);
	}
}

static void delete_all(void)
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
		destroy_trayicon(THIS);
		GB.Unref(POINTER(&_object));
	}
	
	_list.clear();
}

static void define_tooltip(CTRAYICON *_object)
{
	if (!TRAYICON)
		return;

	TRAYICON->setToolTip(TO_QSTRING(THIS->tooltip));
}

static void define_icon(CTRAYICON *_object)
{
	if (!TRAYICON)
		return;

	if (THIS->icon)
		TRAYICON->setIcon(*THIS->icon->pixmap);
	else
	{
		if (!_default_trayicon)
		{
			_default_trayicon = new QPixmap;
			_default_trayicon->loadFromData(_default_trayicon_data, sizeof(_default_trayicon_data), "PNG");
		}

		TRAYICON->setIcon(*_default_trayicon);
	}
}

static void define_menu(CTRAYICON *_object)
{
	QMenu *menu;
	
	if (!TRAYICON)
		return;
	
	menu = NULL;
	
	if (THIS->popup)
		menu = QT_FindMenu(GB.Parent(THIS), THIS->popup);
	
	TRAYICON->setContextMenu(menu);
}

static CTRAYICON *find_trayicon(const QObject *o)
{
	int i;
	CTRAYICON *_object;

	for (i = 0; i < _list.count(); i++)
	{
		_object = _list.at(i);
		if (TRAYICON && THIS->indicator == o)
			return THIS;
	}

	return NULL;
}

//---------------------------------------------------------------------------

BEGIN_METHOD_VOID(TrayIcon_new)

	THIS->tag.type = GB_T_NULL;
	_list.append(THIS);
	GB.Ref(THIS);

END_METHOD


BEGIN_METHOD_VOID(TrayIcon_free)

	//qDebug("TrayIcon_free");

	_list.removeAt(_list.indexOf(THIS));

	GB.StoreObject(NULL, POINTER(&THIS->icon));
	GB.FreeString(&THIS->tooltip);
	GB.FreeString(&THIS->popup);
	GB.StoreVariant(NULL, &THIS->tag);

	destroy_trayicon(THIS);

END_METHOD


BEGIN_PROPERTY(TrayIcon_Picture)

	if (READ_PROPERTY)
		GB.ReturnObject(THIS->icon);
	else
	{
		GB.StoreObject(PROP(GB_OBJECT), POINTER(&THIS->icon));
		define_icon(THIS);
	}
	

END_PROPERTY


BEGIN_PROPERTY(TrayIcon_Text)

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
	{
		GB.StoreString(PROP(GB_STRING), &(THIS->popup));
		define_menu(THIS);
	}

END_PROPERTY


BEGIN_METHOD_VOID(TrayIcon_Show)

	if (!TRAYICON)
	{
		QSystemTrayIcon *indicator = new QSystemTrayIcon();

		QObject::connect(indicator, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), &TrayIconManager::manager, SLOT(activated(QSystemTrayIcon::ActivationReason)));
		indicator->installEventFilter(&TrayIconManager::manager);
		
		THIS->indicator = indicator;
		QT_PreventQuit(true);

		define_tooltip(THIS);
		define_icon(THIS);
		define_menu(THIS);

		TRAYICON->show();
	}

END_METHOD


BEGIN_METHOD_VOID(TrayIcon_Hide)

	destroy_trayicon(THIS);

END_METHOD


BEGIN_PROPERTY(TrayIcon_Visible)

	if (READ_PROPERTY)
	{
		GB.ReturnBoolean(TRAYICON != NULL);
		return;
	}

	if (VPROP(GB_BOOLEAN))
		TrayIcon_Show(_object, _param);
	else
		TrayIcon_Hide(_object, _param);

END_PROPERTY


BEGIN_PROPERTY(TrayIcon_Tag)

	if (READ_PROPERTY)
		GB.ReturnVariant(&THIS->tag);
	else
		GB.StoreVariant(PROP(GB_VARIANT), (void *)&THIS->tag);

END_METHOD


BEGIN_METHOD_VOID(TrayIcons_next)

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


BEGIN_METHOD(TrayIcons_get, GB_INTEGER index)

	int index = (uint)VARG(index);

	if (index >= _list.count())
	{
		GB.Error("Bad index");
		return;
	}
	
	GB.ReturnObject(_list.at(index));

END_METHOD


BEGIN_PROPERTY(TrayIcons_Count)

	GB.ReturnInteger(_list.count());

END_PROPERTY

BEGIN_METHOD_VOID(TrayIcon_unknown)

	static char prop[32];
	char *name = GB.GetUnknown();
	
	if (strcasecmp(name, "ScreenX") == 0 || strcasecmp(name, "ScreenY") == 0)
	{
		sprintf(prop, "TrayIcon.%s", name);
		GB.Deprecated(QT_NAME, prop, NULL);
		
		if (READ_PROPERTY)
		{
			GB.ReturnInteger(0);
			GB.ReturnConvVariant();
			return;
		}
		else
			GB.Error(GB_ERR_NWRITE, GB.GetClassName(NULL), name);
	}
	else if (strcasecmp(name, "W") == 0 || strcasecmp(name, "Width") == 0 || strcasecmp(name, "H") == 0 || strcasecmp(name, "Height") == 0)
	{
		sprintf(prop, "TrayIcon.%s", name);
		GB.Deprecated(QT_NAME, prop, NULL);
		
		if (READ_PROPERTY)
		{
			GB.ReturnInteger(24);
			GB.ReturnConvVariant();
			return;
		}
		else
			GB.Error(GB_ERR_NWRITE, GB.GetClassName(NULL), name);
	}
	else
	{
		GB.Error(GB_ERR_NSYMBOL, GB.GetClassName(NULL), name);
	}

END_METHOD

BEGIN_METHOD_VOID(TrayIcon_exit)

	if (_default_trayicon)
		delete _default_trayicon;

END_METHOD

BEGIN_METHOD_VOID(TrayIcons_DeleteAll)

	delete_all();

END_METHOD

//---------------------------------------------------------------------------

TrayIconManager TrayIconManager::manager;

void TrayIconManager::activated(QSystemTrayIcon::ActivationReason reason)
{
	CTRAYICON *_object = find_trayicon(sender());
	if (THIS)
	{
		//qDebug("reason = %d", (int)reason);
		switch(reason)
		{
			//case QSystemTrayIcon::DoubleClick:
			//	GB.Raise(THIS, EVENT_TrayIconDblClick, 0);
			//	break;
				
			case QSystemTrayIcon::Trigger:
				GB.Raise(THIS, EVENT_Click, 0);
				break;
			
			case QSystemTrayIcon::MiddleClick:
				GB.Raise(THIS, EVENT_MiddleClick, 0);
				break;
			
			default:
				break;
		}
	}
}

bool TrayIconManager::eventFilter(QObject *o, QEvent *e)
{
	if (e->type() == QEvent::Wheel)
	{
		CTRAYICON *_object = find_trayicon(o);
		if (THIS)
		{
			bool cancel;
			QWheelEvent *ev = (QWheelEvent *)e;
			
			cancel = GB.Raise(THIS, EVENT_Scroll, 2, GB_T_FLOAT, ev->delta() / 120.0, GB_T_INTEGER, ev->orientation() == Qt::Vertical);
			
			if (cancel)
				return true;
		}
	}
	
	return QObject::eventFilter(o, e);
}

//---------------------------------------------------------------------------

GB_DESC TrayIconsDesc[] =
{
	GB_DECLARE_STATIC("TrayIcons"),

	GB_STATIC_METHOD("_next", "TrayIcon", TrayIcons_next, NULL),
	GB_STATIC_METHOD("_get", "TrayIcon", TrayIcons_get, "(Index)i"),
	GB_STATIC_PROPERTY_READ("Count", "i", TrayIcons_Count),
	GB_STATIC_METHOD("DeleteAll", NULL, TrayIcons_DeleteAll, NULL),

	GB_END_DECLARE
};


GB_DESC TrayIconDesc[] =
{
	GB_DECLARE("TrayIcon", sizeof(CTRAYICON)),

	GB_STATIC_METHOD("_exit", NULL, TrayIcon_exit, NULL),

	GB_CONSTANT("Horizontal", "i", 0),
	GB_CONSTANT("Vertical", "i", 1),

	GB_METHOD("_new", NULL, TrayIcon_new, NULL),
	GB_METHOD("_free", NULL, TrayIcon_free, NULL),

	GB_METHOD("Show", NULL, TrayIcon_Show, NULL),
	GB_METHOD("Hide", NULL, TrayIcon_Hide, NULL),
	GB_METHOD("Delete", NULL, TrayIcon_Hide, NULL),

	GB_PROPERTY("Picture", "Picture", TrayIcon_Picture),
	GB_PROPERTY("Icon", "Picture", TrayIcon_Picture),
	GB_PROPERTY("Visible", "b", TrayIcon_Visible),

	GB_PROPERTY("Text", "s", TrayIcon_Text),
	GB_PROPERTY("PopupMenu", "s", TrayIcon_PopupMenu),
	GB_PROPERTY("Tooltip", "s", TrayIcon_Text),
	GB_PROPERTY("Tag", "v", TrayIcon_Tag),
	
	GB_EVENT("Click", NULL, NULL, &EVENT_Click),
	GB_EVENT("MiddleClick", NULL, NULL, &EVENT_MiddleClick),
	GB_EVENT("Scroll", NULL, "(Delta)f(Orientation)i", &EVENT_Scroll),

	GB_METHOD("_unknown", "v", TrayIcon_unknown, "."),

	TRAYICON_DESCRIPTION,

	GB_END_DECLARE
};

