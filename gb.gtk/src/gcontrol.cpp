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
#include <stdio.h>

#include <gtk/gtk.h>
#ifdef GDK_WINDOWING_X11
#include <gdk/gdkx.h>
#endif
#include <stdlib.h>
#include <string.h>

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/keysymdef.h>
#include <X11/X.h>

#include "widgets.h"
#include "widgets_private.h"

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

   

/*****************************************************************

CREATION AND DESTRUCTION

******************************************************************/
void gControl::cleanRemovedControls()
{
	GList *iter;

	if (!controls_destroyed) return;

	iter=g_list_first(controls_destroyed);

	while (iter)
	{
		gtk_widget_destroy(((gControl*)iter->data)->border);
		iter=g_list_next(iter);
	}

	g_list_free(controls_destroyed);
	controls_destroyed=NULL;
}


void gControl::initAll(gControl* parent)
{
	bufW=1;
	bufH=1;
	bufX=0;
	bufY=0;
	curs=NULL;
	g_typ=0;
	dsg=false;
	expa=false;
	font_change=false;
	drops=0;
	mous=-1;
	pr=parent;

	controls=g_list_append(controls,this);
}

gControl::gControl()
{

	initAll(NULL);
}

gControl::gControl(gControl* parent) 
{	

	initAll(parent);
}

gControl::~gControl()
{
	GList *iter;
	
	long bucle,b2;
	gControl *par=this->pr;


	controls=g_list_remove(controls,this);
	if (par) ((gContainer*)par)->ch_list=g_list_remove( ((gContainer*)par)->ch_list,this);
	
	if (curs) { delete curs; curs=NULL; }
	
	if (getClass()==Type_gDrawingArea)
	{
		((gDrawingArea*)this)->setCached(false);
	}
	
	if (getClass()==Type_gMovieBox)
	{
		
		if (  ((gMovieBox*)this)->playing() ) ((gMovieBox*)this)->setPlaying(false);
		if (  ((gMovieBox*)this)->animation ) g_object_unref(G_OBJECT(((gMovieBox*)this)->animation));
	}
	
	if (getClass()==Type_gButton)
	{	
		if (((gButton*)this)->rendpix) g_object_unref(((gButton*)this)->rendpix);
		if (((gButton*)this)->rendinc) g_object_unref(((gButton*)this)->rendinc);
		if (((gButton*)this)->bufText) g_free(((gButton*)this)->bufText);
	}
	
	if(getClass()==Type_gPictureBox)
	{
		if ( ((gPictureBox*)this)->pix) {
			g_object_unref( G_OBJECT(((gPictureBox*)this)->pix) );
			((gPictureBox*)this)->pix=NULL; 
		}
	}
}


gboolean gPlugin_OnUnplug(GtkSocket *socket,gPlugin *data)
{
	if (data->onUnplug) data->onUnplug(data);
	return true;
}

void gPlugin_OnPlug(GtkSocket *socket,gPlugin *data)
{
	if (data->onPlug) data->onPlug(data);
}


gPlugin::gPlugin(gControl *parent) : gControl(parent)
{
	GtkWidget *fr;
	
	g_typ=Type_gPlugin;
	
	border=gtk_event_box_new();
	fr=gtk_frame_new("");
	gtk_frame_set_label_widget(GTK_FRAME(fr),NULL);
	widget=gtk_socket_new();
	gtk_container_add (GTK_CONTAINER(fr),widget);
	gtk_container_add (GTK_CONTAINER(border),fr);	
	connectParent();
	initSignals();
	onPlug=NULL;
	onUnplug=NULL;
	g_signal_connect(G_OBJECT(widget),"plug-removed",G_CALLBACK(gPlugin_OnUnplug),(gpointer)this);
	g_signal_connect(G_OBJECT(widget),"plug-added",G_CALLBACK(gPlugin_OnPlug),(gpointer)this);

}

long gPlugin::client()
{
	GdkNativeWindow win=gtk_socket_get_id(GTK_SOCKET(widget));
	return (long)win;
	
}

long gPlugin::handle()
{
	GdkNativeWindow win=gtk_socket_get_id(GTK_SOCKET(widget));
	return (long)win;
}

void gPlugin::plug(long id,bool prepared)
{
	if (!prepared) gtk_socket_steal(GTK_SOCKET(widget),(GdkNativeWindow)id);
	gtk_socket_add_id(GTK_SOCKET(widget),(GdkNativeWindow)id);
}

void gPlugin::discard()
{
	
	#ifdef GDK_WINDOWING_X11
	GtkWidget *fr;
	long color;
	
	GdkNativeWindow win=gtk_socket_get_id(GTK_SOCKET(widget));
	GdkDisplay* dsp=gdk_display_get_default();
	
	if (!win) return;
	
	XReparentWindow(GDK_DISPLAY_XDISPLAY(dsp),win,GDK_ROOT_WINDOW(), 0, 0);
	
	color=foreGround();
	gtk_widget_destroy(widget);
	
	widget=gtk_socket_new();
	fr=gtk_bin_get_child(GTK_BIN(border));
	gtk_container_add(GTK_CONTAINER(fr),widget);
	gtk_widget_show(widget);
	setForeGround(color);
	#else
	stub("no-X11/gPlugin:discard()");
	#endif
	
	
}

long gPlugin::backGround()
{
	return get_gdk_bg_color(border);
}

void gPlugin::setBackGround(long color)
{
	set_gdk_bg_color(widget,color);	
	if (!border->window) gtk_widget_realize(border);
	gdk_window_process_updates(border->window,true);
}

long gPlugin::foreGround()
{
	return get_gdk_fg_color(widget);
}

void gPlugin::setForeGround(long color)
{	
	set_gdk_fg_color(widget,color);
	if (!border->window) gtk_widget_realize(border);
	gdk_window_process_updates(border->window,true);
}

int gPlugin::getBorder()
{
	return Frame_getBorder(GTK_FRAME(widget->parent));
}

void gPlugin::setBorder(int vl)
{
	Frame_setBorder(GTK_FRAME(widget->parent),vl);
}

void gControl::destroy()
{
	gtk_widget_hide(border);
	controls_destroyed=g_list_prepend(controls_destroyed,(gpointer)this);
	//gtk_widget_destroy(border);
}

int gControl::getClass()
{
	return g_typ;
}


gControl* gControl::next()
{
	stub("gControl::next");
	return NULL;
}



gControl* gControl::previous()
{
	stub("gControl::previous");
	return NULL;
}


/*****************************************************************

VISIBILITY

******************************************************************/
bool gControl::enabled()
{
	return GTK_WIDGET_SENSITIVE(border);
}

bool gControl::visible()
{
	return GTK_WIDGET_VISIBLE(border);
}

void gControl::setEnabled(bool vl)
{
	gtk_widget_set_sensitive (border,vl);	
}

void gControl::setVisible(bool vl)
{	
	if (vl!=visible())
	{ 
		g_object_set(G_OBJECT(border),"visible",vl,NULL);
		if (pr &&  vl) ((gContainer*)pr)->performArrange();
	}
}

void gControl::show()
{
	gtk_widget_show(border);
	if (pr) ((gContainer*)pr)->performArrange();
}

void gControl::hide()
{
	gtk_widget_hide(border);
}

/*****************************************************************

POSITION AND SIZE

******************************************************************/
long gControl::screenX()
{
	gint x,y;
	
	gdk_window_get_root_origin(border->window,&x,&y);
	return x;
}

long gControl::screenY()
{
	gint x,y;
	
	gdk_window_get_root_origin(border->window,&x,&y);
	return y;
}

long gControl::left()
{
	return bufX;
}

long gControl::top()
{	
	return bufY;
}

long gControl::width()
{
	return bufW;
}

long gControl::height()
{
	return bufH;
}

void gControl::setLeft(long l)
{
	move(l,top());
}

void gControl::setTop(long t)
{
	move(left(),t);
}

void gControl::setWidth(long w)
{
	resize(w,height());
}

void gControl::setHeight(long h)
{
	resize(width(),h);
}

void gControl::moveResize(long x,long y,long w,long h)
{
	move(x,y);
	resize(w,h);
}

void gControl::move(long x,long y)
{
	GtkLayout *fx;
	
	if (parent()->getClass()==Type_gSplitter) return;

	if ( (x==this->bufX) && (y==this->bufY) ) return;
	
	this->bufX=x;
	this->bufY=y;
	
	fx=GTK_LAYOUT(pr->widget);
	gtk_layout_move(fx,border,x,y);
	((gContainer*)parent())->performArrange();
}

void gControl::resize(long w,long h)
{
	if (w<1) w=1;
	if (h<1) h=1;	
	
	if ( (this->bufW==w) && (this->bufH==h) ) return;

	if (!pr)
	{
		((gMainWindow*)this)->resize_flag=true;
		((gMainWindow*)this)->bufW=w;
		((gMainWindow*)this)->bufH=h;
		
		
		
		gdk_window_enable_synchronized_configure (border->window);
		
		if (gtk_window_get_resizable(GTK_WINDOW(border)))
			gtk_window_resize(GTK_WINDOW(border),w,h);
		else
			gtk_widget_set_size_request(border,w,h);
		return;
	}
	
	this->bufW=w;
	this->bufH=h;
	
	if ( getClass()==Type_gSlider ) ((gSlider*)this)->orientation(w,h);
	if ( getClass()==Type_gScrollBar ) ((gScrollBar*)this)->orientation(w,h);
		
	gtk_widget_set_size_request(border,w,h);
	
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
	// TODO: update when changing
	expa=vl;
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
	return new gFont(widget);
}


void gControl::setFont(gFont *ft)
{
	PangoFontDescription *desc=pango_context_get_font_description(ft->ct);
	
	font_change=true;
	gtk_widget_modify_font(widget,desc);
}

long gControl::backGround()
{
	GtkStyle* st;
	GdkColor gcol;
	
	
	st=gtk_widget_get_style(border);
	gcol=st->bg[GTK_STATE_NORMAL];
	
	return get_gdk_color(&gcol);
	
}

long gControl::foreGround()
{
	GtkStyle* st;
	GdkColor gcol;
	
	
	st=gtk_widget_get_style(widget);
	gcol=st->fg[GTK_STATE_NORMAL];
	
	return get_gdk_color(&gcol);
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

void gControl::setBackGround(long color)
{
	GdkColor gcol;
	GtkRcStyle* st;

	st = gtk_rc_style_new ();
	
	fill_gdk_color(&gcol,color);
	
	st->color_flags[GTK_STATE_NORMAL] = GTK_RC_BG;
	st->bg[GTK_STATE_NORMAL]=gcol;
	
	
	
	gtk_widget_modify_bg (border,GTK_STATE_NORMAL,&gcol);
	if (getClass()==Type_gSlider) gtk_widget_modify_bg(widget,GTK_STATE_NORMAL,&gcol);
	if (getClass()==Type_gScrollBar) gtk_widget_modify_bg(widget,GTK_STATE_NORMAL,&gcol);
	if (!border->window) gtk_widget_realize(border);
	gdk_window_process_updates(border->window,true);
	
	g_object_unref(G_OBJECT(st));

	if (getClass()==Type_gDrawingArea)
	{
		if ( ((gDrawingArea*)this)->cached())
			((gDrawingArea*)this)->clear();
	}
}

void gControl::setForeGround(long color)
{
	GdkColor gcol;
	GtkRcStyle *st;
	
	st = gtk_rc_style_new ();
	fill_gdk_color(&gcol,color);
	
	st->color_flags[GTK_STATE_NORMAL] = GTK_RC_FG;
	st->fg[GTK_STATE_NORMAL]=gcol;
			
	gtk_widget_modify_style (widget,st);
		
	g_object_unref(G_OBJECT(st));

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

void gControl::setMouse(int m)
{
	GdkCursor *cr=NULL;
	GdkPixmap *pix;
	GdkColor col={0,0,0,0};
	
	mous=m;
	if (!border->window) return;
	
	if (m==-2)
	{
		if (!curs)
		{
			mous=-1;
			gdk_window_set_cursor(border->window,NULL);
		}
		if (!curs->cur)
		{
			mous=-1;
			gdk_window_set_cursor(border->window,NULL);
		}
		gdk_window_set_cursor(border->window,curs->cur);
		return;	
	}
	
	if (m!=-1) 
	{
		if (m<GDK_LAST_CURSOR)
		{
			cr=gdk_cursor_new((GdkCursorType)m);
		}
		else
		{
			if (m==(GDK_LAST_CURSOR+1)) //FDiag
			{
				pix=gdk_bitmap_create_from_data(NULL,_cursor_fdiag,16,16);
				cr=gdk_cursor_new_from_pixmap(pix,pix,&col,&col,0,0);
				g_object_unref(pix);
			}
			else if (m==(GDK_LAST_CURSOR+2)) //BDiag
			{
				pix=gdk_bitmap_create_from_data(NULL,_cursor_bdiag,16,16);
				cr=gdk_cursor_new_from_pixmap(pix,pix,&col,&col,0,0);
				g_object_unref(pix);
			}
			else if (m==(GDK_LAST_CURSOR+3)) //SplitH
			{
				pix=gdk_bitmap_create_from_data(NULL,_cursor_splith,16,16);
				cr=gdk_cursor_new_from_pixmap(pix,pix,&col,&col,0,0);
				g_object_unref(pix);
			}
			else if (m==(GDK_LAST_CURSOR+4)) //SplitV
			{
				pix=gdk_bitmap_create_from_data(NULL,_cursor_splitv,16,16);
				cr=gdk_cursor_new_from_pixmap(pix,pix,&col,&col,0,0);
				g_object_unref(pix);
			}
		}
	}
	
	
	gdk_window_set_cursor(border->window,cr);
}



/*****************************************************************

HANDLES

******************************************************************/
void gControl::reparent(gContainer *newpr)
{
	gContainer *oldpr;
	
	if (!newpr->widget) return;
	oldpr=(gContainer*)pr;
	gtk_widget_reparent(border,newpr->widget);
	pr=newpr;
	
	((gContainer*)oldpr)->ch_list=g_list_remove(((gContainer*)oldpr)->ch_list,this);
	((gContainer*)newpr)->ch_list=g_list_remove(((gContainer*)newpr)->ch_list,this);
	
	oldpr->performArrange();
	newpr->performArrange();
}

gControl* gControl::parent()
{
	return pr;
}
gControl* gControl::window()
{
	return gApplication::controlItem(gtk_widget_get_toplevel(border));
}
long gControl::handle()
{
	#ifdef GDK_WINDOWING_X11
	return (long)GDK_WINDOW_XID(border->window);
	#else
	stub("no-X11/gControl::handle()");
	return 0;
	#endif
}
/*****************************************************************

MISC

******************************************************************/


void gControl::refresh(long x,long y,long w,long h)
{
	GdkRectangle rect;
	GdkWindow *win=border->window;

	if (!win) return;

	if ( (x<0) || (y<0) || (w<0) || (h<0) )
	{
		rect.x=0;
		rect.y=0;
		rect.width=width();
		rect.height=height();
	}
	else
	{
		rect.x=x;
		rect.y=y;
		rect.width=w;
		rect.height=h;
	}

	if (getClass()==Type_gDrawingArea)
	{
		win=((GtkLayout*)border)->bin_window;
		((gDrawingArea*)this)->berase=true;
	}

	gdk_window_invalidate_rect(win,&rect,true);
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
	gtk_widget_grab_focus(widget);
}

void gControl::lower()
{
	GList *iter;
	GList *chd;
	GtkWidget *child;
	gControl *Br;
	long x,y,bucle;

	if (!pr) return;
	if (parent()->getClass()==Type_gSplitter) return;
	
	if (!(chd=gtk_container_get_children(GTK_CONTAINER(pr->widget)))) return;
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
			gtk_container_remove(GTK_CONTAINER(pr->widget),Br->border);
			gtk_layout_put(GTK_LAYOUT(pr->widget),Br->border,x,y);
			g_object_unref(G_OBJECT(Br->border));
		}
		
		chd=g_list_next(chd);
	}
}

void gControl::raise()
{
	long x,y;

	if (!pr) return;
	if (parent()->getClass()==Type_gSplitter) return;
	
	x=left();
	y=top();
	g_object_ref(G_OBJECT(border));
	gtk_container_remove(GTK_CONTAINER(pr->widget),border);
	gtk_layout_put(GTK_LAYOUT(pr->widget),border,x,y);
	g_object_unref(G_OBJECT(border));
}

gPicture* gControl::grab()
{
	return Grab_gdkWindow(border->window);
}

/*********************************************************************

Drag & Drop

**********************************************************************/
char* drag_text_buffer=NULL;
GdkPixbuf* drag_picture_buffer=NULL;
gControl *drag_widget=NULL;

gControl* gControl::dragWidget()
{
	return drag_widget;
}

void gControl::setDragWidget(gControl *ct)
{
	drag_widget=ct;
}

char* gControl::dragTextBuffer()
{
	return drag_text_buffer;
}

GdkPixbuf *gControl::dragPictureBuffer()
{
	return drag_picture_buffer;
}

void gControl::freeDragBuffer()
{
	if (drag_text_buffer)
	{
		g_free(drag_text_buffer);
		drag_text_buffer=NULL;
	}
	
	if (drag_picture_buffer)
	{
		g_object_unref(G_OBJECT(drag_picture_buffer));
		drag_picture_buffer=NULL;
	}
}

void gControl::dragText(char *txt)
{
	GtkTargetList *list;
	GdkDragContext *ct;
	
	gControl::freeDragBuffer();
	
	if (txt)
	{
		drag_text_buffer=(char*)g_malloc( sizeof(char)*(strlen(txt)+1) );
		if (drag_text_buffer) strcpy(drag_text_buffer,txt);
	}
	
	list=gtk_target_list_new (0,NULL);

	
	gtk_target_list_add_text_targets (list, 0);
	
	//gtk_target_list_add (list,gdk_atom_intern("UTF8_STRING",false),0,0);
	//gtk_target_list_add (list,gdk_atom_intern("COMPOUND_TEXT",false),0,0);
	//gtk_target_list_add (list,gdk_atom_intern("TEXT",false),0,0);
	//gtk_target_list_add (list,GDK_TARGET_STRING,0,0);
	//gtk_target_list_add (list,gdk_atom_intern("text/plain;charset=utf-8",false),0,0);
	//gtk_target_list_add (list,gdk_atom_intern("text/plain",false),0,0);
	

	ct=gtk_drag_begin(border,list,GDK_ACTION_COPY,1,NULL);
	gtk_target_list_unref (list);
	
	if (gDrag_getIcon()) gtk_drag_set_icon_pixbuf(ct,gDrag_getIcon(),0,0);
}

void gControl::dragImage(gImage *img)
{
	GtkTargetList *list;
	GdkPixbuf *buf;
	GdkDragContext *ct;
	
	gControl::freeDragBuffer();
	if (img) {
		drag_picture_buffer=img->image;
		if (drag_picture_buffer) g_object_ref(drag_picture_buffer);
	}
	
	list=gtk_target_list_new (NULL,0);
	
	gtk_target_list_add (list,gdk_atom_intern("image/png",false),0,0);
	gtk_target_list_add (list,gdk_atom_intern("image/jpg",false),0,0);
	gtk_target_list_add (list,gdk_atom_intern("image/jpeg",false),0,0);
	gtk_target_list_add (list,gdk_atom_intern("image/gif",false),0,0);
	
	ct=gtk_drag_begin(border,list,GDK_ACTION_COPY,1,NULL);
	gtk_target_list_unref (list);
	
	if (gDrag_getIcon()) gtk_drag_set_icon_pixbuf(ct,gDrag_getIcon(),0,0);
                                            
}


bool gControl::acceptDrops()
{
	return drops & 1;
}

void gControl::setAcceptDrops(bool vl)
{
	GtkWidget *w;
	GtkTargetEntry entry[7];
	

	if (!pr) w=border;
	else w=widget;
	
	w=border;
	
	if (vl && !(drops & 1))
	{	
		drops = 1;

		entry[0].target="text/plain";
		entry[0].flags=0;
		entry[0].info=0;
		
		entry[1].target="TEXT";
		entry[1].flags=0;
		entry[1].info=0;
		
		gtk_drag_dest_set(border,(GtkDestDefaults)0,0, \
							0, \
							(GdkDragAction)(GDK_ACTION_COPY | GDK_ACTION_MOVE | GDK_ACTION_LINK));
		
	}
	
	if (!vl && (drops & 1))
	{
		drops=0;
		gtk_drag_dest_unset(border);
	}
}

/*********************************************************************

Internal

**********************************************************************/
void gControl::connectParent()
{
	GtkLayout *fx;
	
	gtk_widget_set_redraw_on_allocate(border,false);
	
	if (parent()->getClass()==Type_gSplitter)
	{
		((gSplitter*)parent())->addWidget(border);
	}
	else
	{
		fx=GTK_LAYOUT(parent()->widget);
		gtk_layout_put(fx,border,0,0);
	}
	
	gtk_widget_show_all(border);
	((gContainer*)parent())->performArrange();

	
	if (pr)
	{
		((gContainer*)pr)->ch_list=g_list_append(((gContainer*)pr)->ch_list,this);
		
		set_gdk_bg_color(widget,pr->backGround());
		set_gdk_fg_color(widget,pr->foreGround());
		if (border!=widget)
		{
			set_gdk_bg_color(border,pr->backGround());
			set_gdk_fg_color(border,pr->foreGround());
		}
		setFont(pr->font());
		if (!border->window) gtk_widget_realize(border);
		gdk_window_process_updates(border->window,true);
	}
}

GList* gControl::controlList()
{
	return controls; 
}
