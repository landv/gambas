/***************************************************************************

  gspinbox.h

  (c) 2000-2009 Benoît Minisini <gambas@users.sourceforge.net>

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
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

***************************************************************************/
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
