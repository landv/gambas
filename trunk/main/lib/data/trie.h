/*
 * trie.h
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

#ifndef TRIE_H
#define TRIE_H

#include <stddef.h>
#include <stdint.h>

struct trie {
	uint64_t mask[4]; /* 256 Bits */
	struct trie **children;
	unsigned int nchildren;
	void *value;
	size_t len;
	char key[];
};

enum trie_path {
	TRIE_UNSET = 0, /* No chance to get a match from here */
	TRIE_EXIST,     /* We are within a key                */
	TRIE_EXACT,     /* We got an exact match              */
};

struct trie_prefix {
	enum trie_path state;
	struct trie *node;
	int i;
};

extern struct trie *new_trie(void);
extern void destroy_trie(struct trie *trie, void (*dtor)(void *));
extern void clear_trie(struct trie *trie, void (*dtor)(void *));

extern void *trie_insert(struct trie *trie, const char *key, size_t len,
			void *value);
extern void trie_remove(struct trie *trie, const char *key, size_t len,
			void (*dtor)(void *));

extern struct trie *trie_find(const struct trie *trie, const char *key,
			      size_t len);
extern void *trie_value(const struct trie *trie, const char *key,
			size_t len);

static inline void trie_reset_prefix(struct trie_prefix *p)
{
	p->state = TRIE_UNSET;
	p->node = NULL;
	p->i = 0;
}

extern void trie_constrain(const struct trie *trie,
			   struct trie_prefix *p, char c);
extern void trie_constrain2(const struct trie *trie,
		     	    struct trie_prefix *p, const char *str,
			    size_t len);
extern struct trie *trie_find2(const struct trie *trie,
			       const struct trie_prefix *p,
			       const char *key, size_t len);
extern void *trie_value2(const struct trie *trie,
			 const struct trie_prefix *p,
			 const char *key, size_t len);

#endif /* TRIE_H */
