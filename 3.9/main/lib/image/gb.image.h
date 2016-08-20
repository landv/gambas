/***************************************************************************

  gb.image.h

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

#ifndef __GB_IMAGE_H
#define __GB_IMAGE_H

#include "gambas.h"

// Constants used by image data format

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

// Format test functions

#define GB_IMAGE_FMT_IS_24_BITS(_format) ((_format) & 4)
#define GB_IMAGE_FMT_IS_32_BITS(_format) (((_format) & 4) == 0)

#define GB_IMAGE_FMT_IS_RGBA(_format) ((_format) & 2)

#define GB_IMAGE_FMT_IS_SWAPPED(_format) ((_format) & 1)

#define GB_IMAGE_FMT_IS_TRANSPARENT(_format) ((_format) & 8)

#define GB_IMAGE_FMT_IS_PREMULTIPLIED(_format) ((_format) & 16)
#define GB_IMAGE_FMT_SET_PREMULTIPLIED(_format) ((_format) | 16)
#define GB_IMAGE_FMT_CLEAR_PREMULTIPLIED(_format) ((_format) & ~16)

// Image owner information

struct GB_IMG;

typedef
	struct {
		const char *name;                                   // owner name (this is the name of the component)
		int format;                                         // preferred format
		void (*free)(struct GB_IMG *img, void *handle);     // free owner handle
		void (*release)(struct GB_IMG *img, void *handle);  // free temporary handle
		void *(*temp)(struct GB_IMG *img);                  // create a temporary handle for an image and returns it
		void (*sync)(struct GB_IMG *img);                   // synchronize the data. Called only if the GB_IMG.sync flag is set
		}
	GB_IMG_OWNER;
	
// Gambas image

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
		unsigned modified : 1;                    // data has been modified by gb.image
		unsigned sync : 1;                        // data must be synchronized by calling GB_IMG_OWNER.sync()
		unsigned is_void : 1;                     // void image (no data)
		}
	GB_IMG;

#ifndef __GB_IMAGE_DEFINED
#define __GB_IMAGE_DEFINED
typedef
	void *GB_IMAGE;
#endif
	
// Pixel color: the color is not premultiplied, and the alpha component is inverted (0 = solid / 255 = transparent)

#ifndef __GB_COLOR_DEFINED
#define __GB_COLOR_DEFINED
typedef
	unsigned int GB_COLOR;
#endif

// Split a color into its component. Uninvert the alpha component

#define GB_COLOR_SPLIT(_color, _r, _g, _b, _a) \
({ \
	uint _c = (uint)(_color); \
	_b = _c & 0xFF; \
	_g = (_c >> 8) & 0xFF; \
	_r = (_c >> 16) & 0xFF; \
	_a = (_c >> 24) ^ 0xFF; \
})

// Create a GB_COLOR from rgba components

#define GB_COLOR_MAKE(_r, _g, _b, _a) (((_b) & 0xFF) | (((_g) & 0xFF) << 8) | (((_r) & 0xFF) << 16) | ((((_a) & 0xFF) ^ 0xFF) << 24))

// Gambas image component interface

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
		// Synchronize the image data if needed
		void (*Synchronize)(GB_IMG *img);
		// Return the size of the image data in bytes
		int (*Size)(GB_IMG *img);
		// Set the default format used when creating images
		void (*SetDefaultFormat)(int format);
		// Get the default format used when creating images
		int (*GetDefaultFormat)(void);
		// Get the color of a pixel
		GB_COLOR (*GetPixel)(GB_IMG *img, int x, int y);
		// Converts an image to one of the following formats: BGRA, RGBA, BGRP, RGBP
		void (*Convert)(GB_IMG *img, int format);
		// Merge two colors
		GB_COLOR (*MergeColor)(GB_COLOR col1, GB_COLOR col2, double weight);
		// Make a color lighter
		GB_COLOR (*LighterColor)(GB_COLOR col);
		// Make a color darker
		GB_COLOR (*DarkerColor)(GB_COLOR col);
		// Return the image format as a string
		const char *(*FormatToString)(int format);
		}
	IMAGE_INTERFACE;

#define GB_IMG_HANDLE(_image) ((_image)->temp_handle)

#define SYNCHRONIZE_IMAGE(_image) (IMAGE.Synchronize(_image))
#define MODIFY_IMAGE(_image) ((_image)->modified = 1)

#define COLOR_DEFAULT ((GB_COLOR)-1)
#define GB_COLOR_DEFAULT ((GB_COLOR)-1)

#endif

