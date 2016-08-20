/***************************************************************************

  c_surface.c

  (c) 2000-2013 Benoît Minisini <gambas@users.sourceforge.net>

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

#define __C_SURFACE_C

#include <cairo-pdf.h>
#include <cairo-ps.h>
#include <cairo-svg.h>

#include "c_surface.h"

BEGIN_METHOD_VOID(CairoSurface_free)

	GB.FreeString(&THIS->path);
	cairo_surface_destroy(THIS->surface);

END_METHOD

BEGIN_PROPERTY(CairoSurface_Status)

	GB.ReturnInteger(cairo_surface_status(THIS->surface));

END_PROPERTY

BEGIN_PROPERTY(CairoSurface_Path)

	GB.ReturnString(THIS->path);

END_PROPERTY

BEGIN_METHOD(CairoSurface_Save, GB_STRING path)

	GB.ReturnInteger(cairo_surface_write_to_png(THIS->surface, GB.FileName(STRING(path), LENGTH(path))));

END_METHOD

BEGIN_PROPERTY(CairoSurface_Resolution)

	if (READ_PROPERTY)
	{
		double x, y;
		cairo_surface_get_fallback_resolution(THIS->surface, &x, &y);
		GB.ReturnFloat(MAX(x, y));
	}
	else
	{
		double r = VPROP(GB_FLOAT);
		cairo_surface_set_fallback_resolution(THIS->surface, r, r);
	}

END_PROPERTY

BEGIN_METHOD_VOID(CairoSurface_Finish)

	cairo_surface_finish(THIS->surface);

END_METHOD

GB_DESC CairoSurfaceDesc[] = 
{
	GB_DECLARE("CairoSurface", sizeof(CAIRO_SURFACE)), GB_NOT_CREATABLE(),

	GB_METHOD("_free", NULL, CairoSurface_free, NULL),
	
	//GB_STATIC_METHOD("_call", "CairoSurface", CairoSurface_call, "(Path)s(Width)f(Height)f"),
	GB_METHOD("Save", "i", CairoSurface_Save, "(Path)s"),
	GB_METHOD("Finish", NULL, CairoSurface_Finish, NULL),
	
	GB_PROPERTY_READ("Status", "i", CairoSurface_Status),
	GB_PROPERTY_READ("Path", "s", CairoSurface_Path),
	GB_PROPERTY("Resolution", "f", CairoSurface_Resolution),

	GB_END_DECLARE	
};


/**************************************************************************/

#define CM_TO_PT(_cm) ((_cm) / 25.4 * 72.0)

BEGIN_METHOD(CairoPdfSurface_new, GB_STRING path; GB_FLOAT width; GB_FLOAT height; GB_STRING version)

	char *version = NULL;
	
	if (!MISSING(version))
		version = GB.ToZeroString(ARG(version));
	
	THIS->path = GB.NewZeroString(GB.FileName(STRING(path), LENGTH(path)));
	THIS->surface = cairo_pdf_surface_create(THIS->path, CM_TO_PT(VARG(width)), CM_TO_PT(VARG(height)));

	#if CAIRO_VERSION >= CAIRO_VERSION_ENCODE(1, 10, 0)
	if (version)
	{
		if (!strcmp(version, "1.4"))
			cairo_pdf_surface_restrict_to_version(THIS->surface, CAIRO_PDF_VERSION_1_4);
		else if (!strcmp(version, "1.5"))
			cairo_pdf_surface_restrict_to_version(THIS->surface, CAIRO_PDF_VERSION_1_5);
	}
	#endif

END_METHOD

BEGIN_METHOD(CairoPdfSurface_Resize, GB_FLOAT width; GB_FLOAT height)

	cairo_pdf_surface_set_size(THIS->surface, CM_TO_PT(VARG(width)), CM_TO_PT(VARG(height)));

END_METHOD

GB_DESC CairoPdfSurfaceDesc[] = 
{
	GB_DECLARE("CairoPdfSurface", sizeof(CAIRO_SURFACE)), GB_INHERITS("CairoSurface"),

	GB_METHOD("_new", NULL, CairoPdfSurface_new, "(Path)s(Width)f(Height)f[(Version)s]"),
	GB_METHOD("Resize", NULL, CairoPdfSurface_Resize, "(Width)f(Height)f"),
	
	GB_END_DECLARE	
};


/**************************************************************************/

BEGIN_METHOD(CairoPsSurface_new, GB_STRING path; GB_FLOAT width; GB_FLOAT height; GB_BOOLEAN eps; GB_STRING level)

	char *level = NULL;
	
	if (!MISSING(level))
		level = GB.ToZeroString(ARG(level));
	
	THIS->path = GB.NewZeroString(GB.FileName(STRING(path), LENGTH(path)));
	THIS->surface = cairo_ps_surface_create(THIS->path, CM_TO_PT(VARG(width)), CM_TO_PT(VARG(height)));

	if (level)
	{
		if (!strcmp(level, "2"))
			cairo_ps_surface_restrict_to_level(THIS->surface, CAIRO_PS_LEVEL_2);
		else if (!strcmp(level, "3"))
			cairo_ps_surface_restrict_to_level(THIS->surface, CAIRO_PS_LEVEL_3);
	}
	
	cairo_ps_surface_set_eps(THIS->surface, VARGOPT(eps, FALSE));

END_METHOD

BEGIN_METHOD(CairoPsSurface_Resize, GB_FLOAT width; GB_FLOAT height)

	cairo_ps_surface_set_size(THIS->surface, CM_TO_PT(VARG(width)), CM_TO_PT(VARG(height)));

END_METHOD

GB_DESC CairoPsSurfaceDesc[] = 
{
	GB_DECLARE("CairoPsSurface", sizeof(CAIRO_SURFACE)), GB_INHERITS("CairoSurface"),

	GB_METHOD("_new", NULL, CairoPsSurface_new, "(Path)s(Width)f(Height)f[(Encapsulated)b(Level)s]"),
	GB_METHOD("Resize", NULL, CairoPsSurface_Resize, "(Width)f(Height)f"),
	
	GB_END_DECLARE	
};


/**************************************************************************/

BEGIN_METHOD(CairoSvgSurface_new, GB_STRING path; GB_FLOAT width; GB_FLOAT height; GB_STRING version)

	char *version = NULL;
	
	if (!MISSING(version))
		version = GB.ToZeroString(ARG(version));
	
	THIS->path = GB.NewZeroString(GB.FileName(STRING(path), LENGTH(path)));
	THIS->surface = cairo_svg_surface_create(THIS->path, CM_TO_PT(VARG(width)), CM_TO_PT(VARG(height)));

	if (version)
	{
		if (!strcmp(version, "1.1"))
			cairo_svg_surface_restrict_to_version(THIS->surface, CAIRO_SVG_VERSION_1_1);
		else if (!strcmp(version, "1.2"))
			cairo_svg_surface_restrict_to_version(THIS->surface, CAIRO_SVG_VERSION_1_2);
	}

END_METHOD

GB_DESC CairoSvgSurfaceDesc[] = 
{
	GB_DECLARE("CairoSvgSurface", sizeof(CAIRO_SURFACE)), GB_INHERITS("CairoSurface"),

	GB_METHOD("_new", NULL, CairoSvgSurface_new, "(Path)s(Width)f(Height)f[(Version)s]"),
	
	GB_END_DECLARE	
};
