/***************************************************************************

  SDL_h.h

  Gambas extension using SDL

  (c) 2006 Laurent Carlier <lordheavy@users.sourceforge.net>
           Benoît Minisini <gambas@users.sourceforge.net>

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

#ifndef __SDL_H_H
#define __SDL_H_H

#include "SDL.h"
#include "SDL_opengl.h"
#include <GL/glx.h>
#include <X11/cursorfont.h>

#define hSurface (this->hSurfaceInfo)->Surface
#define hTexture (this->hSurfaceInfo)->TextureIndex
#define hTextureWidth (this->hSurfaceInfo)->TextureWidth
#define hTextureHeight (this->hSurfaceInfo)->TextureHeight
#define hTextureStatus (this->hSurfaceInfo)->TextureStatus
#define hDrawable (this->hSurfaceInfo)->Drawable
#define hContext (this->hSurfaceInfo)->Ctx

// internal texture status values
#define TEXTURE_OK		(0)
#define TEXTURE_TO_RELOAD	(1<<0)

/*
   Surface and texture definitions

   Surface : current graphic surface (SDLwindow or SDLsurface)
   TextureIndex : index of the associated texture
   TextureWidth/Height : ratio from the Surface (texture must be 2^n size !)
   TextureStatus : status of the texture (reload/...);
   Ctx : Keep track of the context for offscreen/onscreen rendering
*/
typedef struct {
	SDL_Surface *Surface;
	GLuint TextureIndex;
	GLdouble TextureWidth, TextureHeight;
	Uint8 TextureStatus;
	GLXContext Ctx;
	}
	SDL_INFO;

class SDL
{
public:
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
};

#endif /* __SDL_H_H */

