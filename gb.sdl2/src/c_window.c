/***************************************************************************

  c_window.c

  (c) 2014 Benoît Minisini <gambas@users.sourceforge.net>

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

#define __C_WINDOW_C

#include "c_draw.h"
#include "c_window.h"

#define THIS ((CWINDOW *)_object)
#define WINDOW THIS->window

DECLARE_EVENT(EVENT_Open);
DECLARE_EVENT(EVENT_Close);
DECLARE_EVENT(EVENT_Show);
DECLARE_EVENT(EVENT_Hide);
DECLARE_EVENT(EVENT_Move);
DECLARE_EVENT(EVENT_Resize);
DECLARE_EVENT(EVENT_Enter);
DECLARE_EVENT(EVENT_Leave);
DECLARE_EVENT(EVENT_GotFocus);
DECLARE_EVENT(EVENT_LostFocus);
DECLARE_EVENT(EVENT_Draw);

CWINDOW *WINDOW_list = NULL;

static void update_geometry(void *_object)
{
	if (!THIS->opened)
		return;
	
	if (THIS->fullscreen)
	{
		SDL_SetWindowFullscreen(WINDOW, SDL_WINDOW_FULLSCREEN_DESKTOP);
		SDL_RenderSetLogicalSize(THIS->renderer, THIS->width, THIS->height);
	}
	else
	{
		SDL_SetWindowFullscreen(WINDOW, 0);
		SDL_SetWindowPosition(WINDOW, THIS->x, THIS->y);
		SDL_SetWindowSize(WINDOW, THIS->width, THIS->height);
	}
}

static void open_window(void *_object)
{
	if (THIS->opened)
		return;
	
	if (GB.Raise(THIS, EVENT_Open, 0))
		return;
	
	THIS->opened = TRUE;
	GB.Ref(THIS);
	LIST_insert(&WINDOW_list, THIS, &THIS->list);
	
	SDL_ShowWindow(WINDOW);
	update_geometry(THIS);
	
	/*if (!THIS->opengl)
	{
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glViewport(0, 0, this->GetWidth(), this->GetHeight());
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(0.0f, GLdouble(this->GetWidth()), GLdouble(this->GetHeight()), 0.0f, -1, 1);
		glMatrixMode(GL_MODELVIEW);
	}*/

	/*if (THIS->opengl)
	{
		if (GB.CanRaise(hWindow, EVENT_Resize))
			GB.Raise(hWindow, EVENT_Resize,0);
	}*/
}


static void close_window(void *_object)
{
	if (!THIS->opened)
		return;
	
	if (GB.Raise(THIS, EVENT_Close, 0))
		return;
	
	LIST_remove(&WINDOW_list, THIS, &THIS->list);
	SDL_HideWindow(WINDOW);
	THIS->opened = FALSE;
	GB.Unref(POINTER(&_object));
}

void WINDOW_handle_event(SDL_WindowEvent *event)
{
	SDL_Window *window;
	CWINDOW *_object;
	
	window = SDL_GetWindowFromID(event->windowID);
	_object = SDL_GetWindowData(window, "gambas-object");
	if (!THIS)
		return;
	
	switch(event->event)
	{
		case SDL_WINDOWEVENT_SHOWN:
			GB.Raise(THIS, EVENT_Show, 0);
			break;
		case SDL_WINDOWEVENT_HIDDEN:
			GB.Raise(THIS, EVENT_Hide, 0);
			break;
		/*case SDL_WINDOWEVENT_EXPOSED:*/
		case SDL_WINDOWEVENT_MOVED:
			THIS->x = event->data1;
			THIS->y = event->data2;
			GB.Raise(THIS, EVENT_Move, 0);
 			break;
		case SDL_WINDOWEVENT_RESIZED:
			THIS->width = event->data1;
			THIS->height = event->data2;
			GB.Raise(THIS, EVENT_Resize, 0);
			break;
		/*case SDL_WINDOWEVENT_MINIMIZED:
				SDL_Log("Window %d minimized", event->window.windowID);
				break;
		case SDL_WINDOWEVENT_MAXIMIZED:
				SDL_Log("Window %d maximized", event->window.windowID);
				break;
		case SDL_WINDOWEVENT_RESTORED:
				SDL_Log("Window %d restored", event->window.windowID);
				break;*/
		case SDL_WINDOWEVENT_ENTER:
			GB.Raise(THIS, EVENT_Enter, 0);
			break;
		case SDL_WINDOWEVENT_LEAVE:
			GB.Raise(THIS, EVENT_Leave, 0);
			break;
		case SDL_WINDOWEVENT_FOCUS_GAINED:
			GB.Raise(THIS, EVENT_GotFocus, 0);
			break;
		case SDL_WINDOWEVENT_FOCUS_LOST:
			GB.Raise(THIS, EVENT_LostFocus, 0);
			break;
		case SDL_WINDOWEVENT_CLOSE:
			close_window(THIS);
			break;
	}
}

void WINDOW_update(void)
{
	CWINDOW *_object;
	uint current_time;
	uint diff;
	bool at_least_one = FALSE;
	
	current_time = SDL_GetTicks();
	
	LIST_for_each(_object, WINDOW_list)
	{
		if (!GB.CanRaise(THIS, EVENT_Draw))
			continue;
		
		if (THIS->frame_time > 0)
		{
			double d = THIS->last_time + THIS->frame_time;
			if (d > current_time)
				continue;
			THIS->last_time = d;
		}
		
		DRAW_begin(THIS);
		GB.Raise(THIS, EVENT_Draw, 0);
		DRAW_end();
		//if (!cancel)
		//SDL_RenderPresent(THIS->renderer);
		//SDL_UpdateWindowSurface(WINDOW);

		THIS->frame_count++;
		THIS->total_frame_count++;
		
		if (THIS->start_time == 0)
			THIS->start_time = current_time;
		else
		{
			diff = current_time - THIS->start_time;
			if (diff > 1000)
			{
				THIS->frame_rate = THIS->frame_count;
				THIS->frame_count = 0;
				THIS->start_time += 1000;
			}
		}
		
		at_least_one = TRUE;
	}
	
	if (!at_least_one)
		SDL_Delay(1);
}

//-------------------------------------------------------------------------

BEGIN_METHOD(Window_new, GB_BOOLEAN opengl)

	THIS->opengl = VARGOPT(opengl, FALSE);
	THIS->fullscreen = FALSE;
	THIS->width = 640;
	THIS->height = 400;
	
	if (SDL_CreateWindowAndRenderer(0, 0, SDL_WINDOW_HIDDEN, &THIS->window, &THIS->renderer))
	{
		RAISE_ERROR("Unable to create window");
		return;
	}
	
	SDL_SetWindowData(THIS->window, "gambas-object", THIS);
	
END_METHOD

BEGIN_METHOD_VOID(Window_free)

	SDL_DestroyRenderer(THIS->renderer);
	SDL_DestroyWindow(WINDOW);

END_METHOD

BEGIN_METHOD_VOID(Window_Show)

	open_window(THIS);

END_METHOD

BEGIN_METHOD_VOID(Window_Hide)

	SDL_HideWindow(WINDOW);

END_METHOD

BEGIN_METHOD_VOID(Window_Close)

	close_window(THIS);

END_METHOD

BEGIN_PROPERTY(Window_Visible)

	if (READ_PROPERTY)
		GB.ReturnBoolean(SDL_GetWindowFlags(WINDOW) & SDL_WINDOW_SHOWN);
	else
	{
		if (VPROP(GB_BOOLEAN))
			open_window(THIS);
		else
			SDL_HideWindow(WINDOW);
	}

END_PROPERTY

BEGIN_METHOD(Window_Move, GB_INTEGER x; GB_INTEGER y; GB_INTEGER width; GB_INTEGER height)

	int w = VARGOPT(width, -1);
	int h = VARGOPT(height, -1);
	
	THIS->x = VARG(x);
	THIS->y = VARG(y);
	if (w > 0) THIS->width = w;
	if (h > 0) THIS->height = h;
	
	update_geometry(THIS);

END_METHOD

BEGIN_METHOD(Window_Resize, GB_INTEGER width; GB_INTEGER height)

	int w = VARG(width);
	int h = VARG(height);
	
	if (w > 0) THIS->width = w;
	if (h > 0) THIS->height = h;
	
	update_geometry(THIS);

END_METHOD

BEGIN_PROPERTY(Window_X)

	GB.ReturnInteger(THIS->x);
	
END_PROPERTY

BEGIN_PROPERTY(Window_Y)

	GB.ReturnInteger(THIS->y);
	
END_PROPERTY

BEGIN_PROPERTY(Window_Width)

	GB.ReturnInteger(THIS->width);
	
END_PROPERTY

BEGIN_PROPERTY(Window_Height)

	GB.ReturnInteger(THIS->height);
	
END_PROPERTY

BEGIN_PROPERTY(Window_FullScreen)

	if (READ_PROPERTY)
		GB.ReturnBoolean(THIS->fullscreen);
	else
	{
		THIS->fullscreen = VPROP(GB_BOOLEAN);
		update_geometry(THIS);
	}

END_PROPERTY

BEGIN_PROPERTY(Window_FrameRate)

	if (READ_PROPERTY)
		GB.ReturnFloat(THIS->frame_rate);
	else
	{
		double val = VPROP(GB_FLOAT);

		if (val < 0)
			return;

		THIS->frame_time = val ? 1000.0 / val : 0;
		THIS->last_time = SDL_GetTicks();
	}

END_PROPERTY

BEGIN_PROPERTY(Window_FrameCount)

	GB.ReturnInteger(THIS->total_frame_count);

END_PROPERTY

BEGIN_PROPERTY(Window_Text)

	if (READ_PROPERTY)
		GB.ReturnNewZeroString(SDL_GetWindowTitle(WINDOW));
	else
		SDL_SetWindowTitle(WINDOW, GB.ToZeroString(PROP(GB_STRING)));

END_PROPERTY

//-------------------------------------------------------------------------

GB_DESC WindowDesc[] =
{
	GB_DECLARE("Window", sizeof(CWINDOW)),
	
	GB_METHOD("_new", NULL, Window_new, "[(OpenGL)b]"),
	GB_METHOD("_free", NULL, Window_free, NULL),

	GB_METHOD("Show", NULL, Window_Show, NULL),
	GB_METHOD("Hide", NULL, Window_Hide, NULL),
	GB_METHOD("Close", NULL, Window_Close, NULL),
	GB_METHOD("Move", NULL, Window_Move, "(X)i(Y)i[(Width)i(Height)i]"),
	GB_METHOD("Resize", NULL, Window_Resize, "(Width)i(Height)i"),
	
	GB_PROPERTY("Visible", "b", Window_Visible),
	GB_PROPERTY("FullScreen", "b", Window_FullScreen),
	
	GB_PROPERTY_READ("X", "i", Window_X),
	GB_PROPERTY_READ("Y", "i", Window_Y),
	GB_PROPERTY_READ("W", "i", Window_Width),
	GB_PROPERTY_READ("H", "i", Window_Height),
	GB_PROPERTY_READ("Width", "i", Window_Width),
	GB_PROPERTY_READ("Height", "i", Window_Height),
	
	GB_PROPERTY("FrameRate", "f", Window_FrameRate),
	GB_PROPERTY_READ("FrameCount", "i", Window_FrameCount),
	
	GB_PROPERTY("Text", "s", Window_Text),
	GB_PROPERTY("Title", "s", Window_Text),
	
	GB_EVENT("Open", NULL, NULL, &EVENT_Open),
	GB_EVENT("Close", NULL, NULL, &EVENT_Close),
	GB_EVENT("Show", NULL, NULL, &EVENT_Show),
	GB_EVENT("Hide", NULL, NULL, &EVENT_Hide),
	GB_EVENT("Move", NULL, NULL, &EVENT_Move),
	GB_EVENT("Resize", NULL, NULL, &EVENT_Resize),
	GB_EVENT("Enter", NULL, NULL, &EVENT_Enter),
	GB_EVENT("Leave", NULL, NULL, &EVENT_Leave),
	GB_EVENT("GotFocus", NULL, NULL, &EVENT_GotFocus),
	GB_EVENT("LostFocus", NULL, NULL, &EVENT_LostFocus),
	GB_EVENT("Draw", NULL, NULL, &EVENT_Draw),
	
/*	GB_METHOD("_new", NULL, CWINDOW_new, "[(OpenGL)b]"),
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
	GB_EVENT("Open", NULL, NULL, &EVENT_Open),*/

	GB_END_DECLARE
};
