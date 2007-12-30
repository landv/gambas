#ifndef __GPROGRESSBAR_H
#define __GPROGRESSBAR_H

class gProgressBar : public gControl
{
public:
	gProgressBar(gContainer *parent);

	bool label();
	double value();

	void setLabel(bool vl);
	void setValue(double vl);

	void reset();

//"Private"
	bool lbl;
};

#endif
