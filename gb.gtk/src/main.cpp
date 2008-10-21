/***************************************************************************

  main.cpp

  (c) 2004-2006 - Daniel Campos Fernández <dcamposf@gmail.com>

  GTK+ component

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

#define __MAIN_C

#include <stdio.h>

#include "gmemory.h"
#include "main.h"
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
#include "CIconView.h"
#include "CGridView.h"
#include "CSeparator.h"

#include <gtk/gtk.h>
#include <string.h>

static int hook_image(CIMAGE **pimage, GB_IMAGE_INFO *info);
static int hook_picture(CPICTURE **ppicture, GB_PICTURE_INFO *info);
static void my_lang(char *lang,int rtl1);
static void my_error(int code,char *error,char *where);
static void my_quit (void);
static void my_main(int *argc, char **argv);
static void my_timer(GB_TIMER *timer,bool on);
static void my_wait(int duration);
static void my_post(void);
static int my_loop();
static void my_watch(int fd, int type, void *callback, intptr_t param);
bool post_Check=false;


int MAIN_scale = 8;



extern "C"
{
	GB_INTERFACE GB EXPORT;

	GB_DESC *GB_CLASSES[] EXPORT =
	{
		CDesktopDesc,
		CApplicationTooltipDesc,
		CApplicationDesc,
		CSelectDesc,
		CAlignDesc,
		CArrangeDesc,
		CBorderDesc,
		CScrollDesc,
		CLineDesc,
		CFillDesc,
		CColorDesc,
		CColorInfoDesc,
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
		CIconViewItemDesc,
		CIconViewDesc,
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

		NULL
	};

	void *GB_GTK_1[] EXPORT =
	{
		(void *)GTK_INTERFACE_VERSION,
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
		GB.Hook(GB_HOOK_IMAGE, (void *)hook_image);
    GB.Hook(GB_HOOK_PICTURE, (void *)hook_picture);

		GB.LoadComponent("gb.draw");

		return TRUE;
	}

	void EXPORT GB_EXIT()
	{
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
	CWINDOW *win=WINDOW_get_main();

	gApplication::suspendEvents(false);
	if (win)
	{
		while (gtk_events_pending ()) gtk_main_iteration();
		if (win->ob.widget) ((gMainWindow*)win->ob.widget)->destroy();
		while (gtk_events_pending ()) gtk_main_iteration();
	}

}

static void my_main(int *argc, char **argv)
{
	gApplication::init(argc, &argv);
	gApplication::setDefaultTitle(GB.Application.Title());
	gDesktop::init();
	MAIN_scale = gDesktop::scale();
	#ifdef GDK_WINDOWING_X11
  	X11_init(gdk_x11_display_get_xdisplay(gdk_display_get_default()), gdk_x11_get_default_root_xwindow());
  #endif
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
	post_Check=true;
}

static bool must_quit(void)
{
	//fprintf(stderr, "must_quit: %p %d\n", WINDOW_get_main(), CWatcher::count());
  return !WINDOW_get_main() && CWatcher::count() == 0; // && in_event_loop;
}

static int my_loop()
{
	while (!must_quit())
		do_iteration(false);

	while (gtk_events_pending ()) 	gtk_main_iteration();
  
	CWatcher::Clear();
	gApplication::exit();

  return 0;
}

static void my_wait(int duration)
{
	do_iteration(true, true);
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

	gMessage::setTitle(GB.Application.Name());
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

void do_iteration(bool do_not_block, bool do_not_sleep)
{
	struct timespec mywait;

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
	
	if (post_Check) { post_Check=false; GB.CheckPost(); }
	gControl::cleanRemovedControls();
}

// BM: New image & picture hook syntax
// BM: Conversion functions are now in the interpreter API

static int hook_image(CIMAGE **pimage, GB_IMAGE_INFO *info)
{
	CIMAGE *image = *pimage;
	gPicture *img;
	const guchar *buf;

	if (!image)
	{
		img = new gPicture(gPicture::MEMORY, info->width, info->height, GB_IMAGE_TRANSPARENT(info->format));
		buf = gdk_pixbuf_get_pixels(img->getPixbuf());

		if (info->data) 
		  GB.Image.Convert((void *)buf, GB_IMAGE_RGBA, info->data, info->format, info->width, info->height);

    image = CIMAGE_create(img);
		*pimage = image;
	}
	else
	{
		info->data = gdk_pixbuf_get_pixels(image->picture->getPixbuf());
		info->width = image->picture->width();
		info->height = image->picture->height();
		info->format = image->picture->isTransparent() ? GB_IMAGE_RGBA : GB_IMAGE_RGBX;
	}

	return 0;
}

static int hook_picture(CPICTURE **ppicture, GB_PICTURE_INFO *info)
{
	CPICTURE *picture = *ppicture;
	gPicture *img;
	void *buf;

	if (!picture)
	{
		img = new gPicture(gPicture::MEMORY, info->width, info->height, GB_IMAGE_TRANSPARENT(info->format));
		buf = gdk_pixbuf_get_pixels(img->getPixbuf());

		if (info->data) 
		  GB.Image.Convert((void *)buf, GB_IMAGE_RGBA, info->data, info->format, info->width, info->height);

    picture = CPICTURE_create(img);
		*ppicture = picture;		
	}
	else
	{
		info->data = NULL;
		info->width = picture->picture->width();
		info->height = picture->picture->height();
		info->format = picture->picture->isTransparent() ? GB_IMAGE_RGBA : GB_IMAGE_RGBX;
	}

	return 0;
}


