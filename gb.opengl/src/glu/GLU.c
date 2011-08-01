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
#include "GLUquadratic.h"
#include "GLUnurb.h"

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
	GB_STATIC_METHOD("NewQuadric", "p", GLUNEWQUADRIC, NULL),
	GB_STATIC_METHOD("QuadricNormals", NULL, GLUQUADRICNORMALS, "(Quad)p(Normal)i"),
	GB_STATIC_METHOD("QuadricTexture", NULL, GLUQUADRICTEXTURE, "(Quad)p(Texture)b"),
	GB_STATIC_METHOD("DeleteQuadric", NULL, GLUDELETEQUADRIC, "(Quad)p"),
	GB_STATIC_METHOD("Sphere", NULL, GLUSPHERE, "(Quad)p(Radius)f(Slices)i(Stacks)i"),
	GB_STATIC_METHOD("Cylinder", NULL, GLUCYLINDER, "(Quad)p(Base)f(Top)f(Height)f(Slices)i(Stacks)i"),
	GB_STATIC_METHOD("Disk", NULL, GLUDISK, "(Quad)p(Inner)f(Outer)f(Slices)i(Loops)i"),
	GB_STATIC_METHOD("PartialDisk", NULL, GLUPARTIALDISK, "(Quad)p(Inner)f(Outer)f(Slices)i(Loops)i(Start)f(Sweep)f"),
	
	/* NURBS - SEE GLUnurbs.h */
	GB_STATIC_METHOD("BeginCurve", NULL, GLUBEGINCURVE, "(nurb)p"),
	GB_STATIC_METHOD("BeginSurface", NULL, GLUBEGINSURFACE, "(nurb)p"),
	GB_STATIC_METHOD("BeginTrim", NULL, GLUBEGINTRIM, "(nurb)p"),
	GB_STATIC_METHOD("DeleteNurbsRenderer", NULL, GLUDELETENURBSRENDERER, "(nurb)p"),
	GB_STATIC_METHOD("EndCurve", NULL, GLUENDCURVE, "(nurb)p"),
	GB_STATIC_METHOD("EndSurface", NULL, GLUENDSURFACE, "(nurb)p"),
	GB_STATIC_METHOD("EndTrim", NULL, GLUENDTRIM, "(nurb)p"),
	/*GB_STATIC_METHOD(GLUGETNURBSPROPERTY);
	GB_STATIC_METHOD(GLULOADSAMPLINGMATRICES);
	GB_STATIC_METHOD(GLUNURBSCALLBACK);
	GB_STATIC_METHOD(GLUNURBSCALLBACKDATA);
	GB_STATIC_METHOD(GLUNURBSCALLBACKDATAEXT);*/
	GB_STATIC_METHOD("NurbsCurve", NULL, GLUNURBSCURVE, "(nurb)p(knotCount)i(knots)Single[];(stride)i(control)Single[];(order)i(type)i"),
	GB_STATIC_METHOD("NurbsProperty", NULL, GLUNURBSPROPERTY, "(nurb)p(property)i(value)f"),
	GB_STATIC_METHOD("NurbsSurface", NULL, GLUNURBSSURFACE, "(nurb)p(sKnotCount)i(sKnots)Single[];(tKnotCount)i(tKnots)Single[];(sStride)i(tStride)i(sOrder)i(tOrder)i(type)i(control)Single[]"),
	GB_STATIC_METHOD("NewNurbsRenderer","p", GLUNEWNURBSRENDERER, NULL),
	//GB_STATIC_METHOD("PwlCurve", NULL, GLUPWLCURVE, "(nurb)p(count)i(stride)i(type)f(data)Float[]"),


	/********************/
	/* opengl constants */
	/********************/

	/* Errors */
	GB_CONSTANT("GLU_INVALID_ENUM", "i", GLU_INVALID_ENUM),
	GB_CONSTANT("GLU_INVALID_VALUE", "i", GLU_INVALID_OPERATION),
	GB_CONSTANT("GLU_OUT_OF_MEMORY", "i", GLU_OUT_OF_MEMORY),

	GB_END_DECLARE
};
