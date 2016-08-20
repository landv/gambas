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
#include "glabel.h"

#ifdef GTK3
static gboolean cb_draw(GtkWidget *draw, cairo_t *cr, gLabel *d)
{
	GdkRGBA rgba;
	int vw, vh, lw, lh;
	int fw = d->getFramePadding() + d->getFrameWidth();
	int xa = d->lay_x;

	//d->drawBackground(cr);
	d->drawBorder(cr);

	gt_from_color(d->realForeground(true), &rgba);
	gdk_cairo_set_source_rgba(cr, &rgba);

	if (xa == 3)
	{
		if (gtk_widget_get_default_direction() == GTK_TEXT_DIR_RTL)
			xa = 2;
		else
			xa = 0;
	}

	switch (xa)
	{
		case 0: pango_layout_set_alignment(d->layout, PANGO_ALIGN_LEFT); break;
		case 1: pango_layout_set_alignment(d->layout, PANGO_ALIGN_CENTER); break;
		case 2: pango_layout_set_alignment(d->layout, PANGO_ALIGN_RIGHT); break;
	}

	vw = d->width();
	vh = d->height();

	pango_layout_get_pixel_size(d->layout, &lw, &lh);

	if (!d->markup || !d->wrap())
	{
		switch (xa)
		{
			case 0: vw = fw; break;
			case 1: vw = (vw - lw) / 2; break;
			case 2: vw =  vw - lw - fw; break;
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

	//vw += draw->allocation.x;
	//vh += draw->allocation.y;

	cairo_move_to(cr, vw, vh);
	pango_cairo_show_layout(cr, d->layout);

	return false;
}
#else
static gboolean cb_expose(GtkWidget *draw, GdkEventExpose *e, gLabel *d)
{
	GtkStyle *style = gtk_widget_get_style(draw);
	cairo_t *cr;
	int vw, vh, lw, lh;
	int fw = d->getFramePadding() + d->getFrameWidth();
	int xa = d->lay_x;

	cr = gdk_cairo_create(draw->window);
	gdk_cairo_region(cr, e->region);
	cairo_clip(cr);

	if (style)
		gdk_cairo_set_source_color(cr, &style->fg[GTK_STATE_NORMAL]);

	if (xa == 3)
	{
		if (gtk_widget_get_default_direction() == GTK_TEXT_DIR_RTL)
			xa = 2;
		else
			xa = 0;
	}

	vw = d->width();
	vh = d->height();

	pango_layout_set_alignment(d->layout, PANGO_ALIGN_LEFT);
	pango_layout_get_pixel_size(d->layout, &lw, &lh);

	switch (xa)
	{
		case 0: pango_layout_set_alignment(d->layout, PANGO_ALIGN_LEFT); break;
		case 1: pango_layout_set_alignment(d->layout, PANGO_ALIGN_CENTER); break;
		case 2: pango_layout_set_alignment(d->layout, PANGO_ALIGN_RIGHT); break;
	}

	if (!d->markup || !d->wrap())
	{
		switch (xa)
		{
			case 0: vw = fw; break;
			case 1: vw = (vw - lw) / 2; break;
			case 2: vw =  vw - lw - fw; break;
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

	cairo_move_to(cr, vw, vh);
	pango_cairo_show_layout(cr, d->layout);
	cairo_destroy(cr);

	d->drawBorder(e);

	return false;
}
#endif

gLabel::gLabel(gContainer *parent) : gControl(parent)
{
	textdata = NULL;
	g_typ = Type_gLabel;
	markup = false;
	_autoresize = false;
	_mask_dirty = false;
	_transparent = false;
	_locked = false;
	_wrap = false;
	align = -1;
	
	border = widget = gtk_fixed_new();
	layout = gtk_widget_create_pango_layout(border, "");

	realize(false);

	ON_DRAW(widget, this, cb_expose, cb_draw);

	setAlignment(ALIGN_NORMAL);
	setText("");
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
	
	updateLayout();

	if (_locked || !textdata || !*textdata)
		return;
	
	fw = getFrameWidth() + getFramePadding();

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

	if (!adjust && _wrap)
		w = width();
	else
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
	//gtk_widget_shape_combine_mask(border, NULL, 0, 0);
	//refresh();
}

char *gLabel::text()
{
	return textdata;
}

void gLabel::setText(const char *vl)
{
	//bool no_text_before = !textdata || !*textdata;
	
	g_free(textdata);
	
	if (vl)
		textdata = g_strdup(vl);
	else
		textdata = 0;

	updateSize();
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

/*gColor gLabel::getFrameColor()
{
	return realForeground();
}*/

void gLabel::updateSize()
{
	updateSize(false);
}


void gLabel::updateBorder()
{
	gControl::updateBorder();
	updateSize(false);
}