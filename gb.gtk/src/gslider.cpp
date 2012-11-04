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
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
  MA 02110-1301, USA.

***************************************************************************/

#include "widgets.h"
#include "widgets_private.h"
#include "gdraw.h"
#include "gscrollbar.h"
#include "gslider.h"

void slider_Change(GtkRange *range,gSlider *data)
{
	data->_value = gtk_range_get_value(GTK_RANGE(data->widget));
	if (data->onChange) data->onChange(data);
}

gboolean slider_Expose(GtkWidget *widget,GdkEventExpose *event,gSlider *data)
{
	GtkAdjustment* adj=gtk_range_get_adjustment(GTK_RANGE(data->widget));
	int max=(int)(adj->upper-adj->lower);
	int b;
	int myh;
	int step;
	
	if (!data->_mark || !max) return false;
	
	if ( GTK_WIDGET_TYPE(data->widget)==GTK_TYPE_HSCALE )
	{
		step = data->_page_step * data->width() / max;
		if (!step)
			return false;
		
		while (step < 4)
			step *= 2;
		
		myh=(data->height()-20)/2;
		if (myh<=0) myh=1;
		gDraw *dr=new gDraw();
		dr->connect(data);
		dr->setForeground(get_gdk_fg_color(data->border, data->enabled()));
		
		for(b = step; b <= (data->width() - step); b += step)
		{
			dr->line(b, 0, b,myh);
			dr->line(b, data->height(), b,data->height()-myh);
		}
		dr->disconnect();
		delete dr;
	}
	else
	{
		step = data->_page_step * data->height() / max;
		if (!step)
			return false;
		
		while (step < 4)
			step *= 2;
		
		myh=(data->width()-20)/2;
		if (myh<=0) myh=1;
		gDraw *dr=new gDraw();
		dr->connect(data);
		dr->setForeground(get_gdk_fg_color(data->border, data->enabled()));
		
		for(b = 0; b < data->height(); b += step)
		{
			dr->line(0, b, myh, b);
			dr->line(data->width(), b, data->width() - myh, b);
		}
		dr->disconnect();
		delete dr;
	}
	
	
	return false;
}

void gSlider::init()
{
	GtkAdjustment* adj = gtk_range_get_adjustment(GTK_RANGE(widget));
	
	if (_min == _max)
		_max = _min + 1;
	
	if (_value < _min)
		_value = _min;
	else if (_value > _max)
		_value = _max;
	
	if (g_typ == Type_gSlider)
	{
		gtk_range_set_range(GTK_RANGE(widget), (gdouble)_min, (gdouble)_max);
		gtk_range_set_increments(GTK_RANGE(widget), (gdouble)_step, (gdouble)_page_step);
	}
	else
	{
		gtk_range_set_range(GTK_RANGE(widget), (gdouble)_min, (gdouble)_max + _page_step);
		gtk_range_set_increments(GTK_RANGE(widget), (gdouble)_step, (gdouble)_page_step);
		gtk_adjustment_set_page_size(adj, _page_step);
	}
	gtk_range_set_value(GTK_RANGE(widget), _value);
	gtk_range_set_update_policy(GTK_RANGE(widget), _tracking ? GTK_UPDATE_CONTINUOUS : GTK_UPDATE_DISCONTINUOUS);
}

gSlider::gSlider(gContainer *par, bool scrollbar) : gControl(par)
{	
	g_typ = Type_gSlider;
	
	_mark = false;
	_step = 1;
	_page_step = 10;
	_value = 0;
	_min = 0;
	_max = 100;
	_tracking = true;
	
	border = gtk_alignment_new(0,0,1,1);
	
	if (scrollbar)
		return;
	
	widget = gtk_vscale_new(NULL);
	gtk_scale_set_draw_value(GTK_SCALE(widget), false);
		
	init();
	
	realize(false);
	
	onChange = NULL;
	
	g_signal_connect(G_OBJECT(widget),"value-changed",G_CALLBACK(slider_Change),(gpointer)this);
	g_signal_connect_after(G_OBJECT(border),"expose-event",G_CALLBACK(slider_Expose),(gpointer)this);
}

gScrollBar::gScrollBar(gContainer *par) : gSlider(par, true)
{
	g_typ = Type_gScrollBar;
	widget = gtk_hscrollbar_new(NULL);
	realize(false);
	
	init();
	onChange = NULL;
	
	gtk_range_set_update_policy(GTK_RANGE(widget),GTK_UPDATE_CONTINUOUS);
	g_signal_connect(G_OBJECT(widget),"value-changed",G_CALLBACK(slider_Change),(gpointer)this);
}

bool gSlider::mark()
{
	return _mark;
}

void gSlider::setMark(bool vl)
{
	if (vl == _mark) return;
	
	_mark = vl;
	gtk_widget_queue_draw(widget);
}

int gSlider::step()
{
	return _step;
}

int gSlider::pageStep()
{
	return _page_step;
}

void gSlider::setStep(int vl)
{
	if (vl < 1) vl = 1;
	if (vl == _step) return;
	
	_step = vl;
	init();
	if (_mark) gtk_widget_queue_draw(widget);
}

void gSlider::setPageStep(int vl)
{
	if (vl < 1) vl = 1;
	if (vl == _page_step) return;
	
	_page_step = vl;
	init();
	if (_mark) gtk_widget_queue_draw(widget);
}

int gSlider::max()
{
	return _max;
}

int gSlider::min()
{
	return _min;
}

int gSlider::value()
{
	return _value;
}
	
void gSlider::setMax(int vl)
{
	_max = vl;
	if (_min > _max)
		_min = _max;
	init();
}

void gSlider::setMin(int vl)
{
	_min = vl;
	if (_min > _max)
		_max = _min;
	init();
}

bool gSlider::tracking()
{
	return _tracking;
}

void gSlider::setTracking(bool vl)
{
	_tracking = vl;
	init();
}

void gSlider::setValue(int vl)
{
	if (vl < _min)
		vl = _min;
	else if (vl > _max)
		vl = _max;
	
	_value = vl;
	init();
}


void gSlider::orientation(int w,int h)
{
	GtkAdjustment *adj;
	GtkType type;
	
	type = (w < h) ? GTK_TYPE_VSCALE : GTK_TYPE_HSCALE;
	
	if (type != G_OBJECT_TYPE(widget))
	{
		adj = gtk_range_get_adjustment(GTK_RANGE(widget));
		g_object_ref(adj);
		
		gtk_widget_destroy(widget);
		
		if (type == GTK_TYPE_VSCALE)
			widget = gtk_vscale_new(adj);
		else
			widget = gtk_hscale_new(adj);
		
		gtk_container_add(GTK_CONTAINER(border), widget);
		
		gtk_scale_set_draw_value(GTK_SCALE(widget), false);
		gtk_widget_show(widget);
		widgetSignals();
		g_signal_connect(G_OBJECT(widget),"value-changed",G_CALLBACK(slider_Change),(gpointer)this);
		
		g_object_unref(adj);
		
		init();
	}
}

void gSlider::resize(int w, int h)
{
	gControl::resize(w, h);
	orientation(width(), height());
}

void gScrollBar::resize(int w, int h)
{
	GtkAdjustment* adj;
	GType type;
	
	gControl::resize(w, h);
	
	type = (w < h) ? GTK_TYPE_VSCROLLBAR : GTK_TYPE_HSCROLLBAR;
	
	if (type != G_OBJECT_TYPE(widget))
	{
		adj = gtk_range_get_adjustment(GTK_RANGE(widget));
		g_object_ref(adj);

		gtk_widget_destroy(widget);
		
		if (type == GTK_TYPE_VSCROLLBAR)
			widget = gtk_vscrollbar_new(adj);
		else
			widget = gtk_hscrollbar_new(adj);
		
		gtk_container_add(GTK_CONTAINER(border), widget);
		gtk_widget_show(widget);
		widgetSignals();
		g_signal_connect(G_OBJECT(widget),"value-changed",G_CALLBACK(slider_Change),(gpointer)this);
		
		g_object_unref(adj);	
		
		init();
	}
}

int gSlider::getDefaultSize()
{
	GtkRequisition req;
	
	gtk_widget_size_request(GTK_WIDGET(widget), &req);
	
	if (width() < height())
		return req.width;
	else
		return req.height;
}
