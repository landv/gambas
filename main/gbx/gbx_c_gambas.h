/***************************************************************************

  CGambas.h

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

#ifndef __GBX_C_GAMBAS_H
#define __GBX_C_GAMBAS_H

#include "gambas.h"
#include "gbx_object.h"
#include "gb_list.h"

#ifndef __GBX_C_GAMBAS_C
extern GB_DESC NATIVE_Gambas[];
extern GB_DESC NATIVE_Param[];
extern GB_DESC NATIVE_Observer[];
#endif

typedef
	struct {
		OBJECT ob;
		LIST list;
		ushort *event;
		bool after;
	}
	COBSERVER;

#endif
