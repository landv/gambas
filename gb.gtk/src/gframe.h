/***************************************************************************

  gframe.h

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

#ifndef __GFRAME_H
#define __GFRAME_H

#include "gcontainer.h"
#include "gcolor.h"

class gPanel : public gContainer
{
public:
	gPanel(gContainer *parent);

	int getBorder() { return getFrameBorder(); }
	void setBorder(int vl) { setFrameBorder(vl); }
	virtual void setBackground(gColor color = COLOR_DEFAULT);

private:

	void create();
};

class gFrame : public gContainer
{
public:
	gFrame(gContainer *parent);

	const char *text();
	void setText(const char *vl);

	virtual int containerX();
	virtual int containerY();
	virtual int clientX();
	virtual int clientY();
	virtual int clientWidth();
	virtual int clientHeight();

	virtual void updateFont();
#ifdef GTK3
	virtual void updateColor();
#else
	virtual void setRealForeground(gColor color);
#endif

//"Private"
	GtkWidget *fr;
};

#endif
