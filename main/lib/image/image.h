/***************************************************************************

  image.h

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

#ifndef __IMAGE_H
#define __IMAGE_H

#include "main.h"

#ifndef __IMAGE_C
extern bool IMAGE_debug;
#endif

static inline int RED(uint rgba) { return ((rgba >> 16) & 0xff); }
static inline int GREEN(uint rgba) { return ((rgba >> 8) & 0xff); }
static inline int BLUE(uint rgba) { return (rgba & 0xff); }
static inline int ALPHA(uint rgba) { return ((rgba >> 24) & 0xff); }
static inline uint RGB(int r, int g, int b) { return (0xff << 24) | ((r & 0xff) << 16) | ((g & 0xff) << 8) | (b & 0xff); }
static inline uint RGBA(int r, int g, int b, int a) { return ((a & 0xff) << 24) | ((r & 0xff) << 16) | ((g & 0xff) << 8) | (b & 0xff); }
static inline int GRAY(uint rgba) { return (RED(rgba) * 11 + GREEN(rgba) * 16 + BLUE(rgba) * 5) / 32; }

int IMAGE_size(GB_IMG *img);

void IMAGE_create(GB_IMG *img, int width, int height, int format);
void IMAGE_create_with_data(GB_IMG *img, int width, int height, int format, unsigned char *data);

void IMAGE_take(GB_IMG *img, GB_IMG_OWNER *owner, void *owner_handle, int width, int height, unsigned char *data);
void *IMAGE_check(GB_IMG *img, GB_IMG_OWNER *temp_owner);
void IMAGE_synchronize(GB_IMG *img);

void IMAGE_delete(GB_IMG *img);

void IMAGE_convert(GB_IMG *img, int format);

void IMAGE_fill(GB_IMG *img, GB_COLOR col);
void IMAGE_fill_rect(GB_IMG *img, int x, int y, int w, int h, GB_COLOR col, bool opaque);

void IMAGE_make_gray(GB_IMG *img);
void IMAGE_make_transparent(GB_IMG *img, GB_COLOR color);

GB_COLOR IMAGE_get_pixel(GB_IMG *img, int x, int y);
void IMAGE_set_pixel(GB_IMG *img, int x, int y, GB_COLOR col);

void IMAGE_get_pixels(GB_IMG *img, int *data);
void IMAGE_set_pixels(GB_IMG *img, int *data);

void IMAGE_replace(GB_IMG *img, GB_COLOR src, GB_COLOR dst, bool noteq);

void IMAGE_set_default_format(int format);
int IMAGE_get_default_format();
const char *IMAGE_format_to_string(int fmt);

void IMAGE_bitblt(GB_IMG *dst, int dx, int dy, int dw, int dh, GB_IMG *src, int sx, int sy, int sw, int sh);
void IMAGE_draw_alpha(GB_IMG *dst, int dx, int dy, GB_IMG *src, int sx, int sy, int sw, int sh);
void IMAGE_compose(GB_IMG *dst, int dx, int dy, int dw, int dh, GB_IMG *src, int sx, int sy, int sw, int sh);
void IMAGE_colorize(GB_IMG *img, GB_COLOR color);
void IMAGE_mask(GB_IMG *img, GB_COLOR color);
void IMAGE_mirror(GB_IMG *src, GB_IMG *dst, bool horizontal, bool vertical);
void IMAGE_rotate(GB_IMG *src, GB_IMG *dst, bool left);
void IMAGE_transform(GB_IMG *dst, GB_IMG *src, double sx, double sy, double sdx, double sdy);
void IMAGE_set_opacity(GB_IMG *dst, uchar opacity);
void IMAGE_blur(GB_IMG *img, int radius);
void IMAGE_balance(GB_IMG *img, int brightness, int contrast, int gamma, int hue, int saturation, int lightness);

#define IMAGE_is_void(_image) ((_image)->is_void)

#endif
