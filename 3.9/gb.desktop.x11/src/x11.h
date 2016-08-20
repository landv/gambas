/***************************************************************************

  x11.h

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

#ifndef __X11_H
#define __X11_H

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/extensions/XTest.h>
#include <X11/keysym.h>
#include <X11/XKBlib.h>

#ifdef QT_VERSION
#undef FocusIn
#undef FocusOut
#undef KeyPress
#undef KeyRelease
#endif

#include "gambas.h"
#include "gb_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __X11_C
EXTERN Atom X11_atom_net_wm_state;
EXTERN Atom X11_atom_net_wm_state_above;
EXTERN Atom X11_atom_net_wm_state_below;
EXTERN Atom X11_atom_net_wm_state_stays_on_top;
EXTERN Atom X11_atom_net_wm_state_skip_taskbar;
EXTERN Atom X11_atom_net_wm_window_type;
EXTERN Atom X11_atom_net_wm_desktop;

EXTERN Window X11_root;
EXTERN Display *X11_display;
EXTERN bool X11_ready;
EXTERN bool X11_event_filter_enabled;

EXTERN Atom X11_UTF8_STRING;
#endif

typedef
	struct {
		char *title;
		char *klass;
		char *role;
		}
	X11_WINDOW_INFO;

bool X11_do_init();
#define X11_init() (!X11_ready && X11_do_init())
void X11_exit();
void X11_sync(void);

#define X11_get_screen_count() ScreenCount(X11_display)
#define X11_get_root_window(_screen) RootWindow(X11_display, (_screen))

/* Functions to deal with the _NET_WM_STATE and _NET_WM_TYPE property */
void X11_window_change_property(Window window, bool visible, Atom property, bool set);
bool X11_window_has_property(Window window, Atom property);
void X11_window_save_properties(Window window);
void X11_window_restore_properties(Window window);
/* Function to dock a window in the system tray */
void X11_window_dock(Window window);
/* Function to define startup position hints for a window being shown */
void X11_window_startup(Window window, int x, int y, int w, int h);
/* Functions to search for a specific top-level window */
void X11_find_windows(Window **window_list, int *count);
void X11_get_window_title(Window window, char **result, int *length);
void X11_get_window_class(Window window, char **result, int *length);
void X11_get_window_role(Window window, char **result, int *length);
void X11_get_window_geometry(Window win, int *wx, int *wy, int *ww, int *wh);
/* Function to make a tool window */
void X11_set_window_tool(Window window, int tool, Window parent);
int X11_get_window_tool(Window window);
void X11_window_set_desktop(Window window, bool visible, int desktop);
int X11_window_get_desktop(Window window);
int X11_get_current_desktop();
char *X11_send_key(char *key, bool press);

Atom X11_intern_atom(const char *name, bool create);
char *X11_get_property(Window wid, Atom prop, Atom *type, int *format, int *count);
Atom X11_get_property_type(Window wid, Atom prop, int *format);
void X11_set_property(Window wid, Atom prop, Atom type, int format, void *data, int count);
void X11_send_client_message(Window dest, Window window, Atom message, char *data, int format, int count);
void X11_event_filter(XEvent *e);
void X11_enable_event_filter(bool enable);

#ifdef __cplusplus
}
#endif

#endif
