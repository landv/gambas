/***************************************************************************

  glistbox.h

  (c) 2000-2011 Benoît Minisini <gambas@users.sourceforge.net>

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
#ifndef __GLISTBOX_H
#define __GLISTBOX_H

#include "gtreeview.h"

class gListBox : public gTreeView
{
public:
	gListBox(gContainer *parent);

//"Properties"
	int   index();
	bool   isItemSelected(int ind);
	char*  itemText(int ind);
	//char** list();
	char *text() { return itemText(index()); }

	void setIndex(int vl);
	void setItemSelected(int ind, bool vl);
	void setItemText(int ind, char *txt);
	//void setList(char **items);

//"Methods"
	void add(char *vl, int pos = -1);
	int find(char *text);
	void remove(int pos);

private:
	char *indexToKey(int index);
	GtkTreePath *indexToPath(int index);
	int _last_key;
};

#endif
