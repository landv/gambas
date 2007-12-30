/***************************************************************************

  gtreeview.cpp

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
#include "gtree.h"
#include <gtk/gtk.h>
#include <string.h>
#include <stdlib.h>

#include <stdio.h>

/**************************************************************************

gTreeView

***************************************************************************/

void gTV_Activate(GtkTreeView *tv,GtkTreePath *a1,GtkTreeViewColumn *a2,gTreeView *data)
{
	if (data->onActivate) data->onActivate(data);
}

void gTV_change(GtkTreeSelection *ts,gTreeView *data)
{
	
	if (data->onSelect) data->onSelect(data);
}

bool gTV_click(GtkWidget *widget,GdkEventButton *e,gTreeView *data)
{
	
	if (data->onClick) 
	{
		if (gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(data->widget),e->x,e->y,NULL,NULL,NULL,NULL))
			data->onClick(data);
	}
	
	return false;
}

gTreeView::gTreeView(gControl *parent) : gControl(parent)
{
	GtkTreeSelection *sel;
	
	g_typ=Type_gTreeView;
	
	tree=new gTree();
	tree->addColumn();
	tree->setHeaders(false);
	
	gtk_tree_view_set_rules_hint(GTK_TREE_VIEW(tree->tree),true);
	
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
	
	sel=gtk_tree_view_get_selection(GTK_TREE_VIEW(tree->tree));
	
	g_signal_connect(G_OBJECT(widget),"row-activated",G_CALLBACK(gTV_Activate),(gpointer)this);
	g_signal_connect(G_OBJECT(sel),"changed",G_CALLBACK(gTV_change),(gpointer)this);
	g_signal_connect(G_OBJECT(widget),"button-press-event",G_CALLBACK(gTV_click),(gpointer)this);
	
}

gTreeView::~gTreeView()
{
}

void gTreeView::clear()
{
	tree->clearRows();
}

bool gTreeView::add(char *key,char *text,gPicture *pic,char *after,char *parent)
{
	GdkPixbuf *buf=NULL;
	gTreeRow *row;
	
	row=tree->addRow(key,parent,after);
	
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

bool gTreeView::remove(char *key)
{
	return tree->removeRow(key);
}

bool gTreeView::exists(char *key)
{
	return tree->rowExists(key);
}

unsigned long gTreeView::count()
{
	return tree->rowCount();
}

int gTreeView::mode()
{
	GtkTreeSelection *sel=gtk_tree_view_get_selection(GTK_TREE_VIEW(tree->tree));
	
	switch (gtk_tree_selection_get_mode (sel))
	{
		case GTK_SELECTION_NONE:     return 0;
		case GTK_SELECTION_SINGLE:   return 1;
		case GTK_SELECTION_MULTIPLE: return 2;
	}
	
	return 0;
}

void gTreeView::setMode(int vl)
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

long gTreeView::visibleWidth()
{
	return tree->visibleWidth();
}

long gTreeView::visibleHeight()
{
	return tree->visibleHeight();
}


char* gTreeView::itemText(char *key)
{
	gTreeRow *row;
	
	if (!key) return NULL;
	row=tree->getRow(key);
	if (!row) return NULL;
	return row->get(0)->text;
	
}

char* gTreeView::itemParent(char *key)
{
	gTreeRow *row;
	
	if (!key) return NULL;
	row=tree->getRow(key);
	if (!row) return NULL;
	return row->parentKey();
	
}



void gTreeView::itemSetText(char *key,char *vl)
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

long gTreeView::itemChildren(char *key)
{
	gTreeRow *row;
	
	if (!key) return 0;
	row=tree->getRow(key);
	if (!row) return 0;
	return row->children();

}

gPicture* gTreeView::itemPicture(char *key)
{
	gTreeRow *row;
	gPicture *pic=NULL;
	
	if (!key) return NULL;
	row=tree->getRow(key);
	if (!row) return NULL;
	pic=gPicture::fromPixbuf(row->get(0)->pic);

        return pic;
}

void gTreeView::itemSetPicture(char *key,gPicture *vl)
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

bool gTreeView::itemSelected(char *key)
{
	if (!key) return false;
	return tree->rowSelected(key);
	
}

char* gTreeView::current()
{
	return tree->cursor();
}

char* gTreeView::firstItem()
{
	return tree->firstTopRow();
}

char* gTreeView::lastItem()
{
	return tree->lastTopRow();
}

char* gTreeView::nextItem(char *vl)
{
	gTreeRow *row=tree->getRow(vl);
	if (!row) return NULL;
	return tree->nextRow(row);
}

char* gTreeView::prevItem(char *vl)
{
	gTreeRow *row=tree->getRow(vl);
	if (!row) return NULL;
	return tree->prevRow(row);
}

char* gTreeView::find(long x,long y)
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

bool gTreeView::sorted()
{
	return false;
}


void gTreeView::setSorted(bool vl)
{

}

bool gTreeView::getBorder()
{
	if (GTK_SHADOW_NONE==gtk_scrolled_window_get_shadow_type(GTK_SCROLLED_WINDOW(border)))
		return false;
	return true;
}

void gTreeView::setBorder(bool vl)
{
	GtkScrolledWindow *wr=GTK_SCROLLED_WINDOW(border);
	
	if (vl)
		gtk_scrolled_window_set_shadow_type(wr,GTK_SHADOW_ETCHED_OUT);
	else
		gtk_scrolled_window_set_shadow_type(wr,GTK_SHADOW_NONE);

}

long gTreeView::scrollBar()
{
	GtkPolicyType h,v;
	long ret=3;
	
	gtk_scrolled_window_get_policy(GTK_SCROLLED_WINDOW(border),&h,&v);
	if (h==GTK_POLICY_NEVER) ret--;
	if (v==GTK_POLICY_NEVER) ret-=2;
	
	return ret;
}

void gTreeView::setScrollBar(long vl)
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


