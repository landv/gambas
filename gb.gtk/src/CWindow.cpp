/***************************************************************************

  CWindow.cpp

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

#define __CWINDOW_CPP

#include "x11.h"
#include "main.h"
#include "CWindow.h"
#include "CWidget.h"
#include "CContainer.h"
#include "CPicture.h"
#include "CMenu.h"
#include "CDraw.h"
#include "gapplication.h"

int CWINDOW_Embedder = 0;
bool CWINDOW_Embedded = false;

CWINDOW *CWINDOW_Active = NULL;
CWINDOW *CWINDOW_Main = NULL;
static int MODAL_windows = 0;

DECLARE_EVENT(EVENT_Open);
DECLARE_EVENT(EVENT_Show);
DECLARE_EVENT(EVENT_Hide);
DECLARE_EVENT(EVENT_Close);
DECLARE_EVENT(EVENT_Activate);
DECLARE_EVENT(EVENT_Deactivate);
DECLARE_EVENT(EVENT_Move);
DECLARE_EVENT(EVENT_Resize);
DECLARE_EVENT(EVENT_Title);
DECLARE_EVENT(EVENT_Icon);
//DECLARE_EVENT(EVENT_State);

void CWINDOW_check_main_window(CWINDOW *win)
{
	if (CWINDOW_Main == win) 
	{
		//fprintf(stderr, "CWINDOW_Main = NULL\n");
		CWINDOW_Main = NULL;
	}
}


static void gb_raise_window_Open(gMainWindow *sender)
{
	CWIDGET *_ob=GetObject(sender);

	if (!_ob) return;
	GB.Raise((void*)_ob,EVENT_Open,0);
}

/*static void gb_post_window_Open(gMainWindow *sender)
{
	GB.Post((GB_POST_FUNC)gb_raise_window_Open, (long)sender);
}*/

static void gb_raise_window_Show(gMainWindow *sender)
{
	CWIDGET *ob = GetObject(sender);

	if (!ob) return;
	GB.Ref(ob);
	GB.Raise((void*)ob,EVENT_Show,0);
	if (!sender->spontaneous())
		CACTION_raise(ob);
	GB.Unref(POINTER(&ob));
}

/*static void gb_post_window_Show(gMainWindow *sender)
{
	GB.Post( (void (*)())gb_raise_window_Show,(long)sender);
}*/

static void gb_raise_window_Hide(gMainWindow *sender)
{
	CWIDGET *ob=GetObject(sender);

	if (!ob) return;
	GB.Ref(ob);
	GB.Raise((void*)ob,EVENT_Hide,0);
	if (!sender->spontaneous())
		CACTION_raise(ob);
	GB.Unref(POINTER(&ob));
}

static void gb_raise_window_Move(gMainWindow *sender)
{
	CWIDGET *_ob=GetObject(sender);

	if (!_ob) return;
	GB.Raise((void*)_ob,EVENT_Move,0);
}

static void gb_raise_window_Resize(gMainWindow *sender)
{
	CWIDGET *_ob=GetObject(sender);

	if (!_ob) return;
	GB.Raise((void*)_ob,EVENT_Resize,0);
}

static bool close_window(CWINDOW *_object, int ret = 0)
{
	THIS->ret = ret;

	return WINDOW->close();
}

void CWINDOW_delete_all()
{
	int i;
	gMainWindow *win;
	CWINDOW *window;
	
	for(i = 0;; i++)
	{
		win = gMainWindow::get(i);
		if (!win)
			break;
		
		window = (CWINDOW *)GetObject(win);
		if (window == CWINDOW_Main)
			continue;
		
		win->destroy();
	}
}

bool CWINDOW_must_quit()
{
	int i;
	gMainWindow *win;
	
	for(i = 0; i < gMainWindow::count(); i++)
	{
		win = gMainWindow::get(i);
		if (win->isTopLevel() && win->isOpened())
			return false;
	}
	
	return true;
}

static bool gb_raise_window_Close(gMainWindow *sender)
{
	CWINDOW *_object = (CWINDOW*)GetObject(sender);

	if (!THIS)
		return false;
	
	if (GB.Raise(THIS, EVENT_Close, 0)) 
		return true;
	
	if (CWINDOW_Main && sender == CWINDOW_Main->ob.widget)
	{
		if (gMainWindow::closeAll())
			return true;
		if (!sender->isPersistent())
		{
			CWINDOW_delete_all();
			CWINDOW_Main = NULL;
		}
	}

	if (THIS->embed)
	{
		CWINDOW_Embedder = 0;
		CWINDOW_Embedded = false;
	}
	
	MAIN_check_quit();
	
	return false;
}

static void activate_window(gMainWindow *window)
{
	CWINDOW *active;

	if (window)
	{
		for(;;)
		{
			active = (CWINDOW *)GetObject(window);
			if (window->isTopLevel())
				break;
			if (GB.CanRaise(active, EVENT_Activate))
				break;
			window = window->parent()->window();
		}
	}
	else
		active = NULL;

	if (active == CWINDOW_Active)
		return;

	if (CWINDOW_Active)
	{
		GB.Raise(CWINDOW_Active, EVENT_Deactivate, 0);
		CWINDOW_Active = 0;
	}

	if (active)
	{
		GB.Raise(active, EVENT_Activate, 0);
	}

	CWINDOW_Active = active;
}

static void cb_activate(gMainWindow *sender)
{
	activate_window(sender);
}

static void cb_deactivate(gMainWindow *sender)
{
	activate_window(NULL);
}

/*static void cb_state(gMainWindow *sender)
{
	CWINDOW *_object = (CWINDOW*)GetObject(sender);
	GB.Raise(THIS, EVENT_State, 0);
}*/


/***************************************************************************

	Window

***************************************************************************/

BEGIN_METHOD(CWINDOW_new, GB_OBJECT parent;)

	GB_CLASS CLASS_container;
	CWIDGET *parent = NULL;
	int plug = 0;

	if (!MISSING(parent) && VARG(parent))
	{
		CLASS_container = GB.FindClass("Container");
		if (GB.Conv((GB_VALUE *)(void *)ARG(parent), (GB_TYPE)CLASS_container))
			return;
	
		parent=(CWIDGET*)VARG(parent);
		parent=GetContainer ((CWIDGET*)parent);
	}

	if ( CWINDOW_Embedder && (!CWINDOW_Embedded) && (!parent) )
	{
		plug=CWINDOW_Embedder;
		THIS->embed=true;
	}

	if (!parent) 
		THIS->ob.widget = new gMainWindow(plug);
	else
		THIS->ob.widget = new gMainWindow((gContainer *)parent->widget);

	InitControl(THIS->ob.widget, (CWIDGET*)THIS);
	
	WINDOW->onOpen = gb_raise_window_Open;
	WINDOW->onShow = gb_raise_window_Show;
	WINDOW->onHide = gb_raise_window_Hide;
	WINDOW->onMove = gb_raise_window_Move;
	WINDOW->onResize = gb_raise_window_Resize;
	WINDOW->onClose = gb_raise_window_Close;
	WINDOW->onActivate = cb_activate;
	WINDOW->onDeactivate = cb_deactivate;
	//WINDOW->onState = cb_state;

END_METHOD

BEGIN_METHOD_VOID(CWINDOW_free)

END_METHOD


BEGIN_METHOD_VOID(CWINDOW_next)

	int *vl;

	vl = (int *)GB.GetEnum();
	
	if (gMainWindow::count() <= *vl)
		GB.StopEnum();
	else
	{
		GB.ReturnObject(gMainWindow::get(*vl)->hFree);
		(*vl)++;
	}

END_METHOD


BEGIN_PROPERTY(CWINDOW_count)

	GB.ReturnInteger(gMainWindow::count());

END_PROPERTY


BEGIN_METHOD(CWINDOW_get_from_id, GB_INTEGER index)

	gMainWindow *ob = gMainWindow::get(VARG(index));

	GB.ReturnObject(ob ? ob->hFree : NULL);

END_METHOD


BEGIN_METHOD(CWINDOW_close, GB_INTEGER ret)

	GB.ReturnBoolean(close_window(THIS, VARGOPT(ret, 0)));

END_METHOD


BEGIN_METHOD_VOID(CWINDOW_raise)

	WINDOW->raise();

END_METHOD


BEGIN_METHOD_VOID(CWINDOW_show_modal)

	THIS->ret = 0;
	MODAL_windows++;
	WINDOW->showModal();
	MODAL_windows--;
	GB.ReturnInteger(THIS->ret);

END_METHOD


BEGIN_METHOD(Window_ShowPopup, GB_INTEGER x; GB_INTEGER y)

	THIS->ret = 0;
	MODAL_windows++;
	if (!MISSING(x) && !MISSING(y))
		WINDOW->showPopup(VARG(x), VARG(y));
	else
		WINDOW->showPopup();
	MODAL_windows--;
	GB.ReturnInteger(THIS->ret);

END_METHOD


BEGIN_METHOD_VOID(CWINDOW_show)

	WINDOW->showActivate();

END_METHOD


BEGIN_PROPERTY(CWINDOW_modal)

	GB.ReturnBoolean(WINDOW->isModal());

END_PROPERTY

BEGIN_PROPERTY(CWINDOW_top_level)

	GB.ReturnBoolean(WINDOW->isTopLevel());

END_PROPERTY

// 


BEGIN_PROPERTY(CWINDOW_persistent)

	if (READ_PROPERTY)
		GB.ReturnBoolean (WINDOW->isPersistent());
	else
	WINDOW->setPersistent(VPROP(GB_BOOLEAN));

END_PROPERTY


BEGIN_PROPERTY(CWINDOW_text)

	if (READ_PROPERTY) { GB.ReturnNewZeroString(WINDOW->text()); return; }
	WINDOW->setText((const char*)GB.ToZeroString(PROP(GB_STRING)));
	GB.Raise(THIS, EVENT_Title, 0);

END_PROPERTY


BEGIN_PROPERTY(CWINDOW_border)

	if (READ_PROPERTY)
		GB.ReturnBoolean(WINDOW->hasBorder());
	else
		WINDOW->setBorder(VPROP(GB_BOOLEAN));

END_PROPERTY


BEGIN_PROPERTY(CWINDOW_resizable)

	if (READ_PROPERTY)
		GB.ReturnBoolean(WINDOW->isResizable());
	else
		WINDOW->setResizable(VPROP(GB_BOOLEAN));

END_PROPERTY

#if 0
BEGIN_PROPERTY(CWINDOW_type)

	if (READ_PROPERTY)
		GB.ReturnInteger(WINDOW->getType());
	else
	{
		fprintf(stderr, "gb.gtk: warning: Window.Type is deprecated\n");
		WINDOW->setType(VPROP(GB_INTEGER));
	}

END_PROPERTY
#endif

BEGIN_PROPERTY(Window_Utility)

	if (READ_PROPERTY)
		GB.ReturnBoolean(WINDOW->isUtility());
	else
		WINDOW->setUtility(VPROP(GB_BOOLEAN));

END_PROPERTY


BEGIN_PROPERTY(CWINDOW_icon)

	if (READ_PROPERTY)
	{
		gPicture *pic = WINDOW->icon();
		GB.ReturnObject(pic ? pic->getTagValue() : 0);
	}
	else
	{
		CPICTURE *pic = (CPICTURE *)VPROP(GB_OBJECT);
		WINDOW->setIcon(pic ? pic->picture : 0);
		GB.Raise(THIS, EVENT_Icon, 0);
	}

END_PROPERTY


BEGIN_PROPERTY(CWINDOW_top_only)

	if (READ_PROPERTY) { GB.ReturnBoolean(WINDOW->topOnly()); return; }
	WINDOW->setTopOnly(VPROP(GB_BOOLEAN));

END_PROPERTY


BEGIN_PROPERTY(CWINDOW_skip_taskbar)

	if (READ_PROPERTY) { GB.ReturnBoolean(WINDOW->skipTaskBar()); return; }
	WINDOW->setSkipTaskBar(VPROP(GB_BOOLEAN));

END_PROPERTY


BEGIN_PROPERTY(CWINDOW_mask)

	if (READ_PROPERTY){  GB.ReturnBoolean(WINDOW->mask()); return; }
	WINDOW->setMask(VPROP(GB_BOOLEAN));

END_PROPERTY

BEGIN_PROPERTY(CWINDOW_picture)

	if (READ_PROPERTY)
	{
		gPicture *pic = WINDOW->picture();
		GB.ReturnObject(pic ? pic->getTagValue() : 0);
	}
	else
	{
		CPICTURE *pic = (CPICTURE *)VPROP(GB_OBJECT);
		WINDOW->setPicture(pic ? pic->picture : 0);
	}
	
END_PROPERTY


BEGIN_PROPERTY(CWINDOW_minimized)

	if (READ_PROPERTY) { GB.ReturnBoolean ( WINDOW->minimized() ); return; }
	WINDOW->setMinimized(VPROP(GB_BOOLEAN));

END_PROPERTY

BEGIN_PROPERTY(CWINDOW_maximized)

	if (READ_PROPERTY) { GB.ReturnBoolean ( WINDOW->maximized() ); return; }
	WINDOW->setMaximized(VPROP(GB_BOOLEAN));

END_PROPERTY

BEGIN_PROPERTY(CWINDOW_fullscreen)

	if (READ_PROPERTY) { GB.ReturnBoolean ( WINDOW->fullscreen() ); return; }
	WINDOW->setFullscreen(VPROP(GB_BOOLEAN));

END_PROPERTY

BEGIN_METHOD_VOID(CWINDOW_center)

	WINDOW->center();

END_METHOD


BEGIN_PROPERTY(CWINDOW_menu_count)

	GB.ReturnInteger(WINDOW->menuCount());

END_PROPERTY

BEGIN_PROPERTY(CWINDOW_stacking)

	if (READ_PROPERTY)
		GB.ReturnInteger(WINDOW->getStacking());
	else
		WINDOW->setStacking(VPROP(GB_INTEGER));

END_PROPERTY

BEGIN_PROPERTY(CWINDOW_sticky)

	if (READ_PROPERTY)
		GB.ReturnBoolean(WINDOW->getSticky());
	else
		WINDOW->setSticky(VPROP(GB_BOOLEAN));

END_PROPERTY

BEGIN_METHOD_VOID(CWINDOW_menu_next)

	gMenu *mn;
	int *ct;

	ct = (int *)GB.GetEnum();

	if (*ct >= gMenu::winChildCount(WINDOW)) 
	{ 
		GB.StopEnum(); 
		return; 
	}
	
	mn = gMenu::winChildMenu(WINDOW, *ct);
	(*ct)++;
	
	GB.ReturnObject(mn->hFree);

END_PROPERTY


BEGIN_METHOD(CWINDOW_menu_get, GB_INTEGER index)

	int index = VARG(index);
	
	if (index < 0 || index >= gMenu::winChildCount(WINDOW))
	{
		GB.Error(GB_ERR_BOUND);
		return;
	}
	
	GB.ReturnObject(gMenu::winChildMenu(WINDOW, index));

END_METHOD

BEGIN_PROPERTY(CWINDOW_control_count)

	GB.ReturnInteger(WINDOW->controlCount());

END_PROPERTY


BEGIN_METHOD_VOID(CWINDOW_control_next)

	int index;
	gControl *control;

	index = ENUM(int);
	
	control = WINDOW->getControl(index);
	
	if (!control)
	{
		GB.StopEnum();
		return;
	}
	
	ENUM(int) = index + 1;
	GB.ReturnObject(GetObject(control));

END_PROPERTY


BEGIN_METHOD(CWINDOW_get, GB_STRING name)

	gControl *control = WINDOW->getControl(GB.ToZeroString(ARG(name)));

	if (!control)
		GB.ReturnNull();
	else
		GB.ReturnObject(GetObject(control));

END_METHOD

BEGIN_PROPERTY(CWINDOW_closed)

	GB.ReturnBoolean(WINDOW->isClosed());

END_PROPERTY

BEGIN_PROPERTY(CWINDOW_menu_visible)

	if (READ_PROPERTY)
		GB.ReturnBoolean(WINDOW->isMenuBarVisible());
	else
		WINDOW->setMenuBarVisible(VPROP(GB_BOOLEAN));

END_PROPERTY

BEGIN_METHOD_VOID(CWINDOW_menu_show)

	WINDOW->setMenuBarVisible(true);

END_METHOD

BEGIN_METHOD_VOID(CWINDOW_menu_hide)

	WINDOW->setMenuBarVisible(false);

END_METHOD

BEGIN_PROPERTY(Window_Opacity)

	if (READ_PROPERTY)
		GB.ReturnInteger(WINDOW->opacity() * 100);
	else
	{
		double opacity = VPROP(GB_INTEGER) / 100.0;
		
		if (opacity < 0.0)
			opacity = 0.0;
		else if (opacity > 1.0)
			opacity = 1.0;
		
		WINDOW->setOpacity(opacity);
	}

END_PROPERTY

BEGIN_PROPERTY(Window_Screen)

	GB.ReturnInteger(WINDOW->screen());

END_PROPERTY

BEGIN_PROPERTY(Window_Transparent)

	bool trans = WINDOW->isTransparent();

	if (READ_PROPERTY)
		GB.ReturnBoolean(trans);
	else
	{
		bool new_trans = VPROP(GB_BOOLEAN);
		if (trans == new_trans)
			return;

		if (!new_trans)
		{
			GB.Error("Transparent property cannot be reset");
			return;
		}

		WINDOW->setTransparent(true);
	}

END_PROPERTY

//-------------------------------------------------------------------------

BEGIN_METHOD_VOID(CFORM_new)

	if (!GB.Parent(_object))
		GB.Attach(_object, _object, "Form");
	
	WINDOW->setName(GB.GetClassName((void *)THIS));

END_METHOD


BEGIN_METHOD_VOID(CFORM_main)

	CWINDOW *form;

	form = (CWINDOW *)GB.AutoCreate(GB.GetClass(NULL), 0);
	if (!((gMainWindow *)form->ob.widget)->isHidden())
		CWINDOW_show(form, NULL);

END_METHOD


BEGIN_METHOD(CFORM_load, GB_OBJECT parent)

	if (!MISSING(parent))
		GB.Push(1, GB_T_OBJECT, VARG(parent));

	GB.AutoCreate(GB.GetClass(NULL), MISSING(parent) ? 0 : 1);

END_METHOD

/***************************************************************************

	Declarations

***************************************************************************/

GB_DESC CWindowMenusDesc[] =
{
	GB_DECLARE(".Window.Menus", 0), GB_VIRTUAL_CLASS(),

	GB_METHOD("_next", "Menu", CWINDOW_menu_next, 0),
	GB_METHOD("_get", "Menu", CWINDOW_menu_get, "(Index)i"),
	GB_PROPERTY_READ("Count", "i", CWINDOW_menu_count),
	GB_METHOD("Show", NULL, CWINDOW_menu_show, NULL),
	GB_METHOD("Hide", NULL, CWINDOW_menu_hide, NULL),
	GB_PROPERTY("Visible", "b", CWINDOW_menu_visible),

	GB_END_DECLARE
};

GB_DESC CWindowControlsDesc[] =
{
	GB_DECLARE(".Window.Controls", 0), GB_VIRTUAL_CLASS(),

	GB_METHOD("_next", "Control", CWINDOW_control_next, 0),
	GB_METHOD("_get", "Control", CWINDOW_get, "(Name)s"),
	GB_PROPERTY_READ("Count", "i", CWINDOW_control_count),

	GB_END_DECLARE
};

#if 0
GB_DESC CWindowTypeDesc[] =
{
	GB_DECLARE("WindowType", 0), GB_VIRTUAL_CLASS(),

	GB_CONSTANT("Normal", "i", _NET_WM_WINDOW_TYPE_NORMAL),
	GB_CONSTANT("Dock", "i", _NET_WM_WINDOW_TYPE_DOCK),
	GB_CONSTANT("Toolbar", "i", _NET_WM_WINDOW_TYPE_TOOLBAR),
	GB_CONSTANT("Menu", "i", _NET_WM_WINDOW_TYPE_MENU),
	GB_CONSTANT("Utility", "i", _NET_WM_WINDOW_TYPE_UTILITY),
	GB_CONSTANT("Splash", "i", _NET_WM_WINDOW_TYPE_SPLASH),
	GB_CONSTANT("Dialog", "i", _NET_WM_WINDOW_TYPE_DIALOG),
	GB_CONSTANT("DropDownMenu", "i", _NET_WM_WINDOW_TYPE_DROPDOWN_MENU),
	GB_CONSTANT("PopupMenu", "i", _NET_WM_WINDOW_TYPE_POPUP_MENU),
	GB_CONSTANT("Tooltip", "i", _NET_WM_WINDOW_TYPE_TOOLTIP),
	GB_CONSTANT("Notification", "i", _NET_WM_WINDOW_TYPE_NOTIFICATION),
	GB_CONSTANT("Combo", "i", _NET_WM_WINDOW_TYPE_COMBO),
	GB_CONSTANT("DragAndDrop", "i", _NET_WM_WINDOW_TYPE_DND),
	GB_CONSTANT("Desktop", "i", _NET_WM_WINDOW_TYPE_DESKTOP),

	GB_END_DECLARE
};
#endif

GB_DESC CWindowDesc[] =
{
	GB_DECLARE("Window", sizeof(CWINDOW)), GB_INHERITS("Container"),

	GB_CONSTANT("Normal", "i", 0),
	GB_CONSTANT("Above", "i", 1),
	GB_CONSTANT("Below", "i", 2),

	GB_METHOD("_new", NULL, CWINDOW_new, "[(Parent)Control;]"),
	GB_METHOD("_free", NULL, CWINDOW_free, NULL),
	GB_METHOD("_get", "Control", CWINDOW_get, "(Name)s"),

	GB_METHOD("Close", "b", CWINDOW_close, "[(Return)i]"),
	GB_METHOD("Raise", NULL, CWINDOW_raise, NULL),
	GB_METHOD("Show", NULL, CWINDOW_show, NULL),
	//GB_METHOD("Hide", NULL, CWINDOW_hide, NULL),
	//GB_PROPERTY("Visible", "b", CWINDOW_visible),
	GB_METHOD("ShowModal", "i", CWINDOW_show_modal, NULL),
	GB_METHOD("ShowDialog", "i", CWINDOW_show_modal, NULL),
	GB_METHOD("ShowPopup", "i", Window_ShowPopup, "[(X)i(Y)i]"),
	GB_METHOD("Center", NULL, CWINDOW_center, NULL),
	GB_PROPERTY_READ("Modal", "b", CWINDOW_modal),
	GB_PROPERTY_READ("TopLevel", "b", CWINDOW_top_level),
	GB_PROPERTY_READ("Closed", "b", CWINDOW_closed),

	GB_PROPERTY("Stacking","i",CWINDOW_stacking),
	GB_PROPERTY("Sticky","b",CWINDOW_sticky),
	GB_PROPERTY("Persistent", "b", CWINDOW_persistent),
	GB_PROPERTY("Text", "s", CWINDOW_text),
	GB_PROPERTY("Title", "s", CWINDOW_text),
	GB_PROPERTY("Caption", "s", CWINDOW_text),
	GB_PROPERTY("Icon", "Picture", CWINDOW_icon),

	GB_PROPERTY("Minimized", "b", CWINDOW_minimized),
	GB_PROPERTY("Maximized", "b", CWINDOW_maximized),
	GB_PROPERTY("FullScreen", "b", CWINDOW_fullscreen),

	GB_PROPERTY("TopOnly", "b", CWINDOW_top_only),
	GB_PROPERTY("SkipTaskbar", "b", CWINDOW_skip_taskbar),
	GB_PROPERTY("Opacity", "i", Window_Opacity),
	GB_PROPERTY("Transparent", "b", Window_Transparent),

	GB_PROPERTY("Arrangement", "i", Container_Arrangement),
	GB_PROPERTY("AutoResize", "b", Container_AutoResize),
	GB_PROPERTY("Padding", "i", Container_Padding),
	GB_PROPERTY("Spacing", "b", Container_Spacing),
	GB_PROPERTY("Margin", "b", Container_Margin),
	GB_PROPERTY("Indent", "b", Container_Indent),
	GB_PROPERTY("Invert", "b", Container_Invert),

	//GB_PROPERTY("Type", "i", CWINDOW_type),
	GB_PROPERTY("Utility", "b", Window_Utility),
	GB_PROPERTY("Border", "b", CWINDOW_border),
	GB_PROPERTY("Resizable", "b", CWINDOW_resizable),

	GB_PROPERTY("Mask","b",CWINDOW_mask),
	GB_PROPERTY("Picture", "Picture", CWINDOW_picture),
	
	GB_PROPERTY_READ("Screen", "i", Window_Screen),

	GB_PROPERTY_SELF("Menus", ".Window.Menus"),
	GB_PROPERTY_SELF("Controls", ".Window.Controls"),

	WINDOW_DESCRIPTION,

	GB_EVENT("Close", "b", NULL, &EVENT_Close),
	GB_EVENT("Open", NULL, NULL, &EVENT_Open),
	GB_EVENT("Activate", NULL, NULL, &EVENT_Activate),
	GB_EVENT("Deactivate", NULL, NULL, &EVENT_Deactivate),
	GB_EVENT("Move", NULL, NULL, &EVENT_Move),
	GB_EVENT("Resize", NULL, NULL, &EVENT_Resize),
	GB_EVENT("Show", NULL, NULL, &EVENT_Show),
	GB_EVENT("Hide", NULL, NULL, &EVENT_Hide),
	GB_EVENT("Title", NULL, NULL, &EVENT_Title),
	GB_EVENT("Icon", NULL, NULL, &EVENT_Icon),
	//GB_EVENT("State", NULL, NULL, &EVENT_State),

	GB_END_DECLARE
};


GB_DESC CWindowsDesc[] =
{
	GB_DECLARE("Windows", 0), GB_VIRTUAL_CLASS(),

	GB_STATIC_METHOD("_next", "Window", CWINDOW_next, 0),
	GB_STATIC_METHOD("_get", "Window", CWINDOW_get_from_id, "(Id)i"),
	GB_STATIC_PROPERTY_READ("Count", "i", CWINDOW_count),

	GB_END_DECLARE
};


GB_DESC CFormDesc[] =
{
	GB_DECLARE("Form", sizeof(CFORM)), GB_INHERITS("Window"),
	GB_AUTO_CREATABLE(),

	GB_STATIC_METHOD("Main", 0, CFORM_main, 0),
	GB_STATIC_METHOD("Load", 0, CFORM_load, "[(Parent)Control;]"),
	GB_METHOD("_new", 0, CFORM_new, 0),
	
	FORM_DESCRIPTION,

	GB_END_DECLARE
};
