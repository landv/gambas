/***************************************************************************

  gdraw.cpp

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

#ifdef OS_SOLARIS
/* Make math.h define M_PI and a few other things */
#define __EXTENSIONS__
/* Get definition for finite() */
#include <ieeefp.h>
#endif
#include <math.h>

#include "widgets.h"
#include "widgets_private.h"
#include "gdrawingarea.h"
#include "gdesktop.h"
#include "gdraw.h"

#ifdef GDK_WINDOWING_X11
#ifndef GAMBAS_DIRECTFB
#define GOT_X11_PLATFORM
#endif
#endif

static const gchar _back_diagonal_bits[]={'\x01','\x02','\x04','\x08','\x10','\x20','\x40','\x80'};
static const gchar _cross_bits[]={'\x08', '\x08', '\x08', '\xff', '\x08', '\x08', '\x08', '\x08' };
static const gchar _cross_diagonal_bits[]={'\x11','\x0a','\x04','\x0a','\x11','\xa0','\x40','\xa0'};
static const gchar _dense12_bits[]={'\x11', '\x00','\x44','\x00','\x11','\x00','\x44','\x00'};
static const gchar _dense37_bits[]={'\x54','\x8a','\x51','\xaa','\x44','\xaa','\x11','\xaa'};
static const gchar _dense50_bits[]={'\x55','\xaa','\x55','\xaa','\x55','\xaa','\x55','\xaa'};
static const gchar _dense63_bits[]={'\xdd','\xaa','\x77','\xaa','\xdd','\xaa','\x77','\xaa'};
static const gchar _dense88_bits[]={'\xee','\xff','\xbb','\xff','\xee','\xff','\xbb','\xff'};
static const gchar _dense6_bits[]={'\x22', '\x00', '\x00', '\x00', '\x22','\x00','\x00','\x00'};
static const gchar _dense94_bits[]={'\xdd','\xff','\xff','\xff','\xdd','\xff','\xff','\xff' };
static const gchar _diagonal_bits[]={'\x08', '\x04', '\x02', '\x01', '\x80', '\x40', '\x20', '\x10' };
static const gchar _horizontal_bits[]={'\x00','\x00','\x01','\x00','\x00','\x00' };
static const gchar _vertical_bits[]={'\x10'};


void gDraw::init()
{
	_shadow = GTK_SHADOW_NONE;
	_state = GTK_STATE_NORMAL;
	stl = NULL;
	stl_name = NULL;
	dr = drm = NULL;
	gc = gcm = NULL;
	ft = NULL;
	stipple = NULL;
	tag = NULL;
	_x = _y = _width = _height = _resolution = 0;
	_gc_stack = NULL;
	_widget = NULL;
}

void gDraw::clear()
{
	dArea = NULL;
	
	gFont::assign(&ft);
	
	if (dr) g_object_unref(G_OBJECT(dr));
	if (drm) g_object_unref(G_OBJECT(drm));
	if (gc) g_object_unref(G_OBJECT(gc));
	if (gcm) g_object_unref(G_OBJECT(gcm));
	if (stipple) g_object_unref(G_OBJECT(stipple));
	
	dr = NULL;
	drm = NULL;
	gc = NULL;
	gcm = NULL;
	stipple = NULL;
	
	if (_gc_stack)
	{
		uint i;
		
		for (i = 0; i < _gc_stack->len; i++)
			g_object_unref(G_OBJECT(g_array_index(_gc_stack, GdkGC *, i)));
		g_array_free(_gc_stack, TRUE);
		_gc_stack = NULL;
	}
}

gDraw::gDraw()
{
	init();
}


gDraw::~gDraw()
{	
	disconnect();
	clear();
}

void gDraw::reset()
{
	clear();
	init();
	
	_shadow = GTK_SHADOW_NONE;
	_state = GTK_STATE_NORMAL;
	line_style = LINE_SOLID;
	clip_enabled = false;
	clip.x = 0;
	clip.y = 0;
	clip.width = 0;
	clip.height = 0;
	stipple = NULL;
	fillCol = 0;
	fill = 0;
	_transparent = false;
	_fillX = _fillY = 0;
}

void gDraw::initGC()
{
	if (dr) 
	{
		g_object_ref(G_OBJECT(dr));
		gc = gdk_gc_new(dr);
		gdk_gc_set_fill(gc,GDK_SOLID);
		#ifdef GOT_X11_PLATFORM
		XSetArcMode(GDK_GC_XDISPLAY(gc), GDK_GC_XGC(gc), ArcPieSlice);
		#endif
	}
	if (drm) 
	{
		g_object_ref(G_OBJECT(drm));
		gcm = gdk_gc_new(drm);
		gdk_gc_set_fill(gcm,GDK_SOLID);
		#ifdef GOT_X11_PLATFORM
		XSetArcMode(GDK_GC_XDISPLAY(gcm), GDK_GC_XGC(gcm), ArcPieSlice);
		#endif
	}

	setTransparent(true);
	setBackground(COLOR_DEFAULT);
	setForeground(COLOR_DEFAULT);
}

void gDraw::connect(gControl *wid)
{
	GdkSubwindowMode mode = GDK_CLIP_BY_CHILDREN;
	
	reset();
	
	ft = wid->font()->copy(); //ft = new gFont(wid->widget); 
	
	_width = wid->width();
	_height = wid->height();
	
	_default_bg = wid->realBackground();
	_default_fg = wid->realForeground();
	if (_default_bg == COLOR_DEFAULT)
		_default_bg = gDesktop::bgColor();
	if (_default_fg == COLOR_DEFAULT)
		_default_fg = gDesktop::fgColor();
	
	_widget = wid->widget;

	switch (wid->getClass())
	{
		case Type_gMainWindow: 
			dr = gtk_layout_get_bin_window (GTK_LAYOUT(wid->widget)); 
			mode = GDK_INCLUDE_INFERIORS;
			break;
			
		case Type_gDrawingArea:
			if ( ((gDrawingArea*)wid)->cached() )
			{
				dArea=(gDrawingArea*)wid;
				dArea->resizeCache();
				dr=dArea->buffer;
			//	gdk_window_freeze_updates (gtk_layout_get_bin_window (GTK_LAYOUT(wid->widget)));
			}
			else
			{
				GtkAllocation *a;
				gtk_widget_get_allocation (wid->widget,a);
				dr = gtk_widget_get_window(wid->widget);
				_x = a->x;
				_y = a->y;
			}

			break;
			
		case Type_gFrame:
			dr=gtk_layout_get_bin_window (GTK_LAYOUT(wid->widget)); 
			break;
			
		default:
			dr=gtk_widget_get_window (wid->widget);
			break;
		
	}
	
	initGC();
	
	if (mode != GDK_CLIP_BY_CHILDREN)
		gdk_gc_set_subwindow(gc, mode);
}


void gDraw::connect(gPicture *wid)
{
	reset();
	
	ft = new gFont();
	_width = wid->width();
	_height = wid->height();
	_default_bg = 0xFFFFFF;
	_default_fg = 0;
	
	dr = wid->getPixmap();
	drm = wid->getMask();

	wid->invalidate();
	initGC();
}

void gDraw::disconnect()
{
	//GdkRectangle rect;

	if (stl)
	{
		g_object_unref(G_OBJECT(stl));
		stl = NULL;
	}

	if (dr) 
	{
		if (dArea && dArea->cached())
		{
			dArea->setCache();
			//dArea->updateCache();
			dArea = NULL;
		}
		g_object_unref(G_OBJECT(dr));
		dr = NULL;
		if (drm)
		{
			g_object_unref(G_OBJECT(drm));
			drm = NULL;
		}
	}
}

/**************************************************************************

Save / restore GC state

***************************************************************************/

void gDraw::save()
{
	GdkGC *copy;
	
	if (!_gc_stack)
		_gc_stack = g_array_new(FALSE, FALSE, sizeof(GdkGC *));
	
	copy = gdk_gc_new(dr);
	gdk_gc_copy(copy, gc);
	g_array_append_val(_gc_stack, copy);
	
	if (gcm)
	{
		copy = gdk_gc_new(drm);
		gdk_gc_copy(copy, gcm);
		g_array_append_val(_gc_stack, copy);
	}
}

void gDraw::restore()
{
	GdkGC *copy;
	
	if (!_gc_stack || _gc_stack->len <= 0)
		return;
		
	copy = g_array_index(_gc_stack, GdkGC *, _gc_stack->len - 1);
	gdk_gc_copy(gc, copy);
	g_object_unref(G_OBJECT(copy));
	g_array_remove_index(_gc_stack, _gc_stack->len - 1);
	
	if (gcm && _gc_stack->len > 0)
	{
		copy = g_array_index(_gc_stack, GdkGC *, _gc_stack->len - 1);
		gdk_gc_copy(gcm, copy);
		g_object_unref(G_OBJECT(copy));
		g_array_remove_index(_gc_stack, _gc_stack->len - 1);
	}
}

/**************************************************************************

Information

***************************************************************************/

GtkStyle *gDraw::style(const char *name, GType type)
{
	if (stl && (!name || (stl_name && !strcmp(name, stl_name))))
		return stl;
	
	if (stl)
	{
		g_object_unref(stl);
		stl = NULL;
	}
	
	if (!name && _widget)
	{
		stl = gtk_style_copy(gtk_widget_get_style(_widget)); //deprecated use _get_style_context instead
		stl = gtk_style_attach(stl, gtk_widget_get_window (_widget));
	}
	else
	{
		if (name)
			stl = gtk_style_copy(gt_get_style(name, type));
		else
			stl = gtk_style_copy(gtk_widget_get_default_style());
		
		stl = gtk_style_attach(stl, (GdkWindow*)dr);
	}
	
	stl_name = name;
	
	return stl;
}

void gDraw::setState(int vl)
{
	switch (vl)
	{
		case GTK_STATE_NORMAL:
		case GTK_STATE_ACTIVE:
		case GTK_STATE_PRELIGHT:
		case GTK_STATE_SELECTED:
		case GTK_STATE_INSENSITIVE: _state=vl;
	}
}

void gDraw::setShadow(int vl)
{
	switch (vl)
	{
		case GTK_SHADOW_NONE:
		case GTK_SHADOW_IN:
		case GTK_SHADOW_OUT:
		case GTK_SHADOW_ETCHED_IN:
		case GTK_SHADOW_ETCHED_OUT: _shadow=vl;
	}
}

/**************************************************************************

Line properties

***************************************************************************/

int gDraw::lineWidth()
{
	GdkGCValues val;

	gdk_gc_get_values(gc,&val);
	return val.line_width;
	
}

void gDraw::setLineWidth(int vl)
{
	GdkGCValues val;

	if (vl<1) vl=1;
	
	gdk_gc_get_values(gc, &val);
	gdk_gc_set_line_attributes(gc, vl, val.line_style, val.cap_style, val.join_style);
	if (drm)
	{
		gdk_gc_get_values(gcm, &val);
		gdk_gc_set_line_attributes(gcm, vl, val.line_style, val.cap_style, val.join_style);
	}
}

int gDraw::lineStyle()
{
	return line_style;
}

void gDraw::setLineStyle(int vl)
{
	gint8 _dash[6],bucle;
	GdkGCValues val;
	GdkLineStyle style;
	
	if ( (vl<0) || (vl>5) ) return;
	
	line_style=vl;
	
	gdk_gc_get_values(gc, &val);
	style = _transparent ? GDK_LINE_ON_OFF_DASH : GDK_LINE_DOUBLE_DASH;
	
	if (val.line_width>5)
		_dash[0]=val.line_width*3;
	else
		_dash[0]=12;
	
	
	for (bucle=1;bucle<6;bucle++)
	{
		if (val.line_width>5)
			_dash[bucle]=val.line_width;
		else
			_dash[bucle]=3;
	}
	
	switch(vl)
	{
		case LINE_DASH:
			gdk_gc_set_dashes(gc, 0, _dash, 2);
			if (gcm) gdk_gc_set_dashes(gcm, 0, _dash, 2);
			break;
			
		case LINE_DASH_DOT:   
			gdk_gc_set_dashes(gc,0,_dash,4); 
			if (gcm) gdk_gc_set_dashes(gcm, 0, _dash, 4);
			break;
		
		case LINE_DASH_DOT_DOT:
			gdk_gc_set_dashes(gc,0,_dash,6); 
			if (gcm) gdk_gc_set_dashes(gcm, 0, _dash, 6);
			break;
		
		case LINE_DOT:  
			if (val.line_width>5)
				_dash[0]=val.line_width;
			else
				_dash[0]=3;      
			gdk_gc_set_dashes(gc,0,_dash,2);
			if (gcm) gdk_gc_set_dashes(gcm,0,_dash,2);
			break;
		
		case LINE_SOLID:
			style = GDK_LINE_SOLID;
			break;
			
		case LINE_NONE:       
			break;
	}

	gdk_gc_set_line_attributes(gc, val.line_width, style, val.cap_style, val.join_style);
	if (gcm) gdk_gc_set_line_attributes(gcm, val.line_width, style, val.cap_style, val.join_style);
}


/**************************************************************************

Colors

***************************************************************************/
gColor gDraw::foreground()
{
	GdkGCValues val;

	gdk_gc_get_values(gc,&val);
	return val.foreground.pixel & 0xFFFFFF;
}

gColor gDraw::background()
{
	GdkGCValues val;

	gdk_gc_get_values(gc,&val);
	return val.background.pixel & 0xFFFFFF;
}

gColor gDraw::fillColor()
{
	return fillCol;
}
	
bool gDraw::invert()
{
	GdkGCValues val;

	gdk_gc_get_values(gc,&val);
	return val.function == GDK_XOR;
}

void gDraw::setForeground(gColor vl)
{
	GdkColormap *cmap;
	GdkColor gcol;

	if (vl == COLOR_DEFAULT)
		vl = _default_fg;	

	//if ( foreground()==vl) return;
			
	cmap=gdk_drawable_get_colormap(dr);
	fill_gdk_color(&gcol, vl, cmap);
	gdk_gc_set_foreground(gc, &gcol);
	
	if (gcm)
	{
	  GdkGCValues val;
		val.foreground.pixel = (vl & 0xFF000000) ? 0 : 1;
		gdk_gc_set_values(gcm, &val, GDK_GC_FOREGROUND);
	}
}

void gDraw::setBackground(gColor vl)
{
	GdkColormap *cmap;
	GdkColor gcol;

	if (vl == COLOR_DEFAULT)
		vl = _default_bg;
	
	//if ( background()==vl) return;
		
	cmap=gdk_drawable_get_colormap(dr);
	fill_gdk_color(&gcol, vl, cmap);
	gdk_gc_set_background(gc, &gcol);

	if (gcm)
	{
	  GdkGCValues val;
		val.background.pixel = (vl & 0xFF000000) ? 0 : 1;
		gdk_gc_set_values(gcm, &val, GDK_GC_BACKGROUND);
	}
}

void gDraw::setFillColor(gColor vl)
{
	fillCol = vl;
}

void gDraw::setInvert(bool vl)
{
	gdk_gc_set_function(gc,vl ? GDK_XOR : GDK_COPY);
}

/**************************************************************************

Fill

***************************************************************************/
int gDraw::fillX()
{
	return _fillX;
}

void gDraw::setFillX(int vl)
{
	GdkGCValues val;

	_fillX = vl;
	gdk_gc_get_values(gc,&val);
	gdk_gc_offset(gc,vl,val.ts_y_origin);
	if (gcm) gdk_gc_offset(gcm, vl, val.ts_y_origin);
}

int gDraw::fillY()
{
	return _fillY;
}

void gDraw::setFillY(int vl)
{
	GdkGCValues val;

	_fillY = vl;
	gdk_gc_get_values(gc,&val);
	gdk_gc_offset(gc,val.ts_x_origin,vl);
	if (gcm) gdk_gc_offset(gcm, val.ts_x_origin, vl);
}

int gDraw::fillStyle()
{
	return fill;
}

void gDraw::setFillStyle(int vl)
{
	if ( (fill<0) || (fill>14) ) return;
	
	fill=vl;
	if (stipple) {
		g_object_unref(G_OBJECT(stipple));
		stipple=NULL;
	}
	
	if ( (fill==FILL_NONE) || (fill==FILL_SOLID) ) return;
	
	switch (vl)
	{       
		case FILL_DENSE_94: 
			stipple=gdk_bitmap_create_from_data(NULL,_dense94_bits,8,8); break;     
		case FILL_DENSE_88:    
			stipple=gdk_bitmap_create_from_data(NULL,_dense88_bits,8,8); break;  
		case FILL_DENSE_63:     
			stipple=gdk_bitmap_create_from_data(NULL,_dense63_bits,8,8); break; 
		case FILL_DENSE_50:  
			stipple=gdk_bitmap_create_from_data(NULL,_dense50_bits,8,8); break;    
		case FILL_DENSE_37: 
			stipple=gdk_bitmap_create_from_data(NULL,_dense37_bits,8,8); break;     
		case FILL_DENSE_12:  
			stipple=gdk_bitmap_create_from_data(NULL,_dense12_bits,8,8); break;    
		case FILL_DENSE_06:      
			stipple=gdk_bitmap_create_from_data(NULL,_dense6_bits,8,8); break;
		case FILL_HORIZONTAL: 
			stipple=gdk_bitmap_create_from_data(NULL,_horizontal_bits,1,6); break;        
		case FILL_VERTICAL:        
			stipple=gdk_bitmap_create_from_data(NULL,_vertical_bits,6,1); break;
		case FILL_CROSS: 
			stipple=gdk_bitmap_create_from_data(NULL,_cross_bits,8,8); break;     
		case FILL_BACK_DIAGONAL:      
			stipple=gdk_bitmap_create_from_data(NULL,_back_diagonal_bits,8,8); break;
		case FILL_DIAGONAL: 
			stipple=gdk_bitmap_create_from_data(NULL,_diagonal_bits,8,8); break;     
		case FILL_CROSS_DIAGONAL: 
			stipple=gdk_bitmap_create_from_data(NULL,_cross_diagonal_bits,8,8); break;
	}
	if (stipple) 
	{
		gdk_gc_set_stipple (gc,stipple);
		if (gcm) gdk_gc_set_stipple (gcm,stipple);
	}
}

/**************************************************************************

Clip Area

***************************************************************************/

int gDraw::clipX()
{
	return clip.x;
}

int gDraw::clipY()
{
	return clip.y;
}

int gDraw::clipWidth()
{
	return clip.width; 
}

int gDraw::clipHeight()
{
	return clip.height;
}

bool gDraw::clipEnabled()
{
	return clip_enabled;
}

void gDraw::setClipEnabled(bool vl)
{
	if (vl)
	{
		gdk_gc_set_clip_rectangle(gc,&clip);
		if (gcm) gdk_gc_set_clip_rectangle(gcm, &clip);
		clip_enabled=true;
	}
	else
	{
		gdk_gc_set_clip_rectangle(gc,NULL);
		if (gcm) gdk_gc_set_clip_rectangle(gcm, NULL);
		clip_enabled=false;
	}
}

void gDraw::setClip(int x,int y,int w,int h)
{
	clip_enabled=true;
	clip.x=x;
	clip.y=y;
	clip.width=w;
	clip.height=h;
	gdk_gc_set_clip_rectangle(gc,&clip);
	if (gcm) gdk_gc_set_clip_rectangle(gcm, &clip);
}


/**************************************************************************

Primitives

***************************************************************************/

void gDraw::line(int x1,int y1,int x2,int y2)
{
	if (!line_style) return;
	
	gdk_draw_line(dr,gc,x1,y1,x2,y2);
	if (drm) gdk_draw_line(drm,gcm,x1,y1,x2,y2);
}

void gDraw::point(int x,int y)
{
	gdk_draw_point(dr,gc,x,y);
	if (drm) gdk_draw_point(drm,gcm,x,y);
}

void gDraw::startFill()
{
	if (fill > 1) 
	{
		gdk_gc_set_fill(gc,_transparent ? GDK_STIPPLED : GDK_OPAQUE_STIPPLED);
		if (gcm) gdk_gc_set_fill(gcm,_transparent ? GDK_STIPPLED : GDK_OPAQUE_STIPPLED);
	}
	_save_fg = foreground();
	setForeground(fillCol);
}

void gDraw::endFill()
{
	setForeground(_save_fg);
	gdk_gc_set_fill(gc,GDK_SOLID);
	if (gcm) gdk_gc_set_fill(gcm,GDK_SOLID);	
}

void gDraw::rect(int x,int y,int width,int height)
{
	x += _x;
	y += _y;
	if (width<0) { x+=width; width=-width; }
	if (height<0) { y+=height; height=-height; }
	
	if (fill)
	{
		startFill();
		gdk_draw_rectangle (dr,gc,true,x,y,width,height);	
		if (drm) gdk_draw_rectangle (drm,gcm,true,x,y,width,height);	
		endFill();
	}
	gdk_gc_set_fill(gc,GDK_SOLID);
	if (gcm) gdk_gc_set_fill(gcm,GDK_SOLID);
	
	if (!line_style) return;
	
	gdk_draw_rectangle(dr,gc,false,x,y,width-1,height-1);
	if (drm) gdk_draw_rectangle(drm,gcm,false,x,y,width-1,height-1);
}

void gDraw::ellipse(int x, int y, int w, int h, double start, double end)
{
	int as, ae;

	if (start == end)
	{
		as = 0;
		ae = 64 * 360;
	}
	else
	{
		as = (int)(start * 180 / M_PI * 64);
		ae = (int)(end * 180 / M_PI * 64) - as;
	}

	if (fill)
	{
		startFill();
		gdk_draw_arc(dr,gc,true,x,y,w-1,h-1, as, ae);	
		if (drm) gdk_draw_arc(drm,gcm,true,x,y,w-1,h-1, as, ae);	
		endFill();
	}
	
	if (!line_style) return;
	
	gdk_draw_arc(dr, gc, false, x, y, w - 1, h - 1, as, ae);
	if (drm) gdk_draw_arc(drm, gcm, false, x, y, w - 1, h - 1, as, ae);
	
	if (start != end)
	{
		int xc, yc;
		int xs, ys;
		int xe, ye;
		
		#define CROP_XY(_x, _y) \
			if (_x < x) _x = x; else if (_x >= (x + w)) _x = x + w - 1; \
			if (_y < y) _y = y; else if (_y >= (y + h)) _y = y + h - 1;
		
		xc = (x + x + w) / 2; //+ cos(start) / 2;
		yc = (y + y + h) / 2; //- sin(start) / 2;
		
		xs = xc + ((double)w / 2) * cos(start) + 0.5;
		ys = yc - ((double)h / 2) * sin(start) + 0.5;
		CROP_XY(xs, ys);

		line(xc, yc, xs, ys);
		
		//xc = (x + x + w - 1) / 2 + cos(end) / 2;
		//yc = (y + y + h - 1) / 2 - sin(end) / 2;
		
		xe = xc + ((double)w / 2) * cos(end) + 0.5;
		ye = yc - ((double)h / 2)  * sin(end) + 0.5;
		CROP_XY(xe, ye);

		line(xc, yc, xe, ye);
	}
} 

void gDraw::arc(int x, int y, int w, int h, double start, double end)
{
	int as, ae;

	if (!line_style)
		return;
	
	if (start == end)
	{
		as = 0;
		ae = 64 * 360;
	}
	else
	{
		as = (int)(start * 180 / M_PI * 64);
		ae = (int)(end * 180 / M_PI * 64) - as;
	}

	gdk_draw_arc(dr, gc, false, x, y, w - 1, h - 1, as, ae);
	if (drm) gdk_draw_arc(drm, gcm, false, x, y, w - 1, h - 1, as, ae);
} 

void gDraw::polyline(int *vl, int nel)
{
	GdkPoint *points;
	
	if (!line_style) return; 

	if (nel <= 0) return;

	// BM: This is actually the same data structure, so let's avoid the copy!
	/*points=(GdkPoint*)g_malloc(sizeof(GdkPoint)*nel);
	for (bucle=0;bucle<nvl;bucle+=2)
	{
		points[b2].x=(gint)vl[bucle];
		points[b2].y=(gint)vl[bucle+1];
		b2++;
	}*/
	
	points = (GdkPoint *)vl;
	
	gdk_draw_lines(dr,gc,points,nel);
	if (drm) gdk_draw_lines(drm,gcm,points,nel);
	//g_free(points);
}

void gDraw::polygon (int *vl, int nel)
{
	GdkPoint *points;
	
	if (!GDK_IS_DRAWABLE (dr)) return;
	if (nel <= 0) return;

	// BM: This is actually the same data structure, so let's avoid the copy!
	/*points=(GdkPoint*)g_malloc(sizeof(GdkPoint)*nel);
	for (bucle=0;bucle<nvl;bucle+=2)
	{
		points[b2].x=(gint)vl[bucle];
		points[b2].y=(gint)vl[bucle+1];
		b2++;
	}*/
	
	points = (GdkPoint *)vl;
	
	if (fill)
	{
		startFill();
		gdk_draw_polygon(dr,gc,true,points,nel);	
		if (drm) gdk_draw_polygon(drm,gcm,true,points,nel);	
		endFill();
	}
	if (!line_style)
		return;
	
	gdk_draw_polygon(dr,gc,false,points,nel);
	if (drm) gdk_draw_polygon(drm,gcm,false,points,nel);
	//g_free(points);
}

/****************************************************************************************

Rendering

*****************************************************************************************/

void gDraw::picture(gPicture *pic, int x, int y, int w, int h, int sx, int sy, int sw, int sh)
{
	GdkBitmap *mask = NULL;
	bool del = false;
	
	if (!pic || pic->isVoid()) return;
	
	GT_NORMALIZE(x, y, w, h, sx, sy, sw, sh, pic->width(), pic->height());
	
	if (clip_enabled)
	{
		GdkRectangle rect = { x, y, w, h };
		GdkRectangle dst;
		if (!gdk_rectangle_intersect(&rect, &clip, &dst))
			return;
			
		sx += dst.x - x;
		sy += dst.y - y;
		sw += dst.width - w;
		sh += dst.height - h;
		x = dst.x;
		y = dst.y;
		w = dst.width;
		h = dst.height;
	}
	
	if (pic->type() == gPicture::SERVER && w == sw && h == sh)
	{
		if (pic->isTransparent())
			mask = pic->getMask();
	
		if (mask)
		{
			GdkGC *tmp_gc = gdk_gc_new(dr);
			gdk_gc_set_clip_mask(tmp_gc, mask);
			gdk_gc_set_clip_origin(tmp_gc, x, y);
			gdk_draw_drawable(dr, tmp_gc, pic->getPixmap(), sx, sy, x, y, sw, sh);
			gdk_gc_set_clip_mask(tmp_gc, NULL);
			g_object_unref(tmp_gc);
		}
		else
			gdk_draw_drawable(dr, gc, pic->getPixmap(), sx, sy, x, y, sw, sh);
	}
  else
  {
  	if (w != sw || h != sh)
  	{
			bool smooth;
			
			if (w >= sw && h >= sw && (w % sw) == 0 && (h % sh) == 0)
				smooth = false;
			else
				smooth = true;
			
  		gPicture *pic2;
  		pic2 = pic->copy(sx, sy, sw, sh);
  		pic = pic2->stretch(w, h, smooth);
  		delete pic2;
  		del = true;
  		sx = 0; sy = 0; sw = w; sh = h;
  	}
		
		gdk_draw_pixbuf(dr, gc, pic->getPixbuf(), sx, sy, x, y, sw, sh, GDK_RGB_DITHER_MAX, 0, 0);
		
	}

	if (drm)
	{
		//GdkGCValues val;
		//val.foreground.pixel = 0xFFFFFF;
		//gdk_gc_set_values(gcm, &val, GDK_GC_FOREGROUND);
		
		if (pic->isTransparent())
			mask = pic->getMask();
		
		if (mask)
		{
			gdk_gc_set_function(gcm, GDK_OR);
			gdk_draw_drawable(drm, gcm, mask, sx, sy, x, y, sw, sh);
			gdk_gc_set_function(gcm, GDK_COPY);
		}
		else
		{
			gdk_draw_rectangle(drm, gcm, true, x, y, w, h);	
		}
		
		//val.foreground.pixel = (foreground() & 0xFF) ? 0 : 0xFFFFFF;
		//gdk_gc_set_values(gcm, &val, GDK_GC_FOREGROUND);
	}

	if (del)
		delete pic;
}

void gDraw::tiledPicture(gPicture *pic, int x, int y, int w, int h)
{
	bool save_clip_enabled;
	GdkRectangle save_clip;
	GdkRectangle new_clip = {x, y, w, h};
	int px, py, pw, ph;
	int fx, fy;
	
	if (!pic || pic->isVoid() || w < 1 || h < 1) return;
	
	pic->getPixmap();
	
	save_clip_enabled = clip_enabled;
	save_clip = clip;
	
	if (clip_enabled)
	{
		GdkRectangle dst;
		if (!gdk_rectangle_intersect(&new_clip, &clip, &dst))
			return;
		new_clip = dst;
	}	

	setClip(&new_clip);
	
	pw = pic->width();
	ph = pic->height();

	fx = -fillX();
	if (fx > 0)
		fx = fx % pw;
	else
		fx = pw - ((-fx) % pw);

	fy = -fillY();
	if (fy > 0)
		fy = fy % ph;
	else
		fy = ph - ((-fy) % ph);
	
	x -= fx;
	y -= fy;
	w += fx;
	h += fy;
	
	px = x;
	py = y;
	
	for(;;)
	{
		picture(pic, px, py, pw, ph);
		px += pw;
		if (px >= (x + w))
		{
			px = x;
			py += ph;
			if (py >= (y + h))
				break;
		}
	}
	
	if (save_clip_enabled)
		setClip(&save_clip);
	else
		setClipEnabled(false);
}

/****************************************************************************************

Text

*****************************************************************************************/

gFont* gDraw::font()
{
	return ft;
}

void gDraw::setFont(gFont *f)
{
	//if (f != ft)
	  gFont::set(&ft, f->copy());
}


int gDraw::textWidth(char *txt, int len)
{
	if (!txt) return 0;
	if (!len) return 0;
	
	return ft->width((const char*)txt, len);
}

int gDraw::textHeight(char *txt, int len)
{
	if (!txt) return 0;
	if (!len) return 0;
	
	return ft->height((const char*)txt, len);
}

void gDraw::drawLayout(PangoLayout *ly, bool markup, int x, int y, int w, int h, int align)
{
	int offx = 0, offy = 0;
	float foffx, foffy;
	int tw, th;
	float ftw, fth;

	gt_add_layout_from_font(ly, font());
	
	gt_layout_alignment(ly, (float)w, (float)h, &ftw, &fth, align, &foffx, &foffy);
	tw = (int)ftw;
	th = (int)fth;
	offx = (int)foffx;
	offy = (int)foffy;

	if (markup)
		offx = 0;

	if (!_transparent)
	{
		gColor buf = foreground();
		setForeground(background());
		gdk_draw_rectangle (dr, gc, true, x + offx, y + offy, tw, th);	
		if (drm) gdk_draw_rectangle (drm, gcm, true, x + offx, y + offy, tw, th);	
		setForeground(buf);
	}
	
	gdk_draw_layout(dr, gc, x + offx, y + offy, ly);
	
	if (drm && _transparent)
	{
		GdkBitmap *mask;
		
		mask = gt_make_text_mask(dr, tw, th, ly, 0, 0);
		
		gdk_gc_set_function(gcm, GDK_OR);
		gdk_draw_drawable(drm, gcm, mask, 0, 0, x + offx, y + offy, tw, th);
		gdk_gc_set_function(gcm, GDK_COPY);
		
		g_object_unref(mask);
	}
	
	g_object_unref(G_OBJECT(ly));
}

void gDraw::text(char *txt,int len,int x,int y,int w,int h,int align)
{
	PangoLayout *ly;
	
	if (!txt || !len) return;

	ly = pango_layout_new(ft->ct);
	pango_layout_set_text(ly, txt, len);
	drawLayout(ly, false, x, y, w, h, align);
}

void gDraw::richText(char *txt,int len,int x,int y,int w,int h,int align)
{
	PangoLayout *ly;
	char *html;
	
	if (!txt || !len) return;

	ly = pango_layout_new(ft->ct);
	if (w > 0)
		pango_layout_set_width(ly, w * PANGO_SCALE);
	html = gt_html_to_pango_string(txt, len, false);
	pango_layout_set_markup(ly, html, -1);
	pango_layout_set_wrap(ly, PANGO_WRAP_WORD_CHAR);
	drawLayout(ly, true, x, y, w, h, align);
	g_free(html);
}

void gDraw::richTextSize(char *txt, int len, int sw, int *w, int *h)
{
	PangoLayout *ly;
	int tw = 0, th = 0;
	char *html;
	
	if (txt && len)
	{
		ly = pango_layout_new(ft->ct);
		html = gt_html_to_pango_string(txt, len, false);
		pango_layout_set_markup(ly, html, -1);	
		pango_layout_set_wrap(ly, PANGO_WRAP_WORD_CHAR);
		if (sw > 0)
			pango_layout_set_width(ly, sw * PANGO_SCALE);
		pango_layout_get_pixel_size(ly, &tw, &th);
		g_free(html);
	}
	
	if (w) *w = tw;
	if (h) *h = th;
}


int gDraw::resolution()
{
	GdkScreen *screen;
	gint h, hmm;
	
	if (!GDK_IS_DRAWABLE(dr)) 
		return 0;
	
	screen = gdk_drawable_get_screen(dr);
	h = gdk_screen_get_height(screen);
	hmm = gdk_screen_get_height_mm(screen);
	
	return (h * 254 + hmm * 5) / (hmm * 10);
}

bool gDraw::isTransparent()
{
	return _transparent;
}

void gDraw::setTransparent(bool vl)
{
	_transparent = vl;
	
	if (lineStyle() > 1) setLineStyle(lineStyle());
}

