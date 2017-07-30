/***************************************************************************

  GLpixelOperations.c

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

#define __GLPIXELOPERATIONS_C

#include "GL.h"

BEGIN_METHOD(GLCOPYPIXELS, GB_INTEGER x; GB_INTEGER y; GB_INTEGER width; GB_INTEGER height; GB_INTEGER type)

	glCopyPixels(VARG(x), VARG(y), VARG(width), VARG(height), VARG(type));

END_METHOD

BEGIN_METHOD(GLPIXELSTOREF, GB_INTEGER pname; GB_FLOAT param)

	glPixelStoref(VARG(pname), VARG(param));

END_METHOD

BEGIN_METHOD(GLPIXELSTOREI, GB_INTEGER pname; GB_INTEGER param)

	glPixelStorei(VARG(pname), VARG(param));

END_METHOD

BEGIN_METHOD(GLPIXELTRANSFERF, GB_INTEGER pname; GB_FLOAT param)

	glPixelTransferf(VARG(pname), VARG(param));

END_METHOD

BEGIN_METHOD(GLPIXELTRANSFERI, GB_INTEGER pname; GB_INTEGER param)

	glPixelTransferi(VARG(pname), VARG(param));

END_METHOD

BEGIN_METHOD(GLREADBUFFER, GB_INTEGER mode)

	glReadBuffer(VARG(mode));

END_METHOD

BEGIN_METHOD(GLDRAWPIXELS, GB_OBJECT Image)

	GB_IMG *image;
	int format;
	
	if (IMAGE_get(ARG(Image), &image, &format))
		return;

	glDrawPixels(image->width, image->height, format, GL_UNSIGNED_BYTE, image->data);

END_METHOD

