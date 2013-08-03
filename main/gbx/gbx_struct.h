/***************************************************************************

  gbx_struct.h

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

#ifndef __GBX_STRUCT_H
#define __GBX_STRUCT_H

#include "gbx_object.h"
#include "gbx_value.h"

#define STRUCT_CONST ((void *)-1)

typedef
	struct {
		OBJECT ob;
		void *ref;
	}
	CSTRUCT;
	
typedef
	struct {
		OBJECT ob;
		void *ref;
		char *addr;
	}
	CSTATICSTRUCT;
	
void *CSTRUCT_create_static(void *ref, CLASS *class, char *addr);
int CSTRUCT_get_size(CLASS *class);
void CSTRUCT_release(CSTRUCT *ob);

#endif
