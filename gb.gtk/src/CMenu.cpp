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
DECLARE_EVENT(EVENT_Hide);


static void send_click_event(void *_object)
{
	if (MENU->toggle())
		MENU->setChecked(!MENU->checked());
  GB.Raise(THIS, EVENT_Click, 0);
	CACTION_raise(THIS);
  GB.Unref(POINTER(&_object));
}

static int CMENU_check(void *_object)
{
	if (!MENU) return true;
  	return false;
}

static void cb_finish(gMenu *sender)
{
	CMENU *_object = (CMENU*)sender->hFree;
	if (_object) 
	{ 
		//CACTION_register(THIS, NULL);
		THIS->widget = NULL;
		GB.Unref(POINTER(&_object));
	}
}

static void cb_click(gMenu *sender)
{
	void *_object = sender->hFree;
	GB.Ref(THIS);
	GB.Post((GB_POST_FUNC)send_click_event, (intptr_t)THIS);
}

static void cb_show(gMenu *sender)
{
	void *_object = sender->hFree;
	GB.Raise(THIS, EVENT_Show, 0);
}

static void cb_hide(gMenu *sender)
{
	void *_object = sender->hFree;
	GB.Raise(THIS, EVENT_Hide, 0);
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
		if (!((CWINDOW*)parent)->ob.widget)
		{
			GB.Error("Invalid window");
			return;
		}
		THIS->widget=new gMenu( (gMainWindow*)((CWINDOW*)parent)->ob.widget,hidden);
		MENU->hFree=(void*)THIS;
		MENU->onFinish=cb_finish;
		MENU->onShow=cb_show;
		MENU->onHide=cb_hide;
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
		MENU->onFinish=cb_finish;
		MENU->onClick=cb_click;
		MENU->onShow=cb_show;
		MENU->onHide=cb_hide;
		GB.Ref((void*)THIS);
		return;
	}
	
	GB.Error("Type mismatch. The parent control of a Menu must be a Window or another Menu.");

END_METHOD


BEGIN_METHOD_VOID(CMENU_free)

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

	if (READ_PROPERTY)
	{
		gPicture *pic = MENU->picture();
		GB.ReturnObject(pic ? pic->getTagValue() : 0);
	}
	else
	{
		CPICTURE *pic = (CPICTURE *)VPROP(GB_OBJECT);
		MENU->setPicture(pic ? pic->picture : 0);
	}

END_PROPERTY


BEGIN_PROPERTY(CMENUITEM_enabled)

	if (READ_PROPERTY) { GB.ReturnBoolean(MENU->enabled()); return; }
	MENU->setEnabled(VPROP(GB_BOOLEAN));

END_PROPERTY


BEGIN_PROPERTY(CMENUITEM_checked)

	if (READ_PROPERTY)
		GB.ReturnBoolean(MENU->checked());
	else
		MENU->setChecked(VPROP(GB_BOOLEAN));

END_PROPERTY


BEGIN_PROPERTY(CMENU_value)

  if (MENU->toggle())
  {
    CMENUITEM_checked(_object, _param);
    return;
  }

  if (READ_PROPERTY)
  {
    GB.ReturnBoolean(0);
  }
  else if (!MENU->topLevel())
  {
    GB.Ref(THIS);
    send_click_event(THIS);
  }
  
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

	if (READ_PROPERTY) { GB.ReturnBoolean(MENU->isVisible()); return; }
	MENU->setVisible(VPROP(GB_BOOLEAN));

END_PROPERTY

BEGIN_METHOD_VOID(CMENU_show)

	MENU->setVisible(true);

END_METHOD

BEGIN_METHOD_VOID(CMENU_hide)

	MENU->setVisible(false);

END_METHOD


BEGIN_METHOD_VOID(CMENU_delete)

	MENU->destroy();

END_METHOD


BEGIN_PROPERTY(CMENU_count)

	GB.ReturnInteger(MENU->childCount());

END_PROPERTY


BEGIN_METHOD_VOID(CMENU_next)

	CMENU *Mn;
	gMenu *mn;
	int *ct;
	
	ct=(int*)GB.GetEnum();
	
	if ( ct[0]>=MENU->childCount()  ) { GB.StopEnum(); return; }
	mn=MENU->childMenu(ct[0]);
	Mn=(CMENU*)mn->hFree;
	ct[0]++;
	GB.ReturnObject(Mn);

END_PROPERTY


BEGIN_METHOD(CMENU_get, GB_INTEGER index)

	int index = VARG(index);
	
	if (index < 0 || index >= MENU->childCount())
	{
		GB.Error(GB_ERR_BOUND);
		return;
	}
	
	GB.ReturnObject(MENU->childMenu(index)->hFree);

END_METHOD

BEGIN_METHOD_VOID(CMENU_clear)

	gMenu *mn;

	int bucle,max;
	
	max=MENU->childCount();
	
	for (bucle=0;bucle<max;bucle++)
	{
		mn = MENU->childMenu(0);
		if (!mn)
			break;
		mn->destroy();
	}

END_PROPERTY


BEGIN_METHOD(CMENU_popup, GB_INTEGER x; GB_INTEGER y)

	if (!MISSING(x) && !MISSING(y))
		MENU->popup(VARG(x), VARG(y));
	else
		MENU->popup();

END_METHOD


BEGIN_PROPERTY(CMENU_tag)

	if (READ_PROPERTY) { GB.ReturnPtr(GB_T_VARIANT, &THIS->tag); return; }
    GB.StoreVariant(PROP(GB_VARIANT), (void *)&THIS->tag);
  
END_METHOD

BEGIN_PROPERTY(CMENU_toggle)

	if (READ_PROPERTY)
		GB.ReturnBoolean(MENU->toggle());
	else
		MENU->setToggle(VPROP(GB_BOOLEAN));

END_PROPERTY

BEGIN_PROPERTY(CMENU_window)

  GB.ReturnObject(GetObject(MENU->window()));  

END_PROPERTY

BEGIN_PROPERTY(CMENU_name)

	if (READ_PROPERTY)
		GB.ReturnNewZeroString(MENU->name());
	else
		MENU->setName(GB.ToZeroString(PROP(GB_STRING)));

END_PROPERTY



GB_DESC CMenuChildrenDesc[] =
{
  GB_DECLARE(".MenuChildren", sizeof(CMENU)), GB_VIRTUAL_CLASS(),

  GB_METHOD("_next", "Menu", CMENU_next, 0),
  GB_METHOD("_get", "Menu", CMENU_get, "(Index)i"),
  GB_METHOD("Clear", 0, CMENU_clear, 0),
  GB_PROPERTY_READ("Count", "i", CMENU_count),

  GB_END_DECLARE
};


GB_DESC CMenuDesc[] =
{
  GB_DECLARE("Menu", sizeof(CMENU)), 
  GB_HOOK_CHECK(CMENU_check),

  GB_STATIC_METHOD("_init", 0, CMENU_init, 0),
  GB_METHOD("_new", 0, CMENU_new, "(Parent)o[(Hidden)b]"),
  GB_METHOD("_free", 0, CMENU_free, 0),


  GB_PROPERTY("Name", "s", CMENU_name),
  GB_PROPERTY("Caption", "s", CMENU_text),
  GB_PROPERTY("Text", "s", CMENU_text),
  GB_PROPERTY("Enabled", "b", CMENUITEM_enabled),
  GB_PROPERTY("Checked", "b", CMENUITEM_checked),
  GB_PROPERTY("Tag", "v", CMENU_tag),
  GB_PROPERTY("Picture", "Picture", CMENU_picture),
  GB_PROPERTY("Shortcut", "s", CMENU_shortcut),
  GB_PROPERTY("Visible", "b", CMENU_visible),
  GB_PROPERTY("Toggle", "b", CMENU_toggle),
  GB_PROPERTY("Value", "b", CMENU_value),
  GB_PROPERTY("Action", "s", CCONTROL_action),
  GB_PROPERTY_READ("Window", "Window", CMENU_window),

  GB_PROPERTY_SELF("Children", ".MenuChildren"),

  GB_CONSTANT("_Properties", "s", "Action,Text,Picture,Enabled=True,Toggle,Checked,Visible=True,Tag,Shortcut"),
  GB_CONSTANT("_DefaultEvent", "s", "Click"),

  GB_METHOD("Popup", 0, CMENU_popup, "[(X)i(Y)i]"),
  GB_METHOD("Delete", 0, CMENU_delete, 0),
  GB_METHOD("Show", 0, CMENU_show, 0),
  GB_METHOD("Hide", 0, CMENU_hide, 0),

  GB_EVENT("Click", 0, 0, &EVENT_Click),
  GB_EVENT("Show", 0, 0, &EVENT_Show),
  GB_EVENT("Hide", 0, 0, &EVENT_Hide),

  GB_END_DECLARE
};


