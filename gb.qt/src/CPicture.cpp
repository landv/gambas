/***************************************************************************

  CPicture.cpp

  The Picture class

  (c) 2000-2007 Benoit Minisini <gambas@users.sourceforge.net>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 1, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

***************************************************************************/

#define __CPICTURE_CPP

#include <string.h>

#include <qpixmap.h>
#include <qbitmap.h>
#include <qnamespace.h>
#include <qdict.h>
#include <qpainter.h>
#include <qwmatrix.h>

#include "gambas.h"
#include "main.h"

#include "CDraw.h"
#include "CScreen.h"
#include "CImage.h"
#include "CPicture.h"

#ifndef NO_X_WINDOW
#include <X11/Xlib.h>
#endif


static QDict<CPICTURE> dict;

static void create(CPICTURE **ppicture)
{
  GB.New(POINTER(ppicture), GB.FindClass("Picture"), NULL, NULL);
}

static void insert_cache(const char *key, CPICTURE *pict)
{
  CPICTURE *old = dict[key];
  
  if (old)
  	dict.remove(key);
  
  if (pict)
  {
		dict.replace(key, pict); 
		GB.Ref(pict);
	}
  
  if (old)
  	GB.Unref(POINTER(&old));
}

#define LOAD_IMAGE_FUNC CPICTURE_load_image

#define IMAGE_TYPE QImage

#define CREATE_IMAGE_FROM_MEMORY(_image, _addr, _len, _ok) \
{ \
	QImage img; \
	_ok = img.loadFromData((const uchar *)_addr, (uint)_len); \
	if (_ok) \
	{ \
		if (img.depth() < 32 && !img.isNull()) \
			img = img.convertDepth(32); \
	} \
	_image = new QImage(img); \
}

#define CREATE_PICTURE_FROM_IMAGE(_cpicture, _image) \
{ \
	create(&(_cpicture)); \
	if ((_image) && !(_image)->isNull()) \
		(_cpicture)->pixmap->convertFromImage(*(_image)); \
}

#define GET_FROM_CACHE(_key) (dict[_key])

#define INSERT_INTO_CACHE(_key, _cpicture) insert_cache(_key, _cpicture)

#define APPLICATION_THEME CAPPLICATION_Theme

#include "gb.form.picture.h"



CPICTURE *CPICTURE_get_picture(const char *path)
{
	return get_picture(path, strlen(path));
}

static void flush_picture()
{
  QDictIterator<CPICTURE> it(dict);
  CPICTURE *pict;

  //qDebug("flush_picture");

  while (it.current())
  {
    //delete it.current()->pixmap;
    pict = it.current();
    //qDebug("flushing: %s %p", it.currentKey().latin1(), pict);
    GB.Unref(POINTER(&pict));
    ++it;
  }

  dict.clear();
}


CPICTURE *CPICTURE_grab(QWidget *wid)
{
  CPICTURE *pict;
  int id;

  create(&pict);

  if (!wid)
  {
		#ifdef NO_X_WINDOW
			qDebug("Qt/Embedded: Full screen grab not implemented");
		#else
			#if QT_VERSION >= 0x030100
				id = QPaintDevice::x11AppRootWindow();
			#else
				id = RootWindow(QPaintDevice::x11AppDisplay(), QPaintDevice::x11AppScreen());
			#endif
	
			*pict->pixmap = QPixmap::grabWindow(id);
		#endif
  }
  else
  {
    *pict->pixmap = QPixmap::grabWindow(wid->winId());
  }

  return pict;
}


/*void CPICTURE_update_mask(CPICTURE *_object)
{
  if (THIS->pixmap && THIS->pixmap->hasAlpha())
    THIS->pixmap->setMask(THIS->pixmap->createHeuristicMask());
}*/


/*******************************************************************************

  class Picture

*******************************************************************************/


BEGIN_METHOD(CPICTURE_get, GB_STRING path)

  GB.ReturnObject(get_picture(STRING(path), LENGTH(path)));

END_METHOD


BEGIN_METHOD(CPICTURE_put, GB_OBJECT picture; GB_STRING path)

  set_picture(STRING(path), LENGTH(path), (CPICTURE *)VARG(picture));

END_METHOD


BEGIN_METHOD_VOID(CPICTURE_flush)

  flush_picture();

END_METHOD


BEGIN_METHOD(CPICTURE_new, GB_INTEGER w; GB_INTEGER h; GB_BOOLEAN trans)

  int w, h;

  THIS->pixmap = new QPixmap;

	if (!MISSING(w) && !MISSING(h))
	{
		w = VARG(w);
		h = VARG(h);
		if (h <= 0 || w <= 0)
		{
			GB.Error("Bad dimension");
			return;
		}

		THIS->pixmap->resize(w, h);
		if (VARGOPT(trans, false))
		{
			QBitmap b(w, h);
			b.fill(Qt::color0);
			THIS->pixmap->setMask(b);
		}
	}

END_METHOD


BEGIN_METHOD_VOID(CPICTURE_free)

  delete THIS->pixmap;
  THIS->pixmap = 0;

END_METHOD


BEGIN_METHOD(CPICTURE_resize, GB_INTEGER width; GB_INTEGER height)

  THIS->pixmap->resize(VARG(width), VARG(height));

END_METHOD


BEGIN_PROPERTY(CPICTURE_width)

  GB.ReturnInteger(THIS->pixmap->width());

END_PROPERTY


BEGIN_PROPERTY(CPICTURE_height)

  GB.ReturnInteger(THIS->pixmap->height());

END_PROPERTY


BEGIN_PROPERTY(CPICTURE_depth)

  GB.ReturnInteger(THIS->pixmap->depth());

END_PROPERTY


BEGIN_METHOD(CPICTURE_load, GB_STRING path)

	CPICTURE *pict;
	QImage *img;

	if (!LOAD_IMAGE_FUNC(&img, STRING(path), LENGTH(path)))
	{
    GB.Error("Unable to load picture");
		return;
	}
		
	CREATE_PICTURE_FROM_IMAGE(pict, img);
	GB.ReturnObject(pict);

END_METHOD


BEGIN_METHOD(CPICTURE_save, GB_STRING path; GB_INTEGER quality)

  QString path = TO_QSTRING(GB.FileName(STRING(path), LENGTH(path)));
  bool ok = false;
  const char *fmt = CIMAGE_get_format(path);

  if (!fmt)
  {
    GB.Error("Unknown format");
    return;
  }

  ok = THIS->pixmap->save(path, fmt, VARGOPT(quality, -1));

  if (!ok)
    GB.Error("Unable to save picture");

END_METHOD


BEGIN_METHOD_VOID(CPICTURE_clear)

  delete THIS->pixmap;
  THIS->pixmap = new QPixmap;

END_METHOD


BEGIN_METHOD(CPICTURE_fill, GB_INTEGER col)

  int col = VARG(col);
  QBitmap mask;

  THIS->pixmap->fill(QColor(QRgb(col & 0xFFFFFF)));

END_METHOD


BEGIN_METHOD(CPICTURE_copy, GB_INTEGER x; GB_INTEGER y; GB_INTEGER w; GB_INTEGER h)

  CPICTURE *pict;
  int x = VARGOPT(x, 0);
  int y = VARGOPT(y, 0);
  int w = VARGOPT(w, THIS->pixmap->width());
  int h = VARGOPT(h, THIS->pixmap->height());

  create(&pict);
  pict->pixmap = new QPixmap(w, h);
  #if QT_VERSION >= 0x030200
  copyBlt(pict->pixmap, 0, 0, THIS->pixmap, x, y, w, h);
  #else
  bitBlt(THIS->pixmap, 0, 0, pict->pixmap, x, y, w, h, Qt::CopyROP, TRUE);
  #endif

  GB.ReturnObject(pict);

END_METHOD


BEGIN_PROPERTY(CPICTURE_image)

  CIMAGE *img;

  GB.New(POINTER(&img), GB.FindClass("Image"), NULL, NULL);
  *(img->image) = THIS->pixmap->convertToImage();
  if (!img->image->isNull())
  	img->image->convertDepth(32);

  GB.ReturnObject(img);

END_PROPERTY


BEGIN_PROPERTY(CPICTURE_transparent)

  if (READ_PROPERTY)
    GB.ReturnBoolean(THIS->pixmap->hasAlpha());
  else
  {
    bool a = THIS->pixmap->hasAlpha();

    if (a == VPROP(GB_BOOLEAN))
      return;

    if (a)
      THIS->pixmap->setMask(QBitmap());
    else
      THIS->pixmap->setMask(THIS->pixmap->createHeuristicMask());
  }

END_PROPERTY


GB_DESC CPictureDesc[] =
{
  GB_DECLARE("Picture", sizeof(CPICTURE)),

  GB_STATIC_METHOD("_exit", NULL, CPICTURE_flush, NULL),

  GB_METHOD("_new", NULL, CPICTURE_new, "[(Width)i(Height)i(Transparent)b]"),
  GB_METHOD("_free", NULL, CPICTURE_free, NULL),

  GB_STATIC_METHOD("_get", "Picture", CPICTURE_get, "(Path)s"),
  GB_STATIC_METHOD("_put", NULL, CPICTURE_put, "(Picture)Picture;(Path)s"),
  GB_STATIC_METHOD("Flush", NULL, CPICTURE_flush, NULL),

  GB_PROPERTY_READ("W", "i", CPICTURE_width),
  GB_PROPERTY_READ("Width", "i", CPICTURE_width),
	GB_PROPERTY_READ("H", "i", CPICTURE_height),
  GB_PROPERTY_READ("Height", "i", CPICTURE_height),
  GB_PROPERTY_READ("Depth", "i", CPICTURE_depth),

  GB_STATIC_METHOD("Load", "Picture", CPICTURE_load, "(Path)s"),
  GB_METHOD("Save", NULL, CPICTURE_save, "(Path)s[(Quality)i]"),
  GB_METHOD("Resize", NULL, CPICTURE_resize, "(Width)i(Height)i"),

  GB_METHOD("Clear", NULL, CPICTURE_clear, NULL),
  GB_METHOD("Fill", NULL, CPICTURE_fill, "(Color)i"),
  //GB_METHOD("Mask", NULL, CPICTURE_mask, "[(Color)i]"),

  GB_PROPERTY("Transparent", "b", CPICTURE_transparent),

  GB_METHOD("Copy", "Picture", CPICTURE_copy, "[(X)i(Y)i(Width)i(Height)i]"),
  GB_PROPERTY_READ("Image", "Image", CPICTURE_image),
  
  GB_INTERFACE("Draw", &DRAW_Interface),

  GB_END_DECLARE
};

