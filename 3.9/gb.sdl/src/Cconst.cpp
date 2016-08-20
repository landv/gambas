/***************************************************************************

  Cconst.cpp

  (c) 2006 Laurent Carlier <lordheavy@users.sourceforge.net>

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

#include "main.h"
#include "Cconst.h"
#include "SDL_h.h"

GB_DESC CLine[] =
{
  GB_DECLARE("Line", 0), GB_VIRTUAL_CLASS(),

  GB_CONSTANT("None", "i", SDL::NoLine),
  GB_CONSTANT("Solid", "i", SDL::SolidLine),
  GB_CONSTANT("Dash", "i", SDL::DashLine),
  GB_CONSTANT("Dot", "i", SDL::DotLine),
  GB_CONSTANT("DashDot", "i", SDL::DashDotLine),
  GB_CONSTANT("DashDotDot", "i", SDL::DashDotDotLine),

  GB_END_DECLARE
};

GB_DESC CFill[] =
{
  GB_DECLARE("Fill", 0), GB_VIRTUAL_CLASS(),

  GB_CONSTANT("None", "i", SDL::NoFill),
  GB_CONSTANT("Solid", "i", SDL::SolidFill),
  GB_CONSTANT("Dense94", "i", SDL::Dense1Fill),
  GB_CONSTANT("Dense88", "i", SDL::Dense2Fill),
  GB_CONSTANT("Dense63", "i", SDL::Dense3Fill),
  GB_CONSTANT("Dense50", "i", SDL::Dense4Fill),
  GB_CONSTANT("Dense37", "i", SDL::Dense5Fill),
  GB_CONSTANT("Dense12", "i", SDL::Dense6Fill),
  GB_CONSTANT("Dense6", "i", SDL::Dense7Fill),

  GB_CONSTANT("Horizontal", "i", SDL::HorizontalFill),
  GB_CONSTANT("Vertical", "i", SDL::VerticalFill),
  GB_CONSTANT("Cross", "i", SDL::CrossFill),
  GB_CONSTANT("BackDiagonal", "i", SDL::BackDiagFill),
  GB_CONSTANT("Diagonal", "i", SDL::DiagFill),
  GB_CONSTANT("CrossDiagonal", "i", SDL::DiagCrossFill),

  GB_END_DECLARE
};

