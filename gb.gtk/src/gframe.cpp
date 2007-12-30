/***************************************************************************

  gframe.cpp

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
#include "gframe.h"

// static void frame_show (GtkWidget *widget, gContainer *data)
// {
// 	data->performArrange();
// }

static void cb_frame_resize (GtkWidget *wid, GtkAllocation *req, gFrame *data)
{
	gtk_widget_set_size_request(data->fr, req->width, req->height);
} 


/****************************************************************************

Panel

****************************************************************************/

gPanel::gPanel(gContainer *parent) : gContainer(parent)
{
	g_typ = Type_gPanel;

	border = gtk_event_box_new();
	widget= gtk_layout_new(0,0);
	frame = widget;
	realize(true);
	
	//g_signal_connect(G_OBJECT(border),"size-allocate",G_CALLBACK(frame_resize),(gpointer)this);
	//g_signal_connect(G_OBJECT(border),"show",G_CALLBACK(frame_show),(gpointer)this);
}


// void gPanel::resize(int w, int h)
// {
// 	gControl::resize(w, h);
// }

/****************************************************************************

Frame

****************************************************************************/

gFrame::gFrame(gContainer *parent) : gContainer(parent)
{
	g_typ=Type_gFrame;

	border = gtk_event_box_new();
	widget=gtk_layout_new(0,0);
	
	fr = gtk_frame_new(NULL);
	gtk_container_set_border_width(GTK_CONTAINER(fr),0);
	gtk_frame_set_shadow_type(GTK_FRAME(fr), GTK_SHADOW_ETCHED_IN);
	//gtk_frame_set_label_widget(GTK_FRAME(frame),NULL);
	gtk_container_add(GTK_CONTAINER(widget), fr);
	
	//gtk_container_add(GTK_CONTAINER(widget),fr);
	//gtk_container_add(GTK_CONTAINER(border),widget);	

	gtk_widget_add_events(widget,GDK_POINTER_MOTION_MASK);
	gtk_widget_add_events(widget,GDK_BUTTON_RELEASE_MASK);
	
  realize(false);
  
	g_signal_connect(G_OBJECT(border), "size-allocate", G_CALLBACK(cb_frame_resize), (gpointer)this);
	//g_signal_connect(G_OBJECT(border), "show", G_CALLBACK(cb_frame_show), (gpointer)this);
}

#if 0
long gFrame::foreground()
{
	return get_gdk_fg_color(widget);
}

void gFrame::setForeground(long color)
{	
	GtkWidget *lbl;
	GtkWidget *fr;
	GList *chd;
	
	chd=gtk_container_get_children(GTK_CONTAINER(widget));
	fr=(GtkWidget*)chd->data;
	g_list_free(chd);
	
	lbl=gtk_frame_get_label_widget(GTK_FRAME(fr));
	if (lbl) set_gdk_fg_color(lbl,color);
	set_gdk_fg_color(widget,color);
	if (!border->window) gtk_widget_realize(border);
	gdk_window_process_updates(border->window,true);
}

long gFrame::background()
{
	return get_gdk_bg_color(border);
}

void gFrame::setBackground(long color)
{	
	GtkWidget *lbl;
	
	set_gdk_bg_color(border,color);
	set_gdk_bg_color(widget,color);
	if (!border->window) gtk_widget_realize(border);
	gdk_window_process_updates(border->window,true);
}
#endif

char* gFrame::text()
{
	GtkWidget *lbl;
	
	lbl = gtk_frame_get_label_widget(GTK_FRAME(fr));
	if (!lbl) return NULL;
	return (char*)gtk_label_get_text(GTK_LABEL(lbl));
}

void gFrame::setText(char *vl)
{
	GtkWidget *lbl;
	bool remove=false;
	
	remove = !vl || !*vl;
	
	lbl = gtk_frame_get_label_widget(GTK_FRAME(fr));
	
	if (remove)
	{
		if (lbl) gtk_frame_set_label_widget( GTK_FRAME(fr),NULL );
		return;
	}
	
	if (!lbl)
	{
		lbl=gtk_label_new(vl);
		gtk_frame_set_label_widget(GTK_FRAME(fr), lbl);
		gtk_widget_show(lbl);
		return;
	}
	
	gtk_label_set_text(GTK_LABEL(lbl), (const gchar*)vl);
}

// void gFrame::resize(long w,long h)
// {
// 	gControl::resize(w,h);
//   gtk_widget_set_size_request(fr,w,h);
// }
