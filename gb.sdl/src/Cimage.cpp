/***************************************************************************

  Cimage.cpp

  (c) 2006 Laurent Carlier <lordheavy@users.sourceforge.net>

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

#define __CIMAGE_CPP

#include "Cimage.h"

static void free_image(GB_IMG *img, void *image)
{
	delete (SDLsurface *)image;
}

static void *temp_image(GB_IMG *img)
{
	SDLsurface *image;
		
	if (!img->data)
		image = new SDLsurface();
	else
		image = new SDLsurface((char *)img->data, img->width, img->height);
	image->SetAlphaBuffer(true);
	
	return image;
}

static GB_IMG_OWNER _image_owner = {
	"gb.sdl",
	GB_IMAGE_RGBA,
	free_image,
	free_image,
	temp_image
	};

SDLsurface *CIMAGE_get(CIMAGE *_object)
{
	return (SDLsurface *)IMAGE.Check(THIS_IMAGE, &_image_owner);
}

#define check_image CIMAGE_get

static void take_image(CIMAGE *_object, SDLsurface *image)
{
	IMAGE.Take(THIS_IMAGE, &_image_owner, image, image->width(), image->height(), image->data());
}

CIMAGE *CIMAGE_create(SDLsurface *image)
{
	CIMAGE *img;
	static GB_CLASS class_id = NULL;

	if (!class_id)
		class_id = GB.FindClass("Image");

	GB.New(POINTER(&img), class_id, NULL, NULL);
  
	if (image)
		take_image(img, image);
	else
		take_image(img, new SDLsurface());
	
	return img;
}

/***************************************************************************/

BEGIN_METHOD(CIMAGE_load, GB_STRING path)

	char *addr;
	int len;
	SDLsurface *mySurface = new SDLsurface();

	if (!(GB.LoadFile(STRING(path), LENGTH(path), &addr, &len)))
	{
		mySurface->LoadFromMem(addr, len);

		if (mySurface->GetDepth() != 32)
			mySurface->ConvertDepth(32);

		GB.ReturnObject(CIMAGE_create(mySurface));

		GB.ReleaseFile(addr, len);
	}
	else
		GB.Error("Unable to load image");

END_METHOD

/*BEGIN_METHOD(CIMAGE_resize, GB_INTEGER width; GB_INTEGER height)

	check_image(THIS);
	take_image(THIS, IMAGEID);
	IMAGEID->Resize(VARG(width), VARG(height));

END_METHOD*/

/***************************************************************************/

GB_DESC CImage[] =
{
  GB_DECLARE("Image", sizeof(CIMAGE)),

  GB_STATIC_METHOD("Load", "Image", CIMAGE_load, "(Path)s"),

  GB_END_DECLARE
};
