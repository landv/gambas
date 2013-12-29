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
#include "gdesktop.h"
#include "gseparator.h"

#ifdef GTK3
static gboolean cb_draw(GtkWidget *wid, cairo_t *cr, gSeparator *data)
{
	gint x, y, w, h;
	gColor color;

	x = y = 0;
	w = data->width();
	h = data->height();

	if (w == 1 || h == 1)
	{
		color = data->foreground();
		if (color == COLOR_DEFAULT)
			color = gDesktop::lightfgColor();

		gt_cairo_set_source_color(cr, color);

		cairo_rectangle(cr, x, y, w, h);
		cairo_fill(cr);
	}
	else if (w>=h)
		gtk_render_line(gtk_widget_get_style_context(wid), cr, x, y + (h / 2), x + w - 1, y + (h / 2));
	else
		gtk_render_line(gtk_widget_get_style_context(wid), cr, x + (w / 2), y, x + (w / 2), y + h - 1);

	return false;
}
#else
static gboolean cb_expose(GtkWidget *wid, GdkEventExpose *e, gSeparator *data)
{
	gint x, y, w, h;
	gColor color;

	x = wid->allocation.x;
	y = wid->allocation.y;
	w = data->width();
	h = data->height();
	
	if (w == 1 || h == 1)
	{
		cairo_t *cr;
		
		cr = gdk_cairo_create(wid->window);
		
		color = data->foreground();
		if (color == COLOR_DEFAULT)
			color = gDesktop::lightfgColor();
		
		gt_cairo_set_source_color(cr, color);
		
		cairo_rectangle(cr, e->area.x, e->area.y, e->area.width, e->area.height);
		cairo_fill(cr);
		cairo_destroy(cr);
	}
	else if (w>=h)
		gtk_paint_hline(wid->style, wid->window, GTK_STATE_NORMAL, &e->area, wid, NULL, x, x + w, y + h / 2);
	else
		gtk_paint_vline(wid->style, wid->window, GTK_STATE_NORMAL, &e->area, wid, NULL, y, y + h, x + w / 2);

	return false;
} 
#endif

gSeparator::gSeparator(gContainer *parent) : gControl(parent)
{
	g_typ=Type_gSeparator;
	
	border = widget = gtk_fixed_new();

	realize(false);

	ON_DRAW(widget, this, cb_expose, cb_draw);
}


