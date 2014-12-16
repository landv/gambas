/***************************************************************************

  gkey.cpp

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

#define __GKEY_CPP

#include <ctype.h>
#include <time.h>
#include <unistd.h>

#include "widgets.h"
#include "gapplication.h"
#include "gtrayicon.h"
#include "gdesktop.h"
#include "gmainwindow.h"
#include "gkey.h"

//#define DEBUG_IM 1

/*************************************************************************

gKey

**************************************************************************/

bool gKey::_valid = false;
bool gKey::_canceled = false;
GdkEventKey gKey::_event;

static GtkIMContext *_im_context = NULL;
static gControl *_im_control = NULL;
static bool _im_no_commit = false;
static bool _no_input_method = false;
static GdkWindow *_im_window = NULL;
static signed char _im_state_required = -1;

//#define MAX_CODE 16
//static uint _key_code[MAX_CODE] = { 0 };


//char *_im_text = NULL;

const char *gKey::text()
{
	if (!_valid) 
		return 0;
	else
		return _event.string;
}

int gKey::code()
{
	if (!_valid)
		return 0;
	
	int code = _event.keyval;
	
	if (code >= GDK_a && code <= GDK_z)
		code += GDK_A - GDK_a;
	else if (code == GDK_Alt_R)
		code = GDK_Alt_L;
	else if (code == GDK_Control_R)
		code = GDK_Control_L;
	else if (code == GDK_Meta_R)
		code = GDK_Meta_L;
	else if (code == GDK_Shift_R)
		code = GDK_Shift_L;
	else
	{
		int unicode = gdk_keyval_to_unicode(code);
		if (unicode >= 32 && unicode < 127)
			code = unicode;
	}

	return code;
}

int gKey::state()
{
	if (!_valid)
		return 0;
	else
		return _event.state;
}

bool gKey::alt()
{
	return state() & GDK_MOD1_MASK; // || _event.keyval == GDK_Alt_L || _event.keyval == GDK_Alt_R;
}

bool gKey::control()
{
	return state() & GDK_CONTROL_MASK; // || _event.keyval == GDK_Control_L || _event.keyval == GDK_Control_R;
}

bool gKey::meta()
{
	return state() & GDK_META_MASK; // || _event.keyval == GDK_Meta_L || _event.keyval == GDK_Meta_R;
}

bool gKey::normal()
{
	return (state() & (GDK_MOD1_MASK | GDK_CONTROL_MASK | GDK_META_MASK | GDK_SHIFT_MASK)) == 0;
}

bool gKey::shift()
{
	return state() & GDK_SHIFT_MASK; // || _event.keyval == GDK_Shift_L || _event.keyval == GDK_Shift_R;
}

int gKey::fromString(char *str)
{
	char *lstr;
	int key;
	
	if (!str || !*str)
		return 0;
	
	lstr = g_ascii_strup(str, -1);
	key = gdk_keyval_from_name(lstr);
	g_free(lstr);
	if (key) return key;

	lstr = g_ascii_strdown(str, -1);
	key = gdk_keyval_from_name(lstr);
	g_free(lstr);
	if (key) return key;

	key = gdk_keyval_from_name(str);
	if (key) return key;

	if (!str[1] && isascii(str[0]))
		return str[0];
	else
		return 0;
}

void gKey::disable()
{
	if (!_valid)
		return;
		
	_valid = false;
	_event.keyval = 0;
	_event.state = 0;
	//g_free(_event.string);
}

bool gKey::enable(gControl *control, GdkEventKey *event)
{
	bool f = false;
	
	if (_valid)
		disable();

	_valid = true;
	_canceled = false;
	if (event)
	{
		//if (event->type == GDK_KEY_RELEASE)
			_im_no_commit = false;

		_event = *event;
		_event.window = _im_window;

		if (!_no_input_method && control == _im_control)
		{
			#if DEBUG_IM
			fprintf(stderr, "gKey::enable: %p flag = %d event->string = %d\n", event, (event->state & (1 << 25)) != 0, *event->string);
			#endif

			f = gtk_im_context_filter_keypress(_im_context, &_event);
			#if DEBUG_IM
			fprintf(stderr, "gKey::enable: %p filter -> %d\n", event, f);
			#endif
		}
	}

  return f || _canceled;
}

bool gKey::mustIgnoreEvent(GdkEventKey *e)
{
	if (_im_state_required < 0)
	{
		_im_state_required = (e->state & (1 << 25)) != 0;
		return false;
	}

	if (((e->state & (1 << 25)) != 0) == _im_state_required)
		return false;

#if DEBUG_IM
	fprintf(stderr, "ignore event\n");
#endif
	return true;
}

static void cb_im_commit(GtkIMContext *context, const char *str, gpointer pointer)
{
	bool disable = false;

	#if DEBUG_IM
	fprintf(stderr, "cb_im_commit: %s (_im_no_commit = %d)\n", str, _im_no_commit);
	#endif
	
	if (gKey::mustIgnoreEvent(&gKey::_event))
		return;

	if (!gKey::valid())
	{
		gKey::enable(_im_control, NULL);
		disable = true;
	}

	gKey::_canceled = gKey::raiseEvent(gEvent_KeyPress, _im_control, str);
#if DEBUG_IM
	fprintf(stderr, "cb_im_commit: canceled = %d\n", gKey::_canceled);
#endif

	if (disable)
		gKey::disable();

	_im_no_commit = true;
}

void gKey::init()
{
	GdkWindowAttr attr;

	attr.event_mask = GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE_MASK;
	attr.width = attr.height = 1;
	attr.wclass = GDK_INPUT_ONLY;
	attr.window_type = GDK_WINDOW_TOPLEVEL;

	_im_window = gdk_window_new(NULL, &attr, 0);

	_im_context = gtk_im_multicontext_new();
  g_signal_connect (_im_context, "commit", G_CALLBACK(cb_im_commit), NULL);
}

void gKey::exit()
{
	disable();
	g_object_unref(_im_context);
}

void gKey::setActiveControl(gControl *control)
{
	if (_im_control)
	{
		if (!_no_input_method)
		{
			#if DEBUG_IM
			fprintf(stderr, "gtk_im_context_focus_out\n");
			#endif
	  	gtk_im_context_set_client_window (_im_context, 0);
			gtk_im_context_focus_out(_im_context);
		}
		_im_control = NULL;
		_no_input_method = false;
	}
	
	if (control)
	{
		_im_control = control;
		_no_input_method = control->noInputMethod();
		
		if (!_no_input_method)
		{
	  	gtk_im_context_set_client_window (_im_context, _im_window); //gtk_widget_get_window(_im_control->widget));
			gtk_im_context_focus_in(_im_context);
			gtk_im_context_reset(_im_context);
			#if DEBUG_IM
			fprintf(stderr, "gtk_im_context_focus_in: %s\n", gtk_im_multicontext_get_context_id(GTK_IM_MULTICONTEXT(_im_context)));
			if (control->getClass() == Type_gTextBox)
			{
				char *method;
				g_object_get(GTK_ENTRY(control->widget), "im-module", &method, (char *)NULL);
				fprintf(stderr, "GtkEntry im-module: %s\n", method);
			}
			#endif
			//_im_state_required = -1;
		}

		//memset(_key_code, 0, sizeof(uint) * MAX_CODE);
	}
}

static bool raise_key_event_to_parent_window(gControl *control, int type)
{
	gMainWindow *win;

	while (control->parent())
	{
		win = control->parent()->window();
		if (win->onKeyEvent && win->canRaise(win, type))
		{
			//fprintf(stderr, "onKeyEvent: %d %p %s\n", type, win, win->name());
			if (win->onKeyEvent(win, type))
				return true;
		}

		control = win;
	}

	return false;
}

#if 0
static bool can_raise(GdkEventKey *event)
{
	int i;

	if (event->type == GDK_KEY_PRESS)
	{
		for (i = 0; i < MAX_CODE; i++)
		{
			if (event->keyval == _key_code[i])
				return false;
		}
		for (i = 0; i < MAX_CODE; i++)
		{
			if (!_key_code[i])
			{
				//fprintf(stderr, "store key %d\n", event->keyval);
				_key_code[i] = event->keyval;
				break;
			}
		}
		return true;
	}
	else
	{
		for (i = 0; i < MAX_CODE; i++)
		{
			if (event->keyval == _key_code[i])
			{
				//fprintf(stderr, "remove key %d\n", event->keyval);
				_key_code[i] = 0;
				return true;
			}
		}
		return false;
	}
}
#endif

bool gKey::raiseEvent(int type, gControl *control, const char *text)
{
	bool parent_got_it = false;
	bool cancel = false;

	if (text)
		_event.string = (gchar *)text;

	//if (!can_raise(&_event))
	//	return false;

__KEY_TRY_PROXY:

	if (!parent_got_it)
	{
		parent_got_it = true;

		if (gApplication::onKeyEvent)
			cancel = gApplication::onKeyEvent(type);

		if (!cancel)
			cancel = raise_key_event_to_parent_window(control, type);
	}

	if (!cancel && control->onKeyEvent && control->canRaise(control, type))
	{
		//fprintf(stderr, "gEvent_KeyPress on %p %s\n", control, control->name());
		//fprintf(stderr, "onKeyEvent: %p %d %p %s\n", event, type, control, control->name());
		cancel = control->onKeyEvent(control, type);
	}

	if (cancel)
		return true;

	if (control->_proxy_for)
	{
		control = control->_proxy_for;
		goto __KEY_TRY_PROXY;
	}

	return false;
}