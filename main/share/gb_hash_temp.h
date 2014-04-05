/***************************************************************************

  gb_hash_temp.h

  (c) 2000-2013 Benoît Minisini <gambas@users.sourceforge.net>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
  MA 02110-1301, USA.

***************************************************************************/

#define __GB_HASH_C

#include <ctype.h>

#include "gb_common.h"
#include "gb_common_case.h"
#include "gb_common_string.h"
#include "gb_alloc.h"
#include "gb_hash.h"


#define NODE_value(_node) ((void *)((char *)_node) + sizeof(HASH_NODE))
//#define NODE_length(_table, _node) (*((ushort *)(((char *)_node) + sizeof(HASH_NODE) + (_table)->s_value)))
#define NODE_key(_table, _node) ((HASH_KEY *)(((char *)_node) + sizeof(HASH_NODE) + (_table)->s_value))

static void hash_table_resize(HASH_TABLE *hash_table);
static HASH_NODE **hash_table_lookup_node(HASH_TABLE *hash_table, const char *key, int len);
static HASH_NODE *hash_node_new(HASH_TABLE *hash_table, const char *key, int len);
static void hash_node_destroy(HASH_NODE *hash_node);

#ifdef KEEP_ORDER
#else
static void hash_nodes_destroy(HASH_NODE *hash_node);
#endif

// Put a random number in that if you want to be safe on the Internet
uint HASH_seed = 0x9A177BA5;

static const int primes[] =
{
	11, 19, 37, 73, 109, 163, 251, 367, 557, 823, 1237, 1861, 2777, 4177, 6247, 9371,
	14057, 21089, 31627, 47431, 71143, 106721, 160073, 240101, 360163, 540217, 810343,
	1215497, 1823231, 2734867, 4102283, 6153409, 9230113, 13845163
};

//static const uint seed[] = { 0x9A177BA5, 0x9A177BA4, 0x9A177BA7, 0x9A177BA6, 0x9A177BA1, 0x9A177BA0, 0x9A177BA3, 0x9A177BA2, 0x9A177BAD };

static const int nprimes = sizeof (primes) / sizeof (primes[0]);

static int spaced_primes_closest(int num)
{
	int i;

	for (i = 0; i < nprimes; i++)
		if (primes[i] > num)
			return primes[i];

	return primes[nprimes - 1];
}


// Fast hashing functions. Sometimes Microsoft Research produces useful things.

static uint key_hash_binary(const char *key, int len)
{
	static const void *jump[] = { &&__LEN_0, &&__LEN_1, &&__LEN_2, &&__LEN_3, &&__LEN_4, &&__LEN_5, &&__LEN_6, &&__LEN_7, &&__LEN_8 };
	uint seed = HASH_seed;
	uint hash = seed ^ len;

	if (len > 8)
	{
		key += len - 8;
		len = 8;
	}
	
	goto *jump[len];

__LEN_8:
	hash = hash * seed + key[7];
__LEN_7:
	hash = hash * seed + key[6];
__LEN_6:
	hash = hash * seed + key[5];
__LEN_5:
	hash = hash * seed + key[4];
__LEN_4:
	hash = hash * seed + key[3];
__LEN_3:
	hash = hash * seed + key[2];
__LEN_2:
	hash = hash * seed + key[1];
__LEN_1:
	hash = hash * seed + key[0];
__LEN_0:

	return hash;
}

static uint key_hash_text(const char *key, int len)
{
	static const void *jump[] = { &&__LEN_0, &&__LEN_1, &&__LEN_2, &&__LEN_3, &&__LEN_4, &&__LEN_5, &&__LEN_6, &&__LEN_7, &&__LEN_8 };
	uint seed = HASH_seed;
	uint hash = seed ^ len;

	if (len > 8)
	{
		key += len - 8;
		len = 8;
	}
	
	goto *jump[len];

__LEN_8:
	hash = hash * seed + ((uint)key[7] & ~0x20U);
__LEN_7:
	hash = hash * seed + ((uint)key[6] & ~0x20U);
__LEN_6:
	hash = hash * seed + ((uint)key[5] & ~0x20U);
__LEN_5:
	hash = hash * seed + ((uint)key[4] & ~0x20U);
__LEN_4:
	hash = hash * seed + ((uint)key[3] & ~0x20U);
__LEN_3:
	hash = hash * seed + ((uint)key[2] & ~0x20U);
__LEN_2:
	hash = hash * seed + ((uint)key[1] & ~0x20U);
__LEN_1:
	hash = hash * seed + ((uint)key[0] & ~0x20U);
__LEN_0:

	return hash;
}


#define get_hash_func(_hash) ((_hash)->mode ? key_hash_text : key_hash_binary)


void HASH_TABLE_create(HASH_TABLE **hash, size_t s_value, HASH_FLAG mode)
{
	HASH_TABLE *hash_table;
	/*int i;*/

	ALLOC_ZERO(&hash_table, sizeof(HASH_TABLE));

	hash_table->size = HASH_TABLE_MIN_SIZE;
	hash_table->s_value = s_value;

	ALLOC_ZERO(&hash_table->nodes, sizeof(HASH_NODE *) * hash_table->size);

	if (mode == HF_IGNORE_CASE)
		hash_table->mode = mode;
	else
		hash_table->mode = HF_NORMAL;

	*hash = hash_table;
}


void HASH_TABLE_delete(HASH_TABLE **hash)
{
	HASH_TABLE *hash_table = *hash;

	if (hash_table == NULL)
		return;
	
	#ifdef KEEP_ORDER
	HASH_NODE *node, *next;
	
	node = hash_table->sfirst;
	hash_table->sfirst = NULL;
	hash_table->slast = NULL;
	
	while (node)
	{
		next = node->snext;
		FREE(&node);
		node = next;
	}
	#else
	int i;
	
	for (i = 0; i < hash_table->size; i++)
		hash_nodes_destroy(hash_table->nodes[i]);
	#endif

	FREE(&hash_table->nodes);
	FREE(hash);
}


int HASH_TABLE_size(HASH_TABLE *hash_table)
{
	return hash_table->nnodes;
}


static HASH_NODE **hash_table_lookup_node(HASH_TABLE *hash_table, const char *key, int len)
{
	HASH_NODE **node;
	HASH_KEY *node_key;
	uint hash;
	//int n;

	if (hash_table->mode)
	{
		hash = key_hash_text(key, len);
		node = &hash_table->nodes[hash % hash_table->size];

		//n = 0;
		while (*node)
		{
			node_key = NODE_key(hash_table, *node);
			//n++;
			if (node_key->len == len && STRING_equal_ignore_case_same(key, node_key->key, len))
				break;
			node = &(*node)->next;
		}
	}
	else
	{
		hash = key_hash_binary(key, len);
		node = &hash_table->nodes[hash % hash_table->size];
	
		//n = 0;
		while (*node)
		{
			node_key = NODE_key(hash_table, *node);
			//n++;
			if (node_key->len == len && STRING_equal_same(key, node_key->key, len))
				break;
			node = &(*node)->next;
		}
	}
	
	//fprintf(stderr, "hash_table_lookup_node %p: %d %d -> %d\n", hash_table, hash_table->size, hash_table->nnodes, n);
	return node;
}


void *HASH_TABLE_lookup(HASH_TABLE *hash_table, const char *key, int len, bool set_last)
{
	HASH_NODE *node;

	if (len == 0)
		return NULL;
	
	node = *hash_table_lookup_node(hash_table, key, len);
	if (set_last)
		hash_table->last = node;

	return node ? NODE_value(node) : NULL;
}


void *HASH_TABLE_insert(HASH_TABLE *hash_table, const char *key, int len)
{
	HASH_NODE **node;
	void *value;

	node = hash_table_lookup_node(hash_table, key, len);

	if (UNLIKELY(*node != NULL))
		return NODE_value(*node);

	*node = hash_node_new(hash_table, key, len);
	hash_table->nnodes++;
	/*if (!hash_table->frozen)*/

	value = NODE_value(*node);
	hash_table_resize(hash_table);
	return value;
}


void HASH_TABLE_remove(HASH_TABLE *hash_table, const char *key, int len)
{
	HASH_NODE **node, *dest;

	node = hash_table_lookup_node(hash_table, key, len);

	if (LIKELY(*node != NULL))
	{
		dest = *node;
		(*node) = dest->next;

		#ifdef KEEP_ORDER

		if (dest->sprev)
			dest->sprev->snext = dest->snext;
		else
			hash_table->sfirst = dest->snext;

		if (dest->snext)
			dest->snext->sprev = dest->sprev;
		else
			hash_table->slast = dest->sprev;

		#endif

		hash_node_destroy(dest);
		hash_table->nnodes--;

		hash_table->last = NULL;

		/*if (!hash_table->frozen)*/
		hash_table_resize(hash_table);
	}
}


void *HASH_TABLE_next(HASH_TABLE *hash_table, HASH_ENUM *iter, bool set_last)
{
	#ifdef KEEP_ORDER

	if (iter->node == NULL)
		iter->node = hash_table->sfirst;
	else
		iter->node = iter->next;

	if (set_last)
		hash_table->last = iter->node;

	if (iter->node)
	{
		iter->next = iter->node->snext;
		return NODE_value(iter->node);
	}
	else
		return NULL;

	#else

	HASH_NODE *enum_node = iter->node;
	int enum_i = iter->index;

	if (enum_node)
		enum_node = enum_node->next;

	while (enum_node == NULL)
	{
		enum_i++;

		if (enum_i >= hash_table->size)
			break;

		enum_node = hash_table->nodes[enum_i];
	};

	iter->node = enum_node;
	iter->index = enum_i;

	if (set_last)
		hash_table->last = enum_node;

	if (enum_node)
		return NODE_value(enum_node);
	else
		return NULL;

	#endif
}

#if 0
static void dump_hash_table(HASH_TABLE *hash_table)
{
	int i;
	HASH_NODE *node;
	HASH_KEY *node_key;
	
	for (i = 0; i < hash_table->size; i++)
	{
		fprintf(stderr, "%5d: ", i);
		for (node = hash_table->nodes[i]; node; node = node->next)
		{
			node_key = NODE_key(hash_table, node);
			fprintf(stderr, "%.*s ", node_key->len, node_key->key);
		}
		fprintf(stderr, "\n");
	}
	fprintf(stderr, "\n");
}
#endif

static void hash_table_resize(HASH_TABLE *hash_table)
{
	HASH_NODE **old_nodes;
	HASH_NODE **new_nodes;
	HASH_NODE *node;
	HASH_NODE *next;
	HASH_KEY *node_key;
	int hash_val;
	int new_size;
	int i;
	HASH_FUNC hash_func = get_hash_func(hash_table);

	if (!((hash_table->size >= 3 * hash_table->nnodes && hash_table->size > HASH_TABLE_MIN_SIZE) ||
	      (3 * hash_table->size <= hash_table->nnodes && hash_table->size < HASH_TABLE_MAX_SIZE)))
		return;
	
	new_size = MinMax(spaced_primes_closest(hash_table->nnodes), HASH_TABLE_MIN_SIZE, HASH_TABLE_MAX_SIZE);

	//fprintf(stderr, "**** hash_table_resize %p %d: %d -> %d\n", hash_table, hash_table->nnodes, hash_table->size, new_size);
	
	//fprintf(stderr, "BEFORE:\n");
	//dump_hash_table(hash_table);
	
	old_nodes = hash_table->nodes;
	ALLOC_ZERO(&new_nodes, new_size * sizeof(HASH_NODE *));

	for (i = 0; i < hash_table->size; i++)
		for (node = hash_table->nodes[i]; node; node = next)
			{
				next = node->next;

				node_key = NODE_key(hash_table, node);
				hash_val = (*hash_func)(node_key->key, node_key->len) % new_size;

				node->next = new_nodes[hash_val];
				new_nodes[hash_val] = node;
			}

	FREE(&old_nodes);
	hash_table->nodes = new_nodes;
	hash_table->size = new_size;

	//fprintf(stderr, "AFTER:\n");
	//dump_hash_table(hash_table);
}


static HASH_NODE *hash_node_new(HASH_TABLE *hash_table, const char *key, int len)
{
	HASH_NODE *hash_node;
	int size;
	HASH_KEY *node_key;

	if (len > 65535)
		len = 65535;
	
	size = sizeof(HASH_NODE) + hash_table->s_value + sizeof(HASH_KEY) + len;
	ALLOC_ZERO(&hash_node, size);

	node_key = NODE_key(hash_table, hash_node);
	memcpy(node_key->key, key, len);
	node_key->len = len;
	//node_key->hash = hash_table->mode ? key_hash_text(key, len) : key_hash_binary(key, len);

	#ifdef KEEP_ORDER

	if (UNLIKELY(!hash_table->sfirst))
	{
		hash_table->sfirst = hash_node;
		hash_table->slast = hash_node;
	}
	else
	{
		hash_node->sprev = hash_table->slast;
		hash_node->sprev->snext = hash_node;
		hash_table->slast = hash_node;
	}

	#endif

	return hash_node;
}


static void hash_node_destroy(HASH_NODE *hash_node)
{
	FREE(&hash_node);
}

#ifdef KEEP_ORDER
#else
static void hash_nodes_destroy(HASH_NODE *hash_node)
{
	HASH_NODE *node = hash_node;
	HASH_NODE *next;

	for(;;)
	{
		if (UNLIKELY(node == NULL))
			return;

		next = node->next;
		FREE(&node);
		node = next;
	}
}
#endif

void HASH_TABLE_get_key(HASH_TABLE *hash_table, HASH_NODE *node, char **key, int *len)
{
	HASH_KEY *node_key;
	
	if (LIKELY(node != NULL))
	{
		node_key = NODE_key(hash_table, node);
		*key = node_key->key;
		*len = node_key->len;
	}
	else
		*len = 0;
}


bool HASH_TABLE_get_last_key(HASH_TABLE *hash_table, char **key, int *len)
{
	if (hash_table->last == NULL)
		return TRUE;

	HASH_TABLE_get_key(hash_table, hash_table->last, key, len);
	return FALSE;
}

void HASH_TABLE_set_last_key(HASH_TABLE *hash_table, char *key, int len)
{
	hash_table->last = (len == 0) ? NULL : *hash_table_lookup_node(hash_table, key, len);
}