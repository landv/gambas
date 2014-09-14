/*
 * c_heap.c - (Min-/Max-)Heap and PrioSet
 *
 * Copyright (C) 2014 Tobias Boege <tobias@gambas-buch.de>
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

#define __C_HEAP_C

#include "gambas.h"
#include "gb_common.h" /* EXTERN for gbx_compare.h */
#include "gbx_compare.h" /* GB_COMP_{ASCENT,DESCENT} */
#include "gbx_type.h" /* TYPE_is_object() */

#include "c_heap.h"

typedef struct {
	GB_BASE ob;
	int mode;
	int count;
	GB_VARIANT_VALUE *h;
} CHEAP; /* "Hey! Don't talk like this about my data structures!" */

static inline int compare(CHEAP *heap, int i, int j)
{
	int res = GB.CompVariant(&heap->h[i], &heap->h[j]);

	return heap->mode == GB_COMP_ASCENT ? res : -res;
}

static inline int compare1(CHEAP *heap, GB_VARIANT_VALUE *x, int j)
{
	int res = GB.CompVariant(x, &heap->h[j]);

	return heap->mode == GB_COMP_ASCENT ? res : -res;
}

static inline int compare3(CHEAP *heap, GB_VARIANT_VALUE *x,
			   GB_VARIANT_VALUE *y)
{
	int res = GB.CompVariant(x, y);

	return heap->mode == GB_COMP_ASCENT ? res : -res;
}

static inline void copy(CHEAP *heap, int src, int dst)
{
	memmove(&heap->h[dst], &heap->h[src], sizeof(GB_VARIANT_VALUE));
}

static inline void copy1(CHEAP *heap, int src, GB_VARIANT_VALUE *dst)
{
	memmove(dst, &heap->h[src], sizeof(GB_VARIANT_VALUE));
}

static inline void copy2(CHEAP *heap, GB_VARIANT_VALUE *src, int dst)
{
	memmove(&heap->h[dst], src, sizeof(GB_VARIANT_VALUE));
}

#define left(k)		(2 * (k) + 1)
#define right(k)	(left(k) + 1)
#define parent(k)	(((k) - 1) / 2)

static int upheap(CHEAP *heap, int k)
{
	GB_VARIANT_VALUE x;
	int r = 0;

	copy1(heap, k, &x);
	while (k && compare1(heap, &x, parent(k)) < 0) {
		copy(heap, parent(k), k);
		k = parent(k);
		r++;
	}
	copy2(heap, &x, k);
	return r;
}

static int downheap(CHEAP *heap, int k)
{
	int count = GB.Count(heap->h), r = 0;
	GB_VARIANT_VALUE x;

	copy1(heap, k, &x);
	while (k <= parent(count - 1)) {
		int j, l = j = left(k), r = right(k);

		if (r < count && compare(heap, l, r) > 0)
			j = r;
		if (compare1(heap, &x, j) <= 0)
			break;
		copy(heap, j, k);
		k = j;
		r++;
	}
	copy2(heap, &x, k);
	return r;
}

static void new_heap(CHEAP *heap, int count)
{
	GB.NewArray(&heap->h, sizeof(*heap->h), count);
}

static void rebuild(CHEAP *heap)
{
	int i;

	for (i = parent(GB.Count(heap->h) - 1); i >= 0; i--)
		downheap(heap, i);
}

static void from_array(CHEAP *heap, GB_ARRAY array)
{
	int count = GB.Array.Count(array), i;

	new_heap(heap, count);
	for (i = 0; i < count; i++) {
		memcpy(&heap->h[i], GB.Array.Get(array, i),
		       sizeof(*heap->h));
		if (TYPE_is_object(heap->h[i].type))
			GB.Ref(heap->h[i].value._object);
	}
	rebuild(heap);
}

#define THIS	((CHEAP *) _object)

/**G
 * Creates a new Heap.
 *
 * If 'Mode' is gb.Ascent, it's a MinHeap, i.e. the smallest element is at
 * the beginning. If 'mode' is gb.Descent, it's a MaxHeap.
 *
 * If the 'Array' argument is given, a copy of that array is transformed
 * into a Heap (by using a bottom-up algorithm which is O(n)).
 */
BEGIN_METHOD(Heap_new, GB_INTEGER mode; GB_OBJECT array)

	THIS->mode = VARG(mode);
	if (THIS->mode != GB_COMP_ASCENT && THIS->mode != GB_COMP_DESCENT) {
		GB.Error("Invalid mode");
		return;
	}

	if (MISSING(array)) {
		new_heap(THIS, 0);
	} else {
		GB_ARRAY array = (GB_ARRAY) VARG(array);

		if (GB.CheckObject(array))
			return;
		from_array(THIS, array);
	}

END_METHOD

/**G
 * Free up storage of the Heap
 */
BEGIN_METHOD_VOID(Heap_free)

	int count = GB.Count(THIS->h), i;

	for (i = 0; i < count; i++)
		GB.StoreVariant(NULL, &THIS->h[i]);
	GB.FreeArray(&THIS->h);

END_METHOD

/**G
 * Insert an element into the Heap.
 */
BEGIN_METHOD(Heap_Insert, GB_VARIANT data)

	GB.StoreVariant(ARG(data), GB.Add(&THIS->h));
	upheap(THIS, GB.Count(THIS->h) - 1);

END_METHOD

static void delete(CHEAP *heap, int i, GB_VARIANT_VALUE *x)
{
	int count = GB.Count(heap->h);

	copy1(heap, i, x);
	copy(heap, count - 1, i);
	GB.Remove(&heap->h, count - 1, 1);
	downheap(heap, i);
}

/**G
 * Remove the first element.
 */
BEGIN_METHOD_VOID(Heap_Remove)

	int count = GB.Count(THIS->h);
	GB_VARIANT_VALUE x;

	if (!count) {
		GB.Error(GB_ERR_BOUND);
		return;
	}

	delete(THIS, 0, &x);
	GB.ReturnVariant(&x);
	GB.ReturnBorrow();
	GB.StoreVariant(NULL, &x);
	GB.ReturnRelease();

END_METHOD

/*
 * Comparison by identity works as follows:
 *  - if both are objects and have _identity() methods which return
 *    variants, the return value is the comparison of those variants,
 *  - if they are objects without _identity(), they are compared based
 *    on their addresses in memory,
 *  - else they are not objects and are normally compared.
 */
static int compare_identity(CHEAP *heap, GB_VARIANT_VALUE *a,
			    GB_VARIANT_VALUE *b)
{
	static int has_identity;
	static GB_FUNCTION aid;
	static GB_VARIANT_VALUE *last = NULL;
	static GB_VALUE x;

	GB_FUNCTION bid;
	GB_VALUE *y;

	if (!TYPE_is_object(a->type) || !TYPE_is_object(b->type))
		return compare3(heap, a, b);

	if (last != a) {
		if (!GB.GetFunction(&aid, a->value._object, "_identity",
				    NULL, "v")) {
			has_identity = 1;
			memcpy(&x, GB.Call(&aid, 0, 0), sizeof(x));
		} else {
			has_identity = 0;
		}
		GB.Error(NULL); /* Clear any GetFunction error */
		last = a;
	}

	/* No _identity()? */
	if (!has_identity || GB.GetFunction(&bid, b->value._object,
					    "_identity", NULL, "v")) {
		GB.Error(NULL);
		return a->value._object != b->value._object;
	}

	y = GB.Call(&bid, 0, 0);
	return compare3(heap, &x._variant.value, &y->_variant.value);
}

/**G
 * Find all occurences of `Old' and replace them by `New'. This is an O(n)
 * operation. Additionally the heap has to be rebuilt as soon as there is
 * more than one replacement made.
 *
 * If `New' is Null, then the entry will be deleted.
 *
 * The search for objects is done by identity and *not* by using the
 * _compare() method. "By identity" means that if two objects are to be
 * compared and both have a special _identity() method returning a Variant,
 * the return values of these methods are compared.
 *
 * If one of the objects does not implement the _identity() method, the
 * default is comparison by object addresses in memory. This strategy lets
 * you save primitive data types (Integer, Boolean, String), etc. in the
 * heap. But you can also *distinct* objects in equivalence classes. The
 * _compare() method defines your equivalence relation, the identity
 * comparison enables to discern different objects.
 *
 * If we are a heap of objects whose _compare() methods compare some sort of
 * priority, and `Old' is the same object as `New', this can be used to
 * propagate a priority change made to the object and will correct its
 * position in the heap.
 *
 * This has an application in Dijkstra's algorithm (or any priority first
 * search like Prim or A*) where you would update a node's priority.
 */
BEGIN_METHOD(Heap_Update, GB_VARIANT old; GB_VARIANT new)

	int count = GB.Count(THIS->h), i, found = 0, idx = -1;
	GB_VARIANT_VALUE *old, *new;

	old = &VARG(old); new = &VARG(new);
	for (i = 0; i < count; i++) {
		if (compare_identity(THIS, old, &THIS->h[i]))
			continue;
		/*
		 * Make Null delete the entry. We don't count that as a
		 * finding because we can fix the heap up on the fly.
		 */
		if (new->type == GB_T_NULL) {
			GB_VARIANT_VALUE buf;

			delete(THIS, i, &buf);
			GB.StoreVariant(NULL, &buf);
			count = GB.Count(THIS->h);
			continue;
		}
		/* XXX: If `old' and `new' are the same and if they're
		 * objects, memory errors will occur if you store one over
		 * the other... Maybe StoreVariant() wasn't made to replace
		 * an object by itself (refcount goes to zero before it is
		 * incremented again, maybe?) */
		if (!TYPE_is_object(THIS->h[i].type)
		 || THIS->h[i].value._object != new->value._object)
			GB.StoreVariant(ARG(new), &THIS->h[i]);
		found++;
		idx = i;
	}
	/*
	 * Most applications will have pairwise distinct elements in the
	 * heap, so that at most one element is changed. In this case, we
	 * can get away more quickly (O(log n) vs. O(n)) with an upheap()
	 * or downheap() call.
	 */
	if (found == 1) {
		if (!upheap(THIS, idx))
			downheap(THIS, idx);
	} else if (found) {
		rebuild(THIS);
	}

END_METHOD

/**G
 * Return or set the first element of the Heap.
 */
BEGIN_PROPERTY(Heap_First)

	if (!GB.Count(THIS->h)) {
		GB.Error(GB_ERR_BOUND);
		return;
	}

	if (READ_PROPERTY) {
		GB.ReturnVariant(&THIS->h[0]);
		return;
	}
	GB.StoreVariant(PROP(GB_VARIANT), &THIS->h[0]);
	downheap(THIS, 0);

END_PROPERTY

/**G
 * Return the number of elements in the Heap.
 */
BEGIN_PROPERTY(Heap_Count)

	GB.ReturnInteger(GB.Count(THIS->h));

END_PROPERTY

/**G
 * Return whether the Heap is empty.
 */
BEGIN_PROPERTY(Heap_IsEmpty)

	GB.ReturnBoolean(GB.Count(THIS->h) == 0);

END_PROPERTY

GB_DESC CHeap[] = {
	/**G Heap
	 * This class implements a dynamic heap. It can be a MinHeap or a
	 * MaxHeap, depending on what mode you specify on construction.
	 *
	 * Being `dynamic' means that this class allows you to update the
	 * data or the position of elements which are already in the heap.
	 */
	GB_DECLARE("Heap", sizeof(CHEAP)),

	GB_METHOD("_new", NULL, Heap_new, "(Mode)i[(Array)Variant[];]"),
	GB_METHOD("_free", NULL, Heap_free, NULL),

	GB_METHOD("Insert", NULL, Heap_Insert, "(Data)v"),
	GB_METHOD("Remove", "v", Heap_Remove, NULL),
	GB_METHOD("Update", NULL, Heap_Update, "(Old)v(New)v"),

	GB_PROPERTY("First", "v", Heap_First),

	GB_PROPERTY_READ("Count", "i", Heap_Count),
	GB_PROPERTY_READ("IsEmpty", "b", Heap_IsEmpty),

	GB_END_DECLARE
};
