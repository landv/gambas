/***************************************************************************

  gslider.cpp

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
#include "widgets_private.h"
#include "gdraw.h"
#include "gscrollbar.h"
#include "gslider.h"

void slider_Change(GtkRange *range,gSlider *data)
{
	if (data->onChange) data->onChange(data);
}

gboolean slider_Expose(GtkWidget *widget,GdkEventExpose *event,gSlider *data)
{
	GtkAdjustment* adj=gtk_range_get_adjustment(GTK_RANGE(data->widget));
	int max=(int)(adj->upper-adj->lower);
	int b;
	int fact=1;
	int myh;
	
	if (!data->bDraw) return false;
	
	if ( GTK_WIDGET_TYPE(data->widget)==GTK_TYPE_HSCALE )
	{
		myh=(data->height()-20)/2;
		if (myh<=0) myh=1;
		if (max) fact = data->width()/max;
		gDraw *dr=new gDraw();
		dr->connect(data);
		dr->setForeground(get_gdk_fg_color(data->border));
		for (b=0;b<data->width();b+=data->p_step)
		{
			dr->line(b*fact,0,b*fact,myh);
			dr->line(b*fact,data->height(),b*fact,data->height()-myh);
		}
		dr->disconnect();
		delete dr;
	}
	else
	{
		myh=(data->width()-20)/2;
		if (myh<=0) myh=1;
		if (max) fact = data->height()/max;
		gDraw *dr=new gDraw();
		dr->connect(data);
		dr->setForeground(get_gdk_fg_color(data->border));
		for (b=0;b<data->height();b+=data->p_step)
		{
			dr->line(0,b*fact,myh,b*fact);
			dr->line(data->width(),b*fact,data->width()-myh,b*fact);
		}
		dr->disconnect();
		delete dr;
	}
	
	
	return false;
}

gSlider::gSlider(gContainer *par) : gControl(par)
{	
	bDraw=false;
	g_typ=Type_gSlider;
	border=gtk_event_box_new();
	widget=gtk_vscale_new_with_range(0,99,1);
		
	gtk_container_add (GTK_CONTAINER(border),widget);
	gtk_scale_set_draw_value(GTK_SCALE(widget),false);
	p_step=1;
	p_page=10;
	gtk_range_set_increments(GTK_RANGE(widget),p_step,p_page);
	connectParent();
	initSignals();
	
	onChange=NULL;
	g_signal_connect(G_OBJECT(widget),"value-changed",G_CALLBACK(slider_Change),(gpointer)this);
	g_signal_connect_after(G_OBJECT(border),"expose-event",G_CALLBACK(slider_Expose),(gpointer)this);
}

gScrollBar::gScrollBar(gContainer *par) : gSlider(par)
{
	GtkAdjustment* adj=gtk_range_get_adjustment(GTK_RANGE(widget));
	GtkWidget *box;
	GtkWidget *lbl;
	
	
	g_typ=Type_gScrollBar;
	g_object_ref(adj);
	gtk_widget_destroy(widget);
	
	box=gtk_vbox_new(false,0);
	lbl=gtk_label_new("");
	widget=gtk_hscrollbar_new(adj);
	g_object_unref(adj);
	
	gtk_box_pack_start(GTK_BOX(box),widget,false,0,0);
    gtk_box_pack_start(GTK_BOX(box),lbl,true,0,0);
	gtk_container_add (GTK_CONTAINER(border),box);
	
	setForeground(parent()->foreground());
	setBackground(parent()->background());
	gtk_widget_show_all(box);
	gtk_range_set_update_policy(GTK_RANGE(widget),GTK_UPDATE_CONTINUOUS);
	g_signal_connect(G_OBJECT(widget),"value-changed",G_CALLBACK(slider_Change),(gpointer)this);
	
}

bool gSlider::mark()
{
	return bDraw;
}

void gSlider::setMark(bool vl)
{
	if (vl==bDraw) return;
	
	bDraw=vl;
	gtk_widget_queue_draw(widget);
}

int gSlider::step()
{
	return p_step;
}

int gSlider::pageStep()
{
	return p_page;
}

void gSlider::setStep(int vl)
{
	if (vl<1) vl=1;
	p_step=vl;
	gtk_range_set_increments(GTK_RANGE(widget),p_step,p_page);
	if (bDraw) gtk_widget_queue_draw(widget);
}

void gSlider::setPageStep(int vl)
{
	if (vl<1) vl=1;
	if (vl==p_page) return;
	p_page=vl;
	gtk_range_set_increments(GTK_RANGE(widget),p_step,p_page);
}

void gSlider::setBackground(int color)
{
	set_gdk_bg_color(border,color);
	set_gdk_bg_color(widget,color);
	if (!border->window) gtk_widget_realize(border);
	gdk_window_process_updates(border->window,true);
}

int gSlider::background()
{
	return get_gdk_bg_color(border);
}

void gSlider::setForeground(int color)
{
	set_gdk_fg_color(border,color);
	set_gdk_fg_color(widget,color);
		
	if (!border->window) gtk_widget_realize(border);
	gdk_window_process_updates(border->window,true);
	if (bDraw) gtk_widget_queue_draw(widget);
}

int gSlider::foreground()
{
	return get_gdk_fg_color(border);
}

int gSlider::max()
{
	GtkAdjustment* adj=gtk_range_get_adjustment(GTK_RANGE(widget));
	return (int)adj->upper;
}

int gSlider::min()
{
	GtkAdjustment* adj=gtk_range_get_adjustment(GTK_RANGE(widget));
	return (int)adj->lower;
}

int gSlider::value()
{
	return (int)gtk_range_get_value(GTK_RANGE(widget));
}
	
void gSlider::setMax(int vl)
{
	GtkAdjustment* adj=gtk_range_get_adjustment(GTK_RANGE(widget));
	
	if (adj->lower>vl) g_object_set((gpointer)adj,"lower",(double)vl,(void *)NULL);
	g_object_set((gpointer)adj,"upper",(double)vl,(void *)NULL);
	setValue(value());
}

void gSlider::setMin(int vl)
{
	GtkAdjustment* adj=gtk_range_get_adjustment(GTK_RANGE(widget));
	
	if (adj->upper<vl) g_object_set((gpointer)adj,"upper",(double)vl,(void *)NULL);
	g_object_set((gpointer)adj,"lower",(double)vl,(void *)NULL);
	setValue(value());
}

bool gSlider::tracking()
{
	if ( gtk_range_get_update_policy(GTK_RANGE(widget))==GTK_UPDATE_DISCONTINUOUS) return false;
	return true;
}

void gSlider::setTracking(bool vl)
{
	if (vl) gtk_range_set_update_policy(GTK_RANGE(widget),GTK_UPDATE_CONTINUOUS);
	else gtk_range_set_update_policy(GTK_RANGE(widget),GTK_UPDATE_DISCONTINUOUS);
}

void gSlider::setValue(int vl)
{
	GtkAdjustment* adj=gtk_range_get_adjustment(GTK_RANGE(widget));
	
	if (vl < adj->lower) vl=(int)adj->lower;
	if (vl > adj->upper) vl=(int)adj->upper;
	
	gtk_adjustment_set_value(adj,(gdouble)vl);
}


void gSlider::orientation(int w,int h)
{
	GtkAdjustment* adj;
	bool trk;
	
	if (w<h)
	{
		if (G_OBJECT_TYPE(widget)==GTK_TYPE_HSCALE)
		{
			trk=tracking();
			adj=gtk_range_get_adjustment(GTK_RANGE(widget));
			g_object_ref(adj);
			
		 	gtk_widget_destroy(widget);
			
			widget=gtk_vscale_new(adj);
			gtk_container_add (GTK_CONTAINER(border),widget);
			
			gtk_scale_set_draw_value(GTK_SCALE(widget),false);
			gtk_widget_show(widget);
			widgetSignals();
			g_signal_connect(G_OBJECT(widget),"value-changed",G_CALLBACK(slider_Change),(gpointer)this);
			setBackground(background());
			setTracking(trk);
			g_object_unref(adj);	
		}
	}
	else
	{
		if (G_OBJECT_TYPE(widget)==GTK_TYPE_VSCALE)
		{
			trk=tracking();
			adj=gtk_range_get_adjustment(GTK_RANGE(widget));
			g_object_ref(adj);
		 	gtk_widget_destroy(widget);
			widget=gtk_hscale_new(adj);
			gtk_container_add (GTK_CONTAINER(border),widget);
			gtk_scale_set_draw_value(GTK_SCALE(widget),false);
			gtk_widget_show(widget);
			widgetSignals();
			g_signal_connect(G_OBJECT(widget),"value-changed",G_CALLBACK(slider_Change),(gpointer)this);
			setBackground(background());
			setTracking(trk);
			g_object_unref(adj);
			
		}
	}
}

void gSlider::resize(int w, int h)
{
	gControl::resize(w, h);
	orientation(width(), height());
}

void gScrollBar::resize(int w, int h)
{
	GtkWidget *box,*lbl;
	GtkAdjustment* adj;
	bool trk;
	
	gControl::resize(w, h);
	
	if (w<h)
	{
		if (G_OBJECT_TYPE(widget)==GTK_TYPE_HSCROLLBAR)
		{
			trk=tracking();
			adj=gtk_range_get_adjustment(GTK_RANGE(widget));
			g_object_ref(adj);
		 	
			
			box=gtk_bin_get_child(GTK_BIN(border));
			gtk_widget_destroy(box);
			
			lbl=gtk_label_new("");
			box=gtk_hbox_new(false,0);
			widget=gtk_vscrollbar_new(adj);
			gtk_box_pack_start(GTK_BOX(box),widget,false,false,0);
			gtk_box_pack_start(GTK_BOX(box),lbl,true,false,0);
			gtk_container_add (GTK_CONTAINER(border),box);
			gtk_widget_show_all(box);
			
			widgetSignals();
			g_signal_connect(G_OBJECT(widget),"value-changed",G_CALLBACK(slider_Change),(gpointer)this);
			setBackground(background());
			setTracking(trk);
			g_object_unref(adj);	
		}
	}
	else
	{
		if (G_OBJECT_TYPE(widget)==GTK_TYPE_VSCROLLBAR)
		{
			trk=tracking();
			adj=gtk_range_get_adjustment(GTK_RANGE(widget));
			g_object_ref(adj);
		 	
			box=gtk_bin_get_child(GTK_BIN(border));
			gtk_widget_destroy(box);
			
			lbl=gtk_label_new("");
			box=gtk_vbox_new(false,0);
			widget=gtk_hscrollbar_new(adj);
			gtk_box_pack_start(GTK_BOX(box),widget,false,false,0);
			gtk_box_pack_start(GTK_BOX(box),lbl,true,false,0);
			gtk_container_add (GTK_CONTAINER(border),box);
			gtk_widget_show_all(box);
			
			
			widgetSignals();
			g_signal_connect(G_OBJECT(widget),"value-changed",G_CALLBACK(slider_Change),(gpointer)this);
			setBackground(background());
			setTracking(trk);
			g_object_unref(adj);
			
		}
	}
}
