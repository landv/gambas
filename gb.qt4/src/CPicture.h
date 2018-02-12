/***************************************************************************

  CPicture.h

  (c) 2000-2017 Benoît Minisini <g4mba5@gmail.com>

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

CPICTURE *CPICTURE_grab(QWidget *wid, int screen = -1, int x = 0, int y = 0, int w = -1, int h = -1);
CPICTURE *CPICTURE_get_picture(const char *path);
bool CPICTURE_load_image(QImage **p, const char *path, int lenp);
bool CPICTURE_from_string(QImage **p, const char *addr, int len);
CPICTURE *CPICTURE_create(const QPixmap *pixmap);

#endif
