/***************************************************************************

  Ckey.cpp

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

#define __CKEY_CPP

#include "gambas.h"
#include "main.h"

#include "Ckey.h"

#include "SDL.h"

CKEY_INFO CKEY_info = { 0 };

static bool _key_repeat = false;

#define CHECK_VALID() \
  if (CKEY_info.valid <= 0) \
  { \
    GB.Error("No keyboard event data"); \
    return; \
  }

/***************************************************************************/

BEGIN_METHOD(CKEY_get, GB_STRING key)

	char *key = GB.ToZeroString(ARG(key));
	int code = 0;

	if (key[0] && !key[1] && !(key[0] & 0x80))
	{
		GB.ReturnInteger(key[0]);
		return;
	}
	else
	{
		for (code = 1; code <= 255; code++)
		{
			if (!strcasecmp(SDL_GetKeyName((SDLKey)code), key))
			{
				GB.ReturnInteger(code);
				return;
			}
		}
	}
	
	GB.ReturnInteger(0);
	
END_METHOD

BEGIN_PROPERTY(CKEY_code)

	CHECK_VALID();
	GB.ReturnInteger(CKEY_info.code);

END_PROPERTY

BEGIN_PROPERTY(CKEY_state)

	CHECK_VALID();
	GB.ReturnInteger(CKEY_info.state);

END_PROPERTY

BEGIN_PROPERTY(CKEY_shift)

	CHECK_VALID();
	GB.ReturnBoolean(CKEY_info.state & KMOD_SHIFT);

END_PROPERTY

BEGIN_PROPERTY(CKEY_control)

	CHECK_VALID();
	GB.ReturnBoolean(CKEY_info.state & KMOD_CTRL);

END_PROPERTY

BEGIN_PROPERTY(CKEY_alt)

	CHECK_VALID();
	GB.ReturnBoolean(CKEY_info.state & KMOD_ALT);

END_PROPERTY

BEGIN_PROPERTY(CKEY_meta)

	CHECK_VALID();
	GB.ReturnBoolean(CKEY_info.state & KMOD_META);

END_PROPERTY

BEGIN_PROPERTY(CKEY_normal)

	CHECK_VALID();
	GB.ReturnBoolean(CKEY_info.state != 0);

END_PROPERTY

BEGIN_PROPERTY(CKEY_text)

	CHECK_VALID();
	GB.ReturnNewZeroString(CKEY_info.text);

END_PROPERTY

BEGIN_PROPERTY(Key_Repeat)

	if (READ_PROPERTY)
		GB.ReturnBoolean(_key_repeat);
	else
	{
		_key_repeat = VPROP(GB_BOOLEAN);
		SDL_EnableKeyRepeat(_key_repeat ? SDL_DEFAULT_REPEAT_DELAY : 0, SDL_DEFAULT_REPEAT_INTERVAL);
	}

END_PROPERTY

/***************************************************************************/

GB_DESC CKey[] =
{
  GB_DECLARE("Key", 0), GB_VIRTUAL_CLASS(),

	GB_STATIC_PROPERTY("Repeat", "b", Key_Repeat),

	GB_STATIC_METHOD("_get", "i", CKEY_get, "(Key)s"),

  GB_STATIC_PROPERTY_READ("Code", "i", CKEY_code),
  GB_STATIC_PROPERTY_READ("State", "i", CKEY_state),
  GB_STATIC_PROPERTY_READ("Shift", "b", CKEY_shift),
  GB_STATIC_PROPERTY_READ("Control", "b", CKEY_control),
  GB_STATIC_PROPERTY_READ("Alt", "b", CKEY_alt),
  GB_STATIC_PROPERTY_READ("Meta", "b", CKEY_meta),
  GB_STATIC_PROPERTY_READ("Normal", "b", CKEY_normal),
  GB_STATIC_PROPERTY_READ("Text", "s", CKEY_text),

	GB_CONSTANT("Backspace", "i", SDLK_BACKSPACE),
	GB_CONSTANT("Tab", "i", SDLK_TAB),
	//GB_CONSTANT("Clear", "i", SDLK_CLEAR		= 12,
	GB_CONSTANT("Return", "i", SDLK_RETURN),
	GB_CONSTANT("Pause", "i", SDLK_PAUSE),
	GB_CONSTANT("Escape", "i", SDLK_ESCAPE),
	GB_CONSTANT("Esc", "i", SDLK_ESCAPE),
	GB_CONSTANT("Space", "i", SDLK_SPACE),
	GB_CONSTANT("Delete", "i", SDLK_DELETE),
	GB_CONSTANT("KP0", "i", SDLK_KP0),
	GB_CONSTANT("KP1", "i", SDLK_KP1),
	GB_CONSTANT("KP2", "i", SDLK_KP2),
	GB_CONSTANT("KP3", "i", SDLK_KP3),
	GB_CONSTANT("KP4", "i", SDLK_KP4),
	GB_CONSTANT("KP5", "i", SDLK_KP5),
	GB_CONSTANT("KP6", "i", SDLK_KP6),
	GB_CONSTANT("KP7", "i", SDLK_KP7),
	GB_CONSTANT("KP8", "i", SDLK_KP8),
	GB_CONSTANT("KP9", "i", SDLK_KP9),
	GB_CONSTANT("KPPeriod", "i", SDLK_KP_PERIOD),
	GB_CONSTANT("KPDivide", "i", SDLK_KP_DIVIDE),
	GB_CONSTANT("KPMultiply", "i", SDLK_KP_MULTIPLY),
	GB_CONSTANT("KPMinus", "i", SDLK_KP_MINUS),
	GB_CONSTANT("KPPlus", "i", SDLK_KP_PLUS),
	GB_CONSTANT("KPEnter", "i", SDLK_KP_ENTER),
	GB_CONSTANT("KPEquals", "i", SDLK_KP_EQUALS),
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
	GB_CONSTANT("NumLock", "i", SDLK_NUMLOCK),
	GB_CONSTANT("CapsLock", "i", SDLK_CAPSLOCK),
	GB_CONSTANT("ScrollLock", "i", SDLK_SCROLLOCK),
	GB_CONSTANT("RightShift", "i", SDLK_RSHIFT),
	GB_CONSTANT("LeftShift", "i", SDLK_LSHIFT),
	GB_CONSTANT("RightControl", "i", SDLK_RCTRL),
	GB_CONSTANT("LeftControl", "i", SDLK_LCTRL),
	GB_CONSTANT("RightAlt", "i", SDLK_RALT),
	GB_CONSTANT("LeftAlt", "i", SDLK_LALT),
	GB_CONSTANT("RightMeta", "i", SDLK_RMETA),
	GB_CONSTANT("LeftMeta", "i", SDLK_LMETA),
	GB_CONSTANT("RightSuper", "i", SDLK_RSUPER),
	GB_CONSTANT("LeftSuper", "i", SDLK_LSUPER),
	GB_CONSTANT("AltGr", "i", SDLK_MODE),
	GB_CONSTANT("Compose", "i", SDLK_COMPOSE),
	GB_CONSTANT("Help", "i", SDLK_HELP),
	GB_CONSTANT("Print", "i", SDLK_PRINT),
	GB_CONSTANT("SysReq", "i", SDLK_SYSREQ),
	GB_CONSTANT("Break", "i", SDLK_BREAK),
	GB_CONSTANT("Menu", "i", SDLK_MENU),
	GB_CONSTANT("Power", "i", SDLK_POWER),
	GB_CONSTANT("Euro", "i", SDLK_EURO),
	GB_CONSTANT("Undo", "i", SDLK_UNDO),

  GB_END_DECLARE
};

