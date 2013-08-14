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
#include "widgets_private.h"
#include "gapplication.h"
#include "gtrayicon.h"
#include "gdesktop.h"
#include "gkey.h"

//#define DEBUG_IM 1

/*************************************************************************

gKey

**************************************************************************/

bool gKey::_valid = false;
bool gKey::_no_input_method = false;
GdkEventKey gKey::_event;
GtkIMContext *gKey::_im_context = NULL;
gControl *gKey::_im_control = NULL;
char *_im_text = NULL;

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
	return state() & GDK_MOD2_MASK; // || _event.keyval == GDK_Meta_L || _event.keyval == GDK_Meta_R;
}

bool gKey::normal()
{
	return (state() & 0xFF) != 0;
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
	return key;
}

void gKey::disable()
{
	if (!_valid)
		return;
		
	_valid = false;
	_event.keyval = 0;
	_event.state = 0;
	g_free(_event.string);
}

bool gKey::enable(gControl *control, GdkEventKey *event)
{
	bool filter;
	
	//if (widget != _im_widget)
	//	return true;
	
	if (_valid)
		disable();
		
	_valid = true;
	_event = *event;
	
	if (_event.type == GDK_KEY_PRESS && !_no_input_method && control == _im_control)
	{
		#if DEBUG_IM
		fprintf(stderr, "gKey::enable: event->string = '%s'\n", event->string);
		#endif
		filter = gtk_im_context_filter_keypress(_im_context, &_event);
		#if DEBUG_IM
		fprintf(stderr, "gKey::enable: filter -> %d event->string = '%s'\n", filter, event->string);
		#endif
	}
	else
		filter = false;
  
  if (filter && _im_text)
  {
		_event.string = g_strdup(_im_text);
		//_event.keyval = 0;
		filter = false;
  }
  else
  	_event.string = g_strdup(_event.string);
  
  if (!filter)
  {
		//#if DEBUG_IM
  	//fprintf(stderr, "gKey::enable: gtk_im_context_reset\n");
		//#endif
  	//gtk_im_context_reset(_im_context);
  	if (_im_text)
  	{
  		g_free(_im_text);
  		_im_text = NULL;
  	}
  }
  
  //fprintf(stderr, "gKey::enable: --> %d\n", filter);
  return filter;
}

static void cb_im_commit(GtkIMContext *context, const gchar *str, gpointer pointer)
{
	#if DEBUG_IM
	fprintf(stderr, "cb_im_commit: %s\n", str);
	#endif
	
	if (_im_text)
		g_free(_im_text);
		
	_im_text = g_strdup(str);
}

void gKey::init()
{
	_im_context = gtk_im_multicontext_new();
  g_signal_connect (_im_context, "commit", G_CALLBACK(cb_im_commit), NULL);
}

void gKey::exit()
{
	disable();
	if (_im_text)
		g_free(_im_text);
	g_object_unref(_im_context);
}

void gKey::setActiveControl(gControl *control)
{
	if (_im_control)
	{
		if (!_no_input_method)
		{
			#if DEBUG_IM
			fprintf(stderr, "gtm_im_context_focus_out\n");
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
	  	gtk_im_context_set_client_window (_im_context, _im_control->widget->window);
			gtk_im_context_focus_in(_im_context);
			gtk_im_context_reset(_im_context);
			#if DEBUG_IM
			fprintf(stderr, "gtm_im_context_focus_in\n");
			#endif
		}		
	}
}
