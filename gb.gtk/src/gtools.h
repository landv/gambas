/***************************************************************************

  gtools.h

  (c) 2000-2009 Benoît Minisini <gambas@users.sourceforge.net>

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
#ifndef __GTOOLS_H
#define __GTOOLS_H

#include "widgets.h"
#include "gpicture.h"
#include "gcontrol.h"

void stub(const char *function);

// bool drag_IsEnabled();
// int drag_Action();
// int drag_Type();
// char* drag_Format();
// char* drag_Text();
// gPicture* drag_Image();
// int drag_X();
// int drag_Y();
// gControl* drag_Widget();
// void drag_setIcon(gPicture *pic);
// gPicture* drag_Icon();

void gt_exit();

void g_stradd(gchar **res, const gchar *s);

// Frees a string later

char *gt_free_later(char *ptr);

// Parse a shortcut string

void gt_shortcut_parse(char *shortcut, guint *key, GdkModifierType *mods);

// Converts HTML to Pango format

char* gt_html_to_pango_string(char *html, int len_html, bool newline);

// Converts to/from GTK+ alignment

int gt_to_alignment(double halign, double valign = 0.5);
double gt_from_alignment(int align, bool vertical = false);

// Gets a style associated with a specified class, or else the default style

GtkStyle *gt_get_style(const char *name, int type);

// Global signal handlers

gboolean gcb_keypress (GtkWidget *widget, GdkEventKey *event, gControl *data);
gboolean gcb_keyrelease (GtkWidget *widget, GdkEventKey *event, gControl *data);
gboolean gcb_button_press(GtkWidget *widget, GdkEventButton *event, gControl *data);
gboolean gcb_button_release(GtkWidget *widget, GdkEventButton *event, gControl *data);
gboolean gcb_focus_in(GtkWidget *widget, GdkEventFocus *event, gControl *data);
gboolean gcb_focus_out(GtkWidget *widget, GdkEventFocus *event, gControl *data);

// Where to scroll to ensure that a specific area is visible

typedef
	struct {
		int clientWidth;
		int clientHeight;
		int scrollX;
		int scrollY;
		int scrollWidth;
		int scrollHeight;
		}
	GtEnsureVisible;
	
void gt_ensure_visible(GtEnsureVisible *arg, int x, int y, int w, int h);

#define GT_NORMALIZE(x, y, w, h, sx, sy, sw, sh, width, height) \
	if (w < 0) w = width; \
	if (h < 0) h = height; \
	if (sw < 0) sw = width; \
	if (sh < 0) sh = height; \
  if (sx >= (width) || sy >= (height) || sw <= 0 || sh <= 0) \
    return; \
  if (sx < 0) x -= sx, sx = 0; \
  if (sy < 0) y -= sy, sy = 0; \
  if (sw > ((width) - sx)) \
    sw = ((width) - sx); \
  if (sh > ((height) - sy)) \
    sh = ((height) - sy);

#endif

void gt_drawable_fill(GdkDrawable *d, gColor col, GdkGC *gc);

// Creates a disabled version of a pixbuf

GdkPixbuf *gt_pixbuf_create_disabled(GdkPixbuf *img);
void gt_pixbuf_render_pixmap_and_mask(GdkPixbuf *pixbuf, GdkPixmap **pixmap_return, GdkBitmap **mask_return, int alpha_threshold);
void gt_pixbuf_make_alpha(GdkPixbuf *pixbuf, gColor color);
void gt_pixbuf_make_gray(GdkPixbuf *pixbuf);

// Makes a bitmap mask from a Pango layout

GdkBitmap *gt_make_text_mask(GdkDrawable *dr, int w, int h, PangoLayout *ly, int x, int y);

// Enable/disable warning messages

void gt_disable_warnings(bool disable);

// Initialize a GtkCellRendererText from a font

void gt_set_cell_renderer_text_from_font(GtkCellRendererText *cell, gFont *font);

// Initialize a PangoLayout from a font

void gt_set_layout_from_font(PangoLayout *layout, gFont *font);
void gt_add_layout_from_font(PangoLayout *layout, gFont *font);

#define gt_pango_to_pixel(_pango) (((_pango) + PANGO_SCALE - 1) / PANGO_SCALE)
