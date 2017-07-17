/***************************************************************************

  gcursor.cpp

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

#include "widgets.h"
#include "gb.form.font.h"

#ifndef GAMBAS_DIRECTFB
#ifdef GDK_WINDOWING_X11
#include <gdk/gdkx.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include "x11.h"
#endif
#endif

#include "gcursor.h"

gCursor::gCursor(gPicture *pic, int px, int py)
{
	static bool check = false;
	GdkDisplay *dp = gdk_display_get_default();
	
	if (!check)
	{
		if (!gdk_display_supports_cursor_color(dp) || !gdk_display_supports_cursor_alpha(dp))
			fprintf(stderr, "gb.gtk: warning: RGBA cursors are not supported\n");
		check = true;
	}
	
	x = px;
	y = py;
	cur = NULL;
	if (!pic || pic->isVoid())
		return;
	
	if (pic->width() <= x)
		x = pic->width() - 1;
	if (pic->height() <= y)
		y = pic->height() - 1;
	
	cur=gdk_cursor_new_from_pixbuf(dp, pic->getPixbuf(), x, y);
}

gCursor::gCursor(gCursor *cursor)
{
	cur = NULL;
	if (!cursor) return;
	if (!cursor->cur) return;
	
	cur = cursor->cur;
	x = cursor->x;
	y = cursor->y;
	if (cur)
#ifdef GTK3
		g_object_ref(cur);
#else
		gdk_cursor_ref(cur);
#endif
}

gCursor::~gCursor()
{
	if (cur)
#ifdef GTK3
		g_object_unref(cur);
#else
		gdk_cursor_unref(cur);
#endif
}
