/*
 * c_circular.c - Circular/Ring buffer type
 *
 * Copyright (C) 2012/3 Tobias Boege <tobias@gambas-buch.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#define __C_CIRCULAR_C

#include "gambas.h"
#include "c_circular.h"

typedef struct {
	GB_BASE ob;
	GB_VARIANT_VALUE *elements;
	size_t size; /* Mirror of the array's size to speed things up */
	int reader;
	int writer;
	int overwrite;
	int empty : 1;
	int full  : 1;
} CCIRCULAR;

static int CCIRCULAR_size(CCIRCULAR *circ)
{
	return circ->size;
}

static int CCIRCULAR_is_empty(CCIRCULAR *circ)
{
	return circ->empty;
}

static int CCIRCULAR_is_full(CCIRCULAR *circ)
{
	return circ->full;
}

/*
 * All movements go forward. This means that moving the writer could never
 * induce an 'empty' state and conversely moving the reader could never make
 * the circular 'full'.
 */

static void CCIRCULAR_move_index(CCIRCULAR *circ, int *idx, int pos)
{
	size_t size = CCIRCULAR_size(circ);

	if (!size)
		pos = 0;
	else if (pos >= size)
		pos %= size;
	*idx = pos;
	/* Set empty/full flags */
	if (circ->reader == circ->writer) {
		/* But only if we really operated on the given @circ */
		if (idx == &circ->reader)
			circ->empty = 1;
		else if (idx == &circ->writer)
			circ->full = 1;
	} else {
		circ->empty = circ->full = 0;
	}
}

static void CCIRCULAR_inc_index(CCIRCULAR *circ, int *idx)
{
	CCIRCULAR_move_index(circ, idx, *idx + 1);
}

static void CCIRCULAR_reset(CCIRCULAR *circ)
{
	circ->reader = circ->writer = 0;
	circ->empty = 1;
	if (!circ->size)
		circ->full = 1;
	else
		circ->full = 0;
}

static void CCIRCULAR_init(CCIRCULAR *circ, size_t size, int overwrite)
{
	circ->size = size;
	GB.NewArray(&circ->elements, sizeof(GB_VARIANT_VALUE), circ->size);
	CCIRCULAR_reset(circ);
	circ->overwrite = overwrite;
}

static void CCIRCULAR_destroy(CCIRCULAR *circ)
{
	GB.FreeArray((void *) &circ->elements);
}

#define THIS	((CCIRCULAR *) _object)

BEGIN_METHOD(Circular_new, GB_INTEGER size; GB_BOOLEAN overwrite)

	CCIRCULAR_init(THIS, VARG(size), VARGOPT(overwrite, 1));

END_METHOD

static GB_VARIANT_VALUE *CCIRCULAR_read(CCIRCULAR *circ)
{
	GB_VARIANT_VALUE *var;

	if (CCIRCULAR_is_empty(circ))
		return NULL;
	var = &circ->elements[circ->reader];
	CCIRCULAR_inc_index(circ, &circ->reader);
	return var;
}

static void CCIRCULAR_read_and_free_all(CCIRCULAR *circ)
{
	int i;

	for (i = 0; i < circ->size; i++)
		GB.StoreVariant(NULL, &circ->elements[i]);
	CCIRCULAR_reset(circ);
}

BEGIN_METHOD_VOID(Circular_free)

	CCIRCULAR_read_and_free_all(THIS);
	CCIRCULAR_destroy(THIS);

END_METHOD

static void CCIRCULAR_write(CCIRCULAR *circ, GB_VARIANT *variant)
{
	if (CCIRCULAR_is_full(circ)) {
		if (circ->overwrite) /* Consume oldest value and continue */
			CCIRCULAR_read(circ);
		else /* Do nothing */
			return;
	}
	GB.StoreVariant(variant, &circ->elements[circ->writer]);
	CCIRCULAR_inc_index(circ, &circ->writer);
}

BEGIN_METHOD(Circular_Write, GB_VARIANT value)

	CCIRCULAR_write(THIS, ARG(value));

END_METHOD

BEGIN_METHOD_VOID(Circular_Read)

	if (CCIRCULAR_is_empty(THIS)) {
		GB.ReturnNull();
		GB.ReturnConvVariant();
		return;
	}
	GB.ReturnVariant(CCIRCULAR_read(THIS));

END_METHOD

BEGIN_METHOD_VOID(Circular_Peek)

	if (CCIRCULAR_is_empty(THIS)) {
		GB.ReturnNull();
		GB.ReturnConvVariant();
		return;
	}
	GB.ReturnVariant(&THIS->elements[THIS->reader]);

END_METHOD

static void CCIRCULAR_resize(CCIRCULAR *circ, size_t new)
{
	size_t old = CCIRCULAR_size(circ);

	if (old == new)
		return;
	if (old < new) {
		GB_VARIANT_VALUE *buf;
		int i;

		buf = GB.Insert(&circ->elements, old, new - old);
		for (i = 0; old < new; old++, i++)
			buf[i].type = GB_T_NULL;
	} else {
		int i;

		for (i = new; i < old; i++)
			GB.StoreVariant(NULL, &circ->elements[i]);
		GB.Remove(&circ->elements, new, old - new);
		/* Move the indices accordingly */
		if (circ->reader > new)
			circ->reader = new;
		if (circ->writer > new)
			circ->writer = new;
		/* 0-length circular? */
		if (!new) {
			circ->empty = circ->full = 1;
		}
	}
	circ->size = new;
}

BEGIN_METHOD(Circular_Resize, GB_INTEGER size)

	CCIRCULAR_resize(THIS, VARG(size));

END_METHOD

BEGIN_METHOD_VOID(Circular_Clear)

	CCIRCULAR_read_and_free_all(THIS);

END_METHOD

BEGIN_METHOD_VOID(Circular_Reset)

	CCIRCULAR_reset(THIS);

END_METHOD

BEGIN_PROPERTY(Circular_IsEmpty)

	GB.ReturnBoolean(CCIRCULAR_is_empty(THIS));

END_PROPERTY

BEGIN_PROPERTY(Circular_IsFull)

	GB.ReturnBoolean(CCIRCULAR_is_full(THIS));

END_PROPERTY

BEGIN_PROPERTY(Circular_Reader)

	if (READ_PROPERTY) {
		GB.ReturnInteger(THIS->reader);
		return;
	}
	CCIRCULAR_move_index(THIS, &THIS->reader, VPROP(GB_INTEGER));

END_PROPERTY

BEGIN_PROPERTY(Circular_Writer)

	if (READ_PROPERTY) {
		GB.ReturnInteger(THIS->writer);
		return;
	}
	CCIRCULAR_move_index(THIS, &THIS->writer, VPROP(GB_INTEGER));

END_PROPERTY

BEGIN_PROPERTY(Circular_Size)

	if (READ_PROPERTY) {
		GB.ReturnInteger(CCIRCULAR_size(THIS));
		return;
	}
	CCIRCULAR_resize(THIS, VPROP(GB_INTEGER));

END_PROPERTY

BEGIN_PROPERTY(Circular_Overwrite)

	if (READ_PROPERTY) {
		GB.ReturnBoolean(THIS->overwrite);
		return;
	}
	THIS->overwrite = VPROP(GB_BOOLEAN);

END_PROPERTY

GB_DESC CCircular[] = {
	GB_DECLARE("Circular", sizeof(CCIRCULAR)),

	GB_METHOD("_new", NULL, Circular_new, "(Size)i[(Overwrite)b]"),
	GB_METHOD("_free", NULL, Circular_free, NULL),

	GB_METHOD("Write", NULL, Circular_Write, "(Value)v"),
	GB_METHOD("Read", "v", Circular_Read, NULL),
	GB_METHOD("Peek", "v", Circular_Peek, NULL),

	GB_METHOD("Resize", NULL, Circular_Resize, "(Size)i"),
	GB_METHOD("Clear", NULL, Circular_Clear, NULL),
	GB_METHOD("Reset", NULL, Circular_Reset, NULL),

	GB_PROPERTY_READ("IsEmpty", "b", Circular_IsEmpty),
	GB_PROPERTY_READ("IsFull", "b", Circular_IsFull),
	GB_PROPERTY("Reader", "i", Circular_Reader),
	GB_PROPERTY("Writer", "i", Circular_Writer),
	GB_PROPERTY("Size", "i", Circular_Size),
	GB_PROPERTY("Overwrite", "b", Circular_Overwrite),

	GB_END_DECLARE
};
