/***************************************************************************

  CScreen.cpp

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

#define __CSCREEN_CPP

#include <QApplication>
#include <QDesktopWidget>
#include <QToolTip>
#include <QSessionManager>

#include "gambas.h"
#include "main.h"
#include "gb.draw.h"
#include "cpaint_impl.h"
#include "CPicture.h"
#include "CWidget.h"
#include "CWindow.h"
#include "CFont.h"
#include "CScreen.h"

#ifndef NO_X_WINDOW
#include <QX11Info>
#include "x11.h"
#endif

#if QT_VERSION >= 0x040600
#define NUM_SCREENS() (QApplication::desktop()->screenCount())
#else
#define NUM_SCREENS() (QApplication::desktop()->numScreens())
#endif

#define MAX_SCREEN 16

char *CAPPLICATION_Theme = 0;
GB_ARRAY CAPPLICATION_Restart = NULL;

static int screen_busy = 0;
static CSCREEN *_screens[MAX_SCREEN] = { NULL };

static CSCREEN *get_screen(int num)
{
	if (num < 0 || num >= MAX_SCREEN || num >= NUM_SCREENS())
		return NULL;
	
	if (!_screens[num])
	{
		_screens[num] = (CSCREEN *)GB.New(GB.FindClass("Screen"), NULL, 0);
		_screens[num]->index = num;
		GB.Ref(_screens[num]);
	}
	
	return _screens[num];
}

static void free_screens(void)
{
	int i;
	
	for (i = 0; i < MAX_SCREEN; i++)
	{
		if (_screens[i])
			GB.Unref(POINTER(&_screens[i]));
	}
}

//-------------------------------------------------------------------------

BEGIN_PROPERTY(Desktop_X)

	GB.ReturnInteger(QApplication::desktop()->availableGeometry().x());

END_PROPERTY

BEGIN_PROPERTY(Desktop_Y)

	GB.ReturnInteger(QApplication::desktop()->availableGeometry().y());

END_PROPERTY

BEGIN_PROPERTY(Desktop_Width)

	GB.ReturnInteger(QApplication::desktop()->availableGeometry().width());

END_PROPERTY

BEGIN_PROPERTY(Desktop_Height)

	GB.ReturnInteger(QApplication::desktop()->availableGeometry().height());

END_PROPERTY


BEGIN_PROPERTY(Desktop_Resolution)

	#ifdef NO_X_WINDOW
		GB.ReturnInteger(72);
	#else
		GB.ReturnInteger(QX11Info::appDpiY());
	#endif

END_PROPERTY

BEGIN_PROPERTY(Desktop_HasSystemTray)

	#ifdef NO_X_WINDOW
		GB.Return(FALSE);
	#else
		GB.ReturnBoolean(X11_get_system_tray() != None);
	#endif

END_PROPERTY

BEGIN_METHOD(Desktop_Screenshot, GB_INTEGER x; GB_INTEGER y; GB_INTEGER w; GB_INTEGER h)

	GB.ReturnObject(CPICTURE_grab(0, VARGOPT(x, 0), VARGOPT(y, 0), VARGOPT(w, 0), VARGOPT(h, 0)));

END_METHOD


BEGIN_PROPERTY(Desktop_Scale)

	GB.ReturnInteger(MAIN_scale);

END_PROPERTY

//-------------------------------------------------------------------------

BEGIN_METHOD_VOID(Application_exit)

	GB.FreeString(&CAPPLICATION_Theme);
	GB.StoreObject(NULL, POINTER(&CAPPLICATION_Restart));
	free_screens();

END_METHOD

static void set_font(QFont &font, void *object = 0)
{
	qApp->setFont(font);
	MAIN_update_scale();
}

BEGIN_PROPERTY(Application_Font)

	if (READ_PROPERTY)
		GB.ReturnObject(CFONT_create(qApp->font(), set_font));
	else
		CFONT_set(set_font, (CFONT *)VPROP(GB_OBJECT), NULL);

END_PROPERTY


BEGIN_PROPERTY(Application_ActiveWindow)

	//GB.ReturnObject(CWidget::get(qApp->activeWindow()));
	GB.ReturnObject(CWINDOW_Active);

END_PROPERTY


BEGIN_PROPERTY(Application_ActiveControl)

	GB.ReturnObject(CWIDGET_active_control);

END_PROPERTY


BEGIN_PROPERTY(Application_PreviousControl)

	GB.ReturnObject(CWIDGET_previous_control);

END_PROPERTY


BEGIN_PROPERTY(Application_Busy)

	int busy;

	if (READ_PROPERTY)
		GB.ReturnInteger(screen_busy);
	else
	{
		busy = VPROP(GB_INTEGER);

		if (screen_busy == 0 && busy > 0)
			qApp->setOverrideCursor(Qt::WaitCursor);
		else if (screen_busy > 0 && busy == 0)
			qApp->restoreOverrideCursor();

		screen_busy = busy;
		if (MAIN_debug_busy)
			qDebug("%s: Application.Busy = %d", GB.Debug.GetCurrentPosition(), busy);
	}

END_PROPERTY


BEGIN_PROPERTY(Application_ShowTooltips)

	if (READ_PROPERTY)
		GB.ReturnBoolean(MyApplication::isTooltipEnabled());
	else
		MyApplication::setTooltipEnabled(VPROP(GB_BOOLEAN));

END_PROPERTY


BEGIN_PROPERTY(Application_MainWindow)

	if (READ_PROPERTY)
		GB.ReturnObject(CWINDOW_Main);
	else
	{
		CWINDOW_Main = (CWINDOW *)VPROP(GB_OBJECT);
		if (CWINDOW_Main && CWINDOW_MainDesktop >= 0)
		{
			MyMainWindow *win = (MyMainWindow *)CWINDOW_Main->widget.widget;
			X11_window_set_desktop(win->winId(), win->isVisible(), CWINDOW_MainDesktop);
			CWINDOW_MainDesktop = -1;
		}
	}

END_PROPERTY


BEGIN_PROPERTY(Application_Embedder)

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


BEGIN_PROPERTY(Application_Theme)

	if (READ_PROPERTY)
		GB.ReturnString(CAPPLICATION_Theme);
	else
		GB.StoreString(PROP(GB_STRING), &CAPPLICATION_Theme);

END_PROPERTY


BEGIN_PROPERTY(Application_Restart)

	if (READ_PROPERTY)
		GB.ReturnObject(CAPPLICATION_Restart);
	else
		GB.StoreObject(PROP(GB_OBJECT), POINTER(&CAPPLICATION_Restart));

END_PROPERTY

//-------------------------------------------------------------------------

BEGIN_PROPERTY(Screens_Count)

	GB.ReturnInteger(NUM_SCREENS());

END_PROPERTY


BEGIN_METHOD(Screens_get, GB_INTEGER screen)

	GB.ReturnObject(get_screen(VARG(screen)));

END_METHOD


BEGIN_METHOD_VOID(Screens_next)

	int *index = (int *)GB.GetEnum();

	if (*index >= NUM_SCREENS())
		GB.StopEnum();
	else
	{
		GB.ReturnObject(get_screen(*index));
		(*index)++;
	}
	
END_METHOD


BEGIN_PROPERTY(Screen_X)

	GB.ReturnInteger(QApplication::desktop()->screenGeometry(SCREEN->index).x());

END_PROPERTY

BEGIN_PROPERTY(Screen_Y)

	GB.ReturnInteger(QApplication::desktop()->screenGeometry(SCREEN->index).y());

END_PROPERTY

BEGIN_PROPERTY(Screen_Width)

	GB.ReturnInteger(QApplication::desktop()->screenGeometry(SCREEN->index).width());

END_PROPERTY

BEGIN_PROPERTY(Screen_Height)

	GB.ReturnInteger(QApplication::desktop()->screenGeometry(SCREEN->index).height());

END_PROPERTY


BEGIN_PROPERTY(Screen_AvailableX)

	GB.ReturnInteger(QApplication::desktop()->availableGeometry(SCREEN->index).x());

END_PROPERTY

BEGIN_PROPERTY(Screen_AvailableY)

	GB.ReturnInteger(QApplication::desktop()->availableGeometry(SCREEN->index).y());

END_PROPERTY

BEGIN_PROPERTY(Screen_AvailableWidth)

	GB.ReturnInteger(QApplication::desktop()->availableGeometry(SCREEN->index).width());

END_PROPERTY

BEGIN_PROPERTY(Screen_AvailableHeight)

	GB.ReturnInteger(QApplication::desktop()->availableGeometry(SCREEN->index).height());

END_PROPERTY

//-------------------------------------------------------------------------

GB_DESC ScreenDesc[] =
{
	GB_DECLARE("Screen", sizeof(CSCREEN)), GB_NOT_CREATABLE(), GB_AUTO_CREATABLE(),

	GB_PROPERTY_READ("X", "i", Screen_X),
	GB_PROPERTY_READ("Y", "i", Screen_Y),
	GB_PROPERTY_READ("W", "i", Screen_Width),
	GB_PROPERTY_READ("H", "i", Screen_Height),
	GB_PROPERTY_READ("Width", "i", Screen_Width),
	GB_PROPERTY_READ("Height", "i", Screen_Height),

	GB_PROPERTY_READ("AvailableX", "i", Screen_AvailableX),
	GB_PROPERTY_READ("AvailableY", "i", Screen_AvailableY),
	GB_PROPERTY_READ("AvailableWidth", "i", Screen_AvailableWidth),
	GB_PROPERTY_READ("AvailableHeight", "i", Screen_AvailableHeight),

	GB_END_DECLARE
};

GB_DESC ScreensDesc[] =
{
	GB_DECLARE("Screens", 0), GB_VIRTUAL_CLASS(),

	GB_STATIC_PROPERTY_READ("Count", "i", Screens_Count),
	GB_STATIC_METHOD("_get", "Screen", Screens_get, "(Screen)i"),
	GB_STATIC_METHOD("_next", "Screen", Screens_next, NULL),
	
	GB_END_DECLARE
};

GB_DESC DesktopDesc[] =
{
	GB_DECLARE("Desktop", 0), GB_VIRTUAL_CLASS(),

	GB_STATIC_PROPERTY_READ("X", "i", Desktop_X),
	GB_STATIC_PROPERTY_READ("Y", "i", Desktop_Y),
	GB_STATIC_PROPERTY_READ("W", "i", Desktop_Width),
	GB_STATIC_PROPERTY_READ("H", "i", Desktop_Height),
	GB_STATIC_PROPERTY_READ("Width", "i", Desktop_Width),
	GB_STATIC_PROPERTY_READ("Height", "i", Desktop_Height),

	GB_CONSTANT("Charset", "s", "UTF-8"),
	GB_STATIC_PROPERTY_READ("Resolution", "i", Desktop_Resolution),
	GB_STATIC_PROPERTY_READ("Scale", "i", Desktop_Scale),

	GB_STATIC_PROPERTY_READ("HasSystemTray", "b", Desktop_HasSystemTray),
	
	GB_STATIC_METHOD("Screenshot", "Picture", Desktop_Screenshot, "[(X)i(Y)i(Width)i(Height)i]"),

	GB_END_DECLARE
};

GB_DESC ApplicationDesc[] =
{
	GB_DECLARE("Application", 0), GB_VIRTUAL_CLASS(),

	GB_STATIC_METHOD("_exit", NULL, Application_exit, NULL),
	GB_STATIC_PROPERTY("Font", "Font", Application_Font),
	GB_STATIC_PROPERTY_READ("ActiveWindow", "Window", Application_ActiveWindow),
	GB_STATIC_PROPERTY_READ("ActiveControl", "Control", Application_ActiveControl),
	GB_STATIC_PROPERTY_READ("PreviousControl", "Control", Application_PreviousControl),
	GB_STATIC_PROPERTY("MainWindow", "Window", Application_MainWindow),
	GB_STATIC_PROPERTY("Busy", "i", Application_Busy),
	GB_STATIC_PROPERTY("ShowTooltips", "b", Application_ShowTooltips),
	GB_STATIC_PROPERTY("Embedder", "i", Application_Embedder),
	GB_STATIC_PROPERTY("Theme", "s", Application_Theme),
	GB_STATIC_PROPERTY("Restart", "String[]", Application_Restart),

	GB_END_DECLARE
};
