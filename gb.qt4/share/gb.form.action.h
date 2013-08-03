/***************************************************************************

  gb.form.action.h

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

#define __GB_FORM_ACTION_H

/***************************************************************************

#define HAS_ACTION(_widget)
If a widget has the action flag set.

#define SET_ACTION(_widget, _flag)
Sets or clears the action flag.

***************************************************************************/

static GB_FUNCTION _action_register_func;
static GB_FUNCTION _action_raise_func;
//static GB_FUNCTION _action_get_func;

static void init_action()
{
	static bool init = false;
	void *klass;

	if (init)
		return;

	klass = (void *)GB.FindClass("Action");
	GB.GetFunction(&_action_register_func, klass, "_Register", "oss", "");
	GB.GetFunction(&_action_raise_func, klass, "Raise", "o", "");
	//GB.GetFunction(&_action_get_func, klass, "Get", "o", "s");

	init = true;
}

void CACTION_register(void *control, const char *old, const char *key)
{
	//qDebug("CACTION_register: (%s %p) %s", GB.GetClassName(control), control, key);
	//fprintf(stderr, "CACTION_register: (%s %p %p) %s\n", GB.GetClassName(control), control, ((CWIDGET *)control)->widget, key);
	
	if ((!key || !*key) && !HAS_ACTION(control)) 
		//&& !GB.Is(control, CLASS_UserControl) && !GB.Is(control, CLASS_UserContainer))
		return;

	init_action();

	GB.Push(3,
		GB_T_OBJECT, control,
		GB_T_STRING, old, 0,
		GB_T_STRING, key, 0);

	// The register function must not raise an error, otherwise bad things may happen
	GB.Call(&_action_register_func, 3, true);

	SET_ACTION(control, key && *key);
}

void CACTION_raise(void *control)
{
	init_action();

	//qDebug("CACTION_raise: (%s %p)", GB.GetClassName(THIS), THIS);

	if (!HAS_ACTION(control))
		return;

	GB.Push(1, GB_T_OBJECT, control);
	GB.Call(&_action_raise_func, 1, true);
}
