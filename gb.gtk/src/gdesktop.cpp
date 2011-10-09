/***************************************************************************

  gdesktop.cpp

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
#include "widgets_private.h"
#include "gb.form.font.h"

#ifndef GAMBAS_DIRECTFB
#ifdef GDK_WINDOWING_X11
#include <gdk/gdkx.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include "x11.h"
#endif
#endif

#include "gapplication.h"
#include "gmainwindow.h"
#include "gdesktop.h"

/***********************************************************************

Desktop

************************************************************************/

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

gColor gDesktop::tooltipForeground()
{
	GtkStyle *st = gt_get_widget_style("gtk-tooltip");

	if (!st) return fgColor();
	return get_gdk_color(&st->fg[GTK_STATE_NORMAL]);
}

gColor gDesktop::tooltipBackground()
{
	GtkStyle *st = gt_get_widget_style("gtk-tooltip");

	if (!st) return bgColor();
	return get_gdk_color(&st->bg[GTK_STATE_NORMAL]);
}

gColor gDesktop::lightbgColor()
{
	uint col = IMAGE.MergeColor(gDesktop::selbgColor(), gDesktop::selfgColor(), 0.2);
	return col;
}

gColor gDesktop::lightfgColor()
{
	uint col = IMAGE.MergeColor(gDesktop::bgColor(), gDesktop::fgColor(), 0.2);
	return col;
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
	/*int d_pix=gdk_screen_get_height(gdk_screen_get_default());
	int d_mm=gdk_screen_get_height_mm(gdk_screen_get_default());
	
	return (int)(d_pix*25.4)/d_mm;*/
	return gdk_screen_get_resolution(gdk_screen_get_default());
}

int gDesktop::scale()
{
	if (!_desktop_scale)
	{
		gFont *ft = font();
		_desktop_scale = GET_DESKTOP_SCALE(ft->size(), resolution());
	}
	
/*	PangoLanguage *lng=NULL;
	PangoContext* ct=gdk_pango_context_get();
	GtkStyle *sty=gtk_widget_get_default_style();
	PangoFontDescription *ft=sty->font_desc;
	PangoFontMetrics* fm;
	int val;
	
	if (!_desktop_scale)
	{
		if (getenv("LANG")) 
			lng = pango_language_from_string(getenv("LANG"));
			
		fm = pango_context_get_metrics (ct,ft,lng);
		
		val = 1 + (pango_font_metrics_get_ascent(fm) + pango_font_metrics_get_descent(fm)) / PANGO_SCALE;
		val = GET_DESKTOP_SCALE(val);
		pango_font_metrics_unref(fm);
		g_object_unref(G_OBJECT(ct));
		
		if (!val) val = 1;
		_desktop_scale = val;
	}*/
	
	return _desktop_scale;
}

gPicture* gDesktop::screenshot(int x, int y, int w, int h)
{
	return gt_grab_window(gdk_get_default_root_window(), x, y, w, h);	
}

int gDesktop::count()
{
	return gdk_screen_get_n_monitors(gdk_screen_get_default());
}

void gDesktop::geometry(int screen, GdkRectangle *rect)
{
	gdk_screen_get_monitor_geometry(gdk_screen_get_default(), screen, rect);
}

void gDesktop::availableGeometry(int screen, GdkRectangle *rect)
{
	if (X11_get_available_geometry(screen, &rect->x, &rect->y, &rect->width, &rect->height))
		geometry(screen, rect);
}

