/***************************************************************************

  gdesktop.cpp

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

#include "widgets.h"
#include "widgets_private.h"

#ifndef GAMBAS_DIRECTFB
#ifdef GDK_WINDOWING_X11
#include <gdk/gdkx.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#endif
#endif

#include "gapplication.h"
#include "gmainwindow.h"
#include "gdesktop.h"

/***********************************************************************

	Cursor

************************************************************************/

gCursor::gCursor(gPicture *pic,int px,int py)
{
	GdkPixbuf *buf;
	GdkDisplay *dp=gdk_display_get_default();
	
	x=px;
	y=py;
	
	cur=NULL;
	if (!pic) return;
	if (!pic->pic) return;
	
	if (pic->width()<=x) x=pic->width()-1;
	if (pic->height()<=y) y=pic->height()-1;
	
	buf=pic->getPixbuf();
	cur=gdk_cursor_new_from_pixbuf(dp,buf,x,y);
	g_object_unref(buf);
	
}

gCursor::gCursor(gCursor *cursor)
{
	cur=NULL;
	if (!cursor) return;
	if (!cursor->cur) return;
	
	cur=cursor->cur;
	x=cursor->x;
	y=cursor->y;
	if (cur) gdk_cursor_ref(cur);
}

gCursor::~gCursor()
{
	if (cur) gdk_cursor_unref(cur);
}

int gCursor::left()
{
	return x;
}

int gCursor::top()
{
	return y;
}

/***********************************************************************

Desktop

************************************************************************/

gControl *gDesktop::_active_control = NULL;
gFont *gDesktop::_desktop_font = NULL;
int gDesktop::_desktop_scale = 0;

bool gDesktop::rightToLeft()
{
	return gtk_widget_get_default_direction() == GTK_TEXT_DIR_RTL;
}

void gDesktop::init()
{
	_desktop_font = new gFont();
	_desktop_scale = 0;
}

void gDesktop::exit()
{
  gFont::assign(&_desktop_font);
}

gFont* gDesktop::font()
{
	if (!_desktop_font) gDesktop::init();
	return _desktop_font;
}

void gDesktop::setFont(gFont *ft)
{
  gFont::set(&_desktop_font, ft->copy());
  _desktop_scale = 0;
}

/*gControl* gDesktop::activeControl()
{
	gControl *test, *curr=NULL;
	GList *iter=gControl::controlList();

	if (!iter) return NULL;

	iter=g_list_first(iter);
	while (iter)
	{
		test=(gControl*)iter->data;
		if (test->hasFocus())
		{
			curr=test;
			break;
		}
		iter=iter->next;
	}
	
	return curr;
	
}*/

void gDesktop::setActiveControl(gControl *control)
{
	_active_control = control;
}

gMainWindow* gDesktop::activeWindow()
{
	return gMainWindow::_active ? gMainWindow::_active->topLevel() : NULL;
}

gColor gDesktop::buttonfgColor()
{
	GtkStyle *st = gt_get_style("GtkButton", GTK_TYPE_BUTTON);

	if (!st) return 0;
	return get_gdk_color(&st->fg[GTK_STATE_NORMAL]);
}

gColor gDesktop::buttonbgColor()
{
	GtkStyle *st = gt_get_style("GtkButton", GTK_TYPE_BUTTON);	

	if (!st) return 0xC0C0C0;
	return get_gdk_color(&st->bg[GTK_STATE_NORMAL]);
}

gColor gDesktop::fgColor()
{	
	GtkStyle *st = gt_get_style("GtkEntry", GTK_TYPE_ENTRY);

	if (!st) return 0;
	return get_gdk_color(&st->fg[GTK_STATE_NORMAL]);
}

gColor gDesktop::bgColor()
{
	GtkStyle *st = gt_get_style("GtkLayout", GTK_TYPE_LAYOUT);

	if (!st) return 0xC0C0C0;
	return get_gdk_color(&st->bg[GTK_STATE_NORMAL]);
}

gColor gDesktop::textfgColor()
{
	GtkStyle *st = gt_get_style("GtkEntry", GTK_TYPE_ENTRY);

	if (!st) return 0;
	return get_gdk_color(&st->text[GTK_STATE_NORMAL]);
}

gColor gDesktop::textbgColor()
{
	GtkStyle *st = gt_get_style("GtkEntry", GTK_TYPE_ENTRY);

	if (!st) return 0xFFFFFF;
	return get_gdk_color(&st->base[GTK_STATE_NORMAL]);
}

gColor gDesktop::selfgColor()
{
	GtkStyle *st = gt_get_style("GtkEntry", GTK_TYPE_ENTRY);

	if (!st) return 0XFFFFFF;
	return get_gdk_color(&st->text[GTK_STATE_SELECTED]);
}

gColor gDesktop::selbgColor()
{
	GtkStyle *st = gt_get_style("GtkEntry", GTK_TYPE_ENTRY);

	if (!st) return 0x0000FF;
	return get_gdk_color(&st->base[GTK_STATE_SELECTED]);
}

int gDesktop::height()
{
	return gdk_screen_get_height(gdk_screen_get_default ());
}

int gDesktop::width()
{
	return gdk_screen_get_width(gdk_screen_get_default ());
}

int gDesktop::resolution()
{
	int d_pix=gdk_screen_get_width(gdk_screen_get_default());
	int d_mm=gdk_screen_get_width_mm(gdk_screen_get_default());
	
	return (int)(d_pix*25.4)/d_mm;
}

int gDesktop::scale()
{
	gFont *ft;
	
	if (!_desktop_scale)
	{
		ft = font();
		_desktop_scale = (ft->ascent() + ft->descent() + 1) / 2;
	}
	
	return _desktop_scale;
}

gPicture* gDesktop::grab()
{
	return Grab_gdkWindow(gdk_get_default_root_window());	
	
}


