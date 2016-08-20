/***************************************************************************

	gpicture.h

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

#ifndef __GPICTURE_H
#define __GPICTURE_H

#include "widgets.h"
#include "gshare.h"
#include "gcolor.h"
#include "gtag.h"

class alphaCache;

class gPicture : public gShare
{
public:

	enum gPictureType
	{
		VOID,
		PIXBUF,
#ifndef GTK3
		PIXMAP,
#endif
		SURFACE
	};

	gPicture();
	gPicture(gPictureType type, int w, int h, bool trans);
	~gPicture();
	gPicture *copy();

	bool isVoid() { return _type == VOID; }

	static void assign(gPicture **dst, gPicture *src = 0) { gShare::assign((gShare **)dst, src); }

	gPictureType type() const { return _type; }
	int width() const { return _width; }
	int height() const { return _height; }
	int depth();
	bool isTransparent() const { return _transparent; }
	unsigned char *data();

	void setTransparent(bool vl);

	void clear();
	void resize(int width,int height);
	int save(const char *path, int quality = -1);
	void fill(gColor col);
	gPicture *copy(int x, int y, int w, int h);

	gPicture *flip(bool mirror = false);
	gPicture *mirror() { return flip(true); }
	gPicture *rotate(double ang);
	gPicture *stretch(int w, int h, bool smooth);

	gColor getPixel(int x, int y);
	void putPixel(int x, int y, gColor col);
	void draw(gPicture *src, int x, int y, int w = -1, int h = -1, int sx = 0, int sy = 0, int sw = -1, int sh = -1);
	void makeGray();
	void makeTransparent(gColor color);

	static gPicture *fromNamedIcon(const char *name, int len = -1);
	static gPicture *fromMemory(char *addr, unsigned int len);
	static gPicture *fromData(const char *data, int width, int height);

//"Private"
#ifndef GTK3
	GdkPixmap *pixmap;
	GdkBitmap *mask;
#endif
	GdkPixbuf *pixbuf;
	cairo_surface_t *surface;

	gPictureType _type;
	bool _transparent;
	int _width;
	int _height;

	gPicture(GdkPixbuf *image, bool trans = true);
#ifdef GTK3
	gPicture(cairo_surface_t *surf);
#else
	gPicture(GdkPixmap *pix);
#endif
	void initialize();
	GdkPixbuf *getPixbuf();
#ifndef GTK3
	GdkPixmap *getPixmap();
	GdkBitmap *getMask();
#endif
	GdkPixbuf *getIconPixbuf();
	cairo_surface_t *getSurface();

	static gPicture* fromPixbuf(GdkPixbuf *buf) { return new gPicture(buf); }

// "Private"
	void invalidate();
	void createMask(bool white);
};

class gPictureCache
{
public:
	static void put(const char *key, gPicture *img);
	static gPicture *get(const char *key);
	static void flush();
	static void init();
	static void exit();
private:
	static GHashTable *cache;
};

#endif
