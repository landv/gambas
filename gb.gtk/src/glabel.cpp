/***************************************************************************

  glabel.cpp

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
#include "html.h"
#include "widgets.h"
#include "widgets_private.h"
#include <string.h>
#include <stdlib.h>

int gen_prueba=0;
/*****************************************************************************

Simple Label

******************************************************************************/
gboolean label_exp(GtkWidget *wid,GdkEventExpose *e,gSimpleLabel *d)
{
	GtkStyle *sty=gtk_widget_get_style(wid);
	GdkGC *gc=gdk_gc_new(wid->window);
	GdkGC *gcmap;
	int vw,vh,lw,lh,mapw,maph;
	
	if (sty) gdk_gc_set_foreground(gc,&sty->fg[GTK_STATE_NORMAL]);
	
	if (d->markup)
		pango_layout_set_width(d->layout,d->width()*PANGO_SCALE);
	else
		pango_layout_set_width(d->layout,-1);
	
	switch (d->lay_x)
	{
		case 0: pango_layout_set_alignment(d->layout,PANGO_ALIGN_LEFT); break;
		case 1: pango_layout_set_alignment(d->layout,PANGO_ALIGN_CENTER); break;
		case 2: pango_layout_set_alignment(d->layout,PANGO_ALIGN_RIGHT); break;
		case 3: pango_layout_set_alignment(d->layout,PANGO_ALIGN_LEFT); break;
	}
	
	gdk_drawable_get_size (GDK_DRAWABLE(wid->window),&vw,&vh);
	pango_layout_get_size(d->layout,&lw,&lh);
	lw/=PANGO_SCALE;
	lh/=PANGO_SCALE;
	
	if (!d->markup)
	{
		switch (d->lay_x)
		{
			case 0: vw=0; break;
			case 1: vw=(vw/2)-(lw/2); break;
			case 2: vw=vw-lw; break;
			case 3: if (gtk_widget_get_default_direction()==GTK_TEXT_DIR_RTL ) vw=vw-lw;
					else vw=0;
					break;
		}
	}
	else
		vw=0;

	
	switch (d->lay_y)
	{
		case 0: vh=0; break;
		case 1: vh=(vh/2)-(lh/2); break;
		case 2: vh=vh-lh; break;
	}
	
	if (vh<0) vh=0;
	
	if (d->mask)
	{
		gdk_drawable_get_size(d->mask,&mapw,&maph);
        if ( (mapw != d->width()) || (maph != d->height()) )
		{
			g_object_unref(G_OBJECT(d->mask));
			d->mask=gdk_pixmap_new(NULL,d->width(),d->height(),1);
		}
		
		gcmap=gdk_gc_new(d->mask);
		gdk_draw_rectangle(d->mask,gcmap,true,0,0,d->width(),d->height());
		gdk_draw_layout(d->mask,gc,vw,vh,d->layout);
		gtk_widget_shape_combine_mask(d->border,d->mask,0,0);
		g_object_unref(G_OBJECT(gcmap));
	}
	
	gdk_draw_layout(d->border->window,gc,vw,vh,d->layout);
	g_object_unref(G_OBJECT(gc));
	return false;
}

gSimpleLabel::gSimpleLabel(gControl *parent) : gControl(parent)
{
	GtkWidget *fr;	

	textdata=NULL;
	g_typ=Type_gLabel;
	markup=false;
	autoresize=false;
	mask=NULL;
	border=gtk_event_box_new();
	widget=border;
	fr=gtk_frame_new("");
	gtk_frame_set_shadow_type(GTK_FRAME(fr),GTK_SHADOW_NONE);
	gtk_frame_set_label_widget(GTK_FRAME(fr),NULL);
	gtk_container_add (GTK_CONTAINER(border),fr);
	
	layout=gtk_widget_create_pango_layout(border,"");
		
	connectParent();
	initSignals();
	g_signal_connect_after(G_OBJECT(border),"expose-event",G_CALLBACK(label_exp),(gpointer)this);
	setAlignment(alignNormal);
}

gSimpleLabel::~gSimpleLabel()
{
	if (textdata) g_free(textdata);
	if (mask) g_object_unref(G_OBJECT(mask));
	g_object_unref(G_OBJECT(layout));
}

bool gSimpleLabel::transparent()
{
	return (bool)mask;
}

void gSimpleLabel::setAutoResize(bool vl)
{
	gint h;
	
	autoresize=vl;
	
	if (autoresize) {
		pango_layout_get_size(layout,NULL,&h);
		h/=PANGO_SCALE;
		h+=2;
		resize(width(),h);
	}
}

bool gSimpleLabel::autoResize()
{
	return autoresize;
}

void gSimpleLabel::setTransparent(bool vl)
{
	GdkRectangle rect={0,0,0,0};

	if ( (!vl) && mask )
	{
		gtk_widget_shape_combine_mask(border,NULL,0,0);
		g_object_unref(G_OBJECT(mask));
		mask=NULL;
		return;
	}
	
	if ( vl && (!mask) )
	{
		mask=gdk_pixmap_new(NULL,width(),height(),1);
		if (visible()) gtk_widget_queue_draw_area(border,0,0,width(),height());
	}
}

char *gSimpleLabel::text()
{
	return textdata;
}

void gSimpleLabel::setText(char *vl)
{
	char *bpango;

	if (textdata) { g_free(textdata); textdata=NULL; }
	
	if (vl)
	{
		textdata=(char*)g_malloc(sizeof(char)*(strlen(vl)+1));
		strcpy(textdata,vl);
	}

	if (!textdata) 
		pango_layout_set_text(layout,"",-1);
	else
	{
		if (markup)
		{
			bpango=html_string_to_pango_string(textdata);
			if (!bpango)
				pango_layout_set_text(layout,"",-1);
			else
			{
				pango_layout_set_markup(layout,bpango,-1);
				g_free(bpango);
			}
		}
		else
			pango_layout_set_text(layout,textdata,-1);
	}
		
	if (visible()) gtk_widget_queue_draw_area(border,0,0,width(),height());
}

void gSimpleLabel::enableMarkup(bool vl)
{
	if (markup != vl)
	{
		markup=vl;
		if (visible()) gtk_widget_queue_draw_area(border,0,0,width(),height());
	}
}

int gSimpleLabel::getBorder()
{
	return Frame_getBorder(GTK_FRAME(gtk_bin_get_child(GTK_BIN(border))));
}

void gSimpleLabel::setBorder(int vl)
{
	GtkFrame *fr=(GtkFrame*)gtk_bin_get_child(GTK_BIN(border));
	
	Frame_setBorder(GTK_FRAME(fr),vl);
	if (visible()) gtk_widget_queue_draw_area(border,0,0,width(),height());
}

int gSimpleLabel::alignment()
{
	return align;
}

void gSimpleLabel::setAlignment(int al)
{	
	switch (al)
	{
		case alignBottom:       lay_y=2;  lay_x=1; break;
		case alignBottomLeft:   lay_y=2;  lay_x=0; break;
		case alignBottomNormal: lay_y=2;  lay_x=3; break;
		case alignBottomRight:  lay_y=2;  lay_x=2; break;
		case alignCenter:       lay_y=1;  lay_x=1; break;
		case alignLeft:         lay_y=1;  lay_x=0; break;
		case alignNormal:       lay_y=1;  lay_x=3; break;
		case alignRight:        lay_y=1;  lay_x=2; break;
		case alignTop:          lay_y=0;  lay_x=1; break;
		case alignTopLeft:      lay_y=0;  lay_x=0; break;
		case alignTopNormal:    lay_y=0;  lay_x=3; break;
		case alignTopRight:     lay_y=0;  lay_x=2; break;
		default: return;
	}
	
	align=al;
	if (visible()) gtk_widget_queue_draw_area(border,0,0,width(),height());

}

long gSimpleLabel::backGround()
{
	return get_gdk_bg_color(border);
}

void gSimpleLabel::setBackGround(long color)
{
	set_gdk_bg_color(widget,color);	
	if (!border->window) gtk_widget_realize(border);
	gdk_window_process_updates(border->window,true);
	if (visible()) gtk_widget_queue_draw_area(border,0,0,width(),height());
}

long gSimpleLabel::foreGround()
{
	return get_gdk_fg_color(widget);
}

void gSimpleLabel::setForeGround(long color)
{	
	set_gdk_fg_color(widget,color);
	if (!border->window) gtk_widget_realize(border);
	gdk_window_process_updates(border->window,true);
	if (visible()) gtk_widget_queue_draw_area(border,0,0,width(),height());
}

void gSimpleLabel::setFont(gFont *ft)
{
	PangoFontDescription *desc=pango_context_get_font_description(ft->ct);
	
	font_change=true;
	gtk_widget_modify_font(widget,desc);
	pango_layout_set_font_description(layout,desc);
	pango_layout_context_changed(layout);
	setAutoResize(autoresize);
	if (visible()) gtk_widget_queue_draw_area(border,0,0,width(),height());
}

void gSimpleLabel::resize(long w,long h)
{
	gint mh;
	
	if (autoresize) {
		pango_layout_get_size(layout,NULL,&mh);
		mh/=PANGO_SCALE;
		mh+=2;
		h=mh;	
	}
	
	
	gControl::resize(w,h);
	
	if (visible()) gtk_widget_queue_draw_area(border,0,0,width(),height());
}