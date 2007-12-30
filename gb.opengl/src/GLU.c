/***************************************************************************

  GLU.c

  openGL component

  (c) 2005 Laurent Carlier <lordheavy@users.sourceforge.net>
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

#define __GLU_C

#include "gambas.h"
#include "gb_common.h"
#include "main.h"

#include <GL/glu.h>

#include "GLUcoordTransf.h"
#include "GLUtextureImage.h"

/**************************************************************************/

GB_DESC Cglu[] =
{
	GB_DECLARE("Glu",0), GB_NOT_CREATABLE(),
	
	/* Coordinate Transformation - see GLUcoordTransf.h */
	GB_STATIC_METHOD("LookAt", NULL, GLULOOKAT, "(EyeX)f(EyeY)f(EyeZ)f(CenterX)f(CenterY)f(CenterZ)f(UpX)f(UpY)f(UpZ)f"),
	GB_STATIC_METHOD("Ortho2D", NULL, GLUORTHO2D, "(left)f(right)f(bottom)f(top)f"),
	GB_STATIC_METHOD("Perspective", NULL, GLUPERSPECTIVE, "(fovy)f(aspect)f(zNear)f(zFar)f"),

	/* Texture Image - see GLUtextureImage.h */
	GB_STATIC_METHOD("Build1DMipmaps", NULL, GLUBUILD1DMIPMAPS, "(Image)Image;"),
	GB_STATIC_METHOD("Build2DMipmaps", NULL, GLUBUILD2DMIPMAPS, "(Image)Image;"),

	GB_END_DECLARE
};
