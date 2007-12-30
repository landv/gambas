/***************************************************************************

  hash.c

  hash table routines, adapted from glib 1.2.8

  (c) 2000-2006 Benoît Minisini <gambas@users.sourceforge.net>

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

#define __GBX_HASH_C

#include <ctype.h>

#include "gb_common.h"
#include "gb_common_case.h"
#include "gb_alloc.h"
#include "gbx_hash.h"


#define NODE_key(htable, node) (((char *)node) + sizeof(HASH_NODE) + htable->s_value)
#define NODE_value(node) ((void *)((char *)node) + sizeof(HASH_NODE))

static void hash_table_resize(HASH_TABLE *hash_table);
static HASH_NODE **hash_table_lookup_node(HASH_TABLE *hash_table, const char *key, long len);
static HASH_NODE *hash_node_new(HASH_TABLE *hash_table, const char *key, long len);
static void hash_node_destroy(HASH_NODE *hash_node);
static void hash_nodes_destroy(HASH_NODE *hash_node);


static const long primes[] =
{
  11, 19, 37, 73, 109, 163, 251, 367, 557, 823, 1237, 1861, 2777, 4177, 6247, 9371,
  14057, 21089, 31627, 47431, 71143, 106721, 160073, 240101, 360163, 540217, 810343,
  1215497, 1823231, 2734867, 4102283, 6153409, 9230113, 13845163
};

static const long nprimes = sizeof (primes) / sizeof (primes[0]);

static long spaced_primes_closest(long num)
{
  int i;

  for (i = 0; i < nprimes; i++)
    if (primes[i] > num)
      return primes[i];

  return primes[nprimes - 1];
}


static boolean key_equal_binary(const char *key_comp, const char *key, long len)
{
  return (!strncmp(key_comp, key, len) && (!key_comp[len]));
}

static boolean key_equal_text(const char *key_comp, const char *key, long len)
{
  return (!strncasecmp(key_comp, key, len) && (!key_comp[len]));
}


static ulong key_hash_binary(const char *key, long len)
{
  int i;
  ulong hash = 0;

  if (len >= 0)
  {
    for (i = 0; i < len; i++)
      hash = (hash << 4) + (hash ^ key[i]);
  }
  else
  {
    for(i = 0; ; i++)
    {
      if (!key[i])
        break;
      hash = (hash << 4) + (hash ^ key[i]);
    }
  }

  return hash;
}


static ulong key_hash_text(const char *key, long len)
{
  int i;
  ulong hash = 0;

  if (len >= 0)
  {
    for (i = 0; i < len; i++)
      hash = (hash << 4) + (hash ^ toupper(key[i]));
  }
  else
  {
    for(i = 0; ; i++)
    {
      if (!key[i])
        break;
      hash = (hash << 4) + (hash ^ toupper(key[i]));
    }
  }

  return hash;
}


static HASH_FUNC get_hash_func(HASH_TABLE *hash)
{
  if (hash->mode == GB_COMP_TEXT)
    return key_hash_text;
  else
    return key_hash_binary;
}


PUBLIC void HASH_TABLE_create(HASH_TABLE **hash, size_t s_value, int mode)
{
  HASH_TABLE *hash_table;
  /*int i;*/

  ALLOC_ZERO(&hash_table, sizeof(HASH_TABLE), "HASH_TABLE_create");

  hash_table->size = HASH_TABLE_MIN_SIZE;
  hash_table->s_value = s_value;

  ALLOC_ZERO(&hash_table->nodes, sizeof(HASH_NODE) * hash_table->size, "HASH_TABLE_create");

  if (mode == GB_COMP_TEXT)
    hash_table->mode = mode;
  else
    hash_table->mode = GB_COMP_BINARY;

  *hash = hash_table;
}


PUBLIC void HASH_TABLE_delete(HASH_TABLE **hash)
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


PUBLIC long HASH_TABLE_size(HASH_TABLE *hash_table)
{
  return hash_table->nnodes;
}


static HASH_NODE **hash_table_lookup_node(HASH_TABLE *hash_table, const char *key, long len)
{
  HASH_NODE **node;

	if (hash_table->mode == GB_COMP_TEXT)
	{
	  node = &hash_table->nodes[key_hash_text(key, len) % hash_table->size];

  	while (*node && !key_equal_text(NODE_key(hash_table, *node), key, len))
    	node = &(*node)->next;
	}
	else
	{
		node = &hash_table->nodes[key_hash_binary(key, len) % hash_table->size];
	
		while (*node && !key_equal_binary(NODE_key(hash_table, *node), key, len))
			node = &(*node)->next;
	}

  return node;
}


PUBLIC void *HASH_TABLE_lookup(HASH_TABLE *hash_table, const char *key, long len)
{
  HASH_NODE *node;

  node = *hash_table_lookup_node(hash_table, key, len);
  hash_table->last = node;

  return node ? NODE_value(node) : NULL;
}


PUBLIC void *HASH_TABLE_insert(HASH_TABLE *hash_table, const char *key, long len)
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


PUBLIC void HASH_TABLE_remove(HASH_TABLE *hash_table, const char *key, long len)
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


PUBLIC void *HASH_TABLE_next(HASH_TABLE *hash_table, HASH_ENUM *iter)
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
  long enum_i = iter->index;

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

/*
void HASH_TABLE_foreach (HASH_TABLE *hash_table,
        GHFunc   func,
        gpointer   user_data)
{
  HASH_NODE *node;
  gint i;

  g_return_if_fail (hash_table != NULL);
  g_return_if_fail (func != NULL);

  for (i = 0; i < hash_table->size; i++)
    for (node = hash_table->nodes[i]; node; node = node->next)
      (* func) (node->key, node->value, user_data);
}
*/

static void hash_table_resize(HASH_TABLE *hash_table)
{
  HASH_NODE **new_nodes;
  HASH_NODE *node;
  HASH_NODE *next;
  char *node_key;
  double nodes_per_list;
  long hash_val;
  long new_size;
  long i;
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
        hash_val = (*hash_func)(node_key, -1) % new_size;

        node->next = new_nodes[hash_val];
        new_nodes[hash_val] = node;
      }

  FREE(&hash_table->nodes, "hash_table_resize");
  hash_table->nodes = new_nodes;
  hash_table->size = new_size;
}


static HASH_NODE *hash_node_new(HASH_TABLE *hash_table, const char *key, long len)
{
  HASH_NODE *hash_node;
  int size;
  char *node_key;

  size = sizeof(HASH_NODE) + hash_table->s_value + len + 1;
  ALLOC_ZERO(&hash_node, size, "hash_node_new");

  node_key = NODE_key(hash_table, hash_node);
  strncpy(node_key, key, len);
  node_key[len] = 0;

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


PUBLIC void HASH_TABLE_get_key(HASH_TABLE *hash_table, HASH_NODE *node, char **key, long *len)
{
  if (node)
  {
    *key = NODE_key(hash_table, node);
    *len = strlen(*key);
  }
  else
    *len = 0;
}


PUBLIC bool HASH_TABLE_get_last_key(HASH_TABLE *hash_table, char **key, long *len)
{
  if (hash_table->last == NULL)
    return TRUE;

  HASH_TABLE_get_key(hash_table, hash_table->last, key, len);
  return FALSE;
}
