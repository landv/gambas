/***************************************************************************

  GLcoordTransf.c

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

#define __GLCOORDTRANSF_C

#include "GL.h"

BEGIN_METHOD(GLDEPTHRANGE, GB_FLOAT near; GB_FLOAT far)

	glDepthRange(VARG(near),VARG(far));
  
END_METHOD

BEGIN_METHOD(GLFRUSTUM, GB_FLOAT left; GB_FLOAT right; GB_FLOAT bottom; GB_FLOAT top; GB_FLOAT near; GB_FLOAT far)

	glFrustum(VARG(left),VARG(right),VARG(bottom),VARG(top),VARG(near),VARG(far));
  
END_METHOD

BEGIN_METHOD_VOID(GLLOADIDENTITY)

	glLoadIdentity();
  
END_METHOD

BEGIN_METHOD(GLLOADMATRIXF, GB_OBJECT array)

	GLdouble params[16];
	GB_ARRAY matrix = (GB_ARRAY) VARG(array);
	int i, count = GB.Array.Count(matrix);
	
      count = (count > 16 ? 16 : count);

	for (i=0; i<count; i++)
		params[i] = *((double *)GB.Array.Get(matrix,i));
	
	glLoadMatrixd(params);
	
END_PROPERTY

BEGIN_METHOD(GLMATRIXMODE, GB_INTEGER mode)

	glMatrixMode(VARG(mode));

END_PROPERTY

BEGIN_METHOD(GLMULTMATRIXF, GB_OBJECT array)

	GLdouble params[16];
	GB_ARRAY matrix = (GB_ARRAY) VARG(array);
	int i, count = GB.Array.Count(matrix);
	
      count = (count > 16 ? 16 : count);

	for (i=0; i<count; i++)
		params[i] = *((double *)GB.Array.Get(matrix,i));
	
	glMultMatrixd(params);
	
END_PROPERTY

BEGIN_METHOD(GLORTHO, GB_FLOAT left; GB_FLOAT right; GB_FLOAT bottom; GB_FLOAT top; GB_FLOAT near; GB_FLOAT far)

	glOrtho(VARG(left),VARG(right),VARG(bottom),VARG(top),VARG(near),VARG(far));
  
END_METHOD

BEGIN_METHOD_VOID(GLPOPMATRIX)

	glPopMatrix();
  
END_METHOD

BEGIN_METHOD_VOID(GLPUSHMATRIX)

	glPushMatrix();
  
END_METHOD

BEGIN_METHOD(GLROTATEF, GB_FLOAT angle; GB_FLOAT x; GB_FLOAT y; GB_FLOAT z)

	glRotatef(VARG(angle), VARG(x), VARG(y), VARG(z));
  
END_METHOD

BEGIN_METHOD(GLSCALEF, GB_FLOAT x; GB_FLOAT y; GB_FLOAT z)

	glScalef(VARG(x), VARG(y), VARG(z));
  
END_METHOD

BEGIN_METHOD(GLTRANSLATEF, GB_FLOAT x; GB_FLOAT y; GB_FLOAT z)

	glTranslatef(VARG(x), VARG(y), VARG(z));
  
END_METHOD

BEGIN_METHOD(GLVIEWPORT, GB_INTEGER x; GB_INTEGER y; GB_INTEGER width; GB_INTEGER height)

	glViewport(VARG(x),VARG(y),VARG(width),VARG(height));
  
END_METHOD
