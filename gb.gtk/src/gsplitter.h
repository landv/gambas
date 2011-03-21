/***************************************************************************

  gsplitter.h

  (c) 2000-2011 Benoît Minisini <gambas@users.sourceforge.net>

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
#ifndef __GSPLITTER_H
#define __GSPLITTER_H

class gSplitter : public gContainer
{
public:
	gSplitter(gContainer *parent, bool vert);

//"Properties"
	int layoutCount();
	void getLayout(int *array);
	void setLayout(int *array, int count);

	//int childCount();
	//gControl* child(int index);

//"Methods"
	virtual void performArrange();
	virtual void resize(int w, int h);

//"Signals"
	void (*onResize)(gControl *sender);

//"Private"
	virtual void insert(gControl *child, bool realize = false);
	virtual void remove(gControl *child);
	void updateChild(GtkWidget *wid = 0);
	void updateVisibility();
	GtkPaned *curr;
	bool vertical;
	GtkPaned *next(GtkPaned *iter);
	int handleCount();
	bool _notify;
};

#endif
