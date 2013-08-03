/***************************************************************************

  CImage.h

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

#ifndef __CIMAGE_H
#define __CIMAGE_H

#include "gambas.h"
#include "gb.image.h"
#include "widgets.h"

typedef 
  struct 
  {
    GB_IMG img;
  }
  CIMAGE;

#ifndef __CIMAGE_CPP

extern GB_DESC CImageDesc[];

#else

#define THIS ((CIMAGE *)_object)
#define PICTURE ((gPicture *)THIS->img.temp_handle)
#define GET_PICTURE(_image) ((gPicture *)(_image->img.temp_handle))

#endif

void *GTK_GetImage(GdkPixbuf *buf);

CIMAGE *CIMAGE_create(gPicture *picture);
gPicture *CIMAGE_get(CIMAGE *);

#endif
