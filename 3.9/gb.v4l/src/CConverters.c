/***************************************************************************

  CConverters.c

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

/*

Collection of conversion routines from various sources ..
All provided under GPL or "as-is" licenses.

*/

#include <stdio.h>
#include <stdlib.h>

int convert_rgb_to_yuv_pixel(int r, int g, int b)
{
	unsigned int pixel32 = 0;
	unsigned char *pixel = (unsigned char *)&pixel32;
	int y, u, v;

	y =   0.299 * (r - 128) + 0.587 * (g - 128) + 0.114 * (b - 128) + 128;
	u = - 0.147 * (r - 128) - 0.289 * (g - 128) + 0.436 * (b - 128) + 128;
	v =   0.615 * (r - 128) - 0.515 * (g - 128) - 0.100 * (b - 128) + 128;

	if(y > 255) y = 255;
	if(u > 255) u = 255;
	if(v > 255) v = 255;
	if(y < 0) y = 0;
	if(u < 0) u = 0;
	if(v < 0) v = 0;

	pixel[0] = y;
	pixel[1] = u;
	pixel[2] = v;

	return pixel32;
}


int convert_rgb_to_yuv_buffer(unsigned char *rgb, unsigned char *yuv, unsigned int width, unsigned int height)
{
	unsigned int in, out = 0;
	unsigned int pixel32;
	int y0, u0, v0, y1, u1, v1;

	for(in = 0; in < width * height * 3; in += 6) {
		pixel32 = convert_rgb_to_yuv_pixel(rgb[in], rgb[in + 1], rgb[in + 2]);
		y0 = (pixel32 & 0x000000ff);
		u0 = (pixel32 & 0x0000ff00) >>  8;
		v0 = (pixel32 & 0x00ff0000) >> 16;

		pixel32 = convert_rgb_to_yuv_pixel(rgb[in + 3], rgb[in + 4], rgb[in + 5]);
		y1 = (pixel32 & 0x000000ff);
		u1 = (pixel32 & 0x0000ff00) >>  8;
		v1 = (pixel32 & 0x00ff0000) >> 16;

		yuv[out++] = y0;
		yuv[out++] = (u0 + u1) / 2;
		yuv[out++] = y1;
		yuv[out++] = (v0 + v1) / 2;
	}

	return 0;
}

int convert_yuv_to_rgb_pixel(int y, int u, int v)
{
	unsigned int pixel32 = 0;
	unsigned char *pixel = (unsigned char *)&pixel32;
	int r, g, b;

	r = y + (1.370705 * (v-128));
	g = y - (0.698001 * (v-128)) - (0.337633 * (u-128));
	b = y + (1.732446 * (u-128));

	if(r > 255) r = 255;
	if(g > 255) g = 255;
	if(b > 255) b = 255;
	if(r < 0) r = 0;
	if(g < 0) g = 0;
	if(b < 0) b = 0;

	pixel[0] = r * 220 / 256;
	pixel[1] = g * 220 / 256;
	pixel[2] = b * 220 / 256;

	return pixel32;
}


int convert_yuv_to_rgb_buffer(unsigned char *yuv, unsigned char *rgb, unsigned int width, unsigned int height)
{
	unsigned int in, out = 0;
	unsigned int pixel_16;
	unsigned char pixel_24[3];
	unsigned int pixel32;
	int y0, u, y1, v;

	for(in = 0; in < width * height * 2; in += 4) {
		pixel_16 =
			yuv[in + 3] << 24 |
			yuv[in + 2] << 16 |
			yuv[in + 1] <<  8 |
			yuv[in + 0];

		y0 = (pixel_16 & 0x000000ff);
		u  = (pixel_16 & 0x0000ff00) >>  8;
		y1 = (pixel_16 & 0x00ff0000) >> 16;
		v  = (pixel_16 & 0xff000000) >> 24;

		pixel32 = convert_yuv_to_rgb_pixel(y0, u, v);
		pixel_24[0] = (pixel32 & 0x000000ff);
		pixel_24[1] = (pixel32 & 0x0000ff00) >> 8;
		pixel_24[2] = (pixel32 & 0x00ff0000) >> 16;

		rgb[out++] = pixel_24[0];
		rgb[out++] = pixel_24[1];
		rgb[out++] = pixel_24[2];

		pixel32 = convert_yuv_to_rgb_pixel(y1, u, v);
		pixel_24[0] = (pixel32 & 0x000000ff);
		pixel_24[1] = (pixel32 & 0x0000ff00) >> 8;
		pixel_24[2] = (pixel32 & 0x00ff0000) >> 16;

		rgb[out++] = pixel_24[0];
		rgb[out++] = pixel_24[1];
		rgb[out++] = pixel_24[2];
	}

	return 0;
}

static inline void move_420_block (int yTL, int yTR, int yBL, int yBR, int u,
                                   int v, int rowPixels, unsigned char *rgb,
                                   int bits);

#define LIMIT(x) ((x)>0xffffff?0xff: ((x)<=0xffff?0:((x)>>16)))

void
yuv420p_to_rgb (unsigned char *image, unsigned char *image2, int x, int y, int z) {
    const int numpix = x * y;
    const int bytes = z;        /* (z*8) >> 3; */
    int i, j, y00, y01, y10, y11, u, v;
    unsigned char *pY = image;
    unsigned char *pU = pY + numpix;
    unsigned char *pV = pU + numpix / 4;

    for (j = 0; j <= y - 2; j += 2) {
        for (i = 0; i <= x - 2; i += 2) {
            y00 = *pY;
            y01 = *(pY + 1);
            y10 = *(pY + x);
            y11 = *(pY + x + 1);
            u = (*pU++) - 128;
            v = (*pV++) - 128;

                move_420_block (y00, y01, y10, y11, u, v, x, image2, z * 8);

                pY += 2;
                image2 += 2 * bytes;
            }
            pY += x;
            image2 += x * bytes;
        }
}

void move_420_block (int yTL, int yTR, int yBL, int yBR, int u, int v,
                int rowPixels, unsigned char *rgb, int bits)
{
    const int rvScale = 91881;
    const int guScale = -22553;
    const int gvScale = -46801;
    const int buScale = 116129;
    const int yScale = 65536;
    int r, g, b;

    g = guScale * u + gvScale * v;
    if (1) {
        r = buScale * u;
        b = rvScale * v;
    } else {
        r = rvScale * v;
        b = buScale * u;
    }

    yTL *= yScale;
    yTR *= yScale;
    yBL *= yScale;
    yBR *= yScale;

    if (bits == 24) {
        /* Write out top two pixels */
        rgb[0] = LIMIT (b + yTL);
        rgb[1] = LIMIT (g + yTL);
        rgb[2] = LIMIT (r + yTL);

        rgb[3] = LIMIT (b + yTR);
        rgb[4] = LIMIT (g + yTR);
        rgb[5] = LIMIT (r + yTR);

        /* Skip down to next line to write out bottom two pixels */
        rgb += 3 * rowPixels;
        rgb[0] = LIMIT (b + yBL);
        rgb[1] = LIMIT (g + yBL);
        rgb[2] = LIMIT (r + yBL);

        rgb[3] = LIMIT (b + yBR);
        rgb[4] = LIMIT (g + yBR);
        rgb[5] = LIMIT (r + yBR);
    } else if (bits == 16) {
        /* Write out top two pixels */
        rgb[0] = ((LIMIT (b + yTL) >> 3) & 0x1F)
            | ((LIMIT (g + yTL) << 3) & 0xE0);
        rgb[1] = ((LIMIT (g + yTL) >> 5) & 0x07)
            | (LIMIT (r + yTL) & 0xF8);

        rgb[2] = ((LIMIT (b + yTR) >> 3) & 0x1F)
            | ((LIMIT (g + yTR) << 3) & 0xE0);
        rgb[3] = ((LIMIT (g + yTR) >> 5) & 0x07)
            | (LIMIT (r + yTR) & 0xF8);

        /* Skip down to next line to write out bottom two pixels */
        rgb += 2 * rowPixels;

        rgb[0] = ((LIMIT (b + yBL) >> 3) & 0x1F)
            | ((LIMIT (g + yBL) << 3) & 0xE0);
        rgb[1] = ((LIMIT (g + yBL) >> 5) & 0x07)
            | (LIMIT (r + yBL) & 0xF8);

        rgb[2] = ((LIMIT (b + yBR) >> 3) & 0x1F)
            | ((LIMIT (g + yBR) << 3) & 0xE0);
        rgb[3] = ((LIMIT (g + yBR) >> 5) & 0x07)
            | (LIMIT (r + yBR) & 0xF8);
    }
}

