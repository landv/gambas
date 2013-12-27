/***************************************************************************

  gmainwindow.cpp

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

#include <ctype.h>
#include <time.h>

#include "widgets.h"
#include "widgets_private.h"

#ifdef GDK_WINDOWING_X11
#include <X11/extensions/shape.h>
#endif

#include "x11.h"

#include "gapplication.h"
#include "gdesktop.h"
#include "gkey.h"
#include "gmenu.h"
#include "gmessage.h"
#include "gdialog.h"
#include "gmouse.h"
#include "gmainwindow.h"

static gboolean win_frame(GtkWidget *widget,GdkEventWindowState *event,gMainWindow *data)
{
	data->performArrange();
	data->emit(SIGNAL(data->onState));
	return false;
}

static gboolean cb_show(GtkWidget *widget, gMainWindow *data)
{
	data->emitOpen();
	
	if (data->opened)
	{
		if (data->isTopLevel())
		{
			if (data->isModal() && data->isResizable())
			{
				GdkGeometry geometry;
				
				geometry.min_width = data->bufW;
				geometry.min_height = data->bufH;
				gdk_window_set_geometry_hints(gtk_widget_get_window(data->border), &geometry, (GdkWindowHints)(GDK_HINT_MIN_SIZE | GDK_HINT_POS));
			}
		}
		
		//data->performArrange();
		data->emitResize();
		data->emit(SIGNAL(data->onShow));
		data->_not_spontaneous = false;
	}
	return false;
}

static gboolean cb_hide(GtkWidget *widget, gMainWindow *data)
{
	data->emit(SIGNAL(data->onHide));
	data->_not_spontaneous = false;
	return false;
	//if (data == gDesktop::activeWindow())
	//	gMainWindow::setActiveWindow(NULL);
}

static gboolean win_close(GtkWidget *widget,GdkEvent  *event,gMainWindow *data)
{
	if (!gMainWindow::_current || data == gMainWindow::_current)
		data->doClose();
	
	return true;
}

static gboolean cb_configure(GtkWidget *widget, GdkEventConfigure *event, gMainWindow *data)
{
	gint x, y;
	
	if (data->opened)
	{
		if (data->isTopLevel())
		{
			gtk_window_get_position(GTK_WINDOW(data->border), &x, &y);
		}
		else
		{
			x = event->x;
			y = event->y;
		}
		
		//fprintf(stderr, "cb_configure: %s: (%d %d %d %d) -> (%d %d %d %d) window = %p resized = %d\n", data->name(), data->bufX, data->bufY, data->bufW, data->bufH, x, y, event->width, event->height, event->window, data->_resized);
	
		if (x != data->bufX || y != data->bufY)
		{
			data->bufX = x;
			data->bufY = y;
			if (data->onMove) data->onMove(data);
		}
		
		if ((event->width != data->bufW) || (event->height != data->bufH) || (data->_resized) || !event->window)
		{
			data->_resized = false;
			data->bufW = event->width;
			data->bufH = event->height;
			data->emitResize();
		}
	}
	
	return false;
}

static gboolean cb_expose(GtkWidget *wid, GdkEventExpose *e, gMainWindow *data)
{
	if (data->_picture)
	{
		cairo_t *cr = gdk_cairo_create(wid->window);
		cairo_pattern_t *pattern;

		gdk_cairo_region(cr, e->region);
		cairo_clip(cr);

		pattern = cairo_pattern_create_for_surface(data->_picture->getSurface());
		cairo_pattern_set_extend(pattern, CAIRO_EXTEND_REPEAT);

		cairo_set_source(cr, pattern);
		cairo_rectangle(cr, 0, 0, data->width(), data->height());
		cairo_fill(cr);

		cairo_pattern_destroy(pattern);
		cairo_destroy(cr);
	}
	
	return false;
}



GList *gMainWindow::windows = NULL;
gMainWindow *gMainWindow::_active = NULL;
gMainWindow *gMainWindow::_current = NULL;

void gMainWindow::initialize()
{
	//fprintf(stderr, "new window: %p in %p\n", this, parent());

	opened = false;
	sticky = false;
	persistent = false;
	stack = 0;
	_type = 0;
	_mask = false;
	_masked = false;
	_resized = false;
	accel = NULL;
	_default = NULL;
	_cancel = NULL;
	menuBar = NULL;
	layout = NULL;
	top_only = false;
	_icon = NULL;
	_picture = NULL;
	focus = 0;
	_closing = false;
	_title = NULL;
	_not_spontaneous = false;
	_skip_taskbar = false;
	_current = NULL;
	_style = NULL;
	_xembed = false;
	_activate = false;
	_hidden = false;
	_hideMenuBar = false;
	_showMenuBar = true;
	_popup = false;
	_maximized = _minimized = _fullscreen = false;
	_resize_last_w = _resize_last_h = -1;

	onOpen = NULL;
	onShow = NULL;
	onHide = NULL;
	onMove = NULL;
	onResize = NULL;
	onActivate = NULL;
	onDeactivate = NULL;
	onState = NULL;
	
	accel = gtk_accel_group_new();
}

void gMainWindow::initWindow()
{
	//resize(200,150);
	
	if (!isTopLevel())
	{
		g_signal_connect(G_OBJECT(border), "configure-event", G_CALLBACK(cb_configure), (gpointer)this);
		g_signal_connect_after(G_OBJECT(border), "map", G_CALLBACK(cb_show), (gpointer)this);
		g_signal_connect(G_OBJECT(border),"unmap",G_CALLBACK(cb_hide),(gpointer)this);
		//g_signal_connect_after(G_OBJECT(border), "size-allocate", G_CALLBACK(cb_configure), (gpointer)this);
		g_signal_connect(G_OBJECT(widget), "expose-event", G_CALLBACK(cb_expose), (gpointer)this);
		gtk_widget_add_events(border, GDK_STRUCTURE_MASK);	
	}
	else
	{
		//g_signal_connect(G_OBJECT(border),"size-request",G_CALLBACK(cb_realize),(gpointer)this);
		g_signal_connect(G_OBJECT(border), "show",G_CALLBACK(cb_show),(gpointer)this);
		g_signal_connect(G_OBJECT(border), "hide",G_CALLBACK(cb_hide),(gpointer)this);
		g_signal_connect(G_OBJECT(border), "configure-event",G_CALLBACK(cb_configure),(gpointer)this);
		g_signal_connect(G_OBJECT(border), "delete-event",G_CALLBACK(win_close),(gpointer)this);
		g_signal_connect(G_OBJECT(border), "window-state-event",G_CALLBACK(win_frame),(gpointer)this);
		
		gtk_widget_add_events(widget,GDK_BUTTON_MOTION_MASK);
		g_signal_connect(G_OBJECT(widget), "expose-event", G_CALLBACK(cb_expose), (gpointer)this);
	}
	
	gtk_window_add_accel_group(GTK_WINDOW(topLevel()->border), accel);
	
	have_cursor = true; //parent() == 0 && !_xembed;
}

gMainWindow::gMainWindow(int plug) : gContainer(NULL)
{
  initialize();
	g_typ = Type_gMainWindow;
	
	windows = g_list_append(windows, (gpointer)this);
	
	_xembed = plug != 0;
	
	if (_xembed)
		border = gtk_plug_new(plug);
	else
		border = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	
	widget = gtk_fixed_new(); //gtk_layout_new(0,0);
	
	realize(false);
	initWindow();
	
	gtk_widget_realize(border);
	gtk_widget_show(widget);
	
	gtk_widget_set_size_request(border, 1, 1);
}

gMainWindow::gMainWindow(gContainer *par) : gContainer(par)
{
	initialize();
	g_typ = Type_gMainWindow;
	
	border = gtk_alignment_new(0,0,1,1); //gtk_fixed_new(); //gtk_event_box_new();
	widget = gtk_fixed_new();
	
	realize(false);
	
	initWindow();
}

gMainWindow::~gMainWindow()
{
	//fprintf(stderr, "delete window %p %s\n", this, name());
	
	if (opened)
	{
		emit(SIGNAL(onClose));
		opened = false;
		if (GTK_IS_WINDOW(border) && isModal())
			gApplication::exitLoop(this);
	}
	
	gPicture::assign(&_picture);
	gPicture::assign(&_icon);
	if (_title) g_free(_title);
	g_object_unref(accel);
	if (_style) g_object_unref(_style);
	
	if (_active == this)
		_active = NULL;
	
	if (gApplication::mainWindow() == this)
		gApplication::setMainWindow(NULL);
	
	windows = g_list_remove(windows, (gpointer)this);
}

bool gMainWindow::getSticky()
{
	return sticky;
}

int gMainWindow::getStacking()
{
	return stack;
}

void gMainWindow::setSticky(bool vl)
{
	sticky=vl;
	if (!isTopLevel()) return;

	if (vl) gtk_window_stick(GTK_WINDOW(border));
	else    gtk_window_unstick(GTK_WINDOW(border));
}

void gMainWindow::setStacking(int vl)
{
  stack=vl;
	if (!isTopLevel()) return;

	switch (vl)
	{
		case 0:
			gtk_window_set_keep_below(GTK_WINDOW(border),FALSE);
			gtk_window_set_keep_above(GTK_WINDOW(border),FALSE);
			break;
		case 1:
			gtk_window_set_keep_below(GTK_WINDOW(border),FALSE);
			gtk_window_set_keep_above(GTK_WINDOW(border),TRUE);
			break;
		case 2:
			gtk_window_set_keep_above(GTK_WINDOW(border),FALSE);
			gtk_window_set_keep_below(GTK_WINDOW(border),TRUE);
			break;
	}
}

void gMainWindow::setRealBackground(gColor color)
{
	if (!_picture)
	{
		gControl::setRealBackground(color);
		gMenu::updateColor(this);
	}
}

void gMainWindow::setRealForeground(gColor color)
{
	gControl::setRealForeground(color);
	gMenu::updateColor(this);
}

void gMainWindow::move(int x, int y)
{
	gint ox, oy;
	
	if (isTopLevel())
	{
		if (x == bufX && y == bufY) 
			return;
	
		#ifdef GDK_WINDOWING_X11
		gdk_window_get_origin(border->window, &ox, &oy);
		ox = x + ox - bufX;
		oy = y + oy - bufY;
		bufX = x;
		bufY = y;
		if (bufW > 0 && bufH > 0)
		{
			if (!X11_send_move_resize_event(GDK_WINDOW_XID(border->window), ox, oy, width(), height()))
				return;
		}
		#else
		bufX = x;
		bufY = y;
		#endif
		
		gtk_window_move(GTK_WINDOW(border), x, y);
	}
	else
	{
		gContainer::move(x,y);
	}
}


void gMainWindow::resize(int w, int h)
{
	if (w == bufW && h == bufH)
		return;
			
	_resized = true;
		
	if (isTopLevel())
	{
		//fprintf(stderr, "resize: %s: %d %d\n", name(), w, h);
	
		//gdk_window_enable_synchronized_configure (border->window);
		
		bufW = w < 0 ? 0 : w;
		bufH = h < 0 ? 0 : h;
		
		if (w < 1 || h < 1)
		{
			if (visible)
				gtk_widget_hide(border);			
		}
		else
		{
			if (isResizable())
				gtk_window_resize(GTK_WINDOW(border), w, h);
			else
				gtk_widget_set_size_request(border, w, h);
				
			if (visible)
				gtk_widget_show(border);
		}
	}
	else
	{
		//fprintf(stderr, "resize %p -> (%d %d) (%d %d)\n", this, bufW, bufH, w, h);
		gContainer::resize(w, h);
	}
}

void gMainWindow::moveResize(int x, int y, int w, int h)
{
	//if (isTopLevel())
	//	gdk_window_move_resize(border->window, x, y, w, h);
	//else
		gContainer::moveResize(x, y, w, h);
}

void gMainWindow::emitOpen()
{
	//fprintf(stderr, "emit Open: %p (%d %d) %d resizable = %d fullscreen = %d\n", this, width(), height(), opened, isResizable(), fullscreen());
	
	if (!opened)
	{
		opened = true;
		//_no_resize_event = true; // If the event loop is run during emitOpen(), some spurious configure events are received.
		
		// FIXME: the following does not work, it makes the window growing endlessly!
			
		/*if (isTopLevel()) 
		{
			GdkGeometry geometry = { 0 };
	
			if (isModal())
			{
				geometry.min_width = 2; //bufW;
				geometry.min_height = 2; //bufH;
			}
			else
			{
				geometry.min_width = 0;
				geometry.min_height = 0;
			}
			
			fprintf(stderr, "gtk_window_set_geometry_hints: %d %d\n", geometry.min_width, geometry.min_height);
			gtk_window_set_geometry_hints(GTK_WINDOW(border), border, &geometry, GDK_HINT_MIN_SIZE);
		}*/
		
		gtk_widget_realize(border);
			
		performArrange();
		emit(SIGNAL(onOpen));
		if (opened)
		{
			//fprintf(stderr, "emit Move & Resize: %p\n", this);			
			emit(SIGNAL(onMove));
			emitResize();
		}
	}
	
	//_no_resize_event = false;
}

void gMainWindow::afterShow()
{
	if (_activate)
	{
		gtk_window_present(GTK_WINDOW(border));
		_activate = false;
	}
}

void gMainWindow::setVisible(bool vl)
{
	if (vl == isVisible())
		return;

	if (vl)
	{
		bool arr = !isVisible();
	
		emitOpen();
		if (!opened)
			return;
		
		_not_spontaneous = !visible;
		visible = true;
		_hidden = false;
		
		if (isTopLevel())
		{
			if (!_title || !*_title)
				gtk_window_set_title(GTK_WINDOW(border), gApplication::defaultTitle());
			
			/*if (!_xembed)
			{
				fprintf(stderr, "gtk_window_group_add_window: %p -> %p\n", border, gApplication::currentGroup());
				gtk_window_group_add_window(gApplication::currentGroup(), GTK_WINDOW(border));
				fprintf(stderr, "-> %p\n", gtk_window_get_group(GTK_WINDOW(border)));
			}*/
			
			// Thanks for Ubuntu's GTK+ patching :-(
			#if GTK_CHECK_VERSION(3,0,0)
			gtk_window_set_has_resize_grip(GTK_WINDOW(border), false);
			#else
			if (g_object_class_find_property(G_OBJECT_GET_CLASS(border), "has-resize-grip"))
				g_object_set(G_OBJECT(border), "has-resize-grip", false, (char *)NULL);
			#endif
				
			gtk_window_move(GTK_WINDOW(border), bufX, bufY);
			if (isPopup())
			{
				gtk_widget_show_now(border);
				gtk_widget_grab_focus(border);
			}
			else
				gtk_window_present(GTK_WINDOW(border));

			if (isUtility())
			{
				gMainWindow *parent = _current;
				
				if (!parent && gApplication::mainWindow() && gApplication::mainWindow() != this)
					parent = gApplication::mainWindow();
				
				if (parent)
					gtk_window_set_transient_for(GTK_WINDOW(border), GTK_WINDOW(parent->border));
			}
		}
		else 
		{
			gtk_widget_show(border);
			parent()->performArrange();
		}
		
		drawMask();
		
		if (focus)
		{
			//fprintf(stderr, "focus = %s\n", focus->name());
			focus->setFocus();
			focus = 0;
		}
		
		if (skipTaskBar())
			_activate = true;

		if (arr)
			performArrange();
	}
	else
	{
		if (this == _active)
			focus = gApplication::activeControl();
			
		_not_spontaneous = visible;
		_hidden = true;
		gContainer::setVisible(false);
		
		if (_popup)
			gApplication::exitLoop(this);
	}
}


void gMainWindow::setMinimized(bool vl)
{
	if (!isTopLevel()) return;
	
	_minimized = vl;
	if (vl) gtk_window_iconify(GTK_WINDOW(border));
	else    gtk_window_deiconify(GTK_WINDOW(border));
}

void gMainWindow::setMaximized(bool vl)
{
	if (!isTopLevel())
		return;

	_maximized = vl;
	
	if (vl)
		gtk_window_maximize(GTK_WINDOW(border));
	else
		gtk_window_unmaximize(GTK_WINDOW(border));
}

void gMainWindow::setFullscreen(bool vl)
{
	if (!isTopLevel())
		return;
	
	_fullscreen = vl;
	
	if (vl)
	{
		gtk_window_fullscreen(GTK_WINDOW(border));
		if (isVisible())
			gtk_window_present(GTK_WINDOW(border));
	}
	else
		gtk_window_unfullscreen(GTK_WINDOW(border));
}

void gMainWindow::center()
{
	GdkRectangle rect;
	int x, y;
	
	if (!isTopLevel()) return;
	
	gDesktop::availableGeometry(screen(), &rect);
	
	x = rect.x + (rect.width - width()) / 2;
	y = rect.y + (rect.height - height()) / 2;
	
	move(x, y);
}

bool gMainWindow::isModal() const
{
	if (!isTopLevel()) return false;

	return gtk_window_get_modal(GTK_WINDOW(border));
}

void gMainWindow::showModal()
{
  gMainWindow *save;
	
	if (!isTopLevel()) return;
	if (isModal()) return;
	
	//show();

	gtk_window_set_modal(GTK_WINDOW(border), true);
  center();
	//show();
	gtk_grab_add(border);
	
	if (_active)
		gtk_window_set_transient_for(GTK_WINDOW(border), GTK_WINDOW(_active->topLevel()->border));
	
	save = _current;
	_current = this;

	gApplication::enterLoop(this, true);
	
	_current = save;
	
	gtk_grab_remove(border);
	gtk_window_set_modal(GTK_WINDOW(border), false);

	if (!persistent)
		destroyNow();
	else
		hide();
}

void gMainWindow::showPopup(int x, int y)
{
  gMainWindow *save;
	bool has_border;
	int oldx, oldy;
	//int type;
	
	if (!isTopLevel()) return;
	if (isModal()) return;
	
	//gtk_widget_unrealize(border);
	//((GtkWindow *)border)->type = GTK_WINDOW_POPUP;
	//gtk_widget_realize(border);

	oldx = left();
	oldy = top();
	
	has_border = gtk_window_get_decorated(GTK_WINDOW(border));
	//type = getType();
	
	//setType(_NET_WM_WINDOW_TYPE_COMBO);
	gtk_window_set_decorated(GTK_WINDOW(border), false);
	//gtk_window_set_type_hint(GTK_WINDOW(border), GDK_WINDOW_TYPE_HINT_POPUP_MENU);
	
  move(x, y);
	gtk_window_resize(GTK_WINDOW(border), bufW, bufH);
	
	//reparent(NULL, x, y, GTK_WINDOW_POPUP);

	_popup = true;
	save = _current;
	_current = this;
	
	gApplication::enterPopup(this);
	
	_current = save;
	_popup = false;
	
	if (!persistent)
	{
		destroyNow();
	}
	else
	{
		hide();
		
		//gdk_window_set_override_redirect(gtk_widget_get_window(GTK_WINDOW(border)), false);
		gtk_window_set_decorated(GTK_WINDOW(border), has_border);
		//setType(type);
		//gtk_window_set_type_hint(GTK_WINDOW(border), type);
		
		move(oldx, oldy);
	}
}

void gMainWindow::showPopup()
{
	int x, y;
	gMouse::getScreenPos(&x, &y);
	showPopup(x, y);
}

void gMainWindow::raise()
{
	if (!isTopLevel()) { gControl::raise(); return; }
	gtk_window_present(GTK_WINDOW(border));
}

const char* gMainWindow::text()
{
	return _title;
}

bool gMainWindow::skipTaskBar()
{
	if (!isTopLevel()) 
		return false;
	else
		return _skip_taskbar;
}


void gMainWindow::setText(const char *txt)
{
	if (_title) g_free(_title);
	_title = g_strdup(txt);
	
	if (isTopLevel())
		gtk_window_set_title(GTK_WINDOW(border), txt);
}

bool gMainWindow::hasBorder()
{
	if (isTopLevel())
		return gtk_window_get_decorated(GTK_WINDOW(border));
	else
		return false;
}

bool gMainWindow::isResizable()
{
	if (isTopLevel())
		return gtk_window_get_resizable(GTK_WINDOW(border));
	else
		return false;
}

void gMainWindow::setBorder(bool b)
{
  if (!isTopLevel()) 
  	return;
	
	gtk_window_set_decorated(GTK_WINDOW(border), b);
	/*#ifdef GDK_WINDOWING_X11
	XSetWindowAttributes attr;
	attr.override_redirect = !b;
	XChangeWindowAttributes(GDK_WINDOW_XDISPLAY(border->window), GDK_WINDOW_XID(border->window), CWOverrideRedirect, &attr);
	#endif*/
}

void gMainWindow::setResizable(bool b)
{
  if (!isTopLevel()) 
  	return;
	
	if (b == isResizable())
		return;

	gtk_window_set_resizable(GTK_WINDOW(border), b);

	if (b)
		gtk_widget_set_size_request(border, 1, 1);
	else
		gtk_widget_set_size_request(border, bufW, bufH);
}


void gMainWindow::setSkipTaskBar(bool b)
{
	_skip_taskbar = b;
	if (!isTopLevel()) return;
	gtk_window_set_skip_taskbar_hint (GTK_WINDOW(border), b);
}


/*gPicture* gMainWindow::icon()
{
	GdkPixbuf *buf;
	gPicture *pic;
	
	if (!isTopLevel()) return NULL;
	
	buf=gtk_window_get_icon(GTK_WINDOW(border));
	if (!buf) return NULL;
	
	pic=gPicture::fromPixbuf(buf);
	
	return pic;
}*/

void gMainWindow::setIcon(gPicture *pic)
{
  gPicture::assign(&_icon, pic);

	if (!isTopLevel()) return;
  gtk_window_set_icon(GTK_WINDOW(border), pic ? pic->getPixbuf() : NULL);
}

bool gMainWindow::topOnly()
{
	if (!isTopLevel()) return false;
	
	return top_only;
}

void gMainWindow::setTopOnly(bool vl)
{
	if (!isTopLevel()) return;

	gtk_window_set_keep_above (GTK_WINDOW(border),vl);
	top_only=vl;
}


void gMainWindow::setMask(bool vl)
{
	if (_mask == vl)
		return;
		
	_mask = vl;
	drawMask();
}

void gMainWindow::setPicture(gPicture *pic)
{
  gPicture::assign(&_picture, pic);
  drawMask();
}

void gMainWindow::remap()
{
	if (!isVisible())
		return;

	gtk_widget_unmap(border);
	gtk_widget_map(border);
	
	if (_skip_taskbar) { setSkipTaskBar(false);	setSkipTaskBar(true); }
	if (top_only) { setTopOnly(false); setTopOnly(true); }
	if (sticky) { setSticky(false); setSticky(true); }
	if (stack) { setStacking(0); setStacking(stack); }
	X11_set_window_type(handle(), _type);
}

void gMainWindow::drawMask()
{
	GdkBitmap *mask;
	bool do_remap = false;
	 
	/*if (win_style) 
	{ 
		g_object_unref(win_style); 
		win_style = NULL; 
	}*/
	
	/*if (_picture)
	{
		back = _picture->getPixmap();
		g_object_ref(back);
		win_style = gtk_style_copy(widget->style);
		win_style->bg_pixmap[GTK_STATE_NORMAL] = back;
		gtk_widget_set_style(widget, win_style);
		//gdk_window_invalidate_rect(border->window,NULL,true);
	}
	else
	{
		gtk_widget_set_style(widget, NULL);
	}*/
	
	if (!isVisible())
		return;
	
	mask = (_mask && _picture) ? _picture->getMask() : NULL;
	do_remap = !mask && _masked;
	
	#ifdef GDK_WINDOWING_X11
  XShapeCombineMask(GDK_WINDOW_XDISPLAY(border->window), GDK_WINDOW_XID(border->window), ShapeBounding, 0, 0,
  	mask ? GDK_PIXMAP_XID(mask) : None, ShapeSet);
  #else
	//gdk_window_shape_combine_mask(border->window, mask, 0, 0);
  #endif

	//back = _picture ? _picture->getPixmap() : NULL;
	//gtk_widget_set_double_buffered(border, back == NULL);
	//gtk_widget_set_double_buffered(widget, back == NULL);
	
	if (_picture)
	{
		gtk_widget_realize(border);
		gtk_widget_realize(widget);
		//gdk_window_set_back_pixmap(border->window, back, FALSE);
		//gdk_window_clear(widget->window);
		//gtk_widget_set_app_paintable(border, true);
		//gtk_widget_set_app_paintable(widget, true);
		for (int i = 0; i < controlCount(); i++)
			getControl(i)->refresh();
	}
	else
	{
		setRealBackground(background());
	}
	
	_masked = mask != NULL;
	
	if (do_remap)
		remap();
	else
	{
		if (!_skip_taskbar)
		{
			setSkipTaskBar(true);
			setSkipTaskBar(false);
		}
	}
}

int gMainWindow::menuCount()
{
	if (!menuBar) return 0;
	return gMenu::winChildCount(this);
}

void gMainWindow::setPersistent(bool vl)
{
  persistent = vl;
}

bool gMainWindow::doClose()
{
	if (_closing)
		return false;
	
	if (opened)
	{
		if (isModal() && !gApplication::hasLoop(this))
			return true;
		
		_closing = true;
		if (onClose) 
		{
			if (!onClose(this))
				opened = false;
		}
		else
			opened = false;
		_closing = false;
		
		if (!opened && isModal())
			gApplication::exitLoop(this);
  }
  
  if (!opened) // && !modal())
  {
		if (_active == this)
			setActiveWindow(NULL);
		
  	if (!isModal())
  	{
			if (persistent)
				hide();
			else
				destroy();
		}
		return false;
	}
	else
		return opened;
}


bool gMainWindow::close()
{
	return doClose();
}

static void hide_hidden_children(gContainer *cont)
{
	int i;
	gControl *child;
	
	for (i = 0;; i++)
	{
		child = cont->child(i);
		if (!child)
			break;
		if (!child->isVisible())
			gtk_widget_hide(child->border);
		else if (child->isContainer())
			hide_hidden_children((gContainer *)child);
	}
}

void gMainWindow::reparent(gContainer *newpr, int x, int y)
{
	GtkWidget *new_border;
	int w, h;
	gColor fg, bg;
	
	if (_xembed)
		return;
	
	bg = background();
	fg = foreground();
	
	if (isTopLevel() && newpr)
	{
		gtk_window_remove_accel_group(GTK_WINDOW(topLevel()->border), accel);
		
		new_border = gtk_event_box_new();
		gtk_widget_reparent(widget, new_border);
		embedMenuBar(new_border);
		_no_delete = true;
		gtk_widget_destroy(border);
		_no_delete = false;
		
		border = new_border;
		registerControl();
		
		setParent(newpr);
		connectParent();
		borderSignals();
		initWindow();	
		
		setBackground(bg);
		setForeground(fg);
		setFont(font());
		
		checkMenuBar();
		
		bufX = bufY = 0;
		move(x, y);
		gtk_widget_set_size_request(border, width(), height());
		
		// Hidden children are incorrectly shown. Fix that!
		hideHiddenChildren();
	}
	else if ((!isTopLevel() && !newpr)
	         || (isTopLevel() && isPopup()))
	         //|| (isTopLevel() && (isPopup() ^ (type == GTK_WINDOW_POPUP))))
	{
		gtk_window_remove_accel_group(GTK_WINDOW(topLevel()->border), accel);
		// TODO: test that
		new_border = gtk_window_new(GTK_WINDOW_TOPLEVEL);
		gtk_widget_reparent(widget, new_border);
		embedMenuBar(new_border);
		_no_delete = true;
		gtk_widget_destroy(border);
		_no_delete = false;

		border = new_border;
		registerControl();
		
		if (parent())
		{
			parent()->remove(this);
			parent()->arrange();
			setParent(NULL);
		}
		initWindow();	
		borderSignals();
		setBackground(bg);
		setForeground(fg);
		setFont(font());
		
		move(x, y);
		w = width();
		h = height();
		bufW = bufH = -1;
		resize(w, h);
		
		hideHiddenChildren();
		_popup = false; //type == GTK_WINDOW_POPUP;
	}
	else
		gContainer::reparent(newpr, x, y);	
}


int gMainWindow::controlCount()
{
	GList *list = gControl::controlList();
	gControl *ctrl;
	int n = 0;
	
	while (list)
	{
		ctrl = (gControl *)list->data;
		if (ctrl->window() == this && !ctrl->isDestroyed())
			n++;
		list = g_list_next(list);
	}
	
	return n;
}

gControl *gMainWindow::getControl(char *name)
{
	GList *list = gControl::controlList();
	gControl *ctrl;
	
	while (list)
	{
		ctrl = (gControl *)list->data;
		if (ctrl->window() == this && !strcasecmp(ctrl->name(), name) && !ctrl->isDestroyed())
			return ctrl;
		list = g_list_next(list);
	}
	
	return NULL;
}

gControl *gMainWindow::getControl(int index)
{
	GList *list = gControl::controlList();
	gControl *ctrl;
	int i = 0;
	
	while (list)
	{
		ctrl = (gControl *)list->data;
		if (ctrl->window() == this && !ctrl->isDestroyed())
		{
			if (i == index)
				return ctrl;
			i++;
		}
		list = g_list_next(list);
	}
	
	return NULL;
}


int gMainWindow::clientX()
{
	return 0;
}

int gMainWindow::containerX()
{
	return 0;
}

int gMainWindow::clientY()
{
	if (isMenuBarVisible())
		return menuBarHeight();
	else
		return 0;
}

int gMainWindow::containerY()
{
	return 0;
}


int gMainWindow::clientWidth()
{
	return width();
}


int gMainWindow::menuBarHeight()
{
	GtkRequisition req = { 0, 0 };
	int h = 0;

	if (menuBar)
	{
		//gtk_widget_show(GTK_WIDGET(menuBar));
		//fprintf(stderr, "menuBarHeight: gtk_widget_get_visible: %d\n", gtk_widget_get_visible(GTK_WIDGET(menuBar)));
		gtk_widget_size_request(GTK_WIDGET(menuBar), &req);
		h = req.height;
		//fprintf(stderr, "menuBarHeight: %d\n", h);
	}
	
	return h;
}

int gMainWindow::clientHeight()
{
	if (isMenuBarVisible())
		return height() - menuBarHeight();
	else
		return height();
}

void gMainWindow::setActiveWindow(gControl *control)
{
	gMainWindow *window = control ? control->window() : NULL;
	gMainWindow *old = _active;
	
	if (window == _active)
		return;
		
	_active = window;
	
	//fprintf(stderr, "setActiveWindow: %p %s\n", _active, _active ? _active->name() : "");
	
	if (old)
		old->emit(SIGNAL(old->onDeactivate));
		
	if (window)
		window->emit(SIGNAL(window->onActivate));
}

#ifdef GDK_WINDOWING_X11
bool gMainWindow::isUtility()
{
	return gtk_window_get_type_hint(GTK_WINDOW(border)) == GDK_WINDOW_TYPE_HINT_UTILITY;
}

void gMainWindow::setUtility(bool v)
{
	if (!isTopLevel())
		return;
	
	gtk_window_set_type_hint(GTK_WINDOW(border), v ? GDK_WINDOW_TYPE_HINT_UTILITY : GDK_WINDOW_TYPE_HINT_NORMAL);
}
#else
bool gMainWindow::isUtility()
{
	return false;
}

void gMainWindow::setUtility(bool v)
{
}
#endif

void gMainWindow::configure()
{
	int h;
	
	if (bufW < 1 || bufH < 1)
		return;
	
	h = menuBarHeight();
	
	//fprintf(stderr, "configure: %s: %d %d - %d %d\n", name(), isMenuBarVisible(), h, width(), height());
	
	if (isMenuBarVisible())
	{
		gtk_fixed_move(layout, GTK_WIDGET(menuBar), 0, 0);
		if (h > 1)
			gtk_widget_set_size_request(GTK_WIDGET(menuBar), width(), h);
		gtk_fixed_move(layout, widget, 0, h);
		gtk_widget_set_size_request(widget, width(), Max(0, height() - h));
	}
	else
	{
		if (layout)
		{
			if (menuBar)
				gtk_fixed_move(layout, GTK_WIDGET(menuBar), 0, -h);
			gtk_fixed_move(layout, widget, 0, 0);
		}
		gtk_widget_set_size_request(widget, width(), height());
	}
}

void gMainWindow::setMenuBarVisible(bool v)
{
	_showMenuBar = v;
	
	if (!menuBar)
		return;
	
	configure();
	performArrange();
}

bool gMainWindow::isMenuBarVisible()
{
	//fprintf(stderr, "isMenuBarVisible: %d\n", !!(menuBar && !_hideMenuBar && _showMenuBar));
	return menuBar && !_hideMenuBar && _showMenuBar; //|| (menuBar && GTK_WIDGET_MAPPED(GTK_WIDGET(menuBar)));
}

void gMainWindow::setFont(gFont *ft)
{
	gContainer::setFont(ft);
	gMenu::updateFont(this);
}

void gMainWindow::checkMenuBar()
{
	int i;
	gMenu *menu;

	//fprintf(stderr, "gMainWindow::checkMenuBar\n");
	
	if (menuBar)
	{
		_hideMenuBar = true;
		for (i = 0;; i++)
		{
			menu = gMenu::winChildMenu(this, i);
			if (!menu)
				break;
			if (menu->isVisible() && !menu->isSeparator())
			{
				_hideMenuBar = false;
				break;
			}
		}
	}
		
	configure();
	performArrange();
}

void gMainWindow::embedMenuBar(GtkWidget *border)
{
	if (menuBar)
	{
		// layout is automatically destructed ?
		layout = GTK_FIXED(gtk_fixed_new());

		g_object_ref(G_OBJECT(menuBar));
		
		if (gtk_widget_get_parent(GTK_WIDGET(menuBar)))
			gtk_container_remove(GTK_CONTAINER(gtk_widget_get_parent(GTK_WIDGET(menuBar))), GTK_WIDGET(menuBar));
	
		gtk_fixed_put(layout, GTK_WIDGET(menuBar), 0, 0);

		g_object_unref(G_OBJECT(menuBar));
	
		gtk_widget_reparent(widget, GTK_WIDGET(layout));
		gtk_container_add(GTK_CONTAINER(border), GTK_WIDGET(layout));
	
		gtk_widget_show(GTK_WIDGET(menuBar));
		gtk_widget_show(GTK_WIDGET(layout));
		gtk_widget_show(GTK_WIDGET(widget));
		
		gMenu::updateFont(this);
		gMenu::updateColor(this);
		
		checkMenuBar();
	}
}

void gMainWindow::getScreenPos(int *x, int *y)
{
	gContainer::getScreenPos(x, y);
	//if (y && isMenuBarVisible())
	//	*y += menuBarHeight();
}

double gMainWindow::opacity()
{
	if (isTopLevel())
		return gtk_window_get_opacity(GTK_WINDOW(border));
	else
		return 1.0;
}

void gMainWindow::setOpacity(double v)
{
	if (isTopLevel())
		gtk_window_set_opacity(GTK_WINDOW(border), v);
}

int gMainWindow::screen()
{
	gMainWindow *tl = topLevel();
	return gdk_screen_get_number(gtk_window_get_screen(GTK_WINDOW(tl->border)));
}

void gMainWindow::emitResize()
{
	if (bufW == _resize_last_w && bufH == _resize_last_h)
		return;
	
	_resize_last_w = bufW;
	_resize_last_h = bufH;
	configure();
	performArrange();
	emit(SIGNAL(onResize));
}