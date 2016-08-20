/***************************************************************************

  image_stat.h

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

#ifndef __IMAGE_STAT_H
#define __IMAGE_STAT_H

#include "main.h"

typedef
	struct {
		char *addr;
		int len;
		int pos;
		}
	IMAGE_STREAM;

typedef
	enum
	{
		IMAGE_TYPE_ERROR = -1,
		IMAGE_TYPE_UNKNOWN = 0,
		IMAGE_TYPE_GIF = 1,
		IMAGE_TYPE_JPEG,
		IMAGE_TYPE_PNG,
		IMAGE_TYPE_BMP,
		IMAGE_TYPE_TIFF_INTEL,
		IMAGE_TYPE_TIFF_MOTOROLA
	} 
	IMAGE_FILETYPE;

typedef
	struct {
		char *type;
		unsigned int width;
		unsigned int height;
		unsigned int depth;
	}
	IMAGE_INFO;

#ifndef __IMAGE_STAT_C
extern const char *IMAGE_error;
#endif

bool IMAGE_get_info(IMAGE_STREAM *stream, IMAGE_INFO *info);

#endif