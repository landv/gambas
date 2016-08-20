/***************************************************************************

  c_clipper.h

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

#ifndef __C_CLIPPER_H
#define __C_CLIPPER_H

#include "gambas.h"
#include "clipper.hpp"

using namespace ClipperLib;

#ifndef __C_CLIPPER_CPP

extern GB_DESC PolygonDesc[];
extern GB_DESC ClipperDesc[];

#endif

typedef
	struct {
		GB_BASE ob;
		Path *poly;
		//bool closed;
	}
	CPOLYGON;


#endif
