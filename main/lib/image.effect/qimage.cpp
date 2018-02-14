/***************************************************************************

  qimage.cpp

  (c) 2000-2017 Benoît Minisini <g4mba5@gmail.com>

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
	jt = NULL;
	inv = FALSE;
}

void QImage::check()
{
	if (!img)
		return;
	if (GB_IMAGE_FMT_IS_SWAPPED(img->format))
		fprintf(stderr, "gb.image.effect: warning: unsupported image format: %s\n", IMAGE.FormatToString(img->format));
	inv = GB_IMAGE_FMT_IS_RGBA(img->format);
}

void QImage::create(int w, int h, bool t)
{
	img = IMAGE.Create(w, h, t ? GB_IMAGE_BGRA : GB_IMAGE_BGRX, NULL);
	//GB.Ref(img);
	check();
	jumpTable();
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
	//GB.Ref(img);
	SYNCHRONIZE_IMAGE(img);
	check();
	jumpTable();
}

QImage::QImage(const QImage &copy)
{
	init();
	img = (GB_IMG *)copy.object();
	//GB.Ref(img);
	SYNCHRONIZE_IMAGE(img);
	check();
}

QImage::~QImage()
{
	if (jt)
	{
		free(jt);
		jt = NULL;
	}
}

void QImage::release()
{
	GB.Unref(POINTER(&img));
	img = NULL;
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
		int bpl = 4 * width();
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
