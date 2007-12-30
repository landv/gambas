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
	gPicture(gPictureType type, long w, long h, bool trans);
	~gPicture();
	gPicture *copy();
	
	bool isVoid() { return _type == VOID; }
  
  static void assign(gPicture **dst, gPicture *src = 0) { gShare::assign((gShare **)dst, src); }

  gPictureType type() { return _type; }
	int width() { return _width; }
	int height() { return _height; }
	int depth();
	bool transparent() { return _transparent; }

	void setTransparent(bool vl);

	void clear();
	void resize(int width,int height);
	int save(char *path);
	void fill(gColor col);
	gPicture *copy(int x, int y, int w, int h);

	gPicture *flip(bool mirror = false);
	gPicture *mirror() { return flip(true); }
	gPicture *rotate(double ang);
	gPicture *stretch(int w, int h, bool smooth);

	gColor getPixel(int x, int y);
	void putPixel(int x, int y, gColor col);
	void replace(gColor src, gColor dst);

	static gPicture *fromNamedIcon(char* name);
	static gPicture *fromMemory(char *addr, unsigned int len);

//"Private"
	GdkPixmap *pic;
	GdkPixbuf *img;
	alphaCache *cache;
  
  gPictureType _type;
  bool _transparent;
  int _width;
  int _height;
  
  gPicture(GdkPixbuf *image);
  gPicture(GdkPixmap *pixmap);
  void initialize();
	GdkPixbuf *getPixbuf();
	GdkPixmap *getPixmap();
	
	static gPicture* fromPixbuf(GdkPixbuf *buf) { return new gPicture(buf); }

// "Private"
	void invalidate();
};

class gPictureCache
{
public:
	static void put(char *key, gPicture *img);
	static gPicture *get(char *key);
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
	gPicture(long w,long h,bool trans);
	~gPicture();

	long width();
	long height();
	long depth();
	bool transparent();

	void setTransparent(bool vl);

	void resize(int width,int height);
	int  save(char *path);
	void fromMemory(char *addr,unsigned int len);
	static gPicture* fromNamedIcon(char* name);
	void Fill(long col);
	gPicture* getImage();
	gPicture* copy(long x,long y,long w,long h);

	void ref();
	void unref();

//"Private"
	GdkDrawable *pic;
	alphaCache *cache;
        int _transparent;
	GdkPixbuf* getPixbuf();
	static gPicture* fromPixbuf(GdkPixbuf *buf);
	long refcount;
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
