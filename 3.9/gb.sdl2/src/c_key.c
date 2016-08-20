/***************************************************************************

  c_key.c

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

#define __C_KEY_C

#include "c_window.h"
#include "c_key.h"

static SDL_Event *_current = NULL;
static bool _text_event = FALSE;

//-------------------------------------------------------------------------

static void update_event()
{
	if (!_current)
		return;

	_text_event = _current->type == SDL_TEXTINPUT;
}

SDL_Event *KEY_enter_event(SDL_Event *event)
{
	SDL_Event *old = _current;
	_current = event;
	update_event();
	return old;
}

void KEY_leave_event(SDL_Event *event)
{
	_current = event;
	update_event();
}

static bool check_event(void)
{
	if (!_current)
	{
		GB.Error("No keyboard event");
		return TRUE;
	}
	else
		return FALSE;
}

#define CHECK_EVENT() if (check_event()) return

//-------------------------------------------------------------------------

BEGIN_METHOD(Key_get, GB_STRING key)

	char *key = GB.ToZeroString(ARG(key));
	int code = 0;

	if (*key)
	{
		if (!key[1] && ((uchar)key[0] < 127))
		{
			GB.ReturnInteger(key[0]);
			return;
		}
		else
		{
			for (code = 127; code <= 255; code++)
			{
				if (!strcasecmp(SDL_GetKeyName((SDL_Keycode)code), key))
				{
					GB.ReturnInteger(code);
					return;
				}
			}
		}
	}

	GB.ReturnInteger(0);

END_METHOD

BEGIN_PROPERTY(Key_Code)

	CHECK_EVENT();
	GB.ReturnInteger(_text_event ? 0 : _current->key.keysym.sym);

END_PROPERTY

BEGIN_PROPERTY(Key_Shift)

	CHECK_EVENT();
	GB.ReturnBoolean((_text_event ? SDL_GetModState() : _current->key.keysym.mod) & KMOD_SHIFT);

END_PROPERTY

BEGIN_PROPERTY(Key_Control)

	CHECK_EVENT();
	GB.ReturnBoolean((_text_event ? SDL_GetModState() : _current->key.keysym.mod) & KMOD_CTRL);

END_PROPERTY

BEGIN_PROPERTY(Key_Alt)

	CHECK_EVENT();
	GB.ReturnBoolean((_text_event ? SDL_GetModState() : _current->key.keysym.mod) & KMOD_ALT);

END_PROPERTY

BEGIN_PROPERTY(Key_AltGr)

	CHECK_EVENT();
	GB.ReturnBoolean((_text_event ? SDL_GetModState() : _current->key.keysym.mod) & KMOD_MODE);

END_PROPERTY

BEGIN_PROPERTY(Key_Meta)

	CHECK_EVENT();
	GB.ReturnBoolean((_text_event ? SDL_GetModState() : _current->key.keysym.mod) & KMOD_GUI);

END_PROPERTY

BEGIN_PROPERTY(Key_Normal)

	CHECK_EVENT();
	GB.ReturnBoolean(((_text_event ? SDL_GetModState() : _current->key.keysym.mod) & (KMOD_CTRL | KMOD_ALT | KMOD_MODE | KMOD_GUI)) == 0);

END_PROPERTY

BEGIN_PROPERTY(Key_Text)

	CHECK_EVENT();
	if (!_text_event)
		GB.ReturnNull();
	else
		GB.ReturnNewZeroString(_current->text.text);

END_PROPERTY

BEGIN_PROPERTY(Key_Repeat)

	CHECK_EVENT();
	GB.ReturnBoolean(_text_event ? FALSE : _current->key.repeat);

END_PROPERTY

//-------------------------------------------------------------------------

GB_DESC KeyDesc[] =
{
	GB_DECLARE_STATIC("Key"),
	
	GB_STATIC_METHOD("_get", "i", Key_get, "(Key)s"),

  GB_STATIC_PROPERTY_READ("Code", "i", Key_Code),
  GB_STATIC_PROPERTY_READ("Shift", "b", Key_Shift),
  GB_STATIC_PROPERTY_READ("Control", "b", Key_Control),
  GB_STATIC_PROPERTY_READ("Alt", "b", Key_Alt),
  GB_STATIC_PROPERTY_READ("AltGr", "b", Key_AltGr),
  GB_STATIC_PROPERTY_READ("Meta", "b", Key_Meta),
  GB_STATIC_PROPERTY_READ("Normal", "b", Key_Normal),
  GB_STATIC_PROPERTY_READ("Text", "s", Key_Text),
	GB_STATIC_PROPERTY("Repeat", "b", Key_Repeat),

	GB_CONSTANT("Backspace", "i", SDLK_BACKSPACE),
	GB_CONSTANT("Tab", "i", SDLK_TAB),
	GB_CONSTANT("Return", "i", SDLK_RETURN),
	GB_CONSTANT("Pause", "i", SDLK_PAUSE),
	GB_CONSTANT("Escape", "i", SDLK_ESCAPE),
	GB_CONSTANT("Esc", "i", SDLK_ESCAPE),
	GB_CONSTANT("Space", "i", SDLK_SPACE),
	GB_CONSTANT("Delete", "i", SDLK_DELETE),
	GB_CONSTANT("Num0", "i", SDLK_KP_0),
	GB_CONSTANT("Num1", "i", SDLK_KP_1),
	GB_CONSTANT("Num2", "i", SDLK_KP_2),
	GB_CONSTANT("Num3", "i", SDLK_KP_3),
	GB_CONSTANT("Num4", "i", SDLK_KP_4),
	GB_CONSTANT("Num5", "i", SDLK_KP_5),
	GB_CONSTANT("Num6", "i", SDLK_KP_6),
	GB_CONSTANT("Num7", "i", SDLK_KP_7),
	GB_CONSTANT("Num8", "i", SDLK_KP_8),
	GB_CONSTANT("Num9", "i", SDLK_KP_9),
	GB_CONSTANT("NumPeriod", "i", SDLK_KP_PERIOD),
	GB_CONSTANT("NumDivide", "i", SDLK_KP_DIVIDE),
	GB_CONSTANT("NumMultiply", "i", SDLK_KP_MULTIPLY),
	GB_CONSTANT("NumMinus", "i", SDLK_KP_MINUS),
	GB_CONSTANT("NumPlus", "i", SDLK_KP_PLUS),
	GB_CONSTANT("NumEnter", "i", SDLK_KP_ENTER),
	//GB_CONSTANT("NumEquals", "i", SDLK_KP_EQUALS),
	GB_CONSTANT("Up", "i", SDLK_UP),
	GB_CONSTANT("Down", "i", SDLK_DOWN),
	GB_CONSTANT("Right", "i", SDLK_RIGHT),
	GB_CONSTANT("Left", "i", SDLK_LEFT),
	GB_CONSTANT("Insert", "i", SDLK_INSERT),
	GB_CONSTANT("Home", "i", SDLK_HOME),
	GB_CONSTANT("End", "i", SDLK_END),
	GB_CONSTANT("PageUp", "i", SDLK_PAGEUP),
	GB_CONSTANT("PageDown", "i", SDLK_PAGEDOWN),
	GB_CONSTANT("F1", "i", SDLK_F1),
	GB_CONSTANT("F2", "i", SDLK_F2),
	GB_CONSTANT("F3", "i", SDLK_F3),
	GB_CONSTANT("F4", "i", SDLK_F4),
	GB_CONSTANT("F5", "i", SDLK_F5),
	GB_CONSTANT("F6", "i", SDLK_F6),
	GB_CONSTANT("F7", "i", SDLK_F7),
	GB_CONSTANT("F8", "i", SDLK_F8),
	GB_CONSTANT("F9", "i", SDLK_F9),
	GB_CONSTANT("F10", "i", SDLK_F10),
	GB_CONSTANT("F11", "i", SDLK_F11),
	GB_CONSTANT("F12", "i", SDLK_F12),
	GB_CONSTANT("F13", "i", SDLK_F13),
	GB_CONSTANT("F14", "i", SDLK_F14),
	GB_CONSTANT("F15", "i", SDLK_F15),
	GB_CONSTANT("NumLock", "i", SDLK_NUMLOCKCLEAR),
	GB_CONSTANT("CapsLock", "i", SDLK_CAPSLOCK),
	GB_CONSTANT("ScrollLock", "i", SDLK_SCROLLLOCK),
	GB_CONSTANT("RightShift", "i", SDLK_RSHIFT),
	GB_CONSTANT("LeftShift", "i", SDLK_LSHIFT),
	GB_CONSTANT("RightControl", "i", SDLK_RCTRL),
	GB_CONSTANT("LeftControl", "i", SDLK_LCTRL),
	GB_CONSTANT("RightAlt", "i", SDLK_RALT),
	GB_CONSTANT("LeftAlt", "i", SDLK_LALT),
	GB_CONSTANT("RightMeta", "i", SDLK_RGUI),
	GB_CONSTANT("LeftMeta", "i", SDLK_LGUI),
	GB_CONSTANT("AltGrKey", "i", SDLK_MODE),
	//GB_CONSTANT("Compose", "i", SDLK_COMPOSE),
	//GB_CONSTANT("Help", "i", SDLK_HELP),
	//GB_CONSTANT("Print", "i", SDLK_PRINT),
	GB_CONSTANT("SysReq", "i", SDLK_SYSREQ),
	//GB_CONSTANT("Break", "i", SDLK_BREAK),
	GB_CONSTANT("Menu", "i", SDLK_MENU),
	//GB_CONSTANT("Power", "i", SDLK_POWER),
	//GB_CONSTANT("Euro", "i", SDLK_EURO),
	//GB_CONSTANT("Undo", "i", SDLK_UNDO),

	GB_END_DECLARE
};
