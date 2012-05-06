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
};

typedef
	unsigned char uchar;

#endif
