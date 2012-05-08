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
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
  MA 02110-1301, USA.

***************************************************************************/

#define __CSVGIMAGE_CPP

#include <cairo.h>
#include <cairo-svg.h>

#include "main.h"
#include "gambas.h"
#include "widgets.h"
#include "cpaint_impl.h"
#include "csvgimage.h"

#define MM_TO_PT(_mm) ((_mm) * 72 / 25.4)
#define PT_TO_MM(_pt) ((_pt) / 72 * 25.4)

static void release(CSVGIMAGE *_object)
{
	if (HANDLE)
	{
		g_object_unref(G_OBJECT(HANDLE));
		HANDLE = NULL;
	}

	if (SURFACE)
	{
		cairo_surface_destroy(SURFACE);
		THIS->surface = NULL;
		unlink(THIS->file);
		GB.FreeString(&THIS->file);
	}
	
	THIS->width = THIS->height = 0;
}

cairo_surface_t *SVGIMAGE_begin(CSVGIMAGE *_object)
{
	if (!SURFACE)
	{
		if (THIS->width <= 0 || THIS->height <= 0)
		{
			GB.Error("SvgImage size is not defined");
			return NULL;
		}
		
		THIS->file = GB.NewZeroString(GB.TempFile(NULL));
		SURFACE = cairo_svg_surface_create(THIS->file, THIS->width, THIS->height);
		
		if (HANDLE)
		{
			cairo_t *context = cairo_create(SURFACE);
			rsvg_handle_render_cairo(HANDLE, context);
			cairo_destroy(context);
		}
	}
	
	return SURFACE;
}

void SVGIMAGE_end(CSVGIMAGE *_object)
{
}

BEGIN_METHOD(SvgImage_new, GB_FLOAT width; GB_FLOAT height)

	THIS->width = VARGOPT(width, 0);
	THIS->height = VARGOPT(height, 0);

END_METHOD

BEGIN_METHOD_VOID(SvgImage_free)

	release(THIS);

END_METHOD

BEGIN_PROPERTY(SvgImage_Width)

	if (READ_PROPERTY)
		GB.ReturnFloat(THIS->width);
	else
		THIS->width = VPROP(GB_FLOAT);
	
END_PROPERTY

BEGIN_PROPERTY(SvgImage_Height)
	
	if (READ_PROPERTY)
		GB.ReturnFloat(THIS->height);
	else
		THIS->height = VPROP(GB_FLOAT);
	
END_PROPERTY

BEGIN_METHOD(SvgImage_Resize, GB_FLOAT width; GB_FLOAT height)

	THIS->width = VARG(width);
	THIS->height = VARG(height);

END_METHOD

static const char *load_file(CSVGIMAGE *_object, const char *path, int len_path)
{
	RsvgHandle *handle = NULL;
	RsvgDimensionData dim;
	char *addr;
	int len;
	const char *err = NULL;

	if (GB.LoadFile(path, len_path, &addr, &len))
		return "Unable to load SVG file";

	handle = rsvg_handle_new_from_data((const guint8 *)addr, len / sizeof(guint8), NULL);
	if (!handle) 
	{
		err = "Unable to load SVG file: invalid format"; 
		goto __RETURN;
	}

	rsvg_handle_set_dpi(handle, 72);
	
	release(THIS);
	THIS->handle = handle;
	rsvg_handle_get_dimensions(handle, &dim);
	THIS->width = dim.width;
	THIS->height = dim.height;

	handle = NULL;

__RETURN:

	if (handle)
		g_object_unref(G_OBJECT(handle));
	
	GB.ReleaseFile(addr, len);
	return err;
}

BEGIN_METHOD(SvgImage_Load, GB_STRING path)

	CSVGIMAGE *svgimage;
	const char *err;

	svgimage = (CSVGIMAGE *)GB.New(CLASS_SvgImage, NULL, NULL);

	if ((err = load_file(svgimage, STRING(path), LENGTH(path))))
	{
		GB.Unref(POINTER(&svgimage));
		GB.Error(err);
		return;
	}
	
	GB.ReturnObject(svgimage);

END_METHOD

BEGIN_METHOD_VOID(SvgImage_Paint)

	cairo_t *context = PAINT_get_current_context();
	const char *err;
	cairo_matrix_t matrix;
	double tx, ty, sx, sy;
	RsvgDimensionData dim;
	
	if (!context)
		return;
	
	if (THIS->file)
	{
		cairo_surface_finish(SURFACE);
		if ((err = load_file(THIS, THIS->file, GB.StringLength(THIS->file))))
		{
			GB.Error(err);
			return;
		}
	}
	
	if (!HANDLE)
		return;
	
	if (THIS->width <= 0 || THIS->height <= 0)
		return;
	
	rsvg_handle_get_dimensions(HANDLE, &dim);
	sx = THIS->width / dim.width;
	sy = THIS->height / dim.height;
	
	cairo_get_matrix(context, &matrix);
	cairo_scale(context, sx, sy);
	cairo_get_current_point(context, &tx, &ty);
	cairo_translate(context, tx, ty);
	rsvg_handle_render_cairo(HANDLE, context);
	cairo_set_matrix(context, &matrix);

END_METHOD

BEGIN_METHOD(SvgImage_Save, GB_STRING file)
	
	if (!THIS->file)
	{
		if (!SVGIMAGE_begin(THIS))
		{
			GB.Error("Void image");
			return;
		}
	}
	
	cairo_surface_finish(SURFACE);
	if (GB.CopyFile(THIS->file, GB.FileName(STRING(file), LENGTH(file))))
		return;
	
	load_file(THIS, THIS->file, GB.StringLength(THIS->file));

END_METHOD

BEGIN_METHOD_VOID(SvgImage_Clear)

	release(THIS);

END_METHOD

GB_DESC SvgImageDesc[] =
{
  GB_DECLARE("SvgImage", sizeof(CSVGIMAGE)),

  GB_METHOD("_new", NULL, SvgImage_new, "[(Width)f(Height)f]"),
  GB_METHOD("_free", NULL, SvgImage_free, NULL),

  GB_PROPERTY("Width", "f", SvgImage_Width),
  GB_PROPERTY("Height", "f", SvgImage_Height),
  GB_METHOD("Resize", NULL, SvgImage_Resize, "(Width)f(Height)f"),

  GB_STATIC_METHOD("Load", "SvgImage", SvgImage_Load, "(Path)s"),
  GB_METHOD("Save", NULL, SvgImage_Save, "(Path)s"),
	GB_METHOD("Paint", NULL, SvgImage_Paint, NULL),

  GB_METHOD("Clear", NULL, SvgImage_Clear, NULL),

  GB_INTERFACE("Paint", &PAINT_Interface),

  GB_END_DECLARE
};

