/***************************************************************************

  CKey.cpp

  (c) 2000-2013 Benoît Minisini <gambas@users.sourceforge.net>

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
#include "CKey.h"

CKEY_INFO CKEY_info = { 0 };

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
  if (!CKEY_is_valid()) \
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
	
	/*switch(CKEY_info.code)
	{
		case Qt::Key_Shift:
		case Qt::Key_Control:
		case Qt::Key_Alt:
		case Qt::Key_Meta:
			GB.ReturnInteger(0);
			break;
		
		default:*/
			GB.ReturnInteger(CKEY_info.code);
	//}

END_PROPERTY

BEGIN_PROPERTY(CKEY_state)

  CHECK_VALID();
  GB.ReturnInteger(CKEY_info.state);

END_PROPERTY

BEGIN_PROPERTY(CKEY_shift)

  CHECK_VALID();
  GB.ReturnBoolean(CKEY_info.state & Qt::ShiftModifier); // || (CKEY_info.code == Qt::Key_Shift));

END_PROPERTY

BEGIN_PROPERTY(CKEY_control)

  CHECK_VALID();
  GB.ReturnBoolean(CKEY_info.state & Qt::ControlModifier); // || (CKEY_info.code == Qt::Key_Control));

END_PROPERTY

BEGIN_PROPERTY(CKEY_alt)

  CHECK_VALID();
  GB.ReturnBoolean(CKEY_info.state & Qt::AltModifier); // || (CKEY_info.code == Qt::Key_Alt));

END_PROPERTY

BEGIN_PROPERTY(CKEY_meta)

  CHECK_VALID();
  GB.ReturnBoolean(CKEY_info.state & Qt::MetaModifier); // || (CKEY_info.code == Qt::Key_Meta));

END_PROPERTY

BEGIN_PROPERTY(CKEY_normal)

  CHECK_VALID();
  GB.ReturnBoolean((CKEY_info.state & (Qt::ShiftModifier | Qt::ControlModifier | Qt::AltModifier | Qt::MetaModifier)) == 0);

END_PROPERTY


GB_DESC CKeyDesc[] =
{
  GB_DECLARE("Key", 0), GB_VIRTUAL_CLASS(),

  GB_STATIC_METHOD("_get", "i", CKEY_get, "(Key)s"),
  GB_STATIC_METHOD("_exit", NULL, CKEY_exit, NULL),

  GB_CONSTANT("Esc", "i", Qt::Key_Escape),
  GB_CONSTANT("Escape", "i", Qt::Key_Escape),
  GB_CONSTANT("Tab", "i", Qt::Key_Tab),
  GB_CONSTANT("BackTab", "i", Qt::Key_Backtab),
  GB_CONSTANT("BackSpace", "i", Qt::Key_Backspace),
  GB_CONSTANT("Return", "i", Qt::Key_Return),
  GB_CONSTANT("Enter", "i", Qt::Key_Enter),
  GB_CONSTANT("Ins", "i", Qt::Key_Insert),
  GB_CONSTANT("Del", "i", Qt::Key_Delete),
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
  GB_CONSTANT("PgUp", "i", Qt::Key_PageUp),
  GB_CONSTANT("PgDown", "i", Qt::Key_PageDown),
  GB_CONSTANT("PageUp", "i", Qt::Key_PageUp),
  GB_CONSTANT("PageDown", "i", Qt::Key_PageDown),
  GB_CONSTANT("ShiftKey", "i", Qt::Key_Shift),
  GB_CONSTANT("ControlKey", "i", Qt::Key_Control),
  GB_CONSTANT("MetaKey", "i", Qt::Key_Meta),
  GB_CONSTANT("AltKey", "i", Qt::Key_Alt),
  GB_CONSTANT("AltGrKey", "i", Qt::Key_AltGr),
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

