/***************************************************************************

  c_image.c

  (c) 2014 Benoît Minisini <gambas@users.sourceforge.net>

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

#define __C_IMAGE_C

#include "c_image.h"

#define THIS ((CIMAGE *)_object)
#define THIS_IMAGE (&THIS->img)

SDL_Image *SDL_CreateImage(SDL_Surface *surface)
{
	SDL_Image *image;
	
	GB.Alloc(POINTER(&image), sizeof(SDL_Image));
	
	image->texture = NULL;
	image->window = NULL;
	image->surface = surface;
	
	return image;
}

void SDL_FreeImage(SDL_Image *image)
{
	if (image->texture)
	{
		SDL_DestroyTexture(image->texture);
		image->texture = NULL;
		GB.Unref(POINTER(&image->window));
	}

	if (image->surface)
	{
		SDL_FreeSurface(image->surface);
		image->surface = NULL;
	}

	GB.Free(POINTER(&image));
}

static void free_image(GB_IMG *img, void *image)
{
	SDL_FreeImage((SDL_Image *)image);
}

static void *temp_image(GB_IMG *img)
{
	SDL_Surface *surface = NULL;
	
	if (img && img->data)
		surface = SDL_CreateRGBSurfaceFrom(img->data, img->width, img->height, 32, img->width * sizeof(int), RMASK, GMASK, BMASK, AMASK);
	
	return SDL_CreateImage(surface);
}

/*static void sync_image(GB_IMG *image)
{
	// Synchronize l'image texture->image
	
	// Puis mets le flag de synchro à false
	image->sync = false;
}*/

static GB_IMG_OWNER _image_owner = {
	"gb.sdl2",
	DEFAULT_IMAGE_FORMAT,
	free_image,
	free_image,
	temp_image,
	NULL, //sync_image,
	};

SDL_Image *IMAGE_get(CIMAGE *_object)
{
	return (SDL_Image *)IMAGE.Check(THIS_IMAGE, &_image_owner);
}

#define check_image IMAGE_get

static void take_image(CIMAGE *_object, SDL_Image *image)
{
	if (image && image->surface)
		IMAGE.Take(THIS_IMAGE, &_image_owner, image, image->surface->w, image->surface->h, image->surface->pixels);
	else
		IMAGE.Take(THIS_IMAGE, &_image_owner, image, 0, 0, NULL);
}

CIMAGE *IMAGE_create(SDL_Image *image)
{
	CIMAGE *img;

	img = (CIMAGE *)GB.New(CLASS_Image, NULL, NULL);
	take_image(img, image);
	return img;
}

SDL_Texture *SDL_GetTextureFromImage(SDL_Image *image, CWINDOW *window)
{
	if (image->texture && image->window != window)
	{
		SDL_DestroyTexture(image->texture);
		GB.Unref(POINTER(&image->window));
		image->texture = NULL;
	}

	if (!image->texture)
	{
		image->texture = SDL_CreateTextureFromSurface(window->renderer, image->surface);
		//SDL_SetTextureBlendMode(image->texture, SDL_BLENDMODE_BLEND);
		image->window = window;
		GB.Ref(window);
	}

	return image->texture;
}

SDL_Texture *IMAGE_get_texture(CIMAGE *_object, CWINDOW *window)
{
	return SDL_GetTextureFromImage(IMAGE_get(THIS), window);
}

CIMAGE *IMAGE_create_from_window(CWINDOW *window, int x, int y, int w, int h)
{
	SDL_Surface *surface;
	SDL_Rect rect = { x, y, w, h };

	surface = SDL_CreateRGBSurface(0, w, h, 32, RMASK, GMASK, BMASK, AMASK);
	SDL_RenderReadPixels(window->renderer, &rect, DEFAULT_SDL_IMAGE_FORMAT, surface->pixels, surface->pitch);
	return IMAGE_create(SDL_CreateImage(surface));
}

//-------------------------------------------------------------------------

BEGIN_METHOD(Image_Load, GB_STRING path)

	char *addr;
	int len;
	SDL_Surface *surface;

	if (GB.LoadFile(STRING(path), LENGTH(path), &addr, &len))
		return;
	
	surface = IMG_Load_RW(SDL_RWFromConstMem(addr, len), TRUE);
	GB.ReleaseFile(addr, len);
	
	if (!surface)
	{
		GB.Error("Unable to load image: &1", IMG_GetError());
		return;
	}
	
	GB.ReturnObject(IMAGE_create(SDL_CreateImage(surface)));

END_METHOD

BEGIN_METHOD(Image_Save, GB_STRING path)

	char *path = GB.FileName(STRING(path), LENGTH(path));

	if (SDL_SaveBMP(IMAGE_get(THIS)->surface, path))
		RAISE_ERROR("Unable to save image: &1");

END_METHOD

//-------------------------------------------------------------------------

GB_DESC ImageDesc[] =
{
	GB_DECLARE("Image", sizeof(CIMAGE)),
	
	GB_STATIC_METHOD("Load", "Image", Image_Load, "(Path)s"),
	GB_METHOD("Save", NULL, Image_Save, "(Path)s"),

	GB_END_DECLARE
};
