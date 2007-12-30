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

#include <stdio.h>

#include "gambas.h"
#include "main.h"
#include "widgets.h"
#include "CWindow.h"
#include "CWidget.h"
#include "CContainer.h"
#include "CPicture.h"
#include "CMenu.h"

long CWINDOW_Embedder = 0;
bool CWINDOW_Embedded = false;

static CWINDOW *MAIN_Window=NULL;
static long MODAL_windows=0;
static long WINDOW_return=0;

DECLARE_EVENT(EVENT_Open);
DECLARE_EVENT(EVENT_Show);
DECLARE_EVENT(EVENT_Hide);
DECLARE_EVENT(EVENT_Close);
DECLARE_EVENT(EVENT_Activate);
DECLARE_EVENT(EVENT_Deactivate);
DECLARE_EVENT(EVENT_Move);
DECLARE_EVENT(EVENT_Resize);

CWINDOW* WINDOW_get_main()
{
	return MAIN_Window;
}

void WINDOW_kill(CWINDOW *win)
{
	if (MAIN_Window==win) MAIN_Window=NULL;
}


void gb_raise_window_Open(gMainWindow *sender)
{
	CWIDGET *_ob=GetObject(sender);

	if (!_ob) return;
	GB.Raise((void*)_ob,EVENT_Open,0);
}

void gb_post_window_Open(gMainWindow *sender)
{
	GB.Post( (void (*)())gb_raise_window_Open,(long)sender);
}

void gb_raise_window_Show(gMainWindow *sender)
{
	CWIDGET *_ob=GetObject(sender);

	if (!_ob) return;
	GB.Raise((void*)_ob,EVENT_Show,0);
}

void gb_post_window_Show(gMainWindow *sender)
{
	GB.Post( (void (*)())gb_raise_window_Show,(long)sender);
}

void gb_raise_window_Hide(gMainWindow *sender)
{
	CWIDGET *_ob=GetObject(sender);

	if (!_ob) return;
	GB.Raise((void*)_ob,EVENT_Hide,0);
}

void gb_raise_window_Move(gMainWindow *sender)
{
	CWIDGET *_ob=GetObject(sender);

	if (!_ob) return;
	GB.Raise((void*)_ob,EVENT_Move,0);
}

void gb_raise_window_Resize(gMainWindow *sender)
{
	CWIDGET *_ob=GetObject(sender);

	if (!_ob) return;
	GB.Raise((void*)_ob,EVENT_Resize,0);
}

bool gb_raise_window_Close(gMainWindow *sender)
{
	CWINDOW *_ob=(CWINDOW*)GetObject(sender);

	if (!_ob) return false;
	if (GB.Raise((void*)_ob,EVENT_Close,0)) return true;

	if (_ob->persistent)
	{
		sender->hide();
		return true;
	}

	if (_ob->embed)
	{
		CWINDOW_Embedder = 0;
		CWINDOW_Embedded = false;
	}
	_ob->modal=0;
	return false;

}


/***************************************************************************

  Window

***************************************************************************/
BEGIN_METHOD(CWINDOW_new, GB_OBJECT parent;)

	GB_CLASS CLASS_container;
	CWIDGET *parent=NULL;
	long plug=0;

	THIS->persistent=0;
	if (!MISSING(parent))
	{
		if (VARG(parent))
		{
			CLASS_container = GB.FindClass("Container");
			if ( !GB.Is(VARG(parent), CLASS_container) )
			{
				GB.Error("The parent of a Window must be a Container or a Workspace");
				return;
			}
			parent=(CWIDGET*)VARG(parent);
			parent=GetContainer ((CWIDGET*)parent);
		}
	}


	if ( CWINDOW_Embedder && (!CWINDOW_Embedded) && (!parent) )
	{
		plug=CWINDOW_Embedder;
		THIS->embed=true;
	}

	if (!parent) THIS->widget=new gMainWindow(plug);
	else         THIS->widget=new gMainWindow(parent->widget);

	InitControl(THIS->widget,(CWIDGET*)THIS);
	WINDOW->onOpen=gb_post_window_Open;
	WINDOW->onShow=gb_post_window_Show;
	WINDOW->onHide=gb_raise_window_Hide;
	WINDOW->onMove=gb_raise_window_Move;
	WINDOW->onResize=gb_raise_window_Resize;
	WINDOW->onClose=gb_raise_window_Close;

	if ( (!MAIN_Window) && (!parent)) MAIN_Window=THIS;

	if (parent)
	{
		gb_post_window_Open(WINDOW);
		gb_post_window_Show(WINDOW);
	}


END_METHOD

BEGIN_METHOD_VOID(CWINDOW_free)

	if (THIS->icon) GB.Unref((void**)&THIS->icon);
	if (THIS->picture) GB.Unref((void**)&THIS->picture);
	THIS->icon=NULL;
	THIS->picture=NULL;

END_METHOD

BEGIN_METHOD_VOID(CFORM_new)

	GB.Attach(_object, _object, "Form");

END_METHOD


BEGIN_METHOD_VOID(CFORM_main)

	CWINDOW *form;

	form = (CWINDOW *)GB.AutoCreate(GB.GetClass(NULL), 0);
	form->widget->show();

END_METHOD


BEGIN_METHOD(CFORM_load, GB_OBJECT parent;GB_BOOLEAN Plug;)

	void *Par=NULL;
	bool Plg=false;
	long npar=0;

	if (!MISSING(parent)) { Par=VARG(parent); npar++; }
	if (!MISSING(Plug)) { Plg=VARG(Plug); npar++; }

	GB.Push(1, GB_T_OBJECT,Par);
	GB.AutoCreate(GB.GetClass(NULL),npar);

END_METHOD



BEGIN_METHOD_VOID(CWINDOW_next)

	long *vl;

	vl=(long*)GB.GetEnum();
	if (gApplication::winCount()<=vl[0])
	{
		GB.StopEnum();
	}
	else
	{
		GB.ReturnObject ( gApplication::winItem(vl[0])->hFree );
		vl[0]++;
	}

END_METHOD


BEGIN_PROPERTY(CWINDOW_count)

	GB.ReturnInteger( gApplication::winCount());

END_PROPERTY


BEGIN_METHOD(CWINDOW_get, GB_INTEGER id)

	gControl *ob=gApplication::winItem(VARG(id));

	if (ob) GB.ReturnObject(ob->hFree);

END_METHOD

BEGIN_METHOD(CWINDOW_close, GB_INTEGER ret)

	if (THIS->modal)
	{
		WINDOW_return=VARG(ret);
		THIS->modal=0;
	}

	if (THIS->persistent)
		WINDOW->hide();
	else
		WINDOW->destroy();

END_METHOD


BEGIN_METHOD_VOID(CWINDOW_raise)

	WINDOW->raise();

END_METHOD


BEGIN_METHOD_VOID(CWINDOW_show_modal)

	WINDOW_return=0;
	THIS->modal=1;
	MODAL_windows++;
	WINDOW->showModal();
	while (THIS->modal)
	{
		do_iteration();
		gControl::cleanRemovedControls();
		if (!MAIN_Window) break;
	}
	MODAL_windows--;
	GB.ReturnInteger(WINDOW_return);

END_METHOD


BEGIN_METHOD_VOID(CWINDOW_show)

	if (MODAL_windows)
		CWINDOW_show_modal(THIS, NULL);
	else
		WINDOW->show();

END_METHOD


BEGIN_PROPERTY(CWINDOW_modal)

	GB.ReturnBoolean(WINDOW->modal());

END_PROPERTY




BEGIN_PROPERTY(CWINDOW_persistent)

	if (READ_PROPERTY)
	{
		GB.ReturnBoolean (THIS->persistent);
		return;
	}

	if (WINDOW->pr) return;

	if (VPROP(GB_BOOLEAN))
		THIS->persistent=1;
	else
		THIS->persistent=0;

END_PROPERTY


BEGIN_PROPERTY(CWINDOW_text)

	if (READ_PROPERTY) { GB.ReturnNewString(WINDOW->text(),0); return; }
	WINDOW->setText((const char*)GB.ToZeroString(PROP(GB_STRING)));

END_PROPERTY




BEGIN_PROPERTY(CWINDOW_border)

	if (READ_PROPERTY) { GB.ReturnInteger(WINDOW->_border()); return; }
	WINDOW->setBorder(VPROP(GB_INTEGER));

END_PROPERTY


BEGIN_PROPERTY(CWINDOW_icon)

	CPICTURE *pic=NULL;
	gPicture *hPic=NULL;

	if (READ_PROPERTY)
	{
		GB.ReturnObject(THIS->icon);
		return;
	}

	pic=(CPICTURE*)VPROP(GB_OBJECT);
	if (pic) GB.Ref((void*)pic);
	if (THIS->icon) GB.Unref((void**)&THIS->icon);
	THIS->icon=pic;
	if (!pic) WINDOW->setIcon(NULL);
	else      WINDOW->setIcon(pic->picture);


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

	CPICTURE *pic;

	if (READ_PROPERTY)
	{
		GB.ReturnObject(THIS->picture);
		return;
	}

	pic=(CPICTURE*)VPROP(GB_OBJECT);
	if (pic) GB.Ref((void*)pic);
	if (THIS->picture) GB.Unref((void**)&THIS->picture);
	THIS->picture=pic;
	if (!pic) WINDOW->setPicture(NULL);
	else      WINDOW->setPicture(pic->picture);


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

	if (READ_PROPERTY) { GB.ReturnInteger(WINDOW->getStacking()); return; }
	WINDOW->setStacking(VPROP(GB_INTEGER));

END_PROPERTY

BEGIN_PROPERTY(CWINDOW_sticky)

	if (READ_PROPERTY) { GB.ReturnInteger(WINDOW->getSticky()); return; }
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




/***************************************************************************

  Declarations

***************************************************************************/


GB_DESC CWindowMenusDesc[] =
{
  GB_DECLARE(".WindowMenus", 0), GB_VIRTUAL_CLASS(),

  GB_METHOD("_next", "Menu", CWINDOW_menu_next, NULL),
  GB_PROPERTY_READ("Count", "i", CWINDOW_menu_count),

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

  GB_METHOD("_new", NULL, CWINDOW_new, "[(Parent)Control;]"),
  GB_METHOD("_free",NULL,CWINDOW_free,NULL),

  GB_METHOD("Close", "b", CWINDOW_close, "[(Return)i]"),
  GB_METHOD("Raise", NULL, CWINDOW_raise, NULL),
  GB_METHOD("Show", NULL, CWINDOW_show, NULL),
  GB_METHOD("ShowModal", "i", CWINDOW_show_modal, NULL),
  GB_METHOD("ShowDialog", "i", CWINDOW_show_modal, NULL),
  GB_METHOD("Center", NULL, CWINDOW_center, NULL),
  GB_PROPERTY_READ("Modal", "b", CWINDOW_modal),

  GB_PROPERTY("Stacking","i",CWINDOW_stacking),
  GB_PROPERTY("Sticky","b",CWINDOW_sticky),
  GB_PROPERTY("Persistent", "b", CWINDOW_persistent),
  GB_PROPERTY("Text", "s", CWINDOW_text),
  GB_PROPERTY("Title", "s", CWINDOW_text),
  GB_PROPERTY("Caption", "s", CWINDOW_text),
  GB_PROPERTY("Icon", "Picture", CWINDOW_icon),
  GB_PROPERTY("Border", "i<Window,None,Fixed,Resizable>", CWINDOW_border),

  GB_PROPERTY("Minimized", "b", CWINDOW_minimized),
  GB_PROPERTY("Maximized", "b", CWINDOW_maximized),
  GB_PROPERTY("FullScreen", "b", CWINDOW_fullscreen),

  GB_PROPERTY("TopOnly", "b", CWINDOW_top_only),
  GB_PROPERTY("SkipTaskbar", "b", CWINDOW_skip_taskbar),
  GB_PROPERTY("Arrangement", "i<Arrange>", CCONTAINER_arrangement),

  GB_PROPERTY("Padding", "i", CCONTAINER_padding),
  GB_PROPERTY("Spacing", "i", CCONTAINER_spacing),

  GB_PROPERTY("Mask","b",CWINDOW_mask),
  GB_PROPERTY("Picture", "Picture", CWINDOW_picture),

  GB_PROPERTY_SELF("Menus", ".WindowMenus"),

  GB_CONSTANT("_Properties", "s", CWINDOW_PROPERTIES),
  GB_CONSTANT("_DefaultEvent", "s", "Open"),
  GB_CONSTANT("_Arrangement", "i", ARRANGE_FILL),

  GB_EVENT("Close", "b", NULL, &EVENT_Close),
  GB_EVENT("Open", NULL, NULL, &EVENT_Open),
  GB_EVENT("Activate", NULL, NULL, &EVENT_Activate),
  GB_EVENT("Deactivate", NULL, NULL, &EVENT_Deactivate),
  GB_EVENT("Move", NULL, NULL, &EVENT_Move),
  GB_EVENT("Resize", NULL, NULL, &EVENT_Resize),
  GB_EVENT("Show", NULL, NULL, &EVENT_Show),
  GB_EVENT("Hide", NULL, NULL, &EVENT_Hide),

  GB_END_DECLARE
};


GB_DESC CWindowsDesc[] =
{
  GB_DECLARE("Windows", 0), GB_VIRTUAL_CLASS(),

  GB_STATIC_METHOD("_next", "Window", CWINDOW_next, NULL),
  GB_STATIC_METHOD("_get", "Window", CWINDOW_get, "(Id)i"),
  GB_STATIC_PROPERTY_READ("Count", "i", CWINDOW_count),

  GB_END_DECLARE
};


GB_DESC CFormDesc[] =
{
  GB_DECLARE("Form", sizeof(CFORM)), GB_INHERITS("Window"),
  GB_AUTO_CREATABLE(),

  GB_STATIC_METHOD("Main", NULL, CFORM_main, NULL),
  GB_STATIC_METHOD("Load", NULL, CFORM_load, "[(Parent)Control;]"),
  GB_METHOD("_new", NULL, CFORM_new, NULL),

  GB_END_DECLARE
};
