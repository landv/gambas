/***************************************************************************

  gbx_c_subcollection.c

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

#define __GBX_C_SUBCOLLECTION_C

#include "gbx_info.h"

#ifndef GBX_INFO

#include "gb_common.h"
#include "gb_error.h"

#include "gambas.h"
#include "gb_array.h"
#include "gbx_api.h"
#include "gbx_class.h"
#include "gbx_object.h"
#include "gbx_string.h"

#include "gbx_c_subcollection.h"


static void free_string_array(char ***parray)
{
  int i;
  char **array = *parray;

  if (!*parray)
    return;

  for (i = 0; i < ARRAY_count(array); i++)
    GB_FreeString(&array[i]);

  ARRAY_delete(parray);
}



BEGIN_METHOD_VOID(CSUBCOLLECTION_free)

  *THIS->store = NULL;
  GB_Unref((void **)&THIS->container);
  free_string_array(&THIS->list);
  HASH_TABLE_delete(&THIS->hash_table);

  #if 0
  void *value;
  HASH_ENUM iter;

  CLEAR(&iter);

  for(;;)
  {
    value = HASH_TABLE_next(THIS->hash_table, &iter);
    if (value == NULL)
      break;

    GB_Unref(&value);
  }
  #endif

END_METHOD


BEGIN_PROPERTY(CSUBCOLLECTION_count)

  /*free_string_array(&THIS->list);*/
  if (!THIS->list)
    (*THIS->desc->list)(THIS->container, &THIS->list);

  if (THIS->list)
    GB_ReturnInt(ARRAY_count(THIS->list));
  else
    GB_ReturnInt(0);

END_PROPERTY


BEGIN_METHOD(CSUBCOLLECTION_exist, GB_STRING key)

	char *key = GB_ToZeroString(ARG(key));

	if (!key || !*key)
		GB_ReturnBoolean(FALSE);
	else
  	GB_ReturnBoolean((*THIS->desc->exist)(THIS->container, key));

END_METHOD


static void *get_from_key(CSUBCOLLECTION *_object, const char *key, long len)
{
  void *data;
  char *tkey;

	if (!key || !*key)
		return NULL;

  if (len <= 0)
    len = strlen(key);

  data =  HASH_TABLE_lookup(THIS->hash_table, key, len);
  if (!data)
  {
    STRING_new_temp(&tkey, key, len);
    data = (*THIS->desc->get)(THIS->container, tkey);
    if (!data)
      *((void **)HASH_TABLE_insert(THIS->hash_table, key, len)) = data;
  }

  return data;
}

BEGIN_METHOD_VOID(CSUBCOLLECTION_next)

  int *pos = (int *)GB_GetEnum();

  if (THIS->desc->list)
  {
    char *key = NULL;
    int n;

    if (*pos == 0)
    {
      free_string_array(&THIS->list);
      (*THIS->desc->list)(THIS->container, &THIS->list);
    }

    if (THIS->list)
    {
      if (*pos < ARRAY_count(THIS->list))
      {
        n = (*pos)++;
        key = THIS->list[n];
      }
    }

    if (!key || !*key)
      GB_StopEnum();
    else
      GB_ReturnObject(get_from_key(THIS, key, 0));
  }
  else
  {
    /*void *elt;

    elt = get_from_key(THIS, NULL, *pos);
    (*pos)++;
    if (elt)
      GB_ReturnObject(elt);
    else*/
      GB_StopEnum();
  }

END_METHOD


BEGIN_METHOD(CSUBCOLLECTION_get, GB_STRING key)

  GB_ReturnObject(get_from_key(THIS, STRING(key), LENGTH(key)));

END_METHOD

#endif

PUBLIC GB_DESC NATIVE_SubCollection[] =
{
  GB_DECLARE(".SubCollection", sizeof(CSUBCOLLECTION)),

  GB_METHOD("_free", NULL, CSUBCOLLECTION_free, NULL),

  GB_PROPERTY_READ("Count", "i", CSUBCOLLECTION_count),
  GB_PROPERTY_READ("Length", "i", CSUBCOLLECTION_count),

  GB_METHOD("Exist", "b", CSUBCOLLECTION_exist, "(Key)s"),
  GB_METHOD("_next", "o", CSUBCOLLECTION_next, NULL),
  GB_METHOD("_get", "o", CSUBCOLLECTION_get, "(Key)s"),

  GB_END_DECLARE
};


#ifndef GBX_INFO

PUBLIC void GB_SubCollectionNew(GB_SUBCOLLECTION *subcollection, GB_SUBCOLLECTION_DESC *desc, void *container)
{
  CSUBCOLLECTION *ob;
  CLASS *class;

  if (*subcollection)
    return;

  if (!desc->klass)
    class = (void *)CLASS_SubCollection;
  else
    class = CLASS_find(desc->klass);

  OBJECT_create_native((void **)(void *)&ob, class, NULL);

  ob->container = container;
  ob->store = subcollection;
  GB_Ref(container);
  ob->desc = desc;
  HASH_TABLE_create(&ob->hash_table, TYPE_sizeof(T_OBJECT), 0);

  *subcollection = ob;
}


PUBLIC void *GB_SubCollectionContainer(void *_object)
{
  return THIS->container;
}


PUBLIC void GB_SubCollectionAdd(void *_object, const char *key, long len, void *value)
{
  void **data;

  if (len < 0)
    len = strlen(key);

  data = (void **)HASH_TABLE_insert(THIS->hash_table, key, len);
  //GB_Ref(value);
  //GB_Unref(data);
  *data = value;
}

PUBLIC void GB_SubCollectionRemove(void *_object, const char *key, long len)
{
  void *data;

  if (!THIS)
    return;

  if (len < 0)
    len = strlen(key);

  data = HASH_TABLE_lookup(THIS->hash_table, key, len);
  if (data == NULL)
    return;

  //GB_Unref(data);
  HASH_TABLE_remove(THIS->hash_table, key, len);
}


PUBLIC void *GB_SubCollectionGet(void *_object, const char *key, long len)
{
  return get_from_key(THIS, key, len);
}

#endif
