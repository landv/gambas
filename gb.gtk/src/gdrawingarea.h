#ifndef __GDRAWINGAREA_H
#define __GDRAWINGAREA_H

class gDrawingArea : public gContainer
{
public:
	gDrawingArea(gContainer *parent);
	~gDrawingArea();

	int getBorder() { return getFrameBorder(); }
	bool cached() { return _cached; }
	bool canFocus();
	bool getTracking();

	void setBorder(int vl) { setFrameBorder(vl); }
	void setCached(bool vl);
	void setCanFocus(bool vl);
	void setTracking(bool vl);

//"Methods"
	void clear();
	virtual void resize(int w, int h);
	virtual void setEnabled(bool vl);

//"Events"
	void (*onExpose)(gDrawingArea *sender,int x,int y,int w,int h);

//"Private"
	void updateCache();
	void resizeCache();
	void refreshCache();
	void updateEventMask();
	void setCache();
	GdkPixmap *buffer;
	uint _event_mask;
	uint _old_bg_id;
	unsigned track : 1;
	unsigned _cached : 1;
	unsigned _resize_cache : 1;
};

#endif
