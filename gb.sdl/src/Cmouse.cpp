/***************************************************************************

  Cmouse.cpp

  Gambas extension using SDL

  (c) 2006 Laurent Carlier <lordheavy@infonie.fr>
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

#include "SDLapp.h"
#include "SDL.h"

CMOUSE_INFO CMOUSE_info = { 0 };

#define CHECK_VALID() \
  if (!CMOUSE_info.valid) \
  { \
    GB.Error("No mouse event data"); \
    return; \
  }

/***************************************************************************/

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

  GB_END_DECLARE
};

