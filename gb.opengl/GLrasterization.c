/***************************************************************************

  GLrasterization.c

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

#define __GLRASTERIZATION_C

#include "GL.h"

BEGIN_METHOD_VOID(GLBITMAP)

	WARNING("glBitmap() Not implemented !");

END_METHOD

BEGIN_METHOD(GLCULLFACE, GB_INTEGER mode)

	glCullFace(VARG(mode));

END_METHOD

BEGIN_METHOD_VOID(GLGETPOLYGONSTIPPLE)

	WARNING("glGetPolygonStipple() Not implemented !");

END_METHOD

BEGIN_METHOD(GLLINESTIPPLE, GB_INTEGER factor; GB_INTEGER pattern)

	glLineStipple(VARG(factor), VARG(pattern));

END_METHOD

BEGIN_METHOD(GLLINEWIDTH, GB_FLOAT width)

	glLineWidth(VARG(width));

END_METHOD

BEGIN_METHOD(GLPOINTSIZE, GB_FLOAT size)

	glPointSize(VARG(size));
  
END_PROPERTY

BEGIN_METHOD(GLPOLYGONMODE, GB_INTEGER face; GB_INTEGER mode)

	glPolygonMode(VARG(face), VARG(mode));

END_METHOD
#if 0
BEGIN_METHOD(GLPOLYGONSTIPPLE, GB_INTEGER mask)

	GLubyte mask[4*32];
	GB_ARRAY iArray = (GB_ARRAY) VARG(mask);
	uint i;
	
	if (GB.Array.Count(iArray) != 32)
		return;
		
	for (i=0; i<32; i++)
		mask[i*4] = *((GLint *)GB.Array.Get(iArray,i));

	glPolygonStipple(mask);

END_METHOD
#endif
BEGIN_METHOD(GLPOLYGONOFFSET, GB_FLOAT factor; GB_FLOAT units)

	glPolygonOffset(VARG(factor), VARG(units));

END_METHOD

BEGIN_METHOD(GLRASTERPOS2F, GB_FLOAT x; GB_FLOAT y)

      glRasterPos2d(VARG(x), VARG(y));

END_PROPERTY

BEGIN_METHOD(GLRASTERPOS3F, GB_FLOAT x; GB_FLOAT y; GB_FLOAT z)

      glRasterPos3d(VARG(x), VARG(y), VARG(z));

END_PROPERTY

BEGIN_METHOD(GLRASTERPOSF, GB_FLOAT x; GB_FLOAT y; GB_FLOAT z; GB_FLOAT w)

      if (MISSING(z))
      {
            glRasterPos2d(VARG(x), VARG(y));
            return;
      }

      if (MISSING(w))
      {
            glRasterPos3d(VARG(x), VARG(y), VARG(z));
            return;
      }

      glRasterPos4d(VARG(x), VARG(y), VARG(z), VARG(w));

END_PROPERTY

BEGIN_METHOD(GLRASTERPOS2I, GB_INTEGER x; GB_INTEGER y)

      glRasterPos2i(VARG(x), VARG(y));

END_PROPERTY

BEGIN_METHOD(GLRASTERPOS3I, GB_INTEGER x; GB_INTEGER y; GB_INTEGER z)

      glRasterPos3i(VARG(x), VARG(y), VARG(z));

END_PROPERTY

BEGIN_METHOD(GLRASTERPOSI, GB_INTEGER x; GB_INTEGER y; GB_INTEGER z; GB_INTEGER w)

      if (MISSING(z))
      {
            glRasterPos2i(VARG(x), VARG(y));
            return;
      }

      if (MISSING(w))
      {
            glRasterPos3i(VARG(x), VARG(y), VARG(z));
            return;
      }

      glRasterPos4i(VARG(x), VARG(y), VARG(z), VARG(w));

END_PROPERTY

BEGIN_METHOD(GLRASTERPOSFV, GB_OBJECT array)

	GLdouble x,y,z,w;
	GB_ARRAY fArray = (GB_ARRAY) VARG(array);
	int count = GB.Array.Count(fArray);
	
	if (count<2)
		return;

	x = *((GLdouble *)GB.Array.Get(fArray,0));
	y = *((GLdouble *)GB.Array.Get(fArray,1));
	
	if (count==2)
	{
		glRasterPos2d(x, y);
		return;
	}

	z = *((GLdouble *)GB.Array.Get(fArray,2));

	if (count==3)
	{
		glRasterPos3d(x, y, z);
	}
	else
	{
		w = *((double *)GB.Array.Get(fArray,3));
		glRasterPos4d(x, y, z, w);
	}

END_METHOD

BEGIN_METHOD(GLRASTERPOSIV, GB_OBJECT array)

	GLint x,y,z,w;
	GB_ARRAY fArray = (GB_ARRAY) VARG(array);
	int count = GB.Array.Count(fArray);
	
	if (count<2)
		return;

	x = *((GLint *)GB.Array.Get(fArray,0));
	y = *((GLint *)GB.Array.Get(fArray,1));
	
	if (count==2)
	{
		glRasterPos2i(x, y);
		return;
	}
	
	z = *((GLint *)GB.Array.Get(fArray,2));

	if (count==3)
	{
		glRasterPos3i(x, y, z);
	}
	else
	{
		w = *((double *)GB.Array.Get(fArray,3));
		glRasterPos4i(x, y, z, w);
	}
	  
END_METHOD
