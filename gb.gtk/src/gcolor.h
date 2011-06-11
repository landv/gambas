/***************************************************************************

  gcolor.h

  (c) 2000-2011 Benoît Minisini <gambas@users.sourceforge.net>

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
#ifndef __GCOLOR_H
#define __GCOLOR_H

#include "gb.image.h"
#include <gdk/gdk.h>
typedef
	int gColor;

/* Functions implemented in gtools.cpp */

gColor get_gdk_color(GdkColor *gcol);
void fill_gdk_color(GdkColor *gcol,gColor color, GdkColormap *cmap = NULL);
gColor get_gdk_text_color(GtkWidget *wid);
void set_gdk_text_color(GtkWidget *wid,gColor color);
gColor get_gdk_base_color(GtkWidget *wid);
void set_gdk_base_color(GtkWidget *wid,gColor color);
gColor get_gdk_fg_color(GtkWidget *wid);
void set_gdk_fg_color(GtkWidget *wid,gColor color);
gColor get_gdk_bg_color(GtkWidget *wid);
void set_gdk_bg_color(GtkWidget *wid,gColor color);

void gt_color_to_rgb(gColor color, int *r, int *g, int *b);
gColor gt_rgb_to_color(int r, int g, int b);
void gt_color_to_rgba(gColor color, int *r, int *g, int *b, int *a);
gColor gt_rgba_to_color(int r, int g, int b, int a);
void gt_rgb_to_hsv(int r, int g, int b, int *h, int *s, int *v);
void gt_hsv_to_rgb(int h, int s, int v, int *r, int *g, int *b);

#endif
