/***************************************************************************

	c_cairo.c

	gb.cairo component

	(c) 2009 Benoît Minisini <gambas@users.sourceforge.net>

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 1, or (at your option)
	any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

***************************************************************************/

#define __C_CAIRO_C

#include "c_cairo.h"

typedef
	struct CAIRO_DRAW {
		struct CAIRO_DRAW *previous;
		void *device;
		cairo_surface_t *surface;
		cairo_t *context;
		CAIRO_PATTERN *pattern;
		}
	CAIRO_DRAW;

CAIRO_DRAW *_current = NULL;

#define CNT (_current->context)
#define SURFACE (_current->surface)

/***************************************************************************/

#define IMPLEMENT_EXTENTS_PROPERTY(_method, _field) \
BEGIN_PROPERTY(_method) \
	GB.ReturnFloat(THIS_EXTENTS->_field); \
END_PROPERTY

IMPLEMENT_EXTENTS_PROPERTY(CAIRO_EXTENTS_x1, x1)
IMPLEMENT_EXTENTS_PROPERTY(CAIRO_EXTENTS_y1, y1)
IMPLEMENT_EXTENTS_PROPERTY(CAIRO_EXTENTS_x2, x2)
IMPLEMENT_EXTENTS_PROPERTY(CAIRO_EXTENTS_y2, y2)

GB_DESC CairoExtentsDesc[] = 
{
	GB_DECLARE("CairoExtents", sizeof(CAIRO_EXTENTS)),
	
	GB_PROPERTY_READ("X1", "f", CAIRO_EXTENTS_x1),
	GB_PROPERTY_READ("Y1", "f", CAIRO_EXTENTS_y1),
	GB_PROPERTY_READ("X2", "f", CAIRO_EXTENTS_x2),
	GB_PROPERTY_READ("Y2", "f", CAIRO_EXTENTS_y2),
	
	GB_END_DECLARE
};

/***************************************************************************/

BEGIN_METHOD_VOID(CAIRO_PATTERN_free)

	cairo_pattern_destroy(THIS_PATTERN->pattern);

END_METHOD


GB_DESC CairoPatternDesc[] = 
{
	GB_DECLARE("CairoPattern", sizeof(CAIRO_PATTERN)), GB_NOT_CREATABLE(),

	GB_METHOD("_free", NULL, CAIRO_PATTERN_free, NULL),

	GB_END_DECLARE	
};



/***************************************************************************/

static void free_image(GB_IMG *img, void *image)
{
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
	return image;
}

static GB_IMG_OWNER _image_owner = {
	"gb.cairo",
	free_image,
	free_image,
	temp_image
	};

static cairo_surface_t *check_image(void *img)
{
	// TODO: format is endian-dependent
	return (cairo_surface_t *)IMAGE.Check((GB_IMG *)img, &_image_owner, GB_IMAGE_BGRP);
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

BEGIN_METHOD(CAIRO_begin, GB_OBJECT device)

	void *device = VARG(device);
	CAIRO_DRAW *draw;

	if (GB.CheckObject(device))
		return;

	GB.Alloc(POINTER(&draw), sizeof(CAIRO_DRAW));
	draw->previous = _current;
	
	if (GB.Is(device, GB.FindClass("Image")))
	{
		draw->surface = check_image(device);
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
	
	_current = draw;

END_METHOD


BEGIN_METHOD_VOID(CAIRO_end)

	CAIRO_DRAW *draw = _current;

	if (check_device())
		return;

	_current = draw->previous;
	
	cairo_destroy(draw->context);
	GB.Free(POINTER(&draw));

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
	GB.New(POINTER(&extents), GB.FindClass("CairoExtents"), NULL, NULL); \
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

static void make_pattern(cairo_pattern_t *pattern)
{
	CAIRO_PATTERN *pat;
	GB.New(POINTER(&pat), GB.FindClass("CairoPattern"), NULL, NULL);
	pat->pattern = pattern;
	GB.ReturnObject(pat);
}

BEGIN_METHOD(CAIRO_solid_pattern, GB_FLOAT r; GB_FLOAT g; GB_FLOAT b; GB_FLOAT a)

	cairo_pattern_t *pattern;
	
	if (MISSING(a))
		pattern = cairo_pattern_create_rgb(VARG(r), VARG(g), VARG(b));
	else
		pattern = cairo_pattern_create_rgba(VARG(r), VARG(g), VARG(b), VARG(a));
	
	make_pattern(pattern);

END_METHOD

BEGIN_METHOD(CAIRO_image_pattern, GB_OBJECT image; GB_FLOAT x; GB_FLOAT y; GB_INTEGER extend; GB_INTEGER filter)

	cairo_surface_t *surface;
	cairo_pattern_t *pattern;
	
	if (GB.CheckObject(VARG(image)))
		return;
	
	surface = check_image((GB_IMG *)VARG(image));
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

	make_pattern(pattern);
	
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
	make_pattern(pattern);

END_METHOD

BEGIN_METHOD(CAIRO_radial_gradient_pattern, GB_FLOAT cx0; GB_FLOAT cy0; GB_FLOAT radius0; GB_FLOAT cx1; GB_FLOAT cy1; GB_FLOAT radius1; GB_OBJECT colors)

	cairo_pattern_t *pattern;
	GB_ARRAY colors;
	
	colors = (GB_ARRAY)VARG(colors);
	if (GB.CheckObject(colors))
		return;
		
	pattern = cairo_pattern_create_radial(VARG(cx0), VARG(cy0), VARG(radius0), VARG(cx1), VARG(cy1), VARG(radius1));
	handle_color_stop(pattern, colors);
	make_pattern(pattern);

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
	GB_CONSTANT("StatusFontTypeMismatch", "i",     CAIRO_STATUS_FONT_TYPE_MISMATCH),
	GB_CONSTANT("StatusUserFontImmutable", "i",    CAIRO_STATUS_USER_FONT_IMMUTABLE),
	GB_CONSTANT("StatusUserFontError", "i",        CAIRO_STATUS_USER_FONT_ERROR),
	GB_CONSTANT("StatusNegativeCount", "i",        CAIRO_STATUS_NEGATIVE_COUNT),
	GB_CONSTANT("StatusInvalidClusters", "i",      CAIRO_STATUS_INVALID_CLUSTERS),
	GB_CONSTANT("StatusInvalidSlant", "i",         CAIRO_STATUS_INVALID_SLANT),
	GB_CONSTANT("StatusInvalidWeight", "i",        CAIRO_STATUS_INVALID_WEIGHT),
	
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
    
	GB_STATIC_METHOD("Begin", NULL, CAIRO_begin, "(Device)o"),
	GB_STATIC_METHOD("End", NULL, CAIRO_end, NULL),
	
	GB_STATIC_PROPERTY_READ("Status", "s", CAIRO_status),
	GB_STATIC_PROPERTY_READ("Device", "o", CAIRO_device),
	
	GB_STATIC_METHOD("Save", NULL, CAIRO_save, NULL),
	GB_STATIC_METHOD("Restore", NULL, CAIRO_save, NULL),
	
	GB_STATIC_METHOD("Clip", NULL, CAIRO_clip, "[(Preserve)b]"),
	GB_STATIC_METHOD("ResetClip", NULL, CAIRO_reset_clip, NULL),
	GB_STATIC_PROPERTY_READ("ClipExtents", "Float[]", CAIRO_clip_extents),
	
	GB_STATIC_METHOD("Fill", NULL, CAIRO_fill, "[(Preserve)b]"),
	GB_STATIC_PROPERTY_READ("FillExtents", "Float[]", CAIRO_fill_extents),
	GB_STATIC_METHOD("InFill", "b", CAIRO_in_fill, "(X)f(Y)f"),
	
	GB_STATIC_METHOD("Mask", NULL, CAIRO_mask, "(Pattern)CairoPattern;"),
	
	GB_STATIC_METHOD("Paint", NULL, CAIRO_paint, "[(Alpha)f]"),

	GB_STATIC_METHOD("Stroke", NULL, CAIRO_stroke, "[(Preserve)b]"),
	GB_STATIC_PROPERTY_READ("StrokeExtents", "Float[]", CAIRO_stroke_extents),
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
	
	GB_STATIC_PROPERTY_READ("PathExtents", "Float[]", CAIRO_path_extents),
	
	GB_STATIC_METHOD("SolidPattern", "CairoPattern", CAIRO_solid_pattern, "(Red)f(Green)f(Blue)f[(Alpha)f]"),
	GB_STATIC_METHOD("ImagePattern", "CairoPattern", CAIRO_image_pattern, "(Image)Image;[(X)f(Y)f(Extend)i(Filter)i]"),
	GB_STATIC_METHOD("LinearGradient", "CairoPattern", CAIRO_linear_gradient_pattern, "(X0)f(Y0)f(X1)f(Y1)f(Colors)Float[][];"),
	GB_STATIC_METHOD("RadialGradient", "CairoPattern", CAIRO_radial_gradient_pattern, "(CX0)f(CY0)f(Radius0)f(CX1)f(CY1)f(Radius1)f(Colors)Float[][];"),

	//GB_METHOD("SetSourceRGB", NULL, CAIRO_set_source_rgb, "(Red)f(Green)f(Blue)f[(Alpha)f]"),

	GB_END_DECLARE	
};
