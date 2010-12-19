/***************************************************************************

  gdrawingarea.cpp

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
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

***************************************************************************/

#include "widgets.h"
#include "gdrawingarea.h"

/****************************************************************************************

gDrawingArea Widget

*****************************************************************************************/

static gboolean cb_expose(GtkWidget *wid, GdkEventExpose *e, gDrawingArea *data)
{
	if (data->cached())
	{
		gdk_window_clear(GTK_WIDGET(wid)->window);
	}
	else
	{
		if (data->onExpose)
			data->onExpose(data,e->area.x,e->area.y,e->area.width,e->area.height);
		data->drawBorder();
	}

	return false;
}

static void cb_size(GtkWidget *wid, GtkAllocation *a, gDrawingArea *data)
{
	data->updateCache();
}

static gboolean cb_button_press(GtkWidget *wid, GdkEventButton *event, gDrawingArea *data)
{
	if (data->canFocus())
		data->setFocus();
	
	return false;
}

gDrawingArea::gDrawingArea(gContainer *parent) : gContainer(parent)
{
	g_typ = Type_gDrawingArea;
	
	_cached = false;
	buffer = NULL;
	_old_bg_id = 0;
	_resize_cache = false;
	_transparent = false;
	
	border = gtk_event_box_new();
	widget = gtk_fixed_new();
	//widget = border; //gtk_layout_new(0,0);
		
  realize(false);
  
	//gtk_event_box_set_visible_window(GTK_EVENT_BOX(border), false);	
  
	gtk_widget_add_events(widget, GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK
		| GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK
		| GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE_MASK 
		| GDK_SCROLL_MASK | GDK_POINTER_MOTION_MASK);

	//GTK_WIDGET_UNSET_FLAGS(border, GTK_APP_PAINTABLE);
	//GTK_WIDGET_SET_FLAGS(widget, GTK_CAN_FOCUS);
		
	_event_mask = gtk_widget_get_events(widget);
	
	onExpose = NULL;
	g_signal_connect(G_OBJECT(widget), "expose-event", G_CALLBACK(cb_expose), (gpointer)this);
	g_signal_connect(G_OBJECT(widget), "size-allocate", G_CALLBACK(cb_size), (gpointer)this);
	g_signal_connect(G_OBJECT(border), "button-press-event",G_CALLBACK(cb_button_press),(gpointer)this);
  
	setTransparent(true);
	
	//resize(100,30);
}

gDrawingArea::~gDrawingArea()
{
  setCached(false);
}

void gDrawingArea::resize(int w, int h)
{
	// TODO Do not resize cache if the DrawingArea is being painted
	gContainer::resize(w,h);
	//updateCache();
}

bool gDrawingArea::canFocus() const
{
	return GTK_WIDGET_CAN_FOCUS(widget);
}

void gDrawingArea::setCanFocus(bool vl)
{
	gtk_widget_set_can_focus(widget, vl);
}

void gDrawingArea::updateEventMask()
{
	static int event_mask;
	XWindowAttributes attr;
		
	gtk_widget_realize(border);
	
	if (!enabled())
	{
		XGetWindowAttributes(gdk_display, GDK_WINDOW_XID(border->window), &attr);
		event_mask = attr.your_event_mask;
		XSelectInput(gdk_display, GDK_WINDOW_XID(border->window), ExposureMask);
	}
	else
	{
		XSelectInput(gdk_display, GDK_WINDOW_XID(border->window), event_mask);
	}
	
}

void gDrawingArea::setEnabled(bool vl)
{
	gContainer::setEnabled(vl);
	updateEventMask();
}

void gDrawingArea::setCached(bool vl)
{
	if (vl == cached()) return;	
	
	_cached = vl;
	
	gtk_widget_set_double_buffered(widget, !_cached);
	
	if (!_cached)
	{
		g_object_unref(G_OBJECT(buffer));
		buffer = NULL;
		set_gdk_bg_color(widget, background());	
		return;
	}
	
	gtk_widget_realize(widget);
	resizeCache();
}

void gDrawingArea::resizeCache()
{
	int bw, bh;
	int w, h;
	GdkPixmap *buf;
	GdkGC *gc2;
	GdkWindow *win;
	
	win = GTK_WIDGET(widget)->window;
	if (!win)
		return;
	
	//gdk_drawable_get_size(border->window, &w, &h);
	w = width();
	h = height();
	
	if (buffer)
		gdk_drawable_get_size(buffer, &bw, &bh);
	else
		bw = bh = 0;
	
	if (bw != w || bh != h)
	{		
		buf = gdk_pixmap_new(win, w, h, -1);
		gc2 = gdk_gc_new(buf);
		gdk_gc_set_foreground(gc2, &widget->style->bg[GTK_STATE_NORMAL]);
		
		if (w > bw || h > bh || !buffer)
			gdk_draw_rectangle(buf, gc2, true, 0, 0, w, h);
	
		if (buffer)
		{
			if (bw > w) bw = w;
			if (bh > h) bh = h;
			gdk_draw_drawable(buf, gc2, buffer, 0, 0, 0, 0, bw, bh);
			g_object_unref(buffer);
		}
		
		buffer = buf;
		g_object_unref(gc2);
	}	
}

void gDrawingArea::setCache()
{
	GdkWindow *win;
	
	win = GTK_WIDGET(widget)->window;
	if (!win)
		return;
	
	drawBorder(buffer);
	gdk_window_set_back_pixmap(win, buffer, FALSE);
	//gdk_window_set_back_pixmap(border->window, buffer, FALSE);
	//gdk_window_set_back_pixmap(widget->window, NULL, TRUE);
	//gdk_window_set_back_pixmap(win, NULL, TRUE);
	refreshCache();
}

static gboolean resize_cache(gDrawingArea *data)
{
	//fprintf(stderr, "resize_cache\n");
	data->resizeCache();
	data->setCache();
	data->_resize_cache = false;
	return false;
}

void gDrawingArea::updateCache()
{
	if (!_cached)
		return;
	
	if (!_resize_cache)
	{
		_resize_cache = true;
		g_timeout_add(10, (GSourceFunc)resize_cache, (gpointer)this);
	}
}

void gDrawingArea::clear()
{
	GdkGC *gc2;
	
	if (_cached && buffer) 
	{
		gc2=gdk_gc_new(buffer);
		gdk_gc_set_foreground(gc2,&widget->style->bg[GTK_STATE_NORMAL]);
		gdk_draw_rectangle(buffer,gc2,true,0,0,width(),height());
		g_object_unref(G_OBJECT(gc2));
		drawBorder(buffer);
		refreshCache();
		return;
	}
	
	refresh(); //gdk_window_clear(GTK_LAYOUT(widget)->bin_window);
}

void gDrawingArea::refreshCache()
{
	if (_cached)
	{
		if (GTK_WIDGET(widget)->window)
			gdk_window_clear(GTK_WIDGET(widget)->window);
	}
}

void gDrawingArea::setTransparent(bool vl)
{
	_transparent = vl;
	//gtk_widget_set_double_buffered(widget, !_transparent);
	//gtk_widget_set_app_paintable(border, _transparent);
}
