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
#include "gtreeview.h"

/**************************************************************************

gTreeView

***************************************************************************/

static void cb_activate(GtkTreeView *widget, GtkTreePath *path, GtkTreeViewColumn *column, gTreeView *control)
{
	char *key = control->find(path);
	control->setItemExpanded(key, !control->isItemExpanded(key));
	// Be careful, do not reuse key, as it can be destroyed during the Expand event
	control->emit(SIGNAL(control->onActivate), control->find(path));
}

static void cb_select(GtkTreeSelection *selection, gTreeView *control)
{
	control->emit(SIGNAL(control->onSelect));
}

static void cb_click(GtkWidget *widget, gTreeView *control)
{
	control->emit(SIGNAL(control->onClick));
}

static bool cb_button_press(GtkWidget *widget, GdkEventButton *e, gTreeView *control)
{
	if (e->button != 1)
		control->setCurrent(control->find((int)e->x, (int)e->y));
	return false;
}

static void cb_expand(GtkTreeView *widget, GtkTreeIter *iter, GtkTreePath *path, gTreeView *control)
{
	if (control->locked())
		return;
		
	char *key = control->find(path);
	control->refreshExpanded(key, true);
	control->emit(SIGNAL(control->onExpand), key);
}

static void cb_collapse(GtkTreeView *widget, GtkTreeIter *iter, GtkTreePath *path, gTreeView *control)
{
	if (control->locked())
		return;
		
	char *key = control->find(path);
	control->refreshExpanded(key, false);
	control->emit(SIGNAL(control->onCollapse), key);
}

static void cb_remove(gTree *tree, char *key)
{
	gTreeView *view = (gTreeView *)tree->view;
  // This signal ignores locks
	if (view->onRemove)
		(*(view->onRemove))(view, key);
}

gTreeView::gTreeView(gContainer *parent, bool list) : gControl(parent)
{
	GtkTreeSelection *sel;
	
	g_typ=Type_gTreeView;
	use_base = true;

	tree=new gTree(this);
	tree->addColumn();
	tree->setHeaders(false);
	tree->onRemove = cb_remove;
	
	gtk_tree_view_set_rules_hint(GTK_TREE_VIEW(tree->widget), true);
	
	border=gtk_scrolled_window_new(NULL,NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(border), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	widget=tree->widget;
	
	// BM: must occurs before!
	g_signal_connect(G_OBJECT(widget),"button-press-event",G_CALLBACK(cb_button_press),(gpointer)this);
	
	realize();
		
	setMode(SELECT_SINGLE);
	setBorder(true);

	onActivate = NULL;	
	onSelect = NULL;
	onClick = NULL;
	onCollapse = NULL;
	onExpand = NULL;
	onRemove = NULL;
	onRename = NULL;
	onCancel = NULL;
	onCompare = NULL;
	
	sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(tree->widget));
	
	g_signal_connect(G_OBJECT(widget),"row-activated",G_CALLBACK(cb_activate),(gpointer)this);
	g_signal_connect(G_OBJECT(sel),"changed",G_CALLBACK(cb_select),(gpointer)this);
	g_signal_connect(G_OBJECT(widget),"row-expanded",G_CALLBACK(cb_expand),(gpointer)this);
	g_signal_connect(G_OBJECT(widget),"row-collapsed",G_CALLBACK(cb_collapse),(gpointer)this);
	g_signal_connect(G_OBJECT(widget),"cursor-changed",G_CALLBACK(cb_click),(gpointer)this);
}

gTreeView::~gTreeView()
{
  delete tree;
}


bool gTreeView::add(char *key,char *text,gPicture *pic,char *after,char *parent)
{
	gTreeRow *row;
	gTreeCell *cell;
	
	row = tree->addRow(key,parent,after);
	
	if (!row) return false;
	
	cell = row->get(0);
	if (cell)
	{
		cell->setText(text);
		cell->setPicture(pic);
		tree->sortLater();
	}
	
	//if ( tree->rowCount()==1) tree->setCursor(key);
	
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

int gTreeView::count()
{
	return tree->rowCount();
}

int gTreeView::mode()
{
	GtkTreeSelection *sel=gtk_tree_view_get_selection(GTK_TREE_VIEW(tree->widget));
	
	switch (gtk_tree_selection_get_mode (sel))
	{
		case GTK_SELECTION_NONE:     return 0;
		case GTK_SELECTION_SINGLE:   return 1;
		default: return 2;
	}
}

void gTreeView::setMode(int vl)
{
	GtkTreeSelection *sel=gtk_tree_view_get_selection(GTK_TREE_VIEW(tree->widget));
	
	switch (vl)
	{
		case SELECT_NONE:
			gtk_tree_selection_set_mode(sel,GTK_SELECTION_NONE); break;
		case SELECT_SINGLE:
			gtk_tree_selection_set_mode(sel,GTK_SELECTION_SINGLE); break;
		case SELECT_MULTIPLE:
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
	return itemText(key, 0);
}

void gTreeView::setItemText(char *key, char *vl)
{
	setItemText(key, 0, vl);
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
	
	if (!key) return NULL;
	row=tree->getRow(key);
	if (!row) return NULL;
	
	return row->get(0)->picture();
}

void gTreeView::setItemPicture(char *key,gPicture *vl)
{
	gTreeRow *row;
	
	if (!key) return;
	row=tree->getRow(key);
	if (!row) return;
	row->get(0)->setPicture(vl);
}

bool gTreeView::isItemSelected(char *key)
{
	if (!key) return false;
	return tree->rowSelected(key);
}

void gTreeView::setItemSelected(char *key, bool vl)
{
	if (key)
	{
		if (vl && mode() == SELECT_SINGLE)
			tree->setCursor(key);
		tree->setRowSelected(key, vl);
	}
}

char* gTreeView::firstItem(char *parent)
{
	if (!parent)
		return tree->firstRow();
		
	gTreeRow *row=tree->getRow(parent);
	if (!row) return NULL;
	return row->child();
}

char* gTreeView::lastItem(char *parent)
{
	if (!parent)
		return tree->lastRow();
		
	gTreeRow *row=tree->getRow(parent);
	if (!row) return NULL;
	return row->last();
}

char* gTreeView::nextItem(char *vl)
{
	gTreeRow *row=tree->getRow(vl);
	if (!row) return NULL;
	return row->next();
}

char* gTreeView::prevItem(char *vl)
{
	gTreeRow *row=tree->getRow(vl);
	if (!row) return NULL;
	return row->prev();
}

char* gTreeView::parentItem(char *vl)
{
	gTreeRow *row=tree->getRow(vl);
	if (!row) return NULL;
	return row->parent();
}

char* gTreeView::aboveItem(char *vl)
{
	gTreeRow *row=tree->getRow(vl);
	if (!row) return NULL;
	return row->above();
}

char* gTreeView::belowItem(char *vl)
{
	gTreeRow *row=tree->getRow(vl);
	if (!row) return NULL;
	return row->below();
}


char* gTreeView::find(int x, int y)
{
	GtkTreePath *path;

	if (gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(tree->widget), x, y, &path, NULL, NULL, NULL))
		return tree->pathToKey(path);
	else
		return NULL;
}

bool gTreeView::getBorder()
{
	return gtk_scrolled_window_get_shadow_type(GTK_SCROLLED_WINDOW(border)) != GTK_SHADOW_NONE;
}

void gTreeView::setBorder(bool vl)
{
	GtkScrolledWindow *wr=GTK_SCROLLED_WINDOW(border);
	
	if (vl)
		gtk_scrolled_window_set_shadow_type(wr,GTK_SHADOW_IN);
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


bool gTreeView::isItemExpanded(char *key)
{
  return (*tree)[key]->isExpanded();
}

void gTreeView::setItemExpanded(char *key, bool ex)
{
	gTreeRow *row = (*tree)[key];
	if (row->isExpanded() != ex)
  	row->setExpanded(ex);
}

void gTreeView::setColumnCount(int ncol)
{
	while (columnCount() < ncol)
		tree->addColumn();
	while (columnCount() > ncol)
		tree->removeColumn();
}

char *gTreeView::columnText(int col)
{
	return tree->columnName(col);
}

void gTreeView::setColumnText(int col, char *text)
{
	tree->setColumnName(col, text);
}

char *gTreeView::itemText(char *key, int col)
{
	gTreeRow *row;
	gTreeCell *cell;
	
	if (!key) return NULL;
	row=tree->getRow(key);
	if (!row) return NULL;
	cell = row->get(col);
	if (!cell) return NULL;
	return cell->text();
}

void gTreeView::setItemText(char *key, int col, char *text)
{
	gTreeRow *row;
	gTreeCell *cell;
	
	if (!key) return;
	row=tree->getRow(key);
	if (!row) return;
	cell = row->get(col);
	if (!cell) return;
	
	cell->setText(text);
	if (col == getSortColumn())
		tree->sortLater();
	
	row->update();	
}

void gTreeView::refreshExpanded(char *parent, bool ex)
{
	char *key;
	gTreeRow *row;
	
	//fprintf(stderr, "refreshExpanded: %s %d\n", parent, ex);
	
	row = tree->getRow(parent);
	row->updateExpanded(ex);
	if (!row->isExpanded())
		return;
	
	key = tree->getRow(parent)->child();
	if (!key)
		return;
		
	while (key)
	{
		//fprintf(stderr, "check: %s\n", key);
		row = tree->getRow(key);
		if (row->isExpanded())
		{
			row->setExpanded();
			refreshExpanded(key, true);
		}
		key = row->next();	
	}
}


void gTreeView::moveItemFirst(char *key)
{
	(*tree)[key]->moveFirst();
}

void gTreeView::moveItemLast(char *key)
{
	(*tree)[key]->moveLast();
}

void gTreeView::moveItemBefore(char *key, char *before)
{
	(*tree)[key]->moveBefore(before);
}

void gTreeView::moveItemAfter(char *key, char *after)
{
	(*tree)[key]->moveAfter(after);
}

char *gTreeView::intern(char *key)
{
	gTreeRow *row = (*tree)[key];
	return row ? row->key() : NULL;
}
