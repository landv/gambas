/***************************************************************************

  gcombobox.cpp

  (c) 2004-2005 - Daniel Campos Fernández <danielcampos@netcourrier.com>
  
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STORE_LIST ((GtkListStore*)gtk_combo_box_get_model(GTK_COMBO_BOX(widget)))

GtkWidget* combo_get_entry(GtkWidget *combo)
{
	GtkWidget *txtentry;
	
	txtentry=gtk_bin_get_child(GTK_BIN(combo));
	if (txtentry)
		if (G_OBJECT_TYPE(txtentry)!=GTK_TYPE_ENTRY)
			txtentry=NULL;
	
	return txtentry;
}

void combo_changed(GtkComboBox *widget,gComboBox *data)
{
	GtkWidget *txtentry=combo_get_entry(data->widget);

	if (txtentry)
		if (data->count()) 
			gtk_entry_set_text(GTK_ENTRY(txtentry),data->itemText(data->index()));

	if (data->onChange) data->onChange(data);
}


gComboBox::gComboBox(gControl *parent) : gControl(parent)
{
	GtkListStore *store;

	onChange=NULL;
	sort=false;
	g_typ=Type_gComboBox;
	store=gtk_list_store_new (1, G_TYPE_STRING);
	border=gtk_event_box_new();
	widget=gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
	g_object_unref (store);
	cell = gtk_cell_renderer_text_new ();
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (widget), cell, true);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (widget),cell,"text",0,NULL);
	gtk_container_add(GTK_CONTAINER(border),widget);
	connectParent();
	initSignals();
	
	g_signal_connect(G_OBJECT(widget),"changed",G_CALLBACK(combo_changed),(gpointer)this);
	setReadOnly(false);

}

void gComboBox::popup()
{
        gtk_combo_box_popup(GTK_COMBO_BOX(widget));
}

long gComboBox::backGround()
{
	return get_gdk_bg_color(widget);
}


void gComboBox::setBackGround(long color)
{
	GdkColor gcol;
	GtkWidget *txtentry=combo_get_entry(widget);
	
	set_gdk_bg_color(widget,color);
	set_gdk_base_color(widget,color);

	if (txtentry) set_gdk_base_color(txtentry,color);
	
	fill_gdk_color(&gcol,color);
	g_object_set(G_OBJECT(cell),"background-gdk",&gcol,NULL);
	g_object_set(G_OBJECT(cell),"ypad",0,NULL);
	
	if (!border->window) gtk_widget_realize(border);
	gdk_window_process_updates(border->window,true);
}

long gComboBox::foreGround()
{
	return get_gdk_fg_color(widget);
}

void gComboBox::setForeGround(long color)
{
	GdkColor gcol;
	GtkWidget *txtentry=combo_get_entry(widget);

	if (txtentry) set_gdk_text_color(txtentry,color);
	set_gdk_fg_color(widget,color);	
	fill_gdk_color(&gcol,color);
	g_object_set(G_OBJECT(cell),"foreground-gdk",&gcol,NULL);
	
	if (!border->window) gtk_widget_realize(border);
	gdk_window_process_updates(border->window,true);
}

long gComboBox::count()
{
	return gStoreList_Count(STORE_LIST);

}

long gComboBox::index()
{
	GtkTreeModel *store;
	GtkTreeIter iter;
	
	store = gtk_combo_box_get_model(GTK_COMBO_BOX(widget));
	if (!gtk_tree_model_get_iter_first(store,&iter)) return 0;

	return gtk_combo_box_get_active (GTK_COMBO_BOX(widget));
}

char* gComboBox::itemText(long ind)
{
	GtkTreeModel *store;
	GtkTreeIter iter;
	char *ret=NULL;
	long len=0;
	
	if (ind<0) return NULL;
	if (ind>=count()) return NULL;
	
	store = gtk_combo_box_get_model(GTK_COMBO_BOX(widget));
	gtk_tree_model_get_iter_first(store,&iter);
	while (len<ind)
	{
		len++;
		gtk_tree_model_iter_next(store,&iter);
	}
	gtk_tree_model_get (GTK_TREE_MODEL(store),&iter,0,&ret,-1);
	return ret;
}

long gComboBox::length()
{
	GtkWidget *txtentry=combo_get_entry(widget);
	const gchar *buf;
	long len;
	long bfree=true;
	
	
	if (!txtentry)
	{
		if (!count()) return 0;
		buf=itemText(index());
		if (!buf) return 0;
		
	}
	else
	{
		bfree=false;
		buf=gtk_entry_get_text(GTK_ENTRY(txtentry));
		if (!buf) return 0;
	}
	len=g_utf8_strlen(buf,-1);
	if (bfree) g_free((void*)buf);
	return len;
}

char** gComboBox::list()
{
	return gStoreList_ArrayList(STORE_LIST);
}


bool gComboBox::readOnly()
{
	GtkWidget *txtentry=combo_get_entry(widget);
	
	return !txtentry;
}

bool gComboBox::sorted()
{
	return sort;
}

char* gComboBox::text()
{
	GtkWidget *txtentry=combo_get_entry(widget);
	char *buf1,*buf2;
	
	if (!txtentry)
	{
		if (count()) return itemText(index());
		return NULL;
	}
	else
	{
		buf1=(char*)gtk_entry_get_text(GTK_ENTRY(txtentry));
		if (!buf1) return NULL;
		buf2=(char*)g_malloc(sizeof(char)*(strlen(buf1)+1));
		if (buf2) strcpy(buf2,buf1);
		return buf2;
	}
}

void gComboBox::setIndex(long vl)
{
	if (vl<0) return;
	if (vl>=count()) return;
	gtk_combo_box_set_active(GTK_COMBO_BOX(widget),vl);
}

void gComboBox::setItemText(long ind,char *txt)
{
	long buf;
	
	if (ind<0) return;
	if (ind>=count()) return;
	buf=index();
	add(txt,ind);
	remove(ind+1);
	setIndex(buf);
	if (sort) gStoreList_sortData(STORE_LIST);
}

void gComboBox::setList(char **vl)
{
	GtkTreeIter iter;

	gStoreList_setArrayList(STORE_LIST,vl);
	
	if (sort) gStoreList_sortData(STORE_LIST);
	if(gtk_tree_model_get_iter_first(GTK_TREE_MODEL(STORE_LIST),&iter))
		gtk_combo_box_set_active_iter(GTK_COMBO_BOX(widget),&iter);

}

void gComboBox::setReadOnly(bool vl)
{
	GtkWidget *txtentry=combo_get_entry(widget);
	GtkListStore *store;
	gint x,y;
	
	if ( (!vl) && (!txtentry) )
	{
		
		txtentry=gtk_entry_new();
		gtk_container_add(GTK_CONTAINER(widget),txtentry);
		if (count()) gtk_entry_set_text(GTK_ENTRY(txtentry),itemText(index()));
		gtk_widget_get_size_request(widget,&x,&y);
		gtk_widget_set_size_request(txtentry,x,y);
		set_gdk_base_color(txtentry,backGround());
		set_gdk_text_color(txtentry,foreGround());
		gtk_widget_show(txtentry);
	}
	
	if ( vl && txtentry ) {
		gtk_container_remove(GTK_CONTAINER(widget),txtentry);
	}
	
}

void gComboBox::setSorted(bool vl)
{
	sort=vl;
	if (sort) gStoreList_sortData(STORE_LIST);
}

void gComboBox::setText(char *vl)
{
	GtkWidget *txtentry=combo_get_entry(widget);

	if (!txtentry) return;
	gtk_entry_set_text(GTK_ENTRY(txtentry),(const gchar*)vl);
}

void gComboBox::add(char *vl,long pos)
{
	GtkTreeIter iter,curr;
	GtkListStore *store;
	bool set=false;

	if (pos<0) return;
	if (pos>count()) return;
	
	store = (GtkListStore*)gtk_combo_box_get_model(GTK_COMBO_BOX(widget));
	
	if (!gtk_tree_model_get_iter_first(GTK_TREE_MODEL(store),&iter))
	{
		gtk_list_store_append (store, &iter);
		gtk_list_store_set (store, &iter,0,vl, -1);
		gtk_combo_box_set_active_iter(GTK_COMBO_BOX(widget),&iter);
		return;
	}
	
	set=gtk_combo_box_get_active_iter(GTK_COMBO_BOX(widget),&curr);
	gtk_list_store_insert(store,&iter,pos);
	gtk_list_store_set (store, &iter,0,vl, -1);
	if (set) gtk_combo_box_set_active_iter(GTK_COMBO_BOX(widget),&curr); 
	if (sort) gStoreList_sortData(STORE_LIST);
}

void gComboBox::clear()
{
	gStoreList_Clear(STORE_LIST);
}

long gComboBox::find(char *ptr)
{
	return gStoreList_Find(STORE_LIST,ptr);
}

void gComboBox::remove(long pos)
{
	gStoreList_Remove(STORE_LIST,pos);
	if (sort) gStoreList_sortData(STORE_LIST);
}

void gComboBox::resize(long w,long h)
{
	GtkWidget *txtentry=combo_get_entry(widget);
	
	gControl::resize(w,h);
	
	if ( txtentry )
		gtk_widget_set_size_request(txtentry,w,h);

}


