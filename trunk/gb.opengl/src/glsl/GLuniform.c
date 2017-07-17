/***************************************************************************

  GLuniform.c

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

#define __GLPROGRAM_C

#include "GL.h"

BEGIN_METHOD(GLGETUNIFORMLOCATION, GB_INTEGER program; GB_STRING name)

	if (!LENGTH(name))
	{
		GB.ReturnInteger(0);
		return;
	}
	
	GB.ReturnInteger(glGetUniformLocation(VARG(program), GB.ToZeroString(ARG(name))));

END_METHOD

BEGIN_METHOD(GLUNIFORM1F, GB_INTEGER location; GB_FLOAT v0)

	glUniform1f(VARG(location), VARG(v0));

END_METHOD

BEGIN_METHOD(GLUNIFORM2F, GB_INTEGER location; GB_FLOAT v0; GB_FLOAT v1)

	glUniform2f(VARG(location), VARG(v0), VARG(v1));

END_METHOD

BEGIN_METHOD(GLUNIFORM3F, GB_INTEGER location; GB_FLOAT v0; GB_FLOAT v1; GB_FLOAT v2)

	glUniform3f(VARG(location), VARG(v0), VARG(v1), VARG(v2));

END_METHOD

BEGIN_METHOD(GLUNIFORM4F, GB_INTEGER location; GB_FLOAT v0; GB_FLOAT v1; GB_FLOAT v2; GB_FLOAT v3)

	glUniform4f(VARG(location), VARG(v0), VARG(v1), VARG(v2), VARG(v3));

END_METHOD

BEGIN_METHOD(GLUNIFORM1I, GB_INTEGER location; GB_INTEGER v0)

	glUniform1i(VARG(location), VARG(v0));

END_METHOD

BEGIN_METHOD(GLUNIFORM2I, GB_INTEGER location; GB_INTEGER v0; GB_INTEGER v1)

	glUniform2i(VARG(location), VARG(v0), VARG(v1));

END_METHOD

BEGIN_METHOD(GLUNIFORM3I, GB_INTEGER location; GB_INTEGER v0; GB_INTEGER v1; GB_INTEGER v2)

	glUniform3i(VARG(location), VARG(v0), VARG(v1), VARG(v2));

END_METHOD

BEGIN_METHOD(GLUNIFORM4I, GB_INTEGER location; GB_INTEGER v0; GB_INTEGER v1; GB_INTEGER v2; GB_INTEGER v3)

	glUniform4i(VARG(location), VARG(v0), VARG(v1), VARG(v2), VARG(v3));

END_METHOD

BEGIN_METHOD(GLUNIFORM1FV, GB_INTEGER location; GB_OBJECT array)

	GB_ARRAY fArray = VARG(array);
	int count = GB.Array.Count(fArray);
	
	if (!count)
		return;

	GLfloat values[count];
	int i;
	
	for (i=0; i<count; i++)
		values[i] = *((GLfloat *)GB.Array.Get(fArray, i));
	
	glUniform1fv(VARG(location), count, values);

END_METHOD

BEGIN_METHOD(GLUNIFORM2FV, GB_INTEGER location; GB_OBJECT array)

	GB_ARRAY fArray = VARG(array);
	int count = GB.Array.Count(fArray);
	int fill = count & 1; // fill=1 if number isn't pair, first bit = 1
	
	if (!count)
		return;

	GLfloat values[count+fill];
	int i;
	
	for (i=0; i<count; i++)
		values[i] = *((GLfloat *)GB.Array.Get(fArray, i));
	
	if (fill)
		values[count+1] = 0;

	count = (count/2)+fill;
	glUniform2fv(VARG(location), count, values);

END_METHOD

BEGIN_METHOD(GLUNIFORM3FV, GB_INTEGER location; GB_OBJECT array)

	GB_ARRAY fArray = VARG(array);
	int count = GB.Array.Count(fArray);
	int fill = count%3 ? (3-(count%3)) : 0;
	
	if (!count)
		return;

	GLfloat values[count+fill];
	int i;
	
	for (i=0; i<count; i++)
		values[i] = *((GLfloat *)GB.Array.Get(fArray, i));
	
	if (fill)
	{
		for (i=1; i<=fill; i++)
			values[count+i] = 0;
		fill = 1;
	}

	count = (count/3)+fill;
	glUniform3fv(VARG(location), count, values);

END_METHOD

BEGIN_METHOD(GLUNIFORM4FV, GB_INTEGER location; GB_OBJECT array)

	GB_ARRAY fArray = VARG(array);
	int count = GB.Array.Count(fArray);
	int fill = count%4 ? (4-(count%4)) : 0;
	
	if (!count)
		return;

	GLfloat values[count+fill];
	int i;
	
	for (i=0; i<count; i++)
		values[i] = *((GLfloat *)GB.Array.Get(fArray, i));
	
	if (fill)
	{
		for (i=1; i<=fill; i++)
			values[count+i] = 0;
		fill = 1;
	}

	count = (count/4)+fill;
	glUniform4fv(VARG(location), count, values);

END_METHOD

BEGIN_METHOD(GLUNIFORM1IV, GB_INTEGER location; GB_OBJECT array)

	GB_ARRAY iArray = VARG(array);
	int count = GB.Array.Count(iArray);
	
	if (!count)
		return;

	GLint values[count];
	int i;
	
	for (i=0; i<count; i++)
		values[i] = *((GLint *)GB.Array.Get(iArray, i));
	
	glUniform1iv(VARG(location), count, values);

END_METHOD

BEGIN_METHOD(GLUNIFORM2IV, GB_INTEGER location; GB_OBJECT array)

	GB_ARRAY iArray = VARG(array);
	int count = GB.Array.Count(iArray);
	int fill = count & 1; // fill=1 if number isn't pair, first bit = 1
	
	if (!count)
		return;

	GLint values[count+fill];
	int i;
	
	for (i=0; i<count; i++)
		values[i] = *((GLint *)GB.Array.Get(iArray, i));
	
	if (fill)
		values[count+1] = 0;

	count = (count/2)+fill;
	glUniform2iv(VARG(location), count, values);

END_METHOD

BEGIN_METHOD(GLUNIFORM3IV, GB_INTEGER location; GB_OBJECT array)

	GB_ARRAY iArray = VARG(array);
	int count = GB.Array.Count(iArray);
	int fill = count%3 ? (3-(count%3)) : 0;
	
	if (!count)
		return;

	GLint values[count+fill];
	int i;
	
	for (i=0; i<count; i++)
		values[i] = *((GLint *)GB.Array.Get(iArray, i));
	
	if (fill)
	{
		for (i=1; i<=fill; i++)
			values[count+i] = 0;
		fill = 1;
	}

	count = (count/3)+fill;
	glUniform3iv(VARG(location), count, values);

END_METHOD

BEGIN_METHOD(GLUNIFORM4IV, GB_INTEGER location; GB_OBJECT array)

	GB_ARRAY iArray = VARG(array);
	int count = GB.Array.Count(iArray);
	int fill = count%4 ? (4-(count%4)) : 0;
	
	if (!count)
		return;

	GLint values[count+fill];
	int i;
	
	for (i=0; i<count; i++)
		values[i] = *((GLint *)GB.Array.Get(iArray, i));
	
	if (fill)
	{
		for (i=1; i<=fill; i++)
			values[count+i] = 0;
		fill = 1;
	}

	count = (count/4)+fill;
	glUniform4iv(VARG(location), count, values);

END_METHOD

BEGIN_METHOD(GLUNIFORMMATRIX2FV, GB_INTEGER location; GB_BOOLEAN transpose; GB_OBJECT array)

	GB_ARRAY fArray = VARG(array);
	int count = GB.Array.Count(fArray);
	int fill = count%4 ? (4-(count%4)) : 0;
	
	if (!count)
		return;

	GLfloat values[count+fill];
	int i;
	
	for (i=0; i<count; i++)
		values[i] = *((GLfloat *)GB.Array.Get(fArray, i));
	
	if (fill)
	{
		for (i=1; i<=fill; i++)
			values[count+i] = 0;
		fill = 1;
	}

	count = (count/4)+fill;
	glUniformMatrix2fv(VARG(location), count, VARG(transpose), values);

END_METHOD

BEGIN_METHOD(GLUNIFORMMATRIX3FV, GB_INTEGER location; GB_BOOLEAN transpose; GB_OBJECT array)

	GB_ARRAY fArray = VARG(array);
	int count = GB.Array.Count(fArray);
	int fill = count%9 ? (9-(count%9)) : 0;
	
	if (!count)
		return;

	GLfloat values[count+fill];
	int i;
	
	for (i=0; i<count; i++)
		values[i] = *((GLfloat *)GB.Array.Get(fArray, i));
	
	if (fill)
	{
		for (i=1; i<=fill; i++)
			values[count+i] = 0;
		fill = 1;
	}

	count = (count/9)+fill;
	glUniformMatrix3fv(VARG(location), count, VARG(transpose), values);

END_METHOD

BEGIN_METHOD(GLUNIFORMMATRIX4FV, GB_INTEGER location; GB_BOOLEAN transpose; GB_OBJECT array)

	GB_ARRAY fArray = VARG(array);
	int count = GB.Array.Count(fArray);
	int fill = count%16 ? (16-(count%16)) : 0;
	
	if (!count)
		return;

	GLfloat values[count+fill];
	int i;
	
	for (i=0; i<count; i++)
		values[i] = *((GLfloat *)GB.Array.Get(fArray, i));
	
	if (fill)
	{
		for (i=1; i<=fill; i++)
			values[count+i] = 0;
		fill = 1;
	}

	count = (count/16)+fill;
	glUniformMatrix4fv(VARG(location), count, VARG(transpose), values);

END_METHOD

BEGIN_METHOD(GLUNIFORMMATRIX2X3FV, GB_INTEGER location; GB_BOOLEAN transpose; GB_OBJECT array)

	GB_ARRAY fArray = VARG(array);
	int count = GB.Array.Count(fArray);
	int fill = count%6 ? (6-(count%6)) : 0;
	
	if (!count)
		return;

	GLfloat values[count+fill];
	int i;
	
	for (i=0; i<count; i++)
		values[i] = *((GLfloat *)GB.Array.Get(fArray, i));
	
	if (fill)
	{
		for (i=1; i<=fill; i++)
			values[count+i] = 0;
		fill = 1;
	}

	count = (count/6)+fill;
	glUniformMatrix2x3fv(VARG(location), count, VARG(transpose), values);

END_METHOD

BEGIN_METHOD(GLUNIFORMMATRIX3X2FV, GB_INTEGER location; GB_BOOLEAN transpose; GB_OBJECT array)

	GB_ARRAY fArray = VARG(array);
	int count = GB.Array.Count(fArray);
	int fill = count%6 ? (6-(count%6)) : 0;
	
	if (!count)
		return;

	GLfloat values[count+fill];
	int i;
	
	for (i=0; i<count; i++)
		values[i] = *((GLfloat *)GB.Array.Get(fArray, i));
	
	if (fill)
	{
		for (i=1; i<=fill; i++)
			values[count+i] = 0;
		fill = 1;
	}

	count = (count/6)+fill;
	glUniformMatrix3x2fv(VARG(location), count, VARG(transpose), values);

END_METHOD

BEGIN_METHOD(GLUNIFORMMATRIX2X4FV, GB_INTEGER location; GB_BOOLEAN transpose; GB_OBJECT array)

	GB_ARRAY fArray = VARG(array);
	int count = GB.Array.Count(fArray);
	int fill = count%8 ? (8-(count%8)) : 0;
	
	if (!count)
		return;

	GLfloat values[count+fill];
	int i;
	
	for (i=0; i<count; i++)
		values[i] = *((GLfloat *)GB.Array.Get(fArray, i));
	
	if (fill)
	{
		for (i=1; i<=fill; i++)
			values[count+i] = 0;
		fill = 1;
	}

	count = (count/8)+fill;
	glUniformMatrix2x4fv(VARG(location), count, VARG(transpose), values);

END_METHOD

BEGIN_METHOD(GLUNIFORMMATRIX4X2FV, GB_INTEGER location; GB_BOOLEAN transpose; GB_OBJECT array)

	GB_ARRAY fArray = VARG(array);
	int count = GB.Array.Count(fArray);
	int fill = count%8 ? (8-(count%8)) : 0;
	
	if (!count)
		return;

	GLfloat values[count+fill];
	int i;
	
	for (i=0; i<count; i++)
		values[i] = *((GLfloat *)GB.Array.Get(fArray, i));
	
	if (fill)
	{
		for (i=1; i<=fill; i++)
			values[count+i] = 0;
		fill = 1;
	}

	count = (count/8)+fill;
	glUniformMatrix4x2fv(VARG(location), count, VARG(transpose), values);

END_METHOD

BEGIN_METHOD(GLUNIFORMMATRIX3X4FV, GB_INTEGER location; GB_BOOLEAN transpose; GB_OBJECT array)

	GB_ARRAY fArray = VARG(array);
	int count = GB.Array.Count(fArray);
	int fill = count%12 ? (12-(count%12)) : 0;
	
	if (!count)
		return;

	GLfloat values[count+fill];
	int i;
	
	for (i=0; i<count; i++)
		values[i] = *((GLfloat *)GB.Array.Get(fArray, i));
	
	if (fill)
	{
		for (i=1; i<=fill; i++)
			values[count+i] = 0;
		fill = 1;
	}

	count = (count/12)+fill;
	glUniformMatrix3x4fv(VARG(location), count, VARG(transpose), values);

END_METHOD

BEGIN_METHOD(GLUNIFORMMATRIX4X3FV, GB_INTEGER location; GB_BOOLEAN transpose; GB_OBJECT array)

	GB_ARRAY fArray = VARG(array);
	int count = GB.Array.Count(fArray);
	int fill = count%12 ? (12-(count%12)) : 0;
	
	if (!count)
		return;

	GLfloat values[count+fill];
	int i;
	
	for (i=0; i<count; i++)
		values[i] = *((GLfloat *)GB.Array.Get(fArray, i));
	
	if (fill)
	{
		for (i=1; i<=fill; i++)
			values[count+i] = 0;
		fill = 1;
	}

	count = (count/12)+fill;
	glUniformMatrix4x3fv(VARG(location), count, VARG(transpose), values);

END_METHOD
