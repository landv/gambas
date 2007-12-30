/***************************************************************************

  qimage.cpp

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

#include "qimage.h"

void QImage::init()
{
	img = NULL;
	info.width = info.height = 0;
	bpl = 0;
	inv = FALSE;
	jt = NULL;
	created = false;
}

void QImage::getInfo()
{
	GB.Image.Info(img, &info);
	bpl = 4 * info.width;
	inv = info.format == GB_IMAGE_ARGB || info.format == GB_IMAGE_XRGB;
	if (inv)
		fprintf(stderr, "ARGB\n");
	jumpTable();
}

void QImage::create(int w, int h, bool t)
{
	GB.Image.Create(&img, NULL, w, h, t ? GB_IMAGE_ARGB : GB_IMAGE_XRGB);
	created = true;
	getInfo();
}

QImage::QImage()
{
	init();
}

QImage::QImage(const QSize &size, bool t)
{
	init();
	create(size.width(), size.height(), t);
}

QImage::QImage(int w, int h, bool t)
{
	init();
	create(w, h, t);
}

QImage::QImage(GB_IMAGE image)
{
	init();
	img = image;
	getInfo();
}

QImage::~QImage()
{
	if (jt)
    free(jt);
}

void QImage::release()
{
	if (created)
		GB.Unref((void **)&img);
}

/*static inline uint invert(uint col)
{
	return (col >> 24) | (col << 24) | ((col & 0x00FF0000) >> 8) | ((col & 0x0000FF00) << 8);
}*/

void QImage::invert()
{
	uint i, n;
	uchar *p, t;

	n = width() * height();
	p = (uchar *)bits();

	for (i = 0; i < n; i++, p += 4)
	{
		t = p[0];
		p[0] = p[2];
		p[2] = t;
	}
}


uchar **QImage::jumpTable()
{
	if (!jt && info.data)
	{
    jt = (uchar**)malloc(height() * sizeof(uchar*));
    for (int i = 0; i < height(); i++)
			jt[i] = (uchar *)info.data + i * bpl;
	}

	return jt;
}

void QImage::invertPixels()
{
	uint i, n;
	uint *p;

	n = width() * height();
	p = (uint *)bits();

	for (i = 0; i < n; i++)
		p[i] ^= 0xFFFFFF;
}


