/***************************************************************************

  main.c

  (c) 2008 Benoît Minisini <gambas@users.sourceforge.net>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 1, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
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
#include "image.h"
#include "main.h"

GB_INTERFACE GB EXPORT;


GB_DESC *GB_CLASSES[] EXPORT =
{
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
	(void *)IMAGE_delete,
	(void *)IMAGE_take,
	(void *)IMAGE_check,
	(void *)IMAGE_convert,
	(void *)IMAGE_fill,
	(void *)IMAGE_make_gray,
	(void *)IMAGE_make_transparent,
  NULL
};

int EXPORT GB_INIT(void)
{
	return FALSE;
}

void EXPORT GB_EXIT()
{
}
