/***************************************************************************

  gtabstrip.cpp

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
#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>

void gTabStrip_click(GtkNotebook *nb,GtkNotebookPage *pg,guint pnum,gTabStrip *data)
{
	if (data->onClick) data->onClick(data);
}

gTabStrip::gTabStrip(gControl *parent) : gContainer(parent)
{
	GtkWidget *hbox;
	GtkWidget *lbl;
	
	onClick=NULL;
	g_typ=Type_gTabStrip;
	border=gtk_notebook_new();
	widget=gtk_layout_new(0,0);
	hbox=gtk_hbox_new(false,0);
	lbl=gtk_label_new_with_mnemonic("");
	gtk_container_add(GTK_CONTAINER(hbox),lbl);
	gtk_widget_show_all(hbox);
	gtk_notebook_append_page(GTK_NOTEBOOK(border),widget,hbox);

	connectParent();
	initSignals();

	g_signal_connect(G_OBJECT(border),"switch-page",G_CALLBACK(gTabStrip_click),(gpointer)this);

}

long gTabStrip::backGround()
{
	return get_gdk_bg_color(border);
}

void gTabStrip::setBackGround(long color)
{	
	GtkWidget *lbl;
	GtkWidget *fix;
	long b;
	
	set_gdk_bg_color(border,color);
	for (b=0;b<count();b++)
	{
		fix=gtk_notebook_get_nth_page(GTK_NOTEBOOK(border),b);
		set_gdk_bg_color(fix,color);
	}
	
	if (!border->window) gtk_widget_realize(border);
	gdk_window_process_updates(border->window,true);
}

long gTabStrip::foreGround()
{
	GtkWidget *lbl;
	GtkWidget *fix;
	GtkWidget *hbox;
	GList *chd;
		
	fix=gtk_notebook_get_nth_page(GTK_NOTEBOOK(border),0);
	hbox=gtk_notebook_get_tab_label(GTK_NOTEBOOK(border),fix);
	chd=gtk_container_get_children(GTK_CONTAINER(hbox));
	if ( G_OBJECT_TYPE(G_OBJECT(chd->data)) != GTK_TYPE_LABEL ) chd=chd->next;
	lbl=GTK_WIDGET(chd->data);
	g_list_free(chd);
	return get_gdk_fg_color(lbl);
}

void gTabStrip::setForeGround(long color)
{	
	GtkWidget *lbl;
	GtkWidget *fix;
	GtkWidget *hbox;
	GList *chd;
	long b;

	for (b=0;b<count();b++)
	{	
		fix=gtk_notebook_get_nth_page(GTK_NOTEBOOK(border),b);
		hbox=gtk_notebook_get_tab_label(GTK_NOTEBOOK(border),fix);
		chd=gtk_container_get_children(GTK_CONTAINER(hbox));
		if ( G_OBJECT_TYPE(G_OBJECT(chd->data)) != GTK_TYPE_LABEL ) chd=chd->next;
		lbl=GTK_WIDGET(chd->data);
		g_list_free(chd);
		set_gdk_fg_color(lbl,color);
	}
	
	if (!border->window) gtk_widget_realize(border);
	gdk_window_process_updates(border->window,true);
}

long gTabStrip::count()
{
	return (long)gtk_notebook_get_n_pages(GTK_NOTEBOOK(border));
}

long gTabStrip::index()
{
	return gtk_notebook_get_current_page(GTK_NOTEBOOK(border));
}

long gTabStrip::orientation()
{
	switch ( gtk_notebook_get_tab_pos(GTK_NOTEBOOK(border)) )
	{
		case GTK_POS_TOP: return 0;
		default: return 1;
	}
}

gPicture* gTabStrip::picture(long ind)
{
	GdkPixbuf *buf=NULL;
	GtkWidget *fix;
	GtkWidget *hbox;
	gPicture *ret=NULL;
	GList *chd;
	
	if ( (ind<0) || (ind>=count()) ) return NULL;
	
	fix=gtk_notebook_get_nth_page(GTK_NOTEBOOK(border),ind);
	hbox=gtk_notebook_get_tab_label(GTK_NOTEBOOK(border),fix);
	chd=gtk_container_get_children(GTK_CONTAINER(hbox));
	
	if ( G_OBJECT_TYPE(chd->data) != GTK_TYPE_LABEL )
	{
		buf=gtk_image_get_pixbuf(GTK_IMAGE(chd->data));
		ret=gPicture::fromPixbuf(buf);
	}
	
	g_list_free(chd);
	return ret;
	
}

bool gTabStrip::tabEnabled(long ind)
{
	GtkWidget *fix;
	if ( (ind<0) || (ind>=count()) ) return false;
	
	fix=gtk_notebook_get_nth_page(GTK_NOTEBOOK(border),ind);
	return GTK_WIDGET_SENSITIVE(fix);
}

char* gTabStrip::text(long ind)
{
	GtkWidget *fix;
	GtkWidget *hbox;
	GtkWidget *lbl;	
	GList *chd;
	char *buf;

	if ( (ind<0) || (ind>=count()) ) return NULL;
	
	fix=gtk_notebook_get_nth_page(GTK_NOTEBOOK(border),ind);
	hbox=gtk_notebook_get_tab_label(GTK_NOTEBOOK(border),fix);
	chd=gtk_container_get_children(GTK_CONTAINER(hbox));
	if ( G_OBJECT_TYPE(G_OBJECT(chd->data)) != GTK_TYPE_LABEL ) chd=chd->next;
	lbl=GTK_WIDGET(chd->data);
	g_list_free(chd);
	gMnemonic_returnText((char*)gtk_label_get_text(GTK_LABEL(lbl)),&buf);
	return buf;
}

int gTabStrip::setCount(long vl)
{
	GtkWidget *fix;
	GtkWidget *lbl;
	GtkWidget *hbox;
	GList *chd,*iter,*ok=NULL;

	if (vl>count())
	{
		while (vl>count())
		{
			fix=gtk_layout_new(0,0);
			hbox=gtk_hbox_new(false,0);
			lbl=gtk_label_new_with_mnemonic("");
			gtk_container_add(GTK_CONTAINER(hbox),lbl);
			gtk_widget_show_all(hbox);
			gtk_notebook_append_page(GTK_NOTEBOOK(border),fix,hbox);
			gtk_widget_show(fix);
			set_gdk_bg_color(fix,backGround());
			set_gdk_fg_color(lbl,foreGround());
		}
		return 0;
	}
	
	if (vl<1) return -2;
	
	if (vl<count())
	{
		chd=gtk_container_get_children(GTK_CONTAINER(border));
		iter=chd;
		while (iter)
		{
			if (gtk_notebook_page_num(GTK_NOTEBOOK(border),GTK_WIDGET(iter->data))>=vl)
			{
				ok=gtk_container_get_children(GTK_CONTAINER(iter->data));
				if (ok) break;
			}
			iter=g_list_next(iter);
		}
		g_list_free(chd);
		if (ok) return -1;
		while (vl<count())
			gtk_notebook_remove_page(GTK_NOTEBOOK(border),count()-1);
	}
	
	return 0;
}

void gTabStrip::setIndex(long vl)
{
	if ( (vl<0) || (vl>=count()) ) return;
	
	gtk_notebook_set_current_page(GTK_NOTEBOOK(border),vl);
	widget=gtk_notebook_get_nth_page(GTK_NOTEBOOK(border),vl);
}

void gTabStrip::setOrientation(long vl)
{
	switch (vl)
	{
		case 0:
			gtk_notebook_set_tab_pos(GTK_NOTEBOOK(border),GTK_POS_TOP); break;
		case 1:
			gtk_notebook_set_tab_pos(GTK_NOTEBOOK(border),GTK_POS_BOTTOM); break;
	}
}

void gTabStrip::setPicture(long ind,gPicture *pic)
{
	GdkPixbuf *buf=NULL;
	GtkWidget *fix;
	GtkWidget *hbox;
	GtkWidget *img;
	GList *chd;
	
	if ( (ind<0) || (ind>=count()) ) return;
	
	fix=gtk_notebook_get_nth_page(GTK_NOTEBOOK(border),ind);
	hbox=gtk_notebook_get_tab_label(GTK_NOTEBOOK(border),fix);
	chd=gtk_container_get_children(GTK_CONTAINER(hbox));
	
	if (pic) buf=pic->getPixbuf();
	
	if (!buf)
	{
		if (G_OBJECT_TYPE(G_OBJECT(chd->data)) != GTK_TYPE_LABEL)
			gtk_container_remove(GTK_CONTAINER(hbox),GTK_WIDGET(chd->data)); 
		g_list_free(chd);
		return;
	}
	
	if (G_OBJECT_TYPE(G_OBJECT(chd->data)) == GTK_TYPE_LABEL)
	{
		g_object_ref(G_OBJECT(chd->data));
		gtk_container_remove ( GTK_CONTAINER(hbox), GTK_WIDGET(chd->data) );
		img=gtk_image_new_from_pixbuf(buf);
		gtk_container_add( GTK_CONTAINER(hbox),img);
		gtk_container_add( GTK_CONTAINER(hbox),GTK_WIDGET(chd->data) );
		g_object_unref(G_OBJECT(chd->data));
		gtk_widget_show_all(hbox);
				
	}
	else
	{
		img=GTK_WIDGET(chd->data);
		gtk_image_set_from_pixbuf(GTK_IMAGE(img),buf);
	}
	g_list_free(chd);
	g_object_unref(G_OBJECT(buf));
	
}

void gTabStrip::setTabEnabled(long ind,bool vl)
{
	GtkWidget *fix;
	GtkWidget *lbl;
	
	stub("gTabStrip::setTabEnabled - partial stub - ");
	
	if ( (ind<0) || (ind>=count()) ) return;
	
	fix=gtk_notebook_get_nth_page(GTK_NOTEBOOK(border),ind);
	lbl=gtk_notebook_get_tab_label(GTK_NOTEBOOK(border),fix);
	gtk_widget_set_sensitive (fix,vl);
	gtk_widget_set_sensitive (lbl,vl);
}

void gTabStrip::setText(long ind,char *txt)
{
	GtkWidget *fix;
	GtkWidget *lbl;	
	GtkWidget *hbox;
	GList *chd;
	char *buf=NULL;

	if ( (ind<0) || (ind>=count()) ) return;
	
	fix=gtk_notebook_get_nth_page(GTK_NOTEBOOK(border),ind);
	hbox=gtk_notebook_get_tab_label(GTK_NOTEBOOK(border),fix);
	chd=gtk_container_get_children(GTK_CONTAINER(hbox));
	if ( G_OBJECT_TYPE(G_OBJECT(chd->data)) != GTK_TYPE_LABEL ) chd=chd->next;
	lbl=GTK_WIDGET(chd->data);
	g_list_free(chd);
	//gtk_label_set_text(GTK_LABEL(lbl),txt);
	if ( txt )
		if (strlen(txt)) 
			gMnemonic_correctText((char*)txt,&buf);
	
	
	if (buf)
	{
		gtk_label_set_text_with_mnemonic(GTK_LABEL(lbl),buf);
		free(buf);
		return;
	}
	gtk_label_set_text_with_mnemonic(GTK_LABEL(lbl),"");
}


