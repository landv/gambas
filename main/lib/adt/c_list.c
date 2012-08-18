/*
 * c_list.c - (Embedded) circular double-linked lists
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
#include "c_list.h"

static DECLARE_CLIST_ROOT(free_nodes);

/* Component unload. Free all re-use nodes */
void CLIST_exit(void)
{
	CLIST *node, *next;

	clist_for_each_safe(node, &free_nodes, next)
		GB.Free((void **) &node);
}

static int CLIST_is_root(CLIST *list)
{
	/* Root nodes have their data member point to their structure. This
	 * means that they carry no information and that is pointless for
	 * real nodes. */
	return list->data == list;
}

static int CLIST_is_empty(CLIST *list)
{
	return list->next == list;
}

static int CLIST_is_linked(CLIST *node)
{
	return node->has_link_ref;
}

static void CLIST_init(CLIST *node, void *data)
{
	node->prev = node->next = node;
	node->data = data;
}

/* Forward */
static CLIST *CLIST_extract(CLIST *, int);
static void CLIST_add_after(CLIST *, CLIST *);
static void CLIST_unlink(CLIST *);

static CLIST *CLIST_new(void *data)
{
	CLIST *new;

	/* Try to get a recycled node first */
	if (!CLIST_is_empty(&free_nodes))
		new = CLIST_extract(free_nodes.next, 1);
	else
		GB.Alloc((void **) &new, sizeof(*new));
	CLIST_init(new, data);
	return new;
}

static void *CLIST_destroy(CLIST *node)
{
	void *data = node->data;

	/* Recycle */
	CLIST_unlink(node);
	CLIST_add_after(&free_nodes, node);
	return data;
}

static void CLIST_init_root(CLIST *node)
{
	node->prev = node->next = node->data = node;
}

static CLIST *CLIST_new_root(void)
{
	CLIST *new;

	new = CLIST_new(NULL);
	CLIST_init_root(new);
	return new;
}

static void CLIST_destroy_root(CLIST *node)
{
	CLIST_destroy(node);
}

#define THIS	((CLIST *) _object)

BEGIN_METHOD(List_new, GB_OBJECT obj; /* Usually 'Me' from container obj */
		       GB_BOOLEAN embedded) /* obj is container of THIS? */

	int embedded = VARGOPT(embedded, 0);

	CLIST_init(THIS, VARG(obj));
	THIS->embedded = embedded;
	THIS->has_link_ref = 0;
	/* If the list embedded, we must not reference the container as that
	 * would cause not-so-easy to resolve circular references and thus
	 * memory errors. */
	if (!embedded)
		GB.Ref(THIS->data);

END_METHOD

BEGIN_METHOD_VOID(List_free)

	if (!THIS->embedded)
		GB.Unref(&THIS->data);

END_METHOD

static CLIST *CLIST_next_node(CLIST *node)
{
	CLIST *next;

	if (CLIST_is_empty(node))
		return NULL;

	next = node->next;
	/* Erroneously more root nodes in the list? It should not be
	 * possible but catch it with that loop nevertheless. */
	while (CLIST_is_root(next)) {
		next = next->next;
		if (next == node)
			return NULL;
	}
	return next;
}

/* I prefer storing a pointer to this struct in GB.GetEnum() */
struct enum_state {
	CLIST *first, *next; /* Be able to Unlink() in For Each */
};

/*
 * We can either enumerate a list of embedded list nodes which have no root
 * node or a list containing a root node which is then skipped.
 * In both cases, we can enter the enumeration at any node in the list and
 * all nodes in the list are enumerated.
 */

BEGIN_METHOD_VOID(List_next)

	struct enum_state **statep = GB.GetEnum(), *state;
	CLIST *cur;

	state = *statep;
	if (!state) { /* Beginning */
		GB.Alloc((void **) statep, sizeof(*state));
		state = *statep;
		/* Catch the 'root' case and skip the node */
		if (CLIST_is_root(THIS))
			cur = CLIST_next_node(THIS);
		else
			cur = THIS;
		if (!cur)
			goto stop_enum;
		state->first = cur;
		goto done;
	}

	cur = state->next;
	if (!cur || cur == state->first)
		goto stop_enum;

done:
	state->next = CLIST_next_node(cur);
	GB.ReturnObject(cur->data);
	return;

stop_enum:
	GB.Free((void **) statep);
	GB.StopEnum();
	return;

END_METHOD

/*
 * There are no particular 'nodes' when adding because each node is a fully
 * capable list itself. When we pass a @new node, we always add the entire
 * list it is currently linked to. In particular this is a yet unlinked list
 * containing only one node in most cases but it is possible to create
 * sub-lists individually and add some sub-lists to a global root later.
 */

static void CLIST_add_before(CLIST *list, CLIST *new)
{
	CLIST *new_end = new->prev;

	list->prev->next = new;
	new->prev = list->prev;
	new_end->next = list;
	list->prev = new_end;
}

static void CLIST_add_after(CLIST *list, CLIST *new)
{
	CLIST *new_end = new->prev;

	new_end->next = list->next;
	list->next->prev = new_end;
	new->prev = list;
	list->next = new;
}

static void CLIST_ref_once(CLIST *node)
{
	if (!CLIST_is_linked(node)) {
		node->has_link_ref = 1;
		/* ListRoots do not receive ref. This makes almost automatic
		 * cleanup possible. The list may be fully linked but when
		 * the root loses its last ref, the list is cleared. */
		if (CLIST_is_root(node))
			return;
		GB.Ref(node);
		/* It is time now, at the latest, to Ref() the object of an
		 * embedded node because all other refs could get lost */
		if (node->embedded)
			GB.Ref(node->data);
	}
}

static void CLIST_unref_once(CLIST **node)
{
	if (CLIST_is_linked(*node)) {
		(*node)->has_link_ref = 0;
		if (CLIST_is_root(*node))
			return;
		/* Take the ref off from data of embedded node again */
		if ((*node)->embedded)
			GB.Unref(&(*node)->data);
		GB.Unref((void **) node);
	}
}

enum {
	CLIST_BEFORE,
	CLIST_AFTER
};

static void CLIST_add_and_ref(CLIST *node, CLIST *new, int mode)
{
	if (mode == CLIST_BEFORE)
		CLIST_add_before(node, new);
	else /* This function is save from passing invalid modes */
		CLIST_add_after(node, new);
	/* Each node gets Ref()'d once when in a list. */
	CLIST_ref_once(node);
	CLIST_ref_once(new);
}

/* @node and @buf should be variables */
#define CHECK_ADD_ROOT(node, new, buf)				\
	do {							\
		CLIST *_new = (new);				\
		clist_for_each_first((node), _new, (buf)) {	\
			if (CLIST_is_root((node))) {		\
				GB.Error("Attempt to Add a root node."); \
				return;				\
			}					\
		}						\
	} while (0)

BEGIN_METHOD(List_AddPrev, GB_OBJECT new)

	CLIST *new = (CLIST *) VARG(new), *node;
	int buf;

	CHECK_ADD_ROOT(node, new, buf);
	CLIST_add_and_ref(THIS, new, CLIST_BEFORE);

END_METHOD

BEGIN_METHOD(List_AddNext, GB_OBJECT new)

	CLIST *new = (CLIST *) VARG(new), *node;
	int buf;

	CHECK_ADD_ROOT(node, new, buf);
	CLIST_add_and_ref(THIS, new, CLIST_AFTER);

END_METHOD

/*
 * This operates on the exact node.
 */

static void CLIST_unlink(CLIST *node)
{
	node->prev->next = node->next;
	node->next->prev = node->prev;
	/* Link to itself again to form a valid empty list */
	node->prev = node->next = node;
}

static void CLIST_unlink_and_unref(CLIST *node)
{
	CLIST *list;

	/* list is only meaningful if the list gets empty after this
	 * Unlink() so it doesn't matter if THIS->prev or THIS->next
	 * is used. */
	list = node->prev;
	CLIST_unlink(node);
	CLIST_unref_once(&node);

	if (CLIST_is_empty(list))
		CLIST_unref_once(&list);
}

#define CHECK_NOT_LINKED(node)			\
	do {					\
		if (!CLIST_is_linked(node)) {	\
			GB.Error("List node not linked"); \
			return;			\
		}				\
	} while (0)

BEGIN_METHOD_VOID(List_Unlink)

	CHECK_NOT_LINKED(THIS);
	CLIST_unlink_and_unref(THIS);

END_METHOD

/*
 * Extract a sub-list from a list. The @start gets returned if successful.
 */

static CLIST *CLIST_extract(CLIST *start, int count)
{
	int i = count, buf;
	CLIST *last;

	if (!count)
		return NULL;

	clist_for_each_first(last, start, buf) {
		if (!--i)
			break;
	}
	/* Too few nodes in the entire list? */
	if (i)
		return NULL;

	/* Unlink new list */
	start->prev->next = last->next;
	last->next->prev = start->prev;
	/* Relink new list circularly to itself */
	start->prev = last;
	last->next = start;
	return start;
}

BEGIN_METHOD(List_Extract, GB_INTEGER count)

	int count = VARG(count);
	CLIST *node;
	CLIST *start;

	if (!count) {
		GB.Error(GB_ERR_ARG);
		return;
	}

	CHECK_NOT_LINKED(THIS);

	if (count == 1) { /* Is actually Unlink()? */
		GB.ReturnObject(THIS); /* Prevent from vanish */
		CLIST_unlink_and_unref(THIS);
	} else { /* Real Extract() */
		node = THIS->prev;
		start = CLIST_extract(THIS, count);
		if (!start) {
			GB.Error(GB_ERR_BOUND);
			return;
		}
		GB.ReturnObject(start);
		/* Made old list empty? */
		if (CLIST_is_empty(node))
			CLIST_unref_once(&node);
	}

END_METHOD

static void CLIST_unlink_and_unref_all(CLIST *list)
{
	if (!CLIST_is_linked(list))
		return;

	while (!CLIST_is_empty(list))
		CLIST_unlink_and_unref(list->next);
	/* We get automatically Unref()'d by CLIST_unlink_and_unref() */
}

BEGIN_METHOD_VOID(List_Clear)

	/*
	 * This must be called to prevent circular references at program
	 * termination. It is tricky to avoid circular references on
	 * circular linked lists. Therefore, alas, this function must be
	 * called explicitly or all containing nodes must be removed another
	 * way (see ListRoot_free).
	 */
	CLIST_unlink_and_unref_all(THIS);

END_METHOD

BEGIN_PROPERTY(List_Next)

	GB.ReturnObject(THIS->next);

END_PROPERTY

BEGIN_PROPERTY(List_Previous)

	GB.ReturnObject(THIS->prev);

END_PROPERTY

BEGIN_PROPERTY(List_IsLinked)

	GB.ReturnBoolean(CLIST_is_linked(THIS));

END_PROPERTY

BEGIN_PROPERTY(List_IsEmbedded)

	GB.ReturnBoolean(THIS->embedded);

END_PROPERTY

BEGIN_PROPERTY(List_Data)

	if (READ_PROPERTY) {
		GB.ReturnObject(THIS->data);
		return;
	}
	if (THIS->embedded) {
		GB.Error("Attempt to change Data on embedded node");
		return;
	}
	GB.Unref((void **) &THIS->data);
	THIS->data = VPROP(GB_OBJECT);
	GB.Ref(THIS->data);

END_PROPERTY

GB_DESC CListDesc[] = {
	GB_DECLARE("List", sizeof(CLIST)),

	GB_CONSTANT("Embedded", "b", 1),

	GB_METHOD("_new", NULL, List_new, "(Obj)o[(Embedded)b]"),
	GB_METHOD("_free", NULL, List_free, NULL),
	GB_METHOD("_next", "o", List_next, NULL),

	GB_METHOD("AddPrev", NULL, List_AddPrev, "(Node)List;"),
	GB_METHOD("AddNext", NULL, List_AddNext, "(Node)List;"),
	GB_METHOD("Unlink", NULL, List_Unlink, NULL),
	GB_METHOD("Extract", "List", List_Extract, "(Count)i"),
	GB_METHOD("Clear", NULL, List_Clear, NULL),

	GB_PROPERTY_READ("Next", "List", List_Next),
	GB_PROPERTY_READ("Previous", "List", List_Previous),
	GB_PROPERTY_READ("Prev", "List", List_Previous),
	GB_PROPERTY_READ("IsLinked", "b", List_IsLinked),
	GB_PROPERTY_READ("IsEmbedded", "b", List_IsEmbedded),

	GB_PROPERTY("Data", "o", List_Data),

	GB_END_DECLARE
};

BEGIN_METHOD_VOID(ListRoot_new)

	CLIST_init_root(THIS);
	THIS->embedded = 0;
	THIS->has_link_ref = 0;

END_METHOD

BEGIN_METHOD_VOID(ListRoot_free)

	CLIST_unlink_and_unref_all(THIS);

END_METHOD

BEGIN_PROPERTY(ListRoot_IsEmpty)

	GB.ReturnBoolean(CLIST_is_empty(THIS));

END_PROPERTY

GB_DESC CListRootDesc[] = {
	GB_DECLARE("ListRoot", sizeof(CLIST)),

	GB_METHOD("_new", NULL, ListRoot_new, NULL),
	GB_METHOD("_free", NULL, ListRoot_free, NULL),
	GB_METHOD("_next", "o", List_next, NULL),

	GB_METHOD("AddEnd", NULL, List_AddPrev, "(Node)List;"),
	GB_METHOD("AddStart", NULL, List_AddNext, "(Node)List;"),
	GB_METHOD("Unlink", NULL, List_Unlink, NULL),
	GB_METHOD("Extract", "List", List_Extract, "(Count)i"),
	GB_METHOD("Clear", NULL, List_Clear, NULL),

	GB_PROPERTY_READ("First", "List", List_Next),
	GB_PROPERTY_READ("Last", "List", List_Previous),
	GB_PROPERTY_READ("IsEmpty", "b", ListRoot_IsEmpty),

	GB_END_DECLARE
};

CLIST_INTF List = {
	.New		= CLIST_new,
	.Destroy	= CLIST_destroy,
	.NewRoot	= CLIST_new_root,
	.DestroyRoot	= CLIST_destroy_root,
	.IsRoot		= CLIST_is_root,
	.IsEmpty	= CLIST_is_empty,
	.AddBefore	= CLIST_add_before,
	.AddAfter	= CLIST_add_after,
	.Unlink		= CLIST_unlink,
	.Extract	= CLIST_extract
};
