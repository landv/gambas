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
#ifndef GTREE_H
#define GTREE_H

#include <gtk/gtk.h>

class gTree;

class gTreeCell
{
public:
	char *text;
	GdkPixbuf *pic;
	
	void setText(char *vl);
	void setPixbuf(GdkPixbuf *vl);
	gTreeCell();
	~gTreeCell();
};

class gTreeRow
{
public:
	gTree *parent;
	GList *data;
	GtkTreeIter *dataiter;
	char *dataparent;
	
	gTreeRow(gTree *tree,unsigned long count,GtkTreeIter *iter,char *par);
	~gTreeRow();
	
	void  add();
	void  remove();
	void  update();
	void  expand();
	void  collapse();
	long  children();
	char* parentKey();
	
	gTreeCell* get(unsigned long ind);
};

class gTree
{
	
public:
	GtkWidget *tree;
	GtkTreeStore *store;
	GtkCellRenderer *rtext;
	GtkCellRenderer *rgraph;
	GHashTable *datakey;
	
	//General
	long visibleWidth();
	long visibleHeight();
	char* cursor();
	void  setCursor(char *vl);
	
	// Rows
	gTreeRow* addRow(char *key,char *parent=NULL,char *after=NULL);
	gTreeRow* getRow(char *key);
	bool      removeRow(char *key);
	unsigned long rowCount();
	void clearRows();
	bool rowExists(char *key);
	bool rowSelected(char *key);
	void setRowSelected(char *key,bool vl);
	char* firstTopRow();
	char* lastTopRow();
	char* nextRow(gTreeRow *row);
	char* prevRow(gTreeRow *row);
	// Columns
	bool headers();
	void setHeaders(bool vl);
	void addColumn();
	void removeColumn();
	unsigned long columnCount();
	const char* columnName(unsigned long ind);
	void setColumnName(unsigned long ind,char *vl);
	bool columnVisible(unsigned long ind);
	void setColumnVisible(unsigned long ind,bool vl);
	bool columnResizable(unsigned long ind);
	void setColumnResizable(unsigned long ind,bool vl);
	
	gTree();
	~gTree();
};

#endif
