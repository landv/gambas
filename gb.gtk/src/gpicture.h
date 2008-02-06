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
    MEMORY,
    SERVER
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
	void replace(gColor src, gColor dst, bool noteq = false);
	void draw(gPicture *src, int x, int y, int w = -1, int h = -1, int sx = 0, int sy = 0, int sw = -1, int sh = -1);

	static gPicture *fromNamedIcon(const char *name, int len = -1);
	static gPicture *fromMemory(char *addr, unsigned int len);

//"Private"
	GdkPixmap *pic;
	GdkBitmap *mask;
	GdkPixbuf *img;
  
  gPictureType _type;
  bool _transparent;
  int _width;
  int _height;
  
  gPicture(GdkPixbuf *image, bool trans = true);
  gPicture(GdkPixmap *pixmap);
  void initialize();
	GdkPixbuf *getPixbuf();
	GdkPixmap *getPixmap();
	GdkBitmap *getMask();
	
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

#if 0

class gPicture
{
public:
	gPicture(int w,int h,bool trans);
	~gPicture();

	int width();
	int height();
	int depth();
	bool transparent();

	void setTransparent(bool vl);

	void resize(int width,int height);
	int  save(char *path);
	void fromMemory(char *addr,unsigned int len);
	static gPicture* fromNamedIcon(char* name);
	void Fill(int col);
	gPicture* getImage();
	gPicture* copy(int x,int y,int w,int h);

	void ref();
	void unref();

//"Private"
	GdkDrawable *pic;
	alphaCache *cache;
        int _transparent;
	GdkPixbuf* getPixbuf();
	static gPicture* fromPixbuf(GdkPixbuf *buf);
	int refcount;
};

class gPictureCache
{
public:
	static void save(char *key,gPicture *img);
	static gPicture* load(char *key);
	static void flush();
};

#endif

#endif
