/***************************************************************************

  qt.h

  (c) 2000-2007 Benoit Minisini <gambas@users.sourceforge.net>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 1, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

***************************************************************************/

#ifndef __QT_H
#define __QT_H

#include "gb_common.h"

#define Q_EXPORT
#define QT_NO_DATASTREAM
#define QT_NO_STRINGLIST
#define Q_WS_X11

#define QT_STATIC_CONST static const
#define QT_STATIC_CONST_IMPL const

#define FALSE false
#define TRUE true

#define QMAX(a, b)	((b) < (a) ? (a) : (b))
#define QMIN(a, b)	((a) < (b) ? (a) : (b))
#define QABS(a)	((a) >= 0  ? (a) : -(a))

typedef
	unsigned short Q_UINT16;

typedef
	int QCOORD;

typedef
	unsigned int QRgb;

#define QImage MyQImage
#define QPoint myQPoint
#define QSize myQSize
#define QRect myQRect
#define KImageEffect myKImageEffect
#define QColor myQColor

#endif
