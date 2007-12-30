/***************************************************************************

  CMenu.cpp

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

#define __CMENU_CPP

#include "main.h"
#include "gambas.h"
#include "CWidget.h"
#include "CWindow.h"
#include "CPicture.h"
#include "CMenu.h"


static void *CLASS_Menu;
static void *CLASS_Window;

DECLARE_EVENT(EVENT_Click);
DECLARE_EVENT(EVENT_Show);

int CMENU_check(void *_object)
{
	if (!MENU) return true;
  	return false;
}

void CMENU_onFinish(gMenu *sender)
{
	CMENU *Mn=(CMENU*)sender->hFree;
	if (Mn) { Mn->widget=NULL; GB.Unref((void**)&Mn);}
}

void CMENU_onClick(gMenu *sender)
{
	CMENU *Mn=(CMENU*)sender->hFree;
	if (Mn) GB.Raise((void*)Mn,EVENT_Click,0);
}

BEGIN_METHOD_VOID(CMENU_init)

  CLASS_Menu = GB.FindClass("Menu");
  CLASS_Window = GB.FindClass("Window");

END_METHOD



BEGIN_METHOD(CMENU_new, GB_OBJECT parent; GB_BOOLEAN hidden)

	void *parent=VARG(parent);
	bool hidden=false;
	
	if (!MISSING(hidden)) hidden=VARG(hidden);
	
	if (GB.Is(parent,CLASS_Window))
	{
		//TODO: Windows that are not top-level windows
		if ( !((CWINDOW*)parent)->widget )
		{
			GB.Error("Invalid window");
			return;
		}
		THIS->widget=new gMenu( (gMainWindow*)((CWINDOW*)parent)->widget,hidden);
		MENU->hFree=(void*)THIS;
		MENU->onFinish=CMENU_onFinish;
		GB.Ref((void*)THIS);
		return;
	}
	
	if (GB.Is(parent,CLASS_Menu))
	{
		if ( !((CMENU*)parent)->widget )
		{
			GB.Error("Invalid menu");
			return;
		}
		THIS->widget=new gMenu( (gMenu*)((CMENU*)parent)->widget,hidden);
		MENU->hFree=(void*)THIS;
		MENU->onFinish=CMENU_onFinish;
		MENU->onClick=CMENU_onClick;
		GB.Ref((void*)THIS);
		return;
	}
	
	GB.Error("Type mismatch. The parent control of a Menu must be a Window or another Menu.");


END_METHOD


BEGIN_METHOD_VOID(CMENU_free)

	if (THIS->picture) { GB.Unref((void**)&THIS->picture); THIS->picture=NULL; }
	if (MENU) MENU->destroy();

END_METHOD


BEGIN_PROPERTY(CMENU_text)

	if (READ_PROPERTY)
	{
		GB.ReturnNewString(MENU->text(),0);
		return;
	}
	MENU->setText(GB.ToZeroString(PROP(GB_STRING)));

END_PROPERTY


BEGIN_PROPERTY(CMENU_picture)

	CPICTURE *pic=NULL;
	gPicture *hPic=NULL;

	if (READ_PROPERTY)
	{
		GB.ReturnObject(THIS->picture);
		return;
	}
	
	pic=(CPICTURE*)VPROP(GB_OBJECT);
	if (pic) GB.Ref((void*)pic);
	if (THIS->picture) GB.Unref((void**)&THIS->picture);
	THIS->picture=pic;
	if (!pic) MENU->setPicture(NULL);
	else      MENU->setPicture(pic->picture);

END_PROPERTY


BEGIN_PROPERTY(CMENUITEM_enabled)

	if (READ_PROPERTY) { GB.ReturnBoolean(MENU->enabled()); return; }
	MENU->setEnabled(VPROP(GB_BOOLEAN));

END_PROPERTY


BEGIN_PROPERTY(CMENUITEM_checked)

	if (READ_PROPERTY) { GB.ReturnBoolean(MENU->checked()); return; }
	MENU->setChecked(VPROP(GB_BOOLEAN));

END_PROPERTY


BEGIN_PROPERTY(CMENU_shortcut)

	if (READ_PROPERTY)
	{
		GB.ReturnNewString(MENU->shortcut(),0);
		return;
	}
	
	MENU->setShortcut(GB.ToZeroString(PROP(GB_STRING)));

END_PROPERTY


BEGIN_PROPERTY(CMENU_visible)

	if (READ_PROPERTY) { GB.ReturnBoolean(MENU->visible()); return; }
	MENU->setVisible(VPROP(GB_BOOLEAN));

END_PROPERTY


BEGIN_METHOD_VOID(CMENU_delete)

	MENU->destroy();

END_METHOD


BEGIN_PROPERTY(CMENU_count)

	GB.ReturnInteger(MENU->childCount());

END_PROPERTY


BEGIN_METHOD_VOID(CMENU_next)

	CMENU *Mn;
	gMenu *mn;
	long *ct;
	
	ct=(long*)GB.GetEnum();
	
	if ( ct[0]>=MENU->childCount()  ) { GB.StopEnum(); return; }
	mn=MENU->childMenu(ct[0]);
	Mn=(CMENU*)mn->hFree;
	ct[0]++;
	GB.ReturnObject(Mn);

END_PROPERTY


BEGIN_METHOD_VOID(CMENU_clear)

	gMenu *mn;

	long bucle,max;
	
	max=MENU->childCount();
	
	for (bucle=0;bucle<max;bucle++)
	{
		mn=MENU->childMenu(0);
		if (mn) mn->destroy();
	}

END_PROPERTY


BEGIN_METHOD_VOID(CMENU_popup)

	MENU->popup();

END_METHOD


BEGIN_PROPERTY(CMENU_tag)

	if (READ_PROPERTY) { GB.ReturnPtr(GB_T_VARIANT, &THIS->tag); return; }
    GB.StoreVariant(PROP(GB_VARIANT), (void *)&THIS->tag);
  
END_METHOD


GB_DESC CMenuChildrenDesc[] =
{
  GB_DECLARE(".MenuChildren", sizeof(CMENU)), GB_VIRTUAL_CLASS(),

  GB_METHOD("_next", "Menu", CMENU_next, NULL),
  GB_METHOD("Clear", NULL, CMENU_clear, NULL),
  GB_PROPERTY_READ("Count", "i", CMENU_count),

  GB_END_DECLARE
};


GB_DESC CMenuDesc[] =
{
  GB_DECLARE("Menu", sizeof(CMENU)), 
  GB_HOOK_CHECK(CMENU_check),

  GB_STATIC_METHOD("_init", NULL, CMENU_init, NULL),
  GB_METHOD("_new", NULL, CMENU_new, "(Parent)o[(Hidden)b]"),
  GB_METHOD("_free", NULL, CMENU_free, NULL),


  GB_PROPERTY("Caption", "s", CMENU_text),
  GB_PROPERTY("Text", "s", CMENU_text),
  GB_PROPERTY("Enabled", "b", CMENUITEM_enabled),
  GB_PROPERTY("Checked", "b", CMENUITEM_checked),
  GB_PROPERTY("Tag", "v", CMENU_tag),
  GB_PROPERTY("Picture", "Picture", CMENU_picture),
  GB_PROPERTY("Shortcut", "s", CMENU_shortcut),
  GB_PROPERTY("Visible", "b", CMENU_visible),

  GB_PROPERTY_SELF("Children", ".MenuChildren"),

  GB_CONSTANT("_Properties", "s", CMENU_PROPERTIES),
  GB_CONSTANT("_DefaultEvent", "s", "Click"),

  GB_METHOD("Popup", NULL, CMENU_popup, NULL),
  GB_METHOD("Delete", NULL, CMENU_delete, NULL),

  GB_EVENT("Click", NULL, NULL, &EVENT_Click),
  GB_EVENT("Show", NULL, NULL, &EVENT_Show),

  GB_END_DECLARE
};


