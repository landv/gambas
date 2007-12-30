#ifndef __GSCROLLBAR_H
#define __GSCROLLBAR_H

#include "gslider.h"

class gScrollBar : public gSlider
{
public:
	gScrollBar(gContainer *parent);
	virtual void resize(int w, int h);
};

#endif
