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
DECLARE_EVENT(EVENT_KeyPress);
DECLARE_EVENT(EVENT_KeyRelease);
DECLARE_EVENT(EVENT_Drag);
DECLARE_EVENT(EVENT_DragMove);
DECLARE_EVENT(EVENT_Drop);


long Tray_Count=0;
void **Tray_Elements=0;

void Add_Tray(void *element)
{
	if (!Tray_Elements)
	{
		GB.Alloc ((void**)&Tray_Elements,sizeof(void*));
		Tray_Count=1;
	}
	else
	{
		Tray_Count++;
		GB.Realloc ((void**)&Tray_Elements,Tray_Count*sizeof(void*));
	}
	Tray_Elements[Tray_Count-1]=element;
}

void Remove_Tray(void *element)
{
	long bucle,b2;

	for(bucle=0;bucle<Tray_Count;bucle++)
	{
		if (Tray_Elements[bucle]==element)
		{
			for (b2=bucle;b2<(Tray_Count-1);b2++)
				Tray_Elements[b2]=Tray_Elements[b2+1];
			
			Tray_Count--;
			if (!Tray_Count)
			{
				GB.Free((void**)&Tray_Elements);
				Tray_Elements=NULL;
			}
			else
			{
				GB.Realloc ((void**)&Tray_Elements,Tray_Count*sizeof(void*));	
			}
				
			return;
		}
	}
}


void Tray_destroy(gTrayIcon *sender)
{
	CTRAYICON *icon=(CTRAYICON*)sender->hFree;
	icon->icon=NULL;
	Remove_Tray((void*)icon);
	GB.Unref((void**)&icon);
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

	THIS->icon=new gTrayIcon();
	THIS->icon->hFree=(void*)THIS;
	TRAYICON->onMousePress=Tray_press;
	TRAYICON->onMouseRelease=Tray_release;
	TRAYICON->onMenu=Tray_menu;
	TRAYICON->onDestroy=Tray_destroy;
	TRAYICON->onFocusEnter=Tray_gotFocus;
	TRAYICON->onFocusLeave=Tray_lostFocus;
	TRAYICON->onDoubleClick=Tray_dblClick;
	TRAYICON->onEnter=Tray_enter;
	TRAYICON->onLeave=Tray_leave;
	GB.Ref(_object);
	Add_Tray(_object);
	
END_METHOD

BEGIN_METHOD_VOID(CTRAYICON_free)

	if (THIS->picture) { GB.Unref((void**)&THIS->picture); THIS->picture=NULL; }

	Remove_Tray(_object);
	if (TRAYICON) {
		TRAYICON->destroy();
		TRAYICON=NULL;
	}

END_METHOD


BEGIN_METHOD_VOID(CTRAYICON_destroy)

	TRAYICON->destroy();
	TRAYICON=NULL;

END_METHOD


BEGIN_PROPERTY(CTRAYICON_picture)

	CPICTURE *pic;
	gPicture *img;
	
	if (READ_PROPERTY)
	{
		GB.ReturnObject(THIS->picture);
		return;
	}
	
	pic=(CPICTURE*)VPROP(GB_OBJECT);
	
	if (!pic) { GB.Error("Null picture"); return; }
	if (!pic->picture) { GB.Error("Null picture"); return; }
	
	GB.Ref((void*)pic);
	if (THIS->picture) GB.Unref((void**)&THIS->picture);
	THIS->picture=pic;
	TRAYICON->setPicture(pic->picture);

END_PROPERTY

BEGIN_PROPERTY(CTRAYICON_tooltip)

	if (READ_PROPERTY)
	{
		GB.ReturnNewString(TRAYICON->toolTip(),0);
		return;
	}
	
	TRAYICON->setToolTip(GB.ToZeroString(PROP(GB_STRING)));

END_PROPERTY

BEGIN_METHOD_VOID(CTRAYICON_exit)
	
	CTRAYICON *el;
	
	while (Tray_Elements)
	{ 
		el=(CTRAYICON*)Tray_Elements[0];
		if (el->icon) el->icon->destroy();
		Remove_Tray((void*)el);
	}

END_METHOD

BEGIN_METHOD_VOID(CTRAYICON_show)

	TRAYICON->show();

END_METHOD

BEGIN_METHOD_VOID(CTRAYICON_hide)

	TRAYICON->hide();

END_METHOD

BEGIN_PROPERTY(CTRAYICON_visible)

	if (READ_PROPERTY) { GB.ReturnBoolean(TRAYICON->visible()); return; }
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

	GB.ReturnInteger(Tray_Count);

END_PROPERTY

BEGIN_METHOD (CTRAYICONS_get,GB_INTEGER Index;)

	long Index=VARG(Index);
	
	if ( (Index<0) || (Index>=Tray_Count) )
	{
		GB.Error("Bad index");
		return;
	}

	GB.ReturnObject(Tray_Elements[Index]);

END_METHOD

BEGIN_METHOD_VOID(CTRAYICONS_next)

	long *vl;
	
	vl=(long*)GB.GetEnum();
	if (Tray_Count<=vl[0])
	{
		GB.StopEnum();
	}
	else
	{
		GB.ReturnObject ( Tray_Elements[vl[0]] );
		vl[0]++;
	}

END_METHOD

GB_DESC CTrayIconsDesc[] =
{
	GB_DECLARE("TrayIcons", 0), GB_NOT_CREATABLE(),
	
	GB_STATIC_PROPERTY_READ("Count","i",CTRAYICONS_Count),
	GB_STATIC_METHOD("_get","TrayIcon",CTRAYICONS_get,"(Index)i"),
	GB_STATIC_METHOD("_next", "TrayIcon", CTRAYICONS_next, NULL),
	
	GB_END_DECLARE
};

GB_DESC CTrayIconDesc[] =
{
	GB_DECLARE("TrayIcon", sizeof(CTRAYICON)), 
	GB_HOOK_CHECK(CTRAYICON_check),
	
	GB_METHOD("_new",NULL,CTRAYICON_new,NULL),
	GB_METHOD("_free",NULL,CTRAYICON_free,NULL),
	GB_STATIC_METHOD("_exit",NULL,CTRAYICON_exit,NULL),
	
	GB_METHOD("Show", NULL, CTRAYICON_show, NULL),
	GB_METHOD("Hide", NULL, CTRAYICON_hide, NULL),
	GB_METHOD("Delete",NULL,CTRAYICON_destroy,NULL),
	
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
		
	GB_EVENT("MouseDown", NULL, NULL, &EVENT_MouseDown),
	GB_EVENT("MouseUp", NULL, NULL, &EVENT_MouseUp),
	GB_EVENT("Menu", NULL, NULL, &EVENT_Menu),
	GB_EVENT("DblClick", NULL, NULL, &EVENT_DblClick),
	GB_EVENT("Enter", NULL, NULL, &EVENT_Enter), 
	GB_EVENT("Leave", NULL, NULL, &EVENT_Leave), 
	GB_EVENT("GotFocus", NULL, NULL, &EVENT_GotFocus),
	GB_EVENT("LostFocus", NULL, NULL, &EVENT_LostFocus),
	GB_EVENT("KeyPress", NULL, NULL, &EVENT_KeyPress), //TODO
	GB_EVENT("KeyRelease", NULL, NULL, &EVENT_KeyRelease), //TODO
    GB_EVENT("MouseMove", NULL, NULL, &EVENT_MouseMove), //TODO
	GB_EVENT("MouseWheel", NULL, NULL, &EVENT_MouseWheel), //TODO
    GB_EVENT("Drag", NULL, NULL, &EVENT_Drag), //TODO
	GB_EVENT("DragMove", NULL, NULL, &EVENT_DragMove), //TODO
	GB_EVENT("Drop", NULL, NULL, &EVENT_Drop), //TODO
	
	GB_CONSTANT("_Properties", "s", "Visible=False,Tag,Tooltip,Picture"),
	GB_CONSTANT("_DefaultEvent", "s", "Menu"),
	
	GB_END_DECLARE
};







