/***************************************************************************

  gscrollview.h

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

#ifndef __GSCROLLVIEW_H
#define __GSCROLLVIEW_H

class gScrollView : public gContainer
{
public:
	gScrollView(gContainer *parent);

//"Properties"
	virtual int scrollWidth();
	virtual int scrollHeight();

//"Methods"
	virtual void resize(int w,int h);
	void ensureVisible(int x, int y, int w, int h);

//"Signals"
	void (*onScroll)(gScrollView *sender);

//"Private"
	virtual void updateCursor(GdkCursor *cursor);
	virtual void performArrange();
  void updateSize();
  guint _timer;
	
private:
	GtkWidget *viewport;
	int _maxw, _maxh;
  int _mw, _mh;
};

#endif
