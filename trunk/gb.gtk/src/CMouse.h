/***************************************************************************

  CMouse.h

  (c) 2004-2006 - Daniel Campos Fernández <dcamposf@gmail.com>

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

#ifndef __CMOUSE_H
#define __CMOUSE_H

#include "main.h"
#include "CPicture.h"
#include "gmouse.h"

#ifndef __CMOUSE_CPP

extern GB_DESC CMouseDesc[];
extern GB_DESC CCursorDesc[];
extern GB_DESC CPointerDesc[];

#else

#define THIS ((CCURSOR *)_object)

#endif

typedef
	struct 
	{
		GB_BASE ob;
		gCursor *cur;
	}
	CCURSOR;

#define MOUSE_CONSTANTS \
  "<Mouse,Default,Blank,Arrow,Cross,Wait,Text,SizeAll,SizeH,SizeV,SizeN,SizeS,SizeW,SizeE,SizeNWSE," \
  "SizeNESW,SplitH,SplitV,Pointing>"
#endif
