/***************************************************************************

	CTable.c

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

#define __CTABLE_C

#include <ctype.h>

#include "main.h"

#include "CField.h"
#include "CIndex.h"
#include "CTable.h"


static SUBCOLLECTION_DESC _fields_desc =
{
	".Table.Fields",
	(void *)CFIELD_get,
	(void *)CFIELD_exist,
	(void *)CFIELD_list,
	(void *)CFIELD_release
};

static SUBCOLLECTION_DESC _indexes_desc =
{
	".Table.Indexes",
	(void *)CINDEX_get,
	(void *)CINDEX_exist,
	(void *)CINDEX_list,
	(void *)CINDEX_release
};


static int valid_table(CTABLE *_object)
{
	return !THIS->conn || !THIS->conn->db.handle;
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

	//fprintf(stderr, "make_table: '%s'\n", name);
	
	_object = GB.New(GB.FindClass("Table"), NULL, NULL);
	THIS->conn = conn;
	//GB.Ref(conn);
	THIS->driver = conn->driver;
	THIS->name = GB.NewZeroString(name);

	//fprintf(stderr, "make_table: -> %p '%s'\n", THIS, THIS->name);
	
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

void CTABLE_release(CCONNECTION *conn, void *_object)
{
	THIS->conn = NULL;
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
		GB.Free(POINTER(&fp));
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
					THIS->primary[i] = GB.NewZeroString(*((char **)GB.Array.Get(primary, i)));
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

	char *type;

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
		{
			type = THIS->driver->Table.Type(&THIS->conn->db, THIS->name, NULL);
			if (type)
				GB.ReturnNewZeroString(type);
			else
				GB.ReturnVoidString();
		}
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

	//fprintf(stderr, "CTABLE_free: %p '%s'\n", THIS, THIS->name);

	if (!valid_table(THIS))
		GB_SubCollectionRemove(THIS->conn->tables, THIS->name, 0);
	//GB.Unref(POINTER(&THIS->conn));

	GB.FreeString(&THIS->name);
	GB.FreeString(&THIS->type);
	DB_FreeStringArray(&THIS->primary);

	GB.Unref(POINTER(&THIS->fields));
	GB.Unref(POINTER(&THIS->indexes));
	
	free_new_fields(THIS);

END_METHOD


BEGIN_PROPERTY(CTABLE_fields)

	GB_SubCollectionNew(&THIS->fields, &_fields_desc, THIS);
	GB.ReturnObject(THIS->fields);

END_PROPERTY


BEGIN_PROPERTY(CTABLE_indexes)

	GB_SubCollectionNew(&THIS->indexes, &_indexes_desc, THIS);
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

	GB_PROPERTY_READ("Fields", ".Table.Fields", CTABLE_fields),
	GB_PROPERTY_READ("Indexes", ".Table.Indexes", CTABLE_indexes),

	GB_END_DECLARE
};


/***************************************************************************

	.Connection.Tables

***************************************************************************/

#undef THIS
#define THIS ((CSUBCOLLECTION *)_object)

BEGIN_METHOD(CTABLE_add, GB_STRING name; GB_STRING type)

	CCONNECTION *conn = GB_SubCollectionContainer(THIS);
	CTABLE *table;
	char *name = GB.ToZeroString(ARG(name));

	if (DB_CheckNameWith(name, "table", "."))
		return;

	table = make_table(conn, name, FALSE);
	if (!table)
		return;

	GB_SubCollectionAdd(THIS, STRING(name), LENGTH(name), table);

	if (!MISSING(type))
		GB.StoreString(ARG(type), &table->type);

	table->create = TRUE;
	GB.ReturnObject(table);

END_METHOD


BEGIN_METHOD(CTABLE_remove, GB_STRING name)

	CCONNECTION *conn = GB_SubCollectionContainer(THIS);
	char *name = GB.ToZeroString(ARG(name));

	GB_SubCollectionRemove(THIS, STRING(name), LENGTH(name));
	
	if (check_table(conn, name, TRUE))
		return;

	conn->driver->Table.Delete(&conn->db, name);

END_METHOD

GB_DESC CConnectionTablesDesc[] =
{
	GB_DECLARE(".Connection.Tables", 0), GB_INHERITS(".SubCollection"),

	GB_METHOD("Add", "Table", CTABLE_add, "(Name)s[(Type)s]"),
	GB_METHOD("Remove", NULL, CTABLE_remove, "(Name)s"),

	GB_END_DECLARE
};
