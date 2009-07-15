/***************************************************************************

  gIconView.cpp

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
#include "giconview.h"

/**************************************************************************

gIconView

***************************************************************************/

static void cb_activate(GtkIconView *widget, GtkTreePath *path, gIconView *control)
{
	control->emit(SIGNAL(control->onActivate), control->find(path));
}

static void cb_select(GtkIconView *widget, gIconView *control)
{
	control->emit(SIGNAL(control->onSelect));
}

static bool cb_click(GtkIconView *widget, GdkEventButton *event, gIconView *control)
{
	if (control->current() && control->isItemSelected(control->current()))
		control->emit(SIGNAL(control->onClick));	
	return false;
}

static void cb_remove(gIcon *tree, char *key)
{
	gIconView *view = (gIconView *)tree->view;
  // This signal ignores locks
	if (view->onRemove)
		(*(view->onRemove))(view, key);
}

gIconView::gIconView(gContainer *parent) : gControl(parent)
{
	g_typ = Type_gIconView;
	use_base = true;

	tree = new gIcon(this);
	tree->onRemove = cb_remove;
	
	iconview = tree->widget;
	realizeScrolledWindow(iconview);
	
	setMode(SELECT_SINGLE);
	setBorder(true);

	onActivate = NULL;	
	onSelect = NULL;
	onClick = NULL;
	onRemove = NULL;
	onCompare = NULL;
	
	g_signal_connect(G_OBJECT(iconview), "item-activated", G_CALLBACK(cb_activate), (gpointer)this);
	g_signal_connect(G_OBJECT(iconview), "selection-changed", G_CALLBACK(cb_select), (gpointer)this);
	g_signal_connect(G_OBJECT(iconview), "button-release-event", G_CALLBACK(cb_click), (gpointer)this);
}

gIconView::~gIconView()
{
  delete tree;
}

bool gIconView::add(char *key, char *text, gPicture *pic, char *after)
{
	gIconRow *row;
	gTreeCell *cell;
	
	row = tree->addRow(key, after);
	
	if (!row) return false;
	
	cell = row->data;
	if (cell)
	{
		cell->setText(text);
		cell->setPicture(pic);
		tree->sortLater();
	}
	
	return true;
}

bool gIconView::remove(char *key)
{
	return tree->removeRow(key);
}

bool gIconView::exists(char *key)
{
	return tree->rowExists(key);
}

int gIconView::count()
{
	return tree->rowCount();
}

int gIconView::mode()
{
	switch (gtk_icon_view_get_selection_mode(GTK_ICON_VIEW(iconview)))
	{
		case GTK_SELECTION_NONE:     return SELECT_NONE;
		case GTK_SELECTION_SINGLE:   return SELECT_SINGLE;
		default: return SELECT_MULTIPLE;
	}
}

void gIconView::setMode(int vl)
{
	switch (vl)
	{
		case SELECT_NONE:
			gtk_icon_view_set_selection_mode(GTK_ICON_VIEW(iconview), GTK_SELECTION_NONE); break;
		case SELECT_SINGLE:
			gtk_icon_view_set_selection_mode(GTK_ICON_VIEW(iconview), GTK_SELECTION_SINGLE); break;
		case SELECT_MULTIPLE:
			gtk_icon_view_set_selection_mode(GTK_ICON_VIEW(iconview), GTK_SELECTION_MULTIPLE); break;
	}
}

char* gIconView::itemText(char *key)
{
	gIconRow *row;
	
	if (!key) return NULL;
	row=tree->getRow(key);
	if (!row) return NULL;
	
	return row->data->text();
}

void gIconView::setItemText(char *key, char *vl)
{
	gIconRow *row;
	
	if (!key) return;
	row=tree->getRow(key);
	if (!row) return;
	row->data->setText(vl);
	tree->sortLater();
}

gPicture* gIconView::itemPicture(char *key)
{
	gIconRow *row;
	
	if (!key) return NULL;
	row=tree->getRow(key);
	if (!row) return NULL;
	
	return row->data->picture();
}

void gIconView::setItemPicture(char *key, gPicture *vl)
{
	gIconRow *row;
	
	if (!key) return;
	row=tree->getRow(key);
	if (!row) return;
	row->data->setPicture(vl);
}

bool gIconView::isItemSelected(char *key)
{
	if (!key) return false;
	return tree->rowSelected(key);
}

void gIconView::setItemSelected(char *key, bool vl)
{
	if (key)
	{
		if (vl && mode() == SELECT_SINGLE)
			tree->setCursor(key);
		tree->setRowSelected(key, vl);
	}
}

char* gIconView::firstItem()
{
	return tree->firstRow();
}

char* gIconView::lastItem()
{
	return tree->lastRow();
}

char* gIconView::nextItem(char *vl)
{
	gIconRow *row=tree->getRow(vl);
	if (!row) return NULL;
	return row->next();
}

char* gIconView::prevItem(char *vl)
{
	gIconRow *row=tree->getRow(vl);
	if (!row) return NULL;
	return row->prev();
}

char* gIconView::find(int x, int y)
{
	GtkTreePath *path;

	path = gtk_icon_view_get_path_at_pos(GTK_ICON_VIEW(iconview), x, y);
	if (path)
		return tree->pathToKey(path);
	else
		return NULL;
}

void gIconView::moveItemFirst(char *key)
{
	(*tree)[key]->moveFirst();
}

void gIconView::moveItemLast(char *key)
{
	(*tree)[key]->moveLast();
}

void gIconView::moveItemBefore(char *key, char *before)
{
	(*tree)[key]->moveBefore(before);
}

void gIconView::moveItemAfter(char *key, char *after)
{
	(*tree)[key]->moveAfter(after);
}

char *gIconView::intern(char *key)
{
	gIconRow *row = (*tree)[key];
	return row ? row->key() : NULL;
}

int gIconView::clientWidth()
{
	GtkAdjustment* Adj;
	
	Adj=gtk_scrolled_window_get_hadjustment(_scroll);
	return (int)Adj->page_size;
}

int gIconView::clientHeight()
{
	GtkAdjustment* Adj;
	
	Adj=gtk_scrolled_window_get_vadjustment(_scroll);
	return (int)Adj->page_size;
}

