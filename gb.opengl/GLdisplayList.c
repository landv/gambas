/***************************************************************************

  GLdisplayList.c

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

#define __GLDISPLAYLIST_C

#include "GL.h"

BEGIN_METHOD(GLCALLLIST, GB_INTEGER index)

	glCallList(VARG(index));

END_METHOD

BEGIN_METHOD(GLCALLLISTS, GB_OBJECT lists)

	GB_ARRAY iArray = (GB_ARRAY) VARG(lists);
	int i,count = GB.Array.Count(iArray);

	if (!count)
		return;

	for (i=0; i<count; i++)
		glCallList(*((GLuint *)GB.Array.Get(iArray,i)));

END_METHOD

BEGIN_METHOD(GLDELETELISTS, GB_INTEGER index; GB_INTEGER range)

	glDeleteLists(VARG(index), VARG(range));

END_METHOD

BEGIN_METHOD_VOID(GLENDLIST)

	glEndList();

END_METHOD

BEGIN_METHOD(GLGENLISTS, GB_INTEGER count)

	GB.ReturnInteger(glGenLists(VARG(count)));

END_METHOD

BEGIN_METHOD(GLISLIST, GB_INTEGER index)

	GB.ReturnBoolean(glIsList(VARG(index)));

END_METHOD

BEGIN_METHOD(GLLISTBASE, GB_INTEGER index)

	glListBase(VARG(index));

END_METHOD

BEGIN_METHOD(GLNEWLIST, GB_INTEGER index; GB_INTEGER mode)

	glNewList(VARG(index), VARG(mode));

END_METHOD
