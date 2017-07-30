/***************************************************************************

  GLprimitives.c

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

#define __GLPRIMITIVES_C

#include "GL.h"

#include <stdio.h>

BEGIN_METHOD(GLBEGIN, GB_INTEGER primitive)

	glBegin(VARG(primitive));

END_METHOD

BEGIN_METHOD(GLEDGEFLAG, GB_BOOLEAN bValue)

	glEdgeFlag(VARG(bValue));

END_METHOD

BEGIN_METHOD_VOID(GLEND)

	glEnd();

END_METHOD

BEGIN_METHOD(GLRECTF, GB_FLOAT x1; GB_FLOAT y1; GB_FLOAT x2; GB_FLOAT y2)

	glRectf(VARG(x1), VARG(y1), VARG(x2), VARG(y2));

END_METHOD

BEGIN_METHOD(GLRECTI, GB_INTEGER x1; GB_INTEGER y1; GB_INTEGER x2; GB_INTEGER y2)

	glRecti(VARG(x1), VARG(y1), VARG(x2), VARG(y2));
  
END_METHOD

BEGIN_METHOD(GLVERTEX2F, GB_FLOAT x; GB_FLOAT y)

      glVertex2d(VARG(x), VARG(y));

END_METHOD

BEGIN_METHOD(GLVERTEX3F, GB_FLOAT x; GB_FLOAT y; GB_FLOAT z)

      glVertex3d(VARG(x), VARG(y), VARG(z));

END_METHOD

BEGIN_METHOD(GLVERTEXF, GB_FLOAT x; GB_FLOAT y; GB_FLOAT z; GB_FLOAT w)

      if (MISSING(z))
      {
            glVertex2d(VARG(x), VARG(y));
            return;
      }

      if (MISSING(w))
      {
            glVertex3d(VARG(x), VARG(y), VARG(z));
            return;
      }

      glVertex4d(VARG(x), VARG(y), VARG(z), VARG(w));

END_METHOD

BEGIN_METHOD(GLVERTEX2I, GB_INTEGER x; GB_INTEGER y)

      glVertex2i(VARG(x), VARG(y));

END_METHOD

BEGIN_METHOD(GLVERTEX3I, GB_INTEGER x; GB_INTEGER y; GB_INTEGER z)

      glVertex3i(VARG(x), VARG(y), VARG(z));

END_METHOD

BEGIN_METHOD(GLVERTEXI, GB_INTEGER x; GB_INTEGER y; GB_INTEGER z; GB_INTEGER w)

      if (MISSING(z))
      {
            glVertex2i(VARG(x), VARG(y));
            return;
      }

      if (MISSING(w))
      {
            glVertex3i(VARG(x), VARG(y), VARG(z));
            return;
      }

      glVertex4i(VARG(x), VARG(y), VARG(z), VARG(w));

END_METHOD

BEGIN_METHOD(GLVERTEXFV, GB_OBJECT array)

	GLfloat x,y,z,w;
	GB_ARRAY vertex = (GB_ARRAY) VARG(array);
	int count = GB.Array.Count(vertex);
	
	if (count<2)
		return;

	x = *((double *)GB.Array.Get(vertex,0));
	y = *((double *)GB.Array.Get(vertex,1));
	
	if (count==2)
	{
		glVertex2d(x, y);
		return;
	}
	
	z = *((double *)GB.Array.Get(vertex,2));
	
	if (count==3)
		glVertex3d(x, y, z);
	else
	{
		w = *((double *)GB.Array.Get(vertex,3));
		glVertex4d(x, y, z, w);
	}

END_METHOD

BEGIN_METHOD(GLVERTEXIV, GB_OBJECT array)

	GLint x,y,z,w;
	GB_ARRAY vertex = (GB_ARRAY) VARG(array);
	int count = GB.Array.Count(vertex);
	
	if (count<2)
		return;

	x = *((GLint *)GB.Array.Get(vertex,0));
	y = *((GLint *)GB.Array.Get(vertex,1));
	
	if (count==2)
	{
		glVertex2i(x, y);
		return;
	}
	
	z = *((GLint *)GB.Array.Get(vertex,2));
	
	if (count==3)
		glVertex3i(x, y, z);
	else
	{
		w = *((GLint *)GB.Array.Get(vertex,3));
		glVertex4i(x, y, z, w);
	}

END_METHOD
