/***************************************************************************

  gmainwindow.cpp

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
#include "gmainwindow.h"

static gboolean win_frame(GtkWidget *widget,GdkEventWindowState *event,gMainWindow *data)
{
	data->performArrange();
	return false;
}

static void cb_show (GtkWidget *widget, gMainWindow *data)
{
	//fprintf(stderr, "cb_show: %p\n", data);
	data->emitOpen();
	if (data->opened)
	{
		//data->performArrange();
		data->emit(SIGNAL(data->onShow));
		data->_not_spontaneous = false;
	}
}

static void cb_hide (GtkWidget *widget, gMainWindow *data)
{
	data->emit(SIGNAL(data->onHide));
	data->_not_spontaneous = false;
}

static gboolean win_close(GtkWidget *widget,GdkEvent  *event,gMainWindow *data)
{
	data->doClose();
	return true;
}

static gboolean resize_later(gMainWindow *data)
{
	data->bufW = data->_next_w;
	data->bufH = data->_next_h;
	data->_next_timer = 0;
	data->performArrange();

	if (data->onResize) 
		data->onResize(data);
	
	data->refresh();
	
	return false;
}

static gboolean cb_configure(GtkWidget *widget, GdkEventConfigure *event, gMainWindow *data)
{
	gint x, y;
	//fprintf(stderr, "cb_configure: %p\n", data);
	
	if (data->opened)
	{
		if (data->isTopLevel())
			gtk_window_get_position(GTK_WINDOW(data->border), &x, &y);
		else
		{
			x = event->x;
			y = event->y;
		}
		if (x != data->bufX || y != data->bufY)
		{
			data->bufX = x;
			data->bufY = y;
			if (data->onMove) data->onMove(data);
		}
		
		if ( (event->width!=data->bufW) || (event->height!=data->bufH) || (data->_resized) || !event->window )
		{
			data->_next_w = event->width;
			data->_next_h = event->height;
/*			data->bufW=event->width;
			data->bufH=event->height;
			//g_debug("gMainWindow: cb_configure: %d", event->send_event);
			data->performArrange();
			if (data->onResize) data->onResize(data);*/
			if (data->_resized || data->parent())
			{
				data->_resized = false;
				resize_later(data);
			}
			else
			{
				//resize_later(data);
				if (!data->_next_timer)
					data->_next_timer = g_timeout_add(50, (GSourceFunc)resize_later, data);
			}
		}
	}
	
	return false;
}

static void cb_open(GtkWidget *widget, GdkEvent *e, gMainWindow *data)
{
	data->emitOpen();
}

static gboolean cb_expose(GtkWidget *wid, GdkEventExpose *e, gMainWindow *data)
{
	if (data->_background)
	{
		gdk_window_clear(data->border->window);
		gdk_window_clear(GTK_LAYOUT(data->widget)->bin_window);
	}
	
	return false;
}



GList *gMainWindow::windows = NULL;
gMainWindow *gMainWindow::_active = NULL;
gMainWindow *gMainWindow::_current = NULL;

static GtkWindowGroup *_current_group = NULL;

void gMainWindow::initialize()
{
	//fprintf(stderr, "new window: %p in %p\n", this, parent());

	opened = false;
	sticky = false;
	persistent = false;
	stack = 0;
	_mask = false;
	_masked = false;
	_resized = false;
	accel = NULL;
	_default = NULL;
	_cancel = NULL;
	menuBar = NULL;
	top_only = false;
	_icon = NULL;
	_picture = NULL;
	focus = 0;
	_closing = false;
	_title = NULL;
	_not_spontaneous = false;
	_skip_taskbar = false;
	_current = NULL;
	_background = NULL;
	_style = NULL;
	_next_timer = 0;

	onOpen = NULL;
	onShow = NULL;
	onHide = NULL;
	onMove = NULL;
	onResize = NULL;
	onActivate = NULL;
	onDeactivate = NULL;
	
	accel = gtk_accel_group_new();
}

void gMainWindow::initWindow()
{
	//resize(200,150);
	
	if (parent())
	{
		g_signal_connect(G_OBJECT(border), "show", G_CALLBACK(cb_show), (gpointer)this);
		g_signal_connect(G_OBJECT(border), "configure-event", G_CALLBACK(cb_configure), (gpointer)this);
		g_signal_connect_after(G_OBJECT(border), "map-event", G_CALLBACK(cb_open), (gpointer)this);
		g_signal_connect(G_OBJECT(border),"show",G_CALLBACK(cb_show),(gpointer)this);
		g_signal_connect(G_OBJECT(border),"hide",G_CALLBACK(cb_hide),(gpointer)this);
		//g_signal_connect_after(G_OBJECT(border), "size-allocate", G_CALLBACK(cb_configure), (gpointer)this);
	}
	else
	{
		gtk_widget_add_events(widget,GDK_BUTTON_MOTION_MASK);
		
		//g_signal_connect(G_OBJECT(border),"size-request",G_CALLBACK(cb_realize),(gpointer)this);
		g_signal_connect(G_OBJECT(border), "show",G_CALLBACK(cb_show),(gpointer)this);
		g_signal_connect(G_OBJECT(border), "hide",G_CALLBACK(cb_hide),(gpointer)this);
		g_signal_connect(G_OBJECT(border), "configure-event",G_CALLBACK(cb_configure),(gpointer)this);
		g_signal_connect(G_OBJECT(border), "delete-event",G_CALLBACK(win_close),(gpointer)this);
		g_signal_connect(G_OBJECT(border), "window-state-event",G_CALLBACK(win_frame),(gpointer)this);
		g_signal_connect(G_OBJECT(widget), "expose-event", G_CALLBACK(cb_expose), (gpointer)this);
	}
	
	//GTK_WIDGET_SET_FLAGS (widget, GTK_CAN_FOCUS);
	//gtk_widget_add_events(widget, GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE_MASK);
	
	/*	GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK
		| GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK
		| GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE_MASK | GDK_POINTER_MOTION_MASK);*/

	gtk_window_add_accel_group(GTK_WINDOW(topLevel()->border), accel);
}

gMainWindow::gMainWindow(int plug) : gContainer(NULL)
{
  initialize();
	g_typ = Type_gMainWindow;
	
	windows = g_list_append(windows, (gpointer)this);
	
	if (plug)
		border = gtk_plug_new(plug);
	else
	{
		border = gtk_window_new(GTK_WINDOW_TOPLEVEL);
		have_cursor = true;
	}
	
	widget = gtk_layout_new(0,0);
	
	realize(false);
	initWindow();
	
	gtk_widget_realize(border);
	gtk_widget_show(widget);
	
	gtk_widget_set_size_request(border, 4, 4);
}

gMainWindow::gMainWindow(gContainer *par) : gContainer(par)
{
	initialize();
	g_typ = Type_gMainWindow;
	
	border=gtk_event_box_new();
	widget=gtk_layout_new(0,0);
	
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
	}
	
	if (_next_timer)
		g_source_remove(_next_timer);
	
	gPicture::assign(&_picture);
	gPicture::assign(&_icon);
	if (_title) g_free(_title);
	g_object_unref(accel);
	if (_background) g_object_unref(_background);
	if (_style) g_object_unref(_style);
	
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
	if (parent()) return;

	if (vl) gtk_window_stick(GTK_WINDOW(border));
	else    gtk_window_unstick(GTK_WINDOW(border));
}

void gMainWindow::setStacking(int vl)
{
  stack=vl;
	if (parent()) return;

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
		gControl::setRealBackground(color);
}

void gMainWindow::move(int x, int y)
{
	if (isTopLevel())
	{
		if (x == bufX && y == bufY) 
			return;
	
		bufX = x;
		bufY = y;
		gtk_window_move(GTK_WINDOW(border), x, y);
	}
	else
	{
		gContainer::move(x,y);
	}
}


void gMainWindow::resize(int w, int h)
{
	if (isTopLevel())
	{
		if (w == bufW && h == bufH)
			return;
			
		_resized=true;
		bufW = w;
		bufH = h;
		
		//gdk_window_enable_synchronized_configure (border->window);
		
		if (gtk_window_get_resizable(GTK_WINDOW(border)))
			gtk_window_resize(GTK_WINDOW(border),w,h);
		else
			gtk_widget_set_size_request(border, w, h);
	}
	else
	{
		//fprintf(stderr, "resize %p -> (%d %d) (%d %d)\n", this, bufW, bufH, w, h);
		gContainer::resize(w, h);
	}
}

void gMainWindow::emitOpen()
{
	if (!opened)
	{
		//fprintf(stderr, "emit Open: %p (%d %d)\n", this, width(), height());
		opened = true;
		gtk_widget_realize(border);
		performArrange();
		emit(SIGNAL(onOpen));
		if (opened)
		{
			//fprintf(stderr, "emit Move & Resize: %p\n", this);			
			emit(SIGNAL(onMove));
			emit(SIGNAL(onResize));
		}
	}
}

void gMainWindow::setVisible(bool vl)
{
	GtkWindowGroup *group;
  gMainWindow *active;

	if (vl)
	{
		emitOpen();
		if (!opened)
			return;
		
		if (isTopLevel())
		{
			group = _current_group;
			if (!group)
				group = gtk_window_get_group(NULL);
			
			//fprintf(stderr, "adding window %p to group %p\n", this, group);
			gtk_window_group_add_window(group, GTK_WINDOW(border));
		}
			
		_not_spontaneous = !visible;
		visible = true;
		
		if (isTopLevel())
		{
			if (!_title || !*_title)
				gtk_window_set_title(GTK_WINDOW(border), gApplication::defaultTitle());
			
			active = gDesktop::activeWindow();
			if (active && active != this)
				gtk_window_set_transient_for(GTK_WINDOW(border), GTK_WINDOW(active->border));
			
			gtk_window_present(GTK_WINDOW(border));
			drawMask();
		}
		else 
		{
			gtk_widget_show(border);
			parent()->performArrange();
		}
		
		if (!focus)
		{
			focus = findFirstFocus();
			//fprintf(stderr, "findFirstFocus: %s\n", focus ? focus->name() : NULL); 
		}
		
		if (focus)
		{
			focus->setFocus();
			focus = 0;
		}
	}
	else
	{
		_not_spontaneous = visible;
		gControl::setVisible(false);
	}
}


void gMainWindow::setMinimized(bool vl)
{
	if (vl) gtk_window_iconify(GTK_WINDOW(border));
	else    gtk_window_deiconify(GTK_WINDOW(border));
}

void gMainWindow::setMaximized(bool vl)
{
	if (vl) gtk_window_maximize(GTK_WINDOW(border));
	else    gtk_window_unmaximize(GTK_WINDOW(border));
}

void gMainWindow::setFullscreen(bool vl)
{
	if (vl) gtk_window_fullscreen(GTK_WINDOW(border));
	else    gtk_window_unfullscreen(GTK_WINDOW(border));
}

bool gMainWindow::minimized()
{
	return (bool)(gdk_window_get_state(border->window) &  GDK_WINDOW_STATE_ICONIFIED);
}

bool gMainWindow::maximized()
{
	return (bool)(gdk_window_get_state(border->window) &  GDK_WINDOW_STATE_MAXIMIZED);
}

bool gMainWindow::fullscreen()
{
	return (bool)(gdk_window_get_state(border->window) &  GDK_WINDOW_STATE_FULLSCREEN);
}

void gMainWindow::center()
{
	int myx,myy;
	
	if (parent()) return;
	
	myx=(gDesktop::width()/2)-(width()/2);
	myy=(gDesktop::height()/2)-(height()/2);
	
	move(myx,myy);
}

bool gMainWindow::modal()
{
	if (parent()) return false;

	return gtk_window_get_modal (GTK_WINDOW(border));
}

void gMainWindow::showModal()
{
  gMainWindow *save;
  GtkWindowGroup *save_group;
	
	if (!isTopLevel()) return;
	if (modal()) return;
	
	save = _current;
	_current = this;
	
	gtk_window_set_modal(GTK_WINDOW(border), true);
  
  center();
	show();

	save_group = _current_group;
	_current_group = gtk_window_group_new();

	//fprintf(stderr, "showModal: begin %p\n", this);

	do
	{
		do_iteration(false);
	}
	while (opened);
	
	//fprintf(stderr, "showModal: end %p\n", this);

	g_object_unref(_current_group);
	_current_group = save_group;	
	_current = save;
	
	if (!persistent)
		destroyNow();
	else
		hide();
	
}

void gMainWindow::raise()
{
	if (parent()) { gControl::raise(); return; }
	gtk_window_present(GTK_WINDOW(border));
}

const char* gMainWindow::text()
{
	return _title;
}

bool gMainWindow::skipTaskBar()
{
	if (parent()) 
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
	return gtk_window_get_decorated(GTK_WINDOW(border));
}

bool gMainWindow::isResizable()
{
	return gtk_window_get_resizable(GTK_WINDOW(border));
}

void gMainWindow::setBorder(bool b)
{
  if (parent()) 
  	return;
	
	gtk_window_set_decorated(GTK_WINDOW(border), b);
}

void gMainWindow::setResizable(bool b)
{
  if (parent()) 
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
	if (parent()) return;
	gtk_window_set_skip_taskbar_hint (GTK_WINDOW(border), b);
}


/*gPicture* gMainWindow::icon()
{
	GdkPixbuf *buf;
	gPicture *pic;
	
	if (parent()) return NULL;
	
	buf=gtk_window_get_icon(GTK_WINDOW(border));
	if (!buf) return NULL;
	
	pic=gPicture::fromPixbuf(buf);
	
	return pic;
}*/

void gMainWindow::setIcon(gPicture *pic)
{
  gPicture::assign(&_icon, pic);

	if (parent()) return;
  gtk_window_set_icon(GTK_WINDOW(border), pic ? pic->getPixbuf() : NULL);
}

bool gMainWindow::topOnly()
{
	if (parent()) return false;
	
	return top_only;
}

void gMainWindow::setTopOnly(bool vl)
{
	if (parent()) return;

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
}

void gMainWindow::drawMask()
{
	GdkBitmap *mask;
	GdkPixmap *back;
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
	
	//if (_background)
	//	g_object_unref(_background);
		
	mask = (_mask && _picture) ? _picture->getMask() : NULL;
	do_remap = !mask && _masked;
	
	#ifdef GDK_WINDOWING_X11
  XShapeCombineMask(GDK_WINDOW_XDISPLAY(border->window), GDK_WINDOW_XID(border->window), ShapeBounding, 0, 0,
  	mask ? GDK_PIXMAP_XID(mask) : None, ShapeSet);
  #else
	//gdk_window_shape_combine_mask(border->window, mask, 0, 0);
  #endif

	back = _picture ? _picture->getPixmap() : NULL;
	gtk_widget_set_double_buffered(border, back == NULL);	
	gtk_widget_set_double_buffered(widget, back == NULL);	
	
	if (back)
	{
		gdk_window_set_back_pixmap(border->window, back, FALSE);
		gdk_window_set_back_pixmap(GTK_LAYOUT(widget)->bin_window, back, FALSE);
		gdk_window_clear(border->window);
		gdk_window_clear(GTK_LAYOUT(widget)->bin_window);
		gtk_widget_set_app_paintable(border, true);
		gtk_widget_set_app_paintable(widget, true);
	}
	else
	{
		setRealBackground(background());
	}
	
	if (_background)
		g_object_unref(_background);
	_background = back;
	if (_background)
		g_object_ref(_background);
	
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
	
	//gtk_widget_queue_draw(border);
	
	#if 0
	GdkBitmap *map=NULL;
	GdkPixmap *pix=NULL;
	GdkColor black;
	gint pw,ph;
	GdkPixbuf *buf_mask = _picture ? _picture->getPixbuf() : NULL;	
	
	if (win_style) { 
		g_object_unref(win_style); 
		win_style=NULL; 
	}
	
	if (buf_mask)
	{
		pw=gdk_pixbuf_get_width(buf_mask);
		ph=gdk_pixbuf_get_height(buf_mask);
		
		map=gdk_pixmap_new(NULL,pw,ph,1);
		gdk_pixbuf_render_threshold_alpha(buf_mask,map,0,0,0,0,pw,ph,128);
		
		pix=gdk_pixmap_new(border->window,pw,ph,-1);
		GdkGC* gc=gdk_gc_new(border->window);
		fill_gdk_color(&black,0);
		gdk_gc_set_foreground (gc,&black);
		gdk_gc_set_background (gc,&black);
		gdk_draw_rectangle(pix,gc,true,0,0,pw,ph);
		gdk_draw_pixbuf(pix,gc,buf_mask,0,0,0,0,pw,ph,GDK_RGB_DITHER_MAX,0,0);
		g_object_unref(G_OBJECT(gc));
	}
	
	if (buf_mask && _mask) gtk_widget_shape_combine_mask(border,map,0,0);
	else        gtk_widget_shape_combine_mask(border,NULL,0,0);
	
	if (map)
	{
		win_style=gtk_style_copy(widget->style);
		win_style->bg_pixmap[GTK_STATE_NORMAL]=pix;
		gtk_widget_set_style(widget,win_style);
		gdk_window_invalidate_rect( border->window,NULL,true);
		g_object_unref(G_OBJECT(map));
	}
	else
	{
		gtk_widget_set_style(widget,NULL);
		if (border->window) //GTK_WIDGET_REALIZED(widget))
		{
		  gtk_widget_reset_shapes(border);
		  gdk_window_invalidate_rect(border->window,NULL,true);
    }
	}
	#endif
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
		_closing = true;
		if (onClose) 
		{
			if (!onClose(this))
				opened = false;
		}
		else
			opened = false;
		_closing = false;
  }
  
  if (!opened) // && !modal())
  {
  	if (!modal())
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

void gMainWindow::reparent(gContainer *newpr, int x, int y)
{
	GtkWidget *new_border;
	bool again = false;
	
	//fprintf(stderr, "reparent: %p (%p -> %p)\n", this, parent(), newpr);


	if (!parent() && newpr)
	{
		gtk_window_remove_accel_group(GTK_WINDOW(topLevel()->border), accel);
		new_border = gtk_event_box_new();
		gtk_widget_reparent(widget, new_border);
		_no_delete = true;
		gtk_widget_destroy(border);
		_no_delete = false;
		border = new_border;
		again = true;
	}
	else if (parent() && !newpr)
	{
		gtk_window_remove_accel_group(GTK_WINDOW(topLevel()->border), accel);
		// TODO: test that
		new_border = gtk_window_new(GTK_WINDOW_TOPLEVEL);
		gtk_widget_reparent(widget, new_border);
		_no_delete = true;
		gtk_widget_destroy(border);
		_no_delete = false;
		border = new_border;
		again = true;
	}
	
	gControl::reparent(newpr, x, y);
	
	if (again)
	{
		initWindow();	
		initSignals();
	}
}


int gMainWindow::controlCount()
{
	GList *list = gControl::controlList();
	gControl *ctrl;
	int n = 0;
	
	while (list)
	{
		ctrl = (gControl *)list->data;
		if (ctrl->window() == this)
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
		if (ctrl->window() == this && !strcasecmp(ctrl->name(), name))
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
		if (ctrl->window() == this)
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

int gMainWindow::clientY()
{
	GtkRequisition req;

	if (menuBar)
	{
		gtk_widget_size_request(GTK_WIDGET(menuBar), &req);
		return req.height;
	}
	
	return 0;
}

int gMainWindow::clientWidth()
{
	return width();
}

int gMainWindow::clientHeight()
{
	GtkRequisition req;
	int h = 0;

	if (menuBar)
	{
		gtk_widget_size_request(GTK_WIDGET(menuBar), &req);
		h = req.height;
	}
	
	return height() - h;
}

void gMainWindow::setActiveWindow(gControl *control)
{
	gMainWindow *window = control ? control->window() : NULL;
	gMainWindow *old = _active;
	
	if (window == _active)
		return;
		
	_active = window;
	
	if (old)
		old->emit(SIGNAL(old->onDeactivate));
		
	if (window)
		window->emit(SIGNAL(window->onActivate));
}

#ifdef GDK_WINDOWING_X11
int gMainWindow::getType()
{
	if (parent())
		return 0;
	return X11_get_window_type(handle());
}

void gMainWindow::setType(int type)
{
	if (parent())
		return;
	X11_set_window_type(handle(), type);
}
#else
int gMainWindow::getType()
{
	return 0;
}

void gMainWindow::setType()
{
}
#endif

