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
#include "gdesktop.h"
#include "gscrollbar.h"
#include "gslider.h"

static void cb_change(GtkAdjustment *adj, gSlider *data)
{
	int new_value = gtk_adjustment_get_value(adj);

	if (data->_value == new_value)
		return;
	
	data->_value = new_value;
	if (data->onChange) 
		data->onChange(data);
}

void gSlider::update()
{
	GtkAdjustment* adj = gtk_range_get_adjustment(GTK_RANGE(widget));
	int value = _value;
	
	if (value < _min)
		value = _min;
	else if (value > _max)
		value = _max;
	
	if (g_typ == Type_gSlider)
	{
#ifndef GTK3
	if (_min == _max)
		_max = _min + 1;
#endif
		gtk_range_set_range(GTK_RANGE(widget), (gdouble)_min, (gdouble)_max);
		gtk_range_set_increments(GTK_RANGE(widget), (gdouble)_step, (gdouble)_page_step);
	}
	else
	{
		gtk_range_set_range(GTK_RANGE(widget), (gdouble)_min, (gdouble)_max + _page_step);
		gtk_range_set_increments(GTK_RANGE(widget), (gdouble)_step, (gdouble)_page_step);
		gtk_adjustment_set_page_size(adj, _page_step);
	}
	gtk_range_set_value(GTK_RANGE(widget), value);
#ifndef GTK3
	gtk_range_set_update_policy(GTK_RANGE(widget), _tracking ? GTK_UPDATE_CONTINUOUS : GTK_UPDATE_DISCONTINUOUS);
#endif

	checkInverted();
}

void gSlider::init()
{
	GtkAdjustment* adj = gtk_range_get_adjustment(GTK_RANGE(widget));

	_use_wheel = true;
	onChange = NULL;

	g_signal_connect(adj, "value-changed", G_CALLBACK(cb_change), (gpointer)this);
	//g_signal_connect(adj, "changed", G_CALLBACK(cb_change), (gpointer)this);
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

/*#ifdef GTK3
	border = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
#else	
	border = gtk_alignment_new(0, 0, 1, 1);
#endif*/
	
	if (scrollbar)
		return;
	
#ifdef GTK3
	widget = gtk_scale_new(GTK_ORIENTATION_VERTICAL, NULL);
	//gtk_widget_set_hexpand(widget, TRUE);
#else
	widget = gtk_vscale_new(NULL);
#endif
	gtk_scale_set_draw_value(GTK_SCALE(widget), false);
		
	init();
	update();
	realize(false);
	//g_signal_connect_after(G_OBJECT(border),"expose-event",G_CALLBACK(slider_Expose),(gpointer)this);
}

gScrollBar::gScrollBar(gContainer *par) : gSlider(par, true)
{
	g_typ = Type_gScrollBar;
#ifdef GTK3
	widget = gtk_scrollbar_new(GTK_ORIENTATION_HORIZONTAL, NULL);
#else
	widget = gtk_hscrollbar_new(NULL);
#endif
	
	init();
	update();
	realize(false);
	
#ifndef GTK3
	gtk_range_set_update_policy(GTK_RANGE(widget),GTK_UPDATE_CONTINUOUS);
#endif
}

bool gSlider::mark()
{
	return _mark;
}

void gSlider::updateMark()
{
	int i;
	int step;
	
	if (!_mark)
		return;

	gtk_scale_clear_marks(GTK_SCALE(widget));
	
	step = _page_step;
	while (step < ((_max - _min) / 20))
		step *= 2;
	
	for (i = _min; i <= _max; i += step)
		gtk_scale_add_mark(GTK_SCALE(widget), i, isVertical() ? GTK_POS_TOP : GTK_POS_RIGHT, NULL);
}

void gSlider::setMark(bool vl)
{
	
	if (vl == _mark) return;
	
	_mark = vl;
	gtk_scale_clear_marks(GTK_SCALE(widget));
	updateMark();
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
	update();
	if (_mark) gtk_widget_queue_draw(widget);
}

void gSlider::setPageStep(int vl)
{
	if (vl < 1) vl = 1;
	if (vl == _page_step) return;
	
	_page_step = vl;
	update();
	updateMark();
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
	update();
	updateMark();
}

void gSlider::setMin(int vl)
{
	_min = vl;
	if (_min > _max)
		_max = _min;
	update();
	updateMark();
}

bool gSlider::tracking()
{
	return _tracking;
}

void gSlider::setTracking(bool vl)
{
	_tracking = vl;
	update();
}

void gSlider::setValue(int vl)
{
	if (vl < _min)
		vl = _min;
	else if (vl > _max)
		vl = _max;
	
	if (_value == vl)
		return;
	
	_value = vl;
	update();
	
	emit(SIGNAL(onChange));
}

#if 0
void gSlider::orientation(int w,int h)
{
	//GtkAdjustment *adj;
	//GtkOrientation orient;
	
	gtk_orientable_set_orientation(GTK_ORIENTABLE(widget),  (w < h) ? GTK_ORIENTATION_VERTICAL : GTK_ORIENTATION_HORIZONTAL);

	/*if (orient != gtk_orientable_get_orientation(GTK_ORIENTABLE(widget)))
	{
		adj = gtk_range_get_adjustment(GTK_RANGE(widget));
		g_object_ref(adj);
		
		gtk_widget_destroy(widget);
		
		widget = gtk_scale_new(
		if (type == GTK_TYPE_VSCALE)
			widget = gtk_vscale_new(adj);
		else
			widget = gtk_hscale_new(adj);
		
		gtk_container_add(GTK_CONTAINER(border), widget);
		
		gtk_scale_set_draw_value(GTK_SCALE(widget), false);
		gtk_widget_show(widget);
		widgetSignals();
		g_signal_connect(G_OBJECT(widget), "value-changed", G_CALLBACK(cb_change), (gpointer)this);
		
		g_object_unref(adj);
		
		init();
	}*/
}
#endif

void gSlider::resize(int w, int h)
{
	gControl::resize(w, h);
	gtk_orientable_set_orientation(GTK_ORIENTABLE(widget),  (w < h) ? GTK_ORIENTATION_VERTICAL : GTK_ORIENTATION_HORIZONTAL);
}

void gScrollBar::resize(int w, int h)
{
	gControl::resize(w, h);
	gtk_orientable_set_orientation(GTK_ORIENTABLE(widget),  (w < h) ? GTK_ORIENTATION_VERTICAL : GTK_ORIENTATION_HORIZONTAL);
}

int gSlider::getDefaultSize()
{
	GtkRequisition req;
	
#ifdef GTK3
	gtk_widget_get_preferred_size(widget, &req, NULL);
#else
	gtk_widget_size_request(widget, &req);
#endif
	
	if (width() < height())
		return req.width;
	else
		return req.height;
}

bool gSlider::isVertical() const
{
	return gtk_orientable_get_orientation(GTK_ORIENTABLE(widget)) == GTK_ORIENTATION_VERTICAL;
}

void gSlider::checkInverted()
{
	gtk_range_set_inverted(GTK_RANGE(widget), !isVertical() && gDesktop::rightToLeft());
}
