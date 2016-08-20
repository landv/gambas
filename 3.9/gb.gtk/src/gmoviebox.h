/***************************************************************************

  gmoviebox.h

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

#ifndef __GMOVIEBOX_H
#define __GMOVIEBOX_H

class gMovieBox : public gControl
{
public:
	gMovieBox(gContainer *parent);
	~gMovieBox();

//"Properties"
	int getBorder() { return getFrameBorder(); }
	bool playing();
	int alignment();

	void setBorder(int vl) { setFrameBorder(vl); }
	void setPlaying(bool vl);
	void setAlignment(int vl);

//"Methods"
	bool loadMovie(char *buf, int len);

//"Private"
	virtual gColor getFrameColor();
	bool pl;
	guint timeout;
	GdkPixbufAnimation *animation;
	GdkPixbufAnimationIter *iter;
};

#endif
