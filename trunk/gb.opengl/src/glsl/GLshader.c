/***************************************************************************

  GLshader.c

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

#define __GLSHADER_C

#include "GL.h"

BEGIN_METHOD(GLATTACHSHADER, GB_INTEGER program; GB_INTEGER shader)

	glAttachShader(VARG(program), VARG(shader));

END_METHOD

BEGIN_METHOD(GLCOMPILESHADER, GB_INTEGER shader)

	glCompileShader(VARG(shader));

END_METHOD

BEGIN_METHOD(GLCREATESHADER, GB_INTEGER type)

	GLuint shader;
	shader = glCreateShader(VARG(type));
	GB.ReturnInteger(shader);

END_METHOD

BEGIN_METHOD(GLDELETESHADER, GB_INTEGER shader)

	glDeleteShader(VARG(shader));

END_METHOD

BEGIN_METHOD(GLDETACHSHADER, GB_INTEGER program; GB_INTEGER shader)

	glDetachShader(VARG(program), VARG(shader));

END_METHOD

BEGIN_METHOD(GLGETATTACHEDSHADERS, GB_INTEGER program)

	GLint count;
	GB_ARRAY iArray;
	
	glGetProgramiv(VARG(program), GL_ATTACHED_SHADERS, &count);
	
	if (!count)
	{
		GB.ReturnNull();
		return;
	}
	else
	{
		GLuint params[count];
		int i;
		
		GB.Array.New(&iArray , GB_T_INTEGER , count);
		glGetAttachedShaders(VARG(program), count, NULL, params);
		
		for (i=0;i<count; i++)
			*((int *)GB.Array.Get(iArray, i)) = params[i];
		
		GB.ReturnObject(iArray);
	}

END_METHOD

BEGIN_METHOD(GLGETSHADERINFOLOG, GB_INTEGER shader)

	GLint length;
	
	glGetShaderiv(VARG(shader), GL_INFO_LOG_LENGTH, &length);
	
	if (!length)
	{
		GB.ReturnVoidString();
		return;
	}
	else
	{
		GLchar log[length];

		glGetShaderInfoLog(VARG(shader), length, NULL, log);
		GB.ReturnNewZeroString((const char *)log);
	}

END_METHOD

BEGIN_METHOD(GLGETSHADERIV, GB_INTEGER shader; GB_INTEGER pname)

	GLint value;
	GB_ARRAY iArray;
	
	glGetShaderiv(VARG(shader), VARG(pname), &value);

	GB.Array.New(&iArray , GB_T_INTEGER , 1);
	*((int *)GB.Array.Get(iArray, 0)) = value;
	GB.ReturnObject(iArray);

END_METHOD

BEGIN_METHOD(GLGETSHADERSOURCE, GB_INTEGER shader)

	GLint length;
	
	glGetShaderiv(VARG(shader), GL_SHADER_SOURCE_LENGTH, &length);
	
	if (!length)
	{
		GB.ReturnVoidString();
		return;
	}
	else
	{
		GLchar source[length];

		glGetShaderSource(VARG(shader), length, NULL, source);
		GB.ReturnNewZeroString((const char *)source);
	}

END_METHOD

BEGIN_METHOD(GLISSHADER, GB_INTEGER shader)

	GB.ReturnBoolean(glIsShader(VARG(shader)));

END_METHOD

BEGIN_METHOD(GLSHADERSOURCE, GB_INTEGER shader; GB_STRING source)

	const GLchar* src = GB.ToZeroString(ARG(source));
	glShaderSource(VARG(shader), 1, &src, NULL);

END_METHOD
