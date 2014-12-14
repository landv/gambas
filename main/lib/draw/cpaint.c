/***************************************************************************

  cpaint.c

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

#define __CPAINT_C

#include "gb.image.h"
#include "main.h"
#include "cpaint.h"

static GB_PAINT *_current = NULL;
#define THIS _current
#define PAINT _current->desc

#define XTHIS ((PAINT_EXTENTS *)_object)

#define MTHIS ((PAINT_MATRIX *)_object)
#define MPAINT (_matrix_desc)

#define BTHIS ((PAINT_BRUSH *)_object)
#define BPAINT (BTHIS->desc)

#define CHECK_DEVICE() if (check_device()) return
#define CHECK_PATH() if (check_path()) return

#define RAD(_deg) ((_deg) * M_PI / 180)

static GB_PAINT_MATRIX_DESC *_matrix_desc = NULL;

static void load_matrix_interface()
{
	if (!_matrix_desc)
	{
		_matrix_desc = (GB_PAINT_MATRIX_DESC *)GB.GetClassInterface(GB.FindClass("Image"), "PaintMatrix");
		if (!_matrix_desc)
		{
			fprintf(stderr, "gb.draw: error: unable to find PaintMatrix interface\n");
			abort();
		}
	}
}

static bool check_device()
{
	if (!_current || !_current->extra)
	{
		GB.Error("No current device");
		return TRUE;
	}
	else
		return FALSE;
}

static bool check_path()
{
	if (THIS->has_path)
	{
		GB.Error("Pending path");
		return TRUE;
	}
	else
		return FALSE;
}

GB_PAINT *PAINT_get_current()
{
	if (check_device())
		return NULL;
	else
		return _current;
}

GB_PAINT *PAINT_from_device(void *device)
{
	GB_PAINT *d;

	for (d = _current; d; d = d->previous)
	{
		if (d->device == device && d->opened)
			break;
	}

	return d;
}

bool PAINT_is_painted(void *device)
{
	return PAINT_from_device(device) != NULL;
}


bool PAINT_open(GB_PAINT *paint)
{
	if (paint->opened)
		return FALSE;
	
	//fprintf(stderr, "PAINT_open: %p\n", paint);
	GB.Alloc(POINTER(&paint->extra), paint->desc->size);
	memset(paint->extra, 0, paint->desc->size);
	paint->opened = !paint->desc->Begin(paint);

	if (!paint->opened)
		GB.Free(POINTER(&paint->extra));
	
	return !paint->opened;
}

void PAINT_close(GB_PAINT *paint)
{
	if (paint->opened && !paint->other)
	{
		//fprintf(stderr, "PAINT_close: %p\n", paint);
		paint->desc->End(paint);
		GB.Free(POINTER(&paint->extra));
		paint->opened = FALSE;
	}
}

bool PAINT_begin(void *device)
{
	GB_PAINT_DESC *desc;
	GB_PAINT *paint, *other;
	GB_CLASS klass;
	
	klass = GB.GetClass(device);
	desc = (GB_PAINT_DESC *)GB.GetClassInterface(klass, "Paint");
	load_matrix_interface();

	if (!desc)
	{
		GB.Error("Not a paintable object");
		return TRUE;
	}

	GB.Alloc(POINTER(&paint), sizeof(GB_PAINT));
	
	other = PAINT_from_device(device);
	
	paint->desc = desc;
	GB.Ref(device);
	paint->device = device;
	paint->brush = NULL;
	paint->opened = FALSE;
	paint->other = FALSE;
	paint->has_path = FALSE;
	paint->tag = NULL;
	
	paint->previous = _current;
	_current = paint;
	
	//paint->draw = DRAW_from_device(device);
	//if (paint->draw)
	//	DRAW_close(paint->draw);
	
	if (other)
	{
		paint->extra = other->extra;
		paint->opened = TRUE;
		paint->other = TRUE;
		paint->width = other->width;
		paint->height = other->height;
		paint->resolutionX = other->resolutionX;
		paint->resolutionY = other->resolutionY;
		paint->brush = other->brush;
		if (paint->brush)
			GB.Ref(paint->brush);
	}
	else
	{
		if (PAINT_open(paint))
			return TRUE;
	}
	
	return FALSE;
}

void PAINT_end()
{
	GB_PAINT *paint;

	if (!_current)
		return;
		
	paint = _current;
	_current = _current->previous;
	
	PAINT_close(paint);

	//if (paint->draw)
	//	DRAW_open(paint->draw);
	
	if (paint->brush)
		GB.Unref(POINTER(&paint->brush));
	GB.Unref(POINTER(&paint->device));
	GB.StoreObject(NULL, POINTER(&(paint->tag)));
	GB.Free(POINTER(&paint));
}


//---- PaintExtents ---------------------------------------------------------

#define IMPLEMENT_EXTENTS_PROPERTY(_method, _field) \
BEGIN_PROPERTY(_method) \
	GB.ReturnFloat(XTHIS->ext._field); \
END_PROPERTY

IMPLEMENT_EXTENTS_PROPERTY(PaintExtents_X, x1)
IMPLEMENT_EXTENTS_PROPERTY(PaintExtents_Y, y1)
IMPLEMENT_EXTENTS_PROPERTY(PaintExtents_X2, x2)
IMPLEMENT_EXTENTS_PROPERTY(PaintExtents_Y2, y2)

BEGIN_PROPERTY(PaintExtents_Width)

	GB.ReturnFloat(XTHIS->ext.x2 - XTHIS->ext.x1);

END_PROPERTY

BEGIN_PROPERTY(PaintExtents_Height)

	GB.ReturnFloat(XTHIS->ext.y2 - XTHIS->ext.y1);

END_PROPERTY

BEGIN_METHOD(PaintExtents_Merge, GB_OBJECT extents)

	PAINT_EXTENTS *extents = VARG(extents);
	
	if (GB.CheckObject(extents))
		return;
		
	if (extents->ext.x1 < XTHIS->ext.x1) XTHIS->ext.x1 = extents->ext.x1;
	if (extents->ext.y1 < XTHIS->ext.y1) XTHIS->ext.y1 = extents->ext.y1;
	if (extents->ext.x2 > XTHIS->ext.x2) XTHIS->ext.x2 = extents->ext.x2;
	if (extents->ext.y2 > XTHIS->ext.y2) XTHIS->ext.y2 = extents->ext.y2;

END_METHOD

GB_DESC PaintExtentsDesc[] = 
{
	GB_DECLARE("PaintExtents", sizeof(PAINT_EXTENTS)),
	
	GB_PROPERTY_READ("X", "f", PaintExtents_X),
	GB_PROPERTY_READ("Y", "f", PaintExtents_Y),
	GB_PROPERTY_READ("X2", "f", PaintExtents_X2),
	GB_PROPERTY_READ("Y2", "f", PaintExtents_Y2),
	GB_PROPERTY_READ("Width", "f", PaintExtents_Width),
	GB_PROPERTY_READ("Height", "f", PaintExtents_Height),
	
	GB_METHOD("Merge", "PaintExtents", PaintExtents_Merge, "(Extents)PaintExtents;"),
	
	GB_END_DECLARE
};


//---- PaintMatrix ----------------------------------------------------------

static bool _do_not_init = FALSE;

static PAINT_MATRIX *create_matrix(GB_TRANSFORM transform)
{
	PAINT_MATRIX *matrix;
	_do_not_init = TRUE;
	matrix = GB.New(GB.FindClass("PaintMatrix"), NULL, NULL);
	_do_not_init = FALSE;
	matrix->transform = transform;
	return matrix;
}

BEGIN_METHOD(PaintMatrix_new, GB_FLOAT xx; GB_FLOAT xy; GB_FLOAT yx; GB_FLOAT yy; GB_FLOAT x0; GB_FLOAT y0)

	load_matrix_interface();

	if (_do_not_init)
		return;
	
	MPAINT->Create(&MTHIS->transform);
	MPAINT->Init(MTHIS->transform, VARGOPT(xx, 1.0), VARGOPT(xy, 0.0), VARGOPT(yx, 0.0), VARGOPT(yy, 1.0), VARGOPT(x0, 0.0), VARGOPT(y0, 0.0));

END_METHOD

BEGIN_METHOD(PaintMatrix_call, GB_FLOAT xx; GB_FLOAT xy; GB_FLOAT yx; GB_FLOAT yy; GB_FLOAT x0; GB_FLOAT y0)

	GB_TRANSFORM transform;
	
	MPAINT->Create(&transform);
	MPAINT->Init(transform, VARGOPT(xx, 1.0), VARGOPT(xy, 0.0), VARGOPT(yx, 0.0), VARGOPT(yy, 1.0), VARGOPT(x0, 0.0), VARGOPT(y0, 0.0));
	GB.ReturnObject(create_matrix(transform));

END_METHOD

BEGIN_METHOD_VOID(PaintMatrix_free)

	if (MTHIS->transform)
		MPAINT->Delete(&MTHIS->transform);

END_METHOD

BEGIN_METHOD_VOID(PaintMatrix_Copy)

	GB_TRANSFORM transform;

	MPAINT->Copy(&transform, MTHIS->transform);
	GB.ReturnObject(create_matrix(transform));

END_METHOD

BEGIN_METHOD_VOID(PaintMatrix_Reset)

	MPAINT->Init(MTHIS->transform, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0);
	RETURN_SELF();

END_METHOD

BEGIN_METHOD(PaintMatrix_Translate, GB_FLOAT tx; GB_FLOAT ty)

	MPAINT->Translate(MTHIS->transform, (float)VARG(tx), (float)VARG(ty));
	RETURN_SELF();

END_METHOD

BEGIN_METHOD(PaintMatrix_Scale, GB_FLOAT sx; GB_FLOAT sy)

	MPAINT->Scale(MTHIS->transform, (float)VARG(sx), (float)VARG(sy));
	RETURN_SELF();

END_METHOD

BEGIN_METHOD(PaintMatrix_Rotate, GB_FLOAT angle)

	MPAINT->Rotate(MTHIS->transform, (float)VARG(angle));
	RETURN_SELF();

END_METHOD

BEGIN_METHOD_VOID(PaintMatrix_Invert)

	if (MPAINT->Invert(MTHIS->transform))
		GB.ReturnNull();
	else
		RETURN_SELF();

END_METHOD

BEGIN_METHOD(PaintMatrix_Multiply, GB_OBJECT matrix)

	PAINT_MATRIX *matrix = (PAINT_MATRIX *)VARG(matrix);
	
	if (GB.CheckObject(matrix))
		return;

	MPAINT->Multiply(MTHIS->transform, matrix->transform);
	RETURN_SELF();

END_METHOD

BEGIN_METHOD(PaintMatrix_Map, GB_OBJECT point)

	GEOM_POINTF *point = VARG(point);
	double x, y;

	if (GB.CheckObject(point))
		return;

	x = point->x;
	y = point->y;

	MPAINT->Map(MTHIS->transform, &x, &y);

	GB.ReturnObject(GEOM.CreatePointF(x, y));

END_METHOD

GB_DESC PaintMatrixDesc[] = 
{
	GB_DECLARE("PaintMatrix", sizeof(PAINT_MATRIX)),
	
	GB_METHOD("_new", NULL, PaintMatrix_new, "[(XX)f(XY)f(YX)f(YY)f(X0)f(Y0)f]"),
	GB_STATIC_METHOD("_call", "PaintMatrix", PaintMatrix_call, "[(XX)f(XY)f(YX)f(YY)f(X0)f(Y0)f]"),
	GB_METHOD("_free", NULL, PaintMatrix_free, NULL),

	GB_METHOD("Copy", "PaintMatrix", PaintMatrix_Copy, NULL),
	GB_METHOD("Reset", "PaintMatrix", PaintMatrix_Reset, NULL),
	GB_METHOD("Translate", "PaintMatrix", PaintMatrix_Translate, "(TX)f(TY)f"),
	GB_METHOD("Scale", "PaintMatrix", PaintMatrix_Scale, "(SX)f(SY)f"),
	GB_METHOD("Rotate", "PaintMatrix", PaintMatrix_Rotate, "(Angle)f"),
	GB_METHOD("Invert", "PaintMatrix", PaintMatrix_Invert, NULL),
	GB_METHOD("Multiply", "PaintMatrix", PaintMatrix_Multiply, "(Matrix)PaintMatrix;"),	
	GB_METHOD("Map", "PointF", PaintMatrix_Map, "(Point)PointF;"),

	GB_END_DECLARE	
};


//---- PaintBrush -----------------------------------------------------------

BEGIN_METHOD_VOID(PaintBrush_free)

	BPAINT->Brush.Free(BTHIS->brush);

END_METHOD

BEGIN_PROPERTY(PaintBrush_Matrix)

	GB_TRANSFORM transform;
	PAINT_MATRIX *matrix;
	
	if (READ_PROPERTY)
	{
		MPAINT->Create(&transform);
		BPAINT->Brush.Matrix(BTHIS->brush, FALSE, transform);
		GB.ReturnObject(create_matrix(transform));
	}
	else
	{
		matrix = (PAINT_MATRIX *)VPROP(GB_OBJECT);
		if (!matrix)
			BPAINT->Brush.Matrix(BTHIS->brush, TRUE, NULL);
		else
			BPAINT->Brush.Matrix(BTHIS->brush, TRUE, matrix->transform);
	}

END_PROPERTY

BEGIN_METHOD_VOID(PaintBrush_Reset)

	BPAINT->Brush.Matrix(BTHIS->brush, TRUE, NULL);

END_METHOD

BEGIN_METHOD(PaintBrush_Translate, GB_FLOAT tx; GB_FLOAT ty)

	GB_TRANSFORM transform;

	MPAINT->Create(&transform);
	BPAINT->Brush.Matrix(BTHIS->brush, FALSE, transform);
	MPAINT->Translate(transform, (float)VARG(tx), (float)VARG(ty));
	BPAINT->Brush.Matrix(BTHIS->brush, TRUE, transform);
	MPAINT->Delete(&transform);

END_METHOD

BEGIN_METHOD(PaintBrush_Scale, GB_FLOAT sx; GB_FLOAT sy)

	GB_TRANSFORM transform;

	MPAINT->Create(&transform);
	BPAINT->Brush.Matrix(BTHIS->brush, FALSE, transform);
	MPAINT->Scale(transform, (float)VARG(sx), (float)VARG(sy));
	BPAINT->Brush.Matrix(BTHIS->brush, TRUE, transform);
	MPAINT->Delete(&transform);

END_METHOD

BEGIN_METHOD(PaintBrush_Rotate, GB_FLOAT angle)

	GB_TRANSFORM transform;

	MPAINT->Create(&transform);
	BPAINT->Brush.Matrix(BTHIS->brush, FALSE, transform);
	MPAINT->Rotate(transform, (float)VARG(angle));
	BPAINT->Brush.Matrix(BTHIS->brush, TRUE, transform);
	MPAINT->Delete(&transform);

END_METHOD


GB_DESC PaintBrushDesc[] = 
{
	GB_DECLARE("PaintBrush", sizeof(PAINT_BRUSH)), GB_NOT_CREATABLE(),

	GB_METHOD("_free", NULL, PaintBrush_free, NULL),
	
	GB_PROPERTY("Matrix", "PaintMatrix", PaintBrush_Matrix),
	GB_METHOD("Reset", NULL, PaintBrush_Reset, NULL),
	GB_METHOD("Translate", NULL, PaintBrush_Translate, "(TX)f(TY)f"),
	GB_METHOD("Scale", NULL, PaintBrush_Scale, "(SX)f(SY)f"),
	GB_METHOD("Rotate", NULL, PaintBrush_Rotate, "(Angle)f"),

	GB_END_DECLARE
};


//---- Paint ----------------------------------------------------------------


BEGIN_METHOD(Paint_Begin, GB_OBJECT device)

	void *device = VARG(device);

	if (GB.CheckObject(device))
		return;

	PAINT_begin(device);

END_METHOD


BEGIN_METHOD_VOID(Paint_End)

	PAINT_end();
	
END_METHOD


BEGIN_METHOD_VOID(Paint_exit)

	while (_current)
		PAINT_end();

END_METHOD


BEGIN_PROPERTY(Paint_Device)
	
	if (THIS)
		GB.ReturnObject(THIS->device);
	else
		GB.ReturnNull();

END_PROPERTY


BEGIN_PROPERTY(Paint_Width)

	CHECK_DEVICE();
	GB.ReturnFloat(THIS->width);

END_PROPERTY


BEGIN_PROPERTY(Paint_Height)

	CHECK_DEVICE();
	GB.ReturnFloat(THIS->height);

END_PROPERTY


BEGIN_PROPERTY(Paint_ResolutionX)

	CHECK_DEVICE();
	GB.ReturnInteger(THIS->resolutionX);

END_PROPERTY


BEGIN_PROPERTY(Paint_ResolutionY)

	CHECK_DEVICE();
	GB.ReturnInteger(THIS->resolutionY);

END_PROPERTY

#define IMPLEMENT_METHOD(_method, _api) \
BEGIN_METHOD_VOID(_method) \
	CHECK_DEVICE(); \
	PAINT->_api(THIS); \
END_METHOD


#define IMPLEMENT_METHOD_PRESERVE(_method, _api) \
BEGIN_METHOD(_method, GB_BOOLEAN preserve) \
  bool preserve = VARGOPT(preserve, FALSE); \
	CHECK_DEVICE(); \
	PAINT->_api(THIS, preserve); \
	if (!preserve) \
		THIS->has_path = FALSE; \
END_METHOD

#define IMPLEMENT_PROPERTY_EXTENTS(_property, _api) \
BEGIN_PROPERTY(_property) \
	PAINT_EXTENTS *extents; \
	CHECK_DEVICE(); \
	extents = GB.New(GB.FindClass("PaintExtents"), NULL, NULL); \
	PAINT->_api(THIS, &extents->ext); \
	GB.ReturnObject(extents); \
END_METHOD

#define IMPLEMENT_PROPERTY(_property, _api, _type, _gtype, _return) \
BEGIN_PROPERTY(_property) \
	_type value; \
	CHECK_DEVICE(); \
	if (READ_PROPERTY) \
	{ \
		PAINT->_api(THIS, FALSE, &value); \
		_return(value); \
	} \
	else \
	{ \
		value = (_type)VPROP(_gtype); \
		PAINT->_api(THIS, TRUE, &value); \
	} \
END_METHOD

#define IMPLEMENT_PROPERTY_BOOLEAN(_property, _api) \
	IMPLEMENT_PROPERTY(_property, _api, int, GB_BOOLEAN, GB.ReturnBoolean)

#define IMPLEMENT_PROPERTY_INTEGER(_property, _api) \
	IMPLEMENT_PROPERTY(_property, _api, int, GB_INTEGER, GB.ReturnInteger)

#define IMPLEMENT_PROPERTY_UNSIGNED_INTEGER(_property, _api) \
	IMPLEMENT_PROPERTY(_property, _api, uint, GB_INTEGER, GB.ReturnInteger)

#define IMPLEMENT_PROPERTY_FLOAT(_property, _api) \
	IMPLEMENT_PROPERTY(_property, _api, float, GB_FLOAT, GB.ReturnFloat)

IMPLEMENT_PROPERTY_BOOLEAN(Paint_Invert, Invert)
IMPLEMENT_PROPERTY_INTEGER(Paint_FillStyle, FillStyle)

BEGIN_PROPERTY(Paint_Background)

	uint value;

	CHECK_DEVICE();

	if (READ_PROPERTY)
	{
		PAINT->Background(THIS, FALSE, &value);
		GB.ReturnInteger(value);
	}
	else
	{
		value = (uint)VPROP(GB_INTEGER);
		PAINT->Background(THIS, TRUE, &value);
		GB.StoreObject(NULL, POINTER(&THIS->brush));
	}

END_METHOD

IMPLEMENT_PROPERTY_BOOLEAN(Paint_Antialias, Antialias)
IMPLEMENT_METHOD(Paint_Save, Save)
IMPLEMENT_METHOD(Paint_Restore, Restore)
IMPLEMENT_METHOD_PRESERVE(Paint_Clip, Clip)
IMPLEMENT_METHOD(Paint_ResetClip, ResetClip)
IMPLEMENT_PROPERTY_EXTENTS(Paint_ClipExtents, ClipExtents)
IMPLEMENT_METHOD_PRESERVE(Paint_Fill, Fill)
IMPLEMENT_METHOD_PRESERVE(Paint_Stroke, Stroke)
IMPLEMENT_PROPERTY_EXTENTS(Paint_PathExtents, PathExtents)


static GB_ARRAY _outline;
static GB_ARRAY _outline_p;

static void make_path_outline(int cmd, float x, float y)
{
	if (!_outline)
		GB.Array.New(&_outline, GB.FindClass("PointF[]"), 0);

	if (cmd == GB_PAINT_PATH_MOVE)
	{
		GB.Array.New(&_outline_p, GB.FindClass("PointF"), 0);
		GB.Ref(_outline_p);
		*((void **)GB.Array.Add(_outline)) = _outline_p;
	}

	GEOM_POINTF *p = GEOM.CreatePointF(x, y);
	*((void **)GB.Array.Add(_outline_p)) = p;
	GB.Ref(p);
}

BEGIN_PROPERTY(Paint_PathOutline)

	CHECK_DEVICE();

	_outline = NULL;
	PAINT->PathOutline(THIS, make_path_outline);
	GB.ReturnObject(_outline);

END_PROPERTY


BEGIN_PROPERTY(Paint_ClipRect)

	GB_EXTENTS ext;
	GEOM_RECT *rect;
	int w, h;
	
	CHECK_DEVICE();
	
	if (READ_PROPERTY)
	{
		(*PAINT->ClipExtents)(THIS, &ext);
		
		w = floorf(ext.x2) - ceilf(ext.x1);
		h = floorf(ext.y2) - ceilf(ext.y1);
		
		if (w <= 0 || h <= 0)
		{
			GB.ReturnNull();
			return;
		}
		
		rect = GEOM.CreateRect();
		rect->x = ceilf(ext.x1);
		rect->y = ceilf(ext.y1);
		rect->w = w;
		rect->h = h;
		
		GB.ReturnObject(rect);
	}
	else
	{
		rect = (GEOM_RECT *)VPROP(GB_OBJECT);
		if (rect)
			PAINT->ClipRect(THIS, rect->x, rect->y, rect->w, rect->h);
		else
			PAINT->ResetClip(THIS);
	}

END_PROPERTY


BEGIN_METHOD(Paint_PathContains, GB_FLOAT x; GB_FLOAT y)

	CHECK_DEVICE();
	GB.ReturnBoolean(PAINT->PathContains(THIS, (float)VARG(x), (float)VARG(y)));

END_METHOD

IMPLEMENT_PROPERTY_INTEGER(Paint_FillRule, FillRule)
IMPLEMENT_PROPERTY_INTEGER(Paint_LineCap, LineCap)
IMPLEMENT_PROPERTY_INTEGER(Paint_LineJoin, LineJoin)
IMPLEMENT_PROPERTY_INTEGER(Paint_Operator, Operator)
IMPLEMENT_PROPERTY_FLOAT(Paint_LineWidth, LineWidth)
IMPLEMENT_PROPERTY_FLOAT(Paint_MiterLimit, MiterLimit)

BEGIN_PROPERTY(Paint_Brush)

	CHECK_DEVICE();
	
	if (READ_PROPERTY)
		GB.ReturnObject(THIS->brush);
	else
	{
		PAINT_BRUSH *old_brush = THIS->brush;
		PAINT_BRUSH *new_brush = (PAINT_BRUSH *)VPROP(GB_OBJECT);
		if (new_brush)
		{
			GB.Ref(new_brush);
			PAINT->SetBrush(THIS, new_brush->brush);
		}
		GB.Unref(POINTER(&old_brush));
		THIS->brush = new_brush;
	}

END_PROPERTY


BEGIN_PROPERTY(Paint_BrushOrigin)

	float x, y;
	
	CHECK_DEVICE();

	if (READ_PROPERTY)
	{
		PAINT->BrushOrigin(THIS, FALSE, &x, &y);
		GB.ReturnObject(GEOM.CreatePointF(x, y));
	}
	else
	{
		GEOM_POINT *p = VPROP(GB_OBJECT);
		if (!p)
			x = y = 0.0;
		else
		{
			x = p->x;
			y = p->y;
		}
			
		PAINT->BrushOrigin(THIS, TRUE, &x, &y);
	}

END_PROPERTY


BEGIN_PROPERTY(Paint_Dash)

	GB_ARRAY array;
	float *dashes;
	int count, i;
	
	CHECK_DEVICE();
	
	if (READ_PROPERTY)
	{
		PAINT->Dash(THIS, FALSE, &dashes, &count);
		if (!count)
			GB.ReturnNull();
		else
		{
			GB.Array.New(POINTER(&array), GB_T_FLOAT, count);
			for (i = 0; i < count; i++)
				*((double *)GB.Array.Get(array, i)) = (double)dashes[i];
			GB.ReturnObject(array);
		}
		GB.Free(POINTER(&dashes));
	}
	else
	{
		array = (GB_ARRAY)VPROP(GB_OBJECT);
		if (!array)
			count = 0;
		else
			count = GB.Array.Count(array);
		
		if (!count)
		{
			PAINT->Dash(THIS, TRUE, NULL, &count);
			//THIS->lineStyle = GB_PAINT_LINE_STYLE_SOLID;
		}
		else
		{
			GB.Alloc(POINTER(&dashes), sizeof(float) * count);
			for (i = 0; i < count; i++)
				dashes[i] = (float)*((double *)GB.Array.Get(array, i));
			PAINT->Dash(THIS, TRUE, &dashes, &count);
			GB.Free(POINTER(&dashes));
			//THIS->lineStyle = GB_PAINT_LINE_STYLE_CUSTOM;
		}
	}

END_PROPERTY


IMPLEMENT_PROPERTY_FLOAT(Paint_DashOffset, DashOffset)

BEGIN_METHOD_VOID(Paint_NewPath)

	CHECK_DEVICE();

	PAINT->NewPath(THIS);
	THIS->has_path = FALSE;

END_METHOD

IMPLEMENT_METHOD(Paint_ClosePath, ClosePath)


BEGIN_PROPERTY(Paint_X)

	float x, y;
	CHECK_DEVICE();
	PAINT->GetCurrentPoint(THIS, &x, &y);
	GB.ReturnFloat((double)x);

END_PROPERTY


BEGIN_PROPERTY(Paint_Y)

	float x, y;
	CHECK_DEVICE();
	PAINT->GetCurrentPoint(THIS, &x, &y);
	GB.ReturnFloat((double)y);

END_PROPERTY


BEGIN_METHOD(Paint_Arc, GB_FLOAT xc; GB_FLOAT yc; GB_FLOAT radius; GB_FLOAT angle; GB_FLOAT length; GB_BOOLEAN pie)

	CHECK_DEVICE();
	
	bool pie = VARGOPT(pie, FALSE);
	float angle = VARGOPT(angle, 0.0);
	float length = VARGOPT(length, MISSING(angle) ? M_PI * 2 : 0.0);
	
	if (MISSING(length) || length == 0.0)
		pie = FALSE;
	
	PAINT->Arc(THIS, VARG(xc), VARG(yc), VARG(radius), angle, length, pie);
	THIS->has_path = TRUE;

END_METHOD


BEGIN_METHOD(Paint_Ellipse, GB_FLOAT x; GB_FLOAT y; GB_FLOAT width; GB_FLOAT height; GB_FLOAT angle; GB_FLOAT length; GB_BOOLEAN pie)

	CHECK_DEVICE();

	bool pie = VARGOPT(pie, FALSE);
	float angle = VARGOPT(angle, 0.0);
	float length = VARGOPT(length, MISSING(angle) ? M_PI * 2 : 0.0);
	
	if (MISSING(length) || length == 0.0)
		pie = FALSE;
	
	PAINT->Ellipse(THIS, VARG(x), VARG(y), VARG(width), VARG(height), angle, length, pie);
	THIS->has_path = TRUE;

END_METHOD


BEGIN_METHOD(Paint_Polygon, GB_OBJECT points)

	GB_ARRAY points = VARG(points);
	int i, n;
	double *p;
	
	CHECK_DEVICE();
	
	if (!points)
		return;
	
	n = GB.Array.Count(points);
	if (n < 4) 
		return;

	CHECK_DEVICE();
	
	p = (double *)GB.Array.Get(points, 0);
	
	PAINT->MoveTo(THIS, p[0], p[1]);
	for (i = 2; i < n; i+= 2)
		PAINT->LineTo(THIS, p[i], p[i + 1]);
	PAINT->LineTo(THIS, p[0], p[1]);
	THIS->has_path = TRUE;

END_METHOD


BEGIN_METHOD(Paint_CurveTo, GB_FLOAT x1; GB_FLOAT y1; GB_FLOAT x2; GB_FLOAT y2; GB_FLOAT x3; GB_FLOAT y3)

	CHECK_DEVICE();
	PAINT->CurveTo(THIS, VARG(x1), VARG(y1), VARG(x2), VARG(y2), VARG(x3), VARG(y3));
	THIS->has_path = TRUE;

END_METHOD


BEGIN_METHOD(Paint_RelCurveTo, GB_FLOAT x1; GB_FLOAT y1; GB_FLOAT x2; GB_FLOAT y2; GB_FLOAT x3; GB_FLOAT y3)

	float x, y;
	CHECK_DEVICE();
	PAINT->GetCurrentPoint(THIS, &x, &y);
	PAINT->CurveTo(THIS, x + VARG(x1), y + VARG(y1), x + VARG(x2), y + VARG(y2), x + VARG(x3), y + VARG(y3));
	THIS->has_path = TRUE;

END_METHOD


BEGIN_METHOD(Paint_LineTo, GB_FLOAT x; GB_FLOAT y)

	CHECK_DEVICE();
	PAINT->LineTo(THIS, VARG(x), VARG(y));
	THIS->has_path = TRUE;

END_METHOD


BEGIN_METHOD(Paint_RelLineTo, GB_FLOAT x; GB_FLOAT y)

	float fx, fy;
	CHECK_DEVICE();
	PAINT->GetCurrentPoint(THIS, &fx, &fy);
	PAINT->LineTo(THIS, fx + VARG(x), fy + VARG(y));
	THIS->has_path = TRUE;

END_METHOD


BEGIN_METHOD(Paint_MoveTo, GB_FLOAT x; GB_FLOAT y)

	CHECK_DEVICE();
	PAINT->MoveTo(THIS, VARG(x), VARG(y));

END_METHOD


BEGIN_METHOD(Paint_RelMoveTo, GB_FLOAT x; GB_FLOAT y)

	float fx, fy;
	CHECK_DEVICE();
	PAINT->GetCurrentPoint(THIS, &fx, &fy);
	PAINT->MoveTo(THIS, fx + VARG(x), fy + VARG(y));

END_METHOD


BEGIN_METHOD(Paint_Rectangle, GB_FLOAT x; GB_FLOAT y; GB_FLOAT w; GB_FLOAT h; GB_FLOAT radius)

	CHECK_DEVICE();
	
	float x = VARG(x);
	float y = VARG(y);
	float w = VARG(w);
	float h = VARG(h);
	float r = VARGOPT(radius, 0.0);
	
	if (r <= 0.0)
		PAINT->Rectangle(THIS, x, y, w, h);
	else
	{
		r = Min(r, Min(w, h) / 2);
		float r2 = r * (1-0.55228475);

		//PAINT->NewPath(THIS);
		
		PAINT->MoveTo(THIS, x + r, y);
		PAINT->LineTo(THIS, x + w - r, y);
		PAINT->CurveTo(THIS, x + w - r2, y, x + w, y + r2, x + w, y + r);
		PAINT->LineTo(THIS, x + w, y + h - r);
		PAINT->CurveTo(THIS, x + w, y + h - r2, x + w - r2, y + h, x + w - r, y + h);
		PAINT->LineTo(THIS, x + r, y + h);
		PAINT->CurveTo(THIS, x + r2, y + h, x, y + h - r2, x, y + h - r);
		PAINT->LineTo(THIS, x, y + r);
		PAINT->CurveTo(THIS, x, y + r2, x + r2, y, x + r, y);
	}

	THIS->has_path = TRUE;

END_METHOD


IMPLEMENT_PROPERTY(Paint_Font, Font, GB_FONT, GB_OBJECT, GB.ReturnObject)


BEGIN_METHOD(Paint_Text, GB_STRING text; GB_FLOAT x; GB_FLOAT y; GB_FLOAT w; GB_FLOAT h; GB_INTEGER align)

	CHECK_DEVICE();
	
	if (!MISSING(x) && !MISSING(y))
		PAINT->MoveTo(THIS, (float)VARG(x), (float)VARG(y));
	
	PAINT->Text(THIS, STRING(text), LENGTH(text), VARGOPT(w, -1), VARGOPT(h, -1), VARGOPT(align, GB_DRAW_ALIGN_DEFAULT), FALSE);
	THIS->has_path = TRUE;

END_METHOD


BEGIN_METHOD(Paint_RichText, GB_STRING text; GB_FLOAT x; GB_FLOAT y; GB_FLOAT w; GB_FLOAT h; GB_INTEGER align)

	CHECK_DEVICE();
	
	if (!MISSING(x) && !MISSING(y))
		PAINT->MoveTo(THIS, (float)VARG(x), (float)VARG(y));
	
	PAINT->RichText(THIS, STRING(text), LENGTH(text), VARGOPT(w, -1), VARGOPT(h, -1), VARGOPT(align, GB_DRAW_ALIGN_DEFAULT), FALSE);
	THIS->has_path = TRUE;

END_METHOD


BEGIN_METHOD(Paint_DrawText, GB_STRING text; GB_FLOAT x; GB_FLOAT y; GB_FLOAT w; GB_FLOAT h; GB_INTEGER align)

	CHECK_DEVICE();
	CHECK_PATH();

	if (!MISSING(x) && !MISSING(y))
		PAINT->MoveTo(THIS, (float)VARG(x), (float)VARG(y));
	
	PAINT->Text(THIS, STRING(text), LENGTH(text), VARGOPT(w, -1), VARGOPT(h, -1), VARGOPT(align, GB_DRAW_ALIGN_DEFAULT), TRUE);
	
END_METHOD


BEGIN_METHOD(Paint_DrawRichText, GB_STRING text; GB_FLOAT x; GB_FLOAT y; GB_FLOAT w; GB_FLOAT h; GB_INTEGER align)

	CHECK_DEVICE();
	CHECK_PATH();

	if (!MISSING(x) && !MISSING(y))
		PAINT->MoveTo(THIS, (float)VARG(x), (float)VARG(y));
	
	PAINT->RichText(THIS, STRING(text), LENGTH(text), VARGOPT(w, -1), VARGOPT(h, -1), VARGOPT(align, GB_DRAW_ALIGN_DEFAULT), TRUE);
	
END_METHOD


BEGIN_METHOD(Paint_TextExtents, GB_STRING text)

	PAINT_EXTENTS *extents;
	
	CHECK_DEVICE();

	extents = GB.New(GB.FindClass("PaintExtents"), NULL, NULL);
	
	if (!LENGTH(text))
	{
		PAINT->GetCurrentPoint(THIS, &extents->ext.x1, &extents->ext.y1);
		extents->ext.x2 = extents->ext.x1;
		extents->ext.y2 = extents->ext.y1;
	}
	else
		PAINT->TextExtents(THIS, STRING(text), LENGTH(text), &extents->ext);
	
	GB.ReturnObject(extents);

END_METHOD


BEGIN_METHOD(Paint_RichTextExtents, GB_STRING text; GB_FLOAT width)

	PAINT_EXTENTS *extents;
	
	CHECK_DEVICE();

	extents = GB.New(GB.FindClass("PaintExtents"), NULL, NULL);
	PAINT->RichTextExtents(THIS, STRING(text), LENGTH(text), &extents->ext, VARGOPT(width, -1));
	
	GB.ReturnObject(extents);

END_METHOD


BEGIN_METHOD(Paint_TextSize, GB_STRING text)

	float w, h;
	GEOM_RECTF *size;

	CHECK_DEVICE();
	PAINT->TextSize(THIS, STRING(text), LENGTH(text), &w, &h);
	size = GEOM.CreateRectF();
	size->w = w;
	size->h = h;
	GB.ReturnObject(size);

END_METHOD


BEGIN_METHOD(Paint_RichTextSize, GB_STRING text; GB_FLOAT width)

	float w, h;
	GEOM_RECTF *size;
	
	w = VARGOPT(width, -1);
	
	CHECK_DEVICE();
	PAINT->RichTextSize(THIS, STRING(text), LENGTH(text), w, &w, &h);
	size = GEOM.CreateRectF();
	size->w = w;
	size->h = h;
	GB.ReturnObject(size);

END_METHOD


static PAINT_BRUSH *make_brush(GB_PAINT *d, GB_BRUSH brush)
{
	PAINT_BRUSH *that;
	that = GB.New(GB.FindClass("PaintBrush"), NULL, NULL);
	that->desc = d->desc;
	that->brush = brush;
	GB.ReturnObject(that);
	return that;
}


BEGIN_METHOD(Paint_Color, GB_INTEGER color)

	GB_BRUSH brush;

	CHECK_DEVICE();
	
	PAINT->Brush.Color(&brush, VARG(color));
	make_brush(THIS, brush);

END_METHOD


BEGIN_METHOD(Paint_Image, GB_OBJECT image; GB_FLOAT x; GB_FLOAT y)

	GB_BRUSH brush;

	CHECK_DEVICE();
	
	if (GB.CheckObject(VARG(image)))
		return;
	
	PAINT->Brush.Image(&brush, (GB_IMAGE)VARG(image));
	make_brush(THIS, brush);
	
	if (!MISSING(x) || !MISSING(y))
	{
		GB_TRANSFORM transform;
		MPAINT->Create(&transform);
		MPAINT->Translate(transform, VARGOPT(x, 0.0), VARGOPT(y, 0.0));
		PAINT->Brush.Matrix(brush, TRUE, transform);
		MPAINT->Delete(&transform);
	}

END_METHOD


BEGIN_METHOD(Paint_LinearGradient, GB_FLOAT x0; GB_FLOAT y0; GB_FLOAT x1; GB_FLOAT y1; GB_OBJECT colors; GB_OBJECT positions; GB_INTEGER extend)

	GB_BRUSH brush;
	GB_ARRAY positions, colors;
	int nstop;
	
	CHECK_DEVICE();

	positions = (GB_ARRAY)VARG(positions);
	if (GB.CheckObject(positions))
		return;
	colors = (GB_ARRAY)VARG(colors);
	if (GB.CheckObject(colors))
		return;

	nstop = Min(GB.Array.Count(positions), GB.Array.Count(colors));
	
	PAINT->Brush.LinearGradient(&brush, (float)VARG(x0), (float)VARG(y0), (float)VARG(x1), (float)VARG(y1),
		nstop, (double *)GB.Array.Get(positions, 0), (GB_COLOR *)GB.Array.Get(colors, 0), VARGOPT(extend, GB_PAINT_EXTEND_PAD));

	make_brush(THIS, brush);

END_METHOD


BEGIN_METHOD(Paint_RadialGradient, GB_FLOAT cx; GB_FLOAT cy; GB_FLOAT radius; GB_FLOAT fx; GB_FLOAT fy; GB_OBJECT colors; GB_OBJECT positions; GB_INTEGER extend)

	GB_BRUSH brush;
	GB_ARRAY positions, colors;
	int nstop;
	
	CHECK_DEVICE();

	positions = (GB_ARRAY)VARG(positions);
	if (GB.CheckObject(positions))
		return;
	colors = (GB_ARRAY)VARG(colors);
	if (GB.CheckObject(colors))
		return;
	
	nstop = Min(GB.Array.Count(positions), GB.Array.Count(colors));
	
	PAINT->Brush.RadialGradient(&brush, (float)VARG(cx), (float)VARG(cy), (float)VARG(radius), (float)VARG(fx), (float)VARG(fy),
		nstop, (double *)GB.Array.Get(positions, 0), (GB_COLOR *)GB.Array.Get(colors, 0), VARGOPT(extend, GB_PAINT_EXTEND_PAD));
	
	make_brush(THIS, brush);

END_METHOD


BEGIN_PROPERTY(Paint_Matrix)

	GB_TRANSFORM transform;
	PAINT_MATRIX *matrix;
	
	CHECK_DEVICE();
	
	if (READ_PROPERTY)
	{
		MPAINT->Create(&transform);
		PAINT->Matrix(THIS, FALSE, transform);
		GB.ReturnObject(create_matrix(transform));
	}
	else
	{
		matrix = (PAINT_MATRIX *)VPROP(GB_OBJECT);
		if (!matrix)
			PAINT->Matrix(THIS, TRUE, NULL);
		else
			PAINT->Matrix(THIS, TRUE, matrix->transform);
	}

END_PROPERTY


BEGIN_METHOD_VOID(Paint_Reset)

	CHECK_DEVICE();
	PAINT->Matrix(THIS, TRUE, NULL);

END_METHOD


BEGIN_METHOD(Paint_Translate, GB_FLOAT tx; GB_FLOAT ty)

	GB_TRANSFORM transform;

	CHECK_DEVICE();
	MPAINT->Create(&transform);
	PAINT->Matrix(THIS, FALSE, transform);
	MPAINT->Translate(transform, (float)VARG(tx), (float)VARG(ty));
	PAINT->Matrix(THIS, TRUE, transform);
	MPAINT->Delete(&transform);

END_METHOD


BEGIN_METHOD(Paint_Scale, GB_FLOAT sx; GB_FLOAT sy)

	GB_TRANSFORM transform;

	CHECK_DEVICE();
	MPAINT->Create(&transform);
	PAINT->Matrix(THIS, FALSE, transform);
	MPAINT->Scale(transform, (float)VARG(sx), (float)VARG(sy));
	PAINT->Matrix(THIS, TRUE, transform);
	MPAINT->Delete(&transform);

END_METHOD


BEGIN_METHOD(Paint_Rotate, GB_FLOAT angle)

	GB_TRANSFORM transform;

	CHECK_DEVICE();
	MPAINT->Create(&transform);
	PAINT->Matrix(THIS, FALSE, transform);
	MPAINT->Rotate(transform, (float)VARG(angle));
	PAINT->Matrix(THIS, TRUE, transform);
	MPAINT->Delete(&transform);

END_METHOD


BEGIN_METHOD(Paint_DrawImage, GB_OBJECT image; GB_FLOAT x; GB_FLOAT y; GB_FLOAT width; GB_FLOAT height; GB_FLOAT opacity; GB_OBJECT source)

	GB_IMG *image;
	float x, y, w, h;
	float opacity = VARGOPT(opacity, 1.0);
	GEOM_RECT *source = (GEOM_RECT *)VARGOPT(source, NULL);

	CHECK_DEVICE();
	CHECK_PATH();

	image = (GB_IMG *)VARG(image);
	
	if (GB.CheckObject(image))
		return;
	
	x = VARG(x);
	y = VARG(y);
	w = VARGOPT(width, -1);
	h = VARGOPT(height, -1);
	
	if (GB.CheckObject(VARG(image)))
		return;
	
	if (w < 0) w = image->width;
	if (h < 0) h = image->height;
	
	if (w <= 0.0 || h <= 0.0)
		return;
	
	if (image->width <= 0 || image->height <= 0)
		return;
	
	PAINT->DrawImage(THIS, VARG(image), x, y, w, h, opacity, source ? (GB_RECT *)&source->x : NULL);

END_METHOD


BEGIN_METHOD(Paint_DrawPicture, GB_OBJECT picture; GB_FLOAT x; GB_FLOAT y; GB_FLOAT w; GB_FLOAT h; GB_OBJECT source)

	GB_PICTURE picture = VARG(picture);
	GB_PICTURE_INFO info;
	GEOM_RECT *source = (GEOM_RECT *)VARGOPT(source, NULL);
	float x, y, w, h;

	CHECK_DEVICE();
	CHECK_PATH();

	if (GB.CheckObject(picture))
		return;

	PAINT->GetPictureInfo(THIS, picture, &info);

	x = VARG(x);
	y = VARG(y);
	w = VARGOPT(w, -1);
	h = VARGOPT(h, -1);

	if (w < 0) w = info.width;
	if (h < 0) h = info.height;
	
	if (info.width <= 0 || info.height <= 0)
		return;
	
	PAINT->DrawPicture(THIS, picture, x, y, w, h, source ? (GB_RECT *)&source->x : NULL);

END_METHOD


BEGIN_METHOD(Paint_ZoomImage, GB_OBJECT image; GB_INTEGER zoom; GB_INTEGER x; GB_INTEGER y; GB_INTEGER grid; GB_OBJECT source)

	GB_IMAGE image = VARG(image);
	GB_IMG *info = (GB_IMG *)image;
	GEOM_RECT *source = (GEOM_RECT *)VARGOPT(source, NULL);
	int zoom;
	int x, y, sx, sy, sw, sh;
	int i, j, xr, yr;
	bool border;
	GB_COLOR borderColor;
	int antialias = FALSE;
	GB_RECT rect;
	float opacity = 1.0; //VARGOPT(opacity, 1.0);

	CHECK_DEVICE();
	CHECK_PATH();

	if (GB.CheckObject(image))
		return;

	zoom = VARG(zoom);
	if (zoom < 1)
	{
		GB.Error("Bad zoom factor");
		return;
	}
	
	x = VARGOPT(x, 0);
	y = VARGOPT(y, 0);

	if (source)
	{
		sx = source->x;
		sy = source->y;
		sw = source->w;
		sh = source->h;
	}
	else
	{
		sx = sy = 0;
		sw = info->width;
		sh = info->height;
	}

	DRAW_NORMALIZE(x, y, sw, sh, sx, sy, sw, sh, info->width, info->height);

	//DRAW->Fill.GetOrigin(THIS, &ox, &oy);
	
	PAINT->Save(THIS);
	PAINT->Antialias(THIS, TRUE, &antialias);
	
	borderColor = VARGOPT(grid, GB_COLOR_DEFAULT);
	border = borderColor != GB_COLOR_DEFAULT;

	rect.x = sx;
	rect.y = sy;
	rect.w = sw;
	rect.h = sh;
	
	PAINT->DrawImage(THIS, image, x, y, sw * zoom, sh * zoom, opacity, &rect);
	
	if (border && zoom >= 3)
	{
		for (i = sx, xr = x; i < (sx + sw); i++, xr += zoom)
			PAINT->FillRect(THIS, xr, y, 1, sh * zoom, borderColor);
		
		for (j = sy, yr = y; j < (sy + sh); j++, yr += zoom)
			PAINT->FillRect(THIS, x, yr, sw * zoom, 1, borderColor);
	}
	
	PAINT->Restore(THIS);

END_METHOD


#if 0
BEGIN_PROPERTY(Paint_LineStyle)

	int v;
	int count;
	float dashes[6];

	CHECK_DEVICE();

	if (READ_PROPERTY)
	{
		GB.ReturnInteger(THIS->lineStyle);
		return;
	}
	
	v = VPROP(GB_INTEGER);
	
	switch (v)
	{
		case GB_PAINT_LINE_STYLE_NONE:
			break;
			
		case GB_PAINT_LINE_STYLE_SOLID:
			PAINT->Dash(THIS, TRUE, NULL, &count);
			break;
			
		case GB_PAINT_LINE_STYLE_DASH:
			dashes[0] = 3; dashes[1] = 3; count = 2;
			PAINT->Dash(THIS, TRUE, dashes, &count);
			break;
			
		case GB_PAINT_LINE_STYLE_DOT:
			dashes[0] = 1; dashes[1] = 3; count = 2;
			PAINT->Dash(THIS, TRUE, dashes, &count);
			break;
			
		case GB_PAINT_LINE_STYLE_DASH_DOT:
			dashes[0] = 3; dashes[1] = 3; dashes[2] = 3; dashes[3] = 1; dacount = 4;
			PAINT->Dash(THIS, TRUE, dashes, &count);
			break;
			
		case GB_PAINT_LINE_STYLE_DASH_DOT_DOT:
			dashes[0] = 3; dashes[1] = 3; dashes[2] = 3; dashes[3] = 1; dashes[4] = 3; dashes[5] = 1; dacount = 6;
			PAINT->Dash(THIS, TRUE, dashes, &count);
			break;
			
		default:
			return;
	}

	THIS->lineStyle = v;

END_PROPERTY
#endif

BEGIN_METHOD(Paint_FillRect, GB_FLOAT x; GB_FLOAT y; GB_FLOAT w; GB_FLOAT h; GB_INTEGER color)

	CHECK_DEVICE();
	CHECK_PATH();
	PAINT->FillRect(THIS, VARG(x), VARG(y), VARG(w), VARG(h), VARG(color));
	
END_METHOD


BEGIN_PROPERTY(Paint_Tag)

	CHECK_DEVICE();
	
	if (READ_PROPERTY)
		GB.ReturnObject(THIS->tag);
	else
		GB.StoreObject(PROP(GB_OBJECT), POINTER(&(THIS->tag)));

END_PROPERTY

#if 0
Static Public Sub DrawEllipsizedText(Text As String, X As Integer, Y As Integer, W As Integer, H As Integer, Optional Alignment As Integer = Align.TopLeft)

  Dim aElt As String[] = Split(Text, " \n")
  Dim sElt As String
  Dim I As Integer
  Dim HL As Integer = Paint.Font.Height
  Dim YT, WT As Integer
  Dim sText As String

  YT = 0

  For I = 0 To aElt.Max
    sElt = aElt[I]

    If (YT + HL * 2) > H Then
      WT = Paint.Font.TextWidth(LTrim(sText & " " & sElt & "…"))
      If WT > W Then
        Paint.DrawText(sText & "…", X, Y + YT, W, H, Align.TopLeft)
        Return
      Endif
    Else
      WT = Paint.Font.TextWidth(LTrim(sText & " " & sElt))
      If WT > W Then
        Paint.DrawText(sText, X, Y + YT, W, H, Align.TopLeft)
        sText = ""
        YT += HL
      Endif
    Endif

    sText = RTrim(sText & " " & sElt)

  Next

  Paint.DrawText(sText, X, Y + YT, W, H, Align.TopLeft)

End
#endif

//-- Style API --------------------------------------------------------------

#define GET_COORD() \
	int x, y, w, h; \
\
	CHECK_DEVICE(); \
\
	x = VARG(x); \
	y = VARG(y); \
	w = VARG(w); \
	h = VARG(h); \
\
	if (w < 1 || h < 1) \
		return;


GB_DESC PaintDesc[] =
{
	GB_DECLARE("Paint", 0), GB_VIRTUAL_CLASS(),

	GB_STATIC_METHOD("_exit", NULL, Paint_exit, NULL),

	GB_CONSTANT("ExtendPad", "i",        GB_PAINT_EXTEND_PAD),
	GB_CONSTANT("ExtendRepeat", "i",     GB_PAINT_EXTEND_REPEAT),
	GB_CONSTANT("ExtendReflect", "i",    GB_PAINT_EXTEND_REFLECT),

	GB_CONSTANT("FillRuleWinding", "i",  GB_PAINT_FILL_RULE_WINDING),
	GB_CONSTANT("FillRuleEvenOdd", "i",  GB_PAINT_FILL_RULE_EVEN_ODD),

	GB_CONSTANT("LineCapButt", "i",      GB_PAINT_LINE_CAP_BUTT),
	GB_CONSTANT("LineCapRound", "i",     GB_PAINT_LINE_CAP_ROUND),
	GB_CONSTANT("LineCapSquare", "i",    GB_PAINT_LINE_CAP_SQUARE),
	
	GB_CONSTANT("LineJoinMiter", "i",    GB_PAINT_LINE_JOIN_MITER),
	GB_CONSTANT("LineJoinRound", "i",    GB_PAINT_LINE_JOIN_ROUND),
	GB_CONSTANT("LineJoinBevel", "i",    GB_PAINT_LINE_JOIN_BEVEL),
	
	GB_CONSTANT("OperatorClear", "i",    GB_PAINT_OPERATOR_CLEAR),
  GB_CONSTANT("OperatorSource", "i",   GB_PAINT_OPERATOR_SOURCE),
	GB_CONSTANT("OperatorOver", "i",     GB_PAINT_OPERATOR_OVER),
	GB_CONSTANT("OperatorIn", "i",       GB_PAINT_OPERATOR_IN),
	GB_CONSTANT("OperatorOut", "i",      GB_PAINT_OPERATOR_OUT),
	GB_CONSTANT("OperatorATop", "i",     GB_PAINT_OPERATOR_ATOP),
	GB_CONSTANT("OperatorDest", "i",     GB_PAINT_OPERATOR_DEST),
	GB_CONSTANT("OperatorDestOver", "i", GB_PAINT_OPERATOR_DEST_OVER),
	GB_CONSTANT("OperatorDestIn", "i",   GB_PAINT_OPERATOR_DEST_IN),
	GB_CONSTANT("OperatorDestOut", "i",  GB_PAINT_OPERATOR_DEST_OUT),
	GB_CONSTANT("OperatorDestATop", "i", GB_PAINT_OPERATOR_DEST_ATOP),
	GB_CONSTANT("OperatorXor", "i",      GB_PAINT_OPERATOR_XOR),
	GB_CONSTANT("OperatorAdd", "i",      GB_PAINT_OPERATOR_ADD),
	GB_CONSTANT("OperatorSaturate", "i", GB_PAINT_OPERATOR_SATURATE),

	GB_STATIC_METHOD("Begin", NULL, Paint_Begin, "(Device)o"),
	GB_STATIC_METHOD("End", NULL, Paint_End, NULL),
	
	GB_STATIC_PROPERTY_READ("Device", "o", Paint_Device),
	GB_STATIC_PROPERTY_READ("W", "f", Paint_Width),
	GB_STATIC_PROPERTY_READ("H", "f", Paint_Height),
	GB_STATIC_PROPERTY_READ("Width", "f", Paint_Width),
	GB_STATIC_PROPERTY_READ("Height", "f", Paint_Height),
	GB_STATIC_PROPERTY_READ("ResolutionX", "i", Paint_ResolutionX),
	GB_STATIC_PROPERTY_READ("ResolutionY", "i", Paint_ResolutionY),
	GB_STATIC_PROPERTY("AntiAlias", "b", Paint_Antialias),
	
	GB_STATIC_PROPERTY("_Invert", "b", Paint_Invert),
	GB_STATIC_PROPERTY("_FillStyle", "i", Paint_FillStyle),
	GB_STATIC_PROPERTY("Background", "i", Paint_Background),
	GB_STATIC_PROPERTY("_Tag", "o", Paint_Tag),

	GB_STATIC_METHOD("Save", NULL, Paint_Save, NULL),
	GB_STATIC_METHOD("Restore", NULL, Paint_Restore, NULL),
	
	GB_STATIC_METHOD("Clip", NULL, Paint_Clip, "[(Preserve)b]"),
	GB_STATIC_METHOD("ResetClip", NULL, Paint_ResetClip, NULL),
	GB_STATIC_PROPERTY_READ("ClipExtents", "PaintExtents", Paint_ClipExtents),
	GB_STATIC_PROPERTY("ClipRect", "Rect", Paint_ClipRect),
	
	GB_STATIC_METHOD("Fill", NULL, Paint_Fill, "[(Preserve)b]"),
	//GB_STATIC_PROPERTY_READ("FillExtents", "PaintExtents", Paint_FillExtents),
	//GB_STATIC_METHOD("InFill", "b", Paint_InFill, "(X)f(Y)f"),
	
	//GB_STATIC_METHOD("Mask", NULL, CAIRO_mask, "(Pattern)CairoPattern;"),
	
	//GB_STATIC_METHOD("Paint", NULL, CAIRO_paint, "[(Alpha)f]"),

	GB_STATIC_METHOD("Stroke", NULL, Paint_Stroke, "[(Preserve)b]"),
	//GB_STATIC_PROPERTY_READ("StrokeExtents", "PaintExtents", Paint_StrokeExtents),
	//GB_STATIC_METHOD("InStroke", "b", Paint_InStroke, "(X)f(Y)f"),

	GB_STATIC_PROPERTY_READ("PathExtents", "PaintExtents", Paint_PathExtents),
	GB_STATIC_METHOD("PathContains", "b", Paint_PathContains, "(X)f(Y)f"),
	GB_STATIC_PROPERTY_READ("PathOutline", "PointF[][]", Paint_PathOutline),

	GB_STATIC_PROPERTY("Brush", "PaintBrush", Paint_Brush),
	GB_STATIC_PROPERTY("BrushOrigin", "PointF", Paint_BrushOrigin),
	GB_STATIC_PROPERTY("Dash", "Float[]", Paint_Dash),
	GB_STATIC_PROPERTY("DashOffset", "f", Paint_DashOffset),
	GB_STATIC_PROPERTY("FillRule", "i", Paint_FillRule),
	GB_STATIC_PROPERTY("LineCap", "i", Paint_LineCap),
	GB_STATIC_PROPERTY("LineJoin", "i", Paint_LineJoin),
	GB_STATIC_PROPERTY("LineWidth", "f", Paint_LineWidth),
	//GB_STATIC_PROPERTY("LineStyle", "i", Paint_LineStyle),
	GB_STATIC_PROPERTY("MiterLimit", "f", Paint_MiterLimit),
	GB_STATIC_PROPERTY("Operator", "i", Paint_Operator),
	//GB_STATIC_PROPERTY("Tolerance", "f", CAIRO_tolerance),
	
	GB_STATIC_METHOD("NewPath", NULL, Paint_NewPath, NULL),
	GB_STATIC_METHOD("ClosePath", NULL, Paint_ClosePath, NULL),
	
	GB_STATIC_PROPERTY_READ("X", "f", Paint_X),
	GB_STATIC_PROPERTY_READ("Y", "f", Paint_Y),

	GB_STATIC_METHOD("Rectangle", NULL, Paint_Rectangle, "(X)f(Y)f(Width)f(Height)f[(Radius)f]"),
	GB_STATIC_METHOD("FillRect", NULL, Paint_FillRect, "(X)f(Y)f(Width)f(Height)f(Color)i"),
	GB_STATIC_METHOD("Arc", NULL, Paint_Arc, "(XC)f(YC)f(Radius)f[(Angle)f(Length)f(Pie)b]"),
	GB_STATIC_METHOD("Ellipse", NULL, Paint_Ellipse, "(X)f(Y)f(Width)f(Height)f[(Angle)f(Length)f(Pie)b]"),
	GB_STATIC_METHOD("Polygon", NULL, Paint_Polygon, "(Points)Float[];"),

	GB_STATIC_METHOD("CurveTo", NULL, Paint_CurveTo, "(X1)f(Y1)f(X2)f(Y2)f(X3)f(Y3)f"),
	GB_STATIC_METHOD("RelCurveTo", NULL, Paint_RelCurveTo, "(X1)f(Y1)f(X2)f(Y2)f(X3)f(Y3)f"),
	GB_STATIC_METHOD("LineTo", NULL, Paint_LineTo, "(X)f(Y)f"),
	GB_STATIC_METHOD("RelLineTo", NULL, Paint_RelLineTo, "(X)f(Y)f"),
	GB_STATIC_METHOD("MoveTo", NULL, Paint_MoveTo, "(X)f(Y)f"),
	GB_STATIC_METHOD("RelMoveTo", NULL, Paint_RelMoveTo, "(X)f(Y)f"),
	GB_STATIC_PROPERTY("Font", "Font", Paint_Font),
	GB_STATIC_METHOD("Text", NULL, Paint_Text, "(Text)s[(X)f(Y)f(Width)f(Height)f(Alignment)i)]"),
	GB_STATIC_METHOD("TextSize", "RectF", Paint_TextSize, "(Text)s"),
	GB_STATIC_METHOD("RichText", NULL, Paint_RichText, "(Text)s[(X)f(Y)f(Width)f(Height)f(Alignment)i)]"),
	GB_STATIC_METHOD("RichTextSize", "RectF", Paint_RichTextSize, "(Text)s[(Width)f]"),
	GB_STATIC_METHOD("DrawText", NULL, Paint_DrawText, "(Text)s[(X)f(Y)f(Width)f(Height)f(Alignment)i)]"),
	GB_STATIC_METHOD("DrawRichText", NULL, Paint_DrawRichText, "(Text)s[(X)f(Y)f(Width)f(Height)f(Alignment)i)]"),
	GB_STATIC_METHOD("TextExtents", "PaintExtents", Paint_TextExtents, "(Text)s"),
	GB_STATIC_METHOD("RichTextExtents", "PaintExtents", Paint_RichTextExtents, "(Text)s[(Width)f]"),
	
	GB_STATIC_METHOD("Color", "PaintBrush", Paint_Color, "(Color)i"),
	GB_STATIC_METHOD("Image", "PaintBrush", Paint_Image, "(Image)Image;[(X)f(Y)f]"),
	GB_STATIC_METHOD("LinearGradient", "PaintBrush", Paint_LinearGradient, "(X0)f(Y0)f(X1)f(Y1)f(Colors)Integer[];(Positions)Float[];[(Extend)i]"),
	GB_STATIC_METHOD("RadialGradient", "PaintBrush", Paint_RadialGradient, "(CX)f(CY)f(Radius)f(FX)f(FY)f(Colors)Integer[];(Positions)Float[];[(Extend)i]"),

	GB_STATIC_PROPERTY("Matrix", "PaintMatrix", Paint_Matrix),

	GB_STATIC_METHOD("Reset", NULL, Paint_Reset, NULL),
	GB_STATIC_METHOD("Translate", NULL, Paint_Translate, "(TX)f(TY)f"),
	GB_STATIC_METHOD("Scale", NULL, Paint_Scale, "(SX)f(SY)f"),
	GB_STATIC_METHOD("Rotate", NULL, Paint_Rotate, "(Angle)f"),
	
	//GB_STATIC_METHOD("Clear", NULL, Paint_Clear, NULL),
	
	GB_STATIC_METHOD("DrawImage", NULL, Paint_DrawImage, "(Image)Image;(X)f(Y)f[(Width)f(Height)f(Opacity)f(Source)Rect;]"),
	GB_STATIC_METHOD("DrawPicture", NULL, Paint_DrawPicture, "(Picture)Picture;(X)f(Y)f[(Width)f(Height)f(Source)Rect;]"),
	GB_STATIC_METHOD("ZoomImage", NULL, Paint_ZoomImage, "(Image)Image;(Zoom)i(X)i(Y)i[(Grid)i(Source)Rect;]"),


	GB_END_DECLARE
};


