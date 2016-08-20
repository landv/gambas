/***************************************************************************

  gb_common_swap.h

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

#ifndef __GB_COMMON_SWAP_H
#define __GB_COMMON_SWAP_H

void SWAP_int(int *val);
void SWAP_ints(int *val, int n);

void SWAP_short(short *val);
void SWAP_double(double *val);

#define SWAP_float(_val) SWAP_int((int *)_val)
#define SWAP_int64(_val) SWAP_double((double *)(void *)_val)

#if OS_64BITS
#define SWAP_pointer(_val) SWAP_int64(_val)
#else
#define SWAP_pointer(_val) SWAP_int(((int *)(void *)_val))
#endif

#endif
