/***************************************************************************

  GLprogram.c

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

BEGIN_METHOD_VOID(GLCREATEPROGRAM)

	GLuint prog;
	prog = glCreateProgram();
	GB.ReturnInteger(prog);

END_METHOD

BEGIN_METHOD(GLDELETEPROGRAM, GB_INTEGER program)

	glDeleteProgram(VARG(program));

END_METHOD

BEGIN_METHOD(GLGETPROGRAMINFOLOG, GB_INTEGER program)

	GLint length;
	
	glGetProgramiv(VARG(program), GL_INFO_LOG_LENGTH, &length);
	
	if (!length)
	{
		GB.ReturnVoidString();
		return;
	}
	else
	{
		GLchar log[length];

		glGetProgramInfoLog(VARG(program), length, NULL, log);
		GB.ReturnNewZeroString((const char *)log);
	}

END_METHOD

BEGIN_METHOD(GLGETPROGRAMIV, GB_INTEGER program; GB_INTEGER pname)

	GLint value;
	GB_ARRAY iArray;
	
	glGetProgramiv(VARG(program), VARG(pname), &value);

	GB.Array.New(&iArray , GB_T_INTEGER , 1);
	*((int *)GB.Array.Get(iArray, 0)) = value;
	GB.ReturnObject(iArray);

END_METHOD

BEGIN_METHOD(GLISPROGRAM, GB_INTEGER program)

	GB.ReturnBoolean(glIsProgram(VARG(program)));

END_METHOD

BEGIN_METHOD(GLLINKPROGRAM, GB_INTEGER program)

	glLinkProgram(VARG(program));

END_METHOD

BEGIN_METHOD(GLUSEPROGRAM, GB_INTEGER program)

	glUseProgram(VARG(program));

END_METHOD

BEGIN_METHOD(GLVALIDATEPROGRAM, GB_INTEGER program)

	glValidateProgram(VARG(program));

END_METHOD
