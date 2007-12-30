/***************************************************************************

  gb_barray.h

  C Boolean array management

  (c) 2000-2007 Benoit Minisini <gambas@users.sourceforge.net>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 1, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

***************************************************************************/

#ifndef __GB_BARRAY_H
#define __GB_BARRAY_H

#include <string.h>
#include "gb_alloc.h"

typedef
	long *BARRAY;

#define BARRAY_SIZE 32

#define BARRAY_clear_all(_data, _size) (memset((_data), 0, ((_size) + BARRAY_SIZE - 1) / BARRAY_SIZE))

#define BARRAY_create(_pdata, _size) GB.Alloc(POINTER((_pdata)), ((_size) + BARRAY_SIZE - 1) / BARRAY_SIZE)

#define BARRAY_delete(_pdata) GB.Free(POINTER((_pdata)))

#define BARRAY_set(_data, _bit) (_data[(_bit) / BARRAY_SIZE] |= (1 << ((_bit) & (BARRAY_SIZE - 1))))
#define BARRAY_clear(_data, _bit) ((_data)[(_bit) / BARRAY_SIZE] &= ~(1 << ((_bit) & (BARRAY_SIZE - 1))))
#define BARRAY_invert(_data, _bit) ((_data)[(_bit) / BARRAY_SIZE] ^= (1 << ((_bit) & (BARRAY_SIZE - 1))))

#define BARRAY_test(_data, _bit) (((_data)[(_bit) / BARRAY_SIZE] & (1 << ((_bit) & (BARRAY_SIZE - 1)))) != 0)

#endif
