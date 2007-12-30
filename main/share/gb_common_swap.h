/***************************************************************************

  gb_common_swap.h

  Swapping routines for endianess management

  Copyright (c) 2000-2007 Benoit Minisini <gambas@freesurf.fr>

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

#ifndef __GB_COMMON_SWAP_H
#define __GB_COMMON_SWAP_H

PUBLIC void SWAP_long(int *val);
PUBLIC void SWAP_longs(int *val, int n);

PUBLIC void SWAP_short(short *val);
PUBLIC void SWAP_double(double *val);

#define SWAP_long_long(_val) SWAP_double((double *)_val)

#endif
