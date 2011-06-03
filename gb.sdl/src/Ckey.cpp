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

#define CHECK_VALID() \
  if (UNLIKELY(CKEY_info.valid == NULL)) \
  { \
    GB.Error("No keyboard event data"); \
    return; \
  }

/***************************************************************************/

BEGIN_METHOD(CKEY_get, GB_STRING key)

	char *str = GB.ToZeroString(ARG(key));
	GB.ReturnInteger(XStringToKeysym(str));

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
	GB.ReturnBoolean(CKEY_info.state ? 0 : 1);

END_PROPERTY

BEGIN_PROPERTY(CKEY_text)

	CHECK_VALID();
	GB.ReturnNewZeroString((char *) XKeysymToString(CKEY_info.code));

END_PROPERTY

/***************************************************************************/

GB_DESC CKey[] =
{
  GB_DECLARE("Key", 0), GB_VIRTUAL_CLASS(),
  GB_STATIC_METHOD("_get", "i", CKEY_get, "(Key)s"),

  GB_STATIC_PROPERTY_READ("Code", "i", CKEY_code),
  GB_STATIC_PROPERTY_READ("State", "i", CKEY_state),
  GB_STATIC_PROPERTY_READ("Shift", "b", CKEY_shift),
  GB_STATIC_PROPERTY_READ("Control", "b", CKEY_control),
  GB_STATIC_PROPERTY_READ("Alt", "b", CKEY_alt),
  GB_STATIC_PROPERTY_READ("Meta", "b", CKEY_meta),
  GB_STATIC_PROPERTY_READ("Normal", "b", CKEY_normal),
  GB_STATIC_PROPERTY_READ("Text", "s", CKEY_text),

  GB_CONSTANT("Esc", "i", XK_Escape),
  GB_CONSTANT("Escape", "i", XK_Escape),
  GB_CONSTANT("Tab", "i", XK_KP_Tab),
//  GB_CONSTANT("BackTab", "i", Qt::Key_BackTab),
  GB_CONSTANT("BackSpace", "i", XK_BackSpace),
  GB_CONSTANT("Return", "i", XK_Return),
  GB_CONSTANT("Enter", "i", XK_KP_Enter),
  GB_CONSTANT("Insert", "i", XK_Insert),
  GB_CONSTANT("Delete", "i", XK_Delete),
  GB_CONSTANT("Pause", "i", XK_Pause),
  GB_CONSTANT("Print", "i", XK_Print),
  GB_CONSTANT("SysReq", "i", XK_Sys_Req),
  GB_CONSTANT("Home", "i", XK_Home),
  GB_CONSTANT("End", "i", XK_End),
  GB_CONSTANT("Left", "i", XK_Left),
  GB_CONSTANT("Up", "i", XK_Up),
  GB_CONSTANT("Right", "i", XK_Right),
  GB_CONSTANT("Down", "i", XK_Down),
  GB_CONSTANT("PageUp", "i", XK_Page_Up),
  GB_CONSTANT("PageDown", "i", XK_Page_Down),
  GB_CONSTANT("CapsLock", "i", XK_Caps_Lock),
  GB_CONSTANT("NumLock", "i", XK_Num_Lock),
  GB_CONSTANT("ScrollLock", "i", XK_Scroll_Lock),
  GB_CONSTANT("F1",  "i", XK_F1),
  GB_CONSTANT("F2",  "i", XK_F2),
  GB_CONSTANT("F3",  "i", XK_F3),
  GB_CONSTANT("F4",  "i", XK_F4),
  GB_CONSTANT("F5",  "i", XK_F5),
  GB_CONSTANT("F6",  "i", XK_F6),
  GB_CONSTANT("F7",  "i", XK_F7),
  GB_CONSTANT("F8",  "i", XK_F8),
  GB_CONSTANT("F9",  "i", XK_F9),
  GB_CONSTANT("F10", "i", XK_F10),
  GB_CONSTANT("F11", "i", XK_F11),
  GB_CONSTANT("F12", "i", XK_F12),
  GB_CONSTANT("F13", "i", XK_F13),
  GB_CONSTANT("F14", "i", XK_F14),
  GB_CONSTANT("F15", "i", XK_F15),
  GB_CONSTANT("F16", "i", XK_F16),
  GB_CONSTANT("F17", "i", XK_F17),
  GB_CONSTANT("F18", "i", XK_F18),
  GB_CONSTANT("F19", "i", XK_F19),
  GB_CONSTANT("F20",  "i", XK_F21),
  GB_CONSTANT("F21",  "i", XK_F22),
  GB_CONSTANT("F22",  "i", XK_F23),
  GB_CONSTANT("F23",  "i", XK_F24),
  GB_CONSTANT("F24",  "i", XK_F25),
  GB_CONSTANT("Menu", "i", XK_Menu),
  GB_CONSTANT("Help", "i", XK_Help),
  GB_CONSTANT("Space", "i", XK_space),

  GB_END_DECLARE
};

