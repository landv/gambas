/***************************************************************************

  c_cairo.c

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

#define __C_CAIRO_C

#include "c_surface.h"
#include "c_cairo.h"

typedef
	struct CAIRO_DRAW {
		struct CAIRO_DRAW *previous;
		void *device;
		cairo_surface_t *surface;
		cairo_t *context;
		CAIRO_PATTERN *pattern;
		char *font_family;
		cairo_font_weight_t font_weight;
		cairo_font_slant_t font_slant;
		double font_size;
		}
	CAIRO_DRAW;

CAIRO_DRAW *_current = NULL;

#define THIS _current
#define CNT (_current->context)
#define SURFACE (_current->surface)

static void free_image(GB_IMG *img, void *image)
{
	//fprintf(stderr, "free_image: %p %p (%d)\n", img, image, cairo_surface_get_reference_count((cairo_surface_t *)image));
	cairo_surface_destroy((cairo_surface_t *)image);
}

static void *temp_image(GB_IMG *img)
{
	cairo_surface_t *image;

	if (!img->data)
		image = NULL; // TODO: use a static small image surface
	else
		image = cairo_image_surface_create_for_data(img->data, CAIRO_FORMAT_ARGB32, img->width, img->height, 
		                                            cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, img->width));
		
	//fprintf(stderr, "temp_image: %p -> %p (%d)\n", img, image, cairo_surface_get_reference_count(image));
	return image;
}

static GB_IMG_OWNER _image_owner = {
	"gb.cairo",
	GB_IMAGE_BGRP,
	free_image,
	free_image,
	temp_image,
	NULL,
	};

static cairo_surface_t *check_image(void *img)
{
	// TODO: format is endian-dependent
	return (cairo_surface_t *)IMAGE.Check((GB_IMG *)img, &_image_owner);
}

static bool check_device()
{
	if (!_current)
	{
		GB.Error("No current device");
		return TRUE;
	}
	else
		return FALSE;
}

#define CHECK_CNT() if (check_device()) return


/**** CairoExtents *********************************************************/

#define IMPLEMENT_EXTENTS_PROPERTY(_method, _field) \
BEGIN_PROPERTY(_method) \
	GB.ReturnFloat(THIS_EXTENTS->_field); \
END_PROPERTY

IMPLEMENT_EXTENTS_PROPERTY(CAIRO_EXTENTS_x1, x1)
IMPLEMENT_EXTENTS_PROPERTY(CAIRO_EXTENTS_y1, y1)
IMPLEMENT_EXTENTS_PROPERTY(CAIRO_EXTENTS_x2, x2)
IMPLEMENT_EXTENTS_PROPERTY(CAIRO_EXTENTS_y2, y2)

BEGIN_METHOD(CAIRO_EXTENTS_merge, GB_OBJECT extents)

	CAIRO_EXTENTS *extents = VARG(extents);
	
	if (GB.CheckObject(extents))
		return;
		
	if (extents->x1 < THIS_EXTENTS->x1) THIS_EXTENTS->x1 = extents->x1;
	if (extents->y1 < THIS_EXTENTS->y1) THIS_EXTENTS->y1 = extents->y1;
	if (extents->x2 > THIS_EXTENTS->x2) THIS_EXTENTS->x2 = extents->x2;
	if (extents->y2 > THIS_EXTENTS->y2) THIS_EXTENTS->y2 = extents->y2;

END_METHOD

GB_DESC CairoExtentsDesc[] = 
{
	GB_DECLARE("CairoExtents", sizeof(CAIRO_EXTENTS)),
	
	GB_PROPERTY_READ("X1", "f", CAIRO_EXTENTS_x1),
	GB_PROPERTY_READ("Y1", "f", CAIRO_EXTENTS_y1),
	GB_PROPERTY_READ("X2", "f", CAIRO_EXTENTS_x2),
	GB_PROPERTY_READ("Y2", "f", CAIRO_EXTENTS_y2),
	
	GB_METHOD("Merge", "CairoExtents", CAIRO_EXTENTS_merge, "(Extents)CairoExtents;"),
	
	GB_END_DECLARE
};

/**** CairoTextExtents *********************************************************/

#define IMPLEMENT_TEXT_EXTENTS_PROPERTY(_method, _field) \
BEGIN_PROPERTY(_method) \
	GB.ReturnFloat(THIS_TEXT_EXTENTS->e._field); \
END_PROPERTY

IMPLEMENT_TEXT_EXTENTS_PROPERTY(CairoTextExtents_XBearing, x_bearing)
IMPLEMENT_TEXT_EXTENTS_PROPERTY(CairoTextExtents_YBearing, y_bearing)
IMPLEMENT_TEXT_EXTENTS_PROPERTY(CairoTextExtents_Width, width)
IMPLEMENT_TEXT_EXTENTS_PROPERTY(CairoTextExtents_Height, height)
IMPLEMENT_TEXT_EXTENTS_PROPERTY(CairoTextExtents_XAdvance, x_advance)
IMPLEMENT_TEXT_EXTENTS_PROPERTY(CairoTextExtents_YAdvance, y_advance)

GB_DESC CairoTextExtentsDesc[] = 
{
	GB_DECLARE("CairoTextExtents", sizeof(CAIRO_TEXT_EXTENTS)),
	
	GB_PROPERTY_READ("XBearing", "f", CairoTextExtents_XBearing),
	GB_PROPERTY_READ("YBearing", "f", CairoTextExtents_YBearing),
	GB_PROPERTY_READ("Width", "f", CairoTextExtents_Width),
	GB_PROPERTY_READ("Height", "f", CairoTextExtents_Height),
	GB_PROPERTY_READ("XAdvance", "f", CairoTextExtents_XAdvance),
	GB_PROPERTY_READ("YAdvance", "f", CairoTextExtents_YAdvance),
	
	GB_END_DECLARE
};

/**** CairoFontExtents *********************************************************/

#define IMPLEMENT_FONT_EXTENTS_PROPERTY(_method, _field) \
BEGIN_PROPERTY(_method) \
	GB.ReturnFloat(THIS_FONT_EXTENTS->e._field); \
END_PROPERTY

IMPLEMENT_FONT_EXTENTS_PROPERTY(CairoFontExtents_Ascent, ascent)
IMPLEMENT_FONT_EXTENTS_PROPERTY(CairoFontExtents_Descent, descent)
IMPLEMENT_FONT_EXTENTS_PROPERTY(CairoFontExtents_Height, height)
IMPLEMENT_FONT_EXTENTS_PROPERTY(CairoFontExtents_MaxXAdvance, max_x_advance)
IMPLEMENT_FONT_EXTENTS_PROPERTY(CairoFontExtents_MaxYAdvance, max_y_advance)

GB_DESC CairoFontExtentsDesc[] = 
{
	GB_DECLARE("CairoFontExtents", sizeof(CAIRO_TEXT_EXTENTS)),
	
	GB_PROPERTY_READ("Ascent", "f", CairoFontExtents_Ascent),
	GB_PROPERTY_READ("Descent", "f", CairoFontExtents_Descent),
	GB_PROPERTY_READ("Height", "f", CairoFontExtents_Height),
	GB_PROPERTY_READ("MaxXAdvance", "f", CairoFontExtents_MaxXAdvance),
	GB_PROPERTY_READ("MaxYAdvance", "f", CairoFontExtents_MaxYAdvance),
	
	GB_END_DECLARE
};

/**** CairoMatrix **********************************************************/

BEGIN_METHOD(CAIRO_MATRIX_new, GB_FLOAT xx; GB_FLOAT xy; GB_FLOAT yx; GB_FLOAT yy; GB_FLOAT x0; GB_FLOAT y0)

	cairo_matrix_init(&THIS_MATRIX->matrix, 
		VARGOPT(xx, 1.0),
		VARGOPT(xy, 0.0),
		VARGOPT(yx, 0.0),
		VARGOPT(yy, 1.0),
		VARGOPT(x0, 0.0),
		VARGOPT(y0, 0.0));
		
END_METHOD

BEGIN_METHOD(CAIRO_MATRIX_call, GB_FLOAT xx; GB_FLOAT xy; GB_FLOAT yx; GB_FLOAT yy; GB_FLOAT x0; GB_FLOAT y0)

	CAIRO_MATRIX *matrix;
	
	matrix = GB.New(GB.FindClass("CairoMatrix"), NULL, NULL);
	
	cairo_matrix_init(&matrix->matrix, 
		VARGOPT(xx, 1.0),
		VARGOPT(xy, 0.0),
		VARGOPT(yx, 0.0),
		VARGOPT(yy, 1.0),
		VARGOPT(x0, 0.0),
		VARGOPT(y0, 0.0));
		
	GB.ReturnObject(matrix);
		
END_METHOD

BEGIN_METHOD(CAIRO_MATRIX_translate, GB_FLOAT tx; GB_FLOAT ty)

	cairo_matrix_translate(&THIS_MATRIX->matrix, VARG(tx), VARG(ty));
	GB.ReturnObject(THIS_MATRIX);

END_METHOD

BEGIN_METHOD(CAIRO_MATRIX_scale, GB_FLOAT sx; GB_FLOAT sy)

	cairo_matrix_scale(&THIS_MATRIX->matrix, VARG(sx), VARG(sy));
	GB.ReturnObject(THIS_MATRIX);

END_METHOD

BEGIN_METHOD(CAIRO_MATRIX_rotate, GB_FLOAT angle)

	cairo_matrix_rotate(&THIS_MATRIX->matrix, VARG(angle));
	GB.ReturnObject(THIS_MATRIX);

END_METHOD

BEGIN_METHOD_VOID(CAIRO_MATRIX_invert)

	if (cairo_matrix_invert(&THIS_MATRIX->matrix) == CAIRO_STATUS_SUCCESS)
		GB.ReturnObject(THIS_MATRIX);
	else
		GB.ReturnNull();

END_METHOD

BEGIN_METHOD(CAIRO_MATRIX_multiply, GB_OBJECT matrix)

	CAIRO_MATRIX *matrix = (CAIRO_MATRIX *)VARG(matrix);
	
	if (GB.CheckObject(matrix))
		return;
		
	cairo_matrix_multiply(&THIS_MATRIX->matrix, &THIS_MATRIX->matrix, &matrix->matrix);
	GB.ReturnObject(THIS_MATRIX);

END_METHOD

GB_DESC CairoMatrixDesc[] = 
{
	GB_DECLARE("CairoMatrix", sizeof(CAIRO_MATRIX)),

	GB_METHOD("_new", NULL, CAIRO_MATRIX_new, "[(XX)f(YX)f(XY)f(YY)f(X0)f(Y0)f]"),
	GB_STATIC_METHOD("_call", "CairoMatrix", CAIRO_MATRIX_call, "[(XX)f(YX)f(XY)f(YY)f(X0)f(Y0)f]"),
	GB_METHOD("Translate", "CairoMatrix", CAIRO_MATRIX_translate, "(TX)f(TY)f"),
	GB_METHOD("Scale", "CairoMatrix", CAIRO_MATRIX_scale, "(SX)f(SY)f"),
	GB_METHOD("Rotate", "CairoMatrix", CAIRO_MATRIX_rotate, "(Angle)f"),
	GB_METHOD("Invert", "CairoMatrix", CAIRO_MATRIX_invert, NULL),
	GB_METHOD("Multiply", "CairoMatrix", CAIRO_MATRIX_multiply, "(Matrix)CairoMatrix;"),	

	GB_END_DECLARE	
};

/**** CairoPattern *********************************************************/

BEGIN_METHOD_VOID(CAIRO_PATTERN_free)

	cairo_pattern_destroy(THIS_PATTERN->pattern);
	if (THIS_PATTERN->ref)
		GB.Unref(POINTER(&THIS_PATTERN->ref));

END_METHOD

BEGIN_PROPERTY(CAIRO_PATTERN_filter)

	if (READ_PROPERTY)
		GB.ReturnInteger(cairo_pattern_get_filter(THIS_PATTERN->pattern));
	else
		cairo_pattern_set_filter(THIS_PATTERN->pattern, VPROP(GB_INTEGER));

END_PROPERTY

BEGIN_PROPERTY(CAIRO_PATTERN_matrix)

	CAIRO_MATRIX *matrix;
	
	if (READ_PROPERTY)
	{
		matrix = GB.New(GB.FindClass("CairoMatrix"), NULL, NULL);
		cairo_pattern_get_matrix(THIS_PATTERN->pattern, &matrix->matrix);
		GB.ReturnObject(matrix);
	}
	else
	{
		matrix = (CAIRO_MATRIX *)VPROP(GB_OBJECT);
		if (!matrix)
		{
			cairo_matrix_t m;
			cairo_matrix_init_identity(&m);
			cairo_pattern_set_matrix(THIS_PATTERN->pattern, &m);
		}
		else
			cairo_pattern_set_matrix(THIS_PATTERN->pattern, &matrix->matrix);
	}

END_PROPERTY

GB_DESC CairoPatternDesc[] = 
{
	GB_DECLARE("CairoPattern", sizeof(CAIRO_PATTERN)), GB_NOT_CREATABLE(),

	GB_METHOD("_free", NULL, CAIRO_PATTERN_free, NULL),
	
	GB_PROPERTY("Filter", "i", CAIRO_PATTERN_filter),
	GB_PROPERTY("Matrix", "CairoMatrix", CAIRO_PATTERN_matrix),

	GB_END_DECLARE
};


/**** CairoFont *********************************************************/

static void update_font()
{
	char *family = THIS->font_family;
	
	if (!family)
		family = "";
	
	cairo_select_font_face(CNT, family, THIS->font_slant, THIS->font_weight);
}

BEGIN_PROPERTY(CairoFont_Name)

	CHECK_CNT();

	if (READ_PROPERTY)
		GB.ReturnString(THIS->font_family);
	else
	{
		GB.StoreString(PROP(GB_STRING), &(THIS->font_family));
		update_font();
	}

END_PROPERTY

BEGIN_PROPERTY(CairoFont_Bold)

	CHECK_CNT();

	if (READ_PROPERTY)
		GB.ReturnBoolean(THIS->font_weight != CAIRO_FONT_WEIGHT_NORMAL);
	else
	{
		THIS->font_weight = VPROP(GB_BOOLEAN) ? CAIRO_FONT_WEIGHT_BOLD : CAIRO_FONT_WEIGHT_NORMAL;
		update_font();
	}

END_PROPERTY

BEGIN_PROPERTY(CairoFont_Weight)

	CHECK_CNT();

	if (READ_PROPERTY)
		GB.ReturnInteger(THIS->font_weight);
	else
	{
		THIS->font_weight = VPROP(GB_INTEGER);
		update_font();
	}

END_PROPERTY

BEGIN_PROPERTY(CairoFont_Italic)

	CHECK_CNT();

	if (READ_PROPERTY)
		GB.ReturnBoolean(THIS->font_slant != CAIRO_FONT_SLANT_NORMAL);
	else
	{
		THIS->font_slant = VPROP(GB_BOOLEAN) ? CAIRO_FONT_SLANT_ITALIC : CAIRO_FONT_SLANT_NORMAL;
		update_font();
	}

END_PROPERTY

BEGIN_PROPERTY(CairoFont_Slant)

	CHECK_CNT();

	if (READ_PROPERTY)
		GB.ReturnInteger(THIS->font_slant);
	else
	{
		THIS->font_slant = VPROP(GB_INTEGER);
		update_font();
	}

END_PROPERTY

BEGIN_PROPERTY(CairoFont_Size)

	CHECK_CNT();

	if (READ_PROPERTY)
		GB.ReturnFloat(THIS->font_size);
	else
	{
		THIS->font_size = VPROP(GB_FLOAT);
		cairo_set_font_size(CNT, THIS->font_size);
	}

END_PROPERTY

BEGIN_PROPERTY(CairoFont_Matrix)

	CAIRO_MATRIX *matrix;
	
	CHECK_CNT();

	if (READ_PROPERTY)
	{
		matrix = GB.New(GB.FindClass("CairoMatrix"), NULL, NULL);
		cairo_get_font_matrix(CNT, &matrix->matrix);
		GB.ReturnObject(matrix);
	}
	else
	{
		matrix = (CAIRO_MATRIX *)VPROP(GB_OBJECT);
		if (!matrix)
		{
			cairo_matrix_t identity;
			cairo_matrix_init_identity(&identity);
			cairo_set_font_matrix(CNT, &identity);
		}
		else
			cairo_set_font_matrix(CNT, &matrix->matrix);
	}

END_PROPERTY

BEGIN_PROPERTY(CairoFont_Extents)

	CAIRO_FONT_EXTENTS *extents;

	CHECK_CNT();
	
	extents = GB.New(GB.FindClass("CairoFontExtents"), NULL, NULL);
	
	cairo_font_extents(CNT, &extents->e);
	
	GB.ReturnObject(extents);

END_PROPERTY

GB_DESC CairoFontDesc[] = 
{
	GB_DECLARE(".Cairo.Font", 0), GB_VIRTUAL_CLASS(),
	
	GB_STATIC_PROPERTY("Name", "s", CairoFont_Name),
	GB_STATIC_PROPERTY("Bold", "b", CairoFont_Bold),
	GB_STATIC_PROPERTY("Italic", "b", CairoFont_Italic),
	GB_STATIC_PROPERTY("Weight", "i", CairoFont_Weight),
	GB_STATIC_PROPERTY("Slant", "i", CairoFont_Slant),
	GB_STATIC_PROPERTY("Size", "f", CairoFont_Size),
	GB_STATIC_PROPERTY("Matrix", "CairoMatrix", CairoFont_Matrix),
	GB_STATIC_PROPERTY("Extents", "CairoFontExtents", CairoFont_Extents),
	
	GB_END_DECLARE
};


/**** Cairo ****************************************************************/

BEGIN_METHOD(CAIRO_begin, GB_OBJECT device)

	void *device = VARG(device);
	CAIRO_DRAW *draw;

	if (GB.CheckObject(device))
		return;

	GB.GetClassInterface(GB.GetClass(device), "Drawable");
	
	GB.Alloc(POINTER(&draw), sizeof(CAIRO_DRAW));
	draw->previous = _current;
	
	if (GB.Is(device, GB.FindClass("Image")))
	{
		draw->surface = check_image(device);
		draw->context = cairo_create(draw->surface);
	}
	else if (GB.Is(device, GB.FindClass("CairoSurface")))
	{
		draw->surface = ((CAIRO_SURFACE *)device)->surface;
		draw->context = cairo_create(draw->surface);
	}
	else
	{
		GB.Free(POINTER(&draw));
		GB.Error("Bad device");
		return;
	}
	
	draw->device = device;
	GB.Ref(device);
	draw->pattern = NULL;
	draw->font_family = NULL;
	draw->font_slant = CAIRO_FONT_SLANT_NORMAL;
	draw->font_weight = CAIRO_FONT_WEIGHT_NORMAL;
	draw->font_size = 10.0;
	
	_current = draw;

END_METHOD

static void end_current()
{
	CAIRO_DRAW *draw = _current;

	if (!_current)
		return;
		
	_current = draw->previous;
	
	GB.FreeString(&draw->font_family);
	GB.Unref(POINTER(&draw->pattern));
	cairo_destroy(draw->context);
	GB.Unref(POINTER(&draw->device));
	GB.Free(POINTER(&draw));
}

BEGIN_METHOD_VOID(CAIRO_end)

	if (check_device())
		return;
		
	end_current();

END_METHOD

BEGIN_METHOD_VOID(CAIRO_exit)

	while	(_current)
		end_current();

END_METHOD

BEGIN_PROPERTY(CAIRO_status)

	CHECK_CNT();
	GB.ReturnInteger(cairo_status(CNT));

END_PROPERTY

BEGIN_PROPERTY(CAIRO_device)

	if (_current)
		GB.ReturnObject(_current->device);
	else
		GB.ReturnNull();

END_PROPERTY

#define IMPLEMENT_METHOD(_method, _api) \
BEGIN_METHOD_VOID(_method) \
	CHECK_CNT(); \
	_api(CNT); \
END_METHOD

#define IMPLEMENT_METHOD_PRESERVE(_method, _api) \
BEGIN_METHOD(_method, GB_BOOLEAN preserve) \
	CHECK_CNT(); \
	if (VARGOPT(preserve, FALSE)) \
		_api##_preserve(CNT); \
	else \
		_api(CNT); \
END_METHOD

#define IMPLEMENT_PROPERTY_EXTENTS(_property, _api) \
BEGIN_PROPERTY(_property) \
	CAIRO_EXTENTS *extents; \
	CHECK_CNT(); \
	extents = GB.New(GB.FindClass("CairoExtents"), NULL, NULL); \
	_api(CNT, &extents->x1, &extents->y1, &extents->x2, &extents->y2); \
	GB.ReturnObject(extents); \
END_METHOD

#define IMPLEMENT_METHOD_IN_X_Y(_method, _api) \
BEGIN_METHOD(_method, GB_FLOAT x; GB_FLOAT y) \
	CHECK_CNT(); \
	GB.ReturnBoolean(_api(CNT, VARG(x), VARG(y))); \
END_METHOD

#define IMPLEMENT_PROPERTY_INTEGER(_property, _get, _set) \
BEGIN_PROPERTY(_property) \
	CHECK_CNT(); \
	if (READ_PROPERTY) \
		GB.ReturnInteger(_get(CNT)); \
	else \
		_set(CNT, VPROP(GB_INTEGER)); \
END_METHOD

#define IMPLEMENT_PROPERTY_FLOAT(_property, _get, _set) \
BEGIN_PROPERTY(_property) \
	CHECK_CNT(); \
	if (READ_PROPERTY) \
		GB.ReturnFloat(_get(CNT)); \
	else \
		_set(CNT, VPROP(GB_FLOAT)); \
END_METHOD


IMPLEMENT_METHOD(CAIRO_save, cairo_save)
IMPLEMENT_METHOD(CAIRO_restore, cairo_restore)
IMPLEMENT_METHOD_PRESERVE(CAIRO_clip, cairo_clip)
IMPLEMENT_METHOD(CAIRO_reset_clip, cairo_reset_clip)
IMPLEMENT_PROPERTY_EXTENTS(CAIRO_clip_extents, cairo_clip_extents)
IMPLEMENT_METHOD_PRESERVE(CAIRO_fill, cairo_fill)
IMPLEMENT_PROPERTY_EXTENTS(CAIRO_fill_extents, cairo_fill_extents)
IMPLEMENT_METHOD_IN_X_Y(CAIRO_in_fill, cairo_in_fill)

BEGIN_METHOD(CAIRO_mask, GB_OBJECT pattern)

	CAIRO_PATTERN *pattern = (CAIRO_PATTERN *)VARG(pattern);

	CHECK_CNT();

	if (pattern)
		cairo_mask(CNT, pattern->pattern);

END_METHOD

BEGIN_METHOD(CAIRO_paint, GB_FLOAT alpha)

	CHECK_CNT();
	
	if (MISSING(alpha))
		cairo_paint(CNT);
	else
		cairo_paint_with_alpha(CNT, VARG(alpha));

END_METHOD

IMPLEMENT_METHOD_PRESERVE(CAIRO_stroke, cairo_stroke)
IMPLEMENT_PROPERTY_EXTENTS(CAIRO_stroke_extents, cairo_stroke_extents)
IMPLEMENT_METHOD_IN_X_Y(CAIRO_in_stroke, cairo_in_stroke)

BEGIN_PROPERTY(CAIRO_source)

	CHECK_CNT();

	if (READ_PROPERTY)
		GB.ReturnObject(_current->pattern);
	else
	{
		CAIRO_PATTERN *old_pattern = _current->pattern;
		CAIRO_PATTERN *new_pattern = (CAIRO_PATTERN *)VPROP(GB_OBJECT);
		if (new_pattern)
		{
			GB.Ref(new_pattern);
			cairo_set_source(CNT, new_pattern->pattern);
		}
		GB.Unref(POINTER(&old_pattern));
		_current->pattern = new_pattern;
	}

END_PROPERTY

BEGIN_PROPERTY(CAIRO_anti_alias)

	CHECK_CNT();

	if (READ_PROPERTY)
		GB.ReturnInteger(cairo_get_antialias(CNT));
	else
		cairo_set_antialias(CNT, VPROP(GB_INTEGER));

END_PROPERTY

BEGIN_PROPERTY(CAIRO_dash)

	GB_ARRAY array;
	
	CHECK_CNT();

	if (READ_PROPERTY)
	{
		int count = cairo_get_dash_count(CNT);
		GB.Array.New(POINTER(&array), GB_T_FLOAT, count);
		cairo_get_dash(CNT, (double *)GB.Array.Get(array, 0), NULL);
		GB.ReturnObject(array);
	}
	else
	{
		array = (GB_ARRAY)VPROP(GB_OBJECT);
		if (!array || GB.Array.Count(array) == 0)
			cairo_set_dash(CNT, NULL, 0, 0.0);
		else
			cairo_set_dash(CNT, (double *)GB.Array.Get(array, 0), GB.Array.Count(array), 0.0);
	}

END_PROPERTY

BEGIN_PROPERTY(CAIRO_dash_offset)

	CHECK_CNT();

	if (READ_PROPERTY)
	{
		double offset;
		cairo_get_dash(CNT, NULL, &offset);
		GB.ReturnFloat(offset);
	}
	else
	{
		int count = cairo_get_dash_count(CNT);
		double dashes[count];
		cairo_get_dash(CNT, dashes, NULL);
		cairo_set_dash(CNT, dashes, count, VPROP(GB_FLOAT));
	}

END_PROPERTY

IMPLEMENT_PROPERTY_INTEGER(CAIRO_fill_rule, cairo_get_fill_rule, cairo_set_fill_rule)
IMPLEMENT_PROPERTY_INTEGER(CAIRO_line_cap, cairo_get_line_cap, cairo_set_line_cap)
IMPLEMENT_PROPERTY_INTEGER(CAIRO_line_join, cairo_get_line_join, cairo_set_line_join)
IMPLEMENT_PROPERTY_FLOAT(CAIRO_line_width, cairo_get_line_width, cairo_set_line_width)
IMPLEMENT_PROPERTY_FLOAT(CAIRO_miter_limit, cairo_get_miter_limit, cairo_set_miter_limit)
IMPLEMENT_PROPERTY_INTEGER(CAIRO_operator, cairo_get_operator, cairo_set_operator)
IMPLEMENT_PROPERTY_FLOAT(CAIRO_tolerance, cairo_get_tolerance, cairo_set_tolerance)

IMPLEMENT_METHOD(CAIRO_new_path, cairo_new_path)
IMPLEMENT_METHOD(CAIRO_new_sub_path, cairo_new_sub_path)
IMPLEMENT_METHOD(CAIRO_close_path, cairo_close_path)

BEGIN_METHOD(CAIRO_arc, GB_FLOAT xc; GB_FLOAT yc; GB_FLOAT radius; GB_FLOAT angle1; GB_FLOAT angle2)

	CHECK_CNT();
	cairo_arc(CNT, VARG(xc), VARG(yc), VARG(radius), VARGOPT(angle1, 0.0), VARGOPT(angle2, M_PI * 2));

END_METHOD

BEGIN_METHOD(CAIRO_arc_negative, GB_FLOAT xc; GB_FLOAT yc; GB_FLOAT radius; GB_FLOAT angle1; GB_FLOAT angle2)

	CHECK_CNT();
	cairo_arc_negative(CNT, VARG(xc), VARG(yc), VARG(radius), VARGOPT(angle1, 0.0), VARGOPT(angle2, M_PI * 2));

END_METHOD

BEGIN_METHOD(CAIRO_curve_to, GB_FLOAT x1; GB_FLOAT y1; GB_FLOAT x2; GB_FLOAT y2; GB_FLOAT x3; GB_FLOAT y3)

	CHECK_CNT();
	cairo_curve_to(CNT, VARG(x1), VARG(y1), VARG(x2), VARG(y2), VARG(x3), VARG(y3));

END_METHOD

BEGIN_METHOD(CAIRO_line_to, GB_FLOAT x; GB_FLOAT y)

	CHECK_CNT();
	cairo_line_to(CNT, VARG(x), VARG(y));

END_METHOD

BEGIN_METHOD(CAIRO_move_to, GB_FLOAT x; GB_FLOAT y)

	CHECK_CNT();
	cairo_move_to(CNT, VARG(x), VARG(y));

END_METHOD

BEGIN_METHOD(CAIRO_rectangle, GB_FLOAT x; GB_FLOAT y; GB_FLOAT w; GB_FLOAT h)

	CHECK_CNT();
	cairo_rectangle(CNT, VARG(x), VARG(y), VARG(w), VARG(h));

END_METHOD

BEGIN_METHOD(CAIRO_rel_curve_to, GB_FLOAT x1; GB_FLOAT y1; GB_FLOAT x2; GB_FLOAT y2; GB_FLOAT x3; GB_FLOAT y3)

	CHECK_CNT();
	cairo_rel_curve_to(CNT, VARG(x1), VARG(y1), VARG(x2), VARG(y2), VARG(x3), VARG(y3));

END_METHOD

BEGIN_METHOD(CAIRO_rel_line_to, GB_FLOAT x; GB_FLOAT y)

	CHECK_CNT();
	cairo_rel_line_to(CNT, VARG(x), VARG(y));

END_METHOD

BEGIN_METHOD(CAIRO_rel_move_to, GB_FLOAT x; GB_FLOAT y)

	CHECK_CNT();
	cairo_rel_move_to(CNT, VARG(x), VARG(y));

END_METHOD

IMPLEMENT_PROPERTY_EXTENTS(CAIRO_path_extents, cairo_path_extents)

static void make_pattern(cairo_pattern_t *pattern, void *ref)
{
	CAIRO_PATTERN *pat;
	pat = GB.New(GB.FindClass("CairoPattern"), NULL, NULL);
	pat->pattern = pattern;
	if (ref)
	{
		pat->ref = ref;
		GB.Ref(ref);
	}
	GB.ReturnObject(pat);
}

BEGIN_METHOD(CAIRO_color_pattern, GB_INTEGER color)

	cairo_pattern_t *pattern;
	double r, g, b, a;
	uint rgba = VARG(color);
	
	a = ((rgba >> 24) ^ 0xFF) / 255.0;
	r = ((rgba >> 16) & 0xFF) / 255.0;
	g = ((rgba >> 8) & 0xFF) / 255.0;
	b = (rgba & 0xFF) / 255.0;
	
	pattern = cairo_pattern_create_rgba(r, g, b, a);
	make_pattern(pattern, NULL);

END_METHOD

BEGIN_METHOD(CAIRO_solid_pattern, GB_FLOAT r; GB_FLOAT g; GB_FLOAT b; GB_FLOAT a)

	cairo_pattern_t *pattern;
	
	if (MISSING(a))
		pattern = cairo_pattern_create_rgb(VARG(r), VARG(g), VARG(b));
	else
		pattern = cairo_pattern_create_rgba(VARG(r), VARG(g), VARG(b), VARG(a));
	
	make_pattern(pattern, NULL);

END_METHOD

BEGIN_METHOD(CAIRO_image_pattern, GB_OBJECT image; GB_FLOAT x; GB_FLOAT y; GB_INTEGER extend; GB_INTEGER filter)

	cairo_surface_t *surface;
	cairo_pattern_t *pattern;
	GB_IMG *image = (GB_IMG *)VARG(image);
	
	if (GB.CheckObject(image))
		return;
	
	surface = check_image(image);
	pattern = cairo_pattern_create_for_surface(surface);
	
	if (!MISSING(x) || !MISSING(y))
	{
    cairo_matrix_t matrix;
    cairo_matrix_init_translate (&matrix, -VARGOPT(x, 0.0), -VARGOPT(y, 0.0));
    cairo_pattern_set_matrix (pattern, &matrix);
	}
	
	if (!MISSING(extend))
		cairo_pattern_set_extend(pattern, VARG(extend));
	if (!MISSING(filter))
		cairo_pattern_set_filter(pattern, VARG(filter));

	make_pattern(pattern, image);
	
END_METHOD

static void handle_color_stop(cairo_pattern_t *pattern, GB_ARRAY colors)
{
	GB_ARRAY stop;
	int i, ncol;
	double *col;
	
	for (i = 0; i < GB.Array.Count(colors); i++)
	{
		stop = *((GB_ARRAY *)GB.Array.Get(colors, i));
		col = (double *)GB.Array.Get(stop, 0);
		ncol = GB.Array.Count(stop);
		if (ncol == 4)
			cairo_pattern_add_color_stop_rgb(pattern, col[0], col[1], col[2], col[3]);
		else if (ncol == 5)
			cairo_pattern_add_color_stop_rgba(pattern, col[0], col[1], col[2], col[3], col[4]);
	}
}

BEGIN_METHOD(CAIRO_linear_gradient_pattern, GB_FLOAT x0; GB_FLOAT y0; GB_FLOAT x1; GB_FLOAT y1; GB_OBJECT colors)

	cairo_pattern_t *pattern;
	GB_ARRAY colors;
	
	colors = (GB_ARRAY)VARG(colors);
	if (GB.CheckObject(colors))
		return;
		
	pattern = cairo_pattern_create_linear(VARG(x0), VARG(y0), VARG(x1), VARG(y1));
	handle_color_stop(pattern, colors);
	make_pattern(pattern, NULL);

END_METHOD

BEGIN_METHOD(CAIRO_radial_gradient_pattern, GB_FLOAT cx0; GB_FLOAT cy0; GB_FLOAT radius0; GB_FLOAT cx1; GB_FLOAT cy1; GB_FLOAT radius1; GB_OBJECT colors)

	cairo_pattern_t *pattern;
	GB_ARRAY colors;
	
	colors = (GB_ARRAY)VARG(colors);
	if (GB.CheckObject(colors))
		return;
		
	pattern = cairo_pattern_create_radial(VARG(cx0), VARG(cy0), VARG(radius0), VARG(cx1), VARG(cy1), VARG(radius1));
	handle_color_stop(pattern, colors);
	make_pattern(pattern, NULL);

END_METHOD

BEGIN_METHOD(CAIRO_translate, GB_FLOAT tx; GB_FLOAT ty)

	CHECK_CNT();
	cairo_translate(CNT, VARG(tx), VARG(ty));

END_METHOD

BEGIN_METHOD(CAIRO_scale, GB_FLOAT sx; GB_FLOAT sy)

	CHECK_CNT();
	cairo_scale(CNT, VARG(sx), VARG(sy));

END_METHOD

BEGIN_METHOD(CAIRO_rotate, GB_FLOAT angle)

	CHECK_CNT();
	cairo_rotate(CNT, VARG(angle));

END_METHOD

BEGIN_PROPERTY(CAIRO_matrix)

	CAIRO_MATRIX *matrix;
	
	CHECK_CNT();

	if (READ_PROPERTY)
	{
		matrix = GB.New(GB.FindClass("CairoMatrix"), NULL, NULL);
		cairo_get_matrix(CNT, &matrix->matrix);
		GB.ReturnObject(matrix);
	}
	else
	{
		matrix = (CAIRO_MATRIX *)VPROP(GB_OBJECT);
		if (!matrix)
			cairo_identity_matrix(CNT);
		else
			cairo_set_matrix(CNT, &matrix->matrix);
	}

END_PROPERTY

BEGIN_METHOD(Cairo_TextExtents, GB_STRING text)

	CAIRO_TEXT_EXTENTS *extents;

	CHECK_CNT();
	
	extents = GB.New(GB.FindClass("CairoTextExtents"), NULL, NULL);
	
	cairo_text_extents(CNT, GB.ToZeroString(ARG(text)), &extents->e);
	
	GB.ReturnObject(extents);

END_PROPERTY

BEGIN_METHOD(Cairo_Text, GB_STRING text)

	CHECK_CNT();
	
	cairo_text_path(CNT, GB.ToZeroString(ARG(text)));
	
END_METHOD

BEGIN_METHOD(Cairo_DrawText, GB_STRING text)

	CHECK_CNT();
	
	cairo_show_text(CNT, GB.ToZeroString(ARG(text)));
	
END_METHOD

BEGIN_METHOD_VOID(Cairo_ShowPage)

	CHECK_CNT();	
	cairo_show_page(CNT);
	
END_METHOD

BEGIN_METHOD_VOID(Cairo_CopyPage)

	CHECK_CNT();	
	cairo_copy_page(CNT);
	
END_METHOD


GB_DESC CairoDesc[] = 
{
	GB_DECLARE("Cairo", 0), GB_VIRTUAL_CLASS(),

	GB_CONSTANT("StatusSuccess", "i",              CAIRO_STATUS_SUCCESS),
	GB_CONSTANT("StatusNoMemory", "i",             CAIRO_STATUS_NO_MEMORY),
	GB_CONSTANT("StatusInvalidRestore", "i",       CAIRO_STATUS_INVALID_RESTORE),
	GB_CONSTANT("StatusInvalidPopGroup", "i",      CAIRO_STATUS_INVALID_POP_GROUP),
	GB_CONSTANT("StatusNoCurrentPoint", "i",       CAIRO_STATUS_NO_CURRENT_POINT),
	GB_CONSTANT("StatusInvalidMatrix", "i",        CAIRO_STATUS_INVALID_MATRIX),
	GB_CONSTANT("StatusInvalidStatus", "i",        CAIRO_STATUS_INVALID_STATUS),
	GB_CONSTANT("StatusNullPointer", "i",          CAIRO_STATUS_NULL_POINTER),
	GB_CONSTANT("StatusInvalidString", "i",        CAIRO_STATUS_INVALID_STRING),
	GB_CONSTANT("StatusInvalidPathData", "i",      CAIRO_STATUS_INVALID_PATH_DATA),
	GB_CONSTANT("StatusReadError", "i",            CAIRO_STATUS_READ_ERROR),
	GB_CONSTANT("StatusWriteError", "i",           CAIRO_STATUS_WRITE_ERROR),
	GB_CONSTANT("StatusSurfaceFinished", "i",      CAIRO_STATUS_SURFACE_FINISHED),
	GB_CONSTANT("StatusSurfaceTypeMismatch", "i",  CAIRO_STATUS_SURFACE_TYPE_MISMATCH),
	GB_CONSTANT("StatusPatternTypeMismatch", "i",  CAIRO_STATUS_PATTERN_TYPE_MISMATCH),
	GB_CONSTANT("StatusInvalidContent", "i",       CAIRO_STATUS_INVALID_CONTENT),
	GB_CONSTANT("StatusInvalidFormat", "i",        CAIRO_STATUS_INVALID_FORMAT),
	GB_CONSTANT("StatusInvalidVisual", "i",        CAIRO_STATUS_INVALID_VISUAL),
	GB_CONSTANT("StatusFileNotFound", "i",         CAIRO_STATUS_FILE_NOT_FOUND),
	GB_CONSTANT("StatusInvalidDash", "i",          CAIRO_STATUS_INVALID_DASH),
	GB_CONSTANT("StatusInvalidDscComment", "i",    CAIRO_STATUS_INVALID_DSC_COMMENT),
	GB_CONSTANT("StatusInvalidIndex", "i",         CAIRO_STATUS_INVALID_INDEX),
	GB_CONSTANT("StatusClipNotRepresentable", "i", CAIRO_STATUS_CLIP_NOT_REPRESENTABLE),
	GB_CONSTANT("StatusTempFileError", "i",        CAIRO_STATUS_TEMP_FILE_ERROR),
	GB_CONSTANT("StatusInvalidStride", "i",        CAIRO_STATUS_INVALID_STRIDE),
	#if CAIRO_VERSION_MAJOR > 1 || (CAIRO_VERSION_MAJOR == 1 && CAIRO_VERSION_MINOR >= 8)
	GB_CONSTANT("StatusFontTypeMismatch", "i",     CAIRO_STATUS_FONT_TYPE_MISMATCH),
	GB_CONSTANT("StatusUserFontImmutable", "i",    CAIRO_STATUS_USER_FONT_IMMUTABLE),
	GB_CONSTANT("StatusUserFontError", "i",        CAIRO_STATUS_USER_FONT_ERROR),
	GB_CONSTANT("StatusNegativeCount", "i",        CAIRO_STATUS_NEGATIVE_COUNT),
	GB_CONSTANT("StatusInvalidClusters", "i",      CAIRO_STATUS_INVALID_CLUSTERS),
	GB_CONSTANT("StatusInvalidSlant", "i",         CAIRO_STATUS_INVALID_SLANT),
	GB_CONSTANT("StatusInvalidWeight", "i",        CAIRO_STATUS_INVALID_WEIGHT),
	#else
	GB_CONSTANT("StatusFontTypeMismatch", "i",     CAIRO_STATUS_INVALID_STRIDE + 1),
	GB_CONSTANT("StatusUserFontImmutable", "i",    CAIRO_STATUS_INVALID_STRIDE + 2),
	GB_CONSTANT("StatusUserFontError", "i",        CAIRO_STATUS_INVALID_STRIDE + 3),
	GB_CONSTANT("StatusNegativeCount", "i",        CAIRO_STATUS_INVALID_STRIDE + 4),
	GB_CONSTANT("StatusInvalidClusters", "i",      CAIRO_STATUS_INVALID_STRIDE + 5),
	GB_CONSTANT("StatusInvalidSlant", "i",         CAIRO_STATUS_INVALID_STRIDE + 6),
	GB_CONSTANT("StatusInvalidWeight", "i",        CAIRO_STATUS_INVALID_STRIDE + 7),
	#endif
	
	GB_CONSTANT("AntiAliasDefault", "i",           CAIRO_ANTIALIAS_DEFAULT),
	GB_CONSTANT("AntiAliasNone", "i",              CAIRO_ANTIALIAS_NONE),
	GB_CONSTANT("AntiAliasGray", "i",              CAIRO_ANTIALIAS_GRAY),
	GB_CONSTANT("AntiAliasSubPixel", "i",          CAIRO_ANTIALIAS_SUBPIXEL),

	GB_CONSTANT("ExtendNone", "i",                 CAIRO_EXTEND_NONE),
	GB_CONSTANT("ExtendRepeat", "i",               CAIRO_EXTEND_REPEAT),
	GB_CONSTANT("ExtendReflect", "i",              CAIRO_EXTEND_REFLECT),
	GB_CONSTANT("ExtendPad", "i",                  CAIRO_EXTEND_PAD),

	GB_CONSTANT("FilterFast", "i",                 CAIRO_FILTER_FAST),
	GB_CONSTANT("FilterGood", "i",                 CAIRO_FILTER_GOOD),
	GB_CONSTANT("FilterBest", "i",                 CAIRO_FILTER_BEST),
	GB_CONSTANT("FilterNearest", "i",              CAIRO_FILTER_NEAREST),
	GB_CONSTANT("FilterBilinear", "i",             CAIRO_FILTER_BILINEAR),
	GB_CONSTANT("FilterGaussian", "i",             CAIRO_FILTER_GAUSSIAN),

	GB_CONSTANT("FillRuleWinding", "i",            CAIRO_FILL_RULE_WINDING),
	GB_CONSTANT("FillRuleEvenOdd", "i",            CAIRO_FILL_RULE_EVEN_ODD),

	GB_CONSTANT("LineCapButt", "i",                CAIRO_LINE_CAP_BUTT),
	GB_CONSTANT("LineCapRound", "i",               CAIRO_LINE_CAP_ROUND),
	GB_CONSTANT("LineCapSquare", "i",              CAIRO_LINE_CAP_SQUARE),
	
	GB_CONSTANT("LineJoinMiter", "i",              CAIRO_LINE_JOIN_MITER),
	GB_CONSTANT("LineJoinRound", "i",              CAIRO_LINE_JOIN_ROUND),
	GB_CONSTANT("LineJoinBevel", "i",              CAIRO_LINE_JOIN_BEVEL),
	
	GB_CONSTANT("OperatorClear", "i",              CAIRO_OPERATOR_CLEAR),
  GB_CONSTANT("OperatorSource", "i",             CAIRO_OPERATOR_SOURCE),
	GB_CONSTANT("OperatorOver", "i",               CAIRO_OPERATOR_OVER),
	GB_CONSTANT("OperatorIn", "i",                 CAIRO_OPERATOR_IN),
	GB_CONSTANT("OperatorOut", "i",                CAIRO_OPERATOR_OUT),
	GB_CONSTANT("OperatorATop", "i",               CAIRO_OPERATOR_ATOP),
	GB_CONSTANT("OperatorDest", "i",               CAIRO_OPERATOR_DEST),
	GB_CONSTANT("OperatorDestOver", "i",           CAIRO_OPERATOR_DEST_OVER),
	GB_CONSTANT("OperatorDestIn", "i",             CAIRO_OPERATOR_DEST_IN),
	GB_CONSTANT("OperatorDestOut", "i",            CAIRO_OPERATOR_DEST_OUT),
	GB_CONSTANT("OperatorDestATop", "i",           CAIRO_OPERATOR_DEST_ATOP),
	GB_CONSTANT("OperatorXor", "i",                CAIRO_OPERATOR_XOR),
	GB_CONSTANT("OperatorAdd", "i",                CAIRO_OPERATOR_ADD),
	GB_CONSTANT("OperatorSaturate", "i",           CAIRO_OPERATOR_SATURATE),
	
	GB_CONSTANT("FontSlantNormal", "i",            CAIRO_FONT_SLANT_NORMAL),
	GB_CONSTANT("FontSlantItalic", "i",            CAIRO_FONT_SLANT_ITALIC),
	GB_CONSTANT("FontSlantOblique", "i",           CAIRO_FONT_SLANT_OBLIQUE),
	GB_CONSTANT("FontWeightNormal", "i",           CAIRO_FONT_WEIGHT_NORMAL),
	GB_CONSTANT("FontWeightBold", "i",             CAIRO_FONT_WEIGHT_BOLD),
    
	GB_STATIC_METHOD("Begin", NULL, CAIRO_begin, "(Device)o"),
	GB_STATIC_METHOD("End", NULL, CAIRO_end, NULL),
	GB_STATIC_METHOD("_exit", NULL, CAIRO_exit, NULL),
	
	GB_STATIC_PROPERTY_READ("Status", "i", CAIRO_status),
	GB_STATIC_PROPERTY_READ("Device", "o", CAIRO_device),
	
	GB_STATIC_METHOD("Save", NULL, CAIRO_save, NULL),
	GB_STATIC_METHOD("Restore", NULL, CAIRO_restore, NULL),
	
	GB_STATIC_METHOD("Clip", NULL, CAIRO_clip, "[(Preserve)b]"),
	GB_STATIC_METHOD("ResetClip", NULL, CAIRO_reset_clip, NULL),
	GB_STATIC_PROPERTY_READ("ClipExtents", "CairoExtents", CAIRO_clip_extents),
	
	GB_STATIC_METHOD("Fill", NULL, CAIRO_fill, "[(Preserve)b]"),
	GB_STATIC_PROPERTY_READ("FillExtents", "CairoExtents", CAIRO_fill_extents),
	GB_STATIC_METHOD("InFill", "b", CAIRO_in_fill, "(X)f(Y)f"),
	
	GB_STATIC_METHOD("Mask", NULL, CAIRO_mask, "(Pattern)CairoPattern;"),
	
	GB_STATIC_METHOD("Paint", NULL, CAIRO_paint, "[(Alpha)f]"),

	GB_STATIC_METHOD("Stroke", NULL, CAIRO_stroke, "[(Preserve)b]"),
	GB_STATIC_PROPERTY_READ("StrokeExtents", "CairoExtents", CAIRO_stroke_extents),
	GB_STATIC_METHOD("InStroke", "b", CAIRO_in_stroke, "(X)f(Y)f"),

	GB_STATIC_PROPERTY("Source", "CairoPattern", CAIRO_source),
	GB_STATIC_PROPERTY("AntiAlias", "i", CAIRO_anti_alias),
	GB_STATIC_PROPERTY("Dash", "Float[]", CAIRO_dash),
	GB_STATIC_PROPERTY("DashOffset", "f", CAIRO_dash_offset),
	GB_STATIC_PROPERTY("FillRule", "i", CAIRO_fill_rule),
	GB_STATIC_PROPERTY("LineCap", "i", CAIRO_line_cap),
	GB_STATIC_PROPERTY("LineJoin", "i", CAIRO_line_join),
	GB_STATIC_PROPERTY("LineWidth", "f", CAIRO_line_width),
	GB_STATIC_PROPERTY("MiterLimit", "f", CAIRO_miter_limit),
	GB_STATIC_PROPERTY("Operator", "i", CAIRO_operator),
	GB_STATIC_PROPERTY("Tolerance", "f", CAIRO_tolerance),
	
	GB_STATIC_METHOD("NewPath", NULL, CAIRO_new_path, NULL),
	GB_STATIC_METHOD("NewSubPath", NULL, CAIRO_new_sub_path, NULL),
	GB_STATIC_METHOD("ClosePath", NULL, CAIRO_close_path, NULL),
	
	GB_STATIC_METHOD("Arc", NULL, CAIRO_arc, "(XC)f(YC)f(Radius)f[(Angle1)f(Angle2)f]"),
	GB_STATIC_METHOD("ArcNegative", NULL, CAIRO_arc_negative, "(XC)f(YC)f(Radius)f[(Angle1)f(Angle2)f]"),
	GB_STATIC_METHOD("CurveTo", NULL, CAIRO_curve_to, "(X1)f(Y1)f(X2)f(Y2)f(X3)f(Y3)f"),
	GB_STATIC_METHOD("LineTo", NULL, CAIRO_line_to, "(X)f(Y)f"),
	GB_STATIC_METHOD("MoveTo", NULL, CAIRO_move_to, "(X)f(Y)f"),
	GB_STATIC_METHOD("Rectangle", NULL, CAIRO_rectangle, "(X)f(Y)f(Width)f(Height)f"),

	GB_STATIC_METHOD("RelCurveTo", NULL, CAIRO_rel_curve_to, "(DX1)f(DY1)f(DX2)f(DY2)f(DX3)f(DY3)f"),
	GB_STATIC_METHOD("RelLineTo", NULL, CAIRO_rel_line_to, "(DX)f(DY)f"),
	GB_STATIC_METHOD("RelMoveTo", NULL, CAIRO_rel_move_to, "(DX)f(DY)f"),
	
	GB_STATIC_PROPERTY_READ("PathExtents", "CairoExtents", CAIRO_path_extents),
	
	GB_STATIC_METHOD("ColorPattern", "CairoPattern", CAIRO_color_pattern, "(Color)i"),
	GB_STATIC_METHOD("SolidPattern", "CairoPattern", CAIRO_solid_pattern, "(Red)f(Green)f(Blue)f[(Alpha)f]"),
	GB_STATIC_METHOD("ImagePattern", "CairoPattern", CAIRO_image_pattern, "(Image)Image;[(X)f(Y)f(Extend)i(Filter)i]"),
	GB_STATIC_METHOD("LinearGradient", "CairoPattern", CAIRO_linear_gradient_pattern, "(X0)f(Y0)f(X1)f(Y1)f(Colors)Float[][];"),
	GB_STATIC_METHOD("RadialGradient", "CairoPattern", CAIRO_radial_gradient_pattern, "(CX0)f(CY0)f(Radius0)f(CX1)f(CY1)f(Radius1)f(Colors)Float[][];"),

	GB_STATIC_METHOD("Translate", NULL, CAIRO_translate, "(TX)f(TY)f"),
	GB_STATIC_METHOD("Scale", NULL, CAIRO_scale, "(SX)f(SY)f"),
	GB_STATIC_METHOD("Rotate", NULL, CAIRO_rotate, "(Angle)f"),
	GB_STATIC_PROPERTY("Matrix", "CairoMatrix", CAIRO_matrix),
	
	GB_STATIC_PROPERTY_SELF("Font", ".Cairo.Font"),
	GB_STATIC_METHOD("TextExtents", "CairoTextExtents", Cairo_TextExtents, "(Text)s"),
	GB_STATIC_METHOD("Text", NULL, Cairo_Text, "(Text)s"),
	GB_STATIC_METHOD("DrawText", NULL, Cairo_DrawText, "(Text)s"),

	GB_STATIC_METHOD("ShowPage", NULL, Cairo_ShowPage, NULL),
	GB_STATIC_METHOD("CopyPage", NULL, Cairo_CopyPage, NULL),

	GB_END_DECLARE	
};
