/***************************************************************************

  CField.c

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

#define __CFIELD_C

#include "main.h"

#include "CField.h"


static int valid_field(CFIELD *_object)
{
  return (THIS->table->conn->db.handle == NULL);
}

static bool exist_field(CTABLE *table, const char *name)
{
  DB_FIELD *fp;

	if (!name || !*name)
		return FALSE;

  if (table->create)
  {
    for (fp = table->new_fields; fp; fp = fp->next)
    {
      if (!strcasecmp(fp->name, name))
        return TRUE;
    }

    return FALSE;
  }
  else
    return table->driver->Field.Exist(&table->conn->db, table->name, (char *)name);
}

static bool check_field(CTABLE *table, char *name, bool must_exist)
{
  bool exist = exist_field(table, name);

  if (must_exist)
  {
    if (!exist)
    {
      GB.Error("Unknown field: &1.&2", table->name, name);
      return TRUE;
    }
  }
  else
  {
    if (exist)
    {
      GB.Error("Field already exists: &1.&2", table->name, name);
      return TRUE;
    }
  }

  return FALSE;
}


static bool check_type(long type)
{
  if (type == GB_T_BOOLEAN
      || type == GB_T_INTEGER
      || type == GB_T_LONG
      || type == GB_T_FLOAT
      || type == GB_T_DATE
      || type == GB_T_STRING
      || type == DB_T_SERIAL
      || type == DB_T_BLOB)
    return FALSE;

  GB.Error("Bad field type");
  return TRUE;
}


static CFIELD *make_field(CTABLE *table, const char *name, bool must_exist)
{
  CFIELD *_object;

  if (check_field(table, (char *)name, must_exist))
    return NULL;

  GB.New((void **)&_object, GB.FindClass("Field"), NULL, NULL);
  THIS->table = table;
  GB.Ref(table);
  THIS->driver = table->conn->driver;
  GB.NewString(&THIS->name, name, 0);

  return _object;
}


void CFIELD_free_info(DB_FIELD *info)
{
  GB.FreeString(&info->name);
  GB.StoreVariant(NULL, &info->def);
  info->type = GB_T_NULL;
  info->length = 0;
}

static void add_new_field(CTABLE *table, DB_FIELD *field)
{
  DB_FIELD **fp;

  fp = &table->new_fields;

  for(;;)
  {
    if (!*fp)
    {
      *fp = field;
      field->next = NULL;
      return;
    }

    fp = &((*fp)->next);
  }
}


void *CFIELD_get(CTABLE *table, const char *name)
{
  CFIELD *field = make_field(table, name, TRUE);
  table->driver->Field.Info(&table->conn->db, table->name, (char *)name, &field->info);
  return field;
}


int CFIELD_exist(CTABLE *table, const char *name)
{
  return exist_field(table, name);
}

#if 0
long CFIELD_count(CTABLE *table)
{
  DB_FIELD *fp;
  long n;

  if (table->create)
  {
    for (fp = THIS->new_fields, n = 0; fp; fp = fp->next, n++);
    return n;
  }
  else
  {
    DB_FreeStringArray(&table->fields_list);
    table->driver->Field.List(table->conn->handle, &table->fields_list);
    return GB.Count(table->fields_list);
  }
}
#endif

void CFIELD_list(CTABLE *table, char ***list)
{
  table->driver->Field.List(&table->conn->db, table->name, list);
}


/***************************************************************************

  Field

***************************************************************************/

BEGIN_METHOD_VOID(CFIELD_free)

  GB.SubCollection.Remove(THIS->table->fields, THIS->name, 0);
  GB.Unref((void **)&THIS->table);

  GB.FreeString(&THIS->name);

  CFIELD_free_info(&THIS->info);

END_METHOD



/*BEGIN_METHOD_VOID(CFIELD_delete)

  THIS->table->conn->driver->User.Delete(THIS->conn->handle, THIS->name);

END_METHOD*/


BEGIN_PROPERTY(CFIELD_name)

  GB.ReturnString(THIS->name);

END_PROPERTY


BEGIN_PROPERTY(CFIELD_type)

  GB.ReturnInteger(THIS->info.type);

END_PROPERTY


BEGIN_PROPERTY(CFIELD_length)

  GB.ReturnInteger(THIS->info.length);

END_PROPERTY


BEGIN_PROPERTY(CFIELD_default)

  GB.ReturnPtr(GB_T_VARIANT, &THIS->info.def);

END_PROPERTY


BEGIN_PROPERTY(CFIELD_table)

  GB.ReturnObject(THIS->table);

END_PROPERTY


GB_DESC CFieldDesc[] =
{
  GB_DECLARE("Field", sizeof(CFIELD)),
  GB_NOT_CREATABLE(),
  GB_HOOK_CHECK(valid_field),

  GB_METHOD("_free", NULL, CFIELD_free, NULL),

  //GB_METHOD("Delete", NULL, CFIELD_delete, NULL),

  GB_PROPERTY_READ("Name", "s", CFIELD_name),
  GB_PROPERTY_READ("Type", "i", CFIELD_type),
  GB_PROPERTY_READ("Length", "i", CFIELD_length),
  GB_PROPERTY_READ("Default", "v", CFIELD_default),
  GB_PROPERTY_READ("Table", "Table", CFIELD_table),

  GB_END_DECLARE
};


/***************************************************************************

  .TableFields

***************************************************************************/

#undef THIS
#define THIS ((GB_SUBCOLLECTION)_object)

BEGIN_METHOD(CFIELD_add, GB_STRING name; GB_INTEGER type; GB_INTEGER length; GB_VARIANT def)

  CTABLE *table = GB.SubCollection.Container(THIS);
  char *name = GB.ToZeroString(ARG(name));
  DB_FIELD new_field, *info;

  if (!table->create)
  {
    GB.Error("Table already exists");
    return;
  }

  if (DB_CheckName(name, "field"))
    return;

  if (check_field(table, name, FALSE))
    return;

  /*field = make_field(table, name, FALSE);
  if (!field)
    return;

  GB.SubCollection.Add(THIS, STRING(name), LENGTH(name), field);*/

  new_field.next = NULL;

  new_field.type = VARG(type);
  if (check_type(new_field.type))
    return;

  new_field.length = VARGOPT(length, 0);
  if (new_field.length < 0)
    new_field.length = 0;
  else if (new_field.length > 65535)
    new_field.length = 65535;

  /*field->info = new_field;*/

  GB.Alloc((void **)&info, sizeof(DB_FIELD));

  info->next = NULL;
  info->type = new_field.type;
  info->length = new_field.length;

  info->def._object.type = GB_T_NULL;
  if (!MISSING(def))
    GB.StoreVariant(ARG(def), &info->def);

  GB.NewString(&info->name, STRING(name), LENGTH(name));
  DB_LowerString(info->name);

  add_new_field(table, info);

END_METHOD


GB_DESC CTableFieldsDesc[] =
{
  GB_DECLARE(".TableFields", 0), GB_INHERITS(".SubCollection"),

  GB_METHOD("Add", NULL, CFIELD_add, "(Name)s(Type)i[(Length)i(Default)v]"),
  //GB_METHOD("Remove", NULL, CFIELD_remove, "(Name)s"),

  GB_END_DECLARE
};


