/***************************************************************************

  c_subcollection.c

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

#define __C_SUBCOLLECTION_C

#include "main.h"
#include "c_subcollection.h"


static void free_string_array(char ***parray)
{
  int i;
  char **array = *parray;

  if (!*parray)
    return;

  for (i = 0; i < GB.Count(array); i++)
    GB.FreeString(&array[i]);

  GB.FreeArray(parray);
}

static void *get_from_key(CSUBCOLLECTION *_object, const char *key, int len)
{
  void *data;
  char *tkey;

	if (!key || !*key)
		return NULL;

  if (len <= 0)
    len = strlen(key);

	//fprintf(stderr, "get_from_key: %.*s\n", len, key);

  if (GB.HashTable.Get(THIS->hash_table, key, len, &data))
	{
		tkey = GB.TempString(key, len);
		data = (*THIS->desc->get)(THIS->container, tkey);
		if (data)
		{
			//fprintf(stderr, "get_from_key: insert %p '%.*s'\n", data, len, key);
			GB.HashTable.Add(THIS->hash_table, key, len, data);
			GB.Ref(data);
		}
	}
	
  return data;
}

static CSUBCOLLECTION *_current = NULL;

static void clear_one(void *data)
{
	CSUBCOLLECTION *save = _current;

	if (_current->desc->release)
		(*_current->desc->release)(_current->container, data);

	//fprintf(stderr, "clear: %p\n", data);
	GB.Unref(&data);

	_current = save;
}

static void clear_subcollection(CSUBCOLLECTION *_object)
{
	_current = THIS;
	GB.HashTable.Enum(THIS->hash_table, clear_one);
	GB.HashTable.Free(&THIS->hash_table);
}


BEGIN_METHOD_VOID(CSUBCOLLECTION_free)

  //*THIS->store = NULL;
  free_string_array(&THIS->list);
  
  clear_subcollection(THIS);

END_METHOD


BEGIN_PROPERTY(CSUBCOLLECTION_count)

  /*free_string_array(&THIS->list);*/
  if (!THIS->list)
    (*THIS->desc->list)(THIS->container, &THIS->list);

  if (THIS->list)
    GB.ReturnInteger(GB.Count(THIS->list));
  else
    GB.ReturnInteger(0);

END_PROPERTY


BEGIN_METHOD(CSUBCOLLECTION_exist, GB_STRING key)

	char *key = GB.ToZeroString(ARG(key));

	if (!key || !*key)
		GB.ReturnBoolean(FALSE);
	else
  	GB.ReturnBoolean((*THIS->desc->exist)(THIS->container, key));

END_METHOD


BEGIN_METHOD_VOID(CSUBCOLLECTION_next)

  int *pos = (int *)GB.GetEnum();

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
      if (*pos < GB.Count(THIS->list))
      {
        n = (*pos)++;
        key = THIS->list[n];
      }
    }

    if (!key || !*key)
      GB.StopEnum();
    else
      GB.ReturnObject(get_from_key(THIS, key, 0));
  }
  else
  {
    /*void *elt;

    elt = get_from_key(THIS, NULL, *pos);
    (*pos)++;
    if (elt)
      GB_ReturnObject(elt);
    else*/
      GB.StopEnum();
  }

END_METHOD


BEGIN_METHOD(CSUBCOLLECTION_get, GB_STRING key)

  GB.ReturnObject(get_from_key(THIS, STRING(key), LENGTH(key)));

END_METHOD

BEGIN_METHOD_VOID(CSUBCOLLECTION_refresh)

	clear_subcollection(THIS);
	GB.HashTable.New(&THIS->hash_table, GB_COMP_BINARY);

END_METHOD

GB_DESC SubCollectionDesc[] =
{
  GB_DECLARE(".SubCollection", sizeof(CSUBCOLLECTION)),

  GB_METHOD("_free", NULL, CSUBCOLLECTION_free, NULL),

  GB_PROPERTY_READ("Count", "i", CSUBCOLLECTION_count),
  //GB_PROPERTY_READ("Length", "i", CSUBCOLLECTION_count),

  GB_METHOD("Exist", "b", CSUBCOLLECTION_exist, "(Key)s"),
  GB_METHOD("_next", "o", CSUBCOLLECTION_next, NULL),
  GB_METHOD("_get", "o", CSUBCOLLECTION_get, "(Key)s"),
  GB_METHOD("Refresh", NULL, CSUBCOLLECTION_refresh, NULL),

  GB_END_DECLARE
};


void GB_SubCollectionNew(CSUBCOLLECTION **subcollection, SUBCOLLECTION_DESC *desc, void *container)
{
  CSUBCOLLECTION *ob;

  if (*subcollection)
    return;

  ob = GB.New(GB.FindClass(desc->klass), NULL, NULL);

  ob->container = container;
  //ob->store = subcollection;
  //GB_Ref(container);
  ob->desc = desc;
	GB.HashTable.New(&ob->hash_table, GB_COMP_BINARY);

  *subcollection = ob;
  GB.Ref(ob);
}


void *GB_SubCollectionContainer(void *_object)
{
  return THIS->container;
}


void GB_SubCollectionAdd(void *_object, const char *key, int len, void *value)
{
	void *old_value = NULL;

  if (len <= 0)
    len = strlen(key);

	//fprintf(stderr, "GB_SubCollectionAdd: insert %p '%.*s'\n", value, len, key);
	GB.HashTable.Get(THIS->hash_table, key, len, &old_value);
	GB.HashTable.Add(THIS->hash_table, key, len, value);
  GB.Ref(value);
  GB.Unref(&old_value);
}

void GB_SubCollectionRemove(void *_object, const char *key, int len)
{
  void *old_value;

  if (!THIS)
    return;

  if (len <= 0)
    len = strlen(key);
	
	if (!GB.HashTable.Get(THIS->hash_table, key, len, &old_value))
	{
		//fprintf(stderr, "GB_SubCollectionRemove: remove %p '%.*s'\n", old_value, len, key);
		GB.HashTable.Remove(THIS->hash_table, key, len);
		GB.Unref(&old_value);
	}
}


void *GB_SubCollectionGet(void *_object, const char *key, int len)
{
  return get_from_key(THIS, key, len);
}
