/***************************************************************************

  cpaint_impl.h

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

#ifndef __CPAINT_IMPL_H
#define __CPAINT_IMPL_H

#include "gambas.h"
#include "gb.paint.h"
#include <gdk/gdk.h>

#ifndef __CPAINT_IMPL_C

extern GB_PAINT_DESC PAINT_Interface;
extern GB_PAINT_MATRIX_DESC PAINT_MATRIX_Interface;

#endif

void PAINT_begin(void *device);
void PAINT_end();
void PAINT_clip(int x, int y, int w, int h);
#ifndef GTK3
void PAINT_clip_region(GdkRegion *region);
#endif
cairo_t *PAINT_get_current_context();
void *PAINT_get_current_device();
bool PAINT_get_clip(int *x, int *y, int *w, int *h);
void PAINT_apply_offset(int *x, int *y);

#endif
