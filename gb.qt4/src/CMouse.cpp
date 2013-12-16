/***************************************************************************

	CMouse.cpp

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

#define __CMOUSE_CPP

#include <qapplication.h>
#include <qpixmap.h>
#include <qcursor.h>
#include <qnamespace.h>

#include "gambas.h"
#include "main.h"

#include "gb.form.const.h"
#include "CWidget.h"
#include "CPicture.h"
#include "CMouse.h"

MOUSE_INFO MOUSE_info = { 0 };
POINTER_INFO POINTER_info = { 0 };
static int _dx = 0;
static int _dy = 0;

void CMOUSE_clear(int valid)
{
	if (valid)
		MOUSE_info.valid++;
	else
		MOUSE_info.valid--;

	if (MOUSE_info.valid == 0)
		CLEAR(&POINTER_info);
}

void CMOUSE_reset_translate()
{
	_dx = _dy = 0;
}

//int CMOUSE_last_state = 0;

//static CCURSOR PredefinedCursor[LastCursor + 1] = { { { 0, 0 }, NULL, NULL } };
//static int MouseClassID;

#if 0
static int translate_state(int s)
{
	int bst = 0;

	if (s & Button1Mask)
		bst |= Qt::LeftButton;
	if ( s & Button2Mask)
		bst |= Qt::MidButton;
	if ( s & Button3Mask)
		bst |= Qt::RightButton;
	if ( s & ShiftMask)
		bst |= Qt::ShiftModifier;
	if ( s & ControlMask)
		bst |= Qt::ControlModifier;
	if ( s & qt_alt_mask)
		bst |= Qt::AltModifier;
	if ( s & qt_meta_mask)
		bst |= Qt::MetaModifier;

	return bst;
}

static int get_state()
{
	Window root;
	Window child;
	int root_x, root_y, win_x, win_y;
	uint state;
	Display* dpy = QPaintDevice::x11AppDisplay();

	for (int i = 0; i < ScreenCount(dpy); i++)
	{
		if (XQueryPointer(dpy, RootWindow(dpy, i), &root, &child,
					&root_x, &root_y, &win_x, &win_y, &state))
			return translate_state(state);
	}

	return 0;
}
#endif

//-------------------------------------------------------------------------

BEGIN_METHOD(Cursor_new, GB_OBJECT picture; GB_INTEGER x; GB_INTEGER y)

	CPICTURE *pict = (CPICTURE *)VARG(picture);

	THIS->x = VARGOPT(x, -1);
	THIS->y = VARGOPT(y, -1);

	//GB.StoreObject(ARG(picture), POINTER(&THIS->picture));
	if (GB.CheckObject(pict))
		return;
	
	if (THIS->x < 0 || THIS->x >= pict->pixmap->width())
		THIS->x = -1;
		
	if (THIS->y < 0 || THIS->y >= pict->pixmap->height())
		THIS->y = -1;
		
	THIS->cursor = new QCursor(*(pict->pixmap), THIS->x, THIS->y);

END_METHOD


BEGIN_METHOD_VOID(Cursor_Delete)

	//GB.Unref(POINTER(&THIS->picture));
	delete THIS->cursor;

END_METHOD


/*BEGIN_PROPERTY(CCURSOR_picture)

	GB.ReturnObject(THIS->picture);

END_PROPERTY*/


BEGIN_PROPERTY(Cursor_X)

	GB.ReturnInteger(THIS->x);

END_PROPERTY


BEGIN_PROPERTY(Cursor_Y)

	GB.ReturnInteger(THIS->y);

END_PROPERTY


// BEGIN_METHOD(CCURSOR_get, int shape)
//
//   int shape = PARAM(shape);
//   CCURSOR *p;
//
//   if (shape < 0 || shape > LastCursor)
//     GB.ReturnObject(NULL);
//
//   p = &PredefinedCursor[shape];
//   if (p->ob.klass == 0)
//   {
//     p->ob.klass = MouseClassID;
//     p->cursor = new QCursor(shape);
//     GB.Ref(p);
//   }
//
//   GB.ReturnObject(p);
//
// END_METHOD


//-------------------------------------------------------------------------

BEGIN_PROPERTY(Mouse_ScreenX)

	GB.ReturnInteger(MOUSE_info.valid ? MOUSE_info.screenX : QCursor::pos().x());

END_PROPERTY


BEGIN_PROPERTY(Mouse_ScreenY)

	GB.ReturnInteger(MOUSE_info.valid ? MOUSE_info.screenY : QCursor::pos().y());

END_PROPERTY


BEGIN_METHOD(Mouse_Move, GB_INTEGER x; GB_INTEGER y)

	QCursor::setPos(VARG(x), VARG(y));

END_PROPERTY

#define CHECK_VALID() \
	if (MOUSE_info.valid == 0) \
	{ \
		GB.Error("No mouse event data"); \
		return; \
	}

BEGIN_PROPERTY(Mouse_X)

	CHECK_VALID();
	GB.ReturnInteger(MOUSE_info.x + _dx);

END_PROPERTY

BEGIN_PROPERTY(Mouse_Y)

	CHECK_VALID();
	GB.ReturnInteger(MOUSE_info.y + _dy);

END_PROPERTY

BEGIN_PROPERTY(Mouse_StartX)

	CHECK_VALID();
	GB.ReturnInteger(MOUSE_info.sx + _dx);

END_PROPERTY

BEGIN_PROPERTY(Mouse_StartY)

	CHECK_VALID();
	GB.ReturnInteger(MOUSE_info.sy + _dy);

END_PROPERTY

BEGIN_PROPERTY(Mouse_Button)

	CHECK_VALID();
	GB.ReturnInteger((int)MOUSE_info.button);

END_PROPERTY

BEGIN_PROPERTY(Mouse_State)

	CHECK_VALID();
	GB.ReturnInteger((int)MOUSE_info.state);

END_PROPERTY

BEGIN_PROPERTY(Mouse_Left)

	CHECK_VALID();
	GB.ReturnBoolean((MOUSE_info.state | MOUSE_info.button) & Qt::LeftButton);

END_PROPERTY

BEGIN_PROPERTY(Mouse_Right)

	CHECK_VALID();
	GB.ReturnBoolean((MOUSE_info.state | MOUSE_info.button) & Qt::RightButton);

END_PROPERTY

BEGIN_PROPERTY(Mouse_Middle)

	CHECK_VALID();
	GB.ReturnBoolean((MOUSE_info.state | MOUSE_info.button) & Qt::MidButton);

END_PROPERTY

BEGIN_PROPERTY(Mouse_Shift)

	//CHECK_VALID();
	GB.ReturnBoolean(MOUSE_info.modifier & Qt::ShiftModifier);

END_PROPERTY

BEGIN_PROPERTY(Mouse_Control)

	//CHECK_VALID();
	GB.ReturnBoolean(MOUSE_info.modifier & Qt::ControlModifier);

END_PROPERTY

BEGIN_PROPERTY(Mouse_Alt)

	//CHECK_VALID();
	GB.ReturnBoolean(MOUSE_info.modifier & Qt::AltModifier);

END_PROPERTY

BEGIN_PROPERTY(Mouse_Meta)

	//CHECK_VALID();
	GB.ReturnBoolean(MOUSE_info.modifier & Qt::MetaModifier);

END_PROPERTY

BEGIN_PROPERTY(Mouse_Normal)

	//CHECK_VALID();
	GB.ReturnBoolean(MOUSE_info.modifier == Qt::NoModifier);

END_PROPERTY

BEGIN_PROPERTY(Mouse_Orientation)

	CHECK_VALID();
	GB.ReturnInteger(MOUSE_info.orientation);

END_PROPERTY

BEGIN_PROPERTY(Mouse_Delta)

	CHECK_VALID();
	GB.ReturnFloat((double)MOUSE_info.delta / 120);

END_PROPERTY

BEGIN_PROPERTY(Mouse_Forward)

	CHECK_VALID();
	GB.ReturnBoolean(MOUSE_info.delta > 0);

END_PROPERTY

BEGIN_METHOD(Mouse_Inside, GB_OBJECT control)
	
	CWIDGET *control = (CWIDGET *)VARG(control);
	QPoint pos;
	
	if (GB.CheckObject(control))
		return;
	
	if (!CWIDGET_is_visible(control))
	{
		GB.ReturnBoolean(false);
		return;
	}
		
	pos = QCursor::pos() - QWIDGET(control)->mapToGlobal(QPoint(0, 0));
	GB.ReturnBoolean(pos.x() >= 0 && pos.x() < QWIDGET(control)->width() && pos.y() >= 0 && pos.y() < QWIDGET(control)->height());

END_METHOD

BEGIN_METHOD(Mouse_Translate, GB_INTEGER dx; GB_INTEGER dy)

	CHECK_VALID();

	_dx = VARG(dx);
	_dy = VARG(dy);

END_METHOD

//-------------------------------------------------------------------------

BEGIN_PROPERTY(Pointer_X)

	CHECK_VALID();
	GB.ReturnFloat((double)(MOUSE_info.x + _dx) + (POINTER_info.tx - (int)POINTER_info.tx));

END_PROPERTY

BEGIN_PROPERTY(Pointer_Y)

	CHECK_VALID();
	GB.ReturnFloat((double)(MOUSE_info.y + _dy) + (POINTER_info.ty - (int)POINTER_info.ty));

END_PROPERTY

BEGIN_PROPERTY(Pointer_ScreenX)

	CHECK_VALID();
	GB.ReturnFloat(POINTER_info.tx);

END_PROPERTY

BEGIN_PROPERTY(Pointer_ScreenY)

	CHECK_VALID();
	GB.ReturnFloat(POINTER_info.ty);

END_PROPERTY

BEGIN_PROPERTY(Pointer_XTilt)

	CHECK_VALID();
	GB.ReturnFloat(POINTER_info.xtilt);

END_PROPERTY

BEGIN_PROPERTY(Pointer_YTilt)

	CHECK_VALID();
	GB.ReturnFloat(POINTER_info.ytilt);

END_PROPERTY

BEGIN_PROPERTY(Pointer_Pressure)

	CHECK_VALID();
	GB.ReturnFloat(POINTER_info.pressure);

END_PROPERTY

BEGIN_PROPERTY(Pointer_Rotation)

	CHECK_VALID();
	GB.ReturnFloat(POINTER_info.rotation);

END_PROPERTY

BEGIN_PROPERTY(Pointer_Type)

	CHECK_VALID();
	GB.ReturnInteger(POINTER_info.type);

END_PROPERTY


//-------------------------------------------------------------------------

GB_DESC CCursorDesc[] =
{
	GB_DECLARE("Cursor", sizeof(CCURSOR)),

	GB_METHOD("_new", NULL, Cursor_new, "(Picture)Picture;[(X)i(Y)i]"),
	GB_METHOD("_free", NULL, Cursor_Delete, NULL),

	GB_PROPERTY_READ("X", "i", Cursor_X),
	GB_PROPERTY_READ("Y", "i", Cursor_Y),
	//GB_PROPERTY_READ("Picture", "Picture", CCURSOR_picture),

	GB_END_DECLARE
};


GB_DESC CMouseDesc[] =
{
	GB_DECLARE("Mouse", 0), GB_VIRTUAL_CLASS(),

	GB_STATIC_PROPERTY_READ("ScreenX", "i", Mouse_ScreenX),
	GB_STATIC_PROPERTY_READ("ScreenY", "i", Mouse_ScreenY),
	GB_STATIC_METHOD("Move", NULL, Mouse_Move, "(X)i(Y)i"),
	GB_STATIC_METHOD("Inside", "b", Mouse_Inside, "(Control)Control"),

	GB_CONSTANT("Default", "i", CMOUSE_DEFAULT),
	GB_CONSTANT("Custom", "i", CMOUSE_CUSTOM),
	GB_CONSTANT("Blank", "i", Qt::BlankCursor),
	GB_CONSTANT("Arrow", "i", Qt::ArrowCursor),
	GB_CONSTANT("Cross", "i", Qt::CrossCursor),
	GB_CONSTANT("Wait", "i", Qt::WaitCursor),
	GB_CONSTANT("Text", "i", Qt::IBeamCursor),
	GB_CONSTANT("SizeAll", "i", Qt::SizeAllCursor),
	GB_CONSTANT("SizeH", "i", Qt::SizeHorCursor),
	GB_CONSTANT("SizeV", "i", Qt::SizeVerCursor),
	GB_CONSTANT("SizeN", "i", Qt::SizeVerCursor),
	GB_CONSTANT("SizeS", "i", Qt::SizeVerCursor),
	GB_CONSTANT("SizeW", "i", Qt::SizeHorCursor),
	GB_CONSTANT("SizeE", "i", Qt::SizeHorCursor),
	GB_CONSTANT("SizeNW", "i", Qt::SizeFDiagCursor),
	GB_CONSTANT("SizeSE", "i", Qt::SizeFDiagCursor),
	GB_CONSTANT("SizeNE", "i", Qt::SizeBDiagCursor),
	GB_CONSTANT("SizeSW", "i", Qt::SizeBDiagCursor),
	GB_CONSTANT("SizeNWSE", "i", Qt::SizeFDiagCursor),
	GB_CONSTANT("SizeNESW", "i", Qt::SizeBDiagCursor),
	GB_CONSTANT("SplitH", "i", Qt::SplitHCursor),
	GB_CONSTANT("SplitV", "i", Qt::SplitVCursor),
	GB_CONSTANT("Pointing", "i", Qt::PointingHandCursor),

	//GB_CONSTANT("Left", "i", Qt::LeftButton),
	//GB_CONSTANT("Right", "i", Qt::RightButton),
	//GB_CONSTANT("Middle", "i", Qt::MidButton),
	//GB_CONSTANT("Shift", "i", Qt::ShiftButton),
	//GB_CONSTANT("Control", "i", Qt::ControlButton),
	//GB_CONSTANT("Alt", "i", Qt::AltButton),
	GB_CONSTANT("Horizontal", "i", Qt::Horizontal),
	GB_CONSTANT("Vertical", "i", Qt::Vertical),

	GB_STATIC_PROPERTY_READ("X", "i", Mouse_X),
	GB_STATIC_PROPERTY_READ("Y", "i", Mouse_Y),
	GB_STATIC_PROPERTY_READ("StartX", "i", Mouse_StartX),
	GB_STATIC_PROPERTY_READ("StartY", "i", Mouse_StartY),
	GB_STATIC_PROPERTY_READ("Left", "b", Mouse_Left),
	GB_STATIC_PROPERTY_READ("Right", "b", Mouse_Right),
	GB_STATIC_PROPERTY_READ("Middle", "b", Mouse_Middle),
	GB_STATIC_PROPERTY_READ("State", "i", Mouse_State),
	GB_STATIC_PROPERTY_READ("Button", "i", Mouse_Button),
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

