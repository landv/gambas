/***************************************************************************

  widgets.h

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

#ifndef __WIDGETS_H
#define __WIDGETS_H

#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>
#ifndef GAMBAS_DIRECTFB
#ifdef GDK_WINDOWING_X11
#include <gdk/gdkx.h>
#endif
#endif
#include <gtk/gtk.h>

#ifdef GTK3

#ifdef GDK_WINDOWING_X11
#include <gtk/gtkx.h>
#endif
#include <gdk/gdkkeysyms-compat.h>

#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdint.h>
#include <inttypes.h>

#include "gb_common.h"

#include "gb.form.properties.h"
#include "gb.form.const.h"

#include "gpicture.h"
#include "gcursor.h"
#include "gfont.h"
#include "gcontrol.h"
#include "gcontainer.h"
#include "gtools.h"
#include "gmemory.h"

//#define GTK_DEBUG_SIGNALS
//#define GTK_DEBUG_OBJECTS

enum
{
	Type_gLabel       = 0x0001,
	Type_gTextLabel   = 0x0002,
	Type_gButton      = 0x0003,
	Type_gPictureBox  = 0x0007,
	Type_gProgressBar = 0x000A,
	Type_gSlider      = 0x000E,
	Type_gScrollBar   = 0x000F,
	Type_gPlugin      = 0x0013,
	Type_gMovieBox    = 0x0014,
	Type_gSpinBox     = 0x0016,
	Type_gSeparator   = 0x0018,
	Type_gFrame       = 0x0105,
	Type_gMainWindow  = 0x0106,
	Type_gPanel       = 0x0108,
	Type_gDrawingArea = 0x0109,
	Type_gTabStrip    = 0x0111,
	Type_gSplitter    = 0x0117,
	Type_gScrollView  = 0x0115,
	Type_gTextBox     = 0x1004,
	Type_gTextArea    = 0x100B,
	Type_gComboBox    = 0x100C,
	Type_gListBox     = 0x100D,
	Type_gListView    = 0x1010,
	Type_gColumnView  = 0x1012,
	Type_gTreeView    = 0x1014,
	Type_gGLArea      = 0x0018,
};

enum
{
 gEvent_MousePress,
 gEvent_MouseRelease,
 gEvent_MouseMove,
 gEvent_MouseDrag,
 gEvent_MouseWheel,
 gEvent_MouseDblClick,
 gEvent_MouseMenu,
 gEvent_KeyPress,
 gEvent_KeyRelease,
 gEvent_FocusIn,
 gEvent_FocusOut,
 gEvent_Enter,
 gEvent_Leave,
 gEvent_DragMove,
 gEvent_Drop
};

typedef
	unsigned char uchar;

#ifdef GTK3
	#define ON_DRAW_BEFORE(_widget, _this, _gtk, _gtk3) g_signal_connect(G_OBJECT(_widget), "draw", G_CALLBACK(_gtk3), (gpointer)_this)
	#define ON_DRAW(_widget, _this, _gtk, _gtk3) g_signal_connect_after(G_OBJECT(_widget), "draw", G_CALLBACK(_gtk3), (gpointer)_this)
#else
	#define ON_DRAW_BEFORE(_widget, _this, _gtk, _gtk3) g_signal_connect(G_OBJECT(_widget), "expose-event", G_CALLBACK(_gtk), (gpointer)_this)
	#define ON_DRAW(_widget, _this, _gtk, _gtk3) g_signal_connect_after(G_OBJECT(_widget), "expose-event", G_CALLBACK(_gtk), (gpointer)_this)
#endif

#ifdef GTK3

#define STATE_T GtkStateFlags
#define STYLE_T GtkStyleContext

#define STATE_NORMAL GTK_STATE_FLAG_NORMAL
#define STATE_ACTIVE GTK_STATE_FLAG_ACTIVE
#define STATE_INSENSITIVE GTK_STATE_FLAG_INSENSITIVE
#define STATE_PRELIGHT GTK_STATE_FLAG_PRELIGHT
#define STATE_SELECTED GTK_STATE_FLAG_SELECTED
#define STATE_FOCUSED GTK_STATE_FLAG_FOCUSED

#define gtk_hbox_new(_homogeneous, _spacing) gtk_box_new(GTK_ORIENTATION_HORIZONTAL, _spacing)
#define gtk_vbox_new(_homogeneous, _spacing) gtk_box_new(GTK_ORIENTATION_VERTICAL, _spacing)

#define gtk_hscale_new(_adj) gtk_scale_new(GTK_ORIENTATION_HORIZONTAL, _adj)
#define gtk_vscale_new(_adj) gtk_scale_new(GTK_ORIENTATION_VERTICAL, _adj)

#define gtk_hscrollbar_new(_adj) gtk_scrollbar_new(GTK_ORIENTATION_HORIZONTAL, _adj)
#define gtk_vscrollbar_new(_adj) gtk_scrollbar_new(GTK_ORIENTATION_VERTICAL, _adj)

#define gtk_modify_font gtk_override_font

#else

#define STATE_T GtkStateType
#define STYLE_T GtkStyle

#define STATE_NORMAL GTK_STATE_NORMAL
#define STATE_ACTIVE GTK_STATE_ACTIVE
#define STATE_INSENSITIVE GTK_STATE_INSENSITIVE
#define STATE_PRELIGHT GTK_STATE_PRELIGHT
#define STATE_SELECTED GTK_STATE_SELECTED

#endif

#endif
