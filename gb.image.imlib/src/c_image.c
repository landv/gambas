/***************************************************************************

  c_image.c

  (c) 2000-2009 Benoît Minisini <gambas@users.sourceforge.net>

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

#define __C_IMAGE_C

#include "gb.draw.h"
#include "c_image.h"

static void free_image(GB_IMG *img, void *image)
{
	imlib_context_set_image((Imlib_Image)image);
	imlib_free_image();
}

static void *temp_image(GB_IMG *img)
{
	Imlib_Image image;

	if (!img->data)
		image = NULL;
	else
	{
		image = imlib_create_image_using_data(img->width, img->height, (DATA32 *)img->data);
	}
	
	return image;
}

static GB_IMG_OWNER _image_owner = {
	"gb.image.imlib",
	GB_IMAGE_BGRA,
	free_image,
	free_image,
	temp_image
	};

Imlib_Image check_image(void *_object)
{
	Imlib_Image image = (Imlib_Image)IMAGE.Check(THIS, &_image_owner);
	imlib_context_set_image(image);
	imlib_image_set_has_alpha(TRUE);
	return image;
}

static void take_image(CIMAGE *_object, Imlib_Image image)
{
	imlib_context_set_image((Imlib_Image)image);
	IMAGE.Take(THIS, &_image_owner, image, imlib_image_get_width(), imlib_image_get_height(), (void *)imlib_image_get_data());
}

CIMAGE *create_image(Imlib_Image image)
{
	CIMAGE *img;
  static GB_CLASS class_id = 0;

  if (!class_id)
    class_id = GB.FindClass("Image");

  GB.New(POINTER(&img), class_id, NULL, NULL);
  take_image(img, image);
  return img;
}

/***************************************************************************/

// Imlib2 cannot load images from memory

#if 0
static const char *get_error(Imlib_Load_Error error)
{
	switch(error)
	{
		case IMLIB_LOAD_ERROR_NONE: return NULL;
		case IMLIB_LOAD_ERROR_FILE_DOES_NOT_EXIST: return "File does not exist";
		case IMLIB_LOAD_ERROR_FILE_IS_DIRECTORY: return "File is a directory";
		case IMLIB_LOAD_ERROR_PERMISSION_DENIED_TO_READ: return "Read permission denied";
		case IMLIB_LOAD_ERROR_NO_LOADER_FOR_FILE_FORMAT: return "Unknown file format";
		case IMLIB_LOAD_ERROR_PATH_TOO_LONG: return "Path too long";
		case IMLIB_LOAD_ERROR_PATH_COMPONENT_NON_EXISTANT: return "Path component does not exist";
		case IMLIB_LOAD_ERROR_PATH_COMPONENT_NOT_DIRECTORY: return "Path component is not a directory";
		case IMLIB_LOAD_ERROR_PATH_POINTS_OUTSIDE_ADDRESS_SPACE: return "Path points oustide of address space";
		case IMLIB_LOAD_ERROR_TOO_MANY_SYMBOLIC_LINKS: return "Too many symbolic links";
		case IMLIB_LOAD_ERROR_OUT_OF_MEMORY: return "Out of memory";
		case IMLIB_LOAD_ERROR_OUT_OF_FILE_DESCRIPTORS: return "Out of file descriptors";
		case IMLIB_LOAD_ERROR_PERMISSION_DENIED_TO_WRITE: return "Write permission denied";
		case IMLIB_LOAD_ERROR_OUT_OF_DISK_SPACE: return "Out of disk space";
		default: return "Unknown error";
	};
}

BEGIN_METHOD(CIMAGE_load, GB_STRING path)

	Imlib_Image image;
	Imlib_Load_Error error;
	char *addr;
	int len;

	if (GB.LoadFile(STRING(path), LENGTH(path), &addr, &len))
	{
		GB.Error("Unable to load image");
		return;
	}

	image = imlib_load_image_with_error_return(GB.ToZeroString(ARG(path)), &error);

	if (image)
		GB.ReturnObject(create_image(image));
	else
    GB.Error("Unable to load image: &1", get_error(error));

END_METHOD


BEGIN_METHOD(CIMAGE_save, GB_STRING path; GB_INTEGER quality)

	Imlib_Load_Error error;

	check_image(THIS);
	
	imlib_context_set_image(THIS_IMAGE);
	imlib_save_image_with_error_return(GB.ToZeroString(ARG(path)), &error);

  if (error != IMLIB_LOAD_ERROR_NONE)
    GB.Error("Unable to save picture: &1", get_error(error));

END_METHOD
#endif

BEGIN_METHOD(Image_Rotate, GB_FLOAT angle)

	Imlib_Image image;

  check_image(THIS);
	imlib_context_set_anti_alias(TRUE);
	image = imlib_create_rotated_image(VARG(angle));
  GB.ReturnObject(create_image(image));

END_METHOD

BEGIN_METHOD(Image_Stretch, GB_INTEGER width; GB_INTEGER height)

	Imlib_Image image;

  check_image(THIS);
	imlib_context_set_anti_alias(TRUE);
	image = imlib_create_cropped_scaled_image(0, 0, imlib_image_get_width(), imlib_image_get_height(), VARG(width), VARG(height));
  GB.ReturnObject(create_image(image));

END_METHOD

BEGIN_METHOD(Image_Blur, GB_INTEGER radius)

	check_image(THIS);
	imlib_image_blur(VARG(radius));

END_METHOD

BEGIN_METHOD(Image_Sharpen, GB_INTEGER radius)

	check_image(THIS);
	imlib_image_sharpen(VARG(radius));

END_METHOD

BEGIN_METHOD(Image_Draw, GB_OBJECT img; GB_INTEGER x; GB_INTEGER y; GB_INTEGER w; GB_INTEGER h; GB_INTEGER sx; GB_INTEGER sy; GB_INTEGER sw; GB_INTEGER sh)

	int x, y, w, h, sx, sy, sw, sh, src_w, src_h;
	CIMAGE *image = (CIMAGE *)VARG(img);
	Imlib_Image src, dst;

	if (GB.CheckObject(image))
		return;

	src = check_image(image);
	src_w = imlib_image_get_width();
	src_h = imlib_image_get_height();
	dst = check_image(THIS);

	x = VARGOPT(x, 0);
	y = VARGOPT(y, 0);
	w = VARGOPT(w, -1);
	h = VARGOPT(h, -1);

	sx = VARGOPT(sx, 0);
	sy = VARGOPT(sy, 0);
	sw = VARGOPT(sw, src_w);
	sh = VARGOPT(sh, src_h);

	DRAW_NORMALIZE(x, y, w, h, sx, sy, sw, sh, src_w, src_h);
	
	imlib_blend_image_onto_image(src, FALSE, x, y, w, h, sx, sy, sw, sh);
	
END_METHOD

GB_DESC CImageDesc[] =
{
  GB_DECLARE("Image", sizeof(CIMAGE)),

  //GB_STATIC_METHOD("Load", "Image", CIMAGE_load, "(Path)s"),
  //GB_METHOD("Save", NULL, CIMAGE_save, "(Path)s[(Quality)i]"),
	
	GB_METHOD("Rotate", "Image", Image_Rotate, "(Angle)f"),
  GB_METHOD("Stretch", "Image", Image_Stretch, "(Width)i(Height)i"),
	
	GB_METHOD("Blur", NULL, Image_Blur, "(Radius)i"),
	GB_METHOD("Sharpen", NULL, Image_Sharpen, "(Radius)i"),

  GB_METHOD("Draw", NULL, Image_Draw, "(Image)Image;(X)i(Y)i[(Width)i(Height)i(SrcX)i(SrcY)i(SrcWidth)i(SrcHeight)i]"),

	//Gb_INTERFACE("Draw", &DRAW_Interface),

  GB_END_DECLARE
};

