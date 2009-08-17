/***************************************************************************

  Cfont.h

  (c) 2007 Laurent Carlier <lordheavy@users.sourceforge.net>

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
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

***************************************************************************/

#ifndef __CFONT_H
#define __CFONT_H

#include "SDLfont.h"

#include "gambas.h"

#ifndef __CFONT_CPP
//extern GB_DESC CFont[];
extern GB_DESC CFonts[];

#else

//#define THIS OBJECT(CFONT)

#endif

typedef
	struct {
		GB_BASE ob;
		// QString *family;
	}
	CFONTINFO;

#endif
