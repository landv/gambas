/***************************************************************************

  GLU.c

  (c) 2005-2007 Laurent Carlier <lordheavy@users.sourceforge.net>

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
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

***************************************************************************/

#define __GLU_C

#include "GLU.h"

#include "GLUcoordTransf.h"
#include "GLUtextureImage.h"

BEGIN_METHOD(GLUERRORSTRING, GB_INTEGER code)

	const GLubyte *errStr = gluErrorString(VARG(code));
	GB.ReturnNewZeroString((char *) errStr);

END_METHOD

/**************************************************************************/

GB_DESC Cglu[] =
{
	GB_DECLARE("Glu",0), GB_NOT_CREATABLE(),
	
	/* Get error string */
	GB_STATIC_METHOD("ErrorString", "s", GLUERRORSTRING, "(ErrorCode)i"),

	/* Coordinate Transformation - see GLUcoordTransf.h */
	GB_STATIC_METHOD("LookAt", NULL, GLULOOKAT, "(EyeX)f(EyeY)f(EyeZ)f(CenterX)f(CenterY)f(CenterZ)f(UpX)f(UpY)f(UpZ)f"),
	GB_STATIC_METHOD("Ortho2D", NULL, GLUORTHO2D, "(Left)f(Right)f(Bottom)f(Top)f"),
	GB_STATIC_METHOD("Perspective", NULL, GLUPERSPECTIVE, "(Fovy)f(Aspect)f(ZNear)f(ZFar)f"),
	GB_STATIC_METHOD("PickMatrix", NULL, GLUPICKMATRIX, "(X)f(Y)f(DelX)f(DelY)f(Viewport)Integer[]"),

	/* Texture Image - see GLUtextureImage.h */
	GB_STATIC_METHOD("Build1DMipmaps", "i", GLUBUILD1DMIPMAPS, "(Image)Image;"),
	GB_STATIC_METHOD("Build2DMipmaps", "i", GLUBUILD2DMIPMAPS, "(Image)Image;"),

	/* Quadratics - see GLUquadratic.h */


	/********************/
	/* opengl constants */
	/********************/

	/* Errors */
	GB_CONSTANT("GLU_INVALID_ENUM", "i", GLU_INVALID_ENUM),
	GB_CONSTANT("GLU_INVALID_VALUE", "i", GLU_INVALID_OPERATION),
	GB_CONSTANT("GLU_OUT_OF_MEMORY", "i", GLU_OUT_OF_MEMORY),

	GB_END_DECLARE
};
