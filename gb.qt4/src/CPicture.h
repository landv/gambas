/***************************************************************************

  CPicture.h

  The Picture and Graphic class

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

#ifndef __CPICTURE_H
#define __CPICTURE_H

#include <qpixmap.h>
#include <qimage.h>

#include "gambas.h"

typedef
  struct {
    GB_BASE ob;
    QPixmap *pixmap;
    }
  CPICTURE;

#ifndef __CPICTURE_CPP

extern GB_DESC CPictureDesc[];
//extern GB_DESC CPicturePixelsDesc[];

#else


#define THIS OBJECT(CPICTURE)

#endif

#if 0
#define CPICTURE_is_pixmap(_pict)  ((_pict)->pixmap != 0)
#define CPICTURE_is_picture(_pict)  ((_pict)->picture != 0)
#define CPICTURE_is_image(_pict)  ((_pict)->image != 0)
#endif

#define SET_PIXMAP(_method_pixmap, _store, _object) \
{ \
  CPICTURE *_pict = (CPICTURE *)((_object) ? (_object)->value : NULL); \
  GB.StoreObject(_object, POINTER(_store)); \
  if (_pict && !((_pict)->pixmap->isNull())) \
    _method_pixmap(*((_pict)->pixmap)); \
  else \
    _method_pixmap(QPixmap()); \
}

#define SET_PICTURE(_method_pixmap, _method_picture, _store, _object) \
  SET_PIXMAP(_method_pixmap, _store, _object)

#define CLEAR_PICTURE(_store)  GB.StoreObject(NULL, (void **)(void *)_store)

CPICTURE *CPICTURE_grab(QWidget *wid);
CPICTURE *CPICTURE_get_picture(const char *path);
bool CPICTURE_load_image(QImage **p, const char *path, int lenp);

#endif
