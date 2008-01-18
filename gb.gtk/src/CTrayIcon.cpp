/***************************************************************************

  CTrayIcon.cpp

  (c) 2004-2006 - Daniel Campos Fernández <dcamposf@gmail.com>
  
  GTK+ component
  
  Realizado para la Junta de Extremadura. 
  Consejería de Educación Ciencia y Tecnología. 
  Proyecto gnuLinEx

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

#include <stdio.h>

#include "gambas.h"
#include "widgets.h"
#include "CTrayIcon.h"
#include "CPicture.h"
#include "CContainer.h"

DECLARE_EVENT(EVENT_DblClick);
DECLARE_EVENT(EVENT_MouseMove);
DECLARE_EVENT(EVENT_MouseWheel);
DECLARE_EVENT(EVENT_MouseDown);
DECLARE_EVENT(EVENT_MouseUp);
DECLARE_EVENT(EVENT_Menu);
DECLARE_EVENT(EVENT_Enter);
DECLARE_EVENT(EVENT_Leave);
DECLARE_EVENT(EVENT_GotFocus);
DECLARE_EVENT(EVENT_LostFocus);

void Tray_destroy(gTrayIcon *sender)
{
	CTRAYICON *icon=(CTRAYICON*)sender->hFree;
	icon->icon=NULL;
	GB.Unref(POINTER(&icon));
}

void Tray_enter(gTrayIcon *sender)
{
	GB.Raise(sender->hFree,EVENT_Enter,0);
}

void Tray_leave(gTrayIcon *sender)
{
	GB.Raise(sender->hFree,EVENT_Leave,0);
}

void Tray_dblClick(gTrayIcon *sender)
{
	GB.Raise(sender->hFree,EVENT_DblClick,0);
}

void Tray_press(gTrayIcon *sender)
{
	GB.Raise(sender->hFree,EVENT_MouseDown,0);
}

void Tray_release(gTrayIcon *sender)
{
	GB.Raise(sender->hFree,EVENT_MouseUp,0);
}

void Tray_menu(gTrayIcon *sender)
{
	GB.Raise(sender->hFree,EVENT_Menu,0);
}

void Tray_gotFocus(gTrayIcon *sender)
{
	GB.Raise(sender->hFree,EVENT_GotFocus,0);
}

void Tray_lostFocus(gTrayIcon *sender)
{
	GB.Raise(sender->hFree,EVENT_LostFocus,0);
}

int CTRAYICON_check(void *_object)
{
	if (!TRAYICON) return true;
  	return false;
}

BEGIN_METHOD_VOID(CTRAYICON_new)

	THIS->icon = new gTrayIcon();
	THIS->icon->hFree = (void*)THIS;
  
  THIS->tag.type = GB_T_NULL;
	
	TRAYICON->onMousePress=Tray_press;
	TRAYICON->onMouseRelease=Tray_release;
	TRAYICON->onMenu=Tray_menu;
	TRAYICON->onDestroy=Tray_destroy;
	TRAYICON->onFocusEnter=Tray_gotFocus;
	TRAYICON->onFocusLeave=Tray_lostFocus;
	TRAYICON->onDoubleClick=Tray_dblClick;
	TRAYICON->onEnter=Tray_enter;
	TRAYICON->onLeave=Tray_leave;
	
	GB.Ref(THIS);
	//Add_Tray(_object);
	
END_METHOD

BEGIN_METHOD_VOID(CTRAYICON_free)

  GB.StoreObject(NULL, POINTER(&THIS->picture));
  GB.StoreVariant(NULL, &THIS->tag);

	//Remove_Tray(_object);
	if (TRAYICON) 
	{
		delete TRAYICON;
		TRAYICON=NULL;
	}

END_METHOD


BEGIN_METHOD_VOID(CTRAYICON_destroy)

	if (TRAYICON) 
	{
		delete TRAYICON;
		TRAYICON=NULL;
	}

END_METHOD


BEGIN_PROPERTY(CTRAYICON_picture)

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

BEGIN_PROPERTY(CTRAYICON_tooltip)

	if (READ_PROPERTY)
	{
		GB.ReturnNewString(TRAYICON->toolTip(),0);
		return;
	}
	
	TRAYICON->setToolTip(GB.ToZeroString(PROP(GB_STRING)));

END_PROPERTY


BEGIN_METHOD_VOID(CTRAYICON_show)

	TRAYICON->show();

END_METHOD

BEGIN_METHOD_VOID(CTRAYICON_hide)

	TRAYICON->hide();

END_METHOD

BEGIN_PROPERTY(CTRAYICON_visible)

	if (READ_PROPERTY) { GB.ReturnBoolean(TRAYICON->isVisible()); return; }
	TRAYICON->setVisible(VPROP(GB_BOOLEAN));

END_PROPERTY

BEGIN_PROPERTY(CTRAYICON_screen_x)

	GB.ReturnInteger(TRAYICON->screenX());

END_PROPERTY

BEGIN_PROPERTY(CTRAYICON_screen_y)

	GB.ReturnInteger(TRAYICON->screenY());

END_PROPERTY

BEGIN_PROPERTY(CTRAYICON_width)

	GB.ReturnInteger(TRAYICON->width());

END_PROPERTY

BEGIN_PROPERTY(CTRAYICON_height)

	GB.ReturnInteger(TRAYICON->height());

END_PROPERTY

BEGIN_PROPERTY(CTRAYICON_tag)

  if (READ_PROPERTY)
    GB.ReturnPtr(GB_T_VARIANT, &THIS->tag);
  else
    GB.StoreVariant(PROP(GB_VARIANT), (void *)&THIS->tag);

END_METHOD

BEGIN_PROPERTY(CTRAYICONS_Count)

	GB.ReturnInteger(gTrayIcon::count());

END_PROPERTY

BEGIN_METHOD (CTRAYICONS_get, GB_INTEGER index)

	int index = VARG(index);
	
	if (index < 0 || index >= gTrayIcon::count())
	{
		GB.Error("Bad index");
		return;
	}

	GB.ReturnObject(gTrayIcon::get(index));

END_METHOD

BEGIN_METHOD_VOID(CTRAYICONS_next)

	int *vl;
	
	vl = (int *)GB.GetEnum();
	if (*vl >= gTrayIcon::count())
	{
		GB.StopEnum();
	}
	else
	{
		GB.ReturnObject (gTrayIcon::get(*vl));
		(*vl)++;
	}

END_METHOD


GB_DESC CTrayIconsDesc[] =
{
	GB_DECLARE("TrayIcons", 0), GB_NOT_CREATABLE(),
	
	GB_STATIC_PROPERTY_READ("Count","i",CTRAYICONS_Count),
	GB_STATIC_METHOD("_get","TrayIcon",CTRAYICONS_get,"(Index)i"),
	GB_STATIC_METHOD("_next", "TrayIcon", CTRAYICONS_next, 0),
	
	GB_END_DECLARE
};

GB_DESC CTrayIconDesc[] =
{
	GB_DECLARE("TrayIcon", sizeof(CTRAYICON)), 
	GB_HOOK_CHECK(CTRAYICON_check),
	
	GB_METHOD("_new",0,CTRAYICON_new,0),
	GB_METHOD("_free",0,CTRAYICON_free,0),
	
	GB_METHOD("Show", 0, CTRAYICON_show, 0),
	GB_METHOD("Hide", 0, CTRAYICON_hide, 0),
	GB_METHOD("Delete",0,CTRAYICON_destroy,0),
	
	GB_PROPERTY("Visible","b",CTRAYICON_visible),
	GB_PROPERTY("Picture","Picture",CTRAYICON_picture),
	GB_PROPERTY("Icon","Picture",CTRAYICON_picture),
	GB_PROPERTY("Tooltip","s",CTRAYICON_tooltip),
	GB_PROPERTY("Text","s",CTRAYICON_tooltip),
	GB_PROPERTY("Tag", "v", CTRAYICON_tag),
	
	GB_PROPERTY_READ("ScreenX","i",CTRAYICON_screen_x),
	GB_PROPERTY_READ("ScreenY","i",CTRAYICON_screen_y),
	GB_PROPERTY_READ("Width","i",CTRAYICON_width),
	GB_PROPERTY_READ("Height","i",CTRAYICON_height),
	GB_PROPERTY_READ("W","i",CTRAYICON_width),
	GB_PROPERTY_READ("H","i",CTRAYICON_height),
		
	GB_EVENT("MouseDown", 0, 0, &EVENT_MouseDown),
	GB_EVENT("MouseUp", 0, 0, &EVENT_MouseUp),
	GB_EVENT("Menu", 0, 0, &EVENT_Menu),
	GB_EVENT("DblClick", 0, 0, &EVENT_DblClick),
	GB_EVENT("Enter", 0, 0, &EVENT_Enter), 
	GB_EVENT("Leave", 0, 0, &EVENT_Leave), 
	GB_EVENT("GotFocus", 0, 0, &EVENT_GotFocus),
	GB_EVENT("LostFocus", 0, 0, &EVENT_LostFocus),
	GB_EVENT("MouseMove", 0, 0, &EVENT_MouseMove), //TODO
	GB_EVENT("MouseWheel", 0, 0, &EVENT_MouseWheel), //TODO
	
	//GB_CONSTANT("_Properties", "s", "Visible=False,Tag,Tooltip,Picture"),
	TRAYICON_DESCRIPTION,
	
	GB_END_DECLARE
};







