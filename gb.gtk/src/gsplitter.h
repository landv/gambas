#ifndef __GSPLITTER_H
#define __GSPLITTER_H

class gSplitter : public gContainer
{
public:
	gSplitter(gContainer *parent, bool vert);

//"Properties"
	char* layout();
	void  setLayout(char *vl);

	//int childCount();
	//gControl* child(int index);

//"Methods"
	virtual void performArrange();
	virtual void resize(int w, int h);

//"Signals"
	void (*onResize)(gControl *sender);

//"Private"
	GtkPaned *curr;
	virtual void insert(gControl *child);
	virtual void remove(gControl *child);
	bool vertical;
	GtkPaned *next(GtkPaned *iter);
};

#endif
