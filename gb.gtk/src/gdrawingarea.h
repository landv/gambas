#ifndef __GDRAWINGAREA_H
#define __GDRAWINGAREA_H

class gDrawingArea : public gContainer
{
public:
	gDrawingArea(gContainer *parent);
	~gDrawingArea();

	long foreground();
	long background();
	int getBorder() { return getFrameBorder(); }
	bool cached();
	bool canFocus();
	bool getTracking();

	void setForeground(long vl);
	void setBackground(long vl);
	void setBorder(int vl) { setFrameBorder(vl); }
	void setCached(bool vl);
	void setCanFocus(bool vl);
	void setTracking(bool vl);

//"Methods"
	void clear();
	virtual void resize(int w, int h);

//"Events"
	void (*onExpose)(gDrawingArea *sender,long x,long y,long w,long h);

//"Private"
	GdkPixmap *buffer;
	GdkGC *gc;
	bool track;
	bool berase;
};

#endif
