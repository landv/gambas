/***************************************************************************

  widgets.h

  (c) 2004-2006 - Daniel Campos Fernández <dcamposf@gmail.com>

  Gtkmae "GTK+ made easy" classes

  Realizado para la Junta de Extremadura.
  Consejería de Educación Ciencia y Tecnología.
  Proyecto gnuLinEx

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

#ifndef __WIDGETS_H
#define __WIDGETS_H

#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>
#ifdef GDK_WINDOWING_X11
#include <gdk/gdkx.h>
#endif
#include <gtk/gtk.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "gb.form.properties.h"

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
	LINE_NONE = 0,
  LINE_SOLID = 1,
  LINE_DASH = 2,
  LINE_DOT = 3,
  LINE_DASH_DOT = 4,
  LINE_DASH_DOT_DOT = 5
};

enum
{
	FILL_NONE = 0,
  FILL_SOLID = 1,
  FILL_DENSE_94 = 2,
  FILL_DENSE_88 = 3,
  FILL_DENSE_63 = 4,
  FILL_DENSE_50 = 5,
  FILL_DENSE_37 = 6,
  FILL_DENSE_12 = 7,
  FILL_DENSE_06 = 8,
  FILL_HORIZONTAL = 9,
  FILL_VERTICAL = 10,
  FILL_CROSS = 11,
  FILL_DIAGONAL = 12,
  FILL_BACK_DIAGONAL = 13,
  FILL_CROSS_DIAGONAL = 14
};

enum
{
  ALIGN_NORMAL = 0x00,
  ALIGN_LEFT = 0x01,
  ALIGN_RIGHT = 0x02,
  ALIGN_CENTER = 0x03,
  ALIGN_JUSTIFY = 0x04,	
	ALIGN_TOP_NORMAL = 0x10,
	ALIGN_TOP_LEFT = 0x11,
	ALIGN_TOP_RIGHT = 0x12,
  ALIGN_TOP = 0x13,
  ALIGN_BOTTOM_NORMAL = 0x20,
  ALIGN_BOTTOM_LEFT = 0x21,
  ALIGN_BOTTOM_RIGHT = 0x22,
  ALIGN_BOTTOM = 0x23
};

enum {
  BORDER_NONE = 0,
  BORDER_PLAIN = 1,
  BORDER_SUNKEN = 2,
  BORDER_RAISED = 3,
  BORDER_ETCHED = 4
  };

enum {
  ARRANGE_NONE = 0,
  ARRANGE_HORIZONTAL = 1,
  ARRANGE_VERTICAL = 2,
  ARRANGE_ROW = 3,
  ARRANGE_LEFT_RIGHT = 3,
  ARRANGE_COLUMN = 4,
  ARRANGE_TOP_BOTTOM = 4,
  ARRANGE_FILL = 5
  };

enum
{
	SELECT_NONE = 0,
	SELECT_SINGLE = 1,
	SELECT_MULTIPLE = 2
};

enum
{
	SCROLL_NONE = 0,
	SCROLL_HORIZONTAL = 1,
	SCROLL_VERTICAL = 2,
	SCROLL_BOTH = 3
};

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
	Type_gIconView    = 0x1015,
	Type_gGridView    = 0x1016,
};

enum
{
 gEvent_MousePress,
 gEvent_MouseRelease,
 gEvent_MouseMove,
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

#endif
