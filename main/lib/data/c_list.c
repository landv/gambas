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
	int fidx;	/* Relative to ->ck->first */
	int lidx;	/* Relative to ->ck->last */
} VAL;

typedef struct {
	GB_BASE ob;
	LIST list;	/* Beginning of linked CHUNKs */
	VAL current;	/* Current element */
	int count;	/* Do not iterate over all elements to get this */
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

static inline void set_fidx(int fidx, VAL *val)
{
	val->fidx = fidx;
	val->lidx = val->ck->first + fidx - val->ck->last;
}

static inline void set_lidx(int lidx, VAL *val)
{
	val->lidx = lidx;
	val->fidx = val->ck->last + lidx - val->ck->first;
}

static void CHUNK_free_all(CHUNK *ck)
{
	int i;

	if (ck->first < 0 || ck->last  < 0)
		return;
	for (i = ck->first; i <= ck->last; i++)
		if (ck->var[i].type != GB_T_NULL)
			GB.StoreVariant(NULL, &ck->var[i]);
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

END_METHOD

static inline GB_VARIANT_VALUE *VAL_value(VAL *val)
{
	int i = val->fidx + val->ck->first;
	int j = val->lidx + val->ck->last;

	assert(val->fidx >= 0);
	assert(val->lidx <= 0);
	assert(i == j);
	if (val->fidx == -1 || i > val->ck->last)
		return NULL;
	return &val->ck->var[i];
}

static inline int VAL_is_valid(VAL *val)
{
	int i = val->fidx + val->ck->first;
	int j = val->lidx + val->ck->last;

	if (i != j || !val->ck || val->fidx == -1 || i > val->ck->last)
		return 0;
	return 1;
}

static inline int VAL_is_equal(VAL *v1, VAL *v2)
{
	return v1->ck == v2->ck && v1->fidx == v2->fidx;
}

static void CLIST_first(CLIST *list, VAL *buf)
{
	if (!list->count) {
		buf->ck = NULL;
		return;
	}
	buf->ck = get_chunk(list->list.next);
	set_fidx(0, buf);
}

static void CLIST_last(CLIST *list, VAL *buf)
{
	if (!list->count) {
		buf->ck = NULL;
		return;
	}
	buf->ck = get_chunk(list->list.prev);
	set_lidx(0, buf);
}

/*
 * Modify @val so that it points to the next/prev valid value. @first is
 * used to detect the end of an enumeration, i.e. where no other elements
 * should be looked for. In this case, NULL is written to @val->ck.
 */

static void CHUNK_next(CLIST *list, VAL *val)
{
	LIST *node;

	/* Try to just update the index. This approach comes from the
	 * rearrange algorithm. */
	if (val->lidx) {
		set_fidx(val->fidx + 1, val);
		return;
	}

	/* Go to next chunk */
	if ((node = val->ck->list.next) == &list->list)
		node = node->next;
	val->ck = get_chunk(node);
	set_fidx(0, val);
}

static void CHUNK_next_enum(CLIST *list, VAL *first, VAL *val)
{
	CHUNK *ck = val->ck;
	LIST *node;

	assert(first != val);
	if (val->lidx) {
		set_fidx(val->fidx + 1, val);
		if (VAL_is_equal(first, val))
			goto no_next;
		return;
	}
	if ((node = ck->list.next) == &list->list)
		node = node->next;
	ck = get_chunk(node);
	if (ck == first->ck && first->fidx == 0)
		goto no_next;
	val->ck = ck;
	set_fidx(0, val);
	return;

no_next:
	val->ck = NULL;
}

/* Safe version for CLIST_take(), only if there cur->fidx == val->fidx */
static void CHUNK_next_safe(CLIST *list, VAL *val)
{
	LIST *node;

	if (val->fidx <= val->ck->last) {
		set_fidx(val->fidx, val);
		return;
	}
	/* Also prevent from getting over the end! */
	if ((node = val->ck->list.next) == &list->list) {
		CLIST_last(list, val);
	} else {
		val->ck = get_chunk(node);
		set_fidx(0, val);
	}
}

static void CHUNK_prev(CLIST *list, VAL *val)
{
	LIST *node;

	if (val->fidx) {
		set_fidx(val->fidx - 1, val);
		return;
	}
	if ((node = val->ck->list.prev) == &list->list)
		node = node->prev;
	val->ck = get_chunk(node);
	set_lidx(0, val);
}

static void CHUNK_prev_enum(CLIST *list, VAL *first, VAL *val)
{
	CHUNK *ck = val->ck;
	LIST *node;

	assert(first != val);
	if (val->fidx) {
		set_fidx(val->fidx - 1, val);
		if (VAL_is_equal(first, val))
			goto no_prev;
		return;
	}
	if ((node = ck->list.prev) == &list->list)
		node = node->prev;
	ck = get_chunk(node);
	if (ck == first->ck && first->lidx == 0)
		goto no_prev;
	val->ck = ck;
	set_lidx(0, val);
	return;
no_prev:
	val->ck = NULL;
}

/*
 * Random access function.
 * Negative @idx makes go backwards. The @val is filled. Out of bounds is
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
		/*idx = -(idx + 1);*/
		i = list->count - (i + 1);
	}

	node = &list->list;
	/* Compute the offset to node->next or node->prev depending on the
	 * direction we traverse. This saves an if (dir >= 0) branch
	 * prediction catastrophe in loop */
	off = (dir >= 0 ? offsetof(LIST, next) : offsetof(LIST, prev));
	do {
		node = *((LIST **) ((long) node + off));
		assert(node != &list->list);
		ck = get_chunk(node);
		count = CHUNK_count(ck);
		if (i < count) {
			val->ck = ck;
			if (dir < 0)
				set_lidx(-i, val);
			else
				set_fidx(i, val);
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
	VAL start;

	if (!*((int *) state)) { /* Beginning */
		CLIST_first(THIS, &state->next);
		state->first = state->next.ck;
	}
	/* No elements left? */
	if (!state->next.ck)
		goto stop_enum;

	val = VAL_value(&state->next);
	start.ck = state->first;
	set_fidx(0, &start);
	CHUNK_next_enum(THIS, &start, &state->next);
	GB.ReturnVariant(val);
	return;

stop_enum:
	GB.StopEnum();

END_METHOD

/*
 * The same as List_next but backwards.
 */

BEGIN_METHOD_VOID(ListBackwards_next)

	struct enum_state *state = GB.GetEnum();
	GB_VARIANT_VALUE *val;
	VAL start;

	if (!*((int *) state)) { /* Beginning */
		CLIST_last(THIS, &state->next);
		state->first = state->next.ck;
	}
	/* No elements left? */
	if (!state->next.ck)
		goto stop_enum;

	val = VAL_value(&state->next);
	start.ck = state->first;
	set_lidx(0, &start);
	CHUNK_prev_enum(THIS, &start, &state->next);
	GB.ReturnVariant(val);
	return;

stop_enum:
	GB.StopEnum();

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
 * The machinery to modify the List structure, i.e. Append/Prepend/Take
 * shall satisfy the following points:
 *
 * Notification prevents persistent VALs (Current and the enumerators)
 * from getting ("dangerously", as opposed to "intentionally") invalid:
 * (S) Stay. If an element other than that a VAL refers to is removed, or an
 *     element is added, the VAL shall be modified according to the
 *     operation, i.e. it shall stay pointing to the particular value it has
 *     pointed to before.
 * (B) Beginning. If the element a VAL refers to is removed, it shall remain
 *     relative to the beginning of the list (as long as it doesn't get
 *     empty), i.e. it moves on to the next value.
 *     If the list gets empty, the VALs are invalidated.
 *
 * The rearrange algorithm shall assure the following:
 * (C) Coherency. All values in a chunk must be contiguous.
 * (A) Alignment. The first chunk has all elements aligned to its end; the
 *     last chunk has all elements aligned to its beginning. For the special
 *     case of only one chunk in the List (the 'sole chunk'), it is not
 *     specially aligned but its initial element is at CHUNK_SIZE/2-1.
 * (L) Least Copy. Rearrangement for any non-aligned chunk shall strive for
 *     the least copy operations.
 * (O) Order. Values get never reordered within a chunk or rearranged into
 *     other chunks. They always keep their relative position to each other.
 * Note that if (O) would not apply, on the one hand, we could make more
 * intelligent algorithms, but on the other hand, the notification algorithm
 * must be improved so it remains in the first place.
 */

/* XXX: Give only variables! */
#define for_all_persistent_vals(list, vp, es, ebuf)			\
	for (ebuf = GB.BeginEnum(list), vp = &list->current, es = NULL;	\
		vp ? 1 : (GB.EndEnum(ebuf), 0);				\
		!GB.NextEnum() ? (es = (struct enum_state*) GB.GetEnum(),\
			vp = &es->next) : (vp = NULL))

static void CLIST_append(CLIST *list, GB_VARIANT *val)
{
	CHUNK *ck;
	VAL *cur;
	struct enum_state *es;
	void *save_enum;

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

	/* (S) */
	for_all_persistent_vals(list, cur, es, save_enum) {
		if (cur->ck != ck)
			continue;
		/* On Append, the ->lidx + ->ck->last is still correct */
		set_lidx(cur->lidx, cur);
	}
}

static void CLIST_prepend(CLIST *list, GB_VARIANT *val)
{
	CHUNK *ck;
	VAL *cur;
	struct enum_state *es;
	void *save_enum;

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

	/* (S) */
	for_all_persistent_vals(list, cur, es, save_enum) {
		if (cur->ck != ck)
			continue;
		/* Conversely to Append, here ->fidx + ->ck->first is still
		 * correct */
		set_fidx(cur->fidx, cur);
	}
}

static void CLIST_take(CLIST *list, VAL *val, GB_VARIANT_VALUE *buf)
{
	GB_VARIANT_VALUE *v;
	CHUNK *ck = val->ck;
	VAL *cur;
	struct enum_state *es;
	void *save_enum;
	int gets_empty, islast;
	int i, src, dst, phantom;
	int n, m;
	size_t size;

	i = val->fidx + ck->first;
	v = &ck->var[i];
	/* Save that value */
	memcpy(buf, v, sizeof(*buf));
	/* No need to not (O) */

	gets_empty = (CHUNK_count(ck) == 1);
	if (gets_empty) {
		phantom = i;
		goto no_move;
	}

	n = i - ck->first;
	m = ck->last - i;
	islast = CHUNK_is_last(list, ck);
	/* (A) */
	if (CHUNK_is_first(list, ck)) {
		if (islast) /* Sole */
			goto normal;
		goto first; /* Algorithms match */
	} else if (islast) {
		goto last;
	} else {
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
	}
	/* (C) */
	size = n * sizeof(ck->var[0]);
	memmove(&ck->var[dst], &ck->var[src], size);

no_move:
	/* Don't forget to remove the phantom value */
	ck->var[phantom].type = GB_T_NULL;

	list->count--;

	for_all_persistent_vals(list, cur, es, save_enum) {
		/* To not erase information before future curs */
		assert(cur != val);
/* Benoit, this is about the enumerator case. */
#if 0
		if (!cur->ck)
			warnx("cur->ck == NULL => spurious enumerator?");
#endif
		if (cur->ck != val->ck)
			continue;
		/* (R) */
		if (cur->fidx == val->fidx) {
			if (!list->count)
				cur->ck = NULL;
			else
				CHUNK_next_safe(list, cur);
		/* (S) */
		} else if (cur->fidx < val->fidx) {
			set_fidx(cur->fidx, cur);
		} else {
			set_lidx(cur->lidx, cur);
		}
	}
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

	if (!THIS->count) {
		GB.Error(GB_ERR_BOUND);
		return;
	}
	if (MISSING(index)) {
		CHECK_RAISE_CURRENT();
		memcpy(&val, &THIS->current, sizeof(val));
		CLIST_take(THIS, &val, &buf);
		memcpy(&THIS->current, &val, sizeof(val));
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
 * Modify @val to point to the next/prev value equal to @comp. If nothing
 * found - @val is allowed to cycle and point to itself again! - NULL is
 * written to @val->ck.
 */

static void CLIST_find_forward(CLIST *list, VAL *val, GB_VARIANT *comp)
{
	CHUNK *last = NULL;
	int cached_diff;
	VAL start;

	memcpy(&start, val, sizeof(start));

	cached_diff = 1;
	do {
		/* We do actually enumerate but do the checking ourselves */
		CHUNK_next(list, val);
		/* Note that comparing here allows @val to point to itself
		 * again. This is intentional for cyclic lists */
		if (!GB.CompVariant(VAL_value(val), &comp->value))
			return;
		if (val->ck != last)
			last = val->ck;
		if (last == start.ck && val->fidx == start.fidx)
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
		if (last == start.ck && val->fidx == start.fidx)
				cached_diff = 0;
	} while (cached_diff);
	val->ck = NULL;
}

#define CHECK_FOUND()	(THIS->current.ck)
#define CHECK_RET_FOUND()		\
	if (!CHECK_FOUND()) {		\
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
	if (!CHECK_FOUND()) CLIST_first(THIS, &THIS->current))
IMPLEMENT_Find(Prev, backward,
	if (!CHECK_FOUND()) CLIST_last(THIS, &THIS->current))
IMPLEMENT_Find(First, forward, CLIST_first(THIS, &THIS->current))
IMPLEMENT_Find(Last, backward, CLIST_last(THIS, &THIS->current))

BEGIN_PROPERTY(List_Current)

	GB_VARIANT_VALUE *val;

	CHECK_RAISE_CURRENT();
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
