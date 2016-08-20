/***************************************************************************

  gpicturebox.h

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

#ifndef __GPICTUREBOX_H
#define __GPICTUREBOX_H

class gPictureBox : public gControl
{
public:
	gPictureBox(gContainer *parent);
	~gPictureBox();

	int alignment();
	int getBorder() { return getFrameBorder(); }
	bool stretch();
	gPicture* picture() { return _picture; }
	bool isAutoResize() { return _autoresize; }
	int padding() const { return getFramePadding(); }

	void setAlignment(int vl);
	void setBorder(int vl) { setFrameBorder(vl); }
	void setStretch(bool vl);
	void setPicture(gPicture *pic);
	void setAutoResize(bool);
	void setPadding(int vl) { setFramePadding(vl); }

//"Methods"
	void resize(int w, int h);
	virtual void updateBorder();

//"Private"
	virtual gColor getFrameColor();
	void redraw();
	void adjust();
  gPicture *_picture;
  bool _autoresize;
};

#endif
