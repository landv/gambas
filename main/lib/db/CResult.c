/***************************************************************************

  CResult.c

  (c) 2000-2017 Benoît Minisini <g4mba5@gmail.com>

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
	int result;

	if (vpos == THIS->pos)
		return FALSE;

	DB_CurrentDatabase = &THIS->conn->db;

	if (THIS->count < 0)
	{
		if (vpos != (THIS->pos + 1))
		{
			GB.Error("Result is forward only");
			return TRUE;
		}
	}
	else 
	{
		if (vpos < 0 || vpos >= THIS->count || THIS->info.nfield == 0)
		{
			THIS->pos = -1;
			THIS->available = FALSE;
			return TRUE;
		}
	}
	
	pos = DELETE_MAP_virtual_to_real(THIS->dmap, vpos);

	//fprintf(stderr, "Result %p: Loading real %ld\n", THIS, pos);

	void_buffer(THIS);

	if (THIS->handle)
	{
		result = THIS->driver->Result.Fill(&THIS->conn->db, THIS->handle, pos, THIS->buffer, (pos > 0) && (pos == (DELETE_MAP_virtual_to_real(THIS->dmap, THIS->pos) + 1)));
		
		if (result == DB_ERROR)
			return TRUE;
		else if (result == DB_NO_DATA)
		{
			THIS->pos = -1;
			THIS->available = FALSE;
			return TRUE;
		}

		if (THIS->mode == RESULT_EDIT)
		{
			q_init();

			for (i = 0; i < THIS->info.nindex; i++)
			{
				ind = THIS->info.index[i];
				if (i > 0) q_add(" AND ");
				q_add(THIS->driver->GetQuote());
				q_add(THIS->info.field[ind].name);
				q_add(THIS->driver->GetQuote());
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

			conn->driver->Exec(&conn->db, query, NULL, "Query failed: &1");
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


BEGIN_METHOD_VOID(Result_free)

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


BEGIN_PROPERTY(Result_Count)

	GB.ReturnInteger(THIS->count);

END_PROPERTY


BEGIN_PROPERTY(Result_Max)

	GB.ReturnInteger(THIS->count - 1);

END_PROPERTY


BEGIN_PROPERTY(Result_Index)

	GB.ReturnInteger(THIS->pos);

END_PROPERTY


BEGIN_PROPERTY(Result_Available)

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

BEGIN_METHOD(Result_get, GB_STRING field)

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


BEGIN_METHOD(Result_GetAll, GB_STRING field)

	int index;
	int pos;
	GB_TYPE type, atype;
	GB_ARRAY result;
	GB_VARIANT_VALUE *val;

	index = CRESULTFIELD_find(THIS, GB.ToZeroString(ARG(field)), TRUE);
	if (index < 0)
		return;

	atype = type = get_field_type(THIS, index);
	if (atype == DB_T_SERIAL)
		atype = GB_T_LONG;
	else if (atype == DB_T_BLOB)
		atype = GB_T_OBJECT;
	
	GB.Array.New(POINTER(&result), atype, 0);

	pos = THIS->pos;
	load_buffer(THIS, 0);
	
	while (THIS->available)
	{
		if (type == DB_T_BLOB)
			check_blob(THIS, index);

		val = &THIS->buffer[index];
		
		switch (atype)
		{
			case GB_T_BOOLEAN: *(char *)GB.Array.Add(result) = val->value._boolean; break;
			case GB_T_INTEGER: *(int *)GB.Array.Add(result) = val->value._integer; break;
			case GB_T_LONG: *(int64_t *)GB.Array.Add(result) = val->value._long; break;
			case GB_T_FLOAT: *(double *)GB.Array.Add(result) = val->value._float; break;
			case GB_T_DATE: *(GB_DATE_VALUE *)GB.Array.Add(result) = val->value._date; break;
			
			case GB_T_STRING:
				if (val->type == GB_T_CSTRING)
					*(char **)GB.Array.Add(result) = GB.NewString(val->value._string, strlen(val->value._string));
				else
					*(char **)GB.Array.Add(result) = GB.RefString(val->value._string);
				break;
				
			case GB_T_OBJECT: *(void **)GB.Array.Add(result) = val->value._object; GB.Ref(val->value._object); break;
		}
		
		load_buffer(THIS, THIS->pos + 1);
	}
	
	if (THIS->count >= 0)
		load_buffer(THIS, pos);
	
	GB.ReturnObject(result);

END_METHOD


BEGIN_METHOD(Result_put, GB_VARIANT value; GB_STRING field)

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

BEGIN_METHOD_VOID(Result_MoveFirst)

	GB.ReturnBoolean(load_buffer(THIS, 0));

END_METHOD


BEGIN_METHOD_VOID(Result_MoveLast)

	if (THIS->count < 0)
		GB.Error("Result is forward only");
	else
		GB.ReturnBoolean(load_buffer(THIS, THIS->count - 1));

END_METHOD


BEGIN_METHOD_VOID(Result_MovePrevious)

	GB.ReturnBoolean(load_buffer(THIS, THIS->pos - 1));

END_METHOD


BEGIN_METHOD_VOID(Result_MoveNext)

	GB.ReturnBoolean(load_buffer(THIS, THIS->pos + 1));

END_METHOD


BEGIN_METHOD(Result_MoveTo, GB_INTEGER pos)

	GB.ReturnBoolean(load_buffer(THIS, VARG(pos)));

END_METHOD


BEGIN_METHOD_VOID(Result_next)

	int *pos = (int *)GB.GetEnum();

	if (!load_buffer(THIS, *pos))
		(*pos)++;
	else
		GB.StopEnum();

END_METHOD

BEGIN_METHOD_VOID(Result_Update)

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
			q_add(DB_GetQuotedTable(THIS->driver, DB_CurrentDatabase, info->table, -1));
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
			q_add(DB_GetQuotedTable(THIS->driver, DB_CurrentDatabase, info->table, -1));
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


BEGIN_METHOD(Result_Delete, GB_BOOLEAN keep)

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
			q_add(DB_GetQuotedTable(THIS->driver, DB_CurrentDatabase, info->table, -1));
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




BEGIN_PROPERTY(Result_Fields)

	GB_SubCollectionNew(&THIS->fields, &_fields_desc, THIS);
	GB.ReturnObject(THIS->fields);

END_PROPERTY


BEGIN_PROPERTY(Result_Connection)

	GB.ReturnObject(THIS->conn);

END_PROPERTY


GB_DESC CResultDesc[] =
{
	GB_DECLARE("Result", sizeof(CRESULT)), GB_NOT_CREATABLE(),

	GB_HOOK_CHECK(check_result),

	GB_METHOD("_free", NULL, Result_free, NULL),

	GB_PROPERTY_READ("Count", "i", Result_Count),
	GB_PROPERTY_READ("Length", "i", Result_Count),
	GB_PROPERTY_READ("Available", "b", Result_Available),
	GB_PROPERTY_READ("Index", "i", Result_Index),
	GB_PROPERTY_READ("Max", "i", Result_Max),

	GB_METHOD("_get", "v", Result_get, "(Field)s"),
	GB_METHOD("_put", NULL, Result_put, "(Value)v(Field)s"),
	GB_METHOD("_next", NULL, Result_next, NULL),

	GB_METHOD("MoveFirst", "b", Result_MoveFirst, NULL),
	GB_METHOD("MoveLast", "b", Result_MoveLast, NULL),
	GB_METHOD("MovePrevious", "b", Result_MovePrevious, NULL),
	GB_METHOD("MoveNext", "b", Result_MoveNext, NULL),
	GB_METHOD("MoveTo", "b", Result_MoveTo, "(Index)i"),

	GB_METHOD("Update", NULL, Result_Update, NULL),
	GB_METHOD("Delete", NULL, Result_Delete, "[(Keep)b]"),
	
	GB_METHOD("All", "Array", Result_GetAll, "(Field)s"),
	
	GB_PROPERTY_READ("Fields", ".Result.Fields", Result_Fields),
	GB_PROPERTY_READ("Connection", "Connection", Result_Connection),

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
		BLOB->constant = FALSE;
		result->driver->Result.Blob(result->handle, result->pos, field, BLOB);
		if (BLOB->constant)
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

BEGIN_METHOD_VOID(Blob_init)

	CLASS_Blob = GB.FindClass("Blob");

END_METHOD

BEGIN_METHOD_VOID(Blob_free)

	//GB.Unref(POINTER(&BLOB->result));
	set_blob(BLOB, NULL, 0);

END_METHOD

/*BEGIN_PROPERTY(CBLOB_result)

	GB.ReturnObject(BLOB->result);

END_PROPERTY*/

BEGIN_PROPERTY(Blob_Data)

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

BEGIN_PROPERTY(Blob_Length)

	GB.ReturnInteger(BLOB->length);

END_PROPERTY


GB_DESC CBlobDesc[] =
{
	GB_DECLARE("Blob", sizeof(CBLOB)), GB_NOT_CREATABLE(),

	//GB_HOOK_CHECK(check_blob),

	GB_STATIC_METHOD("_init", NULL, Blob_init, NULL),
	GB_METHOD("_free", NULL, Blob_free, NULL),

	//GB_PROPERTY_READ("Result", "Result", CBLOB_result),
	GB_PROPERTY("Data", "s", Blob_Data),
	GB_PROPERTY_READ("Length", "i", Blob_Length),
	GB_INTERFACE("_convert", &_convert_blob),
	//GB_METHOD("_unknown", "v", CBLOB_unknown, "v"),

	GB_END_DECLARE
};
