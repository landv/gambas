/***************************************************************************

  gbx_struct.c

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

#define __GBX_STRUCT_C

#include "gbx_struct.h"

void *CSTRUCT_create_static(void *ref, CLASS *class, char *addr)
{
	CSTATICSTRUCT *object;
	
  ALLOC(&object, sizeof(CSTATICSTRUCT));

  object->ob.class = class;
  object->ob.ref = 0;
	object->ref = ref;
	object->addr = addr;

  class->count++;

	if (ref != STRUCT_CONST)
		OBJECT_REF(ref);
  
	//fprintf(stderr, "CSTRUCT_create_static: %s %p ref = %p addr = %p\n", class->name, object, ref, addr);
	
	return object;
}

int CSTRUCT_get_size(CLASS *class)
{
	return class->size - sizeof(CSTRUCT);
}

void CSTRUCT_release(CSTRUCT *ob)
{
	if (ob->ref != STRUCT_CONST)
		OBJECT_UNREF(ob->ref);
}


#if 0
BEGIN_PROPERTY(Struct_Size)

	GB_ReturnInteger(CLASS_sizeof(OBJECT_class(THIS)));

END_PROPERTY


BEGIN_METHOD(Struct_Read, GB_OBJECT stream)

	void *stream = VARG(stream);
	
	if (GB.CheckObject(stream))
		return;

END_METHOD


BEGIN_METHOD(Struct_write, GB_OBJECT stream)

	void *stream = VARG(stream);
	VALUE temp;
	
	if (GB.CheckObject(stream))
		return;

	temp._object.class = OBJECT_class(THIS);
	temp._object.class = THIS;
	STREAM_write_type(CSTREAM_stream(stream), T_OBJECT, &temp, 0);

END_METHOD

// Beware: if this declaration is modified, the CSTRUCT_NDESC constant must be modified accordingly.

GB_DESC CSTRUCT_desc[CSTRUCT_NDESC] =
{
	GB_PROPERTY_READ("Size", "i", Struct_Size),
	GB_METHOD("Read", NULL, Struct_Read, "(Stream)Stream;"),
	GB_METHOD("Write", NULL, Struct_Write, "(Stream)Stream;"),
};
#endif
