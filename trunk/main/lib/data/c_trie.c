/*
 * c_trie.c - (Patricia) Trie / Prefix tree
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

#define __C_TRIE_C

#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "gambas.h"
#include "c_trie.h"

#include "trie.h"

typedef struct {
	GB_BASE ob;
	struct trie *root;
	char *key;
	size_t count;
	uint64_t time;
} CTRIE;

#define ERR_OOM	"Out of memory"

#define THIS	((CTRIE *) _object)

#define RESET_TIME()	(THIS->time = 0)
#define UPDATE_TIME()	(THIS->time++)

/**G
 * Create a new, empty Trie.
 */
BEGIN_METHOD_VOID(Trie_new)

	THIS->root = new_trie();
	THIS->key = NULL;
	THIS->count = 0;
	RESET_TIME();

END_METHOD

static void value_dtor(void *val)
{
	GB.StoreVariant(NULL, (GB_VARIANT_VALUE *) val);
	GB.Free(&val);
}

BEGIN_METHOD_VOID(Trie_free)

	destroy_trie(THIS->root, value_dtor);
	GB.FreeString(&THIS->key);
	UPDATE_TIME();

END_METHOD

/**G
 * Return the value associated with a key. If the key was not found, return
 * Null.
 */
BEGIN_METHOD(Trie_get, GB_STRING key)

	GB_VARIANT_VALUE *val;

	val = trie_value(THIS->root, STRING(key), LENGTH(key));
	if (!val)
		GB.ReturnNull();
	else
		GB.ReturnVariant(val);

END_METHOD

/**G
 * Associate a value with a given key. If the value is Null, the key is
 * removed.
 */
BEGIN_METHOD(Trie_put, GB_VARIANT value; GB_STRING key)

	GB_VARIANT_VALUE *val;
	void *oldval;

	if (VARG(value).type == GB_T_NULL) {
		trie_remove(THIS->root, STRING(key), LENGTH(key),
			    value_dtor);
		UPDATE_TIME();
		return;
	}
	GB.Alloc((void **) &val, sizeof(*val));
	val->type = GB_T_NULL;
	GB.StoreVariant(ARG(value), val);
	oldval = trie_insert(THIS->root, STRING(key), LENGTH(key), val);
	if (oldval)
		value_dtor(oldval);
	UPDATE_TIME();

END_METHOD

/**G
 * Remove the named element. This is equivalent to _put'ing Null into its
 * key.
 */
BEGIN_METHOD(Trie_Remove, GB_STRING key)

	trie_remove(THIS->root, STRING(key), LENGTH(key), value_dtor);
	UPDATE_TIME();

END_METHOD

struct stack {
	struct trie *node;
	int idx, visited : 1;
	struct stack *prev;
};

struct enum_state {
	struct stack *top;
	int start;
};

/**G
 * Enumerates all values in the Trie in lexicographic key order. The Key
 * property is set for each enumerated value.
 *
 * If you picture the Trie as a tree (and have all child nodes ordered
 * lexicographically), the lexicographic traversal of the trie is the
 * pre-order traversal (of nodes with a value).
 */
BEGIN_METHOD_VOID(Trie_next)

	struct enum_state *state = GB.GetEnum();
	struct stack *top;
	struct trie *node = NULL; /* silence compiler with "goto visit" */

	if (!state->start) {
		state->start = 1;
		GB.FreeString(&THIS->key);
		THIS->key = GB.NewString("", 0);
		GB.Alloc((void **) &state->top, sizeof(*state->top));
		state->top->node = THIS->root;
		state->top->idx = 0;
		state->top->visited = 0;
		state->top->prev = NULL;
		top = state->top;
		goto visit;
	}

	top = state->top;
next:
	if (top->idx >= top->node->nchildren) {
		struct stack *prev = top->prev;
		size_t len = GB.StringLength(THIS->key) - top->node->len;

		THIS->key = GB.ExtendString(THIS->key, len);
		GB.Free((void **) &top);
		top = prev;
		if (!top) {
			GB.StopEnum();
			return;
		}
		top->idx++;
		goto next;
	}

	node = top->node->children[top->idx];
visit:
	if (!top->visited) {
		/* AddString() will take the root node's len == 0 as a
		 * request to use strlen(). Make that a special case. */
		if (top->node->len) {
			THIS->key = GB.AddString(THIS->key,
					top->node->key,
					top->node->len);
		}
	} else {
		struct stack *old = top;

		if (old->node->nchildren) {
			GB.Alloc((void **) &top, sizeof(*top));
			top->node = node;
			top->idx = 0;
			top->visited = 0;
			top->prev = old;
			goto visit;
		}
	}

	top->visited = 1;
	state->top = top;
	if (!top->node->value)
		goto next;
	GB.ReturnVariant(top->node->value);

END_METHOD

/**G
 * Remove all elements from the Trie.
 */
BEGIN_METHOD_VOID(Trie_Clear)

	clear_trie(THIS->root, value_dtor);
	UPDATE_TIME();

END_METHOD

/**G
 * Return whether the named key exists, i.e. if it has a value.
 *
 * This does not return if the given string is *part* of a path to another
 * node, it will only give you exact matches. To test if a given prefix
 * exists, use [../GetPrefix].
 */
BEGIN_METHOD(Trie_Exist, GB_STRING key)

	struct trie *node;

	node = trie_find(THIS->root, STRING(key), LENGTH(key));
	GB.ReturnBoolean(!!node);

END_METHOD

typedef struct CPREFIX {
	GB_BASE ob;
	CTRIE *trie;
	struct trie_prefix p;
	char *key;
	char *prefix;
	uint64_t time;
} CPREFIX;

/**G
 * Return a TriePrefix object to search part of a trie.
 *
 * If the prefix is not found, Null is returned.
 */
BEGIN_METHOD(Trie_GetPrefix, GB_STRING prefix)

	static GB_CLASS TriePrefix;
	struct trie_prefix p;
	CPREFIX *obj;

	trie_reset_prefix(&p);
	trie_constrain2(THIS->root, &p, STRING(prefix), LENGTH(prefix));
	if (!p.node) {
		GB.ReturnNull();
		return;
	}

	if (!TriePrefix)
		TriePrefix = GB.FindClass("TriePrefix");
	obj = GB.New(TriePrefix, NULL, NULL);
	obj->trie = THIS;
	GB.Ref(THIS);
	obj->p = p;
	obj->key = NULL;
	obj->prefix = GB.NewString(STRING(prefix), LENGTH(prefix));
	obj->time = THIS->time;
	GB.ReturnObject(obj);

END_METHOD

/**G
 * Return the completion of the given prefix, that is the longest
 * unambiguous continuation of the prefix, like when you hit <Tab>
 * in the console, your shell might complete a command or file name
 * for you.
 *
 * If the prefix is not found, Null is returned.
 */
BEGIN_METHOD(Trie_Complete, GB_STRING prefix)

	struct trie_prefix p;
	char *s;

	trie_reset_prefix(&p);
	trie_constrain2(THIS->root, &p, STRING(prefix), LENGTH(prefix));
	if (!p.node) {
		GB.ReturnNull();
		return;
	}

	s = GB.NewString(STRING(prefix), LENGTH(prefix));
	/* Again, we need to special-case p.node->len - p.i == 0. */
	if (p.node->len - p.i)
		s = GB.AddString(s, p.node->key + p.i, p.node->len - p.i);
	GB.ReturnString(s);
	GB.ReturnBorrow();
	GB.FreeString(&s);
	GB.ReturnRelease();

END_METHOD

/**G
 * Return the number of keys in the Trie.
 */
BEGIN_PROPERTY(Trie_Count)

	GB.ReturnInteger(THIS->count);

END_PROPERTY

/**G
 * Return the key of the last enumerated element.
 */
BEGIN_PROPERTY(Trie_Key)

	GB.ReturnString(THIS->key);

END_PROPERTY

GB_DESC CTrie[] = {
	/**G
	 * This class implements a Patricia Trie. You can learn about its
	 * semantics from [Wikipedia] (http://en.wikipedia.org/wiki/Radix_tree)
	 */
	GB_DECLARE("Trie", sizeof(CTRIE)),

	GB_METHOD("_new", NULL, Trie_new, NULL),
	GB_METHOD("_free", NULL, Trie_free, NULL),

	GB_METHOD("_get", "v", Trie_get, "(Key)s"),
	GB_METHOD("_put", NULL, Trie_put, "(Value)v(Key)s"),
	GB_METHOD("_next", "v", Trie_next, NULL),

	/**G Trie Add
	 * Add an element to the trie. A synonym for _put.
	 */
	GB_METHOD("Add", NULL, Trie_put, "(Value)v(Key)s"),
	GB_METHOD("Remove", NULL, Trie_Remove, "(Key)s"),
	GB_METHOD("Clear", NULL, Trie_Clear, NULL),
	GB_METHOD("Exist", "b", Trie_Exist, "(Key)s"),
	GB_METHOD("GetPrefix", "TriePrefix", Trie_GetPrefix, "(Prefix)s"),
	GB_METHOD("Complete", "s", Trie_Complete, "(Prefix)s"),

	GB_PROPERTY_READ("Count", "i", Trie_Count),
	GB_PROPERTY_READ("Key", "s", Trie_Key),

	GB_END_DECLARE
};

#undef THIS
#define THIS	((CPREFIX *) _object)
#define TRIE	(THIS->trie)
#define PREFIX	(&THIS->p)

/* A TriePrefix is a valid object if it's non-NULL, the prefix string was
 * found and the trie was not modified since its creation. */
static int check_prefix(CPREFIX *p)
{
	//printf("p=%p, state=%d, time=%lu (%lu)\n", p, p->p.state, p->time, p->trie->time);
	return !p || p->p.state == TRIE_UNSET || p->time != p->trie->time;
}

BEGIN_METHOD_VOID(TriePrefix_free)

	GB.Unref((void **) &TRIE);
	GB.FreeString(&THIS->key);
	GB.FreeString(&THIS->prefix);

END_METHOD

/**G
 * This is the same as hTrie[hPrefix.Prefix & RelKey] where hTrie is the
 * Trie from which hPrefix was created, except that it is faster.
 */
BEGIN_METHOD(TriePrefix_get, GB_STRING rel)

	GB_VARIANT_VALUE *val;

	val = trie_value2(TRIE->root, PREFIX, STRING(rel), LENGTH(rel));
	if (!val)
		GB.ReturnNull();
	else
		GB.ReturnVariant(val);

END_METHOD

/**G
 * Iterate through all keys in the prefix range, in lexicographic order.
 *
 * See also
 *   [../../trie/_next]
 */
BEGIN_METHOD_VOID(TriePrefix_next)

	struct enum_state *state = GB.GetEnum();
	struct stack *top;
	struct trie *node = NULL; /* silence compiler */

	if (!state->start) {
		state->start = 1;
		GB.FreeString(&THIS->key);
		THIS->key = GB.NewString("", 0);
		GB.Alloc((void **) &state->top, sizeof(*state->top));
		state->top->node = PREFIX->node;
		state->top->idx = 0;
		state->top->visited = 0;
		state->top->prev = NULL;
		top = state->top;
		goto visit;
	}

	top = state->top;
next:
	if (top->idx >= top->node->nchildren) {
		struct stack *prev = top->prev;
		size_t len = GB.StringLength(THIS->key) - top->node->len;

		THIS->key = GB.ExtendString(THIS->key, len);
		GB.Free((void **) &top);
		top = prev;
		if (!top) {
			GB.StopEnum();
			return;
		}
		top->idx++;
		goto next;
	}

	node = top->node->children[top->idx];
visit:
	if (!top->visited) {
		int i = 0;

		/* Take the offset in the prefix' root into account */
		if (!top->prev)
			i = PREFIX->i;
		/* If top->node->len - i == 0, we want to add nothing, but
		 * GB.AddString() will take that as a request to use
		 * strlen() itself. So special-case that. */
		if (top->node->len - i) {
			THIS->key = GB.AddString(THIS->key,
					top->node->key + i,
					top->node->len - i);
		}
	} else {
		struct stack *old = top;

		if (old->node->nchildren) {
			GB.Alloc((void **) &top, sizeof(*top));
			top->node = node;
			top->idx = 0;
			top->visited = 0;
			top->prev = old;
			goto visit;
		}
	}

	top->visited = 1;
	state->top = top;
	if (!top->node->value)
		goto next;
	GB.ReturnVariant(top->node->value);

END_METHOD

/**G
 * Return if the given key exists relative to the prefix. This returns the
 * same as hTrie.Exist(hPrefix.Prefix & RelKey).
 *
 * See also
 *   [../_get]
 */
BEGIN_METHOD(TriePrefix_Exist, GB_STRING rel)

	struct trie *node;

	node = trie_find2(TRIE->root, PREFIX, STRING(rel), LENGTH(rel));
	GB.ReturnBoolean(!!node);

END_METHOD

/**G
 * Add bytes to the prefix. If the extended prefix does not exist within the
 * Trie, an error is raised.
 */
BEGIN_METHOD(TriePrefix_Add, GB_STRING rel)

	char *s = THIS->prefix;
	struct trie_prefix new = *PREFIX;

	trie_constrain2(TRIE->root, &new, STRING(rel), LENGTH(rel));
	if (!new.node) {
		GB.Error("Prefix does not exist");
		return;
	}
	*PREFIX = new;
	THIS->prefix = GB.AddString(s, STRING(rel), LENGTH(rel));

END_METHOD

/**G
 * Remove bytes from the end of the prefix. There is no way this function
 * can fail since it removes Min(Len(hPrefix.Prefix), Length) bytes and
 * if the TriePrefix was valid, the weaker prefix will also be valid.
 */
BEGIN_METHOD(TriePrefix_Remove, GB_INTEGER len)

	char *s = THIS->prefix;
	size_t len = VARGOPT(len, 1), l;

	if (len < 0)
		GB.Error("Invalid length");
	if (len <= 0)
		return;

	l = GB.StringLength(s);
	if (len > l)
		len = l;

	/*
	 * Since the struct trie has no uplinks to parent nodes, we cannot
	 * go backwards beyond node boundaries. Thus, we do it the less
	 * elegant way: remove characters from the Prefix property and
	 * recreate the prefix from scratch.
	 *
	 * As noted in the help text above, this will always work.
	 */
	l -= len;
	THIS->prefix = GB.ExtendString(s, l);
	trie_reset_prefix(PREFIX);
	trie_constrain2(TRIE->root, PREFIX, THIS->prefix, l);

END_METHOD

/**G
 * Return the **relative key** of the last enumerated object.
 *
 * See also
 *   [../_get]
 */
BEGIN_PROPERTY(TriePrefix_Key)

	GB.ReturnString(THIS->key);

END_PROPERTY

/**G
 * Return the prefix string of this object.
 */
BEGIN_PROPERTY(TriePrefix_Prefix)

	GB.ReturnString(THIS->prefix);

END_PROPERTY

GB_DESC CTriePrefix[] = {
	/**G
	 * This class provides a read-only view of part of a Trie. It lets
	 * you examine keys with a common prefix. Searches begin in the
	 * middle of the Trie and are thus faster.
	 *
	 * TriePrefix objects are invalidated when you change the Trie, so
	 * be careful if you store them persistently.
	 */
	GB_DECLARE("TriePrefix", sizeof(CPREFIX)),
	GB_NOT_CREATABLE(),

	GB_HOOK_CHECK(check_prefix),

	GB_METHOD("_free", NULL, TriePrefix_free, NULL),

	GB_METHOD("_get", "v", TriePrefix_get, "(RelKey)s"),
	GB_METHOD("_next", "v", TriePrefix_next, NULL),

	GB_METHOD("Exist", "b", TriePrefix_Exist, "(RelKey)s"),
	GB_METHOD("Add", NULL, TriePrefix_Add, "(RelKey)s"),
	GB_METHOD("Remove", NULL, TriePrefix_Remove, "[(Length)i]"),

	GB_PROPERTY_READ("Key", "s", TriePrefix_Key),
	GB_PROPERTY_READ("Prefix", "s", TriePrefix_Prefix),

	GB_END_DECLARE
};
