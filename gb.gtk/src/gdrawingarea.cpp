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
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
  MA 02110-1301, USA.

***************************************************************************/

#include "widgets.h"
#include "gmouse.h"
#include "gdrawingarea.h"

#ifdef GTK3
#define UNREF_BUFFER() (cairo_surface_destroy(buffer), buffer = NULL)
#else
#define UNREF_BUFFER() (g_object_unref(G_OBJECT(buffer)), buffer = NULL)
#endif


/****************************************************************************************

gDrawingArea Widget

*****************************************************************************************/

#ifdef GTK3
static gboolean cb_draw(GtkWidget *wid, cairo_t *cr, gDrawingArea *data)
{
	if (data->cached())
	{
		cairo_set_source_surface(cr, data->buffer, 0, 0);
		cairo_paint(cr);
		data->drawBorder(cr);
	}
	else
	{
		//data->drawBackground();

		if (data->onExpose)
		{
			data->_in_draw_event = true;
			data->onExpose(data, cr);
			data->_in_draw_event = false;
		}
		data->drawBorder(cr);
	}

	return false;
}
#else
static gboolean cb_expose(GtkWidget *wid, GdkEventExpose *e, gDrawingArea *data)
{
	if (data->cached())
	{
		data->drawBorder(e);
	}
	else
	{
		//data->drawBackground();
		
		if (data->onExpose)
		{
			data->_in_draw_event = true;

			/*GdkRectangle *rect;
			int i, n;
			gdk_region_get_rectangles(e->region, &rect, &n);

			for (i = 0; i < n; i++)
				fprintf(stderr, "[%d] %d %d %d %d\n", i, rect[i].x, rect[i].y, rect[i].width, rect[i].height);

			g_free(rect);*/

			data->onExpose(data, e->region, wid->allocation.x, wid->allocation.y);
			data->_in_draw_event = false;
		}
		data->drawBorder(e);
	}

	return false;
}
#endif

static void cb_size(GtkWidget *wid, GtkAllocation *a, gDrawingArea *data)
{
	data->updateCache();
}

/*static gboolean cb_button_press(GtkWidget *wid, GdkEventButton *event, gDrawingArea *data)
{
	if (data->canFocus())
		data->setFocus();
	
	return false;
}*/

void gDrawingArea::create(void)
{
	int i;
	GtkWidget *ch;
	bool doReparent = false;
	bool was_visible = isVisible();
	GdkRectangle rect;
	int bg, fg;
	
	if (border)
	{
		getGeometry(&rect);
		bg = background();
		fg = foreground();
		parent()->remove(this);
		
		for (i = 0; i < childCount(); i++)
		{
			ch = child(i)->border;
			g_object_ref(G_OBJECT(ch));
			gtk_container_remove(GTK_CONTAINER(widget), ch);
		}
		
		_no_delete = true;
		gtk_widget_destroy(border);
		_no_delete = false;
		doReparent = true;
	}
	
	if (_cached || _use_tablet)
	{
		border = gtk_event_box_new();
		widget = gtk_fixed_new();
		box = widget;
		gtk_widget_set_app_paintable(border, TRUE);
		gtk_widget_set_app_paintable(box, TRUE);
	}
	else
	{
		border = widget = gtk_fixed_new();
		box = NULL;
	}

	realize(false);
	
	g_signal_connect(G_OBJECT(border), "size-allocate", G_CALLBACK(cb_size), (gpointer)this);
	ON_DRAW_BEFORE(border, this, cb_expose, cb_draw);
	
	updateUseTablet();
	
	if (doReparent)
	{
		if (box)
			gtk_widget_realize(box);
				
		setBackground(bg);
		setForeground(fg);
		setFont(font());
		bufX = bufY = bufW = bufH = -1;
		setGeometry(&rect);
		
		for (i = 0; i < childCount(); i++)
		{
			ch = child(i)->border;
			gtk_container_add(GTK_CONTAINER(widget), ch);
			moveChild(child(i), child(i)->x(), child(i)->y());
			g_object_unref(G_OBJECT(ch));
		}
		
		if (was_visible)
			show();
		else
			hide();
	}
}

gDrawingArea::gDrawingArea(gContainer *parent) : gContainer(parent)
{
	_cached = false;
	buffer = NULL;
	box = NULL;
	_old_bg_id = 0;
	_resize_cache = false;
	_no_background = false;
	_use_tablet = false;
	
	onExpose = NULL;
	onFontChange = NULL;
		
	g_typ = Type_gDrawingArea;
	
	create();
}

gDrawingArea::~gDrawingArea()
{
	if (buffer)
		UNREF_BUFFER();
}

void gDrawingArea::resize(int w, int h)
{
	// TODO Do not resize cache if the DrawingArea is being painted
	gContainer::resize(w,h);
	//updateCache();
}

void gDrawingArea::updateEventMask()
{
	/*
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
	*/
}

void gDrawingArea::setEnabled(bool vl)
{
	gContainer::setEnabled(vl);
	updateEventMask();
}

void gDrawingArea::setCached(bool vl)
{
	if (vl == _cached) return;	
	
	_cached = vl;
	
	if (!_cached)
	{
		UNREF_BUFFER();
		set_gdk_bg_color(border, background());	
	}
	
	create();
	resizeCache();
}

void gDrawingArea::resizeCache()
{
	int bw, bh;
	int w, h;
#ifdef GTK3
	cairo_surface_t *buf;
#else
	GdkPixmap *buf;
#endif
	GdkWindow *win;
	cairo_t *cr;
	
	if (!_cached)
		return;
	
	win = gtk_widget_get_window(GTK_WIDGET(box));
	if (!win)
		return;
	
	w = width();
	h = height();
	
	if (buffer)
	{
#ifdef GTK3
		bw = cairo_image_surface_get_width(buffer);
		bh = cairo_image_surface_get_height(buffer);
#else
		gdk_drawable_get_size(buffer, &bw, &bh);
#endif
	}
	else
		bw = bh = 0;
	
	if (bw != w || bh != h)
	{
#ifdef GTK3
		buf = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, w, h);
		cr = cairo_create(buf);
#else
		buf = gdk_pixmap_new(win, w, h, -1);
		cr = gdk_cairo_create(buf);
#endif
		
		if (w > bw || h > bh || !buffer)
		{
			gt_cairo_set_source_color(cr, realBackground(true));
			cairo_rectangle(cr, 0, 0, w, h);
			cairo_fill(cr);
		}
	
		if (buffer)
		{
			if (bw > w) bw = w;
			if (bh > h) bh = h;
			//gdk_draw_drawable(buf, gc2, buffer, 0, 0, 0, 0, bw, bh);
#ifdef GTK3
			cairo_set_source_surface(cr, buffer, 0, 0);
#else
			gdk_cairo_set_source_pixmap(cr, buffer, 0, 0);
#endif
			cairo_rectangle(cr, 0, 0, bw, bh);
			cairo_fill(cr);

			UNREF_BUFFER();
		}
		
		buffer = buf;
		cairo_destroy(cr);
	}	
	
	//drawBorder(buffer);
	refreshCache();
}

void gDrawingArea::setCache()
{
	if (!_cached)
		return;

#ifdef GTK3
#else
	gdk_window_set_back_pixmap(gtk_widget_get_window(box), buffer, FALSE);
#endif
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
	if (_cached && buffer) 
	{
		UNREF_BUFFER();
		resizeCache();
		setCache();
	}
}

void gDrawingArea::refreshCache()
{
	gtk_widget_queue_draw(box);
	//if (box && box->window)
	//	gdk_window_clear(box->window);
}

void gDrawingArea::setNoBackground(bool vl)
{
	/*
	GdkWindow *win;
	
	gtk_widget_realize(widget);
	win = widget->window;
	
	if (vl)
		gdk_window_set_back_pixmap(win, NULL, FALSE);
	else
		setBackground(background());
	*/
}

void gDrawingArea::setRealBackground(gColor color)
{
	gControl::setRealBackground(color);
	clear();
}

void gDrawingArea::updateUseTablet()
{
	if (_use_tablet)
		gMouse::initDevices();
#ifndef GTK3
	gtk_widget_set_extension_events(border, _use_tablet ? GDK_EXTENSION_EVENTS_CURSOR : GDK_EXTENSION_EVENTS_NONE);
#endif
	//fprintf(stderr, "gtk_widget_set_extension_events: %s %p: %d\n", name(), border, _use_tablet);
}

void gDrawingArea::setUseTablet(bool vl)
{
	if (vl == _use_tablet)
		return;
	_use_tablet = vl;
	create();
}

void gDrawingArea::updateFont()
{
	gContainer::updateFont();
	emit(SIGNAL(onFontChange));
}
