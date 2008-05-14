/***************************************************************************

  gpicturebox.cpp

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

#include <gdk-pixbuf/gdk-pixbuf.h>
#include "widgets.h"
#include "widgets_private.h"
#include "gmoviebox.h"
#include "gpicturebox.h"

/****************************************************************************************

gMovieBox

*****************************************************************************************/
gMovieBox::gMovieBox(gContainer *parent) : gControl(parent)
{	
	g_typ=Type_gMovieBox;
	
	timeout=0;	
	pl=false;
	animation=NULL;
	
	border = gtk_event_box_new();
	widget = gtk_image_new();
	realize(true);
}

gMovieBox::~gMovieBox()
{
  if (playing()) 
    setPlaying(false);
  if (animation) 
    g_object_unref(G_OBJECT(animation));  
}

long gMovieBox::background()
{
	return get_gdk_bg_color(border);
}

void gMovieBox::setBackground(long color)
{
	set_gdk_bg_color(border,color);	
	if (!border->window) gtk_widget_realize(border);
	gdk_window_process_updates(border->window,true);
}

long gMovieBox::foreground()
{
	return get_gdk_fg_color(widget);
}

void gMovieBox::setForeground(long color)
{	
	set_gdk_fg_color(widget,color);
	if (!border->window) gtk_widget_realize(border);
	gdk_window_process_updates(border->window,true);
}

/*int gMovieBox::getBorder()
{
	return Frame_getBorder(GTK_FRAME(widget->parent));
}*/

bool gMovieBox::playing()
{
	return pl;
}

/*void gMovieBox::setBorder(int vl)
{
	Frame_setBorder(GTK_FRAME(widget->parent),vl);
}*/

gboolean gMovieBox_move(gMovieBox *data)
{
	GTimeVal tim;
	GdkPixbuf *buf;
	

	g_get_current_time(&tim);
	if (!gdk_pixbuf_animation_iter_advance(data->iter,&tim)) return true;
	
	buf=gdk_pixbuf_animation_iter_get_pixbuf(data->iter);
	gtk_image_set_from_pixbuf(GTK_IMAGE(data->widget),buf);
	
	return true;
}

void gMovieBox::setPlaying(bool vl)
{
	GTimeVal tim;
	GdkPixbuf *buf;
	long interval;
	
	if (vl)
	{
		if (!pl)
		{
			if (!animation) return;
			g_get_current_time(&tim);
			iter=gdk_pixbuf_animation_get_iter(animation,&tim);
			buf=gdk_pixbuf_animation_iter_get_pixbuf(iter);
			gtk_image_set_from_pixbuf(GTK_IMAGE(widget),buf);
			interval=gdk_pixbuf_animation_iter_get_delay_time(iter);
			if (interval>0) {
				timeout=g_timeout_add(interval,(GSourceFunc)gMovieBox_move,this);
				pl=true;
			}
		}
		return;
	}
	
	if (pl)
	{
		g_source_remove(timeout);
		pl=false;
	}
}

bool gMovieBox::loadMovie(char *buf,long len)
{
	GdkPixbufLoader* loader;
	bool bplay;
	
	bplay=playing();
	setPlaying(false);
	
	loader=gdk_pixbuf_loader_new();
	if (!gdk_pixbuf_loader_write(loader,(guchar*)buf,(gsize)len,NULL)){
		 g_object_unref(G_OBJECT(loader));
		 setPlaying(bplay);
		 return false;
	}
	
	gdk_pixbuf_loader_close(loader,NULL);
	
	if (animation) g_object_unref(G_OBJECT(animation));
	animation=gdk_pixbuf_loader_get_animation(loader);
	g_object_ref(G_OBJECT(animation));
	g_object_unref(G_OBJECT(loader));
	setPlaying(bplay);
	return true;
}


/****************************************************************************************

gPictureBox

*****************************************************************************************/

gPictureBox::gPictureBox(gContainer *parent) : gControl(parent)
{
	_picture = NULL;
	g_typ=Type_gPictureBox;
	
	border = gtk_event_box_new();
	widget = gtk_image_new();
	gtk_image_set_pixel_size(GTK_IMAGE(widget),0);
	realize(true);
	
	setAlignment(ALIGN_TOP_LEFT);
	_autoresize = false;
	
	// BM: The gControl signal handlers are used only there. Is it really needed ?
	//g_signal_connect(G_OBJECT(border),"button-press-event",G_CALLBACK(sg_button_Press),(gpointer)this);
	//g_signal_connect_after(G_OBJECT(border),"motion-notify-event",G_CALLBACK(sg_motion),(gpointer)this);
	//g_signal_connect(G_OBJECT(border),"button-release-event",G_CALLBACK(sg_button_Release),(gpointer)this);
	//GTK_WIDGET_SET_FLAGS(widget,GTK_CAN_FOCUS);
}

gPictureBox::~gPictureBox()
{
  setPicture(NULL);
}

long gPictureBox::background()
{
	return get_gdk_bg_color(border);
}

void gPictureBox::setBackground(long color)
{
	set_gdk_bg_color(border,color);	
	if (!border->window) gtk_widget_realize(border);
	gdk_window_process_updates(border->window,true);
}

long gPictureBox::foreground()
{
	return get_gdk_fg_color(widget);
}

void gPictureBox::setForeground(long color)
{	
	set_gdk_fg_color(widget,color);
	if (!border->window) gtk_widget_realize(border);
	gdk_window_process_updates(border->window,true);
}


void gPictureBox::setPicture(gPicture *pic)
{
  gPicture::assign(&_picture, pic);
  adjust();
	redraw();
}

/*int gPictureBox::getBorder()
{
	return Frame_getBorder(GTK_FRAME(widget->parent));
}

void gPictureBox::setBorder(int vl)
{
	Frame_setBorder(GTK_FRAME(widget->parent),vl);
}*/

int gPictureBox::alignment()
{
	gfloat x,y;
	int retval=0;
	
	gtk_misc_get_alignment (GTK_MISC(widget),&x,&y);

	if (!x) retval+=1; 
	else if (x==1) retval+=2; 
	
	if (!y) retval+=16; 
	else if (y==1) retval+=32;
	else retval+=64;
	
	
	return retval;
}

void gPictureBox::setAlignment(int al)
{
	gfloat x,y,xn;
		
	xn = gtk_widget_get_default_direction() == GTK_TEXT_DIR_RTL ? 1 : 0;
	
	switch (al)
	{
		case ALIGN_BOTTOM: 	y=1; 	x=0.5; break;
		case ALIGN_BOTTOM_LEFT: 	y=1; 	x=0; break;
		case ALIGN_BOTTOM_NORMAL: y=1; 	x=xn; break;
		case ALIGN_BOTTOM_RIGHT: 	y=1; 	x=1; break;
		case ALIGN_CENTER: 	y=0.5; 	x=0.5; break;
		case ALIGN_LEFT: 	y=0.5; 	x=0; break;
		case ALIGN_NORMAL: 	y=0.5;  x=xn; break;
		case ALIGN_RIGHT: 	y=0.5; 	x=1; break;
		case ALIGN_TOP: 		y=0; 	x=0.5; break;
		case ALIGN_TOP_LEFT: 	y=0; 	x=0; break;
		case ALIGN_TOP_NORMAL: 	y=0; 	x=xn; break;
		case ALIGN_TOP_RIGHT: 	y=0; 	x=1; break;
		default: return;
	}
	
	gtk_misc_set_alignment(GTK_MISC(widget),x,y);
}

bool gPictureBox::stretch()
{
	return (bool)gtk_image_get_pixel_size(GTK_IMAGE(widget));
}

void gPictureBox::setStretch(bool vl)
{
	if (vl)
		gtk_image_set_pixel_size(GTK_IMAGE(widget),-1);
	else
		gtk_image_set_pixel_size(GTK_IMAGE(widget),0);
	
	adjust();
	redraw();
}

void gPictureBox::resize(long w,long h)
{
	gControl::resize(w,h);
	if ( stretch() ) redraw();
}

void gPictureBox::redraw()
{
	GdkPixbuf *buf;
	
	if ( gtk_image_get_pixel_size(GTK_IMAGE(widget)) )
	{
		if (!_picture) return;
		buf = gdk_pixbuf_scale_simple(_picture->getPixbuf(), width(), height(), GDK_INTERP_NEAREST);
		gtk_image_set_from_pixbuf(GTK_IMAGE(widget), buf);
		g_object_unref(G_OBJECT(buf));
		return;
	}
	
	gtk_image_set_from_pixbuf(GTK_IMAGE(widget), _picture ? _picture->getPixbuf() : NULL);
}


void gPictureBox::setAutoResize(bool v)
{
	_autoresize = v;
	adjust();	
}

void gPictureBox::adjust()
{
	if (!_autoresize || stretch() || !_picture)
		return;
		
	resize(_picture->width() + getFrameWidth() * 2, _picture->height() + getFrameWidth() * 2);
}

void gPictureBox::updateBorder()
{
	gControl::updateBorder();
	adjust();
}
