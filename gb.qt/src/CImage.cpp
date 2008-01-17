/***************************************************************************

  CImage.cpp

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

#define __CIMAGE_CPP

#include <string.h>

#include <qpixmap.h>
#include <qbitmap.h>
#include <qpicture.h>
#include <qnamespace.h>
#include <qdict.h>
#include <qpainter.h>
#include <qwmatrix.h>

#ifdef OS_SOLARIS
/* Make math.h define M_PI and a few other things */
#define __EXTENSIONS__
/* Get definition for finite() */
#include <ieeefp.h>
#endif
#include <math.h>

#include "gambas.h"
#include "main.h"

#include "CScreen.h"
#include "CPicture.h"
#include "CDraw.h"
#include "CImage.h"

static void create(CIMAGE **pimage)
{
  static GB_CLASS class_id = NULL;

  if (!class_id)
    class_id = GB.FindClass("Image");

  GB.New(POINTER(pimage), class_id, NULL, NULL);
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

// bool CIMAGE_load_image(QImage &p, char *path, int lenp)
// {
//   char *addr;
//   int len;
//   bool ok;
//   char *path_theme;
//   int pos;
//   
//   if (CAPPLICATION_Theme && lenp > 0 && path[0] != '/')
//   {
//   	pos = lenp - 1;
//   	while (pos >= 0)
//   	{
//   		if (path[pos] == '.')
//   			break;
// 			pos--;
//   	}
//   	GB.NewString(&path_theme, path, pos >= 0 ? pos : lenp);
//   	GB.AddString(&path_theme, "_", 1);
//   	GB.AddString(&path_theme, CAPPLICATION_Theme, GB.StringLength(CAPPLICATION_Theme));
//   	if (pos >= 0)
//   		GB.AddString(&path_theme, &path[pos], lenp - pos);
// 		ok = !GB.LoadFile(path_theme, GB.StringLength(path_theme), &addr, &len);
// 		GB.FreeString(&path_theme);
// 		if (ok)
// 			goto __LOAD;
//   }
// 
// 	GB.Error(NULL);
// 	if (GB.LoadFile(path, lenp, &addr, &len))
// 		return FALSE;
// 		
// __LOAD:
// 		
// 	ok = p.loadFromData((const uchar *)addr, (uint)len);
// 	if (ok)
// 	{
// 		//create((CIMAGE **)&_object);
// 
// 		if (p.depth() < 32)
// 			p = p.convertDepth(32);
// 
// 		//p.setAlphaBuffer(true);
// 	}
// 
// 	GB.ReleaseFile(&addr, len);
//   return ok;
// }
// 


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

  GB.New(POINTER(&pict), GB.FindClass("Picture"), NULL, NULL);
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

  QImage *p;
  CIMAGE *img;

  create(&img);

  if (CPICTURE_load_image(&p, STRING(path), LENGTH(path)))
  {
    img->image = p;
    GB.ReturnObject(img);
	}
  else
  {
  	GB.Unref(POINTER(&img));
    GB.Error("Unable to load image");
	}

END_METHOD


BEGIN_METHOD(CIMAGE_save, GB_STRING path; GB_INTEGER quality)

  QString path = TO_QSTRING(GB.FileName(STRING(path), LENGTH(path)));
  bool ok = false;
  const char *fmt = CIMAGE_get_format(path);

  if (!fmt)
  {
    GB.Error("Unknown format");
    return;
  }

  ok = THIS->image->save(path, fmt, VARGOPT(quality, -1));

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
  img->image->setAlphaBuffer(false);

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
  bool a = THIS->image->hasAlphaBuffer();

  create(&img);

  mat.rotate(VARG(angle) * -360.0 / 2 / M_PI);
  
  THIS->image->setAlphaBuffer(true);
  *(img->image) = THIS->image->xForm(mat);
  THIS->image->setAlphaBuffer(a);
  
  GB.ReturnObject(img);

END_METHOD


BEGIN_METHOD(CIMAGE_replace, GB_INTEGER src; GB_INTEGER dst; GB_BOOLEAN noteq)

	uint *p;
  uint i, n, src, dst;

  THIS->image->setAlphaBuffer(true);

  src = VARG(src) ^ 0xFF000000;
  dst = VARG(dst) ^ 0xFF000000;

	p = (uint *)THIS->image->bits();
	n = THIS->image->height() * THIS->image->width();

	if (VARGOPT(noteq, false))
	{
		for (i = 0; i < n; i++, p++)
		{
			if (*p != src)
				*p = dst;
		}
	}
	else
	{
		for (i = 0; i < n; i++,p ++)
		{
			if (*p == src)
				*p = dst;
		}
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

BEGIN_PROPERTY(CIMAGE_data)

  GB.ReturnPointer((void *)THIS->image->bits());

END_PROPERTY

BEGIN_METHOD(CIMAGE_draw, GB_OBJECT img; GB_INTEGER x; GB_INTEGER y; GB_INTEGER w; GB_INTEGER h; GB_INTEGER sx; GB_INTEGER sy; GB_INTEGER sw; GB_INTEGER sh)

  int x, y, w, h, sx, sy, sw, sh;
  CIMAGE *image = (CIMAGE *)VARG(img);
  QImage img;
  double scale_x, scale_y;

  if (GB.CheckObject(image))
    return;

	img = *(image->image);

  x = VARGOPT(x, 0);
  y = VARGOPT(y, 0);
  w = VARGOPT(w, -1);
  h = VARGOPT(h, -1);

  sx = VARGOPT(sx, 0);
  sy = VARGOPT(sy, 0);
  sw = VARGOPT(sw, img.width());
  sh = VARGOPT(sh, img.height());

  DRAW_NORMALIZE(x, y, w, h, sx, sy, sw, sh, img.width(), img.height());

	if (w != sw || h != sh)
	{
		scale_x = (double)w / sw;
		scale_y = (double)h / sh;
		
		img = img.smoothScale((int)(img.width() * scale_x + 0.5), (int)(img.height() * scale_y + 0.5));
		sx = (int)(sx * scale_x + 0.5);
		sy = (int)(sy * scale_y + 0.5);
		sw = w;
		sh = h;
	}
	
	bitBlt(THIS->image, x, y, &img, sx, sy, sw, sh, Qt::AutoColor); 

END_METHOD


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
  GB_METHOD("Save", NULL, CIMAGE_save, "(Path)s[(Quality)i]"),
  GB_METHOD("Resize", NULL, CIMAGE_resize, "(Width)i(Height)i"),

  GB_METHOD("Clear", NULL, CIMAGE_clear, NULL),
  GB_METHOD("Fill", NULL, CIMAGE_fill, "(Color)i"),
  GB_METHOD("Replace", NULL, CIMAGE_replace, "(OldColor)i(NewColor)i[(NotEqual)b]"),
  //GB_METHOD("Mask", NULL, CPICTURE_mask, "[(Color)i]"),

  GB_METHOD("Copy", "Image", CIMAGE_copy, "[(X)i(Y)i(Width)i(Height)i]"),
  GB_METHOD("Stretch", "Image", CIMAGE_stretch, "(Width)i(Height)i[(Smooth)b]"),
  GB_METHOD("Flip", "Image", CIMAGE_flip, NULL),
  GB_METHOD("Mirror", "Image", CIMAGE_mirror, NULL),
  GB_METHOD("Rotate", "Image", CIMAGE_rotate, "(Angle)f"),

  GB_METHOD("Draw", NULL, CIMAGE_draw, "(Image)Image;(X)i(Y)i[(Width)i(Height)i(SrcX)i(SrcY)i(SrcWidth)i(SrcHeight)i]"),

  GB_PROPERTY_READ("Picture", "Picture", CIMAGE_picture),

  GB_PROPERTY_READ("Data", "p", CIMAGE_data),

  //GB_PROPERTY_SELF("Pixels", ".ImagePixels"),
  //GB_PROPERTY_SELF("Colors", ".PictureColors"),

  GB_END_DECLARE
};

