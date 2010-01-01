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

static void release(CSVGIMAGE *_object)
{
	if (HANDLE)
	{
		rsvg_handle_free(HANDLE);
		HANDLE = NULL;
	}
	
	if (SURFACE)
	{
		cairo_surface_destroy(SURFACE);
		THIS->surface = NULL;
		unlink(THIS->file);
		GB.FreeString(&THIS->file);
	}
}

cairo_surface_t *SVGIMAGE_init(CSVGIMAGE *_object)
{
	if (!SURFACE)
	{
		if (THIS->width <= 0 || THIS->height <= 0)
			return NULL;
		
		GB.NewString(&THIS->file, GB.TempFile(NULL), 0);
		THIS->surface = cairo_svg_surface_create(THIS->file, THIS->width, THIS->height);
	}
	
	return SURFACE;
}

BEGIN_METHOD(SvgImage_new, GB_FLOAT width; GB_FLOAT height)

	THIS->width = MM_TO_PT(VARGOPT(width, 0));
	THIS->height = MM_TO_PT(VARGOPT(height, 0));

END_METHOD

BEGIN_METHOD_VOID(SvgImage_free)

	release(THIS);

END_METHOD

BEGIN_PROPERTY(SvgImage_Width)

	if (READ_PROPERTY)
		GB.ReturnFloat(PT_TO_MM(THIS->width));
	else
		THIS->width = MM_TO_PT(VPROP(GB_FLOAT));
	
END_PROPERTY

BEGIN_PROPERTY(SvgImage_Height)
	
	if (READ_PROPERTY)
		GB.ReturnFloat(PT_TO_MM(THIS->height));
	else
		THIS->height = MM_TO_PT(VPROP(GB_FLOAT));
	
END_PROPERTY

BEGIN_METHOD(SvgImage_Save, GB_STRING file)
	
	if (!THIS->file)
		GB.Error("Void image");
	else
		GB.CopyFile(THIS->file, GB.FileName(STRING(file), LENGTH(file)));

END_METHOD

BEGIN_METHOD_VOID(SvgImage_Clear)

	release(THIS);

END_METHOD

BEGIN_METHOD(SvgImage_Load, GB_STRING path)

	CSVGIMAGE *svgimage;
	RsvgHandle *handle = NULL;
	char *addr;
	int len;
	int len_read;

	if (GB.LoadFile(STRING(path), LENGTH(path), &addr, &len))
	{
		GB.Error("Unable to load SVG file");
		return;
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
	
	GB.New (POINTER(&svgimage), CLASS_SvgImage, NULL, NULL);
	svgimage->handle = handle;
	handle = NULL;
	GB.ReturnObject(svgimage);

__RETURN:

	if (handle)
		rsvg_handle_free(handle);
	
	GB.ReleaseFile(addr,len);
	return;
	
END_METHOD

BEGIN_METHOD_VOID(SvgImage_Paint)

	cairo_t *context = PAINT_get_current_context();
	
	if (!context)
		return;
	
	if (!HANDLE)
		return;
	
	rsvg_handle_render_cairo(HANDLE, context);

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

