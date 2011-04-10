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
#include "SDLtexture.h"

#include <iostream>
#include <cstring>

static void free_image(GB_IMG *img, void *image)
{
	delete (SDLsurface *)image;
}

static void check_modified(GB_IMG *img)
{
	// Vérifie le flag modified
	if (img->modified)
	{
		// Synchronize l'image image->texture
		
		// Réinitialise le flag modified
		img->modified = false;
	}
}

static void *temp_image(GB_IMG *img)
{
	SDLsurface *image;
	
	if (!img->data)
		image = new SDLsurface();
	else
	{
		// Pas besoin de faire de synchro image->texture, vu qu'on crée une nouvelle surface ?
		image = new SDLsurface((char *)img->data, img->width, img->height);
	}
	
	image->SetAlphaBuffer(true);
	return image;
}

static void sync_image(GB_IMG *image)
{
	// Synchronize l'image texture->image
	
	// Puis mets le flag de synchro à false
	image->sync = false;
}


static GB_IMG_OWNER _image_owner = {
	"gb.sdl",
	GB_IMAGE_BGRA,
	free_image,
	free_image,
	temp_image,
	sync_image,
	};

SDLsurface *CIMAGE_get(CIMAGE *_object)
{
	GB_IMG *img = THIS_IMAGE;
	
	// Si ce n'est pas nécessaire de le faire systématiquement chaque fois qu'on a besoin de l'image,
	// alors ne pas le faire ici, mais explicitement où c'est vraiment nécessaire.
	check_modified(img);
	return (SDLsurface *)IMAGE.Check(img, &_image_owner);
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
	{
		(image->GetTexture())->Sync();
		take_image(img, image);
	}
	else
		take_image(img, new SDLsurface());
	
	return img;
}

/***************************************************************************/

GB_DESC CImage[] =
{
  GB_DECLARE("Image", sizeof(CIMAGE)),

  GB_END_DECLARE
};
