/***************************************************************************

  GLUcoordTransf.c

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

#define __GLUQUADRATIC_C

#include "GLU.h"

/**************************************************************************/


BEGIN_METHOD_VOID(GLUNEWQUADRIC)

	GLUquadricObj *quad;
	quad = gluNewQuadric();
	GB.ReturnPointer(quad);
		
END_METHOD

BEGIN_METHOD(GLUQUADRICNORMALS, GB_OBJECT Quad; GB_INTEGER Normal)

	GLUquadricObj *quad;
	quad = VARG(Quad);
	gluQuadricNormals (quad, VARG(Normal));

END_METHOD

BEGIN_METHOD(GLUQUADRICTEXTURE, GB_OBJECT Quad; GB_BOOLEAN Texture)

	GLUquadricObj *quad;
	quad = VARG(Quad);
	gluQuadricTexture (quad, VARG(Texture));

END_METHOD

BEGIN_METHOD(GLUDELETEQUADRIC, GB_OBJECT Quad)

	GLUquadricObj *quad;
	quad = VARG(Quad);
	gluDeleteQuadric(quad);

END_METHOD

BEGIN_METHOD(GLUSPHERE, GB_OBJECT Quad; GB_FLOAT Radius; GB_INTEGER Slices; GB_INTEGER Stacks)

	GLUquadricObj *quad;
	quad = VARG(Quad);
	gluSphere (quad, VARG(Radius), VARG(Slices), VARG(Stacks));
	
END_METHOD

BEGIN_METHOD(GLUCYLINDER, GB_OBJECT Quad; GB_FLOAT Base; GB_FLOAT Top; GB_FLOAT Height; \
	 GB_INTEGER Slices; GB_INTEGER Stacks)

	GLUquadricObj *quad;
	quad = VARG(Quad);
	gluCylinder (quad, VARG(Base), VARG(Top), VARG(Height), VARG(Slices), VARG(Stacks));
	
END_METHOD


BEGIN_METHOD(GLUDISK, GB_OBJECT Quad; GB_FLOAT Inner; GB_FLOAT Outer; \
	 GB_INTEGER Slices; GB_INTEGER Loops)

	GLUquadricObj *quad;
	quad = VARG(Quad);
	gluDisk (quad, VARG(Inner), VARG(Outer), VARG(Slices), VARG(Loops));
	
END_METHOD

BEGIN_METHOD(GLUPARTIALDISK, GB_OBJECT Quad; GB_FLOAT Inner; GB_FLOAT Outer; \
	 GB_INTEGER Slices; GB_INTEGER Loops; GB_FLOAT Start; GB_FLOAT Sweep)

	GLUquadricObj *quad;
	quad = VARG(Quad);
	gluPartialDisk (quad, VARG(Inner), VARG(Outer), VARG(Slices), VARG(Loops), VARG(Start), VARG(Sweep));

	
END_METHOD


