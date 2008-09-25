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

//"Methods"
	void popup();
	void add(const char *vl, int pos = -1);
	virtual void clear();
	int find(const char *ptr);
	void remove(int pos);
	
	virtual void resize(int w, int h);
	virtual void setRealBackground(gColor vl);
	virtual void setRealForeground(gColor vl);
	virtual void setFont(gFont *f);
	virtual void setFocus();
	
//"Signals"
	void (*onClick)(gComboBox *sender);

//"Private"
	bool sort;
	GtkCellRenderer *cell;
	virtual int minimumHeight();
	bool _no_click;
	gTree *tree;
	bool _model_dirty;
	int _last_key;
	
	void updateModel();
	void updateSort();
	char *indexToKey(int index);
	char* find(GtkTreePath *path) { return tree->pathToKey(path, false); }
	void checkIndex();
};

#endif
