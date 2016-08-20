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

#include <qpixmap.h>
#include <qimage.h>

#include "gambas.h"
#include "gb.image.h"

typedef
  struct {
  	GB_IMG img;
    }
  CIMAGE;

#ifndef __CIMAGE_CPP

extern GB_DESC CImageDesc[];

#else

#define THIS OBJECT(CIMAGE)
#define THIS_IMAGE (&THIS->img)
#define QIMAGE ((QImage *)THIS_IMAGE->temp_handle)
#define GET_QIMAGE(_image) ((QImage *)(_image->img.temp_handle))

#endif

const char *CIMAGE_get_format(QString path);
CIMAGE *CIMAGE_create(QImage *image);
QImage *CIMAGE_get(CIMAGE *);

#endif
