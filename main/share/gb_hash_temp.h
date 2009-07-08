/***************************************************************************

  hash.c

  hash table routines, adapted from glib 1.2.8

  (c) 2000-2007 Benoit Minisini <gambas@users.sourceforge.net>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 1, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

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
static void hash_nodes_destroy(HASH_NODE *hash_node);


static const int primes[] =
{
  11, 19, 37, 73, 109, 163, 251, 367, 557, 823, 1237, 1861, 2777, 4177, 6247, 9371,
  14057, 21089, 31627, 47431, 71143, 106721, 160073, 240101, 360163, 540217, 810343,
  1215497, 1823231, 2734867, 4102283, 6153409, 9230113, 13845163
};

static const int nprimes = sizeof (primes) / sizeof (primes[0]);

static int spaced_primes_closest(int num)
{
  int i;

  for (i = 0; i < nprimes; i++)
    if (primes[i] > num)
      return primes[i];

  return primes[nprimes - 1];
}


static uint key_hash_binary(const char *key, int len)
{
	static const void *jump[4] = { &&__LEN_0, &&__LEN_1, &&__LEN_2, &&__LEN_3 };
  uint hash = 0;

  //for (i = 0; i < len; i++)
  //  hash = (hash << 4) + (hash ^ key[i]);

	while (len >= 4)
	{
		len -= 4;
		hash = (hash << 4) + (hash ^ key[len]);
		hash = (hash << 4) + (hash ^ key[len + 1]);
		hash = (hash << 4) + (hash ^ key[len + 2]);
		hash = (hash << 4) + (hash ^ key[len + 3]);
	}

	goto *jump[len];

__LEN_3:
	hash = (hash << 4) + (hash ^ key[2]);
__LEN_2:
	hash = (hash << 4) + (hash ^ key[1]);
__LEN_1:
	hash = (hash << 4) + (hash ^ key[0]);
__LEN_0:
  
  /*while (len)
  {
    len--;
    hash = (hash << 4) + (hash ^ key[len]);
  }*/
  
  return hash;
}


static uint key_hash_text(const char *key, int len)
{
	static const void *jump[4] = { &&__LEN_0, &&__LEN_1, &&__LEN_2, &&__LEN_3 };
  uint hash = 0;

	while (len >= 4)
	{
		len--;
		hash = (hash << 4) + (hash ^ (key[len] & ~0x20));
		len--;
		hash = (hash << 4) + (hash ^ (key[len] & ~0x20));
		len--;
		hash = (hash << 4) + (hash ^ (key[len] & ~0x20));
		len--;
		hash = (hash << 4) + (hash ^ (key[len] & ~0x20));
	}

	goto *jump[len];

__LEN_3:
	hash = (hash << 4) + (hash ^ (key[2] & ~0x20));
__LEN_2:
	hash = (hash << 4) + (hash ^ (key[1] & ~0x20));
__LEN_1:
	hash = (hash << 4) + (hash ^ (key[0] & ~0x20));
__LEN_0:
  
  /*while (len)
  {
    len--;
    hash = (hash << 4) + (hash ^ (key[len] & ~0x20));
  }*/

  return hash;
}


static HASH_FUNC get_hash_func(HASH_TABLE *hash)
{
  if (hash->mode == HF_IGNORE_CASE)
    return key_hash_text;
  else
    return key_hash_binary;
}


void HASH_TABLE_create(HASH_TABLE **hash, size_t s_value, HASH_FLAG mode)
{
  HASH_TABLE *hash_table;
  /*int i;*/

  ALLOC_ZERO(&hash_table, sizeof(HASH_TABLE), "HASH_TABLE_create");

  hash_table->size = HASH_TABLE_MIN_SIZE;
  hash_table->s_value = s_value;

  ALLOC_ZERO(&hash_table->nodes, sizeof(HASH_NODE) * hash_table->size, "HASH_TABLE_create");

  if (mode == HF_IGNORE_CASE)
    hash_table->mode = mode;
  else
    hash_table->mode = HF_NORMAL;

  *hash = hash_table;
}


void HASH_TABLE_delete(HASH_TABLE **hash)
{
  int i;
  HASH_TABLE *hash_table = *hash;

  if (hash_table == NULL)
    return;

  for (i = 0; i < hash_table->size; i++)
    hash_nodes_destroy(hash_table->nodes[i]);

  FREE(&hash_table->nodes, "HASH_TABLE_delete");
  FREE(hash, "HASH_TABLE_delete");
}


int HASH_TABLE_size(HASH_TABLE *hash_table)
{
  return hash_table->nnodes;
}

#if 0
static bool compare_key_ignore_case(const char *key_comp, int len_comp, const char *key, int len)
{
  if (len != len_comp)
    return FALSE;
    
  while (len)
  {
    len--;
    if (toupper(key_comp[len]) != toupper(key[len]))
      return FALSE;      
  }
  
  return TRUE;
}

static bool compare_key(const char *key_comp, int len_comp, const char *key, int len)
{
  if (len != len_comp)
    return FALSE;
    
  while (len)
  {
    len--;
    if (key_comp[len] != key[len])
      return FALSE;      
  }
  
  return TRUE;
}
#endif

static HASH_NODE **hash_table_lookup_node(HASH_TABLE *hash_table, const char *key, int len)
{
  HASH_NODE **node;
  HASH_KEY *node_key;

	if (hash_table->mode == HF_IGNORE_CASE)
	{
	  node = &hash_table->nodes[key_hash_text(key, len) % hash_table->size];

    while (*node)
    {
      node_key = NODE_key(hash_table, *node);
      if (node_key->len == len && STRING_equal_ignore_case(node_key->key, node_key->len, key, len))
        break;
    	node = &(*node)->next;
    }
  }
	else
	{
		node = &hash_table->nodes[key_hash_binary(key, len) % hash_table->size];
	
    while (*node)
    {
      node_key = NODE_key(hash_table, *node);
      if (node_key->len == len && STRING_equal_same(node_key->key, key, len))
        break;
    	node = &(*node)->next;
    }
	}

  return node;
}


void *HASH_TABLE_lookup(HASH_TABLE *hash_table, const char *key, int len)
{
  HASH_NODE *node;

  node = *hash_table_lookup_node(hash_table, key, len);
  hash_table->last = node;

  return node ? NODE_value(node) : NULL;
}


void *HASH_TABLE_insert(HASH_TABLE *hash_table, const char *key, int len)
{
  HASH_NODE **node;

  node = hash_table_lookup_node(hash_table, key, len);

  if (*node)
    return NODE_value(*node);

  /*hash_table_resize(hash_table);*/

  *node = hash_node_new(hash_table, key, len);

  hash_table->nnodes++;

  /*if (!hash_table->frozen)*/

  return NODE_value(*node);
}


void HASH_TABLE_remove(HASH_TABLE *hash_table, const char *key, int len)
{
  HASH_NODE **node, *dest;

  node = hash_table_lookup_node(hash_table, key, len);

  if (*node)
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


void *HASH_TABLE_next(HASH_TABLE *hash_table, HASH_ENUM *iter)
{
  #ifdef KEEP_ORDER

  if (iter->node == NULL)
    iter->node = hash_table->sfirst;
  else
    iter->node = iter->next;

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

  hash_table->last = enum_node;

  if (enum_node)
    return NODE_value(enum_node);
  else
    return NULL;

  #endif
}


static void hash_table_resize(HASH_TABLE *hash_table)
{
  HASH_NODE **new_nodes;
  HASH_NODE *node;
  HASH_NODE *next;
  HASH_KEY *node_key;
  double nodes_per_list;
  int hash_val;
  int new_size;
  int i;
  HASH_FUNC hash_func = get_hash_func(hash_table);


  nodes_per_list = hash_table->nnodes / hash_table->size;

  if ((nodes_per_list > 0.3 || hash_table->size <= HASH_TABLE_MIN_SIZE) &&
      (nodes_per_list < 3.0 || hash_table->size >= HASH_TABLE_MAX_SIZE))
    return;

  new_size = MinMax(spaced_primes_closest(hash_table->nnodes), HASH_TABLE_MIN_SIZE, HASH_TABLE_MAX_SIZE);

  ALLOC_ZERO(&new_nodes, new_size * sizeof(HASH_NODE *), "hash_table_resize");

  for (i = 0; i < hash_table->size; i++)
    for (node = hash_table->nodes[i]; node; node = next)
      {
        next = node->next;

        node_key = NODE_key(hash_table, node);
        hash_val = (*hash_func)(node_key->key, node_key->len) % new_size;

        node->next = new_nodes[hash_val];
        new_nodes[hash_val] = node;
      }

  FREE(&hash_table->nodes, "hash_table_resize");
  hash_table->nodes = new_nodes;
  hash_table->size = new_size;
}


static HASH_NODE *hash_node_new(HASH_TABLE *hash_table, const char *key, int len)
{
  HASH_NODE *hash_node;
  int size;
  HASH_KEY *node_key;

  size = sizeof(HASH_NODE) + hash_table->s_value + len + sizeof(ushort);
  ALLOC_ZERO(&hash_node, size, "hash_node_new");

  node_key = NODE_key(hash_table, hash_node);
  memcpy(node_key->key, key, len);
  node_key->len = len;

  #ifdef KEEP_ORDER

  if (!hash_table->sfirst)
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
  FREE(&hash_node, "hash_node_destroy");
}

static void hash_nodes_destroy(HASH_NODE *hash_node)
{
  HASH_NODE *node = hash_node;
  HASH_NODE *next;

  for(;;)
  {
    if (node == NULL)
      return;

    next = node->next;
    FREE(&node, "hash_nodes_destroy");
    node = next;
  }
}


void HASH_TABLE_get_key(HASH_TABLE *hash_table, HASH_NODE *node, char **key, int *len)
{
  HASH_KEY *node_key;
  
  if (node)
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
