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




static GB_CLASS CLASS_Window;
static GB_CLASS CLASS_Picture;
static GB_CLASS CLASS_DrawingArea;

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


BEGIN_METHOD(CPAINT_arrow,GB_INTEGER X; GB_INTEGER Y; GB_INTEGER Width; GB_INTEGER Height; GB_INTEGER Type; GB_BOOLEAN Filled;)

	GdkRectangle *rect;
	gboolean filled=FALSE;
	GtkArrowType type;

	switch (VARG(Type))
	{
		case 0: type=GTK_ARROW_UP; break;
		case 1: type=GTK_ARROW_DOWN; break;
		case 2: type=GTK_ARROW_LEFT; break;
		case 3: type=GTK_ARROW_RIGHT; break;
		default: { GB.Error("Unknown arrow type"); return; }
	}

	if (!MISSING(Filled)) filled=VARG(Filled);

	CHECK_CURRENT_DEVICE();
	set_clip_rect(&rect);

	gtk_paint_arrow(STYLE,
	              DRAWABLE,
	              (GtkStateType)GTK.Draw.GetState(DR()),
                      (GtkShadowType)GTK.Draw.GetShadow(DR()),
	              rect,NULL,NULL,type,filled,VARG(X),VARG(Y),VARG(Width),VARG(Height));

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

BEGIN_METHOD(CPAINT_boxgap,GB_INTEGER X; GB_INTEGER Y; GB_INTEGER W; GB_INTEGER H; GB_INTEGER GapSide; GB_INTEGER GapX; GB_INTEGER GapW;)

	GdkRectangle *rect;
	GtkPositionType side;

	switch (VARG(GapSide))
	{
		case 0: side=GTK_POS_TOP; break;
		case 1: side=GTK_POS_BOTTOM; break;
		case 2: side=GTK_POS_LEFT; break;
		case 3: side=GTK_POS_RIGHT; break;
		default: GB.Error("Unknown gap side"); return;
	}

	CHECK_CURRENT_DEVICE();
	set_clip_rect(&rect);

	gtk_paint_box_gap(STYLE,
	              DRAWABLE,
	              (GtkStateType)GTK.Draw.GetState(DR()),
                      (GtkShadowType)GTK.Draw.GetShadow(DR()),
	              rect,NULL,NULL,VARG(X),VARG(Y),VARG(W),VARG(H),side,VARG(GapX),VARG(GapW));

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

BEGIN_METHOD(CPAINT_extension,GB_INTEGER X; GB_INTEGER Y; GB_INTEGER Width; GB_INTEGER Height; GB_INTEGER Type;)

	GdkRectangle *rect;
	GtkPositionType type;

	switch(VARG(Type))
	{
		case 0: type=GTK_POS_TOP; break;
		case 1: type=GTK_POS_BOTTOM; break;
		case 2: type=GTK_POS_LEFT; break;
		case 3: type=GTK_POS_RIGHT; break;
		default: GB.Error("Unknown position type"); return;
	}

	CHECK_CURRENT_DEVICE();
	set_clip_rect(&rect);

	gtk_paint_extension(STYLE,
	                  DRAWABLE,
	                  (GtkStateType)GTK.Draw.GetState(DR()),
                          (GtkShadowType)GTK.Draw.GetShadow(DR()),
                	  rect,NULL,NULL,VARG(X),VARG(Y),VARG(Width),VARG(Height),type);


END_METHOD

BEGIN_METHOD(CPAINT_expander,GB_INTEGER X; GB_INTEGER Y; GB_INTEGER Style;)

	GdkRectangle *rect;
	GtkExpanderStyle style;

	switch(VARG(Style))
	{
		case GTK_EXPANDER_COLLAPSED: 
		case GTK_EXPANDER_SEMI_COLLAPSED:
		case GTK_EXPANDER_SEMI_EXPANDED: 
		case GTK_EXPANDER_EXPANDED: style=(GtkExpanderStyle)VARG(Style); break;
		default: GB.Error("Unknown style"); return;
	}

	CHECK_CURRENT_DEVICE();
	set_clip_rect(&rect);

	gtk_paint_expander(STYLE,
	                  DRAWABLE,
	                  (GtkStateType)GTK.Draw.GetState(DR()),
                      rect,NULL,NULL,VARG(X),VARG(Y),style);


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

BEGIN_METHOD(CPAINT_polygon,GB_OBJECT Points;GB_BOOLEAN Filled)

	GdkRectangle *rect;
	GdkPoint *points;
	GB_ARRAY arr=VARG(Points);
	long arrcount,bc,ct=0;
	int *vl;
	gboolean fill=FALSE;

	if (!arr) return;
	arrcount=GB.Array.Count(arr);
	if ( (arrcount % 2) != 0) arrcount--;	
	if (!arrcount) return;
	if (!MISSING(Filled)) fill=VARG(Filled);
	
	GB.Alloc(POINTER(&points),sizeof(GdkPoint)*(arrcount/2) );
	

	for (bc=0; bc<arrcount; bc+=2)
	{
		vl=(int*)GB.Array.Get(arr,bc);
		points[ct].x=*vl;
		vl=(int*)GB.Array.Get(arr,bc+1);
		points[ct].y=(*vl);
		ct++;	
		
	}
	
	CHECK_CURRENT_DEVICE();
	set_clip_rect(&rect);

	gtk_paint_polygon(STYLE,
	                 DRAWABLE,
	                 (GtkStateType)GTK.Draw.GetState(DR()),
                         (GtkShadowType)GTK.Draw.GetShadow(DR()),
                	 rect,NULL,NULL,points,ct,fill);

	GB.Free(POINTER(&points));
	

END_METHOD

BEGIN_METHOD(CPAINT_resizegrip,GB_INTEGER X; GB_INTEGER Y; GB_INTEGER W; GB_INTEGER H; GB_INTEGER Edge)

	GdkRectangle *rect;
	GdkWindowEdge edge;

	switch (VARG(Edge))
	{
		case GDK_WINDOW_EDGE_NORTH_WEST:
		case GDK_WINDOW_EDGE_NORTH:
		case GDK_WINDOW_EDGE_NORTH_EAST:
        case GDK_WINDOW_EDGE_WEST:
        case GDK_WINDOW_EDGE_EAST:
        case GDK_WINDOW_EDGE_SOUTH_WEST:
        case GDK_WINDOW_EDGE_SOUTH:
        case GDK_WINDOW_EDGE_SOUTH_EAST: edge=(GdkWindowEdge)VARG(Edge); break;
		default: GB.Error("Unknown edge"); return;
	}

	CHECK_CURRENT_DEVICE();
	set_clip_rect(&rect);

	gtk_paint_resize_grip(STYLE,
	                      DRAWABLE,
	                      (GtkStateType)GTK.Draw.GetState(DR()),
                          rect,NULL,NULL,edge,VARG(X),VARG(Y),VARG(W),VARG(H));


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

BEGIN_METHOD(CPAINT_shadowgap,GB_INTEGER X; GB_INTEGER Y; GB_INTEGER W; GB_INTEGER H; GB_INTEGER GapSide; GB_INTEGER GapX; GB_INTEGER GapW)

	GdkRectangle *rect;
	GtkPositionType side;

	switch (VARG(GapSide))
	{
		case 0: side=GTK_POS_TOP; break;
		case 1: side=GTK_POS_BOTTOM; break;
		case 2: side=GTK_POS_LEFT; break;
		case 3: side=GTK_POS_RIGHT; break;
		default: GB.Error("Unknown gap side"); return;
	}

	CHECK_CURRENT_DEVICE();
	set_clip_rect(&rect);

	gtk_paint_shadow_gap(STYLE,
	                 DRAWABLE,
	                 (GtkStateType)GTK.Draw.GetState(DR()),
                         (GtkShadowType)GTK.Draw.GetShadow(DR()),
                         rect,NULL,NULL,VARG(X),VARG(Y),VARG(W),VARG(H),side,VARG(GapX),VARG(GapW));


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

GB_DESC CPaintDesc[] =
{
  GB_DECLARE(".DrawTheme", 0), GB_VIRTUAL_CLASS(),

  GB_CONSTANT("Normal","i",GTK_STATE_NORMAL),
  GB_CONSTANT("Active","i",GTK_STATE_ACTIVE),
  GB_CONSTANT("Prelight","i",GTK_STATE_PRELIGHT),
  GB_CONSTANT("Selected","i",GTK_STATE_SELECTED),
  GB_CONSTANT("Insensitive","i",GTK_STATE_INSENSITIVE),
  GB_CONSTANT("None","i",GTK_SHADOW_NONE),
  GB_CONSTANT("In","i",GTK_SHADOW_IN),
  GB_CONSTANT("Out","i",GTK_SHADOW_OUT),
  GB_CONSTANT("EtchedIn","i",GTK_SHADOW_ETCHED_IN),
  GB_CONSTANT("EtchedOut","i",GTK_SHADOW_ETCHED_OUT),
  GB_CONSTANT("NorthWest","i",GDK_WINDOW_EDGE_NORTH_WEST),
  GB_CONSTANT("North","i",GDK_WINDOW_EDGE_NORTH),
  GB_CONSTANT("NorthEast","i",GDK_WINDOW_EDGE_NORTH_EAST),
  GB_CONSTANT("West","i",GDK_WINDOW_EDGE_WEST),
  GB_CONSTANT("East","i",GDK_WINDOW_EDGE_EAST),
  GB_CONSTANT("SouthWest","i",GDK_WINDOW_EDGE_SOUTH_WEST),
  GB_CONSTANT("South","i",GDK_WINDOW_EDGE_SOUTH),
  GB_CONSTANT("SouthEast","i",GDK_WINDOW_EDGE_SOUTH_EAST),
  GB_CONSTANT("Up","i",0),
  GB_CONSTANT("Down","i",1),
  GB_CONSTANT("Left","i",2),
  GB_CONSTANT("Right","i",3),
  GB_CONSTANT("Collapsed","i",GTK_EXPANDER_COLLAPSED),
  GB_CONSTANT("Semicollapsed","i",GTK_EXPANDER_SEMI_COLLAPSED),
  GB_CONSTANT("Semiexpanded","i",GTK_EXPANDER_SEMI_EXPANDED),
  GB_CONSTANT("Expanded","i",GTK_EXPANDER_EXPANDED),
  GB_CONSTANT("Horizontal","i",GTK_ORIENTATION_HORIZONTAL),
  GB_CONSTANT("Vertical","i",GTK_ORIENTATION_VERTICAL),

  GB_STATIC_METHOD("_init",0,CPAINT_init,NULL),

  GB_STATIC_PROPERTY("State","i",CPAINT_state),

  GB_STATIC_PROPERTY("ShadowType","i",CPAINT_shadowtype),

  GB_STATIC_METHOD("Arrow",0,CPAINT_arrow,"(X)i(Y)i(Width)i(Height)i(Type)i[(Filled)b]"),
  GB_STATIC_METHOD("Box",0,CPAINT_box,"(X)i(Y)i(Width)i(Height)i"),

  GB_STATIC_METHOD("BoxGap",0,CPAINT_boxgap,"(X)i(Y)i(Width)i(Height)i(GapSide)i(GapX)i(GapWidth)i"),
  GB_STATIC_METHOD("Check",0,CPAINT_check,"(X)i(Y)i(Width)i(Height)i"),
  GB_STATIC_METHOD("Diamond",0,CPAINT_diamond,"(X)i(Y)i(Width)i(Height)i"),
  GB_STATIC_METHOD("Expander",0,CPAINT_expander,"(X)i(Y)i(Style)i"),
  GB_STATIC_METHOD("Extension",0,CPAINT_extension,"(X)i(Y)i(Width)i(Height)i(Position)i(Type)i"),
  GB_STATIC_METHOD("FlatBox",0,CPAINT_flatbox,"(X)i(Y)i(Width)i(Height)i"),
  GB_STATIC_METHOD("Focus",0,CPAINT_focus,"(X)i(Y)i(Width)i(Height)i"),
  GB_STATIC_METHOD("Handle",0,CPAINT_handle,"(X)i(Y)i(Width)i(Height)i[(Vertical)b]"),
  GB_STATIC_METHOD("HLine",0,CPAINT_hline,"(X1)i(X2)i(Y)i"),
  GB_STATIC_METHOD("Option",0,CPAINT_option,"(X)i(Y)i(Width)i(Height)i"),
  GB_STATIC_METHOD("Polygon",0,CPAINT_polygon,"(Points)Integer[];[(Filled)b]"),
  GB_STATIC_METHOD("ResizeGrip",0,CPAINT_resizegrip,"(X)i(Y)i(Width)i(Height)i(Edge)i"),
  GB_STATIC_METHOD("Shadow",0,CPAINT_shadow,"(X)i(Y)i(Width)i(Height)i"),
  GB_STATIC_METHOD("ShadowGap",0,CPAINT_shadowgap,"(X)i(Y)i(Width)i(Height)i(GapSide)i(GapX)i(GapWidth)i"),
  GB_STATIC_METHOD("Slider",0,CPAINT_slider,"(X)i(Y)i(Width)i(Height)i[(Vertical)b]"),
  GB_STATIC_METHOD("Tab",0,CPAINT_tab,"(X)i(Y)i(Width)i(Height)i"),
  GB_STATIC_METHOD("VLine",0,CPAINT_vline,"(X)i(Y1)i(Y2)i"),
    
  GB_END_DECLARE
};

GB_DESC CDrawDesc[] =
{
  GB_DECLARE("Draw", 0), GB_VIRTUAL_CLASS(),

  GB_STATIC_PROPERTY_SELF("Theme",".DrawTheme"),

  GB_END_DECLARE
};
