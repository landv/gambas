#ifndef __GSCROLLVIEW_H
#define __GSCROLLVIEW_H

class gScrollView : public gContainer
{
public:
	gScrollView(gContainer *parent);

//"Properties"
	bool hasBorder();
	void setBorder(bool vl);

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
  guint _timer;
	
private:
  GtkWidget *_scroll;
	GtkWidget *viewport;
  int _mw, _mh;
};

#endif
