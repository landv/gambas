/***************************************************************************

	CMenu.cpp

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

#define __CMENU_CPP

#include "main.h"
#include "gambas.h"
#include "CWidget.h"
#include "CWindow.h"
#include "CPicture.h"
#include "CMenu.h"


DECLARE_EVENT(EVENT_Click);
DECLARE_EVENT(EVENT_Show);
DECLARE_EVENT(EVENT_Hide);

static CMENU *_popup_menu_clicked = NULL;
static GB_FUNCTION _init_shortcut_func;


static void send_click_event(void *_object)
{
	GB.Raise(THIS, EVENT_Click, 0);
	CACTION_raise(THIS);
	GB.Unref(POINTER(&_object));
}

static int CMENU_check(void *_object)
{
	return (MENU == NULL);
}

#ifndef GTK3
static void delete_later(gMenu *menu)
{
	delete menu;
}
#endif

static void delete_menu(gMenu *menu)
{
#ifdef GTK3
	delete menu;
#else
	void *_object = menu->hFree;

	if (!MENU)
		return;

	menu->willBeDeletedLater();
	THIS->widget = NULL;

	GB.Post((GB_CALLBACK)delete_later, (intptr_t)menu);
#endif
}

static void cb_finish(gMenu *sender)
{
	CMENU *_object = (CMENU*)sender->hFree;
	if (_object)
	{
		CACTION_register(THIS, THIS->action, NULL);
		GB.FreeString(&THIS->action);
		THIS->widget = NULL;
		GB.StoreVariant(NULL, POINTER(&THIS->tag));
		GB.Unref(POINTER(&_object));
	}
}

static void cb_click(gMenu *sender)
{
	void *_object = sender->hFree;

	GB.Ref(THIS);

	if (gMenu::insidePopup())
	{
		GB.Unref(POINTER(&_popup_menu_clicked));
		_popup_menu_clicked = THIS;
	}
	else
		send_click_event(THIS);
}

static void cb_show(gMenu *sender)
{
	static bool init = FALSE;

	void *_object = sender->hFree;

	GB.Ref(THIS);

	GB.Raise(THIS, EVENT_Show, 0);

	if (!THIS->init_shortcut)
	{
		if (!init)
		{
			GB.GetFunction(&_init_shortcut_func, (void *)GB.FindClass("_Gui"), "_DefineShortcut", NULL, NULL);
			init = TRUE;
		}

		THIS->init_shortcut = TRUE;
		GB.Push(1, GB_T_OBJECT, THIS);
		GB.Call(&_init_shortcut_func, 1, FALSE);
	}

	GB.Unref(POINTER(&_object));
}

static void cb_hide(gMenu *sender)
{
	void *_object = sender->hFree;
	GB.Raise(THIS, EVENT_Hide, 0);
}

void CMENU_check_popup_click(void)
{
	if (_popup_menu_clicked)
	{
		CMENU *menu = _popup_menu_clicked;
		_popup_menu_clicked = NULL;
		send_click_event(menu);
	}
}

//-------------------------------------------------------------------------

BEGIN_METHOD(Menu_new, GB_OBJECT parent; GB_BOOLEAN hidden)

	void *parent = VARG(parent);
	bool hidden;
	char *name;

	hidden = VARGOPT(hidden, false);

	if (GB.Is(parent,CLASS_Window))
	{
		if (!((CWINDOW*)parent)->ob.widget)
		{
			GB.Error("Invalid window");
			return;
		}

		THIS->widget = new gMenu((gMainWindow*)((CWINDOW*)parent)->ob.widget, hidden);
		goto __OK;
	}

	if (GB.Is(parent,CLASS_Menu))
	{
		if ( !((CMENU*)parent)->widget )
		{
			GB.Error("Invalid menu");
			return;
		}

		THIS->widget = new gMenu((gMenu*)((CMENU*)parent)->widget, hidden);
		MENU->onClick = cb_click;
		goto __OK;
	}

	GB.Error("Type mismatch. The parent control of a Menu must be a Window or another Menu.");
	return;

__OK:

	MENU->hFree = (void*)THIS;
	MENU->onFinish = cb_finish;
	MENU->onShow = cb_show;
	MENU->onHide = cb_hide;

	name = GB.GetLastEventName();
	if (!name)
		name = GB.GetClassName((void *)THIS);

	MENU->setName(name);

	GB.Ref((void*)THIS);

END_METHOD


BEGIN_METHOD_VOID(Menu_free)

	GB.FreeString(&THIS->save_text);
	if (MENU) MENU->destroy();

END_METHOD


BEGIN_PROPERTY(Menu_Text)

	if (READ_PROPERTY)
	{
		if (THIS->save_text)
			GB.ReturnString(THIS->save_text);
		else
			GB.ReturnNewZeroString(MENU->text());
		return;
	}
	else
	{
		MENU->setText(GB.ToZeroString(PROP(GB_STRING)));

		if (!MENU->topLevel())
			((CMENU *)GetObject((gMenu *)MENU->parent()))->init_shortcut = FALSE;

		GB.FreeString(&THIS->save_text);
	}

END_PROPERTY


BEGIN_PROPERTY(Menu_Picture)

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


BEGIN_PROPERTY(Menu_Enabled)

	if (READ_PROPERTY) { GB.ReturnBoolean(MENU->enabled()); return; }
	MENU->setEnabled(VPROP(GB_BOOLEAN));

END_PROPERTY


BEGIN_PROPERTY(Menu_Checked)

	if (READ_PROPERTY)
		GB.ReturnBoolean(MENU->checked());
	else
		MENU->setChecked(VPROP(GB_BOOLEAN));

END_PROPERTY


BEGIN_PROPERTY(Menu_Value)

	if (MENU->toggle() || MENU->radio())
	{
		Menu_Checked(_object, _param);
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


BEGIN_PROPERTY(Menu_Shortcut)

	if (READ_PROPERTY)
	{
		GB.ReturnNewZeroString(MENU->shortcut());
		return;
	}

	MENU->setShortcut(GB.ToZeroString(PROP(GB_STRING)));

END_PROPERTY


BEGIN_PROPERTY(Menu_Visible)

	if (READ_PROPERTY) { GB.ReturnBoolean(MENU->isVisible()); return; }
	MENU->setVisible(VPROP(GB_BOOLEAN));

END_PROPERTY

BEGIN_METHOD_VOID(Menu_Show)

	MENU->setVisible(true);

END_METHOD

BEGIN_METHOD_VOID(Menu_Hide)

	MENU->setVisible(false);

END_METHOD


BEGIN_METHOD_VOID(Menu_Delete)

	delete_menu(MENU);

END_METHOD


BEGIN_PROPERTY(MenuChildren_Count)

	GB.ReturnInteger(MENU->childCount());

END_PROPERTY


BEGIN_METHOD_VOID(MenuChildren_next)

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


BEGIN_METHOD(MenuChildren_get, GB_INTEGER index)

	int index = VARG(index);

	if (index < 0 || index >= MENU->childCount())
	{
		GB.Error(GB_ERR_BOUND);
		return;
	}

	GB.ReturnObject(MENU->childMenu(index)->hFree);

END_METHOD

BEGIN_METHOD_VOID(MenuChildren_Clear)

	while (MENU->childCount())
		delete_menu(MENU->childMenu(0));

	THIS->init_shortcut = FALSE;

END_PROPERTY


BEGIN_METHOD(Menu_Popup, GB_INTEGER x; GB_INTEGER y)

	if (!MISSING(x) && !MISSING(y))
		MENU->popup(VARG(x), VARG(y));
	else
		MENU->popup();

	CMENU_check_popup_click();

END_METHOD


BEGIN_PROPERTY(Menu_Tag)

	if (READ_PROPERTY)
		GB.ReturnVariant(&THIS->tag);
	else
		GB.StoreVariant(PROP(GB_VARIANT), (void *)&THIS->tag);

END_METHOD

BEGIN_PROPERTY(Menu_Toggle)

	if (READ_PROPERTY)
		GB.ReturnBoolean(MENU->toggle());
	else
		MENU->setToggle(VPROP(GB_BOOLEAN));

END_PROPERTY

BEGIN_PROPERTY(Menu_Radio)

	if (READ_PROPERTY)
		GB.ReturnBoolean(MENU->radio());
	else
		MENU->setRadio(VPROP(GB_BOOLEAN));

END_PROPERTY

BEGIN_PROPERTY(Menu_Window)

	GB.ReturnObject(GetObject(MENU->window()));

END_PROPERTY

BEGIN_PROPERTY(Menu_Name)

	if (READ_PROPERTY)
		GB.ReturnNewZeroString(MENU->name());
	else
		MENU->setName(GB.ToZeroString(PROP(GB_STRING)));

END_PROPERTY

BEGIN_PROPERTY(Menu_Action)

	if (READ_PROPERTY)
		GB.ReturnString(THIS->action);
	else
	{
		CACTION_register(THIS, THIS->action, GB.ToZeroString(PROP(GB_STRING)));
		GB.StoreString(PROP(GB_STRING), &THIS->action);
	}

END_PROPERTY

BEGIN_PROPERTY(Menu_SaveText)

	if (READ_PROPERTY)
		GB.ReturnString(THIS->save_text);
	else
		GB.StoreString(PROP(GB_STRING), &THIS->save_text);

END_PROPERTY


GB_DESC CMenuChildrenDesc[] =
{
	GB_DECLARE(".Menu.Children", sizeof(CMENU)), GB_VIRTUAL_CLASS(),

	GB_METHOD("_next", "Menu", MenuChildren_next, 0),
	GB_METHOD("_get", "Menu", MenuChildren_get, "(Index)i"),
	GB_METHOD("Clear", 0, MenuChildren_Clear, 0),
	GB_PROPERTY_READ("Count", "i", MenuChildren_Count),

	GB_END_DECLARE
};


GB_DESC CMenuDesc[] =
{
	GB_DECLARE("Menu", sizeof(CMENU)),
	GB_HOOK_CHECK(CMENU_check),

	//GB_STATIC_METHOD("_init", 0, CMENU_init, 0),
	GB_METHOD("_new", 0, Menu_new, "(Parent)o[(Hidden)b]"),
	GB_METHOD("_free", 0, Menu_free, 0),


	GB_PROPERTY("Name", "s", Menu_Name),
	GB_PROPERTY("Caption", "s", Menu_Text),
	GB_PROPERTY("Text", "s", Menu_Text),
	GB_PROPERTY("_Text", "s", Menu_SaveText),
	GB_PROPERTY("Enabled", "b", Menu_Enabled),
	GB_PROPERTY("Checked", "b", Menu_Checked),
	GB_PROPERTY("Tag", "v", Menu_Tag),
	GB_PROPERTY("Picture", "Picture", Menu_Picture),
	GB_PROPERTY("Shortcut", "s", Menu_Shortcut),
	GB_PROPERTY("Visible", "b", Menu_Visible),
	GB_PROPERTY("Toggle", "b", Menu_Toggle),
	GB_PROPERTY("Radio", "b", Menu_Radio),
	GB_PROPERTY("Value", "b", Menu_Value),
	//GB_PROPERTY("TearOff", "b", CMENU_tear_off),
	GB_PROPERTY("Action", "s", Menu_Action),
	GB_PROPERTY_READ("Window", "Window", Menu_Window),

	GB_PROPERTY_SELF("Children", ".Menu.Children"),

	MENU_DESCRIPTION,

	GB_METHOD("Popup", 0, Menu_Popup, "[(X)i(Y)i]"),
	GB_METHOD("Delete", 0, Menu_Delete, 0),
	GB_METHOD("Show", 0, Menu_Show, 0),
	GB_METHOD("Hide", 0, Menu_Hide, 0),

	GB_EVENT("Click", 0, 0, &EVENT_Click),
	GB_EVENT("Show", 0, 0, &EVENT_Show),
	GB_EVENT("Hide", 0, 0, &EVENT_Hide),

	GB_END_DECLARE
};


