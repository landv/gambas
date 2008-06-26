/***************************************************************************

  gcontrol.cpp

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

#ifndef GAMBAS_DIRECTFB
#ifdef GDK_WINDOWING_X11
#include <gdk/gdkx.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/keysymdef.h>
#include <X11/X.h>
#endif
#endif

#include "widgets.h"
#include "widgets_private.h"
#include "gapplication.h"
#include "gbutton.h"
#include "gdrawingarea.h"
#include "gmainwindow.h"
#include "gmoviebox.h"
#include "gpicturebox.h"
#include "gplugin.h"
#include "gscrollbar.h"
#include "gslider.h"
#include "gsplitter.h"
#include "gdesktop.h"
#include "gdrag.h"
#include "gcontrol.h"

static GList *controls=NULL;
static GList *controls_destroyed=NULL;

static const gchar _cursor_fdiag[] = {
   0x3f, 0x00, 0x1f, 0x00, 0x0f, 0x00, 0x0f, 0x00, 0x13, 0x00, 0x21, 0x00,
   0x40, 0x00, 0x80, 0x00, 0x00, 0x01, 0x00, 0x02, 0x00, 0x84, 0x00, 0xc8,
   0x00, 0xf0, 0x00, 0xf0, 0x00, 0xf8, 0x00, 0xfc };
   
static const gchar _cursor_bdiag[] = {
   0x00, 0xfc, 0x00, 0xf8, 0x00, 0xf0, 0x00, 0xf0, 0x00, 0xc8, 0x00, 0x84,
   0x00, 0x02, 0x00, 0x01, 0x80, 0x00, 0x40, 0x00, 0x21, 0x00, 0x13, 0x00,
   0x0f, 0x00, 0x0f, 0x00, 0x1f, 0x00, 0x3f, 0x00 };
   
static const gchar _cursor_splith[] = {
   0x60, 0x06, 0x60, 0x06, 0x60, 0x06, 0x60, 0x06, 0x64, 0x26, 0x66, 0x66,
   0x67, 0xe6, 0x7f, 0xfe, 0x7f, 0xfe, 0x67, 0xe6, 0x66, 0x66, 0x64, 0x26,
   0x60, 0x06, 0x60, 0x06, 0x60, 0x06, 0x60, 0x06 };
   
static const gchar _cursor_splitv[] = {
   0xc0, 0x03, 0xe0, 0x07, 0xf0, 0x0f, 0x80, 0x01, 0x80, 0x01, 0xff, 0xff,
   0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x80, 0x01,
   0x80, 0x01, 0xf0, 0x0f, 0xe0, 0x07, 0xc0, 0x03 };


// Geometry optimization hack - Sometimes fails, so it is disabled...
#define GEOMETRY_OPTIMIZATION 0

/****************************************************************************

gPlugin

****************************************************************************/

gboolean gPlugin_OnUnplug(GtkSocket *socket,gPlugin *data)
{
	if (data->onUnplug) data->onUnplug(data);
	return true;
}

void gPlugin_OnPlug(GtkSocket *socket,gPlugin *data)
{
	if (data->onPlug) data->onPlug(data);
}


gPlugin::gPlugin(gContainer *parent) : gControl(parent)
{
	g_typ=Type_gPlugin;
	
	border = gtk_socket_new();
	widget = border;
	realize(false);
	
	onPlug=NULL;
	onUnplug=NULL;
	
	g_signal_connect(G_OBJECT(widget),"plug-removed",G_CALLBACK(gPlugin_OnUnplug),(gpointer)this);
	g_signal_connect(G_OBJECT(widget),"plug-added",G_CALLBACK(gPlugin_OnPlug),(gpointer)this);

	g_object_set(G_OBJECT(widget),"can-focus",TRUE, (void *)NULL);
}

int gPlugin::client()
{
	GdkNativeWindow win=gtk_socket_get_id(GTK_SOCKET(widget));
	return (int)win;
}

void gPlugin::plug(long id,bool prepared)
{
	//int bc;

	//for (bc=0; bc<10; bc++)
	//{
		if (!prepared) gtk_socket_steal(GTK_SOCKET(widget),(GdkNativeWindow)id);
		else gtk_socket_add_id(GTK_SOCKET(widget),(GdkNativeWindow)id);
	//}
}

void gPlugin::discard()
{
	#ifdef GAMBAS_DIRECTFB
	stub("DIRECTFB/gPlugin:discard()");
	#else
	#ifdef GDK_WINDOWING_X11
	GtkWidget *fr;
	gColor color;
	
	GdkNativeWindow win=gtk_socket_get_id(GTK_SOCKET(widget));
	GdkDisplay* dsp=gdk_display_get_default();
	
	if (!win) return;
	
	XReparentWindow(GDK_DISPLAY_XDISPLAY(dsp),win,GDK_ROOT_WINDOW(), 0, 0);
	
	color = foreground();
	gtk_widget_destroy(widget);
	
	border = widget = gtk_socket_new();
	fr=gtk_bin_get_child(GTK_BIN(border));
	gtk_container_add(GTK_CONTAINER(fr),widget);
	gtk_widget_show(widget);
	setForeground(color);
	#else
	stub("no-X11/gPlugin:discard()");
	#endif
	#endif
	
}

/*****************************************************************

CREATION AND DESTRUCTION

******************************************************************/
void gControl::cleanRemovedControls()
{
	GList *iter;
	gControl *control;

	if (!controls_destroyed) return;

	for(;;)
	{
		iter = g_list_first(controls_destroyed);
		if (!iter)
			break;
		control = (gControl *)iter->data;
		//controls_destroyed = g_list_remove(controls_destroyed, (gpointer)control);
		gtk_widget_destroy(control->border);
	}
	
	controls_destroyed = NULL;
}


void gControl::initAll(gContainer *parent)
{
	bufW=0;
	bufH=0;
	bufX=0;
	bufY=0;
	curs=NULL;
	fnt = NULL;
	g_typ=0;
	dsg=false;
	expa=false;
	igno=false;
	_accept_drops = false;
	_drag_enter = false;
	_drag_get_data = false;
	frame_border = 0;
	frame_padding = 0;
	bg_set = false;
	fg_set = false;
	have_cursor = false;
	use_base = false;
	mous=-1;
	pr = parent;
	_name=NULL;
	visible = false;
	_locked = 0;
	_destroyed = false;
	_no_delete = false;
	_action = false;
	_dirty_pos = _dirty_size = false;
	_tracking = false;
	no_input_method = false;
	_no_default_mouse_event = false;

	onFinish=NULL;
	onFocusEvent=NULL;
	onKeyEvent=NULL;
	onMouseEvent=NULL;
	onDrag=NULL;
	onDragMove=NULL;
	onDrop=NULL;
	onEnterLeave=NULL;

	controls = g_list_append(controls,this);
	
	frame = border = widget = 0;
}

gControl::gControl()
{
	initAll(NULL);
}

gControl::gControl(gContainer *parent) 
{	
	initAll(parent);
}

gControl::~gControl()
{
	gMainWindow *win = window();
  //fprintf(stderr, "gControl::~gControl: %p (%p)\n", this, pr);
	
	emit(SIGNAL(onFinish));
	
	if (win && win->focus == this)
		win->focus = 0;
	
	if (pr)
		pr->remove(this);
	
	if (gDrag::getSource() == this)
		gDrag::cancel();
	
	if (curs) { delete curs; curs=NULL; }
	gFont::assign(&fnt);
	setName(NULL);

	controls = g_list_remove(controls, this);
	controls_destroyed = g_list_remove(controls_destroyed, this);
}

void gControl::destroy()
{
	if (_destroyed)
		return;
		
	hide();
	//fprintf(stderr, "added to destroy list: %p\n", this);
	controls_destroyed=g_list_prepend(controls_destroyed,(gpointer)this);
	_destroyed = true;
}


bool gControl::enabled()
{
	return GTK_WIDGET_SENSITIVE(border);
}

bool gControl::isVisible()
{
	return visible; //GTK_WIDGET_VISIBLE(border) || visible;
}

bool gControl::isReallyVisible()
{
	return GTK_WIDGET_MAPPED(border);
}

void gControl::setEnabled(bool vl)
{
	gtk_widget_set_sensitive (border, vl);	
}

void gControl::setVisible(bool vl)
{
	visible = vl;
	
	if (vl)
	{
		if (bufW <= 0 || bufH <= 0)
			return;
			
		gtk_widget_show(border);
	}
	else
	{
		gtk_widget_hide(border);
	}
	
	if (pr) pr->performArrange();
}

/*****************************************************************

POSITION AND SIZE

******************************************************************/
int gControl::screenX()
{
	gint x,y;
	
	if (_dirty_pos)
		g_debug("gControl::screenX: dirty pos");
	
	gdk_window_get_origin(border->window,&x,&y);
	return x;
}

int gControl::screenY()
{
	gint x,y;
	
	if (_dirty_pos)
		g_debug("gControl::screenX: dirty pos");
	
	gdk_window_get_origin(border->window,&x,&y);
	return y;
}

int gControl::left()
{
	return bufX;
}

int gControl::top()
{	
	return bufY;
}

int gControl::width()
{
	return bufW;
}

int gControl::height()
{
	return bufH;
}

void gControl::setLeft(int l)
{
	move(l,top());
}

void gControl::setTop(int t)
{
	move(left(),t);
}

void gControl::setWidth(int w)
{
	resize(w,height());
}

void gControl::setHeight(int h)
{
	resize(width(),h);
}

static void send_configure (gControl *control)
{
  GtkWidget *widget;
  GdkEvent *event;

  widget = control->border;

	if (!GTK_WIDGET_REALIZED(widget))
		return;
	
// 	if (control->isWindow())
// 	 g_debug("send configure to window: %s", control->name());
	
	event = gdk_event_new (GDK_CONFIGURE);

  event->configure.window = NULL; //(GdkWindow *)g_object_ref(widget->window);
  event->configure.send_event = TRUE;
  event->configure.x = control->x();
  event->configure.y = control->y();
  event->configure.width = control->width();
  event->configure.height = control->height();
  
  gtk_widget_event (widget, event);
  gdk_event_free (event);
}

void gControl::move(int x, int y, int w, int h)
{
	move(x,y);
	resize(w,h);
}

void gControl::move(int x, int y)
{
	GtkLayout *fx;
	
	if (pr && pr->getClass() == Type_gSplitter) 
		return;

	if (x == bufX && y == bufY) 
		return;
	
	bufX = x;
	bufY = y;
	
	//g_debug("move: %p: %d %d", this, x, y);
	_dirty_pos = true;
	if (pr)
	{
    fx = GTK_LAYOUT(gtk_widget_get_parent(border));
	  //gtk_layout_move(fx, border, x, y);
	  if ((GtkWidget *)fx == pr->getContainer())
	  	pr->performArrange();
  }
	
	#if GEOMETRY_OPTIMIZATION
  gApplication::setDirty();
	#else
	updateGeometry();
	#endif
	
	send_configure(this);
}

void gControl::resize(int w, int h)
{
	if (pr && pr->getClass() == Type_gSplitter) 
		return;
	
	if (w < 1)
		w = 0;
		
	if (h < minimumHeight())
		h = minimumHeight();
		
	if (bufW == w && bufH == h)
		return;
	
	if (w < 1 || h < 1)
	{
		this->bufW = w;
		this->bufH = h;
		
		if (visible)
			gtk_widget_hide(border);
	}
	else
	{
		this->bufW = w;
		this->bufH = h;
		
		if (frame)
		{
			int fw = getFrameWidth() * 2;
			if (w < fw || h < fw)
				gtk_widget_hide(widget);
			else
				gtk_widget_show(widget);
		}
		
		if (visible)
			gtk_widget_show(border);
	
		//g_debug("resize: %p: %d %d", this, w, h);
		_dirty_size = true;
		
		#if GEOMETRY_OPTIMIZATION
		gApplication::setDirty();
		#else
		updateGeometry();
		#endif
		
		if (pr)
			pr->performArrange();
	}
		
	send_configure(this);
}

void gControl::updateGeometry()
{
// 	if (_dirty_pos)
// 	{
// 		g_debug("move: %p -> %d %d", this, x(), y());
// 		_dirty_pos = false;
// 		GtkLayout *fx = GTK_LAYOUT(gtk_widget_get_parent(border));
// 		gtk_layout_move(fx, border, x(), y());
// 	}
// 	
// 	if (_dirty_size)
// 	{
// 		GtkAllocation a = { x(), y(), width(), height() };
// 		g_debug("resize: %p -> %d %d", this, width(), height());
// 		_dirty_size = false;
// 		//gtk_widget_set_size_request(border, width(), height());
// 		gtk_widget_size_allocate(border, 
// 	}	
	if (_dirty_pos || _dirty_size)
	{
		//g_debug("move-resize: %s: %d %d %d %d", this->name(), x(), y(), width(), height());
		if (_dirty_pos)
		{
			GtkLayout *fx = GTK_LAYOUT(gtk_widget_get_parent(border));
			gtk_layout_move(fx, border, x(), y());
			_dirty_pos = false;
		}
		if (_dirty_size)
		{
			//GtkAllocation a = { x(), y(), width(), height() };
			//gtk_widget_size_allocate(border, &a);
			gtk_widget_set_size_request(border, width(), height());
			_dirty_size = false;
		}
	}
}

/*****************************************************************

APPEARANCE

******************************************************************/
bool gControl::expand()
{
	return expa;
}



void gControl::setExpand (bool vl)
{
  if (vl == expa)
    return;
    
	expa=vl;
	
	if (pr) pr->performArrange();
}

bool gControl::ignore()
{
	return igno;
}



void gControl::setIgnore (bool vl)
{
  if (vl == igno)
    return;
    
	igno=vl;
	if (pr) pr->performArrange();
}

char* gControl::toolTip()
{
	GtkTooltipsData* data;
	
	data=gtk_tooltips_data_get(border);
	if (!data) return NULL;
	return data->tip_text;
	
}

void gControl::setToolTip(char* vl)
{
	char *txt=NULL;
	
	if (vl)
		if (vl[0]) txt=vl;
	
	gtk_tooltips_set_tip(gApplication::tipHandle(),border,txt,NULL);
	
}

gFont* gControl::font()
{
	if (fnt)
		return fnt;
	if (pr)
		return pr->font();
	
	return gDesktop::font();
}


void gControl::setFont(gFont *ft)
{
	gFont::set(&fnt, ft->copy());
	gtk_widget_modify_font(widget, fnt ? fnt->desc() : NULL);
	resize();
}


int gControl::mouse()
{
	return mous;
}

gCursor* gControl::cursor()
{
	if (!curs) return NULL;
	return new gCursor(curs);
}

void gControl::setCursor(gCursor *vl)
{
	if (curs) { delete curs; curs=NULL;}
	if (!vl)
	{
		setMouse(-1);
		return;
	}
	curs=new gCursor(vl);
	setMouse(-2);
}

void gControl::updateCursor(GdkCursor *cursor)
{
  if (GDK_IS_WINDOW(border->window))
    gdk_window_set_cursor(border->window, cursor);
  /*if (frame && GDK_IS_WINDOW(frame->window) && frame->window != border->window)
    gdk_window_set_cursor(frame->window, cursor);
  if (widget != frame && widget != border && GDK_IS_WINDOW(widget->window))
    gdk_window_set_cursor(widget->window, cursor);*/
}

void gControl::setMouse(int m)
{
	GdkCursor *cr = NULL;
	GdkPixmap *pix;
	GdkColor col = {0,0,0,0};
	
	mous = m;
	
	if (gApplication::isBusy())
    m = GDK_WATCH;
	
	if (m == -2)
	{
		if (!curs)
		{
			mous = -1;
			updateCursor(NULL);
		}
		if (!curs->cur)
		{
			mous = -1;
			updateCursor(NULL);
		}
		
		updateCursor(curs->cur);
		return;	
	}
	
	if (m != -1) 
	{
		if (m < GDK_LAST_CURSOR)
		{
			cr = gdk_cursor_new((GdkCursorType)m);
		}
		else
		{
			if (m == (GDK_LAST_CURSOR+1)) //FDiag
			{
				pix = gdk_bitmap_create_from_data(NULL,_cursor_fdiag,16,16);
				cr = gdk_cursor_new_from_pixmap(pix,pix,&col,&col,0,0);
				g_object_unref(pix);
			}
			else if (m == (GDK_LAST_CURSOR+2)) //BDiag
			{
				pix = gdk_bitmap_create_from_data(NULL,_cursor_bdiag,16,16);
				cr = gdk_cursor_new_from_pixmap(pix,pix,&col,&col,0,0);
				g_object_unref(pix);
			}
			else if (m == (GDK_LAST_CURSOR+3)) //SplitH
			{
				pix = gdk_bitmap_create_from_data(NULL,_cursor_splith,16,16);
				cr = gdk_cursor_new_from_pixmap(pix,pix,&col,&col,0,0);
				g_object_unref(pix);
			}
			else if (m == (GDK_LAST_CURSOR+4)) //SplitV
			{
				pix = gdk_bitmap_create_from_data(NULL,_cursor_splitv,16,16);
				cr = gdk_cursor_new_from_pixmap(pix,pix,&col,&col,0,0);
				g_object_unref(pix);
			}
		}
	}
	
  updateCursor(cr);
}



/*****************************************************************

HANDLES

******************************************************************/

bool gControl::isWindow()
{
	return g_typ == Type_gMainWindow; 
}

gMainWindow* gControl::window()
{
  if (isWindow())
    return (gMainWindow *)this;
  
  if (!pr)
    return NULL;
  else
    return pr->window();  
}

gMainWindow* gControl::topLevel()
{
	gControl *child = this;
	
	while (!child->isTopLevel())
		child = child->parent();
		
	return (gMainWindow *)child;
}

int gControl::handle()
{
	#ifndef GAMBAS_DIRECTFB
	#ifdef GDK_WINDOWING_X11
	return GDK_WINDOW_XID(border->window);
	#else
	stub("no-X11/gControl::handle()");
	return 0;
	#endif
	#else
	stub("DIRECTFB/gControl::handle()");
	return 0;
	#endif
	
}

/*****************************************************************

MISC

******************************************************************/

void gControl::refresh()
{
	gtk_widget_queue_draw(border);
	if (frame != border && GTK_IS_WIDGET(frame))
		gtk_widget_queue_draw(frame);
	if (widget != frame  && GTK_IS_WIDGET(widget))
		gtk_widget_queue_draw(widget);

	afterRefresh();
}

void gControl::refresh(int x, int y, int w, int h)
{
	if ( (x<0) || (y<0) || (w<0) || (h<0) )
	  gtk_widget_queue_draw(border);
	else	
	  gtk_widget_queue_draw_area(border, x, y, w, h);

	afterRefresh();
}

void gControl::afterRefresh()
{
}

bool gControl::design()
{
	return dsg;
}

void gControl::setDesign(bool vl)
{
	dsg=vl;
}

void gControl::setFocus()
{
	gMainWindow *win = window();
	
	if (win->isVisible())
	{
		//if (isVisible() && bufW > 0 && bufH > 0)
		gtk_widget_grab_focus(widget);
	}
	else
		win->focus = this;
}

bool gControl::hasFocus()
{
	return GTK_WIDGET_HAS_FOCUS(border) || GTK_WIDGET_HAS_FOCUS(widget);
}

gControl* gControl::next()
{
	GList *list;
	
	if (!pr)
		return NULL;
		
	list = pr->ch_list;
	list = g_list_next(g_list_find(list, (gpointer)this));
	if (!list)
		return NULL;
	else
		return (gControl *)(list->data);
}

gControl* gControl::previous()
{
	GList *list;
	
	if (!pr)
		return NULL;
		
	list = pr->ch_list;
	list = g_list_previous(g_list_find(list, (gpointer)this));
	if (!list)
		return NULL;
	else
		return (gControl *)(list->data);
}


void gControl::lower()
{
	GList *iter;
	GList *chd;
	GtkWidget *child;
	gControl *Br;
	int x,y;

	if (!pr) return;
	if (pr->getClass()==Type_gSplitter) return;
	
	if (border->window)
	{
		gdk_window_lower(border->window);
	}
	else
	{	
		fprintf(stderr, "WARNING: gControl::lower(): no window\n");
		
		if (!(chd=gtk_container_get_children(GTK_CONTAINER(pr->getContainer())))) return;
		chd=g_list_first(chd);
		
		while (chd)
		{
			child=(GtkWidget*)chd->data;
			
			Br=NULL;
			if (controls)
			{
				iter=g_list_first(controls);
				while (iter)
				{
					if ( ((gControl*)iter->data)->border == child)
					{
						Br=(gControl*)iter->data;
						break;
					}
					iter=iter->next;
				}
			}
			
			if (Br && (Br != this))
			{
				x=Br->left();
				y=Br->top();
				g_object_ref(G_OBJECT(Br->border));
				gtk_container_remove(GTK_CONTAINER(pr->getContainer()),Br->border);
				gtk_layout_put(GTK_LAYOUT(pr->getContainer()),Br->border,x,y);
				g_object_unref(G_OBJECT(Br->border));
			}
			
			chd=g_list_next(chd);
		}
	}
		
	pr->ch_list = g_list_remove(pr->ch_list, this);
	pr->ch_list = g_list_prepend(pr->ch_list, this);
	pr->updateFocusChain();
}

void gControl::raise()
{
	int x,y;

	if (!pr) return;
	if (pr->getClass()==Type_gSplitter) return;
	
	if (border->window)
	{
		gdk_window_raise(border->window);
	}
	else
	{	
		fprintf(stderr, "WARNING: gControl::raise(): no window\n");
		
		x=left();
		y=top();
		g_object_ref(G_OBJECT(border));
		gtk_container_remove(GTK_CONTAINER(pr->getContainer()),border);
		gtk_layout_put(GTK_LAYOUT(pr->getContainer()),border,x,y);
		g_object_unref(G_OBJECT(border));
	}
	
	pr->ch_list = g_list_remove(pr->ch_list, this);
	pr->ch_list = g_list_append(pr->ch_list, this);
	pr->updateFocusChain();
}

void gControl::setNext(gControl *ctrl)
{
	#ifdef GAMBAS_DIRECTFB
	stub("DIRECTFB/gControl::setNext()");
	#else
	GList *next;
	Window stack[2];
	
	if (!ctrl)
	{
		raise();
		return;
	}
	
	if (ctrl == this || isTopLevel() || ctrl->parent() != parent())
		return;
	
	stack[0] = GDK_WINDOW_XID(ctrl->border->window);
	stack[1] = GDK_WINDOW_XID(border->window);
	XRestackWindows(GDK_WINDOW_XDISPLAY(border->window), stack, 2 );
	
	next = g_list_find(pr->ch_list, ctrl);
	
	pr->ch_list = g_list_remove(pr->ch_list, this);
	pr->ch_list = g_list_insert_before(pr->ch_list, next, this);
	pr->updateFocusChain();
	#endif
}

void gControl::setPrevious(gControl *ctrl)
{
	if (!ctrl)
		lower();
	else
		setNext(ctrl->next());
}

gPicture* gControl::grab()
{
	return Grab_gdkWindow(border->window);
}

/*********************************************************************

Drag & Drop

**********************************************************************/

void gControl::setAcceptDrops(bool vl)
{
	//GtkWidget *w;
	//GtkTargetEntry entry[7];
	
  /* BM: ??
	if (!pr) w=frame;
	else w=widget;
	*/
	
	if (vl == _accept_drops)
		return;
		
	_accept_drops = vl;
	
	if (vl)
	{
		gtk_drag_dest_set(border,(GtkDestDefaults)0, 0, 0, (GdkDragAction)(GDK_ACTION_COPY | GDK_ACTION_MOVE | GDK_ACTION_LINK));
	}
	else
	{
		gtk_drag_dest_unset(border);
	}
}

/*********************************************************************

Internal

**********************************************************************/

void gControl::connectParent()
{
	if (pr)
	{
    gtk_widget_set_redraw_on_allocate(border, false);
    
  	pr->insert(this);
    
    gtk_widget_realize(border);
		gtk_widget_show_all(border);
    //show(); 
		visible = true;
		/*if (GTK_IS_WIDGET(frame))
			gtk_widget_show(frame);
		if (GTK_IS_WIDGET(widget))
			gtk_widget_show(widget);*/
    
		setBackground();
		setForeground();
    setFont(pr->font());
    //gdk_window_process_updates(border->window,true);
  }
  
	// BM: Widget has been created, so we can set its cursor if application is busy
	if (gApplication::isBusy() && mustUpdateCursor())
    setMouse(mouse());		
}

GList* gControl::controlList()
{
	return controls; 
}

void gControl::drawBorder(GdkDrawable *win)
{
	GtkShadowType shadow;
	gint x, y, w, h;
	//GdkGC *gc;
	GtkStyle* st;
	
	if (getFrameBorder() == BORDER_NONE)
    return;
	
	if (!win)
	{
		GtkWidget *wid;
		
		if (frame)
			wid = frame;
		else
			wid = widget;
			
		if (GTK_IS_LAYOUT(wid))
			win = GTK_LAYOUT(wid)->bin_window;
		else
			win = wid->window;
	}
	
	x = 0;
	y = 0;
  w = width();
  h = height();
  
  if (w < 1 || h < 1)
  	return;
  
	st = gtk_widget_get_style(widget);
  
	switch (getFrameBorder())
	{
    case BORDER_PLAIN:
    {
      gdk_draw_rectangle(win, use_base ? st->text_gc[GTK_STATE_NORMAL] : st->fg_gc[GTK_STATE_NORMAL], FALSE, x, y, w - 1, h - 1); 
      return;
    }
    
    case BORDER_SUNKEN: shadow = GTK_SHADOW_IN; break;
    case BORDER_RAISED: shadow = GTK_SHADOW_OUT; break;
    case BORDER_ETCHED: shadow = GTK_SHADOW_ETCHED_IN; break;
    
    default: 
      return;
	}
	
  gtk_paint_shadow(st, win, GTK_STATE_NORMAL, shadow, NULL, NULL, NULL, x, y, w, h);
}

static gboolean cb_frame_expose(GtkWidget *wid, GdkEventExpose *e, gControl *control)
{
	control->drawBorder();
	return false;
}

static void cb_size_allocate(GtkWidget *wid, GdkEventExpose *e, gContainer *container)
{
	if (!container->isTopLevel())
		container->performArrange();
}


/*
  The different cases managed by gControl::realize()

  border     frame      widget
    0          0          W
    0          F          W
    B          0          W
    B          F          W
*/

static void add_container(GtkWidget *parent, GtkWidget *child)
{
  GtkWidget *ch;
  
  for(;;)
  {
    if (!GTK_IS_BIN(parent))
      break;
    
    ch = gtk_bin_get_child(GTK_BIN(parent));
    if (!ch)
      break;
      
    parent = ch;
  }
  
  gtk_container_add(GTK_CONTAINER(parent), child);
}


void gControl::realize(bool make_frame)
{
  if (!make_frame)
  {
    frame = widget;
  }
  else if (!frame)
  {
    frame = gtk_alignment_new(0, 0, 1, 1);
    gtk_widget_set_redraw_on_allocate(frame, TRUE);
  }
  
  if (!border)
    border = widget;
  
  //printf("border = %p / frame = %p / widget =%p\n", border, frame, widget);
  
  if (border != frame)
  {
    //printf("frame -> border\n");
    add_container(border, frame);
  }
  if (frame != widget && border != widget)
  {
    //printf("widget -> frame\n");
    add_container(frame, widget);
  }
  
  if (!make_frame)
    frame = 0;

	connectParent();
	initSignals();
	
  if (frame)
		g_signal_connect_after(G_OBJECT(frame), "expose-event", G_CALLBACK(cb_frame_expose), (gpointer)this);
		
	if (isContainer() && widget != border)
		g_signal_connect(G_OBJECT(widget), "size-allocate", G_CALLBACK(cb_size_allocate), (gpointer)this);
}

void gControl::updateBorder()
{
  int padding;
  
  if (!frame)
    return;
    
  if (!GTK_IS_ALIGNMENT(frame))
  {
    refresh();
    return;
  }
  
  switch (frame_border)
  {
    case BORDER_NONE: padding = 0; break;
    case BORDER_PLAIN: padding = 1; break;
    default: padding = 2; break; // TODO: Use style
  }
  
  if ((int)frame_padding > padding)
    padding = frame_padding;
  
  gtk_alignment_set_padding(GTK_ALIGNMENT(frame), padding, padding, padding, padding);
  refresh();
  //gtk_widget_queue_draw(frame);
}

int gControl::getFrameWidth()
{
	guint p;
	
	if (frame && GTK_IS_ALIGNMENT(frame))
	{
  	gtk_alignment_get_padding(GTK_ALIGNMENT(frame), &p, NULL, NULL, NULL);
  	return p;
  }
  
  if (GTK_IS_SCROLLED_WINDOW(border))
  {
		if (gtk_scrolled_window_get_shadow_type(GTK_SCROLLED_WINDOW(border)) == GTK_SHADOW_NONE)
			return 0;
		else
			return 2;
  }
  
  return 0;
}

void gControl::setFrameBorder(int border)
{
  if (border < BORDER_NONE || border > BORDER_ETCHED)
    return;
    
  frame_border = border;
  updateBorder();
}


void gControl::setFramePadding(int padding)
{
  if (padding < 0)
    padding = 0;
  frame_padding = padding;
  updateBorder();  
}


void gControl::setName(char *name)
{
	if (_name) g_free(_name);
	_name = NULL;
	if (name) _name = g_strdup(name);
}


gColor gControl::realBackground()
{
	if (bg_set)
		return use_base ? get_gdk_base_color(widget) : get_gdk_bg_color(widget);
	else if (!isWindow())
		return pr->realBackground();
	else
		return COLOR_DEFAULT;
}

gColor gControl::background()
{
	if (bg_set)
		return realBackground();
	else
		return COLOR_DEFAULT;
}

static void set_background(GtkWidget *widget, gColor color, bool use_base)
{
	if (use_base)
		set_gdk_base_color(widget, color);
	else
		set_gdk_bg_color(widget, color);
}

void gControl::setRealBackground(gColor color)
{
	set_background(border, color, use_base);
	if (border != frame && GTK_IS_WIDGET(frame))
		set_background(frame, color, use_base);
	if (frame != widget)
		set_background(widget, color, use_base);
}

void gControl::setBackground(gColor color)
{
	bg_set = color != COLOR_DEFAULT;
	
	if (!bg_set)
	{
		if (isWindow())
			return;
		color = pr->realBackground();
	}
			
	setRealBackground(color);
}

gColor gControl::realForeground()
{
	if (fg_set)
		return use_base ? get_gdk_text_color(widget) : get_gdk_fg_color(widget);
	else if (!isWindow())
		return pr->realForeground();
	else
		return COLOR_DEFAULT;
}

gColor gControl::foreground()
{
	if (fg_set)
		return realForeground();
	else
		return COLOR_DEFAULT;
}

static void set_foreground(GtkWidget *widget, gColor color, bool use_base)
{
	if (use_base)
		set_gdk_text_color(widget, color);
	else
		set_gdk_fg_color(widget, color);
}

void gControl::setRealForeground(gColor color)
{
	set_foreground(widget, color, use_base);
	/*set_foreground(border, color, use_base);
	if (border != frame && GTK_IS_WIDGET(frame))
		set_foreground(frame, color, use_base);
	if (frame != widget)*/
}

void gControl::setForeground(gColor color)
{
	fg_set = color != COLOR_DEFAULT;
	
	if (!fg_set)
	{
		if (isWindow())
			return;
		color = pr->realForeground();
	}
	
	setRealForeground(color);
}

void gControl::emit(void *signal)
{
	if (!signal || locked())
		return;
	(*((void (*)(gControl *))signal))(this);
}

void gControl::emit(void *signal, intptr_t arg)
{
	if (!signal || locked())
		return;
	(*((void (*)(gControl *, intptr_t))signal))(this, arg);
}

void gControl::reparent(gContainer *newpr, int x, int y)
{
	gContainer *oldpr;
	
	// newpr can be equal to pr: for example, to move a control for one
	// tab to another tab of the same TabStrip!
	
	if (!newpr || !newpr->getContainer())
		return;
	
	oldpr = pr;
	pr = newpr;	
	
	if (oldpr == newpr)
	{
		gtk_widget_reparent(border, newpr->getContainer());
		oldpr->performArrange();
		move(x, y);
	}
	else
	{		
		if (oldpr)
		{
			gtk_widget_reparent(border, newpr->getContainer());
			oldpr->remove(this);
			oldpr->performArrange();
		}
		
		newpr->insert(this, x, y);
	}
}

int gControl::scrollX()
{
	if (!GTK_IS_SCROLLED_WINDOW(border))
		return 0;
	
	return (int)gtk_scrolled_window_get_hadjustment(GTK_SCROLLED_WINDOW(border))->value;
}

int gControl::scrollY()
{
	if (!GTK_IS_SCROLLED_WINDOW(border))
		return 0;
	
	return (int)gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(border))->value;
}

void gControl::setScrollX(int vl)
{
	GtkAdjustment* adj;
	int max;
	
	if (!GTK_IS_SCROLLED_WINDOW(border))
		return;
		
	adj = gtk_scrolled_window_get_hadjustment(GTK_SCROLLED_WINDOW(border));
	
	max = (int)(adj->upper - adj->page_size);
	
	if (vl < 0) 
		vl = 0;
	else if (vl > max)
		vl = max;
	
	gtk_adjustment_set_value(adj, (gdouble)vl);
}

void gControl::setScrollY(int vl)
{
	GtkAdjustment* adj;
	int max;
	
	if (!GTK_IS_SCROLLED_WINDOW(border))
		return;
		
	adj = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(border));
	
	max = (int)(adj->upper - adj->page_size);
	
	if (vl < 0) 
		vl = 0;
	else if (vl > max)
		vl = max;
	
	gtk_adjustment_set_value(adj, (gdouble)vl);
}

void gControl::scroll(int x, int y)
{
	setScrollX(x);
	setScrollY(y);
}

int gControl::scrollWidth()
{
	/*if (widget->requisition.width <= 0)
	{
		gtk_widget_realize(widget);
		gtk_widget_size_request(widget, &widget->requisition);
	}*/
	return widget->requisition.width;
}

int gControl::scrollHeight()
{
	/*if (widget->requisition.height <= 0)
	{
		gtk_widget_realize(widget);
		gtk_widget_size_request(widget, &widget->requisition);
	}*/
	return widget->requisition.height;
}

int gControl::scrollBar()
{
	GtkPolicyType h, v;
	int ret = 3;
	
	if (!GTK_IS_SCROLLED_WINDOW(border))
		return 0;
	
	gtk_scrolled_window_get_policy(GTK_SCROLLED_WINDOW(border), &h, &v);
	if (h == GTK_POLICY_NEVER) ret--;
	if (v == GTK_POLICY_NEVER) ret -= 2;
	
	return ret;
}

void gControl::setScrollBar(int vl)
{
	if (!GTK_IS_SCROLLED_WINDOW(border))
		return;
	
	switch(vl)
	{
		case 0:
			gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(border), GTK_POLICY_NEVER, GTK_POLICY_NEVER);
			break;
		case 1:
			gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(border), GTK_POLICY_AUTOMATIC, GTK_POLICY_NEVER);
			break;
		case 2:
			gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(border), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
			break;
		case 3:
			gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(border), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
			break;
	}
}

int gControl::minimumHeight()
{
	return 0;
}
