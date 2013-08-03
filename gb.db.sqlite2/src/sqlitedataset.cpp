/***************************************************************************

  sqlitedataset.cpp

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

/**********************************************************************
 * Copyright (c) 2002, Leo Seib, Hannover
 *
 * Project:SQLiteDataset C++ Dynamic Library
 * Module: SQLiteDataset class realisation file
 * Author: Leo Seib      E-Mail: lev@almaty.pointstrike.net
 * Begin: 5/04/2002
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 **********************************************************************/


#include <stdio.h>
#include <ctype.h>
#include <stdarg.h>
#include <time.h>
#include <iostream>
#include <string.h>
#include <math.h>

#include "sqlitedataset.h"
#include <unistd.h>

#include "gambas.h"


/**************************************************************/

//************* Callback function ***************************

int callback(void* res_ptr,int ncol, char** reslt,char** cols){

	/* NG: Type definition */
//   typedef vector<string> Tables;
   Tables tables;
   Tables::iterator it;

   char *item;
   char *table;

   result_set* r = (result_set*)res_ptr;//dynamic_cast<result_set*>(res_ptr); 
   int sz = r->records.size();

   //if (reslt == NULL ) cout << "EMPTY!!!\n";
   if (!r->record_header.size()){ /*IF*/ 
     for (int i=0; i < ncol; i++) {
         item = strchr(cols[i], (int)'.');
	 if (!item){ /* Field does not include table info */
             item = cols[i];
                 r->record_header[i].name = item; //NG
                 table = NULL;
                 r->record_header[i].field_table = "";
                 r->record_header[i].type = ft_String;//default type to string
	 }
	 else {
             //table = strndup(cols[i], strchr(cols[i], (int)'.') - cols[i]);
             table = GB.NewString(cols[i], strchr(cols[i], (int)'.') - cols[i]);
//             table = strdup(reslt[0]);
             r->record_header[i].name = item + 1;
             r->record_header[i].field_table = table;
             r->record_header[i].type = ft_String;//default type to string
	 }
	 
      if (!table) {
	      /* Field does not contain table info,
	       * so let's default to string.  This
	       * has probably happened because aliases
	       * are being used */
      }
      else {
	      /* Check Table Name and add to list */
	      bool TableRegistered = false;
	      for ( it = tables.begin(); it != tables.end(); it++ ){
		      if (strcmp((*it).data(), table) == 0)
			      TableRegistered = true;
	      }
	      if ( TableRegistered == false ){
		      tables.push_back(table);
	      }
      }
      GB.FreeString(&table); //from strdup
     }

     SetFieldType( r, tables); // Set all the field types

     for (int i=0; i < ncol; i++) {
      /* Should table name be included in field name */
      if (tables.size() > 1){
         r->record_header[i].name = cols[i];
      }
      
/*      if (tables.size() < 1){
            r->record_header[i].type = ft_String;//Where type cannot be found
	                                         //default to string
      }*/
     }
   }/* IF close */ 

	if (reslt != NULL)
	{
		for (int i = 0; i < ncol; i++)
		{
			//fprintf(stderr, "callback: [%d] %s: %s\n", i, cols[i], reslt[i]);
			if (reslt[i] == NULL)
			{
				r->records[sz][i].set_isNull(r->record_header[i].type);
			}
			else
			{
				r->records[sz][i].set_asString(reslt[i], r->record_header[i].type);
			}
		}
	}
   //printf("Fsz: [%i]\n",r->record_header.size());
   //printf("Recs: [%i]\n",r->records.size());
   //cout << " val |" <<reslt<<"|"<<cols<<"|"<<"\n\n";
  return 0;  
}


//************* SqliteDatabase implementation ***************

SqliteDatabase::SqliteDatabase() {

  active = false;	
  _in_transaction = false;		// for transaction

  error = "Unknown database error";//S_NO_CONNECTION;
  host = "";
  port = "";
  db = ":memory:";
  login = "";
  passwd = "";
}

SqliteDatabase::~SqliteDatabase() {
  disconnect();
}


Dataset* SqliteDatabase::CreateDataset() const {
	return new SqliteDataset((SqliteDatabase*)this); 
}

int SqliteDatabase::status(void) {
  if (active == false) return DB_CONNECTION_NONE;
  return DB_CONNECTION_OK;
}

int SqliteDatabase::setErr(int err_code){

	last_err = err_code;

  switch (err_code) {
  case SQLITE_OK: error ="Successful result";
    break;
  case SQLITE_ERROR: error = "SQL error or missing database";
    break;
  case SQLITE_INTERNAL: error = "Internal logic error - Report this error on the mailing-list at sqlite.org";
    break;
  case SQLITE_PERM: error ="Access permission denied";
    break;
  case SQLITE_ABORT: error = "Callback routine requested an abort";
    break;
  case SQLITE_BUSY: error = "The database file is locked";
    break;
  case SQLITE_LOCKED: error = "A table in the database is locked";
    break;
  case SQLITE_NOMEM: error = "Out of memory";
    break;
  case SQLITE_READONLY: error = "Attempt to write a readonly database";
    break;
  case SQLITE_INTERRUPT: error = "Operation terminated by sqlite_interrupt()";
    break;
  case  SQLITE_IOERR: error = "Some kind of disk I/O error occurred";
    break;
  case  SQLITE_CORRUPT: error = "The database disk image is malformed";
    break;
  case SQLITE_NOTFOUND: error = "(Internal Only) Table or record not found";
    break;
  case SQLITE_FULL: error = "Insertion failed because database is full";
    break;
  case SQLITE_CANTOPEN: error = "Unable to open the database file";
    break;
  case SQLITE_PROTOCOL: error = "Database lock protocol error";
    break;
  case SQLITE_EMPTY:  error = "(Internal Only) Database table is empty";
    break;
  case SQLITE_SCHEMA: error = "The database schema changed";
    break;
  case SQLITE_TOOBIG: error = "Too much data for one row of a table";
    break;
  case SQLITE_CONSTRAINT: error = "Abort due to constraint violation";
    break;
  case SQLITE_MISMATCH:  error = "Data type mismatch";
    break;
  default : error = "Undefined SQLite error";
  }
  return err_code;
}

const char *SqliteDatabase::getErrorMsg() {
   return error.c_str();
}

int SqliteDatabase::connect() {
  disconnect();

  if ((conn = sqlite_open(db.c_str(),0,NULL))) {
    //cout << "Connected!\n";
    char* err=NULL;
    if (setErr(sqlite_exec(getHandle(),"PRAGMA empty_result_callbacks=ON",NULL,NULL,&err)) != SQLITE_OK) {
      GB.Error(getErrorMsg());
    }
    active = true;
    if (setErr(sqlite_exec(getHandle(),"PRAGMA full_column_names=ON",NULL,NULL,&err)) != SQLITE_OK) { //NG
      GB.Error(getErrorMsg()); //NG
    }
    return DB_CONNECTION_OK;
  }
  return DB_CONNECTION_NONE;
};

void SqliteDatabase::disconnect(void) {
  if (active == false) return;
  sqlite_close(conn);
  active = false;
};

int SqliteDatabase::create() {
  return connect();
};

int SqliteDatabase::drop() {
  if (active == false) return DB_ERROR;
  disconnect();
  if (!unlink(db.c_str())) 
     return DB_ERROR;
  return DB_COMMAND_OK;
};


long SqliteDatabase::nextid(const char* sname) {
  if (!active) return DB_UNEXPECTED_RESULT;
  int id;
  result_set res;
  char sqlcmd[512];
  sprintf(sqlcmd,"select nextid from %s where seq_name = '%s'",sequence_table.c_str(), sname);
  res.conn = getHandle(); //NG
  if ((last_err = sqlite_exec(getHandle(),sqlcmd,&callback,&res,NULL)) != SQLITE_OK) {
    return DB_UNEXPECTED_RESULT;
    }
  if (res.records.size() == 0) {
    id = 1;
    sprintf(sqlcmd,"insert into %s (nextid,seq_name) values (%d,'%s')",sequence_table.c_str(),id,sname);
    if ((last_err = sqlite_exec(conn,sqlcmd,NULL,NULL,NULL)) != SQLITE_OK) return DB_UNEXPECTED_RESULT;
    return id;
  }
  else {
    id = res.records[0][0].get_asInteger()+1;
    sprintf(sqlcmd,"update %s set nextid=%d where seq_name = '%s'",sequence_table.c_str(),id,sname);
    if ((last_err = sqlite_exec(conn,sqlcmd,NULL,NULL,NULL)) != SQLITE_OK) return DB_UNEXPECTED_RESULT;
    return id;    
  }
}


// methods for transactions
// ---------------------------------------------
void SqliteDatabase::start_transaction() {
  if (active) {
    last_err = sqlite_exec(conn,"begin",NULL,NULL,NULL);
    _in_transaction = true;
  }
}

void SqliteDatabase::commit_transaction() {
  if (active) {
    last_err = sqlite_exec(conn,"commit",NULL,NULL,NULL);
    _in_transaction = false;
  }
}

void SqliteDatabase::rollback_transaction() {
  if (active) {
    last_err = sqlite_exec(conn,"rollback",NULL,NULL,NULL);
    _in_transaction = false;
  }  
}



//************* SqliteDataset implementation ***************

SqliteDataset::SqliteDataset():Dataset() {
  haveError = false;
  db = NULL;
  errmsg = NULL;
}


SqliteDataset::SqliteDataset(SqliteDatabase *newDb):Dataset(newDb) {
  haveError = false;
  db = newDb;
  errmsg = NULL;
}

 SqliteDataset::~SqliteDataset(){
   if (errmsg) sqlite_free_table(&errmsg);
 }



//--------- protected functions implementation -----------------//

sqlite* SqliteDataset::handle(){
  if (db != NULL){
    return dynamic_cast<SqliteDatabase*>(db)->getHandle();
      }
  else return NULL;
}

void SqliteDataset::make_query(StringList &_sql) {
  string query;

 try {

  if (autocommit) db->start_transaction();


  if (db == NULL) GB.Error("No Database Connection");

  //close();


  for (list<string>::iterator i =_sql.begin(); i!=_sql.end(); i++) {
	query = *i;
	char* err=NULL; 
	Dataset::parse_sql(query);
	//cout << "Executing: "<<query<<"\n\n";
	if (db->setErr(sqlite_exec(this->handle(),query.c_str(),NULL,NULL,&err))!=SQLITE_OK) {
	  GB.Error(db->getErrorMsg());
	}
  } // end of for


  if (db->in_transaction() && autocommit) db->commit_transaction();

  active = true;
  ds_state = dsSelect;		
  refresh();

 } // end of try
 catch(...) {
  if (db->in_transaction()) db->rollback_transaction();
 }

}


void SqliteDataset::make_insert() {
  make_query(insert_sql);
  last();
}


void SqliteDataset::make_edit() {
  make_query(update_sql);
}


void SqliteDataset::make_deletion() {
  make_query(delete_sql);
}


void SqliteDataset::fill_fields() {
  //cout <<"rr "<<result.records.size()<<"|" << frecno <<"\n";
  if ((db == NULL) || (result.record_header.size() == 0) || (result.records.size() < (uint)frecno)) return;
  if (fields_object->size() == 0) // Filling columns name
    for (uint i = 0; i < result.record_header.size(); i++) {
      (*fields_object)[i].props = result.record_header[i];
      //(*edit_object)[i].props = result.record_header[i];
    }

  //Filling result
  if (result.records.size() != 0) {
   for (uint i = 0; i < result.records[frecno].size(); i++){
    (*fields_object)[i].val = result.records[frecno][i];
    //(*edit_object)[i].val = result.records[frecno][i];
   }
  }
  else
	{
		field_value tmp;
		for (uint i = 0; i < result.record_header.size(); i++)
			(*fields_object)[i].val = tmp;
	}

}


//------------- public functions implementation -----------------//

// BM: should retry if error = SQLITE_SCHEMA

int SqliteDataset::exec(const string &sql) 
{
  int res;
  int retry;
  
  
  if (!handle())
    GB.Error("No Database Connection");
    
  exec_res.record_header.clear();
  exec_res.records.clear();
  exec_res.conn = handle();//NG
  //if ((strncmp("select",sql.c_str(),6) == 0) || (strncmp("SELECT",sql.c_str(),6) == 0)) 
  
  for (retry = 1; retry <= 2; retry++)
  {
    res = sqlite_exec(handle(), sql.c_str(), &callback, &exec_res, &errmsg);
    if (res != SQLITE_SCHEMA)
      break;
  }
  
  db->setErr(res);
  //if (res != SQLITE_OK)
  //  GB.Error(db->getErrorMsg());
  
  return (res == SQLITE_OK);
}

int SqliteDataset::exec() {
	return exec(sql);
}

const void* SqliteDataset::getExecRes() {
  return &exec_res;
}


bool SqliteDataset::query(const char *query) {

  int res;
  int retry;

  
  //try{
    if (db == NULL) GB.Error("Database is not Defined");
    if(dynamic_cast<SqliteDatabase*>(db)->getHandle() == NULL) GB.Error("No Database Connection");
  if ((strncasecmp("select",query,6) != 0) /*&&
	  (strncasecmp("PRAGMA table",query,12) !=0) &&
	  (strncasecmp("PRAGMA index",query,12) !=0)*/
	  ) GB.Error("MUST be select SQL or PRAGMA table or index!"); 

  //close();

  //cout <<  "Curr size "<<num_rows()<<"\n\n";
  result.conn = handle();//NG

  for (retry = 1; retry <= 2; retry++)
  {
    res = sqlite_exec(handle(), query, &callback, &result, &errmsg);
    if (res != SQLITE_SCHEMA)
      break;
  }
  
  db->setErr(res);
  
  if (res == SQLITE_OK)
  {
    active = true;
    ds_state = dsSelect;
    //cout <<  "Curr size2 "<<num_rows()<<"\n";
    this->first();
    //cout <<  "Curr size3 "<<num_rows()<<"\n";
    //cout <<  "Curr fcount "<<field_count()<<"\n\n";
  }
  
  return (res == SQLITE_OK);
}

bool SqliteDataset::query(const string &q){
  return query(q.c_str());
}

void SqliteDataset::open(const string &sql) {
	set_select_sql(sql);
	open();
}

void SqliteDataset::open() {
  if (select_sql.size()) {
    query(select_sql.c_str()); 
  }
  else {
    ds_state = dsInactive;
  }
}


void SqliteDataset::close() {
  Dataset::close();
  result.record_header.clear();
  result.records.clear();
  edit_object->clear();
  fields_object->clear();
  ds_state = dsInactive;
  active = false;
  delete this;
}


void SqliteDataset::cancel() {
  if ((ds_state == dsInsert) || (ds_state==dsEdit))
	{
    if (result.record_header.size()) ds_state = dsSelect;
    else ds_state = dsInactive;
	}
}


int SqliteDataset::num_rows() {
  return result.records.size();
}


bool SqliteDataset::eof() {
  return feof;
}


bool SqliteDataset::bof() {
  return fbof;
}


void SqliteDataset::first() {
  Dataset::first();
  this->fill_fields();
  //cout << "In first "<< fields_object->size()<<"\n";
}

void SqliteDataset::last() {
  Dataset::last();
  fill_fields();
}

void SqliteDataset::prev(void) {
  Dataset::prev();
  fill_fields();
}

void SqliteDataset::next(void) {
  Dataset::next();
  if (!eof()) 
      fill_fields();
}


//bool SqliteDataset::seek(int pos=0) {
bool SqliteDataset::seek(int pos) {
  if (ds_state == dsSelect) {
    Dataset::seek(pos);
    fill_fields();
    return true;	
    }
  return false;
}



long SqliteDataset::nextid(const char *seq_name) {
  if (handle()) return db->nextid(seq_name);
  else return DB_UNEXPECTED_RESULT;
}

/* Helper function */
void SetFieldType( result_set *r, Tables tables){

   Tables::iterator it;

   sqlite_vm *vm;
   const char *tail;
   const char **vals;
   const char **names;
   int ncol;
   unsigned int len;
	
//  result_set res;
  char sqlcmd[512];

  for ( it = tables.begin(); it != tables.end(); it++ ){
      sprintf(sqlcmd,"PRAGMA table_info('%s')", (*it).data());
      if ( sqlite_compile(r->conn,sqlcmd,&tail,&vm,NULL) != SQLITE_OK) {
	   return;
      }
      while ( sqlite_step(vm, &ncol, &vals, &names) == SQLITE_ROW ){
	      /* Type is in 2 */
	      /* field name is 1 */
	      /* not null indicator is 3 */
	      /* dflt value is 4 */

	      for (uint e=0; e<r->record_header.size(); e++){
 
		  if ( r->record_header[e].name == vals[1] &&
		       r->record_header[e].field_table == (*it).data()) { 


		     r->record_header[e].type = GetFieldType((char *)vals[2], &len);
		     r->record_header[e].field_len = len;
                  /* Is not null */
                     r->record_header[e].notnull = vals[3][0];
		  }
	      } /* For end */
      } /* while end */

      sqlite_finalize(vm, NULL);
  }
}

/* Return fType and length from String field*/
fType GetFieldType(const char *Type, unsigned int *length )
{
	char *upper;
	char *_left, *_right;
	fType rType; /* For return */
	unsigned int rTypeLen;
	int i;

	upper = GB.NewZeroString(Type);
	for (i = 0; i < GB.StringLength(upper); i++)
		upper[i] = toupper(upper[i]);
	
	Type = upper;
	if (!Type) Type = "";

     if (strstr(Type, "BLOB")
        || strstr(Type, "CHAR(") /* note the opening bracket */
        || strstr(Type, "CLOB")
        || strstr(Type, "TEXT") /* also catches TINYTEXT */
        || strstr(Type, "VARCHAR")
        || strstr(Type, "ENUM")
        || strstr(Type, "SET")
        || strstr(Type, "YEAR")) { /* MySQL 2 or 4 digit year (string) */
                 rType = ft_String;
     }
     else if (strstr(Type, "CHAR") /* this is a 1-byte value */
  	     || strstr(Type, "TINYINT")
	     || strstr(Type, "INT1")
	     || strstr(Type, "BOOL")) {
                rType = ft_Boolean;
	        /* Length is for when value is used as a string */
                   rTypeLen = ft_Boolean_Length;
     }
     else if (strstr(Type, "SMALLINT")
             || strstr(Type, "INT2")) {
                rType = ft_Short;
	        /* Length is for when value is used as a string */
                   rTypeLen = ft_Short_Length;
     }
     else if (strstr(Type, "MEDIUMINT")) {
                rType = ft_Short;
	        /* Length is for when value is used as a string */
                   rTypeLen = ft_Short_Length;
     }
     else if (strstr(Type, "BIGINT")
             || strstr(Type, "INT8")) {
                rType = ft_LongDouble;
	        /* Length is for when value is used as a string */
                   rTypeLen = ft_LongDouble_Length;
     }
     else if (strstr(Type, "INTEGER")
       	     || strstr(Type, "INT")
	     || strstr(Type, "INT4")) {
                rType = ft_Long;
	        /* Length is for when value is used as a string */
                rTypeLen = ft_Long_Length;
     }
     else if (strstr(Type, "DECIMAL") ||
  		strstr(Type, "NUMERIC")) {
                rType = ft_Float;
		/* Length is for when value is used as a string */
                rTypeLen = ft_Float_Length;
     }
     else if (strstr(Type, "TIMESTAMP")
             || strstr(Type, "DATETIME")) {
                rType = ft_Date;
                rTypeLen = ft_Date_Length;
     }
     else if (strstr(Type, "DATE")) {
               rType = ft_Date;
	       /* Length is for when value is used as a string */
                rTypeLen = ft_Date_Length;
     }
     else if (strstr(Type, "TIME")) {
               rType = ft_Date;
	       /* Length is for when value is used as a string */
                rTypeLen = ft_Date_Length;
     }
     else if (strstr(Type, "DOUBLE") || strstr(Type, "FLOAT8")) 
     {
              rType = ft_Double;
	      /* Length is for when value is used as a string */
                rTypeLen = ft_Double_Length;
     }
     else if (strstr(Type, "REAL") /* this is PostgreSQL "real", not
					MySQL "real" which is a
					synonym of "double" */
      	  || strstr(Type, "FLOAT")
   	  || strstr(Type, "FLOAT4")) {
             rType = ft_Float;
             /* Length is for when value is used as a string */
                rTypeLen = ft_Float_Length;
     }
     else {
             rType = ft_String;
    			  /* most reasonable default */
     }
 
     if ( rType == ft_String){
    	  /* if a length has been defined it will be between ()*/
     	    _right = (char *)rindex(Type, ')');
     	    _left = (char *)index(Type, '(');
	     if (_right){
	 	_right = '\0';
                rTypeLen = atoi(_left + 1);
	     }
	     else {
        	/* set a default length */
                rTypeLen = DEFAULT_STRING_LENGTH;
     	     }
     }

     if ( length != NULL )
	     *length = rTypeLen;

	GB.FreeString(&upper);
  
	return rType;    
}

