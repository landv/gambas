/***************************************************************************

  CConst.cpp

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

#define __CCONST_CPP


#include "gambas.h"

#include <stdarg.h>

#include <qnamespace.h>
#include <qkeysequence.h>

#include "main.h"
#include "CContainer.h"
#include "CConst.h"

static int _alignment[] =
{
	ALIGN_NORMAL, Qt::AlignVCenter + Qt::AlignLeft,
	ALIGN_LEFT, Qt::AlignVCenter + Qt::AlignLeft + Qt::AlignAbsolute,
	ALIGN_RIGHT, Qt::AlignVCenter + Qt::AlignRight + Qt::AlignAbsolute,
	ALIGN_CENTER, Qt::AlignVCenter + Qt::AlignHCenter,
	ALIGN_TOP_NORMAL, Qt::AlignTop + Qt::AlignLeft,
	ALIGN_TOP_LEFT, Qt::AlignTop + Qt::AlignLeft + Qt::AlignAbsolute,
	ALIGN_TOP_RIGHT, Qt::AlignTop + Qt::AlignRight + Qt::AlignAbsolute,
	ALIGN_TOP, Qt::AlignTop + Qt::AlignHCenter,
	ALIGN_BOTTOM_NORMAL, Qt::AlignBottom + Qt::AlignLeft,
	ALIGN_BOTTOM_LEFT, Qt::AlignBottom + Qt::AlignLeft + Qt::AlignAbsolute,
	ALIGN_BOTTOM_RIGHT, Qt::AlignBottom + Qt::AlignRight + Qt::AlignAbsolute,
	ALIGN_BOTTOM, Qt::AlignBottom + Qt::AlignHCenter,
	ALIGN_JUSTIFY, Qt::AlignVCenter + Qt::AlignJustify,
	CONST_MAGIC
};

static int _horizontal_alignment[] =
{
	ALIGN_NORMAL, Qt::AlignLeft,
	ALIGN_LEFT, Qt::AlignLeft + Qt::AlignAbsolute,
	ALIGN_RIGHT, Qt::AlignRight + Qt::AlignAbsolute,
	ALIGN_CENTER, Qt::AlignHCenter,
	ALIGN_JUSTIFY, Qt::AlignJustify,
	CONST_MAGIC
};

static int _line_style[] =
{
	LINE_NONE, Qt::NoPen,
	LINE_SOLID, Qt::SolidLine,
	LINE_DASH, Qt::DashLine,
	LINE_DOT, Qt::DotLine,
	LINE_DASH_DOT, Qt::DashDotLine,
	LINE_DASH_DOT_DOT, Qt::DashDotDotLine,
	CONST_MAGIC
};

static int _fill_style[] =
{
	FILL_NONE, Qt::NoBrush,
	FILL_SOLID, Qt::SolidPattern,
	FILL_DENSE_94, Qt::Dense1Pattern,
	FILL_DENSE_88, Qt::Dense2Pattern,
	FILL_DENSE_63, Qt::Dense3Pattern,
	FILL_DENSE_50, Qt::Dense4Pattern,
	FILL_DENSE_37, Qt::Dense5Pattern,
	FILL_DENSE_12, Qt::Dense6Pattern,
	FILL_DENSE_06, Qt::Dense7Pattern,
	FILL_HORIZONTAL, Qt::HorPattern,
	FILL_VERTICAL, Qt::VerPattern,
	FILL_CROSS, Qt::CrossPattern,
	FILL_DIAGONAL, Qt::BDiagPattern,
	FILL_BACK_DIAGONAL, Qt::FDiagPattern,
	FILL_CROSS_DIAGONAL, Qt::DiagCrossPattern,
	CONST_MAGIC
};

int CCONST_convert(int *tab, int value, int def, bool to_qt)
{
	int *p = tab;
	int ret;
	
	if (to_qt)
	{
		ret = p[1];
		
		for(;;)
		{
			if (*p == CONST_MAGIC)
				return ret;
			else if (*p == def)
				ret = p[1];
			else if (*p == value)
				return p[1];
			p += 2;
		}
	}
	else
	{
		for(;;)
		{
			if (*p == CONST_MAGIC)
				return def;
			else if (p[1] == value)
				return p[0];
			p += 2;
		}
	}	
}

int CCONST_alignment(int value, int def, bool to_qt)
{
	return CCONST_convert(_alignment, value, def, to_qt);
}

int CCONST_horizontal_alignment(int value, int def, bool to_qt)
{
	return CCONST_convert(_horizontal_alignment, value, def, to_qt);
}

int CCONST_line_style(int value, int def, bool to_qt)
{
	return CCONST_convert(_line_style, value, def, to_qt);
}

int CCONST_fill_style(int value, int def, bool to_qt)
{
	return CCONST_convert(_fill_style, value, def, to_qt);
}

#define IMPLEMENT_ALIGN(_method, _code) \
BEGIN_METHOD(_method, GB_INTEGER align) \
	int a = VARG(align); \
	GB.ReturnBoolean(_code); \
END_METHOD

IMPLEMENT_ALIGN(Align_IsTop, ALIGN_IS_TOP(a))
IMPLEMENT_ALIGN(Align_IsBottom, ALIGN_IS_BOTTOM(a))
IMPLEMENT_ALIGN(Align_IsMiddle, ALIGN_IS_MIDDLE(a))
IMPLEMENT_ALIGN(Align_IsLeft, ALIGN_IS_LEFT(a))
IMPLEMENT_ALIGN(Align_IsRight, ALIGN_IS_RIGHT(a))
IMPLEMENT_ALIGN(Align_IsCenter, ALIGN_IS_CENTER(a))

GB_DESC CAlignDesc[] =
{
  GB_DECLARE("Align", 0), GB_VIRTUAL_CLASS(),

  GB_CONSTANT("Normal", "i", ALIGN_NORMAL),
  GB_CONSTANT("Left", "i", ALIGN_LEFT),
  GB_CONSTANT("Right", "i", ALIGN_RIGHT),
  GB_CONSTANT("Center", "i", ALIGN_CENTER),

  GB_CONSTANT("TopNormal", "i", ALIGN_TOP_NORMAL),
  GB_CONSTANT("TopLeft", "i", ALIGN_TOP_LEFT),
  GB_CONSTANT("TopRight", "i", ALIGN_TOP_RIGHT),
  GB_CONSTANT("Top", "i", ALIGN_TOP),

  GB_CONSTANT("BottomNormal", "i", ALIGN_BOTTOM_NORMAL),
  GB_CONSTANT("BottomLeft", "i", ALIGN_BOTTOM_LEFT),
  GB_CONSTANT("BottomRight", "i", ALIGN_BOTTOM_RIGHT),
  GB_CONSTANT("Bottom", "i", ALIGN_BOTTOM),
  
  GB_CONSTANT("Justify", "i", ALIGN_JUSTIFY),
  
  GB_STATIC_METHOD("IsTop", "b", Align_IsTop, "(Alignment)i"),
  GB_STATIC_METHOD("IsBottom", "b", Align_IsBottom, "(Alignment)i"),
  GB_STATIC_METHOD("IsMiddle", "b", Align_IsMiddle, "(Alignment)i"),
  GB_STATIC_METHOD("IsLeft", "b", Align_IsLeft, "(Alignment)i"),
  GB_STATIC_METHOD("IsCenter", "b", Align_IsCenter, "(Alignment)i"),
  GB_STATIC_METHOD("IsRight", "b", Align_IsRight, "(Alignment)i"),

  GB_END_DECLARE
};


GB_DESC CArrangeDesc[] =
{
  GB_DECLARE("Arrange", 0), GB_VIRTUAL_CLASS(),

  GB_CONSTANT("None", "i", ARRANGE_NONE),
  GB_CONSTANT("Horizontal", "i", ARRANGE_HORIZONTAL),
  GB_CONSTANT("Vertical", "i", ARRANGE_VERTICAL),
  GB_CONSTANT("LeftRight", "i", ARRANGE_ROW),
  GB_CONSTANT("TopBottom", "i", ARRANGE_COLUMN),
  GB_CONSTANT("Row", "i", ARRANGE_ROW),
  GB_CONSTANT("Column", "i", ARRANGE_COLUMN),
  GB_CONSTANT("Fill", "i", ARRANGE_FILL),

  GB_END_DECLARE
};


GB_DESC CBorderDesc[] =
{
  GB_DECLARE("Border", 0), GB_VIRTUAL_CLASS(),

  GB_CONSTANT("None", "i", BORDER_NONE),
  GB_CONSTANT("Plain", "i", BORDER_PLAIN),
  GB_CONSTANT("Sunken", "i", BORDER_SUNKEN),
  GB_CONSTANT("Raised", "i", BORDER_RAISED),
  GB_CONSTANT("Etched", "i", BORDER_ETCHED),

  GB_END_DECLARE
};


GB_DESC CScrollDesc[] =
{
  GB_DECLARE("Scroll", 0), GB_VIRTUAL_CLASS(),

  GB_CONSTANT("None", "i", SCROLL_NONE),
  GB_CONSTANT("Horizontal", "i", SCROLL_HORIZONTAL),
  GB_CONSTANT("Vertical", "i", SCROLL_VERTICAL),
  GB_CONSTANT("Both", "i", SCROLL_BOTH),

  GB_END_DECLARE
};


GB_DESC CSelectDesc[] =
{
  GB_DECLARE("Select", 0), GB_VIRTUAL_CLASS(),

  GB_CONSTANT("None", "i", SELECT_NONE),
  GB_CONSTANT("Single", "i", SELECT_SINGLE),
  GB_CONSTANT("Multiple", "i", SELECT_MULTIPLE),

  GB_END_DECLARE
};



