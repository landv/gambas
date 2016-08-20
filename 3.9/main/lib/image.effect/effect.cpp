/***************************************************************************

  effect.cpp

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

#define __EFFECT_CPP

#include <math.h>

#include "effect.h"
#include "main.h"

static void get_info(GB_IMAGE img, uint **data, uint *width, uint *height, uint *npixels, bool *inv)
{
	GB_IMG *info = (GB_IMG *)img;

	SYNCHRONIZE_IMAGE(info);
	
	*data = (uint *)info->data;
	if (width) *width = info->width;
	if (height) *height = info->height;
	if (npixels) *npixels = info->width * info->height;
	if (inv) *inv = GB_IMAGE_FMT_IS_SWAPPED(info->format); //(info->format == GB_IMAGE_RGBA || info->format == GB_IMAGE_RGBX);
}

static inline int between0And255 (int val)
{
	if (val < 0)
		return 0;
	else if (val > 255)
		return 255;
	else
		return val;
}

static inline uint RGBA(int r, int g, int b, int a)
{
	return ((a & 0xff) << 24) | ((r & 0xff) << 16) | ((g & 0xff) << 8) | (b & 0xff);
}

static inline int R(uint rgb ) { return (int)((rgb >> 16) & 0xff); }
static inline int G(uint rgb ) { return (int)((rgb >> 8) & 0xff); }
static inline int B(uint rgb ) { return (int)(rgb & 0xff); }
static inline int A(uint rgb ) { return (int)((rgb >> 24) & 0xff); }

static inline uint invert(uint col)
{
	return ((col & 0xFF)) << 16 | ((col & 0xFF0000)) >> 16 | (col & 0xFF00FF00);
}

static inline int myRound(double d)
{
	return d >= 0.0 ? int(d + 0.5) : int( d - ((int)d-1) + 0.5 ) + ((int)d-1);
}

static inline int brightness (int base, int strength)
{
	return between0And255(base + strength * 255 / 50);
}

static inline int contrast (int base, int strength)
{
	return between0And255 ((base - 127) * (strength + 50) / 50 + 127);
}

static inline int gamma (int base, int strength)
{
	return between0And255 (myRound(255.0 * pow (base / 255.0, 1.0 / pow (10, strength / 50.0))));
}

static inline int brightnessContrastGamma(int base, int newBrightness, int newContrast, int newGamma)
{
	return gamma(contrast(brightness(base, newBrightness), newContrast), newGamma);
}

void Effect::balance(GB_IMAGE img, int channels, int brightness, int contrast, int gamma)
{
  uchar transformRed[256], transformGreen[256], transformBlue[256];
  uint i;
  uint np;
  uint *p;
	bool inv;
	uint col;

	get_info(img, &p, NULL, NULL, &np, &inv);

	if (!inv)
	{
		for (i = 0; i < 256; i++)
		{
			uchar applied = brightnessContrastGamma(i, brightness, contrast, gamma);

			transformRed[i] = (channels & Red) ? applied : i;
			transformGreen[i] = (channels & Green) ? applied : i;
			transformBlue[i] = (channels & Blue) ? applied : i;
		}
	}
	else
	{
		for (i = 0; i < 256; i++)
		{
			uchar applied = brightnessContrastGamma(i, brightness, contrast, gamma);

			transformBlue[i] = (channels & Red) ? applied : i;
			transformGreen[i] = (channels & Green) ? applied : i;
			transformRed[i] = (channels & Blue) ? applied : i;
		}
	}

	for (i = 0; i < np; i++)
	{
		col = p[i];
		p[i] = RGBA(transformRed[R(col)], transformGreen[G(col)], transformBlue[B(col)], A(col));
	}
}

void Effect::invert(GB_IMAGE img, int channels)
{
  uint np;
  uint *p;
	bool inv;
	uint mask;
	uint i;

	get_info(img, &p, NULL, NULL, &np, &inv);

	if (!inv)
		mask = RGBA((channels & Red) ? 0xFF : 0, (channels & Green) ? 0xFF : 0, (channels & Blue) ? 0xFF : 0, 0);
	else
		mask = RGBA((channels & Blue) ? 0xFF : 0, (channels & Green) ? 0xFF : 0, (channels & Red) ? 0xFF : 0, 0);

	for (i = 0; i < np; i++)
		p[i] ^= mask;
}

