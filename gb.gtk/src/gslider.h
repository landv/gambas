#ifndef __GSLIDER_H
#define __GSLIDER_H

class gSlider : public gControl
{
public:
	gSlider(gContainer *parent);

//"Properties"
	int foreground();
	int background();
	int max();
	int min();
	bool tracking();
	int value();
	bool mark();
	int step();
	int pageStep();

	void setForeground(int vl);
	void setBackground(int vl);
	void setMax(int vl);
	void setMin(int vl);
	void setTracking(bool vl);
	void setValue(int vl);
	void setMark(bool vl);
	void setStep(int vl);
	void setPageStep(int vl);
	
	virtual void resize(int w, int h);

//"Signals"
	void (*onChange)(gSlider *sender);

//"Private"
	virtual void orientation(int w,int h);
	bool bDraw;
	int p_step;
	int p_page;
};

#endif
