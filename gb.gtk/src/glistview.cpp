/***************************************************************************

  glistview.cpp

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
#include "gtree.h"
#include <gtk/gtk.h>
#include <string.h>
#include <stdlib.h>

#include <stdio.h>

/**************************************************************************

gListView

***************************************************************************/

bool gListView_Style=false;

void gLV_Activate(GtkTreeView *tv,GtkTreePath *a1,GtkTreeViewColumn *a2,gListView *data)
{
	if (data->onActivate) data->onActivate(data);
}



gListView::gListView(gControl *parent) : gControl(parent)
{
	
	g_typ=Type_gListView;
		
	tree=new gTree();
	tree->addColumn();
	tree->setHeaders(false);
	gtk_widget_set_name(tree->tree,"ListView");
	
	if (!gListView_Style)
	{
		gtk_rc_parse_string ("style \"listview-style\" {\nGtkTreeView::expander-size = 0}");
		gtk_rc_parse_string ("widget \"*.ListView\" style \"listview-style\"");
		gListView_Style=true;
	}
	
	border=gtk_scrolled_window_new(NULL,NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(border),GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);
	widget=tree->tree;
	gtk_container_add(GTK_CONTAINER(border),widget);
	setMode(0);
	setBorder(true);

	onActivate=NULL;	
	onSelect=NULL;
	connectParent();
	initSignals();
	g_signal_connect(G_OBJECT(widget),"row-activated",G_CALLBACK(gLV_Activate),(gpointer)this);

}

gListView::~gListView()
{
}

void gListView::clear()
{
	tree->clearRows();
}

bool gListView::add(char *key,char *text,gPicture *pic,char *after)
{
	GdkPixbuf *buf=NULL;
	gTreeRow *row=tree->addRow(key,NULL,after);
	
	if (!row) return false;
	
	row->get(0)->setText(text);
	
	if (pic)
	{
		buf=pic->getPixbuf();
		if (buf)
		{
			row->get(0)->setPixbuf(buf);
			g_object_unref(buf);
		}
	}
	
	if ( tree->rowCount()==1) tree->setCursor(key);
	
	return true;
}

bool gListView::remove(char *key)
{
	return tree->removeRow(key);
}

bool gListView::exists(char *key)
{
	return tree->rowExists(key);
}

unsigned long gListView::count()
{
	return tree->rowCount();
}

int gListView::mode()
{
	GtkTreeSelection *sel=gtk_tree_view_get_selection(GTK_TREE_VIEW(tree->tree));
	
	switch (gtk_tree_selection_get_mode (sel))
	{
		case GTK_SELECTION_SINGLE:   return 1;
		case GTK_SELECTION_MULTIPLE: return 2;
		case GTK_SELECTION_NONE:     return 0;
	}
	
	return 0;
}

void gListView::setMode(int vl)
{
	GtkTreeSelection *sel=gtk_tree_view_get_selection(GTK_TREE_VIEW(tree->tree));
	
	switch (vl)
	{
		case 0:
			gtk_tree_selection_set_mode(sel,GTK_SELECTION_NONE); break;
		case 1:
			gtk_tree_selection_set_mode(sel,GTK_SELECTION_SINGLE); break;
		case 2:
			gtk_tree_selection_set_mode(sel,GTK_SELECTION_MULTIPLE); break;
		
	}
}

long gListView::visibleWidth()
{
	return tree->visibleWidth();
}

long gListView::visibleHeight()
{
	return tree->visibleHeight();
}


char* gListView::itemText(char *key)
{
	gTreeRow *row;
	
	if (!key) return NULL;
	row=tree->getRow(key);
	if (!row) return NULL;
	return row->get(0)->text;
	
}

void gListView::itemSetText(char *key,char *vl)
{
	gTreeRow *row;
	GdkRectangle rect;
	gint tx,ty,tw,th;
	
	if (!key) return;
	row=tree->getRow(key);
	if (!row) return;
	row->get(0)->setText(vl);
	row->update();	
}

gPicture* gListView::itemPicture(char *key)
{
	gTreeRow *row;
	gPicture *pic=NULL;
	
	if (!key) return NULL;
	row=tree->getRow(key);
	if (!row) return NULL;
	pic=gPicture::fromPixbuf(row->get(0)->pic);

        return pic;
}

void gListView::itemSetPicture(char *key,gPicture *vl)
{
	gTreeRow *row;
	GdkPixbuf *buf=NULL;
	
	if (vl) buf=vl->getPixbuf();
	
	if (!key) return;
	row=tree->getRow(key);
	if (!row) return;
	row->get(0)->setPixbuf(buf);
	row->update();	
	if (buf) g_object_unref(G_OBJECT(buf));
}

bool gListView::itemSelected(char *key)
{
	if (!key) return false;
	return tree->rowSelected(key);
	
}

void gListView::itemSetSelected(char *key,bool vl)
{
	if (!key) return;
	tree->setRowSelected(key,vl);
}

char* gListView::current()
{
	return tree->cursor();
}

char* gListView::firstItem()
{
	return tree->firstTopRow();
}

char* gListView::lastItem()
{
	return tree->lastTopRow();
}

char* gListView::nextItem(char *vl)
{
	gTreeRow *row=tree->getRow(vl);
	if (!row) return NULL;
	return tree->nextRow(row);
}

char* gListView::prevItem(char *vl)
{
	gTreeRow *row=tree->getRow(vl);
	if (!row) return NULL;
	return tree->prevRow(row);
}

char* gListView::find(long x,long y)
{
	GtkTreePath *path;
	GtkTreeIter iter;
	char *key=NULL;

	if (gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(tree->tree),x,y,&path,NULL,NULL,NULL))
	{
		gtk_tree_model_get_iter(GTK_TREE_MODEL(tree->store),&iter,path);	
		gtk_tree_model_get(GTK_TREE_MODEL(tree->store),&iter,0,&key,-1);
		gtk_tree_path_free(path);
		return key;
	}
	
	return NULL;

}

bool gListView::sorted()
{
	return gtk_tree_sortable_has_default_sort_func (GTK_TREE_SORTABLE(tree->store));
}


gint gListview_sort (GtkTreeModel *md,GtkTreeIter *a,GtkTreeIter *b,gTree *tree)
{
	char *key1=NULL,*key2=NULL;
	char *v1,*v2;
	long ret=0;
	
	gtk_tree_model_get(md,a,0,&key1,-1);
	gtk_tree_model_get(md,b,0,&key2,-1);
	
	if ( key1 && key2 )
	{
		v1=tree->getRow(key1)->get(0)->text;
		v2=tree->getRow(key2)->get(0)->text;
		ret=strcasecmp(v1,v2);
	}
	if (key1) free(key1);
	if (key2) free(key2);
	
	return ret;
}

void gListView::setSorted(bool vl)
{
	GtkTreeViewColumn *col;
	
	if (vl && (!sorted()) )
	{
		gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(tree->store),0,
		                                       (GtkTreeIterCompareFunc)gListview_sort,
		                                        (gpointer)tree,NULL);
		
		col=gtk_tree_view_get_column(GTK_TREE_VIEW(tree->tree),0);
		gtk_tree_view_column_set_sort_column_id(col,0);
		//gtk_tree_view_column_clicked (col);
		return;
	}
	
	if ( (!vl) && sorted() )
	{
		gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(tree->store),0,
		                                        NULL,NULL,NULL);
	}
}

bool gListView::getBorder()
{
	if (GTK_SHADOW_NONE==gtk_scrolled_window_get_shadow_type(GTK_SCROLLED_WINDOW(border)))
		return false;
	return true;
}

void gListView::setBorder(bool vl)
{
	GtkScrolledWindow *wr=GTK_SCROLLED_WINDOW(border);
	
	if (vl)
		gtk_scrolled_window_set_shadow_type(wr,GTK_SHADOW_ETCHED_OUT);
	else
		gtk_scrolled_window_set_shadow_type(wr,GTK_SHADOW_NONE);

}

long gListView::scrollBar()
{
	GtkPolicyType h,v;
	long ret=3;
	
	gtk_scrolled_window_get_policy(GTK_SCROLLED_WINDOW(border),&h,&v);
	if (h==GTK_POLICY_NEVER) ret--;
	if (v==GTK_POLICY_NEVER) ret-=2;
	
	return ret;
}

void gListView::setScrollBar(long vl)
{
	GtkScrolledWindow *sc=GTK_SCROLLED_WINDOW(border);
	switch(vl)
	{
		case 0:
			gtk_scrolled_window_set_policy(sc,GTK_POLICY_NEVER,GTK_POLICY_NEVER);
			break;
		case 1:
			gtk_scrolled_window_set_policy(sc,GTK_POLICY_AUTOMATIC,GTK_POLICY_NEVER);
			break;
		case 2:
			gtk_scrolled_window_set_policy(sc,GTK_POLICY_NEVER,GTK_POLICY_AUTOMATIC);
			break;
		case 3:
			gtk_scrolled_window_set_policy(sc,GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);
			break;
	}
}



