/*
 * c_deque.c - Deque, FILO and FIFO types
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

#define __C_DEQUE_C

#include "../gambas.h"
#include "c_list.h"
#include "c_deque.h"

typedef struct {
	GB_VARIANT_VALUE var;
	CLIST *list;
} CDEQUE_ELEM;

static CDEQUE_ELEM *CDEQUE_new_elem(GB_VARIANT *variant)
{
	CDEQUE_ELEM *new;

	GB.Alloc((void **) &new, sizeof(*new));
	new->var.type = GB_T_NULL;
	new->list = List.New(new);
	GB.StoreVariant(variant, &new->var);
	return new;
}

static void CDEQUE_real_destroy_elem(CDEQUE_ELEM *elem)
{
	GB.StoreVariant(NULL, &elem->var);
	List.Destroy(elem->list);
	GB.Free((void **) &elem);
}

static void CDEQUE_destroy_elem(CDEQUE_ELEM *elem)
{
	List.Unlink(elem->list);
	/* FIXME: This is a 'horrible hack' */
	GB.Post(CDEQUE_real_destroy_elem, (intptr_t) elem);
}

typedef struct {
	GB_BASE ob;
	CLIST *elements;
} CDEQUE;

static int CDEQUE_is_empty(CDEQUE *dq)
{
	return List.IsEmpty(dq->elements);
}

static void CDEQUE_init(CDEQUE *dq)
{
	dq->elements = List.NewRoot();
}

#define THIS	((CDEQUE *) _object)

BEGIN_METHOD_VOID(Deque_new)

	CDEQUE_init(THIS);

END_METHOD

static CDEQUE_ELEM *CDEQUE_pop_front(CDEQUE *dq)
{
	CDEQUE_ELEM *elem;

	if (CDEQUE_is_empty(dq))
		return NULL;
	elem = dq->elements->next->data;
	CDEQUE_destroy_elem(elem);
	return elem;
}

static void CDEQUE_pop_and_free_all(CDEQUE *dq)
{
	while (!CDEQUE_is_empty(dq))
		CDEQUE_pop_front(dq);
}

BEGIN_METHOD_VOID(Deque_free)

	CDEQUE_pop_and_free_all(THIS);
	List.DestroyRoot(THIS->elements);

END_METHOD

static void CDEQUE_push_front(CDEQUE *dq, CDEQUE_ELEM *elem)
{
	List.AddAfter(dq->elements, elem->list);
}

BEGIN_METHOD(Deque_PushFront, GB_VARIANT value)

	CDEQUE_ELEM *elem;

	elem = CDEQUE_new_elem(ARG(value));
	CDEQUE_push_front(THIS, elem);

END_METHOD

static void CDEQUE_push_back(CDEQUE *dq, CDEQUE_ELEM *elem)
{
	List.AddBefore(dq->elements, elem->list);
}

BEGIN_METHOD(Deque_PushBack, GB_VARIANT value)

	CDEQUE_ELEM *elem;

	elem = CDEQUE_new_elem(ARG(value));
	CDEQUE_push_back(THIS, elem);

END_METHOD

BEGIN_METHOD_VOID(Deque_PopFront)

	CDEQUE_ELEM *elem;

	if (CDEQUE_is_empty(THIS)) {
		GB.ReturnNull();
		GB.ReturnConvVariant();
		return;
	}
	elem = CDEQUE_pop_front(THIS);
	GB.ReturnVariant(&elem->var);

END_METHOD

static CDEQUE_ELEM *CDEQUE_pop_back(CDEQUE *dq)
{
	CDEQUE_ELEM *elem;

	if (CDEQUE_is_empty(dq))
		return NULL;
	elem = dq->elements->prev->data;
	CDEQUE_destroy_elem(elem);
	return elem;
}

BEGIN_METHOD_VOID(Deque_PopBack)

	CDEQUE_ELEM *elem;

	if (CDEQUE_is_empty(THIS)) {
		GB.ReturnNull();
		GB.ReturnConvVariant();
		return;
	}
	elem = CDEQUE_pop_back(THIS);
	GB.ReturnVariant(&elem->var);

END_METHOD

BEGIN_METHOD_VOID(Deque_PeekFront)

	CDEQUE_ELEM *elem;

	if (CDEQUE_is_empty(THIS)) {
		GB.ReturnNull();
		GB.ReturnConvVariant();
	} else {
		elem = (CDEQUE_ELEM *) THIS->elements->next->data;
		GB.ReturnVariant(&elem->var);
	}

END_METHOD

BEGIN_METHOD_VOID(Deque_PeekBack)

	CDEQUE_ELEM *elem;

	if (CDEQUE_is_empty(THIS)) {
		GB.ReturnNull();
		GB.ReturnConvVariant();
	} else {
		elem = (CDEQUE_ELEM *) THIS->elements->prev->data;
		GB.ReturnVariant(&elem->var);
	}

END_METHOD

BEGIN_METHOD_VOID(Deque_Clear)

	CDEQUE_pop_and_free_all(THIS);

END_METHOD

BEGIN_PROPERTY(Deque_IsEmpty)

	GB.ReturnBoolean(CDEQUE_is_empty(THIS));

END_PROPERTY

BEGIN_PROPERTY(Deque_Size)

	size_t size;
	CLIST *node;

	size = 0;
	clist_for_each(node, THIS->elements)
		size++;
	GB.ReturnInteger(size);

END_PROPERTY

/*
 * Double-ended queue
 */

GB_DESC CDequeDesc[] = {
	GB_DECLARE("Deque", sizeof(CDEQUE)),

	GB_METHOD("_new", NULL, Deque_new, NULL),
	GB_METHOD("_free", NULL, Deque_free, NULL),

	GB_METHOD("PushFront", NULL, Deque_PushFront, "(Value)v"),
	GB_METHOD("PushBack", NULL, Deque_PushBack, "(Value)v"),
	GB_METHOD("PopFront", "v", Deque_PopFront, NULL),
	GB_METHOD("PopBack", "v", Deque_PopBack, NULL),
	GB_METHOD("PeekFront", "v", Deque_PeekFront, NULL),
	GB_METHOD("PeekBack", "v", Deque_PeekBack, NULL),

	GB_METHOD("Clear", NULL, Deque_Clear, NULL),

	GB_PROPERTY_READ("IsEmpty", "b", Deque_IsEmpty),
	GB_PROPERTY_READ("Size", "i", Deque_Size),

	GB_END_DECLARE
};

/*
 * FILO
 */

GB_DESC CStackDesc[] = {
	GB_DECLARE("Stack", sizeof(CDEQUE)),

	GB_METHOD("_new", NULL, Deque_new, NULL),
	GB_METHOD("_free", NULL, Deque_free, NULL),

	GB_METHOD("Push", NULL, Deque_PushBack, "(Value)v"),
	GB_METHOD("Pop", "v", Deque_PopBack, NULL),
	GB_METHOD("Peek", "v", Deque_PeekBack, NULL),

	GB_METHOD("Clear", NULL, Deque_Clear, NULL),

	GB_PROPERTY_READ("IsEmpty", "b", Deque_IsEmpty),
	GB_PROPERTY_READ("Size", "i", Deque_Size),

	GB_END_DECLARE
};

/*
 * FIFO
 */

GB_DESC CQueueDesc[] = {
	GB_DECLARE("Queue", sizeof(CDEQUE)),

	GB_METHOD("_new", NULL, Deque_new, NULL),
	GB_METHOD("_free", NULL, Deque_free, NULL),

	GB_METHOD("Enqueue", NULL, Deque_PushBack, "(Value)v"),
	GB_METHOD("Enq", NULL, Deque_PushBack, "(Value)v"),
	GB_METHOD("Dequeue", "v", Deque_PopFront, NULL),
	GB_METHOD("Deq", "v", Deque_PopFront, NULL),
	GB_METHOD("Peek", "v", Deque_PeekFront, NULL),

	GB_METHOD("Clear", NULL, Deque_Clear, NULL),

	GB_PROPERTY_READ("IsEmpty", "b", Deque_IsEmpty),
	GB_PROPERTY_READ("Size", "i", Deque_Size),

	GB_END_DECLARE
};

/*
 * We keep the Priority queue sorted by a special Enqueue() function.
 * The higher the priority, the closer to the beginning the value will be
 * enqueued. For equal priorities, the later added element will be enqueued
 * at a higher index.
 */

static void CPRIOQ_enqueue(CDEQUE *dq, CDEQUE_ELEM *elem, int prio)
{
	CLIST *next;

	/* Just find the right node to prepend the new value to */
	clist_for_each(next, dq->elements)
		/* We use CLIST's uptr for the prio */
		if (prio > (int) next->uptr)
			break;
	elem->list->uptr = prio;
	List.AddBefore(next, elem->list);
}

BEGIN_METHOD(PrioQueue_Enqueue, GB_VARIANT value; GB_INTEGER prio)

	CDEQUE_ELEM *elem;

	elem = CDEQUE_new_elem(ARG(value));
	CPRIOQ_enqueue(THIS, elem, VARG(prio));

END_METHOD

/*
 * Priority FIFO
 */

GB_DESC CPrioQueueDesc[] = {
	GB_DECLARE("PrioQueue", sizeof(CDEQUE)),

	GB_METHOD("_new", NULL, Deque_new, NULL),
	GB_METHOD("_free", NULL, Deque_free, NULL),

	GB_METHOD("Enqueue", NULL, PrioQueue_Enqueue, "(Value)v(Prio)i"),
	GB_METHOD("Enq", NULL, PrioQueue_Enqueue, "(Value)v(Prio)i"),
	GB_METHOD("Dequeue", "v", Deque_PopFront, NULL),
	GB_METHOD("Deq", "v", Deque_PopFront, NULL),
	GB_METHOD("Peek", "v", Deque_PeekFront, NULL),

	GB_METHOD("Clear", NULL, Deque_Clear, NULL),

	GB_PROPERTY_READ("IsEmpty", "b", Deque_IsEmpty),
	GB_PROPERTY_READ("Size", "i", Deque_Size),

	GB_END_DECLARE
};
