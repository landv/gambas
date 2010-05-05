/***************************************************************************

  gtree.cpp

  (c) 2004-2006 - Daniel Campos Fernández <dcamposf@gmail.com>

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
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

***************************************************************************/

#include <gtk/gtk.h>
#include <string.h>
#include <stdlib.h>
#include "gmemory.h"
#include "gtree.h"
#include "gtreeview.h"
#include "giconview.h"
	
#if GTK_MAJOR_VERSION >= 2 && GTK_MINOR_VERSION >= 12

#define HIDDEN_COL 0

#else

#define HIDDEN_COL 1

#endif

static GtkTreeViewColumn* gt_tree_view_find_column(GtkTreeView *tree, int ind)
{
	GtkTreeViewColumn *col=NULL;
	GList *cols;
	GList *iter;
	
	if (!tree) return NULL;
	cols=gtk_tree_view_get_columns(GTK_TREE_VIEW(tree));
	if (!cols) return NULL;
	iter=g_list_nth(cols,(guint)ind + HIDDEN_COL);
	if (iter) {
		col=(GtkTreeViewColumn*)iter->data;
	}
	g_list_free(cols);
	
	return col;
}

static int gt_tree_view_find_index(GtkTreeView *tree, GtkTreeViewColumn *column)
{
	int index;
	GList *cols;
	GList *iter;
	
	if (!tree) return -1;
	cols = gtk_tree_view_get_columns(GTK_TREE_VIEW(tree));
	if (!cols) return -1;
	
	iter = cols;
	index = 0;
	if (HIDDEN_COL)
		iter = g_list_next(iter);
		
	while (iter)
	{
		if (iter->data == (gpointer)column) 
			break;
		iter = g_list_next(iter);
		index++;
	}
		
	g_list_free(cols);
	
	if (!iter)
		index = -1;
	
	return index;
}

/************************************************************

 gTreeCell
 
*************************************************************/

gTreeCell::gTreeCell()
{
	_text = NULL;
	_picture = NULL;
}

gTreeCell::~gTreeCell()
{
  setText(NULL);
  setPicture(NULL);
}

void gTreeCell::setText(const char *vl)
{
  if (_text)
    g_free(_text);
  
  _text = vl ? g_strdup(vl) : NULL;
}

void gTreeCell::setPicture(gPicture *vl)
{
  gPicture::assign(&_picture, vl);
}

/************************************************************

 gTreeRow
 
*************************************************************/

gTreeRow::gTreeRow(gTree *tr, char *key, GtkTreeIter *iter)
{
	int count;
	
	data = NULL;
	dataiter = iter;
	tree = tr;
	_key = key;
	_expanded = false;
	_editable = tr->isEditable();
	
	count = tree->columnCount();
	
	while (count > 0) 
	{  
		count--; 
		data = g_list_prepend(data, (gpointer)new gTreeCell()); 
	}
	
	if (data) 
		data = g_list_reverse(data);
	
	//fprintf(stderr, "new key: (%p) %s\n", _key, _key);
}

gTreeRow::~gTreeRow()
{
	GList *iter=NULL;
	
	if (tree->onRemove)
		(*tree->onRemove)(tree, _key);
	
	if (dataiter) gtk_tree_iter_free(dataiter);
	if (data) iter=g_list_first(data);
	
	while (iter)
	{
		delete (gTreeCell*)iter->data;
		iter=g_list_next(iter);
	}
	
	if (data) g_list_free(data);
	
	//fprintf(stderr, "free key: (%p) %s\n", _key, _key);
	g_free(_key);
}

void gTreeRow::add()
{
	data = g_list_append(data, (gpointer)new gTreeCell());
}

void gTreeRow::remove()
{
	GList *iter=NULL;
	gTreeCell *cell;
	
	if (!data) return;
	
	iter = g_list_last(data);
	cell = (gTreeCell *)iter->data;
	data = g_list_remove(data, cell);
	delete cell;
}

int gTreeRow::children()
{
	GtkTreeIter iter;
	int ct=1;
	
	if (!gtk_tree_model_iter_children(GTK_TREE_MODEL(tree->store),&iter,dataiter)) return 0;
	
	while ( gtk_tree_model_iter_next(GTK_TREE_MODEL(tree->store),&iter) ) ct++;
	return ct;
}

void gTreeRow::update()
{
	GtkTreePath *path;
	
	path=gtk_tree_model_get_path(GTK_TREE_MODEL(tree->store),dataiter);
	if (path)
	{
		gtk_tree_model_row_changed (GTK_TREE_MODEL(tree->store),path,dataiter);
		gtk_tree_path_free(path);
	}
}

/*char* gTreeRow::parentKey()
{
	GtkTreeIter it;
	char *key;
	
	if (!gtk_tree_model_iter_parent(GTK_TREE_MODEL(parent->store),&it,dataiter)) return NULL;
	gtk_tree_model_get(GTK_TREE_MODEL(parent->store),&it,0,&key,-1);
	return key;
}*/

void gTreeRow::setExpanded(bool ex)
{
	GtkTreePath *path;
	//char *current;
	
	//fprintf(stderr, "setExpanded: %s = %d\n", _key, ex);
		
	if (!gtk_tree_model_iter_has_child(GTK_TREE_MODEL(tree->store),dataiter))
	{
		_expanded = ex;
		return;
	}
	
	path = gtk_tree_model_get_path(GTK_TREE_MODEL(tree->store),dataiter);
	if (!path)
		return;

	if (ex)
		gtk_tree_view_expand_row(GTK_TREE_VIEW(tree->widget), path, false);
	else
		gtk_tree_view_collapse_row(GTK_TREE_VIEW(tree->widget), path);
	
	gtk_tree_path_free(path);
}

void gTreeRow::setExpanded()
{
	tree->view->lock();
	//_expanded = !_expanded;
	setExpanded(_expanded);
	tree->view->unlock();
}

bool gTreeRow::isExpanded()
{
// 	GtkTreePath *path;
// 	bool real = false;
// 	
// 	path = gtk_tree_model_get_path(GTK_TREE_MODEL(tree->store),dataiter);
// 	if (path)
// 	{
// 		real = gtk_tree_view_row_expanded(GTK_TREE_VIEW(tree->widget),path);
// 		gtk_tree_path_free(path);
// 	}
	
	//fprintf(stderr, "isExpanded: %s: %d (%d)\n", _key, _expanded, real);
	
	return _expanded;
}

void gTreeRow::updateExpanded(bool ex)
{
	//GtkTreePath *path;
	
	/*path = gtk_tree_model_get_path(GTK_TREE_MODEL(tree->store),dataiter);
	if (!path)
		return;
		
	_expanded = gtk_tree_view_row_expanded(GTK_TREE_VIEW(tree->widget),path);
	gtk_tree_path_free(path);*/
	
	_expanded = ex;
	
	//fprintf(stderr, "updateExpanded: %s = %d\n", _key, _expanded);
}

gTreeCell* gTreeRow::get(int ind)
{
	GList *iter;

	if (!data)
		return NULL;
		
	iter = g_list_nth(data, ind);
	if (!iter) 
		return NULL;
		
	return (gTreeCell*)iter->data;
}

void gTreeRow::ensureVisible()
{
	GtkTreePath *path;
	char *parentKey = parent();
	
	if (parentKey)
	{
		path = gtk_tree_model_get_path(GTK_TREE_MODEL(tree->store), (*tree)[parentKey]->dataiter);
		if (path)
		{
			gtk_tree_view_expand_to_path(GTK_TREE_VIEW(tree->widget), path);
			gtk_tree_path_free(path);			
		}
	}
	
	path = gtk_tree_model_get_path(GTK_TREE_MODEL(tree->store),dataiter);
	if (path)
	{
	  gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW(tree->widget), path, NULL, false, 0.0, 0.0);
		gtk_tree_path_free(path);			
	}
}

char *gTreeRow::next()
{
	GtkTreePath* path;
	
	path = gtk_tree_model_get_path(GTK_TREE_MODEL(tree->store), dataiter);
	if (!path)
		return NULL;
	
	gtk_tree_path_next(path);
	return tree->pathToKey(path);
}

char *gTreeRow::prev()
{
	GtkTreePath* path;
	
	path = gtk_tree_model_get_path(GTK_TREE_MODEL(tree->store), dataiter);
	if (!path)
		return NULL;
	
	gtk_tree_path_prev(path);
	return tree->pathToKey(path);
}

char *gTreeRow::child()
{
	GtkTreePath* path;
	
	path = gtk_tree_model_get_path(GTK_TREE_MODEL(tree->store), dataiter);
	if (!path)
		return NULL;
	
	gtk_tree_path_down(path);
	return tree->pathToKey(path);
}

char *gTreeRow::last()
{
	GtkTreePath* path;
	GtkTreeIter iter;
	int count;
	
	path = gtk_tree_model_get_path(GTK_TREE_MODEL(tree->store), dataiter);
	if (!path)
		return NULL;
	
	if (!gtk_tree_model_get_iter(GTK_TREE_MODEL(tree->store), &iter, path)) 
		return NULL;
		
	gtk_tree_path_free(path);
		
	count = gtk_tree_model_iter_n_children(GTK_TREE_MODEL(tree->store), NULL);
	if (!count) return NULL;
	
	while (--count)
		gtk_tree_model_iter_next(GTK_TREE_MODEL(tree->store), &iter);
	
	return tree->iterToKey(&iter);
}

char *gTreeRow::parent()
{
	GtkTreeIter *iter = gtk_tree_iter_copy(dataiter);
	char *key;
	//GtkTreePath* path;
	
	if (!gtk_tree_model_iter_parent(GTK_TREE_MODEL(tree->store), iter, dataiter))
		key = NULL;
	else
		key = tree->iterToKey(iter);
	
	gtk_tree_iter_free(iter);
	return key;
	
	/*path = gtk_tree_model_get_path(GTK_TREE_MODEL(tree->store), dataiter);
	if (!path)
		return NULL;
	
	if (!gtk_tree_path_up(path))
		return NULL;
		
	return tree->pathToKey(path);*/
}

char *gTreeRow::above()
{
	char *key, *key2;
	
	key = prev();
	if (!key)
		return parent();
		
	for(;;)
	{
		key2 = (*tree)[key]->child();
		if (!key2)
			break;
		
		key = key2;
		
		for(;;)
		{
			key2 = (*tree)[key]->next();
			if (!key2)
				break;
			key = key2;
		}
	}
	
	return key;
}

char *gTreeRow::below()
{
	char *key, *key2;
	
	key = child();
	if (key)
		return key;
		
	key = next();
	if (key)
		return key;
	
	key = parent();
	
	for(;;)
	{
		if (!key)
			return NULL;
		
		key2 = (*tree)[key]->next();
		if (key2)
			return key2;
		
		key = (*tree)[key]->parent();
	}
}


void gTreeRow::rect(int *x, int *y, int *w, int *h)
{
	GtkTreePath* path;
	GdkRectangle rect;
	GtkTreeViewColumn *column;
	gint depth;
	gint size;
	gint margin;
	
	path = gtk_tree_model_get_path(GTK_TREE_MODEL(tree->store), dataiter);
	if (!path)
		return;
	
	column = gt_tree_view_find_column(GTK_TREE_VIEW(tree->widget), tree->columnCount() - 1);
	gtk_tree_view_get_cell_area(GTK_TREE_VIEW(tree->widget), path, column, &rect);
	depth = gtk_tree_path_get_depth(path);
	gtk_tree_path_free(path);

  gtk_widget_style_get (tree->widget,
			"expander-size", &size,
			"vertical-separator", &margin,
			(void *)NULL);
			
	size += 4; // Constant that is hard-coded into GTK+ source code

	if (!tree->hasExpanders())
		depth--;

	*x = depth * size;
	*w = rect.x + rect.width - *x;
	*h = rect.height + margin;
	*y = rect.y;
}

void gTreeRow::moveFirst()
{
	gtk_tree_store_move_after(tree->store, dataiter, NULL);
}

void gTreeRow::moveAfter(char *key)
{
	gTreeRow *row;
	
	if (!key || !*key)
	{
		moveFirst();
		return;
	}
	
	row = tree->getRow(key);
	if (!row)
		return;
	if (strcmp(row->parent(), parent()))
		return;
	gtk_tree_store_move_after(tree->store, dataiter, row->dataiter);
}

void gTreeRow::moveLast()
{
	gtk_tree_store_move_before(tree->store, dataiter, NULL);
}

void gTreeRow::moveBefore(char *key)
{
	gTreeRow *row;
	
	if (!key || !*key)
	{
		moveLast();
		return;
	}
	
	row = tree->getRow(key);
	if (!row)
		return;
	if (strcmp(row->parent(), parent()))
		return;
	gtk_tree_store_move_before(tree->store, dataiter, row->dataiter);
}

void gTreeRow::startRename()
{
	GtkTreePath* path;
	
	path = gtk_tree_model_get_path(GTK_TREE_MODEL(tree->store), dataiter);
	if (!path)
		return;
	
	//tree->view->setFocus();
	gtk_tree_view_set_cursor(GTK_TREE_VIEW(tree->widget), path, gt_tree_view_find_column(GTK_TREE_VIEW(tree->widget), 0), true);
	gtk_tree_path_free(path);
}


/************************************************************

 gTree
 
*************************************************************/

static gboolean gTree_equal(char *a,char *b)
{
	return !strcasecmp(a,b);
}

static void cb_tree_edited(GtkCellRendererText *renderer, gchar *spath, gchar *new_text, gTree *tree)
{
	gTreeView *view = tree->view;
	
	if (!tree->_edited_row)
		return;
		
	view->setItemText(tree->_edited_row, new_text);
	view->emit(SIGNAL(view->onRename), tree->_edited_row);
	tree->_edited_row = NULL;	
}

static void cb_tree_started(GtkCellRenderer *renderer, GtkCellEditable *editable, gchar *spath, gTree *tree)
{
	GtkTreePath *path = gtk_tree_path_new_from_string(spath);
	if (path)
		tree->_edited_row = tree->pathToKey(path);
	else
		tree->_edited_row = NULL;	
}

static void cb_tree_canceled(GtkCellRendererText *renderer, gTree *tree)
{
	gTreeView *view = tree->view;
	view->emit(SIGNAL(view->onCancel), tree->_edited_row);
}

static void cb_column_clicked(GtkTreeViewColumn *col, gTree *tree)
{
	int index = gt_tree_view_find_index(GTK_TREE_VIEW(tree->widget), col);
	
	if (index == tree->getSortColumn())
		tree->setSortAscending(!tree->isSortAscending());
	else
		tree->setSortColumn(index);
}

static gint tree_compare(GtkTreeModel *model, GtkTreeIter *a, GtkTreeIter *b, gTree *tree)
{
	bool def = true;
	int comp;
	char *ka = tree->iterToKey(a);
	char *kb = tree->iterToKey(b);
	const char *ta, *tb;
	
	//fprintf(stderr, "ka = '%s' kb = '%s'\n", ka, kb);
	
	if (tree->view && tree->view->onCompare)
		def = tree->view->onCompare(tree->view, ka, kb, &comp);
	
	if (def)
	{
		ta = tree->getRow(ka)->get(tree->getSortColumn())->text();
		if (!ta) ta = "";
		tb = tree->getRow(kb)->get(tree->getSortColumn())->text();
		if (!tb) tb = "";
		
		//fprintf(stderr, "ta = '%s' tb = '%s'\n", ta, tb);
	
		comp = g_utf8_collate(ta, tb);
	}
	
	if (!tree->isSortAscending())
		comp = (-comp);
	
	return comp;
}


void gTree::showExpanders()
{
	#if GTK_MAJOR_VERSION >= 2 && GTK_MINOR_VERSION >= 12
	gtk_tree_view_set_show_expanders(GTK_TREE_VIEW(widget), true);
	#else
	gtk_tree_view_set_expander_column(GTK_TREE_VIEW(widget), gt_tree_view_find_column(GTK_TREE_VIEW(widget), 0));
	#endif
	_expander = true;
}

void gTree::lock()
{
	if (view) view->lock();
}

void gTree::unlock()
{
	if (view) view->unlock();
}


gTree::gTree(gTreeView *v)
{
	onRemove = NULL;
	
	datakey = g_hash_table_new((GHashFunc)g_str_hash,(GEqualFunc)gTree_equal);
	
	if (v)
	{
		store = gtk_tree_store_new(1, G_TYPE_POINTER);
		widget = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
		g_object_unref(G_OBJECT(store));
		
		view = v;
	}
	else // combo-box !
	{
		store = gtk_tree_store_new(2, G_TYPE_STRING, G_TYPE_POINTER);
		view = NULL;
		widget = NULL;
	}
	
	_editable = false;
	_resizable = false;
	_auto_resize = false;
	_edited_row = NULL;
	_sorted = false;
	_ascending = true;
	_sort_column = 0;
	_init_sort = false;
	_sort_dirty = false;
	_expander = false;
	
	if (view)
	{
		#if GTK_MAJOR_VERSION >= 2 && GTK_MINOR_VERSION >= 12
		gtk_tree_view_set_show_expanders(GTK_TREE_VIEW(widget), false);
		#else
		GtkTreeViewColumn *column = gtk_tree_view_column_new();
		//gtk_tree_view_column_pack_start(column,rgraph,false);
		//gtk_tree_view_column_pack_start(column,rtext,true);
		//gtk_tree_view_column_set_cell_data_func(column,rgraph,(GtkTreeCellDataFunc)tree_cell_graph,(gpointer)this,NULL);
		//gtk_tree_view_column_set_cell_data_func(column,rtext,(GtkTreeCellDataFunc)tree_cell_text,(gpointer)this,NULL);
		gtk_tree_view_column_set_visible(column, false);
		gtk_tree_view_append_column(GTK_TREE_VIEW(widget), column);
		gtk_tree_view_set_expander_column(GTK_TREE_VIEW(widget), column);
		#endif
			
		rgraph = gtk_cell_renderer_pixbuf_new();
		g_object_ref_sink(rgraph);
		rtext = gtk_cell_renderer_text_new();
		g_object_ref_sink(rtext);
	
		g_signal_connect(G_OBJECT(rtext), "edited", G_CALLBACK(cb_tree_edited), (gpointer)this);
		g_signal_connect(G_OBJECT(rtext), "editing-started", G_CALLBACK(cb_tree_started), (gpointer)this);
		g_signal_connect(G_OBJECT(rtext), "editing-canceled", G_CALLBACK(cb_tree_canceled), (gpointer)this);
		//addColumn();
		setAutoResize(true);
	}
}

gTree::~gTree()
{
	clear();
	g_hash_table_destroy(datakey);
	if (view)
	{
		g_object_unref(rgraph);
		g_object_unref(rtext);
	}
}

void gTree::clear()
{
	char *key;
	
	lock();
	
	while ((key = firstRow()))
    removeRow(key);
  for (int i = 0; i < columnCount(); i++)
  {
  	setColumnWidth(i, 16);
  	setColumnWidth(i, -1);
  }
  
  unlock();
}

void gTree::clear(char *parent)
{
	gTreeRow *row;
	char *child;
	
	row = (*this)[parent];
	if (!row)
		return;
		
	lock();
	while((child = row->child()))
		removeRow(child);
	unlock();
}


int gTree::visibleWidth()
{
	GdkRectangle rect;
	gint w,h;
	
	gtk_tree_view_get_visible_rect (GTK_TREE_VIEW(widget),&rect);
	gtk_tree_view_tree_to_widget_coords(GTK_TREE_VIEW(widget),rect.width,rect.height,&w,&h);
    return w;
}

int gTree::visibleHeight()
{
	GdkRectangle rect;
	gint w,h;
	
	gtk_tree_view_get_visible_rect (GTK_TREE_VIEW(widget),&rect);
	gtk_tree_view_tree_to_widget_coords(GTK_TREE_VIEW(widget),rect.width,rect.height,&w,&h);
    return h;
}

char *gTree::iterToKey(GtkTreeIter *iter)
{
	char *key;
	gtk_tree_model_get(GTK_TREE_MODEL(store), iter, view ? 0 : 1, &key, -1);
	return key;
}

char *gTree::pathToKey(GtkTreePath *path, bool free)
{
	GtkTreeIter iter;
	char *key;
	
	if (!path) return NULL;
	
	if (gtk_tree_model_get_iter(GTK_TREE_MODEL(store), &iter, path))		
		key = iterToKey(&iter);
	else
		key = NULL;
		
	if (free) gtk_tree_path_free(path);	
	
	return key;
}

char* gTree::cursor()
{
	GtkTreePath *path;

	gtk_tree_view_get_cursor(GTK_TREE_VIEW(widget), &path, NULL);
	return pathToKey(path);
}

void gTree::setCursor(char *vl)
{
	GtkTreePath *path;
	gTreeRow *row = getRow(vl);
	
	if (!row) return;
	path = gtk_tree_model_get_path(GTK_TREE_MODEL(store),row->dataiter);
	if (path)
	{
		gtk_tree_view_set_cursor(GTK_TREE_VIEW(widget),path,NULL,false);
		gtk_tree_path_free(path);
	}
}

char *gTree::firstRow()
{
	GtkTreeIter iter;
	
	if (!gtk_tree_model_get_iter_first (GTK_TREE_MODEL(store),&iter)) return NULL;
	return iterToKey(&iter);
}

char *gTree::lastRow()
{
	GtkTreeIter iter;
	gint count;
	
	if (!gtk_tree_model_get_iter_first(GTK_TREE_MODEL(store), &iter)) 
		return NULL;
		
	count = gtk_tree_model_iter_n_children(GTK_TREE_MODEL(store), NULL);
	if (!count) return NULL;
	
	while (--count)
		gtk_tree_model_iter_next(GTK_TREE_MODEL(store), &iter);
	
	return iterToKey(&iter);
}

bool gTree::rowExists(char *key)
{
	return (bool)(key && *key && g_hash_table_lookup(datakey,(gconstpointer)key));
}

bool gTree::rowSelected(char *key)
{
	GtkTreeSelection *sel;
	gTreeRow *row=(gTreeRow*)g_hash_table_lookup(datakey,(gconstpointer)key);
	if (!row) return false;
	
	sel=gtk_tree_view_get_selection(GTK_TREE_VIEW(widget));
	if (!sel) return false;
	return gtk_tree_selection_iter_is_selected(sel,row->dataiter);
	
}

void gTree::setRowSelected(char *key,bool vl)
{
	GtkTreeSelection *sel;
	gTreeRow *row=(gTreeRow*)g_hash_table_lookup(datakey,(gconstpointer)key);
	if (!row) return;
	
	sel=gtk_tree_view_get_selection(GTK_TREE_VIEW(widget));
	if (!sel) return;
	if (vl)
		gtk_tree_selection_select_iter(sel,row->dataiter);
	else
		gtk_tree_selection_unselect_iter(sel,row->dataiter);
}

bool gTree::isRowEditable(char *key)
{
	gTreeRow *row = getRow(key);
	if (!row)
		return false;
	else
		return row->isEditable();
}

void gTree::setRowEditable(char *key, bool vl)
{
	gTreeRow *row = getRow(key);
	if (!row)
		return;
	
	row->setEditable(vl);
}

int gTree::rowCount()
{
	return g_hash_table_size(datakey);
}


bool gTree::removeRow(char *key)
{
	gTreeRow *row;
	char *child;
	
	if (!key || !*key)  return false;

	row=(gTreeRow*)g_hash_table_lookup(datakey, (gconstpointer)key);
	if (!row) return false;
	
	//fprintf(stderr, "gTree::removeRow: '%s'\n", key);
	
	while((child = row->child()))
		removeRow(child);
	
	//fprintf(stderr, "gTree::removeRow: '%s' removed\n", key);

	g_hash_table_remove (datakey, (gconstpointer)key);
	gtk_tree_store_remove(store, row->dataiter);
	delete row;
	
	return true;
}

gTreeRow* gTree::getRow(char *key) const
{
	if (!key)
		return NULL;
	else
		return (gTreeRow*)g_hash_table_lookup (datakey,(gconstpointer)key);
}

gTreeRow* gTree::addRow(char *key,char *parent,char *after)
{
	GtkTreeIter iter;
	gTreeRow *row,*par,*aft=NULL;
	char *buf;

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
	}
	else
	{
		par=(gTreeRow*)g_hash_table_lookup (datakey,(gconstpointer)parent);
		if (!par) return NULL;
		if (aft) gtk_tree_store_insert_after(store,&iter,par->dataiter,aft->dataiter);
		else     gtk_tree_store_append (store,&iter,par->dataiter);
	}
	
	buf = g_strdup(key); // Will be freed by ~gTreeRow()
	row = new gTreeRow(this, buf, gtk_tree_iter_copy(&iter));
	g_hash_table_insert(datakey,(gpointer)buf,(gpointer)row);
	gtk_tree_store_set(store, &iter, view ? 0 : 1, buf, -1);
  
  if (parent)
  {
  	getRow(parent)->setExpanded();
  	showExpanders();
  }
  
	return row;
}

static void tree_cell_text(GtkTreeViewColumn *col,GtkCellRenderer *cell,GtkTreeModel *md,GtkTreeIter *iter,gTree *Tr)
{
	gTreeRow *row=NULL;
	gTreeCell *data;
	const char *buf = "";
	char *key;
	int index = -1;
	double align;

	key = Tr->iterToKey(iter);
	if (key)
		row=(gTreeRow*)g_hash_table_lookup(Tr->datakey,(gpointer)key);
	
	if (row)
	{
		index = gt_tree_view_find_index(GTK_TREE_VIEW(Tr->widget), col);
		data = row->get(index);
		if (data)
		{
			if (data->text()) 
				buf	=	data->text();
		}
	}

	align = gtk_tree_view_column_get_alignment(col);

	g_object_set(G_OBJECT(cell),
		"text", buf,
		"editable", index == 0 && row->isEditable(),
		"xalign", align,
		(void *)NULL);
}

static void tree_cell_graph(GtkTreeViewColumn *col,GtkCellRenderer *cell,GtkTreeModel *md,GtkTreeIter *iter,gTree *Tr)
{
	gTreeRow *row=NULL;
	gTreeCell *data;
	GdkPixbuf *buf=NULL;
	char *key;
	
	key = Tr->iterToKey(iter);
	if (key)
		row=(gTreeRow*)g_hash_table_lookup(Tr->datakey,(gpointer)key);
	
	if (row)
	{
		data = row->get(gt_tree_view_find_index(GTK_TREE_VIEW(Tr->widget), col));
		if (data)
		{
			buf = data->picture() ? data->picture()->getPixbuf() : NULL;
		}
	}
	
	g_object_set(G_OBJECT(cell),"pixbuf",buf,(void *)NULL);

}

static void gTree_addColumn(char *key,gTreeRow *value,gpointer data)
{
	value->add();
}

void gTree::addColumn()
{
	GtkTreeViewColumn *column;
	
	g_hash_table_foreach(datakey,(GHFunc)gTree_addColumn,NULL);
	
	if (view)
	{
		column = gtk_tree_view_column_new();
		
		gtk_tree_view_column_pack_start(column,rgraph,false);
		gtk_tree_view_column_pack_start(column,rtext,true);
		gtk_tree_view_column_set_cell_data_func(column,rgraph,(GtkTreeCellDataFunc)tree_cell_graph,(gpointer)this,NULL);
		gtk_tree_view_column_set_cell_data_func(column,rtext,(GtkTreeCellDataFunc)tree_cell_text,(gpointer)this,NULL);
		
		gtk_tree_view_column_set_resizable(column, isResizable());
		gtk_tree_view_column_set_sizing(column, isAutoResize() ? GTK_TREE_VIEW_COLUMN_AUTOSIZE : GTK_TREE_VIEW_COLUMN_FIXED);
			
		gtk_tree_view_append_column(GTK_TREE_VIEW(widget),column);
		g_signal_connect(G_OBJECT(column), "clicked", G_CALLBACK(cb_column_clicked), (gpointer)this);	
		updateSort();
	}
}

static void gTree_removeColumn(char *key,gTreeRow *value,gpointer data)
{
	value->remove();
}

void gTree::removeColumn()
{
	GtkTreeViewColumn *column;
	int vl;
	
	if (!(vl=columnCount()) ) return;
	g_hash_table_foreach(datakey,(GHFunc)gTree_removeColumn,NULL);
	
	if (view)
	{
		//column=gtk_tree_view_get_column(GTK_TREE_VIEW(widget),vl-1);
		column = gt_tree_view_find_column(GTK_TREE_VIEW(widget), vl-1);
		gtk_tree_view_remove_column(GTK_TREE_VIEW(widget),column);
		updateSort();
	}
}

int gTree::columnCount()
{
	GList *cols;
	int ret;
	
	if (!view) return 1;
	
	if (!widget) return 0;
	cols=gtk_tree_view_get_columns(GTK_TREE_VIEW(widget));
	if (!cols) return 0;
	ret=g_list_length(cols) - HIDDEN_COL;
	g_list_free(cols);
	return ret;
}


char* gTree::columnName(int ind)
{
	GtkTreeViewColumn *col=gt_tree_view_find_column(GTK_TREE_VIEW(widget),ind);
	
	if (!col) return NULL;
	return (char *)gtk_tree_view_column_get_title(col);
}

void gTree::setColumnName(int ind,char *vl)
{
	GtkTreeViewColumn *col=gt_tree_view_find_column(GTK_TREE_VIEW(widget),ind);

	if (!col) return;
	gtk_tree_view_column_set_title(col,(const gchar*)vl);
}

bool gTree::columnVisible(int ind)
{
	GtkTreeViewColumn *col=gt_tree_view_find_column(GTK_TREE_VIEW(widget),ind);
	
	if (!col) return false;
	return gtk_tree_view_column_get_visible(col);

}

void gTree::setColumnVisible(int ind,bool vl)
{
	GtkTreeViewColumn *col=gt_tree_view_find_column(GTK_TREE_VIEW(widget),ind);

	if (!col) return;
	gtk_tree_view_column_set_visible(col,vl);
}

bool gTree::columnResizable(int ind)
{
	GtkTreeViewColumn *col=gt_tree_view_find_column(GTK_TREE_VIEW(widget),ind);
	
	if (!col) return false;
	return gtk_tree_view_column_get_resizable(col);
}

void gTree::setColumnResizable(int ind, bool vl)
{
	GtkTreeViewColumn *col=gt_tree_view_find_column(GTK_TREE_VIEW(widget),ind);
	if (!col) return;
	gtk_tree_view_column_set_resizable(col,vl);
}

int gTree::columnAlignment(int ind)
{
	GtkTreeViewColumn *col = gt_tree_view_find_column(GTK_TREE_VIEW(widget), ind);
	
	if (!col) 
		return ALIGN_LEFT;
	else
		return gt_to_alignment(gtk_tree_view_column_get_alignment(col));
}

void gTree::setColumnAlignment(int ind, int align)
{
	GtkTreeViewColumn *col = gt_tree_view_find_column(GTK_TREE_VIEW(widget), ind);
	
	if (!col) 
		return;
		
	gtk_tree_view_column_set_alignment(col, gt_from_alignment(align));
}

int gTree::columnWidth(int ind)
{
	GtkTreeViewColumn *col = gt_tree_view_find_column(GTK_TREE_VIEW(widget), ind);
	
	if (!col) 
		return 0;
	else
		return gtk_tree_view_column_get_fixed_width(col);
}

void gTree::setColumnWidth(int ind, int w)
{
	GtkTreeViewColumn *col = gt_tree_view_find_column(GTK_TREE_VIEW(widget), ind);
	
	if (!col) 
		return;
		
	if (w > 0)
	{
		gtk_tree_view_column_set_sizing(col, GTK_TREE_VIEW_COLUMN_FIXED);
		gtk_tree_view_column_set_fixed_width(col, w);
	}
	else
		gtk_tree_view_column_set_sizing(col, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
}

bool gTree::headers()
{
	if (!widget) return false;
	return gtk_tree_view_get_headers_visible(GTK_TREE_VIEW(widget));
}

void gTree::setHeaders(bool vl)
{
	if (!widget) return;
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(widget),vl);
}

void gTree::setResizable(bool vl)
{
	int i;
	
	for (i = 0; i < columnCount(); i++)
		gtk_tree_view_column_set_resizable(gt_tree_view_find_column(GTK_TREE_VIEW(widget), i), vl);
		
	_resizable = vl;
}

void gTree::setAutoResize(bool vl)
{
	int i;
	
	for (i = 0; i < columnCount(); i++)
		gtk_tree_view_column_set_sizing(gt_tree_view_find_column(GTK_TREE_VIEW(widget), i), vl ? GTK_TREE_VIEW_COLUMN_AUTOSIZE : GTK_TREE_VIEW_COLUMN_FIXED);
		
	_auto_resize = vl;
}

void gTree::selectAll()
{
	GtkTreeSelection *sel;
	
	sel=gtk_tree_view_get_selection(GTK_TREE_VIEW(widget));
	if (!sel) 
		return;
	
	gtk_tree_selection_select_all(sel);
}

void gTree::unselectAll()
{
	GtkTreeSelection *sel;
	
	sel=gtk_tree_view_get_selection(GTK_TREE_VIEW(widget));
	if (!sel) 
		return;
	
	gtk_tree_selection_unselect_all(sel);
}

void gTree::setSorted(bool v)
{
	if (v == _sorted)
		return;
		
	_sorted = v;
	_sort_column = v ? 0 : -1;
	if (!_sorted)
	{
		gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(store), GTK_TREE_SORTABLE_UNSORTED_SORT_COLUMN_ID, GTK_SORT_ASCENDING);
		gtk_tree_sortable_set_default_sort_func(GTK_TREE_SORTABLE(store), NULL, NULL, NULL);
	}
	updateSort();
}

void gTree::setSortColumn(int v)
{
	if (_sort_column < 0)
		setSorted(false);
	else
	{
		_sort_column = v;
		_ascending = true;
		updateSort();
	}
}

void gTree::setSortAscending(bool v)
{
	_ascending = v;
	updateSort();
}

void gTree::sort()
{
	if (!_sorted)
		return;
	
	// BM: force the store to be sorted
	gtk_tree_sortable_set_default_sort_func(GTK_TREE_SORTABLE(store), (GtkTreeIterCompareFunc)tree_compare, this, NULL);
	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(store), GTK_TREE_SORTABLE_DEFAULT_SORT_COLUMN_ID, GTK_SORT_ASCENDING);
	_sort_dirty = false;
}

void gTree::updateSort()
{
	int i;
	bool s;
	
	if (view)
	{
		if (_sort_column >= columnCount())
			_sort_column = 0;
		
		for (i = 0; i < columnCount(); i++)
		{
			GtkTreeViewColumn *col = gt_tree_view_find_column(GTK_TREE_VIEW(widget), i);
			if (!_sorted)
			{
				gtk_tree_view_column_set_sort_indicator(col, false);
				gtk_tree_view_column_set_clickable(col, false);
				continue;
			}
			gtk_tree_view_column_set_clickable(col, true);		
			s = i == _sort_column;
			gtk_tree_view_column_set_sort_indicator(col, s);
			if (s)
				gtk_tree_view_column_set_sort_order(col, _ascending ? GTK_SORT_ASCENDING : GTK_SORT_DESCENDING);
		}	
	}
	
	sortLater();
}

static gboolean tree_sort_later(gTree *tree)
{
	tree->sort();
	return FALSE;
}

void gTree::sortLater()
{
	if (!isSorted() || _sort_dirty)
		return;
		
	_sort_dirty = true;
	g_timeout_add(0, (GSourceFunc)tree_sort_later, this);
}

/************************************************************

 gIconRow
 
*************************************************************/

gIconRow::gIconRow(gIcon *tr, char *key, GtkTreeIter *iter)
{
	data = new gTreeCell();
	dataiter = iter;
	tree = tr;
	_key = key;
	_editable = tr->isEditable();
}

gIconRow::~gIconRow()
{
	if (tree->onRemove)
		(*tree->onRemove)(tree, _key);
	
	if (dataiter) gtk_tree_iter_free(dataiter);
	delete data;	
	//fprintf(stderr, "free key: (%p) %s\n", _key, _key);
	g_free(_key);
}

void gIconRow::update()
{
	GtkTreePath *path;
	
	path=gtk_tree_model_get_path(GTK_TREE_MODEL(tree->store),dataiter);
	if (path)
	{
		gtk_tree_model_row_changed (GTK_TREE_MODEL(tree->store),path,dataiter);
		gtk_tree_path_free(path);
	}
}

void gIconRow::ensureVisible()
{
	/*GtkTreePath *path;
	
	path = gtk_tree_model_get_path(GTK_TREE_MODEL(tree->store),dataiter);
	if (path)
	{
	  gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW(tree->widget), path, NULL, false, 0.0, 0.0);
		gtk_tree_path_free(path);			
	}*/
}

char *gIconRow::next()
{
	GtkTreePath* path;
	
	path = gtk_tree_model_get_path(GTK_TREE_MODEL(tree->store), dataiter);
	if (!path)
		return NULL;
	
	gtk_tree_path_next(path);
	return tree->pathToKey(path);
}

char *gIconRow::prev()
{
	GtkTreePath* path;
	
	path = gtk_tree_model_get_path(GTK_TREE_MODEL(tree->store), dataiter);
	if (!path)
		return NULL;
	
	gtk_tree_path_prev(path);
	return tree->pathToKey(path);
}

char *gIconRow::last()
{
	GtkTreePath* path;
	GtkTreeIter iter;
	int count;
	
	path = gtk_tree_model_get_path(GTK_TREE_MODEL(tree->store), dataiter);
	if (!path)
		return NULL;
	
	if (!gtk_tree_model_get_iter(GTK_TREE_MODEL(tree->store), &iter, path)) 
		return NULL;
		
	gtk_tree_path_free(path);
		
	count = gtk_tree_model_iter_n_children(GTK_TREE_MODEL(tree->store), NULL);
	if (!count) return NULL;
	
	while (--count)
		gtk_tree_model_iter_next(GTK_TREE_MODEL(tree->store), &iter);
	
	return tree->iterToKey(&iter);
}


void gIconRow::rect(int *x, int *y, int *w, int *h)
{
	GtkTreePath* path;
	//GdkRectangle rect;
	//GtkTreeViewColumn *column;
	//int depth;
	
	path = gtk_tree_model_get_path(GTK_TREE_MODEL(tree->store), dataiter);
	if (!path)
		return;
	
	/*column = gt_tree_view_find_column(GTK_TREE_VIEW(tree->widget), tree->columnCount() - 1);
	gtk_tree_view_get_cell_area(GTK_TREE_VIEW(tree->widget), path, column, &rect);
	depth = gtk_tree_path_get_depth(path);*/
	gtk_tree_path_free(path);
	
	// TODO
	// *x = depth * 12;
	// *w = rect.x + rect.width - *x;
	// *h = rect.height;
	// *y = rect.y;
}

void gIconRow::moveFirst()
{
	gtk_list_store_move_after(tree->store, dataiter, NULL);
}

void gIconRow::moveAfter(char *key)
{
	gIconRow *row;
	
	if (!key || !*key)
	{
		moveFirst();
		return;
	}
	
	row = tree->getRow(key);
	if (!row)
		return;
	gtk_list_store_move_after(tree->store, dataiter, row->dataiter);
}

void gIconRow::moveLast()
{
	gtk_list_store_move_before(tree->store, dataiter, NULL);
}

void gIconRow::moveBefore(char *key)
{
	gIconRow *row;
	
	if (!key || !*key)
	{
		moveLast();
		return;
	}
	
	row = tree->getRow(key);
	if (!row)
		return;
	gtk_list_store_move_before(tree->store, dataiter, row->dataiter);
}

void gIconRow::startRename()
{
	GtkTreePath* path;
	
	path = gtk_tree_model_get_path(GTK_TREE_MODEL(tree->store), dataiter);
	if (!path)
		return;
	
	//tree->view->setFocus();
	gtk_icon_view_set_cursor(GTK_ICON_VIEW(tree->widget), path, tree->rtext, true);
	gtk_tree_path_free(path);
}

/****************************************************************************

  gIcon
 
****************************************************************************/

static void cb_icon_edited(GtkCellRendererText *renderer, gchar *spath, gchar *new_text, gIcon *tree)
{
	gIconView *view = tree->view;
	
	if (!tree->_edited_row)
		return;
		
	view->setItemText(tree->_edited_row, new_text);
	view->emit(SIGNAL(view->onRename), tree->_edited_row);
	tree->_edited_row = NULL;	
}

/*static void cb_icon_fix_frame(GtkEntry *editable, GdkEventFocus *event, gpointer data)
{
	g_object_set(editable, "has_frame", FALSE, NULL);
}*/

static void cb_icon_started(GtkCellRenderer *renderer, GtkCellEditable *editable, gchar *spath, gIcon *tree)
{
	GtkTreePath *path = gtk_tree_path_new_from_string(spath);
	if (path)
		tree->_edited_row = tree->pathToKey(path);
	else
		tree->_edited_row = NULL;
		
	//if (GTK_IS_ENTRY(editable))
	//	g_signal_connect(editable, "focus-in-event", G_CALLBACK(cb_icon_fix_frame), NULL);
}

static void cb_icon_canceled(GtkCellRendererText *renderer, gIcon *tree)
{
	gIconView *view = tree->view;
	view->emit(SIGNAL(view->onCancel), tree->_edited_row);
}

static void icon_cell_text(GtkIconView *view, GtkCellRenderer *cell, GtkTreeModel *md, GtkTreeIter *iter, gIcon *icon)
{
	gIconRow *row = icon->getRow(icon->iterToKey(iter));
	
	if (!row)
	{
		g_object_set(G_OBJECT(cell),
			"text", "",
			"editable", false,
			//"alignment", PANGO_ALIGN_LEFT,
			//"wrap_mode", PANGO_WRAP_CHAR,
			(void *)NULL);
	}
	else
	{
		g_object_set(G_OBJECT(cell),
			//"xalign", 0.5,
			//"yalign", 0.0,
			"text", row->data->text(),
			"editable", row->isEditable(),
			//"alignment", PANGO_ALIGN_CENTER,
			//"wrap_mode", PANGO_WRAP_CHAR,
			(void *)NULL);
	}
}

static void icon_cell_graph(GtkIconView *view, GtkCellRenderer *cell,GtkTreeModel *md,GtkTreeIter *iter,gIcon *Tr)
{
	gIconRow *row=NULL;
	//gTreeCell *data;
	GdkPixbuf *buf=NULL;
	char *key;
	
	key = Tr->iterToKey(iter);
	if (key)
		row=(gIconRow*)g_hash_table_lookup(Tr->datakey,(gpointer)key);
	
	if (row)
		buf = row->data->picture() ? row->data->picture()->getPixbuf() : NULL;
	
	g_object_set(G_OBJECT(cell),"pixbuf",buf, (void *)NULL);
}

gIcon::gIcon(gIconView *v)
{
	datakey = g_hash_table_new((GHashFunc)g_str_hash,(GEqualFunc)gTree_equal);
	
	store = gtk_list_store_new(1, G_TYPE_POINTER); //, G_TYPE_BOOLEAN);
	widget = gtk_icon_view_new_with_model(GTK_TREE_MODEL(store));
	g_object_unref(G_OBJECT(store));
	
	view = v;
	_word_wrap = false;
	_grid_width = -1;
	_editable = false;
	_edited_row = NULL;	
	_sorted = false;
	_ascending = true;
	_sort_dirty = false;

	gtk_icon_view_set_orientation(GTK_ICON_VIEW(widget), GTK_ORIENTATION_VERTICAL);
	gtk_icon_view_set_spacing(GTK_ICON_VIEW(widget), 4);
	
	rgraph = gtk_cell_renderer_pixbuf_new();
	g_object_ref_sink(rgraph);
	rtext = gtk_cell_renderer_text_new();
	g_object_ref_sink(rtext);
	
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(widget), rgraph, false);
	gtk_cell_layout_pack_end(GTK_CELL_LAYOUT(widget), rtext, false);

	gtk_cell_layout_set_cell_data_func(GTK_CELL_LAYOUT(widget), rgraph, (GtkCellLayoutDataFunc)icon_cell_graph, (gpointer)this, NULL);
	gtk_cell_layout_set_cell_data_func(GTK_CELL_LAYOUT(widget), rtext, (GtkCellLayoutDataFunc)icon_cell_text, (gpointer)this, NULL);

	g_object_set (rgraph,
			"follow_state", TRUE, 
			"xalign", 0.5,
			"yalign", 1.0,
			(void *)NULL);
	
	g_object_set (rtext,
		"xalign", 0.5,
		"yalign", 0.0,
		"alignment", PANGO_ALIGN_CENTER,
		"wrap_mode", PANGO_WRAP_CHAR,
		"editable", false,
		//"ellipsize", _word_wrap ? PANGO_ELLIPSIZE_NONE : PANGO_ELLIPSIZE_END,
		(void *)NULL);
	
	g_signal_connect(G_OBJECT(rtext), "edited", G_CALLBACK(cb_icon_edited), (gpointer)this);
	g_signal_connect(G_OBJECT(rtext), "editing-started", G_CALLBACK(cb_icon_started), (gpointer)this);
	g_signal_connect(G_OBJECT(rtext), "editing-canceled", G_CALLBACK(cb_icon_canceled), (gpointer)this);
	
	updateTextCell();
	//setWordWrap(true);
}

gIcon::~gIcon()
{
	clear();
	g_hash_table_destroy(datakey);
	g_object_unref(rgraph);
	g_object_unref(rtext);
}


char *gIcon::iterToKey(GtkTreeIter *iter)
{
	char *key;
	gtk_tree_model_get(GTK_TREE_MODEL(store), iter, 0, &key, -1);
	return key;
}

char *gIcon::pathToKey(GtkTreePath *path, bool free)
{
	GtkTreeIter iter;
	char *key;
	
	if (!path) return NULL;
	
	if (gtk_tree_model_get_iter(GTK_TREE_MODEL(store), &iter, path))		
		key = iterToKey(&iter);
	else
		key = NULL;
		
	if (free) gtk_tree_path_free(path);	
	
	return key;
}

char* gIcon::cursor()
{
	GtkTreePath *path = NULL;

	gtk_icon_view_get_cursor(GTK_ICON_VIEW(widget), &path, NULL);
	return pathToKey(path);
}

void gIcon::setCursor(char *vl)
{
	GtkTreePath *path;
	gIconRow *row=(gIconRow*)g_hash_table_lookup(datakey,(gconstpointer)vl);
	
	if (!row) return;
	path = gtk_tree_model_get_path(GTK_TREE_MODEL(store),row->dataiter);
	if (path)
	{
		gtk_icon_view_set_cursor(GTK_ICON_VIEW(widget),path,NULL,false);
		gtk_tree_path_free(path);
	}
}

char *gIcon::firstRow()
{
	GtkTreeIter iter;
	
	if (!gtk_tree_model_get_iter_first (GTK_TREE_MODEL(store),&iter)) return NULL;
	return iterToKey(&iter);
}

char *gIcon::lastRow()
{
	GtkTreeIter iter;
	gint count;
	
	if (!gtk_tree_model_get_iter_first(GTK_TREE_MODEL(store), &iter)) 
		return NULL;
		
	count = gtk_tree_model_iter_n_children(GTK_TREE_MODEL(store), NULL);
	if (!count) return NULL;
	
	while (--count)
		gtk_tree_model_iter_next(GTK_TREE_MODEL(store), &iter);
	
	return iterToKey(&iter);
}

bool gIcon::rowExists(char *key)
{
	return (bool)(key && *key && g_hash_table_lookup(datakey,(gconstpointer)key));
}

bool gIcon::rowSelected(char *key)
{
	GtkTreePath *path;
	bool sel;
	gIconRow *row;
	 
	row = (gIconRow*)g_hash_table_lookup(datakey,(gconstpointer)key);
	if (!row) return false;
	
	path = gtk_tree_model_get_path(GTK_TREE_MODEL(store), row->dataiter);
	if (!path) return false;
	
	sel = gtk_icon_view_path_is_selected(GTK_ICON_VIEW(widget), path);
	gtk_tree_path_free(path);
	return sel;
}

void gIcon::setRowSelected(char *key,bool vl)
{
	GtkTreePath *path;
	gIconRow *row;
	
	row = (gIconRow*)g_hash_table_lookup(datakey,(gconstpointer)key);
	if (!row) return;
	
	path = gtk_tree_model_get_path(GTK_TREE_MODEL(store), row->dataiter);
	if (!path) return;

	if (vl)
		gtk_icon_view_select_path(GTK_ICON_VIEW(widget), path);
	else
		gtk_icon_view_unselect_path(GTK_ICON_VIEW(widget), path);

	gtk_tree_path_free(path);
}

bool gIcon::isRowEditable(char *key)
{
	gIconRow *row = getRow(key);
	if (!row)
		return false;
	else
		return row->isEditable();
}

void gIcon::setRowEditable(char *key, bool vl)
{
	gIconRow *row = getRow(key);
	if (!row)
		return;
	
	row->setEditable(vl);
}

int gIcon::rowCount()
{
	return g_hash_table_size(datakey);
}


bool gIcon::removeRow(char *key)
{
	gIconRow *row;
	
	if (!key || !*key)  return false;

	row=(gIconRow*)g_hash_table_lookup(datakey, (gconstpointer)key);
	if (!row) return false;
	
	g_hash_table_remove (datakey, (gconstpointer)key);
	gtk_list_store_remove(store, row->dataiter);
	delete row;
	
	return true;
}

gIconRow* gIcon::getRow(char *key) const
{
	if (!key)
		return NULL;
	else
		return (gIconRow*)g_hash_table_lookup (datakey,(gconstpointer)key);
}

gIconRow* gIcon::addRow(char *key, char *after)
{
	GtkTreeIter iter;
	gIconRow *row, *aft=NULL;
	char *buf;

	if (!key)  return NULL;
	
	if (g_hash_table_lookup (datakey,(gconstpointer)key)) return NULL;
	
	if (after)
	{
		aft=(gIconRow*)g_hash_table_lookup (datakey,(gconstpointer)after);
		if (!aft) return NULL;	
	}
	
	if (aft) gtk_list_store_insert_after(store,&iter,aft->dataiter);
	else     gtk_list_store_append (store, &iter);
	
	buf = g_strdup(key); // Will be freed by ~gIconRow()
	row = new gIconRow(this, buf, gtk_tree_iter_copy(&iter));
	g_hash_table_insert(datakey,(gpointer)buf,(gpointer)row);
	gtk_list_store_set(store, &iter, 0, buf, -1);
  
	return row;
}

void gIcon::selectAll()
{
	gtk_icon_view_select_all(GTK_ICON_VIEW(widget));
}

void gIcon::unselectAll()
{
	gtk_icon_view_unselect_all(GTK_ICON_VIEW(widget));
}

void gIcon::setGridWidth(int w)
{
	_grid_width = w;
	gtk_icon_view_set_item_width(GTK_ICON_VIEW(widget), _grid_width);
	updateTextCell();
}

void gIcon::updateTextCell()
{
	g_object_set (rtext,
		"xalign", 0.5,
		"yalign", 0.0,
		"alignment", PANGO_ALIGN_CENTER,
		"wrap_mode", PANGO_WRAP_CHAR,
		"editable", false,
		"wrap_width", _grid_width,
		//"ellipsize", _word_wrap ? PANGO_ELLIPSIZE_NONE : PANGO_ELLIPSIZE_END,
		(void *)NULL);
}

void gIcon::setWordWrap(bool vl)
{
	_word_wrap = vl;
	updateTextCell();
}


void gIcon::clear()
{
	char *key;
	
	view->lock();
	while ((key = firstRow()))
    removeRow(key);
  view->unlock();
}


void gIcon::setSorted(bool v)
{
	if (v == _sorted)
		return;
		
	_sorted = v;
	updateSort();
}

void gIcon::setSortAscending(bool v)
{
	_ascending = v;
	updateSort();
}

static gint icon_compare(GtkTreeModel *model, GtkTreeIter *a, GtkTreeIter *b, gIcon *tree)
{
	bool def = true;
	int comp;
	char *ka = tree->iterToKey(a);
	char *kb = tree->iterToKey(b);
	const char *ta, *tb;
	
	//fprintf(stderr, "ka = '%s' kb = '%s'\n", ka, kb);
	
	if (tree->view->onCompare)
		def = tree->view->onCompare(tree->view, ka, kb, &comp);
	
	if (def)
	{
		ta = tree->getRow(ka)->get(0)->text();
		if (!ta) ta = "";
		tb = tree->getRow(kb)->get(0)->text();
		if (!tb) tb = "";
		
		//fprintf(stderr, "ta = '%s' tb = '%s'\n", ta, tb);
	
		comp = g_utf8_collate(ta, tb);
	}
	
	if (!tree->isSortAscending())
		comp = (-comp);
	
	return comp;
}

void gIcon::sort()
{
	if (!_sorted)
		return;
	
	// BM: force the store to be sorted
	gtk_tree_sortable_set_default_sort_func(GTK_TREE_SORTABLE(store), (GtkTreeIterCompareFunc)icon_compare, this, NULL);
	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(store), GTK_TREE_SORTABLE_DEFAULT_SORT_COLUMN_ID, GTK_SORT_ASCENDING);
	_sort_dirty = false;
}

static gboolean icon_sort_later(gIcon *tree)
{
	tree->sort();
	return FALSE;
}

void gIcon::sortLater()
{
	if (_sort_dirty)
		return;
		
	_sort_dirty = true;
	g_timeout_add(0, (GSourceFunc)icon_sort_later, this);
}

