/***************************************************************************

  gtree.h

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

#ifndef __GTREE_H
#define __GTREE_H

#include <gtk/gtk.h>

class gTree;
class gTreeView;
class gIconView;

class gTreeCell
{
public:
	gTreeCell();
	~gTreeCell();
	
	char *text() { return _text; }
	gPicture *picture() { return _picture; }
	
	void setText(char *vl);
	void setPicture(gPicture *vl);
	
private:
	char *_text;
	gPicture *_picture;
};

class gTreeRow
{
public:
	GList *data;
	GtkTreeIter *dataiter;
	gTree *tree;
	
	gTreeRow(gTree *tr, char *key, GtkTreeIter *iter);
	~gTreeRow();
	
	void  add();
	void  remove();
	void  update();
	long  children();
	char *key() { return _key; }
	
	void setExpanded();
	void  setExpanded(bool ex);
	bool isExpanded();
	void ensureVisible();
	
	gTreeCell* get(int ind);

	char *next();
	char *prev();
	char *child();
	char *last();
	char *parent();
	char *above();
	char *below();
	
	void moveFirst();
	void moveLast();
	void moveAfter(char *key);
	void moveBefore(char *key);
	
	void rect(int *x, int *y, int *w, int *h);
	void updateExpanded(bool ex);
	
	bool isEditable() { return _editable; }
	void setEditable(bool vl) { _editable = vl; }
	
	void startRename();
	
private:
	char *_key;
	char *_parent;
	bool _expanded;
	bool _editable;
};

class gTree
{
public:
	GtkWidget *widget;
	GtkTreeStore *store;
	GtkCellRenderer *rtext;
	GtkCellRenderer *rgraph;
	GHashTable *datakey;
	gTreeView *view;
	void (*onRemove)(gTree *tree, char *key);
	char *_edited_row;
	unsigned _editable : 1;
	unsigned _resizable : 1;
	unsigned _auto_resize : 1;
	unsigned _expander : 1;
	unsigned _sorted : 1;
	unsigned _ascending : 1;
	unsigned _init_sort : 1;
	unsigned _sort_dirty : 1;
	int _sort_column;
	
	gTree(gTreeView *v);
	~gTree();
	
	//General
	long visibleWidth();
	long visibleHeight();
	char* cursor();
	void  setCursor(char *vl);
	void selectAll();
	void unselectAll();
	bool isEditable() { return _editable; }
	void setEditable(bool vl) { _editable = vl; }
	
	// Rows
	gTreeRow* addRow(char *key,char *parent=NULL,char *after=NULL);
	gTreeRow* getRow(char *key) const;
	gTreeRow* operator[](char *key) const { return getRow(key); }
	
	bool removeRow(char *key);
	int rowCount();
	bool rowExists(char *key);
	bool rowSelected(char *key);
	void setRowSelected(char *key,bool vl);
	bool isRowEditable(char *key);
	void setRowEditable(char *key, bool vl);
	char *firstRow();
	char *lastRow();

	void clear();
	void clear(char *parent);

	// Columns
	bool headers();
	void setHeaders(bool vl);
	bool isResizable() { return _resizable; }
	void setResizable(bool vl);
	bool isAutoResize() { return _auto_resize; }
	void setAutoResize(bool vl);
	void addColumn();
	void removeColumn();
	int columnCount();
	char *columnName(int ind);
	void setColumnName(int ind,char *vl);
	bool columnVisible(int ind);
	void setColumnVisible(int ind, bool vl);
	bool columnResizable(int ind);
	void setColumnResizable(int ind, bool vl);
	int columnAlignment(int ind);
	void setColumnAlignment(int ind, int a);
	int columnWidth(int col);
	void setColumnWidth(int col, int w);
	
	bool isSorted() { return _sorted; }
	void setSorted(bool v);
	int getSortColumn() { return _sort_column; }
	void setSortColumn(int v);
	bool isSortAscending() { return _ascending; }
	void setSortAscending(bool v);

	char *pathToKey(GtkTreePath *path, bool free = true);
	char *iterToKey(GtkTreeIter *iter);
	
	void showExpanders();
	bool hasExpanders() { return _expander; }
	void sortLater();
	void sort();
	void updateSort();
};

class gIcon;
class gIconView;

class gIconRow
{
public:
	gTreeCell *data;
	GtkTreeIter *dataiter;
	gIcon *tree;
	
	gIconRow(gIcon *tr, char *key, GtkTreeIter *iter);
	~gIconRow();
	
	void  add();
	void  remove();
	void  update();
	char *key() { return _key; }
	
	void ensureVisible();
	
	gTreeCell* get(int ind) { return data; }

	char *next();
	char *prev();
	char *last();
	
	void moveFirst();
	void moveLast();
	void moveAfter(char *key);
	void moveBefore(char *key);
	
	void rect(int *x, int *y, int *w, int *h);
	
	bool isEditable() { return _editable; }
	void setEditable(bool vl) { _editable = vl; }
	
	void startRename();
	
private:
	char *_key;
	char *_parent;
	bool _editable;
};

class gIcon
{
public:
	GtkWidget *widget;
	GtkListStore *store;
	GtkCellRenderer *rtext;
	GtkCellRenderer *rgraph;
	GHashTable *datakey;
	gIconView *view;
	void (*onRemove)(gIcon *tree, char *key);
	int _grid_width;
	char *_edited_row;
	unsigned _word_wrap : 1;
	unsigned _editable : 1;
	unsigned _sorted : 1;
	unsigned _ascending : 1;
	unsigned _init_sort : 1;
	unsigned _sort_dirty : 1;

	gIcon(gIconView *v);
	~gIcon();
	
	//General
	char* cursor();
	void  setCursor(char *vl);
	void selectAll();
	void unselectAll();
	bool isEditable() { return _editable; }
	void setEditable(bool vl) { _editable = vl; }
	
	// Rows
	gIconRow* addRow(char *key, char *after=NULL);
	gIconRow* getRow(char *key) const;
	gIconRow* operator[](char *key) const { return getRow(key); }
	
	bool removeRow(char *key);
	int rowCount();
	bool rowExists(char *key);
	bool rowSelected(char *key);
	void setRowSelected(char *key,bool vl);
	bool isRowEditable(char *key);
	void setRowEditable(char *key, bool vl);
	char *firstRow();
	char *lastRow();

	bool isSorted() { return _sorted; }
	void setSorted(bool v);
	bool isSortAscending() { return _ascending; }
	void setSortAscending(bool v);

	char *pathToKey(GtkTreePath *path, bool free = true);
	char *iterToKey(GtkTreeIter *iter);
	
	int gridWidth() { return _grid_width; }
	void setGridWidth(int w);
	bool wordWrap() { return _word_wrap; }
	void setWordWrap(bool vl);
	void clear();
	
	void updateTextCell();
	void sort();
	void updateSort() { sortLater(); }
	void sortLater();
};

#endif
