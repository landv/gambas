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
#include "gglarea.h"
#include "gkey.h"

#include "x11.h"
#include "CScreen.h"
#include "CStyle.h"
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
#include "CSlider.h"
#include "CTabStrip.h"
#include "CTrayIcon.h"
#include "CScrollView.h"
#include "CSpinBox.h"
#include "CStock.h"
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
static void my_main(int *argc, char ***argv);
static void my_timer(GB_TIMER *timer,bool on);
static void my_wait(int duration);
static void my_post(void);
static int my_loop();
static void my_watch(int fd, int type, void *callback, intptr_t param);

static GtkWidget *GTK_CreateGLArea(void *_object, void *parent, void (*init)(GtkWidget *));

static bool _post_check = false;
static bool _must_check_quit = false;

static bool _application_keypress = false;
static GB_FUNCTION _application_keypress_func;

static void *_old_hook_main;

int MAIN_scale = 0;
bool MAIN_debug_busy = false;

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
		CPointerDesc,
		CMessageDesc,
		CDialogDesc,
		CWatcherDesc,
		CWidgetDesc,
		CChildrenDesc,
		CContainerDesc,
		CDrawingAreaDesc,
		CFrameDesc,
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
		CTextBoxSelectionDesc,
		CTextBoxDesc,
		CTextAreaDesc,
		CTextAreaSelectionDesc,
		CComboBoxDesc,
		CComboBoxItemDesc,
		CTabStripDesc,
		CTabStripContainerDesc,
		CTabStripContainerChildrenDesc,
		CPluginDesc,
		CScrollViewDesc,
		CSpinBoxDesc,
		CSeparatorDesc,
		CStockDesc,
		PrinterDesc,
		SvgImageDesc,
		NULL
	};

	void *GB_GTK_1[] EXPORT =
	{
		(void *)GTK_INTERFACE_VERSION,
		(void *)GTK_CreateGLArea,
		NULL
	};

	const char *GB_INCLUDE EXPORT = "gb.draw,gb.gui.base";

	int EXPORT GB_INIT(void)
	{
		char *env;
		
		env = getenv("GB_GUI_BUSY");
		if (env && atoi(env))
			MAIN_debug_busy = true;
		
		GB.Hook(GB_HOOK_QUIT, (void *)my_quit);
		_old_hook_main = GB.Hook(GB_HOOK_MAIN, (void *)my_main);
		GB.Hook(GB_HOOK_WAIT, (void *)my_wait);
		GB.Hook(GB_HOOK_LOOP, (void *)my_loop);
		GB.Hook(GB_HOOK_TIMER,(void *)my_timer);
		GB.Hook(GB_HOOK_WATCH,(void *)my_watch);
		GB.Hook(GB_HOOK_POST,(void*)my_post);
		GB.Hook(GB_HOOK_ERROR,(void*)my_error);
		GB.Hook(GB_HOOK_LANG,(void*)my_lang);

		GB.Component.Load("gb.draw");
		GB.Component.Load("gb.image");
		GB.Component.Load("gb.gui.base");
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
		
#if !defined(GLIB_VERSION_2_36)
		g_type_init();
#endif /* !defined(GLIB_VERSION_2_36) */
		
		my_lang(GB.System.Language(), GB.System.IsRightToLeft());

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
		else if (!strcasecmp(key, "GET_HANDLE"))
		{
			*value = (void *)CWIDGET_get_handle;
			return TRUE;
		}
		else if (!strcasecmp(key, "SET_EVENT_FILTER"))
		{
			*value = (void *)gApplication::setEventFilter;
			return TRUE;
		}
		else if (!strcasecmp(key, "TIME"))
		{
			*value = (void *)(intptr_t)gtk_get_current_event_time(); //gdk_x11_display_get_user_time(gdk_display_get_default());
			return TRUE;
		}
		else
			return FALSE;
	}

	static void activate_main_window(intptr_t)
	{
		if (gMainWindow::_active)
			gtk_window_present(GTK_WINDOW(gMainWindow::_active->topLevel()->border));
	}
	
	void EXPORT GB_SIGNAL(int signal, void *param)
	{
		static GtkWidget *save_popup_grab = NULL;
		
		switch(signal)
		{
			case GB_SIGNAL_DEBUG_BREAK:
				if (gApplication::_popup_grab)
				{
					save_popup_grab = gApplication::_popup_grab;
					gApplication::ungrabPopup();
				}
				break;
				
			case GB_SIGNAL_DEBUG_FORWARD:
				//while (qApp->activePopupWidget())
				//	delete qApp->activePopupWidget();
				if (gdk_display_get_default())
				gdk_display_sync(gdk_display_get_default());
				break;
				
			case GB_SIGNAL_DEBUG_CONTINUE:
				GB.Post((GB_CALLBACK)activate_main_window, 0);
				if (save_popup_grab)
				{
					gApplication::_popup_grab = save_popup_grab;
					save_popup_grab = NULL;
					gApplication::grabPopup();
				}
				break;
		}
	}
}

void my_quit (void)
{
	while (gtk_events_pending())
		gtk_main_iteration();
  
	CWINDOW_delete_all();
	gControl::cleanRemovedControls();

	CWatcher::Clear();
	gApplication::exit();

	#ifdef GDK_WINDOWING_X11
  	X11_exit();
  #endif
}

static bool global_key_event_handler(int type)
{
	GB.Call(&_application_keypress_func, 0, FALSE);
	return GB.Stopped();
}

static void my_main(int *argc, char ***argv)
{
	static bool init = false;
	char *env;
	
	if (init)
		return;
	
	env = getenv("GB_X11_INIT_THREADS");
		if (env && atoi(env))
	XInitThreads();

	gApplication::init(argc, argv);
	gApplication::setDefaultTitle(GB.Application.Title());
	gDesktop::init();
	
	gApplication::onEnterEventLoop = GB.Debug.EnterEventLoop;
	gApplication::onLeaveEventLoop = GB.Debug.LeaveEventLoop;
		
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

	CALL_HOOK_MAIN(_old_hook_main, argc, argv);
}

/*static void raise_timer(GB_TIMER *timer)
{
	GB.RaiseTimer(timer);
	GB.Unref(POINTER(&timer));
}*/

typedef
	struct {
		int source;
		GTimer *timer;
		int timeout;
		}
	MyTimerId;

gboolean my_timer_function(GB_TIMER *timer)
{
	if (timer->id)  
	{
		GB.RaiseTimer(timer);
		
		if (timer->id)
		{
			MyTimerId *id = (MyTimerId *)timer->id;
			GTimer *t = id->timer;
			int elapsed = (int)(g_timer_elapsed(t, NULL) * 1000) - id->timeout;
			int next = timer->delay - elapsed;
			if (next < 10)
				next = 10;
			id->timeout = next;
			g_timer_start(t);
			id->source = g_timeout_add(next, (GSourceFunc)my_timer_function,(gpointer)timer);
			//timer->id = (intptr_t)g_timeout_add(next, (GSourceFunc)my_timer_function,(gpointer)timer);
			//fprintf(stderr, "elapsed = %d  delay = %d  next = %d\n", elapsed, timer->delay, next);
		}
	}
	
	return false;
}

static void my_timer(GB_TIMER *timer,bool on)
{
	if (timer->id)
	{
		MyTimerId *id = (MyTimerId *)timer->id;
		g_source_remove(id->source);
		g_timer_destroy(id->timer);
		g_free(id);
		timer->id = 0;
	}

	if (on)
	{
		MyTimerId *id = g_new(MyTimerId, 1);
		id->timer = g_timer_new();
		id->timeout = timer->delay;
		id->source = (intptr_t)g_timeout_add(timer->delay,(GSourceFunc)my_timer_function,(gpointer)timer);
		timer->id = (intptr_t)id;
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
			if (gApplication::mustQuit())
				break;
			if (CWINDOW_must_quit() && CWatcher::count() == 0 && gTrayIcon::visibleCount() == 0)
				break;
			_must_check_quit = false;
		}
		MAIN_do_iteration(false);
	}

	my_quit();
	
  return 0;
}

static void my_wait(int duration)
{
	if (duration > 0 && gKey::valid())
	{
#ifdef GTK3
		fprintf(stderr, "gb.gtk3: warning: calling the event loop during a keyboard event handler is ignored\n");
#else
		fprintf(stderr, "gb.gtk: warning: calling the event loop during a keyboard event handler is ignored\n");
#endif
		return;
	}

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

static void my_lang(char *lang, int rtl)
{
	int bucle, ct;
	gControl *iter;

	if (rtl)
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

void MAIN_do_iteration_just_events()
{
	if (gtk_events_pending())
		gtk_main_iteration_do(false);
}

void MAIN_do_iteration(bool do_not_block, bool do_not_sleep)
{
	gApplication::_loopLevel++;

	if (do_not_block)
	{
		if (gtk_events_pending ())
			gtk_main_iteration_do (false);
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

static GtkWidget *GTK_CreateGLArea(void *_object, void *parent, void (*init)(GtkWidget *))
{
	gControl *ctrl = new gGLArea(CONTAINER(parent), init);
	InitControl(ctrl, (CWIDGET*)_object);
	//WIDGET->onExpose = Darea_Expose;
	return ctrl->widget;
}

