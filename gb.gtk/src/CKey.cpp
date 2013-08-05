/***************************************************************************

  CKey.cpp

  (c) 2004-2006 - Daniel Campos Fernández <dcamposf@gmail.com>

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

#include "CKey.h"
#include <gdk/gdkkeysyms.h>
#include "gkey.h"


BEGIN_METHOD(CKEY_get, GB_STRING key)

	GB.ReturnInteger(gKey::fromString(STRING(key))); 

END_METHOD

#define CHECK_VALID() \
  if (gKey::valid() == 0) \
  { \
    GB.Error("No keyboard event data"); \
    return; \
  }

BEGIN_PROPERTY(CKEY_text)

  CHECK_VALID();
  GB.ReturnNewZeroString(gKey::text());

END_PROPERTY

BEGIN_PROPERTY(CKEY_code)

  CHECK_VALID();
  GB.ReturnInteger(gKey::code());

END_PROPERTY

BEGIN_PROPERTY(CKEY_state)

  CHECK_VALID();
  GB.ReturnInteger(gKey::state());

END_PROPERTY

BEGIN_PROPERTY(CKEY_shift)

  CHECK_VALID();
  GB.ReturnBoolean(gKey::shift());

END_PROPERTY

BEGIN_PROPERTY(CKEY_control)

  CHECK_VALID();
  GB.ReturnBoolean(gKey::control());

END_PROPERTY

BEGIN_PROPERTY(CKEY_alt)

  CHECK_VALID();
  GB.ReturnBoolean(gKey::alt());

END_PROPERTY

BEGIN_PROPERTY(CKEY_meta)

  CHECK_VALID();
  GB.ReturnBoolean(gKey::meta());

END_PROPERTY

BEGIN_PROPERTY(CKEY_normal)

  CHECK_VALID();
  GB.ReturnBoolean(gKey::normal());

END_PROPERTY



GB_DESC CKeyDesc[] =
{
  GB_DECLARE("Key", 0), GB_VIRTUAL_CLASS(),

  GB_STATIC_METHOD("_get", "i", CKEY_get, "(Key)s"),
 
  GB_CONSTANT("Esc", "i", GDK_KEY_Escape),
  GB_CONSTANT("Escape", "i", GDK_KEY_Escape),
  GB_CONSTANT("Tab", "i", GDK_KEY_Tab),
  GB_CONSTANT("BackTab", "i", GDK_KEY_ISO_Left_Tab),
  GB_CONSTANT("BackSpace", "i", GDK_KEY_BackSpace),
  GB_CONSTANT("Return", "i", GDK_KEY_Return),
  GB_CONSTANT("Enter", "i", GDK_KEY_KP_Enter),
  GB_CONSTANT("Insert", "i", GDK_KEY_Insert),
  GB_CONSTANT("Delete", "i", GDK_KEY_Delete),
  GB_CONSTANT("Pause", "i", GDK_KEY_Pause),
  GB_CONSTANT("Print", "i", GDK_KEY_Print),
  GB_CONSTANT("SysReq", "i", GDK_KEY_Sys_Req),
  GB_CONSTANT("Home", "i", GDK_KEY_Home),
  GB_CONSTANT("End", "i", GDK_KEY_End),
  GB_CONSTANT("Left", "i", GDK_KEY_Left),
  GB_CONSTANT("Up", "i", GDK_KEY_Up),
  GB_CONSTANT("Right", "i", GDK_KEY_Right),
  GB_CONSTANT("Down", "i", GDK_KEY_Down),
  GB_CONSTANT("PageUp", "i", GDK_KEY_Page_Up),
  GB_CONSTANT("PageDown", "i", GDK_KEY_Page_Down),
  GB_CONSTANT("Shift", "i", GDK_KEY_Shift_L),
  GB_CONSTANT("Control", "i", GDK_KEY_Control_L),
  GB_CONSTANT("Meta", "i", GDK_KEY_Meta_L),
  GB_CONSTANT("Alt", "i", GDK_KEY_Alt_L),
  GB_CONSTANT("CapsLock", "i", GDK_KEY_Caps_Lock),
  GB_CONSTANT("NumLock", "i", GDK_KEY_Num_Lock),
  GB_CONSTANT("ScrollLock", "i", GDK_KEY_Scroll_Lock),
  GB_CONSTANT("F1", "i", GDK_KEY_F1),
  GB_CONSTANT("F2", "i", GDK_KEY_F2),
  GB_CONSTANT("F3", "i", GDK_KEY_F3),
  GB_CONSTANT("F4", "i", GDK_KEY_F4),
  GB_CONSTANT("F5", "i", GDK_KEY_F5),
  GB_CONSTANT("F6", "i", GDK_KEY_F6),
  GB_CONSTANT("F7", "i", GDK_KEY_F7),
  GB_CONSTANT("F8", "i", GDK_KEY_F8),
  GB_CONSTANT("F9", "i", GDK_KEY_F9),
  GB_CONSTANT("F10", "i", GDK_KEY_F10),
  GB_CONSTANT("F11", "i", GDK_KEY_F11),
  GB_CONSTANT("F12", "i", GDK_KEY_F12),
  GB_CONSTANT("F13", "i", GDK_KEY_F13),
  GB_CONSTANT("F14", "i", GDK_KEY_F14),
  GB_CONSTANT("F15", "i", GDK_KEY_F15),
  GB_CONSTANT("F16", "i", GDK_KEY_F16),
  GB_CONSTANT("F17", "i", GDK_KEY_F17),
  GB_CONSTANT("F18", "i", GDK_KEY_F18),
  GB_CONSTANT("F19", "i", GDK_KEY_F19),
  GB_CONSTANT("F20", "i", GDK_KEY_F20),
  GB_CONSTANT("F21", "i", GDK_KEY_F21),
  GB_CONSTANT("F22", "i", GDK_KEY_F22),
  GB_CONSTANT("F23", "i", GDK_KEY_F23),
  GB_CONSTANT("F24", "i", GDK_KEY_F24),
  GB_CONSTANT("Menu", "i", GDK_KEY_Menu),
  GB_CONSTANT("Help", "i", GDK_KEY_Help),
  GB_CONSTANT("Space", "i", GDK_KEY_space),

  GB_STATIC_PROPERTY_READ("Text", "s", CKEY_text),
  GB_STATIC_PROPERTY_READ("Code", "i", CKEY_code),
  GB_STATIC_PROPERTY_READ("State", "i", CKEY_state),
  GB_STATIC_PROPERTY_READ("Shift", "b", CKEY_shift),
  GB_STATIC_PROPERTY_READ("Control", "b", CKEY_control),
  GB_STATIC_PROPERTY_READ("Alt", "b", CKEY_alt),
  GB_STATIC_PROPERTY_READ("Meta", "b", CKEY_meta),
  GB_STATIC_PROPERTY_READ("Normal", "b", CKEY_normal),

  GB_END_DECLARE
};

