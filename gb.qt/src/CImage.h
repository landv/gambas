/***************************************************************************

  CImage.h

  The Image class

  (c) 2000-2005 Beno�t Minisini <gambas@users.sourceforge.net>

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

#ifndef __CIMAGE_H
#define __CIMAGE_H

#include <qpixmap.h>
#include <qpicture.h>
#include <qimage.h>

#include "gambas.h"

typedef
  struct {
    GB_BASE ob;
    QImage *image;
    }
  CIMAGE;

#ifndef __CIMAGE_CPP

extern GB_DESC CImageDesc[];

#else

#define THIS OBJECT(CIMAGE)

#endif

bool CIMAGE_load_image(QImage &p, char *path, int lenp);
const char *CIMAGE_get_format(QString path);

#endif
