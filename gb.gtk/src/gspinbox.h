#ifndef __GSPINBOX_H
#define __GSPINBOX_H

class gSpinBox : public gControl
{
public:
	gSpinBox(gContainer *parent);

//"Properties"
	long background();
	long foreground();
	long maxValue() { return _max; }
	long minValue() { return _min; }
	long step();
	long value();
	bool wrap();

	void setBackground(long vl);
	void setForeground(long vl);
	void setMaxValue  (long vl);
	void setMinValue  (long vl);
	void setStep      (long vl);
	void setValue     (long vl);
	void setWrap      (bool vl);

//"Methods"
	void selectAll();

//"Signals"
	void (*onChange)  (gSpinBox *sender);

//"Private"
private:
	int _min;
	int _max;
	virtual void updateCursor(GdkCursor *cursor);
};

#endif
