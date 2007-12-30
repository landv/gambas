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
#include "gclipboard.h"
#include "gdrag.h"
#include "gdesktop.h"

// static bool gDrag_Enabled=false;
// static int gDrag_X=0;
// static int gDrag_Y=0;
// static int gDrag_Action=0;
// static int gDrag_Type=0;
// static char *gDrag_Format=NULL;
// static char *gDrag_Text=NULL;
// static GdkPixbuf *gDrag_Pic=NULL;
// static GdkPixbuf *gDrag_icon=NULL;

static GtkClipboard *_clipboard = NULL;

static int _desktop_scale = 0;

/***********************************************************************

	Clipboard

************************************************************************/

void gClipboard::init()
{
	if (!_clipboard) 
		_clipboard = gtk_clipboard_get_for_display(gdk_display_get_default(), GDK_SELECTION_CLIPBOARD);
}

void gClipboard::clear()
{
	gtk_clipboard_clear(_clipboard);
}

char *gClipboard::getFormat(int n)
{
	gint n_tg;
	GdkAtom *targets;

	if (!gtk_clipboard_wait_for_targets(_clipboard, &targets, &n_tg))
		return NULL;
		
	if (n < 0 || n >= n_tg)
		return NULL;
		
	return gdk_atom_name(targets[n]);
}

static void cb_clear_text(GtkClipboard *clipboard, gpointer text)
{
	if (text) g_free(text);
}

static void cb_get_text(GtkClipboard *clipboard, GtkSelectionData *selection, guint info, gpointer text)
{
	gtk_selection_data_set_text(selection, (const char *)text, -1);
}

int gClipboard::getType()
{
	if (gtk_clipboard_wait_is_text_available(_clipboard)) return Text;
	if (gtk_clipboard_wait_is_image_available(_clipboard)) return Image;
	
	return Nothing;
}

void gClipboard::setImage(gPicture *image)
{
	gtk_clipboard_set_image(_clipboard, image->getPixbuf());
}

gPicture * gClipboard::getImage()
{
  return new gPicture(gtk_clipboard_wait_for_image(_clipboard));
}

static void 
gt_clipboard_set_text (GtkClipboard *clipboard, char *format, const gchar *text, gint len)
{
  GtkTargetList *list;
  GList *l;
  GtkTargetEntry *targets;
  gint n_targets, i;

  list = gtk_target_list_new (NULL, 0);
  if (format)
  	gtk_target_list_add (list, gdk_atom_intern(format, false), 0, 0);
  gtk_target_list_add_text_targets (list, 0);

  n_targets = g_list_length(list->list);
  targets = g_new0 (GtkTargetEntry, n_targets);
  for (l = list->list, i = 0; l; l = l->next, i++)
    {
      GtkTargetPair *pair = (GtkTargetPair *)l->data;
      targets[i].target = gdk_atom_name (pair->target);
    }
  
  if (len < 0)
    len = strlen (text);
  
  gtk_clipboard_set_with_data (clipboard, 
			       targets, n_targets,
			       cb_get_text, cb_clear_text,
			       g_strndup (text, len));
  gtk_clipboard_set_can_store (clipboard, NULL, 0);

  for (i = 0; i < n_targets; i++)
    g_free (targets[i].target);
  g_free (targets);
  gtk_target_list_unref (list);
}

void gClipboard::setText(char *text, char *format)
{
	if (!text)
		return;
		
	gt_clipboard_set_text(_clipboard, format, text, -1);
}

char *gClipboard::getText()
{
	if (!gtk_clipboard_wait_is_text_available(_clipboard))
		return NULL;
		
	return gtk_clipboard_wait_for_text(_clipboard);
}

/***********************************************************************

	Drag & Drop

************************************************************************/

bool gDrag::_active = false;
gPicture *gDrag::_icon = NULL;
int gDrag::_icon_x = 0;
int gDrag::_icon_y = 0;
gControl *gDrag::_source = NULL;
int gDrag::_action = 0;
int gDrag::_type = 0;
gPicture *gDrag::_picture = NULL;
char *gDrag::_text = NULL;
char *gDrag::_format = NULL;
int gDrag::_enabled = 0;
int gDrag::_x = -1;
int gDrag::_y = -1;

void gDrag::setIcon(gPicture *vl)
{  
	//g_debug("gDrag::setIcon: %p", vl);
	gPicture::assign(&_icon, vl);
}

void gDrag::cancel()
{
	//g_debug("gDrag::cancel");
	hide();
	setIcon(NULL);
	setDropText(NULL);
	setDropImage(NULL);
	g_free(_format);
	_format = NULL;
	_source = NULL;
	_type = Nothing;
	_x = _y = -1;

	_active = false;
}

void gDrag::exit()
{
	cancel();
}

void gDrag::dragText(gControl *source, char *text, char *format)
{
	GtkTargetList *list;
	GdkDragContext *ct;
	
	//cancel();
	
	setDropText(text);
	
	list = gtk_target_list_new (NULL, 0);
	if (format)
		gtk_target_list_add(list, gdk_atom_intern(format, false), 0, 0);
	gtk_target_list_add_text_targets(list, 0);
	
	//gtk_target_list_add (list,gdk_atom_intern("UTF8_STRING",false),0,0);
	//gtk_target_list_add (list,gdk_atom_intern("COMPOUND_TEXT",false),0,0);
	//gtk_target_list_add (list,gdk_atom_intern("TEXT",false),0,0);
	//gtk_target_list_add (list,GDK_TARGET_STRING,0,0);
	//gtk_target_list_add (list,gdk_atom_intern("text/plain;charset=utf-8",false),0,0);
	//gtk_target_list_add (list,gdk_atom_intern("text/plain",false),0,0);
	
	_source = source;
	setDropInfo(Text, format);
	_active = true;
		
	ct = gtk_drag_begin(source->border, list, GDK_ACTION_COPY, 1, NULL);
	
	if (_icon) 
		gtk_drag_set_icon_pixbuf(ct, _icon->getPixbuf(), _icon_x, _icon_y);
	
	gtk_target_list_unref(list);	

	//g_debug("dragText: end\n");
}

void gDrag::dragImage(gControl *source, gPicture *image)
{
	GtkTargetList *list;
	//GdkPixbuf *buf;
	GdkDragContext *ct;
	
	//cancel();
	
	//g_debug("dragImage: source = %p image = %p\n", source, image);
	
	setDropImage(image);
	
	list = gtk_target_list_new (NULL,0);
	
	gtk_target_list_add(list, gdk_atom_intern("image/png", false), 0, 0);
	gtk_target_list_add(list, gdk_atom_intern("image/jpg", false), 0, 0);
	gtk_target_list_add(list, gdk_atom_intern("image/jpeg", false), 0, 0);
	gtk_target_list_add(list, gdk_atom_intern("image/gif", false), 0, 0);
	
	_source = source;
	setDropInfo(Image, NULL);
	_active = true;
	
	//g_debug("dragImage: gtk_drag_begin\n");
	
	ct = gtk_drag_begin(source->border, list, GDK_ACTION_COPY, 1, NULL);
	
	if (_icon)
		gtk_drag_set_icon_pixbuf(ct, _icon->getPixbuf(), _icon_x, _icon_y);
	
	gtk_target_list_unref(list);

	//g_debug("dragImage: end\n");
}

void gDrag::setDropInfo(int type, char *format)
{
	_type = type;
	g_free(_format);
	_format = g_strdup(format);
}


void gDrag::setDropData(int action, int x, int y, gControl *source)
{
	//g_debug("gDrag::setDropData: action = %d x = %d y = %d source = %p\n", action, x, y, source);
	
	_x = x;
	_y = y;
	_action = action;
	_source = source;
}

void gDrag::setDropText(char *text, int len)
{
	//g_debug("gDrag::setDropText: text = %s\n", text);
	
	g_free(_text);
	if (text)
		_text = (len < 0) ? g_strdup(text) : g_strndup(text, len);
	else
		_text = NULL;
}

void gDrag::setDropImage(gPicture* image)
{
	//g_debug("gDrag::setDropImage: image = %p\n", image);
	gPicture::assign(&_picture, image);
}

void gDrag::setDropImage(char *buf, int len)
{
	GdkPixbufLoader *ld;
	GdkPixbuf *pixbuf = NULL;
	
	//g_debug("gDrag::setDropImage: buf = %p len = %d\n", buf, len);
	
	if (buf && len > 0)
	{
		ld = gdk_pixbuf_loader_new ();
		if (gdk_pixbuf_loader_write(ld, (const guchar*)buf, len, NULL))
		{
			gdk_pixbuf_loader_close (ld, NULL);
			pixbuf = gdk_pixbuf_loader_get_pixbuf(ld);
		}
		g_object_unref(G_OBJECT(ld));
	}
	
	if (pixbuf)
		setDropImage(new gPicture(pixbuf));
	else
		setDropImage(NULL);
}

bool gDrag::checkThreshold(gControl *control, int x, int y, int sx, int sy)
{
  return gtk_drag_check_threshold(control->border, sx, sy, x, y);
} 

//static GtkWidget *_frame_container = 0;
static GdkWindow *_frame[4] = { 0 };
static bool _frame_visible = false;
static gControl *_frame_control = 0;

static void hide_frame(gControl *control)
{
	int i;
	
	if (!_frame_visible)
		return;
		
	if (control && control != _frame_control)
		return;
		
	for (i = 0; i < 4; i++)
		gdk_window_destroy(_frame[i]);
		
	_frame_visible = false;
}

static void move_frame_border(GdkWindow *window, int x, int y, int w, int h)
{
	gdk_window_move_resize(window, x, y, w, h);
}

static void show_frame(gControl *control, int x, int y, int w, int h)
{
	int i;
	GdkWindowAttr attr = { 0 };
	GdkWindow *window;
	GdkColor color;
	GdkWindow *parent;
	
	if (w < 0) w = control->width();
	if (h < 0) h = control->height();
	
	if (w < 2 || h < 2)
		return;
	
	//g_debug("show %p %d %d %d %d", control->border->window, x, y, w, h);
	
	if (control != _frame_control)
		hide_frame(NULL);
		
	// Don't know why I should do that...
	if (GTK_IS_SCROLLED_WINDOW(control->border))
	{
		parent = control->widget->window;
		w -= control->getFrameWidth();
		h -= control->getFrameWidth();
	}
	else
		parent = control->border->window;
	
	if (!_frame_visible)
	{
		fill_gdk_color(&color, 0);
		
		attr.wclass = GDK_INPUT_OUTPUT;
		attr.window_type = GDK_WINDOW_CHILD;
		
		for (i = 0; i < 4; i++)
		{
			window = gdk_window_new(parent, &attr, 0);
			gdk_window_set_background(window, &color);
			_frame[i] = window;
		}
	}
	
	//x -= 2;
	//y -= 2;
	//w += 4;
	//h += 4;
	move_frame_border(_frame[0], x, y, w, 2);
	move_frame_border(_frame[1], x, y, 2, h);
	move_frame_border(_frame[2], x + w - 2, y, 2, h);
	move_frame_border(_frame[3], x, y + h - 2, w, 2);
	
	for (i = 0; i < 4; i++)
		gdk_window_show(_frame[i]);
	
	_frame_control = control;
	_frame_visible = true;
}

// static gboolean
// cb_drag_highlight_expose (GtkWidget *widget,
// 			   GdkEventExpose *event,
// 			   gpointer        data)
// {
//   gint x, y, width, height;
//   
//   if (GTK_WIDGET_DRAWABLE (widget))
//   {
// 		cairo_t *cr;
// 		
// 		gDrag::getHighlight(&x, &y, &width, &height);
// 		
// 		if (GTK_WIDGET_NO_WINDOW (widget))
// 		{
// 			x += widget->allocation.x;
// 			y += widget->allocation.y;
// 		}
// 
// 		gtk_paint_shadow (widget->style, widget->window,
// 					GTK_STATE_NORMAL, GTK_SHADOW_OUT,
// 					NULL, widget, "dnd",
// 		x, y, width, height);
// 		
// 		cr = gdk_cairo_create (widget->window);
// 		cairo_set_source_rgb (cr, 0.0, 0.0, 0.0); /* black */
// 		cairo_set_line_width (cr, 1.0);
// 		cairo_rectangle (cr,
// 					x + 0.5, y + 0.5,
// 					width - 1, height - 1);
// 		cairo_stroke (cr);
// 		cairo_destroy (cr);
// 	}
// 
//   return FALSE;
// }

void gDrag::show(gControl *control, int x, int y, int w, int h)
{
	show_frame(control, x, y, w, h);
}

void gDrag::hide(gControl *control)
{
	hide_frame(control);
}

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
gFont *desktop_font=NULL;

bool gDesktop::rightToLeft()
{
	return gtk_widget_get_default_direction() == GTK_TEXT_DIR_RTL;
}

void gDesktop::init()
{
	desktop_font = new gFont();
}

void gDesktop::exit()
{
  gFont::assign(&desktop_font);
}

gFont* gDesktop::font()
{
	if (!desktop_font) gDesktop::init();
	return desktop_font;
}

void gDesktop::setFont(gFont *ft)
{
  gFont::assign(&desktop_font, ft);
  _desktop_scale = 0;
}

gControl* gDesktop::activeControl()
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
	
}

gMainWindow* gDesktop::activeWindow()
{
	return gMainWindow::_active ? gMainWindow::_active->topLevel() : NULL;
}

gColor gDesktop::buttonfgColor()
{
	GtkStyle *st = gt_get_style("GtkButton", GTK_TYPE_BUTTON);
	return get_gdk_color(&st->fg[GTK_STATE_NORMAL]);
}

gColor gDesktop::buttonbgColor()
{
	GtkStyle *st = gt_get_style("GtkButton", GTK_TYPE_BUTTON);	
	return get_gdk_color(&st->bg[GTK_STATE_NORMAL]);
}

gColor gDesktop::fgColor()
{	
	GtkStyle *st = gt_get_style("GtkEntry", GTK_TYPE_ENTRY);
	return get_gdk_color(&st->fg[GTK_STATE_NORMAL]);
}

gColor gDesktop::bgColor()
{
	GtkStyle *st = gt_get_style("GtkLayout", GTK_TYPE_LAYOUT);
	return get_gdk_color(&st->bg[GTK_STATE_NORMAL]);
}

gColor gDesktop::textfgColor()
{
	GtkStyle *st = gt_get_style("GtkEntry", GTK_TYPE_ENTRY);
	return get_gdk_color(&st->text[GTK_STATE_NORMAL]);
}

gColor gDesktop::textbgColor()
{
	GtkStyle *st = gt_get_style("GtkEntry", GTK_TYPE_ENTRY);
	return get_gdk_color(&st->base[GTK_STATE_NORMAL]);
}

gColor gDesktop::selfgColor()
{
	GtkStyle *st = gt_get_style("GtkEntry", GTK_TYPE_ENTRY);
	return get_gdk_color(&st->text[GTK_STATE_SELECTED]);
}

gColor gDesktop::selbgColor()
{
	GtkStyle *st = gt_get_style("GtkEntry", GTK_TYPE_ENTRY);
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
	PangoLanguage *lng=NULL;
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
		
		val = (pango_font_metrics_get_ascent(fm) + pango_font_metrics_get_descent(fm)) / 2;
		val /= PANGO_SCALE;
		
		pango_font_metrics_unref(fm);
		g_object_unref(G_OBJECT(ct));
		
		if (!val) val = 1;
		_desktop_scale = val;
	}
	
	return _desktop_scale;
}

gPicture* gDesktop::grab()
{
	return Grab_gdkWindow(gdk_get_default_root_window());	
	
}


