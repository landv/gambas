/***************************************************************************

  gdraw.h

  (c) 2000-2012 Benoît Minisini <gambas@users.sourceforge.net>

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

#ifndef __GDRAW_H
#define __GDRAW_H

class gDrawingArea;

class gDraw
{
public:
	gDraw();
	~gDraw();
	void connect(gControl *wid);
	void connect(gPicture *wid);
	void disconnect();

//"Properties"
	gColor       foreground();
	gFont*       font();
	gColor       background();
	gColor       fillColor();
	int          lineWidth();
	int          lineStyle();
	int          fillX();
	int          fillY();
	int          fillStyle();
	bool         invert();
	int          clipX();
	int          clipY();
	int          clipWidth();
	int          clipHeight();
	bool         clipEnabled();
	int          textWidth(char *txt, int len = -1);
	int          textHeight(char *txt, int len = -1);
	void         richTextSize(char *txt, int len, int sw, int *w, int *h);
	int          width() const { return _width; }
	int          height() const { return _height; }
	int          resolution();
	bool         isTransparent();
	GdkDrawable* drawable() const { return dr; }
	GdkDrawable* mask() const { return drm; }
	GtkStyle*    style(const char *name = NULL, GType type = G_TYPE_NONE);
	GtkWidget *widget() const { return _widget; }
	int          state() const { return (int)_state; };
	int          shadow() const { return (int)_shadow; };

	void setForeground(gColor vl);
	void setFont(gFont *f);
	void setBackground(gColor vl);
	void setFillColor(gColor vl);
	void setLineWidth(int vl);
	void setLineStyle(int vl);
	void setFillX(int vl);
	void setFillY(int vl);
	void setFillStyle(int vl);
	void setInvert(bool vl);
	void setClipEnabled(bool vl);
	void setTransparent(bool vl);
	void setState(int vl);
	void setShadow(int vl);
	

//"Methods"
	void save();
	void restore();
	void point(int x,int y);
	void line(int x1,int y1,int x2,int y2);
	void rect(int x,int y,int width,int height);
	void ellipse(int x,int y,int w,int h,double start,double end);
	void arc(int x,int y,int w,int h,double start,double end);
	void polyline (int *vl,int nel);
	void polygon (int *vl,int nel);
	void setClip(int x,int y,int w,int h);
	void setClip(GdkRectangle *rect) { setClip(rect->x, rect->y, rect->width, rect->height); }
	void text(char *txt, int len, int x, int y, int w, int h, int align);
	void richText(char *txt, int len, int x, int y, int w, int h, int align);

	void picture(gPicture *pic, int x, int y, int w = -1, int h = -1, int sx = 0, int sy = 0, int sw = -1, int sh = -1);
	void tiledPicture(gPicture *pic, int x, int y, int w, int h);

//"Private"
private:
	void init();
	void clear();
	void reset();
	void startFill();
	void endFill();
	void drawLayout(PangoLayout *ly, bool markup, int x, int y, int w, int h, int align);
	void initGC();

	gDrawingArea *dArea;
	gFont *ft;
	GdkRectangle clip;
	bool clip_enabled;
	bool _transparent;
	GtkStyle *stl;
	const char *stl_name;
	GtkWidget *_widget;
	GdkDrawable *dr;
	GdkDrawable *drm;
	GdkPixmap *stipple;
	GdkGC *gc;
	GdkGC *gcm;
	GArray *_gc_stack;
	int fill;
	int fillCol;
	int _fillX, _fillY;
	int line_style;
	void *tag;
	int _width;
	int _height;
	int _resolution;
	int _shadow;
	int _state;
	int _x, _y;
	gColor _default_fg;
	gColor _default_bg;
	gColor _save_fg;
};

#endif
