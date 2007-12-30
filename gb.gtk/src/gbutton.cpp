/***************************************************************************

  gbutton.cpp

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
#include <string.h>
#include <stdlib.h>
#include "widgets.h"
#include "widgets_private.h"

void bt_click (GtkButton *object,gControl *data)
{
	if (!gApplication::userEvents()) return;

	if (((gButton*)data)->type==4)
	{
		if ( ((gButton*)data)->disable )
		{
			((gButton*)data)->disable=false;
			return;
		}
		if (!((gButton*)data)->isToggle)
		{
			((gButton*)data)->disable=true;
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(data->widget),false);
		}
	}

	if (((gButton*)data)->onClick) ((gButton*)data)->onClick((gControl*)data);
	return;
}

bool button_expose(GtkWidget *wid,GdkEventExpose *e,gButton *data)
{
	GdkGC        *gc;
	GdkPixbuf    *img;
	GtkBorder    br={0,0,0,0};
	GdkRectangle rpix={0,0,0,0};
	GdkRectangle rect={0,0,0,0};
	GtkCellRendererState state;
	gint         py,px;
	bool         rtl=false,bcenter=false;
	int          bx,by,width, height, rowstride, n_channels;
  	guchar       p,*pixels;
	char         *test=NULL;

	if (!gApplication::allEvents()) return false;

	if (gtk_widget_get_default_direction()==GTK_TEXT_DIR_RTL) rtl=true;

	gdk_drawable_get_size(GDK_DRAWABLE(wid->window),&rect.width,&rect.height);
	px=rect.width;

	if (data->rendpix)
	{
		if (GTK_WIDGET_STATE(data->widget)==GTK_STATE_INSENSITIVE) img=data->rendinc;
		else img=data->rendpix;

		if (!img)
		{
			img=gdk_pixbuf_copy(data->rendpix);
			width = gdk_pixbuf_get_width (img);
			height = gdk_pixbuf_get_height (img);
			pixels = gdk_pixbuf_get_pixels (img);
			n_channels=gdk_pixbuf_get_n_channels(img);
			for (by=0; by<height; by++)
				for (bx=0;bx<width; bx++)
				{
					p=(pixels[0]+pixels[1]+pixels[2])/3;
					pixels[0]=p; pixels[1]=p; pixels[2]=p;
					pixels+=n_channels;
				}
			data->rendinc=img;
		}

		rpix.width=gdk_pixbuf_get_width(img);
		rpix.height=gdk_pixbuf_get_height(img);
		rect.width-=rpix.width;
		rect.x+=rpix.width;
		py=(rect.height-rpix.height)/2;
		gc=gdk_gc_new(data->border->window);
		gdk_gc_set_clip_origin(gc,0,0);
		gdk_gc_set_clip_rectangle(gc,&e->area);

		g_object_get(G_OBJECT(data->rendtxt),"text",&test,NULL);
		if (!test) bcenter=true;
		else
		{
			if (!strlen(test)) bcenter=true;
			g_free(test);
		}
		if (bcenter) 
		{	
			gdk_draw_pixbuf(GDK_DRAWABLE(wid->window),NULL,img,0,0,(px-rpix.width)/2,py,
                                        -1,-1,GDK_RGB_DITHER_MAX,0,0);
			g_object_unref(gc);
			return false;
		}

		if (rtl)
			gdk_draw_pixbuf(GDK_DRAWABLE(wid->window),gc,img,0,0,rect.width-6,py,
                                        -1,-1,GDK_RGB_DITHER_MAX,0,0);
		else
			gdk_draw_pixbuf(GDK_DRAWABLE(wid->window),gc,img,0,0,6,py,
                                        -1,-1,GDK_RGB_DITHER_MAX,0,0);

		g_object_unref(G_OBJECT(gc));
	}
		

	g_object_set(G_OBJECT(data->rendtxt),"sensitive",true,0);
	switch(GTK_WIDGET_STATE(data->widget))
	{
		case GTK_STATE_NORMAL: state=(GtkCellRendererState)0; break;
		case GTK_STATE_ACTIVE: state=GTK_CELL_RENDERER_PRELIT; break;
		case GTK_STATE_PRELIGHT: state=GTK_CELL_RENDERER_PRELIT; break;
		case GTK_STATE_SELECTED: state=GTK_CELL_RENDERER_SELECTED; break;
		case GTK_STATE_INSENSITIVE: state=GTK_CELL_RENDERER_INSENSITIVE; 
		                           g_object_set(G_OBJECT(data->rendtxt),"sensitive",false,0); 
					   break;
	}
	
	
	rect.width-=12;
	rect.x+=6;
	if (rtl)
	{
		rect.width=px-rect.x-6;
		rect.x=6;
	}
	gtk_cell_renderer_set_fixed_size(data->rendtxt,rect.width,rect.height);
	gtk_cell_renderer_render(data->rendtxt,wid->window,wid,&rect,&rect,&rect,state);
	
	return FALSE;
}

void rd_click (GtkButton *object,gControl *data)
{
	if (!gApplication::userEvents()) return;

	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(object)))
		if (((gButton*)data)->onClick) ((gButton*)data)->onClick((gControl*)data);
	return;
}


//***************************************************************
// Type:
// 0 -> Button
// 1 -> ToggleButton
// 2 -> CheckBox
// 3 -> RadioButton
// 4 -> ToolButton
//***************************************************************
gButton::gButton(gControl *parent,int typ) : gControl(parent)
{
	GtkWidget *lbl;	
	gContainer *ct;

	isToggle=false;
	disable=false;
	g_typ=Type_gButton;
	bufText=NULL;
	rendtxt=NULL;
	rendpix=NULL;
	rendinc=NULL;
	border=gtk_event_box_new();
	scaled=false;
	
	switch(typ)
	{
		case 1:
			rendtxt=gtk_cell_renderer_text_new();
			widget=gtk_toggle_button_new();
			type=1;
			break;
			
		case 2:
			widget=gtk_check_button_new();
			type=2;
			break;
		case 3:
			ct=(gContainer*)pr;
			if (!ct->radiogroup) 
			{
				ct->radiogroup=gtk_radio_button_new(NULL);
				g_object_ref(ct->radiogroup);
				widget=gtk_radio_button_new_from_widget(GTK_RADIO_BUTTON(ct->radiogroup));
				gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(widget),true);
			}
			else 
			{
				widget=gtk_radio_button_new_from_widget(GTK_RADIO_BUTTON(ct->radiogroup));
			}
			type=3;
			break;
		case 4:
			rendtxt=gtk_cell_renderer_text_new();
			widget=gtk_toggle_button_new();
			type=4;
			break;
		
		default:
			widget=gtk_button_new();
			rendtxt=gtk_cell_renderer_text_new();
			type=0;
			break;
	}

	gtk_container_add (GTK_CONTAINER(border),widget);

	if (rendtxt) 
	{
		g_object_set(G_OBJECT(rendtxt),"xalign",0.5,NULL);
		g_object_set(G_OBJECT(rendtxt),"yalign",0.5,NULL);
		g_signal_connect_after(G_OBJECT(border),"expose-event",G_CALLBACK(button_expose),(gpointer)this);
	}
	else
	{	
		lbl=gtk_label_new_with_mnemonic("");
		gtk_label_set_justify(GTK_LABEL(lbl),GTK_JUSTIFY_CENTER);
		gtk_container_add (GTK_CONTAINER(widget),GTK_WIDGET(lbl));
	}
	
	gtk_button_set_focus_on_click (GTK_BUTTON(widget),true);
	gtk_widget_add_events(widget,GDK_POINTER_MOTION_MASK);
	onClick=NULL;
	connectParent();
	initSignals();
	
	if (type==3)
		g_signal_connect(G_OBJECT(widget),"clicked",G_CALLBACK(rd_click),(gpointer)this);
	else
		g_signal_connect(G_OBJECT(widget),"clicked",G_CALLBACK(bt_click),(gpointer)this);	
	
	resize(100,30);
	if (type==4) setBorder(false);
	
}

void gButton::setInconsistent(bool vl)
{
	if (type != 2) return;

	gtk_toggle_button_set_inconsistent (GTK_TOGGLE_BUTTON(widget),vl);
}

bool gButton::inconsistent()
{
	gboolean vl=false;

	if (type != 2) return false;
	g_object_get (G_OBJECT(widget),"inconsistent",&vl,NULL);
	return vl;	
}


void gButton::setFont(gFont *ft)
{
	PangoFontDescription *desc=pango_context_get_font_description(ft->ct);
	GtkBox *box;
	GtkLabel *lbl;
	GList *chd;

	return; /* TODO */
	
	font_change=true;
	if (type==4) { gtk_widget_modify_font(widget,desc); return; }

	if (rendtxt)
	{
	}
	else
	{
		box=(GtkBox*)gtk_bin_get_child(GTK_BIN(widget));
		chd=gtk_container_get_children(GTK_CONTAINER(box));
		chd=chd->next;
		lbl=GTK_LABEL(chd->data);
		gtk_widget_modify_font(GTK_WIDGET(lbl),desc);
		g_list_free(chd);
	}
}

gFont* gButton::font()
{
	GtkBox *box;
	GtkLabel *lbl;
	GList *chd;
	gFont *hFont;

	return NULL; /*TODO*/

	if (type==4) return new gFont(widget);
	
	box=(GtkBox*)gtk_bin_get_child(GTK_BIN(widget));
	chd=gtk_container_get_children(GTK_CONTAINER(box));
	chd=chd->next;
	lbl=GTK_LABEL(chd->data);
	hFont=new gFont(GTK_WIDGET(lbl));
	g_list_free(chd);
	return hFont;
	
	
}

bool gButton::enabled()
{
	return GTK_WIDGET_SENSITIVE(widget);
}

void gButton::setEnabled(bool vl)
{
	gtk_widget_set_sensitive(widget,vl);
}

void gButton::setBackGround(long color)
{
	GtkWidget *wid;

	wid=(type==2)? border:widget;
	set_gdk_bg_color(wid,color);
	if (!border->window) gtk_widget_realize(border);
	gdk_window_process_updates(border->window,true);
}

long gButton::backGround()
{
	if (type==2) get_gdk_bg_color(border);
	return get_gdk_bg_color(widget);
}

void gButton::setForeGround(long color)
{
	GdkColor col;
	GtkWidget *lbl;

	set_gdk_fg_color(widget,color);	
	
	if (rendtxt)
	{
		fill_gdk_color(&col,color);
		g_object_set(G_OBJECT(rendtxt),"foreground-set",TRUE,NULL);
		g_object_set(G_OBJECT(rendtxt),"foreground-gdk",&col,NULL);
	}
	else
	{
		lbl=gtk_bin_get_child(GTK_BIN(widget));
		set_gdk_fg_color(lbl,color);
	}
	
	if (!border->window) gtk_widget_realize(border);
	gdk_window_process_updates(border->window,true);
}

long gButton::foreGround()
{
	return get_gdk_fg_color(widget);
}


const char* gButton::text()
{
	if (type==4) return this->toolTip();
	return bufText;
}



void gButton::setText(const char *st)
{
	GtkLabel *lbl=NULL;
	GtkAccelGroup *accel;
	gControl *par=NULL;
	guint key;
	char *buf;
	GdkRectangle rect={0,0,0,0};
	

	if (type==4) { this->setToolTip((char*)st); return; }
	
	if (bufText) { g_free(bufText); bufText=NULL; }
 	bufText=g_strdup(st);

	if (!rendtxt)
	{
		lbl=(GtkLabel*)gtk_bin_get_child(GTK_BIN(widget));
		if (!st) st="";
		if (!strlen(st))
		{ 
			gtk_label_set_text(lbl,"");
			g_object_set(G_OBJECT(lbl),"visible",false,NULL);
			return;
		}
		gMnemonic_correctText((char*)st,&buf);
		gtk_label_set_text_with_mnemonic(lbl,buf);
		g_object_set(G_OBJECT(lbl),"visible",true,NULL);
		g_free(buf);
		return;
		
	}
	
	par=this->pr;
	while (par->pr) { par=par->pr; }
	accel=((gMainWindow*)par)->accel;
		
	key=gMnemonic_correctMarkup((char*)st,&buf);
	g_object_set(G_OBJECT(rendtxt),"markup",buf,NULL);
	if (key) gtk_widget_add_accelerator(widget,"clicked",accel,key,GDK_MOD1_MASK,(GtkAccelFlags)0);
	g_free(buf);
	if (border->window)
	{
		gdk_drawable_get_size(GDK_DRAWABLE(border->window),&rect.width,&rect.height);
		gdk_window_invalidate_rect(border->window,&rect,TRUE);
	}
	
}


gPicture* gButton::picture()
{
	GdkPixbuf *buf=NULL;
	GdkPixmap *map=NULL;
	GList *chd;
	GtkBox *box;	
	gPicture *pic;
	GdkScreen *scr;
	gint depth;
	gint myw,myh;
	
	if ( (type==2) || (type==3) ) return NULL;
	
	box=(GtkBox*)gtk_bin_get_child(GTK_BIN(widget));
	chd=gtk_container_get_children(GTK_CONTAINER(box));
	chd=g_list_first(chd);
	
	if (gtk_image_get_storage_type(GTK_IMAGE(chd->data))==GTK_IMAGE_PIXBUF)
	{
		buf=gtk_image_get_pixbuf(GTK_IMAGE(chd->data));
		pic=gPicture::fromPixbuf(buf);
	}
	else
	{
		gtk_image_get_pixmap (GTK_IMAGE(chd->data),&map,NULL);
       	pic=new gPicture(0,0,false);
		pic->pic=map;
		g_object_ref(G_OBJECT(map));
	}
	
	g_list_free(chd);
	
	return pic;
	
}

void gButton::setPicture(gPicture *pic)
{
	GdkPixbuf *buf=NULL;
	GdkRectangle rect={0,0,0,0};
	gint bx,by,bw,bh;
	int width, height, rowstride, n_channels;
  	guchar *pixels, *p;
	
	if ( (type==2) || (type==3) ) return;
	if (pic) buf=pic->getPixbuf();

	if (rendpix) g_object_unref(G_OBJECT(rendpix));
	if (rendinc) { g_object_unref(G_OBJECT(rendinc)); rendinc=NULL; }

	rendpix=buf;

}

bool gButton::getBorder()
{
	switch(gtk_button_get_relief(GTK_BUTTON(widget)))
	{
		case GTK_RELIEF_NORMAL:
		case GTK_RELIEF_HALF:
			return true;
		default: 
			return false;
	}
}

void gButton::setBorder(bool vl)
{
	if (vl)
		gtk_button_set_relief (GTK_BUTTON(widget),GTK_RELIEF_NORMAL);
	else
		gtk_button_set_relief (GTK_BUTTON(widget),GTK_RELIEF_NONE);
}

bool gButton::isDefault()
{
	if (GTK_WIDGET_HAS_DEFAULT(widget)) return true;
	
	return false;
}

void gButton::setDefault(bool vl)
{
	if (vl)
	{
		GTK_WIDGET_SET_FLAGS (widget, GTK_CAN_DEFAULT);
		GTK_WIDGET_SET_FLAGS (widget, GTK_CAN_FOCUS);
		gtk_widget_grab_focus (widget);
		gtk_widget_grab_default (widget);
	}
	else
	{
		GTK_WIDGET_UNSET_FLAGS (widget, GTK_CAN_DEFAULT);
		GTK_WIDGET_UNSET_FLAGS (widget, GTK_CAN_FOCUS);
	}
}

bool gButton::value()
{
	switch(type)
	{
		case 1:
		case 2:
		case 3:
		case 4:
			return gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(widget));
		
		default:
			return false;
	}
		
}

void gButton::setValue(bool vl)
{
	switch(type)
	{
		case 1:
		case 2:
		case 3:
		case 4:
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(widget),vl);
			break;
		
		default:
			if (vl) gtk_button_clicked(GTK_BUTTON(widget));
			break;
	}
}

bool gButton::isCancel()
{
	gControl *par=this->pr;
	
	if (type!=0) return false;
	
	while (par->pr) par=par->pr;
	if (((gMainWindow*)par)->esc_button==widget) return true;
	return false;
}

void gButton::setCancel(bool vl)
{
	gControl *par=this->pr;
	
	if (type!=0) return;
	
	while (par->pr) par=par->pr;
	((gMainWindow*)par)->esc_button=widget;
	
}

bool gButton::getToggle()
{
	if (type!=4) return false;
	return isToggle;
}

void gButton::setToggle(bool vl)
{
	if (type!=4) return;
	isToggle=vl;
}

