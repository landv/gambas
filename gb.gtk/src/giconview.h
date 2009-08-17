/***************************************************************************

  giconview.h

  (c) 2000-2009 Benoît Minisini <gambas@users.sourceforge.net>

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
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

***************************************************************************/
#ifndef __GICONVIEW_H
#define __GICONVIEW_H

#include "gcontrol.h"
#include "gtree.h"

class gIconView : public gControl
{
public:
	gIconView(gContainer *parent);
	~gIconView();

//"Properties"
	int count();
	
	char*     itemText(char *key);
	gPicture* itemPicture(char *key);
	bool      isItemSelected(char *key);
	
	void      setItemSelected(char *key, bool vl);
	void      setItemText(char *key,char *vl);
	void      setItemPicture(char *key, gPicture *vl);
	
	bool isItemEditable(char *key) { return tree->isRowEditable(key); }
	void setItemEditable(char *key, bool vl) { tree->setRowEditable(key, vl); }
	
	int       mode();
	void      setMode(int vl);
	int gridWidth() { return tree->gridWidth(); }
	void setGridWidth(int w) { tree->setGridWidth(w); }
	int clientWidth();
	int clientHeight();
	bool isEditable() { return tree->isEditable(); }
	void setEditable(bool vl) { tree->setEditable(vl); }
	
	char*     current() { return tree->cursor(); }
	void      setCurrent(char *key) { tree->setCursor(key); }
	char*     firstItem();
	char*     lastItem();
	char*     nextItem(char *vl);
	char*     prevItem(char *vl);
	
	void moveItemFirst(char *key);
	void moveItemLast(char *key);
	void moveItemBefore(char *key, char *before);
	void moveItemAfter(char *key, char *after);

	bool isSorted() { return tree->isSorted(); }
	void setSorted(bool v) { tree->setSorted(v); }
	bool isSortAscending() { return tree->isSortAscending(); }
	void setSortAscending(bool v) { tree->setSortAscending(v); }

//"Methods"
	bool      add(char *key, char *text, gPicture *pic = NULL, char *after = NULL);
	bool      remove(char *key);
	bool      exists(char *key);
	char*     find(int x, int y);
	void      clear() { tree->clear(); }
	void      ensureItemVisible(char *key) { (*tree)[key]->ensureVisible(); }
	void      rectItem(char *key, int *x, int *y, int *w, int *h) { ((*tree)[key])->rect(x, y, w, h); }
	void			selectAll() { tree->selectAll(); }
	void			unselectAll() { tree->unselectAll(); }
	void startRename(char *key) { (*tree)[key]->startRename(); }
	char *intern(char *key);

//"Events"
	void (*onActivate)(gIconView *sender, char *key);
	void (*onSelect)(gIconView *sender);
	void (*onClick)(gIconView *sender);
	void (*onRemove)(gIconView *sender, char *key);
	void (*onRename)(gIconView *sender, char *key);
	void (*onCancel)(gIconView *sender, char *key);
	bool (*onCompare)(gIconView *sender, char *keya, char *keyb, int *comp);

//"Private"
	char *find(GtkTreePath *path) { return tree->pathToKey(path, false); }

private:
	gIcon *tree;
	GtkWidget *iconview;
};

#endif
