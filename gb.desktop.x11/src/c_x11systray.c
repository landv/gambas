/***************************************************************************

	c_x11systray.c

	(c) 2000-2014 Benoît Minisini <gambas@users.sourceforge.net>

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

#define __C_X11SYSTRAY_C

#include "x11.h"
#include "systray/systray.h"
#include "c_x11systray.h"

//---------------------------------------------------------------------------

static GB_FUNCTION _arrange_func;

void SYSTRAY_raise_arrange(void)
{
	static bool init = FALSE;

	if (!init)
	{
		GB_CLASS startup = GB.Application.StartupClass();
		GB.GetFunction(&_arrange_func, (void *)startup, "X11Systray_Arrange", "", "");
		init = TRUE;
	}

	GB.Call(&_arrange_func, 0, TRUE);
}

//---------------------------------------------------------------------------

BEGIN_METHOD(X11Systray_Show, GB_INTEGER window)

	X11_init();
	SYSTRAY_init(X11_display, VARG(window));

END_METHOD

BEGIN_PROPERTY(X11Systray_Count)

	GB.ReturnInteger(SYSTRAY_get_count());

END_PROPERTY

BEGIN_METHOD(X11Systray_get, GB_INTEGER index)

	int index = VARG(index);

	if (index < 0 || index >= SYSTRAY_get_count())
	{
		GB.Error(GB_ERR_BOUND);
		return;
	}

	GB.ReturnObject(SYSTRAY_get(index));

END_METHOD

BEGIN_METHOD_VOID(X11Systray_Refresh)

	SYSTRAY_refresh();

END_METHOD

//---------------------------------------------------------------------------

#define THIS_ICON ((CX11SYSTRAYICON *)_object)

BEGIN_PROPERTY(X11SystrayIcon_X)

	GB.ReturnInteger(THIS_ICON->x);

END_PROPERTY

BEGIN_PROPERTY(X11SystrayIcon_Y)

	GB.ReturnInteger(THIS_ICON->y);

END_PROPERTY

BEGIN_PROPERTY(X11SystrayIcon_Width)

	GB.ReturnInteger(THIS_ICON->w);

END_PROPERTY

BEGIN_PROPERTY(X11SystrayIcon_Height)

	GB.ReturnInteger(THIS_ICON->h);

END_PROPERTY

BEGIN_PROPERTY(X11SystrayIcon_IconWidth)

	GB.ReturnInteger(THIS_ICON->iw);

END_PROPERTY

BEGIN_PROPERTY(X11SystrayIcon_IconHeight)

	GB.ReturnInteger(THIS_ICON->ih);

END_PROPERTY

BEGIN_METHOD(X11SystrayIcon_Move, GB_INTEGER x; GB_INTEGER y; GB_INTEGER w; GB_INTEGER h)

	THIS_ICON->x = VARG(x);
	THIS_ICON->y = VARG(y),
	THIS_ICON->is_updated = TRUE;

	if (!MISSING(w) && !MISSING(h))
	{
		THIS_ICON->w = VARG(w);
		THIS_ICON->h = VARG(h),
		THIS_ICON->is_resized = TRUE;
	}

END_METHOD

BEGIN_METHOD(X11SystrayIcon_Resize, GB_INTEGER w; GB_INTEGER h)

	THIS_ICON->w = VARG(w);
	THIS_ICON->h = VARG(h),
	THIS_ICON->is_updated = TRUE;
	THIS_ICON->is_resized = TRUE;

END_METHOD

BEGIN_PROPERTY(X11SystrayIcon_Handle)

	GB.ReturnInteger((int)THIS_ICON->wid);

END_PROPERTY


GB_DESC X11SystrayIconDesc[] =
{
	GB_DECLARE("X11SystrayIcon", sizeof(CX11SYSTRAYICON)),
	GB_NOT_CREATABLE(),

	GB_PROPERTY_READ("X", "i", X11SystrayIcon_X),
	GB_PROPERTY_READ("Y", "i", X11SystrayIcon_Y),
	GB_PROPERTY_READ("W", "i", X11SystrayIcon_Width),
	GB_PROPERTY_READ("Width", "i", X11SystrayIcon_Width),
	GB_PROPERTY_READ("H", "i", X11SystrayIcon_Height),
	GB_PROPERTY_READ("Height", "i", X11SystrayIcon_Height),
	GB_PROPERTY_READ("IconW", "i", X11SystrayIcon_IconWidth),
	GB_PROPERTY_READ("IconWidth", "i", X11SystrayIcon_IconWidth),
	GB_PROPERTY_READ("IconH", "i", X11SystrayIcon_IconHeight),
	GB_PROPERTY_READ("IconHeight", "i", X11SystrayIcon_IconHeight),
	GB_PROPERTY_READ("Handle", "i", X11SystrayIcon_Handle),

	GB_METHOD("Move", NULL, X11SystrayIcon_Move, "(X)i(Y)i[(Width)i(Height)i]"),
	GB_METHOD("Resize", NULL, X11SystrayIcon_Resize, "(Width)i(Height)i"),

	GB_END_DECLARE
};

GB_DESC X11SystrayDesc[] =
{
	GB_DECLARE_VIRTUAL("X11Systray"),

	GB_STATIC_METHOD("Show", NULL, X11Systray_Show, "(Window)i"),
	GB_STATIC_PROPERTY_READ("Count", "i", X11Systray_Count),
	GB_STATIC_METHOD("_get", "X11SystrayIcon", X11Systray_get, "(Index)i"),
	GB_STATIC_METHOD("Refresh", NULL, X11Systray_Refresh, NULL),

	GB_END_DECLARE
};
