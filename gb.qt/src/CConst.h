/***************************************************************************

  CConst.h

  The constants

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

#ifndef __CCONST_H
#define __CCONST_H

#include "gambas.h"

#ifndef __CCONST_CPP
extern GB_DESC CAlignDesc[];
extern GB_DESC CArrangeDesc[];
extern GB_DESC CBorderDesc[];
extern GB_DESC CScrollDesc[];
extern GB_DESC CLineDesc[];
extern GB_DESC CFillDesc[];
extern GB_DESC CSelectDesc[];
#endif

enum
{
  ALIGN_NORMAL = Qt::AlignVCenter + Qt::AlignAuto,
  ALIGN_LEFT = Qt::AlignVCenter + Qt::AlignLeft,
  ALIGN_RIGHT = Qt::AlignVCenter + Qt::AlignRight,
  ALIGN_CENTER = Qt::AlignVCenter + Qt::AlignHCenter,
	ALIGN_TOP_NORMAL = Qt::AlignTop + Qt::AlignAuto,
	ALIGN_TOP_LEFT = Qt::AlignTop + Qt::AlignLeft,
	ALIGN_TOP_RIGHT = Qt::AlignTop + Qt::AlignRight,
  ALIGN_TOP = Qt::AlignTop + Qt::AlignHCenter,
  ALIGN_BOTTOM_NORMAL = Qt::AlignBottom + Qt::AlignAuto,
  ALIGN_BOTTOM_LEFT = Qt::AlignBottom + Qt::AlignLeft,
  ALIGN_BOTTOM_RIGHT = Qt::AlignBottom + Qt::AlignRight,
  ALIGN_BOTTOM = Qt::AlignBottom + Qt::AlignHCenter,
  ALIGN_JUSTIFY = Qt::AlignJustify	
};

enum {
  BORDER_NONE = 0,
  BORDER_PLAIN = 1,
  BORDER_SUNKEN = 2,
  BORDER_RAISED = 3,
  BORDER_ETCHED = 4
  };

enum {
  ARRANGE_NONE = 0,
  ARRANGE_HORIZONTAL = 1,
  ARRANGE_VERTICAL = 2,
  ARRANGE_ROW = 3,
  ARRANGE_LEFT_RIGHT = 3,
  ARRANGE_COLUMN = 4,
  ARRANGE_TOP_BOTTOM = 4,
  ARRANGE_FILL = 5
  };

enum
{
	SELECT_NONE = 0,
	SELECT_SINGLE = 1,
	SELECT_MULTIPLE = 2
};


int CCONST_convert(int value, int nconst, ...);

#endif
