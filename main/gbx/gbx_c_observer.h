/***************************************************************************

  gbx_c_observer.h

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

#ifndef __GBX_C_OBSERVER_H
#define __GBX_C_OBSERVER_H

#include "gambas.h"
#include "gbx_object.h"
#include "gb_list.h"

#ifndef __GBX_C_OBSERVER_C
extern GB_DESC NATIVE_Observer[];
//extern GB_DESC NATIVE_Proxy[];
#else
#define THIS ((COBSERVER *)_object)
#endif

// Note: the interpreter automatically allocates an extra OBJECT_EVENT structure. See CLASS_calc_info().

typedef
	struct {
		OBJECT ob;
		LIST list;
		ushort *event;
		void *object;
		GB_VARIANT_VALUE tag;
		unsigned after : 1;
		unsigned locked : 1;
	}
	COBSERVER;

void COBSERVER_attach(COBSERVER *this, void *parent, const char *name);
void COBSERVER_detach(COBSERVER *this);

#define COBSERVER_lock(_this, _lock) (((COBSERVER *)_this)->locked = (_lock))
#define COBSERVER_is_locked(_this) (((COBSERVER *)_this)->locked)

#endif
