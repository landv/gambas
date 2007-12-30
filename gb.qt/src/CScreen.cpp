/***************************************************************************

  CScreen.cpp

  (c) 2000-2005 Beno� Minisini <gambas@users.sourceforge.net>

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

#define __CDESKTOP_CPP



#include <qapplication.h>
#include <qtooltip.h>

#include "gambas.h"
#include "main.h"
#include "CPicture.h"
#include "CWidget.h"
#include "CWindow.h"
#include "CFont.h"
#include "CScreen.h"

#ifndef NO_X_WINDOW
#include "x11.h"
#endif

static int screen_busy = 0;
char *CAPPLICATION_Theme = 0;

#if 0

/* NET WM: read the _NET_WORKAREA property of the root window to get the viewport
   of each desktop
*/
	if (XGetWindowProperty(p->display, p->root, net_workarea, 0l,
			       (p->number_of_desktops * 4), False, XA_CARDINAL,
			       &type_ret, &format_ret, &nitems_ret, &unused,
			       &data_ret)
	    == Success) {
	    if (type_ret == XA_CARDINAL && format_ret == 32 &&
		nitems_ret == (unsigned) (p->number_of_desktops * 4)) {
		long *d = (long *) data_ret;
		int i, j;
		for (i = 0, j = 0; i < p->number_of_desktops; i++) {
		    p->workarea[i].pos.x       = d[j++];
		    p->workarea[i].pos.y       = d[j++];
		    p->workarea[i].size.width  = d[j++];
		    p->workarea[i].size.height = d[j++];
		}
	    }
#endif


BEGIN_METHOD_VOID(CAPPLICATION_exit)

	GB.FreeString(&CAPPLICATION_Theme);

END_METHOD


BEGIN_PROPERTY(CDESKTOP_width)

  GB.ReturnInteger(qApp->desktop()->width());

END_PROPERTY


BEGIN_PROPERTY(CDESKTOP_height)

  GB.ReturnInteger(qApp->desktop()->height());

END_PROPERTY


BEGIN_PROPERTY(CDESKTOP_resolution)

	#ifdef NO_X_WINDOW
  	GB.ReturnInteger(72);
	#else
  	GB.ReturnInteger(QPaintDevice::x11AppDpiY());
  #endif

END_PROPERTY


BEGIN_PROPERTY(CAPP_font)

  if (READ_PROPERTY)
    GB.ReturnObject(CFONT_create(qApp->font(), CFONT_APPLICATION));
  else
  {
    qApp->setFont(*((CFONT *)VPROP(GB_OBJECT))->font);
    MAIN_update_scale();
  }

END_PROPERTY


BEGIN_PROPERTY(CAPP_active_window)

  //GB.ReturnObject(CWidget::get(qApp->activeWindow()));
  GB.ReturnObject(CWINDOW_Active);

END_PROPERTY


BEGIN_PROPERTY(CAPP_active_control)

  GB.ReturnObject(CWidget::get(qApp->focusWidget()));

END_PROPERTY


BEGIN_PROPERTY(CAPP_busy)

  long busy;

  if (READ_PROPERTY)
    GB.ReturnInteger(screen_busy);
  else
  {
    busy = VPROP(GB_INTEGER);

    if (screen_busy == 0 && busy != 0)
      qApp->setOverrideCursor(Qt::waitCursor);
    else if (screen_busy > 0 && busy == 0)
      qApp->restoreOverrideCursor();

    screen_busy = busy;
  }

END_PROPERTY


BEGIN_PROPERTY(CDESKTOP_charset)

  GB.ReturnConstZeroString("UTF-8");

END_PROPERTY


BEGIN_METHOD_VOID(CDESKTOP_grab)

  GB.ReturnObject(CPICTURE_grab(0));

END_METHOD


BEGIN_PROPERTY(CAPP_tooltip_enabled)

  if (READ_PROPERTY)
    GB.ReturnBoolean(QToolTip::isGloballyEnabled());
  else
    QToolTip::setGloballyEnabled(VPROP(GB_BOOLEAN));

END_PROPERTY


BEGIN_PROPERTY(CAPP_tooltip_font)

  if (READ_PROPERTY)
    GB.ReturnObject(CFONT_create(QToolTip::font()));
  else
    QToolTip::setFont(*((CFONT *)VPROP(GB_OBJECT))->font);

END_PROPERTY


BEGIN_PROPERTY(CAPP_main_window)

  GB.ReturnObject(CWINDOW_Main);

END_PROPERTY


BEGIN_PROPERTY(CAPP_embedder)

  if (READ_PROPERTY)
    GB.ReturnInteger(CWINDOW_Embedder);
  else
  {
    if (CWINDOW_Embedded)
    {
      GB.Error("Application is already embedded");
      return;
    }

    CWINDOW_Embedder = VPROP(GB_INTEGER);
  }

END_PROPERTY


BEGIN_PROPERTY(CAPPLICATION_theme)

	if (READ_PROPERTY)
		GB.ReturnString(CAPPLICATION_Theme);
	else
    GB.StoreString(PROP(GB_STRING), &CAPPLICATION_Theme);

END_PROPERTY


BEGIN_PROPERTY(CDESKTOP_scale)

  GB.ReturnInteger(MAIN_scale);

END_PROPERTY


BEGIN_METHOD(CDESKTOP_find, GB_STRING title; GB_STRING klass; GB_STRING role)

	#ifndef NO_X_WINDOW
	Window *windows;
	Window win;
	int count;
	int i;
	char *title = MISSING(title) ? NULL : STRING(title);
	long ltitle = MISSING(title) ? 0 : LENGTH(title);
	char *klass = MISSING(klass) ? NULL : STRING(klass);
	long lklass = MISSING(klass) ? 0 : LENGTH(klass);
	char *role = MISSING(role) ? NULL : STRING(role);
	long lrole = MISSING(role) ? 0 : LENGTH(role);
	char *prop;
	long lprop;
	#endif
	GB_ARRAY result;

	GB.Array.New(&result, GB_T_INTEGER, 0);

	#ifdef NO_X_WINDOW
		qDebug("Qt/Embedded: Desktop.Find not implemented");
	#else
		X11_find_windows(&windows, &count);
	
		for (i = 0; i < count; i++)
		{
			win = windows[i];
			//qDebug("win = %08X", win);
			if (ltitle)
			{
				X11_get_window_title(win, &prop, &lprop);
				//qDebug("title = %.*s", lprop, prop);
				if (!GB.MatchString(title, ltitle, prop, lprop))
					continue;
			}
	
			if (lklass)
			{
				X11_get_window_class(win, &prop, &lprop);
				//qDebug("class = %.*s", lprop, prop);
				if (!GB.MatchString(klass, lklass, prop, lprop))
					continue;
			}
	
			if (lrole)
			{
				X11_get_window_role(win, &prop, &lprop);
				//qDebug("role = %.*s", lprop, prop);
				if (!GB.MatchString(role, lrole, prop, lprop))
					continue;
			}
	
			*((long *)GB.Array.Add(result)) = win;
		}
	
		XFree(windows);
	#endif

	GB.ReturnObject(result);

END_METHOD


GB_DESC CDesktopDesc[] =
{
  GB_DECLARE("Desktop", 0), GB_VIRTUAL_CLASS(),

  GB_STATIC_PROPERTY_READ("W", "i", CDESKTOP_width),
  GB_STATIC_PROPERTY_READ("H", "i", CDESKTOP_height),
  GB_STATIC_PROPERTY_READ("Width", "i", CDESKTOP_width),
  GB_STATIC_PROPERTY_READ("Height", "i", CDESKTOP_height),
  GB_STATIC_PROPERTY_READ("Charset", "s", CDESKTOP_charset),
  GB_STATIC_PROPERTY_READ("Resolution", "i", CDESKTOP_resolution),
  GB_STATIC_PROPERTY_READ("Scale", "i", CDESKTOP_scale),

  GB_STATIC_METHOD("Grab", "Picture", CDESKTOP_grab, NULL),
  GB_STATIC_METHOD("Find", "Integer[]", CDESKTOP_find, "[(Title)s(Application)s(Role)s]"),

  GB_END_DECLARE
};

GB_DESC CApplicationTooltipDesc[] =
{
  GB_DECLARE(".ApplicationTooltip", 0), GB_VIRTUAL_CLASS(),

  GB_STATIC_PROPERTY("Enabled", "b", CAPP_tooltip_enabled),
  GB_STATIC_PROPERTY("Font", "Font", CAPP_tooltip_font),

  GB_END_DECLARE
};

GB_DESC CApplicationDesc[] =
{
  GB_DECLARE("Application", 0), GB_VIRTUAL_CLASS(),

	GB_STATIC_METHOD("_exit", NULL, CAPPLICATION_exit, NULL),
  GB_STATIC_PROPERTY("Font", "Font", CAPP_font),
  GB_STATIC_PROPERTY_READ("ActiveWindow", "Window", CAPP_active_window),
  GB_STATIC_PROPERTY_READ("ActiveControl", "Control", CAPP_active_control),
  GB_STATIC_PROPERTY_READ("MainWindow", "Window", CAPP_main_window),
  GB_STATIC_PROPERTY("Busy", "i", CAPP_busy),
  GB_STATIC_PROPERTY_SELF("ToolTip", ".ApplicationTooltip"),
  GB_STATIC_PROPERTY("Embedder", "i", CAPP_embedder),
  GB_STATIC_PROPERTY("Theme", "s", CAPPLICATION_theme),

  GB_END_DECLARE
};


