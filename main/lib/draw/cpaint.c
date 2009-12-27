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

#define THIS_EXTENTS ((PAINT_EXTENTS *)_object)
#define THIS_BRUSH ((PAINT_BRUSH *)_object)
#define THIS_MATRIX ((PAINT_MATRIX *)_object)

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

PUBLIC GB_PAINT *PAINT_get_current()
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

	GB.Alloc(POINTER(&paint), sizeof(GB_PAINT) + desc->size);
	paint->desc = desc;
	paint->previous = _current;
	GB.Ref(device);
	paint->device = device;
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
	
	PAINT->End(paint);
	
	GB.Unref(POINTER(&paint->device));
	GB.Free(POINTER(&paint));
}


/**** PaintExtents *********************************************************/

#define IMPLEMENT_EXTENTS_PROPERTY(_method, _field) \
BEGIN_PROPERTY(_method) \
	GB.ReturnFloat(THIS_EXTENTS->ext._field); \
END_PROPERTY

IMPLEMENT_EXTENTS_PROPERTY(PaintExtents_X, x1)
IMPLEMENT_EXTENTS_PROPERTY(PaintExtents_Y, y1)
IMPLEMENT_EXTENTS_PROPERTY(PaintExtents_X2, x2)
IMPLEMENT_EXTENTS_PROPERTY(PaintExtents_Y2, y2)

BEGIN_PROPERTY(PaintExtents_Width)

	GB.ReturnFloat(THIS_EXTENTS->ext.x2 - THIS_EXTENTS->ext.x1);

END_PROPERTY

BEGIN_PROPERTY(PaintExtents_Height)

	GB.ReturnFloat(THIS_EXTENTS->ext.y2 - THIS_EXTENTS->ext.y1);

END_PROPERTY

BEGIN_METHOD(PaintExtents_Merge, GB_OBJECT extents)

	PAINT_EXTENTS *extents = VARG(extents);
	
	if (GB.CheckObject(extents))
		return;
		
	if (extents->ext.x1 < THIS_EXTENTS->ext.x1) THIS_EXTENTS->ext.x1 = extents->ext.x1;
	if (extents->ext.y1 < THIS_EXTENTS->ext.y1) THIS_EXTENTS->ext.y1 = extents->ext.y1;
	if (extents->ext.x2 > THIS_EXTENTS->ext.x2) THIS_EXTENTS->ext.x2 = extents->ext.x2;
	if (extents->ext.y2 > THIS_EXTENTS->ext.y2) THIS_EXTENTS->ext.y2 = extents->ext.y2;

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

static GB_PAINT_DESC *handle_matrix(void *_object, bool set, GB_TRANSFORM *pmatrix)
{
	if (!_object)
	{
		PAINT->Matrix(THIS, set, pmatrix);
		return PAINT;
	}
	else
	{
		THIS_BRUSH->desc->Brush.Matrix(THIS_BRUSH->brush, set, pmatrix);
		return THIS_BRUSH->desc;
	}
}

#define IMPLEMENT_MATRIX_METHOD_VOID(_method, _code) \
BEGIN_METHOD_VOID(_method) \
	GB_TRANSFORM matrix; \
	handle_matrix(_object, FALSE, &matrix)->Transform._code; \
	handle_matrix(_object, TRUE, &matrix); \
	RETURN_SELF(); \
END_METHOD

#define IMPLEMENT_MATRIX_METHOD(_method, _arg, _code) \
BEGIN_METHOD(_method, _arg) \
	GB_TRANSFORM matrix; \
	handle_matrix(_object, FALSE, &matrix)->Transform._code; \
	handle_matrix(_object, TRUE, &matrix); \
	RETURN_SELF(); \
END_METHOD

IMPLEMENT_MATRIX_METHOD_VOID(PaintMatrix_Reset, Init(matrix, 1, 0, 0, 1, 0, 0))
IMPLEMENT_MATRIX_METHOD(PaintMatrix_Translate, GB_FLOAT tx; GB_FLOAT ty, Translate(matrix, VARG(tx), VARG(ty)))
IMPLEMENT_MATRIX_METHOD(PaintMatrix_Scale, GB_FLOAT sx; GB_FLOAT sy, Scale(matrix, VARG(sx), VARG(sy)))
IMPLEMENT_MATRIX_METHOD(PaintMatrix_Rotate, GB_FLOAT angle, Rotate(matrix, VARG(angle)))

BEGIN_METHOD_VOID(PaintMatrix_Invert)
	
	GB_TRANSFORM matrix;
	if (handle_matrix(_object, FALSE, &matrix)->Transform.Invert(matrix))
	{
		GB.ReturnNull();
		return;
	}
	handle_matrix(_object, TRUE, &matrix);
	RETURN_SELF();
	
END_METHOD

BEGIN_METHOD(PaintMatrix_Multiply, GB_OBJECT matrix2)

	GB_TRANSFORM matrix;
	PAINT_MATRIX *matrix2 = (PAINT_MATRIX *)VARG(matrix2);
	
	if (GB.CheckObject(matrix2))
		return;
	
	handle_matrix(_object, FALSE, &matrix)->Transform.Multiply(matrix, matrix2->matrix);
	handle_matrix(_object, TRUE, &matrix);
	RETURN_SELF();

END_METHOD


GB_DESC PaintMatrixDesc[] = 
{
	GB_DECLARE(".PaintMatrix", 0), GB_VIRTUAL_CLASS(),

	//GB_METHOD("_new", NULL, PaintMatrix_new, "[(XX)f(YX)f(XY)f(YY)f(X0)f(Y0)f]"),
	//GB_STATIC_METHOD("_call", "PaintMatrix", PaintMatrix_call, "[(XX)f(YX)f(XY)f(YY)f(X0)f(Y0)f]"),
	GB_METHOD("Reset", ".PaintMatrix", PaintMatrix_Reset, NULL),
	GB_METHOD("Translate", ".PaintMatrix", PaintMatrix_Translate, "(TX)f(TY)f"),
	GB_METHOD("Scale", ".PaintMatrix", PaintMatrix_Scale, "(SX)f(SY)f"),
	GB_METHOD("Rotate", ".PaintMatrix", PaintMatrix_Rotate, "(Angle)f"),
	GB_METHOD("Invert", ".PaintMatrix", PaintMatrix_Invert, NULL),
	GB_METHOD("Multiply", ".PaintMatrix", PaintMatrix_Multiply, "(Matrix)PaintMatrix;"),	

	GB_END_DECLARE	
};


/**** PaintBrush ***********************************************************/

BEGIN_METHOD_VOID(PaintBrush_free)

	THIS_BRUSH->desc->Brush.Free(THIS_BRUSH->brush);

END_METHOD

GB_DESC PaintBrushDesc[] = 
{
	GB_DECLARE("PaintBrush", sizeof(PAINT_BRUSH)), GB_NOT_CREATABLE(),

	GB_METHOD("_free", NULL, PaintBrush_free, NULL),
	
	GB_PROPERTY_SELF("Matrix", ".PaintMatrix"),

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

BEGIN_PROPERTY(Paint_Resolution)

	CHECK_DEVICE();
	GB.ReturnInteger(THIS->resolution);

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
IMPLEMENT_PROPERTY_INTEGER(Paint_FillRule, FillRule)
IMPLEMENT_PROPERTY_INTEGER(Paint_LineCap, LineCap)
IMPLEMENT_PROPERTY_INTEGER(Paint_LineJoin, LineJoin)
IMPLEMENT_PROPERTY_INTEGER(Paint_Operator, Operator)
IMPLEMENT_PROPERTY_FLOAT(Paint_LineWidth, LineWidth)
IMPLEMENT_PROPERTY_FLOAT(Paint_MiterLimit, MiterLimit)
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

BEGIN_METHOD(Paint_Arc, GB_FLOAT xc; GB_FLOAT yc; GB_FLOAT radius; GB_FLOAT angle1; GB_FLOAT angle2)

	CHECK_DEVICE();
	PAINT->Arc(THIS, VARG(xc), VARG(yc), VARG(radius), VARGOPT(angle1, 0.0), VARGOPT(angle2, M_PI * 2));

END_METHOD

BEGIN_METHOD(Paint_CurveTo, GB_FLOAT x1; GB_FLOAT y1; GB_FLOAT x2; GB_FLOAT y2; GB_FLOAT x3; GB_FLOAT y3)

	CHECK_DEVICE();
	PAINT->CurveTo(THIS, VARG(x1), VARG(y1), VARG(x2), VARG(y2), VARG(x3), VARG(y3));

END_METHOD

BEGIN_METHOD(Paint_LineTo, GB_FLOAT x; GB_FLOAT y)

	CHECK_DEVICE();
	PAINT->LineTo(THIS, VARG(x), VARG(y));

END_METHOD

BEGIN_METHOD(Paint_MoveTo, GB_FLOAT x; GB_FLOAT y)

	CHECK_DEVICE();
	PAINT->MoveTo(THIS, VARG(x), VARG(y));

END_METHOD

BEGIN_METHOD(Paint_Rectangle, GB_FLOAT x; GB_FLOAT y; GB_FLOAT w; GB_FLOAT h)

	CHECK_DEVICE();
	PAINT->Rectangle(THIS, VARG(x), VARG(y), VARG(w), VARG(h));

END_METHOD

IMPLEMENT_PROPERTY(Paint_Font, Font, GB_FONT, GB_OBJECT, GB.ReturnObject)

BEGIN_METHOD(Paint_Text, GB_STRING text; GB_FLOAT x; GB_FLOAT y; GB_FLOAT w; GB_FLOAT h; GB_INTEGER align)

	CHECK_DEVICE();
	
	if (MISSING(x) || MISSING(y))
	{
		PAINT->Text(THIS, STRING(text), LENGTH(text));
		return;
	}
	
	if (MISSING(w) || MISSING(h))
	{
		PAINT->MoveTo(THIS, (float)VARG(x), (float)VARG(y));
		PAINT->Text(THIS, STRING(text), LENGTH(text));
		return;
	}

	fprintf(stderr, "Paint.Text: Not yet implemented\n");

END_METHOD

BEGIN_METHOD(Paint_TextExtents, GB_STRING text)

	PAINT_EXTENTS *extents;
	
	CHECK_DEVICE();

	GB.New(POINTER(&extents), GB.FindClass("PaintExtents"), NULL, NULL);
	PAINT->TextExtents(THIS, STRING(text), LENGTH(text), &extents->ext);
	
	GB.ReturnObject(extents);

END_METHOD

static void make_brush(GB_BRUSH brush)
{
	PAINT_BRUSH *that;
	GB.New(POINTER(&that), GB.FindClass("PaintBrush"), NULL, NULL);
	that->brush = brush;
	GB.ReturnObject(that);
}

BEGIN_METHOD(Paint_Color, GB_INTEGER color)

	GB_BRUSH brush;

	CHECK_DEVICE();
	
	PAINT->Brush.Color(&brush, VARG(color));
	make_brush(brush);

END_METHOD

BEGIN_METHOD(Paint_Image, GB_OBJECT image; GB_FLOAT x; GB_FLOAT y; GB_INTEGER extend)

	GB_BRUSH brush;

	CHECK_DEVICE();
	
	if (GB.CheckObject(VARG(image)))
		return;
	
	PAINT->Brush.Image(&brush, (GB_IMAGE)VARG(image), (float)VARGOPT(x, 0), (float)VARGOPT(y, 0), VARGOPT(extend, GB_PAINT_EXTEND_PAD));
	make_brush(brush);

END_METHOD

static void handle_color_stop(GB_BRUSH brush, GB_ARRAY positions, GB_ARRAY colors)
{
	int nstop;
	
	nstop = Min(GB.Array.Count(positions), GB.Array.Count(colors));
	if (nstop)
		PAINT->Brush.SetColorStops(brush, nstop, (double *)GB.Array.Get(positions, 0), (GB_COLOR *)GB.Array.Get(colors, 0));
}

BEGIN_METHOD(Paint_LinearGradient, GB_FLOAT x0; GB_FLOAT y0; GB_FLOAT x1; GB_FLOAT y1; GB_OBJECT positions; GB_OBJECT colors)

	GB_BRUSH brush;
	GB_ARRAY positions, colors;
	
	positions = (GB_ARRAY)VARG(positions);
	if (GB.CheckObject(positions))
		return;
	colors = (GB_ARRAY)VARG(colors);
	if (GB.CheckObject(colors))
		return;
	
	PAINT->Brush.LinearGradient(&brush, (float)VARG(x0), (float)VARG(y0), (float)VARG(x1), (float)VARG(y1));
	handle_color_stop(brush, positions, colors);
	make_brush(brush);

END_METHOD

BEGIN_METHOD(Paint_RadialGradient, GB_FLOAT cx0; GB_FLOAT cy0; GB_FLOAT radius0; GB_FLOAT cx1; GB_FLOAT cy1; GB_FLOAT radius1; GB_OBJECT positions; GB_OBJECT colors)

	GB_BRUSH brush;
	GB_ARRAY positions, colors;
	
	positions = (GB_ARRAY)VARG(positions);
	if (GB.CheckObject(positions))
		return;
	colors = (GB_ARRAY)VARG(colors);
	if (GB.CheckObject(colors))
		return;
	
	PAINT->Brush.RadialGradient(&brush, (float)VARG(cx0), (float)VARG(cy0), (float)VARG(radius0), (float)VARG(cx1), (float)VARG(cy1), (float)VARG(radius1));
	handle_color_stop(brush, positions, colors);
	make_brush(brush);

END_METHOD


GB_DESC CPaintDesc[] =
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
	GB_STATIC_PROPERTY_READ("Resolution", "i", Paint_Resolution),

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
	//GB_STATIC_METHOD("PathContains", "b", Paint_InPath, "(X)f(Y)f"),
	
	//GB_STATIC_PROPERTY("Brush", "PaintBrush", Paint_Brush),
	//GB_STATIC_PROPERTY("Dash", "Float[]", Paint_Dash),
	GB_STATIC_PROPERTY("DashOffset", "f", Paint_DashOffset),
	GB_STATIC_PROPERTY("FillRule", "i", Paint_FillRule),
	GB_STATIC_PROPERTY("LineCap", "i", Paint_LineCap),
	GB_STATIC_PROPERTY("LineJoin", "i", Paint_LineJoin),
	GB_STATIC_PROPERTY("LineWidth", "f", Paint_LineWidth),
	GB_STATIC_PROPERTY("MiterLimit", "f", Paint_MiterLimit),
	GB_STATIC_PROPERTY("Operator", "i", Paint_Operator),
	//GB_STATIC_PROPERTY("Tolerance", "f", CAIRO_tolerance),
	
	GB_STATIC_METHOD("NewPath", NULL, Paint_NewPath, NULL),
	//GB_STATIC_METHOD("NewSubPath", NULL, CAIRO_new_sub_path, NULL),
	GB_STATIC_METHOD("ClosePath", NULL, Paint_ClosePath, NULL),
	
	GB_STATIC_PROPERTY_READ("X", "f", Paint_X),
	GB_STATIC_PROPERTY_READ("Y", "f", Paint_Y),
	GB_STATIC_METHOD("Arc", NULL, Paint_Arc, "(XC)f(YC)f(Radius)f[(Angle1)f(Angle2)f]"),
	//GB_STATIC_METHOD("ArcNegative", NULL, CAIRO_arc_negative, "(XC)f(YC)f(Radius)f[(Angle1)f(Angle2)f]"),
	GB_STATIC_METHOD("CurveTo", NULL, Paint_CurveTo, "(X1)f(Y1)f(X2)f(Y2)f(X3)f(Y3)f"),
	GB_STATIC_METHOD("LineTo", NULL, Paint_LineTo, "(X)f(Y)f"),
	GB_STATIC_METHOD("MoveTo", NULL, Paint_MoveTo, "(X)f(Y)f"),
	GB_STATIC_METHOD("Rectangle", NULL, Paint_Rectangle, "(X)f(Y)f(Width)f(Height)f"),

	GB_STATIC_PROPERTY("Font", "Font", Paint_Font),
	GB_STATIC_METHOD("Text", NULL, Paint_Text, "(Text)s[(X)f(Y)f(Width)f(Height)f(Alignment)i)]"),
	GB_STATIC_METHOD("TextExtents", "TextExtents", Paint_TextExtents, "(Text)s"),
	
	GB_STATIC_METHOD("Color", "PaintBrush", Paint_Color, "(Color)i"),
	GB_STATIC_METHOD("Image", "PaintBrush", Paint_Image, "(Image)Image;[(X)f(Y)f(Extend)i]"),
	GB_STATIC_METHOD("LinearGradient", "PaintBrush", Paint_LinearGradient, "(X0)f(Y0)f(X1)f(Y1)f(Positions)Float[];(Colors)Integer[];"),
	GB_STATIC_METHOD("RadialGradient", "PaintBrush", Paint_RadialGradient, "(CX0)f(CY0)f(Radius0)f(CX1)f(CY1)f(Radius1)f(Positions)Float[];(Colors)Integer[];"),

	GB_STATIC_PROPERTY_SELF("Matrix", ".PaintMatrix"),

	GB_END_DECLARE
};


