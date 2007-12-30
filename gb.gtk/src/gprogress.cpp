/***************************************************************************

  gprogress.cpp

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
#include <gtk/gtk.h>
#include <stdio.h>
#include <string.h>

gProgressBar::gProgressBar(gControl *parent)  : gControl(parent)
{
	lbl=true;
	g_typ=Type_gProgressBar;
	border=gtk_event_box_new();
	widget=gtk_progress_bar_new();

	gtk_container_add (GTK_CONTAINER(border),widget);	
	connectParent();
	initSignals();
}

bool gProgressBar::label()
{
	return lbl;
}

double gProgressBar::value()
{	
	return gtk_progress_bar_get_fraction (GTK_PROGRESS_BAR(widget));
}

void gProgressBar::setLabel(bool vl)
{
	lbl=vl;
	setValue(value());
}

void gProgressBar::setValue(double vl)
{
	long myvl;
	char buf[5]={0,0,0,0,0};
	
	if (vl<0) vl=0;
	if (vl>1) vl=1;
	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(widget),vl);
	if (!lbl)
		gtk_progress_bar_set_text(GTK_PROGRESS_BAR(widget),"");
	else
	{
		myvl=(long)(vl*100);
		sprintf(buf,"%d",myvl);
		strcat(buf,"%");
		gtk_progress_bar_set_text(GTK_PROGRESS_BAR(widget),buf);
		
	}
}

void gProgressBar::reset()
{
	lbl=false;
	setValue(0);
}
	
	
	