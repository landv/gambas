/***************************************************************************

  Cimage.cpp

  Gambas extension using SDL

  (c) 2006 Laurent Carlier <lordheavy@users.sourceforge.net>
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

#define __CIMAGE_CPP

#include "Cimage.h"

static void free_image(GB_IMG *img, void *image)
{
	delete (SDLsurface *)image;
}

static GB_IMG_OWNER _image_owner = {
	"gb.sdl",
	free_image,
	free_image
	};

SDLsurface *CIMAGE_get(CIMAGE *_object)
{
	SDLsurface *image;
	
	if (IMAGE.Check(THIS_IMAGE, &_image_owner))
	{
		if (!THIS_IMAGE->data)
		{
			image = new SDLsurface();
		}
		else
		{
			IMAGE.Convert(THIS_IMAGE, GB_IMAGE_RGBA);
			image = new SDLsurface((char *)THIS_IMAGE->data, THIS_IMAGE->width, THIS_IMAGE->height);
		}
		image->SetAlphaBuffer(true);
		
		THIS_IMAGE->temp_handle = image;
	}
	
	return IMAGEID;
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
		SDLerror::RaiseError("Unable to load image");

END_METHOD

BEGIN_METHOD(CIMAGE_resize, GB_INTEGER width; GB_INTEGER height)

	check_image(THIS);
	take_image(THIS, IMAGEID);
	IMAGEID->Resize(VARG(width), VARG(height));

END_METHOD


/***************************************************************************/

GB_DESC CImage[] =
{
  GB_DECLARE("Image", sizeof(CIMAGE)),

  GB_STATIC_METHOD("Load", "Image", CIMAGE_load, "(Path)s"),

  GB_METHOD("Resize", NULL, CIMAGE_resize, "(Width)i(Height)i"),

  GB_END_DECLARE
};

