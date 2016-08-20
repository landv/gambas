/***************************************************************************

  gbutton.h

  (c) 2000-2013 Benoît Minisini <gambas@users.sourceforge.net>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
  MA 02110-1301, USA.

***************************************************************************/

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
	//bool isEnabled() const;
	bool inconsistent();
	bool isStretch() { return _stretch; }
	bool isTristate() const { return _tristate; }
	bool isAutoResize() const { return _autoresize; }

	//void setEnabled(bool vl);
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
	void setAutoResize(bool vl);
	
	virtual void setRealForeground(gColor color);
	//virtual void setRealBackground(gColor color);

//"Method"
	void animateClick(bool on);

//"Signals"
	void (*onClick)(gControl *sender);

//"Private"
	int type;
	char *bufText;
	GtkWidget *_label;
	GtkCellRenderer *rendtxt;
	GdkPixbuf *rendpix,*rendinc;
	gPicture *pic;
	int shortcut;
	unsigned disable : 1;
	unsigned _toggle : 1;
	unsigned _animated : 1;
	unsigned _radio : 1;
	unsigned _stretch : 1;
	unsigned _tristate : 1;
	unsigned _autoresize : 1;
	
	bool hasShortcut();
	void unsetOtherRadioButtons();
	virtual int minimumHeight();
	virtual void updateSize();
	
	static bool isButton(gControl *control) { return control->getClass() == Type_gButton && ((gButton *)control)->type == Button; }
};

#endif
