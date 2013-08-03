/***************************************************************************

  CIndex.c

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

#define __CINDEX_C

#include "main.h"

#include "CIndex.h"


static int valid_index(CINDEX *_object)
{
  return !THIS->table || !THIS->table->conn || !THIS->table->conn->db.handle;
}


static bool exist_index(CTABLE *table, const char *name)
{
	if (!name || !*name)
		return FALSE;

	return table->driver->Index.Exist(&table->conn->db, table->name, (char *)name);
}

static bool check_index(CTABLE *table, const char *name, bool must_exist)
{
  bool exist = exist_index(table, name);

  if (must_exist)
  {
    if (!exist)
    {
      GB.Error("Unknown index: &1.&2", table->name, name);
      return TRUE;
    }
  }
  else
  {
    if (exist)
    {
      GB.Error("Index already exists: &1.&2", table->name, name);
      return TRUE;
    }
  }

  return FALSE;
}


static CINDEX *make_index(CTABLE *table, const char *name, bool must_exist)
{
  CINDEX *_object;

  if (check_index(table, name, must_exist))
    return NULL;

  _object = GB.New(GB.FindClass("Index"), NULL, NULL);
  THIS->table = table;
  THIS->driver = table->conn->driver;
  THIS->name = GB.NewZeroString(name);

  return _object;
}


void *CINDEX_get(CTABLE *table, const char *name)
{
  CINDEX *index = make_index(table, name, TRUE);
  table->driver->Index.Info(&table->conn->db, table->name, (char *)name, &index->info);
  return index;
}


int CINDEX_exist(CTABLE *table, const char *name)
{
  return exist_index(table, name);
}


void CINDEX_list(CTABLE *table, char ***list)
{
  table->driver->Index.List(&table->conn->db, table->name, list);
}

void CINDEX_release(CTABLE *table, void *_object)
{
	THIS->table = NULL;
}


/***************************************************************************

  Index

***************************************************************************/

BEGIN_METHOD_VOID(CINDEX_free)

	if (!valid_index(THIS))
  	GB_SubCollectionRemove(THIS->table->indexes, THIS->name, 0);

  GB.FreeString(&THIS->name);

  GB.FreeString(&THIS->info.name);
  GB.FreeString(&THIS->info.fields);
  THIS->info.unique = FALSE;

END_METHOD



BEGIN_PROPERTY(CINDEX_name)

  GB.ReturnString(THIS->name);

END_PROPERTY


BEGIN_PROPERTY(CINDEX_fields)

	GB_ARRAY array;
	char *fields;
	char *name;
	
	fields = GB.NewZeroString(THIS->info.fields);
	GB.Array.New(&array, GB_T_STRING, 0);
	
	name = strtok(fields, ",");
	while (name)
	{
		*((char **)GB.Array.Add(array)) = GB.NewZeroString(name);
		name = strtok(NULL, ",");
	}

	GB.FreeString(&fields);
  GB.ReturnObject(array);

END_PROPERTY


BEGIN_PROPERTY(CINDEX_unique)

  GB.ReturnBoolean(THIS->info.unique);

END_PROPERTY


BEGIN_PROPERTY(CINDEX_primary)

  GB.ReturnBoolean(THIS->info.primary);

END_PROPERTY


BEGIN_PROPERTY(CINDEX_table)

  GB.ReturnObject(THIS->table);

END_PROPERTY


GB_DESC CIndexDesc[] =
{
  GB_DECLARE("Index", sizeof(CINDEX)),
  GB_NOT_CREATABLE(),
  GB_HOOK_CHECK(valid_index),

  GB_METHOD("_free", NULL, CINDEX_free, NULL),

  GB_PROPERTY_READ("Name", "s", CINDEX_name),
  GB_PROPERTY_READ("Unique", "b", CINDEX_unique),
  GB_PROPERTY_READ("Primary", "b", CINDEX_primary),
  GB_PROPERTY_READ("Fields", "String[]", CINDEX_fields),

  GB_PROPERTY_READ("Table", "Table", CINDEX_table),

  GB_END_DECLARE
};


/***************************************************************************

  .Table.Indexes

***************************************************************************/

#undef THIS
#define THIS ((CSUBCOLLECTION *)_object)

BEGIN_METHOD(CINDEX_add, GB_STRING name; GB_OBJECT fields; GB_BOOLEAN unique)

  CTABLE *table = GB_SubCollectionContainer(THIS);
  char *name = GB.ToZeroString(ARG(name));
  DB_INDEX info;
	int i;
	GB_ARRAY fields;

  if (DB_CheckNameWith(name, "index", "."))
    return;

  if (check_index(table, name, FALSE))
    return;

  info.name = name;
	
	fields = (GB_ARRAY)VARG(fields);
	q_init();
	for (i = 0; i < GB.Array.Count(fields); i++)
	{
		if (i > 0)
			q_add(",");
		
		q_add(table->driver->GetQuote());
		q_add(*(char **)GB.Array.Get(fields, i));
		q_add(table->driver->GetQuote());
	}
	
  info.fields = q_steal();
  info.unique = VARGOPT(unique, FALSE);

  table->driver->Index.Create(&table->conn->db, table->name, name, &info);
	
	GB.FreeString(&info.fields);

END_METHOD


BEGIN_METHOD(CINDEX_remove, GB_STRING name)

  CTABLE *table = GB_SubCollectionContainer(THIS);
  char *name = GB.ToZeroString(ARG(name));

  if (check_index(table, name, TRUE))
    return;

  table->driver->Index.Delete(&table->conn->db, table->name, name);

END_METHOD


GB_DESC CTableIndexesDesc[] =
{
  GB_DECLARE(".Table.Indexes", 0), GB_INHERITS(".SubCollection"),

  GB_METHOD("Add", NULL, CINDEX_add, "(Name)s(Fields)String[];[(Unique)b]"),
  GB_METHOD("Remove", NULL, CINDEX_remove, "(Name)s"),

  GB_END_DECLARE
};


