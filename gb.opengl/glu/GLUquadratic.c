/***************************************************************************

  GLUquadratic.c

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

#define __GLUQUADRATIC_C

#include "GLU.h"
#include "cgluquadric.h"

static GLUquadric *get_quadric(void *ob)
{
	if (GB.CheckObject(ob))
		return NULL;
	else
		return ((CGLUQUADRIC *)ob)->quadric;
}

#define GET_QUADRIC() \
	GLUquadric *quad = get_quadric(VARG(Quad)); \
	if (!quad) return;

BEGIN_METHOD_VOID(GLUNEWQUADRIC)

	GB.ReturnObject(CGLUQUADRIC_create());
		
END_METHOD

BEGIN_METHOD(GLUQUADRICNORMALS, GB_OBJECT Quad; GB_INTEGER Normal)
	
	GET_QUADRIC();
	gluQuadricNormals(quad, VARG(Normal));

END_METHOD

BEGIN_METHOD(GLUQUADRICORIENTATION, GB_OBJECT Quad; GB_INTEGER Orientation)
	
	GET_QUADRIC();
	gluQuadricOrientation(quad, VARG(Orientation));

END_METHOD

BEGIN_METHOD(GLUQUADRICDRAWSTYLE, GB_OBJECT Quad; GB_INTEGER DrawStyle)
	
	GET_QUADRIC();
	gluQuadricDrawStyle(quad, VARG(DrawStyle));

END_METHOD

BEGIN_METHOD(GLUQUADRICTEXTURE, GB_OBJECT Quad; GB_BOOLEAN Texture)

	GET_QUADRIC();
	gluQuadricTexture(quad, VARG(Texture));

END_METHOD

BEGIN_METHOD(GLUDELETEQUADRIC, GB_OBJECT Quad)

	GET_QUADRIC();
	gluDeleteQuadric(quad);
	// Make the quadric Gambas object invalid
	((CGLUQUADRIC *)VARG(Quad))->quadric = NULL;
	
END_METHOD

BEGIN_METHOD(GLUSPHERE, GB_OBJECT Quad; GB_FLOAT Radius; GB_INTEGER Slices; GB_INTEGER Stacks)
	
	GET_QUADRIC();
	gluSphere(quad, VARG(Radius), VARG(Slices), VARG(Stacks));
	
END_METHOD

BEGIN_METHOD(GLUCYLINDER, GB_OBJECT Quad; GB_FLOAT Base; GB_FLOAT Top; GB_FLOAT Height; GB_INTEGER Slices; GB_INTEGER Stacks)

	GET_QUADRIC();
	gluCylinder(quad, VARG(Base), VARG(Top), VARG(Height), VARG(Slices), VARG(Stacks));
	
END_METHOD

BEGIN_METHOD(GLUDISK, GB_OBJECT Quad; GB_FLOAT Inner; GB_FLOAT Outer; GB_INTEGER Slices; GB_INTEGER Loops)

	GET_QUADRIC();
 	gluDisk(quad, VARG(Inner), VARG(Outer), VARG(Slices), VARG(Loops));
	
END_METHOD

BEGIN_METHOD(GLUPARTIALDISK, GB_OBJECT Quad; GB_FLOAT Inner; GB_FLOAT Outer; GB_INTEGER Slices; GB_INTEGER Loops; GB_FLOAT Start; GB_FLOAT Sweep)

	GET_QUADRIC();
 	gluPartialDisk(quad, VARG(Inner), VARG(Outer), VARG(Slices), VARG(Loops), VARG(Start), VARG(Sweep));
	
END_METHOD


