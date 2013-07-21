/*
 * c_list.c - Circular doubly-linked lists
 *
 * Copyright (C) 2012 Tobias Boege <tobias@gambas-buch.de>
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

#define __C_LIST_C

#include <assert.h>

#include "gambas.h"
#include "gb_common.h"
#include "list.h"
#include "c_list.h"

#define CHUNK_SIZE	16

//#define DEBUG_ENUMERATOR

/*
 * List implementation properties:
 * + Increase cache locality by saving CHUNK_SIZE values inside a single
 *   chunk.
 * + A special algorithm to re-arrange the values in a chunk guarantees some
 *   properties we can use to speed up the critical paths, such as
 *   traversals.
 * - Fragmentation in the middle chunks in a list because of the (O)
 *   postulate of the rearrangement algorithm.
 */

typedef struct {
	LIST list;
	GB_VARIANT_VALUE var[CHUNK_SIZE];
	int first;	/* First valid element in var */
	int last;	/* Last valid element in var */
} CHUNK;
#define get_chunk(node)		LIST_data(node, CHUNK, list)

typedef struct {
	CHUNK *ck;
	int idx;	/* Absolute index into ->ck->var */
} VAL;

typedef struct {
	GB_BASE ob;
	LIST list;	/* Beginning of linked CHUNKs */
	VAL current;	/* Current element */
	size_t count;	/* Do not iterate over all elements to get this */
} CLIST;

static void CHUNK_init(CHUNK *ck)
{
	int i;

	LIST_init(&ck->list);
	for (i = 0; i < CHUNK_SIZE; i++)
		ck->var[i].type = GB_T_NULL;
	ck->first = -1;
	ck->last = -1;
}

static CHUNK *CHUNK_new(void)
{
	CHUNK *new;

	GB.Alloc((void **) &new, sizeof(*new));
	CHUNK_init(new);
	return new;
}

static inline int CHUNK_count(CHUNK *ck)
{
	return ck->last - ck->first + 1;
}

static inline int CHUNK_is_first(CLIST *list, CHUNK *ck)
{
	return list->list.next == &ck->list;
}

static inline int CHUNK_is_last(CLIST *list, CHUNK *ck)
{
	return list->list.prev == &ck->list;
}

static void CHUNK_free_all(CHUNK *ck)
{
	int i;

	if (ck->first < 0 || ck->last  < 0)
		return;
	for (i = ck->first; i <= ck->last; i++)
		if (ck->var[i].type != GB_T_NULL)
			GB.StoreVariant(NULL, &ck->var[i]);
	ck->first = ck->last = -1;
}

static void CHUNK_destroy(CHUNK *ck)
{
	/* The chunk *must* be unlinked */
	CHUNK_free_all(ck);
	GB.Free((void **) &ck);
}

#define THIS	((CLIST *) _object)

BEGIN_METHOD_VOID(List_new)

	LIST_init(&THIS->list);
	THIS->current.ck = NULL;
	THIS->count = 0;

END_METHOD

BEGIN_METHOD_VOID(List_free)

	LIST *node, *next;
	CHUNK *ck;

	list_for_each_safe(node, &THIS->list, next) {
		LIST_unlink(node);
		ck = get_chunk(node);
		CHUNK_destroy(ck);
	}
	THIS->current.ck = NULL;
	THIS->count = 0;

END_METHOD

static inline GB_VARIANT_VALUE *VAL_value(VAL *val)
{
#ifdef DEBUG_ME
	if (val->idx < val->ck->first || val->idx > val->ck->last)
		printf(": err: %d : %d,%d\n", val->idx, val->ck->first,
					      val->ck->last);
#endif
	assert(val->idx >= val->ck->first && val->idx <= val->ck->last);
	return &val->ck->var[val->idx];
}

static inline int VAL_is_equal(VAL *v1, VAL *v2)
{
	return v1->ck == v2->ck && v1->idx == v2->idx;
}

static void CLIST_first(CLIST *list, VAL *buf)
{
	if (!list->count) {
		buf->ck = NULL;
		return;
	}
	buf->ck = get_chunk(list->list.next);
	buf->idx = buf->ck->first;
}

static void CLIST_last(CLIST *list, VAL *buf)
{
	if (!list->count) {
		buf->ck = NULL;
		return;
	}
	buf->ck = get_chunk(list->list.prev);
	buf->idx = buf->ck->last;
}

/*
 * Modify 'val' so that it points to the next/prev valid value. 'first' is
 * used to detect the end of an enumeration, i.e. where no other elements
 * should be looked for. In this case, NULL is written to val->ck.
 */

static void CHUNK_next(CLIST *list, VAL *val)
{
	LIST *node;

	/* Try to just update the index. This approach comes from the
	 * rearrange algorithm. */
	if (val->idx < val->ck->last) {
		val->idx++;
		return;
	}

	/* Go to next chunk */
	if ((node = val->ck->list.next) == &list->list)
		node = node->next;
	val->ck = get_chunk(node);
	val->idx = val->ck->first;
}

static void CHUNK_next_enum(CLIST *list, VAL *first, VAL *val)
{
	CHUNK *ck = val->ck;
	LIST *node;

	assert(first != val);
	if (val->idx < ck->last) {
		val->idx++;
		if (VAL_is_equal(first, val))
			goto no_next;
		return;
	}
	if ((node = ck->list.next) == &list->list)
		node = node->next;
	ck = get_chunk(node);
	val->ck = ck;
	val->idx = ck->first;
	if (VAL_is_equal(first, val))
		goto no_next;
	return;

no_next:
	val->ck = NULL;
}

static void CHUNK_prev(CLIST *list, VAL *val)
{
	LIST *node;

	if (val->idx > val->ck->first) {
		val->idx--;
		return;
	}
	if ((node = val->ck->list.prev) == &list->list)
		node = node->prev;
	val->ck = get_chunk(node);
	val->idx = val->ck->last;
}

static void CHUNK_prev_enum(CLIST *list, VAL *first, VAL *val)
{
	CHUNK *ck = val->ck;
	LIST *node;

	assert(first != val);
	if (val->idx > ck->first) {
		val->idx--;
		if (VAL_is_equal(first, val))
			goto no_prev;
		return;
	}
	if ((node = ck->list.prev) == &list->list)
		node = node->prev;
	ck = get_chunk(node);
	val->ck = ck;
	val->idx = ck->last;
	if (VAL_is_equal(first, val))
		goto no_prev;
	return;

no_prev:
	val->ck = NULL;
}

/*
 * Random access function.
 * Negative 'idx' makes go backwards. The 'val' is filled. Out of bounds is
 * signalled by val->ck == NULL.
 */

#ifndef sgn
# define sgn(x)	((x) < 0 ? -1 : ((x) ? 1 : 0))
#endif

static void CLIST_get(CLIST *list, int idx, VAL *val)
{
	LIST *node;
	CHUNK *ck;
	int i;
	int dir = sgn(idx);
	int off, count;

	/* We do _not_ allow indices to wrap around the end, like we could:
	 * idx %= list->count. Don't use a loop just to detect that case. */
	i = dir * idx - (dir < 0 ? 1 : 0);
	if (i >= list->count)
		goto out_of_bounds;

	/* Don't traverse forwards/backwards when the absolute value of
	 * index is above the half of the number of elements */
	if (i > (list->count - 1) / 2) {
		dir = -dir;
		i = list->count - (i + 1); /* idx = -(idx + 1) */
	}

	node = &list->list;
	/* Compute the offset to node->next or node->prev depending on the
	 * direction we traverse. This saves an if (dir >= 0) branch
	 * prediction catastrophe in loop. Still, very awful for caches. */
	off = (dir >= 0 ? offsetof(LIST, next) : offsetof(LIST, prev));
	do {
		node = *((LIST **) ((long) node + off));
		assert(node != &list->list);
		ck = get_chunk(node);
		count = CHUNK_count(ck);
		if (i < count) {
			val->ck = ck;
			if (dir < 0)
				val->idx = ck->last - i;
			else
				val->idx = ck->first + i;
			return;
		}
		i -= count;
	} while (node != &list->list);
	__builtin_trap(); /* XXX */

out_of_bounds:
	/* Not enough elements. */
	val->ck = NULL;
}

/* XXX: sizeof(VAL) may never exceed 3*sizeof(intptr_t)! */
struct enum_state {
	CHUNK *first;
	VAL next;
};

BEGIN_METHOD_VOID(List_next)

	struct enum_state *state = GB.GetEnum();
	GB_VARIANT_VALUE *val;
	VAL start; /* XXX: Would like to cache that in the enum_state... */

	if (!state->first) { /* Beginning */
#ifdef DEBUG_ENUMERATOR
		printf("New enumerator %p\n", state);
#endif
		CLIST_first(THIS, &state->next);
		state->first = state->next.ck;
	}
	/* No elements left? */
	if (!state->next.ck) {
#ifdef DEBUG_ENUMERATOR
		/*
		 * Mark this enumerator invalid.
		 */
		state->first = NULL;
		printf("Leaving enumeration %p\n", state);
#endif
		GB.StopEnum();
		return;
	}

	val = VAL_value(&state->next);
	start.ck = state->first;
	start.idx = start.ck->first;
	CHUNK_next_enum(THIS, &start, &state->next);
	GB.ReturnVariant(val);

END_METHOD

/*
 * The same as List_next but backwards.
 */

BEGIN_METHOD_VOID(ListBackwards_next)

	struct enum_state *state = GB.GetEnum();
	GB_VARIANT_VALUE *val;
	VAL start;

	if (!state->first) { /* Beginning */
#ifdef DEBUG_ENUMERATOR
		printf("New enumerator %p\n", state);
#endif
		CLIST_last(THIS, &state->next);
		state->first = state->next.ck;
	}
	/* No elements left? */
	if (!state->next.ck) {
#ifdef DEBUG_ENUMERATOR
		state->first = NULL;
		printf("Leaving enumeration of %p\n", state);
#endif
		GB.StopEnum();
		return;
	}

	val = VAL_value(&state->next);
	start.ck = state->first;
	start.idx = start.ck->last;
	CHUNK_prev_enum(THIS, &start, &state->next);
	GB.ReturnVariant(val);

END_METHOD

BEGIN_METHOD(List_get, GB_INTEGER index)

	int index = VARG(index);
	VAL val;

	CLIST_get(THIS, index, &val);
	if (!val.ck) {
		GB.Error(GB_ERR_BOUND);
		return;
	}
	GB.ReturnVariant(VAL_value(&val));

END_METHOD

BEGIN_METHOD(List_put, GB_VARIANT var; GB_INTEGER index)

	int index = VARG(index);
	VAL val;

	CLIST_get(THIS, index, &val);
	if (!val.ck) {
		GB.Error(GB_ERR_BOUND);
		return;
	}
	GB.StoreVariant(ARG(var), VAL_value(&val));

END_METHOD

/*
 * The main problem when modifying the list structure is that there are
 * references to VALs in Current and all the enumerators.
 *
 * The following postulates shall be met by the algorithms:
 *
 * (V) Value. If an element other than that a VAL refers to is removed, or
 *     an element is added, the reference shall be modified according to the
 *     operation, i.e. it shall stay pointing to the particular value it has
 *     pointed to before. References are value-bound.
 *
 * (B) Beginning. If the element a reference points to is removed, the
 *     reference shall remain relative to the beginning of the list (as long
 *     as it doesn't get empty), i.e. it moves on to the next value. This
 *     will guarantee that the following code works as expected:
 *
 *	Dim vEnum As Variant
 *
 *	For Each vEnum In hList
 *	  hList.Take(hList.FindFirst(vEnum))
 *	Next
 *
 *     If the list gets empty, the references are invalidated.
 *
 * The rearrangement algorithm shall assure the following:
 *
 * (C) Coherency. All values in a chunk must be contiguous.
 *
 * (A) Alignment. The first chunk has all elements aligned to its end; the
 *     last chunk has all elements aligned to its beginning. For the special
 *     case of only one chunk in the List (the 'sole chunk'), it is not
 *     specially aligned but its initial element is at CHUNK_SIZE/2-1.
 *
 * (L) Least Copy. Rearrangement for any non-aligned chunk shall strive for
 *     the least copy operations.
 *
 * (O) Order. Values get never reordered within a chunk or rearranged into
 *     other chunks. They always keep their relative position to each other.
 *
 * Note that if (O) would not apply, on the one hand, we could make more
 * intelligent algorithms, but on the other hand, the first posulates must
 * be improved so it remains in the first place.
 */

/* XXX: Mind the anatomy of this construct! As a 'for' loop it'd be way too
 *      ugly. */
#define begin_all_references(list)					\
do {									\
	CLIST *__list = list;						\
	VAL *__vp;							\
	void *__ebuf;							\
	struct enum_state *__es = NULL;					\
									\
	__ebuf = GB.BeginEnum(__list);					\
	for (__vp = &__list->current; __vp; __vp = !GB.NextEnum() ?	\
			     (__es = (struct enum_state *) GB.GetEnum(),\
			      &__es->next) : NULL) {

#define end_all_references						\
	}								\
	GB.EndEnum(__ebuf);						\
} while (0)

static void CLIST_append(CLIST *list, GB_VARIANT *val)
{
	CHUNK *ck;

	ck = get_chunk(list->list.next);
	/* (A) */
	if (UNLIKELY(!list->count)) {
		ck = CHUNK_new();
		ck->first = ck->last = CHUNK_SIZE / 2 - 1;
		LIST_append(&list->list, &ck->list);
	} else if (UNLIKELY(ck->first == 0)) {
		ck = CHUNK_new();
		ck->first = ck->last = CHUNK_SIZE - 1;
		LIST_append(&list->list, &ck->list);
	} else {
		ck->first--;
	}
	/* (C), (O) */
	GB.StoreVariant(val, &ck->var[ck->first]);
	list->count++;

	/* (V) */
	begin_all_references(list) {
#ifdef DEBUG_ENUMERATOR
		if (__es && !__es->first)
			printf("Caught spurious enumerator %p\n", __es);
#endif
		if (__vp->ck != ck)
			continue;
		__vp->idx++;
	} end_all_references;
}

static void CLIST_prepend(CLIST *list, GB_VARIANT *val)
{
	CHUNK *ck;

	ck = get_chunk(list->list.prev);
	/* (A) */
	if (UNLIKELY(!list->count)) {
		ck = CHUNK_new();
		ck->first = ck->last = CHUNK_SIZE / 2 - 1;
		LIST_prepend(&list->list, &ck->list);
	} else if (UNLIKELY(ck->last == CHUNK_SIZE - 1)) {
		ck = CHUNK_new();
		ck->first = ck->last = 0;
		LIST_prepend(&list->list, &ck->list);
	} else {
		ck->last++;
	}
	/* (C), (O) */
	GB.StoreVariant(val, &ck->var[ck->last]);
	list->count++;
	/* (V): nothing */
}

static void CLIST_take(CLIST *list, VAL *val, GB_VARIANT_VALUE *buf)
{
	GB_VARIANT_VALUE *v;
	CHUNK *ck = val->ck;
	VAL back;
	int gets_empty, is_last;
	int i, src, dst, phantom;
	int n, m;
	size_t size;

	i = val->idx;
	v = &ck->var[i];
	/* Save that value */
	memcpy(buf, v, sizeof(*buf));
	/* No need to not (O) */

	gets_empty = (CHUNK_count(ck) == 1);
	if (gets_empty) {
		phantom = i;
		src = dst = 0;
		goto no_move;
	}

	n = i - ck->first;
	m = ck->last - i;
	is_last = CHUNK_is_last(list, ck);
	/* (A) */
	if (CHUNK_is_first(list, ck)) {
		if (is_last) /* Sole */
			goto normal;
		goto first; /* Algorithms match */
	} else if (is_last) {
		goto last;
	}
normal:
	/* (L) */
	if (n <= m) {
	first: /* Move block before i upwards */
		src = ck->first;
		dst = src + 1;
		ck->first++;
		phantom = src;
	} else {
	last: /* Move block after i downwards */
		src = i + 1;
		dst = i;
		n = m;
		ck->last--;
		phantom = i + m;
	}
	/* (C) */
	size = n * sizeof(ck->var[0]);
	memmove(&ck->var[dst], &ck->var[src], size);

no_move:
	/* Don't forget to remove the phantom value */
	ck->var[phantom].type = GB_T_NULL;

	list->count--;

	/* Don't accidentally erase information if 'val' is a reference
	 * itself. */
	memcpy(&back, val, sizeof(back));
	begin_all_references(list) {
		if (__es && !__es->first) {
#ifdef DEBUG_ENUMERATOR
			printf("Caught spurious enumerator %p\n", __es);
#endif
			continue;
		}
		if (__vp->ck != back.ck)
			continue;
#ifdef DEBUG_ME
		printf(":  in: %p -> %d %d %d\n", __vp, __vp->idx, back.idx,
						  src < dst);
#endif
		/* (B) */
		if (__vp->idx == back.idx) {
			if (!list->count) {
				__vp->ck = NULL;
			} else if (gets_empty || src < dst) {
				CHUNK_next(list, __vp);
			} else {
				__vp->idx--;
				CHUNK_next(list, __vp);
			}
		/* (V) */
		} else if (__vp->idx > back.idx) {
			__vp->idx--;
			CHUNK_next(list, __vp);
		}
#ifdef DEBUG_ME
		printf(": out: %p -> %d (%p,%d,%d)\n", __vp, __vp->idx,
				__vp->ck, __vp->ck ? __vp->ck->first : 0,
				__vp->ck ? __vp->ck->last : 0);
#endif
	} end_all_references;
	if (gets_empty) {
		LIST_unlink(&ck->list);
		CHUNK_destroy(ck);
	}
}

BEGIN_METHOD(List_Append, GB_VARIANT value)

	CLIST_append(THIS, ARG(value));

END_METHOD

BEGIN_METHOD(List_Prepend, GB_VARIANT value)

	CLIST_prepend(THIS, ARG(value));

END_METHOD

#define CHECK_CURRENT()		(THIS->current.ck)
#define CHECK_RAISE_CURRENT()			\
	if (!CHECK_CURRENT()) {			\
		GB.Error("No current element");	\
		return;				\
	}

BEGIN_METHOD(List_Take, GB_INTEGER index)

	VAL val;
	GB_VARIANT_VALUE buf;

	if (MISSING(index)) {
		CHECK_RAISE_CURRENT();
		CLIST_take(THIS, &THIS->current, &buf);
	} else {
		CLIST_get(THIS, VARG(index), &val);
		if (!val.ck) {
			GB.Error(GB_ERR_BOUND);
			return;
		}
		CLIST_take(THIS, &val, &buf);
	}
	GB.ReturnVariant(&buf);
	GB.ReturnBorrow();
	GB.StoreVariant(NULL, &buf);
	GB.ReturnRelease();

END_METHOD

#define IMPLEMENT_Move(which, magic)			\
BEGIN_METHOD_VOID(List_Move ## which)			\
							\
	if (!THIS->count) {				\
		GB.Error("No elements");		\
		return;					\
	}						\
	magic;						\
							\
END_METHOD

IMPLEMENT_Move(Next, if (!CHECK_CURRENT())CLIST_first(THIS, &THIS->current);
	CHUNK_next(THIS, &THIS->current))
IMPLEMENT_Move(Prev, if (!CHECK_CURRENT()) CLIST_last(THIS, &THIS->current);
	CHUNK_prev(THIS, &THIS->current))
IMPLEMENT_Move(First, CLIST_first(THIS, &THIS->current))
IMPLEMENT_Move(Last, CLIST_last(THIS, &THIS->current))

/*
 * Modify 'val' to point to the next/prev value equal to 'comp'. If nothing
 * found - 'val' is allowed to cycle and point to itself again! - NULL is
 * written to val->ck.
 */

static void CLIST_find_forward(CLIST *list, VAL *val, GB_VARIANT *comp)
{
	CHUNK *last = NULL;
	int cached_diff;
	VAL start;

	memcpy(&start, val, sizeof(start));

	cached_diff = 1;
	do {
		/* We actually enumerate but do the checking ourselves */
		CHUNK_next(list, val);
		/* Note that comparing here allows 'val' to point to itself
		 * again. This is intentional for cyclic lists */
		if (!GB.CompVariant(VAL_value(val), &comp->value))
			return;
		if (val->ck != last)
			last = val->ck;
		if (last == start.ck && val->idx == start.idx)
			cached_diff = 0;
	} while (cached_diff);
	/* Invalidate */
	val->ck = NULL; /* This is most likely &list->current */
}

static void CLIST_find_backward(CLIST *list, VAL *val, GB_VARIANT *comp)
{
	CHUNK *last = NULL;
	int cached_diff;
	VAL start;

	memcpy(&start, val, sizeof(start));

	cached_diff = 1;
	do {
		CHUNK_prev(list, val);
		if (!GB.CompVariant(VAL_value(val), &comp->value))
			return;
		if (val->ck != last)
			last = val->ck;
		if (last == start.ck && val->idx == start.idx)
				cached_diff = 0;
	} while (cached_diff);
	val->ck = NULL;
}

#define CHECK_RET_CURRENT()		\
	if (!CHECK_CURRENT()) {		\
		GB.ReturnNull();	\
		return;			\
	}

#define IMPLEMENT_Find(which, which2, magic)				\
									\
BEGIN_METHOD(List_Find ## which, GB_VARIANT value)			\
									\
	if (!THIS->count) {						\
		GB.Error("No elements");				\
		return;							\
	}								\
	magic;								\
	CLIST_find_ ## which2 (THIS, &THIS->current, ARG(value));	\
									\
END_METHOD

IMPLEMENT_Find(Next, forward,
	if (!CHECK_CURRENT()) CLIST_first(THIS, &THIS->current))
IMPLEMENT_Find(Prev, backward,
	if (!CHECK_CURRENT()) CLIST_last(THIS, &THIS->current))
IMPLEMENT_Find(First, forward, CLIST_first(THIS, &THIS->current))
IMPLEMENT_Find(Last, backward, CLIST_last(THIS, &THIS->current))

BEGIN_PROPERTY(List_Current)

	GB_VARIANT_VALUE *val;

	CHECK_RET_CURRENT();
	val = VAL_value(&THIS->current);
	if (READ_PROPERTY) {
		GB.ReturnVariant(val);
		return;
	}
	GB.StoreVariant(PROP(GB_VARIANT), val);

END_PROPERTY

BEGIN_PROPERTY(List_Count)

	GB.ReturnInteger(THIS->count);

END_PROPERTY

GB_DESC CListDesc[] = {
	GB_DECLARE("List", sizeof(CLIST)),

	GB_METHOD("_new", NULL, List_new, NULL),
	GB_METHOD("_free", NULL, List_free, NULL),
	GB_METHOD("_next", "v", List_next, NULL),
	GB_METHOD("_get", "v", List_get, "(Index)i"),
	GB_METHOD("_put", NULL, List_put, "(Value)v(Index)i"),

	GB_METHOD("Clear", NULL, List_free, NULL),

	GB_METHOD("Append", NULL, List_Append, "(Value)v"),
	GB_METHOD("Prepend", NULL, List_Prepend, "(Value)v"),
	GB_METHOD("Take", "v", List_Take, "[(Index)i]"),

	GB_METHOD("MoveNext", NULL, List_MoveNext, NULL),
	GB_METHOD("MovePrev", NULL, List_MovePrev, NULL),
	GB_METHOD("MovePrevious", NULL, List_MovePrev, NULL),
	GB_METHOD("MoveFirst", NULL, List_MoveFirst, NULL),
	GB_METHOD("MoveLast", NULL, List_MoveLast, NULL),

	GB_METHOD("FindNext", NULL, List_FindNext, "(Value)v"),
	GB_METHOD("FindPrev", NULL, List_FindPrev, "(Value)v"),
	GB_METHOD("FindPrevious", NULL, List_FindPrev, "(Value)v"),
	GB_METHOD("FindFirst", NULL, List_FindFirst, "(Value)v"),
	GB_METHOD("FindLast", NULL, List_FindLast, "(Value)v"),

	/*
	 * XXX: Shouldn't this be rather more exposed as the virtual
	 *      container it actually is? But with an absolute index, of
	 *      course. But can this be done efficiently?
	 */
	GB_PROPERTY("Current", "v", List_Current),
	GB_PROPERTY_READ("Count", "i", List_Count),
	GB_PROPERTY_SELF("Backwards", ".List.Backwards"),

	GB_END_DECLARE
};

GB_DESC CListBackwardsDesc[] = {
	GB_DECLARE(".List.Backwards", 0),
	GB_VIRTUAL_CLASS(),

	GB_METHOD("_next", "v", ListBackwards_next, NULL),

	GB_END_DECLARE
};
