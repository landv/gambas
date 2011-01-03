#ifndef __GSPINBOX_H
#define __GSPINBOX_H

class gSpinBox : public gControl
{
public:
	gSpinBox(gContainer *parent);

//"Properties"
	//int background();
	//int foreground();
	int maxValue() { return _max; }
	int minValue() { return _min; }
	int step();
	int value();
	bool wrap();

	//void setBackground(int vl);
	//void setForeground(int vl);
	void setMaxValue  (int vl);
	void setMinValue  (int vl);
	void setStep      (int vl);
	void setValue     (int vl);
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
