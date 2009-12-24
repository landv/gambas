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
	
	if (paint->desc->Begin(paint))
		return TRUE;
	
	//DRAW->SetBackground(draw, GB_PAINT_COLOR_DEFAULT);
	//DRAW->SetForeground(draw, GB_PAINT_COLOR_DEFAULT);
	//DRAW->Fill.SetColor(draw, GB_PAINT_COLOR_DEFAULT);
	return FALSE;
}


BEGIN_METHOD(Paint_begin, GB_OBJECT device)

	void *device = VARG(device);

	if (GB.CheckObject(device))
		return;

	DRAW_begin(device);

END_METHOD


void DRAW_end()
{
	GB_PAINT *draw;

	if (!_current)
		return;
		
	draw = _current;
	_current = _current->previous;
	
	draw->desc->End(draw);
	
	GB.Unref(POINTER(&draw->device));
	GB.Free(POINTER(&draw));
}


BEGIN_METHOD_VOID(Paint_end)

	DRAW_end();
	
END_METHOD


BEGIN_METHOD_VOID(Paint_exit)

	while (_current)
		DRAW_end();

END_METHOD


BEGIN_METHOD_VOID(Paint_save)

	CHECK_DEVICE();
	DRAW->Save(THIS);
	
END_METHOD


BEGIN_METHOD_VOID(Paint_restore)

	CHECK_DEVICE();
	DRAW->Restore(THIS);
	
END_METHOD

BEGIN_PROPERTY(Paint_device)
	
	CHECK_DEVICE();
	GB.ReturnObject(THIS->device);

END_PROPERTY

BEGIN_PROPERTY(Paint_width)

	CHECK_DEVICE();
	GB.ReturnInteger(THIS->width);

END_PROPERTY


BEGIN_PROPERTY(Paint_height)

	CHECK_DEVICE();
	GB.ReturnInteger(THIS->height);

END_PROPERTY


BEGIN_PROPERTY(Paint_resolution)

	CHECK_DEVICE();
	GB.ReturnInteger(THIS->resolution);

END_PROPERTY


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

GB_DESC PaintBrushDesc[] = 
{
	GB_DECLARE("PaintBrush", sizeof(PAINT_BRUSH)), GB_NOT_CREATABLE(),

	GB_METHOD("_free", NULL, PaintBrush_free, NULL),
	
	GB_PROPERTY("Matrix", "PaintMatrix", PaintBrush_Matrix),

	GB_END_DECLARE
};

GB_DESC PaintMatrixDesc[] = 
{
	GB_DECLARE("PaintMatrix", sizeof(PAINT_MATRIX)),

	GB_METHOD("_new", NULL, PaintMatrix_new, "[(XX)f(YX)f(XY)f(YY)f(X0)f(Y0)f]"),
	GB_STATIC_METHOD("_call", "PaintMatrix", PaintMatrix_call, "[(XX)f(YX)f(XY)f(YY)f(X0)f(Y0)f]"),
	GB_METHOD("Translate", "PaintMatrix", PaintMatrix_translate, "(TX)f(TY)f"),
	GB_METHOD("Scale", "PaintMatrix", PaintMatrix_scale, "(SX)f(SY)f"),
	GB_METHOD("Rotate", "PaintMatrix", PaintMatrix_rotate, "(Angle)f"),
	GB_METHOD("Invert", "PaintMatrix", PaintMatrix_invert, NULL),
	GB_METHOD("Multiply", "PaintMatrix", PaintMatrix_multiply, "(Matrix)PaintMatrix;"),	

	GB_END_DECLARE	
};


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
	
	GB_STATIC_PROPERTY_READ("Device", "o", Paint_device),

	GB_STATIC_METHOD("Save", NULL, Paint_save, NULL),
	GB_STATIC_METHOD("Restore", NULL, Paint_restore, NULL),
	
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
	GB_STATIC_METHOD("PathContains", "b", Paint_InPath, "(X)f(Y)f"),
	
	GB_STATIC_PROPERTY("Brush", "PaintBrush", Paint_Brush),
	//GB_STATIC_PROPERTY("AntiAlias", "i", CAIRO_anti_alias),
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
	//GB_STATIC_METHOD("NewSubPath", NULL, CAIRO_new_sub_path, NULL),
	GB_STATIC_METHOD("ClosePath", NULL, Paint_ClosePath, NULL),
	
	GB_STATIC_METHOD("Arc", NULL, Paint_Arc, "(XC)f(YC)f(Radius)f[(Angle1)f(Angle2)f]"),
	//GB_STATIC_METHOD("ArcNegative", NULL, CAIRO_arc_negative, "(XC)f(YC)f(Radius)f[(Angle1)f(Angle2)f]"),
	GB_STATIC_METHOD("CurveTo", NULL, Paint_CurveTo, "(X1)f(Y1)f(X2)f(Y2)f(X3)f(Y3)f"),
	GB_STATIC_METHOD("LineTo", NULL, Paint_LineTo, "(X)f(Y)f"),
	GB_STATIC_METHOD("MoveTo", NULL, Paint_MoveTo, "(X)f(Y)f"),
	GB_STATIC_METHOD("Rectangle", NULL, Paint_Rectangle, "(X)f(Y)f(Width)f(Height)f"),

	GB_STATIC_PROPERTY("Font", "PaintFont", Paint_Font),
	GB_STATIC_METHOD("Text", NULL, Paint_Text, "(Text)s(X)f(Y)f[(Width)f(Height)f(Alignment)i)]"),
	GB_STATIC_METHOD("TextExtents", "TextExtents", Paint_TextExtents, "(Text)s"),
	//GB_STATIC_METHOD("RelCurveTo", NULL, CAIRO_rel_curve_to, "(DX1)f(DY1)f(DX2)f(DY2)f(DX3)f(DY3)f"),
	//GB_STATIC_METHOD("RelLineTo", NULL, CAIRO_rel_line_to, "(DX)f(DY)f"),
	//GB_STATIC_METHOD("RelMoveTo", NULL, CAIRO_rel_move_to, "(DX)f(DY)f"),
	
	GB_STATIC_METHOD("ColorBrush", "PaintBrush", Paint_ColorBrush, "(Color)i"),
	GB_STATIC_METHOD("ImageBrush", "PaintBrush", Paint_ImageBrush, "(Image)Image;[(X)f(Y)f(Extend)i]"),
	GB_STATIC_METHOD("LinearGradient", "PaintBrush", Paint_LinearGradient, "(X0)f(Y0)f(X1)f(Y1)f(Colors)Float[][];"),
	GB_STATIC_METHOD("RadialGradient", "PaintBrush", Paint_RadialGradient, "(CX0)f(CY0)f(Radius0)f(CX1)f(CY1)f(Radius1)f(Colors)Float[][];"),

	GB_STATIC_METHOD("Translate", NULL, Paint_Translate, "(TX)f(TY)f"),
	GB_STATIC_METHOD("Scale", NULL, Paint_Scale, "(SX)f(SY)f"),
	GB_STATIC_METHOD("Rotate", NULL, Paint_Rotate, "(Angle)f"),
	GB_STATIC_PROPERTY("Matrix", "PaintMatrix", Paint_Matrix),

	GB_END_DECLARE
};


