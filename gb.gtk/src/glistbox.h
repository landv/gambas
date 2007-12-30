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
	int _last_key;
};

#endif
