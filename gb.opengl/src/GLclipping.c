/***************************************************************************

  GLclipping.c

  The Gambas openGL component

  (c) 2005-2007 Laurent Carlier <lordheavy@users.sourceforge.net>
                Benoît Minisini <gambas@users.sourceforge.net>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 1, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

***************************************************************************/

#define __GLCLIPPING_C

#include "gambas.h"
#include "gb_common.h"
#include "main.h"

#include <GL/gl.h>

BEGIN_METHOD(GLCLIPPLANE, GB_INTEGER plane; GB_OBJECT equation)

	GLdouble params[4];
	GB_ARRAY fArray = (GB_ARRAY) VARG(equation);
	int count = GB.Array.Count(fArray);
	uint i;
	
	if (count!=4)
		return;

	for (i=0;i<4; i++)
		params[i]=*((GLdouble *)GB.Array.Get(fArray,i));

	glClipPlane(VARG(plane), params);

END_METHOD

BEGIN_METHOD(GLGETCLIPPLANE, GB_INTEGER plane)

	GLdouble params[4];
	GB_ARRAY fArray;
	uint i;

	GB.Array.New(&fArray , GB_T_FLOAT , 4);
	glGetClipPlane(VARG(plane), params);
	
	for (i=0;i<4; i++)
		*((GLdouble *)GB.Array.Get(fArray, i)) = params[i];
	
	GB.ReturnObject(fArray);

END_METHOD
