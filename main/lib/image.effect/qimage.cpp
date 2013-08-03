/***************************************************************************

  qimage.cpp

  (c) 2000-2013 Benoît Minisini <gambas@users.sourceforge.net>

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

#include "qimage.h"

void QImage::init()
{
	img = NULL;
	bpl = 0;
	inv = FALSE;
	jt = NULL;
	created = false;
}

void QImage::getInfo()
{
	bpl = 4 * img->width;
	inv = GB_IMAGE_FMT_IS_RGBA(img->format);
	
	if (GB_IMAGE_FMT_IS_SWAPPED(img->format))
		fprintf(stderr, "gb.image.effect: warning: unsupported image format: %s\n", IMAGE.FormatToString(img->format));

	jumpTable();
}

void QImage::create(int w, int h, bool t)
{
	img = IMAGE.Create(w, h, t ? GB_IMAGE_BGRA : GB_IMAGE_BGRX, NULL);
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
	img = (GB_IMG *)image;
	SYNCHRONIZE_IMAGE(img);
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
	{
		GB.Unref((void **)&img);
		created = false;
	}
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
	if (!jt && img->data)
	{
    jt = (uchar**)malloc(height() * sizeof(uchar*));
    for (int i = 0; i < height(); i++)
			jt[i] = (uchar *)img->data + i * bpl;
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


