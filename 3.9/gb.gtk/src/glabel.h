/***************************************************************************

  glabel.h

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

#ifndef __GSIMPLELABEL_H
#define __GSIMPLELABEL_H

#include "gcontrol.h"

class gLabel : public gControl
{
public:
	gLabel(gContainer *parent);
	~gLabel();

	int alignment();
	int getBorder() const { return getFrameBorder(); }
	char* text();
	bool isTransparent() const { return _transparent; }
	bool autoResize() const { return _autoresize; }
	int padding() const { return getFramePadding(); }
	bool wrap() const { return _wrap; }

	void setAlignment(int al);
	void setBorder(int vl) { setFrameBorder(vl); }
	void setText(const char *st);
	void setTransparent(bool vl);
	void setAutoResize(bool vl);
	void setPadding(int vl) { setFramePadding(vl); }
	void setWrap(bool vl);

//"Methods"
	void enableMarkup(bool vl);
	void adjust();
	virtual void resize(int w, int h);
	virtual void afterRefresh();

//"Private"
	//virtual gColor getFrameColor();
	virtual void updateSize();
	virtual void updateBorder();
	void updateSize(bool adjust, bool noresize = false);
	void updateLayout();
	PangoLayout *layout;
	int align,lay_x,lay_y;
	unsigned markup : 1;
	unsigned _autoresize : 1;
	unsigned _transparent : 1;
	unsigned _mask_dirty : 1;
	unsigned _locked : 1;
	unsigned _wrap : 1;
	char *textdata;
};

#endif
