/***************************************************************************

  main.h

  (c) 2005-2007 Laurent Carlier <lordheavy@users.sourceforge.net>

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

#ifndef __MAIN_H
#define __MAIN_H

#include "gb_common.h"
#include "gambas.h"
#include "gb.image.h"

#ifndef __MAIN_C
extern GB_INTERFACE GB;
extern IMAGE_INTERFACE IMAGE;
#endif

#ifndef WARNING
#define WARNING(_c) printf("warning: " _c)
#endif /* WARNING */

int IMAGE_get_pixel_format(GB_IMG *image);
bool IMAGE_get(GB_OBJECT *arg, GB_IMG **img, int *format);

#define IMAGE_get_ncolors(_format) (GB_IMAGE_FMT_IS_24_BITS(_format) ? 3 : 4)


#endif

