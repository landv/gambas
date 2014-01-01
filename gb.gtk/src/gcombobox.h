/***************************************************************************

  gcombobox.h

  (c) 2000-2013 Benoît Minisini <gambas@users.sourceforge.net>

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

#ifndef __GCOMBOBOX_H
#define __GCOMBOBOX_H

#include "gtextbox.h"
#include "gtree.h"

class gComboBox : public gTextBox
{
public:
	gComboBox(gContainer *parent);
	~gComboBox();

	int count();
	int index();
	char* itemText(int ind);
	virtual int length();
	//char** list();
	virtual bool isReadOnly();
	bool isSorted();
	virtual char *text();

	void setIndex(int vl);
	void setItemText(int ind, const char *txt);
	//void setList(char **vl);
	virtual void setReadOnly(bool vl);
	void setSorted(bool vl);
	virtual void setText(const char *vl);
	
	bool hasBorder() const;
	void setBorder(bool v);

//"Methods"
	void popup();
	void add(const char *vl, int pos = -1);
	virtual void clear();
	int find(const char *ptr);
	void remove(int pos);
	
	virtual void resize(int w, int h);
#ifdef GTK3
	virtual void updateColor();
#else
	virtual void setRealBackground(gColor vl);
#endif
	virtual void setRealForeground(gColor vl);
	virtual void setFocus();
	
//"Signals"
	void (*onClick)(gComboBox *sender);

//"Private"
	bool sort;
	GtkCellRenderer *cell;
	virtual int minimumHeight();
	gTree *tree;
	bool _model_dirty;
	int _last_key;
	GtkWidget *_button;
	int _model_dirty_timeout;
	
	virtual void updateFont();
	void updateModel();
	void updateSort();
	char *indexToKey(int index);
	char* find(GtkTreePath *path) { return tree->pathToKey(path, false); }
	void checkIndex();
	void updateFocusHandler();
	void create(bool readOnly);
};

#endif
