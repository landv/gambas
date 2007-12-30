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
#include <gtk/gtk.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include "widgets.h"
#include "widgets_private.h"


static long nwindows;
static gMainWindow **windows;

void stub(char *function)
{
	printf("WARNING: %s not yet implemented\n",function);
}


gboolean win_frame(GtkWidget *widget,GdkEventWindowState *event,gMainWindow *data)
{

	data->performArrange();
	return false;
}

void win_open (GtkWidget *widget, gMainWindow *data)
{

	if (data->onOpen) data->onOpen(data);
}

void win_show (GtkWidget *widget, gMainWindow *data)
{

	if (data->onShow) data->onShow(data);
}

void win_hide (GtkWidget *widget, gMainWindow *data)
{

	if (data->onHide) data->onHide(data);
}

gboolean win_close(GtkWidget *widget,GdkEvent  *event,gMainWindow *data)
{
	bool ret=false;
		

	if (data->onClose) ret=data->onClose(data);
	return ret; 
}

gboolean win_motion(GtkWidget *widget,GdkEventConfigure *event,gMainWindow *data)
{
	
	
	if ( (event->width!=data->bufW) || (event->height!=data->bufH) || (data->resize_flag) )
	{
		data->resize_flag=false;
		data->bufW=event->width;
		data->bufH=event->height;
		data->performArrange();
		if (data->onResize) data->onResize(data);
	}
	
	if ( (event->x!=data->bufX) || (event->y!=data->bufY) )
	{
		data->bufX=event->x;
		data->bufY=event->y;
		if (data->onMove) data->onMove(data);
	}
	
	return false;
}

gMainWindow::gMainWindow(long plug) 
{
	sticky=false;
	stack=0;
	buf_mask=NULL;
	m_mask=false;
	resize_flag=false;
	bufW=0;
	bufH=0;
	bufX=0;
	bufY=0;
	esc_button=NULL;
	menuBar=NULL;
	win_style=NULL;
	g_typ=Type_gMainWindow;
	top_only=false;
	nwindows++;
	if (nwindows==1)
		windows=(gMainWindow**)g_malloc(sizeof(gMainWindow*));
	else
		windows=(gMainWindow**)g_realloc(windows,nwindows*sizeof(gMainWindow*));
	windows[nwindows-1]=this;
	
	if (plug)
		border=gtk_plug_new(plug);
	else
		border=gtk_window_new(GTK_WINDOW_TOPLEVEL);
	
	accel=gtk_accel_group_new();
	gtk_window_add_accel_group(GTK_WINDOW(border), accel);
	
	widget=gtk_layout_new(0,0);
	gtk_container_add(GTK_CONTAINER(border),widget);
	
	gtk_widget_realize(border);
	gtk_widget_show(widget);

	gtk_widget_set_size_request(border,1,1);
	resize(200,150);
	setFont(gDesktop::font());
	initSignals();
	onOpen=NULL;
	onShow=NULL;
	onHide=NULL;
	onMove=NULL;
	onResize=NULL;
	
	gtk_widget_add_events(widget,GDK_BUTTON_MOTION_MASK);
	
	g_signal_connect(G_OBJECT(widget),"map",G_CALLBACK(win_open),(gpointer)this);
	g_signal_connect(G_OBJECT(border),"show",G_CALLBACK(win_show),(gpointer)this);
	g_signal_connect(G_OBJECT(border),"hide",G_CALLBACK(win_hide),(gpointer)this);
	g_signal_connect(G_OBJECT(border),"configure-event",G_CALLBACK(win_motion),(gpointer)this);
	g_signal_connect(G_OBJECT(border),"delete-event",G_CALLBACK(win_close),(gpointer)this);
	g_signal_connect(G_OBJECT(border),"window-state-event",G_CALLBACK(win_frame),(gpointer)this);


}

gMainWindow::gMainWindow(gControl *parent)
{

	initAll(parent);
	
	sticky=false;
	stack=0;
	buf_mask=NULL;
	m_mask=false;
	resize_flag=false;
	accel=NULL;
	esc_button=NULL;
	menuBar=NULL;
	win_style=NULL;
	g_typ=Type_gEmbedWindow;
	top_only=false;
	
	border=gtk_event_box_new();
	widget=gtk_layout_new(0,0);
	gtk_container_add(GTK_CONTAINER(border),widget);
	gtk_widget_show_all(border);
	pr=parent;
	connectParent();
	initSignals();
	onOpen=NULL;
	onShow=NULL;
	onHide=NULL;
	onMove=NULL;
	onResize=NULL;


}

gMainWindow::~gMainWindow()
{

	long bucle,b2;
	
	if (pr) return;
	
	if (buf_mask) g_object_unref(G_OBJECT(buf_mask));
	
	for(bucle=0;bucle<nwindows;bucle++)
		if (windows[bucle]==this)
		{
			if (nwindows==1)
			{
				g_free(windows);
				windows=NULL;
				nwindows=0;
			}
			else
			{
				for(b2=bucle;b2<(nwindows-1);b2++)
					windows[b2]=windows[b2+1];
				nwindows--;
				windows=(gMainWindow**)g_realloc(windows,nwindows*sizeof(gMainWindow*));
				
			}
		}


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
	if (pr) return;

	sticky=vl;
	if (vl) gtk_window_stick(GTK_WINDOW(border));
	else    gtk_window_unstick(GTK_WINDOW(border));
}

void gMainWindow::setStacking(int vl)
{
	if (pr) return;

	switch (vl)
	{
		case 0:
			stack=vl;
			gtk_window_set_keep_below(GTK_WINDOW(border),FALSE);
			gtk_window_set_keep_above(GTK_WINDOW(border),FALSE);
			break;
		case 1:
			stack=vl;
			gtk_window_set_keep_below(GTK_WINDOW(border),FALSE);
			gtk_window_set_keep_above(GTK_WINDOW(border),TRUE);
			break;
		case 2:
			stack=vl;
			gtk_window_set_keep_above(GTK_WINDOW(border),FALSE);
			gtk_window_set_keep_below(GTK_WINDOW(border),TRUE);
			break;
	}
}

void gMainWindow::setBackGround(long color)
{
	set_gdk_bg_color(widget,color);
	if (!border->window) gtk_widget_realize(border);
	gdk_window_process_updates(border->window,true);
}

long gMainWindow::backGround()
{
	return get_gdk_bg_color(widget);
}

void gMainWindow::move(long x,long y)
{
	GtkLayout *fx;
	gint root_x,root_y;

	if ( (x==this->bufX) && (y==this->bufY) ) return;
	
	if (pr)
	{
		gControl::move(x,y);
		return;
	}

	gtk_window_move(GTK_WINDOW(border),x,y);
	
	
}


void gMainWindow::show()
{
	if (pr) gtk_widget_show(border);
	else 	gtk_window_present(GTK_WINDOW(border));

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
	long myx,myy;
	
	if (pr) return;
	
	myx=(gDesktop::width()/2)-(width()/2);
	myy=(gDesktop::height()/2)-(height()/2);
	
	move(myx,myy);
}

bool gMainWindow::modal()
{
	if (pr) return false;

	return gtk_window_get_modal (GTK_WINDOW(border));
}

void gMainWindow::showModal()
{
	if (pr) return;
	
	gtk_window_set_modal(GTK_WINDOW(border),true);
	gtk_widget_show(border);
}

void gMainWindow::raise()
{
	if (pr) { gControl::raise(); return; }
	gtk_window_present(GTK_WINDOW(border));
}

const char* gMainWindow::text()
{
	if (pr) return NULL;

	return gtk_window_get_title(GTK_WINDOW(border));
}

bool gMainWindow::skipTaskBar()
{
	if (pr) return false;
	
	return gtk_window_get_skip_taskbar_hint (GTK_WINDOW(border));
}


void gMainWindow::setText(const char *txt)
{
	if (pr) return;

	gtk_window_set_title(GTK_WINDOW(border),txt);
}

int gMainWindow::_border()
{
	if (pr) return 0;
	
	if (!gtk_window_get_decorated(GTK_WINDOW(border))) return 0;
	if (!gtk_window_get_resizable(GTK_WINDOW(border))) return 1;
	return 2;
}

void gMainWindow::setBorder(int b)
{

	 if (pr) return;
	 
	switch (b)
	{
		case 0: // None
			gtk_window_set_decorated(GTK_WINDOW(border),false);
			break;
		case 1: // Fixed
			gtk_window_set_decorated(GTK_WINDOW(border),true);
			if (gtk_window_get_resizable(GTK_WINDOW(border)))
			{
				gtk_window_set_resizable(GTK_WINDOW(border),false);
				gtk_widget_set_size_request(border,bufW,bufH);
			}
			break;
		case 2: // Resizable
			gtk_window_set_decorated(GTK_WINDOW(border),true);
			if (!gtk_window_get_resizable(GTK_WINDOW(border)))
			{
				gtk_window_set_resizable(GTK_WINDOW(border),true);
				gtk_widget_set_size_request(border,1,1);
			}
			break;
	}
}

void gMainWindow::setSkipTaskBar(bool b)
{
	if (pr) return;

	gtk_window_set_skip_taskbar_hint (GTK_WINDOW(border),b);
}


gPicture* gMainWindow::icon()
{
	GdkPixbuf *buf;
	gPicture *pic;
	
	if (pr) return NULL;
	
	buf=gtk_window_get_icon(GTK_WINDOW(border));
	if (!buf) return NULL;
	
	pic=gPicture::fromPixbuf(buf);
	
	return pic;
}

void gMainWindow::setIcon(gPicture *pic)
{
	GdkPixbuf *buf;
	
	if (pr) return;

	if (!pic) { gtk_window_set_icon(GTK_WINDOW(border),NULL); return; }
	
	buf=pic->getPixbuf();
	gtk_window_set_icon(GTK_WINDOW(border),buf);
	if (buf) g_object_unref(G_OBJECT(buf));
	
}

bool gMainWindow::topOnly()
{
	if (pr) return false;
	
	return top_only;
}

void gMainWindow::setTopOnly(bool vl)
{
	if (pr) return;

	gtk_window_set_keep_above (GTK_WINDOW(border),vl);
	top_only=vl;
}


bool gMainWindow::mask()
{
	return m_mask;
}

gPicture* gMainWindow::picture()
{
	return gPicture::fromPixbuf(buf_mask);
}

void gMainWindow::setMask(bool vl)
{
	m_mask=vl;
	drawMask();
}

void gMainWindow::setPicture(gPicture *pic)
{
	if (buf_mask) g_object_unref(G_OBJECT(buf_mask));
	buf_mask=NULL;
	if (pic)
		if (pic->pic)
			buf_mask=pic->getPixbuf();
			
	drawMask();
	
}

void gMainWindow::drawMask()
{
	GdkBitmap *map=NULL;
	GdkPixmap *pix=NULL;
	GdkColor black;
	gint pw,ph;
	
	
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
	
	if (buf_mask && m_mask) gtk_widget_shape_combine_mask(border,map,0,0);
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
		gtk_widget_reset_shapes(border);
		gdk_window_invalidate_rect(border->window,NULL,true);
	}
	
}

long gMainWindow::menuCount()
{
	if (!menuBar) return 0;
	return gMenu::winChildCount(this);
}



/*************************************************************************

gKey

**************************************************************************/
bool gKey_Flags=false;
guint gKey_Key=0;
guint gKey_State=0;


bool gKey::valid()
{
	return gKey_Flags;
}

char* gKey::text()
{
	if (!gKey_Flags) return 0;
	return gdk_keyval_name(gKey_Key);
	
}

int gKey::code()
{
	return gKey_Key;
}

int gKey::state()
{
	return gKey_State;
}

bool gKey::alt()
{
	return gKey_State & GDK_MOD1_MASK;
}

bool gKey::control()
{
	return gKey_State & GDK_CONTROL_MASK;
}

bool gKey::meta()
{
	return gKey_State & GDK_MOD2_MASK;
}

bool gKey::normal()
{
	guint myval=gKey_State & 0xFF;
	if (myval) return false;
	return true;
}

bool gKey::shift()
{
	return gKey_State & GDK_SHIFT_MASK;
}

int gKey::fromString(char* str)
{
	long bucle;
	
	if (!str) return 0;
	for (bucle=0;bucle<strlen(str);bucle++)
		str[bucle]=tolower(str[bucle]);
	return gdk_keyval_from_name (str);
}

void gKey::disable()
{
	gKey_Flags=false;
	gKey_Key=0;
	gKey_State=0;
}

void gKey::enable(GdkEventKey *e)
{
	gKey_Flags=true;
	gKey_Key=e->keyval;
	gKey_State=e->state;
	
}

/********************************************************************

gApplication

*********************************************************************/

long appEvents;
GtkTooltips *app_tooltips_handle;
bool app_tooltips=true;
gFont *app_tooltips_font=NULL;

GtkTooltips* gApplication::tipHandle()
{
	return app_tooltips_handle;
}

bool gApplication::toolTips()
{
	return app_tooltips;
}

void gApplication::setToolTipsFont(gFont *ft)
{
	GList *chd;
	
	PangoFontDescription *desc=pango_context_get_font_description(ft->ct);

	app_tooltips_font->unref();
	app_tooltips_font=ft;
	app_tooltips_font->ref();
	gtk_widget_modify_font(app_tooltips_handle->tip_window,desc);
	chd=gtk_container_get_children(GTK_CONTAINER(app_tooltips_handle->tip_window));
	if (chd)
	{
		do { gtk_widget_modify_font(GTK_WIDGET(chd->data),desc);
		} while (chd->next);
		g_list_free(chd);
	}
}

gFont *gApplication::toolTipsFont()
{
	return app_tooltips_font;
}

void gApplication::enableTooltips(bool vl)
{
	gControl *iter;
	long bucle;
	
	if (vl)
		gtk_tooltips_enable(app_tooltips_handle);
	else
		gtk_tooltips_disable(app_tooltips_handle);
}

void gApplication::suspendEvents(bool vl)
{
	if (!vl) appEvents=3; //all
	else appEvents=1;     //user
}

void gApplication::enableEvents()
{
	appEvents=0;
}

bool gApplication::userEvents()
{
	if (appEvents) return false;
	return true;
}

bool gApplication::allEvents()
{
	if (appEvents & 2) return false;
	return true;
}

void gApplication::init(int *argc,char ***argv)
{
	GtkTooltips* tip;
	
	appEvents=0;
	
	nwindows=0;
	windows=NULL;
	
	gtk_init(argc,argv);
	
	app_tooltips_handle=gtk_tooltips_new();
	g_object_ref(G_OBJECT(app_tooltips_handle));
	gtk_tooltips_force_window (app_tooltips_handle);
	app_tooltips_font=new gFont(app_tooltips_handle->tip_window);
	
	clipBoard_Init();
}

void gApplication::exit()
{
	if (app_tooltips_font) delete app_tooltips_font;
}

void gApplication::iteration()
{
	struct timespec mywait;
	
	if (gtk_events_pending ())
	{
	  gtk_main_iteration_do (false);
	}
	else
	{
		mywait.tv_sec=0;
		mywait.tv_nsec=100000;
		nanosleep(&mywait,NULL);
	}
}

long gApplication::controlCount()
{
	GList *iter;
	long ct=1;
	
	if (!gControl::controlList()) return 0;
	
	iter=g_list_first(gControl::controlList());
	while (iter->next)
	{
		ct++;
		iter=iter->next;
	}
	
	return ct;
}

long gApplication::winCount()
{
	return nwindows;
}

gControl* gApplication::controlItem(GtkWidget *wid)
{
	GList *iter;
	long bucle;
	
	if (!wid) return NULL;

	if (gControl::controlList())
	{
		iter=g_list_first(gControl::controlList());
		
		while (iter)
		{
			if (((gControl*)iter->data)->border == wid )
				return (gControl*)iter->data;
				
			if (((gControl*)iter->data)->widget == wid )
				return (gControl*)iter->data;
		
			iter=iter->next;
		}
		
	}
	
	return NULL;
}

gControl* gApplication::controlItem(long index)
{
	GList *iter;
	
	if (!gControl::controlList()) return NULL;
	iter=g_list_nth(gControl::controlList(),index);
	if (!iter) return NULL;
	return (gControl*)iter->data;
}

gMainWindow* gApplication::winItem(long index)
{
	if ( (index<0) || (index>=nwindows) ) return NULL;
	return windows[index];
}
