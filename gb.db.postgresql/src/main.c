/***************************************************************************

  main.c

  PostgreSQL driver

  (c) 2000-2007 Benoit Minisini <gambas@users.sourceforge.net>

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

#define __MAIN_C

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include <libpq-fe.h>
#include <postgres.h>
#include <pg_type.h>

#ifdef PACKAGE_NAME
	#undef PACKAGE_NAME
	#undef PACKAGE_BUGREPORT
	#undef PACKAGE_STRING
	#undef PACKAGE_TARNAME
	#undef PACKAGE_VERSION
#endif

#include "gb.db.proto.h"
#include "main.h"


GB_INTERFACE GB EXPORT;
DB_INTERFACE DB EXPORT;


static char _buffer[32];
static DB_DRIVER _driver;
static int _last_error;
/*static int _print_query = FALSE;*/

/* Internal function to check the result of a query */

static int check_result(PGresult *res, const char *err)
{
	_last_error = 0;
	
	if (!res)
	{
		GB.Error("Out of memory");
		return TRUE;
	}

	_last_error = PQresultStatus(res);
	switch (_last_error)
	{
		case PGRES_COMMAND_OK:
		case PGRES_TUPLES_OK:
			return FALSE;

		default:
			if (err)
				GB.Error(err, PQresultErrorMessage(res));
			PQclear(res);
			return TRUE;
	}
}


/* internal function to quote a value stored as a string */

static void quote_string(const char *data, int len, DB_FORMAT_CALLBACK add)
{
	int i;
	unsigned char c;
	char buffer[8];

	if (DB.GetCurrentDatabase()->version >= 80200)
		(*add)("E", 1);

	(*add)("'", 1);
	for (i = 0; i < len; i++)
	{
		c = (unsigned char)data[i];
		if (c == '\\')
			(*add)("\\\\", 2);
		else if (c == '\'')
			(*add)("''", 2);
		else if (c < 32 || c > 127)
		{
			buffer[0] = '\\';
			buffer[1] = '0' + ((c >> 6) & 0x7);
			buffer[2] = '0' + ((c >> 3) & 0x7);
			buffer[3] = '0' + (c & 0x7);
			(*add)(buffer, 4);
		}
		else
			(*add)((const char *)&c, 1);
	}
	(*add)("'", 1);
}

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
	
	GB.TempString(&res, NULL, len_res);
	
	p = res;
	for (i = 0; i < len; i++)
	{
		c = str[i];
		if (c == '\\' || c == quote)
		{
			*p++ = c;
			*p++ = c;
		}
		/*else if (c == 0)
		{
			*p++ = '\\';
			*p++ = '0';
		}*/
		else
			*p++ = c;
	}
	*p = 0;
	
	return res;
}

/* internal function to quote a value stored as a blob */

static void quote_blob(const char *data, int len, DB_FORMAT_CALLBACK add)
{
	int i;
	unsigned char c;
	char buffer[8];

	if (DB.GetCurrentDatabase()->version >= 80200)
		(*add)("E", 1);

	(*add)("'", 1);
	for (i = 0; i < len; i++)
	{
		c = (unsigned char)data[i];
		if (c == '\\')
			(*add)("\\\\\\\\", 4);
		else if (c == '\'')
			(*add)("\\'", 2);
		else if (c < 32 || c > 127)
		{
			buffer[0] = '\\';
			buffer[1] = '\\';
			buffer[2] = '0' + ((c >> 6) & 0x7);
			buffer[3] = '0' + ((c >> 3) & 0x7);
			buffer[4] = '0' + (c & 0x7);
			(*add)(buffer, 5);
		}
		else
			(*add)((const char *)&c, 1);
	}
	(*add)("'", 1);
}

/* internal function to unquote a value stored as a string */

static int unquote_string(const char *data, int len, DB_FORMAT_CALLBACK add)
{
	int i;
	char c;

	if (!data || *data != '\'')
		return TRUE;

	for (i = 1;; i++)
	{
		c = data[i];
		if (c == '\'')
			break;
		if (c == '\\')
			i++;
		(*add)(&data[i], 1);
	}

	return FALSE;
}

/* internal function to unquote a value stored as a blob */

static int unquote_blob(const char *data, int len, DB_FORMAT_CALLBACK add)
{
	int i;
	char c;

	//if (!data || *data != '\'')
	//  return TRUE;

	for (i = 0; i < len; i++)
	{
		c = data[i];
		if (c == '\\')
		{
			i++;
			c = data[i];
			if (c >= '0' && c <= '9' && i < (len - 2))
			{
				c = ((data[i] - '0') << 6) + ((data[i + 1] - '0') << 3) + (data[i + 2] - '0');
				i += 2;
				(*add)(&c, 1);
				continue;
			}
		}
		(*add)(&data[i], 1);
	}

	return FALSE;
}

/* Internal function to convert a database type into a Gambas type */

static GB_TYPE conv_type(Oid type)
{
	switch(type)
	{
		case BOOLOID:
			return GB_T_BOOLEAN;

		case INT2OID:
		case INT4OID:
			return GB_T_INTEGER;

		case INT8OID:
			return GB_T_LONG;

		case NUMERICOID:
		case FLOAT4OID:
		case FLOAT8OID:
			return GB_T_FLOAT;

		case ABSTIMEOID:
		case RELTIMEOID:
		case DATEOID:
		case TIMEOID:
		case TIMESTAMPOID:
#ifdef DATETIMEOID
		case DATETIMEOID:
#endif
#ifdef TIMESTAMPTZOID
		case TIMESTAMPTZOID:
#endif
			return GB_T_DATE;

		case BYTEAOID:
			return DB_T_BLOB;

		case CHAROID:
		case BPCHAROID:
		case VARCHAROID:
		case TEXTOID:
		case NAMEOID:
		case CASHOID:
		default:
			return GB_T_STRING;

	}
}


/* Internal function to convert a database boolean value  */

static int conv_boolean(const char *data)
{
	return strcasecmp(data, "t") == 0 || strcasecmp(data, "'t'") == 0;
}

/* Internal function to convert a database value into a Gambas variant value */

static void conv_data(const char *data, int len, GB_VARIANT_VALUE *val, Oid type)
{
	GB_VALUE conv;
	GB_DATE_SERIAL date;
	double sec;
	bool bc;

	switch (type)
	{
		case BOOLOID:

			val->type = GB_T_BOOLEAN;
			val->value._boolean = conv_boolean(data);
			break;

		case INT2OID:
		case INT4OID:

			GB.NumberFromString(GB_NB_READ_INTEGER, data, strlen(data), &conv);

			val->type = GB_T_INTEGER;
			val->value._integer = ((GB_INTEGER *)&conv)->value;

			break;

		case INT8OID:

			GB.NumberFromString(GB_NB_READ_LONG, data, strlen(data), &conv);

			val->type = GB_T_LONG;
			val->value._long = ((GB_LONG *)&conv)->value;

			break;

		case NUMERICOID:
		case FLOAT4OID:
		case FLOAT8OID:

			GB.NumberFromString(GB_NB_READ_FLOAT, data, strlen(data), &conv);

			val->type = GB_T_FLOAT;
			val->value._float = ((GB_FLOAT *)&conv)->value;

			break;

		case ABSTIMEOID:
		case RELTIMEOID:
		case DATEOID:
		case TIMEOID:
		case TIMESTAMPOID:
		#ifdef DATETIMEOID
		case DATETIMEOID:
		#endif
		#ifdef TIMESTAMPTZOID
		case TIMESTAMPTZOID:
		#endif

			memset(&date, 0, sizeof(date));

			if (len > 3 && strcmp(&data[len - 2], "BC") == 0)
				bc = TRUE;
			else
				bc = FALSE;

			switch(type)
			{
				case ABSTIMEOID:
				case RELTIMEOID:
				case DATEOID:

					sscanf(data, "%4hu-%2hu-%2hu", &date.year, &date.month, &date.day);
					break;

				case TIMEOID:

					sscanf(data, "%2hu:%2hu:%lf", &date.hour, &date.min, &sec);
					date.sec = (short)sec;
					date.msec = (short)((sec - date.sec) * 1000 + 0.5);
					break;

				case TIMESTAMPOID:
				#ifdef DATETIMEOID
				case DATETIMEOID:
				#endif
				#ifdef TIMESTAMPTZOID
				case TIMESTAMPTZOID:
				#endif

					sscanf(data, "%4hu-%2hu-%2hu %2hu:%2hu:%lf", &date.year, &date.month, &date.day, &date.hour, &date.min, &sec);
					date.sec = (short)sec;
					date.msec = (short)((sec - date.sec) * 1000 + 0.5);
					break;
			}

			if (bc)
				date.year = (-date.year);

			GB.MakeDate(&date, (GB_DATE *)&conv);

			val->type = GB_T_DATE;
			val->value._date.date = ((GB_DATE *)&conv)->value.date;
			val->value._date.time = ((GB_DATE *)&conv)->value.time;

			break;

		case BYTEAOID:
			// The BLOB are read by the blob_read() driver function
			// You must set NULL there.
			val->type = GB_T_NULL;
			break;

		case CHAROID:
		case BPCHAROID:
		case VARCHAROID:
		case TEXTOID:
		case NAMEOID:
		case CASHOID:
		default:

			val->type = GB_T_CSTRING;
			val->value._string = (char *)data;
			//val->_string.len = len;

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
	
	if (quote == '\'')
	{
		*str = get_quote_string(*str, *len, quote);
		*len = GB.StringLength(*str);
	}
}

/* Internal function to run a query */

static int do_query(DB_DATABASE *db, const char *error, PGresult **pres,
										const char *qtemp, int nsubst, ...)
{
	PGconn *conn = (PGconn *)db->handle;
	va_list args;
	int i;
	const char *query;
	PGresult *res;
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
	{
		fprintf(stderr, "postgresql: %p: %s\n", conn, query);
		fflush(stderr);
	}

	res = PQexec(conn, query);
	ret = check_result(res, error);
	if (!ret)
	{
		if (pres)
			*pres = res;
		else
			PQclear(res);
	}

	db->error = _last_error;
	return ret;
}

/* Internal function to check database version number */

static int db_version(DB_DATABASE *db)
{
	//Check db version
	const char *vquery =
		"select substring(version(),12,5)";
	int dbversion =0;
	PGresult *res;

	if (!do_query(db, NULL, &res, vquery, 0))
	{
		unsigned int verMain, verMajor, verMinor;
		sscanf(PQgetvalue(res, 0, 0),"%2u.%2u.%2u", &verMain, &verMajor, &verMinor);
		dbversion = ((verMain * 10000) + (verMajor * 100) + verMinor);
		PQclear(res);
	}
	return dbversion;
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
	<db> points at the DB_DATABASE structure that must be initialized.

	This function must return TRUE if the connection has failed.

	The name of the database can be NULL, meaning a default database.

*****************************************************************************/

static int open_database(DB_DESC *desc, DB_DATABASE *db)
{
	const char *query =
		"show client_encoding";

	PGconn *conn;
	PGresult *res;
	int status;
	char *name;

	if (desc->name)
		name = desc->name;
	else
		name = "template1";

	conn = PQsetdbLogin(desc->host, desc->port, NULL, NULL, name, desc->user, desc->password);

	if (!conn)
	{
		GB.Error("Out of memory");
		return TRUE;
	}

	if (PQstatus(conn) == CONNECTION_BAD)
	{
		GB.Error("Cannot open database: &1", PQerrorMessage(conn));
		PQfinish(conn);
		return TRUE;
	}

	res = PQexec(conn, "set datestyle=ISO");
	status = PQresultStatus(res);

	if (status != PGRES_COMMAND_OK)
	{
		GB.Error("Cannot set datestyle to ISO: &1", PQerrorMessage(conn));
		PQclear(res);
		PQfinish(conn);
		return TRUE;
	}

	/* get version */

	db->handle = conn;
	db->version = db_version(db);

	/* flags */

	db->flags.no_table_type = TRUE;
	db->flags.no_nest = TRUE;

	/* encoding */

	if (PQsetClientEncoding(conn, GB.System.Charset()))
		fprintf(stderr, "gb.db.postgresql: cannot set encoding to %s\n", GB.System.Charset());

	if (!do_query(db, NULL, &res, query, 0))
	{
		GB.NewString(&db->charset, PQgetvalue(res, 0, 0), 0);
		PQclear(res);
	}
	else
		db->charset = NULL;

	return FALSE;
}

/*****************************************************************************

	close_database()

	Terminates the database connection.

	<db> contains the database handle.

*****************************************************************************/

static void close_database(DB_DATABASE *db)
{
	PGconn *conn = (PGconn *)db->handle;

	if (conn)
		PQfinish(conn);
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
	bool bc;

	switch (arg->type)
	{
		case GB_T_BOOLEAN:

			if (VALUE((GB_BOOLEAN *)arg))
				add("TRUE", 4);
			else
				add("FALSE", 5);

			return TRUE;

		case GB_T_STRING:
		case GB_T_CSTRING:

			quote_string(VALUE((GB_STRING *)arg).addr + VALUE((GB_STRING *)arg).start, VALUE((GB_STRING *)arg).len, add);
			return TRUE;

		case GB_T_DATE:

			date = GB.SplitDate((GB_DATE *)arg);

			bc = date->year < 0;

			l = sprintf(_buffer, "'%04d-%02d-%02d %02d:%02d:%02d",
				abs(date->year), date->month, date->day,
				date->hour, date->min, date->sec);

			add(_buffer, l);

			if (date->msec)
			{
				l = sprintf(_buffer, ".%03d", date->msec);
				add(_buffer, l);
			}

			if (bc)
				add(" BC", 3);

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
	return do_query(db, err, (PGresult **)result, query, 0);
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
	PGresult *res = (PGresult *)result;

	*count = PQntuples(res);

	info->nfield = PQnfields(res);
}


/*****************************************************************************

	query_release()

	Free the info structure filled by query_init() and the result handle.

	<result> is the handle of the query result.
	<info> points to the info structure.

*****************************************************************************/

static void query_release(DB_RESULT result, DB_INFO *info)
{
	PQclear((PGresult *)result);
}


/*****************************************************************************

	query_fill()

	Fill a result buffer with the value of each field of a record.

	<db> is the database handle, as returned by open_database()
	<result> is the handle of the result.
	<pos> is the index of the record in the result.
	<buffer> points to an array having one element for each field in the
	result.

	This function must use GB.StoreVariant() to store the value in the
	buffer.

*****************************************************************************/

static int query_fill(DB_DATABASE *db, DB_RESULT result, int pos, GB_VARIANT_VALUE *buffer, int next)
{
	PGresult *res = (PGresult *)result;
	int i;
	char *data;
	GB_VARIANT value;

	for (i = 0; i < PQnfields(res); i++)
	{
		data = PQgetvalue(res, pos, i);

		value.type = GB_T_VARIANT;
		value.value.type = GB_T_NULL;

		if (!PQgetisnull(res, pos, i))
			conv_data(data, PQgetlength(res, pos, i), &value.value, PQftype(res, i));

		GB.StoreVariant(&value, &buffer[i]);
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

	NOTE: this function is always called after query_fill() with the same
	value of <pos>.

*****************************************************************************/

static void blob_read(DB_RESULT result, int pos, int field, DB_BLOB *blob)
{
	PGresult *res = (PGresult *)result;
	char *data;
	int len;

	data = PQgetvalue(res, pos, field);
	len = PQgetlength(res, pos, field);

	DB.Query.Init();
	if (!unquote_blob(data, len, DB.Query.AddLength))
	{
		len = DB.Query.Length();
		data = DB.Query.GetNew();
	}
	else
		blob->constant = TRUE;

	blob->data = data;
	blob->length = len;
}


/*****************************************************************************

	field_name()

	Return the name of a field in a result from its index.

	<result> is the result handle.
	<field> is the field index.

*****************************************************************************/

static char *field_name(DB_RESULT result, int field)
{
	return PQfname((PGresult *)result, field);
}


/*****************************************************************************

	field_index()

	Return the index of a field in a result from its name.

	<result> is the result handle.
	<name> is the field name.
	<db> is needed by this driver to enable table.field syntax

*****************************************************************************/

static int field_index(DB_RESULT Result, const char *name, DB_DATABASE *db)
{
	PGresult *result = (PGresult *)Result;

	char *fld;
	int index;
	char *table;
	int numfields, oid;
	char *qfield =
		"select oid from pg_class where relname = '&1' "
		"and (relnamespace not in (select oid from pg_namespace where nspname = 'information_schema'))";

	fld = strchr(name, (int)FLD_SEP);

	if (fld){
		//Includes table identity.
		//This feature is only available for 7.4 onwards
		/* check version */

		if (db->version > 70399){ // version 7.4.?
				fld[0] = '.';
				GB.NewString(&table, name, fld - name);
				fld = fld + 1;

				/* Need to find the OID for the table */
				PGresult *oidres;

				if (do_query(db, "Unable to get OID for table &1", &oidres, qfield, 1, table)){
			GB.FreeString(&table);
			return -1;
				}

				if ( PQntuples(oidres) != 1){
				/* Not unique table identifier */
					GB.Error("Table &1 not unique in pg_class", table);
					PQclear(oidres);
					GB.FreeString(&table);
					return -1;
				}

	oid = atoi(PQgetvalue(oidres, 0, 0));
				PQclear(oidres);
	numfields = PQnfields((PGresult *)result);
				index = PQfnumber((PGresult *)result, fld);

	if (PQftable((PGresult *)result, index) != oid){
		numfields = PQnfields((PGresult *)result);
		while ( ++index < numfields){
				if (strcmp(PQfname((PGresult *)result, index),
							fld) == 0){ //Check Fieldname
						if (PQftable((PGresult *)result, index) == oid){ //check oid
				break; // is the required table oid
						}
				}
		}

		if ( index == numfields ){
		/* field not found for OID */
							GB.Error("Field &1.&2 not found", table, fld);
				GB.FreeString(&table);
				return -1;
		}

	}
		GB.FreeString(&table);
		}
		else {
		/* Using tablename and fieldname in a non supported
		* version */
					GB.Error("Field &1.&2 not supported below 7.4.1", table, fld);
		return -1;
		}
	}
	else {
			index = PQfnumber((PGresult *)result, name);
	}

	return index;
}


/*****************************************************************************

	field_type()

	Return the Gambas type of a field in a result from its index.

	<result> is the result handle.
	<field> is the field index.

*****************************************************************************/

static GB_TYPE field_type(DB_RESULT result, int field)
{
	return conv_type(PQftype((PGresult *)result, field));
}


/*****************************************************************************

	field_length()

	Return the length of a field in a result from its index.

	<result> is the result handle.
	<field> is the field index.

*****************************************************************************/

static int field_length(DB_RESULT result, int field)
{
	GB_TYPE type = conv_type(PQftype((PGresult *)result, field));
	int len;

	if (type != GB_T_STRING)
		return 0;

	len = PQfmod((PGresult *)result, field);
	if (len < 0)
		len = 0;
	else
		len -= 4;

	return len;
}


/*****************************************************************************

	begin_transaction()

	Begin a transaction.

	<db> is the database handle.

	This function returns TRUE if the command has failed, and FALSE if
	everything was OK.

*****************************************************************************/

static int begin_transaction(DB_DATABASE *db)
{
	return do_query(db, "Unable to begin transaction: &1", NULL, "BEGIN", 0);
}


/*****************************************************************************

	commi_transaction()

	Commit a transaction.

	<db> is the database handle.

	This function returns TRUE if the command has failed, and FALSE if
	everything was OK.

*****************************************************************************/

static int commit_transaction(DB_DATABASE *db)
{
	return do_query(db, "Unable to commit transaction: &1", NULL, "COMMIT", 0);
}


/*****************************************************************************

	rollback_transaction()

	Rolllback a transaction.

	<db> is the database handle.

	This function returns TRUE if the command has failed, and FALSE if
	everything was OK.

*****************************************************************************/

static int rollback_transaction(DB_DATABASE *db)
{
	return do_query(db, "Unable to rollback transaction: &1", NULL, "ROLLBACK", 0);
}


/*****************************************************************************

	table_init()

	Initialize an info structure from table fields.

	<db> is the database handle.
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
	char *qfield =
		"select pg_attribute.attname, pg_attribute.atttypid::int,pg_attribute.atttypmod "
			"from pg_class, pg_attribute "
			"where pg_class.relname = '&1' "
				"and (pg_class.relnamespace not in (select oid from pg_namespace where nspname = 'information_schema')) "
				"and pg_attribute.attnum > 0 "
				"and pg_attribute.attrelid = pg_class.oid ";

	PGresult *res;
	int i, n;
	DB_FIELD *f;

	/* Nom de la table */

	GB.NewString(&info->table, table, 0);

	/* Liste des champs */

	if (do_query(db, "Unable to get table fields: &1", &res, qfield, 1, table))
		return TRUE;

	info->nfield = n = PQntuples(res);
	if (n == 0)
	{
		PQclear(res);
		return TRUE;
	}

	GB.Alloc(POINTER(&info->field), sizeof(DB_FIELD) * n);

	for (i = 0; i < n; i++)
	{
		f = &info->field[i];

		if (field_info(db, table, PQgetvalue(res, i, 0), f))
		{
			PQclear(res);
			return TRUE;
		}

		GB.NewString(&f->name, PQgetvalue(res, i, 0), 0);

		/*f->type = conv_type(atol(PQgetvalue(res, i, 1)));
		f->length = 0;
		if (f->type == GB_T_STRING)
		{
			f->length = atoi(PQgetvalue(res, i, 2));
			if (f->length < 0)
				f->length = 0;
			else
				f->length -= 4;
		}*/
	}

	PQclear(res);
	return FALSE;
}


/*****************************************************************************

	table_index()

	Initialize an info structure from table primary index.

	<db> is the database handle.
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
	const char *qindex;

	PGresult *res;
	int i, j, n;

	/* Index primaire */

	if (db->version < 80200)
	{
		qindex = "select pg_att1.attname, pg_att1.atttypid::int, pg_cl.relname "
			"from pg_attribute pg_att1, pg_attribute pg_att2, pg_class pg_cl, pg_index pg_ind "
				"where pg_cl.relname = '&1_pkey' AND pg_cl.oid = pg_ind.indexrelid "
					"and (pg_cl.relnamespace not in (select oid from pg_namespace where nspname = 'information_schema')) "
					"and pg_att2.attrelid = pg_ind.indexrelid "
					"and pg_att1.attrelid = pg_ind.indrelid "
					"and pg_att1.attnum = pg_ind.indkey[pg_att2.attnum-1] "
				"order by pg_att2.attnum";
	}
	else
	{
		qindex = "select pg_att1.attname, pg_att1.atttypid::int, pg_cl.relname "
		"from pg_attribute pg_att1, pg_attribute pg_att2, pg_class pg_cl, pg_index pg_ind, pg_class pg_table "
			"where pg_table.relname = '&1' AND pg_table.oid = pg_att1.attrelid AND pg_cl.oid = pg_ind.indexrelid "
					"and (pg_cl.relnamespace not in (select oid from pg_namespace where nspname = 'information_schema')) "
					"and pg_ind.indisprimary "
					"and pg_att2.attrelid = pg_ind.indexrelid "
					"and pg_att1.attrelid = pg_ind.indrelid "
					"and pg_att1.attnum = pg_ind.indkey[pg_att2.attnum-1] "
				"order by pg_att2.attnum";
	}

	if (do_query(db, "Unable to get primary index: &1", &res, qindex, 1, table))
		return TRUE;

	n = info->nindex = PQntuples(res);

	if (n <= 0)
	{
		GB.Error("Table '&1' has no primary index", table);
		PQclear(res);
		return TRUE;
	}

	GB.Alloc(POINTER(&info->index), sizeof(int) * n);

	for (i = 0; i < n; i++)
	{
		for (j = 0; j < info->nfield; j++)
		{
			if (strcmp(info->field[j].name, PQgetvalue(res, i, 0)) == 0)
			{
				info->index[i] = j;
				/*printf("index[%d] = %d\n", i, j);*/
				break;
			}
		}
	}

	PQclear(res);

	return FALSE;
}


/*****************************************************************************

	table_release()

	Free the info structure filled by table_init() and/or table_index()

	<db> is the database handle.
	<info> points at the info structure.

*****************************************************************************/

static void table_release(DB_DATABASE *db, DB_INFO *info)
{
	/* All is done outside the driver */
}


/*****************************************************************************

	table_exist()

	Returns if a table exists

	<db> is the database handle.
	<table> is the table name.

	This function returns TRUE if the table exists, and FALSE if not.

*****************************************************************************/

static int table_exist(DB_DATABASE *db, const char *table)
{
	const char *query73 =
		"select relname from pg_class where (relkind = 'r' or relkind = 'v') "
		"and (relname = '&1') "
		"and (relnamespace not in (select oid from pg_namespace where nspname = 'information_schema'))";

	/*const char *query74_1 =
		"select tablename from pg_tables where schemaname <> 'information_schema' and tablename = lower('&1')";

	const char *query74_2 =
		"select viewname from pg_views where schemaname <> 'information_schema' and viewname = lower('&1')";*/

	PGresult *res;
	int exist;

	//if (1) //version < 70400)
	//{
		if (do_query(db, "Unable to check table: &1", &res, query73, 1, table))
			return FALSE;
	/*}
	else
	{
		if (do_query(conn, "Unable to check table: &1", &res, query74_1, 1, table))
			return TRUE;

		if (PQntuples(res) == 0)
		{
			PQclear(res);
			if (do_query(conn, "Unable to check table: &1", &res, query74_2, 1, table))
				return TRUE;
		}
	}*/

	exist = PQntuples(res) == 1;

	PQclear(res);

	return exist;
}



/*****************************************************************************

	table_list()

	Returns an array containing the name of each table in the database

	<db> is the database handle.
	<tables> points to a variable that will receive the char* array.

	This function returns the number of tables, or -1 if the command has
	failed.

	Be careful: <tables> can be NULL, so that just the count is returned.

*****************************************************************************/

static int table_list_73(DB_DATABASE *db, char ***tables)
{
	const char *query =
		"select relname from pg_class where (relkind = 'r' or relkind = 'v') "
		"and (relnamespace not in (select oid from pg_namespace where nspname = 'information_schema'))";

	PGresult *res;
	int i;
	int count;

	if (do_query(db, "Unable to get tables: &1", &res, query, 0))
		return -1;

	if (tables)
	{
		GB.NewArray(tables, sizeof(char *), PQntuples(res));

		for (i = 0; i < PQntuples(res); i++)
			GB.NewString(&((*tables)[i]), PQgetvalue(res, i, 0), 0);
	}

	count = PQntuples(res);

	PQclear(res);

	return count;
}

#if 0
static int table_list_74(DB_DATABASE handle, char ***tables)
{
	const char *query1 =
		"select tablename from pg_tables where schemaname <> 'information_schema'";

	const char *query2 =
		"select viewname from pg_views where schemaname <> 'information_schema'";

	PGconn *conn = (PGconn *)handle;
	PGresult *res1, *res2;
	int i, j;
	int count;

	if (do_query(conn, "Unable to get tables: &1", &res1, query1, 0))
		return -1;

	if (do_query(conn, "Unable to get tables: &1", &res2, query2, 0))
		return -1;

	count = PQntuples(res1) + PQntuples(res2);

	if (tables)
	{
		GB.NewArray(tables, sizeof(char *), count);

		j = 0;

		for (i = 0; i < PQntuples(res1); i++, j++)
			GB.NewString(&((*tables)[j]), PQgetvalue(res1, i, 0), 0);

		for (i = 0; i < PQntuples(res2); i++, j++)
			GB.NewString(&((*tables)[j]), PQgetvalue(res2, i, 0), 0);
	}

	PQclear(res1);
	PQclear(res2);

	return count;
}
#endif

static int table_list(DB_DATABASE *db, char ***tables)
{
	//if (version < 70400)
		return table_list_73(db, tables);
	//else
	//  return table_list_74(handle, tables);
}



/*****************************************************************************

	table_primary_key()

	Returns a string representing the primary key of a table.

	<db> is the database handle.
	<table> is the table name.
	<key> points to a string that will receive the primary key.

	This function returns TRUE if the command has failed, and FALSE if
	everything was OK.

*****************************************************************************/

static int table_primary_key(DB_DATABASE *db, const char *table, char ***primary)
{
	const char *query;
	PGresult *res;
	int i;

	if (db->version < 80200)
	{
		query = "select pg_att1.attname, pg_att1.atttypid::int, pg_cl.relname "
			"from pg_attribute pg_att1, pg_attribute pg_att2, pg_class pg_cl, pg_index pg_ind "
				"where pg_cl.relname = '&1_pkey' AND pg_cl.oid = pg_ind.indexrelid "
					"and (pg_cl.relnamespace not in (select oid from pg_namespace where nspname = 'information_schema')) "
					"and pg_att2.attrelid = pg_ind.indexrelid "
					"and pg_att1.attrelid = pg_ind.indrelid "
					"and pg_att1.attnum = pg_ind.indkey[pg_att2.attnum-1] "
				"order by pg_att2.attnum";
	}
	else
	{
		query = "select pg_att1.attname, pg_att1.atttypid::int, pg_cl.relname "
			"from pg_attribute pg_att1, pg_attribute pg_att2, pg_class pg_cl, pg_index pg_ind, pg_class pg_table "
				"where pg_table.relname = '&1' AND pg_table.oid = pg_att1.attrelid AND pg_cl.oid = pg_ind.indexrelid "
					"and (pg_cl.relnamespace not in (select oid from pg_namespace where nspname = 'information_schema')) "
					"and pg_ind.indisprimary "
					"and pg_att2.attrelid = pg_ind.indexrelid "
					"and pg_att1.attrelid = pg_ind.indrelid "
					"and pg_att1.attnum = pg_ind.indkey[pg_att2.attnum-1] "
				"order by pg_att2.attnum";
	}

	if (do_query(db, "Unable to get primary key: &1", &res, query, 1, table))
		return TRUE;

	GB.NewArray(primary, sizeof(char *), PQntuples(res));

	for (i = 0; i < PQntuples(res); i++)
		GB.NewString(&((*primary)[i]), PQgetvalue(res, i, 0), 0);

	PQclear(res);

	return FALSE;
}


/*****************************************************************************

	table_is_system()

	Returns if a table is a system table.

	<db> is the database handle.
	<table> is the table name.

	This function returns TRUE if the table is a system table, and FALSE if
	not.

*****************************************************************************/

static int table_is_system(DB_DATABASE *db, const char *table)
{
	const char *query =
		"select 1 from pg_class where (relkind = 'r' or relkind = 'v') "
		"and (relname = '&1') "
		"and (relnamespace not in (select oid from pg_namespace where nspname <> 'public'))";

	//const char *query1 =
	//  "select 1 from pg_tables where tablename = lower('&1') and schemaname = 'public'";

	const char *query2 =
		"select 1 from pg_views where viewname = '&1' and schemaname = 'public'";

	PGresult *res;
	int exist;

	/* check version */
	/*if (version < 70400){ // version 7.3.?
		if (table[0] == 'p' && table[1] == 'g' && table[2] == '_')
				return TRUE;
		else
				return FALSE;
	}*/

	if (do_query(db, "Unable to check table: &1", &res, query, 1, table))
		return FALSE;

	exist = PQntuples(res) == 1;
	PQclear(res);

	if (exist)
		return FALSE;

	if (do_query(db, "Unable to check table: &1", &res, query2, 1, table))
		return FALSE;

	exist = PQntuples(res) == 1;
	PQclear(res);

	if (exist)
		return FALSE;

	return TRUE;
}


/*****************************************************************************

	table_delete()

	Deletes a table.

	<db> is the database handle.
	<table> is the table name.

	This function returns TRUE if the command has failed, and FALSE if
	everything was OK.

*****************************************************************************/

static int table_delete(DB_DATABASE *db, const char *table)
{
	return
		do_query(db, "Unable to delete table: &1", NULL,
			"drop table \"&1\"", 1, table);
}


/*****************************************************************************

	table_create()

	Creates a table.

	<db> is the database handle.
	<table> is the table name.
	<fields> points to a linked list of field descriptions.
	<key> is the primary key.

	This function returns TRUE if the command has failed, and FALSE if
	everything was OK.

*****************************************************************************/

static int table_create(DB_DATABASE *db, const char *table, DB_FIELD *fields, char **primary, const char *not_used)
{
	DB_FIELD *fp;
	int comma;
	char *type;
	int i;

	DB.Query.Init();

	DB.Query.Add("CREATE TABLE \"");
	DB.Query.Add(table);
	DB.Query.Add("\" ( ");

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
			DB.Query.Add(" BIGSERIAL ");
		}
		else if (fp->type == DB_T_BLOB)
		{
			DB.Query.Add(" BYTEA ");
		}
		else
		{
			switch (fp->type)
			{
				case GB_T_BOOLEAN: type = "BOOL"; break;
				case GB_T_INTEGER: type = "INT"; break;
				case GB_T_LONG: type = "BIGINT"; break;
				case GB_T_FLOAT: type = "FLOAT8"; break;
				case GB_T_DATE: type = "TIMESTAMP"; break;
				case GB_T_STRING:

					if (fp->length <= 0)
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

	if (primary)
	{
		DB.Query.Add(", PRIMARY KEY (");

		for (i = 0; i < GB.Count(primary); i++)
		{
			if (i > 0)
				DB.Query.Add(",");

			DB.Query.Add(primary[i]);
		}

		DB.Query.Add(")");
	}

	DB.Query.Add(" )");

	return do_query(db, "Cannot create table: &1", NULL, DB.Query.Get(), 0);
}


/*****************************************************************************

	field_exist()

	Returns if a field exists in a given table

	<db> is the database handle.
	<table> is the table name.
	<field> is the field name.

	This function returns TRUE if the field exists, and FALSE if not.

*****************************************************************************/

static int field_exist(DB_DATABASE *db, const char *table, const char *field)
{
	const char *query = "select pg_attribute.attname from pg_class, pg_attribute "
		"where pg_class.relname = '&1' "
		"and (pg_class.relnamespace not in (select oid from pg_namespace where nspname = 'information_schema')) "
		"and pg_attribute.attname = '&2' "
		"and pg_attribute.attnum > 0 "
		"and pg_attribute.attrelid = pg_class.oid ";

	PGresult *res;
	int exist;

	if (do_query(db, "Unable to check field: &1", &res, query, 2, table, field))
		return FALSE;

	exist = PQntuples(res) == 1;

	PQclear(res);

	return exist;
}


/*****************************************************************************

	field_list()

	Returns an array containing the name of each field in a given table

	<db> is the database handle.
	<table> is the table name.
	<fields> points to a variable that will receive the char* array.

	This function returns the number of fields, or -1 if the command has
	failed.

	Be careful: <fields> can be NULL, so that just the count is returned.

*****************************************************************************/

static int field_list(DB_DATABASE *db, const char *table, char ***fields)
{
	const char *query = "select pg_attribute.attname from pg_class, pg_attribute "
		"where pg_class.relname = '&1' "
		"and (pg_class.relnamespace not in (select oid from pg_namespace where nspname = 'information_schema')) "
		"and pg_attribute.attnum > 0 "
		"and pg_attribute.attrelid = pg_class.oid";

	PGresult *res;
	int i;
	int count;

	if (do_query(db, "Unable to get fields: &1", &res, query, 1, table))
		return -1;

	if (fields)
	{
		GB.NewArray(fields, sizeof(char *), PQntuples(res));

		for (i = 0; i < PQntuples(res); i++)
			GB.NewString(&((*fields)[i]), PQgetvalue(res, i, 0), 0);
	}

	count = PQntuples(res);

	PQclear(res);
	return count;
}


/*****************************************************************************

	field_info()

	Get field description

	<db> is the database handle.
	<table> is the table name.
	<field> is the field name.
	<info> points to a structure filled by the function.

	This function returns TRUE if the command has failed, and FALSE if
	everything was OK.

*****************************************************************************/

static int field_info(DB_DATABASE *db, const char *table, const char *field, DB_FIELD *info)
{
	const char *query =
		"select pg_attribute.attname, pg_attribute.atttypid::int, "
		"pg_attribute.atttypmod, pg_attribute.attnotnull, pg_attrdef.adsrc "
		"from pg_class, pg_attribute "
		"left join pg_attrdef on (pg_attrdef.adrelid = pg_attribute.attrelid and pg_attrdef.adnum = pg_attribute.attnum) "
		"where pg_class.relname = '&1' "
		"and (pg_class.relnamespace not in (select oid from pg_namespace where nspname = 'information_schema')) "
		"and pg_attribute.attname = '&2' "
		"and pg_attribute.attnum > 0 "
		"and pg_attribute.attrelid = pg_class.oid";

	PGresult *res;
	Oid type;
	GB_VARIANT def;
	char *val;

	if (do_query(db, "Unable to get field info: &1", &res, query, 2, table, field))
		return TRUE;

	if (PQntuples(res) != 1)
	{
		GB.Error("Unable to find field &1.&2", table, field);
		return TRUE;
	}

	info->name = NULL;

	type = atoi(PQgetvalue(res, 0, 1));
	info->type = conv_type(type);

	info->length = 0;
	if (info->type == GB_T_STRING)
	{
		info->length = atoi(PQgetvalue(res, 0, 2));
		if (info->length < 0)
			info->length = 0;
		else
			info->length -= 4;
	}

	info->def.type = GB_T_NULL;

	if (conv_boolean(PQgetvalue(res, 0, 3)))
	{
		def.type = GB_T_VARIANT;
		def.value.type = GB_T_NULL;

		val = PQgetvalue(res, 0, 4);
		if (val && *val)
		{
			if (strncmp(val, "nextval(", 8) == 0)
			{
				if (info->type == GB_T_LONG)
					info->type = DB_T_SERIAL;
			}
			else
			{
				switch(info->type)
				{
					case GB_T_BOOLEAN:
						def.value.type = GB_T_BOOLEAN;
						def.value.value._boolean = (val[1] == 't');
						break;

					default:

						DB.Query.Init();
						if (!unquote_string(val, PQgetlength(res, 0, 4), DB.Query.AddLength))
							val = DB.Query.Get();

						conv_data(val, -1, &def.value, type);
				}

				GB.StoreVariant(&def, &info->def);
			}

		}
	}

	PQclear(res);
	return FALSE;
}


/*****************************************************************************

	index_exist()

	Returns if an index exists in a given table

	<db> is the database handle.
	<table> is the table name.
	<field> is the index name.

	This function returns TRUE if the index exists, and FALSE if not.

*****************************************************************************/

static int index_exist(DB_DATABASE *db, const char *table, const char *index)
{
	const char *query =
		"select pg_class.relname from pg_class, pg_index, pg_class pg_class2 "
		"where pg_class2.relname = '&1' "
		"and (pg_class2.relnamespace not in (select oid from pg_namespace where nspname = 'information_schema')) "
		"and pg_index.indrelid = pg_class2.oid "
		"and pg_index.indexrelid = pg_class.oid "
		"and pg_class.relname = '&2'";

	/*const char *query = "select relname from pg_class, pg_index "
		"where pg_class.relname = '&1' "
		"and pg_index.indexrelid = pg_class.oid ";*/

	PGresult *res;
	int exist;

	if (do_query(db, "Unable to check index: &1", &res, query, 2, table, index))
		return FALSE;

	exist = PQntuples(res) == 1;

	PQclear(res);

	return exist;
}


/*****************************************************************************

	index_list()

	Returns an array containing the name of each index in a given table

	<db> is the database handle.
	<table> is the table name.
	<indexes> points to a variable that will receive the char* array.

	This function returns the number of indexes, or -1 if the command has
	failed.

	Be careful: <indexes> can be NULL, so that just the count is returned.

*****************************************************************************/

static int index_list(DB_DATABASE *db, const char *table, char ***indexes)
{
	const char *query = "select pg_class.relname from pg_class, pg_index, pg_class pg_class2 "
		"where pg_class2.relname = '&1' "
		"and (pg_class2.relnamespace not in (select oid from pg_namespace where nspname = 'information_schema')) "
		"and pg_index.indrelid = pg_class2.oid "
		"and pg_index.indexrelid = pg_class.oid ";

	PGresult *res;
	int i;
	int count;

	if (do_query(db, "Unable to get indexes: &1", &res, query, 1, table))
		return TRUE;

	if (indexes)
	{
		GB.NewArray(indexes, sizeof(char *), PQntuples(res));

		for (i = 0; i < PQntuples(res); i++)
			GB.NewString(&((*indexes)[i]), PQgetvalue(res, i, 0), 0);
	}

	count = PQntuples(res);
	PQclear(res);

	return count;
}


/*****************************************************************************

	index_info()

	Get index description

	<db> is the database handle.
	<table> is the table name.
	<field> is the index name.
	<info> points to a structure filled by the function.

	This function returns TRUE if the command has failed, and FALSE if
	everything was OK.

*****************************************************************************/

static int index_info(DB_DATABASE *db, const char *table, const char *index, DB_INDEX *info)
{
	const char *query =
		"select indisunique, indisprimary, indexrelid from pg_class, pg_index, pg_class pg_class2 "
		"where pg_class2.relname = '&1' "
		"and (pg_class2.relnamespace not in (select oid from pg_namespace where nspname = 'information_schema')) "
		"and pg_index.indrelid = pg_class2.oid "
		"and pg_index.indexrelid = pg_class.oid "
		"and pg_class.relname = '&2'";

	const char *query_field =
		"select pg_att1.attname "
			"from pg_attribute pg_att1, pg_attribute pg_att2, pg_index pg_ind "
				"where pg_ind.indexrelid = &1 "
					"and pg_att2.attrelid = pg_ind.indexrelid "
					"and pg_att1.attrelid = pg_ind.indrelid "
					"and pg_att1.attnum = pg_ind.indkey[pg_att2.attnum-1] "
				"order by pg_att2.attnum";

	PGresult *res;
	char indexrelid[16];
	int i;

	if (do_query(db, "Unable to get index info: &1", &res, query, 2, table, index))
		return TRUE;

	if (PQntuples(res) != 1)
	{
		GB.Error("Unable to find index &1.&2", table, index);
		return TRUE;
	}

	info->name = NULL;
	info->unique = conv_boolean(PQgetvalue(res, 0, 0));
	info->primary = conv_boolean(PQgetvalue(res, 0, 1));

	strcpy(indexrelid, PQgetvalue(res, 0, 2));

	PQclear(res);

	if (do_query(db, "Unable to get index info: &1", &res, query_field, 1, indexrelid))
		return TRUE;

	DB.Query.Init();

	for (i = 0; i < PQntuples(res); i++)
	{
		if (i > 0)
			DB.Query.Add(",");
		DB.Query.Add(PQgetvalue(res, i, 0));
	}

	PQclear(res);

	info->fields = DB.Query.GetNew();

	return FALSE;
}


/*****************************************************************************

	index_delete()

	Deletes an index.

	<db> is the database handle.
	<table> is the table name.
	<index> is the index name.

	This function returns TRUE if the command has failed, and FALSE if
	everything was OK.

*****************************************************************************/

static int index_delete(DB_DATABASE *db, const char *table, const char *index)
{
	return
		do_query(db, "Unable to delete index: &1", NULL,
			"drop index \"&1\"", 1, index);
}


/*****************************************************************************

	index_create()

	Creates an index.

	<db> is the database handle.
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
	DB.Query.Add("INDEX \"");
	DB.Query.Add(index);
	DB.Query.Add("\" ON ");
	DB.Query.Add(table);
	DB.Query.Add(" ( ");
	DB.Query.Add(info->fields);
	DB.Query.Add(" )");

	return do_query(db, "Cannot create index: &1", NULL, DB.Query.Get(), 0);
}


/*****************************************************************************

	database_exist()

	Returns if a database exists

	<db> is the database handle.
	<name> is the database name.

	This function returns TRUE if the database exists, and FALSE if not.

*****************************************************************************/

static int database_exist(DB_DATABASE *db, const char *name)
{
	const char *query =
		"select datname from pg_database where (datallowconn = 't') "
		"and (datname = '&1')";

	PGresult *res;
	int exist;

	if (do_query(db, "Unable to check database: &1", &res, query, 1, name))
		return FALSE;

	exist = PQntuples(res) == 1;

	PQclear(res);

	return exist;
}



/*****************************************************************************

	database_list()

	Returns an array containing the name of each database

	<db> is the database handle.
	<databases> points to a variable that will receive the char* array.

	This function returns the number of databases, or -1 if the command has
	failed.

	Be careful: <databases> can be NULL, so that just the count is returned.

*****************************************************************************/

static int database_list(DB_DATABASE *db, char ***databases)
{
	const char *query =
		"select datname from pg_database where datallowconn and datname <> 'template1'";

	PGresult *res;
	int i;
	int count;

	if (do_query(db, "Unable to get databases: &1", &res, query, 0))
		return -1;

	if (databases)
	{
		GB.NewArray(databases, sizeof(char *), PQntuples(res));

		for (i = 0; i < PQntuples(res); i++)
			GB.NewString(&((*databases)[i]), PQgetvalue(res, i, 0), 0);
	}

	count = PQntuples(res);

	PQclear(res);

	return count;
}


/*****************************************************************************

	database_is_system()

	Returns if a database is a system database.

	<db> is the database handle.
	<name> is the database name.

	This function returns TRUE if the database is a system database, and
	FALSE if not.

*****************************************************************************/

static int database_is_system(DB_DATABASE *db, const char *name)
{
	const char *query =
		"select datname from pg_database where datallowconn "
		"and (datname = '&1') and datistemplate";
		//"and (datname = lower('&1')) and datistemplate";

	PGresult *res;
	int system;

	if (do_query(db, "Unable to check database: &1", &res, query, 1, name))
		return TRUE;

	system = PQntuples(res) == 1;

	PQclear(res);

	return system;
}

/*****************************************************************************

	table_type()

	Returns or sets the table type.

	<db> is the database handle.
	<table> is the table name.
	<newtype> is the new type

	This function returns the table type.

	NOTE: Only mySQL seems to have table types at the moment.

*****************************************************************************/

static char *table_type(DB_DATABASE *db, const char *table, const char *newtype)
{
	if (newtype)
		GB.Error("PostgreSQL does not have any table types");

	return NULL;
}

/*****************************************************************************

	database_delete()

	Deletes a database.

	<db> is the database handle.
	<name> is the database name.

	This function returns TRUE if the command has failed, and FALSE if
	everything was OK.

*****************************************************************************/

static int database_delete(DB_DATABASE *db, const char *name)
{
	return
		do_query(db, "Unable to delete database: &1", NULL,
			"drop database \"&1\"", 1, name);
}


/*****************************************************************************

	database_create()

	Creates a database.

	<db> is the database handle.
	<name> is the database name.

	This function returns TRUE if the command has failed, and FALSE if
	everything was OK.

*****************************************************************************/

static int database_create(DB_DATABASE *db, const char *name)
{
	return
		do_query(db, "Unable to create database: &1", NULL,
			"create database \"&1\"", 1, name);
}


/*****************************************************************************

	user_exist()

	Returns if a user exists.

	<db> is the database handle.
	<name> is the user name.

	This function returns TRUE if the user exists, and FALSE if not.

*****************************************************************************/

static int user_exist(DB_DATABASE *db, const char *name)
{
	const char *query = "select usename from pg_user "
		"where usename = '&1' ";

	PGresult *res;
	int exist;

	if (do_query(db, "Unable to check user: &1", &res, query, 1, name))
		return FALSE;

	exist = PQntuples(res) == 1;

	PQclear(res);

	return exist;
}


/*****************************************************************************

	user_list()

	Returns an array containing the name of each user.

	<db> is the database handle.
	<users> points to a variable that will receive the char* array.

	This function returns the number of users, or -1 if the command has
	failed.

	Be careful: <users> can be NULL, so that just the count is returned.

*****************************************************************************/

static int user_list(DB_DATABASE *db, char ***users)
{
	const char *query = "select usename from pg_user ";

	PGresult *res;
	int i;
	int count;

	if (do_query(db, "Unable to get users: &1", &res, query, 0))
		return -1;

	if (users)
	{
		GB.NewArray(users, sizeof(char *), PQntuples(res));

		for (i = 0; i < PQntuples(res); i++)
			GB.NewString(&((*users)[i]), PQgetvalue(res, i, 0), 0);
	}

	count = PQntuples(res);

	PQclear(res);
	return count;
}


/*****************************************************************************

	user_info()

	Get user description

	<db> is the database handle.
	<name> is the user name.
	<info> points to a structure filled by the function.

	This function returns TRUE if the command has failed, and FALSE if
	everything was OK.

*****************************************************************************/

static int user_info(DB_DATABASE *db, const char *name, DB_USER *info)
{
	const char *query =
		"select usecreatedb, usesuper from pg_user "
		"where usename = '&1' ";

	const char *query_passwd =
		"select passwd from pg_shadow "
		"where usename = '&1' ";

	PGresult *res;

	if (do_query(db, "Unable to get user info: &1", &res, query, 1, name))
		return TRUE;

	if (PQntuples(res) != 1)
	{
		GB.Error("Unable to find user &1", name);
		return TRUE;
	}

	info->name = NULL;
	info->admin = conv_boolean(PQgetvalue(res, 0, 1));
	/* conv_boolean(PQgetvalue(res, 0, 0)) || */

	PQclear(res);

	if (!do_query(db, NULL, &res, query_passwd, 1, name))
	{
		if (*PQgetvalue(res, 0, 0))
			GB.NewString(&info->password, "***", 0);
	}

	return FALSE;
}


/*****************************************************************************

	user_delete()

	Deletes a user.

	<db> is the database handle.
	<name> is the user name.

	This function returns TRUE if the command has failed, and FALSE if
	everything was OK.

*****************************************************************************/

static int user_delete(DB_DATABASE *db, const char *name)
{
	return
		do_query(db, "Unable to delete user: &1", NULL,
			"drop user \"&1\"", 1, name);
}


/*****************************************************************************

	user_create()

	Creates a user.

	<db> is the database handle.
	<name> is the user name.
	<info> points to a structure describing the user.

	This function returns TRUE if the command has failed, and FALSE if
	everything was OK.

*****************************************************************************/

static int user_create(DB_DATABASE *db, const char *name, DB_USER *info)
{
	DB.Query.Init();

	DB.Query.Add("CREATE USER ");
	DB.Query.Add(QUOTE_STRING);
	DB.Query.Add(name);
	DB.Query.Add(QUOTE_STRING);

	if (info->admin)
		DB.Query.Add(" CREATEDB CREATEUSER");
	else
		DB.Query.Add(" NOCREATEDB NOCREATEUSER");

	if (info->password && *info->password)
	{
		DB.Query.Add(" PASSWORD '");
		DB.Query.Add(info->password);
		DB.Query.Add("'");
	}

	/*_print_query = TRUE;*/
	return do_query(db, "Cannot create user: &1", NULL, DB.Query.Get(), 0);
}


/*****************************************************************************

	user_set_password()

	Change the user password.

	<db> is the database handle.
	<name> is the user name.
	<password> is the new password

	This function returns TRUE if the command has failed, and FALSE if
	everything was OK.

*****************************************************************************/

static int user_set_password(DB_DATABASE *db, const char *name, const char *password)
{
	DB.Query.Init();

	DB.Query.Add("ALTER USER \"");
	DB.Query.Add(name);
	DB.Query.Add("\" PASSWORD '");
	DB.Query.Add(password);
	DB.Query.Add("'");

	return
		do_query(db, "Cannot change user password: &1",
			NULL, DB.Query.Get(), 0);
}

/*****************************************************************************

	The driver interface

*****************************************************************************/

DECLARE_DRIVER(_driver, "postgresql");

/*****************************************************************************

	The component entry and exit functions.

*****************************************************************************/

int EXPORT GB_INIT(void)
{
	GB.GetInterface("gb.db", DB_INTERFACE_VERSION, &DB);
	DB.Register(&_driver);

	return FALSE;
}

void EXPORT GB_EXIT()
{
}
