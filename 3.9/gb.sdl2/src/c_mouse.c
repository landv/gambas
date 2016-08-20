/***************************************************************************

  c_mouse.c

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

#define __C_MOUSE_C

#include "c_window.h"
#include "c_mouse.h"

static SDL_Event *_current = NULL;
static MOUSE_INFO *_info = NULL;

//-------------------------------------------------------------------------

static void update_event()
{
	MOUSE_INFO *info;
	CWINDOW *window;

	_info = NULL;

	if (!_current)
		return;

	window = WINDOW_get_from_event(_current);
	if (!window)
		return;

	_info = info = &window->mouse;

	switch (_current->type)
	{
		case SDL_MOUSEMOTION:
			info->x = _current->motion.x;
			info->y = _current->motion.y;
			info->wheel_x = 0;
			info->wheel_y = 0;
			info->state = _current->motion.state;
			info->button = 0;
			break;

		case SDL_MOUSEWHEEL:
			info->wheel_x = _current->wheel.x;
			info->wheel_y = _current->wheel.y;
			info->state = SDL_GetMouseState(&info->x, &info->y);
			info->button = 0;
#if SDL_VERSION_ATLEAST(2,0,4)
			if (_current->wheel.direction == SDL_MOUSEWHEEL_FLIPPED)
			{
				info->wheel_x = (- info->wheel_x);
				info->wheel_y = (- info->wheel_y);
			}
#endif
			break;

		case SDL_MOUSEBUTTONDOWN:
			info->x = _current->button.x;
			info->y = _current->button.y;
			info->wheel_x = 0;
			info->wheel_y = 0;
			info->state = SDL_GetMouseState(NULL, NULL);
			info->button = _current->button.button;
			info->start_x = info->x;
			info->start_y = info->y;
			break;

		case SDL_MOUSEBUTTONUP:
			info->x = _current->button.x;
			info->y = _current->button.y;
			info->wheel_x = 0;
			info->wheel_y = 0;
			info->state = SDL_GetMouseState(NULL, NULL);
			info->button = _current->button.button;
			break;
	}
}

SDL_Event *MOUSE_enter_event(SDL_Event *event)
{
	SDL_Event *old = _current;
	_current = event;
	update_event();
	return old;
}

void MOUSE_leave_event(SDL_Event *event)
{
	_current = event;
	update_event();
}

static bool check_event(void)
{
	if (!_info)
	{
		GB.Error("No mouse event");
		return TRUE;
	}
	else
		return FALSE;
}

#define CHECK_EVENT() if (check_event()) return

//-------------------------------------------------------------------------

BEGIN_PROPERTY(Mouse_X)

	CHECK_EVENT();
	GB.ReturnInteger(_info->x);

END_PROPERTY

BEGIN_PROPERTY(Mouse_Y)

	CHECK_EVENT();
	GB.ReturnInteger(_info->y);

END_PROPERTY

BEGIN_PROPERTY(Mouse_WheelX)

	CHECK_EVENT();
	GB.ReturnInteger(_info->wheel_x);

END_PROPERTY

BEGIN_PROPERTY(Mouse_WheelY)

	CHECK_EVENT();
	GB.ReturnInteger(_info->wheel_y);

END_PROPERTY

BEGIN_PROPERTY(Mouse_StartX)

	GB.ReturnInteger(_info->start_x);

END_PROPERTY

BEGIN_PROPERTY(Mouse_StartY)

	GB.ReturnInteger(_info->start_y);

END_PROPERTY

BEGIN_PROPERTY(Mouse_Left)

	CHECK_EVENT();
	GB.ReturnBoolean(_info->button ? _info->button == SDL_BUTTON_LEFT : _info->state & SDL_BUTTON_LMASK);

END_PROPERTY

BEGIN_PROPERTY(Mouse_Middle)

	CHECK_EVENT();
	GB.ReturnBoolean(_info->button ? _info->button == SDL_BUTTON_MIDDLE : _info->state & SDL_BUTTON_MMASK);

END_PROPERTY

BEGIN_PROPERTY(Mouse_Right)

	CHECK_EVENT();
	GB.ReturnBoolean(_info->button ? _info->button == SDL_BUTTON_RIGHT : _info->state & SDL_BUTTON_RMASK);

END_PROPERTY

BEGIN_PROPERTY(Mouse_Button)

	CHECK_EVENT();
	GB.ReturnBoolean(_info->state);

END_PROPERTY

#if 0
BEGIN_METHOD(Mouse_Move, GB_INTEGER x; GB_INTEGER y; GB_OBJECT window)

	CWINDOW *window = (CWINDOW *)VARGOPT(window, NULL);

	if (!window)
	{
#if SDL_VERSION_ATLEAST(2,0,4)
		SDL_WarpMouseGlobal(VARG(x), VARG(y));
#else
		fprintf(stderr, "gb.sdl2: warning: global mouse warp is not supported.\n");
#endif
	}
	else
		SDL_WarpMouseInWindow(window->window, VARG(x), VARG(y));

END_METHOD
#endif

BEGIN_METHOD_VOID(Mouse_Show)

	SDL_ShowCursor(SDL_ENABLE);

END_METHOD

BEGIN_METHOD_VOID(Mouse_Hide)

	SDL_ShowCursor(SDL_DISABLE);

END_METHOD

BEGIN_PROPERTY(Mouse_Visible)

	if (READ_PROPERTY)
		GB.ReturnBoolean(SDL_ShowCursor(SDL_QUERY) == SDL_ENABLE);
	else
		SDL_ShowCursor(VPROP(GB_BOOLEAN) ? SDL_ENABLE : SDL_DISABLE);

END_PROPERTY

BEGIN_PROPERTY(Mouse_Relative)

	if (READ_PROPERTY)
		GB.ReturnBoolean(SDL_GetRelativeMouseMode());
	else
		SDL_SetRelativeMouseMode(VPROP(GB_BOOLEAN));

END_PROPERTY

//-------------------------------------------------------------------------

GB_DESC MouseDesc[] =
{
	GB_DECLARE_STATIC("Mouse"),

	//GB_STATIC_METHOD("Move", NULL, Mouse_Move, "(X)i(Y)i[(Window)Window]"),

	GB_STATIC_METHOD("Show", NULL, Mouse_Show, NULL),
	GB_STATIC_METHOD("Hide", NULL, Mouse_Hide, NULL),
	GB_STATIC_PROPERTY("Visible", "b", Mouse_Visible),
	GB_STATIC_PROPERTY("Relative", "b", Mouse_Relative),

	//GB_STATIC_PROPERTY_READ("ScreenX", "i", Mouse_ScreenX),
	//GB_STATIC_PROPERTY_READ("ScreenY", "i", Mouse_ScreenY),
	GB_STATIC_PROPERTY_READ("StartX", "i", Mouse_StartX),
	GB_STATIC_PROPERTY_READ("StartY", "i", Mouse_StartY),
	GB_STATIC_PROPERTY_READ("X", "i", Mouse_X),
	GB_STATIC_PROPERTY_READ("Y", "i", Mouse_Y),
	GB_STATIC_PROPERTY_READ("WheelX", "i", Mouse_WheelX),
	GB_STATIC_PROPERTY_READ("WheelY", "i", Mouse_WheelY),

	GB_STATIC_PROPERTY_READ("Left", "b", Mouse_Left),
	GB_STATIC_PROPERTY_READ("Right", "b", Mouse_Right),
	GB_STATIC_PROPERTY_READ("Middle", "b", Mouse_Middle),
	GB_STATIC_PROPERTY_READ("Button", "i", Mouse_Button),
	//GB_STATIC_PROPERTY_READ("Shift", "b", Mouse_Shift),
	//GB_STATIC_PROPERTY_READ("Control", "b", Mouse_Control),
	//GB_STATIC_PROPERTY_READ("Alt", "b", Mouse_Alt),
	//GB_STATIC_PROPERTY_READ("Meta", "b", Mouse_Meta),
	//GB_STATIC_PROPERTY_READ("Normal", "b", Mouse_Normal),

	GB_END_DECLARE
};
