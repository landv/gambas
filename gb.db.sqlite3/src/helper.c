/***************************************************************************

  helper.c

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

#define __HELPER_C

#include "helper.h"

#include "gb_buffer.h"

//--------------------------------------------------------------------------

static int _last_error = SQLITE_OK;

SQLITE_DATABASE *sqlite_open_database(const char *path, const char *host)
{
	SQLITE_DATABASE *db;
	sqlite3 *handle;

	if (!path)
		path = ":memory:";

	_last_error = sqlite3_open(path, &handle);

	if (_last_error == SQLITE_OK)
	{
		GB.Alloc(POINTER(&db), sizeof(SQLITE_DATABASE));
		db->handle = handle;
		db->path = GB.NewZeroString(path);
		db->host = GB.NewZeroString(host);
		db->error = SQLITE_OK;
		return db;
	}
	else
		return NULL;
}

void sqlite_close_database(SQLITE_DATABASE *db)
{
	sqlite3_close(db->handle);
	GB.FreeString(&db->path);
	GB.FreeString(&db->host);
	GB.Free(POINTER(&db));
}

const char *sqlite_get_error_message(SQLITE_DATABASE *db)
{
	const char *error;
	int err;

	err = db ? db->error : _last_error;

	switch (err)
	{
		case SQLITE_OK:
			error = "Successful result";
			break;
		case SQLITE_ERROR:
			error = sqlite3_errmsg(db->handle);
			break;
		case SQLITE_INTERNAL:
			error = "Internal logic error - Report this error on the mailing-list at sqlite.org";
			break;
		case SQLITE_PERM:
			error = "Access permission denied";
			break;
		case SQLITE_ABORT:
			error = "Callback routine requested an abort";
			break;
		case SQLITE_BUSY:
			error = "The database file is locked";
			break;
		case SQLITE_LOCKED:
			error = "A table in the database is locked";
			break;
		case SQLITE_NOMEM:
			error = "Out of memory";
			break;
		case SQLITE_READONLY:
			error = "Attempt to write a readonly database";
			break;
		case SQLITE_INTERRUPT:
			error = "Operation terminated by sqlite_interrupt()";
			break;
		case SQLITE_IOERR:
			error = "Some kind of disk I/O error occurred";
			break;
		case SQLITE_CORRUPT:
			error = "The database disk image is malformed";
			break;
		case SQLITE_NOTFOUND:
			error = "(Internal Only) Table or record not found";
			break;
		case SQLITE_FULL:
			error = "Insertion failed because database is full";
			break;
		case SQLITE_CANTOPEN:
			error = "Unable to open the database file";
			break;
		case SQLITE_PROTOCOL:
			error = "Database lock protocol error";
			break;
		case SQLITE_EMPTY:
			error = "(Internal Only) Database table is empty";
			break;
		case SQLITE_SCHEMA:
			error = "The database schema changed";
			break;
		case SQLITE_TOOBIG:
			error = "Too much data for one row of a table";
			break;
		case SQLITE_CONSTRAINT:
			error = "Abort due to constraint violation";
			break;
		case SQLITE_MISMATCH:
			error = "Data type mismatch";
			break;
		default:
			error = "Undefined SQLite error";
	}

	return error;
}

//--------------------------------------------------------------------------

GB_TYPE sqlite_get_type(const char *type, int *length)
{
	int i;
	char *upper;
	GB_TYPE gtype;
	char *left, *right;

	if (length)
		*length = 0;

	if (!type || !*type)
		return GB_T_STRING;
	
	upper = GB.NewZeroString(type);
	for (i = 0; i < GB.StringLength(upper); i++)
		upper[i] = toupper(upper[i]);
	type = upper;

	if (strstr(type, "CHAR(")			/* note the opening bracket */
					|| strstr(type, "CLOB") || strstr(type, "TEXT")	/* also catches TINYTEXT */
					|| strstr(type, "VARCHAR") || strstr(type, "VARYING CHAR")
					|| strstr(type, "ENUM") || strstr(type, "SET") || strstr(type, "YEAR"))
	{															/* MySQL 2 or 4 digit year (string) */
		gtype = GB_T_STRING;
	}
	else if (strstr(type, "CHAR")	/* this is a 1-byte value */
					|| strstr(type, "TINYINT")
					|| strstr(type, "INT1") || strstr(type, "BOOL"))
	{
		gtype = GB_T_BOOLEAN;
	}
	else if (strstr(type, "SMALLINT") || strstr(type, "INT2"))
	{
		gtype = GB_T_INTEGER;
	}
	else if (strstr(type, "MEDIUMINT"))
	{
		gtype = GB_T_INTEGER;
	}
	else if (strstr(type, "BIGINT") || strstr(type, "INT8"))
	{
		gtype = GB_T_LONG;
	}
	else if (strstr(type, "INTEGER")
					|| strstr(type, "INT") || strstr(type, "INT4"))
	{
		gtype = GB_T_INTEGER;
	}
	else if (strstr(type, "DECIMAL") || strstr(type, "NUMERIC"))
	{
		gtype = GB_T_FLOAT;
	}
	else if (strstr(type, "TIMESTAMP") || strstr(type, "DATETIME"))
	{
		gtype = GB_T_DATE;
	}
	else if (strstr(type, "DATE"))
	{
		gtype = GB_T_DATE;
	}
	else if (strstr(type, "TIME"))
	{
		gtype = GB_T_DATE;
	}
	else if (strstr(type, "DOUBLE") || strstr(type, "FLOAT8"))
	{
		gtype = GB_T_FLOAT;
	}
	else if (strstr(type, "REAL")	/* this is PostgreSQL "real", not
																	MySQL "real" which is a
																	synonym of "double" */
					|| strstr(type, "FLOAT")
					|| strstr(type, "FLOAT4"))
	{
		gtype = GB_T_FLOAT;
	}
	else if (strstr(type, "BLOB"))	// BM
	{
		gtype = DB_T_BLOB;;
	}
	else
		gtype = GB_T_STRING;

	if (gtype == GB_T_STRING && type && length != NULL)
	{
		/* if a length has been defined it will be between () */
		right = (char *)rindex(type, ')');
		left = (char *)index(type, '(');
		if (left && right)
		{
			*right = 0;
			*length = atoi(left + 1);
		}
	}

	GB.FreeString(&upper);
	return gtype;
}

//--------------------------------------------------------------------------

static void clear_query(SQLITE_RESULT *result)
{
	int i;

	if (!result->buffer)
		return;
	
	for (i = 0; i < result->ncol; i++)
		GB.FreeString(&result->names[i]);

	GB.Free(POINTER(&result->names));

	GB.Free(POINTER(&result->types));

	GB.Free(POINTER(&result->lengths));

	GB.FreeArray(&result->values);

	BUFFER_delete(&result->buffer);
}

void sqlite_query_free(SQLITE_RESULT *result)
{
	clear_query(result);
	GB.Free(POINTER(&result));
}

static int my_sqlite3_exec(
	sqlite3 *db,                /* The database on which the SQL executes */
	const char *zSql,           /* The SQL to be executed */
	SQLITE_RESULT *result,
	bool need_types
)
{
	int rc = SQLITE_OK;
	const char *zLeftover;
	sqlite3_stmt *pStmt = 0;
	int ncol;
	int nRetry = 0;
	int nCallback;
	int i;
	const char *decltype;
	int n_unknown = 0;
	int type;

	//fprintf(stderr, "my_sqlite3_exec: %s\n", zSql);

	if( zSql==0 ) return SQLITE_OK;
	while( (rc==SQLITE_OK || (rc==SQLITE_SCHEMA && (++nRetry)<2)) && zSql[0] )
	{
		pStmt = 0;
		//fprintf(stderr, "my_sqlite3_exec: sqlite3_prepare_v2 ?\n");
		rc = sqlite3_prepare_v2(db, zSql, -1, &pStmt, &zLeftover);
		if( rc!=SQLITE_OK ){
			//fprintf(stderr, "my_sqlite3_exec: sqlite3_prepare_v2 failed: %s\n", sqlite3_errmsg(db));
			if( pStmt ) sqlite3_finalize(pStmt);
			continue;
		}
		if( !pStmt ){
			/* this happens for a comment or white-space */
			zSql = zLeftover;
			continue;
		}

		nCallback = 0;
		
		clear_query(result);

		BUFFER_create(&result->buffer);
		
		result->ncol = ncol = sqlite3_column_count(pStmt);

		if (ncol > 0)
		{
			GB.AllocZero(POINTER(&result->names), ncol * sizeof(const char *));
			if (need_types)
			{
				GB.Alloc(POINTER(&result->types), ncol * sizeof(int));
				GB.Alloc(POINTER(&result->lengths), ncol * sizeof(int));
			}
		}

		GB.NewArray(POINTER(&result->values), sizeof(int), 0);

		for(;;)
		{
			rc = sqlite3_step(pStmt);

			/* Invoke the callback function if required */
			if((SQLITE_ROW==rc ||
					//(SQLITE_DONE==rc && !nCallback && db->flags&SQLITE_NullCallback)) ){
					(SQLITE_DONE==rc && !nCallback && 1)) )
			{
				if (0 == nCallback)
				{
					for (i = 0; i < ncol; i++)
						result->names[i] = GB.NewZeroString(sqlite3_column_name(pStmt, i));

					if (need_types)
					{
						for (i = 0; i < ncol; i++)
						{
							decltype = sqlite3_column_decltype(pStmt, i);
							if (!decltype)
							{
								result->types[i] = 0;
								result->lengths[i] = 0;
								n_unknown++;
							}
							else
							{
								result->types[i] = (int)sqlite_get_type(decltype, &result->lengths[i]);
							}
						}
					}

					nCallback++;
				}

				if (rc == SQLITE_ROW)
				{
					int *addr = GB.Insert(&result->values, -1, ncol * 2);
					char *value;
					int len;

					result->nrow++;

					for (i = 0; i < ncol; i++)
					{
						type = sqlite3_column_type(pStmt, i);
						
						if (n_unknown && result->types[i] == 0)
						{
							switch(type)
							{
								case SQLITE_INTEGER: result->types[i] = GB_T_LONG; break;
								case SQLITE_FLOAT: result->types[i] = GB_T_FLOAT; break;
								case SQLITE_BLOB: result->types[i] = (int)DB_T_BLOB; break;
								default: result->types[i] = GB_T_STRING;
							}
							
							n_unknown--;
						}
						
						if (type == SQLITE_BLOB)
							value = (char *)sqlite3_column_blob(pStmt, i);
						else
							value = (char *)sqlite3_column_text(pStmt, i);

						len = sqlite3_column_bytes(pStmt, i);

						if (len == 0)
							addr[i * 2] = BUFFER_length(result->buffer) - 1;
						else
						{
							addr[i * 2] = BUFFER_add(&result->buffer, value, len + 1);
							result->buffer[BUFFER_length(result->buffer) - 1] = 0;
						}

						addr[i * 2 + 1] = len;

					}
				}
			}

			if (rc != SQLITE_ROW)
			{
				rc = sqlite3_finalize(pStmt);
				pStmt = 0;

				if (rc != SQLITE_SCHEMA)
				{
					nRetry = 0;
					zSql = zLeftover;
					while( isspace((unsigned char)zSql[0]) ) zSql++;
				}

				break;
			}
		}
	}

	if (pStmt)
		sqlite3_finalize(pStmt);

	return rc;
}


SQLITE_RESULT *sqlite_query_exec(SQLITE_DATABASE *db, const char *query, bool need_types)
{
	SQLITE_RESULT *result;
	int retry;
	int res;

	GB.AllocZero(POINTER(&result), sizeof(SQLITE_RESULT));

	for (retry = 1; retry <= 2; retry++)
	{
		res = my_sqlite3_exec(db->handle, query, result, need_types);
		if (res != SQLITE_SCHEMA)
			break;
	}

	if (res != SQLITE_OK)
	{
		db->error = res;
		sqlite_query_free(result);
		return NULL;
	}
	else
		return result;
}

void sqlite_query_get(SQLITE_RESULT *result, int pos, int col, char **value, int *length)
{
	int i;

	if (pos < 0 || pos >= result->nrow || col < 0 || col >= result->ncol)
	{
		*value = NULL;
		if (length)
			*length = 0;
		return;
	}

	i = pos * result->ncol * 2 + col * 2;
	*value = result->buffer + result->values[i];
	if (length)
		*length = result->values[i + 1];
}

char *sqlite_query_get_string(SQLITE_RESULT *result, int pos, int col)
{
	char *value;
	sqlite_query_get(result, pos, col, &value, NULL);
	return value ? value : "";
}

int sqlite_query_get_int(SQLITE_RESULT *result, int pos, int col)
{
	char *value;

	sqlite_query_get(result, pos, col, &value, NULL);
	if (!value)
		return 0;
	else
		return atoi(value);
}


int sqlite_query_find_field(SQLITE_RESULT *result, const char *name)
{
	int i;
	char *field;
	char *p;

	/*fprintf(stderr, "sqlite_query_find_field: %s\n", name);

	for (i = 0; i < result->ncol; i++)
		fprintf(stderr, "'%s' ", result->names[i]);
	fprintf(stderr, "\n");*/

	if (strchr(name, '.'))
	{
		for (i = 0; i < result->ncol; i++)
		{
			if (strcmp(result->names[i], name) == 0)
					return i;
		}
	}
	else
	{
		for (i = 0; i < result->ncol; i++)
		{
			field = result->names[i];
			p = strchr(field, '.');
			if (p)
				field = p + 1;

			if (strcmp(field, name) == 0)
					return i;
		}
	}

	return -1;
}
