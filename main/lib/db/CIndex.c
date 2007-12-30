/***************************************************************************

  CIndex.c

  (c) 2000-2005 Beno� Minisini <gambas@users.sourceforge.net>

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

#define __CINDEX_C

#include "main.h"

#include "CIndex.h"


static int valid_index(CINDEX *_object)
{
  return (THIS->table->conn->db.handle == NULL);
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

  GB.New((void **)&_object, GB.FindClass("Index"), NULL, NULL);
  THIS->table = table;
  GB.Ref(table);
  THIS->driver = table->conn->driver;
  GB.NewString(&THIS->name, name, 0);

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


/***************************************************************************

  Index

***************************************************************************/

BEGIN_METHOD_VOID(CINDEX_free)

  GB.SubCollection.Remove(THIS->table->indexes, THIS->name, 0);
  GB.Unref((void **)&THIS->table);

  GB.FreeString(&THIS->name);

  GB.FreeString(&THIS->info.name);
  GB.FreeString(&THIS->info.fields);
  THIS->info.unique = FALSE;

END_METHOD



BEGIN_PROPERTY(CINDEX_name)

  GB.ReturnString(THIS->name);

END_PROPERTY


BEGIN_PROPERTY(CINDEX_fields)

  GB.ReturnString(THIS->info.fields);

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
  GB_PROPERTY_READ("Fields", "s", CINDEX_fields),

  GB_PROPERTY_READ("Table", "Table", CINDEX_table),

  GB_END_DECLARE
};


/***************************************************************************

  .TableIndexes

***************************************************************************/

#undef THIS
#define THIS ((GB_SUBCOLLECTION)_object)

BEGIN_METHOD(CINDEX_add, GB_STRING name; GB_STRING fields; GB_BOOLEAN unique)

  CTABLE *table = GB.SubCollection.Container(THIS);
  char *name = GB.ToZeroString(ARG(name));
  DB_INDEX info;

  if (DB_CheckName(name, "index"))
    return;

  if (check_index(table, name, FALSE))
    return;

  info.name = name;
  info.fields = GB.ToZeroString(ARG(fields));
  info.unique = VARGOPT(unique, FALSE);

  table->driver->Index.Create(&table->conn->db, table->name, name, &info);

END_METHOD


BEGIN_METHOD(CINDEX_remove, GB_STRING name)

  CTABLE *table = GB.SubCollection.Container(THIS);
  char *name = GB.ToZeroString(ARG(name));

  if (check_index(table, name, TRUE))
    return;

  table->driver->Index.Delete(&table->conn->db, table->name, name);

END_METHOD


GB_DESC CTableIndexesDesc[] =
{
  GB_DECLARE(".TableIndexes", 0), GB_INHERITS(".SubCollection"),

  GB_METHOD("Add", NULL, CINDEX_add, "(Name)s(Fields)s[(Unique)b]"),
  GB_METHOD("Remove", NULL, CINDEX_remove, "(Name)s"),

  GB_END_DECLARE
};


