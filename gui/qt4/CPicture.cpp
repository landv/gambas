/***************************************************************************

	CPicture.cpp

	(c) 2000-2017 Benoît Minisini <gambas@users.sourceforge.net>

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

#define __CPICTURE_CPP

#include <string.h>

#include <qpixmap.h>
#include <qbitmap.h>
#include <qnamespace.h>
#include <qpainter.h>
#include <qmatrix.h>
#include <QByteArray>
#include <QHash>

#include "gambas.h"
#include "main.h"

#include "CDraw.h"
#include "cpaint_impl.h"
#include "CScreen.h"
#include "CImage.h"
#include "CPicture.h"

#ifdef QT5
#include <QScreen>
#include <QDesktopWidget>
#endif

#include <QX11Info>
#include <X11/Xlib.h>


static CPICTURE *create()
{
	return (CPICTURE *)GB.New(GB.FindClass("Picture"), NULL, NULL);
}

#define CREATE_IMAGE_FROM_MEMORY(_image, _addr, _len, _ok) \
{ \
	QImage img; \
	_ok = img.loadFromData((const uchar *)_addr, (uint)_len); \
	if (_ok) \
	{ \
		if (img.depth() < 32 && !img.isNull()) \
			img = img.convertToFormat(QImage::Format_ARGB32_Premultiplied); \
	} \
	_image = new QImage(img); \
}

#define DELETE_IMAGE(_image) delete _image

#define CREATE_PICTURE_FROM_IMAGE(_cpicture, _image) \
{ \
	_cpicture = create(); \
	if ((_image) && !(_image)->isNull()) \
		*((_cpicture)->pixmap) = QPixmap::fromImage(*(_image)); \
}


bool CPICTURE_load_image(QImage **p, const char *path, int lenp)
{
	char *addr;
	int len;
	bool ok;
	
	*p = 0;
	
	if (GB.LoadFile(path, lenp, &addr, &len))
	{
		GB.Error(NULL);
		return FALSE;
	}
	
	CREATE_IMAGE_FROM_MEMORY(*p, addr, len, ok)
	
	GB.ReleaseFile(addr, len);
	return ok;
}



CPICTURE *CPICTURE_grab(QWidget *wid, int screen, int x, int y, int w, int h)
{
	CPICTURE *pict;

	pict = create();

	if (!wid)
	{
		if (w <= 0 || h <= 0)
		{
			x = 0; y = 0; w = -1; h = -1;
		}
		
#ifdef QT5
		*pict->pixmap = QGuiApplication::primaryScreen()->grabWindow(QX11Info::appRootWindow(), x, y, w, h);
#else
		*pict->pixmap = QPixmap::grabWindow(QX11Info::appRootWindow(), x, y, w, h);
#endif
	}
	else
	{
#ifdef QT5
		*pict->pixmap = QGuiApplication::screens().at(QApplication::desktop()->screenNumber(wid))->grabWindow(wid->winId());
#else
		*pict->pixmap = QPixmap::grabWindow(wid->winId());
#endif
	}

	return pict;
}


/*void CPICTURE_update_mask(CPICTURE *_object)
{
	if (THIS->pixmap && THIS->pixmap->hasAlpha())
		THIS->pixmap->setMask(THIS->pixmap->createHeuristicMask());
}*/

CPICTURE *CPICTURE_create(const QPixmap *pixmap)
{
	CPICTURE *pict = create();
	if (pixmap) *pict->pixmap = *pixmap;
	return pict;
}

/*******************************************************************************

	class Picture

*******************************************************************************/


BEGIN_METHOD(CPICTURE_new, GB_INTEGER w; GB_INTEGER h; GB_BOOLEAN trans)

	int w, h;

	if (!MISSING(w) && !MISSING(h))
	{
		w = VARG(w);
		h = VARG(h);
		if (h <= 0 || w <= 0)
		{
			GB.Error("Bad dimension");
			return;
		}

		THIS->pixmap = new QPixmap(w, h);

		if (VARGOPT(trans, false))
		{
			QBitmap b(w, h);
			b.fill(Qt::color0);
			THIS->pixmap->setMask(b);
		}
	}
	else
		THIS->pixmap = new QPixmap;
	
END_METHOD


BEGIN_METHOD_VOID(CPICTURE_free)

	delete THIS->pixmap;
	THIS->pixmap = 0;

END_METHOD


BEGIN_METHOD(CPICTURE_resize, GB_INTEGER width; GB_INTEGER height)

	QPixmap *pixmap = new QPixmap(VARG(width), VARG(height));
	
	QPainter p(pixmap);
	p.drawPixmap(0, 0, *THIS->pixmap);
	p.end();
		
	delete THIS->pixmap;
	THIS->pixmap = pixmap;

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

	if (!CPICTURE_load_image(&img, STRING(path), LENGTH(path)))
	{
		GB.Error("Unable to load picture");
		return;
	}
		
	CREATE_PICTURE_FROM_IMAGE(pict, img);
	DELETE_IMAGE(img);
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

	pict = create();
	delete pict->pixmap;
	pict->pixmap = new QPixmap(w, h);
	
	QPainter p(pict->pixmap);
	p.drawPixmap(0, 0, *THIS->pixmap, x, y, w, h);
	p.end();

	GB.ReturnObject(pict);

END_METHOD


BEGIN_PROPERTY(CPICTURE_image)

	QImage *image = new QImage();
	
	*image = THIS->pixmap->toImage();
	image->detach();

	GB.ReturnObject(CIMAGE_create(image));

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

	GB_METHOD("_new", NULL, CPICTURE_new, "[(Width)i(Height)i(Transparent)b]"),
	GB_METHOD("_free", NULL, CPICTURE_free, NULL),

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
	
	//GB_INTERFACE("Draw", &DRAW_Interface),
	GB_INTERFACE("Paint", &PAINT_Interface),

	GB_END_DECLARE
};

