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
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
  MA 02110-1301, USA.

***************************************************************************/

#define __GLU_C

#include "gb.image.h"

#include "GLU.h"

#include "GLUcoordTransf.h"
#include "GLUtextureImage.h"
#include "GLUquadratic.h"
#include "GLUnurb.h"
#include "GLUproject.h"

//---------------------------------------------------------------------------

BEGIN_METHOD(Glu_Color, GB_INTEGER color)

	int r, g, b, a;

	GB_COLOR_SPLIT(VARG(color), r, g, b, a);
	//fprintf(stderr, "Glu_Color: %d %d %d %d\n", r, g, b, a);
	if (a == 0)
		glColor3d(r / 255.0, g / 255.0, b / 255.0);
	else
		glColor4d(r / 255.0, g / 255.0, b / 255.0, a / 255.0);

END_METHOD

BEGIN_METHOD(Glu_ClearColor, GB_INTEGER color)

	int r, g, b, a;

	GB_COLOR_SPLIT(VARG(color), r, g, b, a);
	glClearColor(r / 255.0, g / 255.0, b / 255.0, a / 255.0);

END_METHOD

BEGIN_METHOD(GLUERRORSTRING, GB_INTEGER code)

	const GLubyte *errStr = gluErrorString(VARG(code));
	GB.ReturnNewZeroString((char *) errStr);

END_METHOD

//---------------------------------------------------------------------------

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
	GB_STATIC_METHOD("NewQuadric", "GluQuadric", GLUNEWQUADRIC, NULL),
	GB_STATIC_METHOD("QuadricNormals", NULL, GLUQUADRICNORMALS, "(Quad)GluQuadric;(Normal)i"),
	GB_STATIC_METHOD("QuadricTexture", NULL, GLUQUADRICTEXTURE, "(Quad)GluQuadric;(Texture)b"),
	GB_STATIC_METHOD("QuadricOrientation", NULL, GLUQUADRICORIENTATION, "(Quad)GluQuadric;(Orientation)i"),
	GB_STATIC_METHOD("QuadricDrawStyle", NULL, GLUQUADRICDRAWSTYLE, "(Quad)GluQuadric;(DrawStyle)i"),
	GB_STATIC_METHOD("Sphere", NULL, GLUSPHERE, "(Quad)GluQuadric;(Radius)f(Slices)i(Stacks)i"),
	GB_STATIC_METHOD("Cylinder", NULL, GLUCYLINDER, "(Quad)GluQuadric;(Base)f(Top)f(Height)f(Slices)i(Stacks)i"),
	GB_STATIC_METHOD("Disk", NULL, GLUDISK, "(Quad)GluQuadric;(Inner)f(Outer)f(Slices)i(Loops)i"),
	GB_STATIC_METHOD("PartialDisk", NULL, GLUPARTIALDISK, "(Quad)GluQuadric;(Inner)f(Outer)f(Slices)i(Loops)i(Start)f(Sweep)f"),

	/* NURBS - SEE GLUnurbs.h */
	GB_STATIC_METHOD("BeginCurve", NULL, GLUBEGINCURVE, "(Nurb)GluNurb"),
	GB_STATIC_METHOD("BeginSurface", NULL, GLUBEGINSURFACE, "(Nurb)GluNurb"),
	GB_STATIC_METHOD("BeginTrim", NULL, GLUBEGINTRIM, "(Nurb)GluNurb"),
	GB_STATIC_METHOD("DeleteNurbsRenderer", NULL, GLUDELETENURBSRENDERER, "(Nurb)GluNurb"),
	GB_STATIC_METHOD("EndCurve", NULL, GLUENDCURVE, "(Nurb)GluNurb"),
	GB_STATIC_METHOD("EndSurface", NULL, GLUENDSURFACE, "(Nurb)GluNurb"),
	GB_STATIC_METHOD("EndTrim", NULL, GLUENDTRIM, "(Nurb)GluNurb"),
	GB_STATIC_METHOD("NurbsCurve", NULL, GLUNURBSCURVE, "(Nurb)GluNurb;(KnotCount)i(Knots)Single[];(Stride)i(Control)Single[];(Order)i(Type)i"),
	GB_STATIC_METHOD("NurbsProperty", NULL, GLUNURBSPROPERTY, "(Nurb)GluNurb;(Property)i(Value)f"),
	GB_STATIC_METHOD("NurbsSurface", NULL, GLUNURBSSURFACE, "(Nurb)GluNurb;(SKnotCount)i(SKnots)Single[];(TKnotCount)i(TKnots)Single[];(SStride)i(TStride)i(SOrder)i(TOrder)i(Type)i(Control)Single[]"),
	GB_STATIC_METHOD("NewNurbsRenderer","GluNurb", GLUNEWNURBSRENDERER, NULL),

	/* Projections - see GLUproject.h */
	GB_STATIC_METHOD("Project", "Float[]", GLUPROJECT, "(ObjectX)f(ObjectY)f(ObjectZ)f(Modelview)Float[];(Projection)Float[];(Viewport)Integer[];"),
	GB_STATIC_METHOD("UnProject", "Float[]", GLUUNPROJECT, "(WindowX)f(WindowY)f(WindowZ)f(Modelview)Float[];(Projection)Float[];(Viewport)Integer[];"),
	GB_STATIC_METHOD("UnProject4", "Float[]", GLUUNPROJECT4, "(WindowX)f(WindowY)f(WindowZ)f(ClipW)f(Modelview)Float[];(Projection)Float[];(Viewport)Integer[];(NearValue)f(FarValue)f"),
	
	/* Setting a Gambas color */
	GB_STATIC_METHOD("ClearColor", NULL, Glu_ClearColor, "(Color)i"),
	GB_STATIC_METHOD("Color", NULL, Glu_Color, "(Color)i"),

	/********************/
	/* opengl constants */
	/********************/

	/* Errors */
	GB_CONSTANT("INVALID_ENUM", "i", GLU_INVALID_ENUM),
	GB_CONSTANT("INVALID_VALUE", "i", GLU_INVALID_OPERATION),
	GB_CONSTANT("OUT_OF_MEMORY", "i", GLU_OUT_OF_MEMORY),
	
	GB_CONSTANT("NONE", "i", GLU_NONE), 
	GB_CONSTANT("FLAT", "i", GLU_FLAT), 
	GB_CONSTANT("SMOOTH", "i", GLU_SMOOTH), 
	
	GB_CONSTANT("OUTSIDE", "i", GLU_OUTSIDE),
	GB_CONSTANT("INSIDE", "i", GLU_INSIDE),
	
	GB_CONSTANT("FILL", "i", GLU_FILL),
	GB_CONSTANT("LINE", "i", GLU_LINE),
	GB_CONSTANT("POINT", "i", GLU_POINT),
	GB_CONSTANT("SILHOUETTE", "i", GLU_SILHOUETTE),
	
	GB_CONSTANT("CULLING", "i", GLU_CULLING),
	GB_CONSTANT("SAMPLING_TOLERANCE", "i", GLU_SAMPLING_TOLERANCE),
	GB_CONSTANT("SAMPLING_METHOD", "i", GLU_SAMPLING_METHOD),
	GB_CONSTANT("PARAMETRIC_TOLERANCE", "i", GLU_PARAMETRIC_TOLERANCE),
	GB_CONSTANT("DISPLAY_MODE", "i", GLU_DISPLAY_MODE),
	GB_CONSTANT("AUTO_LOAD_MATRIX", "i", GLU_AUTO_LOAD_MATRIX),
	GB_CONSTANT("U_STEP", "i", GLU_U_STEP),
	GB_CONSTANT("V_STEP", "i", GLU_V_STEP),
	GB_CONSTANT("NURBS_MODE", "i", GLU_NURBS_MODE),
	
	GB_END_DECLARE
};
