/***************************************************************************

  CKey.cpp

  Keyboard events management

  (c) 2000-2007 Benoît Minisini <gambas@users.sourceforge.net>

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

#define __CKEY_CPP


#include "gambas.h"

#include <qnamespace.h>
#include <qkeysequence.h>

#include "main.h"
#include "CKey.h"

CKEY_INFO CKEY_info = { 0 };

#if 0
typedef
  struct {
    const char *name;
    int key;
    }
  KEY_DEF;

KEY_DEF _key_dict[] =
{
  { "BackSpace", Qt::Key_BackSpace },
  { "BackTab", Qt::Key_BackTab },
  { "Control", Qt::Key_Control },
  { "Delete", Qt::Key_Delete },
  { "Down", Qt::Key_Down },
  { "End", Qt::Key_End },
  { "Enter", Qt::Key_Enter },
  { "Escape", Qt::Key_Escape },
  { "Home", Qt::Key_Home },
  { "Insert", Qt::Key_Insert },
  { "Left", Qt::Key_Left },
  { "PageDown", Qt::Key_PageDown },
  { "PageUp", Qt::Key_PageUp },
  { "Pause", Qt::Key_Pause },
  { "Print", Qt::Key_Print },
  { "Return", Qt::Key_Return },
  { "Right", Qt::Key_Right },
  { "Shift", Qt::Key_Shift },
  { "SysReq", Qt::Key_SysReq },
  { "tab", Qt::Key_Tab },
  { "Up", Qt::Key_Up },

  { "Meta", Qt::Key_Meta },
  { "Alt", Qt::Key_Alt },
  { "CapsLock", Qt::Key_CapsLock },
  { "NumLock", Qt::Key_NumLock },
  { "ScrollLock", Qt::Key_ScrollLock },
  { "F1", Qt::Key_F1 },
  { "F2", Qt::Key_F2 },
  { "F3", Qt::Key_F3 },
  { "F4", Qt::Key_F4 },
  { "F5", Qt::Key_F5 },
  { "F6", Qt::Key_F6 },
  { "F7", Qt::Key_F7 },
  { "F8", Qt::Key_F8 },
  { "F9", Qt::Key_F9 },
  { "F10", Qt::Key_F10 },
  { "F11", Qt::Key_F11 },
  { "F12", Qt::Key_F12 },
  { "F13", Qt::Key_F13 },
  { "F14", Qt::Key_F14 },
  { "F15", Qt::Key_F15 },
  { "F16", Qt::Key_F16 },
  { "F17", Qt::Key_F17 },
  { "F18", Qt::Key_F18 },
  { "F19", Qt::Key_F19 },
  { "F20", Qt::Key_F20 },
  { "F21", Qt::Key_F21 },
  { "F22", Qt::Key_F22 },
  { "F23", Qt::Key_F23 },
  { "F24", Qt::Key_F24 },
  { "Menu", Qt::Key_Menu },
  { "Help", Qt::Key_Help },
  { "Space", Qt::Key_Space },
  { NULL , 0 }
};
#endif

void CKEY_clear(int valid)
{
  if (valid)
    CKEY_info.valid++;
  else
    CKEY_info.valid--;

  if (CKEY_info.valid == 0)
  {
    GB.FreeString(&CKEY_info.text);
    CLEAR(&CKEY_info);
  }
}


BEGIN_METHOD_VOID(CKEY_exit)

  GB.FreeString(&CKEY_info.text);

END_METHOD


BEGIN_METHOD(CKEY_get, GB_STRING key)

  char *str = GB.ToZeroString(ARG(key));
  QKeySequence ks(str);

#if QT_VERSION <= 0x030005
  GB.ReturnInteger(ks);
#else
  GB.ReturnInteger(ks[0] & ~Qt::UNICODE_ACCEL);
#endif

END_METHOD

#define CHECK_VALID() \
  if (CKEY_info.valid == 0) \
  { \
    GB.Error("No keyboard event data"); \
    return; \
  }

BEGIN_PROPERTY(CKEY_text)

  CHECK_VALID();
  GB.ReturnString(CKEY_info.text);

END_PROPERTY

BEGIN_PROPERTY(CKEY_code)

  CHECK_VALID();
  GB.ReturnInteger(CKEY_info.code);

END_PROPERTY

BEGIN_PROPERTY(CKEY_state)

  CHECK_VALID();
  GB.ReturnInteger(CKEY_info.state);

END_PROPERTY

static bool get_state(int button, int key)
{
	if (CKEY_info.release)
		return ((CKEY_info.state & button) && CKEY_info.code != key);
	else
		return ((CKEY_info.state & button) || CKEY_info.code == key);
}

BEGIN_PROPERTY(CKEY_shift)

  CHECK_VALID();
  GB.ReturnBoolean(get_state(Qt::ShiftButton, Qt::Key_Shift));
  // (CKEY_info.state & Qt::ShiftButton) || (CKEY_info.code == Qt::Key_Shift));

END_PROPERTY

BEGIN_PROPERTY(CKEY_control)

  CHECK_VALID();
  GB.ReturnBoolean((CKEY_info.state & Qt::ControlButton) || (CKEY_info.code == Qt::Key_Control));

END_PROPERTY

BEGIN_PROPERTY(CKEY_alt)

  CHECK_VALID();
  GB.ReturnBoolean((CKEY_info.state & Qt::AltButton) || (CKEY_info.code == Qt::Key_Alt));

END_PROPERTY

BEGIN_PROPERTY(CKEY_meta)

  CHECK_VALID();
  GB.ReturnBoolean((CKEY_info.state & Qt::MetaButton) || (CKEY_info.code == Qt::Key_Meta));

END_PROPERTY

BEGIN_PROPERTY(CKEY_normal)

  CHECK_VALID();
  GB.ReturnBoolean((CKEY_info.state & Qt::KeyButtonMask) == 0);

END_PROPERTY



GB_DESC CKeyDesc[] =
{
  GB_DECLARE("Key", 0), GB_VIRTUAL_CLASS(),

  GB_STATIC_METHOD("_get", "i", CKEY_get, "(Key)s"),
  GB_STATIC_METHOD("_exit", NULL, CKEY_exit, NULL),

  GB_CONSTANT("Esc", "i", Qt::Key_Escape),
  GB_CONSTANT("Escape", "i", Qt::Key_Escape),
  GB_CONSTANT("Tab", "i", Qt::Key_Tab),
  GB_CONSTANT("BackTab", "i", Qt::Key_BackTab),
  GB_CONSTANT("BackSpace", "i", Qt::Key_BackSpace),
  GB_CONSTANT("Return", "i", Qt::Key_Return),
  GB_CONSTANT("Enter", "i", Qt::Key_Enter),
  GB_CONSTANT("Insert", "i", Qt::Key_Insert),
  GB_CONSTANT("Delete", "i", Qt::Key_Delete),
  GB_CONSTANT("Pause", "i", Qt::Key_Pause),
  GB_CONSTANT("Print", "i", Qt::Key_Print),
  GB_CONSTANT("SysReq", "i", Qt::Key_SysReq),
  GB_CONSTANT("Home", "i", Qt::Key_Home),
  GB_CONSTANT("End", "i", Qt::Key_End),
  GB_CONSTANT("Left", "i", Qt::Key_Left),
  GB_CONSTANT("Up", "i", Qt::Key_Up),
  GB_CONSTANT("Right", "i", Qt::Key_Right),
  GB_CONSTANT("Down", "i", Qt::Key_Down),
  GB_CONSTANT("PageUp", "i", Qt::Key_PageUp),
  GB_CONSTANT("PageDown", "i", Qt::Key_PageDown),
  //GB_CONSTANT("Shift", "i", Qt::Key_Shift),
  //GB_CONSTANT("Control", "i", Qt::Key_Control),
  //GB_CONSTANT("Meta", "i", Qt::Key_Meta),
  //GB_CONSTANT("Alt", "i", Qt::Key_Alt),
  GB_CONSTANT("CapsLock", "i", Qt::Key_CapsLock),
  GB_CONSTANT("NumLock", "i", Qt::Key_NumLock),
  GB_CONSTANT("ScrollLock", "i", Qt::Key_ScrollLock),
  GB_CONSTANT("F1", "i", Qt::Key_F1),
  GB_CONSTANT("F2", "i", Qt::Key_F2),
  GB_CONSTANT("F3", "i", Qt::Key_F3),
  GB_CONSTANT("F4", "i", Qt::Key_F4),
  GB_CONSTANT("F5", "i", Qt::Key_F5),
  GB_CONSTANT("F6", "i", Qt::Key_F6),
  GB_CONSTANT("F7", "i", Qt::Key_F7),
  GB_CONSTANT("F8", "i", Qt::Key_F8),
  GB_CONSTANT("F9", "i", Qt::Key_F9),
  GB_CONSTANT("F10", "i", Qt::Key_F10),
  GB_CONSTANT("F11", "i", Qt::Key_F11),
  GB_CONSTANT("F12", "i", Qt::Key_F12),
  GB_CONSTANT("F13", "i", Qt::Key_F13),
  GB_CONSTANT("F14", "i", Qt::Key_F14),
  GB_CONSTANT("F15", "i", Qt::Key_F15),
  GB_CONSTANT("F16", "i", Qt::Key_F16),
  GB_CONSTANT("F17", "i", Qt::Key_F17),
  GB_CONSTANT("F18", "i", Qt::Key_F18),
  GB_CONSTANT("F19", "i", Qt::Key_F19),
  GB_CONSTANT("F20", "i", Qt::Key_F20),
  GB_CONSTANT("F21", "i", Qt::Key_F21),
  GB_CONSTANT("F22", "i", Qt::Key_F22),
  GB_CONSTANT("F23", "i", Qt::Key_F23),
  GB_CONSTANT("F24", "i", Qt::Key_F24),
  GB_CONSTANT("Menu", "i", Qt::Key_Menu),
  GB_CONSTANT("Help", "i", Qt::Key_Help),
  GB_CONSTANT("Space", "i", Qt::Key_Space),

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

