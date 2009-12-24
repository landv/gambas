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
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

***************************************************************************/

#define __GLPROGRAM_C

#include "GL.h"

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
