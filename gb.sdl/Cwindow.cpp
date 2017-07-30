/***************************************************************************

	Cwindow.cpp

	(c) 2006 Laurent Carlier <lordheavy@users.sourceforge.net>

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

#define __CWINDOW_CPP

#include <iostream>

#include "Cwindow.h"
#include "Cjoystick.h"
#include "Ckey.h"
#include "Cmouse.h"
#include "Cdraw.h"
#include "Cimage.h"

#include "SDL.h"

#define THIS      ((CWINDOW *)_object)
#define WINDOWID  ((CWINDOW *)_object)->id
// number of frames before counting FPS
#define FRAMECOUNT 100

// events
DECLARE_EVENT(EVENT_Close);
DECLARE_EVENT(EVENT_Resize);
DECLARE_EVENT(EVENT_Activate);
DECLARE_EVENT(EVENT_Deactivate);
DECLARE_EVENT(EVENT_Enter);
DECLARE_EVENT(EVENT_JoyAxisMotion);
DECLARE_EVENT(EVENT_JoyBallMotion);
DECLARE_EVENT(EVENT_JoyButtonPressed);
DECLARE_EVENT(EVENT_JoyButtonReleased);
DECLARE_EVENT(EVENT_JoyHatMotion);
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
	THIS->lastTime = SDL_GetTicks();
	THIS->startTime = THIS->lastTime;

END_METHOD

BEGIN_METHOD_VOID(CWINDOW_free)

	GB.StoreObject(NULL, POINTER(&(THIS->cursor)));
	delete WINDOWID;

END_METHOD

BEGIN_METHOD_VOID(CWINDOW_show)

	WINDOWID->Show();
	WINDOWID->Refresh();

END_METHOD

BEGIN_METHOD_VOID(CWINDOW_close)

	WINDOWID->Quit();

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

BEGIN_PROPERTY(CWINDOW_framerate)

	if (READ_PROPERTY)
		GB.ReturnFloat(THIS->currentFPS);
	else
	{
		double val = VPROP(GB_FLOAT);

		if (val < 0)
			return;

		THIS->FPSLimit = val ? 1000.0 / val : 0;
		THIS->lastTime = SDL_GetTicks();
	}

END_PROPERTY

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

BEGIN_PROPERTY(CWINDOW_resizable)

	if (READ_PROPERTY)
		GB.ReturnBoolean(WINDOWID->IsResizable());
	else
		WINDOWID->SetResizable(VPROP(GB_BOOLEAN));

END_PROPERTY

BEGIN_PROPERTY(CWINDOW_mouse)

	if (READ_PROPERTY)
		GB.ReturnInteger(WINDOWID->GetCursorShape());
	else
		WINDOWID->SetCursorShape(VPROP(GB_INTEGER));

END_PROPERTY

BEGIN_PROPERTY(CWINDOW_tracking)

	if (READ_PROPERTY)
		GB.ReturnBoolean(THIS->tracking);
	else
		THIS->tracking = VPROP(GB_BOOLEAN);

END_PROPERTY

BEGIN_PROPERTY(CWINDOW_cursor)

	if (READ_PROPERTY)
		GB.ReturnObject(THIS->cursor);
	else
	{
/*
		CCURSOR *curs = (CCURSOR *)VPROP(GB_OBJECT);
		WINDOWID->SetCursor(curs->cursor);
		return;
*/
	}

END_PROPERTY

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

BEGIN_PROPERTY(CWINDOW_shown)

	GB.ReturnBoolean(WINDOWID->IsShown());

END_PROPERTY

BEGIN_PROPERTY(CWINDOW_id)

	GB.ReturnInteger(WINDOWID->Id());

END_PROPERTY

BEGIN_PROPERTY(Window_Grabbed)

	if (READ_PROPERTY)
		GB.ReturnBoolean(WINDOWID->IsInputGrabbed());
	else
		WINDOWID->GrabInput(VPROP(GB_BOOLEAN));

END_PROPERTY

BEGIN_METHOD(Window_Screenshot, GB_INTEGER x; GB_INTEGER y; GB_INTEGER w; GB_INTEGER h)

	GB.ReturnObject(CIMAGE_create_from_window(WINDOWID, VARGOPT(x, 0), VARGOPT(y, 0), VARGOPT(w, -1), VARGOPT(h, -1)));

END_METHOD

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

	GB_PROPERTY("Caption", "s", CWINDOW_text),
//  GB_PROPERTY("Cursor", "Cursor;", CWINDOW_cursor),
	GB_PROPERTY("Framerate", "f", CWINDOW_framerate),
	GB_PROPERTY("FullScreen", "b", CWINDOW_fullscreen),
	GB_PROPERTY("Height", "i", CWINDOW_height),
	GB_PROPERTY("Mouse", "i", CWINDOW_mouse),
	GB_PROPERTY("Text", "s", CWINDOW_text),
	GB_PROPERTY("Title", "s", CWINDOW_text),
	GB_PROPERTY("Tracking", "b", CWINDOW_tracking),
	GB_PROPERTY("Resizable", "b", CWINDOW_resizable),
	GB_PROPERTY("Width", "i", CWINDOW_width),

	GB_PROPERTY_READ("Handle", "i", CWINDOW_id),
	GB_PROPERTY_READ("Id", "i", CWINDOW_id),
	GB_PROPERTY_READ("Shown", "b", CWINDOW_shown),
	GB_PROPERTY("Grabbed", "b", Window_Grabbed),

	GB_METHOD("Screenshot", "Image", Window_Screenshot, "[(X)i(Y)i(Width)i(Height)i]"),

	GB_EVENT("Close", "b", NULL, &EVENT_Close),
	GB_EVENT("Resize", NULL, NULL, &EVENT_Resize),
	GB_EVENT("Activate", NULL, NULL, &EVENT_Activate),
	GB_EVENT("Deactivate", NULL, NULL, &EVENT_Deactivate),
	GB_EVENT("Enter", NULL, NULL, &EVENT_Enter),
	GB_EVENT("JoyAxisMove", NULL, NULL, &EVENT_JoyAxisMotion),
	GB_EVENT("JoyBallMove", NULL, NULL, &EVENT_JoyBallMotion),
	GB_EVENT("JoyButtonPress", NULL, NULL, &EVENT_JoyButtonPressed),
	GB_EVENT("JoyButtonRelease", NULL, NULL, &EVENT_JoyButtonReleased),
	GB_EVENT("JoyHatMove", NULL, NULL, &EVENT_JoyHatMotion),
	GB_EVENT("Leave", NULL, NULL, &EVENT_Leave),
	GB_EVENT("Draw", "b", NULL, &EVENT_Refresh),
	GB_EVENT("KeyPress", NULL, NULL, &EVENT_KeyPressed),
	GB_EVENT("KeyRelease", NULL, NULL, &EVENT_KeyReleased),
	GB_EVENT("MouseMove", NULL, NULL, &EVENT_MouseMove),
	GB_EVENT("MouseDown", NULL, NULL, &EVENT_MouseDown),
	GB_EVENT("MouseUp", NULL, NULL, &EVENT_MouseUp),
	GB_EVENT("Open", NULL, NULL, &EVENT_Open),

	GB_END_DECLARE
};

/***************************************************************************/

#define WINDOW(object) ((CWINDOW *)object)

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
	GB.Raise(hWindow, EVENT_Deactivate, 0);
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
	Uint32 ticks, diff;

	// no refresh event
	if (!GB.CanRaise(hWindow, EVENT_Refresh))
	{
		SDL_Delay(1);
		return;
	}

	ticks = SDL_GetTicks();

	// framerate limitation
	if (WINDOW(hWindow)->FPSLimit > 0)
	{
		double d = WINDOW(hWindow)->lastTime + WINDOW(hWindow)->FPSLimit;

		//fprintf(stderr, "%d %g %g %d\n", ticks, d, WINDOW(hWindow)->lastTime, d < ticks);

		if (d > ticks)
		{
			SDL_Delay(1);
			return;
		}

		WINDOW(hWindow)->lastTime = d;
	}

	DRAW_begin(hWindow);
	bool cancel = GB.Raise(hWindow, EVENT_Refresh, 0);
	DRAW_end();

	// user doesn't want to refresh
	if (cancel)
	{
		//SDL_Delay(1);
		return;
	}
	else
		this->Refresh();

	// calculate the framerate
	/*if (WINDOW(hWindow)->countFrames >= FRAMECOUNT)
	{
		double value = (ticks - WINDOW(hWindow)->startTime) / FRAMECOUNT;

		if (value > 0)
			WINDOW(hWindow)->currentFPS = Uint32(1000 / value + 0.5);
		else
			WINDOW(hWindow)->currentFPS = 0;

		WINDOW(hWindow)->countFrames = 0;
		WINDOW(hWindow)->startTime = ticks;
	}
	else
		WINDOW(hWindow)->countFrames++;*/

	WINDOW(hWindow)->countFrames++;

	diff = ticks - WINDOW(hWindow)->startTime;
	if (diff > 1000)
	{
		WINDOW(hWindow)->currentFPS = WINDOW(hWindow)->countFrames;
		WINDOW(hWindow)->countFrames = 0;
		WINDOW(hWindow)->startTime += 1000;
	}
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
		glOrtho(0.0f, GLdouble(this->GetWidth()), GLdouble(this->GetHeight()), 0.0f, -1, 1);
		glMatrixMode(GL_MODELVIEW);
	}

	if (GB.CanRaise(hWindow, EVENT_Open))
		GB.Raise(hWindow, EVENT_Open,0);

	if ((((CWINDOW *)hWindow)->openGL))
	{
		if (GB.CanRaise(hWindow, EVENT_Resize))
			GB.Raise(hWindow, EVENT_Resize,0);
	}
}

void myWin::JoyEvent(SDL_Event& event)
{
	CJOY_info.valid = true;
	switch(event.type)
	{
	case SDL_JOYAXISMOTION:
	{
		CJOY_info.device = event.jaxis.which;
		CJOY_info.id = event.jaxis.axis;
		CJOY_info.value1 = event.jaxis.value;
		CJOY_info.value2 = 0;
		GB.Raise(hWindow, EVENT_JoyAxisMotion, 0);
		break;
	}
	case SDL_JOYHATMOTION:
	{
		CJOY_info.device = event.jaxis.which;
		CJOY_info.id = event.jhat.hat;
		CJOY_info.value1 = event.jhat.value;
		CJOY_info.value2 = 0;
		GB.Raise(hWindow, EVENT_JoyHatMotion, 0);
		break;
	}
	case SDL_JOYBALLMOTION:
	{
		CJOY_info.device = event.jaxis.which;
		CJOY_info.id = event.jball.ball;
		CJOY_info.value1 = event.jball.xrel;
		CJOY_info.value2 = event.jball.yrel;
		GB.Raise(hWindow, EVENT_JoyBallMotion, 0);
		break;
	}
	case SDL_JOYBUTTONDOWN:
	case SDL_JOYBUTTONUP:
	{
		CJOY_info.device = event.jaxis.which;
		CJOY_info.id = event.jbutton.button;
		CJOY_info.value1 = 0;
		CJOY_info.value2 = 0;
		if (event.jbutton.state == SDL_PRESSED)
			GB.Raise(hWindow, EVENT_JoyButtonPressed, 0);
		else
			GB.Raise(hWindow, EVENT_JoyButtonReleased, 0);
		break;
	}
	default:
		break;
	}

	CJOY_info.valid = false;
}

static void convert_unicode_to_utf8(Uint16 unicode, char *buffer)
{
	if (unicode < 0x80)
	{
		buffer[0] = unicode;
		buffer[1] = 0;
	}
	else if (unicode < 0x800)
	{
		buffer[0] = 0xC0 | (unicode >> 6);
		buffer[1] = 0x80 | (unicode & 0x3F);
		buffer[2] = 0;
	}
	else
	{
		buffer[0] = 0xE0 | (unicode >> 12);
		buffer[1] = 0x80 | ((unicode >> 6) & 0x3F);
		buffer[2] = 0x80 | (unicode & 0x3F);
		buffer[3] = 0;
	}
}

void myWin::KeyEvent(SDL_KeyboardEvent *keyEvent, int eventType)
{
	CKEY_info.valid++;

	CKEY_info.code = keyEvent->keysym.sym;
	CKEY_info.state = keyEvent->keysym.mod;
	convert_unicode_to_utf8(keyEvent->keysym.unicode, CKEY_info.text);

	//SDLapp->LockX11();
	//CKEY_info.code = XKeycodeToKeysym(SDLapp->X11appDisplay(), keyEvent->keysym.scancode, 0);
	//SDLapp->UnlockX11();
	//CKEY_info.state = keyEvent->keysym.mod;

	if (eventType == SDL_KEYDOWN)
		GB.Raise(hWindow, EVENT_KeyPressed,0);
	else
		GB.Raise(hWindow, EVENT_KeyReleased,0);

	CKEY_info.valid--;
}

void myWin::MouseButtonEvent(SDL_MouseButtonEvent *mouseEvent)
{
	CMOUSE_info.valid = true;
	CMOUSE_info.x = mouseEvent->x;
	CMOUSE_info.y = mouseEvent->y;
	CMOUSE_info.state = mouseEvent->button;
	CMOUSE_info.keymod = SDL_GetModState();

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

	// do not raise event if no mouse button are pressed/released && tracking is not set
	if ((!mouseEvent->state) && (!(((CWINDOW *)hWindow)->tracking)))
		return;

	CMOUSE_info.valid = true;
	CMOUSE_info.x = mouseEvent->x;
	CMOUSE_info.y = mouseEvent->y;
	CMOUSE_info.state = mouseEvent->state;
	CMOUSE_info.keymod = SDL_GetModState();
	GB.Raise(hWindow, EVENT_MouseMove,0);
	CMOUSE_info.valid = false;
}
