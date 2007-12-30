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

#include "gambas.h"
#include "widgets.h"
#include "CWidget.h"
#include "CWindow.h"
#include "CFont.h"
#include "CMouse.h"
#include "CPicture.h"
#include "CImage.h"
#include "CContainer.h"
#include "CFrame.h"

#include <stdlib.h>

#include <stdio.h>

extern int MAIN_scale;

static void *CLASS_Image=NULL;

DECLARE_EVENT(EVENT_Enter);
DECLARE_EVENT(EVENT_GotFocus);
DECLARE_EVENT(EVENT_LostFocus);
DECLARE_EVENT(EVENT_KeyPress);
DECLARE_EVENT(EVENT_KeyRelease);
DECLARE_EVENT(EVENT_Leave);
DECLARE_EVENT(EVENT_MouseDown);
DECLARE_EVENT(EVENT_MouseMove);
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

long allWidgetsCount=0;
void **allWidgets=NULL;

void *CLASS_UserContainer=NULL;
void *CLASS_UserControl=NULL;

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



bool gb_raise_MouseEvent(gControl *sender,long type)
{
	CWIDGET *_ob=GetObject(sender);
	
	if (!_ob) return false;
	
	switch(type)
	{
		case gEvent_MousePress:
			GB.Raise((void*)_ob,EVENT_MouseDown,0);
			break;
			
		case gEvent_MouseRelease:
			GB.Raise((void*)_ob,EVENT_MouseUp,0);
			break;
			
		case gEvent_MouseMove:
			GB.Raise((void*)_ob,EVENT_MouseMove,0);
			break;
			
		case gEvent_MouseWheel:
			GB.Raise((void*)_ob,EVENT_MouseWheel,0);
			break;
			
		case gEvent_MouseMenu:
			GB.Raise((void*)_ob,EVENT_Menu,0);
			break;
			
		case gEvent_MouseDblClick:
			GB.Raise((void*)_ob,EVENT_DblClick,0);
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


void gb_raise_Drag(gControl *sender)
{
	CWIDGET *_ob=GetObject(sender);
	
	if (!_ob) return;
	GB.Raise((void*)_ob,EVENT_Drag,0);
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
	CWIDGET *widget=NULL;
	long bucle,b2;
	
	widget=(CWIDGET*)control->hFree;
	
	if (widget) GB.StoreVariant(NULL, (void *)&widget->tag);
	
	for(bucle=0;bucle<allWidgetsCount;bucle++)
		if (allWidgets[bucle]==control)
		{
			
			for (b2=bucle;b2<(allWidgetsCount-1);b2++)
				allWidgets[b2]=allWidgets[b2+1];
			
			allWidgetsCount--;
			if (!allWidgetsCount)
				GB.Free((void**)&allWidgets);
			else
				GB.Realloc((void**)&allWidgets,sizeof(void*)*allWidgetsCount);
		}
	
		
	WINDOW_kill((CWINDOW*)widget);
	
	if (widget) 
	{ 
		widget->widget=NULL;
		GB.Unref((void**)&widget);
		
	}
	
}


void InitControl(gControl *control,CWIDGET *widget)
{
	GB.Ref((void*)widget);
	
	control->hFree=(void*)widget;
	
	if (!allWidgets)
		GB.Alloc((void**)&allWidgets,sizeof(void*));
	else
		GB.Realloc((void**)&allWidgets,(allWidgetsCount+1)*sizeof(void*));
	
	allWidgets[allWidgetsCount++]=control;
	
	control->onFinish=DeleteControl;
	control->onMouseEvent=gb_raise_MouseEvent;
	control->onKeyEvent=gb_raise_KeyEvent;
	control->onFocusEvent=gb_raise_FocusEvent;
	control->onDrag=gb_raise_Drag;
	control->onDragMove=gb_raise_DragMove;
	control->onDrop=gb_raise_Drop;
	control->onEnterLeave=gb_raise_EnterLeave;


}

long ChildrenCount(gControl *control)
{
	long bucle;
	long nnum=0;
	
	for (bucle=0;bucle<allWidgetsCount;bucle++)
		if ( ((gControl*)allWidgets[bucle])->pr==control) nnum++;

	
	return nnum;
}

CWIDGET *GetContainer(CWIDGET *control)
{
	if (!control) return NULL;
	
	if (!CLASS_UserContainer) CLASS_UserContainer=GB.FindClass("UserContainer");
	if (!CLASS_UserControl) CLASS_UserControl=GB.FindClass("UserControl");
	
	if ( GB.Is (control,CLASS_UserContainer) || GB.Is (control,CLASS_UserControl) )
	{
		if (  ((CUSERCONTROL*)control)->ct )
			return (CWIDGET*)((CUSERCONTROL*)control)->ct;
		else
			return control;
	}
	
	return control;
}

CWIDGET *GetObject(gControl *control)
{
	long bucle;
	
	for (bucle=0;bucle<allWidgetsCount;bucle++)
		if (allWidgets[bucle]==control) return (CWIDGET*)control->hFree;
	
	return NULL;
}

CWIDGET *GetChild(gControl *control,long index)
{
	CWIDGET *buf=NULL;
	long bucle;
	long nnum=0;
	
	for (bucle=0;bucle<allWidgetsCount;bucle++)
		if ( ((gControl*)allWidgets[bucle])->pr==control)
		{
			if (nnum==index)
			{
				buf=(CWIDGET*)((gControl*)allWidgets[bucle])->hFree;
				break;
			}
			else
				nnum++;
		}

	
	return buf;
	
	
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

	CCONTAINER *Parent=(CCONTAINER*)VPROP(GB_OBJECT);
	Parent=(CCONTAINER*)GetContainer ((CWIDGET*)Parent);

	((CPLUGIN*)_object)->widget=new gPlugin(Parent->widget);
	InitControl( ((CPLUGIN*)_object)->widget,(CWIDGET*)_object);
	((gPlugin*)((CPLUGIN*)_object)->widget)->onPlug=gb_plug_raise_plugged;
	((gPlugin*)((CPLUGIN*)_object)->widget)->onUnplug=gb_plug_raise_unplugged;

END_METHOD

BEGIN_PROPERTY(CPLUGIN_client)

	GB.ReturnInteger( ((gPlugin*)((CPLUGIN*)_object)->widget)->client() );

END_PROPERTY

BEGIN_METHOD(CPLUGIN_embed,GB_INTEGER id;GB_BOOLEAN Prepared)

	bool Prepared=true;
	
	if (!MISSING(Prepared)) Prepared=VARG(Prepared);
	
	((gPlugin*)((CPLUGIN*)_object)->widget)->plug(VARG(id),Prepared);

END_METHOD

BEGIN_METHOD_VOID(CPLUGIN_discard)

	((gPlugin*)((CPLUGIN*)_object)->widget)->discard();

END_METHOD

BEGIN_PROPERTY(CPLUGIN_border)

  	if (READ_PROPERTY) 
	{
  		GB.ReturnInteger(((gPlugin*)((CPLUGIN*)_object)->widget)->getBorder());
		return;
   	}
	
	((gPlugin*)((CPLUGIN*)_object)->widget)->setBorder(VPROP(GB_INTEGER));

END_PROPERTY


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



BEGIN_PROPERTY(CWIDGET_font)

	CFONT *Font;
	
	if (READ_PROPERTY)
	{
		GB.New((void **)&Font, GB.FindClass("Font"), 0, 0);
		delete Font->font;
		Font->font=CONTROL->font();
		GB.ReturnObject(Font);
		return;
	}
	
	Font=(CFONT*)VPROP(GB_OBJECT);
	if (Font) CONTROL->setFont(Font->font);

END_PROPERTY


BEGIN_PROPERTY(CWIDGET_design)

	if (READ_PROPERTY) { GB.ReturnBoolean(CONTROL->design()); return; }
	CONTROL->setDesign(VPROP(GB_BOOLEAN));

END_PROPERTY


BEGIN_PROPERTY(CWIDGET_visible)

	if (READ_PROPERTY) { GB.ReturnBoolean(CONTROL->visible()); return; }
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
	
	if (CONTROL) { CONTROL->destroy(); THIS->widget=NULL; }
		
END_METHOD


BEGIN_METHOD_VOID(CWIDGET_show)

	CONTROL->show();

END_METHOD


BEGIN_METHOD_VOID(CWIDGET_hide)

	CONTROL->hide();

END_METHOD

BEGIN_METHOD(CWIDGET_reparent,GB_OBJECT Parent;)

	CCONTAINER *Parent=(CCONTAINER*)VARG(Parent);
	
	if (!Parent) { GB.Error("Null parent"); return; }
	if (!Parent->widget) { GB.Error("Invalid parent"); return; }
	if (!CONTROL->parent()) { GB.Error("Unable to reparent a top level window"); return; }
	
	CONTROL->reparent((gContainer*)Parent->widget);
	

END_METHOD


BEGIN_METHOD_VOID(CWIDGET_raise)

	CONTROL->raise();

END_METHOD


BEGIN_METHOD_VOID(CWIDGET_lower)

	CONTROL->lower();

END_METHOD


BEGIN_PROPERTY(CWIDGET_next)

  
  
END_PROPERTY


BEGIN_PROPERTY(CWIDGET_previous)


  
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
		GB.New((void **)&Cur, GB.FindClass("Cursor"), 0, 0);
		Cur->cur=cur;
		GB.ReturnObject((void*)Cur);
		return;
	}
	
	Cur=(CCURSOR*)VPROP(GB_OBJECT);
	if (!Cur) CONTROL->setCursor(NULL);
	else CONTROL->setCursor(Cur->cur);
	
 
END_PROPERTY


BEGIN_PROPERTY(CWIDGET_background)

	if (READ_PROPERTY)	{ GB.ReturnInteger(CONTROL->backGround()); return; }
	CONTROL->setBackGround(VPROP(GB_INTEGER));

END_PROPERTY


BEGIN_PROPERTY(CWIDGET_foreground)

	if (READ_PROPERTY)	{ GB.ReturnInteger(CONTROL->foreGround()); return; }
	CONTROL->setForeGround(VPROP(GB_INTEGER));

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

	GB.New((void **)&img, GB.FindClass("Picture"), 0, 0);
	if (img->picture) delete img->picture;
	img->picture=CONTROL->grab();
	GB.ReturnObject((void*)img);

END_METHOD


BEGIN_METHOD(CWIDGET_drag, GB_VARIANT data; GB_STRING format)

	CPICTURE *pic;
	CIMAGE *img;
	
	if (VARG(data).type == GB_T_STRING)
	{
		CONTROL->dragText(VARG(data)._string.value);
		return;
	}
	
	if (!CLASS_Image) CLASS_Image = GB.FindClass("Image");
	
	if (VARG(data).type >= GB_T_OBJECT && GB.Is(VARG(data)._object.value, CLASS_Image))
	{
		img = (CIMAGE*)VARG(data)._object.value;
		CONTROL->dragImage(img->image);
		return;
	}

END_METHOD


BEGIN_PROPERTY(CWIDGET_drop)

	if (READ_PROPERTY) { GB.ReturnBoolean(CONTROL->acceptDrops()); return; }
	CONTROL->setAcceptDrops(VPROP(GB_BOOLEAN));

END_PROPERTY


// BM: stub for the Name property

BEGIN_PROPERTY(CCONTROL_name)

	if (READ_PROPERTY)
		GB.ReturnNull();

END_PROPERTY


GB_DESC CWidgetDesc[] =
{
  GB_DECLARE("Control", sizeof(CWIDGET)),

  GB_HOOK_NEW(NULL),
  GB_HOOK_CHECK(CWIDGET_check),

  //GB_METHOD("_free", NULL, CWIDGET_delete, NULL),

  GB_METHOD("Move", NULL, CWIDGET_move, "(X)i(Y)i[(Width)i(Height)i]"),
  GB_METHOD("Resize", NULL, CWIDGET_resize, "(Width)i(Height)i"),
  GB_METHOD("MoveScaled", NULL, CWIDGET_moveScaled, "(X)f(Y)f[(Width)f(Height)f]"),
  GB_METHOD("ResizeScaled", NULL, CWIDGET_resizeScaled, "(Width)f(Height)f"),
  GB_METHOD("Delete", NULL, CWIDGET_delete, NULL),
  GB_METHOD("Show", NULL, CWIDGET_show, NULL),
  GB_METHOD("Hide", NULL, CWIDGET_hide, NULL),
  GB_METHOD("Reparent",NULL,CWIDGET_reparent,"(Parent)Container"),
  
  GB_METHOD("Raise", NULL, CWIDGET_raise, NULL),
  GB_METHOD("Lower", NULL, CWIDGET_lower, NULL),

  GB_PROPERTY_READ("Next", "Control", CWIDGET_next),
  GB_PROPERTY_READ("Previous", "Control", CWIDGET_previous),
  
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

  GB_PROPERTY("Font", "Font", CWIDGET_font),
  GB_PROPERTY("Background", "i", CWIDGET_background),
  GB_PROPERTY("BackColor", "i", CWIDGET_background),
  GB_PROPERTY("Foreground", "i", CWIDGET_foreground),
  GB_PROPERTY("ForeColor", "i", CWIDGET_foreground),

  GB_PROPERTY("Design", "b", CWIDGET_design),
  GB_PROPERTY("Name", "s", CCONTROL_name),
  GB_PROPERTY("Tag", "v", CWIDGET_tag),
  GB_PROPERTY("Mouse", "i" MOUSE_CONSTANTS, CWIDGET_mouse), 
  GB_PROPERTY("Cursor", "Cursor", CWIDGET_cursor),
  GB_PROPERTY("ToolTip", "s", CWIDGET_tooltip),
  GB_PROPERTY("Drop", "b", CWIDGET_drop),

  GB_PROPERTY_READ("Parent", "Control", CWIDGET_parent),
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
  GB_EVENT("MouseUp", NULL, NULL, &EVENT_MouseUp),
  GB_EVENT("MouseWheel", NULL, NULL, &EVENT_MouseWheel),
  GB_EVENT("DblClick", NULL, NULL, &EVENT_DblClick),
  GB_EVENT("Menu", NULL, NULL, &EVENT_Menu),
  GB_EVENT("Drag", NULL, NULL, &EVENT_Drag),
  GB_EVENT("DragMove", NULL, NULL, &EVENT_DragMove),
  GB_EVENT("Drop", NULL, NULL, &EVENT_Drop),

  GB_CONSTANT("_DefaultEvent", "s", "MouseDown"),
  GB_CONSTANT("_Properties", "s", CWIDGET_PROPERTIES), 

  GB_END_DECLARE
};

GB_DESC CPluginDesc[] =
{
  GB_DECLARE("Embedder", sizeof(CPLUGIN)), GB_INHERITS("Control"),

  GB_METHOD("_new", NULL, CPLUGIN_new, "(Parent)Container;"),
  GB_METHOD("Embed", NULL, CPLUGIN_embed, "(Client)i[(Prepared)b]"),
  GB_METHOD("Discard", NULL, CPLUGIN_discard, NULL),
  
  GB_PROPERTY_READ("Client", "i", CPLUGIN_client),
  GB_PROPERTY("Border", "i<Border>", CPLUGIN_border),
  
  GB_EVENT("Embed", NULL, NULL, &EVENT_Plugged),
  GB_EVENT("Close", NULL, NULL, &EVENT_Unplugged),
  GB_EVENT("Error", NULL, NULL, &EVENT_PlugError), /* TODO */
  
  GB_CONSTANT("_Properties", "s", CPLUGIN_PROPERTIES),
  
  GB_END_DECLARE
};


