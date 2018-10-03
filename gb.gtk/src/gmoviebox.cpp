/***************************************************************************

  gmovie.cpp

  (c) 2004-2006 Daniel Campos Fernández <dcamposf@gmail.com>
  (c) 2018 Benoît Minisini <g4mba5@gmail.com>

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

#include <gdk-pixbuf/gdk-pixbuf.h>
#include "widgets.h"
#include "gmoviebox.h"

/****************************************************************************************

gMovieBox

*****************************************************************************************/

gMovieBox::gMovieBox(gContainer *parent) : gControl(parent)
{	
	g_typ=Type_gMovieBox;
	
	timeout=0;	
	pl=false;
	animation=NULL;
	
	border = gtk_alignment_new(0,0,1,1);
	widget = gtk_image_new();
	realize(true);

	setAlignment(ALIGN_TOP_LEFT);
}

gMovieBox::~gMovieBox()
{
  if (playing()) 
    setPlaying(false);
  if (animation) 
    g_object_unref(G_OBJECT(animation));  
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
	int interval;
	
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

bool gMovieBox::loadMovie(char *buf, int len)
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

int gMovieBox::alignment()
{
	gfloat x, y;
	
	gtk_misc_get_alignment(GTK_MISC(widget), &x, &y);
	return gt_to_alignment(x, y);
}

void gMovieBox::setAlignment(int al)
{
	gtk_misc_set_alignment(GTK_MISC(widget), gt_from_alignment(al, false), gt_from_alignment(al, true));
}

gColor gMovieBox::getFrameColor()
{
	return realForeground();
}


