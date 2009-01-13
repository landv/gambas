/***************************************************************************

  gdrag.cpp

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

static GtkClipboard *_clipboard = NULL;


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
		
	return gt_free_later(gdk_atom_name(targets[n]));
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
		
	return gt_free_later(gtk_clipboard_wait_for_text(_clipboard));
}

/***********************************************************************

	Drag & Drop

************************************************************************/

bool gDrag::_active = false;
gPicture *gDrag::_icon = NULL;
int gDrag::_icon_x = 0;
int gDrag::_icon_y = 0;
gControl *gDrag::_source = NULL;
gControl *gDrag::_destination = NULL;
gControl *gDrag::_dest = NULL;
int gDrag::_action = 0;
int gDrag::_type = 0;
gPicture *gDrag::_picture = NULL;
char *gDrag::_text = NULL;
char *gDrag::_format = NULL;
int gDrag::_enabled = 0;
int gDrag::_x = -1;
int gDrag::_y = -1;
GdkDragContext *gDrag::_context = NULL;
guint32 gDrag::_time = 0;
volatile bool gDrag::_got_data = false;
bool gDrag::_local = false;

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
	_destination = NULL;
	_dest = NULL;
	_type = Nothing;
	_x = _y = -1;
	_time = 0;
	_got_data = false;
	_local = false;
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
	_local = true;
		
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
	_local = true;
	setDropInfo(Image, NULL);
	
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


void gDrag::setDropData(int action, int x, int y, gControl *source, gControl *dest)
{
	//g_debug("gDrag::setDropData: action = %d x = %d y = %d source = %p\n", action, x, y, source);
	
	_x = x;
	_y = y;
	_action = action;
	_source = source;
	_destination = dest;
	_active = true;
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

GdkDragContext *gDrag::enable(GdkDragContext *context, gControl *control, guint32 time)
{
	GdkDragContext *old = _context;
	_enabled++;
	_context = context;
	_time = time;
	_dest = control;
	return old;
}

GdkDragContext *gDrag::disable(GdkDragContext *context)
{
	GdkDragContext *old = _context;
	_context = context;
	_enabled--;
	return old;
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


char *gDrag::getFormat(int n)
{
	GList *tg;
	gchar *format;
	
	//if (gDrag::getType()) // local DnD
	//	return;
	
	//g_debug("set_from_context: non local\n");
	
	if (_format)
		return _format;
		
	if (!_context)
		return NULL;
	
	tg = g_list_first(_context->targets);
	
	while (tg)
	{
		format = gdk_atom_name((GdkAtom)tg->data);
		if (n <= 0)
		{
			gt_free_later(format);
			return format;
		}
		g_free(format);
		tg = g_list_next(tg);
		n--;
	}
	
	return NULL;
}

int gDrag::getType()
{
	int i;
	char *format;
	
	if (_type)
		return _type;
	
	for (i = 0;; i++)
	{
		format = getFormat(i);
		if (!format)
			return Nothing;
		if (strlen(format) >= 5 && !strncasecmp(format, "text/", 5)) 
			return Text;
		if (strlen(format) >= 6 &&! strncasecmp(format, "image/", 6))
			return Image;
	}
}

static void cb_drag_data_received(GtkWidget *widget, GdkDragContext *context, gint x, gint y, GtkSelectionData *sel, guint info, guint time, gControl *data)
{
	if (gDrag::getType() == gDrag::Text)
	{
		if (sel->length != -1)
			gDrag::setDropText((char*)sel->data, sel->length);
		else
			gDrag::setDropText(NULL);
	}
	
	if (gDrag::getType() == gDrag::Image)
	{
		//fprintf(stderr, "Image\n");
		if (sel->length != -1)
			gDrag::setDropImage((char*)sel->data, sel->length);
		else
			gDrag::setDropImage(NULL);
	}
	
	gDrag::_got_data = true;
}


bool gDrag::getData(const char *prefix)
{
	GList *tg;
	gchar *format = NULL;
	gulong id;
	
	//fprintf(stderr, "getData\n");
	
	if (_local) // local DnD
		return false;
	
	tg = g_list_first(_context->targets);
	
	while (tg)
	{
		g_free(format);
		format = gdk_atom_name((GdkAtom)tg->data);
		//fprintf(stderr, "getData: format = '%s'\n", format);
		
		if (strlen(format) >= strlen(prefix) && !strncasecmp(format, prefix, strlen(prefix)))
		{ 
			g_free(format);
			id = g_signal_connect(_dest->border, "drag-data-received", G_CALLBACK(cb_drag_data_received), (gpointer)_dest);
			
			_got_data = false;
			
			gtk_drag_get_data (_dest->border, _context, (GdkAtom)tg->data, _time);
			
			while (!_got_data)
				do_iteration(true);
	
			g_signal_handler_disconnect(_dest->border, id);
	
			return false;
		}
			
		tg = g_list_next(tg);
	}
	
	g_free(format);
	return true;
}

char *gDrag::getText()
{
	if (_text)
		return _text;
		
	if (getData("text/"))
		return NULL;
	
	return _text;
}

gPicture *gDrag::getImage()
{
	if (_picture)
		return _picture;
		
	if (getData("image/"))
		return NULL;
	
	return _picture;
}

