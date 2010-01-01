/***************************************************************************

  csvgimage.cpp

  (c) 2004-2006 - Daniel Campos Fernández <dcamposf@gmail.com>

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

#define __CSVGIMAGE_CPP

#include <cairo.h>
#include <cairo-svg.h>

#include "main.h"
#include "gambas.h"
#include "widgets.h"
#include "cpaint_impl.h"
#include "csvgimage.h"

#define MM_TO_PT(_mm) ((int)((_mm) * 72 / 25.4 + 0.5))
#define PT_TO_MM(_pt) ((_pt) / 72 * 25.4)

static void init(CSVGIMAGE *_object)
{
	GB.NewString(&THIS->file, GB.TempFile(NULL), 0);
	THIS->surface = cairo_svg_surface_create(THIS->file, THIS->width, THIS->height);
}

static void release(CSVGIMAGE *_object)
{
	cairo_surface_destroy(THIS->surface);
	unlink(THIS->file);
	GB.FreeString(&THIS->file);
}

BEGIN_METHOD(SvgImage_new, GB_FLOAT width; GB_FLOAT height)

	THIS->width = MM_TO_PT(VARG(width));
	THIS->height = MM_TO_PT(VARG(height));
	init(THIS);

END_METHOD

BEGIN_METHOD_VOID(SvgImage_free)

	release(THIS);

END_METHOD

BEGIN_PROPERTY(SvgImage_Width)

	GB.ReturnInteger(PT_TO_MM(THIS->width));
	
END_PROPERTY

BEGIN_PROPERTY(SvgImage_Height)

	GB.ReturnInteger(PT_TO_MM(THIS->height));
	
END_PROPERTY

BEGIN_METHOD(SvgImage_Save, GB_STRING file)

	GB.CopyFile(THIS->file, GB.FileName(STRING(file), LENGTH(file)));

END_METHOD

BEGIN_METHOD_VOID(SvgImage_Clear)

	release(THIS);
	init(THIS);

END_METHOD

GB_DESC SvgImageDesc[] =
{
  GB_DECLARE("SvgImage", sizeof(CSVGIMAGE)),

  GB_METHOD("_new", 0, SvgImage_new, "(Width)f(Height)f"),
  GB_METHOD("_free", 0, SvgImage_free, 0),

  GB_PROPERTY_READ("Width", "f", SvgImage_Width),
  GB_PROPERTY_READ("Height", "f", SvgImage_Height),

  //GB_STATIC_METHOD("Load", "Picture", CPICTURE_load, "(Path)s"),
  GB_METHOD("Save", 0, SvgImage_Save, "(Path)s"),

  GB_METHOD("Clear", 0, SvgImage_Clear, 0),

  GB_INTERFACE("Paint", &PAINT_Interface),

  GB_END_DECLARE
};

