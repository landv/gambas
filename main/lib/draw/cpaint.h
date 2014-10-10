/***************************************************************************

  cpaint.h

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

#ifndef __CPAINT_H
#define __CPAINT_H

#include "gambas.h"
#include "gb.paint.h"

#ifndef __CPAINT_C

extern GB_DESC PaintExtentsDesc[];
extern GB_DESC PaintMatrixDesc[];
extern GB_DESC PaintBrushDesc[];
extern GB_DESC PaintDesc[];

#endif

GB_PAINT *PAINT_get_current();
void *PAINT_get_current_device();
GB_PAINT *PAINT_from_device(void *device);
bool PAINT_is_painted(void *device);

bool PAINT_begin(void *device);
void PAINT_end();
bool PAINT_open(GB_PAINT *paint);
void PAINT_close(GB_PAINT *paint);

#endif
