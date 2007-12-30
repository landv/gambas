#ifndef __GSCROLLVIEW_H
#define __GSCROLLVIEW_H

class gScrollView : public gContainer
{
public:
	gScrollView(gContainer *parent);

//"Properties"
	bool hasBorder();
	void setBorder(bool vl);

//"Methods"
	virtual void resize(int w,int h);
	void ensureVisible(int x, int y, int w, int h);

//"Signals"
	void (*onScroll)(gScrollView *sender);

//"Private"
	virtual void performArrange();
	
private:
  GtkWidget *_scroll;
};

#endif
