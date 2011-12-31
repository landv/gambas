/***************************************************************************

  GLattributes.c

  (c) 2009 Laurent Carlier <lordheavy@users.sourceforge.net>

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

#define __GLATTRIBUTES_C

#include "GL.h"





/*GLAPI void APIENTRY glVertexAttrib4Niv (GLuint index, const GLint *v);
GLAPI void APIENTRY glVertexAttrib4Nuiv (GLuint index, const GLuint *v);


GLAPI void APIENTRY glVertexAttrib4iv (GLuint index, const GLint *v);
GLAPI void APIENTRY glVertexAttrib4uiv (GLuint index, const GLuint *v);
//GLAPI void APIENTRY glVertexAttribPointer (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid *pointer);

	
	
	GB_STATIC_METHOD("glVertexAttrib4Niv", NULL, GLVERTEXATTRIB4NIV, "(Index)i(V)Integer[]"),
	GB_STATIC_METHOD("glVertexAttrib4Nuiv", NULL, GLVERTEXATTRIB4NUIV, "(Index)i(V)Integer[]"),
	
	GB_STATIC_METHOD("glVertexAttrib4iv", NULL, GLVERTEXATTRIB4IV, "(Index)i(V)Integer[]"),
	GB_STATIC_METHOD("glVertexAttrib4uiv", NULL, GLVERTEXATTRIB4UIV, "(Index)i(V)Integer[]"),
	//GB_STATIC_METHOD("glVertexAttribPointer", NULL, GLVERTEXATTRIBPOINTER, "(Index)i(Size)i(Type)i(Normalized)b(Stride)i(Pointer)p"),*/

BEGIN_METHOD(GLBINDATTRIBLOCATION, GB_INTEGER program; GB_INTEGER index; GB_STRING name)

	glBindAttribLocation (VARG(program), VARG(index), GB.ToZeroString(ARG(name)));

END_METHOD

BEGIN_METHOD(GLVERTEXATTRIB1F, GB_INTEGER index; GB_FLOAT x)

	glVertexAttrib1d(VARG(index), VARG(x));

END_METHOD

BEGIN_METHOD(GLVERTEXATTRIB2F, GB_INTEGER index; GB_FLOAT x; GB_FLOAT y)

	glVertexAttrib2d(VARG(index), VARG(x), VARG(y));

END_METHOD

BEGIN_METHOD(GLVERTEXATTRIB3F, GB_INTEGER index; GB_FLOAT x; GB_FLOAT y; GB_FLOAT z)

	 glVertexAttrib3d(VARG(index), VARG(x), VARG(y), VARG(z));

END_METHOD

BEGIN_METHOD(GLVERTEXATTRIB4F, GB_INTEGER index; GB_FLOAT x; GB_FLOAT y; GB_FLOAT z; GB_FLOAT w)

	glVertexAttrib4d(VARG(index), VARG(x), VARG(y), VARG(z), VARG(w));

END_METHOD



BEGIN_METHOD(GLVERTEXATTRIB1FV, GB_INTEGER index; GB_OBJECT v)

	GB_ARRAY fArray = VARG(v);
	int count = GB.Array.Count(fArray);
	
	if (!count)
		return;

	GLdouble values[count];
	int i;
	
	for (i=0; i<count; i++)
		values[i] = *((GLdouble *)GB.Array.Get(fArray, i));
	
	glVertexAttrib1dv (VARG(index), values);
	
END_METHOD

BEGIN_METHOD(GLVERTEXATTRIB2FV, GB_INTEGER index; GB_OBJECT v)

	GB_ARRAY fArray = VARG(v);
	int count = GB.Array.Count(fArray);
	int fill = count & 1; // fill=1 if number isn't pair, first bit = 1
	
	if (!count)
		return;

	GLdouble values[count+fill];
	int i;
	
	for (i=0; i<count; i++)
		values[i] = *((GLdouble *)GB.Array.Get(fArray, i));
	
	if (fill)
		values[count+1] = 0;

	count = (count/2)+fill;
	glVertexAttrib2dv (VARG(index), values);

END_METHOD

BEGIN_METHOD(GLVERTEXATTRIB3FV, GB_INTEGER index; GB_OBJECT v)

	GB_ARRAY fArray = VARG(v);
	int count = GB.Array.Count(fArray);
	int fill = count%3 ? (3-(count%3)) : 0;
	
	if (!count)
		return;

	GLdouble values[count+fill];
	int i;
	
	for (i=0; i<count; i++)
		values[i] = *((GLdouble *)GB.Array.Get(fArray, i));
	
	if (fill)
	{
		for (i=1; i<=fill; i++)
			values[count+i] = 0;
		fill = 1;
	}

	count = (count/3)+fill;
	glVertexAttrib3dv (VARG(index), values);
	
END_METHOD

BEGIN_METHOD( GLVERTEXATTRIB4FV, GB_INTEGER index; GB_OBJECT v)

	GB_ARRAY fArray = VARG(v);
	int count = GB.Array.Count(fArray);
	int fill = count%4 ? (4-(count%4)) : 0;
	
	if (!count)
		return;

	GLdouble values[count+fill];
	int i;
	
	for (i=0; i<count; i++)
		values[i] = *((GLdouble *)GB.Array.Get(fArray, i));
	
	if (fill)
	{
		for (i=1; i<=fill; i++)
			values[count+i] = 0;
		fill = 1;
	}

	count = (count/4)+fill;
	glVertexAttrib4dv (VARG(index), values);

END_METHOD

