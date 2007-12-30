/***************************************************************************

  x11.h

  Common X11 routines for gb.qt and gb.gtk

  (c) 2000-2007 Benoit Minisini <gambas@users.sourceforge.net>

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

PUBLIC void X11_init(Display *display, Window root);
PUBLIC void X11_exit();
PUBLIC void X11_sync(void);

/* Functions to deal with the _NET_WM_STATE and _NET_WM_TYPE property */
PUBLIC void X11_window_change_property(Window window, bool visible, Atom property, bool set);
PUBLIC bool X11_window_has_property(Window window, Atom property);
PUBLIC void X11_window_save_properties(Window window);
PUBLIC void X11_window_restore_properties(Window window);
/* Function to dock a window in the system tray */
PUBLIC void X11_window_dock(Window window);
/* Function to define startup position hints for a window being shown */
PUBLIC void X11_window_startup(Window window, int x, int y, int w, int h);
/* Functions to search for a specific top-level window */
PUBLIC void X11_find_windows(Window **window_list, int *count);
PUBLIC void X11_get_window_title(Window window, char **result, long *length);
PUBLIC void X11_get_window_class(Window window, char **result, long *length);
PUBLIC void X11_get_window_role(Window window, char **result, long *length);
/* Function to make a tool window */
PUBLIC void X11_set_window_tool(Window window, int tool, Window parent);
PUBLIC int X11_get_window_tool(Window window);
PUBLIC void X11_window_set_desktop(Window window, bool visible, int desktop);
PUBLIC long X11_window_get_desktop(Window window);
PUBLIC long X11_get_current_desktop();

#ifdef __cplusplus
}
#endif

#endif
