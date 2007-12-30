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

	//long background();
	//void setBackground(long vl);
	//long foreground();
	//void setForeground(long vl);
	char* text();
	void setText(char* vl);

//"Private"
  GtkWidget *fr;
};

#endif
