/***************************************************************************

  c_mediafilter.c

  gb.media component

  (c) 2012 Benoît Minisini <gambas@users.sourceforge.net>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 1, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
  MA 02110-1301, USA.

***************************************************************************/

#define __C_MEDIAFILTER_C

#include "c_mediafilter.h"

// Just there for the documentation wiki!

BEGIN_METHOD_VOID(MediaFilter_new)

END_METHOD

//-------------------------------------------------------------------------


GB_DESC MediaFilterDesc[] = 
{
	GB_DECLARE("MediaFilter", sizeof(CMEDIAFILTER)),
	GB_INHERITS("MediaControl"),
	
	GB_METHOD("_new", NULL, MediaFilter_new, NULL),

	GB_END_DECLARE
};
