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
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
	MA 02110-1301, USA.

***************************************************************************/

#include "widgets.h"
#include "widgets_private.h"
#include "gframe.h"

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
	widget = gtk_fixed_new();
	frame = widget;
	realize(true);
}


/****************************************************************************

Frame

****************************************************************************/

gFrame::gFrame(gContainer *parent) : gContainer(parent)
{
	g_typ=Type_gFrame;

	border = gtk_event_box_new();
	widget = gtk_fixed_new();
	
	fr = gtk_frame_new(NULL);
	label = gtk_frame_get_label_widget(GTK_FRAME(fr));
	gtk_container_set_border_width(GTK_CONTAINER(fr),0);
	gtk_frame_set_shadow_type(GTK_FRAME(fr), GTK_SHADOW_ETCHED_IN);
	gtk_container_add(GTK_CONTAINER(widget), fr);
	
  realize(false);
  
	g_signal_connect(G_OBJECT(border), "size-allocate", G_CALLBACK(cb_frame_resize), (gpointer)this);
}

char *gFrame::text()
{
	if (!label) return (char *)"";
	return (char*)gtk_label_get_text(GTK_LABEL(label));
}

void gFrame::setText(char *vl)
{
	bool remove = false;
	
	remove = !vl || !*vl;
	
	if (remove)
	{
		if (label) 
		{
			gtk_frame_set_label_widget(GTK_FRAME(fr), NULL);
			label = NULL;
		}
		return;
	}
	
	if (!label)
	{
		label = gtk_label_new(vl);
		gtk_frame_set_label_widget(GTK_FRAME(fr), label);
		setFont(font());
		setForeground(foreground());
		gtk_widget_show(label);
	}
	else
		gtk_label_set_text(GTK_LABEL(label), (const gchar*)vl);
}

void gFrame::setFont(gFont *ft)
{
	gControl::setFont(ft);
	if (label)
		gtk_widget_modify_font(label, fnt ? fnt->desc() : NULL);
}

void gFrame::setRealForeground(gColor color)
{
	gControl::setRealForeground(color);
	if (label) set_gdk_fg_color(label, color);
}
