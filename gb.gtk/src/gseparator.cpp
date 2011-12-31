/***************************************************************************

  gseparator.cpp

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
#include "widgets_private.h"
#include "gdesktop.h"
#include "gseparator.h"

gboolean gSeparator_expose(GtkWidget *wid, GdkEventExpose *e, gSeparator *data)
{
	gint w, h;

	w = data->width();
	h = data->height();
	
	if (w == 1 || h == 1)
	{
		GdkGC *gc;
		GdkGCValues values;

		fill_gdk_color(&values.foreground, gDesktop::lightfgColor(), gdk_drawable_get_colormap(wid->window));
		gc = gtk_gc_get(gdk_drawable_get_depth(wid->window), gdk_drawable_get_colormap(wid->window), &values, GDK_GC_FOREGROUND);
		
		gdk_draw_rectangle(wid->window, gc, TRUE, e->area.x, e->area.y, e->area.width, e->area.height);
		gtk_gc_release(gc);
	}
	else if (w>=h)
		gtk_paint_hline(wid->style,wid->window,GTK_STATE_NORMAL,&e->area,wid,NULL,0,w,h/2);
	else
		gtk_paint_vline(wid->style,wid->window,GTK_STATE_NORMAL,&e->area,wid,NULL,0,h,w/2);

	return false;
} 

gSeparator::gSeparator(gContainer *parent) : gControl(parent)
{
	g_typ=Type_gSeparator;
	
	border=gtk_event_box_new();
	widget=gtk_drawing_area_new();
	gtk_container_add(GTK_CONTAINER(border),widget);

	gtk_widget_add_events(widget,GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK);
	gtk_widget_add_events(widget,GDK_POINTER_MOTION_MASK);
	
	connectParent();
	initSignals();

	g_signal_connect(G_OBJECT(widget),"expose-event",G_CALLBACK(gSeparator_expose),(gpointer)this);
}


