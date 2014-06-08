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


BEGIN_METHOD(Key_get, GB_STRING key)

	char *key = GB.ToZeroString(ARG(key));

	if (GB.GetProperty((void *)GB.FindClass("Key"), key))
	{
		GB.Error(NULL);
		GB.ReturnInteger(gKey::fromString(GB.ToZeroString(ARG(key))));
	}

END_METHOD

#define CHECK_VALID() \
  if (gKey::valid() == 0) \
  { \
    GB.Error("No keyboard event data"); \
    return; \
  }

BEGIN_PROPERTY(Key_Text)

  CHECK_VALID();
  GB.ReturnNewZeroString(gKey::text());

END_PROPERTY

BEGIN_PROPERTY(Key_Code)

  CHECK_VALID();
  GB.ReturnInteger(gKey::code());

END_PROPERTY

BEGIN_PROPERTY(Key_State)

  CHECK_VALID();
  GB.ReturnInteger(gKey::state());

END_PROPERTY

BEGIN_PROPERTY(Key_Shift)

  CHECK_VALID();
  GB.ReturnBoolean(gKey::shift());

END_PROPERTY

BEGIN_PROPERTY(Key_Control)

  CHECK_VALID();
  GB.ReturnBoolean(gKey::control());

END_PROPERTY

BEGIN_PROPERTY(Key_Alt)

  CHECK_VALID();
  GB.ReturnBoolean(gKey::alt());

END_PROPERTY

BEGIN_PROPERTY(Key_Meta)

  CHECK_VALID();
  GB.ReturnBoolean(gKey::meta());

END_PROPERTY

BEGIN_PROPERTY(Key_Normal)

  CHECK_VALID();
  GB.ReturnBoolean(gKey::normal());

END_PROPERTY



GB_DESC CKeyDesc[] =
{
  GB_DECLARE("Key", 0), GB_VIRTUAL_CLASS(),

  GB_STATIC_METHOD("_get", "i", Key_get, "(Key)s"),
 
  GB_CONSTANT("Esc", "i", GDK_Escape),
  GB_CONSTANT("Escape", "i", GDK_Escape),
  GB_CONSTANT("Tab", "i", GDK_Tab),
  GB_CONSTANT("BackTab", "i", GDK_ISO_Left_Tab),
  GB_CONSTANT("BackSpace", "i", GDK_BackSpace),
  GB_CONSTANT("Return", "i", GDK_Return),
  GB_CONSTANT("Enter", "i", GDK_KP_Enter),
  GB_CONSTANT("Ins", "i", GDK_Insert),
  GB_CONSTANT("Del", "i", GDK_Delete),
  GB_CONSTANT("Insert", "i", GDK_Insert),
  GB_CONSTANT("Delete", "i", GDK_Delete),
  GB_CONSTANT("Pause", "i", GDK_Pause),
  GB_CONSTANT("Print", "i", GDK_Print),
  GB_CONSTANT("SysReq", "i", GDK_Sys_Req),
  GB_CONSTANT("Home", "i", GDK_Home),
  GB_CONSTANT("End", "i", GDK_End),
  GB_CONSTANT("Left", "i", GDK_Left),
  GB_CONSTANT("Up", "i", GDK_Up),
  GB_CONSTANT("Right", "i", GDK_Right),
  GB_CONSTANT("Down", "i", GDK_Down),
  GB_CONSTANT("PgUp", "i", GDK_Page_Up),
  GB_CONSTANT("PgDown", "i", GDK_Page_Down),
  GB_CONSTANT("PageUp", "i", GDK_Page_Up),
  GB_CONSTANT("PageDown", "i", GDK_Page_Down),
  GB_CONSTANT("ShiftKey", "i", GDK_Shift_L),
  GB_CONSTANT("ControlKey", "i", GDK_Control_L),
  GB_CONSTANT("MetaKey", "i", GDK_Meta_L),
  GB_CONSTANT("AltKey", "i", GDK_Alt_L),
  GB_CONSTANT("CapsLock", "i", GDK_Caps_Lock),
  GB_CONSTANT("NumLock", "i", GDK_Num_Lock),
  GB_CONSTANT("ScrollLock", "i", GDK_Scroll_Lock),
  GB_CONSTANT("F1", "i", GDK_F1),
  GB_CONSTANT("F2", "i", GDK_F2),
  GB_CONSTANT("F3", "i", GDK_F3),
  GB_CONSTANT("F4", "i", GDK_F4),
  GB_CONSTANT("F5", "i", GDK_F5),
  GB_CONSTANT("F6", "i", GDK_F6),
  GB_CONSTANT("F7", "i", GDK_F7),
  GB_CONSTANT("F8", "i", GDK_F8),
  GB_CONSTANT("F9", "i", GDK_F9),
  GB_CONSTANT("F10", "i", GDK_F10),
  GB_CONSTANT("F11", "i", GDK_F11),
  GB_CONSTANT("F12", "i", GDK_F12),
  GB_CONSTANT("F13", "i", GDK_F13),
  GB_CONSTANT("F14", "i", GDK_F14),
  GB_CONSTANT("F15", "i", GDK_F15),
  GB_CONSTANT("F16", "i", GDK_F16),
  GB_CONSTANT("F17", "i", GDK_F17),
  GB_CONSTANT("F18", "i", GDK_F18),
  GB_CONSTANT("F19", "i", GDK_F19),
  GB_CONSTANT("F20", "i", GDK_F20),
  GB_CONSTANT("F21", "i", GDK_F21),
  GB_CONSTANT("F22", "i", GDK_F22),
  GB_CONSTANT("F23", "i", GDK_F23),
  GB_CONSTANT("F24", "i", GDK_F24),
  GB_CONSTANT("Menu", "i", GDK_Menu),
  GB_CONSTANT("Help", "i", GDK_Help),
  GB_CONSTANT("Space", "i", GDK_space),

  GB_STATIC_PROPERTY_READ("Text", "s", Key_Text),
  GB_STATIC_PROPERTY_READ("Code", "i", Key_Code),
  GB_STATIC_PROPERTY_READ("State", "i", Key_State),
  GB_STATIC_PROPERTY_READ("Shift", "b", Key_Shift),
  GB_STATIC_PROPERTY_READ("Control", "b", Key_Control),
  GB_STATIC_PROPERTY_READ("Alt", "b", Key_Alt),
  GB_STATIC_PROPERTY_READ("Meta", "b", Key_Meta),
  GB_STATIC_PROPERTY_READ("Normal", "b", Key_Normal),

  GB_END_DECLARE
};

