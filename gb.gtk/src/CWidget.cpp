/***************************************************************************

  CWidget.cpp

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

#define __CWIDGET_CPP

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

//Plug
DECLARE_EVENT(EVENT_Plugged);
DECLARE_EVENT(EVENT_Unplugged);
DECLARE_EVENT(EVENT_PlugError);

//static void *CLASS_Image = NULL;
static void *CLASS_UserContainer = NULL;
static void *CLASS_UserControl = NULL;

/** Action *****************************************************************/

static bool has_action(void *control)
{
	if (GB.Is(control, GB.FindClass("Menu")))
		return ((CMENU *)(control))->widget->action();
	else
		return ((CWIDGET *)(control))->widget->action();
}

static void set_action(void *control, bool v)
{
	if (GB.Is(control, GB.FindClass("Menu")))
		((CMENU *)(control))->widget->setAction(v);
	else
		((CWIDGET *)(control))->widget->setAction(v);
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

// static void raise_menu(void *_object)
// {
// 	GB.Raise(THIS, EVENT_Menu, 0);
// 	GB.Unref(POINTER(&_object));
// }

bool gb_raise_MouseEvent(gControl *sender,long type)
{
	void *ob = GetObject(sender);
	
	if (!ob) return false; // possible, for MouseDrag for example
	
	switch(type)
	{
		case gEvent_MousePress:
			GB.Raise(ob, EVENT_MouseDown, 0);
			break;
			
		case gEvent_MouseRelease:
			GB.Raise(ob, EVENT_MouseUp, 0);
			break;
			
		case gEvent_MouseMove:
			GB.Raise(ob, EVENT_MouseMove, 0);
			break;
			
		case gEvent_MouseDrag:
			if (gMouse::button() && GB.CanRaise(ob, EVENT_MouseDrag))
				GB.Raise(ob, EVENT_MouseDrag, 0);
			break;
			
		case gEvent_MouseWheel:
			GB.Raise(ob, EVENT_MouseWheel, 0);
			break;
			
		case gEvent_MouseMenu:
			if (GB.CanRaise(ob, EVENT_Menu))
			{
				GB.Raise(ob, EVENT_Menu, 0);
				return true;
				//GB.Ref((void *)_ob);
				//GB.Post((GB_POST_FUNC)raise_menu, (long)_ob);
				//return true;
			}
			break;
			
		case gEvent_MouseDblClick:
			//fprintf(stderr, "dblclick: %s (%p)\n", sender->name(), _ob);
			GB.Raise(ob, EVENT_DblClick, 0);
			break;
		
	}
	
	return false;
}

bool gb_raise_KeyEvent(gControl *sender,long type)
{
	CWIDGET *_ob=GetObject(sender);
	
	if (!_ob) return false;
	
	switch(type)
	{
		case gEvent_KeyPress:
			return GB.Raise((void*)_ob,EVENT_KeyPress,0);
		case gEvent_KeyRelease:
			GB.Raise((void*)_ob,EVENT_KeyRelease,0);
			return false;
	}
        return false;
}

void gb_raise_EnterLeave(gControl *sender,long type)
{
	CWIDGET *_ob=GetObject(sender);
	
	if (!_ob) return;
	
	switch(type)
	{
		case gEvent_Enter:
			GB.Raise((void*)_ob,EVENT_Enter,0); break;
		case gEvent_Leave:
			GB.Raise((void*)_ob,EVENT_Leave,0); break;
	}
}

void gb_raise_FocusEvent(gControl *sender,long type)
{
	CWIDGET *_ob=GetObject(sender);
	
	if (!_ob) return;
	switch(type)
	{
		case gEvent_FocusIn:
			GB.Raise((void*)_ob,EVENT_GotFocus,0);
			break;
		case gEvent_FocusOut:
			GB.Raise((void*)_ob,EVENT_LostFocus,0);
			break;
	}
}


bool gb_raise_Drag(gControl *sender)
{
	CWIDGET *_ob=GetObject(sender);
	
	if (!_ob) return false;
	return GB.Raise((void*)_ob,EVENT_Drag,0);
}

bool gb_raise_DragMove(gControl *sender)
{
	CWIDGET *_ob=GetObject(sender);
	
	if (!_ob) return false;
	return GB.Raise((void*)_ob,EVENT_DragMove,0);
}

void gb_raise_Drop(gControl *sender)
{
	CWIDGET *_ob=GetObject(sender);
	
	if (!_ob) return;
	GB.Raise((void*)_ob,EVENT_Drop,0);
}

void DeleteControl(gControl *control)
{
	CWIDGET *widget = (CWIDGET*)control->hFree;
	
	if (widget) 
	{
		GB.StoreVariant(NULL, (void *)&widget->tag);
		CACTION_register(widget, NULL);
	}
	
	WINDOW_kill((CWINDOW*)widget);
	
	if (widget) 
	{ 
		widget->widget=NULL;
		GB.Unref(POINTER(&widget));
	}	
}


void InitControl(gControl *control, CWIDGET *widget)
{
	char *name;
	
	GB.Ref((void*)widget);
	
	widget->widget = control;
	control->hFree=(void*)widget;
	
	name = GB.GetLastEventName();
	if (!name)
		name = GB.GetClassName((void *)widget);
		
	control->setName(name);
	
	control->onFinish=DeleteControl;
	control->onMouseEvent=gb_raise_MouseEvent;
	control->onKeyEvent=gb_raise_KeyEvent;
	control->onFocusEvent=gb_raise_FocusEvent;
	control->onDrag=gb_raise_Drag;
	control->onDragMove=gb_raise_DragMove;
	control->onDrop=gb_raise_Drop;
	control->onEnterLeave=gb_raise_EnterLeave;
	
	if (control->isContainer())
		((gContainer *)control)->onArrange = CCONTAINER_cb_arrange;
	
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

int CWIDGET_check(void *_object)
{
	if (!CONTROL) return true;
  	return false;
}

/*************************************************************************************

Embedder

**************************************************************************************/

BEGIN_METHOD(CPLUGIN_new, GB_OBJECT parent)

	InitControl(new gPlugin(CONTAINER(VARG(parent))), (CWIDGET*)_object);
	((gPlugin*)((CPLUGIN*)_object)->widget)->onPlug=gb_plug_raise_plugged;
	((gPlugin*)((CPLUGIN*)_object)->widget)->onUnplug=gb_plug_raise_unplugged;

END_METHOD

BEGIN_PROPERTY(CPLUGIN_client)

	GB.ReturnInteger( ((gPlugin*)((CPLUGIN*)_object)->widget)->client() );

END_PROPERTY

BEGIN_METHOD(CPLUGIN_embed, GB_INTEGER id; GB_BOOLEAN prepared)

	((gPlugin*)((CPLUGIN*)_object)->widget)->plug(VARG(id), VARGOPT(prepared, TRUE));

END_METHOD

BEGIN_METHOD_VOID(CPLUGIN_discard)

	((gPlugin*)((CPLUGIN*)_object)->widget)->discard();

END_METHOD

// BEGIN_PROPERTY(CPLUGIN_border)
// 
//   	if (READ_PROPERTY) 
// 	{
//   		GB.ReturnInteger(((gPlugin*)((CPLUGIN*)_object)->widget)->getBorder());
// 		return;
//    	}
// 	
// 	((gPlugin*)((CPLUGIN*)_object)->widget)->setBorder(VPROP(GB_INTEGER));
// 
// END_PROPERTY


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

  CFONT *font;

  if (READ_PROPERTY)
  {
    GB.ReturnObject(CFONT_create(CONTROL->font(), 0, THIS));
  }
  else
  {
    font = (CFONT *)VPROP(GB_OBJECT);
    CONTROL->setFont(font ? font->font : 0);
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

	if (READ_PROPERTY) { GB.ReturnBoolean(CONTROL->enabled()); return; }
	CONTROL->setEnabled(VPROP(GB_BOOLEAN));

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

	CONTROL->move(VARG(x),VARG(y));
	if (MISSING(w)) return;
	if (MISSING(h))
	{
		CONTROL->resize(VARG(w),CONTROL->height());
		return;
	}
	CONTROL->resize(VARG(w),VARG(h));

END_METHOD


BEGIN_METHOD(CWIDGET_resize, GB_INTEGER w; GB_INTEGER h)

	CONTROL->resize(VARG(w),VARG(h));

END_METHOD

BEGIN_METHOD(CWIDGET_moveScaled, GB_FLOAT x; GB_FLOAT y; GB_FLOAT w; GB_FLOAT h)

	CONTROL->move((long)(VARG(x)*MAIN_scale),(long)(VARG(y)*MAIN_scale));
	if (MISSING(w)) return;
	if (MISSING(h))
	{
		CONTROL->resize((long)(VARG(w)*MAIN_scale),CONTROL->height());
		return;
	}
	CONTROL->resize((long)(VARG(w)*MAIN_scale),(long)(VARG(h)*MAIN_scale));

END_METHOD


BEGIN_METHOD(CWIDGET_resizeScaled, GB_FLOAT w; GB_FLOAT h)

	CONTROL->resize((long)(VARG(w)*MAIN_scale),(long)(VARG(h)*MAIN_scale));

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

	CCONTAINER *parent=(CCONTAINER*)VARG(parent);
	int x, y;
	
	if (GB.CheckObject(parent))
		return;
	
	x = CONTROL->left();
	y = CONTROL->top();
	
	if (!MISSING(x) && !MISSING(y))
	{
    x = VARG(x);
    y = VARG(y);
	}
	
	//if (!CONTROL->parent()) { GB.Error("Unable to reparent a top level window"); return; }
	
	CONTROL->reparent((gContainer*)parent->widget, x, y);

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


BEGIN_METHOD(CWIDGET_refresh, GB_INTEGER x; GB_INTEGER y; GB_INTEGER w; GB_INTEGER h)

	int x=-1,y=-1,w=-1,h=-1;
	
	if (!MISSING(x) && !MISSING(y))
	{
		x=VARG(x);
		y=VARG(y);
		if (MISSING(w)) w=CONTROL->width();
		else			w=VARG(w);
		if (MISSING(h)) h=CONTROL->height();
		else			h=VARG(h);

	}
	
	CONTROL->refresh(x,y,w,h);


END_METHOD


BEGIN_METHOD_VOID(CWIDGET_set_focus)

	CONTROL->setFocus();

END_METHOD


BEGIN_PROPERTY(CWIDGET_tag)

	if (READ_PROPERTY) { GB.ReturnPtr(GB_T_VARIANT, &THIS->tag); return; }
 
    GB.StoreVariant(PROP(GB_VARIANT), (void *)&THIS->tag);
  

END_METHOD


BEGIN_PROPERTY(CWIDGET_mouse)

	if (READ_PROPERTY) { GB.ReturnInteger(CONTROL->mouse()); return; }
	CONTROL->setMouse(VPROP(GB_INTEGER));

END_METHOD


BEGIN_PROPERTY(CWIDGET_cursor)

	CCURSOR *Cur;
	gCursor *cur;
	
	if (READ_PROPERTY)
	{
		cur=CONTROL->cursor();
		if (!cur) return;
		GB.New(POINTER(&Cur), GB.FindClass("Cursor"), 0, 0);
		Cur->cur=cur;
		GB.ReturnObject((void*)Cur);
		return;
	}
	
	Cur=(CCURSOR*)VPROP(GB_OBJECT);
	if (!Cur) CONTROL->setCursor(NULL);
	else CONTROL->setCursor(Cur->cur);
	
 
END_PROPERTY


BEGIN_PROPERTY(CWIDGET_background)

	if (READ_PROPERTY)
		GB.ReturnInteger(CONTROL->background());
	else
		CONTROL->setBackground(VPROP(GB_INTEGER));

END_PROPERTY


BEGIN_PROPERTY(CWIDGET_foreground)

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
		GB.ReturnNewString(CONTROL->toolTip(),0);
		return;
	}
	
	CONTROL->setToolTip(GB.ToZeroString(PROP(GB_STRING)));

END_PROPERTY


BEGIN_METHOD_VOID(CWIDGET_grab)

	CPICTURE *img;

	GB.New(POINTER(&img), GB.FindClass("Picture"), 0, 0);
	if (img->picture) delete img->picture;
	img->picture=CONTROL->grab();
	GB.ReturnObject((void*)img);

END_METHOD


BEGIN_METHOD(CWIDGET_drag, GB_VARIANT data; GB_STRING format)

	CDRAG_drag(THIS, &VARG(data), MISSING(format) ? NULL : GB.ToZeroString(ARG(format)));
	
END_METHOD


BEGIN_PROPERTY(CWIDGET_drop)

	if (READ_PROPERTY)
		GB.ReturnBoolean(CONTROL->acceptDrops());
	else
		CONTROL->setAcceptDrops(VPROP(GB_BOOLEAN));

END_PROPERTY


BEGIN_PROPERTY(CCONTROL_name)

	if (READ_PROPERTY)
		GB.ReturnNewZeroString(CONTROL->name());
	else
		CONTROL->setName(GB.ToZeroString(PROP(GB_STRING)));

END_PROPERTY


BEGIN_PROPERTY(CCONTROL_action)

	if (READ_PROPERTY)
		CACTION_get(THIS);
	else
		CACTION_register(THIS, GB.ToZeroString(PROP(GB_STRING)));

END_PROPERTY


GB_DESC CWidgetDesc[] =
{
  GB_DECLARE("Control", sizeof(CWIDGET)),

  GB_NOT_CREATABLE(),
  GB_HOOK_CHECK(CWIDGET_check),

  //GB_METHOD("_free", NULL, CWIDGET_delete, NULL),

  GB_METHOD("Move", NULL, CWIDGET_move, "(X)i(Y)i[(Width)i(Height)i]"),
  GB_METHOD("Resize", NULL, CWIDGET_resize, "(Width)i(Height)i"),
  GB_METHOD("MoveScaled", NULL, CWIDGET_moveScaled, "(X)f(Y)f[(Width)f(Height)f]"),
  GB_METHOD("ResizeScaled", NULL, CWIDGET_resizeScaled, "(Width)f(Height)f"),
  GB_METHOD("Delete", NULL, CWIDGET_delete, NULL),
  GB_METHOD("Show", NULL, CWIDGET_show, NULL),
  GB_METHOD("Hide", NULL, CWIDGET_hide, NULL),
  GB_METHOD("Reparent",NULL,CWIDGET_reparent,"(Parent)Container;[(X)i(Y)i]"),
  
  GB_METHOD("Raise", NULL, CWIDGET_raise, NULL),
  GB_METHOD("Lower", NULL, CWIDGET_lower, NULL),

  GB_PROPERTY("Next", "Control", CWIDGET_next),
  GB_PROPERTY("Previous", "Control", CWIDGET_previous),
  
  GB_METHOD("SetFocus", NULL, CWIDGET_set_focus, NULL),
  GB_METHOD("Refresh", NULL, CWIDGET_refresh, "[(X)i(Y)i(Width)i(Height)i]"),
  GB_METHOD("Grab", "Picture", CWIDGET_grab, NULL),
  GB_METHOD("Drag", NULL, CWIDGET_drag, "(Data)v[(Format)s]"),

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
  GB_PROPERTY("Expand", "b", CWIDGET_expand),
  GB_PROPERTY("Ignore", "b", CWIDGET_ignore),

  GB_PROPERTY("Font", "Font", CCONTROL_font),
  GB_PROPERTY("Background", "i", CWIDGET_background),
  GB_PROPERTY("BackColor", "i", CWIDGET_background),
  GB_PROPERTY("Foreground", "i", CWIDGET_foreground),
  GB_PROPERTY("ForeColor", "i", CWIDGET_foreground),

  GB_PROPERTY("Design", "b", CWIDGET_design),
  GB_PROPERTY("Name", "s", CCONTROL_name),
  GB_PROPERTY("Tag", "v", CWIDGET_tag),
  GB_PROPERTY("Mouse", "i", CWIDGET_mouse), 
  GB_PROPERTY("Cursor", "Cursor", CWIDGET_cursor),
  GB_PROPERTY("ToolTip", "s", CWIDGET_tooltip),
  GB_PROPERTY("Drop", "b", CWIDGET_drop),
  GB_PROPERTY("Action", "s", CCONTROL_action),

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

	CONTROL_DESCRIPTION,

  GB_END_DECLARE
};

GB_DESC CPluginDesc[] =
{
  GB_DECLARE("Embedder", sizeof(CPLUGIN)), GB_INHERITS("Control"),

  GB_METHOD("_new", NULL, CPLUGIN_new, "(Parent)Container;"),
  GB_METHOD("Embed", NULL, CPLUGIN_embed, "(Client)i[(Prepared)b]"),
  GB_METHOD("Discard", NULL, CPLUGIN_discard, NULL),
  
  GB_PROPERTY_READ("Client", "i", CPLUGIN_client),
  //GB_PROPERTY("Border", "i<Border>", CPLUGIN_border),
  
  GB_EVENT("Embed", NULL, NULL, &EVENT_Plugged),
  GB_EVENT("Close", NULL, NULL, &EVENT_Unplugged),
  GB_EVENT("Error", NULL, NULL, &EVENT_PlugError), /* TODO */
  
  EMBEDDER_DESCRIPTION,
  
  GB_END_DECLARE
};


