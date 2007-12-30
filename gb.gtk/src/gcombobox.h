#ifndef __GCOMBOBOX_H
#define __GCOMBOBOX_H

#include "gtextbox.h"

class gComboBox : public gTextBox
{
public:
	gComboBox(gContainer *parent);

	int count();
	int index();
	char* itemText(int ind);
	virtual int length();
	//char** list();
	virtual bool isReadOnly();
	bool sorted();
	virtual char *text();

	void setIndex(int vl);
	void setItemText(int ind,char *txt);
	//void setList(char **vl);
	virtual void setReadOnly(bool vl);
	void setSorted(bool vl);
	virtual void setText(char *vl);

//"Methods"
	void popup();
	void add(char *vl, int pos = -1);
	virtual void clear();
	int find(char *ptr);
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
};

#endif
