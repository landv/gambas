/***************************************************************************

  gdrawingarea.h

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

#ifndef __GDRAWINGAREA_H
#define __GDRAWINGAREA_H

class gDrawingArea : public gContainer
{
public:
	gDrawingArea(gContainer *parent);
	~gDrawingArea();

	int getBorder() const { return getFrameBorder(); }
	bool cached() const { return _cached; }
	bool hasNoBackground() const { return _no_background; }
	bool useTablet() const { return _use_tablet; }

	void setBorder(int vl) { setFrameBorder(vl); }
	void setCached(bool vl);
	void setNoBackground(bool vl);
	void setUseTablet(bool vl);
	
	bool inDrawEvent() const { return _in_draw_event; }

//"Methods"
	void clear();
	virtual void resize(int w, int h);
	virtual void setEnabled(bool vl);
	virtual void setRealBackground(gColor color);
	virtual void updateFont();

//"Events"
#ifdef GTK3
	void (*onExpose)(gDrawingArea *sender, cairo_t *cr);
#else
	void (*onExpose)(gDrawingArea *sender, GdkRegion *region, int dx, int dy);
#endif
	void (*onFontChange)(gDrawingArea *sender);

//"Private"
	void create();
	void updateCache();
	void resizeCache();
	void refreshCache();
	void updateEventMask();
	void setCache();
	void updateUseTablet();
	
#ifdef GTK3
	cairo_surface_t *buffer;
#else
	GdkPixmap *buffer;
#endif
	GtkWidget *box;
	uint _event_mask;
	uint _old_bg_id;
	unsigned _cached : 1;
	unsigned _resize_cache : 1;
	unsigned _in_draw_event : 1;
	unsigned _no_background : 1;
	unsigned _use_tablet : 1;
};

#endif
