/***************************************************************************

  CDrawing.h

  The Drawing class

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

#ifndef __CDRAWING_H
#define __CDRAWING_H

#include <qpicture.h>

#include "gambas.h"
#include "../gb.qt.h"

typedef
  struct {
    GB_BASE ob;
    QPicture *picture;
    }
  CDRAWING;

#ifndef __CDRAWING_CPP

extern GB_DESC CDrawingDesc[];

#else

#define THIS ((CDRAWING *)_object)

#endif

#endif
