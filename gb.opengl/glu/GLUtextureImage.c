/***************************************************************************

  GLUtextureImage.c

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

#define __GLUTEXTUREIMAGE_C

#include "GLU.h"

BEGIN_METHOD(GLUBUILD1DMIPMAPS, GB_OBJECT Image)

	GB_IMG *image;
	int format;
	int status;

	if (IMAGE_get(ARG(Image), &image, &format))
		return;

	status = gluBuild1DMipmaps(GL_TEXTURE_1D, 4, image->width, format, GL_UNSIGNED_BYTE, image->data);

	GB.ReturnInteger(status);

END_METHOD

BEGIN_METHOD(GLUBUILD2DMIPMAPS, GB_OBJECT Image)

	GB_IMG *image = VARG(Image);
	int status = 0;

	status = gluBuild2DMipmaps(GL_TEXTURE_2D, 4, image->width, image->height, IMAGE_get_pixel_format(image), GL_UNSIGNED_BYTE,
		image->data);

	GB.ReturnInteger(status);

END_METHOD
