/***************************************************************************

  CConst.cpp

  The constants

  (c) 2000-2005 Beno� Minisini <gambas@users.sourceforge.net>

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

#define __CCONST_CPP


#include "gambas.h"

#include <stdarg.h>

#include <qnamespace.h>
#include <qframe.h>
#include <qkeysequence.h>

#include "main.h"
#include "CContainer.h"
#include "CConst.h"

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
  ALIGN_BOTTOM = Qt::AlignBottom + Qt::AlignHCenter	
};


enum
{
	LINE_NONE = Qt::NoPen,
  LINE_SOLID = Qt::SolidLine,
  LINE_DASH = Qt::DashLine,
  LINE_DOT = Qt::DotLine,
  LINE_DOT_LINE = Qt::DashDotLine,
  LINE_DASH_DOT_DOT = Qt::DashDotDotLine
};

enum
{
	FILL_NONE = Qt::NoBrush,
  FILL_SOLID = Qt::SolidPattern,
  FILL_DENSE_94 = Qt::Dense1Pattern,
  FILL_DENSE_88 = Qt::Dense2Pattern,
  FILL_DENSE_63 = Qt::Dense3Pattern,
  FILL_DENSE_50 = Qt::Dense4Pattern,
  FILL_DENSE_37 = Qt::Dense5Pattern,
  FILL_DENSE_12 = Qt::Dense6Pattern,
  FILL_DENSE_06 = Qt::Dense7Pattern,
  FILL_HORIZONTAL = Qt::HorPattern,
  FILL_VERTICAL = Qt::VerPattern,
  FILL_CROSS = Qt::CrossPattern,
  FILL_DIAGONAL = Qt::BDiagPattern,
  FILL_BACK_DIAGONAL = Qt::FDiagPattern,
  FILL_CROSS_DIAGONAL = Qt::DiagCrossPattern
};

int CCONST_convert(int value, int nconst, ...)
{
  va_list args;
  int i, in, out;

  va_start(args, nconst);
  
	if (nconst < 0)
	{
		nconst = (-nconst);
		for (i = 0; i < nconst; i++)
		{
			in = va_arg(args, int);
			out = va_arg(args, int);
			if (value == out)
				return in;
		}
	}
	else  
	{
		for (i = 0; i < nconst; i++)
		{
			in = va_arg(args, int);
			out = va_arg(args, int);
			if (value == in)
				return out;
		}
	}
  
  out = va_arg(args, int);
  va_end(args);
  
  return out;
}


#include "CConst_desc.h"

