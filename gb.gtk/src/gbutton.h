#ifndef __GBUTTON_H
#define __GBUTTON_H

class gButton : public gControl
{
public:

  enum Type
  {
    Button, Toggle, Check, Radio, Tool
  };

	gButton(gContainer *parent, Type type);
  ~gButton();
  
	bool getBorder();
	bool isCancel();
	bool isDefault();
	const char* text();
	gPicture* picture();
	bool value();
	bool isToggle();
	bool isRadio();
	bool enabled();
	bool inconsistent();
	bool isStretch() { return _stretch; }
	bool isTristate() const { return _tristate; }

	void setEnabled(bool vl);
	void setBorder(bool vl);
	void setCancel(bool vl);
	void setDefault(bool vl);
	void setText(const char *st);
	void setPicture(gPicture *pic);
	void setValue(bool vl);
	void setToggle(bool vl);
	void setRadio(bool vl);
	void setInconsistent(bool vl);
	void setStretch(bool vl);
	void setTristate(bool vl);
	
	virtual void setFont(gFont *ft);
	virtual void setRealForeground(gColor color);

//"Method"
	void animateClick(bool on);

//"Signals"
	void (*onClick)(gControl *sender);

//"Private"
	int type;
	char *bufText;
	GtkCellRenderer *rendtxt;
	GtkWidget *label;
	GdkPixbuf *rendpix,*rendinc;
	gPicture *pic;
	int shortcut;
	unsigned scaled : 1;
	unsigned disable : 1;
	unsigned _toggle : 1;
	unsigned _animated : 1;
	unsigned _radio : 1;
	unsigned _stretch : 1;
	unsigned _tristate : 1;
	
	bool hasShortcut();
	void unsetOtherRadioButtons();
	virtual int minimumHeight();
	
	static bool isButton(gControl *control) { return control->getClass() == Type_gButton && ((gButton *)control)->type == Button; }
};

#endif
