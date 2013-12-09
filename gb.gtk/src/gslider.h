/***************************************************************************

  gslider.h

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

#ifndef __GSLIDER_H
#define __GSLIDER_H

class gSlider : public gControl
{
public:
	gSlider(gContainer *parent, bool scrollbar = false);

//"Properties"
	//int foreground();
	//int background();
	int max();
	int min();
	bool tracking();
	int value();
	bool mark();
	int step();
	int pageStep();

	//void setForeground(int vl);
	//void setBackground(int vl);
	void setMax(int vl);
	void setMin(int vl);
	void setTracking(bool vl);
	void setValue(int vl);
	void setMark(bool vl);
	void setStep(int vl);
	void setPageStep(int vl);
	
	int getDefaultSize();
	bool isVertical() const;
	
	virtual void resize(int w, int h);

//"Signals"
	void (*onChange)(gSlider *sender);

//"Private"
	virtual void orientation(int w,int h);
	void updateMark();
	void init();
	void checkInverted();
	
	bool _mark;
	int _step;
	int _page_step;
	int _value;
	int _min, _max;
	bool _tracking;
};

#endif
