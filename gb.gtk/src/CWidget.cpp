/***************************************************************************

	CWidget.cpp

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

#define __CWIDGET_CPP

#include "widgets.h"
#include "gapplication.h"

#include "CWidget.h"
#include "CWindow.h"
#include "CMenu.h"
#include "CFont.h"
#include "CMouse.h"
#include "CPicture.h"
#include "CContainer.h"
#include "CClipboard.h"
#include "CFrame.h"

extern int MAIN_scale;

DECLARE_EVENT(EVENT_Enter);
DECLARE_EVENT(EVENT_GotFocus);
DECLARE_EVENT(EVENT_LostFocus);
DECLARE_EVENT(EVENT_KeyPress);
DECLARE_EVENT(EVENT_KeyRelease);
DECLARE_EVENT(EVENT_Leave);
DECLARE_EVENT(EVENT_MouseDown);
DECLARE_EVENT(EVENT_MouseMove);
DECLARE_EVENT(EVENT_MouseDrag);
DECLARE_EVENT(EVENT_MouseUp);
DECLARE_EVENT(EVENT_MouseWheel);
DECLARE_EVENT(EVENT_DblClick);
DECLARE_EVENT(EVENT_Menu);
DECLARE_EVENT(EVENT_Drag);
DECLARE_EVENT(EVENT_DragMove);
DECLARE_EVENT(EVENT_Drop);
DECLARE_EVENT(EVENT_DragLeave);

//Plug
DECLARE_EVENT(EVENT_Plugged);
DECLARE_EVENT(EVENT_Unplugged);
DECLARE_EVENT(EVENT_PlugError);

//static void *CLASS_Image = NULL;
static GB_CLASS CLASS_UserContainer = 0;
static GB_CLASS CLASS_UserControl = 0;

/** Action *****************************************************************/

static bool has_action(void *control)
{
	if (GB.Is(control, GB.FindClass("Menu")))
	{
		gMenu *menu = ((CMENU *)(control))->widget;
		return menu ? menu->action() : false;
	}
	else
	{
		gControl *ctrl = ((CWIDGET *)(control))->widget;
		return ctrl ? ctrl->action() : false;
	}
}

static void set_action(void *control, bool v)
{
	if (GB.Is(control, GB.FindClass("Menu")))
	{
		gMenu *menu = ((CMENU *)(control))->widget;
		if (menu)
			menu->setAction(v);
	}
	else
	{
		gControl *ctrl = ((CWIDGET *)(control))->widget;
		if (ctrl)
			ctrl->setAction(v);
	}
}

#define HAS_ACTION(_control) has_action(_control)
#define SET_ACTION(_control, _flag) set_action(_control, _flag)

#include "gb.form.action.h"


/** Control ****************************************************************/

void gb_plug_raise_plugged(gControl *sender)
{
	CWIDGET *_ob=GetObject(sender);

	if (!_ob) return;
	GB.Raise((void*)_ob,EVENT_Plugged,0);
}

void gb_plug_raise_unplugged(gControl *sender)
{
	CWIDGET *_ob=GetObject(sender);

	if (!_ob) return;
	GB.Raise((void*)_ob,EVENT_Unplugged,0);
}

void gb_plug_raise_error(gControl *sender)
{
	CWIDGET *_ob=GetObject(sender);

	if (!_ob) return;
	GB.Raise((void*)_ob,EVENT_PlugError,0);
}

// static void raise_menu(void *_object)
// {
// 	GB.Raise(THIS, EVENT_Menu, 0);
// 	GB.Unref(POINTER(&_object));
// }

static int to_gambas_event(int type)
{
	switch (type)
	{
		case gEvent_MousePress: return EVENT_MouseDown;
		case gEvent_MouseRelease: return EVENT_MouseUp;
		case gEvent_MouseMove: return EVENT_MouseMove;
		case gEvent_MouseDrag: return EVENT_MouseDrag;
		case gEvent_MouseWheel: return EVENT_MouseWheel;
		case gEvent_MouseMenu: return EVENT_Menu;
		case gEvent_MouseDblClick: return EVENT_DblClick;
		case gEvent_KeyPress: return EVENT_KeyPress;
		case gEvent_KeyRelease: return EVENT_KeyRelease;
		case gEvent_FocusIn: return EVENT_GotFocus;
		case gEvent_FocusOut: return EVENT_LostFocus;
		case gEvent_Enter: return EVENT_Enter;
		case gEvent_Leave: return EVENT_Leave;
		case gEvent_DragMove: return EVENT_DragMove;
		case gEvent_Drop: return EVENT_Drop;
		default: fprintf(stderr, "warning: to_gambas_event: unhandled event: %d\n", type); return -1;
	}
}

static bool gb_can_raise(gControl *sender, int type)
{
	CWIDGET *ob = GetObject(sender);
	if (!ob)
		return false;

	type = to_gambas_event(type);
	if (type < 0)
		return false;

	return GB.CanRaise(ob, type);
}

bool gb_raise_MouseEvent(gControl *sender, int type)
{
	CWIDGET *ob = GetObject(sender);
	bool ret = false;

	if (!ob) return false; // possible, for MouseDrag for example

	switch(type)
	{
		case gEvent_MouseDrag:
			ret = GB.Raise(ob, EVENT_MouseDrag, 0);
			break;

		case gEvent_MouseMenu:

			for(;;)
			{
				if (GB.CanRaise(ob, EVENT_Menu))
				{
					int old = gMenu::popupCount();
					if (GB.Raise(ob, EVENT_Menu, 0) || old != gMenu::popupCount())
						return true;
				}

				if (ob->popup)
				{
					gMainWindow *window = sender->window();
					gMenu *menu = gMenu::findFromName(window, ob->popup);
					if (menu)
					{
						menu->popup();
						CMENU_check_popup_click();
					}
					return true;
				}

				if (sender->isTopLevel())
					break;

				sender = sender->parent();
				ob = GetObject(sender);
			}

			break;

		default:
			ret = GB.Raise(ob, to_gambas_event(type), 0);

	}

	return ret;
}

bool gb_raise_KeyEvent(gControl *sender, int type)
{
	return GB.Raise(GetObject(sender), to_gambas_event(type), 0);
}

void gb_raise_EnterLeave(gControl *sender, int type)
{
	GB.Raise(GetObject(sender), to_gambas_event(type), 0);
}

void gb_raise_FocusEvent(gControl *sender, int type)
{
	GB.Raise(GetObject(sender), to_gambas_event(type), 0);
}

bool gb_raise_Drag(gControl *sender)
{
	CWIDGET *_object = GetObject(sender);

	if (!THIS)
		return true;

	if (!GB.CanRaise(THIS, EVENT_Drag))
	{
		if (GB.CanRaise(THIS, EVENT_DragMove) || GB.CanRaise(THIS, EVENT_Drop))
			return false;
		else
			return true;
	}

	return GB.Raise(THIS, EVENT_Drag, 0);
}

void gb_raise_DragLeave(gControl *sender)
{
	CWIDGET *_object = GetObject(sender);

	GB.Raise(THIS, EVENT_DragLeave, 0);
}

bool gb_raise_DragMove(gControl *sender)
{
	CWIDGET *_object = GetObject(sender);

	if (!THIS)
		return true;

	if (!GB.CanRaise(THIS, EVENT_DragMove))
		return !GB.CanRaise(THIS, EVENT_Drag);

	return GB.Raise(THIS, EVENT_DragMove, 0);
}

bool gb_raise_Drop(gControl *sender)
{
	CWIDGET *_object = GetObject(sender);

	if (!THIS)
		return false;

	if (!GB.CanRaise(THIS, EVENT_Drop))
		return false;

	GB.Raise(THIS, EVENT_Drop, 0);
	return true;
}

void DeleteControl(gControl *control)
{
	CWIDGET *widget = (CWIDGET*)control->hFree;

	if (!widget)
		return;

	GB.Detach(widget);

	GB.StoreVariant(NULL, POINTER(&widget->tag));
	GB.StoreObject(NULL, POINTER(&widget->cursor));

	CACTION_register(widget, widget->action, NULL);
	GB.FreeString(&widget->action);

	if (control->isTopLevel())
		CWINDOW_check_main_window((CWINDOW*)widget);

	GB.Unref(POINTER(&widget->font));
	GB.FreeString(&widget->popup);

	widget->font = NULL;
	widget->widget = NULL;
	GB.Unref(POINTER(&widget));

	control->hFree = NULL;
}


void InitControl(gControl *control, CWIDGET *widget)
{
	static int n = 0;
	char *name;

	GB.Ref((void*)widget);

	widget->widget = control;
	control->hFree=(void*)widget;
	//fprintf(stderr, "InitControl: %p %p\n", control, widget);

	name = GB.GetLastEventName();
	if (!name)
	{
		char buffer[16];
		n++;
		sprintf(buffer, "#%d", n);
		control->setName(buffer);
	}
	else
		control->setName(name);

	control->onFinish = DeleteControl;
	control->onMouseEvent = gb_raise_MouseEvent;
	control->onKeyEvent = gb_raise_KeyEvent;
	control->onFocusEvent = gb_raise_FocusEvent;
	control->onDrag = gb_raise_Drag;
	control->onDragLeave = gb_raise_DragLeave;
	control->onDragMove = gb_raise_DragMove;
	control->onDrop = gb_raise_Drop;
	control->onEnterLeave = gb_raise_EnterLeave;
	control->canRaise = gb_can_raise;

	if (control->isContainer())
	{
		((gContainer *)control)->onBeforeArrange = CCONTAINER_cb_before_arrange;
		((gContainer *)control)->onArrange = CCONTAINER_cb_arrange;
	}

	if (control->parent())
		CCONTAINER_raise_insert((CCONTAINER *)control->parent()->hFree, widget);
}

CWIDGET *GetContainer(CWIDGET *control)
{
	if (!control) return NULL;

	if (!CLASS_UserContainer) CLASS_UserContainer=GB.FindClass("UserContainer");
	if (!CLASS_UserControl) CLASS_UserControl=GB.FindClass("UserControl");

	if ( GB.Is (control,CLASS_UserContainer) || GB.Is (control,CLASS_UserControl) )
		return (CWIDGET*)((CUSERCONTROL*)control)->container;

	return control;
}

int CWIDGET_get_handle(void *_object)
{
	return CONTROL->handle();
}

int CWIDGET_check(void *_object)
{
	return (!CONTROL || CONTROL->isDestroyed());
}

/*************************************************************************************

Embedder

**************************************************************************************/

BEGIN_METHOD(CPLUGIN_new, GB_OBJECT parent)

	InitControl(new gPlugin(CONTAINER(VARG(parent))), (CWIDGET*)_object);
	PLUGIN->onPlug = gb_plug_raise_plugged;
	PLUGIN->onUnplug = gb_plug_raise_unplugged;
	PLUGIN->onError = gb_plug_raise_error;

END_METHOD

BEGIN_PROPERTY(CPLUGIN_client)

	GB.ReturnInteger(PLUGIN->client());

END_PROPERTY

BEGIN_METHOD(CPLUGIN_embed, GB_INTEGER id)

	PLUGIN->plug(VARG(id));

END_METHOD

BEGIN_METHOD_VOID(CPLUGIN_discard)

	PLUGIN->discard();

END_METHOD


/**************************************************************************************

Control

**************************************************************************************/

BEGIN_PROPERTY(CWIDGET_x)

	if (READ_PROPERTY) {  GB.ReturnInteger(CONTROL->left()); return; }
	CONTROL->setLeft(VPROP(GB_INTEGER));

END_PROPERTY


BEGIN_PROPERTY(CWIDGET_screen_x)

	GB.ReturnInteger(CONTROL->screenX());

END_PROPERTY


BEGIN_PROPERTY(CWIDGET_y)

	if (READ_PROPERTY) {  GB.ReturnInteger(CONTROL->top()); return; }
	CONTROL->setTop(VPROP(GB_INTEGER));

END_PROPERTY


BEGIN_PROPERTY(CWIDGET_screen_y)

	GB.ReturnInteger(CONTROL->screenY());

END_PROPERTY


BEGIN_PROPERTY(CWIDGET_w)

	if (READ_PROPERTY) {  GB.ReturnInteger(CONTROL->width()); return; }
	CONTROL->setWidth(VPROP(GB_INTEGER));

END_PROPERTY


BEGIN_PROPERTY(CWIDGET_h)

	if (READ_PROPERTY) {  GB.ReturnInteger(CONTROL->height()); return; }
	CONTROL->setHeight(VPROP(GB_INTEGER));

END_PROPERTY


BEGIN_PROPERTY(CCONTROL_font)

	if (!THIS->font)
	{
		THIS->font = CFONT_create(new gFont(), 0, THIS);
		GB.Ref(THIS->font);
	}

	if (READ_PROPERTY)
	{
		CONTROL->actualFontTo(((CFONT *)THIS->font)->font);
		GB.ReturnObject(THIS->font);
	}
	else
	{
		CFONT *font = (CFONT *)VPROP(GB_OBJECT);
		if (font)
			CONTROL->setFont(font->font->copy());
		else
			CONTROL->setFont(NULL);
	}

END_PROPERTY


BEGIN_PROPERTY(CWIDGET_design)

	if (READ_PROPERTY) { GB.ReturnBoolean(CONTROL->design()); return; }
	CONTROL->setDesign(VPROP(GB_BOOLEAN));

END_PROPERTY


BEGIN_PROPERTY(CWIDGET_visible)

	if (READ_PROPERTY)
		GB.ReturnBoolean(CONTROL->isVisible());
	else
		CONTROL->setVisible(VPROP(GB_BOOLEAN));

END_PROPERTY


BEGIN_PROPERTY(CWIDGET_enabled)

	if (READ_PROPERTY)
		GB.ReturnBoolean(CONTROL->isEnabled());
	else
		CONTROL->setEnabled(VPROP(GB_BOOLEAN));

END_PROPERTY


BEGIN_PROPERTY(Control_HasFocus)

	GB.ReturnBoolean(CONTROL->hasFocus());

END_PROPERTY

BEGIN_PROPERTY(Control_Hovered)

	GB.ReturnBoolean(CONTROL->hovered());

END_PROPERTY

BEGIN_PROPERTY(CWIDGET_expand)

	if (READ_PROPERTY) { GB.ReturnBoolean(CONTROL->expand()); return; }
	CONTROL->setExpand(VPROP(GB_BOOLEAN));

END_PROPERTY


BEGIN_PROPERTY(CWIDGET_ignore)

	if (READ_PROPERTY) { GB.ReturnBoolean(CONTROL->ignore()); return; }
	CONTROL->setIgnore(VPROP(GB_BOOLEAN));

END_PROPERTY


BEGIN_METHOD(CWIDGET_move, GB_INTEGER x; GB_INTEGER y; GB_INTEGER w; GB_INTEGER h)

	if (MISSING(w) && MISSING(h))
		CONTROL->move(VARG(x), VARG(y));
	else
		CONTROL->moveResize(VARG(x), VARG(y), VARGOPT(w, CONTROL->width()), VARGOPT(h, CONTROL->height()));

END_METHOD


BEGIN_METHOD(CWIDGET_resize, GB_INTEGER w; GB_INTEGER h)

	CONTROL->resize(VARG(w),VARG(h));

END_METHOD

BEGIN_METHOD(CWIDGET_moveScaled, GB_FLOAT x; GB_FLOAT y; GB_FLOAT w; GB_FLOAT h)

	int x, y, w, h;

	x = (int)(VARG(x) * MAIN_scale + 0.5);
	y = (int)(VARG(y) * MAIN_scale + 0.5);
	w = (MISSING(w) ? -1 : (int)(VARG(w) * MAIN_scale + 0.5));
	h = (MISSING(h) ? -1 : (int)(VARG(h) * MAIN_scale + 0.5));

	if (w == 0) w = 1;
	if (h == 0) h = 1;

	CONTROL->move(x, y);
	if (w > 0 && h > 0)
		CONTROL->resize(w, h);

END_METHOD


BEGIN_METHOD(CWIDGET_resizeScaled, GB_FLOAT w; GB_FLOAT h)

	int w, h;

	w = (int)(VARG(w) * MAIN_scale + 0.5);
	h = (int)(VARG(h) * MAIN_scale + 0.5);

	if (w == 0) w = 1;
	if (h == 0) h = 1;

	CONTROL->resize(w, h);

END_METHOD


BEGIN_METHOD_VOID(CWIDGET_delete)

	if (CONTROL)
		CONTROL->destroy();

END_METHOD


BEGIN_METHOD_VOID(CWIDGET_show)

	CONTROL->show();

END_METHOD


BEGIN_METHOD_VOID(CWIDGET_hide)

	CONTROL->hide();

END_METHOD

BEGIN_METHOD(CWIDGET_reparent, GB_OBJECT parent; GB_INTEGER x; GB_INTEGER y)

	CCONTAINER *parent = (CCONTAINER*)VARG(parent);
	int x, y;

	if (parent || !GB.Is(THIS, CLASS_Window))
	{
		if (GB.CheckObject(parent))
			return;
	}

	x = CONTROL->x();
	y = CONTROL->y();

	if (!MISSING(x) && !MISSING(y))
	{
		x = VARG(x);
		y = VARG(y);
	}

	CONTROL->reparent(parent ? (gContainer*)parent->ob.widget : NULL, x, y);

END_METHOD


BEGIN_METHOD_VOID(CWIDGET_raise)

	CONTROL->raise();

END_METHOD


BEGIN_METHOD_VOID(CWIDGET_lower)

	CONTROL->lower();

END_METHOD


BEGIN_PROPERTY(CWIDGET_next)

	if (READ_PROPERTY)
		GB.ReturnObject(GetObject(CONTROL->next()));
	else
	{
		CWIDGET *next = (CWIDGET *)VPROP(GB_OBJECT);
		CONTROL->setNext(next ? next->widget : NULL);
	}

END_PROPERTY


BEGIN_PROPERTY(CWIDGET_previous)

	if (READ_PROPERTY)
		GB.ReturnObject(GetObject(CONTROL->previous()));
	else
	{
		CWIDGET *previous = (CWIDGET *)VPROP(GB_OBJECT);
		CONTROL->setPrevious(previous ? previous->widget : NULL);
	}

END_PROPERTY


BEGIN_METHOD_VOID(CWIDGET_refresh) //, GB_INTEGER x; GB_INTEGER y; GB_INTEGER w; GB_INTEGER h)

	CONTROL->refresh();

END_METHOD


BEGIN_METHOD_VOID(CWIDGET_set_focus)

	CONTROL->setFocus();

END_METHOD


BEGIN_PROPERTY(CWIDGET_tag)

	if (READ_PROPERTY)
		GB.ReturnVariant(&THIS->tag);
	else
		GB.StoreVariant(PROP(GB_VARIANT), (void *)&THIS->tag);

END_METHOD


BEGIN_PROPERTY(CWIDGET_mouse)

	if (READ_PROPERTY)
		GB.ReturnInteger(CONTROL->mouse());
	else
		CONTROL->setMouse(VPROP(GB_INTEGER));

END_METHOD


BEGIN_PROPERTY(Control_Cursor)

	if (READ_PROPERTY)
		GB.ReturnObject(THIS->cursor);
	else
	{
		GB.StoreObject(PROP(GB_OBJECT), &THIS->cursor);
		CCURSOR *c = (CCURSOR *)THIS->cursor;
		CONTROL->setCursor(c ? c->cur : NULL);
	}

END_PROPERTY


BEGIN_PROPERTY(CWIDGET_background)

	if (CONTROL->proxy())
	{
		if (READ_PROPERTY)
			GB.GetProperty(GetObject(CONTROL->proxy()), "Background");
		else
		{
			GB_VALUE value;
			value.type = GB_T_INTEGER;
			value._integer.value = VPROP(GB_INTEGER);
			GB.SetProperty(GetObject(CONTROL->proxy()), "Background", &value);
		}

		return;
	}

	if (READ_PROPERTY)
		GB.ReturnInteger(CONTROL->background());
	else
		CONTROL->setBackground(VPROP(GB_INTEGER));

END_PROPERTY


BEGIN_PROPERTY(CWIDGET_foreground)

	if (CONTROL->proxy())
	{
		if (READ_PROPERTY)
			GB.GetProperty(GetObject(CONTROL->proxy()), "Foreground");
		else
		{
			GB_VALUE value;
			value.type = GB_T_INTEGER;
			value._integer.value = VPROP(GB_INTEGER);
			GB.SetProperty(GetObject(CONTROL->proxy()), "Foreground", &value);
		}

		return;
	}

	if (READ_PROPERTY)
		GB.ReturnInteger(CONTROL->foreground());
	else
		CONTROL->setForeground(VPROP(GB_INTEGER));

END_PROPERTY


BEGIN_PROPERTY(CWIDGET_parent)

	GB.ReturnObject(GetObject(CONTROL->parent()));

END_PROPERTY


BEGIN_PROPERTY(CWIDGET_window)

	GB.ReturnObject(GetObject(CONTROL->window()));

END_PROPERTY


BEGIN_PROPERTY(CWIDGET_id)

	GB.ReturnInteger(CONTROL->handle());

END_PROPERTY


BEGIN_PROPERTY(CWIDGET_tooltip)

	if (READ_PROPERTY)
	{
		GB.ReturnNewZeroString(CONTROL->toolTip());
		return;
	}

	CONTROL->setToolTip(GB.ToZeroString(PROP(GB_STRING)));

END_PROPERTY


/*BEGIN_METHOD_VOID(CWIDGET_screenshot)

	CPICTURE *img;

	GB.New(POINTER(&img), GB.FindClass("Picture"), 0, 0);
	if (img->picture) delete img->picture;
	img->picture=CONTROL->screenshot();
	GB.ReturnObject((void*)img);

END_METHOD*/

BEGIN_METHOD_VOID(CCONTROL_grab)

	CONTROL->grab();

END_METHOD


BEGIN_METHOD(CWIDGET_drag, GB_VARIANT data; GB_STRING format)

	GB.ReturnObject(CDRAG_drag(THIS, &VARG(data), MISSING(format) ? NULL : GB.ToZeroString(ARG(format))));

END_METHOD


BEGIN_PROPERTY(CWIDGET_drop)

	if (READ_PROPERTY)
		GB.ReturnBoolean(CONTROL->acceptDrops());
	else
		CONTROL->setAcceptDrops(VPROP(GB_BOOLEAN));

END_PROPERTY

BEGIN_PROPERTY(CCONTROL_tracking)

	if (READ_PROPERTY)
		GB.ReturnBoolean(CONTROL->isTracking());
	else
		CONTROL->setTracking(VPROP(GB_BOOLEAN));

END_PROPERTY


BEGIN_PROPERTY(CCONTROL_name)

	if (READ_PROPERTY)
		GB.ReturnNewZeroString(CONTROL->name());
	else
		CONTROL->setName(GB.ToZeroString(PROP(GB_STRING)));

END_PROPERTY


BEGIN_PROPERTY(CCONTROL_action)

	if (READ_PROPERTY)
		GB.ReturnString(THIS->action);
	else
	{
		CACTION_register(THIS, THIS->action, GB.ToZeroString(PROP(GB_STRING)));
		GB.StoreString(PROP(GB_STRING), &THIS->action);
	}

END_PROPERTY


BEGIN_PROPERTY(Control_Proxy)

	if (READ_PROPERTY)
		GB.ReturnObject(GetObject(CONTROL->proxy()));
	else
	{
		CWIDGET *proxy = (CWIDGET *)VPROP(GB_OBJECT);
		if (CONTROL->setProxy(proxy ? proxy->widget : NULL))
			GB.Error("Circular proxy chain");
	}

END_PROPERTY


BEGIN_PROPERTY(Control_PopupMenu)

	if (READ_PROPERTY)
		GB.ReturnString(THIS->popup);
	else
		GB.StoreString(PROP(GB_STRING), &(THIS->popup));

END_PROPERTY


BEGIN_PROPERTY(Control_NoTabFocus)

	if (READ_PROPERTY)
		GB.ReturnBoolean(CONTROL->isNoTabFocus());
	else
		CONTROL->setNoTabFocus(VPROP(GB_BOOLEAN));

END_PROPERTY


GB_DESC CWidgetDesc[] =
{
	GB_DECLARE("Control", sizeof(CWIDGET)),

	GB_NOT_CREATABLE(),
	GB_HOOK_CHECK(CWIDGET_check),

	//GB_METHOD("_free", 0, CWIDGET_delete, 0),

	GB_METHOD("Move", NULL, CWIDGET_move, "(X)i(Y)i[(Width)i(Height)i]"),
	GB_METHOD("Resize", NULL, CWIDGET_resize, "(Width)i(Height)i"),
	GB_METHOD("MoveScaled", NULL, CWIDGET_moveScaled, "(X)f(Y)f[(Width)f(Height)f]"),
	GB_METHOD("ResizeScaled", NULL, CWIDGET_resizeScaled, "(Width)f(Height)f"),
	GB_METHOD("Delete", NULL, CWIDGET_delete, NULL),
	GB_METHOD("Show", NULL, CWIDGET_show, NULL),
	GB_METHOD("Hide", NULL, CWIDGET_hide, NULL),
	GB_METHOD("Reparent", NULL, CWIDGET_reparent, "(Parent)Container;[(X)i(Y)i]"),

	GB_METHOD("Raise", NULL, CWIDGET_raise, NULL),
	GB_METHOD("Lower", NULL, CWIDGET_lower, NULL),

	GB_PROPERTY("Next", "Control", CWIDGET_next),
	GB_PROPERTY("Previous", "Control", CWIDGET_previous),

	GB_METHOD("SetFocus", NULL, CWIDGET_set_focus, NULL),
	GB_METHOD("Refresh", NULL, CWIDGET_refresh, NULL),
	//GB_METHOD("Screenshot", "Picture", CWIDGET_screenshot, 0),
	GB_METHOD("Grab", NULL, CCONTROL_grab, NULL),
	GB_METHOD("Drag", "Control", CWIDGET_drag, "(Data)v[(Format)s]"),

	GB_PROPERTY("X", "i", CWIDGET_x),
	GB_PROPERTY("Y", "i", CWIDGET_y),
	GB_PROPERTY_READ("ScreenX", "i", CWIDGET_screen_x),
	GB_PROPERTY_READ("ScreenY", "i", CWIDGET_screen_y),
	GB_PROPERTY("W", "i", CWIDGET_w),
	GB_PROPERTY("H", "i", CWIDGET_h),
	GB_PROPERTY("Left", "i", CWIDGET_x),
	GB_PROPERTY("Top", "i", CWIDGET_y),
	GB_PROPERTY("Width", "i", CWIDGET_w),
	GB_PROPERTY("Height", "i", CWIDGET_h),

	GB_PROPERTY("Visible", "b", CWIDGET_visible),
	GB_PROPERTY("Enabled", "b", CWIDGET_enabled),
	GB_PROPERTY_READ("HasFocus", "b", Control_HasFocus),
	GB_PROPERTY_READ("Hovered", "b", Control_Hovered),

	GB_PROPERTY("Expand", "b", CWIDGET_expand),
	GB_PROPERTY("Ignore", "b", CWIDGET_ignore),

	GB_PROPERTY("Font", "Font", CCONTROL_font),
	GB_PROPERTY("Background", "i", CWIDGET_background),
	GB_PROPERTY("Foreground", "i", CWIDGET_foreground),

	GB_PROPERTY("Design", "b", CWIDGET_design),
	GB_PROPERTY("Name", "s", CCONTROL_name),
	GB_PROPERTY("Tag", "v", CWIDGET_tag),
	GB_PROPERTY("Tracking", "b", CCONTROL_tracking),
	GB_PROPERTY("Mouse", "i", CWIDGET_mouse),
	GB_PROPERTY("Cursor", "Cursor", Control_Cursor),
	GB_PROPERTY("ToolTip", "s", CWIDGET_tooltip),
	GB_PROPERTY("Drop", "b", CWIDGET_drop),
	GB_PROPERTY("Action", "s", CCONTROL_action),
	GB_PROPERTY("PopupMenu", "s", Control_PopupMenu),
	GB_PROPERTY("Proxy", "Control", Control_Proxy),
	GB_PROPERTY("NoTabFocus", "b", Control_NoTabFocus),

	GB_PROPERTY_READ("Parent", "Container", CWIDGET_parent),
	GB_PROPERTY_READ("Window", "Window", CWIDGET_window),
	GB_PROPERTY_READ("Id", "i", CWIDGET_id),
	GB_PROPERTY_READ("Handle", "i", CWIDGET_id),

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
	GB_EVENT("DragLeave", NULL, NULL, &EVENT_DragLeave),

	CONTROL_DESCRIPTION,

	GB_END_DECLARE
};

GB_DESC CPluginDesc[] =
{
	GB_DECLARE("Embedder", sizeof(CPLUGIN)), GB_INHERITS("Control"),

	GB_METHOD("_new", 0, CPLUGIN_new, "(Parent)Container;"),
	GB_METHOD("Embed", 0, CPLUGIN_embed, "(Client)i"),
	GB_METHOD("Discard", 0, CPLUGIN_discard, 0),

	GB_PROPERTY_READ("Client", "i", CPLUGIN_client),
	//GB_PROPERTY("Border", "i<Border>", CPLUGIN_border),

	GB_EVENT("Embed", NULL, NULL, &EVENT_Plugged),
	GB_EVENT("Close", NULL, NULL, &EVENT_Unplugged),
	GB_EVENT("Error", NULL, NULL, &EVENT_PlugError), /* TODO */

	EMBEDDER_DESCRIPTION,

	GB_END_DECLARE
};


