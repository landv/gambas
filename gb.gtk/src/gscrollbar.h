#ifndef __GSCROLLBAR_H
#define __GSCROLLBAR_H

#include "gslider.h"

class gScrollBar : public gSlider
{
public:
	gScrollBar(gContainer *parent);

//"Private"
	virtual void orientation(int w,int h);
};

#endif
