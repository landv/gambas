/*
 * trie.c
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

#include <stdlib.h>
#include <string.h>

#include "trie.h"

#include "c_trie.h"

/**
 * __key_index() - Return a unique number for the character
 * @c: char
 */
static inline int __key_index(char c)
{
	return (int) (unsigned char) c;
}

static inline int popcnt(uint64_t word)
{
	int n;

	for (n = 0; word; n++)
		word &= word - 1;
	return n;
}

#define MASK_SIZE	\
	(__CHAR_BIT__ * sizeof(((struct trie *) 0)->mask[0]))
#define INDEX(i)	(i / MASK_SIZE)
#define OFFSET(i)	(i % MASK_SIZE)

/**
 * __key_to_array_index() - Return array index over a node's ->children
 *                          corresponding to a key character
 * @node: struct trie
 * @c:    the character
 */
static inline int __key_to_array_index(const struct trie *node, char c)
{
	int i = __key_index(c), j, n;

	for (j = n = 0; i >= MASK_SIZE; j++, i -= MASK_SIZE)
		n += popcnt(node->mask[j]);
	n += popcnt(node->mask[j] & ((1ULL << i) - 1));
	return n;
}

static inline void __set_bit(uint64_t mask[4], int i)
{
	mask[INDEX(i)] |= 1ULL << (OFFSET(i));
}

static inline void set_bit(struct trie *node, int i)
{
	__set_bit(node->mask, i);
}

static inline void clear_bit(struct trie *node, int i)
{
	node->mask[INDEX(i)] &= ~(1ULL << (OFFSET(i)));
}

static inline int test_bit(const struct trie *node, int i)
{
	return !!(node->mask[INDEX(i)] & (1ULL << (OFFSET(i))));
}

/**
 * get_continuation() - Return the node continuing the key of another node
 * 			with a given character
 * @node: struct trie
 * @c:    the character
 *
 * If you have a trie like
 *
 *      0
 *      |
 *      te
 *      |
 *   +--+--+
 *   |     |
 *   st    rm
 *
 * and search for the key "term", this function comes in handy at the node
 * "te". You will call get_continuation(te_node, 'r') which yields rm_node.
 *
 * If no such continuation exists, NULL is returned.
 */
static inline struct trie *get_continuation(const struct trie *node, char c)
{
	int i = __key_index(c);
	int j = __key_to_array_index(node, c);

	if (!test_bit(node, i))
		return NULL;
	return node->children[j];
}

struct __trie_find_res {
	struct trie *node, *parent;
	int i, j;
};

/**
 * __trie_find() - Get the node containing a key
 * @trie: struct trie
 * @key:  the key
 * @len:  length
 *
 * This function returns the node in which `key' ends which may NOT be the
 * node which has exactly the key `key'. It returns NULL, if no such node
 * was found.
 */
static struct __trie_find_res __trie_find(const struct trie *trie,
					  const char *key, size_t len)
{
	struct trie *node = (struct trie *) trie, *parent = NULL;
	int i = 0, j = 0;

	while (node) {
		i = 0;
		while (i < node->len && j < len && node->key[i] == key[j]) {
			i++;
			j++;
		}
		/*
		 * Four cases:
		 *  1) the `key' and `node' were entirely consumed: perfect
		 *     match. Get out.
		 *  2) only `key' consumed: we're done as the key lies
		 *     within the node.
		 *  3) only `node' was consumed: recurse to its children.
		 *  4) if neither of the above, node and key deverged here,
		 *     so break the loop since this is as close as we can
		 *     get.
		 */
		if (j == len) {
			break;
		} else if (i == node->len) {
			parent = node;
			node = get_continuation(node, key[j]);
		} else {
			break;
		}
	}
	return (struct __trie_find_res) {node, parent, i, j};
}

/**
 * __is_exact() - Return whether a found node matches a key exactly
 * @res: struct __trie_find_res
 * @len: length
 */
static inline int __is_exact(struct __trie_find_res *res, size_t len)
{
	return res->i == res->node->len && res->j == len;
}

/**
 * __trie_find_exact() - Get the node with ends in a key
 * @trie: struct trie
 * @key:  the key
 * @len:  length
 *
 * Unlike __trie_find(), this function returns the node which has the key
 * `key' or NULL if none. Note, however, that this is not a guarantee that
 * the node contains a non-NULL ->value.
 */
static struct trie *__trie_find_exact(const struct trie *trie,
				      const char *key, size_t len)
{
	struct __trie_find_res res = __trie_find(trie, key, len);

	return (res.node && __is_exact(&res, len)) ? res.node : NULL;
}

/**
 * __new_node() - Allocate a trie node
 * @key:   part of the key
 * @len:   length of `key'
 * @value: payload
 *
 * If `len' equals zero, the length is obtained from `key' via strlen(). For
 * the single case where a zero length is correct, this doesn't do much
 * harm - as opposed to the strangeness of comparing a size_t to -1 or some
 * other clearly invalid value.
 */
static struct trie *new_node(const char *key, size_t len, void *value)
{
	struct trie *trie;

	/*if (!len)
		len = strlen(key);*/

	GB.Alloc((void **) &trie, sizeof(*trie) + len);
	memset(trie->mask, 0, sizeof(trie->mask));
	trie->children = NULL;
	trie->nchildren = 0;
	trie->value = value;
	trie->len = len;
	memcpy(trie->key, key, len);
	return trie;
}

/**
 * new_trie() - Allocate a new trie
 */
struct trie *new_trie(void)
{
	return new_node("", 0, NULL);
}

/**
 * destroy_node() - Deallocate a single node
 * @node: struct trie
 * @dtor: value destructor
 */
static void destroy_node(struct trie *node, void (*dtor)(void *))
{
	GB.Free((void **) &node->children);
	if (node->value && dtor)
		dtor(node->value);
	GB.Free((void **) &node);
}

/**
 * destroy_trie() - Deallocate an entire trie
 * @trie: struct trie
 * @dtor: value destructor
 */
void destroy_trie(struct trie *trie, void (*dtor)(void *))
{
	int i;

	for (i = 0; i < trie->nchildren; i++)
		destroy_trie(trie->children[i], dtor);
	destroy_node(trie, dtor);
}

/**
 * clear_trie() - Remove all but the root
 * @trie: struct trie
 * @dtor: value destructor
 */
void clear_trie(struct trie *trie, void (*dtor)(void *))
{
	int i;

	for (i = 0; i < trie->nchildren; i++)
		destroy_trie(trie->children[i], dtor);
	memset(trie->mask, 0, sizeof(trie->mask));
	GB.Free((void **) &trie->children);
	trie->children = NULL;
	trie->nchildren = 0;
	if (trie->value)
		dtor(trie->value);
	trie->value = NULL;
}

/**
 * __sort_two_children() - Sort (at most) two children into a children array
 * @array:  array with enough space for the element(s)
 * @mask:   buffer
 * @child1: struct trie
 * @child2: struct trie, may be NULL
 *
 * This function writes the apropriate mask for the array into the `mask'
 * argument.
 *
 * The `child2' can be NULL in which case it is ignored and not assigned to
 * the array.
 */
static inline void __sort_two_children(const struct trie *array[2],
				       uint64_t mask[4],
				       const struct trie *child1,
				       const struct trie *child2)
{
	int i, j;

	i = __key_index(*child1->key);
	j = child2 ? __key_index(*child2->key) : 0; /* just to initialise */
	if (!child2 || i < j) {
		array[0] = child1;
		if (child2)
			array[1] = child2;
	} else {
		array[0] = child2;
		array[1] = child1;
	}
	__set_bit(mask, i);
	if (child2)
		__set_bit(mask, j);
}

/**
 * __trie_insert_split() - Split a node to insert a new key
 * @res:   struct __trie_find_res
 * @key:   the key
 * @len:   length
 * @value: the value
 */
static void __trie_insert_split(struct __trie_find_res *res, const char *key,
				size_t len, void *value)
{
	struct trie *node = res->node, *bottom, *branch = NULL;
	struct trie **topchildren;
	/*
	 * If key[res->j] == '\0', the key lies within `node' and will be in
	 * the "top" node already, so we save the `branch'.
	 */
	int have_branch = !!key[res->j];

	/*
	 *  - `bottom' will contain the bottom part of the split node;
	 *  - `branch' will be the new node associated with the wanted key
	 *    (if it is not within the `node')
	 *  - `topchildren' is the new ->children array of the "top" half of
	 *    the split node - which will consist of `bottom' and `branch'.
	 */
	bottom = new_node(&node->key[res->i], node->len - res->i,
			  node->value);

	if (have_branch) {
		branch = new_node(&key[res->j], len - res->j, value);
		GB.Alloc((void **) &topchildren, 2 * sizeof(*topchildren));
	} else {
		GB.Alloc((void **) &topchildren, sizeof(*topchildren));
	}
	/* While doing the Alloc() stuff, we can already Realloc() the
	 * "top" node here... */
	GB.Realloc((void **) &node, sizeof(*node) + res->j);
	/* Link the split node into the trie again */
	int i = __key_to_array_index(res->parent, *node->key);

	res->parent->children[i] = node;

	/*
	 * new_node() set `bottom' already up quite well. However, we need
	 * to tweak: ->mask, ->children and ->nchildren.
	 *
	 * After we have copied them from the "top" node, we can set the
	 * members there correctly to: ->mask, ->children, ->nchildren,
	 * ->value and ->len need tweaking while ->key was cut properly by
	 * Realloc().
	 */
	memcpy(bottom->mask, node->mask, sizeof(bottom->mask));
	bottom->children = node->children;
	bottom->nchildren = node->nchildren;

	/*
	 * __sort_two_children() is aware that `branch' may be NULL.
	 */
	memset(node->mask, 0, sizeof(node->mask));
	__sort_two_children((const struct trie **) topchildren,
			    node->mask, bottom, branch);
	node->children = topchildren;
	node->nchildren = have_branch ? 2 : 1;
	node->value = NULL;
	node->len = res->i;

	/*
	 * The new `branch' has everything right if it exists as it has no
	 * children which need extra care. If it wasn't created, we need to
	 * assign the `value' to the "top" node now.
	 */
	if (!have_branch)
		node->value = value;
}

/**
 * __trie_insert_child() - Extend an already existing key to a new one
 * @res:   struct __trie_find_res
 * @key:   the key
 * @len:   length
 * @value: the value
 */
static void __trie_insert_child(struct __trie_find_res *res, const char *key,
				size_t len, void *value)
{
	struct trie *node = res->parent, *child, **children;
	int i, j, k;

	/*
	 * Here, we CAN'T have the case that `node' has no children and no
	 * value so that we could concatenate the keys. This just cannot
	 * happen while adding nodes: then we would have added a leaf node
	 * without a value which doesn't happen. The case above can only
	 * occur when children are *removed* from an interior parent without
	 * a value and is thus handled in the trie_remove() function.
	 */

	child = new_node(&key[res->j], len - res->j, value);
	i = __key_index(*child->key);
	j = __key_to_array_index(node, *child->key);
	children = node->children;
	GB.Realloc((void **) &children, (node->nchildren + 1) *
					sizeof(*children));

	for (k = node->nchildren; k > j; k--)
		children[k] = children[k - 1];
	children[k] = child;
	node->children = children;
	node->nchildren++;
	set_bit(node, i);
}

/**
 * trie_insert() - Associate a value with a key in the trie
 * @trie:  struct trie
 * @key:   the key
 * @len:   length
 * @value: the value
 *
 * You can use the empty string as `key' to save data in the trie's root
 * node. Note that the NULL pointer is an invalid `value' and is used to
 * detect value-less nodes, so don't use it!
 *
 * If a value was replaced, the old value is returned.
 */
void *trie_insert(struct trie *trie, const char *key, size_t len, void *value)
{
	struct __trie_find_res res = __trie_find(trie, key, len);

	if (res.node) {
		if (__is_exact(&res, len)) {
			void *last = res.node->value;

			res.node->value = value;
			return last;
		}
		__trie_insert_split(&res, key, len, value);
	} else {
		__trie_insert_child(&res, key, len, value);
	}
	return NULL;
}

/**
 * __trie_remove_leaf() - Remove a leaf node
 * @res:  struct __trie_find_res
 * @dtor: value destructor
 */
static void __trie_remove_leaf(struct __trie_find_res *res,
			       void (*dtor)(void *))
{
	struct trie *node = res->node, *parent = res->parent;
	int i, j, k;

	/*
	 * Unlink the node which means
	 *  a) delete it from its parent's mask and
	 *  b) delete it from its parent's children array
	 *
	 * Then we can simply destroy it.
	 */

	i = __key_index(*node->key);
	j = __key_to_array_index(parent, *node->key);

	/*
	 * b) -- yes, we do b) before a) to not need to undo the mask
	 * changes if an allocation fails :-) and the code is different for
	 * each of the cases below, anyway:
	 *    i) if the parent will have no children left, we don't bother
	 *       doing reallocations and stuff.
	 *   ii) if the value-less non-root parent will only have one child
	 *       left, merge these two nodes to save space.
	 *  iii) else, reallocate the children array normally.
	 *
	 * In i), it is impossible that the parent would have no value
	 * because of ii) in a former removal (just saying that every leaf
	 * node (except the root if it becomes a leaf) is guaranteed to have
	 * a value).
	 */
	if (parent->nchildren == 1) { /* i) */
		GB.Free((void **) &parent->children);
		parent->children = NULL;
		parent->nchildren = 0;
		/* a) */
		clear_bit(parent, i);
		parent->nchildren--;
	/* !parent->len is equivalent to parent == trie_root */
	} else if (parent->nchildren == 2 && !parent->value
					  && !parent->len) { /* ii) */
		struct trie *other;

		if (parent->children[0] == node)
			other = parent->children[1];
		else
			other = parent->children[0];
		GB.Realloc((void **) &parent, sizeof(*parent) + parent->len
							      + other->len);
		memcpy(parent->key + parent->len, other->key, other->len);
		parent->len += other->len;
		/* does a) */
		memcpy(parent->mask, other->mask, sizeof(parent->mask));
		GB.Free((void **) &parent->children);
		parent->children = other->children;
		parent->nchildren = other->nchildren;
		parent->value = other->value;
		/* Do NOT destroy_node() as we copied its ->children! */
		GB.Free((void **) &other);
	} else { /* iii) */
		/* does a) */
		for (k = j + 1; k < parent->nchildren; k++)
			parent->children[k - 1] = parent->children[k];
		parent->nchildren--;
		GB.Realloc((void **) &parent->children, parent->nchildren *
						sizeof(*parent->children));
		clear_bit(parent, i);
	}

	destroy_node(node, dtor);
}

/**
 * __trie_remove_interior() - Remove an interior node
 * @res:  struct __trie_find_res
 * @dtor: value destructor
 */
static void __trie_remove_interior(struct __trie_find_res *res,
				   void (*dtor)(void *))
{
	/*
	 * Let's see: an interior node can only have 2 or more children, so
	 * we cannot possibly do any compression of the nodes. We just erase
	 * the value and leave the trie structure as-is.
	 */
	dtor(res->node->value);
	res->node->value = NULL;
}

/**
 * trie_remove() - Remove a key from the trie
 * @trie: struct trie
 * @key:  the key
 * @len:  length
 * @dtor: value destructor
 */
void trie_remove(struct trie *trie, const char *key, size_t len,
		 void (*dtor)(void *))
{
	struct __trie_find_res res;
	struct trie *node;

	res = __trie_find(trie, key, len);
	node = res.node;
	/*
	 * We only want to work with valued, exactly-matching non-roots.
	 * Delete a value from the root anyways.
	 */
	if (!node || !__is_exact(&res, len) || !node->value)
		return;
	if (node == trie) {
		dtor(node->value);
		node->value = NULL;
		return;
	}

	if (!node->children)
		__trie_remove_leaf(&res, dtor);
	else
		__trie_remove_interior(&res, dtor);
}

/**
 * trie_find() - Get a trie node from its key
 * @trie: struct trie
 * @key:  the key
 * @len:  length
 *
 * Returns NULL if the key was not found. The empty string maps to the root
 * node of the trie.
 */
struct trie *trie_find(const struct trie *trie, const char *key, size_t len)
{
	return __trie_find_exact(trie, key, len);
}

/**
 * trie_value() - Get value corresponding to a key
 * @trie: struct trie
 * @key:  the key
 * @len:  length
 *
 * Return NULL if the key was not found. The empty string maps to the root
 * node of the trie.
 */
void *trie_value(const struct trie *trie, const char *key, size_t len)
{
	struct trie *node = trie_find(trie, key, len);

	return node ? node->value : NULL;
}

/**
 * trie_constrain() - Constrain the trie paths
 * @trie: struct trie
 * @p:    struct trie_prefix
 * @c:    character
 *
 * To `constrain' a trie means to limit keys to a given prefix. If you have
 * a trie consisting of keys "test", "tesla" and "term" and you constrain it
 * with the prefix "tes", only "test" and "tesla" will be reachable.
 *
 * The constraint is saved in the struct trie_prefix. Contraining a trie
 * does not alter its structure so that you can constrain the same trie
 * multiple times simultaneously.
 *
 * By calling this function multiple times, you can refine the prefix in
 * `p'. To begin without a constraint, use a prefix filled by
 * trie_reset_prefix().
 *
 * If the prefix is not found, `p' is reset.
 *
 * WARNING
 * Using any of the prefix-aware functions implies that the trie did not
 * change between calls. If it did, the prefix may be invalid and the
 * program may crash in consequence.
 */
void trie_constrain(const struct trie *trie, struct trie_prefix *p, char c)
{
	struct trie *node;
	int i;

	node = p->node ? : (struct trie *) trie;
	i = p->i;

	if (i == node->len) {
		node = get_continuation(node, c);
		if (!node)
			goto reset;
		p->node = node;
		/* node->len is guaranteed to be positive here */
		p->i = 1;
	} else {
		if (node->key[i] != c)
			goto reset;
		p->i = ++i;
	}
	/*
	 * As a `logical' node counts only what has a value. If you insert
	 * "soup" and "sour", the resulting "sou" node would NOT exist
	 * logically.
	 *
	 * Additionally, the match must be exact to count as TRIE_EXACT.
	 */
	if (p->i == node->len && node->value)
		p->state = TRIE_EXACT;
	else
		p->state = TRIE_EXIST;
	return;

reset:
	trie_reset_prefix(p);
}

/**
 * trie_constrain2() - Constrain the trie multiple times
 * @trie: struct trie
 * @p:    struct trie_prefix
 * @str:  string
 * @len:  length
 *
 * This function calls trie_constrain() in a loop - but with the
 * difference that as soon as the prefix is not found, the function
 * returns. In effect, the `str' is taken of consecutive constraints
 * which should *ALL* be applied in row or none of them.
 */
void trie_constrain2(const struct trie *trie, struct trie_prefix *p,
		     const char *str, size_t len)
{
	int i;

	if (!len) {
		p->state = trie->value ? TRIE_EXACT : TRIE_EXIST;
		p->node = (struct trie *) trie;
		p->i = 0;
		return;
	}

	for (i = 0; i < len; i++) {
		trie_constrain(trie, p, str[i]);
		if (p->state == TRIE_UNSET)
			return;
	}
}

/**
 * trie_find2() - Find a key from a trie constraint
 * @trie: struct trie
 * @p:    struct trie_prefix
 * @key:  the key relative (!) to the prefix `p'
 * @len:  length
 *
 * This function is similar to trie_find(), except that keys are relative to
 * the constraint in `p'.
 */
struct trie *trie_find2(const struct trie *trie,
			const struct trie_prefix *p,
			const char *key, size_t len)
{
	struct trie *node;
	int i, j;

	node = p->node ? : (struct trie *) trie;
	i = p->i;
	/*
	 * First consume the rest of the prefix node. If none of the trivial
	 * cases occured, we can then use the normal traversal algorithm.
	 */
	for (j = 0; i < node->len && j < len; i++, j++)
		if (node->key[i] != key[j])
			return NULL;
	if (j == len)
		return node;
	node = get_continuation(node, key[j]);
	if (!node)
		return NULL;
	return __trie_find_exact(node, key, len);
}

/**
 * trie_value2() - Analogon to trie_value() using trie_find2()
 * @trie: struct trie
 * @p:    struct trie_prefix
 * @key:  the key
 * @len:  length
 */
void *trie_value2(const struct trie *trie, const struct trie_prefix *p,
		  const char *key, size_t len)
{
	struct trie *node = trie_find2(trie, p, key, len);

	return node ? node->value : NULL;
}
