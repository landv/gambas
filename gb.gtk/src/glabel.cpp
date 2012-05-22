/***************************************************************************

  glabel.cpp

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
#include "glabel.h"


static gboolean cb_expose(GtkWidget *draw, GdkEventExpose *e, gLabel *d)
{
	GtkStyle *style = gtk_widget_get_style(draw);
	GdkGC *gc;
	int vw, vh, lw, lh;
	int fw = d->getFrameWidth();
	
	gc = gdk_gc_new(draw->window);
	
	if (style) 
		gdk_gc_set_foreground(gc, &style->fg[GTK_STATE_NORMAL]);
	
	switch (d->lay_x)
	{
		case 0: pango_layout_set_alignment(d->layout, PANGO_ALIGN_LEFT); break;
		case 1: pango_layout_set_alignment(d->layout, PANGO_ALIGN_CENTER); break;
		case 2: pango_layout_set_alignment(d->layout, PANGO_ALIGN_RIGHT); break;
		case 3: pango_layout_set_alignment(d->layout, PANGO_ALIGN_LEFT); break;
	}
	
	vw = d->width();
	vh = d->height();
	pango_layout_get_pixel_size(d->layout, &lw, &lh);
	
	if (!d->markup)
	{
		switch (d->lay_x)
		{
			case 0: vw = fw; break;
			case 1: vw = (vw - lw) / 2; break;
			case 2: vw =  vw - lw - fw; break;
			case 3: 
				if (gtk_widget_get_default_direction()==GTK_TEXT_DIR_RTL ) 
					vw = vw - lw - fw;
				else 
					vw = fw;
				break;
		}
	}
	else
		vw = fw;

	
	switch (d->lay_y)
	{
		case 0: vh = fw; break;
		case 1: vh = (vh - lh) / 2; break;
		case 2: vh = vh - lh - fw; break;
	}
	
	if (vh < 0) vh = 0;
	
	vw += draw->allocation.x;
	vh += draw->allocation.y;
	
	if (d->_transparent && d->_mask_dirty)
	{
		GdkBitmap *mask = gt_make_text_mask(draw->window, d->width(), d->height(), d->layout, vw, vh);
		
		if (fw)
		{
			int i;
			GdkGC *gcm = gdk_gc_new(mask);
			GdkColor white;
			
			white.pixel = 1;
			
			gdk_gc_set_foreground(gcm, &white);
			
			for (i = 0; i < fw; i++)
				gdk_draw_rectangle(mask, gcm, false, i, i, d->width() - i * 2 - 1, d->height() - i * 2 - 1);
				
			g_object_unref(gcm);
		}
		
		gtk_widget_shape_combine_mask(d->border, mask, 0, 0);
		g_object_unref(mask);
		d->_mask_dirty = false;
	}
	
	//fprintf(stderr, "draw label: %s: %d %d\n", d->name(), vw, vh);
	gdk_draw_layout(draw->window, gc, vw, vh, d->layout);	
	g_object_unref(G_OBJECT(gc));
	
	d->drawBorder();
	
	return false;
}

gLabel::gLabel(gContainer *parent) : gControl(parent)
{
	textdata = NULL;
	g_typ = Type_gLabel;
	markup = false;
	_autoresize = false;
	_mask_dirty = false;
	_transparent = false;
	_locked = false;
	_wrap = true;
	align = -1;
	
	border = widget = gtk_fixed_new();
	layout = gtk_widget_create_pango_layout(border, "");
	
	realize(false);

	g_signal_connect_after(G_OBJECT(widget), "expose-event", G_CALLBACK(cb_expose), (gpointer)this);
	
	setAlignment(ALIGN_NORMAL);
	updateLayout();
}

gLabel::~gLabel()
{
	if (textdata) g_free(textdata);
	g_object_unref(G_OBJECT(layout));
}

void gLabel::updateLayout()
{
	char *bpango;
	
	if (!textdata) 
		pango_layout_set_text(layout,"",-1);
	else
	{
		if (markup)
		{
			bpango = gt_html_to_pango_string(textdata, -1, false);
			if (!bpango)
				pango_layout_set_text(layout,"",-1);
			else
			{
				pango_layout_set_markup(layout,bpango,-1);
				g_free(bpango);
			}
		}
		else
			pango_layout_set_text(layout,textdata,-1);
	}
	
	gt_add_layout_from_font(layout, font());
}

void gLabel::updateSize(bool adjust, bool noresize_width)
{
	gint w, h;
	int fw;
	
	if (_locked || !textdata || !*textdata)
		return;
	
	fw = getFrameWidth();
	
	if (markup && _wrap)
	{
		w = width() - fw * 2;
		if (w < 0)
			return;
		w *= PANGO_SCALE;
	}
	else
		w = -1;
	
	pango_layout_set_width(layout, w);
	
	pango_layout_get_pixel_size(layout, &w, &h);

	w += fw * 2;
	h += fw * 2;
	
	if ((!_autoresize && !adjust) || (noresize_width && w != width()))
		return;
		
	if ((align == ALIGN_CENTER || align == ALIGN_LEFT || align == ALIGN_NORMAL || align == ALIGN_RIGHT) && h < height())
		h = height();
	
	_locked = true;
	resize(w, h);	
	_locked = false;
}

void gLabel::adjust()
{
	updateSize(true);
}

void gLabel::setAutoResize(bool vl)
{
	_autoresize = vl;
	updateSize();
}

void gLabel::afterRefresh()
{
	if (!isVisible())
		return;
	
	_mask_dirty = _transparent;
}

void gLabel::setTransparent(bool vl)
{
	if (_transparent == vl)
		return;
		
	_transparent = vl;
	gtk_widget_shape_combine_mask(border, NULL, 0, 0);
	refresh();
}

char *gLabel::text()
{
	return textdata;
}

void gLabel::setText(char *vl)
{
	bool no_text_before = !textdata || !*textdata;
	
	g_free(textdata);
	
	if (vl)
		textdata = g_strdup(vl);
	else
		textdata = 0;

	updateLayout();
	updateSize();
	
	if (_transparent)
	{
		if (no_text_before)
			gtk_widget_shape_combine_mask(border, NULL, 0, 0);
	}
	
	refresh();
}

void gLabel::enableMarkup(bool vl)
{
	if (markup != vl)
	{
		markup = vl;
		updateSize();
		refresh();
	}
}


int gLabel::alignment()
{
	return align;
}

void gLabel::setAlignment(int al)
{
	if (align == al)
		return;
	
	switch (al)
	{
		case ALIGN_BOTTOM:        lay_y=2;  lay_x=1; break;
		case ALIGN_BOTTOM_LEFT:   lay_y=2;  lay_x=0; break;
		case ALIGN_BOTTOM_NORMAL: lay_y=2;  lay_x=3; break;
		case ALIGN_BOTTOM_RIGHT:  lay_y=2;  lay_x=2; break;
		case ALIGN_CENTER:        lay_y=1;  lay_x=1; break;
		case ALIGN_LEFT:          lay_y=1;  lay_x=0; break;
		case ALIGN_NORMAL:        lay_y=1;  lay_x=3; break;
		case ALIGN_RIGHT:         lay_y=1;  lay_x=2; break;
		case ALIGN_TOP:           lay_y=0;  lay_x=1; break;
		case ALIGN_TOP_LEFT:      lay_y=0;  lay_x=0; break;
		case ALIGN_TOP_NORMAL:    lay_y=0;  lay_x=3; break;
		case ALIGN_TOP_RIGHT:     lay_y=0;  lay_x=2; break;
		default: return;
	}
	
	align = al;
	refresh();
}


void gLabel::setFont(gFont *ft)
{
	gControl::setFont(ft);
	updateLayout();
	updateSize();
  refresh();
}

void gLabel::resize(int w, int h)
{
	bool update = markup && width() != w;
	gControl::resize(w, h);
	if (update)
		updateSize(false);
}

void gLabel::setWrap(bool v)
{
	_wrap = v;
	updateSize(true);
}
