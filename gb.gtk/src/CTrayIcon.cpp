/***************************************************************************

  CTrayIcon.cpp

  (c) 2004-2006 - Daniel Campos Fernández <dcamposf@gmail.com>

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

#include <stdio.h>

#include "gambas.h"
#include "widgets.h"
#include "CTrayIcon.h"
#include "CPicture.h"
#include "CContainer.h"
#include "CMenu.h"
#include "gmouse.h"

DECLARE_EVENT(EVENT_Click);
DECLARE_EVENT(EVENT_MiddleClick);
DECLARE_EVENT(EVENT_Scroll);

static void cb_destroy(gTrayIcon *sender)
{
	CTRAYICON *_object = (CTRAYICON*)sender->hFree;
	THIS->base.widget = NULL;
	GB.Unref(POINTER(&_object));
}

static void cb_click(gTrayIcon *sender, int button)
{
	if (button == 1)
		GB.Raise(sender->hFree, EVENT_Click, 0);
	else if (button == 2)
		GB.Raise(sender->hFree, EVENT_MiddleClick, 0);
}

static void cb_menu(gTrayIcon *sender)
{
	CTRAYICON *_object = (CTRAYICON *)sender->hFree;
	
	if (THIS->popup)
	{
		void *parent = GB.Parent(THIS);
		if (parent && !CWIDGET_check(parent) && GB.Is(parent, CLASS_Control))
		{
			gMainWindow *window = ((CWIDGET *)parent)->widget->window();
			gMenu *menu = gMenu::findFromName(window, THIS->popup);
			if (menu)
			{
				menu->popup();
				CMENU_check_popup_click();
			}
			return;
		}
	}

	//GB.Raise(sender->hFree, EVENT_Menu, 0);
}

static void cb_scroll(gTrayIcon *sender)
{
	GB.Raise(sender->hFree, EVENT_Scroll, 2, GB_T_FLOAT, (float)gMouse::delta(), GB_T_INTEGER, gMouse::orientation());
}

static int CTRAYICON_check(void *_object)
{
	return TRAYICON == NULL;
}

BEGIN_METHOD_VOID(TrayIcon_new)

	THIS->base.widget = new gTrayIcon();
	TRAYICON->hFree = (void*)THIS;
  
	THIS->base.tag.type = GB_T_NULL;
	
	TRAYICON->onClick = cb_click;
	TRAYICON->onMenu = cb_menu;
	TRAYICON->onDestroy = cb_destroy;
	TRAYICON->onScroll = cb_scroll;
	
	GB.Ref(THIS);
	//Add_Tray(_object);
	
END_METHOD

static void destroy_tray_icon(CTRAYICON *_object)
{
	if (TRAYICON) 
	{
		delete TRAYICON;
		THIS->base.widget = NULL;
		MAIN_check_quit();
	}
}

BEGIN_METHOD_VOID(TrayIcon_free)

	GB.StoreObject(NULL, POINTER(&THIS->picture));
	GB.StoreVariant(NULL, &THIS->base.tag);
	GB.FreeString(&THIS->popup);

	destroy_tray_icon(THIS);

END_METHOD


BEGIN_METHOD_VOID(TrayIcon_Delete)

	destroy_tray_icon(THIS);

END_METHOD


BEGIN_PROPERTY(TrayIcon_Picture)

	if (READ_PROPERTY)
	{
		GB.ReturnObject(THIS->picture);
		return;
	}
	
	GB.StoreObject(PROP(GB_OBJECT), POINTER(&THIS->picture));
	if (THIS->picture)
		TRAYICON->setPicture(THIS->picture->picture);
	else
		TRAYICON->setPicture(0);

END_PROPERTY

BEGIN_PROPERTY(TrayIcon_Text)

	if (READ_PROPERTY)
	{
		GB.ReturnNewZeroString(TRAYICON->tooltip());
		return;
	}
	
	TRAYICON->setTooltip(GB.ToZeroString(PROP(GB_STRING)));

END_PROPERTY


BEGIN_METHOD_VOID(TrayIcon_Show)

	TRAYICON->show();

END_METHOD

BEGIN_METHOD_VOID(TrayIcon_Hide)

	TRAYICON->hide();
	MAIN_check_quit();

END_METHOD

BEGIN_PROPERTY(TrayIcon_Visible)

	if (READ_PROPERTY)
		GB.ReturnBoolean(TRAYICON->isVisible());
	else
	{
		TRAYICON->setVisible(VPROP(GB_BOOLEAN));
		if (!VPROP(GB_BOOLEAN))
			MAIN_check_quit();
	}

END_PROPERTY

BEGIN_PROPERTY(TrayIcon_Tag)

	if (READ_PROPERTY)
		GB.ReturnVariant(&THIS->base.tag);
	else
		GB.StoreVariant(PROP(GB_VARIANT), (void *)&THIS->base.tag);

END_METHOD

BEGIN_PROPERTY(TrayIcons_Count)

	GB.ReturnInteger(gTrayIcon::count());

END_PROPERTY

BEGIN_METHOD(TrayIcons_get, GB_INTEGER index)

	int index = VARG(index);
	
	if (index < 0 || index >= gTrayIcon::count())
	{
		GB.Error("Bad index");
		return;
	}

	GB.ReturnObject(gTrayIcon::get(index)->hFree);

END_METHOD

BEGIN_METHOD_VOID(TrayIcons_next)

	int *vl;
	
	vl = (int *)GB.GetEnum();
	if (*vl >= gTrayIcon::count())
	{
		GB.StopEnum();
	}
	else
	{
		GB.ReturnObject (gTrayIcon::get(*vl)->hFree);
		(*vl)++;
	}

END_METHOD

BEGIN_PROPERTY(TrayIcon_PopupMenu)

	if (READ_PROPERTY)
		GB.ReturnString(THIS->popup);
	else
		GB.StoreString(PROP(GB_STRING), &(THIS->popup));

END_PROPERTY

BEGIN_METHOD_VOID(TrayIcon_unknown)

	static char prop[32];
	char *name = GB.GetUnknown();
	
	if (strcasecmp(name, "ScreenX") == 0 || strcasecmp(name, "ScreenY") == 0)
	{
		sprintf(prop, "TrayIcon.%s", name);
		GB.Deprecated(GTK_NAME, prop, NULL);
		
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
		GB.Deprecated(GTK_NAME, prop, NULL);
		
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

BEGIN_METHOD_VOID(TrayIcons_DeleteAll)

	gTrayIcon::exit();

END_METHOD

//---------------------------------------------------------------------------

GB_DESC TrayIconsDesc[] =
{
	GB_DECLARE("TrayIcons", 0), GB_NOT_CREATABLE(),
	
	GB_STATIC_PROPERTY_READ("Count", "i", TrayIcons_Count),
	GB_STATIC_METHOD("_get","TrayIcon", TrayIcons_get,"(Index)i"),
	GB_STATIC_METHOD("_next", "TrayIcon", TrayIcons_next, NULL),
	GB_STATIC_METHOD("DeleteAll", NULL, TrayIcons_DeleteAll, NULL),
	
	GB_END_DECLARE
};

GB_DESC TrayIconDesc[] =
{
	GB_DECLARE("TrayIcon", sizeof(CTRAYICON)), 
	GB_HOOK_CHECK(CTRAYICON_check),
	
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
