/***************************************************************************

  glistbox.cpp

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
#include <string.h>
#include <stdlib.h>

#include <stdio.h>

#define STORE_LIST ((GtkListStore*)gtk_tree_view_get_model(GTK_TREE_VIEW(widget)))
/**************************************************************************

gStoreList

***************************************************************************/
long gStoreList_Count(GtkListStore *store)
{
	GtkTreeIter iter;
	long len=1;
	
	if (!gtk_tree_model_get_iter_first(GTK_TREE_MODEL(store),&iter)) return 0;
	
	while ( gtk_tree_model_iter_next(GTK_TREE_MODEL(store),&iter) ) len++;

	return len;
}

int gStoreList_Iter(GtkListStore *store,GtkTreeIter *iter,long ind)
{
	long len=0;
	
	if (ind<0) return -1;
	if (ind>=gStoreList_Count(store)) return -1;
	
	if (!gtk_tree_model_get_iter_first(GTK_TREE_MODEL(store),iter)) return -1;
	while (len<ind)
	{
		len++;
		gtk_tree_model_iter_next(GTK_TREE_MODEL(store),iter);
	}
	
	return 0;
}

void gStoreList_setItemSelected(GtkListStore *store,GtkTreeSelection *sel,long ind,bool vl)
{
	GtkTreeIter iter;
	long bucle=0;
	
	if ( gStoreList_Iter(store,&iter,ind) ) return;
	
	if (vl) gtk_tree_selection_select_iter(sel,&iter);
	else	gtk_tree_selection_unselect_iter(sel,&iter);
}

bool gStoreList_itemSelected(GtkTreeSelection *sel,long ind)
{
	GtkTreePath *path;
	GList *lst;
	GList *el;
	gint *indices;
	bool ret=false;
	
	if (!sel) return false;
	
	lst=gtk_tree_selection_get_selected_rows(sel,NULL);
	if (!lst) return false;
	el=g_list_first(lst);
	while (el)
	{
		path=(GtkTreePath*)el->data;
		indices=gtk_tree_path_get_indices(path);
		if (indices[0]==ind) ret=true;
		g_free(path);
		el=g_list_next(el);
	}
	g_list_free(lst);
	
	return ret;
}

void gStoreList_Clear(GtkListStore *store)
{
	GtkTreeIter iter;
 
	while(gtk_tree_model_get_iter_first(GTK_TREE_MODEL(store),&iter))
		 gtk_list_store_remove(store,&iter);
}

char* gStoreList_itemText(GtkListStore *store,long ind)
{
	GtkTreeIter iter;
	char *ret=NULL;

	if (gStoreList_Iter(store,&iter,ind)) return NULL;
	gtk_tree_model_get (GTK_TREE_MODEL(store),&iter,0,&ret,-1);
	return ret;
}

void gStoreList_setItemText(GtkListStore *store,long ind,char *txt)
{
	GtkTreeIter iter;

	if (gStoreList_Iter(store,&iter,ind)) return;
	gtk_list_store_set (store,&iter,0,txt,-1);
}


long gStoreList_Find(GtkListStore *store,char *ptr)
{
	long len=0;
	char *buf;
	GtkTreeIter iter;

	if ( !gtk_tree_model_get_iter_first(GTK_TREE_MODEL(store),&iter) ) return -1;
	
	while (1)
	{
		gtk_tree_model_get (GTK_TREE_MODEL(store),&iter,0,&buf,-1);
		if (!strcmp(buf,ptr))
		{
			g_free(buf);
			return len;
		}
		len++;
		g_free(buf);
		if (!gtk_tree_model_iter_next(GTK_TREE_MODEL(store),&iter)) break;
	}
	
	return -1;
}

void gStoreList_Remove(GtkListStore *store,long pos)
{
	GtkTreeIter iter;

	if (pos<0) return;
	if (pos>=gStoreList_Count(store)) return;
	
	gtk_tree_model_get_iter_first(GTK_TREE_MODEL(store),&iter);
	while (pos--) gtk_tree_model_iter_next(GTK_TREE_MODEL(store),&iter);
	gtk_list_store_remove(store,&iter);
}

char** gStoreList_ArrayList(GtkListStore *store)
{
	char **ret;
	gchar *buf;
	GtkTreeIter iter;
	long bucle,len=2;
	
	ret=NULL;
	if(!gtk_tree_model_get_iter_first(GTK_TREE_MODEL(store),&iter)) return NULL;
	
	while ( gtk_tree_model_iter_next(GTK_TREE_MODEL(store),&iter) ) len++;
	
	ret=(char**)g_malloc(sizeof(char*)*len);
	for (bucle=0;bucle<len;bucle++) ret[bucle]=NULL;
	
	gtk_tree_model_get_iter_first(GTK_TREE_MODEL(store),&iter);
	
	len=0;
	do 
	{
		gtk_tree_model_get (GTK_TREE_MODEL(store),&iter,0,&buf,-1);
		
		if (!buf)
		{
			ret[len]=(char*)g_malloc(sizeof(char));
			ret[len][0]=0;
		}
		else
		{
			ret[len]=(char*)g_malloc(sizeof(char)*(1+strlen(buf)));
			if (ret[len]) strcpy(ret[len],buf);
			g_free(buf);
		}
		if (gtk_tree_model_iter_next(GTK_TREE_MODEL(store),&iter)) strcat(*ret,"\n");
		else break;
		len++;
		
	} while ( true );

	return ret;
	
	
}

void gStoreList_List(GtkListStore *store,char **ret)
{
	gchar *buf;
	GtkTreeIter iter;
	long bucle,len=0;
	
	*ret=NULL;
	if(!gtk_tree_model_get_iter_first(GTK_TREE_MODEL(store),&iter)) return;
	
	do 
	{
		gtk_tree_model_get (GTK_TREE_MODEL(store),&iter,0,&buf,-1);
		len+=strlen(buf)+1;
		g_free(buf);
		
	} while ( gtk_tree_model_iter_next(GTK_TREE_MODEL(store),&iter) );
	
	*ret=(char*)g_malloc(sizeof(char)*len);
	for (bucle=0;bucle<len;bucle++) (*ret)[bucle]=0;
	
	gtk_tree_model_get_iter_first(GTK_TREE_MODEL(store),&iter);
	
	do 
	{
		gtk_tree_model_get (GTK_TREE_MODEL(store),&iter,0,&buf,-1);
		strcat(*ret,buf);
		g_free(buf);
		
		if (gtk_tree_model_iter_next(GTK_TREE_MODEL(store),&iter)) strcat(*ret,"\n");
		else break;
		
	} while ( true );
	
	
}

void gStoreList_setArrayList(GtkListStore *store,char **vl)
{
	long bucle;
	long len=gStoreList_Count(store);
	GtkTreeIter iter;
 
	while(gtk_tree_model_get_iter_first(GTK_TREE_MODEL(store),&iter))
		 gtk_list_store_remove(store,&iter);

	if (!vl) return;

	bucle=0;
	len=0;
	while (vl[len]) 
	{
		gtk_list_store_append (store, &iter);
		gtk_list_store_set (store, &iter, 0, vl[len], -1);
		len++;
	}

}

void gStoreList_setList(GtkListStore *store,char *vl)
{
	gchar **split;
	long bucle;
	long len=gStoreList_Count(store);
	GtkTreeIter iter;
 
	while(gtk_tree_model_get_iter_first(GTK_TREE_MODEL(store),&iter))
		 gtk_list_store_remove(store,&iter);

	bucle=0;
	if (vl) 
	{
		split=g_strsplit(vl,"\n",0);
		while (split[bucle])
		{
			gtk_list_store_append (store, &iter);
			gtk_list_store_set (store, &iter, 0, split[bucle++], -1);
		}
		g_strfreev(split);
	}

}

void gStoreList_sortData(GtkListStore *store)
{
	GtkTreeIter iter,it2;
	char *buf1=NULL,*buf2=NULL;

	if (!gtk_tree_model_get_iter_first(GTK_TREE_MODEL(store),&iter)) return;
	while (true)
	{
		it2=iter;
		if (buf1) { g_free(buf1); buf1=NULL; }
		gtk_tree_model_get (GTK_TREE_MODEL(store),&iter,0,&buf1,-1);
		if (!gtk_tree_model_iter_next(GTK_TREE_MODEL(store),&it2)) break;
		while (true)
		{
			if (buf2) { g_free(buf2); buf2=NULL; }
			gtk_tree_model_get (GTK_TREE_MODEL(store),&it2,0,&buf2,-1);
			
			if (strcmp (buf1,buf2)>0 )
			{
				gtk_list_store_set (store, &iter, 0, buf2, -1);
				gtk_list_store_set (store, &it2, 0, buf1, -1);
				g_free(buf1);
				buf1=g_strdup(buf2);
			}
			
			if (!gtk_tree_model_iter_next(GTK_TREE_MODEL(store),&it2)) break;
		}
		gtk_tree_model_iter_next(GTK_TREE_MODEL(store),&iter);
	}
	
	if (buf1) g_free(buf1);
	if (buf2) g_free(buf2);

}

/**************************************************************************

gListBox

***************************************************************************/

void gListBox_Select(GtkTreeView *tv,gListBox *data)
{
	if (data->onSelect) data->onSelect(data);
}


void gListBox_Activate(GtkTreeView *tv,GtkTreePath *a1,GtkTreeViewColumn *arg2,gListBox *data)
{
	if (data->onActivate && (data->mode()!=3)) data->onActivate(data);
}


gListBox::gListBox(gControl *parent) : gControl(parent)
{
	GtkListStore *store;
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;

	g_typ=Type_gListBox;
	sort=false;
	border=gtk_scrolled_window_new (NULL,NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(border),GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(border),GTK_SHADOW_IN);
	
	store=gtk_list_store_new (1,G_TYPE_STRING,NULL);
	widget=gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(widget),false);
	gtk_container_add(GTK_CONTAINER(border),widget);
	g_object_unref (store);
	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes ("",renderer,"text",0,NULL);
	gtk_tree_view_column_set_alignment(column,0);
	gtk_tree_view_append_column (GTK_TREE_VIEW (widget), column);
		
	connectParent();
	initSignals();

	onSelect=NULL;
	onActivate=NULL;
	g_signal_connect(G_OBJECT(widget),"row-activated",G_CALLBACK(gListBox_Activate),(gpointer)this);
	g_signal_connect(G_OBJECT(widget),"cursor-changed",G_CALLBACK(gListBox_Select),(gpointer)this);
}

long gListBox::backGround()
{
	return get_gdk_base_color(widget);
}

void gListBox::setBackGround(long color)
{
	GtkTreeViewColumn *column;
	GtkCellRenderer *renderer;
	GList *rend;
	GdkColor gcol;

	set_gdk_base_color(widget,color);	
	
	column=gtk_tree_view_get_column(GTK_TREE_VIEW(widget),0);
	rend=gtk_tree_view_column_get_cell_renderers(column);
	renderer=(GtkCellRenderer*)rend->data;
	fill_gdk_color(&gcol,color);
	g_object_set(G_OBJECT(renderer),"cell-background-gdk",&gcol,NULL);
	g_list_free(rend);
	
	if (!border->window) gtk_widget_realize(border);
	gdk_window_process_updates(border->window,true);
}

long gListBox::foreGround()
{
	return get_gdk_text_color(widget);
}

void gListBox::setForeGround(long color)
{	
	set_gdk_text_color(widget,color);
	if (!border->window) gtk_widget_realize(border);
	gdk_window_process_updates(border->window,true);
}

long gListBox::count()
{
	return gStoreList_Count(STORE_LIST);
	
}

long gListBox::index()
{
	GtkTreePath *path;
	GtkTreeViewColumn *col;
	gint *indices;
	
	if (selmode==0) return -1;
	
	gtk_tree_view_get_cursor(GTK_TREE_VIEW(widget),&path,&col);
	if (!path) return -1;
	indices=gtk_tree_path_get_indices(path);
	return indices[0];
}

void gListBox::setIndex(long ind)
{
	GtkTreeIter iter;
	GtkTreePath *path;
	
	if ( gStoreList_Iter(STORE_LIST,&iter,ind) ) return;
	
	path=gtk_tree_model_get_path(GTK_TREE_MODEL(STORE_LIST),&iter);
	gtk_tree_view_set_cursor(GTK_TREE_VIEW(widget),path,NULL,false);
}

bool gListBox::itemSelected(long ind)
{
	GtkTreeSelection *sel;
	
	sel=gtk_tree_view_get_selection(GTK_TREE_VIEW(widget));
	return gStoreList_itemSelected(sel,ind);
}

char* gListBox::itemText(long ind)
{
	return gStoreList_itemText(STORE_LIST,ind);
}

void gListBox::setItemText(long ind,char *txt)
{
	gStoreList_setItemText(STORE_LIST,ind,txt);
}

char** gListBox::list()
{
	return gStoreList_ArrayList(STORE_LIST);
}

long gListBox::mode()
{
	return selmode;
}

bool gListBox::sorted()
{
	return sort;
}

char* gListBox::text()
{
	long ind;
	
	ind=index();
	if (ind==-1) return NULL;
	return gStoreList_itemText(STORE_LIST,ind);
	
}

void gListBox::setItemSelected(long ind,bool vl)
{
	GtkTreeSelection *sel=gtk_tree_view_get_selection(GTK_TREE_VIEW(widget));
	gStoreList_setItemSelected(STORE_LIST,sel,ind,vl);
}

void gListBox::setList(char **vl)
{
	gStoreList_setArrayList(STORE_LIST,vl);
}

void gListBox::setMode(long vl)
{
	GtkTreeSelection* sel=gtk_tree_view_get_selection(GTK_TREE_VIEW(widget));
	
	switch (vl)
	{
		case 0:	// None
			gtk_tree_selection_set_mode (sel,GTK_SELECTION_NONE); 
			selmode=vl; 
			break;
		case 1:	//SINGLE
			gtk_tree_selection_set_mode (sel,GTK_SELECTION_SINGLE); 
			selmode=vl; 
			break;
		case 2:	//MULTIPLE
			gtk_tree_selection_set_mode (sel,GTK_SELECTION_MULTIPLE); 
			selmode=vl; 
			break;
			
		

	}
}

void gListBox::setSorted(bool vl)
{
	sort=vl;
	if (sort) gStoreList_sortData(STORE_LIST);
}

void gListBox::add(char *vl,long pos)
{
	GtkTreeIter iter,curr;
	GtkListStore *store;
	bool set;
	
	store = (GtkListStore*)gtk_tree_view_get_model(GTK_TREE_VIEW(widget));
	
	gtk_list_store_append (store, &iter);
	gtk_list_store_set (store, &iter,0,vl,-1);
}

long gListBox::find(char *ptr)
{
	return gStoreList_Find(STORE_LIST,ptr);
}

void gListBox::remove(long pos)
{
	gStoreList_Remove(STORE_LIST,pos);
}

void gListBox::clear()
{
	gStoreList_Clear(STORE_LIST);
}


