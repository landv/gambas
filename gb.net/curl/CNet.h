/***************************************************************************

  CNet.h

  (c) 2003-2008 Daniel Campos Fernández <dcamposf@gmail.com>

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

#ifndef __CNET_H
#define __CNET_H

#include "gambas.h"

#ifndef __CNET_C

extern GB_DESC CNetDesc[];

#else

#define THIS ((CNET *)_object)

#endif

enum
{
	NET_INACTIVE = 0,
	NET_ACTIVE = 1,
	NET_RECEIVING_DATA = 4,
	NET_CONNECTING = 6,
};

#endif
