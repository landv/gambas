/*
 * list.h - Embedded lists C interface
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

#ifndef __LIST_H
#define __LIST_H

#include "gambas.h"

typedef struct list {
	struct list *prev, *next;
} LIST;

#define INIT_LIST(name)		{&name, &name}
#define DECLARE_LIST(name)	LIST name = INIT_LIST(name)

#define LIST_data(list, type, member)	\
	((type *) ((char *) (list) - offsetof(type, member)))

/* All but the entry node */

#define list_for_each(_node, _list)					\
	for (_node = (_list)->next; _node != (_list); _node = _node->next)

#define list_for_each_prev(_node, _list)				\
	for (_node = (_list)->prev; _node != (_list); _node = _node->prev)

/* All, including the entry node (at the beginning of the loop) */

#define list_for_each_first(_node, _list, _c)				\
	for (_node = (_list), _c = 1; _node != (_list) || _c--;		\
		_node = _node->next)

#define list_for_each_prev_first(_node, _list, _c)			\
	for (_node = (_list), _c = 1; _node != (_list) || _c--;		\
		_node = _node->prev)

/* All but entry node, may modify the list */

#define list_for_each_safe(_node, _list, _next)				\
	for (_node = (_list)->next, _next = _node->next; _node != (_list);\
		_node = _next, _next = _node->next)

static inline void LIST_init(LIST *list)
{
	list->prev = list->next = list;
}

static inline int LIST_is_empty(LIST *list)
{
	return list->next == list;
}

static inline void LIST_prepend(LIST *list, LIST *new)
{
	register LIST *new_end = new->prev;

	list->prev->next = new;
	new->prev = list->prev;
	new_end->next = list;
	list->prev = new_end;
}

static inline void LIST_append(LIST *list, LIST *new)
{
	register LIST *new_end = new->prev;

	new_end->next = list->next;
	list->next->prev = new_end;
	new->prev = list;
	list->next = new;
}

static inline LIST *LIST_unlink(LIST *node)
{
	node->prev->next = node->next;
	node->next->prev = node->prev;
	node->prev = node->next = node;
	return node;
}

#endif /* !__LIST_H */
