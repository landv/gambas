/***************************************************************************

  main.c

  (c) 2004-2007 Andrea Bortolan <andrea_bortolan@yahoo.it>
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

//#define ODBC_TYPE
//#define ODBC_DEBUG
//#define DEBUG_ODBC
//#define ODBC_DEBUG_HEADER
//#define ODBC_DEBUG_MEM

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <math.h>

#include <sql.h>
#include <sqlext.h>
#include <sqltypes.h>

#ifdef HAVE_SYS_TYPES_H
#undef HAVE_SYS_TYPES_H
#endif

#ifdef HAVE_UNISTD_H
#undef HAVE_UNISTD_H
#endif

#include "gb.db.proto.h"
#include "main.h"


GB_INTERFACE GB EXPORT;
DB_INTERFACE DB EXPORT;

static char _buffer[32];
//static char _nullbuffer[10];
static DB_DRIVER _driver; 

typedef struct
	{
	//	SQLHENV odbcEnvHandle;
	//	SQLHDBC odbcHandle;
		SQLHANDLE odbcEnvHandle;
		SQLHANDLE odbcHandle;
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
		SQLHSTMT odbcStatHandle;
		SQLUSMALLINT Function_exist;	//Does the Driver supports the SQLFetchScroll ?
		SQLUSMALLINT Cursor_Scrollable;  //Is it possible to set a Scrollable cursor ?
		ODBC_FIELDS *fields;
		SQLLEN count;
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
	GB.Alloc(&ptr, (int)size);
	return ptr;
}

static void my_free(void *_ptr)
{
	GB.Free(POINTER(&_ptr));
}

#define malloc(_size) my_malloc(_size)
#define free(_ptr) my_free(_ptr)


#if 0
static void quote(char *data, int len, DB_FORMAT_CALLBACK add)
{
#ifdef ODBC_DEBUG_HEADER
fprintf(stderr,"[ODBC][%s][%d]\n",__FILE__,__LINE__);
fprintf(stderr,"\tquote\n");
fflush(stderr);
#endif
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
		
		else
			(*add)((char *)&c, 1);
	}
	(*add)("'", 1);
}
#endif

/* internal function to quote a value stored as a blob */

static void quote_blob(char *data, int len, DB_FORMAT_CALLBACK add)
{
#ifdef ODBC_DEBUG_HEADER
fprintf(stderr,"[ODBC][%s][%d]\n",__FILE__,__LINE__);
fprintf(stderr,"\tquote_blob\n");
fflush(stderr);
#endif
	int i;
	unsigned char c;
	//char buffer[8];

	(*add)("'", 1);
	for (i = 0; i < len; i++)
	{
		c = (unsigned char)data[i];
		if (c == '\\')
			(*add)("\\\\\\\\", 4);
		else if (c == '\'')
			(*add)("''", 2);
		else if (c == 0)
			(*add)("\\\\000", 5);
		/*else if (c < 32 || c == 127)
		{
			int n = sprintf(buffer, "\\\\%03o", c);
			(*add)(buffer, n);
		}*/
		else
			(*add)((char *)&c, 1);
	}
	(*add)("'", 1);
}


/* internal function to unquote a value stored as a string */

#if 0
static int unquote(char *data, int len, DB_FORMAT_CALLBACK add)
{
#ifdef ODBC_DEBUG_HEADER
fprintf(stderr,"[ODBC][%s][%d]\n",__FILE__,__LINE__);
fprintf(stderr,"\tunquote\n");
fflush(stderr);
#endif

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
#endif

/* internal function to unquote a value stored as a blob */

static int unquote_blob(char *data, int len, DB_FORMAT_CALLBACK add)
{
#ifdef ODBC_DEBUG_HEADER
fprintf(stderr,"[ODBC][%s][%d]\n",__FILE__,__LINE__);
fprintf(stderr,"\tunquote_blob\n");
fflush(stderr);
#endif
  int i;
  char c;


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

static GB_TYPE conv_type(int type)
{

#ifdef ODBC_DEBUG_HEADER
fprintf(stderr,"[ODBC][%s][%d]\n",__FILE__,__LINE__);
fprintf(stderr,"\tconv_type :Field type :%d\n",type);
fflush(stderr);
#endif
	switch (type)
	{
		case SQL_BINARY:
			return GB_T_BOOLEAN;

			/*case INT8OID: */
		case SQL_NUMERIC:
			return GB_T_FLOAT;
			
		case SQL_DECIMAL:
			return GB_T_INTEGER;
		
		case SQL_INTEGER:
			return GB_T_INTEGER;
		
		case SQL_SMALLINT:
			return GB_T_INTEGER;
			

// New datatype bigint 64 bits
		case SQL_BIGINT:
			return GB_T_LONG;

		case SQL_FLOAT:
			return GB_T_FLOAT;
		case SQL_REAL:
			return GB_T_FLOAT;
		case SQL_DOUBLE:
			return GB_T_FLOAT;

		case SQL_DATETIME:
		case SQL_TYPE_DATE:
		case SQL_TYPE_TIME:
		case SQL_TYPE_TIMESTAMP:
			return GB_T_DATE;

// Data type for BLOB

		case SQL_LONGVARCHAR:
		case SQL_VARBINARY:
		case SQL_LONGVARBINARY:
			return DB_T_BLOB;

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
	int len;
	int bc = 0;

	switch (type)
	{
		//case SQL_NUMERIC:
		case SQL_DECIMAL:
		case SQL_INTEGER:
		case SQL_SMALLINT:
	
			val->type = GB_T_INTEGER;
			
			if (GB.NumberFromString(GB_NB_READ_INTEGER, data, strlen(data), &conv))
				val->value._integer = 0;
			else
				val->value._integer = conv._integer.value;

			break;
			
		case SQL_NUMERIC:
		case SQL_FLOAT:
		case SQL_REAL:
		case SQL_DOUBLE:
			
			val->type = GB_T_FLOAT;
			
			if (GB.NumberFromString(GB_NB_READ_FLOAT, data, strlen(data), &conv))
				val->value._float = 0;
			else
				val->value._float = conv._float.value;
			
			break;
			
			
// Data type bigint 64 bits
		case SQL_BIGINT:

			val->type = GB_T_LONG;
			
			if (GB.NumberFromString(GB_NB_READ_LONG, data, strlen(data), &conv))
				val->value._long = 0;
			else
				val->value._long = conv._long.value;

			break;

// Data type BLOB 
   	case SQL_LONGVARCHAR:
		case SQL_VARBINARY:
		case SQL_LONGVARBINARY:
			
			// The BLOB are read by the blob_read() driver function
			// You must set NULL there.
			val->type = GB_T_NULL;
			break;

// Data type for Time
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

				sscanf(data, "%4d-%2d-%2d %2d:%2d:%lf", &date.year, &date.month,
							 &date.day, &date.hour, &date.min, &sec);
				date.sec = (short) sec;
				date.msec = (short) ((sec - date.sec) * 1000 + 0.5);
				//break;


				if (bc)
					date.year = (-date.year);

				GB.MakeDate(&date, (GB_DATE *) & conv);

				val->type = GB_T_DATE;
				val->value._date.date = conv._date.value.date;
				val->value._date.time = conv._date.value.time;
				break;
			}

		case SQL_CHAR:
		default:

			val->type = GB_T_CSTRING;
			val->value._string = data;

			break;
	}

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

  The name of the database can be NULL, meaning a default database.

*****************************************************************************/

/* Internal function to allocate the ODBC handle */
static ODBC_CONN * SQL_Handle(void)
{
#ifdef ODBC_DEBUG_HEADER
fprintf(stderr,"[ODBC][%s][%d]\n",__FILE__,__LINE__);
fprintf(stderr,"\tSQL_Handle\n");
fflush(stderr);
#endif
	return (ODBC_CONN *) malloc(sizeof(ODBC_CONN));
}

void SQL_Handle_free(ODBC_CONN * ptr)
{
#ifdef ODBC_DEBUG_HEADER
fprintf(stderr,"[ODBC][%s][%d]\n",__FILE__,__LINE__);
fprintf(stderr,"\tSQL_Handle_free, pointer is %p\n",ptr);
fflush(stderr);
#endif

	free(ptr); 
}


static int open_database(DB_DESC * desc, DB_DATABASE *db)
{
#ifdef ODBC_DEBUG_HEADER
fprintf(stderr,"[ODBC][%s][%d]\n",__FILE__,__LINE__);
fprintf(stderr,"\topen_database\n");
fflush(stderr);
#endif
	//int V_OD_erg;
	SQLRETURN retcode;
	ODBC_CONN *odbc;

	/* Allocate the ODBC handle */
	odbc = SQL_Handle();


	odbc->odbcHandle = NULL;
	odbc->odbcEnvHandle=NULL;
	/* Allocate the Environment handle */
	retcode = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &odbc->odbcEnvHandle);

	if ((retcode != SQL_SUCCESS) && (retcode != SQL_SUCCESS_WITH_INFO))
	{
		free(odbc);
		GB.Error("Unable to allocate the environment handle");
		return TRUE;
	}

	/* Set the Envoronment attributes */
	retcode =SQLSetEnvAttr(odbc->odbcEnvHandle, SQL_ATTR_ODBC_VERSION,(void *) SQL_OV_ODBC3, 0);

	if ((retcode != SQL_SUCCESS) && (retcode != SQL_SUCCESS_WITH_INFO))
	{
		SQLFreeHandle(SQL_HANDLE_ENV, odbc->odbcEnvHandle);
		free(odbc);
		GB.Error("Unable to set the environment attributes");
		return TRUE;
	}

	/* Allocate the Database Connection handle */
	retcode = SQLAllocHandle(SQL_HANDLE_DBC, odbc->odbcEnvHandle, &odbc->odbcHandle);

	if ((retcode != SQL_SUCCESS) && (retcode != SQL_SUCCESS_WITH_INFO))
	{
		SQLFreeHandle(SQL_HANDLE_ENV, odbc->odbcEnvHandle);
		free(odbc);
		GB.Error("Unable to allocate the ODBC handle");
		return TRUE;

	}

	SQLSetConnectAttr(odbc->odbcHandle, SQL_ATTR_LOGIN_TIMEOUT, (SQLPOINTER)(intptr_t)db->timeout, 0);
	/* Connect to Database (Data Source Name) */
	retcode =SQLConnect(odbc->odbcHandle, (SQLCHAR *) desc->host, SQL_NTS, (SQLCHAR *) desc->user, SQL_NTS, (SQLCHAR *) desc->password, SQL_NTS);
	if ((retcode != SQL_SUCCESS) && (retcode != SQL_SUCCESS_WITH_INFO))
	{
		SQLFreeHandle(SQL_HANDLE_DBC, odbc->odbcHandle);
		SQLFreeHandle(SQL_HANDLE_ENV, odbc->odbcEnvHandle);
		free(odbc);
		GB.Error("Unable to connect to data source");
		return TRUE;

	}
	retcode =SQLSetConnectAttr(odbc->odbcHandle, SQL_ATTR_AUTOCOMMIT,(void *) SQL_AUTOCOMMIT_ON, SQL_NTS);
	odbc->dsn_name = malloc(sizeof(char) * strlen(desc->host));
	strcpy(odbc->dsn_name, desc->host);

	odbc->user_name = malloc(sizeof(char) * strlen(desc->user));
	strcpy(odbc->user_name, desc->user);

	db->version = 3;

	retcode =SQLGetFunctions(odbc->odbcHandle, SQL_API_SQLFETCHSCROLL,&odbc->FetchScroll_exist);
	if ((retcode != SQL_SUCCESS) && (retcode != SQL_SUCCESS_WITH_INFO))
	{
		GB.Error("Error calling the ODBC SQLGetFunction API");
		return TRUE;
		//printf("ERROR calling the SQLGetFunction API\n");
	}

	/* flags */
	db->flags.no_table_type = TRUE;
	db->flags.no_seek = (odbc->FetchScroll_exist == SQL_FALSE);
	db->flags.no_serial = TRUE;		// Need to be done!
	db->flags.no_blob = FALSE;			// Need to be done!
	db->flags.no_collation = TRUE;

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
  //SQLRETURN retcode;
#ifdef ODBC_DEBUG_HEADER
fprintf(stderr,"[ODBC][%s][%d]\n",__FILE__,__LINE__);
fprintf(stderr,"\tclose_database\n");
fflush(stderr);
#endif
	
	ODBC_CONN *conn = (ODBC_CONN *)db->handle;

	if (conn->odbcHandle)
		SQLDisconnect(conn->odbcHandle);
	else
		GB.Error("ODBC module internal error");

	if (conn->odbcHandle)
	{
		SQLFreeHandle(SQL_HANDLE_DBC, conn->odbcHandle);
		conn->odbcHandle = NULL;
	}
	else 
		GB.Error("ODBC module internal error");
	
	if (conn->odbcEnvHandle)
	{
		SQLFreeHandle(SQL_HANDLE_ENV, conn->odbcEnvHandle);
		conn->odbcEnvHandle = NULL;
	}
	else 
		GB.Error("ODBC module internal error");

	if (conn->dsn_name)
		free(conn->dsn_name);
	
	if (conn->user_name)
		free(conn->user_name);
	
	if (conn)
	{
		SQL_Handle_free(conn);
		db->handle = NULL;
	}
}


/*****************************************************************************

	get_collations()

	Return the available collations as a Gambas string array.

*****************************************************************************/

static GB_ARRAY get_collations(DB_DATABASE *db)
{
	return NULL;
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

#ifdef ODBC_DEBUG_HEADER
fprintf(stderr,"[ODBC][%s][%d]\n",__FILE__,__LINE__);
fprintf(stderr,"\tformat_value\n");
fflush(stderr);
#endif

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
	#ifdef ODBC_DEBUG_HEADER
	fprintf(stderr,"[ODBC][%s][%d]\n",__FILE__,__LINE__);
	fprintf(stderr,"\tformat_blob\n");
	fflush(stderr);
	#endif
	quote_blob(blob->data, blob->length, add);

}



ODBC_RESULT *SQL_Result(void)
{
	ODBC_RESULT * res;
	#ifdef ODBC_DEBUG_HEADER
	fprintf(stderr,"[ODBC][%s][%d]\n",__FILE__,__LINE__);
	fprintf(stderr,"\tSQL_Result\n");
	fflush(stderr);
	#endif

	res=malloc(sizeof(ODBC_RESULT));
	if (res== NULL){
		GB.Error("Error allocating memory");
	}

	res->fields=NULL;
	res->odbcStatHandle=NULL;

	return(res);

}

static void SQL_Result_Free(ODBC_RESULT * ptr)
{
#ifdef ODBC_DEBUG_HEADER
fprintf(stderr,"[ODBC][%s][%d]\n",__FILE__,__LINE__);
fprintf(stderr,"\tSQL_Result_Free %p\n",ptr);
fflush(stderr);
#endif
if (ptr != NULL)
	free(ptr);
	return;
}

/* Internal function to implement the query execution */

static int do_query(DB_DATABASE *db, const char *error, ODBC_RESULT ** res,
										const char *query, int nsubst, ...)
{
#ifdef ODBC_DEBUG_HEADER
fprintf(stderr,"[ODBC][%s][%d]\n",__FILE__,__LINE__);
fprintf(stderr,"\tdo_query db %p, ODBC_result res %p , db->handle %p,query =%s\n",db,res,db->handle,query);
fflush(stderr);
#endif
	ODBC_CONN *handle = (ODBC_CONN *)db->handle;
	SQLRETURN retcode= SQL_SUCCESS;
	ODBC_RESULT * odbcres;



	odbcres = SQL_Result();	
	odbcres->odbcStatHandle = NULL;


	/* Allocate the space for the result structure */


	retcode = SQLAllocHandle(SQL_HANDLE_STMT, handle->odbcHandle, &odbcres->odbcStatHandle);

	if ((retcode != SQL_SUCCESS) && (retcode != SQL_SUCCESS_WITH_INFO))
	{
		GB.Error("Cannot allocate statement handle");
		return retcode;
	}


	retcode = SQLSetStmtAttr(odbcres->odbcStatHandle, SQL_ATTR_CURSOR_SCROLLABLE, (SQLPOINTER) SQL_SCROLLABLE, 0);

	if ((retcode != SQL_SUCCESS) && (retcode != SQL_SUCCESS_WITH_INFO))
	{
		odbcres->Cursor_Scrollable = SQL_FALSE;
	} 
	else odbcres->Cursor_Scrollable=SQL_TRUE;


	odbcres->Function_exist = handle->FetchScroll_exist;

	/* Execute the query */


	retcode = SQLExecDirect(odbcres->odbcStatHandle, (SQLCHAR *) query, SQL_NTS);
	if ((retcode != SQL_SUCCESS) && (retcode != SQL_SUCCESS_WITH_INFO))
	{
		SQLFreeHandle(SQL_HANDLE_STMT, odbcres->odbcStatHandle);
		GB.Error("Error while executing the statement");
		return retcode;
	}


	if (res)
	{
		retcode = SQLRowCount(odbcres->odbcStatHandle, &odbcres->count);
		if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO)
		{
			SQLFreeHandle(SQL_HANDLE_STMT, odbcres->odbcStatHandle);
			GB.Error("Unable to retrieve row count");
			return retcode;
		}
		*res = odbcres;
	}
	else
	{
		SQLFreeHandle(SQL_HANDLE_STMT, odbcres->odbcStatHandle);
		SQL_Result_Free(odbcres);
	}

	return retcode;
}



/* Internal function - free the result structure create to allocate the result row */
static void query_free_result(ODBC_RESULT * result)
{
#ifdef ODBC_DEBUG_HEADER
fprintf(stderr,"[ODBC][%s][%d]\n",__FILE__,__LINE__);
fprintf(stderr,"\tquery_free_result %p\n",result);
fflush(stderr);
#endif
	
	ODBC_FIELDS *current, *next;
	


	current = (ODBC_FIELDS *) result->fields;
	next = (ODBC_FIELDS *) result->fields;

	while(current!=NULL)
	{
	
		next = (ODBC_FIELDS *) current->next;

		if (current->fieldata != NULL)
		{
		free(current->fieldata);//091107
		current->fieldata=NULL;//091107
		}
		if(current!=NULL)
		{
			free(current);
			current=NULL;//091107
		}
		current = next;
	}
if(result!=NULL){
	free(result);
	result=NULL;//091107
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


static int exec_query(DB_DATABASE *db, const char *query, DB_RESULT * result, const char *err)
{
#ifdef ODBC_DEBUG_HEADER
fprintf(stderr,"[ODBC][%s][%d]\n",__FILE__,__LINE__);
fprintf(stderr,"\texec_query\n");
fflush(stderr);
#endif
	return do_query(db, err, (ODBC_RESULT **) result, query,	0);
}

static int get_num_columns(ODBC_RESULT *result)
{
	SQLSMALLINT colsNum = 0;
	SQLRETURN retcode;
	
	retcode = SQLNumResultCols(result->odbcStatHandle, &colsNum);
	
	if ((retcode != SQL_SUCCESS) && (retcode != SQL_SUCCESS_WITH_INFO))
		GB.Error("ODBC error: Unable to get the number of columns");
	
	return colsNum;
}


/* Internal function - create the space for the result and bind the column to each field-space allocated */
static void query_make_result(ODBC_RESULT * result)
{
#ifdef ODBC_DEBUG_HEADER
fprintf(stderr,"[ODBC][%s][%d]\n",__FILE__,__LINE__);
fprintf(stderr,"\tquery_make_result %p,result->odbcStatHandle %p\n" ,result,result->odbcStatHandle);
fflush(stderr);
#endif
	SQLCHAR colname[32];
	SQLSMALLINT colnamelen;
	SQLULEN precision;
	SQLSMALLINT scale;
	SQLINTEGER i;
	SQLLEN displaysize;
	ODBC_FIELDS *field, *current;
	SQLINTEGER collen;
	int nresultcols;
	//int V_OD_erg=0;

	
	nresultcols = get_num_columns(result);
	result->fields = NULL;

	if (result->fields == NULL)
	{

		field = malloc(sizeof(ODBC_FIELDS));

		result->fields = field;
		current = field;
		current->next=NULL;
		current->fieldata=NULL;

	}


	for (i = 0; i < nresultcols; i++)
	{


		SQLDescribeCol(result->odbcStatHandle, i + 1, current->fieldname, sizeof(current->fieldname), &colnamelen, &current->type, &precision, &scale, NULL);
		collen = precision;

		/* Get display length for column */
		SQLColAttribute(result->odbcStatHandle, i + 1, SQL_COLUMN_DISPLAY_SIZE, NULL,0, NULL, &displaysize);

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

		if(collen <=0){
		collen=1;
		}
		current->fieldata = (SQLCHAR *) malloc(collen);
		current->outlen = collen;
		

		if (collen > 0)
			current->fieldata[collen-1] = '\0';
		current->next = NULL;


		{

			field = malloc(sizeof(ODBC_FIELDS));
			current->next = (struct ODBC_FIELDS *) field;
			current = field;
			current->next=NULL;
			current->fieldata=NULL;
			current->outlen=0;

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

	ODBC_RESULT *res = (ODBC_RESULT *) result;
	SQLSMALLINT colsNum = 0;
#ifdef ODBC_DEBUG_HEADER
fprintf(stderr,"[ODBC][%s][%d]\n",__FILE__,__LINE__);
fprintf(stderr,"\tquery_init, res %p,res->odbcStatHandle %p\n",res,res->odbcStatHandle);
fflush(stderr);
#endif

	colsNum = get_num_columns(res);
	if (!colsNum)
		return;

	//SQLRowCount(res->odbcStatHandle, &rowsNum);
	
	*count = res->count;
	info->nfield = colsNum;
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
#ifdef ODBC_DEBUG_HEADER
fprintf(stderr,"[ODBC][%s][%d]\n",__FILE__,__LINE__);
fprintf(stderr,"\tquery_release\n");
fflush(stderr);
#endif
	ODBC_RESULT *res = (ODBC_RESULT *) result;

	/*if (res != NULL)*/	//query_free_result(res);

	SQLFreeHandle(SQL_HANDLE_STMT, res->odbcStatHandle);
	//free(res->odbcStatHandle);

	query_free_result(res);
	//free(res->odbcStatHandle);
	//res->odbcStatHandle=NULL;

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


static int query_fill(DB_DATABASE *db, DB_RESULT result, int pos, GB_VARIANT_VALUE * buffer, 	int next)
{
	ODBC_RESULT *res = (ODBC_RESULT *) result;
	GB_VARIANT value;
	SQLRETURN retcode2;
	SQLINTEGER i;
	ODBC_FIELDS *current;
	//SQLRETURN retcode;
	int nresultcols;
	SQLINTEGER displaysize;
	//int V_OD_erg=0;
	
#ifdef ODBC_DEBUG_HEADER
fprintf(stderr,"[ODBC][%s][%d]\n",__FILE__,__LINE__);
fprintf(stderr,"\tquery_fill result %p,result->odbcStatHandle %p, pos %d\n",res,res->odbcStatHandle,pos);
fflush(stderr);
#endif
	
	nresultcols = get_num_columns(res);

	current = res->fields;
	for (i = 0; i < nresultcols; i++)
	{
		if(current->next != NULL)
			current = (ODBC_FIELDS *) current->next;
	}


	if (res->Function_exist == SQL_TRUE)
	{
		if(res->Cursor_Scrollable == SQL_TRUE)
		{	
			retcode2 = SQLFetchScroll(res->odbcStatHandle, SQL_FETCH_ABSOLUTE, pos + 1);
		}
		else
		{
			retcode2 = SQLFetchScroll(res->odbcStatHandle, SQL_FETCH_NEXT, pos + 1);
		}
	}
	else
	{
				
		if (!next) return TRUE;
		retcode2 = SQLFetch(res->odbcStatHandle);
	}

	if ((retcode2 != SQL_SUCCESS) && (retcode2 != SQL_SUCCESS_WITH_INFO) && (retcode2 != SQL_NO_DATA_FOUND) && (retcode2 != SQL_NO_DATA))
		GB.Error("ODBC_ERROR");
	current = res->fields;
			
	if((retcode2 == SQL_NO_DATA_FOUND) || (retcode2==SQL_NO_DATA))
		{
		GB.Error("ODBC_END_OF_DATA");
		//return false;
		}

	if (retcode2 != SQL_NO_DATA_FOUND && retcode2 != SQL_NO_DATA)
	{

		for (i = 0; i < nresultcols; i++)
		{
			displaysize=0;
			char * fieldata;
			SQLULEN   precision=0;
			
			SQLSMALLINT colnamelen=0,scale=0,type;
			SQLLEN read=0;
			SQLCHAR namebuff[25];
				
			SQLDescribeCol(res->odbcStatHandle, i+1 , namebuff, sizeof(namebuff), &colnamelen,&type,&precision, &scale,NULL);
				
			SQLColAttribute(res->odbcStatHandle, i+1 , SQL_DESC_LENGTH, "",0, NULL, (SQLPOINTER)&displaysize);
				
			read=0;

		if (displaysize >= strlen((char *) namebuff))
		{
			displaysize = displaysize + 1;
		}
		else
		{
			displaysize=strlen((char *) namebuff) + 1;
		}

if (displaysize>0) 
{
	if (displaysize < 2) displaysize=2; 
	if (type != SQL_LONGVARCHAR && type != SQL_VARBINARY && type != SQL_LONGVARBINARY)
	{
		fieldata=malloc(sizeof(char)*(displaysize));
		SQLGetData(res->odbcStatHandle,i+1,SQL_C_CHAR , fieldata,displaysize,&read); 
	
	} else
	{
		//BLOB field, not retrieved here 
				
		//the BLOB field hasn't the string terminator
		displaysize=displaysize-1;

	}
	current->outlen=displaysize;


}

	value.type = GB_T_VARIANT;
	value.value.type = GB_T_NULL;
	if(current==NULL)GB.Error("ODBC internal error 4");
//fprintf(stderr,"Lunghezza letta = %d\n",read);
	if (current)
	{
		if(current->fieldata==NULL)GB.Error("ODBC internal error 5");
		if (read == -1){
			fieldata[0]=' ';fieldata[1]='\0';
			current->type=SQL_CHAR;
		}
		conv_data((char *) fieldata, &value.value, (int) current->type);
	}
	
	GB.StoreVariant(&value,&buffer[i]);
	
	if(displaysize >0 && fieldata !=NULL)free(fieldata);
	current = (ODBC_FIELDS *) current->next;
	fieldata=NULL;
			
	}/* for all columns in this row  */

	}	/* while rows to fetch */

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
	int i;
	//int outlen;
	//int precision;
	//int scale;
	//int displaysize;
	//char * pointer;
	ODBC_RESULT * res= (ODBC_RESULT *) result;
	ODBC_FIELDS * cfield ;
	SQLLEN strlen;
	SQLRETURN retcode;
	
	i = 0;
	cfield = res->fields;

#ifdef ODBC_DEBUG_HEADER
fprintf(stderr,"[ODBC][%s][%d]\n",__FILE__,__LINE__);
fprintf(stderr,"\tblob_read DB_RESULT %p, dbresult->stathandle %p, pos %d , field %d\n",res,res->odbcStatHandle ,pos,field);
fflush(stderr);
#endif

	while (i < field )
	{
		if (cfield->next== NULL)
		{
			GB.Error("ODBC module :Internal error1"); 
			return;
		}
		
		cfield=(ODBC_FIELDS *) cfield->next;
		
		if (cfield== NULL)
		{
			GB.Error("ODBC module :Internal error2"); 
			return;
		}
		
		i++;
	}
	
	if (i > field)
	{
		GB.Error("ODBC module : Internal error");
		return;
	}
		
	blob->data=NULL;
	if (cfield->outlen > 0) 
	{
		blob->data = malloc( sizeof(char)*cfield->outlen);
		blob->length = cfield->outlen;

		DB.Query.Init();

		retcode = SQLGetData(res->odbcStatHandle,field+1,SQL_C_BINARY , blob->data,blob->length, &strlen);
	
		if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO)
		{
			GB.Error("Unable to retrieve blob data");
			free(blob->data);
			blob->length = 0;
			blob->data = NULL;
			return;
		}
	}
	else
	{
		blob->data = NULL; // 
		blob->length = 0;
		return;
	}
		
	char *data;
	int len;

	if (!unquote_blob(blob->data, blob->length, DB.Query.AddLength))
	{
		len = DB.Query.Length();
		data = DB.Query.GetNew();
	}
	else
		blob->constant = TRUE;
	
	free(blob->data);//091107
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
#ifdef ODBC_DEBUG_HEADER
fprintf(stderr,"[ODBC][%s][%d]\n",__FILE__,__LINE__);
fprintf(stderr,"\tfield_name\n");
fflush(stderr);
#endif
	SQLCHAR colname[32];
	SQLSMALLINT coltype=0;
	SQLSMALLINT colnamelen=0;
	SQLULEN precision=0;
	SQLSMALLINT scale=0;
	
	ODBC_RESULT *res = (ODBC_RESULT *) result;

	SQLDescribeCol(res->odbcStatHandle, field + 1, colname, sizeof(colname),
								 &colnamelen, &coltype, &precision, &scale, NULL);
	//colnamer = malloc(sizeof(char) * strlen((char *) colname) + 1);
	strcpy(_buffer, (char *) colname);
	return _buffer;
}


/*****************************************************************************

  field_index()

  Return the index of a field in a result from its name.

  <result> is the result handle.
  <name> is the field name.
  <handle> is needed by this driver to enable table.field syntax

*****************************************************************************/

static int field_index(DB_RESULT result, const char *name, DB_DATABASE *db)
{
#ifdef ODBC_DEBUG_HEADER
fprintf(stderr,"[ODBC][%s][%d]\n",__FILE__,__LINE__);
fprintf(stderr,"\tfield_index\n");
fflush(stderr);
#endif
	SQLCHAR colname[32];
	SQLSMALLINT coltype;
	SQLSMALLINT colnamelen;
	SQLULEN precision;
	SQLSMALLINT scale;
	SQLSMALLINT colsNum;
	int field;
	ODBC_RESULT *res = (ODBC_RESULT *) result;
	
	colnamelen = 32;
	colsNum = get_num_columns(res);

	for (field = 0; field < colsNum; field++)
	{
		SQLDescribeCol(res->odbcStatHandle, field + 1, colname, sizeof(colname),
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
#ifdef ODBC_DEBUG_HEADER
fprintf(stderr,"[ODBC][%s][%d]\n",__FILE__,__LINE__);
fprintf(stderr,"\tfield_type id %d\n",field);
fflush(stderr);
#endif
	SQLCHAR colname[32];
	SQLSMALLINT coltype;
	SQLSMALLINT colnamelen;
	SQLULEN precision;
	SQLSMALLINT scale;
	ODBC_RESULT *res = (ODBC_RESULT *) result;
	SQLRETURN retcode;

	retcode=SQLDescribeCol(res->odbcStatHandle, field + 1, colname, sizeof(colname),
								 &colnamelen, &coltype, &precision, &scale, NULL);

	if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO)
	{
		GB.Error("Unable to retrieve field type");
		return GB_T_NULL;
	}
	else
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
#ifdef ODBC_DEBUG_HEADER
fprintf(stderr,"[ODBC][%s][%d]\n",__FILE__,__LINE__);
fprintf(stderr,"\tfield_length\n");
fflush(stderr);
#endif
	SQLCHAR colname[32];
	SQLSMALLINT coltype;
	SQLSMALLINT colnamelen;
	SQLULEN precision;
	SQLSMALLINT scale;
	ODBC_RESULT *res = (ODBC_RESULT *) result;

	SQLDescribeCol(res->odbcStatHandle, field + 1, colname, sizeof(colname), &colnamelen, &coltype, &precision, &scale, NULL);

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
	return (do_query(db, "Unable to begin transaction: &1", NULL, "BEGIN", 0));
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
	return (do_query(db, "Unable to commit transaction: &1", NULL, "COMMIT", 0));
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
   - info->types is a GB_TYPE[] giving the gambas type of each field.

  This function returns TRUE if the command has failed, and FALSE if
  everything was OK.

*****************************************************************************/

static int table_init(DB_DATABASE *db, const char *table, DB_INFO * info)
{
#ifdef ODBC_DEBUG_HEADER
fprintf(stderr,"[ODBC][%s][%d]\n",__FILE__,__LINE__);
fprintf(stderr,"\ttable_init\n");
fflush(stderr);
#endif
	SQLCHAR coltype[100];
	SQLCHAR precision[100];
	SQLSMALLINT colsNum;
	SQLHSTMT statHandle;
	//SQLRETURN V_OD_erg;
	SQLRETURN retcode;
	int i;
	DB_FIELD *f;
	ODBC_FIELDS *fieldstr, *current;
	ODBC_CONN *han = (ODBC_CONN *)db->handle;


	info->table = GB.NewZeroString(table);

	retcode =SQLAllocHandle(SQL_HANDLE_STMT, (ODBC_CONN *) han->odbcHandle, &statHandle);


	if ((retcode != SQL_SUCCESS) && (retcode != SQL_SUCCESS_WITH_INFO))
	{
		return retcode;
	}

	if (!SQL_SUCCEEDED
			(colsNum =
			 SQLColumns(statHandle, NULL, 0, NULL, 0, (SQLCHAR *) table, SQL_NTS, NULL,
									0)))
		return -1;

	fieldstr = malloc(sizeof(ODBC_FIELDS));
	current = fieldstr;

	colsNum = 0;
	while (SQL_SUCCEEDED(SQLFetch(statHandle)))
	{
		SQLGetData(statHandle, SQLColumns_COLUMN_NAME, SQL_C_CHAR, current->fieldname,
							 sizeof(current->fieldname), 0);

		if (!SQL_SUCCEEDED
				(SQLGetData
				 (statHandle, SQLColumns_SQL_DATA_TYPE, SQL_C_CHAR, &coltype[0],
					sizeof(coltype), 0)))
			return TRUE;

		current->type = atol((char *)coltype);

		if (!SQL_SUCCEEDED
				(SQLGetData
				 (statHandle, SQLColumns_COLUMN_SIZE, SQL_C_CHAR, precision,
					sizeof(precision), 0)))
			return TRUE;

		current->outlen = atol((char *)precision);
		colsNum = colsNum + 1;
		current->next = malloc(sizeof(ODBC_FIELDS));
		current = (ODBC_FIELDS *) current->next;
	}

	info->nfield = colsNum;
	GB.Alloc(POINTER(&info->field), sizeof(DB_FIELD) * colsNum);
	i = 0;
	current = fieldstr;

	for (i = 0; i < colsNum; i++)
	{
		fieldstr = current;
		f = &info->field[i];
		f->name = GB.NewZeroString((char *)current->fieldname);

		f->type = conv_type(current->type);

		f->length = 0;
		if (f->type == GB_T_STRING)
			f->length = current->outlen;
		free(fieldstr);
		current = (ODBC_FIELDS *) current->next;
	}
	if (current != NULL) free(current);
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

static int table_index(DB_DATABASE *db, const char *table, DB_INFO * info)
{
#ifdef ODBC_DEBUG_HEADER
fprintf(stderr,"[ODBC][%s][%d]\n",__FILE__,__LINE__);
fprintf(stderr,"\ttable_index - info %p,info->nindex %d\n",info,info->nindex);
fflush(stderr);
#endif
	int inx[256];
	SQLHSTMT statHandle, statHandle2;
	//SQLRETURN V_OD_erg;
	SQLRETURN nReturn = -1;
	SQLRETURN retcode;
	SQLCHAR szKeyName[101] = "";
	SQLCHAR szColumnName[101] = "";
	SQLCHAR query[101] = "SELECT * FROM ";
	SQLSMALLINT colsNum;
	int i, n;
	ODBC_CONN *han = (ODBC_CONN *)db->handle;
	ODBC_FIELDS *fieldstr, *current;
	ODBC_RESULT *res;

	strcpy((char *)&query[14], table);


	colsNum = 0;

	retcode =SQLAllocHandle(SQL_HANDLE_STMT, (ODBC_CONN *) han->odbcHandle, &statHandle2);

	if ((retcode != SQL_SUCCESS) && (retcode != SQL_SUCCESS_WITH_INFO))
	{
		return retcode;
	}

	retcode =SQLColumns(statHandle2, NULL, 0, NULL, 0, (SQLCHAR *) table, SQL_NTS, NULL, 0);

	if (!SQL_SUCCEEDED(retcode))
	{
		SQLFreeHandle(SQL_HANDLE_STMT, statHandle2);
		return -1;
	}

	fieldstr = malloc(sizeof(ODBC_FIELDS));

	current = fieldstr;
	current->next=NULL;

	while (SQL_SUCCEEDED(SQLFetch(statHandle2)))
	{
		if (!SQL_SUCCEEDED(SQLGetData (statHandle2, SQLColumns_COLUMN_NAME, SQL_C_CHAR, current->fieldname,	sizeof(current->fieldname), 0)))
			strcpy((char *)current->fieldname, "Unknown");

		colsNum = colsNum + 1;
		current->next = malloc(sizeof(ODBC_FIELDS));
		current = (ODBC_FIELDS *) current->next;
		current->next=NULL;
	}

	current = fieldstr;

retcode=SQLNumResultCols(statHandle2, &colsNum);

	SQLFreeHandle(SQL_HANDLE_STMT, statHandle2);


	res = malloc(sizeof(ODBC_RESULT));

	retcode = SQLAllocHandle(SQL_HANDLE_STMT, han->odbcHandle, &statHandle);


	if ((retcode != SQL_SUCCESS) && (retcode != SQL_SUCCESS_WITH_INFO))
	{
		return retcode;
	}

	

	if (!SQL_SUCCEEDED(nReturn = SQLPrimaryKeys(statHandle, 0, 0, 0, SQL_NTS, (SQLCHAR *)table, SQL_NTS)))
	{

		free(res);
		return TRUE;
	}




	retcode=SQLNumResultCols(statHandle, &colsNum);

	i = 0;

	while (SQL_SUCCEEDED(SQLFetch(statHandle)))
	{

	

		if (!SQL_SUCCEEDED(SQLGetData (statHandle, 4, SQL_C_CHAR, szColumnName, sizeof(szColumnName), 0)))
			strcpy((char *) szColumnName, "Unknown");

		if (!SQL_SUCCEEDED(SQLGetData(statHandle, 6, SQL_C_CHAR, szKeyName, sizeof(szKeyName), 0)))
			strcpy((char *) szKeyName, "Unknown");

		current = fieldstr;

		for (n = 0; n < colsNum; n++)
		{

			if (strcmp((char *)current->fieldname, (char *)szColumnName) == 0)
			{
				inx[i] = n;

				break;
			}
		current=(ODBC_FIELDS *)current->next;
		if (current==NULL) break;

		}

		i++;

	}



	GB.Alloc(POINTER(&info->index), sizeof(int) * i);
	info->nindex = i;


	SQLFreeHandle(SQL_HANDLE_STMT, statHandle);

	for (n = 0; n < i; n++)
		info->index[n] = inx[n];


	free(res);

while(current != NULL){
	if (current->next != NULL){
		fieldstr = (ODBC_FIELDS *)current->next ;
		free (current);
		
		current=fieldstr;
	}else {
	free (current);
	current =NULL;
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

static int table_exist(DB_DATABASE *db, const char *table)
{

#ifdef ODBC_DEBUG_HEADER
fprintf(stderr,"[ODBC][%s][%d]\n",__FILE__,__LINE__);
fprintf(stderr,"\ttable_exist\n");
fflush(stderr);
#endif
	SQLHSTMT statHandle;
	//SQLRETURN V_OD_erg;
	SQLRETURN nReturn = -1;
	SQLRETURN retcode;
	SQLCHAR szTableName[101] = "";
	SQLCHAR szTableType[101] = "";
	SQLCHAR szTableRemarks[301] = "";
	SQLLEN nIndicatorName;
	SQLLEN nIndicatorType;
	SQLLEN nIndicatorRemarks;
	int compare = -1;
	ODBC_CONN *han = (ODBC_CONN *)db->handle;
	int len;

	len = strlen(table);
	if (len == 0)
		return FALSE;
	
	retcode = SQLAllocHandle(SQL_HANDLE_STMT, han->odbcHandle, &statHandle);

	if ((retcode != SQL_SUCCESS) && (retcode != SQL_SUCCESS_WITH_INFO))
	{
		return FALSE; //V_OD_erg;
	}

	// EXECUTE OUR SQL/CALL
	if (SQL_SUCCESS != (nReturn = SQLTables(statHandle, 0, 0, 0, 0, 0, 0, 0, 0)))
	{
		return FALSE; //nReturn;
	}

	SQLBindCol(statHandle, SQLTables_TABLE_NAME, SQL_C_CHAR, szTableName,
						 sizeof(szTableName), &nIndicatorName);
	SQLBindCol(statHandle, SQLTables_TABLE_TYPE, SQL_C_CHAR, szTableType,
						 sizeof(szTableType), &nIndicatorType);
	SQLBindCol(statHandle, SQLTables_REMARKS, SQL_C_CHAR, szTableRemarks,
						 sizeof(szTableRemarks), &nIndicatorRemarks);
	// GET RESULTS
	nReturn = SQLFetch(statHandle);
	while ((nReturn == SQL_SUCCESS || nReturn == SQL_SUCCESS_WITH_INFO)
				 && compare != 0)
	{
		//printf("le tabelle in comparazione %s : %s\n",szTableName,table);
		compare = strncmp((char *)szTableName, table, len);
		szTableName[0] = '\0';
		szTableType[0] = '\0';
		szTableRemarks[0] = '\0';
		nReturn = SQLFetch(statHandle);
	}

	// FREE STATEMENT
	nReturn = SQLFreeHandle(SQL_HANDLE_STMT, statHandle);

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

static int table_list(DB_DATABASE *db, char ***tables)
{
#ifdef ODBC_DEBUG_HEADER
fprintf(stderr,"[ODBC][%s][%d]\n",__FILE__,__LINE__);
fprintf(stderr,"\ttable_list\n");
fflush(stderr);
#endif
	ODBC_TABLES tablelist, *curtable;
	SQLHSTMT statHandle;
	//SQLRETURN V_OD_erg;
	SQLRETURN nReturn = -1;
	SQLRETURN retcode;
	SQLCHAR szTableName[101] = "";
	SQLCHAR szTableType[101] = "";
	SQLCHAR szTableRemarks[301] = "";
	SQLLEN nIndicatorName;
	SQLLEN nIndicatorType;
	SQLLEN nIndicatorRemarks;
	int tablenum = 0;
	int i;
	ODBC_CONN *han = (ODBC_CONN *)db->handle;

	retcode = SQLAllocHandle(SQL_HANDLE_STMT, han->odbcHandle, &statHandle);

	if ((retcode != SQL_SUCCESS) && (retcode != SQL_SUCCESS_WITH_INFO))
	{
		return retcode;
	}

	curtable = &tablelist;

	// EXECUTE OUR SQL/CALL
	if (SQL_SUCCESS != (nReturn = SQLTables(statHandle, 0, 0, 0, 0, 0, 0, 0, 0)))
	{

		return nReturn;
	}

	SQLBindCol(statHandle, SQLTables_TABLE_NAME, SQL_C_CHAR, szTableName,
						 sizeof(szTableName), &nIndicatorName);
	SQLBindCol(statHandle, SQLTables_TABLE_TYPE, SQL_C_CHAR, szTableType,
						 sizeof(szTableType), &nIndicatorType);
	SQLBindCol(statHandle, SQLTables_REMARKS, SQL_C_CHAR, szTableRemarks,
						 sizeof(szTableRemarks), &nIndicatorRemarks);
	// GET RESULTS
	nReturn = SQLFetch(statHandle);

	if (nReturn != SQL_SUCCESS && nReturn != SQL_SUCCESS_WITH_INFO)
	{

		SQLFreeHandle(SQL_HANDLE_STMT, statHandle);
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
		nReturn = SQLFetch(statHandle);
	}

	// FREE STATEMENT
	nReturn = SQLFreeHandle(SQL_HANDLE_STMT, statHandle);

	GB.NewArray(tables, sizeof(char *), tablenum);

	curtable = &tablelist;
	for (i = 0; i < tablenum; i++)
	{
		(*tables)[i] = GB.NewZeroString(curtable->tablename);
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

static int table_primary_key(DB_DATABASE *db, const char *table, char ***primary)
{
#ifdef ODBC_DEBUG_HEADER
fprintf(stderr,"[ODBC][%s][%d]\n",__FILE__,__LINE__);
fprintf(stderr,"\ttable_primary_key\n");
fflush(stderr);
#endif
	SQLHSTMT statHandle;
	SQLRETURN retcode;
	//SQLRETURN V_OD_erg;
	SQLRETURN nReturn = -1;
	SQLCHAR szKeyName[101] = "";
	SQLCHAR szColumnName[101] = "";
	SQLCHAR query[101] = "SELECT * FROM ";
	SQLSMALLINT colsNum;
	int i;
	ODBC_CONN *han = (ODBC_CONN *)db->handle;

	// CREATE A STATEMENT
	ODBC_RESULT *res;

	strcpy((char *)&query[14], table);

	res = malloc(sizeof(ODBC_RESULT));

	retcode = SQLAllocHandle(SQL_HANDLE_STMT, han->odbcHandle, &statHandle);

	if ((retcode != SQL_SUCCESS) && (retcode != SQL_SUCCESS_WITH_INFO))
	{
		return retcode;
	}


	/*if (!SQL_SUCCEEDED
			(nReturn = SQLPrimaryKeys(statHandle, 0, 0, 0, SQL_NTS, (SQLCHAR *)table, SQL_NTS)))*/
	if (!SQL_SUCCEEDED(nReturn = SQLPrimaryKeys(statHandle, (SQLCHAR *)"", 0, (SQLCHAR *)"", 0, (SQLCHAR *)table, SQL_NTS)))
	{
		free(res);
		fprintf(stderr, "return %d\n", nReturn);
		GB.Error("Unable to get primary key: &1", table);
		return TRUE;
	}
	// GET RESULTS



	SQLNumResultCols(statHandle, &colsNum);

	GB.NewArray(primary, sizeof(char *), 0);
	i = 0;

	while (SQL_SUCCEEDED(SQLFetch(statHandle)))
	{

		if (!SQL_SUCCEEDED
				(SQLGetData
				 (statHandle, 4, SQL_C_CHAR, &szColumnName[0], sizeof(szColumnName), 0)))
			strcpy((char *) szColumnName, "Unknown");

		if (!SQL_SUCCEEDED
				(SQLGetData
				 (statHandle, 6, SQL_C_CHAR, &szKeyName[0], sizeof(szKeyName), 0)))
			strcpy((char *) szKeyName, "Unknown");

		*(char **)GB.Add(primary) = GB.NewZeroString((char *)szColumnName);
		i++;
	}


	SQLFreeHandle(SQL_HANDLE_STMT, statHandle);
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

static int table_is_system(DB_DATABASE *db, const char *table)
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

static int table_delete(DB_DATABASE *db, const char *table)
{
	int exit;
	SQLCHAR query[101] = "DROP TABLE ";

	strcpy((char *)&query[11], table);
	exit = do_query(db, "Cannot delete table:&1", NULL, (char *) query, 0);
	if (exit == 0)
	{
		exit = do_query(db, "Cannot delete table:&1", NULL, "COMMIT", 0);
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

static int table_create(DB_DATABASE *db, const char *table, DB_FIELD * fields, char **primary, const char *type_not_used)
{
#ifdef ODBC_DEBUG_HEADER
fprintf(stderr,"[ODBC][%s][%d]\n",__FILE__,__LINE__);
fprintf(stderr,"\ttable_create\n");
fflush(stderr);
#endif
	
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

//AB autoincrement field is mapped to Integer because this is Database dependent
			if (fp->type == DB_T_SERIAL)
			DB.Query.Add(" INTEGER ");// 
//AB Blob field		else if (fp->type == DB_T_BLOB)
			DB.Query.Add(" LONG VARBINARY ");


		switch (fp->type)
		{
			case GB_T_BOOLEAN:
				type = "SMALLINT"; // 
				break;
			case GB_T_INTEGER:
				type = "INTEGER";
				break;
			case GB_T_FLOAT:
				type = "DOUBLE";
				break;
			case GB_T_DATE:
				type = "DATE"; // AB changed the field name used by default from Timestamp to Date 
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

	exit = do_query(db, "Cannot create table: &1", NULL, DB.Query.Get(), 0);
	

	if (exit == 0)
	{
		exit = do_query(db, "Cannot create table:&1", NULL, "COMMIT", 0);
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

static int field_exist(DB_DATABASE *db, const char *table, const char *field)
{
#ifdef ODBC_DEBUG_HEADER
fprintf(stderr,"[ODBC][%s][%d]\n",__FILE__,__LINE__);
fprintf(stderr,"\tfield_exist\n");
fflush(stderr);
#endif
	SQLCHAR colname[256];
	SQLSMALLINT colsNum;
	SQLHSTMT statHandle;
	SQLRETURN retcode;
	//SQLRETURN V_OD_erg;
	ODBC_CONN *han = (ODBC_CONN *)db->handle;



	retcode =
		SQLAllocHandle(SQL_HANDLE_STMT, (ODBC_CONN *) han->odbcHandle, &statHandle);

	if ((retcode != SQL_SUCCESS) && (retcode != SQL_SUCCESS_WITH_INFO))
	{
		return FALSE; //V_OD_erg;
	}

//printf("field exist dopo l'handler\n");

	if (!SQL_SUCCEEDED
			(colsNum =
			 SQLColumns(statHandle, NULL, 0, NULL, 0, (SQLCHAR *) table, SQL_NTS, NULL,
									0)))
	{
		SQLFreeHandle(SQL_HANDLE_STMT, statHandle);
		return FALSE;
	}

//printf("field exist dopo la SQLColumn : %u\n",colsNum);

	while (SQL_SUCCEEDED(SQLFetch(statHandle)))
	{
		SQLGetData(statHandle, SQLColumns_COLUMN_NAME, SQL_C_CHAR, colname,
							 sizeof(colname), 0);

//printf("field exist dopo la get data - field =%s, Colname %s\n",field,colname);

		if (strcmp((char *)colname, field) == 0)
		{

//printf("Trovato il campo\n");

			SQLFreeHandle(SQL_HANDLE_STMT, statHandle);
			return TRUE;

		}

	}

	SQLFreeHandle(SQL_HANDLE_STMT, statHandle);
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

static int field_list(DB_DATABASE *db, const char *table, char ***fields)
{
#ifdef ODBC_DEBUG_HEADER
fprintf(stderr,"[ODBC][%s][%d]\n",__FILE__,__LINE__);
fprintf(stderr,"\tfield_list\n");
fflush(stderr);
#endif
	SQLSMALLINT colsNum;
	SQLHSTMT statHandle;
	SQLRETURN retcode;
	//SQLRETURN V_OD_erg;
	int i;
	ODBC_CONN *han = (ODBC_CONN *)db->handle;
	ODBC_FIELDS *fieldstr, *current;


	colsNum = 0;
	retcode =
		SQLAllocHandle(SQL_HANDLE_STMT, (ODBC_CONN *) han->odbcHandle, &statHandle);

	if ((retcode != SQL_SUCCESS) && (retcode != SQL_SUCCESS_WITH_INFO))
	{
		return retcode;
	}



	retcode =
		SQLColumns(statHandle, NULL, 0, NULL, 0, (SQLCHAR *) table, SQL_NTS, NULL, 0);


	if (!SQL_SUCCEEDED(retcode))
	{

		SQLFreeHandle(SQL_HANDLE_STMT, statHandle);
		return -1;
	}



	fieldstr = malloc(sizeof(ODBC_FIELDS));

	current = fieldstr;



	while (SQL_SUCCEEDED(SQLFetch(statHandle)))
	{
		if (!SQL_SUCCEEDED
				(SQLGetData
				 (statHandle, SQLColumns_COLUMN_NAME, SQL_C_CHAR, current->fieldname,
					sizeof(current->fieldname), 0)))
			strcpy((char *)current->fieldname, "Unknown");


		colsNum = colsNum + 1;
		current->next = malloc(sizeof(ODBC_FIELDS));
		current = (ODBC_FIELDS *) current->next;
	}

	current = fieldstr;
	GB.NewArray(fields, sizeof(char *), colsNum);

	for (i = 0; i < colsNum; i++)
	{
		(*fields)[i] = GB.NewZeroString((char *) current->fieldname);

		current = (ODBC_FIELDS *) current->next;
		free(fieldstr);
		fieldstr = current;
	}
	free(fieldstr);

	SQLFreeHandle(SQL_HANDLE_STMT, statHandle);



	return colsNum;

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

static int field_info(DB_DATABASE *db, const char *table, const char *field, DB_FIELD * info)
{
#ifdef ODBC_DEBUG_HEADER
fprintf(stderr,"[ODBC][%s][%d]\n",__FILE__,__LINE__);
fprintf(stderr,"\tfield_info\n");
fflush(stderr);
#endif
	SQLCHAR colname[32];
	SQLCHAR coltype[100];
	SQLCHAR precision[100];
	char query[200];
	SQLHSTMT statHandle;
	SQLHSTMT statHandle1;
	SQLRETURN retcode;
	//SQLRETURN V_OD_erg;
	SQLLEN auton=SQL_FALSE;
	int i;
	
	ODBC_CONN *han = (ODBC_CONN *)db->handle;
	ODBC_CONN *han1 = (ODBC_CONN *)db->handle;

	*precision = 0;

	strncpy((char *)&query[0], "SELECT ",7);
	strncpy((char *)&query[7], field,strlen(field));
	strncpy((char *)&query[strlen(field)+7], " FROM ",6);
	strncpy((char *)&query[strlen(field)+13], table,strlen(table));
	query[strlen(field)+14+strlen(table)]='\0';
	strncpy((char *)&query[strlen(field)+13+strlen(table)],"\0\n\0\n",4);
	

	for (i = 0; i < 100; i++)
		coltype[i] = '\0';

	retcode =SQLAllocHandle(SQL_HANDLE_STMT, (ODBC_CONN *) han->odbcHandle, &statHandle);

	if ((retcode != SQL_SUCCESS) && (retcode != SQL_SUCCESS_WITH_INFO))
	{
		return retcode;
	}

	retcode =SQLAllocHandle(SQL_HANDLE_STMT, (ODBC_CONN *) han1->odbcHandle, &statHandle1);

	if ((retcode != SQL_SUCCESS) && (retcode != SQL_SUCCESS_WITH_INFO))
	{
		return retcode;
	}

	retcode = SQLExecDirect(statHandle1, (SQLCHAR *) query , SQL_NTS);
	if ((retcode != SQL_SUCCESS) && (retcode != SQL_SUCCESS_WITH_INFO))
	{
		return retcode;
	}
	
	retcode=SQLColAttribute (statHandle1,1,SQL_DESC_AUTO_UNIQUE_VALUE,NULL,0,NULL,&auton);

	SQLFreeHandle(SQL_HANDLE_STMT, statHandle1);

	if (!SQL_SUCCEEDED(retcode = SQLColumns(statHandle, NULL, 0, NULL, 0, (SQLCHAR *) table, SQL_NTS, NULL,0)))
		return -1;



	while (SQL_SUCCEEDED(SQLFetch(statHandle)))
	{
		SQLGetData(statHandle, SQLColumns_COLUMN_NAME, SQL_C_CHAR, colname, sizeof(colname), 0);
		

		if (strcmp((char *) colname, field) == 0)
		{
			SQL_SUCCEEDED(SQLGetData(statHandle, SQLColumns_SQL_DATA_TYPE, SQL_C_CHAR, coltype, sizeof(coltype), 0));
			SQL_SUCCEEDED(SQLGetData(statHandle, SQLColumns_COLUMN_SIZE, SQL_C_CHAR, precision, sizeof(precision), 0));
			

			break;
		}

	}

	info->name = NULL;
	info->type = conv_type(atol((char *)coltype));
	info->length = 0;
	if (*precision)
		info->length = atol((char *) precision);

	if (info->type == GB_T_STRING)
	{
		if (info->length < 0)
			info->length = 0;
	}
	
	if (auton == SQL_TRUE)
		info->type = DB_T_SERIAL;

	info->def.type = GB_T_NULL;

	info->collation = NULL;

	SQLFreeHandle(SQL_HANDLE_STMT, statHandle);

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

static int index_list(DB_DATABASE *db, const char *table, char ***indexes)
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

static int index_info(DB_DATABASE *db, const char *table, const char *index,	DB_INDEX * info)
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

static int index_delete(DB_DATABASE *db, const char *table, const char *index)
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

static int index_create(DB_DATABASE *db, const char *table, const char *index, DB_INDEX * info)
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

static int database_exist(DB_DATABASE *db, const char *name)
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

static int database_list(DB_DATABASE *db, char ***databases)
{
	ODBC_CONN *han = (ODBC_CONN *)db->handle;

//GB.Error("ODBC does not implement this function");
	GB.NewArray(databases, sizeof(char *), 1);
	(*databases)[0] = GB.NewZeroString(han->dsn_name);

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

static int database_is_system(DB_DATABASE *db, const char *name)
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

static char *table_type(DB_DATABASE *db, const char *table, const char *type)
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

static int database_delete(DB_DATABASE *db, const char *name)
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

static int database_create(DB_DATABASE *db, const char *name)
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

static int user_exist(DB_DATABASE *db, const char *name)
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

static int user_list(DB_DATABASE *db, char ***users)
{
	ODBC_CONN *han = (ODBC_CONN *)db->handle;
	GB.NewArray(users, sizeof(char *), 1);
	(*users)[0] = GB.NewZeroString(han->user_name);

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

static int user_info(DB_DATABASE *db, const char *name, DB_USER * info)
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

static int user_delete(DB_DATABASE *db, const char *name)
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

static int user_create(DB_DATABASE *db, const char *name, DB_USER * info)
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

static int user_set_password(DB_DATABASE *db, const char *name, const char *password)
{
	//GB.Error("ODBC can't set user's password");
	return TRUE;
}

/*****************************************************************************

  The driver interface

*****************************************************************************/

DECLARE_DRIVER(_driver, "odbc");

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
