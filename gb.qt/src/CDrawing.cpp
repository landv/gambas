/***************************************************************************

  CDrawing.cpp

  The Drawing class

  (c) 2000-2005 Beno� Minisini <gambas@users.sourceforge.net>

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

#define __CDRAWING_CPP

#include "gambas.h"

#include <qpicture.h>
#include <qnamespace.h>

#include "CDrawing.h"


static void create(CDRAWING **pdrawing)
{
  static GB_CLASS class_id = NULL;

  if (!class_id)
    class_id = GB.FindClass("Drawing");

  GB.New((void **)pdrawing, class_id, NULL, NULL);
}


static const char *get_format(QString path)
{
  int pos;

  pos = path.findRev('.');
  if (pos < 0)
    return NULL;

  path = path.mid(pos + 1).lower();

  if (path == "svg")
    return "svg";
  else if (path == "pic")
    return "";
  else
    return NULL;
}


/*******************************************************************************

  Drawing

*******************************************************************************/


BEGIN_METHOD_VOID(CDRAWING_new)

  THIS->picture = new QPicture();

END_METHOD


BEGIN_METHOD_VOID(CDRAWING_free)

  delete THIS->picture;
  THIS->picture = 0;

END_METHOD

#if 0
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
#endif

BEGIN_PROPERTY(CDRAWING_width)

  GB.ReturnInteger(THIS->picture->boundingRect().width());

END_PROPERTY


BEGIN_PROPERTY(CDRAWING_height)

  GB.ReturnInteger(THIS->picture->boundingRect().height());

END_PROPERTY


BEGIN_PROPERTY(CDRAWING_depth)

  GB.ReturnInteger(24);

END_PROPERTY


BEGIN_METHOD(CDRAWING_load, GB_STRING path)

	CDRAWING *drawing;
  char *addr;
  long len;
  const char *fmt;
  bool ok = false;
  QString path = TO_QSTRING(GB.FileName(STRING(path), LENGTH(path)));

  fmt = get_format(path);
  if (!fmt)
  {
    GB.Error("Unknown drawing format");
    return;
  }

  if (!GB.LoadFile(STRING(path), LENGTH(path), &addr, &len))
  {
    QPicture p;
    QByteArray a;
    a.setRawData(addr, (int)len);
    QBuffer b(a);
    if (*fmt == 0)
      ok = p.load(&b);
    else
      ok = p.load(&b, fmt);
    //ok = p->load("/home/benoit/gambas/test/Test/test.svg", fmt->format);
    a.resetRawData(addr, (int)len);

    if (ok)
    {
      create(&drawing);
      *(drawing->picture) = p;
      //drawing->width = p.boundingRect().width();
      //drawing->height = p.boundingRect().height();
    }

    GB.ReleaseFile(&addr, len);
  }

  if (!ok)
    GB.Error("Unable to load drawing");
	else
		GB.ReturnObject(drawing);

END_METHOD


BEGIN_METHOD(CDRAWING_save, GB_STRING path)

  QString path = TO_QSTRING(GB.FileName(STRING(path), LENGTH(path)));
  bool ok = false;
  const char *fmt = get_format(path);

  if (!fmt)
  {
    GB.Error("Unknown picture format");
    return;
  }

  ok = THIS->picture->save(path, fmt);

  if (!ok)
    GB.Error("Unable to save picture");

END_METHOD


BEGIN_METHOD_VOID(CDRAWING_clear)

  delete THIS->picture;
  THIS->picture = new QPicture;

END_METHOD


BEGIN_METHOD_VOID(CDRAWING_copy)

  CDRAWING *drawing;

  create(&drawing);
  delete drawing->picture;
  drawing->picture = new QPicture(*THIS->picture);
  //drawing->width = THIS->width;
  //drawing->height = THIS->height;

  GB.ReturnObject(drawing);

END_METHOD

#if 0
BEGIN_METHOD(CDRAWING_stretch, GB_INTEGER width; GB_INTEGER height)

  CDRAWING *drawing;

  create(&drawing);
  *(drawing->picture) = THIS->picture->copy();
  drawing->width = VARG(width);
  drawing->height = VARG(height);

  GB.ReturnObject(drawing);

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
#endif



GB_DESC CDrawingDesc[] =
{
  GB_DECLARE("Drawing", sizeof(CDRAWING)),

  //GB_STATIC_METHOD("_exit", NULL, CPICTURE_flush, NULL),

  //GB_CONSTANT("None", "i", TYPE_NONE),
  //GB_CONSTANT("Bitmap", "i", TYPE_PIXMAP),
  //GB_CONSTANT("Pixmap", "i", TYPE_PIXMAP),
  //GB_CONSTANT("Vector", "i", TYPE_PICTURE),
  //GB_CONSTANT("Metafile", "i", TYPE_PICTURE),
  //GB_CONSTANT("Image", "i", TYPE_IMAGE),

  GB_METHOD("_new", NULL, CDRAWING_new, NULL),
  GB_METHOD("_free", NULL, CDRAWING_free, NULL),

  //GB_STATIC_METHOD("_get", "Picture", CPICTURE_get, "(Path)s"),
  //GB_STATIC_METHOD("_put", NULL, CPICTURE_put, "(Picture)Picture;(Path)s"),
  //GB_STATIC_METHOD("Flush", NULL, CPICTURE_flush, NULL),

  //GB_PROPERTY("Type", "i<Picture,None,Pixmap,Image,Metafile>", CPICTURE_type),

  GB_PROPERTY_READ("Width", "i", CDRAWING_width),
  GB_PROPERTY_READ("Height", "i", CDRAWING_height),
  GB_PROPERTY_READ("Depth", "i", CDRAWING_depth),

  GB_STATIC_METHOD("Load", "Drawing", CDRAWING_load, "(Path)s"),
  GB_METHOD("Save", NULL, CDRAWING_save, "(Path)s"),
  //GB_METHOD("Resize", NULL, CIMAGE_resize, "(Width)i(Height)i"),

  GB_METHOD("Clear", NULL, CDRAWING_clear, NULL),

  GB_METHOD("Copy", "Drawing", CDRAWING_copy, NULL),
  //GB_METHOD("Stretch", "Image", CIMAGE_stretch, "(Width)i(Height)i[(Smooth)b]"),
  //GB_METHOD("Flip", "Image", CIMAGE_flip, NULL),
  //GB_METHOD("Mirror", "Image", CIMAGE_mirror, NULL),
  //GB_METHOD("Rotate", "Image", CIMAGE_rotate, "(Angle)f"),

  //GB_PROPERTY_READ("Picture", "Picture", CIMAGE_picture),

  GB_END_DECLARE
};

