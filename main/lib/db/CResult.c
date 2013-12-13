/***************************************************************************

  CResult.c

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

#define __CRESULT_C

#include "main.h"

#include "deletemap.h"
#include "CField.h"
#include "CResultField.h"
#include "CResult.h"

GB_CLASS CLASS_Blob;

static CBLOB *make_blob(CRESULT *result, int field);
static void set_blob(CBLOB *_object, char *data, int length);


static SUBCOLLECTION_DESC _fields_desc =
{
	".Result.Fields",
	(void *)CRESULTFIELD_get,
	(void *)CRESULTFIELD_exist,
	(void *)NULL,
	(void *)CRESULTFIELD_release
};


static int check_result(CRESULT *_object)
{
	return (THIS->conn->db.handle == NULL);
}

static bool check_available(CRESULT *_object)
{
	if (!THIS->available)
	{
		GB.Error("Result is not available");
		return TRUE;
	}
	else
		return FALSE;
}


static void init_buffer(CRESULT *_object)
{
	int i;

	if (THIS->info.nfield == 0)
		return;

	GB.Alloc(POINTER(&THIS->buffer), sizeof(GB_VARIANT_VALUE) * THIS->info.nfield);
	BARRAY_create(&THIS->changed, THIS->info.nfield);
	BARRAY_clear_all(THIS->changed, THIS->info.nfield);

	for (i = 0; i < THIS->info.nfield; i++)
		THIS->buffer[i].type = GB_T_NULL;
}

static void void_buffer(CRESULT *_object)
{
	int i;

	//fprintf(stderr, "void_buffer\n");

	if (THIS->info.nfield == 0)
		return;

	for (i = 0; i < THIS->info.nfield; i++)
		GB.StoreVariant(NULL, &THIS->buffer[i]);
		
	BARRAY_clear_all(THIS->changed, THIS->info.nfield);
}


static void release_buffer(CRESULT *_object)
{
	if (THIS->buffer)
	{
		void_buffer(THIS);
		GB.Free(POINTER(&THIS->buffer));
		BARRAY_delete(&THIS->changed);
	}
}


static bool load_buffer(CRESULT *_object, int vpos)
{
	int i, ind;
	int pos;

	//fprintf(stderr, "load_buffer: %ld -> %d\n", THIS->pos, vpos);

	/* if THIS->count < 0, that's mean that the driver couldn't determine it */

	DB_CurrentDatabase = &THIS->conn->db;

	if (THIS->count >= 0 && (vpos < 0 || vpos >= THIS->count || THIS->info.nfield == 0))
	{
		/* Andrea Bortolan's changes for the ODBC modules*/

		/* ODBC does return the number of rows affected by the query when execute a insert,apdate or delete query,
			for all others case it returns -1 even if the query was execute without errors        	*/
		/* Here the check for this case -1 means that the query was executed correctly
			so get the result. 									*/
		/* If the pos (the result row) does not exist because pos is > of the rows available than the ODBC module
			will rise an Error ODBC_END_OF_DATA that must be catched by the application */

		#if 0
		if (THIS->count == -1)
			{
				if (THIS->handle && pos != THIS->pos)
					{
							THIS->driver->Result.Fill(THIS->handle, pos, THIS->buffer,(pos > 0) && (pos == (THIS->pos + 1)));
					}
				THIS->pos = pos;
					THIS->available = TRUE;
			}
			else
		/* End of Andrea's changes */
		#endif

		{
			THIS->pos = -1;
			THIS->available = FALSE;
			return TRUE;
		}
	}
	else
	{
		if (THIS->handle && vpos != THIS->pos)
		{
			pos = DELETE_MAP_virtual_to_real(THIS->dmap, vpos);

			//fprintf(stderr, "Result %p: Loading real %ld\n", THIS, pos);

			void_buffer(THIS);

			THIS->driver->Result.Fill(&THIS->conn->db, THIS->handle, pos, THIS->buffer,
				(pos > 0) && (pos == (DELETE_MAP_virtual_to_real(THIS->dmap, THIS->pos) + 1)));

			if (THIS->mode == RESULT_EDIT)
			{
				q_init();

				for (i = 0; i < THIS->info.nindex; i++)
				{
					ind = THIS->info.index[i];
					if (i > 0) q_add(" AND ");
					q_add(THIS->info.field[ind].name);
					if (THIS->buffer[ind].type == GB_T_NULL)
						q_add(" IS NULL");
					else
					{
						q_add(" = ");
						DB_FormatVariant(THIS->driver, &THIS->buffer[ind], q_add_length);
					}
				}

				GB.FreeString(&THIS->edit);
				THIS->edit = q_steal();
			}
		}

		THIS->pos = vpos;
		THIS->available = TRUE;
		return FALSE;
	}
}


static void reload_buffer(CRESULT *_object)
{
	int pos = THIS->pos;
	THIS->pos = -1;
	load_buffer(THIS, pos);
}


static void table_release(DB_INFO *info)
{
	int i;

	if (info->table)
		GB.FreeString(&info->table);

	if (info->field)
	{
		for (i = 0; i < info->nfield; i++)
			CFIELD_free_info(&info->field[i]);

		GB.Free(POINTER(&info->field));
	}

	if (info->index)
		GB.Free(POINTER(&info->index));
}


static GB_TYPE get_field_type(CRESULT *_object, int field)
{
	GB_TYPE type;

	if (THIS->info.field)
		type = THIS->info.field[field].type;
	else
		type = THIS->driver->Result.Field.Type(THIS->handle, field);

	//fprintf(stderr, "get_field_type: %d -> %ld\n", field, type);
	return type;
}


CRESULT *DB_MakeResult(CCONNECTION *conn, int mode, char *table_temp, char *query)
{
	CRESULT *_object;
	DB_RESULT res;
	char *duplicate;
	char *token;
	const char *error = NULL;
	char *arg;
	char *table;

	switch (mode)
	{
		case RESULT_FIND:

			if (conn->driver->Exec(&conn->db, query, &res, "Query failed: &1"))
				return NULL;

			break;

		case RESULT_CREATE:

			res = NULL;
			break;

		case RESULT_EDIT:

			if (conn->driver->Exec(&conn->db, query, &res, "Query failed: &1"))
				return NULL;

			break;

		case RESULT_DELETE:

			conn->driver->Exec(&conn->db, query, &res, "Query failed: &1");
			return NULL;
	}

	_object = GB.New(GB.FindClass("Result"), NULL, NULL);

	THIS->conn = conn;
	GB.Ref(conn);

	THIS->driver = conn->driver;
	THIS->available = FALSE;
	THIS->mode = mode;
	THIS->handle = res;
	THIS->pos = -1;
	THIS->dmap = NULL;

	// table must be copied because it can be a temporary string!
	table = GB.NewZeroString(table_temp);

	switch (mode)
	{
		case RESULT_FIND:

			THIS->driver->Result.Init(THIS->handle, &THIS->info, &THIS->count);
			break;

		case RESULT_CREATE:

			if (THIS->driver->Table.Init(&conn->db, table, &THIS->info))
				goto ERROR;

			THIS->count = 1;
			break;

		case RESULT_EDIT:

			THIS->driver->Result.Init(THIS->handle, &THIS->info, &THIS->count);

			if (THIS->driver->Table.Init(&conn->db, table, &THIS->info))
				goto ERROR;

			if (THIS->driver->Table.Index(&conn->db, table, &THIS->info))
			{
				error = "Table '&1' has no primary key";
				arg = table;
				goto ERROR;
			}

			break;
	}

	init_buffer(THIS);
	load_buffer(THIS, 0);

	GB.FreeString(&table);
	return THIS;

ERROR:

	if (!error)
	{
		if (strchr(table, (int)',') == NULL)
		{
			arg = table;
			if (!THIS->driver->Table.Exist(&conn->db, table))
				error = "Unknown table: &1";
			else
				error = "Cannot read information about table &1";
		}
		else
		{
			duplicate = GB.NewZeroString(table);
			token = strtok(duplicate,",");
			do {
				arg = token;
				if (!THIS->driver->Table.Exist(&conn->db, token))
					error = "Unknown table: &1";
				else
					error = "Cannot read information about table '&1'";
			}
			while ((token = strtok(NULL, ".")) != NULL);
			GB.FreeString(&duplicate);
		}
	}

	GB.Free(POINTER(&_object));
	GB.Error(error, arg);
	GB.FreeString(&table);

	return NULL;
}


BEGIN_METHOD_VOID(CRESULT_free)

	release_buffer(THIS);

	if (THIS->mode != RESULT_CREATE)
		THIS->driver->Result.Release(THIS->handle, &THIS->info);

	if (THIS->mode != RESULT_FIND)
		table_release(&THIS->info);

	if (THIS->edit)
		GB.FreeString(&THIS->edit);

	DELETE_MAP_free(&THIS->dmap);

	GB.Unref(POINTER(&THIS->conn));
	GB.Unref(POINTER(&THIS->fields));

END_METHOD


BEGIN_PROPERTY(CRESULT_count)

	GB.ReturnInteger(THIS->count);

END_PROPERTY


BEGIN_PROPERTY(CRESULT_max)

	GB.ReturnInteger(THIS->count - 1);

END_PROPERTY


BEGIN_PROPERTY(CRESULT_index)

	GB.ReturnInteger(THIS->pos);

END_PROPERTY


BEGIN_PROPERTY(CRESULT_available)

	GB.ReturnBoolean(THIS->available);

END_PROPERTY


static void check_blob(CRESULT *_object, int field)
{
	GB_VARIANT val;

	//fprintf(stderr, "check_blob: %d\n", field);

	if (THIS->buffer[field].type == GB_T_NULL)
	{
		val.type = GB_T_VARIANT;
		val.value.type = (GB_TYPE)CLASS_Blob;
		val.value.value._object = make_blob(THIS, field);

		GB.StoreVariant(&val, &THIS->buffer[field]);
	}
}

BEGIN_METHOD(CRESULT_get, GB_STRING field)

	int index;
	GB_TYPE type;

	if (check_available(THIS))
		return;

	index = CRESULTFIELD_find(THIS, GB.ToZeroString(ARG(field)), TRUE);
	if (index < 0)
		return;

	type = get_field_type(THIS, index);

	if (type == DB_T_BLOB)
		check_blob(THIS, index);

	GB.ReturnVariant(&THIS->buffer[index]);

END_METHOD


BEGIN_METHOD(CRESULT_put, GB_VARIANT value; GB_STRING field)

	int index;
	GB_TYPE type;

	if (check_available(THIS))
		return;

	if (THIS->mode == RESULT_FIND)
	{
		GB.Error("Result is read-only");
		return;
	}

	index = CRESULTFIELD_find(THIS, GB.ToZeroString(ARG(field)), TRUE);
	if (index < 0)
		return;

	type = get_field_type(THIS, index);

	if (type == DB_T_SERIAL)
	{
		//GB.Error("Read-only field");
		return;
	}

	if (type == DB_T_BLOB)
	{
		check_blob(THIS, index);

		if (VARG(value).type == (GB_TYPE)CLASS_Blob)
		{
			CBLOB *src = VARG(value).value._object;
			set_blob((CBLOB *)THIS->buffer[index].value._object, src->data, src->length);
		}
		else
		{
			GB_STRING *str = (GB_STRING *)(void *)ARG(value);

			if (GB.Conv((GB_VALUE *)(void *)ARG(value), GB_T_STRING))
				return;

			set_blob((CBLOB *)THIS->buffer[index].value._object, str->value.addr + str->value.start, str->value.len);
		}

		BARRAY_set(THIS->changed, index);
		return;
	}

	if (VARG(value).type != GB_T_NULL && VARG(value).type != type)
	{
		if (GB.Conv((GB_VALUE *)(void *)ARG(value), THIS->info.field[index].type))
		{
			GB.Error("Type mismatch");
			return;
		}

		GB.Conv((GB_VALUE *)(void *)ARG(value), GB_T_VARIANT);
	}

	GB.StoreVariant(ARG(value), &THIS->buffer[index]);
	BARRAY_set(THIS->changed, index);

END_METHOD

#if 0
BEGIN_METHOD(CRESULT_copy, GB_OBJECT result)

	CRESULT *result = (CRESULT *)VARG(result);
	int index;

	if (THIS->mode == RESULT_FIND)
	{
		GB.Error("Result is read-only");
		return;
	}

	for (index = 0; index <

	index = find_field(THIS, GB.ToZeroString(ARG(field)));
	if (index < 0)
		return;

	if (VARG(value).type != GB_T_NULL && VARG(value).type != THIS->info.types[index])
	/*{
		GB.Error("Type mismatch");
		return;
	}*/
	{
		if (GB.Conv((GB_VALUE *)ARG(value), THIS->info.types[index]))
			return;

		GB.Conv((GB_VALUE *)ARG(value), GB_T_VARIANT);
	}

	GB.StoreVariant(ARG(value), &THIS->buffer[index]);

END_METHOD
#endif

BEGIN_METHOD_VOID(CRESULT_move_first)

	GB.ReturnBoolean(load_buffer(THIS, 0));

END_METHOD


BEGIN_METHOD_VOID(CRESULT_move_last)

	GB.ReturnBoolean(load_buffer(THIS, THIS->count - 1));

END_METHOD


BEGIN_METHOD_VOID(CRESULT_move_previous)

	GB.ReturnBoolean(load_buffer(THIS, THIS->pos - 1));

END_METHOD


BEGIN_METHOD_VOID(CRESULT_move_next)

	GB.ReturnBoolean(load_buffer(THIS, THIS->pos + 1));

END_METHOD


BEGIN_METHOD(CRESULT_move_to, GB_INTEGER pos)

	GB.ReturnBoolean(load_buffer(THIS, VARG(pos)));

END_METHOD


BEGIN_METHOD_VOID(CRESULT_next)

	int *pos = (int *)GB.GetEnum();

	if (!load_buffer(THIS, *pos))
		(*pos)++;
	else
		GB.StopEnum();

END_METHOD

BEGIN_METHOD_VOID(CRESULT_update)

	int i;
	bool comma;
	DB_INFO *info = &THIS->info;

	if (check_available(THIS))
		return;

	DB_CurrentDatabase = &THIS->conn->db;

	q_init();

	switch(THIS->mode)
	{
		case RESULT_CREATE:

			if (BARRAY_is_void(THIS->changed, THIS->info.nfield))
				break;
			
			q_add("INSERT INTO ");
			q_add(DB_GetQuotedTable(THIS->driver, DB_CurrentDatabase, info->table));
			q_add(" ( ");
			
			comma = FALSE;
			for (i = 0; i < info->nfield; i++)
			{
				if (THIS->buffer[i].type == GB_T_NULL)
					continue;
				if (!BARRAY_test(THIS->changed, i))
					continue;
				if (comma) q_add(", ");
				q_add(THIS->driver->GetQuote());
				q_add(info->field[i].name);
				q_add(THIS->driver->GetQuote());
				comma = TRUE;
			}
			
			if (!comma)
			{
				q_add(THIS->driver->GetQuote());
				q_add(info->field[0].name);
				q_add(THIS->driver->GetQuote());
			}

			q_add(" ) VALUES ( ");

			comma = FALSE;
			for (i = 0; i < info->nfield; i++)
			{
				if (THIS->buffer[i].type == GB_T_NULL)
					continue;
				if (!BARRAY_test(THIS->changed, i))
					continue;
				if (comma) q_add(", ");
				DB_FormatVariant(THIS->driver, &THIS->buffer[i], q_add_length);
				comma = TRUE;
			}
			
			if (!comma)
			{
				DB_FormatVariant(THIS->driver, &THIS->buffer[0], q_add_length);
			}

			q_add(" )");

			if (!THIS->driver->Exec(&THIS->conn->db, q_get(), NULL, "Cannot create record: &1"))
				void_buffer(THIS);

			break;

		case RESULT_EDIT:

			if (BARRAY_is_void(THIS->changed, THIS->info.nfield))
				break;
			
			q_add("UPDATE ");
			q_add(DB_GetQuotedTable(THIS->driver, DB_CurrentDatabase, info->table));
			q_add(" SET ");

			comma = FALSE;
			for (i = 0; i < info->nfield; i++)
			{
				if (!BARRAY_test(THIS->changed, i))
					continue;
				if (comma) q_add(", ");
				q_add(THIS->driver->GetQuote());
				q_add(THIS->info.field[i].name);
				q_add(THIS->driver->GetQuote());
				q_add(" = ");
				DB_FormatVariant(THIS->driver, &THIS->buffer[i], q_add_length);
				comma = TRUE;
			}

			q_add(" WHERE ");
			q_add(THIS->edit);

			THIS->driver->Exec(&THIS->conn->db, q_get(), NULL, "Cannot modify record: &1");

			break;

		default:

			GB.Error("Result is read-only");
			break;
	}

	BARRAY_clear_all(THIS->changed, THIS->info.nfield);

END_METHOD


BEGIN_METHOD(CRESULT_delete, GB_BOOLEAN keep)

	DB_INFO *info = &THIS->info;
	int *pos;
	void *save_enum;

	if (check_available(THIS))
		return;

	q_init();

	switch(THIS->mode)
	{
		case RESULT_CREATE:

			void_buffer(THIS);
			break;

		case RESULT_EDIT:

			q_add("DELETE FROM ");
			q_add(DB_GetQuotedTable(THIS->driver, DB_CurrentDatabase, info->table));
			q_add(" WHERE ");
			q_add(THIS->edit);

			THIS->driver->Exec(&THIS->conn->db, q_get(), NULL, "Cannot delete record: &1");

			if (!VARGOPT(keep, FALSE))
			{
				DELETE_MAP_add(&THIS->dmap, THIS->pos);
				THIS->count--;
				reload_buffer(THIS);

				save_enum = GB.BeginEnum(THIS);
				while (!GB.NextEnum())
				{
					pos = (int *)GB.GetEnum();
					if (*pos > THIS->pos)
						(*pos)--;
				}
				GB.EndEnum(save_enum);
			}

			break;

		default:

			GB.Error("Result is read-only");
			break;
	}

END_METHOD




BEGIN_PROPERTY(CRESULT_fields)

	GB_SubCollectionNew(&THIS->fields, &_fields_desc, THIS);
	GB.ReturnObject(THIS->fields);

END_PROPERTY


BEGIN_PROPERTY(CRESULT_connection)

	GB.ReturnObject(THIS->conn);

END_PROPERTY


GB_DESC CResultDesc[] =
{
	GB_DECLARE("Result", sizeof(CRESULT)), GB_NOT_CREATABLE(),

	GB_HOOK_CHECK(check_result),

	GB_METHOD("_free", NULL, CRESULT_free, NULL),

	GB_PROPERTY_READ("Count", "i", CRESULT_count),
	GB_PROPERTY_READ("Length", "i", CRESULT_count),
	GB_PROPERTY_READ("Available", "b", CRESULT_available),
	GB_PROPERTY_READ("Index", "i", CRESULT_index),
	GB_PROPERTY_READ("Max", "i", CRESULT_max),

	GB_METHOD("_get", "v", CRESULT_get, "(Field)s"),
	GB_METHOD("_put", NULL, CRESULT_put, "(Value)v(Field)s"),
	GB_METHOD("_next", NULL, CRESULT_next, NULL),

	GB_METHOD("MoveFirst", "b", CRESULT_move_first, NULL),
	GB_METHOD("MoveLast", "b", CRESULT_move_last, NULL),
	GB_METHOD("MovePrevious", "b", CRESULT_move_previous, NULL),
	GB_METHOD("MoveNext", "b", CRESULT_move_next, NULL),
	GB_METHOD("MoveTo", "b", CRESULT_move_to, "(Index)i"),

	GB_METHOD("Update", NULL, CRESULT_update, NULL),
	GB_METHOD("Delete", NULL, CRESULT_delete, "[(Keep)b]"),
	
	GB_PROPERTY_READ("Fields", ".Result.Fields", CRESULT_fields),
	GB_PROPERTY_READ("Connection", "Connection", CRESULT_connection),

	GB_END_DECLARE
};

/** Blob *******************************************************************/

static bool _convert_blob(CBLOB *_object, GB_TYPE type, GB_VALUE *conv)
{
	if (BLOB)
	{
		switch (type)
		{
			case GB_T_STRING:
			case GB_T_CSTRING:
				conv->_string.value.addr = BLOB->data;
				conv->_string.value.start = 0;
				conv->_string.value.len = BLOB->length;
				conv->type = GB_T_CSTRING;
				return FALSE;
				
			default:
				return TRUE;
		}
	}
	else
		return TRUE;
}

/*static int check_blob(CBLOB *_object)
{
	return check_result(BLOB->result) || (BLOB->result->pos != BLOB->pos);
}*/

static CBLOB *make_blob(CRESULT *result, int field)
{
	CBLOB *_object;

	_object = GB.New(CLASS_Blob, NULL, NULL);

	//BLOB->result = result;
	//GB.Ref(result);

	//BLOB->field = field;
	//BLOB->pos = result->pos;
	BLOB->data = NULL;
	BLOB->length = 0;
	BLOB->constant = TRUE;

	if (result->handle && result->pos >= 0)
	{
		result->driver->Result.Blob(result->handle, result->pos, field, BLOB);
		set_blob(BLOB, BLOB->data, BLOB->length);
	}

	//fprintf(stderr, "make_blob: [%d] %d (%d) -> %p\n", result->pos, field, BLOB->length, BLOB);

	//GB.UnrefKeep(POINTER(&_object), FALSE);
	return BLOB;
}

static void set_blob(CBLOB *_object, char *data, int length)
{
	if (!BLOB->constant && BLOB->data)
		GB.FreeString((char **)&BLOB->data);

	if (data && length)
	{
		BLOB->data = GB.NewString(data, length);
		BLOB->constant = FALSE;
	}

	BLOB->length = length;
}

BEGIN_METHOD_VOID(CBLOB_init)

	CLASS_Blob = GB.FindClass("Blob");

END_METHOD

BEGIN_METHOD_VOID(CBLOB_free)

	//GB.Unref(POINTER(&BLOB->result));
	set_blob(BLOB, NULL, 0);

END_METHOD

/*BEGIN_PROPERTY(CBLOB_result)

	GB.ReturnObject(BLOB->result);

END_PROPERTY*/

BEGIN_PROPERTY(CBLOB_data)

	if (READ_PROPERTY)
	{
		if (BLOB->length)
			GB.ReturnConstString(BLOB->data, BLOB->length);
		else
			GB.ReturnVoidString();
	}
	else
	{
		set_blob(BLOB, PSTRING(), PLENGTH());
	}

END_PROPERTY

BEGIN_PROPERTY(CBLOB_length)

	GB.ReturnInteger(BLOB->length);

END_PROPERTY


GB_DESC CBlobDesc[] =
{
	GB_DECLARE("Blob", sizeof(CBLOB)), GB_NOT_CREATABLE(),

	//GB_HOOK_CHECK(check_blob),

	GB_STATIC_METHOD("_init", NULL, CBLOB_init, NULL),
	GB_METHOD("_free", NULL, CBLOB_free, NULL),

	//GB_PROPERTY_READ("Result", "Result", CBLOB_result),
	GB_PROPERTY("Data", "s", CBLOB_data),
	GB_PROPERTY_READ("Length", "i", CBLOB_length),
	GB_INTERFACE("_convert", &_convert_blob),
	//GB_METHOD("_unknown", "v", CBLOB_unknown, "v"),

	GB_END_DECLARE
};


