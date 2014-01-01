/***************************************************************************

  gprogressbar.cpp

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
#include "gprogressbar.h"

gProgressBar::gProgressBar(gContainer *parent)  : gControl(parent)
{
	_label = true;
	_no_background = TRUE;
	
	g_typ = Type_gProgressBar;
	
	border = gtk_alignment_new(0,0,1,1);
	widget = gtk_progress_bar_new();
	realize(false);
}

double gProgressBar::value()
{	
	return gtk_progress_bar_get_fraction (GTK_PROGRESS_BAR(widget));
}

void gProgressBar::setLabel(bool vl)
{
	_label = vl;
	setValue(value());
}

void gProgressBar::setValue(double vl)
{
	char buf[5];
	
	if (vl<0) vl=0;
	if (vl>1) vl=1;
	
	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(widget), vl);
	
	if (!_label)
		gtk_progress_bar_set_text(GTK_PROGRESS_BAR(widget), "");
	else
	{
		int ival = (int)(vl * 100 + 0.5);
		if (ival > 100)
			ival = 100;
		sprintf(buf,"%d%%", ival);
		gtk_progress_bar_set_text(GTK_PROGRESS_BAR(widget), buf);
	}
}

void gProgressBar::reset()
{
	setValue(0);
}
	
