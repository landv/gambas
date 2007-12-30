/***************************************************************************

  gcolumnview.cpp

  (c) 2004-2005 - Daniel Campos Fernández <dcamposf@gmail.com>
  
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

/************************************************************************

 Signal handlers
 
*************************************************************************/
void colview_select(GtkTreeView *tv,gColumnView *data)
{
	if (!gApplication::userEvents()) return;
	if (data->onClick) data->onClick((gControl*)data);
}

void colview_activate(GtkTreeView *tv,GtkTreePath *a1,GtkTreeViewColumn *a2,gColumnView *data)
{
	if (!gApplication::userEvents()) return;
	if (data->onActivate) data->onActivate((gControl*)data);
}


/************************************************************************

 Helper functions
 
*************************************************************************/
bool gColumnView_nextNode(GtkTreeModel *store,GtkTreeIter *iter)
{
	GtkTreeIter chd;
	char *buf;

	if (gtk_tree_model_iter_has_child(store,iter))
	{
		gtk_tree_model_iter_children(store,&chd,iter);
		buf=gtk_tree_model_get_string_from_iter(store,&chd);
		gtk_tree_model_get_iter_from_string(store,iter,buf);
		g_free(buf);
		return true;
	}
	
	if ( gtk_tree_model_iter_next(store,iter) ) return true;
	
	if (gtk_tree_model_iter_parent(store,&chd,iter)) 
	{
		buf=gtk_tree_model_get_string_from_iter(store,&chd);
		gtk_tree_model_get_iter_from_string(store,iter,buf);
		g_free(buf);
	}
	
	return gtk_tree_model_iter_next(store,iter);
}

/**************************************************************************

gColumnView

***************************************************************************/
gColumnView::gColumnView(gControl *parent) : gControl(parent)
{
	GtkTreeStore *store;
	GtkTreeViewColumn *col;
	GtkCellRenderer *renderer,*graph;
	int bucle;
	GType data[66];
	
	g_typ=Type_gListView;
	sort=false;
	border=gtk_scrolled_window_new (NULL,NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(border),GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(border),GTK_SHADOW_IN);
	
	for (bucle=0; bucle<66; bucle++) data[bucle]=G_TYPE_STRING;
	data[1]=GDK_TYPE_PIXBUF;
	store=gtk_tree_store_newv (66,data);
	widget=gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
	gtk_container_add(GTK_CONTAINER(border),widget);
	g_object_unref (store);
	colCount=1;
	
	
	renderer = gtk_cell_renderer_text_new();
	col=gtk_tree_view_column_new_with_attributes("",renderer,"text",0,NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(widget), col);
	gtk_tree_view_column_set_visible(col,false);
	
	col=gtk_tree_view_column_new();
	graph=gtk_cell_renderer_pixbuf_new();
	gtk_tree_view_column_pack_start(col, graph, true);
	gtk_tree_view_column_add_attribute(col, graph, "pixbuf", 1);
	renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_column_pack_start(col, renderer, true);
	gtk_tree_view_column_add_attribute(col, renderer, "text", 2);
	gtk_tree_view_append_column(GTK_TREE_VIEW(widget), col);
	
	connectParent();
	initSignals();
	
	onClick=NULL;
	/* Click */
	g_signal_connect(G_OBJECT(widget),"cursor-changed",G_CALLBACK(colview_select),(gpointer)this);
	/* Double Click */
	g_signal_connect(G_OBJECT(widget),"row-activated",G_CALLBACK(colview_activate),(gpointer)this);
	
	setResizable(true);

}

long gColumnView::columnAlignment(long ind)
{
	GtkTreeViewColumn* col;
	gfloat vl;
	
	if ( (ind<0) || (ind>=colCount) ) return NULL;
	col=gtk_tree_view_get_column(GTK_TREE_VIEW(widget),ind+1);
	
	vl=gtk_tree_view_column_get_alignment(col);
	if (vl==0) return 65;
	if (vl==1) return 66;
	return 68;
}

char* gColumnView::columnText(long ind)
{
	GtkTreeViewColumn* col;
	char *buf;
	
	if ( (ind<0) || (ind>=colCount) ) return NULL;
	col=gtk_tree_view_get_column(GTK_TREE_VIEW(widget),ind+1);
	g_object_get(G_OBJECT(col),"title",&buf,NULL);
	return buf;
}

long gColumnView::columnWidth(long ind)
{
	GtkTreeViewColumn* col;
	
	if ( (ind<0) || (ind>=colCount) ) return 0;
	col=gtk_tree_view_get_column(GTK_TREE_VIEW(widget),ind+1);
	return gtk_tree_view_column_get_width(col);
}

long gColumnView::columnsCount()
{
	return colCount;
}

long gColumnView::count()
{
	GtkTreeIter iter;
	GtkTreeModel *store;
	long len=1;
	
	store =gtk_tree_view_get_model(GTK_TREE_VIEW(widget));
	if (!gtk_tree_model_get_iter_first(store,&iter)) return 0;
	while ( gColumnView_nextNode(store,&iter) ) len++;

	return len;
}

bool gColumnView::expanded(long ind)
{
	GtkTreeIter iter;
	GtkTreeModel *store;
	GtkTreePath *path;
	long len=0;
	bool vl;
	
	if ( (ind<0) || (ind>=count()) ) return false;
	
	store = gtk_tree_view_get_model(GTK_TREE_VIEW(widget));
	gtk_tree_model_get_iter_first(store,&iter);
	while (len<ind) { gColumnView_nextNode(store,&iter); len++; }
	path=gtk_tree_model_get_path(store,&iter);
	vl=gtk_tree_view_row_expanded(GTK_TREE_VIEW(widget),path);
	gtk_tree_path_free(path);
	return vl;
	
}

bool gColumnView::header()
{
	return gtk_tree_view_get_headers_visible(GTK_TREE_VIEW(widget));
}


long gColumnView::itemCount(long ind)
{
	GtkTreeIter iter;
	GtkTreeModel *store;
	GtkTreeIter chd;
	long len=0;
	

	if ( (ind<0) || (ind>=count()) ) return NULL;
	
	store =gtk_tree_view_get_model(GTK_TREE_VIEW(widget));
	gtk_tree_model_get_iter_first(store,&iter);
	while (len<ind) { gColumnView_nextNode(store,&iter); len++; }
	
	len=0;
	if (gtk_tree_model_iter_has_child(store,&iter))
	{
		len++;
		gtk_tree_model_iter_children(store,&chd,&iter);
		while (gtk_tree_model_iter_next(store,&chd)) len++;
	}
	
	return len;
}

gPicture* gColumnView::itemPicture(long ind)
{
	GtkTreeIter iter;
	GtkTreeModel *store;
	long len=0;
	GdkPixbuf *buf=NULL;
	gPicture *ret=NULL;

	if ( (ind<0) || (ind>=count()) ) return NULL;
	
	store =gtk_tree_view_get_model(GTK_TREE_VIEW(widget));
	gtk_tree_model_get_iter_first(store,&iter);
	while (len<ind) { gColumnView_nextNode(store,&iter); len++; }
	gtk_tree_model_get(store,&iter,1,&buf,-1);
	if (buf)
	{
		ret=gPicture::fromPixbuf(buf);
		g_object_unref(buf);
	}
	
	return ret;
}

char* gColumnView::itemText(long ind,long col)
{
	GtkTreeIter iter;
	GtkTreeModel *store;
	long len=0;
	char *buf;

	if ( (ind<0) || (ind>=count()) ) return NULL;
	if ( (col<0) || (col>=colCount) ) return NULL;
	
	store =gtk_tree_view_get_model(GTK_TREE_VIEW(widget));
	gtk_tree_model_get_iter_first(store,&iter);
	while (len<ind) { gColumnView_nextNode(store,&iter); len++; }
	gtk_tree_model_get(store,&iter,col+2,&buf,-1);
	
	return buf;
}

long gColumnView::getCurrentCursor()
{
	GtkTreeModel *store;
	GtkTreePath *path;
	GtkTreeIter iter;
	char *buf;
	long ret=-1;

	if (!count()) return -1;

	gtk_tree_view_get_cursor(GTK_TREE_VIEW(widget),&path,NULL);
	if (!path) return -1; 

	store =gtk_tree_view_get_model(GTK_TREE_VIEW(widget));
	gtk_tree_model_get_iter(store,&iter,path);
	gtk_tree_model_get(store,&iter,0,&buf,-1);
	ret=keyIndex(buf);
	g_free(buf);
	gtk_tree_path_free(path);
	return ret;

}

char* gColumnView::key(long ind)
{
	GtkTreeIter iter;
	GtkTreeModel *store;
	long len=0;
	char *buf;

	if ( (ind<0) || (ind>=count()) ) return NULL;
	
	store =gtk_tree_view_get_model(GTK_TREE_VIEW(widget));
	gtk_tree_model_get_iter_first(store,&iter);
	while (len<ind) { gColumnView_nextNode(store,&iter); len++; }
	gtk_tree_model_get(store,&iter,0,&buf,-1);
	
	return buf;	
}

long gColumnView::keyIndex(char *key)
{
	long ret=0;
	GtkTreeIter curr;
	GtkTreeModel *store;
	char *currkey;
	
	if (!key) return -1;
	store = gtk_tree_view_get_model(GTK_TREE_VIEW(widget));
	
	if (gtk_tree_model_get_iter_first(store,&curr))
	{
		do
		{
			gtk_tree_model_get (store,&curr,0,&currkey,-1);
			if ( !strcmp ( currkey, key ) )
			{
				g_free(currkey);
				return ret;
			}
			g_free(currkey);
			ret++;
		
		} while ( gColumnView_nextNode(store,&curr) );
	}
	
	return -1;
}

bool gColumnView::resizable()
{
	return colResize;
}

long gColumnView::scrollBars()
{
	GtkScrolledWindow *sc=GTK_SCROLLED_WINDOW(border);
	GtkPolicyType h,v;
	long ret=0;
	
	gtk_scrolled_window_get_policy  (sc,&h,&v);
	if (h!=GTK_POLICY_NEVER) ret++;
	if (v!=GTK_POLICY_NEVER) ret+=2;
	return ret;
}

void gColumnView::setColumnAlignment(long ind,long vl)
{
	GtkTreeViewColumn* col;
	gfloat ret=0.5;
	
	if ( (ind<0) || (ind>=colCount) ) return;
	col=gtk_tree_view_get_column(GTK_TREE_VIEW(widget),ind+1);
	
	if (vl & AlignRight) ret=1;
	if (vl & AlignLeft) ret=0;
	
	gtk_tree_view_column_set_alignment(col,ret);

}

void gColumnView::setColumnText(long ind,char *vl)
{
	GtkTreeViewColumn* col;
	
	if ( (ind<0) || (ind>=colCount) ) return;
	if (!vl) return;
	col=gtk_tree_view_get_column(GTK_TREE_VIEW(widget),ind+1);
	g_object_set(G_OBJECT(col),"title",vl,NULL);
}

void gColumnView::setColumnWidth(long ind,long vl)
{
	GtkTreeViewColumn* col;
	
	if ( (ind<0) || (ind>=colCount) ) return;
	col=gtk_tree_view_get_column(GTK_TREE_VIEW(widget),ind+1);
	
	g_object_set(G_OBJECT(col),"sizing",GTK_TREE_VIEW_COLUMN_FIXED,NULL);
	gtk_tree_view_column_set_fixed_width(col,vl);
}

void gColumnView::setColumnsCount(long vl)
{
	GtkTreeViewColumn* col;
	GtkCellRenderer *renderer;
	
	if (vl==colCount) return;
	if ( (vl<1) || (vl>64) ) return;
	
	if (vl<colCount)
	{
		while (vl<colCount)
		{
			col=gtk_tree_view_get_column(GTK_TREE_VIEW(widget),colCount);
			gtk_tree_view_remove_column(GTK_TREE_VIEW(widget),col);
			colCount--;
		}
		return;
	}
	
	renderer=gtk_cell_renderer_text_new();
	while(vl>colCount)
	{
		colCount++;
		col=gtk_tree_view_column_new_with_attributes("",renderer,"text",colCount+1,NULL);
   		gtk_tree_view_append_column(GTK_TREE_VIEW(widget), col);
	}
	setResizable(colResize);
}

void gColumnView::setExpanded(long ind,bool vl)
{
	GtkTreeIter iter;
	GtkTreeModel *store;
	GtkTreePath *path;
	long len=0;
	
	if ( (ind<0) || (ind>=count()) ) return;
	
	store = gtk_tree_view_get_model(GTK_TREE_VIEW(widget));
	gtk_tree_model_get_iter_first(store,&iter);
	while (len<ind) { gColumnView_nextNode(store,&iter); len++; }
	path=gtk_tree_model_get_path(store,&iter);
	if (vl)
		gtk_tree_view_expand_row(GTK_TREE_VIEW(widget),path,false);
	else
		gtk_tree_view_collapse_row(GTK_TREE_VIEW(widget),path);
	gtk_tree_path_free(path);
	
}

void gColumnView::setHeader(bool vl)
{
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(widget),vl);
}

void gColumnView::setItemPicture(long ind,gPicture *pic)
{
	GtkTreeIter iter;
	GtkTreeModel *store;
	GdkPixbuf *buf=NULL;
	long len=0;

	if ( (ind<0) || (ind>=count()) ) return;

	if (pic) buf=pic->getPixbuf();
		
	store =gtk_tree_view_get_model(GTK_TREE_VIEW(widget));
	gtk_tree_model_get_iter_first(store,&iter);
	while (len<ind) { gColumnView_nextNode(store,&iter); len++; }
	gtk_tree_store_set (GTK_TREE_STORE(store), &iter,1,buf,-1);
	if (buf) g_object_unref(buf);

}

void gColumnView::setItemText(long ind,long col,char *buf)
{
	GtkTreeIter iter;
	GtkTreeModel *store;
	long len=0;

	if ( (ind<0) || (ind>=count()) ) return;
	if ( (col<0) || (col>=colCount) ) return;
	
	store =gtk_tree_view_get_model(GTK_TREE_VIEW(widget));
	gtk_tree_model_get_iter_first(store,&iter);
	while (len<ind) { gColumnView_nextNode(store,&iter); len++; }
	gtk_tree_store_set (GTK_TREE_STORE(store), &iter,col+2,buf,-1);

}

void gColumnView::setResizable(bool vl)
{
	long bucle;
	GtkTreeViewColumn* col;
	
	for (bucle=1;bucle<=colCount;bucle++)
	{
		col=gtk_tree_view_get_column(GTK_TREE_VIEW(widget),bucle);
		gtk_tree_view_column_set_resizable(col,vl);
	}
	
	colResize=vl;
}

void gColumnView::setScrollBars(long vl)
{
	GtkScrolledWindow *sc=GTK_SCROLLED_WINDOW(border);

	switch(vl)
	{
		case 0: // None
			gtk_scrolled_window_set_policy(sc,GTK_POLICY_NEVER,GTK_POLICY_NEVER);break;
  		case 1: // Horizontal
			gtk_scrolled_window_set_policy(sc,GTK_POLICY_AUTOMATIC,GTK_POLICY_NEVER);break;
  		case 2: // Vertical
			gtk_scrolled_window_set_policy(sc,GTK_POLICY_NEVER,GTK_POLICY_AUTOMATIC);break;
  		case 3: // Both
			gtk_scrolled_window_set_policy(sc,GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);break;
	}
}


int gColumnView::add(char *key,char *vl,gPicture *pic,char *pos,char *parent)
{
	GdkPixbuf *buf=NULL;
	GtkTreePath *path;
	GtkTreeIter iter,curr,pr;
	GtkTreeStore *store;
	bool set;
	char *currkey;
	long p_ind=-2;
	
	if (!key) return -1;
	
	store = (GtkTreeStore*)gtk_tree_view_get_model(GTK_TREE_VIEW(widget));
	if (parent)
	{
		p_ind=keyIndex(parent);
		if (p_ind==-1) return -2;
		gtk_tree_model_get_iter_first(GTK_TREE_MODEL(store),&pr);
		while (p_ind) { gColumnView_nextNode(GTK_TREE_MODEL(store),&pr); p_ind--; }
	}
	
	if (pic) buf=pic->getPixbuf();
	
	if (gtk_tree_model_get_iter_first(GTK_TREE_MODEL(store),&curr))
	{
		do
		{
			gtk_tree_model_get (GTK_TREE_MODEL(store),&curr,0,&currkey,-1);
			if ( !strcmp ( currkey, key ) )
			{
				if (buf) g_object_unref(G_OBJECT(buf));
				g_free(currkey);
				return -1;
			}
			g_free(currkey);
		
		} while ( gColumnView_nextNode(GTK_TREE_MODEL(store),&curr) );
	}
	
	if (p_ind==-2)  gtk_tree_store_insert(store,&iter,NULL,-1);
	else            gtk_tree_store_insert(store,&iter,&pr,-1);
	gtk_tree_store_set (store, &iter,0,key,1,buf,2,vl,-1);
	if (buf) g_object_unref(buf);

	if (count()==1)
	{
		path=gtk_tree_model_get_path(GTK_TREE_MODEL(store),&iter);
		gtk_tree_view_set_cursor(GTK_TREE_VIEW(widget),path,NULL,FALSE);
		gtk_tree_path_free(path);
        }
	
	return 0;
}

void gColumnView::itemClear(long ind)
{
	GtkTreeIter iter;
	GtkTreeModel *store;
	GtkTreeIter chd;
	long len=0;
	

	if ( (ind<0) || (ind>=count()) ) return;
	
	store =gtk_tree_view_get_model(GTK_TREE_VIEW(widget));
	gtk_tree_model_get_iter_first(store,&iter);
	while (len<ind) { gColumnView_nextNode(store,&iter); len++; }
	
	while (gtk_tree_model_iter_has_child(store,&iter))
	{
		gtk_tree_model_iter_children(store,&chd,&iter);
		gtk_tree_store_remove(GTK_TREE_STORE(store),&chd);
	}

}

void gColumnView::remove(long pos)
{
	GtkTreeIter iter;
	GtkTreeModel *store;

	if (pos<0) return;
	if (pos>=count()) return;
	
	store = gtk_tree_view_get_model(GTK_TREE_VIEW(widget));
	gtk_tree_model_get_iter_first(store,&iter);
	while (pos--) gColumnView_nextNode(store,&iter);
	gtk_tree_store_remove(GTK_TREE_STORE(store),&iter);
}

long gColumnView::getFirstIndex()
{
	if ( count()==0 ) return -1;
	return 0;
}

long gColumnView::getLastIndex()
{
	GtkTreeIter iter;
	GtkTreeModel *store;
	long len=0;

	store =gtk_tree_view_get_model(GTK_TREE_VIEW(widget));
	if (!gtk_tree_model_get_iter_first(store,&iter)) return -1;
	while ( gColumnView_nextNode(store,&iter) ) len++;

	return len;
}

long gColumnView::getNextIndex(long ind)
{
	GtkTreeIter iter;
	GtkTreeModel *store;
	long len=0;
	char *buf;

	if ( (ind<0) || (ind>=count()) ) return -1;

	store =gtk_tree_view_get_model(GTK_TREE_VIEW(widget));
	gtk_tree_model_get_iter_first(store,&iter);
	while (len<ind) { gColumnView_nextNode(store,&iter); len++; }
	
	if (gtk_tree_model_iter_next(store,&iter)==FALSE) return -1;
	
	gtk_tree_model_get(store,&iter,0,&buf,-1);
	len=keyIndex(buf);
	g_free(buf);
	return len;
}

long gColumnView::getPreviousIndex(long ind)
{
	GtkTreeIter iter;
	GtkTreeModel *store;
	GtkTreePath *path;
	long len=0;
	char *buf;

	if ( (ind<0) || (ind>=count()) ) return -1;

	store =gtk_tree_view_get_model(GTK_TREE_VIEW(widget));
	gtk_tree_model_get_iter_first(store,&iter);
	while (len<ind) { gColumnView_nextNode(store,&iter); len++; }
	

	path=gtk_tree_model_get_path(store,&iter);
	if (!gtk_tree_path_prev(path))
	{
		gtk_tree_path_free(path);
		return -1;
	}

	gtk_tree_model_get_iter(store,&iter,path);
	gtk_tree_model_get(store,&iter,0,&buf,-1);
	len=keyIndex(buf);
	g_free(buf);
	gtk_tree_path_free(path);
	return len;
}

long gColumnView::getParentIndex(long ind)
{
	GtkTreeIter iter,testparent;
	GtkTreeModel *store;
	long len=0;
	char *buf;

	if ( (ind<0) || (ind>=count()) ) return -1;

	store =gtk_tree_view_get_model(GTK_TREE_VIEW(widget));
	gtk_tree_model_get_iter_first(store,&iter);
	while (len<ind) { gColumnView_nextNode(store,&iter); len++; }
	
	if (gtk_tree_model_iter_parent(store,&testparent,&iter)==FALSE) return -1;
	
	gtk_tree_model_get(store,&testparent,0,&buf,-1);
	len=keyIndex(buf);
	g_free(buf);
	return len;
}

long gColumnView::getChildIndex(long ind)
{
	GtkTreeIter iter,testchild;
	GtkTreeModel *store;
	long len=0;
	char *buf;

	if ( (ind<0) || (ind>=count()) ) return -1;

	store =gtk_tree_view_get_model(GTK_TREE_VIEW(widget));
	gtk_tree_model_get_iter_first(store,&iter);
	while (len<ind) { gColumnView_nextNode(store,&iter); len++; }
	
	if (gtk_tree_model_iter_children(store,&testchild,&iter)==FALSE) return -1;
	
	gtk_tree_model_get(store,&testchild,0,&buf,-1);
	len=keyIndex(buf);
	g_free(buf);
	return len;
}

long gColumnView::findAt(int x,int y)
{
	GtkTreePath *path;
	GtkTreeIter iter;
	GtkTreeModel *store;
	long len;
	char *buf;

	if (gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(widget),x,y,&path,NULL,NULL,NULL))
	{
		store =gtk_tree_view_get_model(GTK_TREE_VIEW(widget));
		gtk_tree_model_get_iter(store,&iter,path);
		gtk_tree_model_get(store,&iter,0,&buf,-1);
		len=keyIndex(buf);
		g_free(buf);
		gtk_tree_path_free(path);
		return len;	
	}

	return -1;
}





