#ifndef __GCOLOR_H
#define __GCOLOR_H

typedef
	unsigned int gColor;

#define COLOR_DEFAULT ((gColor)-1)

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
