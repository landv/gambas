/***************************************************************************

  Cwindow.cpp

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

#define __CWINDOW_CPP

#include <iostream>

#include "Cwindow.h"
#include "Ckey.h"
#include "Cmouse.h"

#include "SDL.h"

#define THIS      ((CWINDOW *)_object)
#define WINDOWID  ((CWINDOW *)_object)->id

// events
DECLARE_EVENT(EVENT_Close);
DECLARE_EVENT(EVENT_Resize);
DECLARE_EVENT(EVENT_Activate);
DECLARE_EVENT(EVENT_DeActivate);
DECLARE_EVENT(EVENT_Enter);
DECLARE_EVENT(EVENT_Leave);
DECLARE_EVENT(EVENT_Refresh);
DECLARE_EVENT(EVENT_KeyPressed);
DECLARE_EVENT(EVENT_KeyReleased);
DECLARE_EVENT(EVENT_MouseMove);
DECLARE_EVENT(EVENT_MouseDown);
DECLARE_EVENT(EVENT_MouseUp);
DECLARE_EVENT(EVENT_Open);

/***************************************************************************/

BEGIN_METHOD(CWINDOW_new, GB_BOOLEAN openGL)

	WINDOWID = new myWin(THIS);
	WINDOWID->SetTitle(GB.Application.Name());
	THIS->openGL = VARGOPT(openGL, false);

END_METHOD

BEGIN_METHOD_VOID(CWINDOW_free)

	delete WINDOWID;

END_METHOD

BEGIN_METHOD_VOID(CWINDOW_show)

	WINDOWID->Show();
	WINDOWID->Refresh();

END_METHOD

BEGIN_METHOD_VOID(CWINDOW_close)

	WINDOWID->Close();

END_METHOD

BEGIN_METHOD_VOID(CWINDOW_clear)

	WINDOWID->Clear();

END_METHOD

BEGIN_METHOD(CWINDOW_fill, GB_INTEGER color;)

	WINDOWID->Clear(VARG(color));

END_METHOD

BEGIN_METHOD_VOID(CWINDOW_refresh)

	WINDOWID->Refresh();

END_METHOD

BEGIN_PROPERTY(CWINDOW_text)

	if (READ_PROPERTY)
		GB.ReturnNewZeroString(WINDOWID->GetTitle());
	else
		WINDOWID->SetTitle(GB.ToZeroString(PROP(GB_STRING)));

END_PROPERTY

BEGIN_PROPERTY(CWINDOW_fullscreen)

	if (READ_PROPERTY)
		GB.ReturnBoolean(WINDOWID->IsFullScreen());
	else
		WINDOWID->SetFullScreen(VPROP(GB_BOOLEAN));

END_PROPERTY

BEGIN_PROPERTY(CWINDOW_border)

	if (READ_PROPERTY)
		GB.ReturnInteger(WINDOWID->IsResizable());
	else
	{
		if (VPROP(GB_INTEGER) == 0)
			WINDOWID->SetResizable(false);
		if (VPROP(GB_INTEGER) == 1)
			WINDOWID->SetResizable(true);
	}

END_PROPERTY
#if 0
BEGIN_PROPERTY(CWINDOW_x)

	if (READ_PROPERTY)
		GB.ReturnInteger(WINDOWID->GetX());
	else
		THIS->id->SetX(VPROP(GB_INTEGER));

END_PROPERTY

BEGIN_PROPERTY(CWINDOW_y)

	if (READ_PROPERTY)
		GB.ReturnInteger(WINDOWID->GetY());
	else
		WINDOWID->SetY(VPROP(GB_INTEGER));

END_PROPERTY
#endif
BEGIN_PROPERTY(CWINDOW_width)

	if (READ_PROPERTY)
		GB.ReturnInteger(WINDOWID->GetWidth());
	else
		THIS->id->SetWidth(VPROP(GB_INTEGER));

END_PROPERTY

BEGIN_PROPERTY(CWINDOW_height)

	if (READ_PROPERTY)
		GB.ReturnInteger(WINDOWID->GetHeight());
	else
		WINDOWID->SetHeight(VPROP(GB_INTEGER));

END_PROPERTY

BEGIN_PROPERTY(CWINDOW_depth)

	GB.ReturnInteger(WINDOWID->GetDepth());

END_PROPERTY

BEGIN_PROPERTY(CWINDOW_shown)

	GB.ReturnBoolean(WINDOWID->IsShown());

END_PROPERTY

BEGIN_PROPERTY(CWINDOW_id)

	GB.ReturnInteger(WINDOWID->Id());

END_PROPERTY
#if 0
BEGIN_PROPERTY(CWINDOW_background)

	if (READ_PROPERTY)
		GB.ReturnInteger(WINDOWID->GetBackColor());
	else
		WINDOWID->SetBackColor(VPROP(GB_INTEGER));

END_PROPERTY

BEGIN_PROPERTY(CWINDOW_foreground)

	if (READ_PROPERTY)
		GB.ReturnInteger(WINDOWID->GetForeColor());
	else
		WINDOWID->SetForeColor(VPROP(GB_INTEGER));

END_PROPERTY
#endif
/***************************************************************************/

GB_DESC CWindow[] =
{
  GB_DECLARE("Window", sizeof(CWINDOW)),

  GB_METHOD("_new", NULL, CWINDOW_new, "[(OpenGL)b]"),
  GB_METHOD("_free", NULL, CWINDOW_free, NULL),

  GB_METHOD("Show", NULL, CWINDOW_show, NULL),
  GB_METHOD("Close", NULL, CWINDOW_close, NULL),
  GB_METHOD("Clear", NULL, CWINDOW_clear, NULL),
  GB_METHOD("Fill", NULL, CWINDOW_fill, "(Color)i"),
  GB_METHOD("Refresh", NULL, CWINDOW_refresh, NULL),
  GB_METHOD("Update", NULL, CWINDOW_refresh, NULL),

  GB_PROPERTY("Text", "s", CWINDOW_text),
  GB_PROPERTY("Title", "s", CWINDOW_text),
  GB_PROPERTY("Caption", "s", CWINDOW_text),
  GB_PROPERTY("FullScreen", "b", CWINDOW_fullscreen),
  GB_PROPERTY("Border", "i", CWINDOW_border),
  //GB_PROPERTY("X", "i", CWINDOW_x),
  //GB_PROPERTY("Y", "i", CWINDOW_y),
  GB_PROPERTY("Height", "i", CWINDOW_height),
  GB_PROPERTY("Width", "i", CWINDOW_width),
  //GB_PROPERTY("Background", "i", CWINDOW_background),
  //GB_PROPERTY("BackColor", "i", CWINDOW_background),
  //GB_PROPERTY("Foreground", "i", CWINDOW_foreground),
  //GB_PROPERTY("ForeColor", "i", CWINDOW_foreground),

  GB_PROPERTY_READ("Depth", "i", CWINDOW_depth),
  GB_PROPERTY_READ("Shown", "b", CWINDOW_shown),
  GB_PROPERTY_READ("Id", "i", CWINDOW_id),

  GB_EVENT("Close", "b", NULL, &EVENT_Close),
  GB_EVENT("Resize", NULL, NULL, &EVENT_Resize),
  GB_EVENT("Activate", NULL, NULL, &EVENT_Activate),
  GB_EVENT("DeActivate", NULL, NULL, &EVENT_DeActivate),
  GB_EVENT("Enter", NULL, NULL, &EVENT_Enter),
  GB_EVENT("Leave", NULL, NULL, &EVENT_Leave),
  GB_EVENT("Draw", "b", NULL, &EVENT_Refresh),
  GB_EVENT("KeyPressed", NULL, NULL, &EVENT_KeyPressed),
  GB_EVENT("KeyReleased", NULL, NULL, &EVENT_KeyReleased),
  GB_EVENT("MouseMove", NULL, NULL, &EVENT_MouseMove),
  GB_EVENT("MouseDown", NULL, NULL, &EVENT_MouseDown),
  GB_EVENT("MouseUp", NULL, NULL, &EVENT_MouseUp),
  GB_EVENT("Open", NULL, NULL, &EVENT_Open),

  GB_CONSTANT("Fixed", "i", 0),
  GB_CONSTANT("Resizable", "i", 1),

  GB_END_DECLARE
};

/***************************************************************************/

void myWin::Resize(void)
{
	GB.Raise(hWindow, EVENT_Resize, 0);
}

void myWin::GotFocus(void)
{
	GB.Raise(hWindow, EVENT_Activate, 0);
}

void myWin::LostFocus(void)
{
	GB.Raise(hWindow, EVENT_DeActivate, 0);
}

void myWin::MouseEnter(void)
{
	GB.Raise(hWindow, EVENT_Enter, 0);
}

void myWin::MouseLeave(void)
{
	GB.Raise(hWindow, EVENT_Leave, 0);
}

void myWin::Quit(void)
{
	bool cancel = GB.Raise(hWindow, EVENT_Close, 0);

	if (!cancel)
		this->Close();

}

void myWin::Update(void)
{
	if (!GB.CanRaise(hWindow, EVENT_Refresh))
	{
		SDL_Delay(1);
		return;
	}

	bool cancel = GB.Raise(hWindow, EVENT_Refresh, 0);

	if (!cancel)
		this->Refresh();
	else
		SDL_Delay(1);

}

void myWin::Open(void)
{
	if (!(((CWINDOW *)hWindow)->openGL))
	{
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glViewport(0, 0, this->GetWidth(), this->GetHeight());
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluOrtho2D(0.0f, GLdouble(this->GetWidth()), GLdouble(this->GetHeight()), 0.0f);
		// enable blending, should work like 2d sdl does
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		// enable anti-aliasing
		glEnable(GL_POINT_SMOOTH);
		glEnable(GL_LINE_SMOOTH);
		glMatrixMode(GL_MODELVIEW);
	}	

	if (GB.CanRaise(hWindow, EVENT_Open))
		GB.Raise(hWindow, EVENT_Open,0);
}

void myWin::KeyEvent(SDL_KeyboardEvent *keyEvent, int eventType)
{
	CKEY_info.valid = true;
	CKEY_info.code = XKeycodeToKeysym(SDLapp->X11appDisplay(), keyEvent->keysym.scancode, 0);
	CKEY_info.state = keyEvent->keysym.mod;

	if (eventType == SDL_KEYDOWN)
		GB.Raise(hWindow, EVENT_KeyPressed,0);
	else
		GB.Raise(hWindow, EVENT_KeyReleased,0);

	CKEY_info.valid = false;

}
void myWin::MouseButtonEvent(SDL_MouseButtonEvent *mouseEvent)
{
	CMOUSE_info.valid = true;
	CMOUSE_info.x = mouseEvent->x;
	CMOUSE_info.y = mouseEvent->y;

	if (mouseEvent->type == SDL_MOUSEBUTTONDOWN)
		GB.Raise(hWindow, EVENT_MouseDown,0);
	else
		GB.Raise(hWindow, EVENT_MouseUp,0);

	CMOUSE_info.valid = false;

}

void myWin::MouseMotionEvent(SDL_MouseMotionEvent *mouseEvent)
{
	CMOUSE_info.relx = mouseEvent->xrel;
	CMOUSE_info.rely = mouseEvent->yrel;

	// do not raise event if no mouse button are pressed/released
	if (!mouseEvent->state)
		return;

	CMOUSE_info.valid = true;
	CMOUSE_info.x = mouseEvent->x;
	CMOUSE_info.y = mouseEvent->y;
	GB.Raise(hWindow, EVENT_MouseMove,0);
	CMOUSE_info.valid = false;
}
