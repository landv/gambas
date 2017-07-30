/***************************************************************************

  SDL_h.h

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

#ifndef __SDL_H_H
#define __SDL_H_H

#include <GL/glew.h>
#include <SDL.h>
#include <SDL_syswm.h>
#include <X11/cursorfont.h>

#include "SDLapp.h"
#include "SDLerror.h"
#include "SDLdebug.h"

namespace SDL
{
	// constant values for drawing
	// line
	enum LineStyle {
		NoLine = 0,
		SolidLine,
		DashLine,
		DotLine,
		DashDotLine,
		DashDotDotLine};
	// filling
	enum FillStyle {
		NoFill = 0,
		SolidFill,
		VerticalFill,
		HorizontalFill,
		CrossFill,
		BackDiagFill,
		DiagFill,
		DiagCrossFill,
		Dense1Fill,
		Dense2Fill,
		Dense3Fill,
		Dense4Fill,
		Dense5Fill,
		Dense6Fill,
		Dense7Fill};
	// cursor shapes
	enum CursorsShape {
		CustomCursor = -3,
		DefaultCursor = -2,
		BlankCursor = -1,
		ArrowCursor = XC_left_ptr,
		CrossCursor = XC_crosshair,
		WaitCursor = XC_watch,
		PointingHandCursor = XC_hand2,
		SizeAllCursor = XC_fleur,
		SizeHorCursor = XC_sb_h_double_arrow,
		SizeVerCursor = XC_sb_v_double_arrow,
		SizeFDiagCursor = XC_bottom_right_corner,
		SizeBDiagCursor = XC_top_right_corner,
		SplitHCursor = XC_sb_h_double_arrow,
		SplitVCursor = XC_sb_v_double_arrow,
		TextCursor = XC_xterm};
}

#endif /* __SDL_H_H */

