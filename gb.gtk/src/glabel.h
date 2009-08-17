/***************************************************************************

  glabel.h

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
#ifndef __GSIMPLELABEL_H
#define __GSIMPLELABEL_H

#include "gcontrol.h"

class gLabel : public gControl
{
public:
	gLabel(gContainer *parent);
	~gLabel();

	int alignment();
	int getBorder() { return getFrameBorder(); }
	char* text();
	bool isTransparent() { return _transparent; }
	bool autoResize();
	int padding() { return getFramePadding(); }

	void setAlignment(int al);
	void setBorder(int vl) { setFrameBorder(vl); }
	void setText(char *st);
	virtual void setFont(gFont *ft);
	void setTransparent(bool vl);
	void setAutoResize(bool vl);
	void setPadding(int vl) { setFramePadding(vl); }

//"Methods"
	void enableMarkup(bool vl);
	void adjust();
	virtual void resize(int w, int h);
	virtual void afterRefresh();

//"Private"
	void updateSize(bool adjust = false, bool noresize = false);
	void updateLayout();
	PangoLayout *layout;
	int align,lay_x,lay_y;
	unsigned markup : 1;
	unsigned autoresize : 1;
	unsigned _transparent : 1;
	unsigned _mask_dirty : 1;
	unsigned _locked : 1;
	char *textdata;
};

#endif
