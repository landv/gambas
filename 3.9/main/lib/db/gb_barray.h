/***************************************************************************

  gb_barray.h

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

#ifndef __GB_BARRAY_H
#define __GB_BARRAY_H

#include <string.h>
#include "gb_alloc.h"

typedef
	int *BARRAY;

#define BARRAY_SIZE sizeof(int)
#define BARRAY_NBITS (BARRAY_SIZE << 3)

#define BARRAY_clear_all(_data, _size) memset((_data), 0, ((_size) + BARRAY_NBITS - 1) / BARRAY_NBITS * BARRAY_SIZE)

#define BARRAY_create(_pdata, _size) GB.Alloc(POINTER((_pdata)), ((_size) + BARRAY_NBITS - 1) / BARRAY_NBITS * BARRAY_SIZE)

#define BARRAY_delete(_pdata) GB.Free(POINTER((_pdata)))

#define BARRAY_set(_data, _bit) (_data[(_bit) / BARRAY_NBITS] |= (1 << ((_bit) & (BARRAY_NBITS - 1))))
#define BARRAY_clear(_data, _bit) ((_data)[(_bit) / BARRAY_NBITS] &= ~(1 << ((_bit) & (BARRAY_NBITS - 1))))
#define BARRAY_invert(_data, _bit) ((_data)[(_bit) / BARRAY_NBITS] ^= (1 << ((_bit) & (BARRAY_NBITS - 1))))

#define BARRAY_test(_data, _bit) (((_data)[(_bit) / BARRAY_NBITS] & (1 << ((_bit) & (BARRAY_NBITS - 1)))) != 0)

#define BARRAY_is_void(_data, _size) \
({ \
	int i, v = 0; \
	int size = ((_size) + BARRAY_NBITS - 1) / BARRAY_NBITS; \
	for (i = 0; !v && i < size; i++) \
		v |= (_data)[i]; \
	v == 0; \
})
	
#endif
