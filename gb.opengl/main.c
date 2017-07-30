/***************************************************************************

  main.c

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

#define __MAIN_C

#include "GL.h"

#include "gb.gl.h"

GB_INTERFACE GB EXPORT;
IMAGE_INTERFACE IMAGE EXPORT;

bool Init(void);

GB_DESC *GB_CLASSES[] EXPORT =
{
	Cgl,
	NULL
};

void *GB_OPENGL_1[] EXPORT = {

	(void *)1,
	(void *)Init,

	NULL
};

int IMAGE_get_pixel_format(GB_IMG *image)
{
	if ((image->format == GB_IMAGE_RGBA) || (image->format == GB_IMAGE_RGBX))
		return GL_RGBA;
	// is it good ?
	else if ((image->format == GB_IMAGE_BGRA) || (image->format == GB_IMAGE_BGRX))
		return GL_BGRA;
	else if (image->format == GB_IMAGE_RGB)
		return GL_RGB;
	else if (image->format == GB_IMAGE_BGR)
		return GL_BGR;
	else
		return 0;
}

bool IMAGE_get(GB_OBJECT *arg, GB_IMG **img, int *format)
{
	*img = arg->value;
	
	if (GB.CheckObject(*img))
		return TRUE;
		
	*format = IMAGE_get_pixel_format(*img);
	if (!*format)
	{
		IMAGE.Convert(*img, GB_IMAGE_RGBA);
		*format = GL_RGBA;
	}
	
	return FALSE;
}

int EXPORT GB_INIT(void)
{
	GB.GetInterface("gb.image", IMAGE_INTERFACE_VERSION, &IMAGE);

	return 0;
}

void EXPORT GB_EXIT()
{
}

bool Init(void)
{
	static bool _init = FALSE;
	
	if (_init)
		return FALSE;
	
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		/* Problem: glewInit failed, something is seriously wrong. */
		GB.Error("Failed to init GLEW: &1\n", glewGetErrorString(err));
		return TRUE;
	}
	else
	{
		_init = TRUE;
		return FALSE;
	}
}