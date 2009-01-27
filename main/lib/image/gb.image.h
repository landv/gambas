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

#define GB_IMAGE_FMT_IS_24_BITS(_format) ((_format) & 4)
#define GB_IMAGE_FMT_IS_32_BITS(_format) (((_format) & 4)) == 0)

#define GB_IMAGE_FMT_IS_RGBA(_format) ((_format) & 2)

#define GB_IMAGE_FMT_IS_SWAPPED(_format) ((_format) & 1)

#define GB_IMAGE_FMT_IS_TRANSPARENT(_format) ((_format) & 8)

#define GB_IMAGE_FMT_IS_PREMULTIPLIED(_format) ((_format) & 16)
#define GB_IMAGE_FMT_SET_PREMULTIPLIED(_format) ((_format) | 16)
#define GB_IMAGE_FMT_CLEAR_PREMULTIPLIED(_format) ((_format) & ~16)

/* Image owner information */

struct GB_IMG;

typedef
	struct {
		const char *name;                                   // owner name (this is the name of the component)
		int format;                                         // preferred format
		void (*free)(struct GB_IMG *img, void *handle);     // free owner handle
		void (*release)(struct GB_IMG *img, void *handle);  // free temporary handle
		void *(*temp)(struct GB_IMG *img);                  // create a temporary handle for an image and returns it
		//void (*lock)(void *handle); // lock, before accessing pixels
		//void (*unlock)(void *handle); // unlock, after accessing pixels
		}
	GB_IMG_OWNER;
	
/* Gambas image */

typedef
	struct GB_IMG {
		GB_BASE ob;
		unsigned char *data;                      // points at the image data
		int width;                                // image width in pixels
		int height;                               // image height in pixels
		int format;                               // image format (RGB, BGR, RGBA...)
		GB_IMG_OWNER *owner;                      // owner of the data, NULL means gb.image
		void *owner_handle;                       // handle for the owner
		GB_IMG_OWNER *temp_owner;                 // owner of the temporary handle that does not own the data
		void *temp_handle;                        // temporary handle
		}
	GB_IMG;

#ifndef __GB_IMAGE_DEFINED
#define __GB_IMAGE_DEFINED
typedef
	void *GB_IMAGE;
#endif
	
/* Pixel color: the color is not premultiplied, and the alpha component is inverted (0 = solid / 255 = transparent) */

typedef
	unsigned int GB_COLOR;

/* Gambas image component interface */

#define IMAGE_INTERFACE_VERSION 1

typedef
	struct {
		intptr_t version;
		// Create an image
		GB_IMG *(*Create)(int width, int height, int format, unsigned char *data);
		// Take image ownership by giving the image handle and information
		void (*Take)(GB_IMG *img, GB_IMG_OWNER *owner, void *owner_handle, int width, int height, unsigned char *data);
		// Create a temporary handle on the image without becoming the owner.
		void *(*Check)(GB_IMG *img, GB_IMG_OWNER *temp_owner);
		// Return the size of the image data in bytes
		int (*Size)(GB_IMG *img);
		// Set the default format used when creating images
		void (*SetDefaultFormat)(int format);
		// Get the color of a pixel
		GB_COLOR (*GetPixel)(GB_IMG *img, int x, int y);
		// Converts an image to one of the following formats: BGRA, RGBA, BGRP, RGBP
		void (*Convert)(GB_IMG *img, int format);
		}
	IMAGE_INTERFACE;

#define GB_IMG_HANDLE(_image) ((_image)->temp_handle)

#define COLOR_DEFAULT (-1)

#endif

