/*
 * c_list.c - Circular double-linked lists
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

#include "gambas.h"
#include "list.h"
#include "c_list.h"

#if 0
#define CHUNK_SIZE	16
#define CHUNK_SKIP	 2

/*
 * List implementation properties:
 * - Speed up long traversals by maintaining links to the CHUNK_SKIP'th
 *   chunk forwards and backwards.
 * - Increase cache locality by saving CHUNK_SIZE values inside a single
 *   chunk.
 * Note that the two values must be chosen reasonably _together_.
 */

typedef struct {
	LIST list;
	LIST skip;
	struct {	/* We don't want to bother reordering the array when
			   elements are removed in the middle so we mark
			   valid entries. */
		GB_VARIANT_VALUE val;
		int is_valid : 1;
	} var[CHUNK_SIZE];
	int avail;	/* First available in var */
	int highest;	/* Highest available in var; will only be reduced
			   when elements are removed from the end of the
			   list */
	int used;	/* Number of actually used elements, will reduce
			   when elements are removed in the middle of var */
} CHUNK;
#endif

typedef struct {
	LIST list;
	GB_VARIANT_VALUE val;
} CHUNK;
#define get_chunk(node)		LIST_data(node, CHUNK, list)

typedef struct {
	GB_BASE ob;
	LIST list;	/* Beginning of linked CHUNKs */
	CHUNK *current;	/* Current element */
	CHUNK *found;	/* Last found chunk */
	int count;	/* Do not iterate over all elements to get this */
} CLIST;

static void CHUNK_init(CHUNK *ck)
{
	int i;

	LIST_init(&ck->list);
	ck->val.type = GB_T_NULL;
}

static CHUNK *CHUNK_new(void)
{
	CHUNK *new;

	GB.Alloc((void **) &new, sizeof(*new));
	CHUNK_init(new);
	return new;
}

static void CHUNK_free_all(CHUNK *ck)
{
	GB.StoreVariant(NULL, &ck->val);
}

static void CHUNK_destroy(CHUNK *ck)
{
	/* We require that the chunk is unlinked */
	CHUNK_free_all(ck);
	GB.Free((void **) &ck);
}

#define THIS	((CLIST *) _object)

BEGIN_METHOD_VOID(List_new)

	LIST_init(&THIS->list);
	THIS->current = NULL;
	THIS->found = NULL;
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

/*
 * Modify @ck and @index so that they point to the next valid value. @first
 * is used to detect the end of an enumeration, i.e. where no other elements
 * can be found. In this case, NULL is written to @ck.
 */

/*
 * Currently, @index is not used, see CHUNK definitions above.
 */

static void CHUNK_next_val(CLIST *list, CHUNK *first, CHUNK **ck,int *index)
{
	register CHUNK *c;
	LIST *node;

	if (LIST_is_empty(&list->list)) {
		*ck = NULL;
		return;
	}
	if ((node = (*ck)->list.next) == &list->list)
		node = node->next;
	c = get_chunk(node);
	if (c == first)
		*ck = NULL;
	else
		*ck = c;
}

static void CHUNK_prev_val(CLIST *list, CHUNK *first, CHUNK **ck,int *index)
{
	register CHUNK *c = *ck;
	LIST *node;

	if (LIST_is_empty(&list->list)) {
		*ck = NULL;
		return;
	}
	if ((node = (*ck)->list.prev) == &list->list)
		node = node->prev;
	c = get_chunk(node);
	if (c == first)
		*ck = NULL;
	else
		*ck = c;
}

struct enum_state {
	CHUNK *first, *next;	/* Be able to Take() in For Each and resume
				   with the right Current when BREAKing
				   nested enumerations */
	int index;
};

/*
 * XXX: Breaking nested enumerations may confuse user's view of Current.
 *      After a broken nested enumeration, the outer enumeration inherits
 *      the Current from the nested loop but after this enumeration, it
 *      continues with the one it had originally next... - Bad!
 */

BEGIN_METHOD_VOID(List_next)

	struct enum_state *state = GB.GetEnum();
	CHUNK *cur, *next;

	if (!*((char *) state)) { /* Beginning */
		/* No elements? */
		if (LIST_is_empty(&THIS->list))
			goto no_enum;
		cur = next = get_chunk(THIS->current);
		state->first = cur;
	} else {
		cur = next = state->next;
		if (!cur)
			goto stop_enum;
	}

	CHUNK_next_val(THIS, state->first, &next, NULL);
	state->next = next;
	THIS->current = cur;
	GB.ReturnVariant(&cur->val);
	return;

stop_enum:
no_enum:
	GB.StopEnum();
	return;

END_METHOD

static CHUNK *CLIST_get(CLIST *list, int index)
{
	LIST *node;

	list_for_each(node, &list->list) {
		if (!index)
			break;
		index--;
	}
	if (index) /* Not enough elements */
		return NULL;
	return get_chunk(node);
}

BEGIN_METHOD(List_get, GB_INTEGER index)

	CHUNK *ck = CLIST_get(THIS, VARG(index));

	if (!ck) {
		GB.Error(GB_ERR_BOUND);
		return;
	}
	GB.ReturnVariant(&ck->val);

END_METHOD

BEGIN_METHOD(List_put, GB_VARIANT var; GB_INTEGER index)

	CHUNK *ck;

	ck = CLIST_get(THIS, VARG(index));
	if (!ck) {
		GB.Error(GB_ERR_BOUND);
		return;
	}
	GB.StoreVariant(ARG(var), &ck->val);

END_METHOD

/*
 * XXX: Append and Prepend may confuse the user's view of the Current
 *      property. The first inserted elements get the Current status,
 *      however, it remains Current unless one of the Move*() methods is
 *      invoked - no matter if any other nodes are Prepend()'d.
 */

static void CLIST_append(CLIST *list, GB_VARIANT *val)
{
	CHUNK *ck = CHUNK_new();

	GB.StoreVariant(val, &ck->val);
	LIST_append(&list->list, &ck->list);
	if (!list->count)
		list->current = ck;
	list->count++;
}

BEGIN_METHOD(List_Append, GB_VARIANT value)

	CLIST_append(THIS, ARG(value));

END_METHOD

static void CLIST_prepend(CLIST *list, GB_VARIANT *val)
{
	CHUNK *ck = CHUNK_new();

	GB.StoreVariant(val, &ck->val);
	LIST_prepend(&list->list, &ck->list);
	if (!list->count)
		list->current = ck;
	list->count++;
}

BEGIN_METHOD(List_Prepend, GB_VARIANT value)

	CLIST_prepend(THIS, ARG(value));

END_METHOD

static GB_VARIANT_VALUE *CLIST_take(CLIST *list, CHUNK *ck)
{
	GB_VARIANT_VALUE *val = &ck->val;
	LIST *node;
	CHUNK *new;

	/*
	 * Do we operate on the cursor itself? Set a new one. The cursor
	 * remains always relative to the end - arbitrarily.
	 */
	if (ck == list->current) {
		/* At End? */
		if ((node = list->current->list.next) == &list->list)
			node = list->current->list.prev;
		if (node == &list->list) /* Empty */
			list->current = NULL;
		else
			list->current = get_chunk(node);
	}

	/* TODO: Tell enumerations of gone nodes */

	LIST_unlink(&ck->list);
	list->count--;
	return val;
}

#define CHECK_CURRENT()				\
	if (!THIS->current) {			\
		GB.Error("No current element");	\
		return;				\
	}

BEGIN_METHOD(List_Take, GB_INTEGER index)

	CHUNK *ck;

	if (MISSING(index)) {
		CHECK_CURRENT();
		ck = THIS->current;
	} else {
		ck = CLIST_get(THIS, VARG(index));
		if (!ck) {
			GB.Error(GB_ERR_BOUND);
			return;
		}
	}
	GB.ReturnVariant(CLIST_take(THIS, ck));
	GB.ReturnBorrow();
	GB.StoreVariant(NULL, &ck->val);
	GB.ReturnRelease();
	CHUNK_destroy(ck);

END_METHOD

#define IMPLEMENT_Move(which, magic, magic2)		\
BEGIN_METHOD_VOID(List_Move ## which)			\
							\
	LIST *node;					\
							\
	magic;						\
	if (node == &THIS->list) {			\
		magic2;					\
	}						\
	THIS->current = get_chunk(node);		\
							\
END_METHOD

IMPLEMENT_Move(Next, CHECK_CURRENT(); node = THIS->current->list.next,
	node = node->next)
IMPLEMENT_Move(Prev, CHECK_CURRENT(); node = THIS->current->list.prev,
	node = node->prev)
IMPLEMENT_Move(First, node = THIS->list.next,
	node = node->next)
IMPLEMENT_Move(Last, node = THIS->list.prev,
	node = node->prev)

/* XXX: Quite cumbersome... */
static int CLIST_find_forward(CLIST *list, LIST *start, GB_VARIANT *val)
{
	LIST *node;
	CHUNK *ck;
	int i = 0;

	/* We need to get the index */
	list_for_each(node, &list->list) {
		if (node == start) {
			i++; break;
		}
		i++;
	}
	list_for_each(node, start) {
		if (node == &list->list) {
			i = 0; continue;
		}
		ck = get_chunk(node);
		if (!GB.CompVariant(&val->value, &ck->val))
			goto found;
		i++;
	}
	/* The @start at last */
	if (start != &list->list) {
		ck = get_chunk(start);
		if (!GB.CompVariant(&val->value, &ck->val))
			goto found;
	}
	list->found = NULL;
	return -1;

found:
	list->found = ck;
	return i;
}

static int CLIST_find_backward(CLIST *list, LIST *start, GB_VARIANT *val)
{
	LIST *node;
	CHUNK *ck;
	int i = 0;

	list_for_each_prev(node, &list->list) {
		if (node == start) {
			i++; break;
		}
		i++;
	}
	list_for_each_prev(node, start) {
		if (node == &list->list) {
			i = 0; continue;
		}
		ck = get_chunk(node);
		if (!GB.CompVariant(&val->value, &ck->val))
			goto found;
		i++;
	}
	if (start != &list->list) {
		ck = get_chunk(start);
		if (!GB.CompVariant(&val->value, &ck->val))
			goto found;
	}
	list->found = NULL;
	return -1;

found:
	list->found = ck;
	return list->count - i - 1;
}

#define IMPLEMENT_Find(which, which2, magic)				\
									\
BEGIN_METHOD(List_Find ## which, GB_VARIANT value)			\
									\
	LIST *node;							\
									\
	magic;								\
	GB.ReturnInteger(CLIST_find_ ## which2 (THIS, node, ARG(value)));\
									\
END_METHOD

IMPLEMENT_Find(Next, forward,
	node = THIS->found ? &THIS->found->list : &THIS->list)
IMPLEMENT_Find(Prev, backward,
	node = THIS->found ? &THIS->found->list : &THIS->list)
IMPLEMENT_Find(First, forward, node = THIS->list.next)
IMPLEMENT_Find(Last, backward, node = THIS->list.prev)

BEGIN_PROPERTY(List_Current)

	CHECK_CURRENT();
	if (READ_PROPERTY) {
		GB.ReturnVariant(&THIS->current->val);
		return;
	}
	GB.StoreVariant(PROP(GB_VARIANT), &THIS->current->val);

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

	GB_METHOD("FindNext", "i", List_FindNext, "(Value)v"),
	GB_METHOD("FindPrev", "i", List_FindPrev, "(Value)v"),
	GB_METHOD("FindPrevious", "i", List_FindPrev, "(Value)v"),
	GB_METHOD("FindFirst", "i", List_FindFirst, "(Value)v"),
	GB_METHOD("FindLast", "i", List_FindLast, "(Value)v"),

	GB_PROPERTY("Current", "v", List_Current),
	GB_PROPERTY_READ("Count", "i", List_Count),

	GB_END_DECLARE
};
