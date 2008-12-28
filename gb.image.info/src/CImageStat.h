/***************************************************************************

  CImageStat.h

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

#ifndef __CIMAGESTAT_H
#define __CIMAGESTAT_H

#include "gambas.h"

#ifndef __CIMAGESTAT_C

extern GB_DESC CImageStatDesc[];

#else

#define THIS ((CIMAGESTAT *)_object)

#endif

typedef
	struct {
		GB_BASE ob;
		char *path;
		char *type;
		int width;
		int height;
		int depth;
		}
	CIMAGESTAT;

#endif
 
