/***************************************************************************

  gseparator.cpp

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
#include "gseparator.h"

gboolean gSeparator_expose(GtkWidget *wid,GdkEventExpose *e,gSeparator *data)
{
	gint w,h;

	gdk_drawable_get_size(wid->window,&w,&h);
	if (w>=h)
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

long gSeparator::foreground()
{
	return get_gdk_bg_color(widget);
}

long gSeparator::background()
{
	return get_gdk_bg_color(border);
}

void gSeparator::setForeground(long vl)
{
	set_gdk_fg_color(widget,vl);
	if (!border->window) gtk_widget_realize(border);
	gdk_window_process_updates(border->window,true);
}
	
void gSeparator::setBackground(long vl)
{
	set_gdk_bg_color(widget,vl);
	if (!border->window) gtk_widget_realize(border);
	gdk_window_process_updates(border->window,true);
}


