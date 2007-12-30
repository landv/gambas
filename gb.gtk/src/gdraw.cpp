/***************************************************************************

  gdraw.cpp

  (c) 2004-2006 - Daniel Campos Fernández <dcamposf@gmail.com>
  
  Gtkmae "GTK+ made easy" classes
  
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

#include "widgets.h"
#include "widgets_private.h"
#include "gdrawingarea.h"
#include "gdesktop.h"
#include "gdraw.h"

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


gDraw::gDraw()
{
	dr=NULL;
	ft=NULL;
	gc=NULL;
	stipple=NULL;
	_width = _height = _resolution = 0;
	tag = NULL;
}


gDraw::~gDraw()
{	
	pClear();
}


void gDraw::connect(gControl *wid)
{
	pClear();
	line_style=LINE_SOLID;
	clip_enabled=false;
	clip.x=0;
	clip.y=0;
	clip.width=0;
	clip.height=0;
	stipple=NULL;
	fillCol=0;
	fill=0;
	dr=NULL;
	ft=new gFont(wid->widget); 
	_width = wid->width();
	_height = wid->height();
	_default_bg = wid->realBackground();
	_default_fg = wid->realForeground();
	if (_default_bg == COLOR_DEFAULT)
		_default_bg = gDesktop::bgColor();
	if (_default_fg == COLOR_DEFAULT)
		_default_fg = gDesktop::fgColor();
	
	switch (wid->getClass())
	{
		case Type_gMainWindow: 
			dr=wid->widget->window; 
			break;
			
		case Type_gDrawingArea:
			if ( ((gDrawingArea*)wid)->buffer )
			{
				dArea=(gDrawingArea*)wid;
				dr=dArea->buffer;
			//	gdk_window_freeze_updates (GTK_LAYOUT(wid->widget)->bin_window);
			}
			else
			{
				dr=GTK_LAYOUT(wid->widget)->bin_window; 
			}

			break;
			
		case Type_gFrame:
			dr=GTK_LAYOUT(wid->widget)->bin_window; 
			break;
			
		default:
			dr=wid->widget->window;
			break;
		
	}
	
	if (dr) {
		g_object_ref(G_OBJECT(dr));
		gc=gdk_gc_new(dr);
		gdk_gc_set_fill(gc,GDK_SOLID);
	}

	
}


void gDraw::connect(gPicture *wid)
{
	pClear();
	line_style=LINE_SOLID;
	clip_enabled=false;
	clip.x=0;
	clip.y=0;
	clip.width=0;
	clip.height=0;
	stipple=NULL;
	fillCol=0;
	fill=0;
	dr=NULL;
	ft=new gFont();
	_width = wid->width();
	_height = wid->height();
	_default_bg = 0xFFFFFF;
	_default_fg = 0;
	
	dr = wid->getPixmap();
	wid->invalidate();

	if (dr) {

		g_object_ref(G_OBJECT(dr));
		gc=gdk_gc_new(dr);
		gdk_gc_set_fill(gc,GDK_SOLID);
	}
}

void gDraw::disconnect()
{
	GdkRectangle rect;

	if (dr) {
		if (dArea)
		{
			//gdk_window_thaw_updates (GTK_LAYOUT(dArea->widget)->bin_window);
			rect.x=0;
			rect.y=0;
			rect.width=dArea->width();
			rect.height=dArea->height();
			gdk_window_invalidate_rect(GTK_LAYOUT(dArea->widget)->bin_window,&rect,false);
			//gdk_window_process_updates(GTK_LAYOUT(dArea->widget)->bin_window,false);
			dArea=NULL;
		}
		g_object_unref(G_OBJECT(dr));
		dr=NULL;
	}
}

void gDraw::pClear()
{
	dArea=NULL;
	gFont::assign(&ft);
	if (dr) g_object_unref(G_OBJECT(dr));
	if (gc) g_object_unref(G_OBJECT(gc));
	if (stipple) g_object_unref(G_OBJECT(stipple));
	dr=NULL;
	gc=NULL;
	stipple=NULL;
}

/**************************************************************************

Line properties

***************************************************************************/
long gDraw::lineWidth()
{
	GdkGCValues val;

	if (!GDK_IS_DRAWABLE (dr)) return 0;
	

	gdk_gc_get_values(gc,&val);
	return val.line_width;
	
}

void gDraw::setLineWidth(long vl)
{
	GdkGCValues val;

	if (!GDK_IS_DRAWABLE (dr)) return;
	if (vl<1) vl=1;
	
	gdk_gc_get_values(gc,&val);
	gdk_gc_set_line_attributes(gc,vl,val.line_style,val.cap_style,val.join_style);
	
	
}

long gDraw::lineStyle()
{
	if (!GDK_IS_DRAWABLE (dr)) return 0;
	
	return line_style;
}

void gDraw::setLineStyle(long vl)
{
	gint8 _dash[6],bucle;
	GdkGCValues val;
	
	if (!GDK_IS_DRAWABLE (dr)) return;
	if ( (vl<0) || (vl>5) ) return;
	
	line_style=vl;
	
	gdk_gc_get_values(gc,&val);
	gdk_gc_set_line_attributes(gc,val.line_width,GDK_LINE_ON_OFF_DASH,val.cap_style,val.join_style);
	
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
			gdk_gc_set_dashes(gc,0,_dash,2);
			break;
			
		case LINE_DASH_DOT:   
			gdk_gc_set_dashes(gc,0,_dash,4); 
			break;
		
		case LINE_DASH_DOT_DOT:
			gdk_gc_set_dashes(gc,0,_dash,6); 
			break;
		
		case LINE_DOT:  
			if (val.line_width>5)
				_dash[0]=val.line_width;
			else
				_dash[0]=3;      
			gdk_gc_set_dashes(gc,0,_dash,2);
			break;
		
		case LINE_SOLID:
			gdk_gc_set_line_attributes(gc,val.line_width,GDK_LINE_SOLID,val.cap_style,val.join_style);
			break;
			
		case LINE_NONE:       
			break;

	}

}

/**************************************************************************

Colors

***************************************************************************/
gColor gDraw::foreground()
{
	GdkGCValues val;

	if (!GDK_IS_DRAWABLE (dr)) return 0;
	
	gdk_gc_get_values(gc,&val);
	return val.foreground.pixel & 0xFFFFFF;
}

gColor gDraw::background()
{
	GdkGCValues val;

	if (!GDK_IS_DRAWABLE (dr)) return 0;
	
	gdk_gc_get_values(gc,&val);
	return val.background.pixel & 0xFFFFFF;
}

gColor gDraw::fillColor()
{
	if (!GDK_IS_DRAWABLE (dr)) return 0;

	return fillCol;
}
	
bool gDraw::invert()
{
	GdkGCValues val;

	if (!GDK_IS_DRAWABLE (dr)) return false;
	
	gdk_gc_get_values(gc,&val);
	return val.function == GDK_XOR;
}

void gDraw::setForeground(gColor vl)
{
	GdkColormap *cmap;
	GdkColor gcol;

	if (!GDK_IS_DRAWABLE (dr)) return;

	if (vl == COLOR_DEFAULT)
		vl = _default_fg;	

	if ( foreground()==vl) return;
			
	cmap=gdk_drawable_get_colormap(dr);
	fill_gdk_color(&gcol, vl, cmap);
	gdk_gc_set_foreground(gc, &gcol);
}

void gDraw::setBackground(gColor vl)
{
	GdkColormap *cmap;
	GdkColor gcol;

	if (!GDK_IS_DRAWABLE (dr)) return;
	
	if (vl == COLOR_DEFAULT)
		vl = _default_bg;
	
	if ( background()==vl) return;
		
	cmap=gdk_drawable_get_colormap(dr);
	fill_gdk_color(&gcol, vl, cmap);
	gdk_gc_set_background(gc, &gcol);
}

void gDraw::setFillColor(gColor vl)
{
	if (!GDK_IS_DRAWABLE (dr)) return;
	
	fillCol=vl;
}

void gDraw::setInvert(bool vl)
{
	if (!GDK_IS_DRAWABLE (dr)) return;
	
	if (vl)
		gdk_gc_set_function(gc,GDK_XOR);
	else
		gdk_gc_set_function(gc,GDK_COPY);
}

/**************************************************************************

Fill

***************************************************************************/
long gDraw::fillX()
{
	GdkGCValues val;

	if (!GDK_IS_DRAWABLE (dr)) return 0;
	
	gdk_gc_get_values(gc,&val);
	return val.ts_x_origin;
}

void gDraw::setFillX(long vl)
{
	GdkGCValues val;

	if (!GDK_IS_DRAWABLE (dr)) return;
	
	gdk_gc_get_values(gc,&val);
	gdk_gc_offset(gc,vl,val.ts_y_origin);
}

long gDraw::fillY()
{
	GdkGCValues val;

	if (!GDK_IS_DRAWABLE (dr)) return 0;
	
	gdk_gc_get_values(gc,&val);
	return val.ts_y_origin;
}

void gDraw::setFillY(long vl)
{
	GdkGCValues val;

	if (!GDK_IS_DRAWABLE (dr)) return;
	
	gdk_gc_get_values(gc,&val);
	gdk_gc_offset(gc,val.ts_x_origin,vl);
}

long gDraw::fillStyle()
{
	if (!GDK_IS_DRAWABLE (dr)) return 0;
	
	return fill;
}

void gDraw::setFillStyle(long vl)
{
	if (!GDK_IS_DRAWABLE (dr)) return;
	
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
	if (stipple) gdk_gc_set_stipple (gc,stipple);
	
}

/**************************************************************************

Clip Area

***************************************************************************/
long gDraw::clipX()
{
	if (!GDK_IS_DRAWABLE (dr)) return 0;
	return clip.x;
}

long gDraw::clipY()
{
	if (!GDK_IS_DRAWABLE (dr)) return 0;
	return clip.y;
}

long gDraw::clipWidth()
{
	if (!GDK_IS_DRAWABLE (dr)) return 0;
	return clip.width; 
}

long gDraw::clipHeight()
{
	if (!GDK_IS_DRAWABLE (dr)) return 0;
	return clip.height;
}

bool gDraw::clipEnabled()
{
	if (!GDK_IS_DRAWABLE (dr)) return false;
	
	return clip_enabled;
}

void gDraw::setClipEnabled(bool vl)
{
	if (!GDK_IS_DRAWABLE (dr)) return;
	
	if (vl)
	{
		gdk_gc_set_clip_rectangle(gc,&clip);
		clip_enabled=true;
	}
	else
	{
		gdk_gc_set_clip_rectangle(gc,NULL);
		clip_enabled=false;
	}
}

void gDraw::startClip(long x,long y,long w,long h)
{
	if (!GDK_IS_DRAWABLE (dr)) return;
	
	clip_enabled=true;
	clip.x=x;
	clip.y=y;
	clip.width=w;
	clip.height=h;
	gdk_gc_set_clip_rectangle(gc,&clip);
}


/**************************************************************************

Primitives

***************************************************************************/

void gDraw::line(long x1,long y1,long x2,long y2)
{
	if (!line_style) return;
	if (!GDK_IS_DRAWABLE (dr)) return;
	
	gdk_draw_line(dr,gc,x1,y1,x2,y2);
}

void gDraw::point(long x,long y)
{
	if (!GDK_IS_DRAWABLE (dr)) return;	

	gdk_draw_point(dr,gc,x,y);
}

void gDraw::rect(long x,long y,long width,long height)
{
	gColor buf;

	if (!GDK_IS_DRAWABLE (dr)) return;

	if (width<0) { x+=width; width=-width; }
	if (height<0) { y+=height; height=-height; }
	
	if (fill)
	{
		if (fill>1) gdk_gc_set_fill(gc,GDK_STIPPLED);
		buf=foreground();
		setForeground(fillCol);
		gdk_draw_rectangle (dr,gc,true,x,y,width,height);	
		setForeground(buf);
	}
	gdk_gc_set_fill(gc,GDK_SOLID);
	if (!line_style) return;
	gdk_draw_rectangle (dr,gc,false,x,y,width-1,height-1);
}

void gDraw::ellipse(long x,long y,long w,long h,long start,long end)
{
	gColor buf;
	
	if (!GDK_IS_DRAWABLE (dr)) return;	

	if (fill)
	{
		if (fill>1) gdk_gc_set_fill(gc,GDK_STIPPLED);
		buf=foreground();
		setForeground(fillCol);
		gdk_draw_arc(dr,gc,true,x,y,w-1,h-1,start*64,end*64);	
		setForeground(buf);
	}
	gdk_gc_set_fill(gc,GDK_SOLID);
	if (!line_style) return;
	gdk_draw_arc(dr,gc,false,x,y,w-1,h-1,start*64,end*64);
} 

void gDraw::polyline (long *vl,long nvl)
{
	long bucle;
	GdkPoint *points;
	long nel;
	long b2=0;
	
	
	if (!line_style) return; 
	if (!GDK_IS_DRAWABLE (dr)) return;	

	nel=nvl/2;
	if (!nel) return;

	points=(GdkPoint*)g_malloc(sizeof(GdkPoint)*nel);
	for (bucle=0;bucle<nvl;bucle+=2)
	{
		points[b2].x=(gint)vl[bucle];
		points[b2].y=(gint)vl[bucle+1];
		b2++;
	}
	gdk_draw_lines(dr,gc,points,nel);
	g_free(points);
}

void gDraw::polygon (long *vl,long nvl)
{
	long bucle;
	GdkPoint *points;
	long buf;
	long nel,b2=0;

	
	if (!GDK_IS_DRAWABLE (dr)) return;
	
	nel=nvl/2;
	if (!nel) return;

	points=(GdkPoint*)g_malloc(sizeof(GdkPoint)*nel);
	for (bucle=0;bucle<nvl;bucle+=2)
	{
		points[b2].x=(gint)vl[bucle];
		points[b2].y=(gint)vl[bucle+1];
		b2++;
	}
	if (fill)
	{
		if (fill>1) gdk_gc_set_fill(gc,GDK_STIPPLED);
		buf=foreground();
		setForeground(fillCol);
		gdk_draw_polygon(dr,gc,true,points,nel);	
		setForeground(buf);
	}
	gdk_gc_set_fill(gc,GDK_SOLID);
	if (line_style) gdk_draw_polygon(dr,gc,false,points,nel);
	g_free(points);
}

/****************************************************************************************

Rendering

*****************************************************************************************/

void gDraw::picture(gPicture *pic,long x,long y,long Sx,long Sy,long Sw,long Sh)
{
	if (!pic) return;
	if (!GDK_IS_DRAWABLE (dr)) return;
	
	if (pic->type() == gPicture::SERVER && !pic->transparent())
		gdk_draw_drawable(dr,gc,pic->getPixmap(),Sx,Sy,x,y,Sw,Sh);
  else if (!pic->isVoid())
		gdk_draw_pixbuf(dr,gc,pic->getPixbuf(),Sx,Sy,x,y,Sw,Sh,GDK_RGB_DITHER_MAX,0,0); 
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
  gFont::assign(&ft, f);
}


long gDraw::textWidth(char *txt)
{
	if (!txt) return 0;
	if (!strlen(txt)) return 0;
	
	return ft->width((const char*)txt);
}

long gDraw::textHeight(char *txt)
{
	if (!txt) return 0;
	if (!strlen(txt)) return 0;
	
	return ft->height((const char*)txt);
}

void gDraw::text(char *txt,int len,long x,long y,long w,long h,long align)
{
	PangoLayout *ly;
	long OffX=0,OffY=0;

	if (!txt) return;
	if (!strlen(txt)) return;
	if (!GDK_IS_DRAWABLE (dr)) return;

	ly=pango_layout_new(ft->ct);
	pango_layout_set_text(ly,txt,len);
	switch (align)
	{
		case ALIGN_BOTTOM: 	 
			pango_layout_set_alignment(ly,PANGO_ALIGN_CENTER);	
			OffX+=(w/2)-(ft->width((const char*)txt)/2);
			OffY=h-ft->height((const char*)txt); 
			break;
		
		case ALIGN_BOTTOM_LEFT: 		
			pango_layout_set_alignment(ly,PANGO_ALIGN_LEFT);
			OffX=0;
			OffY=h-ft->height((const char*)txt);
			break;
			
		case ALIGN_BOTTOM_NORMAL:  
			OffX=0;
			OffY=h-ft->height((const char*)txt);
			break;
			
		case ALIGN_BOTTOM_RIGHT: 
			pango_layout_set_alignment(ly,PANGO_ALIGN_RIGHT);
			OffX=w-ft->height((const char*)txt);
			OffY=h-ft->height((const char*)txt);
			break;
			
		case ALIGN_CENTER: 
			pango_layout_set_alignment(ly,PANGO_ALIGN_CENTER);
			OffX+=(w/2)-(ft->width((const char*)txt)/2);
			OffY+=(h/2)-(ft->height((const char*)txt)/2);
			break;
			
		case ALIGN_LEFT:
			pango_layout_set_alignment(ly,PANGO_ALIGN_LEFT);
			OffX=0;
			OffY+=(h/2)-(ft->height((const char*)txt)/2);
			break;
			
		case ALIGN_NORMAL:
			OffX=0;
			OffY+=(h/2)-(ft->height((const char*)txt)/2);
			break;
			
		case ALIGN_RIGHT:
			pango_layout_set_alignment(ly,PANGO_ALIGN_RIGHT);
			OffX=w-ft->height((const char*)txt);
			OffY+=(h/2)-(ft->height((const char*)txt)/2);
			break;
			
		case ALIGN_TOP:
			pango_layout_set_alignment(ly,PANGO_ALIGN_CENTER);
			OffX+=(w/2)-(ft->width((const char*)txt)/2);
			OffY=0;
			break;
			
		case ALIGN_TOP_LEFT:
			pango_layout_set_alignment(ly,PANGO_ALIGN_LEFT);
			OffX=0;
			OffY=0;
			break;
			
		case ALIGN_TOP_NORMAL:
			OffY=0;
			break;
			
		case ALIGN_TOP_RIGHT:
			pango_layout_set_alignment(ly,PANGO_ALIGN_RIGHT);
			OffX=w-ft->height((const char*)txt);
			OffY=0; 
			break;

	}
	
	gdk_draw_layout(dr,gc,x+OffX,y+OffY,ly);
	g_object_unref(G_OBJECT(ly));

}

