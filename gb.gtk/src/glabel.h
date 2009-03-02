#ifndef __GSIMPLELABEL_H
#define __GSIMPLELABEL_H

#include "gcontrol.h"

class gLabel : public gControl
{
public:
	gLabel(gContainer *parent);
	~gLabel();

	int alignment();
	int getBorder() { return getFrameBorder(); }
	char* text();
	bool isTransparent() { return _transparent; }
	bool autoResize();
	int padding() { return getFramePadding(); }

	void setAlignment(int al);
	void setBorder(int vl) { setFrameBorder(vl); }
	void setText(char *st);
	virtual void setFont(gFont *ft);
	void setTransparent(bool vl);
	void setAutoResize(bool vl);
	void setPadding(int vl) { setFramePadding(vl); }

//"Methods"
	void enableMarkup(bool vl);
	void adjust();
	virtual void resize(int w, int h);
	virtual void afterRefresh();

//"Private"
	void updateSize(bool adjust = false, bool noresize = false);
	void updateLayout();
	PangoLayout *layout;
	int align,lay_x,lay_y;
	unsigned markup : 1;
	unsigned autoresize : 1;
	unsigned _transparent : 1;
	unsigned _mask_dirty : 1;
	unsigned _locked : 1;
	char *textdata;
};

#endif
