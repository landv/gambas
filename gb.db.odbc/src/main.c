/***************************************************************************

  main.c

  ODBC (unixODBC) driver
  (c) 2004-2005 Andrea Bortolan <andrea_bortolan@yahoo.it>
  (c) 2000-2003 Benoît Minisini <gambas@users.sourceforge.net>

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
#include <errno.h>
#include <math.h>

#include <sql.h>
#include <sqlext.h>
#include <sqltypes.h>

#include "main.h"


GB_INTERFACE GB EXPORT;
DB_INTERFACE DB EXPORT;

static char _buffer[32];
static DB_DRIVER _driver;

typedef struct
	{
		SQLHENV V_OD_Env;
		SQLHDBC V_OD_hdbc;
		SQLUSMALLINT FetchScroll_exist;
		char *dsn_name;
		char *user_name;
	}
ODBC_CONN;


typedef struct
	{
		SQLCHAR fieldname[32];
		int fieldid;
		SQLSMALLINT type;
		SQLINTEGER outlen;
		SQLCHAR *fieldata;
		struct ODBC_FIELDS *next;
		//struct ODBC_FIELDS *prev;
	}
ODBC_FIELDS;


typedef struct
	{
		SQLHSTMT V_OD_hstmt;
		SQLUSMALLINT Function_exist;	//Does the Driver supports the SQLFetchScroll ?
		SQLUSMALLINT Cursor_Scrollable;  //Is is possible to set a Scrollable cursor ?
		ODBC_FIELDS *fields;
	}
ODBC_RESULT;


typedef struct
	{
		char *tablename;
		struct ODBC_TABLES *next;
	}
ODBC_TABLES;



/* BM: Replaces malloc() and free() by GB.Alloc() and GB.Free() */

static void *my_malloc(size_t size)
{
	void *ptr;
	GB.Alloc(&ptr, size);
	return ptr;
}

#define malloc(_size) my_malloc(_size)
#define free(_ptr) GB.Free((void **)&_ptr)


/* Internal function to convert a database type into a Gambas type */

static GB_TYPE conv_type(int type)
{

	switch (type)
	{
		case SQL_BINARY:
			return GB_T_BOOLEAN;

			/*case INT8OID: */
		case SQL_NUMERIC:
		case SQL_DECIMAL:
		case SQL_INTEGER:
		case SQL_SMALLINT:
			return GB_T_INTEGER;

// New datatype bigint 64 bits
		case SQL_BIGINT:
			return GB_T_LONG;

		case SQL_FLOAT:
		case SQL_REAL:
		case SQL_DOUBLE:
			return GB_T_FLOAT;

		case SQL_DATETIME:
		case SQL_TYPE_DATE:
		case SQL_TYPE_TIME:
		case SQL_TYPE_TIMESTAMP:
			return GB_T_DATE;

		case SQL_CHAR:
		default:
			return GB_T_STRING;

	}
}


/* Internal function to convert a database value into a Gambas variant value */

static void conv_data(char *data, GB_VARIANT_VALUE * val, int type)
{
	GB_VALUE conv;
	GB_DATE_SERIAL date;
	double sec;
	long len;
	int bc = 0;

	switch (type)
	{


		case SQL_NUMERIC:
		case SQL_DECIMAL:
		case SQL_INTEGER:
		case SQL_SMALLINT:
			//printf("data : %s, lungo %u\n",data,strlen(data));
			/*  if(strlen(data) == 0) {
			   data[0]='[';
			   data[1]='N';
			   data[2]='U';
			   data[3]='L';
			   data[4]='L';
			   data[5]=']';
			   data[6]='\0';

			   } */
			//printf("data : %s, lungo %u\n",data,strlen(data));

			GB.NumberFromString(GB_NB_READ_INTEGER, data, strlen(data), &conv);
			val->_integer.type = GB_T_INTEGER;
			val->_integer.value = ((GB_INTEGER *) & conv)->value;
			if (strlen(data) == 0)
				val->_integer.value = 0;	//NULL;


			break;

		case SQL_FLOAT:
		case SQL_REAL:
		case SQL_DOUBLE:

			GB.NumberFromString(GB_NB_READ_FLOAT, data, strlen(data), &conv);
			val->_float.type = GB_T_FLOAT;
			val->_float.value = ((GB_FLOAT *) & conv)->value;

			break;

// New data type bigint 64 bits
		case SQL_BIGINT:

			GB.NumberFromString(GB_NB_READ_LONG, data, strlen(data), &conv);

			val->type = GB_T_LONG;
			val->_long.value = ((GB_LONG *) & conv)->value;

			break;


		case SQL_TYPE_DATE:
		case SQL_TYPE_TIME:
		case SQL_TYPE_TIMESTAMP:
		case SQL_DATETIME:
			{
				//case FIELD_TYPE_TIMESTAMP:

				memset(&date, 0, sizeof(date));

				len = strlen(data);
				if (len > 3 && strcmp(&data[len - 2], "BC") == 0)
					bc != 0;
				else
					bc = 0;

				sscanf(data, "%4hu-%2hu-%2hu %2hu:%2hu:%lf", &date.year, &date.month,
							 &date.day, &date.hour, &date.min, &sec);
				date.sec = (short) sec;
				date.msec = (short) ((sec - date.sec) * 1000 + 0.5);
				//break;


				if (bc)
					date.year = (-date.year);

				GB.MakeDate(&date, (GB_DATE *) & conv);

				val->_date.type = GB_T_DATE;
				val->_date.date = ((GB_DATE *) & conv)->value.date;
				val->_date.time = ((GB_DATE *) & conv)->value.time;
				break;
			}



		case SQL_CHAR:
		default:

			//val->_string.type = GB_T_STRING;
			//GB.NewString(&val->_string.value, data, 0);
			val->_string.type = GB_T_CSTRING;
			val->_string.value = data;

			break;
	}

}



/*****************************************************************************

  get_quote()

  Returns the character used for quoting object names.

*****************************************************************************/

static char *get_quote(void)
{
	return QUOTE_STRING;
}

/*****************************************************************************

  open_database()

  Connect to a database.

  <desc> points at a structure describing each connection parameter.

  This function must return a database handle, or NULL if the connection
  has failed.

  The name of the database can be NULL, meaning a default database.

*****************************************************************************/

/* Internal function to allocate the ODBC handle */
static ODBC_CONN *SQL_Handle(void)
{
	return malloc(sizeof(ODBC_CONN));
}

void SQL_Handle_free(ODBC_CONN * ptr)
{
	free(ptr);
}


static int open_database(DB_DESC * desc, DB_DATABASE *db)
{
	long V_OD_erg;
	ODBC_CONN *odbc;

	/* Allocate the ODBC handle */
	odbc = SQL_Handle();
	odbc->V_OD_hdbc = NULL;
	/* Allocate the Environment handle */
	V_OD_erg = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &odbc->V_OD_Env);
	if ((V_OD_erg != SQL_SUCCESS) && (V_OD_erg != SQL_SUCCESS_WITH_INFO))
	{
		free(odbc);
		GB.Error("Unable to allocate the environment handle");
		return TRUE;
	}

	/* Set the Envoronment attributes */
	V_OD_erg =
		SQLSetEnvAttr(odbc->V_OD_Env, SQL_ATTR_ODBC_VERSION,
									(void *) SQL_OV_ODBC3, 0);

	if ((V_OD_erg != SQL_SUCCESS) && (V_OD_erg != SQL_SUCCESS_WITH_INFO))
	{
		SQLFreeHandle(SQL_HANDLE_ENV, odbc->V_OD_Env);
		free(odbc);
		GB.Error("Unable to set the environment attributes");
		return TRUE;
	}

	/* Allocate the Database Connection handle */
	V_OD_erg = SQLAllocHandle(SQL_HANDLE_DBC, odbc->V_OD_Env, &odbc->V_OD_hdbc);
	if ((V_OD_erg != SQL_SUCCESS) && (V_OD_erg != SQL_SUCCESS_WITH_INFO))
	{
		SQLFreeHandle(SQL_HANDLE_ENV, odbc->V_OD_Env);
		free(odbc);
		GB.Error("Unable to allocate the ODBC handle");
		return TRUE;

	}

	/* Connect to Database (Data Source Name) */
	V_OD_erg =
		SQLConnect(odbc->V_OD_hdbc, (SQLCHAR *) desc->host, SQL_NTS, (SQLCHAR *) desc->user,
							 SQL_NTS, (SQLCHAR *) desc->password, SQL_NTS);
	if ((V_OD_erg != SQL_SUCCESS) && (V_OD_erg != SQL_SUCCESS_WITH_INFO))
	{
		SQLFreeHandle(SQL_HANDLE_DBC, odbc->V_OD_hdbc);
		SQLFreeHandle(SQL_HANDLE_ENV, odbc->V_OD_Env);
		free(odbc);
		GB.Error("Unable to connect to data source");
		return TRUE;

	}
	V_OD_erg =
		SQLSetConnectAttr(odbc->V_OD_hdbc, SQL_ATTR_AUTOCOMMIT,
											(void *) SQL_AUTOCOMMIT_ON, SQL_NTS);
	odbc->dsn_name = malloc(sizeof(char) * strlen(desc->host));
	strcpy(odbc->dsn_name, desc->host);

	odbc->user_name = malloc(sizeof(char) * strlen(desc->user));
	strcpy(odbc->user_name, desc->user);

	db->version = 3;

	V_OD_erg =
		SQLGetFunctions(odbc->V_OD_hdbc, SQL_API_SQLFETCHSCROLL,
										&odbc->FetchScroll_exist);
	if ((V_OD_erg != SQL_SUCCESS) && (V_OD_erg != SQL_SUCCESS_WITH_INFO))
	{
		GB.Error("Error calling the ODBC SQLGetFunction API");
		return TRUE;
		//printf("ERROR calling the SQLGetFunction API\n");
	}

	/* flags */
	db->flags.no_table_type = TRUE;
	db->flags.no_seek = (odbc->FetchScroll_exist == SQL_FALSE);

	db->flags.no_serial = TRUE;		// Need to be done!
	db->flags.no_blob = TRUE;			// Need to be done!

	db->handle = odbc;
	return FALSE;
}

/*****************************************************************************

  close_database()

  Terminates the database connection.

  <handle> contains the database handle.

*****************************************************************************/

static void close_database(DB_DATABASE *db)
{
	//SQLFreeHandle(SQL_HANDLE_STMT,V_OD_hstmt);
	ODBC_CONN *conn = (ODBC_CONN *)db->handle;

	SQLDisconnect(conn);
	SQLFreeHandle(SQL_HANDLE_DBC, conn->V_OD_hdbc);
	SQLFreeHandle(SQL_HANDLE_ENV, conn->V_OD_Env);
	free(conn->dsn_name);
	free(conn->user_name);
	if (conn != NULL)
		SQL_Handle_free(conn);
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
	int l;
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

			return FALSE;							// default

		case GB_T_DATE:

			date = GB.SplitDate((GB_DATE *) arg);

			l = sprintf(_buffer, "'%04d-%02d-%02d-%02d.%02d.%02d",
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

static void format_blob(DB_BLOB *blob, DB_FORMAT_CALLBACK add)
{
	// BM: must be done!
}



ODBC_RESULT *SQL_Result(void)
{
	return (malloc(sizeof(ODBC_RESULT)));
}

static void SQL_Result_Free(ODBC_RESULT * ptr)
{

	free(ptr);
	return;
}

/* Internal function to implement the query execution */

static int do_query(ODBC_CONN * handle, const char *error, ODBC_RESULT ** res,
										const char *query, int nsubst, ...)
{

//printf ("do_query function, query :%s \n",query);
	long V_OD_erg = SQL_SUCCESS;
	ODBC_RESULT *odbcres;
	int memory = 0;



	odbcres = SQL_Result();
	odbcres->V_OD_hstmt = NULL;
	memory = 3;



	/* Allocate the space for the result structure */




	V_OD_erg =
		SQLAllocHandle(SQL_HANDLE_STMT, handle->V_OD_hdbc, &odbcres->V_OD_hstmt);
	if ((V_OD_erg != SQL_SUCCESS) && (V_OD_erg != SQL_SUCCESS_WITH_INFO))
	{
		//GB.Error("ODBC - Error - cannor allocate the handler");
		//printf("ODBC - Error - cannor allocate the handler\n");
		return V_OD_erg;
	}


	V_OD_erg =
		SQLSetStmtAttr(odbcres->V_OD_hstmt, SQL_ATTR_CURSOR_SCROLLABLE,
									 (SQLPOINTER) SQL_SCROLLABLE, 0);

	if ((V_OD_erg != SQL_SUCCESS) && (V_OD_erg != SQL_SUCCESS_WITH_INFO))
	{
		//GB.Error("ODBC - Error cannot set the query attributes");
		//printf("ODBC - Error cannot set the query attributes\n");

		odbcres->Cursor_Scrollable=SQL_FALSE;

//*********************
/*
 
SQLCHAR       SqlState[6],  Msg[SQL_MAX_MESSAGE_LENGTH];
SQLINTEGER    NativeError;
SQLSMALLINT   i, MsgLen;
SQLRETURN      rc2;



   i = 1;
   if ((rc2 = SQLGetDiagRec(SQL_HANDLE_STMT, odbcres->V_OD_hstmt, i, SqlState, &NativeError, Msg, sizeof(Msg), &MsgLen)) != SQL_ERROR) {
      printf ("\n\nError %s , %d ,%s , %d \n\n", SqlState,(int)NativeError,Msg,MsgLen);
	//GB.Error("Error %s , %d ,%s ,%d\n" ,SqlState,(int)NativeError,Msg,MsgLen);
      i++;
   }
*/








//*********************
		//SQLFreeHandle(SQL_HANDLE_STMT, odbcres->V_OD_hstmt);
		//return V_OD_erg;
	} else odbcres->Cursor_Scrollable=SQL_TRUE;

//************************
/*
SQLCHAR       SqlState[6],  Msg[SQL_MAX_MESSAGE_LENGTH];
SQLINTEGER    NativeError;
SQLSMALLINT   i, MsgLen;
SQLRETURN      rc2;



   i = 1;
   if ((rc2 = SQLGetDiagRec(SQL_HANDLE_STMT, odbcres->V_OD_hstmt, i, SqlState, &NativeError, Msg, sizeof(Msg), &MsgLen)) != SQL_ERROR) {
      printf ("Error %s , %d ,%s , %d \n", SqlState,(int)NativeError,Msg,MsgLen);
	//GB.Error("Error %s , %d ,%s ,%d\n" ,SqlState,(int)NativeError,Msg,MsgLen);
      i++;
   }
//GB.Error("OK\n");
*/


//***********************

	/* Execute the query */

	V_OD_erg = SQLExecDirect(odbcres->V_OD_hstmt, /*(char *)*/ (SQLCHAR *) query, SQL_NTS);
	if ((V_OD_erg != SQL_SUCCESS) && (V_OD_erg != SQL_SUCCESS_WITH_INFO))
	{
		GB.Error("Error executing the statement");
		//printf("ODBC - Error Executing the statement. %s\n",query);
		SQLFreeHandle(SQL_HANDLE_STMT, odbcres->V_OD_hstmt);
		return V_OD_erg;
	}


	if (res)
	{
		*res = odbcres;
	}
	else
	{

		SQLFreeHandle(SQL_HANDLE_STMT, odbcres->V_OD_hstmt);
		SQL_Result_Free(odbcres);
	}



	odbcres->Function_exist = handle->FetchScroll_exist;



	return V_OD_erg;

}



/* Internal function - free the result structure create to allocate the result row */
static void query_free_result(ODBC_RESULT * result)
{

	SQLSMALLINT V_OD_colanz;
	ODBC_FIELDS *current, *next;
	int i, nresultcols;


	SQLNumResultCols(result->V_OD_hstmt, &V_OD_colanz);

	nresultcols = V_OD_colanz;

	//printf("query_free_result ncol %u\n",nresultcols);
	current = (ODBC_FIELDS *) result->fields;
	next = (ODBC_FIELDS *) result->fields;
	for (i = 0; i < nresultcols; i++)
	{
		//printf("loopppo i:%u , pointer current : %p,bext :%p\n",i,current,next);
		current = next;
		next = (ODBC_FIELDS *) current->next;
		if (current->fieldata != NULL)
			free(current->fieldata);
		free(current);
	}
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


static int exec_query(DB_DATABASE *db, char *query, DB_RESULT * result, char *err)
{
	return do_query((ODBC_CONN *)db->handle, err, (ODBC_RESULT **) result, query,	0);
}



/* Internal function - create the space for the result and bind the column to each field-space allocated */
static void query_make_result(ODBC_RESULT * result)
{

	SQLCHAR colname[32];
	SQLSMALLINT colnamelen;
	SQLUINTEGER precision;
	SQLSMALLINT scale;
	SQLINTEGER i;
	SQLINTEGER displaysize;
	SQLSMALLINT V_OD_colanz;
	ODBC_FIELDS *field, *current;
	SQLINTEGER collen;
	int nresultcols;


	SQLNumResultCols(result->V_OD_hstmt, &V_OD_colanz);

	nresultcols = V_OD_colanz;

	result->fields = NULL;


	if (result->fields == NULL)
	{

		field = malloc(sizeof(ODBC_FIELDS));

		result->fields = field;
		current = field;
	}


	for (i = 0; i < nresultcols; i++)
	{


		SQLDescribeCol(result->V_OD_hstmt, i + 1, current->fieldname,
									 sizeof(current->fieldname), &colnamelen, &current->type,
									 &precision, &scale, NULL);
		collen = precision;

		/* Get display length for column */
		SQLColAttribute(result->V_OD_hstmt, i + 1, SQL_COLUMN_DISPLAY_SIZE, NULL,
										0, NULL, &displaysize);

		/*
		 * Set column length to max of display length, and column name
		 * length. Plus one byte for null terminator
		 */
		//printf("collen : %u e display len %u\n",strlen((char *) colname),displaysize);
		if (displaysize >= strlen((char *) colname))
		{
			collen = displaysize + 1;
		}
		else
		{
			collen = strlen((char *) colname) + 1;
		}

		/* Allocate memory to bind column                             */


		current->fieldata = (SQLCHAR *) malloc(collen);
		if (collen > 0)
			current->fieldata[0] = '\0';
		current->next = NULL;

		/* Bind columns to program vars, converting all types to CHAR */
		SQLBindCol(result->V_OD_hstmt, i + 1, SQL_C_CHAR, current->fieldata,
							 collen, &current->outlen);

		if (i + 1 < nresultcols)
		{

			field = malloc(sizeof(ODBC_FIELDS));
			current->next = (struct ODBC_FIELDS *) field;
			current = field;


		}

	}


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

	long V_OD_erg;
	ODBC_RESULT *res = (ODBC_RESULT *) result;
	SQLINTEGER V_OD_rowanz;
	SQLSMALLINT V_OD_colanz;


	V_OD_erg = SQLNumResultCols(res->V_OD_hstmt, &V_OD_colanz);
	if ((V_OD_erg != SQL_SUCCESS) && (V_OD_erg != SQL_SUCCESS_WITH_INFO))
	{
		//printf("ODBC - Error getting the number of coloumns\n");
		return;
	}
        if (V_OD_colanz == 0 ){
		V_OD_erg = SQLRowCount(res->V_OD_hstmt, &V_OD_rowanz);
		if ((V_OD_erg != SQL_SUCCESS) && (V_OD_erg != SQL_SUCCESS_WITH_INFO))
		{
			//printf("ODBC - Error getting the number of rows\n");
			return;
		}
	} else 
	{
		V_OD_rowanz = -1;
		//printf ( "\n\nIN Colonne : %u Righe :%d\n\n",V_OD_colanz,V_OD_rowanz);
	}
	//printf ( "\n\nOUT Colonne : %u Righe :%d\n\n",V_OD_colanz,V_OD_rowanz);
        
	*count = V_OD_rowanz;
	info->nfield = V_OD_colanz;
	

	query_make_result(res);

}



/*****************************************************************************

  query_release()

  Free the info structure filled by query_init() and the result handle.

  <result> is the handle of the query result.
  <info> points to the info structure.

*****************************************************************************/

static void query_release(DB_RESULT result, DB_INFO * info)
{
	ODBC_RESULT *res = (ODBC_RESULT *) result;

	if (res != NULL)
		query_free_result(res);
	SQLFreeHandle(SQL_HANDLE_STMT, res->V_OD_hstmt);
}



/*****************************************************************************

  query_fill()

  Fill a result buffer with the value of each field of a record.

  <result> is the handle of the result.
  <pos> is the index of the record in the result.
  <buffer> points to an array having one element for each field in the
  result.

  This function must use GB.StoreVariant() to store the value in the
  buffer.

*****************************************************************************/


static int query_fill(DB_RESULT result, int pos, GB_VARIANT_VALUE * buffer, 	int next)
{
	ODBC_RESULT *res = (ODBC_RESULT *) result;
	GB_VARIANT value;
	SQLRETURN rc;
	SQLINTEGER i;
	SQLSMALLINT V_OD_colanz;			//Function_exist;
	ODBC_FIELDS *current;
	int nresultcols;

	//SQLGetFunctions(res->V_OD_hstmt,SQL_API_SQLFETCHSCROLL ,res->Function_exist);
	//printf("SQLFetchScroll function is supported by the driver %u\n", res->Function_exist);

	SQLNumResultCols(res->V_OD_hstmt, &V_OD_colanz);
	nresultcols = V_OD_colanz;


	current = res->fields;
	for (i = 0; i < nresultcols; i++)
	{
		current->fieldata[0] = '\0';
		//current->fieldata[1]='\0';
		current = (ODBC_FIELDS *) current->next;
	}
	/*
	   if (!next){
	   rc = SQLFetchScroll( res->V_OD_hstmt, SQL_FETCH_ABSOLUTE, pos+1 );
	   }
	   else {
	   //rc=SQLFetch(hstmt);
	   rc = SQLFetchScroll( res->V_OD_hstmt, SQL_FETCH_ABSOLUTE, pos+1 );
	 */

	if (res->Function_exist == SQL_TRUE)
	{
		if(res->Cursor_Scrollable == SQL_TRUE)
		{
			rc = SQLFetchScroll(res->V_OD_hstmt, SQL_FETCH_ABSOLUTE, pos + 1);
		//printf("Executing SQLFetchScroll\n");
		}
		else
		{
			rc = SQLFetchScroll(res->V_OD_hstmt, SQL_FETCH_NEXT, pos + 1);
		
		}

	}
	else
	{

		if (!next)
			return TRUE;

		rc = SQLFetch(res->V_OD_hstmt);
		//printf ("Executing SQLFetch instead of SQLFetchScroll\n");
	}

	//}



	if ((rc != SQL_SUCCESS) && (rc != SQL_SUCCESS_WITH_INFO))
		GB.Error("ODBC_END_OF_DATA");
	current = res->fields;

	if (rc != SQL_NO_DATA_FOUND && rc != SQL_NO_DATA)
	{
		//errmsg[0] = '\0';

		for (i = 0; i < nresultcols; i++)
		{

			value.type = GB_T_VARIANT;
			value.value._object.type = GB_T_NULL;


			if (current)
			{
				//conv_data(char *data, GB_VARIANT_VALUE * val, int type)
				//  printf("contenuto %s\n",current->fieldata);
				conv_data((char *) current->fieldata, &value.value, (int) current->type);
			}

			GB.StoreVariant(&value, &buffer[i]);

			current = (ODBC_FIELDS *) current->next;

		}														/* for all columns in this row  */



	}															/* while rows to fetch */

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
	// BM: Need to be done!
}


/*****************************************************************************

  field_name()

  Return the name of a field in a result from its index.

  <result> is the result handle.
  <field> is the field index.

*****************************************************************************/

static char *field_name(DB_RESULT result, int field)
{

	SQLCHAR colname[32];
	SQLSMALLINT coltype;
	SQLSMALLINT colnamelen;
	SQLUINTEGER precision;
	SQLSMALLINT scale;
	char *colnamer;
	ODBC_RESULT *res = (ODBC_RESULT *) result;

	SQLDescribeCol(res->V_OD_hstmt, field + 1, colname, sizeof(colname),
								 &colnamelen, &coltype, &precision, &scale, NULL);
	colnamer = malloc(sizeof(char) * strlen((char *) colname) + 1);
	strcpy(colnamer, (char *) colname);
	return colnamer;
}


/*****************************************************************************

  field_index()

  Return the index of a field in a result from its name.

  <result> is the result handle.
  <name> is the field name.
  <handle> is needed by this driver to enable table.field syntax

*****************************************************************************/

static int field_index(DB_RESULT result, char *name, DB_DATABASE *db)
{
	SQLCHAR colname[32];
	SQLSMALLINT coltype;
	SQLSMALLINT colnamelen;
	SQLUINTEGER precision;
	SQLSMALLINT scale;
	SQLSMALLINT V_OD_colanz;
	int field;
	ODBC_RESULT *res = (ODBC_RESULT *) result;

	SQLNumResultCols(res->V_OD_hstmt, &V_OD_colanz);

	for (field = 0; field < V_OD_colanz; field++)
	{
		SQLDescribeCol(res->V_OD_hstmt, field + 1, colname, sizeof(colname),
									 &colnamelen, &coltype, &precision, &scale, NULL);

		if (strcmp(name, (char *)colname) == 0)
		{
			return (int) (field);
		}

	}

	return (0);
}


/*****************************************************************************

  field_type()

  Return the Gambas type of a field in a result from its index.

  <result> is the result handle.
  <field> is the field index.

*****************************************************************************/

static GB_TYPE field_type(DB_RESULT result, int field)
{

	SQLCHAR colname[32];
	SQLSMALLINT coltype;
	SQLSMALLINT colnamelen;
	SQLUINTEGER precision;
	SQLSMALLINT scale;
	ODBC_RESULT *res = (ODBC_RESULT *) result;

	SQLDescribeCol(res->V_OD_hstmt, field + 1, colname, sizeof(colname),
								 &colnamelen, &coltype, &precision, &scale, NULL);


	return conv_type(coltype);
}


/*****************************************************************************

  field_length()

  Return the length of a field in a result from its index.

  <result> is the result handle.
  <field> is the field index.

*****************************************************************************/

static int field_length(DB_RESULT result, int field)
{
	SQLCHAR colname[32];
	SQLSMALLINT coltype;
	SQLSMALLINT colnamelen;
	SQLUINTEGER precision;
	SQLSMALLINT scale;
	ODBC_RESULT *res = (ODBC_RESULT *) result;

	SQLDescribeCol(res->V_OD_hstmt, field + 1, colname, sizeof(colname),
								 &colnamelen, &coltype, &precision, &scale, NULL);
	return colnamelen;

}


/*****************************************************************************

  begin_transaction()

  Begin a transaction.

  <handle> is the database handle.

  This function returns TRUE if the command has failed, and FALSE if
  everything was OK.

*****************************************************************************/

static int begin_transaction(DB_DATABASE *db)
{
	ODBC_CONN *conn = (ODBC_CONN *)db->handle;

	return (do_query
					(conn, "Unable to begin transaction: &1", NULL, "BEGIN", 0));
}


/*****************************************************************************

  commi_transaction()

  Commit a transaction.

  <handle> is the database handle.

  This function returns TRUE if the command has failed, and FALSE if
  everything was OK.

*****************************************************************************/

static int commit_transaction(DB_DATABASE *db)
{
	ODBC_CONN *conn = (ODBC_CONN *)db->handle;

	//ODBC_RESULT *res;
	//res = SQL_Result();
	//int exit;

	return (do_query
					(conn, "Unable to commit transaction: &1", NULL, "COMMIT", 0));
	//return (exit);
}


/*****************************************************************************

  rollback_transaction()

  Rolllback a transaction.

  <handle> is the database handle.

  This function returns TRUE if the command has failed, and FALSE if
  everything was OK.

*****************************************************************************/

static int rollback_transaction(DB_DATABASE *db)
{
	ODBC_CONN *conn = (ODBC_CONN *)db->handle;
	int exit;

	exit =
		(do_query
		 (conn, "Unable to rollback transaction: &1", NULL, "ROLLBACK", 0));
	return (exit);

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
   - info->types is a GB_TYPE[] giving the gambas type of each field.

  This function returns TRUE if the command has failed, and FALSE if
  everything was OK.

*****************************************************************************/

static int table_init(DB_DATABASE *db, char *table, DB_INFO * info)
{
	SQLCHAR coltype[100];
	SQLCHAR precision[100];
	SQLSMALLINT V_OD_colanz;
	SQLHSTMT hstmt;
	SQLRETURN V_OD_erg;
	int i;
	DB_FIELD *f;
	ODBC_FIELDS *fieldstr, *current;
	ODBC_CONN *han = (ODBC_CONN *)db->handle;


	GB.NewString(&info->table, table, 0);

	V_OD_erg =
		SQLAllocHandle(SQL_HANDLE_STMT, (ODBC_CONN *) han->V_OD_hdbc, &hstmt);

	if ((V_OD_erg != SQL_SUCCESS) && (V_OD_erg != SQL_SUCCESS_WITH_INFO))
	{
		return V_OD_erg;
	}

	if (!SQL_SUCCEEDED
			(V_OD_colanz =
			 SQLColumns(hstmt, NULL, 0, NULL, 0, (SQLCHAR *) table, SQL_NTS, NULL,
									0)))
		return -1;

	fieldstr = malloc(sizeof(ODBC_FIELDS));
	current = fieldstr;

	V_OD_colanz = 0;
	while (SQL_SUCCEEDED(SQLFetch(hstmt)))
	{
		SQLGetData(hstmt, SQLColumns_COLUMN_NAME, SQL_C_CHAR, current->fieldname,
							 sizeof(current->fieldname), 0);

		if (!SQL_SUCCEEDED
				(SQLGetData
				 (hstmt, SQLColumns_SQL_DATA_TYPE, SQL_C_CHAR, &coltype[0],
					sizeof(coltype), 0)))
			return TRUE;

		current->type = atol((char *)coltype);

		if (!SQL_SUCCEEDED
				(SQLGetData
				 (hstmt, SQLColumns_COLUMN_SIZE, SQL_C_CHAR, precision,
					sizeof(precision), 0)))
			return TRUE;

		current->outlen = atol((char *)precision);
		V_OD_colanz = V_OD_colanz + 1;
		current->next = malloc(sizeof(ODBC_FIELDS));
		current = (ODBC_FIELDS *) current->next;
	}

	info->nfield = V_OD_colanz;
	GB.Alloc((void **) &info->field, sizeof(DB_FIELD) * V_OD_colanz);
	i = 0;
	current = fieldstr;

	for (i = 0; i < V_OD_colanz; i++)
	{
		fieldstr = current;
		f = &info->field[i];
		GB.NewString(&f->name, (char *)current->fieldname, 0);

		f->type = conv_type(current->type);

		f->length = 0;
		if (f->type == GB_T_STRING)
			f->length = current->outlen;
		free(fieldstr);
		current = (ODBC_FIELDS *) current->next;
	}

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

static int table_index(DB_DATABASE *db, char *table, DB_INFO * info)
{
	int inx[256];
	SQLHSTMT hstmt, hstmt2;
	SQLRETURN V_OD_erg, nReturn = -1;
	SQLCHAR szKeyName[101] = "";
	SQLCHAR szColumnName[101] = "";
	SQLCHAR query[101] = "SELECT * FROM ";
	SQLSMALLINT V_OD_colanz;
	int i, n;
	ODBC_CONN *han = (ODBC_CONN *)db->handle;
	ODBC_FIELDS *fieldstr, *current;
	ODBC_RESULT *res;

	strcpy((char *)&query[14], table);


	V_OD_colanz = 0;
	V_OD_erg =
		SQLAllocHandle(SQL_HANDLE_STMT, (ODBC_CONN *) han->V_OD_hdbc, &hstmt2);

	if ((V_OD_erg != SQL_SUCCESS) && (V_OD_erg != SQL_SUCCESS_WITH_INFO))
	{
		return V_OD_erg;
	}

	V_OD_erg =
		SQLColumns(hstmt2, NULL, 0, NULL, 0, (SQLCHAR *) table, SQL_NTS, NULL, 0);

	if (!SQL_SUCCEEDED(V_OD_erg))
	{
		SQLFreeHandle(SQL_HANDLE_STMT, hstmt2);
		return -1;
	}


	fieldstr = malloc(sizeof(ODBC_FIELDS));

	current = fieldstr;

	while (SQL_SUCCEEDED(SQLFetch(hstmt2)))
	{
		if (!SQL_SUCCEEDED
				(SQLGetData
				 (hstmt2, SQLColumns_COLUMN_NAME, SQL_C_CHAR, current->fieldname,
					sizeof(current->fieldname), 0)))
			strcpy((char *)current->fieldname, "Unknown");


		V_OD_colanz = V_OD_colanz + 1;
		current->next = malloc(sizeof(ODBC_FIELDS));
		current = (ODBC_FIELDS *) current->next;
	}

	current = fieldstr;

	SQLFreeHandle(SQL_HANDLE_STMT, hstmt2);


	res = malloc(sizeof(ODBC_RESULT));

	V_OD_erg = SQLAllocHandle(SQL_HANDLE_STMT, han->V_OD_hdbc, &hstmt);

	if ((V_OD_erg != SQL_SUCCESS) && (V_OD_erg != SQL_SUCCESS_WITH_INFO))
	{
		return V_OD_erg;
	}

	if (!SQL_SUCCEEDED
			(nReturn = SQLPrimaryKeys(hstmt, 0, 0, 0, SQL_NTS, (SQLCHAR *)table, SQL_NTS)))
	{
		free(res);
		return TRUE;
	}


	SQLNumResultCols(hstmt, &V_OD_colanz);

	i = 0;
	while (SQL_SUCCEEDED(SQLFetch(hstmt)))
	{



		if (!SQL_SUCCEEDED
				(SQLGetData
				 (hstmt, 4, SQL_C_CHAR, szColumnName, sizeof(szColumnName), 0)))
			strcpy((char *) szColumnName, "Unknown");

		if (!SQL_SUCCEEDED
				(SQLGetData(hstmt, 6, SQL_C_CHAR, szKeyName, sizeof(szKeyName), 0)))
			strcpy((char *) szKeyName, "Unknown");

		current = fieldstr;
		for (n = 0; n < V_OD_colanz; n++)
		{
			if (strcmp((char *)current->fieldname, (char *)szColumnName) == 0)
			{
				inx[i] = n;
				break;
			}
		}
		i++;

	}


	GB.Alloc((void **) &info->index, sizeof(int) * i);
	info->nindex = i;
	SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
	for (n = 0; n < i; n++)
		info->index[n] = inx[n];


	free(res);
	current = fieldstr;
	for (i = 0; i < V_OD_colanz; i++)
	{
		current = (ODBC_FIELDS *) current->next;
		free(fieldstr);
		fieldstr = current;

	}


	return FALSE;

}


/*****************************************************************************

  table_release()

  Free the info structure filled by table_init() and/or table_index()

  <handle> is the database handle.
  <info> points at the info structure.

*****************************************************************************/

static void table_release(DB_DATABASE *db, DB_INFO * info)
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

static int table_exist(DB_DATABASE *db, char *table)
{


	SQLHSTMT hstmt;
	SQLRETURN V_OD_erg, nReturn = -1;
	SQLCHAR szTableName[101] = "";
	SQLCHAR szTableType[101] = "";
	SQLCHAR szTableRemarks[301] = "";
	SQLLEN nIndicatorName;
	SQLLEN nIndicatorType;
	SQLLEN nIndicatorRemarks;
	int compare = -1;
	ODBC_CONN *han = (ODBC_CONN *)db->handle;

	V_OD_erg = SQLAllocHandle(SQL_HANDLE_STMT, han->V_OD_hdbc, &hstmt);

	if ((V_OD_erg != SQL_SUCCESS) && (V_OD_erg != SQL_SUCCESS_WITH_INFO))
	{
		return FALSE; //V_OD_erg;
	}



	// EXECUTE OUR SQL/CALL
	if (SQL_SUCCESS != (nReturn = SQLTables(hstmt, 0, 0, 0, 0, 0, 0, 0, 0)))
	{

		return FALSE; //nReturn;
	}

	SQLBindCol(hstmt, SQLTables_TABLE_NAME, SQL_C_CHAR, szTableName,
						 sizeof(szTableName), &nIndicatorName);
	SQLBindCol(hstmt, SQLTables_TABLE_TYPE, SQL_C_CHAR, szTableType,
						 sizeof(szTableType), &nIndicatorType);
	SQLBindCol(hstmt, SQLTables_REMARKS, SQL_C_CHAR, szTableRemarks,
						 sizeof(szTableRemarks), &nIndicatorRemarks);
	// GET RESULTS
	nReturn = SQLFetch(hstmt);
	while ((nReturn == SQL_SUCCESS || nReturn == SQL_SUCCESS_WITH_INFO)
				 && compare != 0)
	{

		//printf("le tabelle in comparazione %s : %s\n",szTableName,table);
		compare = strncmp((char *)szTableName, table, sizeof(table));
		szTableName[0] = '\0';
		szTableType[0] = '\0';
		szTableRemarks[0] = '\0';
		nReturn = SQLFetch(hstmt);
	}

	// FREE STATEMENT
	nReturn = SQLFreeHandle(SQL_HANDLE_STMT, hstmt);

	if (compare == 0)
		return TRUE;
	else
		return FALSE;

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

static long table_list(DB_DATABASE *db, char ***tables)
{
	ODBC_TABLES tablelist, *curtable;
	SQLHSTMT hstmt;
	SQLRETURN V_OD_erg, nReturn = -1;
	SQLCHAR szTableName[101] = "";
	SQLCHAR szTableType[101] = "";
	SQLCHAR szTableRemarks[301] = "";
	SQLLEN nIndicatorName;
	SQLLEN nIndicatorType;
	SQLLEN nIndicatorRemarks;
	int tablenum = 0;
	int i;
	ODBC_CONN *han = (ODBC_CONN *)db->handle;

	V_OD_erg = SQLAllocHandle(SQL_HANDLE_STMT, han->V_OD_hdbc, &hstmt);

	if ((V_OD_erg != SQL_SUCCESS) && (V_OD_erg != SQL_SUCCESS_WITH_INFO))
	{
		return V_OD_erg;
	}

	curtable = &tablelist;

	// EXECUTE OUR SQL/CALL
	if (SQL_SUCCESS != (nReturn = SQLTables(hstmt, 0, 0, 0, 0, 0, 0, 0, 0)))
	{

		return nReturn;
	}

	SQLBindCol(hstmt, SQLTables_TABLE_NAME, SQL_C_CHAR, szTableName,
						 sizeof(szTableName), &nIndicatorName);
	SQLBindCol(hstmt, SQLTables_TABLE_TYPE, SQL_C_CHAR, szTableType,
						 sizeof(szTableType), &nIndicatorType);
	SQLBindCol(hstmt, SQLTables_REMARKS, SQL_C_CHAR, szTableRemarks,
						 sizeof(szTableRemarks), &nIndicatorRemarks);
	// GET RESULTS
	nReturn = SQLFetch(hstmt);

	if (nReturn != SQL_SUCCESS && nReturn != SQL_SUCCESS_WITH_INFO)
	{

		SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
		return (-1);

	}


	while (nReturn == SQL_SUCCESS || nReturn == SQL_SUCCESS_WITH_INFO)
	{


		tablenum = tablenum + 1;
		curtable->tablename = malloc(sizeof(szTableName));
		curtable->next = malloc(sizeof(ODBC_TABLES));
		strncpy(curtable->tablename, (char *)szTableName, sizeof(szTableName));
		curtable = (ODBC_TABLES *) curtable->next;
		szTableName[0] = '\0';
		szTableType[0] = '\0';
		szTableRemarks[0] = '\0';
		nReturn = SQLFetch(hstmt);
	}

	// FREE STATEMENT
	nReturn = SQLFreeHandle(SQL_HANDLE_STMT, hstmt);

	GB.NewArray(tables, sizeof(char *), tablenum);



	curtable = &tablelist;
	for (i = 0; i < tablenum; i++)
	{

		GB.NewString(&(*tables)[i], curtable->tablename, 0);
		free(curtable->tablename);
		curtable = (ODBC_TABLES *) curtable->next;

	}

	curtable = &tablelist;
	int g;

	for (i = tablenum; i > 0; i--)
	{


		for (g = 0; g < i; g++)
		{

			curtable = (ODBC_TABLES *) curtable->next;

		}

		free(curtable);
		curtable = &tablelist;
	}


	return (tablenum);
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

static int table_primary_key(DB_DATABASE *db, char *table, char ***primary)
{
	SQLHSTMT hstmt;
	SQLRETURN V_OD_erg, nReturn = -1;
	SQLCHAR szKeyName[101] = "";
	SQLCHAR szColumnName[101] = "";
	SQLCHAR query[101] = "SELECT * FROM ";
	SQLSMALLINT V_OD_colanz;
	int i;
	ODBC_CONN *han = (ODBC_CONN *)db->handle;

	// CREATE A STATEMENT
	ODBC_RESULT *res;

	strcpy((char *)&query[14], table);

	res = malloc(sizeof(ODBC_RESULT));

	V_OD_erg = SQLAllocHandle(SQL_HANDLE_STMT, han->V_OD_hdbc, &hstmt);

	if ((V_OD_erg != SQL_SUCCESS) && (V_OD_erg != SQL_SUCCESS_WITH_INFO))
	{
		return V_OD_erg;
	}


	if (!SQL_SUCCEEDED
			(nReturn = SQLPrimaryKeys(hstmt, 0, 0, 0, SQL_NTS, (SQLCHAR *)table, SQL_NTS)))
	{
		free(res);
		return TRUE;
	}
	// GET RESULTS



	SQLNumResultCols(hstmt, &V_OD_colanz);

	GB.NewArray(primary, sizeof(char *), 0);
	i = 0;

	while (SQL_SUCCEEDED(SQLFetch(hstmt)))
	{

		if (!SQL_SUCCEEDED
				(SQLGetData
				 (hstmt, 4, SQL_C_CHAR, &szColumnName[0], sizeof(szColumnName), 0)))
			strcpy((char *) szColumnName, "Unknown");

		if (!SQL_SUCCEEDED
				(SQLGetData
				 (hstmt, 6, SQL_C_CHAR, &szKeyName[0], sizeof(szKeyName), 0)))
			strcpy((char *) szKeyName, "Unknown");

		GB.NewString((char **) GB.Add(primary), (char *)szColumnName, 0);
		i++;
	}


	SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
	free(res);
	return FALSE;

}


/*****************************************************************************

  table_is_system()

  Returns if a table is a system table.

  <handle> is the database handle.
  <table> is the table name.

  This function returns TRUE if the table is a system table, and FALSE if
  not.

*****************************************************************************/

static int table_is_system(DB_DATABASE *db, char *table)
{
	return FALSE;
}


/*****************************************************************************

  table_delete()

  Deletes a table.

  <handle> is the database handle.
  <table> is the table name.

  This function returns TRUE if the command has failed, and FALSE if
  everything was OK.

*****************************************************************************/

static int table_delete(DB_DATABASE *db, char *table)
{
	ODBC_CONN *conn = (ODBC_CONN *)db->handle;
	int exit;
	SQLCHAR query[101] = "DROP TABLE ";

	strcpy((char *)&query[11], table);
	exit = do_query(conn, "Cannot delete table:&1", NULL, (char *) query, 0);
	if (exit == 0)
	{
		exit = do_query(conn, "Cannot delete table:&1", NULL, "COMMIT", 0);
	}

	return (exit);
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

static int table_create(DB_DATABASE *db, char *table, DB_FIELD * fields, char **primary, char *not_used)
{
	ODBC_CONN *conn = (ODBC_CONN *)db->handle;
	DB_FIELD *fp;
	int comma;
	char *type;
	int i, exit;

	DB.Query.Init();

	DB.Query.Add("CREATE TABLE ");
	DB.Query.Add(table);
	DB.Query.Add(" ( ");

	comma = FALSE;
	for (fp = fields; fp; fp = fp->next)
	{
		if (comma)
			DB.Query.Add(", ");
		else
			comma = TRUE;

		DB.Query.Add(fp->name);

		switch (fp->type)
		{
			case GB_T_BOOLEAN:
				type = "SMALLINT";
				break;
			case GB_T_INTEGER:
				type = "INTEGER";
				break;
			case GB_T_FLOAT:
				type = "DOUBLE";
				break;
			case GB_T_DATE:
				type = "TIMESTAMP";
				break;
// New datatype BIGINT 64 bits
			case GB_T_LONG:
				type = "BIGINT";
				break;
			case GB_T_STRING:

				if (fp->length <= 0)
					type = "TEXT";
				else
				{
					sprintf(_buffer, "VARCHAR(%ld)", fp->length);
					type = _buffer;
				}

				break;

			default:
				type = "TEXT";
				break;
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


	exit = do_query(conn, "Cannot create table: &1", NULL, DB.Query.Get(), 0);

	if (exit == 0)
	{
		exit = do_query(conn, "Cannot create table:&1", NULL, "COMMIT", 0);
	}

	return (exit);

}


/*****************************************************************************

  field_exist()

  Returns if a field exists in a given table

  <handle> is the database handle.
  <table> is the table name.
  <field> is the field name.

  This function returns TRUE if the field exists, and FALSE if not.

*****************************************************************************/

static int field_exist(DB_DATABASE *db, char *table, char *field)
{
	SQLCHAR colname[256];
	SQLSMALLINT V_OD_colanz;
	SQLHSTMT hstmt;
	SQLRETURN V_OD_erg;
	ODBC_CONN *han = (ODBC_CONN *)db->handle;



	V_OD_erg =
		SQLAllocHandle(SQL_HANDLE_STMT, (ODBC_CONN *) han->V_OD_hdbc, &hstmt);

	if ((V_OD_erg != SQL_SUCCESS) && (V_OD_erg != SQL_SUCCESS_WITH_INFO))
	{
		return FALSE; //V_OD_erg;
	}

//printf("field exist dopo l'handler\n");

	if (!SQL_SUCCEEDED
			(V_OD_colanz =
			 SQLColumns(hstmt, NULL, 0, NULL, 0, (SQLCHAR *) table, SQL_NTS, NULL,
									0)))
	{
		SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
		return FALSE;
	}

//printf("field exist dopo la SQLColumn : %u\n",V_OD_colanz);

	while (SQL_SUCCEEDED(SQLFetch(hstmt)))
	{
		SQLGetData(hstmt, SQLColumns_COLUMN_NAME, SQL_C_CHAR, colname,
							 sizeof(colname), 0);

//printf("field exist dopo la get data - field =%s, Colname %s\n",field,colname);

		if (strcmp((char *)colname, field) == 0)
		{

//printf("Trovato il campo\n");

			SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
			return TRUE;

		}

	}

	SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
	return FALSE;

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

static long field_list(DB_DATABASE *db, char *table, char ***fields)
{
	SQLSMALLINT V_OD_colanz;
	SQLHSTMT hstmt;
	SQLRETURN V_OD_erg;
	int i;
	ODBC_CONN *han = (ODBC_CONN *)db->handle;
	ODBC_FIELDS *fieldstr, *current;


	V_OD_colanz = 0;
	V_OD_erg =
		SQLAllocHandle(SQL_HANDLE_STMT, (ODBC_CONN *) han->V_OD_hdbc, &hstmt);

	if ((V_OD_erg != SQL_SUCCESS) && (V_OD_erg != SQL_SUCCESS_WITH_INFO))
	{
		return V_OD_erg;
	}



	V_OD_erg =
		SQLColumns(hstmt, NULL, 0, NULL, 0, (SQLCHAR *) table, SQL_NTS, NULL, 0);


	if (!SQL_SUCCEEDED(V_OD_erg))
	{

		SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
		return -1;
	}



	fieldstr = malloc(sizeof(ODBC_FIELDS));

	current = fieldstr;



	while (SQL_SUCCEEDED(SQLFetch(hstmt)))
	{
		if (!SQL_SUCCEEDED
				(SQLGetData
				 (hstmt, SQLColumns_COLUMN_NAME, SQL_C_CHAR, current->fieldname,
					sizeof(current->fieldname), 0)))
			strcpy((char *)current->fieldname, "Unknown");


		V_OD_colanz = V_OD_colanz + 1;
		current->next = malloc(sizeof(ODBC_FIELDS));
		current = (ODBC_FIELDS *) current->next;
	}

	current = fieldstr;
	GB.NewArray(fields, sizeof(char *), V_OD_colanz);

	for (i = 0; i < V_OD_colanz; i++)
	{
		GB.NewString(&((*fields)[i]), (char *) current->fieldname, 0);



		current = (ODBC_FIELDS *) current->next;
		free(fieldstr);
		fieldstr = current;
	}


	SQLFreeHandle(SQL_HANDLE_STMT, hstmt);



	return V_OD_colanz;

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

static int field_info(DB_DATABASE *db, char *table, char *field,	DB_FIELD * info)
{
	SQLCHAR colname[32];
	SQLCHAR coltype[100];
	SQLCHAR precision[100];
	SQLHSTMT hstmt;
	SQLRETURN V_OD_erg;
	int i;
	ODBC_CONN *han = (ODBC_CONN *)db->handle;




	for (i = 0; i < 100; i++)
		coltype[i] = '\0';

	V_OD_erg =
		SQLAllocHandle(SQL_HANDLE_STMT, (ODBC_CONN *) han->V_OD_hdbc, &hstmt);

	if ((V_OD_erg != SQL_SUCCESS) && (V_OD_erg != SQL_SUCCESS_WITH_INFO))
	{
		return V_OD_erg;
	}



	if (!SQL_SUCCEEDED
			(V_OD_erg =
			 SQLColumns(hstmt, NULL, 0, NULL, 0, (SQLCHAR *) table, SQL_NTS, NULL,
									0)))
		return -1;



	while (SQL_SUCCEEDED(SQLFetch(hstmt)))
	{
		SQLGetData(hstmt, SQLColumns_COLUMN_NAME, SQL_C_CHAR, colname,
							 sizeof(colname), 0);



		if (strcmp((char *) colname, field) == 0)
		{



			SQL_SUCCEEDED(SQLGetData
										(hstmt, SQLColumns_SQL_DATA_TYPE, SQL_C_CHAR, coltype,
										 sizeof(coltype), 0));

			SQL_SUCCEEDED(SQLGetData
										(hstmt, SQLColumns_COLUMN_SIZE, SQL_C_CHAR, precision,
										 sizeof(precision), 0));


			break;
		}

	}


	info->name = NULL;
	info->type = conv_type(atol((char *)coltype));
	info->length = atol((char *) precision);

	if (info->type == GB_T_STRING)
	{


		info->length = atol((char *)precision);
		if (info->length < 0)
			info->length = 0;

	}
	else
		info->length = atol((char *) precision);

	info->def._object.type = GB_T_NULL;

	SQLFreeHandle(SQL_HANDLE_STMT, hstmt);


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

static int index_exist(DB_DATABASE *db, char *table, char *index)
{
	//GB.Error("ODBC does not implement this function - index_exist");
	return FALSE;
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

static long index_list(DB_DATABASE *db, char *table, char ***indexes)
{
	//GB.Error("ODBC does not implement this function - index_list");
	return (-1);
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

static int index_info(DB_DATABASE *db, char *table, char *index,	DB_INDEX * info)
{
//GB.Error("ODBC does not implement this function");
	return TRUE;
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

static int index_delete(DB_DATABASE *db, char *table, char *index)
{
//GB.Error("ODBC does not implement this function");
	return TRUE;
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

static int index_create(DB_DATABASE *db, char *table, char *index,
												DB_INDEX * info)
{
//GB.Error("ODBC does not implement this function");
	return TRUE;
}


/*****************************************************************************

  database_exist()

  Returns if a database exists

  <handle> is any database handle.
  <name> is the database name.

  This function returns TRUE if the database exists, and FALSE if not.

*****************************************************************************/

static int database_exist(DB_DATABASE *db, char *name)
{
	//GB.Error("ODBC does not implement this function");
	ODBC_CONN *han = (ODBC_CONN *)db->handle;

	if (strcmp(han->dsn_name, name) == 0)
	{

		return TRUE;
	}
	return FALSE;
}



/*****************************************************************************

  database_list()

  Returns an array containing the name of each database

  <handle> is any database handle.
  <databases> points to a variable that will receive the char* array.

  This function returns the number of databases, or -1 if the command has
  failed.

  Be careful: <databases> can be NULL, so that just the count is returned.

*****************************************************************************/

static long database_list(DB_DATABASE *db, char ***databases)
{
	ODBC_CONN *han = (ODBC_CONN *)db->handle;

//GB.Error("ODBC does not implement this function");
	GB.NewArray(databases, sizeof(char *), 1);
	GB.NewString(&(*databases)[0], han->dsn_name, 0);

	return (1);
}


/*****************************************************************************

  database_is_system()

  Returns if a database is a system database.

  <handle> is any database handle.
  <name> is the database name.

  This function returns TRUE if the database is a system database, and
  FALSE if not.

*****************************************************************************/

static int database_is_system(DB_DATABASE *db, char *name)
{
	//GB.Error("ODBC does not implement this function");
	return FALSE;
}

/*****************************************************************************

  table_type()
  Not Valid in postgresql

  <handle> is the database handle.
  <table> is the table name.

*****************************************************************************/

static char *table_type(DB_DATABASE *db, char *table, char *type)
{
	if (type)
		GB.Error("ODBC does not have any table types");
	return NULL;
}

/*****************************************************************************

  database_delete()

  Deletes a database.

  <handle> is the database handle.
  <name> is the database name.

  This function returns TRUE if the command has failed, and FALSE if
  everything was OK.

*****************************************************************************/

static int database_delete(DB_DATABASE *db, char *name)
{
	//GB.Error("ODBC does not implement this function");
	return TRUE;
}


/*****************************************************************************

  database_create()

  Creates a database.

  <handle> is the database handle.
  <name> is the database name.

  This function returns TRUE if the command has failed, and FALSE if
  everything was OK.

*****************************************************************************/

static int database_create(DB_DATABASE *db, char *name)
{
	//GB.Error("ODBC does not implement this function");
	return TRUE;
}


/*****************************************************************************

  user_exist()

  Returns if a user exists.

  <handle> is any database handle.
  <name> is the user name.

  This function returns TRUE if the user exists, and FALSE if not.

*****************************************************************************/

static int user_exist(DB_DATABASE *db, char *name)
{
	ODBC_CONN *han = (ODBC_CONN *)db->handle;

	if (strcmp(han->user_name, name) == 0)
	{

		return TRUE;
	}
	return FALSE;
//  GB.Error("ODBC does not implement this function");

}


/*****************************************************************************

  user_list()

  Returns an array containing the name of each user.

  <handle> is the database handle.
  <users> points to a variable that will receive the char* array.

  This function returns the number of users, or -1 if the command has
  failed.

  Be careful: <users> can be NULL, so that just the count is returned.

*****************************************************************************/

static long user_list(DB_DATABASE *db, char ***users)
{
	ODBC_CONN *han = (ODBC_CONN *)db->handle;
	GB.NewArray(users, sizeof(char *), 1);
	GB.NewString(&(*users)[0], han->user_name, 0);

	//GB.Error("ODBC does not implement this function");
	return (1);
}


/*****************************************************************************

  user_info()

  Get user description

  <handle> is the database handle.
  <name> is the user name.
  <info> points to a structure filled by the function.

  This function returns TRUE if the command has failed, and FALSE if
  everything was OK.

*****************************************************************************/

static int user_info(DB_DATABASE *db, char *name, DB_USER * info)
{
	//GB.Error("ODBC does not implement this function");
	return TRUE;
}


/*****************************************************************************

  user_delete()

  Deletes a user.

  <handle> is any database handle.
  <name> is the user name.

  This function returns TRUE if the command has failed, and FALSE if
  everything was OK.

*****************************************************************************/

static int user_delete(DB_DATABASE *db, char *name)
{
	//GB.Error("ODBC can't delete users");
	return TRUE;
}


/*****************************************************************************

  user_create()

  Creates a user.

  <handle> is the database handle.
  <name> is the user name.
  <info> points to a structure describing the user.

  This function returns TRUE if the command has failed, and FALSE if
  everything was OK.

*****************************************************************************/

static int user_create(DB_DATABASE *db, char *name, DB_USER * info)
{
	//GB.Error("ODBC can't create users");
	return TRUE;
}


/*****************************************************************************

  user_set_password()

  Change the user password.

  <handle> is the database handle.
  <name> is the user name.
  <password> is the new password

  This function returns TRUE if the command has failed, and FALSE if
  everything was OK.

*****************************************************************************/

static int user_set_password(DB_DATABASE *db, char *name, char *password)
{
	//GB.Error("ODBC can't set user's password");
	return TRUE;
}

/*****************************************************************************

  The driver interface

*****************************************************************************/

static DB_DRIVER _driver = {
	"odbc",
	(void *) open_database,
	(void *) close_database,
	(void *) format_value,
	(void *) format_blob,
	(void *) exec_query,
	(void *) begin_transaction,
	(void *) commit_transaction,
	(void *) rollback_transaction,
	(void *) get_quote,
	{
	 (void *) query_init,
	 (void *) query_fill,
	 (void *) blob_read,
	 (void *) query_release,
	 {
		(void *) field_type,
		(void *) field_name,
		(void *) field_index,
		(void *) field_length,
		},
	 },

	{
	 (void *) field_exist,
	 (void *) field_list,
	 (void *) field_info,
	 },

	{
	 (void *) table_init,
	 (void *) table_index,
	 (void *) table_release,
	 (void *) table_exist,
	 (void *) table_list,
	 (void *) table_primary_key,
	 (void *) table_is_system,
	 (void *) table_type,
	 (void *) table_delete,
	 (void *) table_create,
	 },
	{
	 (void *) index_exist,
	 (void *) index_list,
	 (void *) index_info,
	 (void *) index_delete,
	 (void *) index_create,
	 },
	{
	 (void *) database_exist,
	 (void *) database_list,
	 (void *) database_is_system,
	 (void *) database_delete,
	 (void *) database_create,
	 },
	{
	 (void *) user_exist,
	 (void *) user_list,
	 (void *) user_info,
	 (void *) user_delete,
	 (void *) user_create,
	 (void *) user_set_password}

};


/*****************************************************************************

  The component entry and exit functions.

*****************************************************************************/

int EXPORT GB_INIT(void)
{
	GB.GetInterface("gb.db", DB_INTERFACE_VERSION, &DB);
	DB.Register(&_driver);

	return -1;
}

void EXPORT GB_EXIT()
{
}
