/***************************************************************************

  Cimage.cpp

  Gambas extension using SDL

  (c) 2006 Laurent Carlier <lordheavy@infonie.fr>
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

static void create(CIMAGE **pimage)
{
  static GB_CLASS class_id = NULL;

  if (!class_id)
    class_id = GB.FindClass("Image");

  GB.New((void **)pimage, class_id, NULL, NULL);
}

/***************************************************************************/

BEGIN_METHOD(CIMAGE_new, GB_INTEGER w; GB_INTEGER h; GB_BOOLEAN trans)

	int w,h;
	IMAGEID = new SDLsurface();

	if (!MISSING(w) && !MISSING(h))
	{
		w = VARG(w);
		h = VARG(h);
		if (h <= 0 || w <= 0)
		{
			SDLerror::RaiseError("Bad dimension");
			return;
		}

		IMAGEID->Create(w, h, 32);
		IMAGEID->SetAlphaBuffer(VARGOPT(trans, false));
	}

END_METHOD

BEGIN_METHOD_VOID(CIMAGE_free)

	delete IMAGEID;
	IMAGEID = NULL;

END_METHOD

BEGIN_METHOD(CIMAGE_load, GB_STRING path)

	char *addr;
	long len;
	SDLsurface *mySurface = new SDLsurface();
	CIMAGE *image;

	if (!(GB.LoadFile(STRING(path), LENGTH(path), &addr, &len)))
	{
		mySurface->LoadFromMem(addr, len);

		if (mySurface->GetDepth() != 32)
			mySurface->ConvertDepth(32);

		create (&image);

		if (image->id)
			delete image->id;

		image->id = mySurface;
		GB.ReturnObject(image);

		GB.ReleaseFile(&addr, len);
	}
	else
		SDLerror::RaiseError("Unable to load image");

END_METHOD

BEGIN_METHOD_VOID(CIMAGE_clear)

	IMAGEID->Fill();

END_METHOD

BEGIN_METHOD(CIMAGE_fill, GB_INTEGER color)

	IMAGEID->Fill(VARG(color));

END_METHOD

BEGIN_METHOD(CIMAGE_resize, GB_INTEGER width; GB_INTEGER height)

	IMAGEID->Resize(VARG(width), VARG(height));

END_METHOD

BEGIN_PROPERTY(CIMAGE_width)

	GB.ReturnInteger(IMAGEID->GetWidth());

END_PROPERTY

BEGIN_PROPERTY(CIMAGE_height)

	GB.ReturnInteger(IMAGEID->GetHeight());

END_PROPERTY

BEGIN_PROPERTY(CIMAGE_depth)

	GB.ReturnInteger(IMAGEID->GetDepth());

END_PROPERTY

/***************************************************************************/

GB_DESC CImage[] =
{
  GB_DECLARE("Image", sizeof(CIMAGE)),

  GB_METHOD("_new", NULL, CIMAGE_new, "[(Width)i(Height)i(Transparent)b]"),
  GB_METHOD("_free", NULL, CIMAGE_free, NULL),

  GB_STATIC_METHOD("Load", "Image", CIMAGE_load, "(Path)s"),

  GB_METHOD("Clear", NULL, CIMAGE_clear, NULL),
  GB_METHOD("Fill", NULL, CIMAGE_fill, "(Color)i"),
  GB_METHOD("Resize", NULL, CIMAGE_resize, "(Width)i(Height)i"),

  GB_PROPERTY_READ("Width", "i", CIMAGE_width),
  GB_PROPERTY_READ("Height", "i", CIMAGE_height),
  GB_PROPERTY_READ("Depth", "i", CIMAGE_depth),

  GB_END_DECLARE
};

