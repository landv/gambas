/*
 * c_list.c - Circular doubly-linked lists
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
 * + Cached references ('anchors') in the list which can be used calculate
 *   the best starting point for a traversal to a given index.
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
	int lgi;	/* Absolute list global index [-Count; Count - 1] */
} VAL;

typedef struct {
	GB_BASE ob;
	LIST list;	/* Beginning of linked CHUNKs */
	VAL current;	/* Current element */
	size_t count;	/* Do not iterate over all elements to get this */
	int autonorm;	/* Automatically normalise indices */
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

static inline int VAL_is_first(CLIST *list, VAL *val)
{
	CHUNK *ck = val->ck;

	return CHUNK_is_first(list, ck) && val->idx == ck->first;
}

static inline int CHUNK_is_last(CLIST *list, CHUNK *ck)
{
	return list->list.prev == &ck->list;
}

static inline int VAL_is_last(CLIST *list, VAL *val)
{
	CHUNK *ck = val->ck;

	return CHUNK_is_last(list, ck) && val->idx == ck->last;
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
	THIS->autonorm = 0;

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

/*
 * A VAL carries an absolute index of the element it represents in the list.
 * This must be updated by each of the functions that modified a VAL because
 * they are trusted by traversing algorithms (which have access to Current
 * and the enumerators) when looking for the shortest around through the
 * list.
 *
 * The index can be positive or negative. We need some magic to update it
 * correctly everytime and with every implementation of C (that's about the
 * sign of the result from a % operation).
 *
 * (Be sure to compile this code with a high optimisation level.)
 */

#ifndef sgn
# define sgn(x)				\
({					\
	int __x = (x);			\
					\
	__x < 0 ? -1 : (__x ? 1 : 0);	\
})
#endif

/* One's complement abs() */
#define onesabs(x)		\
({				\
	int __x = (x);		\
				\
	__x < 0 ? ~__x : __x;	\
})

/* Get corresponding non-negative index over the list */
#define abslgi(list, i)				\
({						\
	CLIST *__l = (list);			\
	int __i = (i);				\
						\
	__i < 0 ? __l->count + __i : __i;	\
})

#define update_lgi(list, val, i)			\
do {							\
	CLIST *__l = (list);				\
	VAL *__v = (val);				\
	int __i = (i);					\
							\
	if (!__l->count) {				\
		__v->ck = NULL;				\
	} else {					\
		__v->lgi = (onesabs(__i) % __l->count);	\
		if (__i < 0)				\
			__v->lgi = ~__v->lgi;		\
	}						\
} while (0)

static void CLIST_first(CLIST *list, VAL *buf)
{
	if (!list->count) {
		buf->ck = NULL;
		/*
		 * The VAL is invalid but we want to clear this anyways.
		 */
		update_lgi(list, buf, 0);
		return;
	}

	buf->ck = get_chunk(list->list.next);
	buf->idx = buf->ck->first;
	update_lgi(list, buf, 0);
}

static void CLIST_last(CLIST *list, VAL *buf)
{
	if (!list->count) {
		buf->ck = NULL;
		update_lgi(list, buf, 0);
		return;
	}
	buf->ck = get_chunk(list->list.prev);
	buf->idx = buf->ck->last;
	update_lgi(list, buf, -1);
}

/*
 * Modify 'val' so that it points to the next/prev valid value. 'first' is
 * used to detect the end of an enumeration, i.e. where no other elements
 * should be looked for. In this case, NULL is written to val->ck.
 *
 * Since the LGI is said to be in bounds of [-Count; Count - 1], these
 * functions must watch when the list head is traversed and reset the index
 * accordingly.
 */

static void CHUNK_next(CLIST *list, VAL *val)
{
	LIST *node;

	update_lgi(list, val, val->lgi + 1);

	/* Try to just update the index. */
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

	update_lgi(list, val, val->lgi + 1);

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
	update_lgi(list, val, 0);
}

static void CHUNK_prev(CLIST *list, VAL *val)
{
	LIST *node;

	update_lgi(list, val, val->lgi - 1);

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

	update_lgi(list, val, val->lgi - 1);

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
	update_lgi(list, val, 0);
}

/*
 * Random access function stuff (for CLIST_get()).
 * Negative 'idx' makes go backwards. The 'val' is filled. Out of bounds is
 * signalled by val->ck == NULL.
 *
 * We use all references (i.e. Current and the enumerators) to get a good
 * anchor, i.e. a VAL and a direction from where we can get to the desired
 * index the shortest way.
 */

/* XXX: sizeof(VAL) may never exceed 3*sizeof(intptr_t)! */
struct enum_state {
	CHUNK *first;
	VAL next;
};

#define begin_all_references(list)					\
do {									\
	CLIST *__list = list;						\
	VAL *__vp;							\
	void *__ebuf;							\
	struct enum_state *__es = NULL;					\
									\
	__ebuf = GB.BeginEnum(__list);					\
	if (!__list->current.ck) {					\
		if (GB.NextEnum()) {					\
			__vp = NULL;					\
		} else {						\
			__es = (struct enum_state *) GB.GetEnum();	\
			__vp = &__es->next;				\
		}							\
	} else {							\
		__vp = &__list->current;				\
	}								\
	for (; __vp; __vp = GB.NextEnum() ? NULL :			\
	     (__es = (struct enum_state *) GB.GetEnum(), &__es->next)) {

#define end_all_references						\
	}								\
	GB.EndEnum(__ebuf);						\
} while (0)

struct anchor {
	VAL start;
	int direction;
};

/* 'idx' is required to be non-negative and in bounds at this point. */
static inline void get_best_anchor(CLIST *list, int idx, struct anchor *buf)
{
	int d, tmp;

	/* Distance from head forwards/backwards/of all references */
	d = idx;
	tmp = list->count - 1 - idx;
	if (tmp < d) {
		d = tmp;
		CLIST_last(list, &buf->start);
	} else {
		CLIST_first(list, &buf->start);
	}
	begin_all_references(list) {
		tmp = abs(abslgi(list, __vp->lgi) - idx);
		if (tmp < d) {
			d = tmp;
			memcpy(&buf->start, __vp, sizeof(buf->start));
		}
	} end_all_references;

	buf->direction = sgn(idx - abslgi(list, buf->start.lgi));
}

static inline void get_body_forward(CLIST *list, LIST *node, int i,
				    VAL *val)
{
	CHUNK *ck;
	int count;

	while (1) {
		ck = get_chunk(node);
		count = CHUNK_count(ck);

		if (i < count) {
			val->ck = ck;
			val->idx = ck->first + i;
			return;
		}
		i -= count;
		do
			node = node->next;
		while (node == &list->list);
	}
}

static inline void get_body_backward(CLIST *list, LIST *node, int i,
				     VAL *val)
{
	CHUNK *ck;
	int count;

	while (1) {
		do
			node = node->prev;
		while (node == &list->list);
		ck = get_chunk(node);
		count = -CHUNK_count(ck);

		if (i >= count) {
			val->ck = ck;
			val->idx = ck->last + i + 1;
			return;
		}
		i -= count;
	}
}

static void CLIST_get(CLIST *list, int idx, VAL *val)
{
	LIST *node;
	int i, dir;
	struct anchor anchor;

	/* Make a non-negative index */
	i = abslgi(list, idx);
	/* We do _not_ allow indices to wrap around the end, like we could:
	 * i %= list->count. Don't use a loop just to detect that case. */
	if (i >= list->count) {
		/* Not enough elements. */
		val->ck = NULL;
		return;
	}

	get_best_anchor(list, i, &anchor);
	dir = anchor.direction;

	update_lgi(list, val, idx);
	/* Got that index in a reference already? Just copy. */
	if (!dir) {
		val->ck = anchor.start.ck;
		val->idx = anchor.start.idx;
		return;
	}

	node = &anchor.start.ck->list;
	/*
	 * Don't start exactly at the given anchor point (possibly in the
	 * middle of a chunk) but instead at the beginning of the chunk.
	 * Because of cache spatiality, this is not a great loss and the
	 * code is simplified much.
	 */
	i -= abslgi(list, anchor.start.lgi);
	i += anchor.start.idx - anchor.start.ck->first;

	/*
	 * Prevent a if (i < 0) branch prediction catastrophe if we merged
	 * both algorithms into one loop.
	 */
	if (i < 0)
		get_body_backward(list, node, i, val);
	else
		get_body_forward(list, node, i, val);
}

BEGIN_METHOD_VOID(List_next)

	struct enum_state *state = GB.GetEnum();
	GB_VARIANT_VALUE *val;
	/* XXX: Would like to cache that in the enum_state but no space left
	 *      there... */
	VAL start;

	if (!state->first) { /* Beginning */
		CLIST_first(THIS, &state->next);
		state->first = state->next.ck;
	}
	/* No elements left? */
	if (!state->next.ck) {
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
		CLIST_last(THIS, &state->next);
		state->first = state->next.ck;
	}
	/* No elements left? */
	if (!state->next.ck) {
		state->first = NULL;
		GB.StopEnum();
		return;
	}

	val = VAL_value(&state->next);
	start.ck = state->first;
	start.idx = start.ck->last;
	CHUNK_prev_enum(THIS, &start, &state->next);
	GB.ReturnVariant(val);

END_METHOD

static inline int normalise_index(CLIST *list, int index)
{
	int i;

	i = onesabs(index) % list->count;
	if (index < 0)
		i = ~i;
	return i;
}

BEGIN_METHOD(List_get, GB_INTEGER index)

	int index = VARG(index);
	VAL val;

	if (THIS->autonorm)
		index = normalise_index(THIS, index);
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

	if (THIS->autonorm)
		index = normalise_index(THIS, index);
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
 * (V) Value. If an element other than that a VAL refers to is removed or an
 *     element is added, the reference shall be modified according to the
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
 *     Should a chunk be allocated and linked into the middle of the list,
 *     its initial element shall also be at CHUNK_SIZE/2-1.
 *
 * (L) Least Copy. Rearrangement for any non-aligned chunk shall strive for
 *     the least copy operations.
 *
 * (O) Order. Values get never reordered with respect to the list. It is
 *     possible, however, to move elements to other chunks as long as the
 *     order persists.
 *
 * Note that if (O) would not apply, on the one hand, we could make more
 * intelligent algorithms, but on the other hand, the first posulates must
 * be improved so it remains in the first place.
 */

static void CLIST_append(CLIST *list, GB_VARIANT *var)
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
	GB.StoreVariant(var, &ck->var[ck->first]);
	list->count++;

	/* (V) */
	begin_all_references(list) {
		if (__vp->lgi >= 0)
			__vp->lgi++;
		if (__vp->ck == ck)
			__vp->idx++;
	} end_all_references;
}

static void CLIST_prepend(CLIST *list, GB_VARIANT *var)
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
	GB.StoreVariant(var, &ck->var[ck->last]);
	list->count++;

	/* (V) */
	begin_all_references(list) {
		if (__vp->lgi < 0)
			__vp->lgi--;
	} end_all_references;
}

/*
 * With the VAL_append()/prepend() functions it is a little more difficult.
 * We have to distinguish three cases (processed in this order):
 * 1) There is space for the operation in this chunk: so shift elements and
 *    insert the new value;
 * 1a) If the element is to be appended/prepended after/before the
 *     last/first element, do not shift;
 * 2) There is space _immediately_ free in a neighbour chunk for the
 *    operation (for append it's the next chunk, for prepend the previous
 *    one), i.e. we don't need to shift the neighbour chunk, then shift
 *    values into this chunk and insert the new value. This may never cross
 *    the list head!;
 * 2a) See 1a);
 * 3) If none of the previous things worked, allocate a new chunk and shift
 *    in there;
 * 3a) See 1a);
 *
 * Point 2) didn't shift values in the neighbour because this could go
 * infinitely through the list so we just allocate a new chunk to do it fast
 * and easily.
 */

static void VAL_append(CLIST *list, VAL *val, GB_VARIANT *var)
{
	CHUNK *ck, *next = NULL;
	int s, n, shifted_to_next = 0;
	GB_VARIANT_VALUE *buf;
	VAL back;

	ck = val->ck;
	/* Currently not (L). We only shift towards the end. */
	if (ck->last < CHUNK_SIZE - 1) { /* 1) */
		ck->last++;
		if (val->idx == ck->last - 1) { /* 1a) */
			buf = &ck->var[ck->last];
		} else {
		shift:
			s = val->idx + 1;
			n = ck->last - s;
			memmove(&ck->var[s + 1], &ck->var[s],
					n * sizeof(ck->var[0]));
			buf = &ck->var[s];
		}
	} else {
		LIST *node = ck->list.next;

		/* 2) */
		if (node == &list->list)
			goto add_new_chunk;

		next = get_chunk(node);
		if (next->first) {
			next->first--;
			if (val->idx == ck->last) { /* 2a) */
				buf = &next->var[next->first];
				goto have_buf;
			}
		shift_to_next:
			shifted_to_next = 1;
			memcpy(&next->var[next->first], &ck->var[ck->last],
							sizeof(ck->var[0]));
			goto shift;
		} else { /* 3) */
		add_new_chunk:
			next = CHUNK_new();
			next->first = next->last = CHUNK_SIZE / 2 - 1;
			LIST_append(&ck->list, &next->list);
			if (val->idx == ck->last) { /* 3a) */
				buf = &next->var[next->first];
				goto have_buf;
			}
			goto shift_to_next;
		}
	}

have_buf:
	bzero(buf, sizeof(*buf));
	buf->type = GB_T_NULL;
	GB.StoreVariant(var, buf);
	list->count++;

	int lgi, vlgi;

	/*
	 * Nasty: When appending to the last element iff it has a negative
	 * LGI, i.e. val->lgi == -1, then we must not produce a 'lgi' of
	 * value 0. For a positive LGI of the last element, we just
	 * incremented list->count so that's no problem.
	 */
	if (val->lgi == -1)
		lgi = abslgi(list, -1);
	else
		lgi = normalise_index(list, abslgi(list, val->lgi + 1));
	memcpy(&back, val, sizeof(back));
	begin_all_references(list) {
		vlgi = abslgi(list, __vp->lgi);

		if (vlgi <= lgi && __vp->lgi < 0)
			__vp->lgi--;
		else if (vlgi > lgi && __vp->lgi >= 0)
			__vp->lgi++;
		__vp->lgi = normalise_index(list, __vp->lgi);

		if (__vp->ck == back.ck) {
			if (shifted_to_next && __vp->idx == back.ck->last) {
				__vp->ck = next;
				__vp->idx = next->first;
			} else if (__vp->idx > back.idx) {
				__vp->idx++;
			}
		}
	} end_all_references;
}

static void VAL_prepend(CLIST *list, VAL *val, GB_VARIANT *var)
{
	CHUNK *ck, *prev = NULL;
	int s, n, shifted_to_prev = 0;
	GB_VARIANT_VALUE *buf;
	VAL back;

	ck = val->ck;
	/* Not (L) */
	if (ck->first) { /* 1) */
		ck->first--;
		if (val->idx == ck->first + 1) { /* 1a) */
			buf = &ck->var[ck->first];
		} else {
		shift:
			s = ck->first + 1;
			n = val->idx - s;
			memmove(&ck->var[s - 1], &ck->var[s],
					n * sizeof(ck->var[0]));
			buf = &ck->var[s + n - 1];
		}
	} else {
		LIST *node = ck->list.prev;

		/* 2) */
		if (node == &list->list)
			goto add_new_chunk;

		prev = get_chunk(node);
		if (prev->last < CHUNK_SIZE - 1) {
			prev->last++;
			if (val->idx == ck->first) { /* 2a) */
				buf = &prev->var[prev->last];
				goto have_buf;
			}
		shift_to_prev:
			shifted_to_prev = 1;
			memcpy(&prev->var[prev->last], &ck->var[ck->first],
							sizeof(ck->var[0]));
			goto shift;
		} else { /* 3) */
		add_new_chunk:
			prev = CHUNK_new();
			prev->first = prev->last = CHUNK_SIZE / 2 - 1;
			LIST_prepend(&ck->list, &prev->list);
			if (val->idx == ck->first) { /* 3a) */
				buf = &prev->var[prev->last];
				goto have_buf;
			}
			goto shift_to_prev;
		}
	}

have_buf:
	bzero(buf, sizeof(*buf));
	buf->type = GB_T_NULL;
	GB.StoreVariant(var, buf);
	list->count++;

	int lgi, vlgi;

	/*
	 * We have a similar case here as in VAL_append() but the other way
	 * around: if the element is the new first one, we must check if
	 * val->lgi is 0, it then may stay 0. If it was -list->count,
	 * everything is fine because of list->count++ above.
	 */
	if (!val->lgi)
		lgi = 0;
	else
		lgi = normalise_index(list, abslgi(list, val->lgi - 1));
	memcpy(&back, val, sizeof(back));
	begin_all_references(list) {
		vlgi = abslgi(list, __vp->lgi);

		if (vlgi <= lgi && __vp->lgi < 0)
			__vp->lgi--;
		else if (vlgi >= lgi && __vp->lgi >= 0)
			__vp->lgi++;
		__vp->lgi = normalise_index(list, __vp->lgi);

		if (__vp->ck == back.ck) {
			if (shifted_to_prev && __vp->idx == back.ck->first) {
				__vp->ck = prev;
				__vp->idx = prev->last;
			} else if (__vp->idx < back.idx) {
				__vp->idx--;
			}
		}
	} end_all_references;
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
	/* Don't forget to remove the phantom value (overriding the actual
	 * data with zeros to be sure from leakage) */
	bzero(&ck->var[phantom], sizeof(ck->var[0]));
	ck->var[phantom].type = GB_T_NULL;

	list->count--;

	/* Don't accidentally erase information if 'val' is a reference
	 * itself because then once __vp == val and __vp will be changed. */
	memcpy(&back, val, sizeof(back));
	begin_all_references(list) {
#ifdef DEBUG_ME
		printf(":  in: %p -> %d (%d) %d %d\n", __vp, __vp->idx,
					__vp->lgi, back.idx, src < dst);
#endif

		/* The LGI stuff is pretty much independent of the ->ck and
		 * ->idx code and conditions. We only care about negative
		 * LGIs basically because non-negative ones automagically
		 * stay correct according to (B). */
		int vlgi = abslgi(list, __vp->lgi);
		int blgi = abslgi(list, back.lgi);

		if (vlgi >= blgi) {
			/* If this reference is taken and it's the last
			 * element of the list, then set it to zero. Kind
			 * of a nasty condition to occur but this is the
			 * only difficulty here because of (B). */
			if (UNLIKELY(vlgi == list->count && vlgi == blgi))
				__vp->lgi = 0;
			else if (__vp->lgi < 0)
				__vp->lgi++;
		}

		if (__vp->ck != back.ck)
			continue;

		/* (B) */
		if (__vp->idx == back.idx) {
			if (!list->count) {
				__vp->ck = NULL;
				continue;
			} else if (gets_empty) {
				goto next_chunk;
			} else if (src < dst) {
				/* According to ck->first++ above which
				 * happens only when src < dst. */
				__vp->idx++;
			}
		}
		/* (V) */
		LIST *node;

		if (__vp->idx > __vp->ck->last) {
		next_chunk:
			if ((node = __vp->ck->list.next) == &list->list)
				node = node->next;
			__vp->ck = get_chunk(node);
			__vp->idx = __vp->ck->first;
		}

#ifdef DEBUG_ME
		printf(": out: %p -> %d (%d) (%p,%d,%d)\n", __vp, __vp->idx,
				__vp->lgi, __vp->ck, __vp->ck ?
					__vp->ck->first : 0, __vp->ck ?
					__vp->ck->last : 0);
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
	int index;

	if (MISSING(index)) {
		CHECK_RAISE_CURRENT();
		CLIST_take(THIS, &THIS->current, &buf);
	} else {
		index = VARG(index);
		if (THIS->autonorm)
			index = normalise_index(THIS, index);
		CLIST_get(THIS, index, &val);
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

#define IMPLEMENT_Move(which, magic)		\
BEGIN_METHOD_VOID(List_Move ## which)		\
						\
	if (!THIS->count) {			\
		GB.Error("No elements");	\
		return;				\
	}					\
	magic;					\
						\
END_METHOD

IMPLEMENT_Move(Next, if (!CHECK_CURRENT())CLIST_first(THIS, &THIS->current);
	CHUNK_next(THIS, &THIS->current);)
IMPLEMENT_Move(Prev, if (!CHECK_CURRENT()) CLIST_last(THIS, &THIS->current);
	CHUNK_prev(THIS, &THIS->current);)
IMPLEMENT_Move(First, CLIST_first(THIS, &THIS->current);)
IMPLEMENT_Move(Last, CLIST_last(THIS, &THIS->current);)

BEGIN_METHOD(List_MoveTo, GB_INTEGER index)

	int index = VARG(index);

	if (THIS->autonorm)
		index = normalise_index(THIS, index);

	CLIST_get(THIS, index, &THIS->current);
	if (!THIS->current.ck)
		GB.Error(GB_ERR_BOUND);

END_METHOD

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

BEGIN_PROPERTY(List_AutoNormalize)

	if (READ_PROPERTY) {
		GB.ReturnBoolean(THIS->autonorm);
		return;
	}
	THIS->autonorm = VPROP(GB_BOOLEAN);

END_PROPERTY

BEGIN_PROPERTY(ListItem_Value)

	GB_VARIANT_VALUE *val;

	CHECK_RET_CURRENT();
	val = VAL_value(&THIS->current);
	if (READ_PROPERTY) {
		GB.ReturnVariant(val);
		return;
	}
	GB.StoreVariant(PROP(GB_VARIANT), val);

END_PROPERTY

BEGIN_PROPERTY(ListItem_Index)

	int index;

	if (READ_PROPERTY) {
		GB.ReturnInteger(THIS->current.lgi);
		return;
	}
	if (THIS->autonorm)
		index = normalise_index(THIS, VPROP(GB_INTEGER));
	else
		index = VPROP(GB_INTEGER);
	CLIST_get(THIS, index, &THIS->current);
	if (!THIS->current.ck)
		GB.Error(GB_ERR_BOUND);

END_PROPERTY

BEGIN_PROPERTY(List_Count)

	GB.ReturnInteger(THIS->count);

END_PROPERTY

BEGIN_PROPERTY(List_Current)

	CHECK_RAISE_CURRENT();
	GB.ReturnSelf(THIS);

END_PROPERTY

GB_DESC CList[] = {
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
	GB_METHOD("MoveTo", NULL, List_MoveTo, "(Index)i"),

	GB_METHOD("FindNext", NULL, List_FindNext, "(Value)v"),
	GB_METHOD("FindPrev", NULL, List_FindPrev, "(Value)v"),
	GB_METHOD("FindPrevious", NULL, List_FindPrev, "(Value)v"),
	GB_METHOD("FindFirst", NULL, List_FindFirst, "(Value)v"),
	GB_METHOD("FindLast", NULL, List_FindLast, "(Value)v"),

	GB_PROPERTY("AutoNormalize", "b", List_AutoNormalize),
	GB_PROPERTY("Current", ".List.Item", List_Current),
	GB_PROPERTY("Value", "v", ListItem_Value),
	GB_PROPERTY("Index", "i", ListItem_Index),
	GB_PROPERTY_READ("Count", "i", List_Count),
	GB_PROPERTY_SELF("Backwards", ".List.Backwards"),

	GB_END_DECLARE
};

GB_DESC CListBackwards[] = {
	GB_DECLARE_VIRTUAL(".List.Backwards"),

	GB_METHOD("_next", "v", ListBackwards_next, NULL),

	GB_END_DECLARE
};

BEGIN_METHOD(ListItem_Append, GB_VARIANT value)

	VAL_append(THIS, &THIS->current, ARG(value));

END_METHOD

BEGIN_METHOD(ListItem_Prepend, GB_VARIANT value)

	VAL_prepend(THIS, &THIS->current, ARG(value));

END_METHOD

BEGIN_PROPERTY(ListItem_IsFirst)

	GB.ReturnBoolean(VAL_is_first(THIS, &THIS->current));

END_PROPERTY

BEGIN_PROPERTY(ListItem_IsLast)

	GB.ReturnBoolean(VAL_is_last(THIS, &THIS->current));

END_PROPERTY

BEGIN_PROPERTY(ListItem_IsValid)

	GB.ReturnBoolean(!!CHECK_CURRENT());

END_PROPERTY

GB_DESC CListItem[] = {
	GB_DECLARE_VIRTUAL(".List.Item"),

	GB_METHOD("Append", NULL, ListItem_Append, "(Value)v"),
	GB_METHOD("Prepend", NULL, ListItem_Prepend, "(Value)v"),

	GB_PROPERTY_READ("IsFirst", "b", ListItem_IsFirst),
	GB_PROPERTY_READ("IsLast", "b", ListItem_IsLast),
	GB_PROPERTY_READ("IsValid", "b", ListItem_IsValid),
	GB_PROPERTY_READ("Index", "i", ListItem_Index),
	GB_PROPERTY("Value", "v", ListItem_Value),

	GB_END_DECLARE
};
