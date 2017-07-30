/***************************************************************************

  GLeval.c

  (c) 2005-2011 Laurent Carlier <lordheavy@users.sourceforge.net>

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

#define __GLEVAL_C

#include "GL.h"

BEGIN_METHOD(GLMAP1F, GB_INTEGER target; GB_FLOAT u1; GB_FLOAT u2; GB_INTEGER stride; GB_INTEGER order; GB_OBJECT array)

	GB_ARRAY matrix = (GB_ARRAY) VARG(array);
	int i, count = GB.Array.Count(matrix);
	GLdouble params[count];	

        for (i=0; i<count; i++)
		params[i] = *((double *)GB.Array.Get(matrix,i));
	
	glMap1d( VARG(target), VARG(u1), VARG(u2), VARG(stride), VARG(order), params);

END_METHOD

BEGIN_METHOD(GLMAP2F, GB_INTEGER target; GB_FLOAT u1; GB_FLOAT u2; GB_INTEGER ustride; GB_INTEGER uorder; 
			GB_FLOAT v1; GB_FLOAT v2; GB_INTEGER vstride; GB_INTEGER vorder; GB_OBJECT array)

	GB_ARRAY matrix = (GB_ARRAY) VARG(array);
	int i, count = GB.Array.Count(matrix);
	GLdouble params[count];	

        for (i=0; i<count; i++)
		params[i] = *((double *)GB.Array.Get(matrix,i));
	
	glMap2d( VARG(target), VARG(u1), VARG(u2), VARG(ustride), VARG(uorder), 
				VARG(v1), VARG(v2), VARG(vstride), VARG(vorder), params);

END_METHOD

BEGIN_METHOD(GLEVALCOORD1F, GB_FLOAT u)

	glEvalCoord1f(VARG(u));	

END_METHOD

BEGIN_METHOD(GLEVALCOORD2F, GB_FLOAT u; GB_FLOAT v)

	glEvalCoord2f(VARG(u),VARG(v));	

END_METHOD

BEGIN_METHOD(GLEVALCOORD2FV, GB_OBJECT array)

	GB_ARRAY fArray = (GB_ARRAY) VARG(array);
	int count = GB.Array.Count(fArray);
	GLdouble params[2];	

	params[0] = count>0 ? *((double *)GB.Array.Get(fArray,0)) : 0;  
	params[1] = count>1 ? *((double *)GB.Array.Get(fArray,1)) : 0;  
	
	glEvalCoord2dv(params);	

END_METHOD

BEGIN_METHOD(GLMAPGRID1F, GB_INTEGER un; GB_FLOAT u1; GB_FLOAT u2)

	glMapGrid1d(VARG(un), VARG(u1), VARG(u2));

END_METHOD

BEGIN_METHOD(GLMAPGRID2F, GB_INTEGER un; GB_FLOAT u1; GB_FLOAT u2; GB_INTEGER vn; GB_FLOAT v1; GB_FLOAT v2)

	glMapGrid2d(VARG(un), VARG(u1), VARG(u2), VARG(vn), VARG(v1), VARG(v2));	

END_METHOD

BEGIN_METHOD(GLEVALPOINT1, GB_INTEGER i)

	glEvalPoint1(VARG(i));

END_METHOD

BEGIN_METHOD(GLEVALPOINT2, GB_INTEGER i; GB_INTEGER j)

	glEvalPoint2(VARG(i), VARG(j));

END_METHOD

BEGIN_METHOD(GLEVALMESH1, GB_INTEGER mode; GB_INTEGER i1; GB_INTEGER i2)

	 glEvalMesh1(VARG(mode), VARG(i1), VARG(i2));

END_METHOD

BEGIN_METHOD(GLEVALMESH2, GB_INTEGER mode; GB_INTEGER i1; GB_INTEGER i2; GB_INTEGER j1; GB_INTEGER j2)

	 glEvalMesh2(VARG(mode), VARG(i1), VARG(i2), VARG(j1), VARG(j2));

END_METHOD
