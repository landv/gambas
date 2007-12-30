/***************************************************************************

  gdraw.cpp

  (c) 2004-2005 - Daniel Campos Fernández <danielcampos@netcourrier.com>
  
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
#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>

#include <stdio.h>

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
}


gDraw::~gDraw()
{	
	pClear();
}


void gDraw::connect(gControl *wid)
{
	pClear();
	line_style=Line_Solid;
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
	line_style=Line_Solid;
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
	
	if (!wid->pic) return;
	
	dr=wid->pic;

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
	if (ft) ft->unref();
	if (dr) g_object_unref(G_OBJECT(dr));
	if (gc) g_object_unref(G_OBJECT(gc));
	if (stipple) g_object_unref(G_OBJECT(stipple));
	dr=NULL;
}

/**************************************************************************

Line properties

***************************************************************************/
long gDraw::lineWidth()
{
	GdkGCValues val;

	if (!dr) return 0;
	

	gdk_gc_get_values(gc,&val);
	return val.line_width;
	
}

void gDraw::setLineWidth(long vl)
{
	GdkGCValues val;

	if (!dr) return;
	if (vl<1) vl=1;
	
	gdk_gc_get_values(gc,&val);
	gdk_gc_set_line_attributes(gc,vl,val.line_style,val.cap_style,val.join_style);
	
	
}

long gDraw::lineStyle()
{
	if (!dr) return 0;
	
	return line_style;
}

void gDraw::setLineStyle(long vl)
{
	gint8 _dash[6],bucle;
	GdkGCValues val;
	
	if (!dr) return;
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
		case Line_Dash:
			gdk_gc_set_dashes(gc,0,_dash,2);
			break;
			
		case Line_DashDot:   
			gdk_gc_set_dashes(gc,0,_dash,4); 
			break;
		
		case Line_DashDotDot:
			gdk_gc_set_dashes(gc,0,_dash,6); 
			break;
		
		case Line_Dot:  
			if (val.line_width>5)
				_dash[0]=val.line_width;
			else
				_dash[0]=3;      
			gdk_gc_set_dashes(gc,0,_dash,2);
			break;
		
		case Line_Solid:
			gdk_gc_set_line_attributes(gc,val.line_width,GDK_LINE_SOLID,val.cap_style,val.join_style);
			break;
			
		case Line_None:       
			break;

	}

}

/**************************************************************************

Colors

***************************************************************************/
long gDraw::foreGround()
{
	GdkGCValues val;

	if (!dr) return 0;
	
	gdk_gc_get_values(gc,&val);
	return 0xFF000000 | val.foreground.pixel;
}

long gDraw::backGround()
{
	GdkGCValues val;

	if (!dr) return 0;
	
	gdk_gc_get_values(gc,&val);
	return 0xFF000000 | val.background.pixel;
}

long gDraw::fillColor()
{
	if (!dr) return 0;

	return fillCol;
}
	
bool gDraw::invert()
{
	GdkGCValues val;

	if (!dr) return false;
	
	gdk_gc_get_values(gc,&val);
	return val.function == GDK_XOR;
}

void gDraw::setForeGround(long vl)
{
	GdkColormap *cmap;
	GdkColor gcol;

	if (!dr) return;

	if ( foreGround()==vl) return;
			
	cmap=gdk_drawable_get_colormap(dr);
	if (!cmap) cmap=gdk_colormap_get_system();
	gcol.red=0xFF + ((vl & 0xFF0000)>>8);
	gcol.green=0xFF + (vl & 0x00FF00);
	gcol.blue=0xFF + ((vl & 0x0000FF)<<8);
	gdk_color_alloc(cmap,&gcol);
	gdk_gc_set_foreground(gc,&gcol);
		

}

void gDraw::setBackGround(long vl)
{
	GdkColormap *cmap;
	GdkColor gcol;

	if (!dr) return;
	
	if ( backGround()==vl) return;
		
	cmap=gdk_drawable_get_colormap(dr);
	if (!cmap) cmap=gdk_colormap_get_system();
	gcol.red=0xFF + ((vl & 0xFF0000)>>8);
	gcol.green=0xFF + (vl & 0x00FF00);
	gcol.blue=0xFF + ((vl & 0x0000FF)<<8);
	gdk_color_alloc(cmap,&gcol);
	gdk_gc_set_background(gc,&gcol);
}

void gDraw::setFillColor(long vl)
{
	if (!dr) return;
	
	fillCol=vl;
}

void gDraw::setInvert(bool vl)
{
	if (!dr) return;
	
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

	if (!dr) return 0;
	
	gdk_gc_get_values(gc,&val);
	return val.ts_x_origin;
}

void gDraw::setFillX(long vl)
{
	GdkGCValues val;

	if (!dr) return;
	
	gdk_gc_get_values(gc,&val);
	gdk_gc_offset(gc,vl,val.ts_y_origin);
}

long gDraw::fillY()
{
	GdkGCValues val;

	if (!dr) return 0;
	
	gdk_gc_get_values(gc,&val);
	return val.ts_y_origin;
}

void gDraw::setFillY(long vl)
{
	GdkGCValues val;

	if (!dr) return;
	
	gdk_gc_get_values(gc,&val);
	gdk_gc_offset(gc,val.ts_x_origin,vl);
}

long gDraw::fillStyle()
{
	if (!dr) return 0;
	
	return fill;
}

void gDraw::setFillStyle(long vl)
{
	if (!dr) return;
	
	if ( (fill<0) || (fill>14) ) return;
	
	fill=vl;
	if (stipple) {
		g_object_unref(G_OBJECT(stipple));
		stipple=NULL;
	}
	
	if ( (fill==FillNoBrush) || (fill==FillSolidPattern) ) return;
	
	switch (vl)
	{       
		case FillDense1Pattern: 
			stipple=gdk_bitmap_create_from_data(NULL,_dense94_bits,8,8); break;     
		case FillDense2Pattern:    
			stipple=gdk_bitmap_create_from_data(NULL,_dense88_bits,8,8); break;  
		case FillDense3Pattern:     
			stipple=gdk_bitmap_create_from_data(NULL,_dense63_bits,8,8); break; 
		case FillDense4Pattern:  
			stipple=gdk_bitmap_create_from_data(NULL,_dense50_bits,8,8); break;    
		case FillDense5Pattern: 
			stipple=gdk_bitmap_create_from_data(NULL,_dense37_bits,8,8); break;     
		case FillDense6Pattern:  
			stipple=gdk_bitmap_create_from_data(NULL,_dense12_bits,8,8); break;    
		case FillDense7Pattern:      
			stipple=gdk_bitmap_create_from_data(NULL,_dense6_bits,8,8); break;
		case FillHorPattern: 
			stipple=gdk_bitmap_create_from_data(NULL,_horizontal_bits,1,6); break;        
		case FillVerPattern:        
			stipple=gdk_bitmap_create_from_data(NULL,_vertical_bits,6,1); break;
		case FillCrossPattern: 
			stipple=gdk_bitmap_create_from_data(NULL,_cross_bits,8,8); break;     
		case FillBDiagPattern:      
			stipple=gdk_bitmap_create_from_data(NULL,_back_diagonal_bits,8,8); break;
		case FillFDiagPattern: 
			stipple=gdk_bitmap_create_from_data(NULL,_diagonal_bits,8,8); break;     
		case FillDiagCrossPattern: 
			stipple=gdk_bitmap_create_from_data(NULL,_cross_diagonal_bits,8,8); break;
	}
	if (stipple) gdk_gc_set_stipple (gc,stipple);
	
}

/**************************************************************************

Clip Area

***************************************************************************/
long gDraw::clipX()
{
	if (!dr) return 0;
	return clip.x;
}

long gDraw::clipY()
{
	if (!dr) return 0;
	return clip.y;
}

long gDraw::clipWidth()
{
	if (!dr) return 0;
	return clip.width; 
}

long gDraw::clipHeight()
{
	if (!dr) return 0;
	return clip.height;
}

bool gDraw::clipEnabled()
{
	GdkGCValues val;

	if (!dr) return false;
	
	return clip_enabled;
}

void gDraw::setClipEnabled(bool vl)
{
	if (!dr) return;
	
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
	if (!dr) return;
	
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
	if (!dr) return;
	if (!line_style) return;
	
	gdk_draw_line(dr,gc,x1,y1,x2,y2);
}

void gDraw::point(long x,long y)
{
	if (!dr) return;
	
	gdk_draw_point(dr,gc,x,y);
}

void gDraw::rect(long x,long y,long width,long height)
{
	long buf;

	if (!dr) return;


	if (width<0) { x+=width; width*=-1; }
	if (height<0) { y+=height; height*=-1; }
	
	if (fill)
	{
		if (fill>1) gdk_gc_set_fill(gc,GDK_STIPPLED);
		buf=foreGround();
		setForeGround(fillCol);
		gdk_draw_rectangle (dr,gc,true,x,y,width,height);	
		setForeGround(buf);
	}
	gdk_gc_set_fill(gc,GDK_SOLID);
	if (!line_style) return;
	gdk_draw_rectangle (dr,gc,false,x,y,width,height);
}

void gDraw::ellipse(long x,long y,long w,long h,long start,long end)
{
	long buf;
	
	if (!dr) return;
	
	if (fill)
	{
		if (fill>1) gdk_gc_set_fill(gc,GDK_STIPPLED);
		buf=foreGround();
		setForeGround(fillCol);
		gdk_draw_arc(dr,gc,true,x,y,w,h,start*64,end*64);	
		setForeGround(buf);
	}
	gdk_gc_set_fill(gc,GDK_SOLID);
	if (!line_style) return;
	gdk_draw_arc(dr,gc,false,x,y,w,h,start*64,end*64);
} 

void gDraw::polyline (long *vl,long nvl)
{
	long bucle;
	GdkPoint *points;
	long nel;
	long b2=0;
	
	
	if (!dr) return;
	if (!line_style) return; 
	
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

	
	if (!dr) return;
	
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
		buf=foreGround();
		setForeGround(fillCol);
		gdk_draw_polygon(dr,gc,true,points,nel);	
		setForeGround(buf);
	}
	gdk_gc_set_fill(gc,GDK_SOLID);
	if (line_style) gdk_draw_polygon(dr,gc,false,points,nel);
	g_free(points);
}

/****************************************************************************************

Rendering

*****************************************************************************************/
void gDraw::image(gImage *img,long x,long y,long Sx,long Sy,long Sw,long Sh)
{
	if (!dr) return;
	if (!img) return;
	if (!img->image) return;
	
	gdk_draw_pixbuf(dr,gc,img->image,Sx,Sy,x,y,Sw,Sh,GDK_RGB_DITHER_MAX,0,0); 
}

void gDraw::picture(gPicture *pic,long x,long y,long Sx,long Sy,long Sw,long Sh)
{
	GdkPixbuf *buf;
	
	if (!dr) return;
	if (!pic) return;
	if (!pic->pic) return;

	if (pic->transparent())
	{
		buf=pic->getPixbuf();
		if (buf)
		{	
			gdk_draw_pixbuf(dr,gc,buf,Sx,Sy,x,y,Sw,Sh,GDK_RGB_DITHER_MAX,0,0); 
			g_object_unref(G_OBJECT(buf));
		}
	}
	else
	{
		gdk_draw_drawable(dr,gc,pic->pic,Sx,Sy,x,y,Sw,Sh);
	}
}
/****************************************************************************************

Text

*****************************************************************************************/
gFont* gDraw::font()
{
	ft->ref();
	return ft;
}

void gDraw::setFont(gFont *f)
{
	if (ft) ft->unref();
	ft=f;
	ft->ref();
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

void gDraw::text(char *txt,long x,long y,long w,long h,long align)
{
	PangoLayout *ly;
	long OffX=0,OffY=0;

	if (!txt) return;
	if (!strlen(txt)) return;

	ly=pango_layout_new(ft->ct);
	pango_layout_set_text(ly,txt,-1);
	switch (align)
	{
		case alignBottom: 	 
			pango_layout_set_alignment(ly,PANGO_ALIGN_CENTER);	
			OffX+=(w/2)-(ft->width((const char*)txt)/2);
			OffY=h-ft->height((const char*)txt); 
			break;
		
		case alignBottomLeft: 		
			pango_layout_set_alignment(ly,PANGO_ALIGN_LEFT);
			OffX=0;
			OffY=h-ft->height((const char*)txt);
			break;
			
		case alignBottomNormal:  
			OffX=0;
			OffY=h-ft->height((const char*)txt);
			break;
			
		case alignBottomRight: 
			pango_layout_set_alignment(ly,PANGO_ALIGN_RIGHT);
			OffX=w-ft->height((const char*)txt);
			OffY=h-ft->height((const char*)txt);
			break;
			
		case alignCenter: 
			pango_layout_set_alignment(ly,PANGO_ALIGN_CENTER);
			OffX+=(w/2)-(ft->width((const char*)txt)/2);
			OffY+=(h/2)-(ft->height((const char*)txt)/2);
			break;
			
		case alignLeft:
			pango_layout_set_alignment(ly,PANGO_ALIGN_LEFT);
			OffX=0;
			OffY+=(h/2)-(ft->height((const char*)txt)/2);
			break;
			
		case alignNormal:
			OffX=0;
			OffY+=(h/2)-(ft->height((const char*)txt)/2);
			break;
			
		case alignRight:
			pango_layout_set_alignment(ly,PANGO_ALIGN_RIGHT);
			OffX=w-ft->height((const char*)txt);
			OffY+=(h/2)-(ft->height((const char*)txt)/2);
			break;
			
		case alignTop:
			pango_layout_set_alignment(ly,PANGO_ALIGN_CENTER);
			OffX+=(w/2)-(ft->width((const char*)txt)/2);
			OffY=0;
			break;
			
		case alignTopLeft:
			pango_layout_set_alignment(ly,PANGO_ALIGN_LEFT);
			OffX=0;
			OffY=0;
			break;
			
		case alignTopNormal:
			OffY=0;
			break;
			
		case alignTopRight:
			pango_layout_set_alignment(ly,PANGO_ALIGN_RIGHT);
			OffX=w-ft->height((const char*)txt);
			OffY=0; 
			break;

	}
	
	gdk_draw_layout(dr,gc,x+OffX,y+OffY,ly);
	g_object_unref(G_OBJECT(ly));

}

/****************************************************************************************

gDrawingArea Widget

*****************************************************************************************/
gboolean gDA_Expose(GtkWidget *wid,GdkEventExpose *e,gDrawingArea *data)
{
	GtkLayout *ly=GTK_LAYOUT(data->widget);
	GtkShadowType shadow=GTK_SHADOW_NONE;
	gint w,h;

	if (data->berase) data->clear();
	
	if (data->buffer)
	{
		gdk_draw_drawable(ly->bin_window,data->gc,data->buffer,e->area.x, \
		                  e->area.y,e->area.x,e->area.y,e->area.width,e->area.height);
		
		
	}
	else
	{
		if (data->onExpose)
			data->onExpose(data,e->area.x,e->area.y,e->area.width,e->area.height);
	}

	if (data->btype)
	{
		switch(data->btype)
		{
			case BORDER_PLAIN:    shadow=GTK_SHADOW_ETCHED_OUT; break;
			case BORDER_SUNKEN:   shadow=GTK_SHADOW_IN; break;
			case BORDER_RAISED:   shadow=GTK_SHADOW_OUT; break;
			case BORDER_ETCHED:   shadow=GTK_SHADOW_ETCHED_IN; break;
		}

		gdk_drawable_get_size(ly->bin_window,&w,&h);
		gtk_paint_shadow(wid->style,ly->bin_window,GTK_STATE_NORMAL,shadow,
                                 &e->area,NULL,NULL,0,0,w,h);
	}
	
	return false;
}

gDrawingArea::gDrawingArea(gControl *parent) : gContainer(parent)
{
	gint ev;

	g_typ=Type_gDrawingArea;
	
	track=false;
	berase=false;
	btype=0;
	gc=NULL;
	buffer=NULL;
	widget=gtk_layout_new(0,0);
	border=widget;
		
	gtk_widget_add_events(border,GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK);
	gtk_widget_add_events(border,GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK);
	
	connectParent();
	initSignals();
	
	onExpose=NULL;
	g_signal_connect(G_OBJECT(widget),"expose-event",G_CALLBACK(gDA_Expose),(gpointer)this);
	resize(100,30);

}

void gDrawingArea::resize(long w,long h)
{
	GdkPixmap *buf;
	GdkGC *gc2;
	gint myw,myh;

	if (w<1) w=1;
	if (h<1) h=1;

	if (buffer)
	{
		gdk_drawable_get_size(buffer,&myw,&myh);
		if (myw > w) myw=w;
		if (myh > h) myh=h; 

		buf=gdk_pixmap_new(GTK_LAYOUT(widget)->bin_window,w,h,-1);
		gc2=gdk_gc_new(buf);
		gdk_gc_set_foreground(gc2,&widget->style->bg[GTK_STATE_NORMAL]);
		gdk_draw_rectangle(buf,gc2,true,0,0,w,h);

		gdk_draw_drawable(buf,gc2,buffer,0,0,0,0,myw,myh);
		g_object_unref(buffer);
		buffer=buf;
		g_object_unref(gc2);
	}

	gControl::resize(w,h);
}

bool gDrawingArea::canFocus()
{
	return GTK_WIDGET_CAN_FOCUS(border);
}

void gDrawingArea::setCanFocus(bool vl)
{
	if (vl) GTK_WIDGET_SET_FLAGS(border,GTK_CAN_FOCUS);
	else    GTK_WIDGET_UNSET_FLAGS(border,GTK_CAN_FOCUS);
}

bool gDrawingArea::getTracking()
{
	return track;
}

void gDrawingArea::setTracking(bool vl)
{
	track=vl;
}

int gDrawingArea::getBorder()
{
	return btype;
}

void gDrawingArea::setBorder(int vl)
{
	GdkRectangle rect={0,0,0,0};

	if (btype==vl) return;

	switch(vl)
	{
		case BORDER_NONE:
		case BORDER_PLAIN:
		case BORDER_SUNKEN:
		case BORDER_RAISED:
		case BORDER_ETCHED:
			break;
		default: 
			return;
	}

	btype=vl;
	if (GTK_LAYOUT(border)->bin_window)
	{
		gdk_drawable_get_size(GTK_LAYOUT(border)->bin_window,&rect.width,&rect.height);
		gdk_window_invalidate_rect(GTK_LAYOUT(border)->bin_window,&rect,TRUE);
	}
}

long gDrawingArea::backGround()
{
	return get_gdk_bg_color(widget);
}

void gDrawingArea::setBackGround(long color)
{
	set_gdk_bg_color(widget,color);	
	if (!GTK_LAYOUT(widget)->bin_window) gtk_widget_realize(widget);
	gdk_window_process_updates(GTK_LAYOUT(widget)->bin_window,true);
	if (cached()) clear();
}

long gDrawingArea::foreGround()
{
	return get_gdk_fg_color(widget);
}

void gDrawingArea::setForeGround(long color)
{	
	set_gdk_text_color(widget,color);
	set_gdk_fg_color(widget,color);
	if (!GTK_LAYOUT(widget)->bin_window) gtk_widget_realize(widget);
	gdk_window_process_updates(GTK_LAYOUT(widget)->bin_window,true);
}


bool gDrawingArea::cached()
{
	return (bool)buffer;
}

void gDrawingArea::setCached(bool vl)
{
	GdkGC *gc2;
	
	if (vl == cached()) return;	
	
	if (!vl)
	{
		g_object_unref(G_OBJECT(gc));
		g_object_unref(G_OBJECT(buffer));
		buffer=NULL;
		gc=NULL;
		return;
	}
	
	buffer=gdk_pixmap_new(GTK_LAYOUT(widget)->bin_window,width(),height(),-1);
	gc2=gdk_gc_new(buffer);
	gdk_gc_set_foreground(gc2,&widget->style->bg[GTK_STATE_NORMAL]);
	gdk_draw_rectangle(buffer,gc2,true,0,0,width(),height());
	g_object_unref(G_OBJECT(gc2));
	gc=gdk_gc_new(buffer);

}

void gDrawingArea::clear()
{
	GdkGC *gc2;
	
	if (buffer) 
	{
		gc2=gdk_gc_new(buffer);
		gdk_gc_set_foreground(gc2,&widget->style->bg[GTK_STATE_NORMAL]);
		gdk_draw_rectangle(buffer,gc2,true,0,0,width(),height());
		g_object_unref(G_OBJECT(gc2));
		return;
	}
	
	gdk_window_clear(GTK_LAYOUT(border)->bin_window);
	
	
}

