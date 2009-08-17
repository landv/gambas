/***************************************************************************

  gscrollview.h

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
#ifndef __GSCROLLVIEW_H
#define __GSCROLLVIEW_H

class gScrollView : public gContainer
{
public:
	gScrollView(gContainer *parent);

//"Properties"
	virtual int scrollWidth() { return _mw; }
	virtual int scrollHeight() { return _mh; }

//"Methods"
	virtual void resize(int w,int h);
	void ensureVisible(int x, int y, int w, int h);

//"Signals"
	void (*onScroll)(gScrollView *sender);

//"Private"
	virtual void performArrange();
  void updateSize();
  guint _timer;
	
private:
	GtkWidget *viewport;
  int _mw, _mh;
};

#endif
