#ifndef __GSCROLLVIEW_H
#define __GSCROLLVIEW_H

class gScrollView : public gContainer
{
public:
	gScrollView(gContainer *parent);

//"Properties"
	virtual int scrollWidth() { return _mw; }
	virtual int scrollHeight() { return _mh; }

//"Methods"
	virtual void resize(int w,int h);
	void ensureVisible(int x, int y, int w, int h);

//"Signals"
	void (*onScroll)(gScrollView *sender);

//"Private"
	virtual void performArrange();
  void updateSize();
	
private:
	GtkWidget *viewport;
  int _mw, _mh;
};

#endif
