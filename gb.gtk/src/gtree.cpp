/***************************************************************************

  gtree.cpp

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

#include <gtree.h>
#include <gtk/gtk.h>
#include <string.h>
#include <stdlib.h>

/************************************************************

 gTreeCell
 
*************************************************************/
gTreeCell::gTreeCell()
{
	text=NULL;
	pic=NULL;
}

gTreeCell::~gTreeCell()
{
	if (text) free(text);
	if (pic) g_object_unref(G_OBJECT(pic));
}

void gTreeCell::setText(char *vl)
{
	if (text) { free(text); text=NULL; }
	if (!vl) return;
	text=(char*)g_malloc(sizeof(char)*(strlen(vl)+1));
	if (text) strcpy(text,vl);
}

void gTreeCell::setPixbuf(GdkPixbuf *vl)
{
	if (pic) { g_object_unref(G_OBJECT(pic)); pic=NULL; }
	pic=vl;
	if (pic) g_object_ref(G_OBJECT(pic));
}
/************************************************************

 gTreeRow
 
*************************************************************/

gTreeRow::gTreeRow(gTree *tree,unsigned long count,GtkTreeIter *iter,char *par)
{
	long ct=0;
	
	data=NULL;
	dataiter=iter;
	parent=tree;
	dataparent=par;
	
	while( ct<count) {  ct++; data=g_list_prepend (data,(gpointer)new gTreeCell()); }
	if (data) data=g_list_reverse(data);
}

gTreeRow::~gTreeRow()
{
	GList *iter=NULL;
	
	if (dataiter) gtk_tree_iter_free(dataiter);
	if (data) iter=g_list_first(data);
	
	while (iter)
	{
		delete (gTreeCell*)iter->data;
		iter=g_list_next(iter);
	}
	
	if (data) g_list_free(data);
}

void gTreeRow::add()
{
	data=g_list_append(data,(gpointer)new gTreeCell());
}

void gTreeRow::remove()
{
	GList *iter=NULL;
	if (!data) return;
	
	iter=g_list_last(data);
	delete (gTreeRow*)iter->data;
	g_list_remove(iter,iter->data);
}

long gTreeRow::children()
{
	GtkTreeIter iter;
	long ct=1;
	
	if (!gtk_tree_model_iter_children(GTK_TREE_MODEL(parent->store),&iter,dataiter)) return 0;
	
	while ( gtk_tree_model_iter_next(GTK_TREE_MODEL(parent->store),&iter) ) ct++;
	return ct;
}

void gTreeRow::update()
{
	GtkTreePath *path;
	
	path=gtk_tree_model_get_path(GTK_TREE_MODEL(parent->store),dataiter);
	if (path)
	{
		gtk_tree_model_row_changed (GTK_TREE_MODEL(parent->store),path,dataiter);
		gtk_tree_path_free(path);
	}
}

char* gTreeRow::parentKey()
{
	GtkTreeIter it;
	char *key;
	
	if (!gtk_tree_model_iter_parent(GTK_TREE_MODEL(parent->store),&it,dataiter)) return NULL;
	gtk_tree_model_get(GTK_TREE_MODEL(parent->store),&it,0,&key,-1);
	return key;
	
}

void gTreeRow::expand()
{
	GtkTreePath *path;
	
	path=gtk_tree_model_get_path(GTK_TREE_MODEL(parent->store),dataiter);
	if (path)
	{
		gtk_tree_view_expand_row (GTK_TREE_VIEW(parent->tree),path,false);
		gtk_tree_path_free(path);
	}
}

void gTreeRow::collapse()
{
	GtkTreePath *path;
	
	path=gtk_tree_model_get_path(GTK_TREE_MODEL(parent->store),dataiter);
	if (path)
	{
		gtk_tree_view_collapse_row (GTK_TREE_VIEW(parent->tree),path);
		gtk_tree_path_free(path);
	}
}

gTreeCell* gTreeRow::get(unsigned long ind)
{
	GList *iter=NULL;

	if (data) iter=g_list_nth(data,ind);
	
	if (!iter) return NULL;
	return (gTreeCell*)iter->data;
}

/************************************************************

 gTree
 
*************************************************************/

gboolean  gTree_equal(char *a,char *b)
{
	return !strcasecmp(a,b);
}


gTree::gTree()
{
	GtkTreeViewColumn *column;
	
	datakey=g_hash_table_new((GHashFunc)g_str_hash,(GEqualFunc)gTree_equal);
	
	store=gtk_tree_store_new(2,G_TYPE_STRING,GDK_TYPE_PIXBUF);
	tree=gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
	g_object_unref(G_OBJECT(store));
	
	rgraph=gtk_cell_renderer_pixbuf_new ();
	rtext=gtk_cell_renderer_text_new ();
}

gTree::~gTree()
{
}

long gTree::visibleWidth()
{
	GdkRectangle rect;
	gint w,h;
	
	gtk_tree_view_get_visible_rect (GTK_TREE_VIEW(tree),&rect);
	gtk_tree_view_tree_to_widget_coords(GTK_TREE_VIEW(tree),rect.width,rect.height,&w,&h);
    return w;
}

long gTree::visibleHeight()
{
	GdkRectangle rect;
	gint w,h;
	
	gtk_tree_view_get_visible_rect (GTK_TREE_VIEW(tree),&rect);
	gtk_tree_view_tree_to_widget_coords(GTK_TREE_VIEW(tree),rect.width,rect.height,&w,&h);
    return h;
}

char* gTree::cursor()
{
	GtkTreePath *path;
	GtkTreeIter iter;
	char *key;

	gtk_tree_view_get_cursor(GTK_TREE_VIEW(tree),&path,NULL);
	
	if (!path) return NULL;
	gtk_tree_model_get_iter(GTK_TREE_MODEL(store),&iter,path);
	gtk_tree_model_get(GTK_TREE_MODEL(store),&iter,0,&key,-1);
	gtk_tree_path_free(path);
	
	return key;
}

void gTree::setCursor(char *vl)
{
	GtkTreePath *path;
	gTreeRow *row=(gTreeRow*)g_hash_table_lookup(datakey,(gconstpointer)vl);
	
	if (!row) return;
	path=gtk_tree_model_get_path(GTK_TREE_MODEL(store),row->dataiter);
	if (path)
	{
		gtk_tree_view_set_cursor(GTK_TREE_VIEW(tree),path,NULL,false);
		gtk_tree_path_free(path);
	}
}

char* gTree::firstTopRow()
{
	GtkTreeIter iter;
	char *key;
	
	if (!gtk_tree_model_get_iter_first (GTK_TREE_MODEL(store),&iter)) return NULL;
	gtk_tree_model_get(GTK_TREE_MODEL(store),&iter,0,&key,-1);
	return key;
}

char* gTree::lastTopRow()
{
	GtkTreeIter iter;
	char *key;	
	gint count;
	gint bucle;
	
	if (!gtk_tree_model_get_iter_first (GTK_TREE_MODEL(store),&iter)) return NULL;
	count=gtk_tree_model_iter_n_children(GTK_TREE_MODEL(store),NULL);
	if (!count) return NULL;
	
	for (bucle=0;bucle<(count-1);bucle++)
		gtk_tree_model_iter_next(GTK_TREE_MODEL(store),&iter);
		
	gtk_tree_model_get(GTK_TREE_MODEL(store),&iter,0,&key,-1);
	return key;
}

char* gTree::nextRow(gTreeRow *row)
{
	GtkTreePath* path;
	GtkTreeIter iter;
	char *key=NULL;
	char *prev=NULL;
	
	if (!row) return NULL;
	
	gtk_tree_model_get(GTK_TREE_MODEL(store),row->dataiter,0,&prev,-1);
	if (!prev) return NULL;
	
	path=gtk_tree_model_get_path(GTK_TREE_MODEL(store),row->dataiter);
	if (path)
	{
		gtk_tree_path_next(path);
		if (gtk_tree_model_get_iter(GTK_TREE_MODEL(store),&iter,path))
			gtk_tree_model_get(GTK_TREE_MODEL(store),&iter,0,&key,-1);
		gtk_tree_path_free(path);
	}
	
	if (key)
	{
		if (!strcasecmp(key,prev)) { g_free(key); key=NULL; }
	}
	
	g_free(prev);
	return key;
}

char* gTree::prevRow(gTreeRow *row)
{
	GtkTreePath* path;
	GtkTreeIter iter;
	char *key=NULL;
	
	if (!row) return NULL;
	
	path=gtk_tree_model_get_path(GTK_TREE_MODEL(store),row->dataiter);
	if (path)
	{
		if (gtk_tree_path_prev(path))
			if (gtk_tree_model_get_iter(GTK_TREE_MODEL(store),&iter,path))
				gtk_tree_model_get(GTK_TREE_MODEL(store),&iter,0,&key,-1);
		gtk_tree_path_free(path);
	}
	return key;
}


void gTree_RemoveRow(char *key,gTreeRow *value,char *tree)
{
	if (!value->dataparent) return;
	if(!strcmp(tree,value->dataparent))
		value->parent->removeRow(key);
	
	return;
}

bool gTree::rowExists(char *key)
{
	return (bool)g_hash_table_lookup(datakey,(gconstpointer)key);
}

bool gTree::rowSelected(char *key)
{
	GtkTreeSelection *sel;
	gTreeRow *row=(gTreeRow*)g_hash_table_lookup(datakey,(gconstpointer)key);
	if (!row) return false;
	
	sel=gtk_tree_view_get_selection(GTK_TREE_VIEW(tree));
	if (!sel) return false;
	return gtk_tree_selection_iter_is_selected(sel,row->dataiter);
	
}

void gTree::setRowSelected(char *key,bool vl)
{
	GtkTreeSelection *sel;
	gTreeRow *row=(gTreeRow*)g_hash_table_lookup(datakey,(gconstpointer)key);
	if (!row) return;
	
	sel=gtk_tree_view_get_selection(GTK_TREE_VIEW(tree));
	if (!sel) return;
	if (vl)
		gtk_tree_selection_select_iter(sel,row->dataiter);
	else
		gtk_tree_selection_unselect_iter(sel,row->dataiter);
}

unsigned long gTree::rowCount()
{
	if (!tree) return 0;
	return g_hash_table_size(datakey);
}


void gTree_clearRows(char *key,gTreeRow *row,char **data)
{
	*data=key;
}

void gTree::clearRows()
{
	char *key=NULL;
	
	while (g_hash_table_find(datakey,(GHRFunc)gTree_clearRows,(gpointer)&key)) removeRow(key);
		
}

bool gTree::removeRow(char *key)
{
	gTreeRow *row;
	
	if (!tree) return false;
	if (!key)  return false;

	row=(gTreeRow*)g_hash_table_lookup (datakey,(gconstpointer)key);
	if (!row) return false;
	
	g_hash_table_foreach(datakey,(GHFunc)gTree_RemoveRow,(gpointer)key);
	
	g_hash_table_remove (datakey,(gconstpointer)key);
	gtk_tree_store_remove(store,row->dataiter);
	delete row;
	return true;
	
	
}

gTreeRow* gTree::getRow(char *key)
{
	return (gTreeRow*)g_hash_table_lookup (datakey,(gconstpointer)key);
}

gTreeRow* gTree::addRow(char *key,char *parent,char *after)
{
	GtkTreeIter iter;
	GtkTreePath *path;
	gTreeRow *row,*par,*aft=NULL;
	char *buf;

	if (!tree) return NULL;
	if (!key)  return NULL;
	
	if (g_hash_table_lookup (datakey,(gconstpointer)key)) return NULL;
	
	if (after)
	{
		aft=(gTreeRow*)g_hash_table_lookup (datakey,(gconstpointer)after);
		if (!aft) return NULL;	
	}
	
	if (!parent)
	{
		if (aft) gtk_tree_store_insert_after(store,&iter,NULL,aft->dataiter);
		else     gtk_tree_store_append (store, &iter, NULL);
		gtk_tree_store_set(store,&iter,0,key,-1);
		row=new gTreeRow(this,columnCount(),gtk_tree_iter_copy(&iter),NULL);
	}
	else
	{
		par=(gTreeRow*)g_hash_table_lookup (datakey,(gconstpointer)parent);
		if (!par) return NULL;
		if (aft) gtk_tree_store_insert_after(store,&iter,par->dataiter,aft->dataiter);
		else     gtk_tree_store_append (store,&iter,par->dataiter);
		gtk_tree_store_set(store,&iter,0,key,-1);
		row=new gTreeRow(this,columnCount(),gtk_tree_iter_copy(&iter),parent);
	}
	
	buf=(char*)g_malloc(sizeof(char)*(strlen(key)+1));
	if (!buf) return NULL;
	strcpy(buf,key);
	g_hash_table_insert(datakey,(gpointer)buf,(gpointer)row);
		
	path=gtk_tree_model_get_path(GTK_TREE_MODEL(store),&iter);
	buf=gtk_tree_path_to_string(path);
	gtk_tree_path_free(path);
	
	return row;
}

void tree_cell_text(GtkTreeViewColumn *col,GtkCellRenderer *cell,GtkTreeModel *md,GtkTreeIter *iter,gTree *Tr)
{
	GList *cols,*it=NULL;
	gTreeRow *row=NULL;
	gTreeCell *data;
	char *buf="";
	char *key;
	long ct=0;

	gtk_tree_model_get(md,iter,0,&key,-1);
	if (key) {
		row=(gTreeRow*)g_hash_table_lookup(Tr->datakey,(gpointer)key);
		g_free(key);
	}
	
	if (row)
	{
		cols=gtk_tree_view_get_columns(GTK_TREE_VIEW(Tr->tree));
		it=cols;
		while (it)
		{
			if ( it->data==(gpointer)col) break;
			it=g_list_next(it);
			ct++;
		}
		if (cols) g_list_free(cols);
		if (it)
		{
			if ( (data=row->get(ct)) )
			{
				if (data->text) buf=data->text;
			}
		}
	}

	g_object_set(G_OBJECT(cell),"text",buf,NULL);
}

void tree_cell_graph(GtkTreeViewColumn *col,GtkCellRenderer *cell,GtkTreeModel *md,GtkTreeIter *iter,gTree *Tr)
{
	GList *cols,*it=NULL;
	gTreeRow *row=NULL;
	gTreeCell *data;
	GdkPixbuf *buf=NULL;
	char *key;
	long ct=0;
	
	gtk_tree_model_get(md,iter,0,&key,-1);
	if (key) {
		row=(gTreeRow*)g_hash_table_lookup(Tr->datakey,(gpointer)key);
		g_free(key);
	}
	
	if (row)
	{
		cols=gtk_tree_view_get_columns(GTK_TREE_VIEW(Tr->tree));
		it=cols;
		while (it)
		{
			if ( it->data==(gpointer)col) break;
			it=g_list_next(it);
			ct++;
		}
		if (cols) g_list_free(cols);
		if (it)
		{
			if ( (data=row->get(ct)) )
			{
				buf=data->pic;
			}
		}
	}
	
	g_object_set(G_OBJECT(cell),"pixbuf",buf,NULL);

}

void gTree_addColumn(char *key,gTreeRow *value,gpointer data)
{
	value->add();
}

void gTree::addColumn()
{
	GtkTreeViewColumn *column;
	
	g_hash_table_foreach(datakey,(GHFunc)gTree_addColumn,NULL);
	
	column = gtk_tree_view_column_new();
	gtk_tree_view_column_pack_start(column,rgraph,false);
	gtk_tree_view_column_pack_start(column,rtext,true);
	gtk_tree_view_column_set_cell_data_func(column,rgraph,(GtkTreeCellDataFunc)tree_cell_graph,(gpointer)this,NULL);
	gtk_tree_view_column_set_cell_data_func(column,rtext,(GtkTreeCellDataFunc)tree_cell_text,(gpointer)this,NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(tree),column);
}

void gTree_removeColumn(char *key,gTreeRow *value,gpointer data)
{
	value->remove();
}

void gTree::removeColumn()
{
	GtkTreeViewColumn *column;
	long vl;
	
	if (!(vl=columnCount()) ) return;
	g_hash_table_foreach(datakey,(GHFunc)gTree_removeColumn,NULL);
	
	column=gtk_tree_view_get_column(GTK_TREE_VIEW(tree),vl-1);
	gtk_tree_view_remove_column(GTK_TREE_VIEW(tree),column);
}

unsigned long gTree::columnCount()
{
	GList *cols;
	long ret;
	
	if (!tree) return 0;
	cols=gtk_tree_view_get_columns(GTK_TREE_VIEW(tree));
	if (!cols) return 0;
	ret=(unsigned long)g_list_length(cols);
	g_list_free(cols);
	return ret;
}

GtkTreeViewColumn* gTree_findColumn(GtkTreeView *tree,unsigned long ind)
{
	GtkTreeViewColumn *col=NULL;
	GList *cols;
	GList *iter;
	
	if (!tree) return NULL;
	cols=gtk_tree_view_get_columns(GTK_TREE_VIEW(tree));
	if (!cols) return NULL;
	iter=g_list_nth(cols,(guint)ind);
	if (iter) {
		col=(GtkTreeViewColumn*)iter->data;
	}
	g_list_free(cols);
	return col;
}


const char* gTree::columnName(unsigned long ind)
{
	GtkTreeViewColumn *col=gTree_findColumn(GTK_TREE_VIEW(tree),ind);
	
	if (!col) return NULL;
	return (const char*)gtk_tree_view_column_get_title(col);

}

void gTree::setColumnName(unsigned long ind,char *vl)
{
	GtkTreeViewColumn *col=gTree_findColumn(GTK_TREE_VIEW(tree),ind);

	if (!col) return;
	gtk_tree_view_column_set_title(col,(const gchar*)vl);
}

bool gTree::columnVisible(unsigned long ind)
{
	GtkTreeViewColumn *col=gTree_findColumn(GTK_TREE_VIEW(tree),ind);
	
	if (!col) return NULL;
	return gtk_tree_view_column_get_visible(col);

}

void gTree::setColumnVisible(unsigned long ind,bool vl)
{
	GtkTreeViewColumn *col=gTree_findColumn(GTK_TREE_VIEW(tree),ind);

	if (!col) return;
	gtk_tree_view_column_set_visible(col,vl);
}

bool gTree::columnResizable(unsigned long ind)
{
	GtkTreeViewColumn *col=gTree_findColumn(GTK_TREE_VIEW(tree),ind);
	
	if (!col) return NULL;
	return gtk_tree_view_column_get_resizable(col);

}

void gTree::setColumnResizable(unsigned long ind,bool vl)
{
	GtkTreeViewColumn *col=gTree_findColumn(GTK_TREE_VIEW(tree),ind);

	if (!col) return;
	gtk_tree_view_column_set_resizable(col,vl);
}

bool gTree::headers()
{
	if (!tree) return false;
	return gtk_tree_view_get_headers_visible(GTK_TREE_VIEW(tree));
}

void gTree::setHeaders(bool vl)
{
	if (!tree) return;
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(tree),vl);
}
