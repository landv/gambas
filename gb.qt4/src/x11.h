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
//#include <X11/extensions/shape.h>
const int XFocusIn = FocusIn;
#undef FocusIn
const int XFocusOut = FocusOut;
#undef FocusOut
const int XKeyPress = KeyPress;
#undef KeyPress
const int XKeyRelease = KeyRelease;
#undef KeyRelease

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
#endif

typedef
	struct {
		char *title;
		char *klass;
		char *role;
		}
	X11_WINDOW_INFO;

enum
{
	_NET_WM_WINDOW_TYPE_NORMAL,
	_NET_WM_WINDOW_TYPE_DESKTOP,
	_NET_WM_WINDOW_TYPE_DOCK,
	_NET_WM_WINDOW_TYPE_TOOLBAR,
	_NET_WM_WINDOW_TYPE_MENU,
	_NET_WM_WINDOW_TYPE_UTILITY,
	_NET_WM_WINDOW_TYPE_SPLASH,
	_NET_WM_WINDOW_TYPE_DIALOG,
	_NET_WM_WINDOW_TYPE_DROPDOWN_MENU,
	_NET_WM_WINDOW_TYPE_POPUP_MENU,
	_NET_WM_WINDOW_TYPE_TOOLTIP,
	_NET_WM_WINDOW_TYPE_NOTIFICATION,
	_NET_WM_WINDOW_TYPE_COMBO,
	_NET_WM_WINDOW_TYPE_DND
};

void X11_init(Display *display, Window root);
void X11_exit();
void X11_sync(void);

/* Functions to deal with the _NET_WM_STATE and _NET_WM_TYPE property */
void X11_window_change_property(Window window, bool visible, Atom property, bool set);
bool X11_window_has_property(Window window, Atom property);
void X11_window_save_properties(Window window);
void X11_window_restore_properties(Window window);
/* Function to dock a window in the system tray */
bool X11_window_dock(Window window);
Window X11_get_system_tray();
/* Function to define startup position hints for a window being shown */
void X11_window_startup(Window window, int x, int y, int w, int h);
/* Functions to search for a specific top-level window */
void X11_find_windows(Window **window_list, int *count);
void X11_get_window_title(Window window, char **result, int *length);
void X11_get_window_class(Window window, char **result, int *length);
void X11_get_window_role(Window window, char **result, int *length);
/* Function to make a tool window */
void X11_set_window_tool(Window window, int tool, Window parent);
int X11_get_window_tool(Window window);
void X11_window_set_desktop(Window window, bool visible, int desktop);
int X11_window_get_desktop(Window window);
int X11_get_current_desktop();

int X11_get_window_type(Window window);
void X11_set_window_type(Window window, int type);
void X11_set_transient_for(Window window, Window parent);
void X11_set_window_decorated(Window window, bool decorated);
void X11_window_remap(Window window);
void X11_window_activate(Window window);
bool X11_get_available_geometry(int screen, int *x, int *y, int *w, int *h);

char *X11_get_property(Window wid, Atom prop, Atom *type, int *format, int *pcount);
bool X11_is_supported_by_WM(Atom atom);
bool X11_send_move_resize_event(Window window, int x, int y, int w, int h);

void X11_flush();

#ifdef __cplusplus
}
#endif

#endif
