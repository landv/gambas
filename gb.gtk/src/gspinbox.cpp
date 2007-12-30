/***************************************************************************

  glistbox.cpp

  (c) 2004-2005 - Daniel Campos Fernández <danielcampos@netcourrier.com>
  
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
#include <gtk/gtk.h>

void  spin_change(GtkSpinButton *spinbutton,gSpinBox *data)
{
	if (data->onChange) data->onChange(data);
}

gSpinBox::gSpinBox(gControl *parent) : gControl(parent)
{
	g_typ=Type_gSpinBox;
	border=gtk_spin_button_new_with_range(0,99,1);
	widget=border;
	connectParent();
	initSignals();
	onChange=NULL;
	g_signal_connect(G_OBJECT(widget),"value-changed",G_CALLBACK(spin_change),(gpointer)this);
	
}


long gSpinBox::backGround()
{
	return get_gdk_base_color(widget);
}

void gSpinBox::setBackGround(long color)
{
	set_gdk_base_color(widget,color);	
	if (!border->window) gtk_widget_realize(border);
	gdk_window_process_updates(border->window,true);
}

long gSpinBox::foreGround()
{
	return get_gdk_text_color(widget);
}

void gSpinBox::setForeGround(long color)
{	
	set_gdk_text_color(widget,color);
	if (!border->window) gtk_widget_realize(border);
	gdk_window_process_updates(border->window,true);
}
long gSpinBox::maxValue()
{
	gdouble max;

	gtk_spin_button_get_range(GTK_SPIN_BUTTON(widget),NULL,&max);
	return (long)max;
}

long gSpinBox::minValue()
{
	gdouble min;

	gtk_spin_button_get_range(GTK_SPIN_BUTTON(widget),&min,NULL);
	return (long)min;
}

long gSpinBox::step()
{
	gdouble step;
	
	gtk_spin_button_get_increments(GTK_SPIN_BUTTON(widget),&step,NULL);
	return (long)step;
}

long gSpinBox::value()
{
	return (long)gtk_spin_button_get_value(GTK_SPIN_BUTTON(widget));
}

bool gSpinBox::wrap()
{
	return gtk_spin_button_get_wrap(GTK_SPIN_BUTTON(widget));
}
	
void gSpinBox::setMaxValue(long vl)
{
	gtk_spin_button_set_range(GTK_SPIN_BUTTON(widget),minValue(),(gdouble)vl);
}

void gSpinBox::setMinValue(long vl)
{
	gtk_spin_button_set_range(GTK_SPIN_BUTTON(widget),(gdouble)vl,maxValue());
}

void gSpinBox::setStep(long vl)
{
	gdouble step = 0.0;
	
	gtk_spin_button_set_increments(GTK_SPIN_BUTTON(widget),step,step);
}

void gSpinBox::setValue(long vl)
{
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),(gdouble)vl);
}

void gSpinBox::setWrap(bool vl)
{
	gtk_spin_button_set_wrap(GTK_SPIN_BUTTON(widget),vl);
}


