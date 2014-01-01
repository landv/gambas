/***************************************************************************

  gspinbox.h

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

#ifndef __GSPINBOX_H
#define __GSPINBOX_H

class gSpinBox : public gControl
{
public:
	gSpinBox(gContainer *parent);

//"Properties"
	int maxValue() const { return _max; }
	int minValue() const { return _min; }
	int step();
	int value();
	bool wrap();
	bool hasBorder() const;

	void setMaxValue  (int vl);
	void setMinValue  (int vl);
	void setStep      (int vl);
	void setValue     (int vl);
	void setWrap      (bool vl);
	void setBorder(bool vl);

//"Methods"
	void selectAll();

//"Signals"
	void (*onChange)  (gSpinBox *sender);

#ifndef GTK3
	virtual void updateCursor(GdkCursor *cursor);
#else
	virtual void resize(int w, int h);
#endif

//"Private"
private:
	int _min;
	int _max;
};

#endif
