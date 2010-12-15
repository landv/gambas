/***************************************************************************

  CImage.cpp

  (c) 2000-2009 Benoît Minisini <gambas@users.sourceforge.net>

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
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

***************************************************************************/

#define __CIMAGE_CPP

#include <string.h>

#include <qpixmap.h>
#include <qbitmap.h>
#include <qnamespace.h>
#include <qpainter.h>
#include <qmatrix.h>

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
#include "cpaint_impl.h"
#include "CImage.h"

const char *CIMAGE_get_format(QString path)
{
  int pos;

  pos = path.lastIndexOf('.');
  if (pos < 0)
    return NULL;

  path = path.mid(pos + 1).toLower();

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


/*******************************************************************************

  Image

*******************************************************************************/

static void free_image(GB_IMG *img, void *image)
{
	delete (QImage *)image;
}

static void *temp_image(GB_IMG *img)
{
	QImage *image;

	if (!img->data)
		image = new QImage();
	else
		image = new QImage((uchar *)img->data, img->width, img->height, QImage::Format_ARGB32);
	
	return image;
}

static GB_IMG_OWNER _image_owner = {
	"gb.qt4",
	GB_IMAGE_BGRA,
	free_image,
 	free_image,
	temp_image,
	NULL,
	};

QImage *CIMAGE_get(CIMAGE *_object)
{
	return (QImage *)IMAGE.Check(THIS_IMAGE, &_image_owner);
}

#define check_image CIMAGE_get

static void take_image(CIMAGE *_object, QImage *image)
{
	bool d = image->isDetached();
	//qDebug("take_image: %d x %d / %d [%c]", image->width(), image->height(), image->bytesPerLine(), image->isDetached() ? 'D' : '+');
	const uchar *data = ((const QImage *)image)->bits();
	//qDebug("take_image: [%c]", image->isDetached() ? 'D' : '+');
	if (image->isDetached() != d)
		qDebug("image has been detached! %d x %d", image->width(), image->height());
	IMAGE.Take(THIS_IMAGE, &_image_owner, image, image->width(), image->height(), (uchar *)data);
}

CIMAGE *CIMAGE_create(QImage *image)
{
	CIMAGE *img;
  static GB_CLASS class_id = NULL;

  if (!class_id)
    class_id = GB.FindClass("Image");

  GB.New(POINTER(&img), class_id, NULL, NULL);
  
  if (image)
	{
		if (!image->isNull() && image->format() != QImage::Format_ARGB32)
			*image = image->convertToFormat(QImage::Format_ARGB32);
  	take_image(img, image);
	}
	else
  	take_image(img, new QImage());
	
  return img;
}

BEGIN_PROPERTY(CIMAGE_picture)

  CPICTURE *pict;
  
  check_image(THIS);

  GB.New(POINTER(&pict), GB.FindClass("Picture"), NULL, NULL);
  if (!QIMAGE->isNull())
  	*pict->pixmap = QPixmap::fromImage(*QIMAGE);

  GB.ReturnObject(pict);

END_PROPERTY

#if 0
BEGIN_METHOD(CIMAGE_resize, GB_INTEGER width; GB_INTEGER height)

  check_image(THIS);

  if (QIMAGE->isNull())
  {
  	take_image(THIS, new QImage(VARG(width), VARG(height), QImage::Format_ARGB32));
  }
  else
  {
    take_image(THIS, new QImage(QIMAGE->copy(0, 0, VARG(width), VARG(height))));
  }

END_METHOD
#endif

BEGIN_METHOD(CIMAGE_load, GB_STRING path)

  QImage *p;
  CIMAGE *img;

  if (CPICTURE_load_image(&p, STRING(path), LENGTH(path)))
  {
  	p->convertToFormat(QImage::Format_ARGB32);
	  img = CIMAGE_create(p);
    GB.ReturnObject(img);
	}
  else
    GB.Error("Unable to load image");

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

  check_image(THIS);

	ok = QIMAGE->save(path, fmt, VARGOPT(quality, -1));

  if (!ok)
    GB.Error("Unable to save picture");

END_METHOD

BEGIN_METHOD(CIMAGE_stretch, GB_INTEGER width; GB_INTEGER height)

	//static int count = 0;
  QImage *stretch;

  check_image(THIS);

	if (QIMAGE->isNull())
	{
    stretch = new QImage(VARG(width), VARG(height), QImage::Format_ARGB32);
	}
	else
	{
		stretch = new QImage();
		*stretch = QIMAGE->scaled(VARG(width), VARG(height), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
		stretch->detach();
	}

  GB.ReturnObject(CIMAGE_create(stretch));

END_METHOD

#if 0
BEGIN_METHOD_VOID(CIMAGE_flip)

	QImage *mirror = new QImage();
	
  check_image(THIS);
  *mirror = QIMAGE->mirrored(true, false);
	
  GB.ReturnObject(CIMAGE_create(mirror));

END_METHOD
#endif

#if 0
BEGIN_METHOD_VOID(CIMAGE_mirror)

	QImage *mirror = new QImage();
	
  check_image(THIS);
  *mirror = QIMAGE->mirrored(false, true);
	
  GB.ReturnObject(CIMAGE_create(mirror));

END_METHOD
#endif

BEGIN_METHOD(CIMAGE_rotate, GB_FLOAT angle; GB_BOOLEAN smooth)

  QImage *rotate = new QImage();
  QMatrix mat;
  
  check_image(THIS);
  
  mat.rotate(VARG(angle) * -360.0 / 2 / M_PI);
  *rotate = QIMAGE->transformed(mat);
  GB.ReturnObject(CIMAGE_create(rotate));

END_METHOD


BEGIN_METHOD(CIMAGE_draw, GB_OBJECT img; GB_INTEGER x; GB_INTEGER y; GB_INTEGER w; GB_INTEGER h; GB_INTEGER sx; GB_INTEGER sy; GB_INTEGER sw; GB_INTEGER sh)

  int x, y, w, h, sx, sy, sw, sh;
  CIMAGE *image = (CIMAGE *)VARG(img);
  QImage *src, *dst;
  double scale_x, scale_y;

  if (GB.CheckObject(image))
    return;

	src = check_image(image);
	dst = check_image(THIS);

  x = VARGOPT(x, 0);
  y = VARGOPT(y, 0);
  w = VARGOPT(w, -1);
  h = VARGOPT(h, -1);

  sx = VARGOPT(sx, 0);
  sy = VARGOPT(sy, 0);
  sw = VARGOPT(sw, src->width());
  sh = VARGOPT(sh, src->height());

  DRAW_NORMALIZE(x, y, w, h, sx, sy, sw, sh, src->width(), src->height());

	if (w != sw || h != sh)
	{
		QImage tmp;
		
		scale_x = (double)w / sw;
		scale_y = (double)h / sh;
		
		tmp = src->scaled((int)(src->width() * scale_x + 0.5), (int)(src->height() * scale_y + 0.5), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
		sx = (int)(sx * scale_x + 0.5);
		sy = (int)(sy * scale_y + 0.5);
		sw = w;
		sh = h;
	
		QPainter p(dst);
		p.drawImage(x, y, tmp, sx, sy, sw, sh);
		p.end();
	}
	else
	{
		QPainter p(dst);
		p.drawImage(x, y, *src, sx, sy, sw, sh);
		p.end();
	}

END_METHOD

GB_DESC CImageDesc[] =
{
  GB_DECLARE("Image", sizeof(CIMAGE)),

  GB_STATIC_METHOD("Load", "Image", CIMAGE_load, "(Path)s"),
  GB_METHOD("Save", NULL, CIMAGE_save, "(Path)s[(Quality)i]"),

	GB_METHOD("Stretch", "Image", CIMAGE_stretch, "(Width)i(Height)i"),
  GB_METHOD("Rotate", "Image", CIMAGE_rotate, "(Angle)f"),

  GB_METHOD("Draw", NULL, CIMAGE_draw, "(Image)Image;(X)i(Y)i[(Width)i(Height)i(SrcX)i(SrcY)i(SrcWidth)i(SrcHeight)i]"),

  GB_PROPERTY_READ("Picture", "Picture", CIMAGE_picture),
	GB_INTERFACE("Paint", &PAINT_Interface),
	
  GB_END_DECLARE
};

