/***************************************************************************

  main.c

  (c) 2000-2011 Benoît Minisini <gambas@users.sourceforge.net>

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

#define __MAIN_C

#include <unistd.h>
#include <stdio.h>

#include "CImage.h"
#include "CImageStat.h"
#include "c_color.h"
#include "image.h"
#include "main.h"

GB_INTERFACE GB EXPORT;


GB_DESC *GB_CLASSES[] EXPORT =
{
	CColorInfoDesc,
	CColorDesc,
	CImageDesc,
	CImageStatDesc,
  NULL
};

static GB_IMG *create_image(int width, int height, int format, unsigned char *data)
{
	CIMAGE *image;
	
  GB.New(POINTER(&image), GB.FindClass("Image"), NULL, NULL);
  IMAGE_create_with_data(&image->image, width, height, format, data);
  return (GB_IMG *)image;
}

void *GB_IMAGE_1[] EXPORT = 
{
	(void *)IMAGE_INTERFACE_VERSION,
	(void *)create_image,
	(void *)IMAGE_take,
	(void *)IMAGE_check,
	(void *)IMAGE_synchronize,
	(void *)IMAGE_size,
	(void *)IMAGE_set_default_format,
	(void *)IMAGE_get_default_format,
	(void *)IMAGE_get_pixel,
	(void *)IMAGE_convert,
	(void *)COLOR_merge,
	(void *)COLOR_lighter,
	(void *)COLOR_darker,
  NULL
};

int EXPORT GB_INIT(void)
{
	return 0;
}

void EXPORT GB_EXIT()
{
}
