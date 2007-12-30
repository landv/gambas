/***************************************************************************

  Cmouse.cpp

  Gambas extension using SDL

  (c) 2006 Laurent Carlier <lordheavy@users.sourceforge.net>
           Benoît Minisini <gambas@users.sourceforge.net>

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

#include "gambas.h"
#include "main.h"

#include "Cmouse.h"
#include "Cimage.h"

#include "SDL_h.h"
#include "SDLapp.h"

CMOUSE_INFO CMOUSE_info = { 0 };

#define CHECK_VALID() \
  if (!CMOUSE_info.valid) \
  { \
    GB.Error("No mouse event data"); \
    return; \
  }

/***************************************************************************/

BEGIN_METHOD(CURSOR_new, GB_OBJECT image; GB_INTEGER x; GB_INTEGER y)

	CIMAGE *img = (CIMAGE *)VARG(image);

	THIS->x = VARGOPT(x, -1);
	THIS->y = VARGOPT(y, -1);

	if (GB.CheckObject(img))
		return;

	if (THIS->x < 0 || THIS->x >= img->id->GetWidth())
		THIS->x = -1;

	if (THIS->y < 0 || THIS->y >= img->id->GetHeight())
		THIS->y = -1;

	THIS->cursor = new SDLcursor();
	THIS->cursor->SetCursor(img->id, THIS->x, THIS->y);

END_METHOD


BEGIN_METHOD_VOID(CURSOR_delete)

	delete THIS->cursor;

END_METHOD

BEGIN_PROPERTY(CURSOR_x)

	GB.ReturnInteger(THIS->x);

END_PROPERTY


BEGIN_PROPERTY(CURSOR_y)

	GB.ReturnInteger(THIS->y);

END_PROPERTY

BEGIN_METHOD(CMOUSE_move, GB_INTEGER x; GB_INTEGER y)

	SDL_WarpMouse(VARG(x), VARG(y));

END_METHOD

BEGIN_PROPERTY(CMOUSE_screenx)

	int x, y, toto;
	Window tata;
	unsigned int mask;

	SDLapp->LockX11();
	XQueryPointer(SDLapp->X11appDisplay(), SDLapp->X11appRootWin(), &tata, &tata, &x,&y, &toto,&toto, &mask);
	SDLapp->UnlockX11();

	GB.ReturnInteger(x);

END_PROPERTY

BEGIN_PROPERTY(CMOUSE_screeny)

	int x, y, toto;
	Window tata;
	unsigned int mask;

	SDLapp->LockX11();
	XQueryPointer(SDLapp->X11appDisplay(), SDLapp->X11appRootWin(), &tata, &tata, &x,&y, &toto,&toto, &mask);
	SDLapp->UnlockX11();

	GB.ReturnInteger(y);

END_PROPERTY

BEGIN_PROPERTY(CMOUSE_x)

	GB.ReturnInteger(CMOUSE_info.x);

END_PROPERTY

BEGIN_PROPERTY(CMOUSE_y)

	GB.ReturnInteger(CMOUSE_info.y);

END_PROPERTY

BEGIN_PROPERTY(CMOUSE_relx)

	CHECK_VALID()
	GB.ReturnInteger(CMOUSE_info.relx);

END_PROPERTY

BEGIN_PROPERTY(CMOUSE_rely)

	CHECK_VALID()
	GB.ReturnInteger(CMOUSE_info.rely);

END_PROPERTY

/***************************************************************************/

GB_DESC CCursor[] =
{
  GB_DECLARE("Cursor", sizeof(CCURSOR)),

  GB_METHOD("_new", NULL, CURSOR_new, "(Image)Image;[(X)i(Y)i]"),
  GB_METHOD("_free", NULL, CURSOR_delete, NULL),

  GB_PROPERTY_READ("X", "i", CURSOR_x),
  GB_PROPERTY_READ("Y", "i", CURSOR_y),

  GB_END_DECLARE
};

GB_DESC CMouse[] =
{
  GB_DECLARE("Mouse", 0), GB_VIRTUAL_CLASS(),

  GB_STATIC_METHOD("Move", NULL, CMOUSE_move, "(X)i(Y)i"),

  GB_STATIC_PROPERTY_READ("ScreenX", "i", CMOUSE_screenx),
  GB_STATIC_PROPERTY_READ("ScreenY", "i", CMOUSE_screeny),
  GB_STATIC_PROPERTY_READ("X", "i", CMOUSE_x),
  GB_STATIC_PROPERTY_READ("Y", "i", CMOUSE_y),
  GB_STATIC_PROPERTY_READ("RelativeX", "i", CMOUSE_relx),
  GB_STATIC_PROPERTY_READ("RelativeY", "i", CMOUSE_rely),

  GB_CONSTANT("Default", "i", SDL::DefaultCursor),
  GB_CONSTANT("Custom", "i", SDL::CustomCursor),
  GB_CONSTANT("Blank", "i", SDL::BlankCursor),
  GB_CONSTANT("Arrow", "i", SDL::ArrowCursor),
  GB_CONSTANT("Cross", "i", SDL::CrossCursor),
  GB_CONSTANT("Wait", "i", SDL::WaitCursor),
  GB_CONSTANT("Text", "i", SDL::TextCursor),
  GB_CONSTANT("SizeAll", "i", SDL::SizeAllCursor),
  GB_CONSTANT("SizeH", "i", SDL::SizeHorCursor),
  GB_CONSTANT("SizeV", "i", SDL::SizeVerCursor),
  GB_CONSTANT("SizeN", "i", SDL::SizeVerCursor),
  GB_CONSTANT("SizeS", "i", SDL::SizeVerCursor),
  GB_CONSTANT("SizeW", "i", SDL::SizeHorCursor),
  GB_CONSTANT("SizeE", "i", SDL::SizeHorCursor),
  GB_CONSTANT("SizeNW", "i", SDL::SizeFDiagCursor),
  GB_CONSTANT("SizeSE", "i", SDL::SizeFDiagCursor),
  GB_CONSTANT("SizeNE", "i", SDL::SizeBDiagCursor),
  GB_CONSTANT("SizeSW", "i", SDL::SizeBDiagCursor),
  GB_CONSTANT("SizeNWSE", "i", SDL::SizeFDiagCursor),
  GB_CONSTANT("SizeNESW", "i", SDL::SizeBDiagCursor),
  GB_CONSTANT("SplitH", "i", SDL::SplitHCursor),
  GB_CONSTANT("SplitV", "i", SDL::SplitVCursor),
  GB_CONSTANT("Pointing", "i", SDL::PointingHandCursor),

  GB_END_DECLARE
};

