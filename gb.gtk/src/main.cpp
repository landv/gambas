/***************************************************************************

  main.cpp

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

#define __MAIN_C

#include <stdio.h>

#include "gmemory.h"
#include "main.h"
#include "gb.image.h"
#include "gb.gtk.h"
#include "watcher.h"

#include "x11.h"
#include "CScreen.h"
#include "CDraw.h"
#include "CConst.h"
#include "CColor.h"
#include "CFont.h"
#include "CKey.h"
#include "CPicture.h"
#include "CImage.h"
#include "CClipboard.h"
#include "CMouse.h"
#include "CMessage.h"
#include "CDialog.h"
#include "CWatcher.h"
#include "CWidget.h"
#include "CProgress.h"
#include "CDrawingArea.h"
#include "CContainer.h"
#include "CFrame.h"
#include "CMenu.h"
#include "CWindow.h"
#include "CLabel.h"
#include "CButton.h"
#include "CPictureBox.h"
#include "CTextBox.h"
#include "CTextArea.h"
#include "CListBox.h"
#include "CTreeView.h"
#include "CSlider.h"
#include "CTabStrip.h"
#include "CTrayIcon.h"
#include "CScrollView.h"
#include "CSpinBox.h"
#include "CSplitter.h"
#include "CStock.h"
#include "CGridView.h"
#include "CSeparator.h"
#include "cprinter.h"
#include "csvgimage.h"

#include <gtk/gtk.h>
#include <string.h>

GB_CLASS CLASS_Picture;
GB_CLASS CLASS_Image;
GB_CLASS CLASS_DrawingArea;
GB_CLASS CLASS_Menu;
GB_CLASS CLASS_Window;
GB_CLASS CLASS_Printer;
GB_CLASS CLASS_SvgImage;

static void my_lang(char *lang,int rtl1);
static void my_error(int code,char *error,char *where);
static void my_quit (void);
static void my_main(int *argc, char **argv);
static void my_timer(GB_TIMER *timer,bool on);
static void my_wait(int duration);
static void my_post(void);
static int my_loop();
static void my_watch(int fd, int type, void *callback, intptr_t param);

static bool _post_check = false;
static bool _must_check_quit = false;

static bool _application_keypress = false;
static GB_FUNCTION _application_keypress_func;

int MAIN_scale = 0;

extern "C"
{
	GB_INTERFACE GB EXPORT;
	IMAGE_INTERFACE IMAGE EXPORT;

	GB_DESC *GB_CLASSES[] EXPORT =
	{
		ScreenDesc,
		ScreensDesc,
		DesktopDesc,
		ApplicationDesc,
		StyleDesc,
		CSelectDesc,
		CAlignDesc,
		CArrangeDesc,
		CBorderDesc,
		CScrollDesc,
		CLineDesc,
		CFillDesc,
		CColorDesc,
		CFontsDesc,
		CFontDesc,
		CKeyDesc,
		CImageDesc,
		CPictureDesc,
		CClipboardDesc,
		CDragDesc,
		CCursorDesc,
		CMouseDesc,
		CMessageDesc,
		CDialogDesc,
		CWatcherDesc,
		CWidgetDesc,
		CChildrenDesc,
		CContainerDesc,
		CDrawingAreaDesc,
		CFrameDesc,
		CHSplitDesc,
		CVSplitDesc,
		CUserControlDesc,
		CUserContainerDesc,
		CPanelDesc,
		CHBoxDesc,
		CVBoxDesc,
		CHPanelDesc,
		CVPanelDesc,
		CMenuDesc,
		CMenuChildrenDesc,
		CTrayIconDesc,
		CTrayIconsDesc,
		CWindowMenusDesc,
		CWindowControlsDesc,
		CWindowDesc,
		CWindowsDesc,
		CFormDesc,
		CProgressDesc,
		CLabelDesc,
		CTextLabelDesc,
		CSliderDesc,
		CScrollBarDesc,
		CButtonDesc,
		CToggleButtonDesc,
		CCheckBoxDesc,
		CRadioButtonDesc,
		CToolButtonDesc,
		CPictureBoxDesc,
		CMovieBoxDesc,
		CListBoxDesc,
		CListBoxItemDesc,
		CTextBoxSelectionDesc,
		CTextBoxDesc,
		CTextAreaDesc,
		CTextAreaSelectionDesc,
		CComboBoxDesc,
		CComboBoxItemDesc,
		CListViewItemDesc,
		CListViewDesc,
		CTreeViewItemDesc,
		CTreeViewDesc,
		CColumnViewItemDesc,
		CColumnViewColumnDesc,
		CColumnViewColumnsDesc,
		CColumnViewDesc,
		CTabStripDesc,
		CTabDesc,
		CTabChildrenDesc,
		CPluginDesc,
		CScrollViewDesc,
		CSpinBoxDesc,
		CSeparatorDesc,
		CGridViewItemDesc,
		CGridViewDataDesc,
		CGridViewColumnDesc,
		CGridViewRowDesc,
		CGridViewColumnsDesc,
		CGridViewRowsDesc,
		CGridViewDesc,
		CStockDesc,
		PrinterDesc,
		SvgImageDesc,
		NULL
	};

	void *GB_GTK_1[] EXPORT =
	{
		(void *)GTK_INTERFACE_VERSION,
		(void *)my_main,
		(void *)GTK_GetPicture,
		(void *)GTK_GetImage,
		(void *)CGRIDVIEW_footer,
		(void *)CGRIDVIEW_column_footer_text,
		(void *)CGRIDVIEW_columns_get,
		(void *)DRAW_get_drawable,
		(void *)DRAW_get_style,
		(void *)DRAW_get_state,
		(void *)DRAW_get_shadow,
		(void *)DRAW_set_state,
		(void *)DRAW_set_shadow,
		NULL
	};

	const char *GB_INCLUDE EXPORT = "gb.draw";

	int EXPORT GB_INIT(void)
	{
		GB.Hook(GB_HOOK_QUIT, (void *)my_quit);
		GB.Hook(GB_HOOK_MAIN, (void *)my_main);
		GB.Hook(GB_HOOK_WAIT, (void *)my_wait);
		GB.Hook(GB_HOOK_LOOP, (void *)my_loop);
		GB.Hook(GB_HOOK_TIMER,(void *)my_timer);
		GB.Hook(GB_HOOK_WATCH,(void *)my_watch);
		GB.Hook(GB_HOOK_POST,(void*)my_post);
		GB.Hook(GB_HOOK_ERROR,(void*)my_error);
		GB.Hook(GB_HOOK_LANG,(void*)my_lang);

		// Thanks again to GTK+ 2.18 :-(
		#if GTK_MAJOR_VERSION >= 2 && GTK_MINOR_VERSION >= 18
		//putenv((char *)"GDK_NATIVE_WINDOWS=1");
		#endif
		
		GB.LoadComponent("gb.draw");
		GB.GetInterface("gb.image", IMAGE_INTERFACE_VERSION, &IMAGE);
		IMAGE.SetDefaultFormat(GB_IMAGE_RGBA);
		DRAW_init();
		
		CWatcher::init();

		//CLASS_Control = GB.FindClass("Control");
		//CLASS_Container = GB.FindClass("Container");
		//CLASS_UserControl = GB.FindClass("UserControl");
		//CLASS_UserContainer = GB.FindClass("UserContainer");
		CLASS_Window = GB.FindClass("Window");
		CLASS_Menu = GB.FindClass("Menu");
		CLASS_Picture = GB.FindClass("Picture");
		//CLASS_Drawing = GB.FindClass("Drawing");
		CLASS_DrawingArea = GB.FindClass("DrawingArea");
		CLASS_Printer = GB.FindClass("Printer");
		CLASS_Image = GB.FindClass("Image");
		CLASS_SvgImage = GB.FindClass("SvgImage");
		
		return -1;
	}

	void EXPORT GB_EXIT()
	{
		CWatcher::exit();
	}

	int EXPORT GB_INFO(const char *key, void **value)
	{
		if (!strcasecmp(key, "DISPLAY"))
		{
			#ifdef GDK_WINDOWING_X11
			#ifndef GAMBAS_DIRECTFB			
			*value = (void *)gdk_x11_display_get_xdisplay(gdk_display_get_default());
			#else
			// For DirectFB
			*value=0;
			stub("DIRECTFB/EXPORT GB_INFO");
			#endif
			#else
			// For Win32
			*value=0;
			stub("no-X11/EXPORT GB_INFO");
			#endif
			return TRUE;
		}
		else if (!strcasecmp(key, "ROOT_WINDOW"))
		{
			#ifdef GDK_WINDOWING_X11
			#ifndef GAMBAS_DIRECTFB
			*value = (void *)gdk_x11_get_default_root_xwindow();
			#else
			// For DirectFB
			*value=0;
			stub("DIRECTFB/EXPORT GB_INFO");
			#endif
			#else
			// For Win32
			*value=0;
			stub("no-X11/EXPORT GB_INFO");
			#endif
			return TRUE;
		}
		else
			return FALSE;
	}
}

void my_quit (void)
{
	CWINDOW *win = CWINDOW_Main;

	gApplication::suspendEvents(false);
	if (win)
	{
		while (gtk_events_pending ()) gtk_main_iteration();
		if (win->ob.widget) ((gMainWindow*)win->ob.widget)->destroy();
		while (gtk_events_pending ()) gtk_main_iteration();
	}

}

static bool global_key_event_handler(int type)
{
	GB.Call(&_application_keypress_func, 0, FALSE);
	return GB.Stopped();
}

static void my_main(int *argc, char **argv)
{
	static bool init = false;
	
	if (init)
		return;
	
	gApplication::init(argc, &argv);
	gApplication::setDefaultTitle(GB.Application.Title());
	gDesktop::init();
	MAIN_scale = gDesktop::scale();
	#ifdef GDK_WINDOWING_X11
  	X11_init(gdk_x11_display_get_xdisplay(gdk_display_get_default()), gdk_x11_get_default_root_xwindow());
  #endif

	if (GB.GetFunction(&_application_keypress_func, (void *)GB.Application.StartupClass(), "Application_KeyPress", "", "") == 0)
	{
		_application_keypress = true;
		gApplication::onKeyEvent = global_key_event_handler;
	}

	init = true;
}

/*static void raise_timer(GB_TIMER *timer)
{
	GB.RaiseTimer(timer);
	GB.Unref(POINTER(&timer));
}*/

typedef
	struct {
		GTimer *timer;
		int timeout;
		}
	MyTimerTag;

gboolean my_timer_function(GB_TIMER *timer)
{
	if (timer->id)  
	{
		GB.RaiseTimer(timer);
		
		if (timer->id)
		{
			MyTimerTag *tag = (MyTimerTag *)timer->tag;
			GTimer *t = tag->timer;
			int elapsed = (int)(g_timer_elapsed(t, NULL) * 1000) - tag->timeout;
			int next = timer->delay - elapsed;
			if (next < 10)
				next = 10;
			tag->timeout = next;
			g_timer_start(t);
			timer->id = (intptr_t)g_timeout_add(next, (GSourceFunc)my_timer_function,(gpointer)timer);
			//fprintf(stderr, "elapsed = %d  delay = %d  next = %d\n", elapsed, timer->delay, next);
		}
	}
	
	return false;
}

static void my_timer(GB_TIMER *timer,bool on)
{
	if (timer->id) {
		MyTimerTag *tag = (MyTimerTag *)timer->tag;
		g_source_remove(timer->id);
		g_timer_destroy(tag->timer);
		g_free(tag);
		timer->id = 0;
		timer->tag = 0;
	}

	if (on)
	{
		MyTimerTag *tag = g_new(MyTimerTag, 1);
		tag->timer = g_timer_new();
		tag->timeout = timer->delay;
		timer->tag = (intptr_t)tag;
		timer->id = (intptr_t)g_timeout_add(timer->delay,(GSourceFunc)my_timer_function,(gpointer)timer);
		return;
	}
}

static void my_post(void)
{
	_post_check = true;
}

void MAIN_check_quit()
{
	_must_check_quit = true;
}

static int my_loop()
{
	gControl::cleanRemovedControls();
	_must_check_quit = true;
	
	for(;;)
	{
		if (_must_check_quit)
		{
			if (CWINDOW_must_quit() && CWatcher::count() == 0)
				break;
			_must_check_quit = false;
		}
		MAIN_do_iteration(false);
	}

	while (gtk_events_pending())
		gtk_main_iteration();
  
	CWINDOW_delete_all();
	gControl::cleanRemovedControls();

	CWatcher::Clear();
	gApplication::exit();

  return 0;
}

static void my_wait(int duration)
{
	MAIN_do_iteration(true, true);
}

static void my_watch(int fd, int type, void *callback, intptr_t param)
{
	CWatcher::Add(fd,type,callback,param);
}

static void my_error(int code,char *error,char *where)
{
	char *showstr;
	char scode[10];

	sprintf(scode,"%d",code);

	showstr=g_strconcat("<b>This application has raised an unexpected<br>error and must abort.</b><p>[", scode, "] ", error, ".\n", where, (void *)NULL);

	gMessage::setTitle(GB.Application.Title());
	gMessage::showError(showstr,NULL,NULL,NULL);

	g_free(showstr);
}

static void my_lang(char *lang,int rtl)
{
	int bucle, ct;
	gControl *iter;

	if (rtl==1)
		gtk_widget_set_default_direction(GTK_TEXT_DIR_RTL);
	else
		gtk_widget_set_default_direction(GTK_TEXT_DIR_LTR);

	ct=gApplication::controlCount();
	for (bucle=0;bucle<ct;bucle++)
	{
		iter=gApplication::controlItem(bucle);
		if (iter->isVisible() && iter->isContainer())
			((gContainer*)iter)->performArrange();
	}
}

void MAIN_do_iteration(bool do_not_block, bool do_not_sleep)
{
	struct timespec mywait;

	gApplication::_loopLevel++;

	if (do_not_block)
	{
		if (gtk_events_pending ())
			gtk_main_iteration_do (false);
		else if (!do_not_sleep)
		{
			mywait.tv_sec=0;
			mywait.tv_nsec=100000;
			nanosleep(&mywait,NULL);
		}
	}
	else	
		gtk_main_iteration_do(true);
	
	gApplication::_loopLevel--;

	if (_post_check)
	{
		_post_check = false;
		GB.CheckPost();
	}
	
	gControl::cleanRemovedControls();
}
