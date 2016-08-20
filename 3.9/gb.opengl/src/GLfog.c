/***************************************************************************

  GLfog.c

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

#define __GLFOG_C

#include "GL.h"

BEGIN_METHOD(GLFOGF, GB_INTEGER pname; GB_FLOAT param)

	glFogf(VARG(pname), VARG(param));

END_METHOD

BEGIN_METHOD(GLFOGI, GB_INTEGER face; GB_INTEGER pname; GB_INTEGER param)

	glMateriali(VARG(face), VARG(pname), VARG(param));

END_METHOD

BEGIN_METHOD(GLFOGFV, GB_INTEGER pname; GB_OBJECT params)

	GLfloat params[4];
	GB_ARRAY fArray = (GB_ARRAY) VARG(params);
	uint i, count = GB.Array.Count(fArray);
	
      count = (count > 4 ? 4 : count);

	for (i=0; i<count; i++)
		params[i]=*((double *)GB.Array.Get(fArray,i));

	glFogfv(VARG(pname), params);

END_METHOD

BEGIN_METHOD(GLFOGIV, GB_INTEGER pname; GB_OBJECT params)

	GLint params[4];
	GB_ARRAY iArray = (GB_ARRAY) VARG(params);
	uint i, count = GB.Array.Count(iArray);
	
      count = (count > 4 ? 4 : count);

	for (i=0; i<count; i++)
		params[i]=*((GLint *)GB.Array.Get(iArray,i));

	glFogiv(VARG(pname), params);

END_METHOD
