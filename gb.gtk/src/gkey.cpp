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
int gKey::_last_key_press = 0;
int gKey::_last_key_release = 0;

static GtkIMContext *_im_context = NULL;
static char *_im_default_slave = NULL;
static bool _im_has_input_method = FALSE;
static gControl *_im_control = NULL;
static bool _im_no_commit = false;
static GdkWindow *_im_window = NULL;
static bool _im_is_xim = FALSE;
static bool _im_ignore_event = FALSE;
static bool _im_got_commit = FALSE;

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
		_im_no_commit = false;

		_event = *event;
		_event.window = _im_window;

		if (gKey::mustIgnoreEvent(event))
			return true;

		if (control == _im_control)
		{
			#if DEBUG_IM
			fprintf(stderr, "gKey::enable: [%p] flag = %d event->string = %d\n", event, (event->state & (1 << 25)) != 0, *event->string);
			#endif

#if 0
			if (_im_slave_is_xim)
			{
				/*if (_im_xim_abort == 2)
				{
					f = true;
					_im_xim_abort = 0;
				}
				else*/
				{
					GdkEventKey save = *event;
					if (save.string)
						save.string = g_strdup(save.string);

					f = gtk_im_context_filter_keypress(_im_context, event);
					*event = save;
					_im_xim_abort++;
				}
			}
			else
				f = gtk_im_context_filter_keypress(_im_context, event);
#endif

			if (!_im_has_input_method)
				f = gtk_im_context_filter_keypress(_im_context, event);

			#if DEBUG_IM
			fprintf(stderr, "gKey::enable: [%p] filter -> %d\n", event, f);
			#endif
		}
	}

  return f || _canceled;
}

bool gKey::mustIgnoreEvent(GdkEventKey *event)
{
	if (!_im_has_input_method)
		return false;
	else
		return (event->type == GDK_KEY_PRESS) && ((uchar)*event->string >= 32 || event->keyval == 0);
}

void gcb_im_commit(GtkIMContext *context, const char *str, gpointer pointer)
{
	bool disable = false;

	// Not called from a key press event!
	if (!_im_control)
		return;

	#if DEBUG_IM
	fprintf(stderr, "cb_im_commit: \"%s\"  _im_no_commit = %d  gKey::valid = %d\n", str, _im_no_commit, gKey::valid());
	#endif
	
	if (!gKey::valid())
	{
		gKey::enable(_im_control, NULL);
		gKey::_event.keyval = gKey::_last_key_press;
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

static gboolean hook_commit(GSignalInvocationHint *ihint, guint n_param_values, const GValue *param_values, gpointer data)
{
	_im_got_commit = TRUE;
	return true;
}

void gKey::init()
{
	GdkWindowAttr attr;

	attr.event_mask = GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE_MASK;
	attr.width = attr.height = 10;
	attr.wclass = GDK_INPUT_OUTPUT;
	attr.window_type = GDK_WINDOW_TOPLEVEL;

	_im_window = gdk_window_new(NULL, &attr, 0);

	_im_context = gtk_im_multicontext_new();
	gtk_im_context_set_client_window (_im_context, _im_window);

	_im_default_slave = g_strdup(gtk_im_multicontext_get_context_id(GTK_IM_MULTICONTEXT(_im_context)));

  g_signal_connect(_im_context, "commit", G_CALLBACK(gcb_im_commit), NULL);

	g_signal_add_emission_hook(g_signal_lookup("commit", GTK_TYPE_IM_CONTEXT), (GQuark)0, hook_commit, (gpointer)0, NULL);
}

void gKey::exit()
{
	disable();
	g_free(_im_default_slave);
	g_object_unref(_im_context);
}

void gKey::setActiveControl(gControl *control)
{
	const char *slave;

	if (_im_control)
	{
#if DEBUG_IM
		fprintf(stderr, "gtk_im_context_focus_out\n");
#endif
		if (!_im_has_input_method)
		{
			gtk_im_context_set_client_window (_im_context, 0);
			gtk_im_context_focus_out(_im_context);
		}

		_im_control = NULL;
	}
	
	if (control)
	{
		_im_control = control;
		
		if (!control->hasInputMethod())
		{
			_im_has_input_method = FALSE;
			gtk_im_context_set_client_window (_im_context, _im_window);
			gtk_im_context_focus_in(_im_context);
			gtk_im_context_reset(_im_context);
			//slave = gtk_im_multicontext_get_context_id(GTK_IM_MULTICONTEXT(_im_context));
			_im_is_xim = FALSE;
		}
		else
		{
			_im_has_input_method = TRUE;
			slave = gtk_im_multicontext_get_context_id(GTK_IM_MULTICONTEXT(control->getInputMethod()));
			_im_is_xim = slave && strcmp(slave, "xim") == 0;
		}

		_im_ignore_event = FALSE;

#if DEBUG_IM
		fprintf(stderr,"\n------------------------\n");
		fprintf(stderr, "gtk_im_context_focus_in\n");
#endif
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

static bool check_button(gControl *w)
{
	return w && w->isVisible() && w->isEnabled();
}

gboolean gcb_key_event(GtkWidget *widget, GdkEvent *event, gControl *control)
{
	gMainWindow *win;
	int type;
	bool cancel;

#if DEBUG_IM
	fprintf(stderr, "gcb_key_event %s for %p %s\n", event->type == GDK_KEY_PRESS ? "GDK_KEY_PRESS" : "GDK_KEY_RELEASE", control, control->name());
#endif

	/*if (!control->_grab && gApplication::activeControl())
		control = gApplication::activeControl();*/
	if (!control || control != gApplication::activeControl())
		return false;

	//if (event->type == GDK_KEY_PRESS)
	//	fprintf(stderr, "GDK_KEY_PRESS: control = %p %s %p %08X\n", control, control ? control->name() : "", event, event->key.state);

	if (_im_is_xim)
	{
		_im_ignore_event = !_im_ignore_event;
		if (_im_ignore_event)
			return false;
	}

	type =  (event->type == GDK_KEY_PRESS) ? gEvent_KeyPress : gEvent_KeyRelease;

	if (gKey::enable(control, &event->key))
	{
		gKey::disable();
		return gKey::canceled();
	}

	if (gKey::mustIgnoreEvent(&event->key))
	{
		gKey::disable();
		return true;
	}

	cancel = gKey::raiseEvent(type, control, NULL);
	gKey::disable();

	if (cancel)
		return true;

	win = control->window();

	if (event->key.keyval == GDK_Escape)
	{
		if (control->_grab)
		{
			gApplication::exitLoop(control);
			return true;
		}

		if (check_button(win->_cancel))
		{
			win->_cancel->setFocus();
			win->_cancel->animateClick(type == gEvent_KeyRelease);
			return true;
		}
	}
	else if (event->key.keyval == GDK_Return || event->key.keyval == GDK_KP_Enter)
	{
		if (check_button(win->_default))
		{
			win->_default->setFocus();
			win->_default->animateClick(type == gEvent_KeyRelease);
			return true;
		}
	}

	if (control->_grab)
		return true;

	return false;
}

bool gKey::gotCommit()
{
	bool ret = _im_got_commit;
	_im_got_commit = FALSE;
	return ret;
}
