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
#include "gdrawingarea.h"

/****************************************************************************************

gDrawingArea Widget

*****************************************************************************************/

static gboolean gDA_Expose(GtkWidget *wid,GdkEventExpose *e,gDrawingArea *data)
{
	GtkLayout *ly=GTK_LAYOUT(data->widget);
	
	if (data->berase) 
		data->clear();
	
	if (data->buffer)
	{
		gdk_draw_drawable(ly->bin_window,data->gc,data->buffer,e->area.x,
		                  e->area.y,e->area.x,e->area.y,e->area.width,e->area.height);
	}
	else
	{
		if (data->onExpose)
			data->onExpose(data,e->area.x,e->area.y,e->area.width,e->area.height);
	}

	return false;
}

gDrawingArea::gDrawingArea(gContainer *parent) : gContainer(parent)
{
	g_typ=Type_gDrawingArea;
	
	track=false;
	berase=false;
	gc=NULL;
	buffer=NULL;
	border=gtk_event_box_new();
	widget=gtk_layout_new(0,0);
		
  realize(true);
  
	gtk_widget_add_events(widget, GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK
		| GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK
		| GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE_MASK);
	
	onExpose=NULL;
	g_signal_connect(G_OBJECT(widget),"expose-event",G_CALLBACK(gDA_Expose),(gpointer)this);
	//resize(100,30);
}

gDrawingArea::~gDrawingArea()
{
  setCached(false);
}

void gDrawingArea::resize(int w, int h)
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
	return GTK_WIDGET_CAN_FOCUS(widget);
}

void gDrawingArea::setCanFocus(bool vl)
{
	if (vl) GTK_WIDGET_SET_FLAGS(widget,GTK_CAN_FOCUS);
	else    GTK_WIDGET_UNSET_FLAGS(widget,GTK_CAN_FOCUS);
}

bool gDrawingArea::getTracking()
{
	return track;
}

void gDrawingArea::setTracking(bool vl)
{
	track=vl;
}

/*int gDrawingArea::getBorder()
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
}*/

long gDrawingArea::background()
{
	return get_gdk_bg_color(widget);
}

void gDrawingArea::setBackground(long color)
{
	set_gdk_bg_color(widget,color);	
	if (!GTK_LAYOUT(widget)->bin_window) gtk_widget_realize(widget);
	gdk_window_process_updates(GTK_LAYOUT(widget)->bin_window,true);
	if (cached()) clear();
}

long gDrawingArea::foreground()
{
	return get_gdk_fg_color(widget);
}

void gDrawingArea::setForeground(long color)
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
	
	gtk_widget_set_double_buffered(widget, !vl);
	
	if (!vl)
	{
		g_object_unref(G_OBJECT(gc));
		g_object_unref(G_OBJECT(buffer));
		buffer=NULL;
		gc=NULL;
		return;
	}
	
	gtk_widget_realize(widget);
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
	
	berase = false;
	
	if (buffer) 
	{
		gc2=gdk_gc_new(buffer);
		gdk_gc_set_foreground(gc2,&widget->style->bg[GTK_STATE_NORMAL]);
		gdk_draw_rectangle(buffer,gc2,true,0,0,width(),height());
		g_object_unref(G_OBJECT(gc2));
		return;
	}
	
	gdk_window_clear(GTK_LAYOUT(widget)->bin_window);
}

