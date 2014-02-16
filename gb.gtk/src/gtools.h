/***************************************************************************

  gtools.h

  (c) 2000-2013 Benoît Minisini <gambas@users.sourceforge.net>

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

#ifndef __GTOOLS_H
#define __GTOOLS_H

#include "widgets.h"
#include "gpicture.h"
#include "gcontrol.h"
#include "gb.paint.h"

void stub(const char *function);

void gt_exit();

void g_stradd(gchar **res, const gchar *s);

// Frees a string later

char *gt_free_later(char *ptr);

// Parse a shortcut string

void gt_shortcut_parse(char *shortcut, guint *key, GdkModifierType *mods);

// Converts HTML to Pango format

char* gt_html_to_pango_string(const char *html, int len_html, bool newline);

// Converts to/from GTK+ alignment

int gt_to_alignment(double halign, double valign = 0.5);
double gt_from_alignment(int align, bool vertical = false);

// Global signal handlers

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

#ifndef GTK3
void gt_pixmap_fill(GdkPixmap *pix, gColor col, GdkGC *gc);
#endif

// Creates a disabled version of a pixbuf

GdkPixbuf *gt_pixbuf_create_disabled(GdkPixbuf *img);
#ifndef GTK3
void gt_pixbuf_render_pixmap_and_mask(GdkPixbuf *pixbuf, GdkPixmap **pixmap_return, GdkBitmap **mask_return, int alpha_threshold);
#endif
void gt_pixbuf_make_alpha(GdkPixbuf *pixbuf, gColor color);
void gt_pixbuf_make_gray(GdkPixbuf *pixbuf);

// Enable/disable warning messages

void gt_disable_warnings(bool disable);

// Initialize a GtkCellRendererText from a font

void gt_set_cell_renderer_text_from_font(GtkCellRendererText *cell, gFont *font);

// Initialize a PangoLayout from a font

void gt_set_layout_from_font(PangoLayout *layout, gFont *font, int dpi = 0);
void gt_add_layout_from_font(PangoLayout *layout, gFont *font, int dpi = 0);

#define gt_pango_to_pixel(_pango) (((_pango) + (PANGO_SCALE / 2)) / PANGO_SCALE)

// Grab a window

gPicture *gt_grab_window(GdkWindow *win, int x = 0, int y = 0, int w = 0, int h = 0);

// Compute the alignment of a PangoLayout

void gt_layout_alignment(PangoLayout *layout, float w, float h, float *tw, float *th, int align, float *offX, float *offY);

#if GTK_CHECK_VERSION(2, 18, 0)
#else
void gtk_widget_set_can_focus(GtkWidget *widget, gboolean can_focus);
void gtk_widget_get_allocation(GtkWidget *widget, GtkAllocation *allocation);
#endif

void gt_lower_widget(GtkWidget *widget);

#if GTK_CHECK_VERSION(2, 22, 0)
#else
int gdk_device_get_source(GdkDevice *device);
GtkWidget *gtk_window_group_get_current_grab(GtkWindowGroup *window_group);
#endif

// Cairo support

cairo_surface_t *gt_cairo_create_surface_from_pixbuf(const GdkPixbuf *pixbuf);

void gt_cairo_set_source_color(cairo_t *cr, GB_COLOR color);
void gt_cairo_draw_rect(cairo_t *cr, int x, int y, int w, int h, GB_COLOR color);
void gt_cairo_draw_pixbuf(cairo_t *cr, GdkPixbuf *pixbuf, float x, float y, float w, float h, float opacity, GB_RECT *source);

// Color functions

gColor get_gdk_color(GdkColor *gcol);
#ifdef GTK3
void fill_gdk_color(GdkColor *gcol, gColor color);
#else
void fill_gdk_color(GdkColor *gcol, gColor color, GdkColormap *cmap = NULL);
#endif
gColor get_gdk_text_color(GtkWidget *wid, bool enabled);
void set_gdk_text_color(GtkWidget *wid,gColor color);
gColor get_gdk_base_color(GtkWidget *wid, bool enabled);
void set_gdk_base_color(GtkWidget *wid,gColor color);
gColor get_gdk_fg_color(GtkWidget *wid, bool enabled);
void set_gdk_fg_color(GtkWidget *wid,gColor color);
gColor get_gdk_bg_color(GtkWidget *wid, bool enabled);
void set_gdk_bg_color(GtkWidget *wid,gColor color);

void gt_color_to_rgb(gColor color, int *r, int *g, int *b);
gColor gt_rgb_to_color(int r, int g, int b);
void gt_color_to_rgba(gColor color, int *r, int *g, int *b, int *a);
gColor gt_rgba_to_color(int r, int g, int b, int a);
void gt_rgb_to_hsv(int r, int g, int b, int *h, int *s, int *v);
void gt_hsv_to_rgb(int h, int s, int v, int *r, int *g, int *b);
void gt_color_to_frgba(gColor color, double *r, double *g, double *b, double *a);
gColor gt_frgba_to_color(double r, double g, double b, double a);

#ifdef GTK3

void gt_from_color(gColor color, GdkRGBA *rgba);
gColor gt_to_color(GdkRGBA *rgba);

void gt_widget_set_color(GtkWidget *widget, bool fg, gColor color, const char *name = NULL, const GdkRGBA *def_color = NULL);
bool gt_style_lookup_color(GtkStyleContext *style, const char **names, const char **pname, GdkRGBA *rgba);

#endif

// Draw a control border

#ifdef GTK3
void gt_draw_border(cairo_t *cr, GtkStyleContext *st, GtkStateFlags state, int border, gColor color, int x, int y, int w, int h, bool bg = false);
#endif

// Style management

#ifdef GTK3
GtkStyleContext *gt_get_style(GType type);
GtkStyle *gt_get_old_style(GType type);
#else
GtkStyle *gt_get_style(GType type);
#define gt_get_old_style gt_get_style
#endif

void gMnemonic_correctText(char *st,char **buf);
guint gMnemonic_correctMarkup(char *st,char **buf);
void gMnemonic_returnText(char *st,char **buf);

/*#ifdef GTK3
int gt_get_preferred_width(GtkWidget *widget);
#endif*/

#define gt_get_control(_widget) ((gControl *)g_object_get_data(G_OBJECT(_widget), "gambas-control"))
#define gt_register_control(_widget, _control) g_object_set_data(G_OBJECT(_widget), "gambas-control", _control)

#endif
