/*
 * c_list.h
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

#ifndef __C_LIST_H
#define __C_LIST_H

#include "../gambas.h"

extern GB_INTERFACE GB;

typedef struct clist {
	union {
		GB_BASE ob; /* C functions maintain the memory themselves */
		intptr_t uptr; /* They get a private data pointer instead */
	};
	struct clist *prev, *next;
	void *data;
	int embedded : 1;
	int has_link_ref : 1;
} CLIST;

typedef struct {
	CLIST *(*New)(void *);
#define DECLARE_CLIST(name, obj) \
	CLIST name = {		\
		.uptr = (intptr_t) NULL, \
		.prev = &name,	\
		.next = &name,	\
		.data = obj,	\
		.embedded = 0,	\
		.has_link_ref = 0\
	}

	void  *(*Destroy)(CLIST *);
	CLIST *(*NewRoot)(void);
#define DECLARE_CLIST_ROOT(name) \
	CLIST name = {		\
		.uptr = (intptr_t) NULL, \
		.prev = &name,	\
		.next = &name,	\
		.data = &name,	\
		.embedded = 0,	\
		.has_link_ref = 0 \
	}

	void   (*DestroyRoot)(CLIST *);
	int    (*IsRoot)(CLIST *);
	int    (*IsEmpty)(CLIST *);
	void   (*AddBefore)(CLIST *, CLIST *);
	void   (*AddAfter)(CLIST *, CLIST *);
	void   (*Unlink)(CLIST *);
	CLIST *(*Extract)(CLIST *, int);
} CLIST_INTF;

#define clist_for_each(node, list)	\
	for (node = (list)->next; node != (list); node = node->next)

#define clist_for_each_first(node, list, c)	\
	for (node = (list), c = 1; node != (list) || c--; node = node->next)

#define clist_for_each_safe(node, list, next)	\
	for (node = (list)->next, next = node->next; node != (list);	\
		node = next, next = node->next)


#ifndef __C_LIST_C
extern GB_DESC CListDesc[];
extern GB_DESC CListRootDesc[];
extern CLIST_INTF List;

extern void CLIST_exit(void);
#endif

#endif /* !__C_LIST_H */
