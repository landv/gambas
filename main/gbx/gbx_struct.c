/***************************************************************************

  gbx_struct.c

  (c) 2000-2009 Benoît Minisini <gambas@users.sourceforge.net>

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

#define __GBX_STRUCT_C

#include "gbx_struct.h"

void *CSTRUCT_create_static(void *ref, CLASS *class, char *addr)
{
	CSTATICSTRUCT *object;
	
  ALLOC(&object, sizeof(CSTATICSTRUCT), "OBJECT_alloc");

  object->ob.class = class;
  object->ob.ref = 0;
	object->ref = ref;
	object->addr = addr;

  class->count++;

	OBJECT_REF(ref, "CSTRUCT_create_static");
  
	//fprintf(stderr, "CSTRUCT_create_static: %s %p ref = %p addr = %p\n", class->name, object, ref, addr);
	
	return object;
}

int CSTRUCT_get_size(CLASS *class)
{
	return class->size - sizeof(CSTRUCT);
}

void CSTRUCT_release(CSTRUCT *ob)
{
	OBJECT_UNREF(ob->ref, "CSTRUCT_release");
}
