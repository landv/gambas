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

#define MM_TO_PT(_mm) ((_mm) * 72 / 25.4)
#define PT_TO_MM(_pt) ((_pt) / 72 * 25.4)

static void release_handle(CSVGIMAGE *_object)
{
	if (HANDLE)
	{
		rsvg_handle_free(HANDLE);
		HANDLE = NULL;
	}
}

static void release(CSVGIMAGE *_object)
{
	release_handle(THIS);
	
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
		
		GB.NewString(&THIS->file, GB.TempFile(NULL), 0);
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

static bool load_file(CSVGIMAGE *_object, const char *path, int len_path)
{
	RsvgHandle *handle = NULL;
	RsvgDimensionData dim;
	char *addr;
	int len;
	int len_read;
	bool ret = true;

	if (GB.LoadFile(path, len_path, &addr, &len))
	{
		GB.Error("Unable to load SVG file");
		return true;
	}

	/*if (!len) 
	{
		GB.ReleaseFile(addr,len); 
		GB.Error("Invalid format");
		GB.ReturnNull(); 
		return; 
	}*/

	handle = rsvg_handle_new();
	if (!handle) 
	{
		GB.Error("Unable to load SVG file: unable to create SVG handle"); 
		goto __RETURN;
	}

	rsvg_handle_set_dpi(handle, 72);
	
	len_read = 1024;
	while (len)
	{
		if (len < len_read)
			len_read = len;

		len -= len_read;	

		if (!rsvg_handle_write(handle, (const guchar*)addr, len_read, NULL))
		{
			GB.Error("Unable to load SVG file: invalid format");
			goto __RETURN;
		}
		
		addr += len_read;
	}

	if (!rsvg_handle_close(handle, NULL))
	{
		GB.Error("Unable to load SVG file: invalid format");
		goto __RETURN;
	}
	
	release(THIS);
	THIS->handle = handle;
	rsvg_handle_get_dimensions(handle, &dim);
	THIS->width = dim.width;
	THIS->height = dim.height;

	handle = NULL;
	ret = false;

__RETURN:

	if (handle)
		rsvg_handle_free(handle);
	
	GB.ReleaseFile(addr,len);
	return ret;
}

BEGIN_METHOD(SvgImage_Load, GB_STRING path)

	CSVGIMAGE *svgimage;

	GB.New(POINTER(&svgimage), CLASS_SvgImage, NULL, NULL);

	if (load_file(svgimage, STRING(path), LENGTH(path)))
	{
		GB.Unref(POINTER(&svgimage));
		return;
	}
	
	GB.ReturnObject(svgimage);

END_METHOD

BEGIN_METHOD_VOID(SvgImage_Paint)

	cairo_t *context = PAINT_get_current_context();
	
	if (!context)
		return;
	
	if (THIS->file)
	{
		cairo_surface_finish(SURFACE);
		load_file(THIS, THIS->file, GB.StringLength(THIS->file));
	}
	
	if (!HANDLE)
		return;
	
	rsvg_handle_render_cairo(HANDLE, context);

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
	GB.CopyFile(THIS->file, GB.FileName(STRING(file), LENGTH(file)));
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

  GB_STATIC_METHOD("Load", "SvgImage", SvgImage_Load, "(Path)s"),
  GB_METHOD("Save", NULL, SvgImage_Save, "(Path)s"),
	GB_METHOD("Paint", NULL, SvgImage_Paint, NULL),

  GB_METHOD("Clear", NULL, SvgImage_Clear, NULL),

  GB_INTERFACE("Paint", &PAINT_Interface),

  GB_END_DECLARE
};

