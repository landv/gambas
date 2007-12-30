/***************************************************************************

  CImage.cpp

  (c) 2000-2006 Benoît Minisini <gambas@users.sourceforge.net>

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

#define __CIMAGE_CPP

#include <string.h>

#include <qpixmap.h>
#include <qbitmap.h>
#include <qpicture.h>
#include <qnamespace.h>
#include <qdict.h>
#include <qpainter.h>
#include <qwmatrix.h>

#include "gambas.h"
#include "main.h"

#include "CScreen.h"
#include "CPicture.h"
#include "CImage.h"

//#if QT_VERSION < 0x030100
//#include <X11/Xlib.h>
//#endif


static void create(CIMAGE **pimage)
{
  static GB_CLASS class_id = NULL;

  if (!class_id)
    class_id = GB.FindClass("Image");

  GB.New((void **)pimage, class_id, NULL, NULL);
}


const char *CIMAGE_get_format(QString path)
{
  int pos;

  pos = path.findRev('.');
  if (pos < 0)
    return NULL;

  path = path.mid(pos + 1).lower();

  if (path == "png")
    return "PNG";
  else if (path == "jpg" || path == "jpeg")
    return "JPEG";
  else if (path == "gif")
    return "GIF";
  else if (path == "bmp")
    return "BMP";
  else if (path == "xpm")
    return "XPM";
  else
    return NULL;
}

bool CIMAGE_load_image(QImage &p, char *path, long lenp)
{
  char *addr;
  long len;
  bool ok;
  char *path_theme;
  int pos;
  
  if (CAPPLICATION_Theme && lenp > 0 && path[0] != '/')
  {
  	pos = lenp - 1;
  	while (pos >= 0)
  	{
  		if (path[pos] == '.')
  			break;
			pos--;
  	}
  	GB.NewString(&path_theme, path, pos >= 0 ? pos : lenp);
  	GB.AddString(&path_theme, "_", 1);
  	GB.AddString(&path_theme, CAPPLICATION_Theme, GB.StringLength(CAPPLICATION_Theme));
  	if (pos >= 0)
  		GB.AddString(&path_theme, &path[pos], lenp - pos);
		ok = !GB.LoadFile(path_theme, GB.StringLength(path_theme), &addr, &len);
		GB.FreeString(&path_theme);
		if (ok)
			goto __LOAD;
  }

	GB.Error(NULL);
	if (GB.LoadFile(path, lenp, &addr, &len))
		return FALSE;
		
__LOAD:
		
	ok = p.loadFromData((const uchar *)addr, (uint)len);
	if (ok)
	{
		//create((CIMAGE **)&_object);

		if (p.depth() < 32)
			p = p.convertDepth(32);

		//p.setAlphaBuffer(true);
	}

	GB.ReleaseFile(&addr, len);
  return ok;
}

#if 0


static void create_new(CPICTURE **ppict)
{
  *ppict = NULL;
  create(ppict);
}


static bool load(CPICTURE **ppict, char *file)
{
  FORMAT_MATCH *fmt;
  bool ok;
  char *addr;
  long len;
  CPICTURE *pict;

  //qDebug("Loading %s", file);

  if (GB.LoadFile(file, 0, &addr, &len))
    return true;

  QImage p;
  ok = p.loadFromData((const uchar *)addr, (uint)len);
  if (ok)
  {
    GB.New((void **)ppict, class_id, NULL, NULL);
    create(ppict);
    pict = *ppict;

    clear(pict);
    pict->image = new QImage;

    //qDebug("%s: w = %d h = %d", file, p.width(), p.height());

    if (p.depth() < 32)
    {
      //qDebug("Convert to depth 32");
      *(pict->image) = p.convertDepth(32);
      //delete pict->image;
      //pict->image = p;
    }
    else
      *(pict->image) = p;

    pict->image->setAlphaBuffer(true);
  }

  GB.ReleaseFile(&addr, len);

  if (!ok)
    GB.Error("Unable to load image");

  //qDebug("Loading %s => %s", file, ok ? "OK" : "*ERR*");

  return !ok;
}


static CPICTURE *get_picture(char *key)
{
  CPICTURE *pict;

  pict = dict[key];
  if (!pict)
  {
    create_new(&pict);
    if (load(&pict, key))
    {
      GB.Unref((void **)&pict);
      return NULL;
    }

    // Optimize for using in the interface
    if (get_type(pict) == TYPE_IMAGE)
      set_type(pict, TYPE_PIXMAP);

    GB.Ref(pict);
    dict.insert(key, pict);
  }

  return pict;
}


static void set_picture(char *key, CPICTURE *new_pict)
{
  CPICTURE *pict;

  pict = dict[key];
  if (!pict)
    GB.Unref((void **)&pict);

  if (new_pict)
    dict.replace(key, new_pict);
}


static void flush_picture()
{
  QDictIterator<CPICTURE> it(dict);
  CPICTURE *pict;

  while (it.current())
  {
    //delete it.current()->pixmap;
    pict = it.current();
    GB.Unref((void **)&pict);
    ++it;
  }
}

static int get_width(CPICTURE *_object)
{
  if (CPICTURE_is_pixmap(THIS))
    return THIS->pixmap->width();
  else if (CPICTURE_is_image(THIS))
    return THIS->image->width();
  else if (CPICTURE_is_picture(THIS))
    return THIS->picture->boundingRect().width();
  else
    return 0;
}


static int get_height(CPICTURE *_object)
{
  if (CPICTURE_is_pixmap(THIS))
    return THIS->pixmap->height();
  else if (CPICTURE_is_image(THIS))
    return THIS->image->height();
  else if (CPICTURE_is_picture(THIS))
    return THIS->picture->boundingRect().height();
  else
    return 0;
}

CPICTURE *CPICTURE_grab(int wid)
{
  CPICTURE *pict;

  #if QT_VERSION >= 0x030100
  if (wid == 0)
    wid = QPaintDevice::x11AppRootWindow();
  #else
  if (wid == 0)
    wid = RootWindow(QPaintDevice::x11AppDisplay(), QPaintDevice::x11AppScreen());
  #endif

  create_new(&pict);

  pict->pixmap = new QPixmap();
  *pict->pixmap = QPixmap::grabWindow(wid);

  return pict;
}


void CPICTURE_update_mask(CPICTURE *_object)
{
  if (THIS->pixmap && THIS->pixmap->hasAlpha())
    THIS->pixmap->setMask(THIS->pixmap->createHeuristicMask());
}

#endif


/*******************************************************************************

  Image

*******************************************************************************/


BEGIN_METHOD(CIMAGE_new, GB_INTEGER w; GB_INTEGER h; GB_BOOLEAN trans)

  int w, h;

  THIS->image = new QImage();

	if (!MISSING(w) && !MISSING(h))
	{
		w = VARG(w);
		h = VARG(h);
		if (h <= 0 || w <= 0)
		{
			GB.Error("Bad dimension");
			return;
		}

    THIS->image->create(w, h, 32);
    THIS->image->setAlphaBuffer(VARGOPT(trans, false));
  }

END_METHOD


BEGIN_METHOD_VOID(CIMAGE_free)

  delete THIS->image;
  THIS->image = 0;

END_METHOD


BEGIN_PROPERTY(CIMAGE_picture)

  CPICTURE *pict;

  GB.New((void **)&pict, GB.FindClass("Picture"), NULL, NULL);
  pict->pixmap->convertFromImage(*(THIS->image));

  GB.ReturnObject(pict);

END_PROPERTY


BEGIN_METHOD(CIMAGE_resize, GB_INTEGER width; GB_INTEGER height)

  if (THIS->image->isNull())
  {
    THIS->image->create(VARG(width), VARG(height), 32);
    THIS->image->setAlphaBuffer(true);
  }
  else
  {
    QImage img = THIS->image->copy(0, 0, VARG(width), VARG(height));
    delete THIS->image;
    THIS->image = new QImage(img);
  }

END_METHOD


BEGIN_PROPERTY(CIMAGE_width)

  GB.ReturnInteger(THIS->image->width());

END_PROPERTY


BEGIN_PROPERTY(CIMAGE_height)

  GB.ReturnInteger(THIS->image->height());

END_PROPERTY


BEGIN_PROPERTY(CIMAGE_depth)

  //if (READ_PROPERTY)
    GB.ReturnInteger(THIS->image->depth());
  /*else
  {
    if (!THIS->image->isNull())
    {
      int depth = VPROP(GB_INTEGER);

      if (depth != THIS->image->depth())
      {
        QImage img = THIS->image->convertDepth(depth);
        if (!img.isNull())
        {
          delete THIS->image;
          THIS->image = new QImage(img);
        }
      }
    }
  }*/

END_PROPERTY


BEGIN_PROPERTY(CIMAGE_transparent)

	if (READ_PROPERTY)
  	GB.ReturnBoolean(THIS->image->hasAlphaBuffer());
	else
		THIS->image->setAlphaBuffer(VPROP(GB_BOOLEAN));

END_PROPERTY


BEGIN_METHOD(CIMAGE_load, GB_STRING path)

  QImage p;
  CIMAGE *img;

  create(&img);

  if (CIMAGE_load_image(p, STRING(path), LENGTH(path)))
  {
    *(img->image) = p;
    GB.ReturnObject(img);
	}
  else
  {
  	GB.Unref((void **)&img);
    GB.Error("Unable to load image");
	}

END_METHOD


BEGIN_METHOD(CIMAGE_save, GB_STRING path)

  QString path = TO_QSTRING(GB.FileName(STRING(path), LENGTH(path)));
  bool ok = false;
  const char *fmt = CIMAGE_get_format(path);

  if (!fmt)
  {
    GB.Error("Unknown format");
    return;
  }

  ok = THIS->image->save(path, fmt);

  if (!ok)
    GB.Error("Unable to save picture");

END_METHOD


BEGIN_METHOD_VOID(CIMAGE_clear)

  delete THIS->image;
  THIS->image = new QImage();

END_METHOD


BEGIN_METHOD(CIMAGE_fill, GB_INTEGER col)

  //bool a;
  int col = VARG(col);

  //a = THIS->image->hasAlphaBuffer();
  THIS->image->setAlphaBuffer(false);

  col ^= 0xFF000000;
  THIS->image->fill(col);

  THIS->image->setAlphaBuffer(true);

END_METHOD


BEGIN_METHOD(CIMAGE_copy, GB_INTEGER x; GB_INTEGER y; GB_INTEGER w; GB_INTEGER h)

  CIMAGE *img;
  int x = VARGOPT(x, 0);
  int y = VARGOPT(y, 0);
  int w = VARGOPT(w, THIS->image->width());
  int h = VARGOPT(h, THIS->image->height());

  create(&img);

  img->image->create(w, h, 32);

  bool a = THIS->image->hasAlphaBuffer();
  THIS->image->setAlphaBuffer(false);

  bitBlt(img->image, 0, 0, THIS->image, x, y, w, h);

  THIS->image->setAlphaBuffer(a);
  img->image->setAlphaBuffer(a);

  GB.ReturnObject(img);

END_METHOD


BEGIN_METHOD(CIMAGE_stretch, GB_INTEGER width; GB_INTEGER height; GB_BOOLEAN smooth)

  CIMAGE *img;
  QImage stretch;

  create(&img);

	if (THIS->image->isNull())
	{
    img->image->create(VARG(width), VARG(height), 32);
    img->image->setAlphaBuffer(true);
	}
	else
	{
		if (VARGOPT(smooth, TRUE))
			*(img->image) = THIS->image->smoothScale(VARG(width), VARG(height));
		else
			*(img->image) = THIS->image->scale(VARG(width), VARG(height));
	}

  GB.ReturnObject(img);

END_METHOD


BEGIN_METHOD_VOID(CIMAGE_flip)

  CIMAGE *img;

  create(&img);
  *(img->image) = THIS->image->mirror(true, false);
  GB.ReturnObject(img);

END_METHOD


BEGIN_METHOD_VOID(CIMAGE_mirror)

  CIMAGE *img;

  create(&img);
  *(img->image) = THIS->image->mirror(false, true);
  GB.ReturnObject(img);

END_METHOD


BEGIN_METHOD(CIMAGE_rotate, GB_FLOAT angle)

  CIMAGE *img;
  QWMatrix mat;

  create(&img);

  mat.rotate(VARG(angle));
  *(img->image) = THIS->image->xForm(mat);
  GB.ReturnObject(img);

END_METHOD


BEGIN_METHOD(CIMAGE_replace, GB_INTEGER src; GB_INTEGER dst)

  uint x, y, src, dst;

  THIS->image->setAlphaBuffer(true);

  src = VARG(src) ^ 0xFF000000;
  dst = VARG(dst) ^ 0xFF000000;

  for (y = 0; y < (uint)THIS->image->height(); y++)
    for (x = 0; x < (uint)THIS->image->width(); x++)
    {
      if (THIS->image->pixel(x, y) != src)
        continue;
      THIS->image->setPixel(x, y, dst);
    }

END_METHOD



/*******************************************************************************

  .ImagePixels

*******************************************************************************/


BEGIN_METHOD(CIMAGE_pixels_get, GB_INTEGER x; GB_INTEGER y)

  int col;
  int x, y;
  //unsigned char r, g, b, a;

  x = VARG(x);
  y = VARG(y);

  if (!THIS->image->valid(x, y))
  {
    GB.ReturnInteger(-1);
    return;
  }

  col = THIS->image->pixel(x, y) ^ 0xFF000000;
  GB.ReturnInteger(col);

  #if 0
  a = (col >> 24);

  //qDebug("[%d,%d] = (%d %d %d) / %d", x, y, r, g, b, a);

  if (!THIS->image->hasAlphaBuffer() || a == 255)
    GB.ReturnInteger( col & 0xFFFFFF);
  else if (a == 0)
    GB.ReturnInteger(-1);
  else
  {
    r = col & 0xFF;
    g = (col >> 8) & 0xFF;
    b = (col >> 16) & 0xFF;

    r = 255 - ((255 - r) * a) / 255;
    g = 255 - ((255 - g) * a) / 255;
    b = 255 - ((255 - b) * a) / 255;

    GB.ReturnInteger(r + (g << 8) + (b << 16));
  }
  #endif

END_METHOD


BEGIN_METHOD(CIMAGE_pixels_put, GB_INTEGER col; GB_INTEGER x; GB_INTEGER y)

  int x, y;
  int col;

  x = VARG(x);
  y = VARG(y);

  if (!THIS->image->valid(x, y))
    return;

  col = VARG(col) ^ 0xFF000000;

  THIS->image->setPixel(x, y, col);

END_METHOD

#if 0
BEGIN_PROPERTY(CPICTURE_pixels_data)

  GB.ReturnInteger((long)THIS->image->bits());

END_PROPERTY
#endif

GB_DESC CImageDesc[] =
{
  GB_DECLARE("Image", sizeof(CIMAGE)),

  //GB_STATIC_METHOD("_exit", NULL, CPICTURE_flush, NULL),

  //GB_CONSTANT("None", "i", TYPE_NONE),
  //GB_CONSTANT("Bitmap", "i", TYPE_PIXMAP),
  //GB_CONSTANT("Pixmap", "i", TYPE_PIXMAP),
  //GB_CONSTANT("Vector", "i", TYPE_PICTURE),
  //GB_CONSTANT("Metafile", "i", TYPE_PICTURE),
  //GB_CONSTANT("Image", "i", TYPE_IMAGE),

  GB_METHOD("_new", NULL, CIMAGE_new, "[(Width)i(Height)i(Transparent)b]"),
  GB_METHOD("_free", NULL, CIMAGE_free, NULL),

  GB_METHOD("_get", "i", CIMAGE_pixels_get, "(X)i(Y)i"),
  GB_METHOD("_put", NULL, CIMAGE_pixels_put, "(Color)i(X)i(Y)i"),

  //GB_STATIC_METHOD("_get", "Picture", CPICTURE_get, "(Path)s"),
  //GB_STATIC_METHOD("_put", NULL, CPICTURE_put, "(Picture)Picture;(Path)s"),
  //GB_STATIC_METHOD("Flush", NULL, CPICTURE_flush, NULL),

  //GB_PROPERTY("Type", "i<Picture,None,Pixmap,Image,Metafile>", CPICTURE_type),

  GB_PROPERTY_READ("W", "i", CIMAGE_width),
  GB_PROPERTY_READ("Width", "i", CIMAGE_width),
  GB_PROPERTY_READ("H", "i", CIMAGE_height),
  GB_PROPERTY_READ("Height", "i", CIMAGE_height),
  GB_PROPERTY_READ("Depth", "i", CIMAGE_depth),
  GB_PROPERTY("Transparent", "b", CIMAGE_transparent),

  GB_STATIC_METHOD("Load", "Image", CIMAGE_load, "(Path)s"),
  GB_METHOD("Save", NULL, CIMAGE_save, "(Path)s"),
  GB_METHOD("Resize", NULL, CIMAGE_resize, "(Width)i(Height)i"),

  GB_METHOD("Clear", NULL, CIMAGE_clear, NULL),
  GB_METHOD("Fill", NULL, CIMAGE_fill, "(Color)i"),
  GB_METHOD("Replace", NULL, CIMAGE_replace, "(OldColor)i(NewColor)i"),
  //GB_METHOD("Mask", NULL, CPICTURE_mask, "[(Color)i]"),

  GB_METHOD("Copy", "Image", CIMAGE_copy, "[(X)i(Y)i(Width)i(Height)i]"),
  GB_METHOD("Stretch", "Image", CIMAGE_stretch, "(Width)i(Height)i[(Smooth)b]"),
  GB_METHOD("Flip", "Image", CIMAGE_flip, NULL),
  GB_METHOD("Mirror", "Image", CIMAGE_mirror, NULL),
  GB_METHOD("Rotate", "Image", CIMAGE_rotate, "(Angle)f"),

  GB_PROPERTY_READ("Picture", "Picture", CIMAGE_picture),

  //GB_PROPERTY_SELF("Pixels", ".ImagePixels"),
  //GB_PROPERTY_SELF("Colors", ".PictureColors"),

  GB_END_DECLARE
};

