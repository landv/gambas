#ifndef __GFRAME_H
#define __GFRAME_H

#include "gcontainer.h"

class gPanel : public gContainer
{
public:
	gPanel(gContainer *parent);

	int getBorder() { return getFrameBorder(); }
	void setBorder(int vl) { setFrameBorder(vl); }
};

class gFrame : public gContainer
{
public:
	gFrame(gContainer *parent);

	char* text();
	void setText(char* vl);

	virtual void setFont(gFont *ft);
	virtual void setRealForeground(gColor color);

//"Private"
  GtkWidget *fr;
	GtkWidget *label;
};

#endif
