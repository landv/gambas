/***************************************************************************

  CMouse.cpp

  (c) 2004-2006 - Daniel Campos Fernández <dcamposf@gmail.com>
  
  GTK+ component
  
  Realizado para la Junta de Extremadura. 
  Consejería de Educación Ciencia y Tecnología. 
  Proyecto gnuLinEx
  
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 1, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

***************************************************************************/

#define __CMOUSE_CPP

#include "CMouse.h"

/************************************************************************************

Cursor Class

*************************************************************************************/
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


/********************************************************************************

Mouse Class

*********************************************************************************/

BEGIN_PROPERTY(CMOUSE_screen_x)

	GB.ReturnInteger(gMouse::screenX());

END_PROPERTY


BEGIN_PROPERTY(CMOUSE_screen_y)

	GB.ReturnInteger(gMouse::screenY());

END_PROPERTY


BEGIN_METHOD(CMOUSE_move, GB_INTEGER x; GB_INTEGER y)

	gMouse::move(VARG(x),VARG(y));

END_PROPERTY



BEGIN_PROPERTY(CMOUSE_x)

	if (!gMouse::valid()) { GB.Error("No mouse event data"); return; }
	GB.ReturnInteger(gMouse::x());

END_PROPERTY

BEGIN_PROPERTY(CMOUSE_y)

	if (!gMouse::valid()) { GB.Error("No mouse event data"); return; }
	GB.ReturnInteger(gMouse::y());

END_PROPERTY

BEGIN_PROPERTY(CMOUSE_button)
	
	if (!gMouse::valid()) { GB.Error("No mouse event data"); return; }
	GB.ReturnInteger(gMouse::button());

END_PROPERTY

BEGIN_PROPERTY(CMOUSE_left)

	if (!gMouse::valid()) { GB.Error("No mouse event data"); return; }
	GB.ReturnBoolean(gMouse::left());

END_PROPERTY

BEGIN_PROPERTY(CMOUSE_right)

	if (!gMouse::valid()) { GB.Error("No mouse event data"); return; }
	GB.ReturnBoolean(gMouse::right());

END_PROPERTY

BEGIN_PROPERTY(CMOUSE_middle)

	if (!gMouse::valid()) { GB.Error("No mouse event data"); return; }
	GB.ReturnBoolean(gMouse::middle());

END_PROPERTY

BEGIN_PROPERTY(CMOUSE_shift)

	if (!gMouse::valid()) { GB.Error("No mouse event data"); return; }
	GB.ReturnBoolean(gMouse::shift());

END_PROPERTY

BEGIN_PROPERTY(CMOUSE_control)

	if (!gMouse::valid()) { GB.Error("No mouse event data"); return; }
	GB.ReturnBoolean(gMouse::control());

END_PROPERTY

BEGIN_PROPERTY(CMOUSE_alt)

	if (!gMouse::valid()) { GB.Error("No mouse event data"); return; }
	GB.ReturnBoolean(gMouse::alt());

END_PROPERTY

BEGIN_PROPERTY(CMOUSE_meta)

	if (!gMouse::valid()) { GB.Error("No mouse event data"); return; }
	GB.ReturnBoolean(gMouse::meta());

END_PROPERTY

BEGIN_PROPERTY(CMOUSE_normal)

	if (!gMouse::valid()) { GB.Error("No mouse event data"); return; }
	GB.ReturnBoolean(gMouse::normal());

END_PROPERTY

BEGIN_PROPERTY(CMOUSE_delta)

	if (!gMouse::valid()) { GB.Error("No mouse event data"); return; }
	GB.ReturnInteger(gMouse::delta());

END_PROPERTY

BEGIN_PROPERTY(CMOUSE_orientation)

	if (!gMouse::valid()) { GB.Error("No mouse event data"); return; }
	GB.ReturnInteger(gMouse::orientation());

END_PROPERTY



GB_DESC CCursorDesc[] =
{
  GB_DECLARE("Cursor", sizeof(CCURSOR)),

  GB_METHOD("_new", NULL, CCURSOR_new, "(Picture)Picture;[(X)i(Y)i]"),
  GB_METHOD("_free", NULL, CCURSOR_delete, NULL),

  GB_PROPERTY_READ("X", "i", CCURSOR_x),
  GB_PROPERTY_READ("Y", "i", CCURSOR_y),

  GB_END_DECLARE
};


GB_DESC CMouseDesc[] =
{
  GB_DECLARE("Mouse", 0), GB_VIRTUAL_CLASS(),

  GB_STATIC_PROPERTY_READ("ScreenX", "i", CMOUSE_screen_x),
  GB_STATIC_PROPERTY_READ("ScreenY", "i", CMOUSE_screen_y),
  GB_STATIC_METHOD("Move", NULL, CMOUSE_move, "(X)i(Y)i"),

  GB_CONSTANT("Default", "i", -1),
  GB_CONSTANT("Custom", "i", -2),
  GB_CONSTANT("Blank", "i", GDK_LAST_CURSOR),
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

  GB_STATIC_PROPERTY_READ("X", "i", CMOUSE_x),
  GB_STATIC_PROPERTY_READ("Y", "i", CMOUSE_y),
  GB_STATIC_PROPERTY_READ("Left", "b", CMOUSE_left),
  GB_STATIC_PROPERTY_READ("Right", "b", CMOUSE_right),
  GB_STATIC_PROPERTY_READ("Middle", "b", CMOUSE_middle),
  GB_STATIC_PROPERTY_READ("Button", "i", CMOUSE_button),
  GB_STATIC_PROPERTY_READ("Shift", "b", CMOUSE_shift),
  GB_STATIC_PROPERTY_READ("Control", "b", CMOUSE_control),
  GB_STATIC_PROPERTY_READ("Alt", "b", CMOUSE_alt),
  GB_STATIC_PROPERTY_READ("Meta", "b", CMOUSE_meta),
  GB_STATIC_PROPERTY_READ("Normal", "b", CMOUSE_normal),
  GB_STATIC_PROPERTY_READ("Orientation", "i", CMOUSE_orientation),
  GB_STATIC_PROPERTY_READ("Delta", "f", CMOUSE_delta),

  GB_END_DECLARE
};
