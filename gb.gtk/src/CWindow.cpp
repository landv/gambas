/***************************************************************************

  CWindow.cpp

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

#define __CWINDOW_CPP

#include "main.h"
#include "CWindow.h"
#include "CWidget.h"
#include "CContainer.h"
#include "CPicture.h"
#include "CMenu.h"
#include "CDraw.h"
#include "gapplication.h"

long CWINDOW_Embedder = 0;
bool CWINDOW_Embedded = false;

CWINDOW *CWINDOW_Active = NULL;

static CWINDOW *MAIN_Window=NULL;
static long MODAL_windows=0;

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


CWINDOW* WINDOW_get_main()
{
	return MAIN_Window;
}

void WINDOW_kill(CWINDOW *win)
{
	if (MAIN_Window==win) 
	{
		//fprintf(stderr, "MAIN_Window = NULL\n");
		MAIN_Window=NULL;
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

static bool closeAll()
{
  int i;
  gMainWindow *win;
  CWINDOW *window;
  
  for(i = 0; i < gMainWindow::count(); i++)
  {
    win = gMainWindow::get(i);
    if (!win)
      break;
    //fprintf(stderr, "closeAll: try %p\n", win);
    window = (CWINDOW *)GetObject(win);
    if (window == MAIN_Window)
      continue;
    if (close_window(window))
      return true;
  }
  
  return false;
}

static bool deleteAll()
{
  int i;
  gMainWindow *win;
  CWINDOW *window;
  
  for(i = 0; i < gMainWindow::count(); i++)
  {
    win = gMainWindow::get(i);
    if (!win)
      break;
    //fprintf(stderr, "closeAll: try %p\n", win);
    window = (CWINDOW *)GetObject(win);
    if (window == MAIN_Window)
      continue;
    win->destroy();
  }
  
  return false;
}

static bool gb_raise_window_Close(gMainWindow *sender)
{
	CWINDOW *_ob=(CWINDOW*)GetObject(sender);

	if (!_ob) return false;
	if (GB.Raise((void*)_ob,EVENT_Close,0)) return true;

  if (MAIN_Window && sender == MAIN_Window->ob.widget)
  {
    if (closeAll())
      return true;
    if (!sender->isPersistent())
    {
    	deleteAll();
    	MAIN_Window = NULL;
		}
  }

	if (_ob->embed)
	{
		CWINDOW_Embedder = 0;
		CWINDOW_Embedded = false;
	}
	
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
	WINDOW->resize(200,150);

	if ( (!MAIN_Window) && (!parent)) 
	{
		MAIN_Window = THIS;
		//fprintf(stderr, "MAIN_Window = %p\n", MAIN_Window);
	}

	/*if (parent)
	{
		gb_post_window_Open(WINDOW);
		gb_post_window_Show(WINDOW);
	}*/

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


BEGIN_METHOD_VOID(CWINDOW_show)

	//if (MODAL_windows)
	//	CWINDOW_show_modal(THIS, NULL);
	//else
	WINDOW->show();

END_METHOD


BEGIN_PROPERTY(CWINDOW_modal)

	GB.ReturnBoolean(WINDOW->modal());

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

	if (READ_PROPERTY) { GB.ReturnNewString(WINDOW->text(),0); return; }
	WINDOW->setText((const char*)GB.ToZeroString(PROP(GB_STRING)));
	GB.Raise(THIS, EVENT_Title, 0);

END_PROPERTY


BEGIN_PROPERTY(CWINDOW_border)

	if (READ_PROPERTY) { GB.ReturnInteger(WINDOW->getBorder()); return; }
	WINDOW->setBorder(VPROP(GB_INTEGER));

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

BEGIN_PROPERTY(CWINDOW_toolbox)

	if (READ_PROPERTY)
		GB.ReturnBoolean(WINDOW->isToolBox());
	else
		WINDOW->setToolBox(VPROP(GB_BOOLEAN));

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

	CMENU *Mn;
	gMenu *mn;
	long *ct;

	ct=(long*)GB.GetEnum();

	if ( ct[0]>=gMenu::winChildCount(WINDOW)  ) { GB.StopEnum(); return; }
	mn=gMenu::winChildMenu(WINDOW,ct[0]);
	Mn=(CMENU*)mn->hFree;
	ct[0]++;
	GB.ReturnObject(Mn);

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

BEGIN_METHOD_VOID(CFORM_new)

	GB.Attach(_object, _object, "Form");

END_METHOD


BEGIN_METHOD_VOID(CFORM_main)

	CWINDOW *form;

	form = (CWINDOW *)GB.AutoCreate(GB.GetClass(NULL), 0);
	if (!((gMainWindow *)form->ob.widget)->isHidden())
		CWINDOW_show(form, NULL);

END_METHOD


BEGIN_METHOD(CFORM_load, GB_OBJECT parent)

  if (!MISSING(parent))
  {
    GB.Push(1, GB_T_OBJECT, VARG(parent));
    //qDebug("CFORM_load + parent");
  }

  GB.AutoCreate(GB.GetClass(NULL), MISSING(parent) ? 0 : 1);

END_METHOD


/***************************************************************************

  Declarations

***************************************************************************/

GB_DESC CWindowMenusDesc[] =
{
  GB_DECLARE(".WindowMenus", 0), GB_VIRTUAL_CLASS(),

  GB_METHOD("_next", "Menu", CWINDOW_menu_next, 0),
  GB_METHOD("_get", "Menu", CWINDOW_menu_get, "(Index)i"),
  GB_PROPERTY_READ("Count", "i", CWINDOW_menu_count),

  GB_END_DECLARE
};

GB_DESC CWindowControlsDesc[] =
{
  GB_DECLARE(".WindowControls", 0), GB_VIRTUAL_CLASS(),

  GB_METHOD("_next", "Control", CWINDOW_control_next, 0),
  GB_METHOD("_get", "Control", CWINDOW_get, "(Name)s"),
  GB_PROPERTY_READ("Count", "i", CWINDOW_control_count),

  GB_END_DECLARE
};

GB_DESC CWindowDesc[] =
{
  GB_DECLARE("Window", sizeof(CWINDOW)), GB_INHERITS("Container"),

  GB_CONSTANT("None", "i", 0),
  GB_CONSTANT("Fixed", "i", 1),
  GB_CONSTANT("Resizable", "i", 2),

  GB_CONSTANT("Normal", "i", 0),
  GB_CONSTANT("Above", "i", 1),
  GB_CONSTANT("Below", "i", 2),

  GB_METHOD("_new", 0, CWINDOW_new, "[(Parent)Control;]"),
  GB_METHOD("_free",0,CWINDOW_free,0),
  GB_METHOD("_get", "Control", CWINDOW_get, "(Name)s"),

  GB_METHOD("Close", "b", CWINDOW_close, "[(Return)i]"),
  GB_METHOD("Raise", 0, CWINDOW_raise, 0),
  GB_METHOD("Show", 0, CWINDOW_show, 0),
  //GB_METHOD("Hide", 0, CWINDOW_hide, 0),
  //GB_PROPERTY("Visible", "b", CWINDOW_visible),
  GB_METHOD("ShowModal", "i", CWINDOW_show_modal, 0),
  GB_METHOD("ShowDialog", "i", CWINDOW_show_modal, 0),
  GB_METHOD("Center", 0, CWINDOW_center, 0),
  GB_PROPERTY_READ("Modal", "b", CWINDOW_modal),
  GB_PROPERTY_READ("TopLevel", "b", CWINDOW_top_level),
  GB_PROPERTY_READ("Closed", "b", CWINDOW_closed),

  GB_PROPERTY("Stacking", "i", CWINDOW_stacking),
  GB_PROPERTY("Sticky", "b", CWINDOW_sticky),
  GB_PROPERTY("Persistent", "b", CWINDOW_persistent),
  GB_PROPERTY("Text", "s", CWINDOW_text),
  GB_PROPERTY("Title", "s", CWINDOW_text),
  GB_PROPERTY("Caption", "s", CWINDOW_text),
  GB_PROPERTY("Icon", "Picture", CWINDOW_icon),
  GB_PROPERTY("Border", "i", CWINDOW_border),

  GB_PROPERTY("Minimized", "b", CWINDOW_minimized),
  GB_PROPERTY("Maximized", "b", CWINDOW_maximized),
  GB_PROPERTY("FullScreen", "b", CWINDOW_fullscreen),

  GB_PROPERTY("TopOnly", "b", CWINDOW_top_only),
  GB_PROPERTY("SkipTaskbar", "b", CWINDOW_skip_taskbar),
  GB_PROPERTY("ToolBox", "b", CWINDOW_toolbox),
  GB_PROPERTY("Arrangement", "i", CCONTAINER_arrangement),

  GB_PROPERTY("Padding", "i", CCONTAINER_padding),
  GB_PROPERTY("Spacing", "i", CCONTAINER_spacing),
  GB_PROPERTY("AutoResize", "b", CCONTAINER_auto_resize),

  GB_PROPERTY("Mask","b",CWINDOW_mask),
  GB_PROPERTY("Picture", "Picture", CWINDOW_picture),

  GB_PROPERTY_SELF("Menus", ".WindowMenus"),
  GB_PROPERTY_SELF("Controls", ".WindowControls"),

  GB_CONSTANT("_Properties", "s", CWINDOW_PROPERTIES),
  GB_CONSTANT("_DefaultEvent", "s", "Open"),
  GB_CONSTANT("_Arrangement", "i", ARRANGE_FILL),

  GB_EVENT("Close", "b", 0, &EVENT_Close),
  GB_EVENT("Open", 0, 0, &EVENT_Open),
  GB_EVENT("Activate", 0, 0, &EVENT_Activate),
  GB_EVENT("Deactivate", 0, 0, &EVENT_Deactivate),
  GB_EVENT("Move", 0, 0, &EVENT_Move),
  GB_EVENT("Resize", 0, 0, &EVENT_Resize),
  GB_EVENT("Show", 0, 0, &EVENT_Show),
  GB_EVENT("Hide", 0, 0, &EVENT_Hide),
  GB_EVENT("Title", 0, 0, &EVENT_Title),
  GB_EVENT("Icon", 0, 0, &EVENT_Icon),

  GB_INTERFACE("Draw", &DRAW_Interface),

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

  GB_END_DECLARE
};
