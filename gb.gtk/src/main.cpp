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
#include "gambas.h"
#include "main.h"
#include "widgets.h"
#include "watcher.h"

#include "CScreen.h"
#include "CDraw.h"
#include "CConst.h"
#include "CColor.h"
#include "CFont.h"
#include "CKey.h"
#include "CImage.h"
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
#include "CListView.h"
#include "CTreeView.h"
#include "CColumnView.h"
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
static void my_wait(long duration);
static void my_post(void);
static int my_loop();
static void my_watch(int fd, int type, void *callback, long param);
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
		CDrawDesc,
		CDrawClipDesc,
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
		CWindowDesc,
		CWindowsDesc,
		CFormDesc,
		CProgressDesc,
		CLabelDesc,
		CTextViewDesc,
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
		(void *)1,
		(void *)GTK_GetPicture,
		(void *)GTK_GetImage,
		NULL
	};

	int EXPORT GB_INIT(void)
	{
		setGeneralMemoryHandler();
		CWatcher::Init();
		gMessage::setTitle(GB.Application.Name());
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

		return TRUE;
	}

	void EXPORT GB_EXIT()
	{
		gFont::exit();
		gDesktop::exit();
		CWatcher::Clear();
		gMessage::setTitle(NULL);
		gDialog::exit();
		gDialog::setFilter(NULL,0);
		gApplication::exit();

	}



}

void my_quit (void)
{
	CWINDOW *win=WINDOW_get_main();

	gApplication::suspendEvents(false);
	if (win)
	{
		while (gtk_events_pending ()) gtk_main_iteration();
		if (win->widget) ((gMainWindow*)win->widget)->destroy();
		while (gtk_events_pending ()) gtk_main_iteration();
	}

}

static void my_main(int *argc, char **argv)
{
	gApplication::init(argc,&argv);
	gDesktop::init();
	MAIN_scale=gDesktop::scale();
}

gboolean my_timer_function(GB_TIMER *timer)
{
	if (timer->id)  GB.RaiseTimer(timer);
	if (!timer->id) return false;
	return true;
}

static void my_timer(GB_TIMER *timer,bool on)
{
	if (timer->id) {
		g_source_remove(timer->id);
		timer->id=0;
	}

	if (on)
	{
		timer->id=(long)g_timeout_add(timer->delay,(GSourceFunc)my_timer_function,(gpointer)timer);
		return;
	}
}

static void my_post(void)
{
	post_Check=true;
}

static int my_loop()
{
	while (1)
	{
		do_iteration();
		gControl::cleanRemovedControls();
		if (!WINDOW_get_main()) break;
	}

	while (gtk_events_pending ()) gtk_main_iteration();
        return 0;
}

static void my_wait(long duration)
{
	do_iteration();
}

static void my_watch(int fd, int type, void *callback, long param)
{
	CWatcher::Add(fd,type,callback,param);
}

static void my_error(int code,char *error,char *where)
{
	char *showstr;
	char scode[10];
	char *msg="<b>This application has raised an unexpected<br>error and must abort.</b><p>[";

	sprintf(scode,"%d",code);

	showstr=g_strconcat(msg,scode,"] ",error,".\n",where,NULL);

	gMessage::setTitle(GB.Application.Name());
	gMessage::showError(showstr,NULL,NULL,NULL);

	g_free(showstr);
}

static void my_lang(char *lang,int rtl)
{
	long bucle,ct;
	gControl *iter;

	if (rtl==1)
		gtk_widget_set_default_direction(GTK_TEXT_DIR_RTL);
	else
		gtk_widget_set_default_direction(GTK_TEXT_DIR_LTR);

	ct=gApplication::controlCount();
	for (bucle=0;bucle<ct;bucle++)
	{
		iter=gApplication::controlItem(bucle);
		if (iter->visible())
			if (iter->getClass() & 0x0100)
				((gContainer*)iter)->performArrange();
	}
}

void do_iteration(void)
{
	gApplication::iteration();
	if (post_Check) { post_Check=false; GB.CheckPost(); }
	CWatcher::Perform();
}

#if 0
static void convert_image_data(void *dst, void *src, int w, int h, int format)
{
	int i;
	char *s;
	char *d;
	/*unsigned long *ls;
	unsigned long *ld;
	unsigned long tmp;*/

	switch (format)
	{
		case GB_IMAGE_RGBA: case GB_IMAGE_RGBX:
			memcpy(dst, src, w * h * 4);
			break;

		case GB_IMAGE_ARGB: case GB_IMAGE_XRGB:
			s = (char *)src;
			d = (char *)dst;
			for (i = 0; i < (w * h); i++)
			{
				d[0] = s[3];
				d[1] = s[0];
				d[2] = s[1];
				d[3] = s[2];
				s += 4;
				d += 4;
			}
			break;

		case GB_IMAGE_BGRA:
			s = (char *)src;
			d = (char *)dst;
			for (i = 0; i < (w * h); i++)
			{
				d[0] = s[2];
				d[1] = s[1];
				d[2] = s[0];
				d[3] = s[3];
				s += 4;
				d += 4;
			}
			break;

		case GB_IMAGE_BGRX:
			s = (char *)src;
			d = (char *)dst;
			for (i = 0; i < (w * h); i++)
			{
				//tmp = ls[i];
				//ld[i] = ((tmp & 0xFF) << 16) | (tmp & 0xFF00) | ((tmp & 0xFF0000) >> 16) | 0xFF000000;
				d[0] = s[2];
				d[1] = s[1];
				d[2] = s[0];
				d[3] = 0xFF;
				s += 4;
				d += 4;
			}
			break;

		case GB_IMAGE_BGR:
			s = (char *)src;
			d = (char *)dst;
			for (i = 0; i < (w * h); i++)
			{
				d[2] = s[0];
				d[1] = s[1];
				d[0] = s[2];
				d[3] = 0xFF;
				s += 3;
				d += 4;
			}
			break;

		case GB_IMAGE_RGB:
			s = (char *)src;
			d = (char *)dst;
			for (i = 0; i < (w * h); i++)
			{
				d[2] = s[2];
				d[1] = s[1];
				d[0] = s[0];
				d[3] = 0xFF;
				s += 3;
				d += 4;
			}
			break;
	}
}
#endif

// BM: New image & picture hook syntax
// BM: Conversion functions are now in the interpreter API

static int hook_image(CIMAGE **pimage, GB_IMAGE_INFO *info)
{
	CIMAGE *image = *pimage;
	gImage *img;
	const guchar *buf;

	if (!image)
	{
		img=new gImage(info->width,info->height);
		buf=gdk_pixbuf_get_pixels(img->image);

		if (info->data) 
		  GB.Image.Convert((void *)buf, GB_IMAGE_RGBA, info->data, info->format, info->width, info->height);

		GB.New((void **)(void *)&image, GB.FindClass("Image"), NULL, NULL);
		delete image->image;
		image->image = img;
		*pimage = image;
	}
	else
	{
		info->data = gdk_pixbuf_get_pixels(image->image->image);
		info->width = image->image->width();
		info->height = image->image->height();
		info->format = GB_IMAGE_RGBA;
	}

	return 0;
}

#if 0
static int hook_picture(CPICTURE **ppicture, void *data, int width, int height, int format)
{

	CPICTURE *picture=NULL;
	GdkPixbuf *img;
	void *buf;

	if (format == GB_IMAGE_ARGB)
	{
		img = gdk_pixbuf_new_from_data((const guchar*)data,GDK_COLORSPACE_RGB,true,8,width,height,4*width,NULL,NULL);
	}
	else
	{
		img = gdk_pixbuf_new(GDK_COLORSPACE_RGB,true,8,width,height);
		buf = (void*)gdk_pixbuf_get_pixels(img);
		convert_image_data(buf, data, width, height, format);
	}

	GB.New((void **)&picture, GB.FindClass("Picture"), NULL, NULL);
	delete picture->picture;
	picture->picture = gPicture::fromPixbuf(img);
	g_object_unref(G_OBJECT(img));

	*ppicture = picture;
	return 0;
}
#endif

static int hook_picture(CPICTURE **ppicture, GB_PICTURE_INFO *info)
{
	CPICTURE *picture=*ppicture;
	GdkPixbuf *img;
	void *buf;

	if (!picture)
	{
		if (info->format == GB_IMAGE_RGBA || info->format == GB_IMAGE_RGBX)
		{
			img = gdk_pixbuf_new_from_data((const guchar*)info->data,GDK_COLORSPACE_RGB,true,8,info->width,info->height,4*info->width,NULL,NULL);
		}
		else
		{
			img = gdk_pixbuf_new(GDK_COLORSPACE_RGB,true,8,info->width,info->height);
			buf = (void*)gdk_pixbuf_get_pixels(img);
			//convert_image_data(buf, info->data, info->width, info->height, info->format);
		  GB.Image.Convert(buf, GB_IMAGE_RGBA, info->data, info->format, info->width, info->height);
    }

		GB.New((void **)(void *)&picture, GB.FindClass("Picture"), NULL, NULL);
		delete picture->picture;
		picture->picture = gPicture::fromPixbuf(img);
		g_object_unref(G_OBJECT(img));

		*ppicture = picture;
	}
	else
	{
		info->data = NULL;
		info->format = GB_IMAGE_BGRA;
		info->width = picture->picture->width();
		info->height = picture->picture->height();
	}

	return 0;
}


