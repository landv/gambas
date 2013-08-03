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
		IMAGE_FILETYPE_ERROR = -1,
		IMAGE_FILETYPE_UNKNOWN = 0,
		IMAGE_FILETYPE_GIF = 1,
		IMAGE_FILETYPE_JPEG,
		IMAGE_FILETYPE_PNG,
		/*IMAGE_FILETYPE_SWF,
		IMAGE_FILETYPE_PSD,*/
		IMAGE_FILETYPE_BMP,
		IMAGE_FILETYPE_TIFF_II, /* intel */
		IMAGE_FILETYPE_TIFF_MM /* motorola */
		/*IMAGE_FILETYPE_JPC,
		IMAGE_FILETYPE_JP2,
		IMAGE_FILETYPE_JPX,
		IMAGE_FILETYPE_JB2,
		IMAGE_FILETYPE_SWC,
		IMAGE_FILETYPE_IFF,
		IMAGE_FILETYPE_WBMP,*/
		/* IMAGE_FILETYPE_JPEG2000 is a userland alias for IMAGE_FILETYPE_JPC */
		/*IMAGE_FILETYPE_XBM*/
	/* WHEN EXTENDING: PLEASE ALSO REGISTER IN image.c:PHP_MINIT_FUNCTION(imagetypes) */
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

int IMAGE_get_info(IMAGE_STREAM *stream, IMAGE_INFO *info);

#endif