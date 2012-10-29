/***************************************************************************

  gtree.h

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
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
  MA 02110-1301, USA.

***************************************************************************/

#ifndef __GTREE_H
#define __GTREE_H

#include <gtk/gtk.h>

class gTree;

class gTreeCell
{
public:
	gTreeCell();
	~gTreeCell();
	
	char *text() { return _text; }
	gPicture *picture() { return _picture; }
	
	void setText(const char *vl);
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
	//int  children();
	char *key() { return _key; }
	
	//void setExpanded();
	//void  setExpanded(bool ex);
	//bool isExpanded();
	void ensureVisible();
	
	gTreeCell* get(int ind);

	/*char *next();
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
	void updateExpanded(bool ex);*/
	
	//bool isEditable() { return _editable; }
	//void setEditable(bool vl) { _editable = vl; }
	
	//void startRename();
	
private:
	char *_key;
	char *_parent;
	//bool _expanded;
	//bool _editable;
};

class gTree
{
public:
	GtkWidget *widget;
	GtkTreeStore *store;
	GtkCellRenderer *rtext;
	GtkCellRenderer *rgraph;
	GHashTable *datakey;
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
	int _no_click;
	int _sort_column;
	
	gTree();
	~gTree();
	
	//General
	//int visibleWidth();
	//int visibleHeight();
	char* cursor();
	void  setCursor(char *vl);
	void selectAll();
	void unselectAll();
	//bool isEditable() { return _editable; }
	//void setEditable(bool vl) { _editable = vl; }
	//bool isRenaming() const { return _edited_row != NULL; }
	
	// Rows
	gTreeRow* addRow(char *key, char *after = NULL, bool before = false);
	gTreeRow* getRow(char *key) const;
	gTreeRow* operator[](char *key) const { return getRow(key); }
	
	bool removeRow(char *key);
	int rowCount();
	bool rowExists(char *key);
	bool rowSelected(char *key);
	void setRowSelected(char *key,bool vl);
	//bool isRowEditable(char *key);
	//void setRowEditable(char *key, bool vl);
	char *firstRow();
	//char *lastRow();

	void clear();
	//void clear(char *parent);

	// Columns
	//bool headers();
	//void setHeaders(bool vl);
	/*bool isResizable() { return _resizable; }
	void setResizable(bool vl);*/
	bool isAutoResize() { return _auto_resize; }
	void setAutoResize(bool vl);
	void addColumn();
	void removeColumn();
	int columnCount();
	/*char *columnName(int ind);
	void setColumnName(int ind,char *vl);
	bool columnVisible(int ind);
	void setColumnVisible(int ind, bool vl);
	bool columnResizable(int ind);
	void setColumnResizable(int ind, bool vl);
	int columnAlignment(int ind);
	void setColumnAlignment(int ind, int a);
	int columnWidth(int col);
	void setColumnWidth(int col, int w);*/
	
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

#endif
