/***************************************************************************

  GLUtextureImage.c

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

#define __GLUTEXTUREIMAGE_C

#include "gambas.h"
#include "gb_common.h"
#include "main.h"

#include <GL/glu.h>

BEGIN_METHOD(GLUBUILD1DMIPMAPS, GB_OBJECT Image)

	GB_IMG *image = VARG(Image);
	int status = 0;

	status = gluBuild1DMipmaps(GL_TEXTURE_1D, 4, image->width, IMAGE_get_pixel_format(image), GL_UNSIGNED_BYTE, image->data);

	GB.ReturnInteger(status);

END_METHOD

BEGIN_METHOD(GLUBUILD2DMIPMAPS, GB_OBJECT Image)

	GB_IMG *image = VARG(Image);
	int status = 0;

	status = gluBuild2DMipmaps(GL_TEXTURE_2D, 4, image->width, image->height, IMAGE_get_pixel_format(image), GL_UNSIGNED_BYTE,
		image->data);

	GB.ReturnInteger(status);

END_METHOD
