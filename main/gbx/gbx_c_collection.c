/***************************************************************************

  gbx_c_collection.c

  (c) 2000-2009 Benoît Minisini <gambas@users.sourceforge.net>

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
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

***************************************************************************/

#define __GBX_C_COLLECTION_C

#include "gbx_info.h"

#ifndef GBX_INFO

#include "gb_common.h"
#include "gb_error.h"

#include "gbx_variant.h"
#include "gambas.h"
#include "gbx_api.h"
#include "gbx_class.h"
#include "gbx_object.h"

#include "gbx_c_collection.h"


static void collection_free(CCOLLECTION *col)
{
  VARIANT *value;
  HASH_ENUM iter;

  CLEAR(&iter);
	col->locked = TRUE;
	
  for(;;)
  {
    value = HASH_TABLE_next(col->hash_table, &iter);
    if (value == NULL)
      break;

    VARIANT_free(value);
		value->type = T_NULL;
  }

  HASH_TABLE_delete(&col->hash_table);
	col->locked = FALSE;
}

static void *collection_get_key(CCOLLECTION *col, const char *key, int len)
{
  if (len == 0)
    return NULL;

  if (len <= 0)
    len = strlen(key);

  return HASH_TABLE_lookup(col->hash_table, key, len);
}


static void *collection_add_key(CCOLLECTION *col, const char *key, int len)
{
  if (len == 0)
  {
    GB_Error((char *)E_VKEY);
    return NULL;
  }

  if (len <= 0)
    len = strlen(key);

  return HASH_TABLE_insert(col->hash_table, key, len);
}


static void collection_remove_key(CCOLLECTION *col, const char *key, int len)
{
  void *value;
  HASH_NODE *last;

  if (len == 0)
  {
    GB_Error((char *)E_VKEY);
    return;
  }

  if (len < 0)
    len = strlen(key);

  value = HASH_TABLE_lookup(col->hash_table, key, len);
  if (value == NULL)
    return;

  last = col->hash_table->last;

  GB_ListEnum(col);
  while (!GB_NextEnum())
  {
    HASH_ENUM *iter = (HASH_ENUM *)GB_GetEnum();
    /*char *ekey;
    int elen;

    HASH_TABLE_get_key(col->hash_table, iter->node, &ekey, &elen);
    fprintf(stderr, "iter->node: %.*s\n", elen, ekey);
    HASH_TABLE_get_key(col->hash_table, iter->next, &ekey, &elen);
    fprintf(stderr, "iter->next: %.*s\n", elen, ekey);*/

    if (iter->next == last)
    {
      iter->next = iter->next->snext;
      /*HASH_TABLE_get_key(col->hash_table, iter->next, &ekey, &elen);
      fprintf(stderr, "iter->next: => %.*s\n", elen, ekey);*/
    }
  }

  //col->hash_table->last = last;

  VARIANT_free(value);
	if (!col->locked)
		HASH_TABLE_remove(col->hash_table, key, len);
}


BEGIN_METHOD(CCOLLECTION_new, GB_INTEGER mode)

  int mode = MISSING(mode) ? 0 : VARG(mode);

  HASH_TABLE_create(&THIS->hash_table, TYPE_sizeof(T_VARIANT), mode);
  THIS->last = NULL;

END_METHOD


BEGIN_METHOD_VOID(CCOLLECTION_free)

  collection_free(THIS);

END_METHOD


BEGIN_PROPERTY(collection_count)

  GB_ReturnInt(HASH_TABLE_size(THIS->hash_table));

END_PROPERTY


BEGIN_PROPERTY(collection_key)

  char *key;
  int len;

  if (HASH_TABLE_get_last_key(THIS->hash_table, &key, &len))
    GB_ReturnNull();
  else
    GB_ReturnNewString(key, len);

END_PROPERTY


BEGIN_METHOD(collection_add, GB_VARIANT value; GB_STRING key)

  void *data;

  data = collection_add_key(THIS, STRING(key), LENGTH(key));
  if (!data)
    return;

  GB_StoreVariant(ARG(value), data);

END_METHOD


BEGIN_METHOD(collection_exist, GB_STRING key)

	/*if (LENGTH(key) == 0)
		GB_ReturnBoolean(FALSE);
	else*/
  	GB_ReturnBoolean(collection_get_key(THIS, STRING(key), LENGTH(key)) != NULL);

END_METHOD


BEGIN_METHOD(collection_remove, GB_STRING key)

  collection_remove_key(THIS, STRING(key), LENGTH(key));

END_METHOD


BEGIN_METHOD_VOID(collection_clear)

  int mode = THIS->hash_table->mode;

  /* Stops all iterators */
  GB_StopAllEnum(THIS);

  collection_free(THIS);
  HASH_TABLE_create(&THIS->hash_table, TYPE_sizeof(T_VARIANT), mode);

END_METHOD


#if 0
BEGIN_METHOD_VOID(collection_first)

  HASH_ENUM *iter = (HASH_ENUM *)GB_GetEnum();

  iter->index = (-1);
  iter->node = NULL;

END_METHOD
#endif


BEGIN_METHOD_VOID(collection_next)

  void *value;
  HASH_TABLE *hash_table = OBJECT(CCOLLECTION)->hash_table;
  HASH_ENUM *iter = (HASH_ENUM *)GB_GetEnum();

  value = HASH_TABLE_next(hash_table, iter);

  if (value == NULL)
    GB_StopEnum();
  else
    GB_ReturnPtr(T_VARIANT, value);

END_METHOD


/*
BEGIN_METHOD(collection_copy, CCOLLECTION *copy)

  CCOLLECTION *copy = PARAM(copy);
  HASH_TABLE *hash_table = THIS->hash_table;
  CCOL_ENUM enum_state;
  void *value;

  collection_clear(THIS, NULL);

  enum_state.i = -1;
  enum_state.node = NULL;

  for(;;)
  {
    value = HASH_TABLE_next(hash_table, &enum_state->i, &enum_state->node);
    if (value == NULL)
      break;

    GB_ReturnPtr(OBJECT(CCOLLECTION)->type, value);
    data = collection_add_key(THIS, PARAM(key).addr, PARAM(key).len);
    GB_Store(T_VARIANT, &PARAM(value), data);

END_METHOD
*/

BEGIN_METHOD(collection_get, GB_STRING key)

  GB_ReturnPtr(T_VARIANT, collection_get_key(THIS, STRING(key), LENGTH(key)));

END_METHOD


BEGIN_METHOD(collection_put, GB_VARIANT value; GB_STRING key)

  void *data;

  if (VARIANT_is_null((VARIANT *)&VARG(value)))
    collection_remove_key(THIS, STRING(key), LENGTH(key));
  else
  {
    data = collection_add_key(THIS, STRING(key), LENGTH(key));
    if (!data)
      return;
    GB_StoreVariant(ARG(value), data);
  }

END_METHOD

#endif


GB_DESC NATIVE_Collection[] =
{
  GB_DECLARE("Collection", sizeof(CCOLLECTION)),

  GB_METHOD("_new", NULL, CCOLLECTION_new, "[(Mode)i]"),
  GB_METHOD("_free", NULL, CCOLLECTION_free, NULL),

  GB_PROPERTY_READ("Count", "i", collection_count),
  GB_PROPERTY_READ("Length", "i", collection_count),
  GB_PROPERTY_READ("Key", "s", collection_key),

  GB_METHOD("Add", NULL, collection_add, "(Value)v(Key)s"),
  GB_METHOD("Exist", "b", collection_exist, "(Key)s"),
  GB_METHOD("Remove", NULL, collection_remove, "(Key)s"),
  GB_METHOD("Clear", NULL, collection_clear, NULL),
  /*GB_METHOD("_first", NULL, collection_first, NULL),*/
  GB_METHOD("_next", "v", collection_next, NULL),
  GB_METHOD("_get", "v", collection_get, "(Key)s"),
  GB_METHOD("_put", NULL, collection_put, "(Value)v(Key)s"),

  GB_END_DECLARE
};

#ifndef GBX_INFO

void GB_CollectionNew(GB_COLLECTION *col, int mode)
{
  GB_INTEGER param;

  param.type = GB_T_INTEGER;
  param.value = mode;

  OBJECT_create_native((void **)col, CLASS_Collection, (VALUE *)&param);
}

int GB_CollectionCount(GB_COLLECTION col)
{
  return HASH_TABLE_size(((CCOLLECTION *)col)->hash_table);
}

int GB_CollectionSet(GB_COLLECTION col, const char *key, int len, GB_VARIANT *value)
{
  VARIANT *data;

  if (VARIANT_is_null((VARIANT *)&value->value))
    collection_remove_key((CCOLLECTION *)col, key, len);
  else
  {
    data = (VARIANT *)collection_add_key((CCOLLECTION *)col, key, len);
    if (!data)
    	return TRUE;
    GB_StoreVariant(value, data);
  }
  return FALSE;
}

int GB_CollectionGet(GB_COLLECTION col, const char *key, int len, GB_VARIANT *value)
{
  VARIANT *var;

  var = (VARIANT *)collection_get_key((CCOLLECTION *)col, key, len);
  if (var)
  {
    value->type = GB_T_VARIANT;
    value->value.type = var->type;
		value->value._long.value = var->value.data;
    return FALSE;
  }
  else
  {
    value->type = GB_T_NULL;
    return TRUE;
  }
}

int GB_CollectionEnum(GB_COLLECTION col, GB_VARIANT *value, char **key, int *len)
{
	static HASH_ENUM iter;
  VARIANT *val;
  HASH_TABLE *hash_table = ((CCOLLECTION *)col)->hash_table;

	if (!value || !key)
	{
		CLEAR(&iter);
		return FALSE;
	}

  val = HASH_TABLE_next(hash_table, &iter);
  if (!val)
  	return TRUE;

  value->type = GB_T_VARIANT;
	value->value._long.value = val->value.data;

	HASH_TABLE_get_last_key(hash_table, key, len);
	return FALSE;
}

#endif
