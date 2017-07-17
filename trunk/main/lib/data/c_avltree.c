/*
 * c_avltree.c - AvlTree class
 *
 * Copyright (C) 2013 Tobias Boege <tobias@gambas-buch.de>
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

#define __C_AVLTREE_C

#include <assert.h>

#include "gambas.h"
#include "gb_common.h"
#include "string_compare.h"

#include "c_avltree.h"

typedef struct node NODE;

struct node {
	char *key;		/* Key string */
	size_t length;		/* Key length */
	int balance;		/* Balance value in {-1, 0, 1} */
	NODE *left, *right;	/* Children or NULL */
	NODE *parent;		/* Parent */
	GB_VARIANT_VALUE val;	/* Payload */
};

typedef struct {
	GB_BASE ob;
	NODE *root;		/* The root of the tree */
	NODE *last;		/* Last used node */
	size_t count;		/* Element count */
	size_t height;		/* Tree height */
} CAVLTREE;

static void CAVLTREE_init(CAVLTREE *tree)
{
	tree->root = tree->last = NULL;
	tree->count = tree->height = 0;
}

static NODE *NODE_new(NODE *parent, char *key, size_t length)
{
	NODE *node;

	GB.Alloc((void **) &node, sizeof(*node));
	node->key = GB.NewString(key, length);
	node->length = length;
	node->balance = 0;
	node->left = node->right = NULL;
	/* If 'parent' is NULL, this shall be its own parent */
	node->parent = parent ? : node;
	node->val.type = GB_T_NULL;
	return node;
}

static void NODE_destroy(NODE *node)
{
	GB.FreeString(&node->key);
	GB.StoreVariant(NULL, &node->val);
	GB.Free((void **) &node);
}

static NODE *CAVLTREE_first(CAVLTREE *tree)
{
	NODE *first = tree->root;

	if (!first)
		return NULL;
	while (first->left)
		first = first->left;
	return first;
}

#define THIS	((CAVLTREE *) _object)

/**G
 * Create a new, empty AvlTree.
 **/
BEGIN_METHOD_VOID(AvlTree_new)

	CAVLTREE_init(THIS);

END_METHOD

struct enum_state {
	int started;
	NODE *next;
};

static void CAVLTREE_clear(CAVLTREE *tree)
{
	NODE *node = CAVLTREE_first(tree), *parent;

	/* We ought to traverse the tree from children to parents */
	while (node) {
		while (node->left)
			node = node->left;
		while (node->right)
			node = node->right;
		parent = node->parent;
		if (node == parent) {
			parent = NULL;
		} else {
			if (parent->left == node)
				parent->left = NULL;
			else
				parent->right = NULL;
		}
		NODE_destroy(node);
		node = parent;
	}

	/* Fix enumerators */
	void *ebuf;
	struct enum_state *state;

	ebuf = GB.BeginEnum(tree);
	while (!GB.NextEnum()) {
		state = GB.GetEnum();
		state->next = NULL;
	}
	GB.EndEnum(ebuf);

	tree->root = tree->last = NULL;
	tree->count = 0;
	tree->height = 0;
}

/**G
 * Clears the tree automatically.
 **/
BEGIN_METHOD_VOID(AvlTree_free)

	CAVLTREE_clear(THIS);

END_METHOD

static NODE *CAVLTREE_find(CAVLTREE *tree, char *key, size_t length)
{
	NODE *node = tree->root;
	int res;

	while (node) {
		res = STRING_compare(key, length, node->key, node->length);

		if (!res)
			return node;
		else if (res < 0)
			node = node->left;
		else
			node = node->right;
	}
	return NULL;
}

/**G
 * Return the value associated with the given key. If no node with this key
 * was found, Null is returned.
 **/
BEGIN_METHOD(AvlTree_get, GB_STRING key)

	NODE *node;

	node = CAVLTREE_find(THIS, STRING(key), LENGTH(key));
	THIS->last = node;
	if (!node) {
		GB.ReturnNull();
		return;
	}
	GB.ReturnVariant(&node->val);

END_METHOD

/* In-order. */
static NODE *CAVLTREE_next(CAVLTREE *tree, NODE *node)
{
	NODE *next;

	if ((next = node->right)) {
		while (next->left)
			next = next->left;
		return next;
	}
	for (next = node->parent; node == next->right; next = next->parent)
		node = next;
	/* This condition is only met when climing from the right subtree to
	 * root in the above loop. We're done then. */
	if (node == next)
		return NULL;
	return next;
}

#if 0
/* Reverse in-order */
static NODE *CAVLTREE_prev(CAVLTREE *tree, NODE *node)
{
	NODE *prev;

	if ((prev = node->left)) {
		while (prev->right)
			prev = prev->right;
		return prev;
	}
	for (prev = node->parent; node == prev->left; prev = prev->parent)
		node = prev;
	if (node == prev)
		return NULL;
	return prev;
}
#endif

static inline void rotate_left(CAVLTREE *tree, NODE *rot)
{
	NODE *right = rot->right, *parent = rot->parent;

	if (rot == tree->root) {
		tree->root = right;
		/* Don't forget to add the root signature */
		right->parent = right;
	} else {
		if (rot == parent->left)
			parent->left = right;
		else
			parent->right = right;
		right->parent = parent;
	}
	rot->parent = right;

	rot->right = right->left;
	if (rot->right)
		rot->right->parent = rot;
	right->left = rot;
}

static inline void rotate_right(CAVLTREE *tree, NODE *rot)
{
	NODE *left = rot->left, *parent = rot->parent;

	if (rot == tree->root) {
		tree->root = left;
		left->parent = left;
	} else {
		if (rot == parent->left)
			parent->left = left;
		else
			parent->right = left;
		left->parent = parent;
	}
	rot->parent = left;

	rot->left = left->right;
	if (rot->left)
		rot->left->parent = rot;
	left->right = rot;
}

#define sgn(x)				\
({					\
	typeof(x) __x = (x);		\
	__x ? (__x < 0 ? -1 : 1) : 0;	\
})

static NODE *CAVLTREE_find_add(CAVLTREE *tree, char *key, size_t length)
{
	NODE *node, *parent, *reb, *rot, *new;
	int res;

	/* Make GCC happy */
	parent = NULL;
	res = 0;

	/* Slightly extended version of NODE_find() which gathers additional
	 * data for insertion. I'd like to have them separate. */
	reb = node = tree->root;
	while (node) {
		res = STRING_compare(key, length, node->key, node->length);

		if (!res)
			return node;
		if (node->balance)
			reb = node;
		parent = node;
		if (res < 0)
			node = node->left;
		else
			node = node->right;
	}
	res = sgn(res);

	new = node = NODE_new(parent, key, length);
	tree->count++;

	/* Fix enumerations, pt. I: empty tree */
	void *ebuf;
	struct enum_state *state;

	if (!tree->root) {
		tree->root = node;
		tree->height++;
		ebuf = GB.BeginEnum(tree);
		while (!GB.NextEnum()) {
			state = GB.GetEnum();
			state->next = node;
		}
		GB.EndEnum(ebuf);
		return node;
	}
	if (res == -1)
		parent->left = node;
	else
		parent->right = node;

	/* Fix enumerators, pt. II: all other cases */
	ebuf = GB.BeginEnum(tree);
	while (!GB.NextEnum()) {
		state = GB.GetEnum();
		/*
		 * Nasty. If a new element is inserted before the current
		 * state->next OR state->next is NULL and a new last
		 * element is added, update the state.
		 */
		if (state->next == parent
		 || (!state->next && !CAVLTREE_next(tree, new)))
			state->next = new;
	}
	GB.EndEnum(ebuf);

	/* Adjust balance factors */
	while (node != reb) {
		if (node == parent->left)
			parent->balance--;
		else
			parent->balance++;
		node = parent;
		parent = parent->parent;
	}

	/* Rebalance the tree */
	switch (reb->balance) {
	case 1:
	case -1:
		tree->height++;
		break;
	case 2: /* Right heavy */
		rot = reb->right;
		if (rot->balance == 1) { /* Right-right */
			reb->balance = 0;
			rot->balance = 0;
		} else { /* Right-left */
			switch (rot->left->balance) {
			case 1:
				reb->balance = -1;
				rot->balance = 0;
				break;
			case 0:
				reb->balance = 0;
				rot->balance = 0;
				break;
			case -1:
				reb->balance = 0;
				rot->balance = 1;
				break;
			}
			rot->left->balance = 0;
			rotate_right(tree, rot);
		}
		rotate_left(tree, reb);
		break;
	case -2: /* Left heavy */
		rot = reb->left;
		if (rot->balance == -1) { /* Left-left */
			reb->balance = 0;
			rot->balance = 0;
		} else { /* Left-right */
			switch (rot->right->balance) {
			case 1:
				reb->balance = 0;
				rot->balance = -1;
				break;
			case 0:
				reb->balance = 0;
				rot->balance = 0;
				break;
			case -1:
				reb->balance = 1;
				rot->balance = 0;
				break;
			}
			rot->right->balance = 0;
			rotate_left(tree, rot);
		}
		rotate_right(tree, reb);
		break;
	}

	return new;
}

#ifdef DEBUG_ME
static void dump_node(NODE *node)
{
	fprintf(stderr, "%p \"%s\" (parent=%p \"%s\", left=%p \"%s\", "
			"right=%p \"%s\") balance=%d\n", node, node->key,
			node->parent, node->parent->key, node->left,
			node->left ? node->left->key : "", node->right,
			node->right ? node->right->key : "", node->balance);
	if (node->left)
		dump_node(node->left);
	if (node->right)
		dump_node(node->right);
}
#endif

static void CAVLTREE_remove(CAVLTREE *tree, char *key, size_t length)
{
	NODE *node, *rep, *child, *reb;
	int d; /* A balance delta */
	int process_root = 1;

	node = CAVLTREE_find(tree, key, length);

#ifdef DEBUG_ME
	fprintf(stderr, "Deletion of %p \"%s\"\n", node, key);
	dump_node(tree->root);
	fprintf(stderr, "-->\n");
#endif

	if (!node)
		return;
	tree->count--;
	if (node == tree->last)
		tree->last = NULL;

	if (!node->left || !node->right) {
		rep = node->left ? : node->right;
		reb = node->parent;
		d = node == reb->left ? 1: -1;
		goto replace;
	}

	rep = CAVLTREE_next(tree, node);

	/* Detach replacement node */
	reb = rep->parent;
	d = LIKELY(rep == reb->left) ? 1 : -1;
	if (reb == node)
		goto replace;
	child = rep->left ? : rep->right;
	if (child)
		child->parent = reb;
	if (LIKELY(rep == reb->left))
		reb->left = child;
	else
		reb->right = child;

	/* Replace 'node' by 'rep'. At this point, 'rep' may be anything
	 * from an inner node to a half-leaf or leaf. */
replace:;
	/* Fix enumerations */
	void *ebuf;
	struct enum_state *state;

	ebuf = GB.BeginEnum(tree);
	while (!GB.NextEnum()) {
		state = GB.GetEnum();
		if (state->next == node)
			state->next = rep;
	}
	GB.EndEnum(ebuf);

	if (node == tree->root) {
		tree->root = rep;
		if (rep) {
			rep->parent = rep;
		} else { /* Tree gets empty */
			tree->count = 0;
			tree->height = 0;
			NODE_destroy(node);
			return;
		}
	} else {
		if (node == node->parent->left)
			node->parent->left = rep;
		else
			node->parent->right = rep;
		if (rep)
			rep->parent = node->parent;
	}
	if (rep) {
		rep->balance = node->balance;
		if (rep != node->left) {
			rep->left = node->left;
			if (rep->left)
				rep->left->parent = rep;
		} else {
			rep->balance++;
		}
		if (rep != node->right) {
			rep->right = node->right;
			if (rep->right)
				rep->right->parent = rep;
		} else {
			rep->balance--;
		}
	}

	NODE_destroy(node);

	/* Rebalance */
	if (reb == tree->root)
		process_root = 0;
	do {
		int old_balance = reb->balance;
		NODE *rot;

		reb->balance += d;
		switch (reb->balance) {
		case 2: /* Right heavy */
			rot = reb->right;
			if (rot->balance == 1) { /* Right-right */
				reb->balance = 0;
				rot->balance = 0;
			} else { /* Right-left */
				switch (rot->left->balance) {
				case 1:
					reb->balance = -1;
					rot->balance = 0;
					break;
				case 0:
					reb->balance = 0;
					rot->balance = 0;
					break;
				case -1:
					reb->balance = 0;
					rot->balance = 1;
					break;
				}
				rot->left->balance = 0;
				rotate_right(tree, rot);
			}
			rotate_left(tree, reb);
			break;
		case -2: /* Left heavy */
			rot = reb->left;
			if (rot->balance == -1) { /* Left-left */
				reb->balance = 0;
				rot->balance = 0;
			} else { /* Left-right */
				switch (rot->right->balance) {
				case 1:
					reb->balance = 0;
					rot->balance = -1;
					break;
				case 0:
					reb->balance = 0;
					rot->balance = 0;
					break;
				case -1:
					reb->balance = 1;
					rot->balance = 0;
					break;
				}
				rot->right->balance = 0;
				rotate_left(tree, rot);
			}
			rotate_right(tree, reb);
			break;
		case -1:
		case 1:
			goto end;
		}
		d = reb->balance - old_balance;
		if (reb == reb->parent->right)
			d = -d;
		reb = reb->parent;
	} while (reb != tree->root || process_root--);
	tree->height--;
end:;
#ifdef DEBUG_ME
	dump_node(tree->root);
	fprintf(stderr, "Deletion complete\n\n");
#endif
}

/**G
 * Creates a new node with the given key and value or changes the value of
 * an already existing key. If the value is Null, then the node is removed.
 **/
BEGIN_METHOD(AvlTree_put, GB_VARIANT value; GB_STRING key)

	NODE *node;

	if (VARG(value).type == GB_T_NULL) {
		CAVLTREE_remove(THIS, STRING(key), LENGTH(key));
		return;
	}
	node = CAVLTREE_find_add(THIS, STRING(key), LENGTH(key));
	GB.StoreVariant(ARG(value), &node->val);
	THIS->last = node;

END_METHOD

/**G
 * Visit each element of the tree in-order, i.e. from the smallest key to
 * the greatest. The Key property of the tree is set according to the value
 * in the enumerator.
 *
 * {example
 * 	Dim v As Variant
 *
 * 	Print "Key", "Value"
 * 	For Each v In hTree
 * 	  Print hTree.Key, v
 * 	Next
 * }
 **/
BEGIN_METHOD_VOID(AvlTree_next)

	NODE *node;
	struct enum_state *state = GB.GetEnum();

	if (!state->started) {
		state->started = 1;
		node = CAVLTREE_first(THIS);
	} else {
		node = state->next;
	}
	if (!node) {
		GB.StopEnum();
		return;
	}
	state->next = CAVLTREE_next(THIS, node);
	THIS->last = node;
	GB.ReturnVariant(&node->val);

END_METHOD

/**G
 * Clear the tree, i.e. remove all elements. This is **way** faster than
 * removing every element by assigning Null to it like this:
 *
 * 	For Each v In hTree
 * 	  hTree[hTree.Key] = Null
 * 	Next
 *
 * Because removing an element may require to rebalance the tree at most
 * hTree.Height times. For each element, this is a great overhead to have an
 * empty tree.
 *
 * So use hTree.Clear() if the tree shall be emptied.
 **/
BEGIN_METHOD_VOID(AvlTree_Clear)

	CAVLTREE_clear(THIS);

END_METHOD

/**G
 * Return whether an element with the given key exists.
 **/
BEGIN_METHOD(AvlTree_Exist, GB_STRING key)

	NODE *node;

	node = CAVLTREE_find(THIS, STRING(key), LENGTH(key));
	/* TODO: Use this as a cache for subsequent MoveTo() or anything */
	THIS->last = node;
	GB.ReturnBoolean(!!node);

END_METHOD

/**G
 * Return the balance factor of the AvlTree. It is either -1, 0 or 1.
 **/
BEGIN_PROPERTY(AvlTree_Balance)

	if (!THIS->root) {
		GB.ReturnInteger(0);
		return;
	}
	GB.ReturnInteger(THIS->root->balance);

END_PROPERTY

/**G
 * Return the number of elements in the tree.
 **/
BEGIN_PROPERTY(AvlTree_Count)

	GB.ReturnInteger(THIS->count);

END_PROPERTY

/**G
 * Return the height of the tree.
 **/
BEGIN_PROPERTY(AvlTree_Height)

	GB.ReturnInteger(THIS->height);

END_PROPERTY

/**G
 * Return the last used key. This can be Null if the element was removed
 * meanwhile.
 *
 * Be careful as this property changes with nearly every operation on the
 * tree.
 **/
BEGIN_PROPERTY(AvlTree_Key)

	if (!THIS->last)
		GB.ReturnNull();
	else
		GB.ReturnString(THIS->last->key);

END_PROPERTY

GB_DESC CAvlTree[] = {
	GB_DECLARE("AvlTree", sizeof(CAVLTREE)),

	GB_METHOD("_new", NULL, AvlTree_new, NULL),
	GB_METHOD("_free", NULL, AvlTree_free, NULL),
	GB_METHOD("_get", "v", AvlTree_get, "(Key)s"),
	GB_METHOD("_put", NULL, AvlTree_put, "(Value)v(Key)s"),
	GB_METHOD("_next", "v", AvlTree_next, NULL),

	GB_METHOD("Clear", NULL, AvlTree_Clear, NULL),
	GB_METHOD("Exist", "b", AvlTree_Exist, "(Key)s"),

#if 0
	/* Returns left and right subtree of root as array of new
	 * (deeply-copied) trees. This is trivial as any subtree in an
	 * AvlTree is also an AvlTree. */
	GB_METHOD("Split", ".AvlTree.Split", AvlTree_Split, "(Key)s"),
	/* Merge into this tree. */
	GB_METHOD("Merge", NULL, AvlTree_Merge, "(OtherTree)AvlTree"),
	/* Return a deep copy of the AvlTree */
	GB_METHOD("Copy", "AvlTree", AvlTree_Copy, NULL),
#endif

	GB_PROPERTY_READ("Balance", "i", AvlTree_Balance),
	GB_PROPERTY_READ("Count", "i", AvlTree_Count),
	GB_PROPERTY_READ("Height", "i", AvlTree_Height),

	GB_PROPERTY_READ("Key", "s", AvlTree_Key),

	GB_END_DECLARE
};

#if 0
GB_DESC CAvlTreeSplit[] = {
	GB_DECLARE_VIRTUAL(".AvlTree.Split"),

	GB_PROPERTY_READ("Left", "AvlTree", AvlTreeSplit_Left),
	GB_PROPERTY_READ("Right", "AvlTree", AvlTreeSplit_Right),
	GB_PROPERTY_READ("Both", "AvlTree[]", AvlTreeSplit_Both),

	GB_END_DECLARE
};
#endif
