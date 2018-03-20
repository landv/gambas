/***************************************************************************

  main.c

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

#define __MAIN_C

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <libgen.h>							/* For basename function */
#include <pwd.h>								/* For passwd file functions */
#include <grp.h>								/* For group file functions */
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <errno.h>

#include "gb.db.proto.h"
#include "helper.h"
#include "main.h"

GB_INTERFACE GB EXPORT;
DB_INTERFACE DB EXPORT;

static DB_DRIVER _driver;

static char _buffer[32];

static int _print_query = FALSE;
static bool _need_field_type = FALSE;
static char *_table_schema = NULL;


//--------------------------------------------------------------------------

/* Internal function to convert a database value into a Gambas variant value */

static void conv_data(const char *data, GB_VARIANT_VALUE * val, int type)
{
	GB_VALUE conv;
	GB_DATE_SERIAL date;
	double sec;

	switch (type)
	{
		case GB_T_BOOLEAN:

			val->type = GB_T_BOOLEAN;
			if (data[0] == 't' || data[0] == 'T')
				val->value._boolean = -1;
			else
				val->value._boolean = atoi(data) ? -1 : 0;
			break;

		case GB_T_INTEGER:

			GB.NumberFromString(GB_NB_READ_INTEGER, data, strlen(data), &conv);

			val->type = GB_T_INTEGER;
			val->value._integer = conv._integer.value;

			break;

		case GB_T_FLOAT:

			GB.NumberFromString(GB_NB_READ_FLOAT, data, strlen(data), &conv);

			val->type = GB_T_FLOAT;
			val->value._float = conv._float.value;

			break;

		case GB_T_LONG:

			GB.NumberFromString(GB_NB_READ_LONG, data, strlen(data), &conv);

			val->type = GB_T_LONG;
			val->value._long = conv._long.value;

			break;

		case GB_T_DATE:

			memset(&date, 0, sizeof(date));

			switch (strlen(data))
			{
				case 14:
					sscanf(data, "%4d%2d%2d%2d%2d%lf", &date.year, &date.month,
								 &date.day, &date.hour, &date.min, &sec);
					date.sec = (short) sec;
					date.msec = (short) ((sec - date.sec) * 1000 + 0.5);
					break;
				case 12:
					sscanf(data, "%2d%2d%2d%2d%2d%lf", &date.year, &date.month,
								 &date.day, &date.hour, &date.min, &sec);
					date.sec = (short) sec;
					date.msec = (short) ((sec - date.sec) * 1000 + 0.5);
					break;
				case 10:
					if (sscanf(data, "%4d-%2d-%2d", &date.year, &date.month,
										 &date.day) != 3)
					{
						if (sscanf(data, "%4d/%2d/%2d", &date.year, &date.month,
											 &date.day) != 3)
						{
							if (sscanf(data, "%4d:%2d:%lf", &date.hour, &date.min,
												 &sec) == 3)
							{
								date.sec = (short) sec;
								date.msec = (short) ((sec - date.sec) * 1000 + 0.5);
							}
							else
							{
								sscanf(data, "%2d%2d%2d%2d%2d", &date.year,
											 &date.month, &date.day, &date.hour, &date.min);
							}
						}
					}

					break;
				case 8:
					if (sscanf(data, "%4d%2d%2d", &date.year, &date.month,
										 &date.day) != 3)
					{
						sscanf(data, "%2d/%2d/%2d", &date.year, &date.month,
									 &date.day);
					}
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
				default:
					sscanf(data, "%4d-%2d-%2d %2d:%2d:%lf", &date.year,
								 &date.month, &date.day, &date.hour, &date.min, &sec);
					date.sec = (short) sec;
					date.msec = (short) ((sec - date.sec) * 1000 + 0.5);
			}
			if (date.year < 100)
				date.year += 1900;

			GB.MakeDate(&date, (GB_DATE *) & conv);

			val->type = GB_T_DATE;
			val->value._date.date = conv._date.value.date;
			val->value._date.time = conv._date.value.time;

			break;

		case (int)DB_T_BLOB:
			// Blob fields are read by blob_read()
			val->type = GB_T_NULL;
			break;

		default:

			val->type = GB_T_CSTRING;
			val->value._string = (char *)data;
	}
}

/* internal function to quote a value stored as a blob */

static void quote_blob(const char *data, int len, DB_FORMAT_CALLBACK add)
{
	int i;
	unsigned char c;
	char buffer[2];
	static const char *hexa_digit = "0123456789ABCDEF";

	if (len == 0)
	{
		(*add) ("NULL", 4);
		return;
	}

	(*add) ("X'", 2);
	for (i = 0; i < len; i++)
	{
		c = (unsigned char) data[i];
		buffer[0] = hexa_digit[c >> 4];
		buffer[1] = hexa_digit[c & 15];

		(*add) (buffer, 2);
	}
	(*add) ("'", 1);
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

	if (quote == '\'')
	{
		*str = DB.QuoteString(*str, *len, quote);
		*len = GB.StringLength(*str);
	}
}

/* Internal function to run a query */

static int do_query(DB_DATABASE *db, const char *error, SQLITE_RESULT **pres, const char *qtemp, int nsubst, ...)
{
	SQLITE_DATABASE *conn = (SQLITE_DATABASE *)db->handle;
	SQLITE_RESULT *res;
	va_list args;
	int i;
	const char *query;
	int err;
	int retry = 0;
	int max_retry;

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

	if (_print_query)
	{
		_print_query = FALSE;
	}

	if (DB.IsDebug())
		fprintf(stderr, "gb.db.sqlite3: %p: %s\n", conn, query);

	if (db->timeout > 0)
		max_retry = db->timeout * 5;
	else if (db->timeout == 0)
		max_retry = 600; // 120 s max
	else
		max_retry = 0;

	for(;;)
	{
		err = 0;

		res = sqlite_query_exec(conn, query, _need_field_type);

		if (res)
		{
			if (pres)
				*pres = res;
			else
				sqlite_query_free(res);
			break;
		}

		err = conn->error;

		if (err != SQLITE_BUSY || retry >= max_retry)
		{
			GB.Error(error, sqlite_get_error_message(conn));
			break;
		}

		retry++;
		usleep(200000);
	}

 	db->error = err;
	_need_field_type = FALSE;
	return err != 0;
}

/* Internal function to check whether a file is a sqlite database file */

static bool is_sqlite2_database(const char *filename)
{
  /*                   SQLite databases start with the string:
   *                  ** This file contains an SQLite 2.1 database **
   *                                  */
  FILE* fp;
  bool res;
  char magic_text[48];

  fp = fopen(filename, "r");
  if (!fp)
    return FALSE;

  res = fread(magic_text, 1, 47, fp) == 47;
  fclose(fp);

  if (!res)
    return FALSE;

  magic_text[47] = '\0';

  if (strcmp(magic_text, "** This file contains an SQLite 2.1 database **"))
    return FALSE;

  return TRUE;
}

static bool is_sqlite3_database(const char *filename)
{
	FILE *fp;
	bool res;
	char magic_text[16];

	fp = fopen(filename, "r");
	if (!fp)
		return FALSE;

	res = fread(magic_text, 1, 15, fp) == 15;
	fclose(fp);

	if (!res)
		return FALSE;

	magic_text[15] = '\0';

	if (strcmp(magic_text, "SQLite format 3"))
		return FALSE;

	return TRUE;
}

static bool is_database_file(const char *filename)
{
	return is_sqlite3_database(filename) || is_sqlite2_database(filename);
}


/* Internal function to locate database and return full qualified */
/* path. */
static char *find_database(const char *name, const char *hostName)
{
	char *dbhome = NULL;
	char *fullpath = NULL;

	/* Does Name includes fullpath */
	if (*name == '/')
	{
		if (is_database_file(name))
			fullpath = GB.NewZeroString(name);

		return fullpath;
	}

	/* Hostname contains home area */
	fullpath = GB.NewZeroString(hostName);
	fullpath = GB.AddChar(fullpath, '/');
	fullpath = GB.AddString(fullpath, name, 0);
	if (is_database_file(fullpath))
	{
		return fullpath;
	}
	GB.FreeString(&fullpath);

	/* Check the GAMBAS_SQLITE_DBHOME setting */
	dbhome = getenv("GAMBAS_SQLITE_DBHOME");

	if (dbhome != NULL)
	{
		fullpath = GB.NewZeroString(dbhome);
		fullpath = GB.AddChar(fullpath, '/');
		fullpath = GB.AddString(fullpath, name, 0);

		if (is_database_file(fullpath))
			return fullpath;

		GB.FreeString(&fullpath);
	}

	fullpath = GB.NewZeroString(GB.TempDir());
	fullpath = GB.AddString(fullpath, "/sqlite/", 0);
	fullpath = GB.AddString(fullpath, name, 0);

	if (is_database_file(fullpath))
		return fullpath;

	GB.FreeString(&fullpath);
	return NULL;
}

/* Internal function to return database home directory */
/* - GAMBAS_SQLITE_HOMEDB if set or temporary directory /tmp/gambas.%pid%/ */

static char *get_database_home()
{
	char *env = NULL;
	char *dbhome = NULL;

	GB.Alloc(POINTER(&dbhome), PATH_MAX);

	/* Check for Environment variable */

	env = getenv("GAMBAS_SQLITE_DBHOME");

	/* if not set then set to current working directory */
	if (env == NULL)
	{
		/*
		   if (getcwd(dbhome, PATH_MAX) == NULL){
		   GB.Error("Unable to get databases: &1", "Can't find current directory");
		   GB.Free((void **)&dbhome);
		   return NULL;
		   }
		 */

		sprintf(dbhome, "%s/sqlite", GB.TempDir());
	}
	else
	{
		strcpy(dbhome, env);
	}

	return dbhome;
}


// Internal function to walk a directory and list files
// Used by database_list

static int walk_directory(const char *dir, char ***databases)
{
	DIR *dp;
	struct dirent *entry;
	struct stat statbuf;
	char cwd[PATH_MAX];

	if ((dp = opendir(dir)) == NULL)
		return -1;

	if (getcwd(cwd, PATH_MAX) == NULL)
	{
		fprintf(stderr, "gb.db.sqlite3: warning: getcwd: %s\n", strerror(errno));
		return -1;
	}

	if (chdir(dir))
	{
		fprintf(stderr, "gb.db.sqlite3: warning: chdir: %s\n", strerror(errno));
		return -1;
	}

	while ((entry = readdir(dp)) != NULL)
	{
		stat(entry->d_name, &statbuf);

		if (S_ISREG(statbuf.st_mode))
		{
			if (is_database_file(entry->d_name))
				*(char **)GB.Add(databases) = GB.NewZeroString(entry->d_name);
		}
	}

	// BM: you must call closedir()
	closedir(dp);

	if (chdir(cwd))
		fprintf(stderr, "gb.db.sqlite3: warning: chdir: %s\n", strerror(errno));

	return GB.Count(databases);
}


/* Internal function to check database version number */
static int db_version()
{
	//Check db version
	int dbversion = 0;
	unsigned int verMain, verMajor, verMinor;

	sscanf(sqlite3_libversion(), "%2u.%2u.%2u", &verMain, &verMajor, &verMinor);
	dbversion = ((verMain * 10000) + (verMajor * 100) + verMinor);
	return dbversion;
}

/* Get the schema of a table */
static char *get_table_schema(DB_DATABASE *db, const char *table)
{
	char *schema = NULL;
	SQLITE_RESULT *res;

	if (!do_query(db, "Unable to get table schema: &1", &res, "select sql from sqlite_master where type = 'table' and tbl_name = '&1'", 1, table))
	{
		schema = GB.NewZeroString(sqlite_query_get_string(res, 0, 0));
		sqlite_query_free(res);
	}

	return schema;
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

  In Sqlite, there is no such thing as a host.  If this is set then check
  to see whether this is actually a path to a home area. NG 01/04/04

  This function must return a database handle, or NULL if the connection
  has failed.

*****************************************************************************/

static int open_database(DB_DESC *desc, DB_DATABASE *db)
{
	SQLITE_DATABASE *conn;
	char *path;
	char *host;

	host = desc->host;
	if (!host)
		host = "";

	if (desc->name)
	{
		path = find_database(desc->name, host);
		if (!path)
		{
			GB.Error("Unable to locate database `&1` in `&2`", desc->name, host);
			return TRUE;
		}
	}
	else
		path = NULL;

	if (path)
	{
		if (is_sqlite2_database(path))
		{
			DB.TryAnother("sqlite2");
			GB.FreeString(&path);
			return TRUE;
		}
	}

	conn = sqlite_open_database(path, host);
	GB.FreeString(&path);

	if (!conn)
	{
		GB.Error("Cannot open database: &1", sqlite_get_error_message(NULL));
		return TRUE;
	}

	db->handle = conn;
	/* set dbversion */
	db->version = db_version();

	if (do_query(db, "Unable to initialize connection: &1", NULL, "PRAGMA empty_result_callbacks = ON", 0))
		goto CANNOT_OPEN;

	if (db->version < 30803)
	{
		/* NG 29/12/2005 - 3.2.1 introduced a problem with columns names
		* which is resolved by setting short columns off first */

		if (do_query(db, "Unable to initialize connection: &1", NULL, "PRAGMA short_column_names = OFF", 0))
			goto CANNOT_OPEN;
	}

	if (do_query(db, "Unable to initialize connection: &1", NULL, "PRAGMA full_column_names = ON", 0))
		goto CANNOT_OPEN;

	/* Character set cannot be set for sqlite. A sqlite re-compile
	 * is required.                                             */
	db->charset = GB.NewZeroString("UTF-8");

	/* flags */
	db->flags.no_table_type = TRUE;
	db->flags.no_nest = TRUE;

	db->db_name_char = ".";

	return FALSE;

CANNOT_OPEN:

	sqlite_close_database(conn);
	return TRUE;
}


/*****************************************************************************

  close_database()

  Terminates the database connection.

  <handle> contains the database handle.

*****************************************************************************/

static void close_database(DB_DATABASE * db)
{
	sqlite_close_database((SQLITE_DATABASE *)db->handle);
}


/*****************************************************************************

	get_collations()

	Return the available collations as a Gambas string array.

*****************************************************************************/

static GB_ARRAY get_collations(DB_DATABASE *db)
{
	static const char *const collations[] = { "BINARY", "NOCASE", "RTRIM" };
	GB_ARRAY array;
	int i;

	GB.Array.New(&array, GB_T_STRING, 3);
	for (i = 0; i < 3; i++)
		*((char **)GB.Array.Get(array, i)) = GB.NewZeroString(collations[i]);

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

static int format_value(GB_VALUE * arg, DB_FORMAT_CALLBACK add)
{
	char *s;
	int i, l;
	GB_DATE_SERIAL *date;

	switch (arg->type)
	{
		case GB_T_BOOLEAN:
/*Note this is likely to go to a tinyint  */
			if (VALUE((GB_BOOLEAN *) arg))
				add("'1'", 3);
			else
				add("'0'", 3);
			return TRUE;

		case GB_T_STRING:
		case GB_T_CSTRING:

			s = VALUE((GB_STRING *)arg).addr + VALUE((GB_STRING *)arg).start;
			l = VALUE((GB_STRING *)arg).len;

			add("'", 1);

			for (i = 0; i < l; i++, s++)
			{
				add(s, 1);
				if (*s == '\'')
					add(s, 1);
			}

			add("'", 1);
			
			return TRUE;

		case GB_T_DATE:

			date = GB.SplitDate((GB_DATE *) arg);

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

static void format_blob(DB_BLOB * blob, DB_FORMAT_CALLBACK add)
{
	quote_blob(blob->data, blob->length, add);
}


/*****************************************************************************

  exec_query()

  Send a query to the server and gets the result.

  <handle> is the database handle, as returned by open_database()
  <query> is the query string.
  <result> will receive the result handle of the query.
  <err> is an error message used when the query failed.

  <result> can be NULL, when we don't care getting the result.

*****************************************************************************/

static int exec_query(DB_DATABASE *db, const char *query, DB_RESULT *result, const char *err)
{
	_need_field_type = TRUE;
	return do_query(db, err, (SQLITE_RESULT **)result, query, 0);
}


/*****************************************************************************

	get_last_insert_id()

	Return the value of the last serial field used in an INSERT statement

	<db> is the database handle, as returned by open_database()

*****************************************************************************/

static int64_t get_last_insert_id(DB_DATABASE *db)
{
	SQLITE_RESULT *res;

	if (do_query(db, "Unable to retrieve last insert id", &res, "select last_insert_rowid();", 0))
		return -1;

	return atoll(sqlite_query_get_string(res, 0, 0));
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

static void query_init(DB_RESULT result, DB_INFO * info, int *count)
{
	SQLITE_RESULT *res = (SQLITE_RESULT *)result;

	if (res)
	{
		*count = res->nrow;
		info->nfield = res->ncol;
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

static void query_release(DB_RESULT result, DB_INFO * info)
{
	sqlite_query_free((SQLITE_RESULT *)result);
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

	This function must return DB_OK, DB_ERROR or DB_NO_DATA

	This function must use GB.StoreVariant() to store the value in the
	buffer.

*****************************************************************************/

static int query_fill(DB_DATABASE *db, DB_RESULT result, int pos, GB_VARIANT_VALUE * buffer, int next)
{
	SQLITE_RESULT *res = (SQLITE_RESULT *)result;
	int i;
	char *data;
	int len;
	GB_VARIANT value;
	int type;

	for (i = 0; i < res->ncol; i++)
	{
		type = res->types[i];

		if (type == DB_T_BLOB)
			data = NULL;
		else
		{
			sqlite_query_get(res, pos, i, &data, &len);
			if (len == 0)
				data = NULL;
		}

		value.type = GB_T_VARIANT;
		value.value.type = GB_T_NULL;

		if (data)
			conv_data(data, &value.value, type);

		//GB.FreeString(&data);
		GB.StoreVariant(&value, &buffer[i]);
	}

	return DB_OK;
}


/*****************************************************************************

  blob_read()

  Returns the value of a BLOB field.

  <result> is the handle of the result.
  <pos> is the index of the record in the result.
  <blob> points at a DB_BLOB structure that will receive a pointer to the
  data and its length.

*****************************************************************************/

static void blob_read(DB_RESULT result, int pos, int field, DB_BLOB * blob)
{
	sqlite_query_get((SQLITE_RESULT *)result, pos, field, &blob->data, &blob->length);
	blob->constant = TRUE;
}


/*****************************************************************************

  field_name()

  Return the name of a field in a result from its index.

  <result> is the result handle.
  <field> is the field index.

*****************************************************************************/

static char *field_name(DB_RESULT result, int field)
{
	return (char *)((SQLITE_RESULT *)result)->names[field];
}


/*****************************************************************************

  field_index()

  Return the index of a field in a result from its name.

  <Result> is the result handle.
  <name> is the field name.
  <handle> can be ignored by this driver.

*****************************************************************************/

static int field_index(DB_RESULT result, const char *name, DB_DATABASE *db)
{
	return sqlite_query_find_field((SQLITE_RESULT *)result, name);
}


/*****************************************************************************

  field_type()

  Return the Gambas type of a field in a result from its index.

  <result> is the result handle.
  <field> is the field index.

*****************************************************************************/

static GB_TYPE field_type(DB_RESULT result, int field)
{
	return (GB_TYPE)((SQLITE_RESULT *)result)->types[field];
}


/*****************************************************************************

  field_length()

  Return the length of a field in a result from its index.

  <result> is the result handle.
  <field> is the field index.

*****************************************************************************/

static int field_length(DB_RESULT result, int field)
{
	return ((SQLITE_RESULT *)result)->lengths[field];
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

static int begin_transaction(DB_DATABASE * db)
{
	return do_query(db, "Unable to begin transaction: &1", NULL, "BEGIN", 0);
}


/*****************************************************************************

  commit_transaction()

  Commit a transaction.

  <handle> is the database handle.

  This function returns TRUE if the command has failed, and FALSE if
  everything was OK.

*****************************************************************************/

static int commit_transaction(DB_DATABASE * db)
{
	return do_query(db, "Unable to commit transaction: &1", NULL, "COMMIT", 0);
}


/*****************************************************************************

  rollback_transaction()

  Rolllback a transaction.

  <handle> is the database handle.

  This function returns TRUE if the command has failed, and FALSE if
  everything was OK.

*****************************************************************************/

static int rollback_transaction(DB_DATABASE * db)
{
	return do_query(db, "Unable to rollback transaction: &1", NULL, "ROLLBACK", 0);
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

static int field_info(DB_DATABASE * db, const char *table, const char *field, DB_FIELD * info);

static int table_init(DB_DATABASE * db, const char *table, DB_INFO * info)
{
	const char *qfield = "PRAGMA table_info('&1')";

	SQLITE_RESULT *res;
	int i, n;
	DB_FIELD *f;
	char *field;

	/* Nom de la table */

	info->table = GB.NewZeroString(table);

	/* Liste des champs */

	if (do_query(db, "Unable to get table fields: &1", &res, qfield, 1, table))
		return TRUE;

	info->nfield = n = res->nrow;
	if (n == 0)
	{
		sqlite_query_free(res);
		return TRUE;
	}

	GB.Alloc(POINTER(&info->field), sizeof(DB_FIELD) * n);

	for (i = 0; i < n; i++)
	{
		sqlite_query_get(res, i, 1, &field, NULL);
		f = &info->field[i];

		if (field_info(db, table, field, f))
		{
			sqlite_query_free(res);
			return TRUE;
		}

		f->name = GB.NewZeroString(field);
	}

	sqlite_query_free(res);
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

static int table_index(DB_DATABASE * db, const char *table, DB_INFO * info)
{
	const char *qindex1 = "PRAGMA index_list('&1')";
	const char *qindex2 = "PRAGMA index_info('&1')";

	SQLITE_RESULT *res;
	int n = 0;
	int i;
	char *sql;
	char *data;

	/* Index primaire */

	info->nindex = 0;

	if (do_query(db, "Unable to get primary index: &1", &res, qindex1, 1, table))
		return TRUE;

	n = res->nrow;

	for (i = 0; i < n; i++)
	{
		data = sqlite_query_get_string(res, i, 2);
		if (*data != '1')
			continue;

		data = sqlite_query_get_string(res, i, 1);
		if (strstr(data, "autoindex") == NULL)
			continue;

		sql = GB.NewZeroString(data);

		sqlite_query_free(res);

		if (do_query(db, "Unable to get information on primary index: &1", &res, qindex2, 1, sql))
		{
			GB.FreeString(&sql);
			return TRUE;
		}
		GB.FreeString(&sql);

		info->nindex = res->nrow;
		GB.Alloc(POINTER(&info->index), sizeof(int) * info->nindex);

		for (i = 0; i < info->nindex; i++)
			info->index[i] = sqlite_query_get_int(res, i, 1);
		break;
	}

	sqlite_query_free(res);

	if (info->nindex == 0)
	{
		// [BM] If there is no primary key, we suppose that the first field of INTEGER datatype is the primary key.
		// Because we use INTEGER only when creating the AUTOINCREMENT field.

		if (do_query(db, "Unable to get primary index: &1", &res, "PRAGMA table_info('&1')", 1, table))
			return TRUE;

		info->nindex = 1;
		GB.Alloc(POINTER(&info->index), sizeof(int));

		for (i = 0; i < res->nrow; i++)
		{
			if (strcasecmp(sqlite_query_get_string(res, i, 2), "INTEGER") == 0)
			{
				info->index[0] = i;
				break;
			}
		}

		sqlite_query_free(res);

		if (i >= res->nrow)
		{
			GB.Free(POINTER(&info->index));
			return TRUE;
		}
		else
		{
			return FALSE;
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

static void table_release(DB_DATABASE * db, DB_INFO * info)
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

static int table_exist(DB_DATABASE * db, const char *table)
{
	const char *query = "select tbl_name from "
		"( select tbl_name from sqlite_master where type = 'table' union "
		"select tbl_name from sqlite_temp_master where type = 'table' ) "
		"where tbl_name = '&1'";

	if (strcmp(table, "sqlite_master") == 0
			|| strcmp(table, "sqlite_temp_master") == 0)
	{
		return TRUE;
	}

	SQLITE_RESULT *res;
	int exist;

	if (do_query(db, "Unable to check table: &1", &res, query, 1, table))
		return FALSE;

	exist = res->nrow > 0;
	sqlite_query_free(res);
	return exist;
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

static int table_list(DB_DATABASE * db, char ***tables)
{
	const char *query =
		"select tbl_name from "
		"( select tbl_name from sqlite_master where type = 'table' union "
		"   select tbl_name from sqlite_temp_master where type = 'table')";

	SQLITE_RESULT *res;
	int nrow;
	char *data;
	int len;
	int i;

	if (do_query(db, "Unable to get tables: &1", &res, query, 0))
		return -1;

	nrow = res->nrow;

	// sqlite_master and sqlite_temp_master need to be added to the list
	GB.NewArray(tables, sizeof(char *), nrow + 2);

	for (i = 0; i < nrow; i++)
	{
		sqlite_query_get(res, i, 0, &data, &len);
		(*tables)[i] = GB.NewString(data, len);
	}

	sqlite_query_free(res);

	(*tables)[nrow] = GB.NewZeroString("sqlite_master");
	(*tables)[nrow + 1] = GB.NewZeroString("sqlite_temp_master");
	return nrow + 2;
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

#if 0
static int old_table_primary_key(DB_DATABASE * db, const char *table, char ***primary)
{
	const char *qindex1 = "PRAGMA index_list('&1')";
	const char *qindex2 = "PRAGMA index_info('&1')";

	Dataset *res;
	int i, n;
	char *sql;
	result_set *r;

	if (do_query
			(db, "Unable to get primary key: &1", &res, qindex1, 1, table))
		return TRUE;

	GB.NewArray(primary, sizeof(char *), 0);

	r = (result_set *) res->getResult();
	n = r->records.size();

	for (i = 0; i < n; i++)
	{
		if (strstr(r->records[i][1].get_asString().data(), "autoindex"))
		{
			if (strncmp("1", r->records[i][2].get_asString().data(), 1) == 0)
			{
				sql = GB.NewZeroString(r->records[i][1].get_asString().data());
				res->close();

				if (do_query(db, "Unable to get primary key: &1", &res, qindex2, 1, sql))
				{
					res->close();
					GB.FreeString(&sql);
					return TRUE;
				}
				GB.FreeString(&sql);

				r = (result_set *) res->getResult();
				if ((n = r->records.size()) < 1)
				{
					// No information returned for key
					res->close();
					return TRUE;
				}

				for (i = 0; i < n; i++)
				{
					*(char **)GB.Add(primary) = GB.NewZeroString(r->records[i][2].get_asString().data());
				}
				break;
			}
		}
	}

	res->close();

	// [BM] If there is no primary key, we suppose that the first field of INTEGER datatype is the primary key.
	// Because we use INTEGER only when creating the AUTOINCREMENT field.

	if (GB.Count(*primary) == 0)
	{
		if (do_query
				(db, "Unable to get primary key: &1", &res,
				 "PRAGMA table_info('&1')", 1, table))
			return TRUE;

		r = (result_set *) res->getResult();

		for (i = 0; i < (int) r->records.size(); i++)
		{
			if (strcasecmp(r->records[i][5].get_asString().data(), "0") != 0)
			{
				*(char **)GB.Add(primary) = GB.NewZeroString(r->records[i][1].get_asString().data());
				break;
			}
		}
	}

	return FALSE;
}
#endif

static int table_primary_key(DB_DATABASE * db, const char *table, char ***primary)
{
	SQLITE_RESULT *res;
	int i, j, n;
	char *data;
	int len;

	if (do_query(db, "Unable to get primary key: &1", &res, "PRAGMA table_info('&1')", 1, table))
		return TRUE;

	n = 0;
	for (i = 0; i < res->nrow; i++)
	{
		j = sqlite_query_get_int(res, i, 5);
		if (j > n)
			n = j;
	}

	GB.NewArray(primary, sizeof(char *), n);

	for (i = 0; i < res->nrow; i++)
	{
		j = sqlite_query_get_int(res, i, 5);
		if (j > 0)
		{
			sqlite_query_get(res, i, 1, &data, &len);
			(*primary)[j - 1] = GB.NewString(data, len);
		}
	}

	sqlite_query_free(res);
	return FALSE;
}

/*****************************************************************************

  table_is_system()

  Returns if a table is a system table.

  <handle> is the database handle.
  <table> is the table name.

  This function returns TRUE if the table is a system table, and FALSE if
  not.

  Note: According to the documentation, all tables beginning without
        "sqlite_" are reserved.

*****************************************************************************/

static int table_is_system(DB_DATABASE * db, const char *table)
{
	return strncmp(table, "sqlite_", 7) == 0;
}

/*****************************************************************************

  table_type()

  Not Valid in Sqlite

  <handle> is the database handle.
  <table> is the table name.

*****************************************************************************/

static char *table_type(DB_DATABASE * db, const char *table, const char *type)
{
	if (type)
		GB.Error("SQLite does not have any table types");
	return NULL;
}

/*****************************************************************************

  table_delete()

  Deletes a table.

  <handle> is the database handle.
  <table> is the table name.

  This function returns TRUE if the command has failed, and FALSE if
  everything was OK.

*****************************************************************************/

static int table_delete(DB_DATABASE * db, const char *table)
{
	return do_query(db, "Unable to delete table: &1", NULL, "drop table '&1'", 1, table);
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

*****************************************************************************/

static int table_create(DB_DATABASE * db, const char *table, DB_FIELD * fields, char **primary, const char *not_used)
{
	DB_FIELD *fp;
	int comma;
	const char *type;
	int i;
	bool no_pkey = FALSE;

	DB.Query.Init();

	DB.Query.Add("CREATE TABLE ");
	DB.Query.Add(QUOTE_STRING);
	DB.Query.Add(table);
	DB.Query.Add(QUOTE_STRING);
	DB.Query.Add(" ( ");

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
		{
			DB.Query.Add(" INTEGER PRIMARY KEY AUTOINCREMENT");
			no_pkey = TRUE;
		}
		else if (fp->type == DB_T_BLOB)
		{
			DB.Query.Add(" BLOB ");
		}
		else
		{
			switch (fp->type)
			{
				case GB_T_BOOLEAN:
					type = "BOOL";
					break;
				case GB_T_INTEGER:
					type = "INT4";
					break;
				case GB_T_LONG:
					type = "BIGINT";
					break;
				case GB_T_FLOAT:
					type = "FLOAT8";
					break;
				case GB_T_DATE:
					type = "DATETIME";
					break;
				case GB_T_STRING:

					if (fp->length <= 0)
						type = "TEXT";
					else
					{
						sprintf(_buffer, "VARCHAR(%d)", fp->length);
						type = _buffer;
					}

					break;

				default:
					type = "TEXT";
					break;
			}

			DB.Query.Add(" ");
			DB.Query.Add(type);

			if (fp->collation && *fp->collation)
			{
				DB.Query.Add(" COLLATE ");
				DB.Query.Add(fp->collation);
			}

			if (fp->def.type != GB_T_NULL)
			{
				DB.Query.Add(" NOT NULL DEFAULT ");
				DB.FormatVariant(&_driver, &fp->def, DB.Query.AddLength);
			}
			else if (DB.StringArray.Find(primary, fp->name) >= 0)
			{
				DB.Query.Add(" NOT NULL ");
			}
		}
	}

	if (primary && !no_pkey)
	{
		DB.Query.Add(", PRIMARY KEY (");

		for (i = 0; i < GB.Count(primary); i++)
		{
			if (i > 0)
				DB.Query.Add(",");

			DB.Query.Add(QUOTE_STRING);
			DB.Query.Add(primary[i]);
			DB.Query.Add(QUOTE_STRING);
		}

		DB.Query.Add(")");
	}

	DB.Query.Add(" )");

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

static int field_exist(DB_DATABASE * db, const char *table, const char *field)
{
	const char *query = "PRAGMA table_info('&1')";

	int i;
	SQLITE_RESULT *res;
	int exist = 0;

	if (do_query(db, "Unable to find field: &1.&2", &res, query, 2, table, field))
		return FALSE;

	for (i = 0; i < res->nrow; i++)
	{

		if (strcmp(field, sqlite_query_get_string(res, i, 1)) == 0)
		{
			exist++;
			break;
		}
	}

	sqlite_query_free(res);
	return exist;
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

static int field_list(DB_DATABASE * db, const char *table, char ***fields)
{
	const char *query = "PRAGMA table_info('&1')";

	int i, n;
	SQLITE_RESULT *res;

	if (do_query(db, "Unable to get fields: &1", &res, query, 1, table))
		return -1;

	n = res->nrow;

	if (fields) /* (BM) see the function commentary */
	{
		GB.NewArray(fields, sizeof(char *), n);

		for (i = 0; i < n; i++)
			(*fields)[i] = GB.NewZeroString(sqlite_query_get_string(res, i, 1));
	}

	sqlite_query_free(res);
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
	const char *query = "PRAGMA table_info('&1')";

	SQLITE_RESULT *res;
	GB_VARIANT def;
	char *val;
	char *_fieldName = NULL;
	char *_fieldType = NULL;
	char *_defaultValue = NULL;
	bool _fieldNotNull = FALSE;
	int i, n;
	bool autoinc;
	char *schema;
	int type;

	if (do_query(db, "Unable to get fields: &1", &res, query, 1, table))
		return TRUE;

	n = res->nrow;

	//fprintf(stderr, "field_info: %s.%s\n", table, field);

	for (i = 0; i < n; i++)
	{
		_fieldName = sqlite_query_get_string(res, i, 1);

		if (strcmp(_fieldName, field) == 0)
		{
			_fieldType = sqlite_query_get_string(res, i, 2);
			_fieldNotNull = sqlite_query_get_int(res, i, 3) != 0;
			_defaultValue = sqlite_query_get_string(res, i, 4);
			break;
		}
	}

	if (i >= n)
	{
		GB.Error("Unable to find field &1.&2", table, field);
		sqlite_query_free(res);
		return TRUE;
	}

	//fprintf(stderr, "field_info: type = %s  not_null = %s  default = %s\n", _fieldType, _fieldNotNull, _defaultValue);

	info->name = NULL;

	// This API is not always defined!
	//(sqlite3_table_column_metadata(db_handle, NULL, table, field, NULL, NULL, NULL, NULL, &autoinc) != SQLITE_OK)

	/*{
		// [BM] We use INTEGER only when creating the AUTOINCREMENT field.
		fprintf(stderr, "_fieldType = %s\n", _fieldType);
		autoinc = strstr(_fieldType, "INTEGER") && strstr(_fieldType, "AUTOINCREMENT");
	}*/

	autoinc = FALSE;
	info->collation = NULL;

	if (_table_schema)
		schema = _table_schema;
	else
		schema = get_table_schema(db, table);

	if (schema)
	{
		char *p, *p2;
		char *field_desc;
		int len;

		p = strchr(schema, '(');
		if (p)
		{
			while (*p != ')')
			{
				p++;
				p2 = strchr(p, ',');
				if (!p2)
					p2 = p + strlen(p) - 1;

				while (p < p2 && *p == ' ')
					p++;

				if (*p == '\'' || *p == '"')
					p++;

				len = strlen(_fieldName);
				if ((p2 - p) < len || strncasecmp(p, _fieldName, len))
				{
					p = p2;
					continue;
				}

				p += len;
				if (*p == '\'')
					p++;

				len = p2 - p;
				if (len <= 0)
					break;

				field_desc = GB.NewString(p, len);

				if (strstr(_fieldType, "INTEGER"))
				{
					if (strstr(field_desc, "AUTOINCREMENT"))
						autoinc = TRUE;
				}

				p = strstr(field_desc, "COLLATE");
				if (p)
				{
					p += 7;
					while (*p == ' ')
						p++;

					p2 = strchr(p, ' ');
					if (!p2)
						p2 = field_desc + len;
					info->collation = GB.NewString(p, p2 - p);
				}

				GB.FreeString(&field_desc);
				break;
			}

		}
	}

	if (!_table_schema)
		GB.FreeString(&schema);

	type = sqlite_get_type(_fieldType, &info->length);

	if (autoinc)
		info->type = DB_T_SERIAL;
	else
		info->type = type;

	//fprintf(stderr, "field_info: type = %d  length = %d\n", info->type, info->length);

	info->def.type = GB_T_NULL;

	if (_fieldNotNull)
	{
		def.type = GB_T_VARIANT;
		def.value.type = GB_T_NULL;

		val = DB.UnquoteString(_defaultValue, strlen(_defaultValue), '\'');

		if (val && *val)
		{
			conv_data(val, &def.value, type);
			GB.StoreVariant(&def, &info->def);
		}
	}

	sqlite_query_free(res);
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

static int index_exist(DB_DATABASE * db, const char *table, const char *index)
{
	const char *query = "select tbl_name from "
		"( select tbl_name from sqlite_master where type = 'index' and "
		" name = '&2' union "
		"select tbl_name from sqlite_temp_master where type = 'index' and "
		" name = '&2' ) " "where tbl_name = '&1'";

	SQLITE_RESULT *res;
	int exist;

	if (do_query(db, "Unable to check table: &1", &res, query, 2, table, index))
		return FALSE;

	exist = res->nrow > 0;
	sqlite_query_free(res);
	return exist;
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

static int index_list(DB_DATABASE * db, const char *table, char ***indexes)
{
  const char *query =
		"select name from "
		"( select name from sqlite_master where type = 'index' and tbl_name = '&1' "
		" union select name from sqlite_temp_master where type = 'index' and "
		" tbl_name = '&1')";

	SQLITE_RESULT *res;
	int nindex, i;

	if (do_query(db, "Unable to get tables: &1", &res, query, 1, table))
		return -1;

	nindex = res->nrow;
	GB.NewArray(indexes, sizeof(char *), nindex);

	for (i = 0; i < nindex; i++)
		(*indexes)[i] = GB.NewZeroString(sqlite_query_get_string(res, i, 0));

	sqlite_query_free(res);
	return nindex;
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

static int index_info(DB_DATABASE * db, const char *table, const char *index, DB_INDEX * info)
{
	/* sqlite indexes are unique to the database, and not to the table */

	const char *qindex1 = "PRAGMA index_list('&1')";
	const char *qindex2 = "PRAGMA index_info('&1')";

	SQLITE_RESULT *res;
	int i, j, n;

	if (do_query(db, "Unable to get index info for table: &1", &res, qindex1, 1, table))
		return TRUE;

	if ((n = res->nrow) == 0)
	{
		/* no indexes found */
		sqlite_query_free(res);
		GB.Error("Unable to find index &1.&2", table, index);
		return TRUE;
	}

	for (i = 0, j = 0; i < n; i++)
	{															/* Find the required index */
		if (strcmp(index, sqlite_query_get_string(res, i, 1)) == 0)
		{
			j++;
			break;
		}
	}

	if (j == 0)
	{
		GB.Error("Unable to find index &1.&2", table, index);
		sqlite_query_free(res);
		return TRUE;
	}

	info->name = NULL;
	info->unique = sqlite_query_get_int(res, i, 2) != 0;
	info->primary = strstr(sqlite_query_get_string(res, i, 1), "autoindex") != NULL;

	sqlite_query_free(res);

	DB.Query.Init();

	if (do_query(db, "Unable to get index info for : &1", &res, qindex2, 1, index))
		return TRUE;

	n = res->nrow;
	i = 0;
	/* (BM) row can be null if we are seeking the last index */
	while (i < n)
	{
		if (i > 0)
			DB.Query.Add(",");

		/* Get fields in key */
		DB.Query.Add(sqlite_query_get_string(res, i, 2));
		i++;
	}

	sqlite_query_free(res);
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

static int index_delete(DB_DATABASE * db, const char *table, const char *index)
{
	return do_query(db, "Unable to delete index: &1", NULL, "drop index &1", 1, index);
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

static int index_create(DB_DATABASE * db, const char *table, const char *index, DB_INDEX *info)
{
	DB.Query.Init();

	DB.Query.Add("CREATE ");
	if (info->unique)
		DB.Query.Add("UNIQUE ");
	DB.Query.Add("INDEX ");
	DB.Query.Add(QUOTE_STRING);
	DB.Query.Add(index);
	DB.Query.Add(QUOTE_STRING);
	DB.Query.Add(" ON ");
	DB.Query.Add(QUOTE_STRING);
	DB.Query.Add(table);
	DB.Query.Add(QUOTE_STRING);
	DB.Query.Add(" ( ");
	DB.Query.Add(info->fields);
	DB.Query.Add(" )");

	return do_query(db, "Cannot create index: &1", NULL, DB.Query.Get(), 0);
}

/*****************************************************************************

   database_exist()

   Returns if a database exists

   <handle> is any database handle.
   <name> is the database name.

   This function returns TRUE if the database exists, and FALSE if not.
   SQLite: Databases are just files, so we need to ceck to see whether
   the file exists and is a sqlite file.

******************************************************************************/

static int database_exist(DB_DATABASE *db, const char *name)
{
	SQLITE_DATABASE *conn = (SQLITE_DATABASE *)db->handle;
	char *fullpath;
	bool exist;

	if (!name || !*name || strcmp(name, ":memory:") == 0)
		return TRUE;								//Database is loaded in memory only

	fullpath = find_database(name, conn->host);
	exist = fullpath != NULL;
	GB.FreeString(&fullpath);
	return exist;
}

/*****************************************************************************

    database_list()

    Returns an array containing the name of each database

    <handle> is any database handle.
    <databases> points to a variable that will receive the char* array.

    This function returns the number of databases, or -1 if the command has
    failed.

    Be careful: <databases> can be NULL, so that just the count is returned.

    Sqlite databases are files. Using we will only list
    files within a designated directory. Propose that all
    areas are walked through.

 ******************************************************************************/

static int database_list(DB_DATABASE *db, char ***databases)
{
	SQLITE_DATABASE *conn = (SQLITE_DATABASE *)db->handle;
	const char *dbhome;

	GB.NewArray(databases, sizeof(char *), 0);

	/* Hostname contains home area */
	if (conn->host && *conn->host)
	{
		walk_directory(conn->host, databases);
	}
	else
	{
		dbhome = get_database_home();
		if (dbhome)
		{
			walk_directory(dbhome, databases);
			GB.Free(POINTER(&dbhome));
		}
	}

	return GB.Count(databases);
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
 *   Note: Sqlite doesn't have such a thing.
 ******************************************************************************/

static int database_is_system(DB_DATABASE * db, const char *name)
{
	return FALSE;
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

static int database_delete(DB_DATABASE * db, const char *name)
{
	SQLITE_DATABASE *conn = (SQLITE_DATABASE *)db->handle;
	char *fullpath = NULL;
	bool err;

	fullpath = find_database(name, conn->host);

	if (!fullpath)
	{
		GB.Error("Cannot find database: &1", name);
		err = TRUE;
	}
	else if (remove(fullpath) != 0)
	{
		GB.Error("Unable to delete database  &1", fullpath);
		err = TRUE;
	}
	else
		err = FALSE;

	GB.FreeString(&fullpath);
	return err;
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
 *   SQLite automatically creates a database on connect if the file
 *   does not exist.
 ******************************************************************************/

static int database_create(DB_DATABASE *db, const char *name)
{
	SQLITE_DATABASE *conn, *save;
	char *fullpath = NULL;
	const char *host = NULL;
	char *dir;

	save = (SQLITE_DATABASE *)db->handle;

	/* Does name include fullpath? */
	if (name && name[0] == '/')
	{
		fullpath = GB.NewZeroString(name);
		goto _CREATE_DATABASE;
	}

	/* Hostname contains home area */
	host = save->host;
	if (host && host[0])
	{
		fullpath = GB.NewZeroString(host);
	}
	else
	{
		dir = get_database_home();
		mkdir(dir, S_IRWXU);
		fullpath = GB.NewZeroString(dir);
		GB.Free(POINTER(&dir));
	}

	if (fullpath[strlen(fullpath) - 1] != '/')
		fullpath = GB.AddChar(fullpath, '/');

	fullpath = GB.AddString(fullpath, name, 0);

_CREATE_DATABASE:

	if (DB.IsDebug())
		fprintf(stderr, "sqlite3: create database: %s\n", fullpath);

	conn = sqlite_open_database(fullpath, host);
	GB.FreeString(&fullpath);

	if (!conn)
	{
		GB.Error("Cannot create database: &1", sqlite_get_error_message(NULL));
		return TRUE;
	}

	//Create and remove a table to initialize database
	db->handle = conn;
	if (!do_query(db, "Unable to initialise database", NULL, "CREATE TABLE GAMBAS (FIELD1 TEXT)", 0))
		do_query(db, NULL, NULL, "DROP TABLE GAMBAS", 0);

	sqlite_close_database(conn);
	db->handle = save;

	return FALSE;
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
 *  Sqlite does not have different  users.  Access is controlled by
 *  access rightd on the file.
 *  We can check that the user exists on the machine and has access to
 *  database file!
 *  [Currently only checks against /etc/passwd.
 *   Does not check against /etc/shadow or pam.
 *
 ******************************************************************************/

static int user_exist(DB_DATABASE *db, const char *name)
{
	return FALSE;
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
 *   Sqlite does not have users.
 *
 ******************************************************************************/


static int user_list(DB_DATABASE * db, char ***users)
{
	if (users)
		GB.NewArray(users, sizeof(char *), 0);
	return 0;
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
 *   Sqlite privileges are just file privileges. We will return Admin
 *   rights where privilege allows Write. There is no password.
 ******************************************************************************/

static int user_info(DB_DATABASE * db, const char *name, DB_USER * info)
{
	GB.Error("Invalid user &1", name);
	return TRUE;
}

/*****************************************************************************

  user_delete()

  Deletes a user.

  <handle> is any database handle.
  <name> is the user name.

  This function returns TRUE if the command has failed, and FALSE if
  everything was OK.

  Sqlite users are operated by the O/S

*****************************************************************************/

static int user_delete(DB_DATABASE * db, const char *name)
{
	GB.Error("Invalid user: &1", name);
	return TRUE;
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
 *
 *   Sqlite: No user create
 ******************************************************************************/

static int user_create(DB_DATABASE * db, const char *name, DB_USER * info)
{
	GB.Error("SQLite has no users");
	return TRUE;
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
 *   Sqlite : No user passwords.
 ******************************************************************************/

static int user_set_password(DB_DATABASE * db, const char *name, const char *password)
{
	GB.Error("SQLite has no users");
	return TRUE;
}


/*****************************************************************************

  The driver interface

*****************************************************************************/

DECLARE_DRIVER(_driver, "sqlite3");

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
