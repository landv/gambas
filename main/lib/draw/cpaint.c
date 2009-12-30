/***************************************************************************

  cpaint.c

  (c) 2000-2009 Benoît Minisini <gambas@users.sourceforge.net>

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

#define __CPAINT_C

#include "gb.image.h"
#include "main.h"
#include "cpaint.h"

static GB_PAINT *_current = NULL;
#define THIS _current
#define PAINT _current->desc

#define XTHIS ((PAINT_EXTENTS *)_object)

#define MTHIS ((PAINT_MATRIX *)_object)
#define MPAINT (MTHIS->desc)

#define BTHIS ((PAINT_BRUSH *)_object)
#define BPAINT (BTHIS->desc)

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

GB_PAINT *PAINT_get_current()
{
	check_device();
	return _current;
}

#define CHECK_DEVICE() if (check_device()) return

bool PAINT_begin(void *device)
{
	GB_PAINT_DESC *desc;
	GB_PAINT *paint;
	GB_CLASS klass;
	
	klass = GB.GetClass(device);
	desc = (GB_PAINT_DESC *)GB.GetClassInterface(klass, "Paint");

	if (!desc)
	{
		GB.Error("Not a paintable object");
		return TRUE;
	}

	GB.Alloc(POINTER(&paint), sizeof(GB_PAINT));
	GB.Alloc(POINTER(&paint->extra), desc->size);
	memset(paint->extra, 0, desc->size);
	
	paint->desc = desc;
	paint->previous = _current;
	GB.Ref(device);
	paint->device = device;
	paint->brush = NULL;

	_current = paint;
	
	if (PAINT->Begin(paint))
		return TRUE;
	
	//DRAW->SetBackground(draw, GB_PAINT_COLOR_DEFAULT);
	//DRAW->SetForeground(draw, GB_PAINT_COLOR_DEFAULT);
	//DRAW->Fill.SetColor(draw, GB_PAINT_COLOR_DEFAULT);
	return FALSE;
}

void PAINT_end()
{
	GB_PAINT *paint;

	if (!_current)
		return;
		
	paint = _current;
	_current = _current->previous;
	
	paint->desc->End(paint);

	if (paint->brush)
		GB.Unref(POINTER(&paint->brush));
	GB.Unref(POINTER(&paint->device));
	GB.Free(POINTER(&paint->extra));
	GB.Free(POINTER(&paint));
}


/**** PaintExtents *********************************************************/

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

/**** PaintMatrix **********************************************************/

static PAINT_MATRIX *create_matrix(GB_PAINT_DESC *desc, GB_TRANSFORM *transform)
{
	PAINT_MATRIX *matrix;
	GB.New(POINTER(&matrix), GB.FindClass("PaintMatrix"), NULL, NULL);
	matrix->desc = desc;
	matrix->transform = transform;
	return matrix;
}

BEGIN_METHOD_VOID(PaintMatrix_free)

	MPAINT->Transform.Delete(&MTHIS->transform);

END_METHOD

BEGIN_METHOD_VOID(PaintMatrix_Reset)

	MPAINT->Transform.Init(MTHIS->transform, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0);
	RETURN_SELF();

END_METHOD

BEGIN_METHOD(PaintMatrix_Translate, GB_FLOAT tx; GB_FLOAT ty)

	MPAINT->Transform.Translate(MTHIS->transform, (float)VARG(tx), (float)VARG(ty));
	RETURN_SELF();

END_METHOD

BEGIN_METHOD(PaintMatrix_Scale, GB_FLOAT sx; GB_FLOAT sy)

	MPAINT->Transform.Scale(MTHIS->transform, (float)VARG(sx), (float)VARG(sy));
	RETURN_SELF();

END_METHOD

BEGIN_METHOD(PaintMatrix_Rotate, GB_FLOAT angle)

	MPAINT->Transform.Rotate(MTHIS->transform, (float)VARG(angle));
	RETURN_SELF();

END_METHOD

BEGIN_METHOD_VOID(PaintMatrix_Invert)

	if (MPAINT->Transform.Invert(MTHIS->transform))
		GB.ReturnNull();
	else
		RETURN_SELF();

END_METHOD

BEGIN_METHOD(PaintMatrix_Multiply, GB_OBJECT matrix)

	PAINT_MATRIX *matrix = (PAINT_MATRIX *)VARG(matrix);
	
	if (GB.CheckObject(matrix))
		return;

	MPAINT->Transform.Multiply(MTHIS->transform, matrix->transform);
	RETURN_SELF();

END_METHOD

GB_DESC PaintMatrixDesc[] = 
{
	GB_DECLARE("PaintMatrix", sizeof(PAINT_MATRIX)), GB_NOT_CREATABLE(),

	GB_METHOD("_free", NULL, PaintMatrix_free, NULL),

	GB_METHOD("Reset", "PaintMatrix", PaintMatrix_Reset, NULL),
	GB_METHOD("Translate", "PaintMatrix", PaintMatrix_Translate, "(TX)f(TY)f"),
	GB_METHOD("Scale", "PaintMatrix", PaintMatrix_Scale, "(SX)f(SY)f"),
	GB_METHOD("Rotate", "PaintMatrix", PaintMatrix_Rotate, "(Angle)f"),
	GB_METHOD("Invert", "PaintMatrix", PaintMatrix_Invert, NULL),
	GB_METHOD("Multiply", "PaintMatrix", PaintMatrix_Multiply, "(Matrix)PaintMatrix;"),	

	GB_END_DECLARE	
};


/**** PaintBrush ***********************************************************/

BEGIN_METHOD_VOID(PaintBrush_free)

	BPAINT->Brush.Free(BTHIS->brush);

END_METHOD

BEGIN_PROPERTY(PaintBrush_Matrix)

	GB_TRANSFORM transform;
	PAINT_MATRIX *matrix;
	
	if (READ_PROPERTY)
	{
		BPAINT->Transform.Create(&transform);
		BPAINT->Brush.Matrix(BTHIS->brush, FALSE, transform);
		GB.ReturnObject(create_matrix(BPAINT, transform));
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

	BPAINT->Transform.Create(&transform);
	BPAINT->Brush.Matrix(BTHIS->brush, FALSE, transform);
	BPAINT->Transform.Translate(transform, (float)VARG(tx), (float)VARG(ty));
	BPAINT->Brush.Matrix(BTHIS->brush, TRUE, transform);
	BPAINT->Transform.Delete(&transform);

END_METHOD

BEGIN_METHOD(PaintBrush_Scale, GB_FLOAT sx; GB_FLOAT sy)

	GB_TRANSFORM transform;

	BPAINT->Transform.Create(&transform);
	BPAINT->Brush.Matrix(BTHIS->brush, FALSE, transform);
	BPAINT->Transform.Scale(transform, (float)VARG(sx), (float)VARG(sy));
	BPAINT->Brush.Matrix(BTHIS->brush, TRUE, transform);
	BPAINT->Transform.Delete(&transform);

END_METHOD

BEGIN_METHOD(PaintBrush_Rotate, GB_FLOAT angle)

	GB_TRANSFORM transform;

	BPAINT->Transform.Create(&transform);
	BPAINT->Brush.Matrix(BTHIS->brush, FALSE, transform);
	BPAINT->Transform.Rotate(transform, (float)VARG(angle));
	BPAINT->Brush.Matrix(BTHIS->brush, TRUE, transform);
	BPAINT->Transform.Delete(&transform);

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


/**** Paint ****************************************************************/

BEGIN_METHOD(Paint_begin, GB_OBJECT device)

	void *device = VARG(device);

	if (GB.CheckObject(device))
		return;

	PAINT_begin(device);

END_METHOD


BEGIN_METHOD_VOID(Paint_end)

	PAINT_end();
	
END_METHOD


BEGIN_METHOD_VOID(Paint_exit)

	while (_current)
		PAINT_end();

END_METHOD

BEGIN_PROPERTY(Paint_Device)
	
	CHECK_DEVICE();
	GB.ReturnObject(THIS->device);

END_PROPERTY

BEGIN_PROPERTY(Paint_Width)

	CHECK_DEVICE();
	GB.ReturnInteger(THIS->width);

END_PROPERTY

BEGIN_PROPERTY(Paint_Height)

	CHECK_DEVICE();
	GB.ReturnInteger(THIS->height);

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
	CHECK_DEVICE(); \
	PAINT->_api(THIS, VARGOPT(preserve, FALSE)); \
END_METHOD

#define IMPLEMENT_PROPERTY_EXTENTS(_property, _api) \
BEGIN_PROPERTY(_property) \
	PAINT_EXTENTS *extents; \
	CHECK_DEVICE(); \
	GB.New(POINTER(&extents), GB.FindClass("PaintExtents"), NULL, NULL); \
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

#define IMPLEMENT_PROPERTY_INTEGER(_property, _api) \
	IMPLEMENT_PROPERTY(_property, _api, int, GB_INTEGER, GB.ReturnInteger)

#define IMPLEMENT_PROPERTY_FLOAT(_property, _api) \
	IMPLEMENT_PROPERTY(_property, _api, float, GB_FLOAT, GB.ReturnFloat)

IMPLEMENT_METHOD(Paint_Save, Save)
IMPLEMENT_METHOD(Paint_Restore, Restore)
IMPLEMENT_METHOD_PRESERVE(Paint_Clip, Clip)
IMPLEMENT_METHOD(Paint_ResetClip, ResetClip)
IMPLEMENT_PROPERTY_EXTENTS(Paint_ClipExtents, ClipExtents)
IMPLEMENT_METHOD_PRESERVE(Paint_Fill, Fill)
IMPLEMENT_METHOD_PRESERVE(Paint_Stroke, Stroke)
IMPLEMENT_PROPERTY_EXTENTS(Paint_PathExtents, PathExtents)

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
		}
		else
		{
			GB.Alloc(POINTER(&dashes), sizeof(float) * count);
			for (i = 0; i < count; i++)
				dashes[i] = (float)*((double *)GB.Array.Get(array, i));
			PAINT->Dash(THIS, TRUE, &dashes, &count);
			GB.Free(POINTER(&dashes));
		}
	}

END_PROPERTY

IMPLEMENT_PROPERTY_FLOAT(Paint_DashOffset, DashOffset)
IMPLEMENT_METHOD(Paint_NewPath, NewPath)
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

BEGIN_METHOD(Paint_Arc, GB_FLOAT xc; GB_FLOAT yc; GB_FLOAT radius; GB_FLOAT angle; GB_FLOAT length)

	CHECK_DEVICE();
	PAINT->Arc(THIS, VARG(xc), VARG(yc), VARG(radius), VARGOPT(angle, 0.0), VARGOPT(length, MISSING(angle) ? M_PI * 2 : 0.0));

END_METHOD

BEGIN_METHOD(Paint_CurveTo, GB_FLOAT x1; GB_FLOAT y1; GB_FLOAT x2; GB_FLOAT y2; GB_FLOAT x3; GB_FLOAT y3)

	CHECK_DEVICE();
	PAINT->CurveTo(THIS, VARG(x1), VARG(y1), VARG(x2), VARG(y2), VARG(x3), VARG(y3));

END_METHOD

BEGIN_METHOD(Paint_RelCurveTo, GB_FLOAT x1; GB_FLOAT y1; GB_FLOAT x2; GB_FLOAT y2; GB_FLOAT x3; GB_FLOAT y3)

	float x, y;
	CHECK_DEVICE();
	PAINT->GetCurrentPoint(THIS, &x, &y);
	PAINT->CurveTo(THIS, x + VARG(x1), y + VARG(y1), x + VARG(x2), y + VARG(y2), x + VARG(x3), y + VARG(y3));

END_METHOD

BEGIN_METHOD(Paint_LineTo, GB_FLOAT x; GB_FLOAT y)

	CHECK_DEVICE();
	PAINT->LineTo(THIS, VARG(x), VARG(y));

END_METHOD

BEGIN_METHOD(Paint_RelLineTo, GB_FLOAT x; GB_FLOAT y)

	float fx, fy;
	CHECK_DEVICE();
	PAINT->GetCurrentPoint(THIS, &fx, &fy);
	PAINT->LineTo(THIS, fx + VARG(x), fy + VARG(y));

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

BEGIN_METHOD(Paint_Rectangle, GB_FLOAT x; GB_FLOAT y; GB_FLOAT w; GB_FLOAT h)

	CHECK_DEVICE();
	PAINT->Rectangle(THIS, VARG(x), VARG(y), VARG(w), VARG(h));

END_METHOD

IMPLEMENT_PROPERTY(Paint_Font, Font, GB_FONT, GB_OBJECT, GB.ReturnObject)

BEGIN_METHOD(Paint_Text, GB_STRING text; GB_FLOAT x; GB_FLOAT y; GB_FLOAT w; GB_FLOAT h; GB_INTEGER align)

	CHECK_DEVICE();
	
	if (!MISSING(x) && !MISSING(y))
		PAINT->MoveTo(THIS, (float)VARG(x), (float)VARG(y));
	
	PAINT->Text(THIS, STRING(text), LENGTH(text), VARGOPT(w, -1), VARGOPT(h, -1), VARGOPT(align, GB_DRAW_ALIGN_DEFAULT));
	
END_METHOD

/*
BEGIN_METHOD(Paint_RichText, GB_STRING text; GB_FLOAT x; GB_FLOAT y; GB_FLOAT w; GB_FLOAT h; GB_INTEGER align)

	CHECK_DEVICE();
	
	if (!MISSING(x) && !MISSING(y))
		PAINT->MoveTo(THIS, (float)VARG(x), (float)VARG(y));
	
	PAINT->RichText(THIS, STRING(text), LENGTH(text), VARGOPT(w, -1), VARGOPT(h, -1), VARGOPT(align, GB_DRAW_ALIGN_DEFAULT));
	
END_METHOD
*/

/*BEGIN_METHOD(Paint_TextExtents, GB_STRING text)

	PAINT_EXTENTS *extents;
	
	CHECK_DEVICE();

	GB.New(POINTER(&extents), GB.FindClass("PaintExtents"), NULL, NULL);
	PAINT->TextExtents(THIS, STRING(text), LENGTH(text), &extents->ext);
	
	GB.ReturnObject(extents);

END_METHOD*/

static PAINT_BRUSH *make_brush(GB_PAINT *d, GB_BRUSH brush)
{
	PAINT_BRUSH *that;
	GB.New(POINTER(&that), GB.FindClass("PaintBrush"), NULL, NULL);
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

	PAINT_BRUSH *pb;
	GB_BRUSH brush;

	CHECK_DEVICE();
	
	if (GB.CheckObject(VARG(image)))
		return;
	
	PAINT->Brush.Image(&brush, (GB_IMAGE)VARG(image));
	pb = make_brush(THIS, brush);
	
	if (!MISSING(x) || !MISSING(y))
	{
		GB_TRANSFORM transform;
		PAINT->Transform.Create(&transform);
		PAINT->Transform.Translate(transform, -VARGOPT(x, 0.0), -VARGOPT(y, 0.0));
		PAINT->Brush.Matrix(brush, TRUE, transform);
		PAINT->Transform.Delete(&transform);
	}

END_METHOD

BEGIN_METHOD(Paint_LinearGradient, GB_FLOAT x0; GB_FLOAT y0; GB_FLOAT x1; GB_FLOAT y1; GB_OBJECT colors; GB_OBJECT positions; GB_INTEGER extend)

	GB_BRUSH brush;
	GB_ARRAY positions, colors;
	int nstop;
	
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
		PAINT->Transform.Create(&transform);
		PAINT->Matrix(THIS, FALSE, transform);
		GB.ReturnObject(create_matrix(PAINT, transform));
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
	PAINT->Transform.Create(&transform);
	PAINT->Matrix(THIS, FALSE, transform);
	PAINT->Transform.Translate(transform, (float)VARG(tx), (float)VARG(ty));
	PAINT->Matrix(THIS, TRUE, transform);
	PAINT->Transform.Delete(&transform);

END_METHOD

BEGIN_METHOD(Paint_Scale, GB_FLOAT sx; GB_FLOAT sy)

	GB_TRANSFORM transform;

	CHECK_DEVICE();
	PAINT->Transform.Create(&transform);
	PAINT->Matrix(THIS, FALSE, transform);
	PAINT->Transform.Scale(transform, (float)VARG(sx), (float)VARG(sy));
	PAINT->Matrix(THIS, TRUE, transform);
	PAINT->Transform.Delete(&transform);

END_METHOD

BEGIN_METHOD(Paint_Rotate, GB_FLOAT angle)

	GB_TRANSFORM transform;

	CHECK_DEVICE();
	PAINT->Transform.Create(&transform);
	PAINT->Matrix(THIS, FALSE, transform);
	PAINT->Transform.Rotate(transform, (float)VARG(angle));
	PAINT->Matrix(THIS, TRUE, transform);
	PAINT->Transform.Delete(&transform);

END_METHOD

GB_DESC PaintDesc[] =
{
	GB_DECLARE("Paint", 0), GB_VIRTUAL_CLASS(),

	GB_STATIC_METHOD("_exit", NULL, Paint_exit, NULL),

	GB_CONSTANT("ExtendPad", "i",                  GB_PAINT_EXTEND_PAD),
	GB_CONSTANT("ExtendRepeat", "i",               GB_PAINT_EXTEND_REPEAT),
	GB_CONSTANT("ExtendReflect", "i",              GB_PAINT_EXTEND_REFLECT),

	GB_CONSTANT("FillRuleWinding", "i",            GB_PAINT_FILL_RULE_WINDING),
	GB_CONSTANT("FillRuleEvenOdd", "i",            GB_PAINT_FILL_RULE_EVEN_ODD),

	GB_CONSTANT("LineCapButt", "i",                GB_PAINT_LINE_CAP_BUTT),
	GB_CONSTANT("LineCapRound", "i",               GB_PAINT_LINE_CAP_ROUND),
	GB_CONSTANT("LineCapSquare", "i",              GB_PAINT_LINE_CAP_SQUARE),
	
	GB_CONSTANT("LineJoinMiter", "i",              GB_PAINT_LINE_JOIN_MITER),
	GB_CONSTANT("LineJoinRound", "i",              GB_PAINT_LINE_JOIN_ROUND),
	GB_CONSTANT("LineJoinBevel", "i",              GB_PAINT_LINE_JOIN_BEVEL),
	
	GB_CONSTANT("OperatorClear", "i",              GB_PAINT_OPERATOR_CLEAR),
  GB_CONSTANT("OperatorSource", "i",             GB_PAINT_OPERATOR_SOURCE),
	GB_CONSTANT("OperatorOver", "i",               GB_PAINT_OPERATOR_OVER),
	GB_CONSTANT("OperatorIn", "i",                 GB_PAINT_OPERATOR_IN),
	GB_CONSTANT("OperatorOut", "i",                GB_PAINT_OPERATOR_OUT),
	GB_CONSTANT("OperatorATop", "i",               GB_PAINT_OPERATOR_ATOP),
	GB_CONSTANT("OperatorDest", "i",               GB_PAINT_OPERATOR_DEST),
	GB_CONSTANT("OperatorDestOver", "i",           GB_PAINT_OPERATOR_DEST_OVER),
	GB_CONSTANT("OperatorDestIn", "i",             GB_PAINT_OPERATOR_DEST_IN),
	GB_CONSTANT("OperatorDestOut", "i",            GB_PAINT_OPERATOR_DEST_OUT),
	GB_CONSTANT("OperatorDestATop", "i",           GB_PAINT_OPERATOR_DEST_ATOP),
	GB_CONSTANT("OperatorXor", "i",                GB_PAINT_OPERATOR_XOR),
	GB_CONSTANT("OperatorAdd", "i",                GB_PAINT_OPERATOR_ADD),
	GB_CONSTANT("OperatorSaturate", "i",           GB_PAINT_OPERATOR_SATURATE),

	GB_STATIC_METHOD("Begin", NULL, Paint_begin, "(Device)o"),
	GB_STATIC_METHOD("End", NULL, Paint_end, NULL),
	
	GB_STATIC_PROPERTY_READ("Device", "o", Paint_Device),
	GB_STATIC_PROPERTY_READ("W", "i", Paint_Width),
	GB_STATIC_PROPERTY_READ("H", "i", Paint_Height),
	GB_STATIC_PROPERTY_READ("Width", "i", Paint_Width),
	GB_STATIC_PROPERTY_READ("Height", "i", Paint_Height),
	GB_STATIC_PROPERTY_READ("ResolutionX", "i", Paint_ResolutionX),
	GB_STATIC_PROPERTY_READ("ResolutionY", "i", Paint_ResolutionY),

	GB_STATIC_METHOD("Save", NULL, Paint_Save, NULL),
	GB_STATIC_METHOD("Restore", NULL, Paint_Restore, NULL),
	
	GB_STATIC_METHOD("Clip", NULL, Paint_Clip, "[(Preserve)b]"),
	GB_STATIC_METHOD("ResetClip", NULL, Paint_ResetClip, NULL),
	GB_STATIC_PROPERTY_READ("ClipExtents", "PaintExtents", Paint_ClipExtents),
	
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
	
	GB_STATIC_PROPERTY("Brush", "PaintBrush", Paint_Brush),
	GB_STATIC_PROPERTY("Dash", "Float[]", Paint_Dash),
	GB_STATIC_PROPERTY("DashOffset", "f", Paint_DashOffset),
	GB_STATIC_PROPERTY("FillRule", "i", Paint_FillRule),
	GB_STATIC_PROPERTY("LineCap", "i", Paint_LineCap),
	GB_STATIC_PROPERTY("LineJoin", "i", Paint_LineJoin),
	GB_STATIC_PROPERTY("LineWidth", "f", Paint_LineWidth),
	GB_STATIC_PROPERTY("MiterLimit", "f", Paint_MiterLimit),
	GB_STATIC_PROPERTY("Operator", "i", Paint_Operator),
	//GB_STATIC_PROPERTY("Tolerance", "f", CAIRO_tolerance),
	
	GB_STATIC_METHOD("NewPath", NULL, Paint_NewPath, NULL),
	GB_STATIC_METHOD("ClosePath", NULL, Paint_ClosePath, NULL),
	
	GB_STATIC_PROPERTY_READ("X", "f", Paint_X),
	GB_STATIC_PROPERTY_READ("Y", "f", Paint_Y),

	GB_STATIC_METHOD("Rectangle", NULL, Paint_Rectangle, "(X)f(Y)f(Width)f(Height)f"),
	GB_STATIC_METHOD("Arc", NULL, Paint_Arc, "(XC)f(YC)f(Radius)f[(Angle)f(Length)f]"),

	GB_STATIC_METHOD("CurveTo", NULL, Paint_CurveTo, "(X1)f(Y1)f(X2)f(Y2)f(X3)f(Y3)f"),
	GB_STATIC_METHOD("RelCurveTo", NULL, Paint_RelCurveTo, "(X1)f(Y1)f(X2)f(Y2)f(X3)f(Y3)f"),
	GB_STATIC_METHOD("LineTo", NULL, Paint_LineTo, "(X)f(Y)f"),
	GB_STATIC_METHOD("RelLineTo", NULL, Paint_RelLineTo, "(X)f(Y)f"),
	GB_STATIC_METHOD("MoveTo", NULL, Paint_MoveTo, "(X)f(Y)f"),
	GB_STATIC_METHOD("RelMoveTo", NULL, Paint_RelMoveTo, "(X)f(Y)f"),
	GB_STATIC_PROPERTY("Font", "Font", Paint_Font),
	GB_STATIC_METHOD("Text", NULL, Paint_Text, "(Text)s[(X)f(Y)f(Width)f(Height)f(Alignment)i)]"),
	//GB_STATIC_METHOD("RichText", NULL, Paint_RichText, "(Text)s[(X)f(Y)f(Width)f(Height)f(Alignment)i)]"),
	//GB_STATIC_METHOD("TextExtents", "TextExtents", Paint_TextExtents, "(Text)s"),
	
	GB_STATIC_METHOD("Color", "PaintBrush", Paint_Color, "(Color)i"),
	GB_STATIC_METHOD("Image", "PaintBrush", Paint_Image, "(Image)Image;[(X)f(Y)f]"),
	GB_STATIC_METHOD("LinearGradient", "PaintBrush", Paint_LinearGradient, "(X0)f(Y0)f(X1)f(Y1)f(Colors)Integer[];(Positions)Float[];[(Extend)i]"),
	GB_STATIC_METHOD("RadialGradient", "PaintBrush", Paint_RadialGradient, "(CX)f(CY)f(Radius)f(FX)f(FY)f(Colors)Integer[];(Positions)Float[];[(Extend)i]"),

	GB_STATIC_PROPERTY("Matrix", "PaintMatrix", Paint_Matrix),

	GB_STATIC_METHOD("Reset", NULL, Paint_Reset, NULL),
	GB_STATIC_METHOD("Translate", NULL, Paint_Translate, "(TX)f(TY)f"),
	GB_STATIC_METHOD("Scale", NULL, Paint_Scale, "(SX)f(SY)f"),
	GB_STATIC_METHOD("Rotate", NULL, Paint_Rotate, "(Angle)f"),

	GB_END_DECLARE
};


