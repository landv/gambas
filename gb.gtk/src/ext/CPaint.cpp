/***************************************************************************

  CPaint.cpp

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


#define __CPAINT_CPP

#include "main.h"
#include "CPaint.h"

#include "../CWidget.h"
#include "../gdraw.h"

#include <gtk/gtk.h>




static void *CLASS_Window;
static void *CLASS_Picture;
static void *CLASS_DrawingArea;

static GB_DRAW *DRIVER;
static GdkDrawable *DRAWABLE;
static GtkStyle *STYLE;

typedef	struct 
{
	gDraw *dr;
}
GB_DRAW_EXTRA;

#define EXTRA() ((GB_DRAW_EXTRA *)(&(DRIVER->extra)))
#define DR() (EXTRA()->dr)

static GdkRectangle genrect;


#define CHECK_CURRENT_DEVICE() { DRIVER=DRAW.GetCurrent(); \
                                 if (!DRIVER) { GB.Error("No current device"); return; } \
                                 DRAWABLE=(GdkDrawable*)GTK.Draw.GetDrawable(DR()); \
                                 STYLE=(GtkStyle*)GTK.Draw.GetStyle(DR()); }


void set_clip_rect(GdkRectangle **rect)
{
	if (!DRIVER) { *rect=NULL; return; }
	if (!DRIVER->desc->Clip.IsEnabled(DRIVER)) { *rect=NULL; return; }

	DRIVER->desc->Clip.Get(DRIVER,&genrect.x,&genrect.y,&genrect.width,&genrect.height);
	
	*rect=&genrect;

}


BEGIN_METHOD_VOID(CPAINT_init)

	CLASS_Window = GB.FindClass("Window");
	CLASS_Picture = GB.FindClass("Picture");
	CLASS_DrawingArea = GB.FindClass("DrawingArea");

	DRIVER=NULL;
	

END_METHOD


BEGIN_METHOD(CPAINT_box,GB_INTEGER X; GB_INTEGER Y; GB_INTEGER Width; GB_INTEGER Height)

	GdkRectangle *rect;

	CHECK_CURRENT_DEVICE();
	set_clip_rect(&rect);

	gtk_paint_box(STYLE,
	              DRAWABLE,
	              (GtkStateType)GTK.Draw.GetState(DR()),
                      (GtkShadowType)GTK.Draw.GetShadow(DR()),
	              rect,NULL,NULL,VARG(X),VARG(Y),VARG(Width),VARG(Height));

END_METHOD

BEGIN_METHOD(CPAINT_check,GB_INTEGER X; GB_INTEGER Y; GB_INTEGER Width; GB_INTEGER Height)

	GdkRectangle *rect;

	CHECK_CURRENT_DEVICE();
	set_clip_rect(&rect);

	gtk_paint_check(STYLE,
	                DRAWABLE,
	                (GtkStateType)GTK.Draw.GetState(DR()),
                        (GtkShadowType)GTK.Draw.GetShadow(DR()),
                	rect,NULL,NULL,VARG(X),VARG(Y),VARG(Width),VARG(Height));


END_METHOD

BEGIN_METHOD(CPAINT_diamond,GB_INTEGER X; GB_INTEGER Y; GB_INTEGER Width; GB_INTEGER Height)

	GdkRectangle *rect;

	CHECK_CURRENT_DEVICE();
	set_clip_rect(&rect);

	gtk_paint_diamond(STYLE,
	                  DRAWABLE,
	                  (GtkStateType)GTK.Draw.GetState(DR()),
                          (GtkShadowType)GTK.Draw.GetShadow(DR()),
                	  rect,NULL,NULL,VARG(X),VARG(Y),VARG(Width),VARG(Height));


END_METHOD

BEGIN_METHOD(CPAINT_flatbox,GB_INTEGER X; GB_INTEGER Y; GB_INTEGER Width; GB_INTEGER Height)

	GdkRectangle *rect;

	CHECK_CURRENT_DEVICE();
	set_clip_rect(&rect);

	gtk_paint_flat_box(STYLE,
	                   DRAWABLE,
	                   (GtkStateType)GTK.Draw.GetState(DR()),
                           (GtkShadowType)GTK.Draw.GetShadow(DR()),
	                   rect,NULL,NULL,VARG(X),VARG(Y),VARG(Width),VARG(Height));


END_METHOD

BEGIN_METHOD(CPAINT_focus,GB_INTEGER X; GB_INTEGER Y; GB_INTEGER Width; GB_INTEGER Height)

	GdkRectangle *rect;

	CHECK_CURRENT_DEVICE();
	set_clip_rect(&rect);

	gtk_paint_focus(STYLE,
	                DRAWABLE,
	                (GtkStateType)GTK.Draw.GetState(DR()),
                        rect,NULL,NULL,VARG(X),VARG(Y),VARG(Width),VARG(Height));


END_METHOD

BEGIN_METHOD(CPAINT_handle,GB_INTEGER X; GB_INTEGER Y; GB_INTEGER Width; GB_INTEGER Height; GB_BOOLEAN Vertical;)

	GdkRectangle *rect;
	int orientation=GTK_ORIENTATION_HORIZONTAL;

	CHECK_CURRENT_DEVICE();
	set_clip_rect(&rect);

	if (!MISSING(Vertical))
		if (VARG(Vertical)) orientation=GTK_ORIENTATION_VERTICAL;

	gtk_paint_handle(STYLE,
	                 DRAWABLE,
	                 (GtkStateType)GTK.Draw.GetState(DR()),
                         (GtkShadowType)GTK.Draw.GetShadow(DR()),
                	 rect,NULL,NULL,VARG(X),VARG(Y),VARG(Width),VARG(Height),
                         (GtkOrientation)orientation);


END_METHOD

BEGIN_METHOD(CPAINT_hline,GB_INTEGER X1; GB_INTEGER X2; GB_INTEGER Y;)

	GdkRectangle *rect;

	CHECK_CURRENT_DEVICE();
	set_clip_rect(&rect);

	gtk_paint_hline(STYLE,
	                DRAWABLE,
	                (GtkStateType)GTK.Draw.GetState(DR()),
                        rect,NULL,NULL,VARG(X1),VARG(X2),VARG(Y));


END_METHOD

BEGIN_METHOD(CPAINT_option,GB_INTEGER X; GB_INTEGER Y; GB_INTEGER Width; GB_INTEGER Height)

	GdkRectangle *rect;

	CHECK_CURRENT_DEVICE();
	set_clip_rect(&rect);

	gtk_paint_option(STYLE,
	                 DRAWABLE,
	                 (GtkStateType)GTK.Draw.GetState(DR()),
                         (GtkShadowType)GTK.Draw.GetShadow(DR()),
                	 rect,NULL,NULL,VARG(X),VARG(Y),VARG(Width),VARG(Height));


END_METHOD

BEGIN_METHOD(CPAINT_shadow,GB_INTEGER X; GB_INTEGER Y; GB_INTEGER Width; GB_INTEGER Height)

	GdkRectangle *rect;

	CHECK_CURRENT_DEVICE();
	set_clip_rect(&rect);

	gtk_paint_shadow(STYLE,
	                 DRAWABLE,
	                 (GtkStateType)GTK.Draw.GetState(DR()),
                         (GtkShadowType)GTK.Draw.GetShadow(DR()),
                         rect,NULL,NULL,VARG(X),VARG(Y),VARG(Width),VARG(Height));


END_METHOD

BEGIN_METHOD(CPAINT_slider,GB_INTEGER X; GB_INTEGER Y; GB_INTEGER Width; GB_INTEGER Height; GB_BOOLEAN Vertical;)

	GdkRectangle *rect;
	int orientation=GTK_ORIENTATION_HORIZONTAL;

	CHECK_CURRENT_DEVICE();
	set_clip_rect(&rect);

	if (!MISSING(Vertical))
		if (VARG(Vertical)) orientation=GTK_ORIENTATION_VERTICAL;

	gtk_paint_slider(STYLE,
	                 DRAWABLE,
	                 (GtkStateType)GTK.Draw.GetState(DR()),
                         (GtkShadowType)GTK.Draw.GetShadow(DR()),
                	 rect,NULL,NULL,VARG(X),VARG(Y),VARG(Width),VARG(Height),
                         (GtkOrientation)orientation);


END_METHOD

BEGIN_METHOD(CPAINT_tab,GB_INTEGER X; GB_INTEGER Y; GB_INTEGER Width; GB_INTEGER Height)

	GdkRectangle *rect;

	CHECK_CURRENT_DEVICE();
	set_clip_rect(&rect);

	gtk_paint_tab(STYLE,
	              DRAWABLE,
	              (GtkStateType)GTK.Draw.GetState(DR()),
                      (GtkShadowType)GTK.Draw.GetShadow(DR()),
                      rect,NULL,NULL,VARG(X),VARG(Y),VARG(Width),VARG(Height));


END_METHOD

BEGIN_METHOD(CPAINT_vline,GB_INTEGER X; GB_INTEGER Y1; GB_INTEGER Y2;)

	GdkRectangle *rect;

	CHECK_CURRENT_DEVICE();
	set_clip_rect(&rect);

	gtk_paint_vline(STYLE,
	                DRAWABLE,
	                (GtkStateType)GTK.Draw.GetState(DR()),
                        rect,NULL,NULL,VARG(Y1),VARG(Y2),VARG(X));


END_METHOD

BEGIN_PROPERTY(CPAINT_state)


	CHECK_CURRENT_DEVICE();
	
	if (READ_PROPERTY)
	{
		GB.ReturnInteger( GTK.Draw.GetState(DR()) ); return;
	}

	GTK.Draw.SetState(DR(),VPROP(GB_INTEGER));
	
	

END_PROPERTY

BEGIN_PROPERTY(CPAINT_shadowtype)

	CHECK_CURRENT_DEVICE();
	
	if (READ_PROPERTY)
	{
		GB.ReturnInteger( GTK.Draw.GetShadow(DR()) ); return;
	}

	GTK.Draw.SetShadow(DR(),VPROP(GB_INTEGER));

END_PROPERTY


/***************************************************************************

  Gambas Interfaces

***************************************************************************/
GB_DESC CStateDesc[] =
{
	GB_DECLARE("State",0), GB_VIRTUAL_CLASS(),

	GB_CONSTANT("Normal","i",GTK_STATE_NORMAL),
	GB_CONSTANT("Active","i",GTK_STATE_ACTIVE),
	GB_CONSTANT("Prelight","i",GTK_STATE_PRELIGHT),
	GB_CONSTANT("Selected","i",GTK_STATE_SELECTED),
	GB_CONSTANT("Insensitive","i",GTK_STATE_INSENSITIVE),

	GB_END_DECLARE
};

GB_DESC CShadowDesc[] =
{
	GB_DECLARE("Shadow",0), GB_VIRTUAL_CLASS(),

	GB_CONSTANT("None","i",GTK_SHADOW_NONE),
	GB_CONSTANT("In","i",GTK_SHADOW_IN),
	GB_CONSTANT("Out","i",GTK_SHADOW_OUT),
	GB_CONSTANT("EtchedIn","i",GTK_SHADOW_ETCHED_IN),
	GB_CONSTANT("EtchedOut","i",GTK_SHADOW_ETCHED_OUT),

	GB_END_DECLARE
};


GB_DESC CPaintDesc[] =
{
  GB_DECLARE(".DrawTheme", 0), GB_VIRTUAL_CLASS(),

  GB_CONSTANT("Horizontal","i",GTK_ORIENTATION_HORIZONTAL),
  GB_CONSTANT("Vertical","i",GTK_ORIENTATION_VERTICAL),

  GB_STATIC_METHOD("_init",NULL,CPAINT_init,NULL),

  GB_STATIC_PROPERTY("State","i",CPAINT_state),
  GB_STATIC_PROPERTY("ShadowType","i",CPAINT_shadowtype),

  GB_STATIC_METHOD("Box",NULL,CPAINT_box,"(X)i(Y)i(Width)i(Height)i"),
  GB_STATIC_METHOD("Check",NULL,CPAINT_check,"(X)i(Y)i(Width)i(Height)i"),
  GB_STATIC_METHOD("Diamond",NULL,CPAINT_diamond,"(X)i(Y)i(Width)i(Height)i"),
  GB_STATIC_METHOD("FlatBox",NULL,CPAINT_flatbox,"(X)i(Y)i(Width)i(Height)i"),
  GB_STATIC_METHOD("Focus",NULL,CPAINT_focus,"(X)i(Y)i(Width)i(Height)i"),
  GB_STATIC_METHOD("Handle",NULL,CPAINT_handle,"(X)i(Y)i(Width)i(Height)i[(Vertical)b]"),
  GB_STATIC_METHOD("HLine",NULL,CPAINT_hline,"(X1)i(X2)i(Y)i"),
  GB_STATIC_METHOD("Option",NULL,CPAINT_option,"(X)i(Y)i(Width)i(Height)i"),
  GB_STATIC_METHOD("Shadow",NULL,CPAINT_shadow,"(X)i(Y)i(Width)i(Height)i"),
  GB_STATIC_METHOD("Slider",NULL,CPAINT_slider,"(X)i(Y)i(Width)i(Height)i[(Vertical)b]"),
  GB_STATIC_METHOD("Tab",NULL,CPAINT_tab,"(X)i(Y)i(Width)i(Height)i"),
  GB_STATIC_METHOD("VLine",NULL,CPAINT_vline,"(X)i(Y1)i(Y2)i"),
    
  GB_END_DECLARE
};

GB_DESC CDrawDesc[] =
{
  GB_DECLARE("Draw", 0), GB_VIRTUAL_CLASS(),

  GB_STATIC_PROPERTY_SELF("Theme",".DrawTheme"),

  GB_END_DECLARE
};
