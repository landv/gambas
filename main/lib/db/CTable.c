/***************************************************************************

  CTable.c

  The Table, Field and Index class

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

#define __CTABLE_C

#include <ctype.h>

#include "main.h"

#include "CField.h"
#include "CIndex.h"
#include "CTable.h"


static GB_SUBCOLLECTION_DESC _fields_desc =
{
  ".TableFields",
  (void *)CFIELD_get,
  (void *)CFIELD_exist,
  (void *)CFIELD_list
};

static GB_SUBCOLLECTION_DESC _indexes_desc =
{
  ".TableIndexes",
  (void *)CINDEX_get,
  (void *)CINDEX_exist,
  (void *)CINDEX_list
};


static int valid_table(CTABLE *_object)
{
  return (THIS->conn->db.handle == NULL);
}


static bool check_table(CCONNECTION *conn, char *name, bool must_exist)
{
  bool exist = conn->driver->Table.Exist(&conn->db, name);

  if (must_exist)
  {
    if (!exist)
    {
      GB.Error("Unknown table: &1", name);
      return TRUE;
    }
  }
  else
  {
    if (exist)
    {
      GB.Error("Table already exists: &1", name);
      return TRUE;
    }
  }

  return FALSE;
}


static CTABLE *make_table(CCONNECTION *conn, const char *name, bool must_exist)
{
  CTABLE *_object;

  if (check_table(conn, (char *)name, must_exist))
    return NULL;

  GB.New((void **)&_object, GB.FindClass("Table"), NULL, NULL);
  THIS->conn = conn;
  GB.Ref(conn);
  THIS->driver = conn->driver;
  GB.NewString(&THIS->name, name, 0);

  return _object;
}


void *CTABLE_get(CCONNECTION *conn, const char *name)
{
  return make_table(conn, name, TRUE);
}


int CTABLE_exist(CCONNECTION *conn, const char *name)
{
  return conn->driver->Table.Exist(&conn->db, (char *)name);
}


void CTABLE_list(CCONNECTION *conn, char ***list)
{
  conn->driver->Table.List(&conn->db, list);
}


/***************************************************************************

  Table

***************************************************************************/

static void free_new_fields(CTABLE *_object)
{
  DB_FIELD *fp;
  DB_FIELD *next;

  for (fp = THIS->new_fields; fp; fp = next)
  {
    next = fp->next;
    CFIELD_free_info(fp);
    GB.Free((void **)&fp);
  }

  THIS->new_fields = NULL;
}


BEGIN_PROPERTY(CTABLE_primary_key)

  GB_ARRAY primary;
  int i, n;
  char *field;

  if (THIS->create)
  {
    if (READ_PROPERTY)
    {
      if (!THIS->primary)
      {
        GB.ReturnNull();
        return;
      }

      GB.ReturnObject(DB_StringArrayToGambasArray(THIS->primary));
    }
    else
    {
      primary = (GB_ARRAY)VPROP(GB_OBJECT);
      if (primary)
        n = GB.Array.Count(primary);
      else
        n = 0;

      for (i = 0; i < n; i++)
      {
        field = *((char **)GB.Array.Get(primary, i));
        if (!CFIELD_exist(THIS, field))
        {
          GB.Error("Unknown field: &1", field);
          return;
        }
      }

      DB_FreeStringArray(&THIS->primary);
      if (n)
      {
        GB.NewArray(&THIS->primary, sizeof(char *), n);
        for (i = 0; i < n; i++)
          GB.NewString(&THIS->primary[i], *((char **)GB.Array.Get(primary, i)), 0);
      }
    }
  }
  else
  {
    if (READ_PROPERTY)
    {
      if (THIS->driver->Table.PrimaryKey(&THIS->conn->db, THIS->name, &THIS->primary))
        return;

      GB.ReturnObject(DB_StringArrayToGambasArray(THIS->primary));
      DB_FreeStringArray(&THIS->primary);
    }
    else
      GB.Error("Read-only property");
  }

END_PROPERTY


BEGIN_PROPERTY(CTABLE_name)

  GB.ReturnString(THIS->name);

END_PROPERTY


BEGIN_PROPERTY(CTABLE_system)

  GB.ReturnBoolean(THIS->driver->Table.IsSystem(&THIS->conn->db, THIS->name));

END_PROPERTY


BEGIN_PROPERTY(CTABLE_type)

  if (THIS->create)
  {
    if (READ_PROPERTY)
      GB.ReturnString(THIS->type);
    else
      GB.StoreString(PROP(GB_STRING), &THIS->type);
  }
  else
  {
    if (READ_PROPERTY)
      GB.ReturnNewZeroString(THIS->driver->Table.Type(&THIS->conn->db, THIS->name, NULL));
    else
      THIS->driver->Table.Type(&THIS->conn->db, THIS->name, GB.ToZeroString(PROP(GB_STRING)));
  }

END_PROPERTY


BEGIN_METHOD_VOID(CTABLE_update)

	DB_FIELD *fp;
	DB_FIELD *fp_serial = NULL;

  if (!THIS->new_fields)
  {
    GB.Error("No field");
    return;
  }

  for (fp = THIS->new_fields; fp; fp = fp->next)
  {
  	if (fp->type == DB_T_SERIAL)
  	{
  		if (THIS->conn->db.flags.no_serial)
  		{
  			GB.Error("Serial fields are not supported");
  			return;
  		}
  		if (fp_serial)
  		{
  			GB.Error("Only one serial field is allowed");
  			return;
  		}
  		fp_serial = fp;
  	}
  	else if (fp->type == DB_T_BLOB)
  	{
  		if (THIS->conn->db.flags.no_blob)
  		{
  			GB.Error("Blob fields are not supported");
  			return;
  		}
  	}
  }

	if (fp_serial)
	{
		if (!(THIS->primary && GB.Count(THIS->primary) == 1 && strcmp(THIS->primary[0], fp_serial->name) == 0))
		{
			GB.Error("The serial field must be the primary key");
			return;
		}
	}

  /*if (!THIS->primary || GB.Count(THIS->primary) == 0)
  {
    GB.Error("No primary key");
    return;
  }*/

  if (THIS->driver->Table.Create(&THIS->conn->db, THIS->name, THIS->new_fields, THIS->primary, THIS->type))
    return;

  free_new_fields(THIS);
  DB_FreeStringArray(&THIS->primary);
  THIS->create = FALSE;

  //GB.Unref(THIS);

END_METHOD


BEGIN_METHOD_VOID(CTABLE_free)

  if (!valid_table(THIS))
    GB.SubCollection.Remove(THIS->conn->tables, THIS->name, 0);
  GB.Unref((void **)&THIS->conn);

  GB.FreeString(&THIS->name);
  GB.FreeString(&THIS->type);
  DB_FreeStringArray(&THIS->primary);

  free_new_fields(THIS);

END_METHOD


BEGIN_PROPERTY(CTABLE_fields)

  GB.SubCollection.New(&THIS->fields, &_fields_desc, THIS);
  GB.ReturnObject(THIS->fields);

END_PROPERTY


BEGIN_PROPERTY(CTABLE_indexes)

  GB.SubCollection.New(&THIS->indexes, &_indexes_desc, THIS);
  GB.ReturnObject(THIS->indexes);

END_PROPERTY


BEGIN_PROPERTY(CTABLE_connection)

  GB.ReturnObject(THIS->conn);

END_PROPERTY



GB_DESC CTableDesc[] =
{
  GB_DECLARE("Table", sizeof(CTABLE)),
  GB_NOT_CREATABLE(),
  GB_HOOK_CHECK(valid_table),

  GB_METHOD("_free", NULL, CTABLE_free, NULL),

  //GB_METHOD("AddField", NULL, CTABLE_add_field, "(Name)s(Type)i[(Length)i(Default)v"])

  GB_PROPERTY_READ("Name", "s", CTABLE_name),
  GB_PROPERTY_READ("System", "b", CTABLE_system),
  GB_PROPERTY("PrimaryKey", "String[]", CTABLE_primary_key),
  GB_PROPERTY("Type", "s", CTABLE_type),
  GB_PROPERTY_READ("Connection", "Connection", CTABLE_connection),

  GB_METHOD("Update", NULL, CTABLE_update, NULL),

  GB_PROPERTY_READ("Fields", ".TableFields", CTABLE_fields),
  GB_PROPERTY_READ("Indexes", ".TableIndexes", CTABLE_indexes),

  GB_END_DECLARE
};


/***************************************************************************

  .ConnectionTables

***************************************************************************/

#undef THIS
#define THIS ((GB_SUBCOLLECTION)_object)

BEGIN_METHOD(CTABLE_add, GB_STRING name; GB_STRING type)

  CCONNECTION *conn = GB.SubCollection.Container(THIS);
  CTABLE *table;
  char *name = GB.ToZeroString(ARG(name));

  if (DB_CheckName(name, "table"))
    return;

  table = make_table(conn, name, FALSE);
  if (!table)
    return;

  GB.SubCollection.Add(THIS, STRING(name), LENGTH(name), table);
  //GB.Ref(table);

  if (!MISSING(type))
    GB.StoreString(ARG(type), &table->type);

  table->create = TRUE;
  GB.ReturnObject(table);

END_METHOD


BEGIN_METHOD(CTABLE_remove, GB_STRING name)

  CCONNECTION *conn = GB.SubCollection.Container(THIS);
  char *name = GB.ToZeroString(ARG(name));

  if (check_table(conn, name, TRUE))
    return;

  GB.SubCollection.Remove(THIS, STRING(name), LENGTH(name));
  conn->driver->Table.Delete(&conn->db, name);

END_METHOD


GB_DESC CConnectionTablesDesc[] =
{
  GB_DECLARE(".ConnectionTables", 0), GB_INHERITS(".SubCollection"),

  GB_METHOD("Add", "Table", CTABLE_add, "(Name)s[(Type)s]"),
  GB_METHOD("Remove", NULL, CTABLE_remove, "(Name)s"),

  GB_END_DECLARE
};
