/***************************************************************************

  main.c

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

#define __MAIN_C

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>

#include <mysql.h>

#include "gb.db.proto.h"
#include "main.h"

typedef
	struct {
		const char *pattern;
		int type;
		}
	CONV_STRING_TYPE;


GB_INTERFACE GB EXPORT;
DB_INTERFACE DB EXPORT;

static char _buffer[125];
static DB_DRIVER _driver;
/*static int _print_query = FALSE;*/

// Cached queries
#define CACHE(_db) ((GB_HASHTABLE)(_db)->data)

typedef
	struct {
		MYSQL_RES *res;
		time_t timestamp;
	}
	CACHE_ENTRY;

/* internal function to quote a value stored as a string */

static void quote_string(const char *data, long len, DB_FORMAT_CALLBACK add)
{
	int i;
	unsigned char c;
	//char buffer[8];

	(*add)("'", 1);
	for (i = 0; i < len; i++)
	{
		c = (unsigned char)data[i];
		if (c == '\\')
			(*add)("\\\\", 2);
		else if (c == '\'')
			(*add)("''", 2);
		else if (c == 0)
			(*add)("\\0", 2);
		else
			(*add)((char *)&c, 1);
	}
	(*add)("'", 1);
}

/* internal function to quote a value stored as a blob */

#define quote_blob quote_string

/* Quote a string and returns the result as a temporary string */

static char *get_quote_string(const char *str, int len, char quote)
{
	char *res, *p, c;
	int len_res;
	int i;
	
	len_res = len;
	for (i = 0; i < len; i++)
	{
		c = str[i];
		if (c == quote || c == '\\' || c == 0)
			len_res++;
	}
	
	res = GB.TempString(NULL, len_res);
	
	p = res;
	for (i = 0; i < len; i++)
	{
		c = str[i];
		if (c == '\\' || c == quote)
		{
			*p++ = c;
			*p++ = c;
		}
		else if (c == 0)
		{
			*p++ = '\\';
			*p++ = '0';
		}
		else
			*p++ = c;
	}
	*p = 0;
	
	return res;
}

/* Internal function to convert a database type into a Gambas type */

static GB_TYPE conv_type(int type, int len)
{
	switch(type)
	{
		case FIELD_TYPE_TINY:
			return (len == 1 ? GB_T_BOOLEAN : GB_T_INTEGER);

		case FIELD_TYPE_INT24:
		case FIELD_TYPE_SHORT:
		case FIELD_TYPE_LONG:
		case FIELD_TYPE_YEAR:
			return GB_T_INTEGER;

		case FIELD_TYPE_LONGLONG:
			return GB_T_LONG;

		case FIELD_TYPE_FLOAT:
		case FIELD_TYPE_DOUBLE:
		case FIELD_TYPE_DECIMAL:
			return GB_T_FLOAT;

		case FIELD_TYPE_DATE:
		case FIELD_TYPE_DATETIME:
		case FIELD_TYPE_TIME:
		case FIELD_TYPE_TIMESTAMP:
			return GB_T_DATE;

		case FIELD_TYPE_LONG_BLOB:
			//fprintf(stderr, "FIELD_TYPE_LONG_BLOB: %d\n", len);
			return DB_T_BLOB;

		case FIELD_TYPE_BLOB: // TEXT
			//fprintf(stderr, "FIELD_TYPE_BLOB: %d\n", len);
			if (len >= 0x1000000 || len <= 0) // LONG BLOB
				return DB_T_BLOB;
			else
				return GB_T_STRING;
			
		case FIELD_TYPE_BIT:
			if (len == 1)
				return GB_T_BOOLEAN;
			else if (len <= 32)
				return GB_T_INTEGER;
			else if (len <= 64)
				return GB_T_LONG;

		case FIELD_TYPE_TINY_BLOB:
		case FIELD_TYPE_MEDIUM_BLOB:
		case FIELD_TYPE_STRING:
		case FIELD_TYPE_VAR_STRING:
		case FIELD_TYPE_SET:
		case FIELD_TYPE_ENUM:
		default:
			//fprintf(stderr, "FIELD_TYPE_*: %d\n", len);
			return GB_T_STRING;

	}
}


/* Internal function to convert a string database type into an integer database type */

static int conv_string_type(const char *type, long *len)
{
	static CONV_STRING_TYPE types[] =
	{
		{ "tinyint", FIELD_TYPE_TINY },
		{ "smallint", FIELD_TYPE_SHORT },
		{ "mediumint", FIELD_TYPE_INT24 },
		{ "int", FIELD_TYPE_LONG },
		{ "bigint", FIELD_TYPE_LONGLONG },
		{ "decimal", FIELD_TYPE_DECIMAL },
		{ "numeric", FIELD_TYPE_DECIMAL },
		{ "float", FIELD_TYPE_FLOAT },
		{ "double", FIELD_TYPE_DOUBLE },
		{ "real", FIELD_TYPE_DOUBLE },
		{ "timestamp", FIELD_TYPE_TIMESTAMP },
		{ "date", FIELD_TYPE_DATE },
		{ "time", FIELD_TYPE_TIME },
		{ "datetime", FIELD_TYPE_DATETIME },
		{ "year", FIELD_TYPE_YEAR },
		{ "char", FIELD_TYPE_STRING },
		{ "varchar", FIELD_TYPE_VAR_STRING },
		{ "blob", FIELD_TYPE_BLOB },
		{ "tinyblob", FIELD_TYPE_TINY_BLOB },
		{ "mediumblob", FIELD_TYPE_MEDIUM_BLOB },
		{ "longblob", FIELD_TYPE_LONG_BLOB },
		{ "text", FIELD_TYPE_BLOB },
		{ "tinytext", FIELD_TYPE_TINY_BLOB },
		{ "mediumtext", FIELD_TYPE_MEDIUM_BLOB },
		{ "longtext", FIELD_TYPE_LONG_BLOB },
		{ "set", FIELD_TYPE_SET },
		{ "enum", FIELD_TYPE_ENUM },
		{ "bit", FIELD_TYPE_BIT },
		{ "null", FIELD_TYPE_NULL },
		{ NULL, 0 },
	};

	CONV_STRING_TYPE *cst;
	long l;

	if (strncmp(type, "national ", 9) == 0)
		type += 9;

	for (cst = types; cst->pattern; cst++)
	{
		if (strncmp(type, cst->pattern, strlen(cst->pattern)) == 0)
			break;
	}

	if (cst->type)
	{
		if (len)
		{
			type += strlen(cst->pattern);
			if (sscanf(type, "(%ld)", &l) == 1)
				*len = l;
			else if (cst->type == FIELD_TYPE_LONG_BLOB)
				*len = -1;
			else if (cst->type == FIELD_TYPE_MEDIUM_BLOB || cst->type == FIELD_TYPE_BLOB)
				*len = 65535;
			else
				*len = 0;
		}
	}

	return cst->type;
}


/* Internal function to convert a database value into a Gambas variant value */

static void conv_data(int version, const char *data, long data_length, GB_VARIANT_VALUE *val, int type, int len)
{
	GB_VALUE conv;
	GB_DATE_SERIAL date;
	double sec;

	switch (type)
	{
		case FIELD_TYPE_TINY:

			if (len == 1)
			{
				val->type = GB_T_BOOLEAN;
				/*GB.NumberFromString(GB_NB_READ_INTEGER, data, strlen(data), &conv);*/
				val->value._boolean = atoi(data) != 0 ? -1 : 0;
			}
			else
			{
				GB.NumberFromString(GB_NB_READ_INTEGER, data, strlen(data), &conv);

				val->type = GB_T_INTEGER;
				val->value._integer = conv._integer.value;
			}

			break;

		case FIELD_TYPE_INT24:
		case FIELD_TYPE_SHORT:
		case FIELD_TYPE_LONG:
		/*case FIELD_TYPE_TINY:*/
		case FIELD_TYPE_YEAR:

			GB.NumberFromString(GB_NB_READ_INTEGER, data, strlen(data), &conv);

			val->type = GB_T_INTEGER;
			val->value._integer = conv._integer.value;

			break;

		case FIELD_TYPE_LONGLONG:

			GB.NumberFromString(GB_NB_READ_LONG, data, strlen(data), &conv);

			val->type = GB_T_LONG;
			val->value._long = conv._long.value;

			break;

		case FIELD_TYPE_FLOAT:
		case FIELD_TYPE_DOUBLE:
		case FIELD_TYPE_DECIMAL:

			GB.NumberFromString(GB_NB_READ_FLOAT, data, strlen(data), &conv);

			val->type = GB_T_FLOAT;
			val->value._float = conv._float.value;

			break;

		case FIELD_TYPE_DATE:
		case FIELD_TYPE_DATETIME:
		case FIELD_TYPE_TIME:
		case FIELD_TYPE_TIMESTAMP:

			// TIMESTAMP display format changed since MySQL 4.1!
			if (type == FIELD_TYPE_TIMESTAMP && version >= 40100)
				type = FIELD_TYPE_DATETIME;

			memset(&date, 0, sizeof(date));

			switch(type)
			{
				case FIELD_TYPE_DATE:

					sscanf(data, "%4d-%2d-%2d", &date.year, &date.month, &date.day);
					break;

				case FIELD_TYPE_TIME:

					sscanf(data, "%4d:%2d:%lf", &date.hour, &date.min, &sec);
					date.sec = (short)sec;
					date.msec = (short)((sec - date.sec) * 1000 + 0.5);
					break;

				case FIELD_TYPE_DATETIME:

					sscanf(data, "%4d-%2d-%2d %2d:%2d:%lf", &date.year, &date.month, &date.day, &date.hour, &date.min, &sec);
					date.sec = (short)sec;
					date.msec = (short)((sec - date.sec) * 1000 + 0.5);
					break;

				case FIELD_TYPE_TIMESTAMP:
					switch(strlen(data))
					{
						case 14:
							sscanf(data, "%4d%2d%2d%2d%2d%lf", &date.year, &date.month, &date.day, &date.hour, &date.min, &sec);
							date.sec = (short)sec;
							date.msec = (short)((sec - date.sec) * 1000 + 0.5);
							break;
						case 12:
							sscanf(data, "%2d%2d%2d%2d%2d%lf", &date.year, &date.month, &date.day, &date.hour, &date.min, &sec);
							date.sec = (short)sec;
							date.msec = (short)((sec - date.sec) * 1000 + 0.5);
							break;
						case 10:
							sscanf(data, "%2d%2d%2d%2d%2d", &date.year, &date.month, &date.day, &date.hour, &date.min );
							break;
						case 8:
							sscanf(data, "%4d%2d%2d", &date.year, &date.month, &date.day);
							break;
						case 6:
							sscanf(data, "%2d%2d%2d", &date.year, &date.month, &date.day);
							break;
						case 4:
							sscanf(data, "%2d%2d", &date.year, &date.month);
							break;
						case 2:
							sscanf(data, "%2d", &date.year);
							break;
					}
					if (date.year < 100)
							date.year += 1900;
				break;
			}

			GB.MakeDate(&date, (GB_DATE *)&conv);

			val->type = GB_T_DATE;
			val->value._date.date = conv._date.value.date;
			val->value._date.time = conv._date.value.time;

			break;

		case FIELD_TYPE_LONG_BLOB:
			// The BLOB are read by the blob_read() driver function
			// You must set NULL there.
			val->type = GB_T_NULL;
			break;

		// query_fill() only gets this constant, whatever the blob is
		case FIELD_TYPE_BLOB:
			if (len == 16777215 || len <= 0) // LONG BLOB
			{
				val->type = GB_T_NULL;
				break;
			}
			// else continue!

		case FIELD_TYPE_TINY_BLOB:
		case FIELD_TYPE_MEDIUM_BLOB:
		case FIELD_TYPE_STRING:
		case FIELD_TYPE_VAR_STRING:
		case FIELD_TYPE_SET:
		case FIELD_TYPE_ENUM:
		default:
			val->type = GB_T_CSTRING;
			val->value._string = (char *)data;
			//val->_string.start = 0;
			//if (data && data_length == 0)
			//	data_length = strlen(data);
			//val->_string.len = data_length;
			//fprintf(stderr, "conv_data: len = %d\n", len);
			/*GB.NewString(&val->_string.value, data, strlen(data));*/

			break;
	}

}

/* Internal function to substitute the table name into a query */

static char *query_param[3];

static void query_get_param(int index, char **str, int *len, char quote)
{
	if (index > 3)
		return;

	index--;
	*str = query_param[index];
	*len = strlen(*str);
	
	if (quote == '\'' || quote == '`')
	{
		*str = get_quote_string(*str, *len, quote);
		*len = GB.StringLength(*str);
	}
}

static void check_connection(MYSQL *conn)
{
	unsigned long thread_id;

	thread_id = mysql_thread_id(conn);

	mysql_ping(conn);

	if (mysql_thread_id(conn) != thread_id)
	{
		if (DB.IsDebug())
			fprintf(stderr, "gb.db.mysql: connection lost\n");
		// Connection has been reestablished, set utf8 again
		mysql_query(conn, "set names 'utf8'");
	}
}

/* Internal function to run a query */

static int do_query(DB_DATABASE *db, const char *error, MYSQL_RES **pres, const char *qtemp, int nsubst, ...)
{
	MYSQL *conn = (MYSQL *)db->handle;
	va_list args;
	int i;
	const char *query;
	MYSQL_RES *res;
	int ret;

	if (nsubst)
	{
		va_start(args, nsubst);
		if (nsubst > 3)
			nsubst = 3;
		for (i = 0; i < nsubst; i++)
			query_param[i] = va_arg(args, char *);

		query = DB.SubstString(qtemp, 0, query_get_param);
	}
	else
		query = qtemp;

	if (DB.IsDebug())
		fprintf(stderr, "gb.db.mysql: %p: %s\n", conn, query);

	check_connection(conn);

	if (mysql_query(conn, query))
	{
		ret = TRUE;
		if (error)
			GB.Error(error, mysql_error(conn));
	}
	else 
	{
		res = mysql_store_result(conn);
		ret = FALSE;
		if (pres)
			*pres = res;
		else
			mysql_free_result(res);
	}

	db->error = mysql_errno(conn);
	return ret;
}

static int do_query_cached(DB_DATABASE *db, const char *error, MYSQL_RES **pres, char *key, const char *qtemp, int nsubst, ...)
{
	CACHE_ENTRY *entry;
	int len_key;
	bool added;
	time_t t;
	va_list args;
	int i;
	const char *query;
	int ret;

	if (nsubst)
	{
		va_start(args, nsubst);
		if (nsubst > 3)
			nsubst = 3;
		for (i = 0; i < nsubst; i++)
			query_param[i] = va_arg(args, char *);

		query = DB.SubstString(qtemp, 0, query_get_param);
		key = DB.SubstString(key, 0, query_get_param);
	}
	else
		query = qtemp;

	len_key = strlen(key);
	added = GB.HashTable.Get(CACHE(db), key, len_key, POINTER(&entry));
	if (added)
	{
		GB.AllocZero(POINTER(&entry), sizeof(CACHE_ENTRY));
		GB.HashTable.Add(CACHE(db), key, len_key, entry);
	}

	t = time(NULL);

	//fprintf(stderr, "-- do_query_cached: %s [ %p %ld ]\n", key, entry->res, entry->timestamp);

	if (entry->res &&  (t - entry->timestamp) < 30)
	{
		mysql_data_seek(entry->res, 0);
		*pres = entry->res;
		return 0;
	}

	entry->timestamp = t;

	if (entry->res)
		mysql_free_result(entry->res);

	ret = do_query(db, error, &entry->res, query, 0);
	if (ret == 0)
		*pres = entry->res;
	return ret;
}


/* Internal function to return database version number as a XXYYZZ integer number*/

static int db_version(DB_DATABASE *db)
{
	//Check db version
	const char *vquery = "select left(version(),6)";
	long dbversion =0;
	MYSQL_RES *res;
	MYSQL_ROW row;

	if (!do_query(db, NULL, &res, vquery, 0))
	{
		unsigned int verMain, verMajor, verMinor;
		row = mysql_fetch_row(res);
		sscanf(row[0],"%2u.%2u.%2u", &verMain, &verMajor, &verMinor);
		dbversion = ((verMain * 10000) + (verMajor * 100) + verMinor);
		mysql_free_result(res);
	}
	return dbversion;
}

/* Search in the first column a result for a specific name */

static bool search_result(MYSQL_RES *res, const char *name, MYSQL_ROW *row)
{
	int i;
	MYSQL_ROW r;
	
	for (i = 0; i < mysql_num_rows(res); i++ )
	{
		r = mysql_fetch_row(res);

		if (!strcmp(r[0], name))
		{
			if (row) 
				*row = r;
			break;
		}
	}
		
	return (i >= mysql_num_rows(res));
}

// Clear the query cache

static void free_cache(void *data)
{
	GB.Free(&data);
}

static void clear_cache(DB_DATABASE *db)
{
	GB.HashTable.Enum(CACHE(db), free_cache);
	GB.HashTable.Free((GB_HASHTABLE *)&db->data);
}

static void remove_cache_entry(DB_DATABASE *db, char *key)
{
	CACHE_ENTRY *entry;

	if (GB.HashTable.Get(CACHE(db), key, -1, POINTER(&entry)))
		return;

	mysql_free_result(entry->res);
	GB.Free(POINTER(&entry));
	GB.HashTable.Remove((GB_HASHTABLE *)&db->data, key, -1);
}

static void clear_table_cache(DB_DATABASE *db, const char *table)
{
	char key[strlen(table) + 5];

	strcpy(key, "sts:"); strcat(key, table); remove_cache_entry(db, key);
	//strcpy(key, "sc:"); strcat(key, table); remove_cache_entry(db, key);
	strcpy(key, "sfc:"); strcat(key, table); remove_cache_entry(db, key);
	strcpy(key, "si:"); strcat(key, table); remove_cache_entry(db, key);
}

/*****************************************************************************

	get_quote()

	Returns the character used for quoting object names.

*****************************************************************************/

static const char *get_quote(void)
{
	return QUOTE_STRING;
}

/*****************************************************************************

	open_database()

	Connect to a database.

	<desc> points at a structure describing each connection parameter.

	This function must return a database handle, or NULL if the connection
	has failed.

*****************************************************************************/

static void set_character_set(DB_DATABASE *db)
{
	MYSQL_RES *res;
	MYSQL_ROW row;

//	sys_charset = GB.System.Charset();
//	db_charset = NULL;

// 	for (i = 0; i < strlen(sys_charset); i++)
// 	{
// 		c = sys_charset[i];
// 		if (!c)
// 			break;
// 		c = tolower(c);
// 		if (!((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9')))
// 			continue;
// 		GB.AddString(&db_charset, (const char *)&c, 1);
// 	}
// 	
// 	db_charset[i] = 0;

	if (do_query(db, NULL, NULL, "set names 'utf8'", 0))
		fprintf(stderr, "WARNING: Unable to set database charset to UTF-8\n");
		
//	GB.FreeString(&db_charset);
	
	if (do_query(db, "Unable to get database charset: &1", &res, "show variables like 'character_set_client'", 0))
		return;
	
	if (search_result(res, "character_set_client", &row))
		return;
	
	db->charset = GB.NewZeroString(row[1]);
	//fprintf(stderr, "charset is '%s'\n", db->charset);
	mysql_free_result(res);
}

static int open_database(DB_DESC *desc, DB_DATABASE *db)
{
	MYSQL *conn;
	char *name;
	char *host;
	char *socket;
	my_bool reconnect = TRUE;
	unsigned int timeout;

	conn = mysql_init(NULL);

	/* (BM) connect by default to the mysql database */

	if (desc->name)
		name = desc->name;
	else
		name = "mysql"; /* Note: Users may not have access to database mysql */

	//mysql_options(conn, MYSQL_READ_DEFAULT_GROUP,"Gambas");

	//fprintf(stderr, "mysql_real_connect: host = '%s'\n", desc->host);

	host = desc->host;
	if (host && *host == '/')
	{
		socket = host;
		host = NULL;
	}
	else
		socket = NULL;
	
	mysql_options(conn, MYSQL_OPT_RECONNECT, &reconnect);
	
	timeout = db->timeout;
	mysql_options(conn, MYSQL_OPT_CONNECT_TIMEOUT, &timeout);
	/*timeout /= 3;
	mysql_options(conn, MYSQL_OPT_READ_TIMEOUT, &timeout);*/
	
	if (!mysql_real_connect(conn, host, desc->user, desc->password,
			name, desc->port == NULL ? 0 : atoi(desc->port), socket,
			CLIENT_MULTI_RESULTS | CLIENT_REMEMBER_OPTIONS /*client flag */)){
		mysql_close(conn);
		GB.Error("Cannot open database: &1", mysql_error(conn));
		return TRUE;
	}

	db->handle = conn;
	db->version = db_version(db);

	set_character_set(db);
	
	GB.HashTable.New(POINTER(&db->data), GB_COMP_BINARY);
	/* flags: none at the moment */
	return FALSE;
}


/*****************************************************************************

	close_database()

	Terminates the database connection.

	<handle> contains the database handle.

*****************************************************************************/

static void close_database(DB_DATABASE *db)
{
	MYSQL *conn = (MYSQL *)db->handle;

	clear_cache(db);

	if (conn)
		mysql_close(conn);
}


/*****************************************************************************

	get_collations()

	Return the available collations as a Gambas string array.

*****************************************************************************/

static GB_ARRAY get_collations(DB_DATABASE *db)
{
	const char *query = "show collation like '%'";

	MYSQL_RES *res;
	GB_ARRAY array;
	MYSQL_ROW row;
	int i, n;

	if (do_query(db, "Unable to get collations: &1", &res, query, 0))
		return NULL;

	n = mysql_num_rows(res);

	GB.Array.New(&array, GB_T_STRING, n);
	for (i = 0; i < n; i++)
	{
		row = mysql_fetch_row(res);
		*((char **)GB.Array.Get(array, i)) = GB.NewZeroString(row[0]);
	}

	return array;
}

/*****************************************************************************

	format_value()

	This function transforms a gambas value into a string value that can
	be inserted into a SQL query.

	<arg> points to the value.
	<add> is a callback called to insert the string into the query.

	This function must return TRUE if it translates the value, and FALSE if
	it does not.

	If the value is not translated, then a default translation is used.

*****************************************************************************/

static int format_value(GB_VALUE *arg, DB_FORMAT_CALLBACK add)
{
	int l;
	GB_DATE_SERIAL *date;

	switch (arg->type)
	{
		case GB_T_BOOLEAN:
/*Note this is likely to go to a tinyint  */

			if (VALUE((GB_BOOLEAN *)arg))
				add("'1'", 3);
			else
				add("'0'", 3);
			return TRUE;

		case GB_T_STRING:
		case GB_T_CSTRING:

			quote_string(VALUE((GB_STRING *)arg).addr + VALUE((GB_STRING *)arg).start, VALUE((GB_STRING *)arg).len, add);
			return TRUE;

		case GB_T_DATE:

			date = GB.SplitDate((GB_DATE *)arg);

			l = sprintf(_buffer, "'%04d-%02d-%02d %02d:%02d:%02d",
					date->year, date->month, date->day,
					date->hour, date->min, date->sec);

			add(_buffer, l);

			if (date->msec)
			{
				l = sprintf(_buffer, ".%03d", date->msec);
				add(_buffer, l);
			}

			add("'", 1);

			//fprintf(stderr, "format_value: %s / %d %d\n", _buffer, ((GB_DATE *)arg)->value.time, date->msec);

			return TRUE;

		default:
			return FALSE;
	}
}


/*****************************************************************************

	format_blob()

	This function transforms a blob value into a string value that can
	be inserted into a SQL query.

	<blob> points to the DB_BLOB structure.
	<add> is a callback called to insert the string into the query.

*****************************************************************************/

static void format_blob(DB_BLOB *blob, DB_FORMAT_CALLBACK add)
{
	quote_blob(blob->data, blob->length, add);
}


/*****************************************************************************

	exec_query()

	Send a query to the server and gets the result.

	<db> is the database handle, as returned by open_database()
	<query> is the query string.
	<result> will receive the result handle of the query.
	<err> is an error message used when the query failed.

	<result> can be NULL, when we don't care getting the result.

*****************************************************************************/

static int exec_query(DB_DATABASE *db, const char *query, DB_RESULT *result, const char *err)
{
	return do_query(db, err, (MYSQL_RES **)result, query, 0);
}


/*****************************************************************************

	query_init()

	Initialize an info structure from a query result.

	<result> is the handle of the query result.
	<info> points to the info structure.
	<count> will receive the number of records returned by the query.

	This function must initialize the info->nfield field with the number of
	field in the query result.

*****************************************************************************/

static void query_init(DB_RESULT result, DB_INFO *info, int *count)
{
	MYSQL_RES *res = (MYSQL_RES *)result;

	if (res)
	{
		*count = mysql_num_rows(res);
		info->nfield = mysql_num_fields(res);
	}
	else
	{
		*count = 0;
		info->nfield = 0;
	}
}


/*****************************************************************************

	query_release()

	Free the info structure filled by query_init() and the result handle.

	<result> is the handle of the query result.
	<info> points to the info structure.

*****************************************************************************/

static void query_release(DB_RESULT result, DB_INFO *info)
{
	mysql_free_result((MYSQL_RES *)result);
}


/*****************************************************************************

	query_fill()

	Fill a result buffer with the value of each field of a record.

	<db> is the database handle, as returned by open_database()
	<result> is the handle of the result.
	<pos> is the index of the record in the result.
	<buffer> points to an array having one element for each field in the
	result.
	<next> is a boolean telling if we want the next row.

	This function must use GB.StoreVariant() to store the value in the
	buffer.

*****************************************************************************/

static int query_fill(DB_DATABASE *db, DB_RESULT result, int pos, GB_VARIANT_VALUE *buffer, int next)
{
	MYSQL_RES *res = (MYSQL_RES *)result;
	MYSQL_FIELD *field;
	MYSQL_ROW row;
	int i;
	char *data;
	GB_VARIANT value;

	if (!next)
		mysql_data_seek(res, pos);/* move to record */

	row = mysql_fetch_row(res);
	mysql_field_seek(res, 0);
	for ( i=0; i < mysql_num_fields(res); i++)
	{
		field = mysql_fetch_field(res);
		data = row[i];

		value.type = GB_T_VARIANT;
		value.value.type = GB_T_NULL;

		if (data)
			conv_data(db->version, data, mysql_fetch_lengths(res)[i], &value.value, field->type, field->length);

		GB.StoreVariant(&value, &buffer[i]);
	
		//fprintf(stderr, "query_fill: %d: (%d, %d) : %s : %d\n", i, field->type, field->length, data, buffer[i].type);
	}

	return FALSE;
}


/*****************************************************************************

	blob_read()

	Returns the value of a BLOB field.

	<result> is the handle of the result.
	<pos> is the index of the record in the result.
	<blob> points at a DB_BLOB structure that will receive a pointer to the
	data and its length.

*****************************************************************************/

static void blob_read(DB_RESULT result, int pos, int field, DB_BLOB *blob)
{
	MYSQL_RES *res = (MYSQL_RES *)result;
	MYSQL_ROW row;

	mysql_data_seek(res, pos);/* move to record */
	row = mysql_fetch_row(res);

	blob->data = row[field];
	blob->length = mysql_fetch_lengths(res)[field];
	blob->constant = TRUE;

	//fprintf(stderr, "blob_read: %ld: %s\n", blob->length, blob->data);
}


/*****************************************************************************

	field_name()

	Return the name of a field in a result from its index.

	<result> is the result handle.
	<field> is the field index.

*****************************************************************************/

static char *field_name(DB_RESULT result, int field)
{
	MYSQL_FIELD *fld;
	int i, num_fields = mysql_num_fields((MYSQL_RES *)result);
	char *table1 = mysql_fetch_field_direct((MYSQL_RES *)result, 0)->table;
	bool MultiTables = FALSE;

	// Need to identify whether multiple tables included
				fld = mysql_fetch_fields((MYSQL_RES *)result);
				for ( i = 1; i < num_fields; i++ ){
		if (strcmp(table1, fld[i].table) != 0){
			MultiTables = TRUE;
			break;
		}
	}
				fld = mysql_fetch_field_direct((MYSQL_RES *)result, field);
	// GB.Alloc((void **)&full, strlen(fld->table) + strlen(fld->name));
	if (MultiTables && *fld->table){
		sprintf(_buffer, "%s.%s", fld->table, fld->name);
		return _buffer;
	}
	else {
		return fld->name;
	}
	//return mysql_fetch_field_direct((MYSQL_RES *)result, field)->name;
}


/*****************************************************************************

	field_index()

	Return the index of a field in a result from its name.

	<Result> is the result handle.
	<name> is the field name.

*****************************************************************************/

static int field_index(DB_RESULT Result, const char *name, DB_DATABASE *db)
{
	unsigned int num_fields;
	unsigned int i;
	MYSQL_FIELD *field;
	//char *table = NULL, *fld = NULL;
	char *table;
	const char *fld;
	MYSQL_RES *result = (MYSQL_RES *)Result;

	fld = strchr(name, (int)FLD_SEP);
	if (fld)
	{ /* Field does includes table info */
		table = GB.NewString(name, fld - name);
		fld = fld + 1;
	}
	else 
	{
		table = NULL;
		fld = name;
	}

	num_fields = mysql_num_fields(result);

	if (strcmp(name,fld)!=0)
	{ /* table name included */
		mysql_field_seek(result,0); /* start at beginning */
		for (i = 0; i < num_fields; i++)
		{
			field = mysql_fetch_field(result);
			if (strcmp( fld, field->name) == 0 && strcmp( table, field->table) == 0)
			{
				GB.FreeString(&table); 
				return i;
			}
		}
		fld = name;
	}

	if (table)
		GB.FreeString(&table);

	/* Do not consider table name, also reached where table cannot be found. *
	* Mysql can include . in the fieldname!!                                */
	mysql_field_seek(result, 0); /* start at beginning */
	for (i = 0; i < num_fields; i++)
	{
		field = mysql_fetch_field(result);
		if (strcmp( fld, field->name) == 0)
			return i;
	}

	return -1;
}


/*****************************************************************************

	field_type()

	Return the Gambas type of a field in a result from its index.

	<result> is the result handle.
	<field> is the field index.

*****************************************************************************/

static GB_TYPE field_type(DB_RESULT result, int field)
{
	MYSQL_FIELD *f = mysql_fetch_field_direct((MYSQL_RES *)result, field);
	return conv_type(f->type, f->length);
}


/*****************************************************************************

	field_length()

	Return the length of a field in a result from its index.

	<result> is the result handle.
	<field> is the field index.

*****************************************************************************/

static int field_length(DB_RESULT result, int field)
{
	MYSQL_FIELD *f = mysql_fetch_field_direct((MYSQL_RES *)result, field);
	GB_TYPE type = conv_type(f->type, f->length);

	if (type != GB_T_STRING)
		return 0;
	else
		return f->length;
}


/*****************************************************************************

	begin_transaction()

	Begin a transaction.

	<handle> is the database handle.

	This function returns TRUE if the command has failed, and FALSE if
	everything was OK.

	In mysql commit/rollback can only be used with transaction safe tables (BDB,
	or InnoDB tables)

	ISAM, MyISAM and HEAP tables will commit straight away. The transaction
	methods are therefore ignored.

*****************************************************************************/

static int begin_transaction(DB_DATABASE *db)
{
	/* Autocommit is on by default. Lets set it off. */
	/* BM: why not doing that when we open the connection ? */
	do_query(db, "Unable to set autocommit to 0: &1", NULL, "set autocommit=0", 0);
	return do_query(db, "Unable to begin transaction: &1", NULL, "BEGIN", 0);
}


/*****************************************************************************

	commit_transaction()

	Commit a transaction.

	<handle> is the database handle.

	This function returns TRUE if the command has failed, and FALSE if
	everything was OK.

*****************************************************************************/

static int commit_transaction(DB_DATABASE *db)
{
	bool ret = do_query(db, "Unable to commit transaction: &1", NULL, "COMMIT", 0);
	/* Autocommit needs to be set back on. */
	/* BM: and what happens if transactions are imbricated ? */
	do_query(db, "Unable to set autocommit to On: &1", NULL, "set autocommit=1", 0);
	return ret;
}


/*****************************************************************************

	rollback_transaction()

	Rollback a transaction.

	<handle> is the database handle.

	This function returns TRUE if the command has failed, and FALSE if
	everything was OK.

	In mysql commit/rollback can only be used with transaction safe tables (BDB,
	or InnoDB tables)

	ISAM, MyISAM and HEAP tables will commit straight away. Therefore a rollback
	cannot occur!

*****************************************************************************/

static int rollback_transaction(DB_DATABASE *db)
{
	bool ret = do_query(db, "Unable to rollback transaction: &1", NULL, "ROLLBACK", 0);
	/* Autocommit needs to be set back on. */
	/* BM: and what happens if transactions are imbricated ? */
	do_query(db, "Unable to set autocommit to On: &1", NULL, "set autocommit=1", 0);
	return ret;
}

/*****************************************************************************

	table_init()

	Initialize an info structure from table fields.

	<handle> is the database handle.
	<table> is the table name.
	<info> points at the info structure.

	This function must initialize the following info fields:
	- info->nfield must contain the number of fields in the table.
	- info->fields is a char*[] pointing at the name of each field.

	This function returns TRUE if the command has failed, and FALSE if
	everything was OK.

*****************************************************************************/

static int field_info(DB_DATABASE *db, const char *table, const char *field, DB_FIELD *info);

static int table_init(DB_DATABASE *db, const char *table, DB_INFO *info)
{
	MYSQL_FIELD *field;

	MYSQL *conn = (MYSQL *)db->handle;
	MYSQL_RES *res;
	int i, n;
	DB_FIELD *f;

	/* Nom de la table */

	info->table = GB.NewZeroString(table);

	check_connection(conn);

	res = mysql_list_fields( conn, table, 0);
	if (!res)
		return TRUE;

	info->nfield = n = mysql_num_fields(res);
	if (n == 0)
		return TRUE;

	GB.Alloc((void **)POINTER(&info->field), sizeof(DB_FIELD) * n);

	i = 0;

	while ((field = mysql_fetch_field(res)))
	{
		f = &info->field[i];

		if (field_info(db, table, field->name, f))
		{
			mysql_free_result(res);
			return TRUE;
		}

		f->name = GB.NewZeroString(field->name);

		/*f->type = conv_type(field->type, field->length);
		f->length = 0;
		if (f->type == GB_T_STRING)
			f->length = field->length;*/

		i++;
	}

	mysql_free_result(res);

	return FALSE;
}


/*****************************************************************************

	table_index()

	Initialize an info structure from table primary index.

	<handle> is the database handle.
	<table> is the table name.
	<info> points at the info structure.

	This function must initialize the following info fields:
	- info->nindex must contain the number of fields in the primary index.
	- info->index is a int[] giving the index of each index field in
		info->fields.

	This function must be called after table_init().

	This function returns TRUE if the command has failed, and FALSE if
	everything was OK.

*****************************************************************************/

static int table_index(DB_DATABASE *db, const char *table, DB_INFO *info)
{
	char *qindex = "show index from `&1`";

	MYSQL_RES *res;
	MYSQL_ROW row;
	int i, j, n;

	/* Index primaire */

	if (do_query_cached(db, "Unable to get primary index: &1", &res, "si:&1", qindex, 1, table))
		return TRUE;

	for ( i = 0, n = 0;  i < mysql_num_rows(res); i++ )
	{
		row = mysql_fetch_row(res);
		if (strcmp("PRIMARY", row[2]) == 0) /* Use only Primary key */
			n++;
	}

	mysql_data_seek(res, 0);/* move back to first record */
	info->nindex = n;
	/* Note: Col 3 is Key_name, Col 4 is Sq_in_index, Col 5 is Field Name */

	if (n <= 0)
	{
		GB.Error("Table '&1' has no primary index", table);
		return TRUE;
	}

	GB.Alloc((void **)POINTER(&info->index), sizeof(int) * n);

	for (i = 0; i < n; i++)
	{
		row = mysql_fetch_row(res);
		if (strcmp("PRIMARY", row[2]) == 0) /* Use only Primary key */
		{
				for (j = 0; j < info->nfield; j++)
				{
						if (strcmp(info->field[j].name, row[4]) == 0)
						{
								info->index[i] = j;
								break;
						}
			}
		}
	}

	return FALSE;
}


/*****************************************************************************

	table_release()

	Free the info structure filled by table_init() and/or table_index()

	<handle> is the database handle.
	<info> points at the info structure.

*****************************************************************************/

static void table_release(DB_DATABASE *db, DB_INFO *info)
{
	/* All is done outside the driver */
}


/*****************************************************************************

	table_exist()

	Returns if a table exists

	<handle> is the database handle.
	<table> is the table name.

	This function returns TRUE if the table exists, and FALSE if not.

*****************************************************************************/

static int table_exist(DB_DATABASE *db, const char *table)
{
	MYSQL_RES *res;

	if (do_query_cached(db, "Unable to check table: &1", &res, "st", "show tables", 0))
		return FALSE;

	return !search_result(res, table, NULL);
}


/*****************************************************************************

	table_list()

	Returns an array containing the name of each table in the database

	<handle> is the database handle.
	<tables> points to a variable that will receive the char* array.

	This function returns the number of tables, or -1 if the command has
	failed.

	Be careful: <tables> can be NULL, so that just the count is returned.

*****************************************************************************/

static int table_list(DB_DATABASE *db, char ***tables)
{
	MYSQL_RES *res;
	MYSQL_ROW row;
	int i;
	int rows;

	if (do_query_cached(db, "Unable to get tables", &res, "st", "show tables", 0))
		return -1;

	rows = mysql_num_rows(res);
	GB.NewArray(tables, sizeof(char *), rows);

	for (i = 0; i < rows; i++)
	{
		row = mysql_fetch_row(res);
		(*tables)[i] = GB.NewZeroString(row[0]);
	}

	return rows;
}

/*****************************************************************************

	table_primary_key()

	Returns a string representing the primary key of a table.

	<handle> is the database handle.
	<table> is the table name.
	<key> points to a string that will receive the primary key.

	This function returns TRUE if the command has failed, and FALSE if
	everything was OK.

*****************************************************************************/

static int table_primary_key(DB_DATABASE *db, const char *table, char ***primary)
{
	const char *query = "show index from `&1`";

	MYSQL_RES *res;
	MYSQL_ROW row;
	int i;

	if (do_query_cached(db, "Unable to get primary key: &1", &res, "si:&1", query, 1, table))
		return TRUE;

	GB.NewArray(primary, sizeof(char *), 0);

	for (i = 0; i < mysql_num_rows(res); i++)
	{
		row = mysql_fetch_row(res);
		if (strcmp("PRIMARY", row[2]) == 0)
			*(char **)GB.Add(primary) = GB.NewZeroString(row[4]);
	}

	return FALSE;
}

/*****************************************************************************

	table_is_system()

	Returns if a table is a system table.

	<handle> is the database handle.
	<table> is the table name.

	This function returns TRUE if the table is a system table, and FALSE if
	not.

	Note: In mysql the system tables are stored in a separate database.
	The tables are mysql.columns_priv, mysql.db, mysql.func, mysql.host,
	mysql.tables_priv, mysql.user. This has therefore not been implemented.

*****************************************************************************/

static int database_is_system(DB_DATABASE *db, const char *name);

static int table_is_system(DB_DATABASE *db, const char *table)
{
	return db->flags.system;
}


/*****************************************************************************

	table_type()

	Returns the table type.

	<handle> is the database handle.
	<table> is the table name.

	This function returns a string containing table type or NULL if error.

*****************************************************************************/

static char *table_type(DB_DATABASE *db, const char *table, const char *settype)
{
	static char buffer[16];

	const char *query = "show table status like '&1'";

	const char *update =
		"alter table `&1` type = &2";

	MYSQL_RES *res;
	MYSQL_ROW row;

	if (settype)
	{
		clear_table_cache(db, table);
		if (do_query(db, "Cannot set table type: &1", &res, update, 2, table, settype))
				return NULL;
	}

	if (do_query_cached(db, "Invalid table: &1", &res, "sts:&1", query, 1, table))
		return NULL;

	if (search_result(res, table, &row))
	{
		GB.Error("Unable to check table for: &1", table);
		return NULL;
	}
	
	if (!row[1]) return "VIEW"; // the table is a view

	strcpy(buffer, row[1]);
	return buffer;
}


/*****************************************************************************

	table_delete()

	Deletes a table.

	<handle> is the database handle.
	<table> is the table name.

	This function returns TRUE if the command has failed, and FALSE if
	everything was OK.

*****************************************************************************/

static int table_delete(DB_DATABASE *db, const char *table)
{
	clear_table_cache(db, table);
	remove_cache_entry(db, "st");
	return do_query(db, "Unable to delete table: &1", NULL, "drop table `&1`", 1, table);
}

/*****************************************************************************

	table_create()

	Creates a table.

	<handle> is the database handle.
	<table> is the table name.
	<fields> points to a linked list of field descriptions.
	<key> is the primary key.

	This function returns TRUE if the command has failed, and FALSE if
	everything was OK.

	MySql has several different table types: InnoDB and BDB are transaction safe
	whilst HEAP, ISAM, MERGE and MYISAM are not.

TYPE =

*****************************************************************************/

static int table_create(DB_DATABASE *db, const char *table, DB_FIELD *fields, char **primary, const char *tabletype)
{
	DB_FIELD *fp;
	char *type = NULL;
	int comma;
	int i;

	if (db->version < 40100 && !strcasecmp(tabletype, "MEMORY"))
		tabletype = "HEAP";
	
	DB.Query.Init();
	
	DB.Query.Add("CREATE TABLE `");
	DB.Query.Add(table);
	DB.Query.Add("` ( ");

	comma = FALSE;
	for (fp = fields; fp; fp = fp->next)
	{
		if (comma)
			DB.Query.Add(", ");
		else
			comma = TRUE;

		DB.Query.Add(QUOTE_STRING);
		DB.Query.Add(fp->name);
		DB.Query.Add(QUOTE_STRING);

		if (fp->type == DB_T_SERIAL)
			DB.Query.Add(" BIGINT UNSIGNED NOT NULL AUTO_INCREMENT ");
		else if (fp->type == DB_T_BLOB)
			DB.Query.Add(" LONGBLOB ");
		else
		{
			switch (fp->type)
			{
				case GB_T_BOOLEAN: type = "BOOL"; break;
				case GB_T_INTEGER: type = "INT"; break;
				case GB_T_LONG: type = "BIGINT"; break;
				case GB_T_FLOAT: type = "DOUBLE"; break;
				case GB_T_DATE: type = "DATETIME"; break;
				case GB_T_STRING:

					if (fp->length <= 0 || fp->length > 255) //mysql supports upto 255 as varchar
						type = "TEXT";
					else
					{
						sprintf(_buffer, "VARCHAR(%d)", fp->length);
						type = _buffer;
					}

					break;

				default: type = "TEXT"; break;
			}

			DB.Query.Add(" ");
			DB.Query.Add(type);

			if (fp->collation && *fp->collation)
			{
				char *p = strchr(fp->collation, '_');
				if (!p || p == fp->collation)
				{
					GB.Error("Incorrect collation");
					return TRUE;
				}

				DB.Query.Add(" CHARACTER SET ");
				DB.Query.AddLength(fp->collation, p - fp->collation);
				DB.Query.Add(" COLLATE ");
				DB.Query.Add(fp->collation);
			}

			if (fp->def.type != GB_T_NULL)
			{
				DB.Query.Add(" NOT NULL DEFAULT");
				DB.FormatVariant(&_driver, &fp->def, DB.Query.AddLength);
			}
			else if (DB.StringArray.Find(primary, fp->name) >= 0)
			{
				DB.Query.Add(" NOT NULL");
			}
		}
	}

	if (primary)
	{
		DB.Query.Add(", PRIMARY KEY (");

		for (i = 0; i < GB.Count(primary); i++)
		{
			if (i > 0)
				DB.Query.Add(",");

			DB.Query.Add("`");
			DB.Query.Add(primary[i]);
			DB.Query.Add("`");

			for (fp = fields; fp; fp = fp->next) //Check type of primary field
			{
							if(strcmp(fp->name, primary[i]) == 0){
								if (fp->length <= 0 || fp->length > 255){
												if (fp->type == GB_T_STRING)
													DB.Query.Add("(255)");
								}
							}
			}
		}

		DB.Query.Add(")");
	}

	DB.Query.Add(" )");

	if (tabletype)
	{
		if (db->version < 40018)
			DB.Query.Add(" TYPE = ");
		else
			DB.Query.Add(" ENGINE = ");
		
		DB.Query.Add(tabletype);
	}

	remove_cache_entry(db, "st");
	/* printf("table_create syntax: %s\n", DB.Query.Get());*/
	return do_query(db, "Cannot create table: &1", NULL, DB.Query.Get(), 0);
}

/*****************************************************************************

	field_exist()

	Returns if a field exists in a given table

	<handle> is the database handle.
	<table> is the table name.
	<field> is the field name.

	This function returns TRUE if the field exists, and FALSE if not.

*****************************************************************************/

static int field_exist(DB_DATABASE *db, const char *table, const char *field)
{
	const char *query = "show full columns from `&1`";

	MYSQL_RES *res;

	if (do_query_cached(db, "Unable to check field: &1", &res, "sfc:&1", query, 1, table))
		return FALSE;

	return !search_result(res, field, NULL);
}



/*****************************************************************************

	field_list()

	Returns an array containing the name of each field in a given table

	<handle> is the database handle.
	<table> is the table name.
	<fields> points to a variable that will receive the char* array.

	This function returns the number of fields, or -1 if the command has
	failed.

	Be careful: <fields> can be NULL, so that just the count is returned.

*****************************************************************************/

static int field_list(DB_DATABASE *db, const char *table, char ***fields)
{
	const char *query = "show full columns from `&1`";

	long i, n;
	MYSQL_RES *res;
	MYSQL_ROW row;

	if (do_query_cached(db, "Unable to get field: &1", &res, "sfc:&1", query, 1, table))
		return -1;

	n = mysql_num_rows(res);

	if (fields) /* (BM) see the function commentary */
	{
		GB.NewArray(fields, sizeof(char *), n);

		for (i = 0; i < n; i++){
			row = mysql_fetch_row(res);
			(*fields)[i] = GB.NewZeroString(row[0]);
		}
	}

	return n;
}


/*****************************************************************************

	field_info()

	Get field description

	<handle> is the database handle.
	<table> is the table name.
	<field> is the field name.
	<info> points to a structure filled by the function.

	This function returns TRUE if the command has failed, and FALSE if
	everything was OK.

*****************************************************************************/

static int field_info(DB_DATABASE *db, const char *table, const char *field, DB_FIELD *info)
{
	const char *query = "show full columns from `&1`";

	MYSQL_RES *res;
	MYSQL_ROW row;
	GB_VARIANT def;
	char *val;
	int type;
	long len = 0;

	if (do_query_cached(db, "Unable to get field info: &1", &res, "sfc:&1", query, 1, table))
		return TRUE;

	if (search_result(res, field, &row))
	{
		GB.Error("Unable to find field &2 in table &1", table, field);
		return TRUE;
	}

	info->name = NULL;
	type = conv_string_type(row[1], &len);

	info->type = conv_type(type, len);
	if (info->type == GB_T_STRING)
	{
		info->length = len;
		/* (BM) That's new! a TEXT field has a length of 65535 */
		if (info->length >= 65535)
			info->length = 0;
	}
	else
		info->length = 0;

	info->def.type = GB_T_NULL;

	if ((info->type == GB_T_INTEGER || info->type == GB_T_LONG) && strstr(row[6], "auto_increment"))
		info->type = DB_T_SERIAL;
	else
	{
		if (!*row[3] || row[3][0] != 'Y')
		{
			def.type = GB_T_VARIANT;
			def.value.type = GB_T_NULL;

			val = row[5];

			/* (BM) seems there is a bug in mysql */
			if (info->type == GB_T_DATE && val && strlen(val) >= 5 && strncmp(val, "00000", 5) == 0)
				val = NULL;

			if (val && *val)
			{
				conv_data(db->version, val, 0, &def.value, type, len);
				GB.StoreVariant(&def, &info->def);
			}
		}
	}

	if (row[2] && *row[2])
		info->collation = GB.NewZeroString(row[2]);
	else
		info->collation = NULL;

	return FALSE;
}

/*****************************************************************************

	index_exist()

	Returns if an index exists in a given table

	<handle> is the database handle.
	<table> is the table name.
	<field> is the index name.

	This function returns TRUE if the index exists, and FALSE if not.

*****************************************************************************/

static int index_exist(DB_DATABASE *db, const char *table, const char *index)
{
	const char *query = "show index from `&1`";

	MYSQL_RES *res;
	MYSQL_ROW row;
	int i, n;

	if (do_query_cached(db, "Unable to check index: &1", &res, "si:&1", query, 1, table))
		return FALSE;

	for ( i = 0, n = 0; i < mysql_num_rows(res); i++ )
	{
		row = mysql_fetch_row(res);
		if (strcmp(index, row[2]) == 0)
				n++;
	}

	return (n > 0);
}

/*****************************************************************************

	index_list()

	Returns an array containing the name of each index in a given table

	<handle> is the database handle.
	<table> is the table name.
	<indexes> points to a variable that will receive the char* array.

	This function returns the number of indexes, or -1 if the command has
	failed.

	Be careful: <indexes> can be NULL, so that just the count is returned.

*****************************************************************************/

static int index_list(DB_DATABASE *db, const char *table, char ***indexes)
{
	const char *query = "show index from `&1`";

	MYSQL_RES *res;
	MYSQL_ROW row;
	long i, n, no_indexes;

	if (do_query_cached(db, "Unable to get indexes: &1", &res, "si:&1", query, 1, table))
		return -1;

	for (i = 0, no_indexes = 0; i < mysql_num_rows(res); i++ )
	{
		/* Count the number of 1st sequences in Seq_in_index to
			give nmber of indexes. row[3] */
		row = mysql_fetch_row(res);
		if (atoi(row[3]) == 1)
			no_indexes++;
	}


	GB.NewArray(indexes, sizeof(char *), no_indexes);
	mysql_data_seek(res, 0); /* move back to first record */

	for (i = 0, n = 0; i < mysql_num_rows(res); i++ )
	{
		row = mysql_fetch_row(res);
		if (atoi(row[3]) == 1 /* Start of a new index */)
			(*indexes)[n++] = GB.NewZeroString(row[2]); /* (BM) The name is row[2], not row[4] */
	}

	return no_indexes;
}

/*****************************************************************************

	index_info()

	Get index description

	<handle> is the database handle.
	<table> is the table name.
	<field> is the index name.
	<info> points to a structure filled by the function.

	This function returns TRUE if the command has failed, and FALSE if
	everything was OK.

*****************************************************************************/

static int index_info(DB_DATABASE *db, const char *table, const char *index, DB_INDEX *info)
{
	const char *query = "show index from `&1`";

	MYSQL_RES *res;
	MYSQL_ROW row = 0;
	int i, n;

	if (do_query_cached(db, "Unable to get index info: &1", &res, "si:&1", query, 1, table))
		return TRUE;

	n = mysql_num_rows(res);
	for (i = 0; i < n; i++ )
	{
		row = mysql_fetch_row(res);
		if ( strcmp( index, row[2]) == 0)
		{ /* (BM) With braces, it should work better :-) */
				n = 1;
				break;
		}
	}

	if (n != 1)
	{
		GB.Error("Unable to find index &2 in table &1", table, index);
		return TRUE;
	}

	info->name = NULL;
	info->unique = strcmp(row[1], "0") == 0;
	info->primary = strcmp("PRIMARY", row[2]) == 0 ? TRUE : FALSE;

	DB.Query.Init();

	i = 0;
	/* (BM) row can be null if we are seeking the last index */
	while ( row && strcmp(index, row[2]) == 0 )
	{
		if (i > 0)
			DB.Query.Add(",");

		DB.Query.Add(row[4]);
		row = mysql_fetch_row(res);
		i++; /* (BM) i must be incremented */
	}

	info->fields = DB.Query.GetNew();

	return FALSE;
}

/*****************************************************************************

	index_delete()

	Deletes an index.

	<handle> is the database handle.
	<table> is the table name.
	<index> is the index name.

	This function returns TRUE if the command has failed, and FALSE if
	everything was OK.

*****************************************************************************/

static int index_delete(DB_DATABASE *db, const char *table, const char *index)
{
	clear_table_cache(db, table);
	return do_query(db, "Unable to delete index: &1", NULL, "drop index `&1` on `&2`", 1, index, table);
}

/*****************************************************************************

	index_create()

	Creates an index.

	<handle> is the database handle.
	<table> is the table name.
	<index> is the index name.
	<info> points to a structure describing the index.

	This function returns TRUE if the command has failed, and FALSE if
	everything was OK.

*****************************************************************************/

static int index_create(DB_DATABASE *db, const char *table, const char *index, DB_INDEX *info)
{
	DB.Query.Init();

	DB.Query.Add("CREATE ");
	if (info->unique)
		DB.Query.Add("UNIQUE ");
	DB.Query.Add("INDEX `");
	DB.Query.Add(index);
	DB.Query.Add("` ON ");
	DB.Query.Add(table);
	DB.Query.Add(" ( ");
	DB.Query.Add(info->fields);
	DB.Query.Add(" )");

	clear_table_cache(db, table);
	return do_query(db, "Cannot create index: &1", NULL, DB.Query.Get(), 0);
}

/*****************************************************************************
*
*   database_exist()
*
*   Returns if a database exists
*
*   <handle> is any database handle.
*   <name> is the database name.
*
*   This function returns TRUE if the database exists, and FALSE if not.
*
******************************************************************************/

static int database_exist(DB_DATABASE *db, const char *name)
{
	MYSQL *conn = (MYSQL *)db->handle;
	MYSQL_RES *res;
	int exist;

	check_connection(conn);
	res = mysql_list_dbs(conn, name);
	if (!res)
	{
		db->error = mysql_errno(conn);	
		GB.Error("Unable to check database: &1", mysql_error(conn));
		return FALSE;
	}

	exist = mysql_num_rows(res);
	mysql_free_result(res);
	return exist;
}

/*****************************************************************************
*
*   database_list()
*
*   Returns an array containing the name of each database
*
*   <handle> is any database handle.
*   <databases> points to a variable that will receive the char* array.
*
*   This function returns the number of databases, or -1 if the command has
*   failed.
*
*   Be careful: <databases> can be NULL, so that just the count is returned.
*
******************************************************************************/

static int database_list(DB_DATABASE *db, char ***databases)
{
	long i, rows;
	MYSQL *conn = (MYSQL *)db->handle;
	MYSQL_RES *res;
	MYSQL_ROW row;

	check_connection(conn);
	res = mysql_list_dbs(conn, 0);
	if (!res){
		db->error = mysql_errno(conn);
		GB.Error("Unable to get databases: &1", mysql_error(conn));
		return -1;
	}

	rows = mysql_num_rows(res);
	/*printf("Got %d databases\n", rows); */
	GB.NewArray(databases, sizeof(char *), rows);

	for (i = 0; i < rows; i++){
		row = mysql_fetch_row(res);
		(*databases)[i] = GB.NewZeroString(row[0]);
		/*printf("%s\n", (*databases)[i]); */
	}

	mysql_free_result(res);

	return rows;
}

/*****************************************************************************
*
*   database_is_system()
*
*   Returns if a database is a system database.
*
*   <handle> is any database handle.
*   <name> is the database name.
*
*   This function returns TRUE if the database is a system database, and
*   FALSE if not.
*
******************************************************************************/

static int database_is_system(DB_DATABASE *db, const char *name)
{
	return (strcmp("mysql", name) == 0 || strcmp("information_schema", name) == 0);
}

/*****************************************************************************
*
*   database_delete()
*
*   Deletes a database.
*
*   <handle> is the database handle.
*   <name> is the database name.
*
*   This function returns TRUE if the command has failed, and FALSE if
*   everything was OK.
*
******************************************************************************/

static int database_delete(DB_DATABASE *db, const char *name)
{
	if (database_is_system(db, name))
	{
		GB.Error("Unable to delete database: &1", "system database");
		return TRUE;
	}

	return do_query(db, "Unable to delete database: &1", NULL, "drop database `&1`", 1, name);
}

/*****************************************************************************
*
*   database_create()
*
*   Creates a database.
*
*   <handle> is the database handle.
*   <name> is the database name.
*
*   This function returns TRUE if the command has failed, and FALSE if
*   everything was OK.
*
******************************************************************************/

static int database_create(DB_DATABASE *db, const char *name)
{
	return do_query(db, "Unable to create database: &1", NULL, "create database `&1`", 1, name);
}


/*****************************************************************************
*
*  user_exist()
*
*  Returns if a user exists.
*
*  <handle> is any database handle.
*  <name> is the user name.
*
*  This function returns TRUE if the user exists, and FALSE if not.
*
******************************************************************************/

static int user_exist(DB_DATABASE *db, const char *name)
{
	const char *query = "select user from mysql.user "
			"where user = '&1' and host = '&2' ";
	MYSQL_RES *res;
	int exist;
	char *_name, *_host, *_token;

	if (!strrchr(name,'@')){
		//To be done: maybe we should  check hostname we are running
		//from and use this instead of localhost
		/* (BM) you forgot the last 0 character */
		_name = malloc(strlen(name) + strlen("@localhost") + 1);
		sprintf(_name,"%s@localhost", name);
	}
	else {
		_name = malloc(strlen(name) + 1);
		strcpy(_name,name);
	}

	_host = strrchr(_name,'@') + 1;
	_token = _host - 1;
	_token[0] = 0;


	if (do_query(db, "Unable to check user: &1@&2", &res, query, 2, _name, _host))
	{
		free(_name);
		return FALSE;
	}

	exist = mysql_num_rows(res) == 1;

	free(_name);
	mysql_free_result(res);

	return exist;
}

/*****************************************************************************
*
*   user_list()
*
*   Returns an array containing the name of each user.
*
*   <handle> is the database handle.
*   <users> points to a variable that will receive the char* array.
*
*   This function returns the number of users, or -1 if the command has
*   failed.
*
*   Be careful: <users> can be NULL, so that just the count is returned.
*
******************************************************************************/

static int user_list(DB_DATABASE *db, char ***users)
{
	const char *query = "select user, host from mysql.user";

	MYSQL_RES *res;
	MYSQL_ROW row;
	MYSQL_FIELD *field;
	long i, count, length;
	char *_username;

	if (do_query(db, "Unable to get users: &1", &res, query, 0))
		return -1;

	count = mysql_num_rows(res);

	if (users)
	{
		GB.NewArray(users, sizeof(char *), count);
		field = mysql_fetch_field(res); //user field
		length = field->max_length;
		field = mysql_fetch_field(res); //host field
		length += field->max_length;
		_username = malloc(length + 2); /* (BM) +2 because there is the last 0 character ! */

		for ( i = 0; i < count; i++ )
		{
			row = mysql_fetch_row(res);
			sprintf(_username,"%s@%s", row[0], row[1]);
			(*users)[i] = GB.NewZeroString(_username);
		}
		free(_username);
	}

	mysql_free_result(res);

	return count;
}

/*****************************************************************************
*
*   user_info()
*
*   Get user description
*
*   <handle> is the database handle.
*   <name> is the user name.
*   <info> points to a structure filled by the function.
*
*   This function returns TRUE if the command has failed, and FALSE if
*   everything was OK.
*
* Mysql notes: Privileges set to Y in the mysql.user table are global settings
*              and apply to all databases. eg. These are super user privileges.
*              mysql.tables_priv lists the access granted to a user on a
*                   particular table
*              mysql.columns_priv grant column specific privileges.
*              mysql.db and mysql.host grant database specific privileges.
*
*              User may also not be unique as mysql.user also contains
*              users from different hosts.  e.g host and user columns will
*              make it unique! Using 'localhost' here to limit.
*
*              The privileges are: grant_priv - allows user to grant
*                                          privileges to others - which
*                                          includes the ability to create
*                                          users;
*                                  create_priv/drop_priv - create database,
*                                     tables etc.
******************************************************************************/

static int user_info(DB_DATABASE *db, const char *name, DB_USER *info )
{
	const char *query =
		"select create_priv, drop_priv, grant_priv, password from mysql.user "
		"where user = '&1' and host = '&2'";

	MYSQL_RES *res;
	MYSQL_ROW row;
	char *_name, *_host, *_token;

	if (!strrchr(name,'@')){
		//To be done: check hostname we are running
		//from use instead of localhost
		/* (BM) You forgot the last 0 character */
		_name = malloc(strlen(name) + strlen("@localhost") + 1);
		sprintf(_name,"%s@localhost", name);
	}
	else {
		_name = malloc(strlen(name) + 1);
		strcpy(_name,name);
	}

	_host = strrchr(_name,'@') + 1;
	_token = _host - 1;
	_token[0] = 0;

	if (do_query(db, "Unable to check user info: &1@&2", &res, query, 2, _name, _host))
	{
		free(_name);
		return TRUE;
	}

	if (mysql_num_rows(res) != 1)
	{
		GB.Error("user_info: Non unique user found");
		free(_name);
					mysql_free_result(res);
		return TRUE;
	}

	row = mysql_fetch_row(res);

	info->name = NULL;
	if ( strcmp(row[0], "Y") == 0 || strcmp(row[1], "Y") == 0)
		info->admin = 1;
	else
		info->admin = 0;

	if (row[3])
		info->password = GB.NewZeroString(row[3]); //password is encrypted in mysql

	mysql_free_result(res);
	free(_name);

	return FALSE;
}

/*****************************************************************************

	user_delete()

	Deletes a user.

	<handle> is any database handle.
	<name> is the user name.

	This function returns TRUE if the command has failed, and FALSE if
	everything was OK.

*****************************************************************************/

static int user_delete(DB_DATABASE *db, const char *name)
{
	const char *_delete =
		"delete from mysql.user where user = '&1' and host = '&2'";
//   "delete from mysql.user, mysql.db, mysql.columns_priv, mysql.tables_priv "
//   "where user = '&1' and host = '&2'";
	char *_name, *_host, *_token;
	int _ret;

	if (!strrchr(name,'@')){
		//To be done: maybe hostname we are running
		//from should be used rather than localhost
		_name = malloc(strlen(name) + strlen("@localhost")) + 1;
		sprintf(_name,"%s@localhost", name);
	}
	else {
		_name = malloc(strlen(name) + 1);
		strcpy(_name,name);
	}

	_host = strrchr(_name,'@') + 1;
	_token = _host - 1;
	_token[0] = 0;

//Still need to look at the removal of privileges
// _ret =  do_query(db, "Unable to delete user: &1", NULL,
//	           "revoke all on *.* from &1@&2", 2, _name, _host);
	_ret =  do_query(db, "Unable to delete user: &1", NULL, _delete, 2, _name, _host);
	free(_name);
	return _ret;
}

/*****************************************************************************
*
*   user_create()
*
*     Creates a user.
*
*     <handle> is the database handle.
*     <name> is the user name.
*     <info> points to a structure describing the user.
*
*     This function returns TRUE if the command has failed, and FALSE if
*           everything was OK.
*
******************************************************************************/

static int user_create(DB_DATABASE *db, const char *name, DB_USER *info)
{
	char *_name;

	DB.Query.Init();

	if (!strrchr(name,'@')){
		_name = malloc(strlen(name) + strlen("@localhost") + 1);
		sprintf(_name,"%s@localhost", name);
	}
	else {
		_name = malloc(strlen(name) + 1);
		strcpy(_name,name);
	}

	if (info->admin) {
		DB.Query.Add("GRANT ALL PRIVILEGES ON *.* TO ");
		DB.Query.Add(_name);
	}
	else {
		DB.Query.Add("GRANT USAGE ON * TO ");
		DB.Query.Add(_name);
	}

	if (info->password)
	{
		DB.Query.Add(" IDENTIFIED BY '");
		DB.Query.Add(info->password);
		DB.Query.Add("'");
	}

	if (info->admin)
		DB.Query.Add(" WITH GRANT OPTION");

	free(_name);

	return do_query(db, "Cannot create user: &1", NULL, DB.Query.Get(), 0);
}

/*****************************************************************************
*
*   user_set_password()
*
*   Change the user password.
*
*   <handle> is the database handle.
*   <name> is the user name.
*   <password> is the new password
*
*   This function returns TRUE if the command has failed, and FALSE if
*   everything was OK.
*
******************************************************************************/

static int user_set_password(DB_DATABASE *db, const char *name, const char *password)
{
	char *_name;
	DB.Query.Init();

	if (!strrchr(name,'@')){
		_name = malloc(strlen(name) + strlen("@localhost") + 1);
		sprintf(_name,"%s@localhost", name);
	}
	else {
		_name = malloc(strlen(name) + 1);
		strcpy(_name,name);
	}

	DB.Query.Add("SET PASSWORD FOR ");
	DB.Query.Add(_name);
	DB.Query.Add(" = PASSWORD ('");
	DB.Query.Add(password);
	DB.Query.Add("')");

	free(_name);

	return do_query(db, "Cannot change user password: &1", NULL, DB.Query.Get(), 0);
}


/*****************************************************************************

	The driver interface

*****************************************************************************/

DECLARE_DRIVER(_driver, "mysql");

/*****************************************************************************

	The component entry and exit functions.

*****************************************************************************/

int EXPORT GB_INIT(void)
{
	GB.GetInterface("gb.db", DB_INTERFACE_VERSION, &DB);
	DB.Register(&_driver);

	return 0;
}

void EXPORT GB_EXIT()
{
}
