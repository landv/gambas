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
#include "glistbox.h"

gListBox::gListBox(gContainer *parent) : gTreeView(parent, true)
{
	g_typ=Type_gListBox;
	_last_key = 0;
}

static int pathToIndex(GtkTreePath *path)
{
	gint *indices;
	
	if (!path) 
		return -1;
	
	indices = gtk_tree_path_get_indices(path);
	return indices[0];
}

static GtkTreePath *indexToPath(int index)
{
	char buffer[16];
	sprintf(buffer, "%d", index);
	return gtk_tree_path_new_from_string(buffer);
}

char *gListBox::indexToKey(int index)
{
	char *key;
	GtkTreePath *path = indexToPath(index);
	key = gTreeView::find(path);
	gtk_tree_path_free(path);
	return key;	
}

int gListBox::index()
{
	GtkTreePath *path;
	
	if (mode() == SELECT_NONE)
		return -1;
	
	gtk_tree_view_get_cursor(GTK_TREE_VIEW(widget), &path, NULL);
	return pathToIndex(path);
}

void gListBox::setIndex(int ind)
{
	GtkTreePath *path = indexToPath(ind);
	gtk_tree_view_set_cursor(GTK_TREE_VIEW(widget), path, NULL, false);
	gtk_tree_path_free(path);
}

bool gListBox::isItemSelected(int ind)
{
	return gTreeView::isItemSelected(indexToKey(ind));
}

void gListBox::setItemSelected(int ind, bool vl)
{
	gTreeView::setItemSelected(indexToKey(ind), vl);
}

char *gListBox::itemText(int ind)
{
	if (ind < 0)
		return NULL;
		
	return gTreeView::itemText(indexToKey(ind));	
}

void gListBox::setItemText(int ind, char *txt)
{
	gTreeView::setItemText(indexToKey(ind), txt);
}

/*
char** gListBox::list()
{
	char **items;
	int i;
	
	if (count() == 0)
		return NULL;
	
	items = g_malloc(sizeof(char *) * count());
	for (i = 0; i < count(); i++)
		items[i] = g_strdup(itemText(i));

	return items;
}

void gListBox::setList(char **items, int count)
{
	int i;
	
	clear();
	
	for (i = 0; i < count; i++)
		add(items[i]);
}
*/

void gListBox::add(char *text, int pos)
{
	char key[16];
	char *after;
	
	_last_key++;
	sprintf(key, "%d", _last_key);
	
	if (pos < 0)
		gTreeView::add(key, text);
	else
	{
		after = indexToKey(pos);
		gTreeView::add(key, text, NULL, after);
	}
}

void gListBox::remove(int pos)
{
	gTreeView::remove(indexToKey(pos));
}


int gListBox::find(char *text)
{
	int i;
	
	for (i = 0; i < count(); i++)
	{
		if (!strcmp(itemText(i), text))
			return i;
	}
	
	return -1;
}
