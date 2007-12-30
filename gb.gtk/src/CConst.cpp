/***************************************************************************

  CConst.cpp

  (c) 2004-2006 - Daniel Campos Fernández <dcamposf@gmail.com>
  
  GTK+ component

  Realizado para la Junta de Extremadura. 
  Consejería de Educación Ciencia y Tecnología. 
  Proyecto gnuLinEx
  
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
#include "main.h"
#include "widgets.h"
#include "CConst.h"


GB_DESC CSelectDesc[] =
{
  GB_DECLARE("Select", 0), GB_VIRTUAL_CLASS(),

  GB_CONSTANT("None", "i", 0),
  GB_CONSTANT("Single", "i", 1),
  GB_CONSTANT("Multiple", "i", 2),

  GB_END_DECLARE
};

GB_DESC CAlignDesc[] =
{
  GB_DECLARE("Align", 0), GB_VIRTUAL_CLASS(),

  GB_CONSTANT("Normal", "i", AlignVCenter + AlignAuto),
  GB_CONSTANT("Left", "i", AlignVCenter + AlignLeft),
  GB_CONSTANT("Right", "i", AlignVCenter + AlignRight),
  GB_CONSTANT("Center", "i", AlignVCenter + AlignHCenter),

  GB_CONSTANT("TopNormal", "i", AlignTop + AlignAuto),
  GB_CONSTANT("TopLeft", "i", AlignTop + AlignLeft),
  GB_CONSTANT("TopRight", "i", AlignTop + AlignRight),
  GB_CONSTANT("Top", "i", AlignTop + AlignHCenter),

  GB_CONSTANT("BottomNormal", "i", AlignBottom + AlignAuto),
  GB_CONSTANT("BottomLeft", "i", AlignBottom + AlignLeft),
  GB_CONSTANT("BottomRight", "i", AlignBottom + AlignRight),
  GB_CONSTANT("Bottom", "i", AlignBottom + AlignHCenter),

  GB_END_DECLARE
};


GB_DESC CArrangeDesc[] =
{
  GB_DECLARE("Arrange", 0), GB_VIRTUAL_CLASS(),

  GB_CONSTANT("None", "i", ARRANGE_NONE),
  GB_CONSTANT("Horizontal", "i", ARRANGE_HORIZONTAL),
  GB_CONSTANT("Vertical", "i", ARRANGE_VERTICAL),
  GB_CONSTANT("LeftRight", "i", ARRANGE_LEFT_RIGHT),
  GB_CONSTANT("TopBottom", "i", ARRANGE_TOP_BOTTOM),
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

  GB_CONSTANT("None", "i", 0),
  GB_CONSTANT("Horizontal", "i", 1),
  GB_CONSTANT("Vertical", "i", 2),
  GB_CONSTANT("Both", "i", 3),

  GB_END_DECLARE
};


GB_DESC CLineDesc[] =
{
  GB_DECLARE("Line", 0), GB_VIRTUAL_CLASS(),

  GB_CONSTANT("None", "i", Line_None),
  GB_CONSTANT("Solid", "i", Line_Solid),
  GB_CONSTANT("Dash", "i", Line_Dash),
  GB_CONSTANT("Dot", "i", Line_Dot),
  GB_CONSTANT("DashDot", "i", Line_DashDot),
  GB_CONSTANT("DashDotDot", "i", Line_DashDotDot),

  GB_END_DECLARE
};


GB_DESC CFillDesc[] =
{
  GB_DECLARE("Fill", 0), GB_VIRTUAL_CLASS(),

  GB_CONSTANT("None", "i", FillNoBrush),
  GB_CONSTANT("Solid", "i", FillSolidPattern),
  GB_CONSTANT("Dense94", "i", FillDense1Pattern),
  GB_CONSTANT("Dense88", "i", FillDense2Pattern),
  GB_CONSTANT("Dense63", "i", FillDense3Pattern),
  GB_CONSTANT("Dense50", "i", FillDense4Pattern),
  GB_CONSTANT("Dense37", "i", FillDense5Pattern),
  GB_CONSTANT("Dense12", "i", FillDense6Pattern),
  GB_CONSTANT("Dense6", "i", FillDense7Pattern),
  GB_CONSTANT("Horizontal", "i", FillHorPattern),
  GB_CONSTANT("Vertical", "i", FillVerPattern),
  GB_CONSTANT("Cross", "i", FillCrossPattern),
  GB_CONSTANT("Diagonal", "i", FillBDiagPattern),
  GB_CONSTANT("BackDiagonal", "i", FillFDiagPattern),
  GB_CONSTANT("CrossDiagonal", "i", FillDiagCrossPattern),

  GB_END_DECLARE
};

