/***************************************************************************

  CPicture.cpp

  (c) 2004-2006 - Daniel Campos Fernández <dcamposf@gmail.com>

  GTK+ component

  Realizado para la Junta de Extremadura.
  Consejería de Educación Ciencia y Tecnología.
  Proyecto gnuLinEx

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

#include "main.h"
#include "gambas.h"
#include "widgets.h"
#include "ggambastag.h"
#include "CDraw.h"
#include "CScreen.h"
#include "CPicture.h"
#include "CImage.h"

static void free_image(GB_IMG *img, void *image)
{
	((gPicture *)image)->unref();
}

static void *temp_image(GB_IMG *img)
{
	gPicture *image;
	
	if (!img->data)
		image = new gPicture();
	else
		image = gPicture::fromData((const char *)img->data, img->width, img->height);
	
	image->setTag(new gGambasTag((void *)img));
	
	return image;
}

static GB_IMG_OWNER _image_owner = {
	"gb.gtk",
	GB_IMAGE_RGBA,
	free_image,
	free_image,
	temp_image
	};

gPicture *CIMAGE_get(CIMAGE *_object)
{
	return (gPicture *)IMAGE.Check(&THIS->img, &_image_owner);
}

#define check_image CIMAGE_get

static void take_image(CIMAGE *_object, gPicture *image)
{
	IMAGE.Take(&THIS->img, &_image_owner, image, image->width(), image->height(), image->data());

	if (!image->getTag())
		image->setTag(new gGambasTag(THIS));
}

CIMAGE *CIMAGE_create(gPicture *image)
{
	CIMAGE *img;
  static GB_CLASS class_id = NULL;

  if (!class_id)
    class_id = GB.FindClass("Image");

  GB.New(POINTER(&img), class_id, NULL, NULL);
  
  if (image)
  	take_image(img, image);
	else
  	take_image(img, new gPicture());
	
  return img;
}

/*CIMAGE *CIMAGE_create(gPicture *picture)
{
	CIMAGE *pic;
	GB.New((void **)POINTER(&pic), GB.FindClass("Image"), 0, 0);
	if (picture)
	{
		pic->picture->unref();
		pic->picture = picture;
		picture->setTag(new gGambasTag((void *)pic));
	}
	return pic;
}*/

void* GTK_GetImage(GdkPixbuf *buf)
{
  CIMAGE *pic = CIMAGE_create(new gPicture(buf));
	g_object_ref(buf);
	return (void*)pic;
}

/*******************************************************************************

  Image

*******************************************************************************/

#if 0
BEGIN_METHOD(CIMAGE_new, GB_INTEGER w; GB_INTEGER h; GB_BOOLEAN trans)

  int w = VARGOPT(w, 0);
  int h = VARGOPT(h, 0);
  bool trans = VARGOPT(trans, false);

	IMAGE = new gPicture(gPicture::MEMORY, w, h, trans);
	PICTURE->setTag(new gGambasTag((void *)THIS));

END_METHOD


BEGIN_METHOD_VOID(CIMAGE_free)

	PICTURE->unref();

END_METHOD
#endif

BEGIN_PROPERTY(CIMAGE_picture)

	check_image(THIS);
	
	CPICTURE *pic = CPICTURE_create(PICTURE->copy());
	pic->picture->getPixmap();
	GB.ReturnObject(pic);

END_PROPERTY


BEGIN_METHOD(CIMAGE_load, GB_STRING path)

	CIMAGE *image;
	char *addr;
	int len;

	if (!GB.LoadFile(STRING(path), LENGTH(path), &addr, &len))
	{
		gPicture *pic = gPicture::fromMemory(addr, len);
		GB.ReleaseFile(addr, len);
		
		if (pic)
		{
			image = CIMAGE_create(pic);
			pic->getPixbuf();
			GB.ReturnObject(image);
			return;
		}
	}

	GB.Error("Unable to load image");

END_METHOD


BEGIN_METHOD(CIMAGE_save, GB_STRING path; GB_INTEGER quality)

	check_image(THIS);

	switch (PICTURE->save(GB.FileName(STRING(path), LENGTH(path)), VARGOPT(quality, -1)))
	{
		case 0: break;
		case -1: GB.Error("Unknown format"); break;
		case -2: GB.Error("Unable to save picture"); break;
	}

END_METHOD


BEGIN_METHOD(CIMAGE_stretch, GB_INTEGER width; GB_INTEGER height; GB_BOOLEAN smooth)

	CIMAGE *img;
	bool smooth = VARGOPT(smooth, true);

	check_image(THIS);
  img = CIMAGE_create(PICTURE->stretch(VARG(width), VARG(height), smooth));
	GB.ReturnObject((void*)img);

END_METHOD


BEGIN_METHOD(CIMAGE_rotate, GB_FLOAT angle)

	CIMAGE *img;

	check_image(THIS);
  img = CIMAGE_create(PICTURE->rotate(VARG(angle)));
	GB.ReturnObject((void*)img);

END_METHOD


BEGIN_METHOD(CIMAGE_draw, GB_OBJECT img; GB_INTEGER x; GB_INTEGER y; GB_INTEGER w; GB_INTEGER h; GB_INTEGER sx; GB_INTEGER sy; GB_INTEGER sw; GB_INTEGER sh)

  int x, y, w, h, sx, sy, sw, sh;
  CIMAGE *image = (CIMAGE *)VARG(img);
	gPicture *src;

  if (GB.CheckObject(image))
    return;

	src = check_image(image);
	check_image(THIS);

  x = VARGOPT(x, 0);
  y = VARGOPT(y, 0);
  w = VARGOPT(w, -1);
  h = VARGOPT(h, -1);

  sx = VARGOPT(sx, 0);
  sy = VARGOPT(sy, 0);
  sw = VARGOPT(sw, -1);
  sh = VARGOPT(sh, -1);

  //DRAW_NORMALIZE(x, y, w, h, sx, sy, sw, sh, pic->width(), pic->height());

	PICTURE->draw(src, x, y, w, h, sx, sy, sw, sh);

END_METHOD


GB_DESC CImageDesc[] =
{
  GB_DECLARE("Image", sizeof(CIMAGE)),

  GB_STATIC_METHOD("Load", "Image", CIMAGE_load, "(Path)s"),
  GB_METHOD("Save", 0, CIMAGE_save, "(Path)s[(Quality)i]"),

  GB_METHOD("Stretch", "Image", CIMAGE_stretch, "(Width)i(Height)i[(Smooth)b]"),
  GB_METHOD("Rotate", "Image", CIMAGE_rotate, "(Angle)f"),

  GB_METHOD("Draw", 0, CIMAGE_draw, "(Image)Image;(X)i(Y)i[(Width)i(Height)i(SrcX)i(SrcY)i(SrcWidth)i(SrcHeight)i]"),

  GB_PROPERTY_READ("Picture", "Picture", CIMAGE_picture),

  GB_END_DECLARE
};

