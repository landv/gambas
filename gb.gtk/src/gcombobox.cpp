/***************************************************************************

  gcombobox.cpp

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
#include "gmainwindow.h"
#include "gcombobox.h"

#define STORE_LIST ((GtkListStore*)gtk_combo_box_get_model(GTK_COMBO_BOX(widget)))

/**************************************************************************

gStoreList

***************************************************************************/

int gStoreList_Count(GtkListStore *store)
{
	GtkTreeIter iter;
	int len=1;
	
	if (!gtk_tree_model_get_iter_first(GTK_TREE_MODEL(store),&iter)) return 0;
	
	while ( gtk_tree_model_iter_next(GTK_TREE_MODEL(store),&iter) ) len++;

	return len;
}

int gStoreList_Iter(GtkListStore *store,GtkTreeIter *iter,int ind)
{
	int len=0;
	
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

void gStoreList_setItemSelected(GtkListStore *store,GtkTreeSelection *sel,int ind,bool vl)
{
	GtkTreeIter iter;
	
	if ( gStoreList_Iter(store,&iter,ind) ) return;
	
	if (vl) gtk_tree_selection_select_iter(sel,&iter);
	else	gtk_tree_selection_unselect_iter(sel,&iter);
}

bool gStoreList_itemSelected(GtkTreeSelection *sel,int ind)
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

char* gStoreList_itemText(GtkListStore *store,int ind)
{
	GtkTreeIter iter;
	char *ret=NULL;

	if (gStoreList_Iter(store,&iter,ind)) return NULL;
	gtk_tree_model_get (GTK_TREE_MODEL(store),&iter,0,&ret,-1);
	return ret;
}

void gStoreList_setItemText(GtkListStore *store,int ind, const char *txt)
{
	GtkTreeIter iter;

	if (!txt) txt = "";
	if (gStoreList_Iter(store,&iter,ind)) return;
	gtk_list_store_set (store,&iter,0,txt,-1);
}


int gStoreList_Find(GtkListStore *store, const char *ptr)
{
	int len=0;
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

void gStoreList_Remove(GtkListStore *store,int pos)
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
	int bucle,len=2;
	
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
	int bucle,len=0;
	
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
	int bucle;
	int len=gStoreList_Count(store);
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
	int bucle;
	//int len=gStoreList_Count(store);
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
	
	gComboBox
	
**************************************************************************/

static void cb_click(GtkComboBox *widget,gComboBox *data)
{
	if (!data->isReadOnly() && data->count())
	{
		char *text = data->itemText(data->index());
		if (text)
			gtk_entry_set_text(GTK_ENTRY(data->entry), text);
	}
	
	data->emit(SIGNAL(data->onClick));
}

gComboBox::gComboBox(gContainer *parent) : gTextBox(parent, true)
{
	GtkListStore *store;

	onChange = NULL;
	onClick = NULL;
	onActivate = NULL;
	
	sort = false;
	entry = NULL;
	
	g_typ=Type_gComboBox;
	store=gtk_list_store_new (1, G_TYPE_STRING);
	border=gtk_event_box_new();
	widget=gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
	g_object_unref (store);
	
	cell = gtk_cell_renderer_text_new ();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(widget), cell, true);
	gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(widget), cell, "text", 0, (void *)NULL);
	g_object_set(cell, "ypad", 0, (void *)NULL);

	realize(false);
	
	g_signal_connect(G_OBJECT(widget), "changed", G_CALLBACK(cb_click), (gpointer)this);
	
	setReadOnly(false);
}

void gComboBox::popup()
{
	gtk_combo_box_popup(GTK_COMBO_BOX(widget));
}

void gComboBox::setRealBackground(gColor color)
{
	gControl::setRealBackground(color);
	if (entry) 
		set_gdk_base_color(entry,color);
}


void gComboBox::setRealForeground(gColor color)
{
	gControl::setRealForeground(color);
	if (entry) 
		set_gdk_text_color(entry, color);
}

int gComboBox::count()
{
	return gStoreList_Count(STORE_LIST);

}

int gComboBox::index()
{
	GtkTreeModel *store;
	GtkTreeIter iter;
	
	store = gtk_combo_box_get_model(GTK_COMBO_BOX(widget));
	if (!gtk_tree_model_get_iter_first(store,&iter)) return 0;

	return gtk_combo_box_get_active (GTK_COMBO_BOX(widget));
}

char* gComboBox::itemText(int ind)
{
	GtkTreeModel *store;
	GtkTreeIter iter;
	char *ret=NULL;
	int len=0;
	
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
	return gt_free_later(ret);
}

int gComboBox::length()
{
	gchar *buf;
	
	if (!entry)
	{
		buf = itemText(index());
		if (!buf) 
			return 0;
		else
			return g_utf8_strlen(buf, -1);
	}
	else
		return gTextBox::length();
}

// char** gComboBox::list()
// {
// 	return gStoreList_ArrayList(STORE_LIST);
// }


bool gComboBox::sorted()
{
	return sort;
}

char* gComboBox::text()
{
	if (entry)
		return gTextBox::text();
	else
		return itemText(index());
}

void gComboBox::setIndex(int vl)
{
	if (vl < 0)
		vl = -1;
	else if (vl >= count()) 
		return;
		
	gtk_combo_box_set_active(GTK_COMBO_BOX(widget),vl);
}

void gComboBox::setItemText(int ind, const char *txt)
{
	int buf;
	
	if (ind<0) return;
	if (ind>=count()) return;
	buf=index();
	add(txt,ind);
	remove(ind+1);
	setIndex(buf);
	if (sort) gStoreList_sortData(STORE_LIST);
}

// void gComboBox::setList(char **vl)
// {
// 	GtkTreeIter iter;
// 
// 	gStoreList_setArrayList(STORE_LIST,vl);
// 	
// 	if (sort) gStoreList_sortData(STORE_LIST);
// 	if(gtk_tree_model_get_iter_first(GTK_TREE_MODEL(STORE_LIST),&iter))
// 		gtk_combo_box_set_active_iter(GTK_COMBO_BOX(widget),&iter);
// 
// }

void gComboBox::setReadOnly(bool vl)
{
	if (isReadOnly() == vl)
		return;
	
	if ((!vl) && (!entry) )
	{		
		entry = gtk_entry_new();
		gtk_container_add(GTK_CONTAINER(widget), entry);
		if (count())
			gTextBox::setText(itemText(index()));
		
		//gtk_widget_get_size_request(widget,&x,&y);
		gtk_widget_set_size_request(entry, width(), height());
		//set_gdk_base_color(txtentry,background());
		//set_gdk_text_color(txtentry,foreground());
		setBackground(background());
		setForeground(foreground());
		setFont(font());
		
		initEntry();
		g_signal_connect(G_OBJECT(entry),"key-press-event",G_CALLBACK(gcb_keypress),(gpointer)this);
		g_signal_connect(G_OBJECT(entry),"key-release-event",G_CALLBACK(gcb_keyrelease),(gpointer)this);
		
		gtk_widget_show(entry);
	}
	
	if ( vl && entry ) 
	{
		gtk_widget_destroy(entry);
		//gtk_container_remove(GTK_CONTAINER(widget), entry);
		entry = NULL;
	}
}

void gComboBox::setSorted(bool vl)
{
	sort=vl;
	if (sort) gStoreList_sortData(STORE_LIST);
}

void gComboBox::setText(const char *vl)
{
	setIndex(find(vl));
	if (entry)
		gTextBox::setText(vl);
}

void gComboBox::add(const char *vl,int pos)
{
	GtkTreeIter iter,curr;
	GtkListStore *store;
	bool set=false;
	int ind;

	if (pos < 0 || pos > count())
		pos = count();
	
	ind = index();
	
	if (!vl) 
		vl = "";
	
	store = (GtkListStore*)gtk_combo_box_get_model(GTK_COMBO_BOX(widget));
	
	if (!gtk_tree_model_get_iter_first(GTK_TREE_MODEL(store),&iter))
	{
		gtk_list_store_append (store, &iter);
		gtk_list_store_set (store, &iter,0,vl, -1);
		gtk_combo_box_set_active_iter(GTK_COMBO_BOX(widget),&iter);
	}
	else
	{
		set=gtk_combo_box_get_active_iter(GTK_COMBO_BOX(widget),&curr);
		gtk_list_store_insert(store,&iter,pos);
		gtk_list_store_set (store, &iter,0,vl, -1);
		if (set) gtk_combo_box_set_active_iter(GTK_COMBO_BOX(widget),&curr); 
		if (sort) gStoreList_sortData(STORE_LIST);
	}
	
	setIndex(ind);
}

void gComboBox::clear()
{
	gStoreList_Clear(STORE_LIST);
	gTextBox::clear();
	setIndex(-1);
}

int gComboBox::find(const char *ptr)
{
	return gStoreList_Find(STORE_LIST, ptr);
}

void gComboBox::remove(int pos)
{
	gStoreList_Remove(STORE_LIST,pos);
	if (sort) gStoreList_sortData(STORE_LIST);
}

void gComboBox::resize(int w, int h)
{
	gControl::resize(w,h);
	
	if (entry)
		gtk_widget_set_size_request(entry, width(), height());
}

void gComboBox::setFont(gFont *f)
{
	gControl::setFont(f);
	g_object_set(G_OBJECT(cell), "font-desc", font() ? font()->desc() : NULL, (void *)NULL);
	if (entry)
		gtk_widget_modify_font(entry, font() ? font()->desc() : NULL);
}

void gComboBox::setFocus()
{
	gControl::setFocus();
	if (entry && window()->isVisible())
		gtk_widget_grab_focus(entry);
}

int gComboBox::minimumHeight()
{
	GtkRequisition req;
	
	gtk_widget_size_request(widget, &req);
	if (entry)
		return req.height - 4;
	else
		return req.height;
}


bool gComboBox::isReadOnly()
{
	return entry == NULL;
}

