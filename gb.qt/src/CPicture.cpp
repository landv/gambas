/***************************************************************************

  CPicture.cpp

  The Picture class

  (c) 2000-2005 Benoît Minisini <gambas@users.sourceforge.net>

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

#include "CImage.h"
#include "CPicture.h"

//#if QT_VERSION < 0x030100
#include <X11/Xlib.h>
//#endif


static QDict<CPICTURE> dict;


static void create(CPICTURE **ppicture)
{
  static GB_CLASS class_id = NULL;

  if (!class_id)
    class_id = GB.FindClass("Picture");

  GB.New((void **)ppicture, class_id, NULL, NULL);
}


static bool load(CPICTURE *_object, char *path, long lenp)
{
  QImage img;

  if (CIMAGE_load_image(img, path, lenp))
  {
    THIS->pixmap->convertFromImage(img);
    return true;
  }
  else
    return false;
}

static GB_FUNCTION _stock_get_func;

static bool init_stock()
{
	static bool init = false;
	static bool error = false;

	if (!init)
	{
		error = GB.GetFunction(&_stock_get_func, GB.FindClass("Stock"), "_get", "s", "Picture");
		init = true;
	}

  return error;
}



static CPICTURE *get_picture(char *path, long len)
{
  CPICTURE *pict = NULL;
  char key[MAX_KEY + 1];
  GB_VALUE *value;

  snprintf(key, sizeof(key), "%s\n%.*s", GB.CurrentComponent(), (int)len, path);

  pict = dict[key];
  if (!pict)
  {
		if (len >= STOCK_PREFIX_LEN && strncmp(path, STOCK_PREFIX, STOCK_PREFIX_LEN) == 0)
		{
			if (len == STOCK_PREFIX_LEN)
				return NULL;

			if (init_stock())
				return NULL;

			GB.Push(1, GB_T_STRING, &path[STOCK_PREFIX_LEN], len - STOCK_PREFIX_LEN);
			value = GB.Call(&_stock_get_func, 1, false);
			if (value->type >= GB_T_OBJECT)
				pict = (CPICTURE *)((GB_OBJECT *)value)->value;
			if (!pict)
				return NULL;
		}
		else
		{
			create(&pict);
			if (!load(pict, path, len))
			{
				GB.Unref((void **)&pict);
				return NULL;
			}
		}

    GB.Ref(pict);
    dict.insert(key, pict);
  }

  //qDebug("Picture[\"%s\"] => %p", key, pict);

  return pict;
}

CPICTURE *CPICTURE_get_picture(char *path)
{
	return get_picture(path, strlen(path));
}


static void set_picture(char *path, long len, CPICTURE *newpict)
{
  CPICTURE *pict;
  char key[MAX_KEY + 1];

  snprintf(key, sizeof(key), "%s\n%.*s", GB.CurrentComponent(), (int)len, path);
  //qDebug("Picture[\"%s\"] <= %p", key, newpict);

  pict = dict[key];
  if (!pict)
    GB.Unref((void **)&pict);

  if (newpict)
  {
    GB.Ref(newpict);
    dict.replace(key, newpict);
	}
}


static void flush_picture()
{
  QDictIterator<CPICTURE> it(dict);
  CPICTURE *pict;

  while (it.current())
  {
    //delete it.current()->pixmap;
    pict = it.current();
    //qDebug("flushing: %s\n", it.currentKey().latin1());
    GB.Unref((void **)&pict);
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
    #if QT_VERSION >= 0x030100
      id = QPaintDevice::x11AppRootWindow();
    #else
      id = RootWindow(QPaintDevice::x11AppDisplay(), QPaintDevice::x11AppScreen());
    #endif

    *pict->pixmap = QPixmap::grabWindow(id);
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

	create(&pict);

  if (!load(pict, STRING(path), LENGTH(path)))
  {
  	GB.Unref((void **)&pict);
    GB.Error("Unable to load picture");
	}
	else
		GB.ReturnObject(pict);

END_METHOD


BEGIN_METHOD(CPICTURE_save, GB_STRING path)

  QString path = TO_QSTRING(GB.FileName(STRING(path), LENGTH(path)));
  bool ok = false;
  const char *fmt = CIMAGE_get_format(path);

  if (!fmt)
  {
    GB.Error("Unknown format");
    return;
  }

  ok = THIS->pixmap->save(path, fmt);

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
  if (THIS->pixmap->hasAlpha())
    THIS->pixmap->setMask(mask);

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

  GB.New((void **)&img, GB.FindClass("Image"), NULL, NULL);
  *(img->image) = THIS->pixmap->convertToImage();
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
  GB_METHOD("Save", NULL, CPICTURE_save, "(Path)s"),
  GB_METHOD("Resize", NULL, CPICTURE_resize, "(Width)i(Height)i"),

  GB_METHOD("Clear", NULL, CPICTURE_clear, NULL),
  GB_METHOD("Fill", NULL, CPICTURE_fill, "(Color)i"),
  //GB_METHOD("Mask", NULL, CPICTURE_mask, "[(Color)i]"),

  GB_PROPERTY("Transparent", "b", CPICTURE_transparent),

  GB_METHOD("Copy", "Picture", CPICTURE_copy, "[(X)i(Y)i(Width)i(Height)i]"),
  GB_PROPERTY_READ("Image", "Image", CPICTURE_image),

  GB_END_DECLARE
};

