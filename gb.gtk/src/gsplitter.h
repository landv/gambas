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
	virtual void insert(gControl *child);
	virtual void remove(gControl *child);
	void updateChild(GtkWidget *wid = 0);
	void updateVisibility();
	GtkPaned *curr;
	bool vertical;
	GtkPaned *next(GtkPaned *iter);
	int handleCount();
	bool _notify;
};

#endif
