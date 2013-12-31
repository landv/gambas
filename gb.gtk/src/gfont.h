/***************************************************************************

  gfont.h

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

#ifndef __GFONT_H
#define __GFONT_H

#include "gshare.h"

class gFont : public gShare
{
public:
	gFont();
	gFont(const char *name);
	virtual ~gFont();
  
  static void assign(gFont **dst, gFont *src = 0) { gShare::assign((gShare **)dst, src); }
  static void set(gFont **dst, gFont *src = 0) { gShare::assign((gShare **)dst, src); src->unref(); }
  
	static void init();
	static void exit();
	static int count();
	static const char *familyItem(int pos);

	gFont *copy();
	void copyTo(gFont *dst);
	void mergeFrom(gFont *src);
	int ascent();
	float ascentF();
	int descent();
	bool fixed();
	bool scalable();
	char **styles();

	bool bold();
	bool italic();
	char* name();
	int resolution();
	double size();
	bool strikeout();
	bool underline();
	int grade();

	void setBold(bool vl);
	void setItalic(bool vl);
	void setName(char *nm);
	void setResolution(int vl);
	void setSize(double sz);
	void setGrade(int grade);
	void setStrikeout(bool vl);
	void setUnderline(bool vl);

	const char *toString();
	const char *toFullString();
	int width(const char *text, int len = -1);
	int height(const char *text, int len = -1);
	int height();
	void richTextSize(const char *txt, int len, float sw, float *w, float *h);

//"Private"
	gFont(GtkWidget *wg);
	gFont(PangoFontDescription *fd);
	PangoContext* ct;
	PangoFontDescription *desc() { return pango_context_get_font_description(ct); }
	bool isAllSet();
	void setAll(bool v);
	void setAllFrom(gFont *font);
	void reset();
	
	unsigned _bold_set : 1;
	unsigned _italic_set : 1;
	unsigned _name_set : 1;
	unsigned _size_set : 1;
	unsigned _strikeout_set : 1;
	unsigned _underline_set : 1;

private:
	
	bool uline;
	bool strike;
	void realize();
	void initFlags();
	int _height;
};

#endif
