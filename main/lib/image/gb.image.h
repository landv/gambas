/***************************************************************************

  gb.image.h

  (c) 2000-2007 Benoit Minisini <gambas@freesurf.fr>

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

#ifndef __GB_IMAGE_H
#define __GB_IMAGE_H

#include "gambas.h"

/* constants used by image data format */

#define GB_IMAGE_BGRX  0       // 00000
#define GB_IMAGE_XRGB  1       // 00001
#define GB_IMAGE_RGBX  2       // 00010
#define GB_IMAGE_XBGR  3       // 00011
#define GB_IMAGE_BGR   4       // 00100
#define GB_IMAGE_RGB   5       // 00101

#define GB_IMAGE_BGRA  8       // 01000
#define GB_IMAGE_ARGB  9       // 01001
#define GB_IMAGE_RGBA  10      // 01010
#define GB_IMAGE_ABGR  11      // 01011

#define GB_IMAGE_BGRP  24      // 11000
#define GB_IMAGE_PRGB  25      // 11001 
#define GB_IMAGE_RGBP  26      // 11010
#define GB_IMAGE_PBGR  27      // 11011

/* format test functions */

#define GB_IMAGE_IS_TRANSPARENT(_format) ((_format) & 8)
#define GB_IMAGE_IS_PREMULTIPLIED(_format) ((_format) & 16)
#define GB_IMAGE_IS_24_BITS(_format) ((_format) & 4)
#define GB_IMAGE_IS_32_BITS(_format) (((_format) & 4)) == 0)

/* Gambas image type */

typedef
	struct {
		const char *name;                         // name of the image type
		void (*free)(void *);                     // free the image
		void (*begin)(void *);                    // starts to modify the image contents
		void (*end)(void *);                      // ends modifying the image contents
		}
	GB_IMAGE_TYPE;

/* Gambas image */

typedef
	struct {
		GB_IMAGE_TYPE *type;                      // image type
		void *param;                              // argument that will be passed to image type functions
		unsigned char *data;                      // points at the image data
		int width;                                // image width in pixels
		int height;                               // image height in pixels
		int stride;                               // number of bytes by lines
		int format;                               // image format (RGB, BGR, RGBA...)
		}
	GB_IMAGE;

/* Gambas image component interface */

typedef
	struct {
		intptr_t version;
		void (*Convert)(GB_IMAGE *src, GB_IMAGE *dst);
		void (*MakeGray)(GB_IMAGE *img);
		void (*MakeTransparent)(GB_IMAGE *img);
		}
	IMAGE_INTERFACE;

#endif

