/***************************************************************************

	CMouse.cpp

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

#define __CMOUSE_CPP

#include "CMouse.h"

//-------------------------------------------------------------------------

BEGIN_METHOD(CCURSOR_new, GB_OBJECT picture; GB_INTEGER x; GB_INTEGER y)

	CPICTURE *Pic=(CPICTURE*)VARG(picture);
	gPicture *pic=NULL;
	long X=0,Y=0;
	
	if (!MISSING(x)) X=VARG(x);
	if (!MISSING(y)) Y=VARG(y); 
	if (Pic) pic=Pic->picture;
	
	THIS->cur=new gCursor(pic,X,Y);

END_METHOD


BEGIN_METHOD_VOID(CCURSOR_delete)

	delete THIS->cur;

END_METHOD


BEGIN_PROPERTY(CCURSOR_x)

	GB.ReturnInteger(THIS->cur->left());

END_PROPERTY


BEGIN_PROPERTY(CCURSOR_y)

	GB.ReturnInteger(THIS->cur->top());

END_PROPERTY


//-------------------------------------------------------------------------

BEGIN_PROPERTY(Mouse_ScreenX)

	GB.ReturnInteger(gMouse::screenX());

END_PROPERTY


BEGIN_PROPERTY(Mouse_ScreenY)

	GB.ReturnInteger(gMouse::screenY());

END_PROPERTY


BEGIN_METHOD(Mouse_Move, GB_INTEGER x; GB_INTEGER y)

	gMouse::move(VARG(x),VARG(y));

END_PROPERTY

#define CHECK_VALID() \
	if (!gMouse::isValid()) \
	{ \
		GB.Error("No mouse event data"); \
		return; \
	}

BEGIN_PROPERTY(Mouse_X)

	CHECK_VALID();
	GB.ReturnInteger(gMouse::x());

END_PROPERTY

BEGIN_PROPERTY(Mouse_Y)

	CHECK_VALID();
	GB.ReturnInteger(gMouse::y());

END_PROPERTY

BEGIN_PROPERTY(Mouse_StartX)

	CHECK_VALID();
	GB.ReturnInteger(gMouse::startX());

END_PROPERTY

BEGIN_PROPERTY(Mouse_StartY)

	CHECK_VALID();
	GB.ReturnInteger(gMouse::startY());

END_PROPERTY

BEGIN_PROPERTY(Mouse_Button)
	
	CHECK_VALID();
	GB.ReturnInteger(gMouse::button());

END_PROPERTY

BEGIN_PROPERTY(Mouse_State)
	
	CHECK_VALID();
	GB.ReturnInteger(gMouse::state());

END_PROPERTY

BEGIN_PROPERTY(Mouse_Left)

	CHECK_VALID();
	GB.ReturnBoolean(gMouse::left());

END_PROPERTY

BEGIN_PROPERTY(Mouse_Right)

	CHECK_VALID();
	GB.ReturnBoolean(gMouse::right());

END_PROPERTY

BEGIN_PROPERTY(Mouse_Middle)

	CHECK_VALID();
	GB.ReturnBoolean(gMouse::middle());

END_PROPERTY

BEGIN_PROPERTY(Mouse_Shift)

	CHECK_VALID();
	GB.ReturnBoolean(gMouse::shift());

END_PROPERTY

BEGIN_PROPERTY(Mouse_Control)

	CHECK_VALID();
	GB.ReturnBoolean(gMouse::control());

END_PROPERTY

BEGIN_PROPERTY(Mouse_Alt)

	CHECK_VALID();
	GB.ReturnBoolean(gMouse::alt());

END_PROPERTY

BEGIN_PROPERTY(Mouse_Meta)

	CHECK_VALID();
	GB.ReturnBoolean(gMouse::meta());

END_PROPERTY

BEGIN_PROPERTY(Mouse_Normal)

	CHECK_VALID();
	GB.ReturnBoolean(gMouse::normal());

END_PROPERTY

BEGIN_PROPERTY(Mouse_Delta)

	CHECK_VALID();
	GB.ReturnInteger(gMouse::delta());

END_PROPERTY

BEGIN_PROPERTY(Mouse_Orientation)

	CHECK_VALID();
	GB.ReturnInteger(gMouse::orientation());

END_PROPERTY

BEGIN_PROPERTY(Mouse_Forward)

	CHECK_VALID();
	GB.ReturnBoolean(gMouse::delta() > 0);

END_PROPERTY

BEGIN_METHOD(Mouse_Inside, GB_OBJECT control)
	
	CWIDGET *control = (CWIDGET *)VARG(control);
	gControl *widget;
	int x, y, xw, yw;
	
	if (GB.CheckObject(control))
		return;
	
	widget = control->widget;
	
	if (!widget->isVisible())
	{
		GB.ReturnBoolean(false);
		return;
	}
	
	gMouse::getScreenPos(&x, &y);
	widget->getScreenPos(&xw, &yw);
	x -= xw;
	y -= yw;
	GB.ReturnBoolean(x >= 0 && x < widget->width() && y >= 0 && y < widget->height());

END_METHOD

BEGIN_METHOD(Mouse_Translate, GB_INTEGER dx; GB_INTEGER dy)

	CHECK_VALID();
	gMouse::translate(VARG(dx), VARG(dy));

END_METHOD

//-------------------------------------------------------------------------

BEGIN_PROPERTY(Pointer_ScreenX)

	CHECK_VALID();
	GB.ReturnFloat(gMouse::getPointerScreenX());

END_PROPERTY

BEGIN_PROPERTY(Pointer_ScreenY)

	CHECK_VALID();
	GB.ReturnFloat(gMouse::getPointerScreenY());

END_PROPERTY

BEGIN_PROPERTY(Pointer_X)

	CHECK_VALID();
	GB.ReturnFloat(gMouse::getPointerX());

END_PROPERTY

BEGIN_PROPERTY(Pointer_Y)

	CHECK_VALID();
	GB.ReturnFloat(gMouse::getPointerY());

END_PROPERTY

BEGIN_PROPERTY(Pointer_XTilt)

	CHECK_VALID();
	GB.ReturnFloat(gMouse::getAxis(GDK_AXIS_XTILT));

END_PROPERTY

BEGIN_PROPERTY(Pointer_YTilt)

	CHECK_VALID();
	GB.ReturnFloat(gMouse::getAxis(GDK_AXIS_YTILT));

END_PROPERTY

BEGIN_PROPERTY(Pointer_Pressure)

	CHECK_VALID();
	GB.ReturnFloat(gMouse::getAxis(GDK_AXIS_PRESSURE));

END_PROPERTY

BEGIN_PROPERTY(Pointer_Rotation)

	CHECK_VALID();
	GB.ReturnFloat(gMouse::getAxis(GDK_AXIS_WHEEL));

END_PROPERTY

BEGIN_PROPERTY(Pointer_Type)

	CHECK_VALID();
	GB.ReturnInteger(gMouse::getType());

END_PROPERTY

//-------------------------------------------------------------------------

GB_DESC CCursorDesc[] =
{
	GB_DECLARE("Cursor", sizeof(CCURSOR)),

	GB_METHOD("_new", 0, CCURSOR_new, "(Picture)Picture;[(X)i(Y)i]"),
	GB_METHOD("_free", 0, CCURSOR_delete, NULL),

	GB_PROPERTY_READ("X", "i", CCURSOR_x),
	GB_PROPERTY_READ("Y", "i", CCURSOR_y),

	GB_END_DECLARE
};


GB_DESC CMouseDesc[] =
{
	GB_DECLARE_VIRTUAL("Mouse"),

	GB_STATIC_PROPERTY_READ("ScreenX", "i", Mouse_ScreenX),
	GB_STATIC_PROPERTY_READ("ScreenY", "i", Mouse_ScreenY),
	GB_STATIC_METHOD("Move", 0, Mouse_Move, "(X)i(Y)i"),
	GB_STATIC_METHOD("Inside", "b", Mouse_Inside, "(Control)Control"),

	GB_CONSTANT("Default", "i", CURSOR_DEFAULT),
	GB_CONSTANT("Custom", "i", CURSOR_CUSTOM),
	GB_CONSTANT("Blank", "i", GDK_BLANK_CURSOR),
	GB_CONSTANT("Arrow", "i", GDK_LEFT_PTR),
	GB_CONSTANT("Cross", "i", GDK_CROSSHAIR),
	GB_CONSTANT("Wait", "i", GDK_WATCH),
	GB_CONSTANT("Text", "i", GDK_XTERM),
	GB_CONSTANT("SizeAll", "i", GDK_FLEUR),
	GB_CONSTANT("SizeH", "i", GDK_SB_H_DOUBLE_ARROW),
	GB_CONSTANT("SizeV", "i", GDK_SB_V_DOUBLE_ARROW),
	GB_CONSTANT("SizeN", "i", GDK_TOP_SIDE),
	GB_CONSTANT("SizeS", "i", GDK_BOTTOM_SIDE),
	GB_CONSTANT("SizeW", "i", GDK_LEFT_SIDE),
	GB_CONSTANT("SizeE", "i", GDK_RIGHT_SIDE),
	//GB_CONSTANT("SizeNW", "i", GDK_LAST_CURSOR+1), //FDiag
	GB_CONSTANT("SizeNW", "i", GDK_TOP_LEFT_CORNER), //FDiag
	//GB_CONSTANT("SizeSE", "i", GDK_LAST_CURSOR+1),
	GB_CONSTANT("SizeSE", "i", GDK_BOTTOM_RIGHT_CORNER),
	//GB_CONSTANT("SizeNE", "i", GDK_LAST_CURSOR+2), //BDiag
	GB_CONSTANT("SizeNE", "i", GDK_TOP_RIGHT_CORNER), //BDiag
	//GB_CONSTANT("SizeSW", "i", GDK_LAST_CURSOR+2),
	GB_CONSTANT("SizeSW", "i", GDK_BOTTOM_LEFT_CORNER),
	GB_CONSTANT("SizeNWSE", "i", GDK_LAST_CURSOR+1),
	GB_CONSTANT("SizeNESW", "i", GDK_LAST_CURSOR+2),
	//GB_CONSTANT("SplitH", "i", GDK_LAST_CURSOR+3), // SplitH
	//GB_CONSTANT("SplitV", "i", GDK_LAST_CURSOR+4), // SplitV
	GB_CONSTANT("SplitH", "i", GDK_SB_H_DOUBLE_ARROW), // SplitH
	GB_CONSTANT("SplitV", "i", GDK_SB_V_DOUBLE_ARROW), // SplitV
	GB_CONSTANT("Pointing", "i", GDK_HAND2),

	GB_CONSTANT("Horizontal", "i", 0),
	GB_CONSTANT("Vertical", "i", 1),

	GB_STATIC_PROPERTY_READ("X", "i", Mouse_X),
	GB_STATIC_PROPERTY_READ("Y", "i", Mouse_Y),
	GB_STATIC_PROPERTY_READ("StartX", "i", Mouse_StartX),
	GB_STATIC_PROPERTY_READ("StartY", "i", Mouse_StartY),
	GB_STATIC_PROPERTY_READ("Left", "b", Mouse_Left),
	GB_STATIC_PROPERTY_READ("Right", "b", Mouse_Right),
	GB_STATIC_PROPERTY_READ("Middle", "b", Mouse_Middle),
	GB_STATIC_PROPERTY_READ("Button", "i", Mouse_Button),
	GB_STATIC_PROPERTY_READ("State", "i", Mouse_State),
	GB_STATIC_PROPERTY_READ("Shift", "b", Mouse_Shift),
	GB_STATIC_PROPERTY_READ("Control", "b", Mouse_Control),
	GB_STATIC_PROPERTY_READ("Alt", "b", Mouse_Alt),
	GB_STATIC_PROPERTY_READ("Meta", "b", Mouse_Meta),
	GB_STATIC_PROPERTY_READ("Normal", "b", Mouse_Normal),
	GB_STATIC_PROPERTY_READ("Orientation", "i", Mouse_Orientation),
	GB_STATIC_PROPERTY_READ("Delta", "f", Mouse_Delta),
	GB_STATIC_PROPERTY_READ("Forward", "b", Mouse_Forward),
	
	GB_STATIC_METHOD("Translate", NULL, Mouse_Translate, "(DX)i(DY)i"),

	GB_END_DECLARE
};
	
GB_DESC CPointerDesc[] =
{
	GB_DECLARE_VIRTUAL("Pointer"),
	
	GB_CONSTANT("Mouse", "i", POINTER_MOUSE),
	GB_CONSTANT("Pen", "i", POINTER_PEN),
	GB_CONSTANT("Eraser", "i", POINTER_ERASER),
	GB_CONSTANT("Cursor", "i", POINTER_CURSOR),
	
	GB_STATIC_PROPERTY_READ("Type", "i", Pointer_Type),
	GB_STATIC_PROPERTY_READ("X", "f", Pointer_X),
	GB_STATIC_PROPERTY_READ("Y", "f", Pointer_Y),
	GB_STATIC_PROPERTY_READ("ScreenX", "f", Pointer_ScreenX),
	GB_STATIC_PROPERTY_READ("ScreenY", "f", Pointer_ScreenY),
	GB_STATIC_PROPERTY_READ("XTilt", "f", Pointer_XTilt),
	GB_STATIC_PROPERTY_READ("YTilt", "f", Pointer_YTilt),
	GB_STATIC_PROPERTY_READ("Pressure", "f", Pointer_Pressure),
	GB_STATIC_PROPERTY_READ("Rotation", "f", Pointer_Rotation),
	
	GB_END_DECLARE
};

