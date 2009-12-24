/***************************************************************************

  main.cpp

  (c) 2000-2009 Benoît Minisini <gambas@users.sourceforge.net>

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
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

***************************************************************************/

#define __MAIN_C

#include <iostream>

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include <algorithm>
#include <map>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>

#define IBPP_LINUX
#define IBPP_GCC

#include <ibase.h>
#include <ibpp.h>
#include <iberror.h>

#include "main.h"

static char _buffer[32];
static int _last_error;

extern "C"{
  GB_INTERFACE GB EXPORT;
  DB_INTERFACE DB EXPORT;
}

typedef std::map<int, GB_VARIANT> GB_ROW;
typedef std::map<int, GB_ROW> GB_ROWS;
typedef std::vector<IBPP::User> UsersL;


class FBResult{
  private:
    GB_ROWS gb_table;
    unsigned nrecord;
    IBPP::Statement st1;
  public:
    FBResult(){
      nrecord=0;
    }

    GB_VARIANT GetData(int l, int c){
      return gb_table[l][c];
    };

    bool IsNull(int p){
      return st1->IsNull(p);
    };

    void Get(int p,int64_t &data){
      st1->Get(p,data);
    };

    void Get(int p,int &data){
      st1->Get(p,data);
    };

    void Get(int p,float &data){
      st1->Get(p,data);
    };

    void Get(int p,double &data){
      st1->Get(p,data);
    };

    void Get(int p,IBPP::Time &data){
      st1->Get(p,data);
    };

    void Get(int p,IBPP::Timestamp &data){
      st1->Get(p,data);
    };

    void Get(int p,IBPP::Date &data){
      st1->Get(p,data);
    };

    void Get(int p,std::string &data){
      st1->Get(p,data);
    };

    void Get(int p,IBPP::Blob &data){
      st1->Get(p,data);
    };

    void SetData(int l,int c,int data){
      gb_table[l][c].type = GB_T_VARIANT;
      gb_table[l][c].value.type = GB_T_INTEGER;
      if(!IsNull(c+1)){
        gb_table[l][c].value.value._integer = data;
      };
    };

    void SetData(int l,int c,int64_t data){
      gb_table[l][c].type = GB_T_VARIANT;
      gb_table[l][c].value.type = GB_T_LONG;
      if(!IsNull(c+1)){
        gb_table[l][c].value.value._long = data;
      };
    };

    void SetData(int l,int c,float data){
      gb_table[l][c].type = GB_T_VARIANT;
      gb_table[l][c].value.type = GB_T_FLOAT;
      if(!IsNull(c+1)){
        gb_table[l][c].value.value._float = data;
      };
    };

    void SetData(int l,int c,double data){
      gb_table[l][c].type = GB_T_VARIANT;
      gb_table[l][c].value.type = GB_T_FLOAT;
      if(!IsNull(c+1)){
        gb_table[l][c].value.value._float = data;
      };
    };

    void SetData(int l,int c,IBPP::Time data){
      GB_DATE_SERIAL dt1;
      GB_DATE dt2;
      int hour,min,sec;
      gb_table[l][c].type = GB_T_VARIANT;
      gb_table[l][c].value.type = GB_T_DATE;
      if(!IsNull(c+1)){
        data.GetTime(hour,min,sec);
        memset(&dt1, 0, sizeof(dt1));
        dt1.hour=hour;
        dt1.min=min;
        dt1.sec=sec;
        GB.MakeDate(&dt1,&dt2);
        gb_table[l][c].value.value._date.date = 0;
        gb_table[l][c].value.value._date.time = dt2.value.time;
      };
    };

    void SetData(int l,int c,IBPP::Timestamp data){
      GB_DATE_SERIAL dt1;
      GB_DATE dt2;
      int year,month,day,hour,min,sec;
      gb_table[l][c].type = GB_T_VARIANT;
      gb_table[l][c].value.type = GB_T_DATE;
      if(!IsNull(c+1)){
        data.GetDate(year,month,day);
        data.GetTime(hour,min,sec);
        memset(&dt1, 0, sizeof(dt1));
        dt1.year=year;
        dt1.month=month;
        dt1.day=day;
        dt1.hour=hour;
        dt1.min=min;
        dt1.sec=sec;
        GB.MakeDate(&dt1,&dt2);
        gb_table[l][c].value.value._date.date = dt2.value.date;
        gb_table[l][c].value.value._date.time = dt2.value.time;
      };
    };

    void SetData(int l,int c,IBPP::Date data){
      GB_DATE_SERIAL dt1;
      GB_DATE dt2;
      int year,month,day;
      gb_table[l][c].type = GB_T_VARIANT;
      gb_table[l][c].value.type = GB_T_DATE;
      if(!IsNull(c+1)){
        data.GetDate(year,month,day);
        memset(&dt1, 0, sizeof(dt1));
        dt1.year=year;
        dt1.month=month;
        dt1.day=day+1;
        GB.MakeDate(&dt1,&dt2);
        gb_table[l][c].value.value._date.date = dt2.value.date;
        gb_table[l][c].value.value._date.time = 0;
      };
    };

    void SetData(int l,int c,std::string data){
      gb_table[l][c].type = GB_T_VARIANT;
      gb_table[l][c].value.type = GB_T_CSTRING;
      if(!IsNull(c+1))
        GB.NewString(&gb_table[l][c].value.value._string, data.c_str(), 0);
    };

    IBPP::STT GetType(void){
      return st1->Type();
    };

    bool Fetch(void){
      return st1->Fetch();
    };

		bool Execute(char *query)
		{
			try
			{
				st1->Execute(query);
			}
			catch (IBPP::Exception& e)
			{
				std::cout<<e.what()<<std::endl;
				IBPP::SQLException* pe=dynamic_cast<IBPP::SQLException *>(&e);
				if (pe && pe->EngineCode()!=335544321)
				{
					_last_error = pe->SqlCode();
					GB.Error("Query failed");
					return FALSE;
				}
				else
				{
					_last_error = pe->EngineCode();
					GB.Error("Arithmetic exception, numeric overflow, or string truncation !");
					return FALSE;
				}
			}
			return TRUE;
		};

    void SetStatement(IBPP::Database db1,IBPP::Transaction tr1){
      st1 = IBPP::StatementFactory(db1, tr1);
    };

    IBPP::Statement Statement(void){
      return st1;
    };
    unsigned Columns(void){
      return st1->Columns();
    };

    char * ColumnName(unsigned field){
      return (char *) st1->ColumnName(field);
    };

    int ColumnNum(std::string name){
      return st1->ColumnNum(name);
    };

    IBPP::SDT ColumnType(unsigned field){
      return st1->ColumnType(field);
    };

    int ColumnSize(unsigned field){
      return st1->ColumnSize(field);
    };

    int ColumnScale(unsigned field){
      return st1->ColumnScale(field);
    };

    const char* ColumnTable(unsigned field){
      return st1->ColumnTable(field);
    };

    unsigned GetnRecord(void){
      return nrecord;
    };

    unsigned SetnRecord(unsigned nrecordtmp=0){
      return nrecord=nrecordtmp;
    };

    void ClearResult(void){
      for(unsigned i=0;i<gb_table.size();i++){
        for(unsigned j=0;j<gb_table[i].size();j++){
          if(gb_table[i][j].value.type == GB_T_CSTRING){
            GB.FreeString(&gb_table[i][j].value.value._string);
          }
        }
        gb_table[i].clear();
      };
      gb_table.clear();
      nrecord=0;
      st1->Close();
    };

    ~FBResult(){
      ClearResult();
      st1->Close();
    };
};

class FBConnect{
  private:
    bool in_connection;
    std::string Host;
    std::string Db;
    std::string User;
    std::string Password;
    std::string Role;
    IBPP::Database db1;
    IBPP::Transaction tr1;
  public:
    FBConnect(){
      in_connection=FALSE;
    }

    bool Connect(void){
      try{
        db1=IBPP::DatabaseFactory(Host, Db, User, Password);
        db1->Connect();
        in_connection=TRUE;
      }
      catch (IBPP::Exception& e){
        std::cout<<e.what()<<std::endl;
        IBPP::SQLException* pe=dynamic_cast<IBPP::SQLException *>(&e);
        _last_error = pe->EngineCode();
        if(pe && pe->EngineCode()==335544472){
          GB.Error("Your user name and password not defined !");
          return FALSE;
        }
        if(pe && pe->EngineCode()!=335544344){
          GB.Error(pe->ErrorMessage());
          return FALSE;
        }
        if(!CreateDataBase()){
          GB.Error("Error on creation, look at user permission\nor file or directory");
          return FALSE;
        };
      }
      return TRUE;
    };

    bool Connect(std::string host, std::string name, std::string user, std::string password){
      Host=host;
      Db=name;
      User=user;
      Password=password;
      Role="";
      try{
        db1=IBPP::DatabaseFactory(host, name, user, password);
        db1->Connect();
        in_connection=TRUE;
      }
      catch (IBPP::Exception& e){
        std::cout<<e.what()<<std::endl;
        IBPP::SQLException* pe=dynamic_cast<IBPP::SQLException *>(&e);
        _last_error = pe->EngineCode();
        if(pe && pe->EngineCode()==335544472){
          GB.Error("Your user name and password not defined !");
          return FALSE;
        }
        if(pe && pe->EngineCode()!=335544344){
          GB.Error(e.what());
          return FALSE;
        }
        if(!CreateDataBase()){
          return FALSE;
        };
      }
      return TRUE;
    };

    bool Connected(void){
      return in_connection;
    };

    bool Disconnect(void){
      try{
        db1->Disconnect();
      }
      catch(IBPP::Exception& e){
        std::cout<<e.what()<<std::endl;
        GB.Error("Error on Disconnect !");
        return FALSE;
      }
      in_connection=FALSE;
      return TRUE;
    };

    bool SetTransaction(IBPP::TAM am = IBPP::amWrite,IBPP::TIL il = IBPP::ilConcurrency, IBPP::TLR lr = IBPP::lrWait, IBPP::TFF flags = IBPP::TFF(0)){
      try{
        tr1 = IBPP::TransactionFactory(db1,am,il,lr,flags);
      }
      catch(IBPP::Exception& e){
        std::cout<<e.what()<<std::endl;
        GB.Error("Error to Setup Transaction !");
        return FALSE;
      };
      return TRUE;
    };

    std::string DataBase(void){
      return Db;
    };

    std::string DataBase(std::string dbnametmp){
      Db=dbnametmp;
      return Db;
    };

    bool CreateDataBase(int dialect=3){
      try{
        db1->Create(dialect);
      }
      catch(IBPP::Exception& e){
        std::cout<<e.what()<<std::endl;
        in_connection=FALSE;
        GB.Error("Error to creat DataBase, look at user permission or file or directory");
        return FALSE;
      };
      in_connection=TRUE;
      return TRUE;
    };

    bool DropDataBase(void){
      if(db1->Connected()){
        try{
          db1->Drop();
        }
        catch(IBPP::Exception& e){
	  std::cout<<e.what()<<std::endl;
	  GB.Error("Error to Drop DataBase !");
          return FALSE;
        };
        Db="";
        in_connection=FALSE;
        return TRUE;
      };
      return FALSE;
    };

    void Start(void){
      tr1->Start();
    };

    bool Started(void){
      return tr1->Started();
    };

    void Commit(void){
      tr1->Commit();
    };

    void Rollback(void){
      tr1->Rollback();
    };

    ~FBConnect(){
      tr1->Commit();
      db1->Disconnect();
    };

    std::string GetVersion(std::string host, std::string user, std::string password){
      std::string str1;
      IBPP::Service service = IBPP::ServiceFactory(host, user, password);
      try{
        service->Connect();
      }
      catch(IBPP::Exception& e){
        std::cout<<e.what()<<std::endl;
	GB.Error(e.ErrorMessage());
        return str1;
      };
      try{
        service->GetVersion(str1);
      }
      catch(IBPP::Exception& e){
        GB.Error("your requete is not authorized, look at permission please !");
      }
      service->Disconnect();
      return str1;
    };

    std::string GetVersion(void){
      std::string str1;
      IBPP::Service service = IBPP::ServiceFactory(Host, User, Password);
      try{
      service->Connect();
      }
      catch(IBPP::Exception& e){
        std::cout<<e.what()<<std::endl;
	GB.Error("Error to Connect on Service Version !");
        return str1;
      }
      try{
      service->GetVersion(str1);
      }
      catch(IBPP::Exception& e){
        GB.Error("your requete is not authorized, look at permission please !");
      }
      service->Disconnect();
      return str1;
    };

    UsersL UserList(void){
      UsersL userl1;
      IBPP::Service service = IBPP::ServiceFactory(Host, User, Password);
      try{
      service->Connect();
      }
      catch(IBPP::Exception& e){
        std::cout<<e.what()<<std::endl;
	GB.Error("Error to Connect on Service User List !");
      }
      try{
      service->GetUsers(userl1);
      }
      catch(IBPP::Exception& e){
        GB.Error("your requete is not authorized, look at permission please !");
      }
      service->Disconnect();
      return userl1;
    };

    UsersL UserList(std::string host, std::string  user, std::string  password){
      UsersL userl1;
      IBPP::Service service = IBPP::ServiceFactory(host, user, password);
      try{
      service->Connect();
      }
      catch(IBPP::Exception& e){
        std::cout<<e.what()<<std::endl;
	GB.Error("Error to Connect on Service Get User List !");
      }
      try{
      service->GetUsers(userl1);
      }
      catch(IBPP::Exception& e){
        GB.Error("your requete is not authorized, look at permission please !");
      }
      service->Disconnect();
      return userl1;
    };

    bool UserInfo(std::string name, DB_USER *info){
      /*std::string username, password, first, middle, last;
      IBPP::Service service = IBPP::ServiceFactory(Host, User, Password);
      service->Connect();
      username=name;
      try{
        service->ModifyUser(username, password, first, middle, last);
        service->Disconnect();
      }
      catch(IBPP::Exception& e){
        return FALSE;
    };*/
      return TRUE;
    };

    bool RemoveUser(std::string username){
      IBPP::Service service = IBPP::ServiceFactory(Host, User, Password);

      try{
        service->Connect();
      }
      catch(IBPP::Exception& e){
        std::cout<<e.what()<<std::endl;
	GB.Error("Error to Connect on Service Remove User");
        return FALSE;
      };
      try{
      service->RemoveUser(username);
      }
      catch(IBPP::Exception& e){
        GB.Error("your requete is not authorized, look at permission please !");
      }
      service->Disconnect();
      return TRUE;
    };

    bool AddUser(std::string username,std::string password){
      IBPP::Service service = IBPP::ServiceFactory(Host, User, Password);
      IBPP::User user;

      try{
        service->Connect();
      }
      catch(IBPP::Exception& e){
        std::cout<<e.what()<<std::endl;
	GB.Error("Error to Connect on Service Add user !");
        return FALSE;
      };
      try{
        user.username = username;
        user.password = password;
        service->AddUser(user);
			}
      catch(IBPP::Exception& e){
        GB.Error("your requete is not authorized, look at permission please !");
        }
      service->Disconnect();
      return TRUE;
    };

    bool ModifyUser(std::string username,std::string password){
      IBPP::Service service = IBPP::ServiceFactory(Host, User, Password);
      IBPP::User user;

      try{
        service->Connect();
      }
      catch(IBPP::Exception& e){
        std::cout<<e.what()<<std::endl;
	GB.Error("Error to Connect on Service Modify User !");
        return FALSE;
      }
      try{
      user.username = username;
      user.password = password;
      service->ModifyUser(user);
      }
      catch(IBPP::Exception& e){
        GB.Error("your requete is not authorized, look at permission please !");
      }
      service->Disconnect();
      return TRUE;
    };

    int Execute(char *query=NULL, DB_RESULT *result=NULL, char *err=NULL);
};

int FBConnect::Execute(char *query, DB_RESULT *result, char *err){
  int autocommit=0, retour=FALSE;
  unsigned i;
  int i1;
  int64_t s1;
  float fl1;
  double dbl1;
  std::string str1;
  IBPP::Date d1;
  IBPP::Time t1;
  IBPP::Timestamp ts1;
  IBPP::SDT DataType;
  IBPP::Blob blb1=IBPP::BlobFactory(db1, tr1);
  FBResult *res;
  std::stringstream sstr;
  
  if (!query)
  	return retour;
  
  res=new FBResult;
  res->SetnRecord();
  if(!tr1->Started()){
    tr1->Start();
    autocommit=1;
  }
  res->SetStatement(db1, tr1);
  if(!res->Execute(query)){
    return TRUE;
  };
  if(res->GetType()==IBPP::stSelect){
    while(res->Fetch()){
      for(i=0;i<res->Columns();i++){
        DataType=res->ColumnType(i+1);
        if(res->ColumnScale(i+1))
          DataType=IBPP::sdDouble;
        switch(DataType){
          case IBPP::sdSmallint:
          case IBPP::sdInteger:
            i1=0;
            if(!res->IsNull(i+1)){
              res->Get(i+1,i1);
            }
            res->SetData(res->GetnRecord(),i,i1);
            break;

          case IBPP::sdLargeint:
            s1=0;
            if(!res->IsNull(i+1)){
              res->Get(i+1,s1);
            }
            res->SetData(res->GetnRecord(),i,s1);
            break;

          case IBPP::sdDouble:
            dbl1=0.00;
            if(!res->IsNull(i+1)){
              res->Get(i+1,dbl1);
            }
            res->SetData(res->GetnRecord(),i,dbl1);
            break;

          case IBPP::sdFloat:
            fl1=0.00;
            if(!res->IsNull(i+1)){
              res->Get(i+1,fl1);
            }
            res->SetData(res->GetnRecord(),i,fl1);
            break;

          case IBPP::sdTime:
            t1.Clear();
            if(!res->IsNull(i+1)){
              res->Get(i+1,t1);
            }
            res->SetData(res->GetnRecord(),i,t1);
            break;

          case IBPP::sdTimestamp:
            ts1.Clear();
            if(!res->IsNull(i+1)){
              res->Get(i+1,ts1);
            }
            res->SetData(res->GetnRecord(),i,ts1);
            break;

          case IBPP::sdDate:
            d1.Clear();
            if(!res->IsNull(i+1)){
              res->Get(i+1,d1);
            }
            res->SetData(res->GetnRecord(),i,d1);
            break;

          case IBPP::sdArray:
            str1="";
            if(!res->IsNull(i+1)){
              if(strncmp(res->ColumnTable(i+1),"RDB$",4)==0){
                res->Get(i+1,str1);
              }
              else{
                str1="<ARRAY>";
              }
            }
            res->SetData(res->GetnRecord(),i,str1);
            break;

          case IBPP::sdBlob:
            str1="";
            if(!res->IsNull(i+1)){
              char *buffer;
              int size, largest, segments;
                res->Get(i+1,blb1);
                blb1->Open();
                blb1->Info(&size, &largest, &segments);
                if((buffer=(char*) malloc(sizeof(char)*(size+1)))==NULL){
                  GB.Error("Error on Allocation memory for <BLOB>");
                  return TRUE;
                }
                blb1->Read(buffer,size);
                buffer[size]='\0';
                str1=buffer;
                blb1->Close();
                free(buffer);
            }
            res->SetData(res->GetnRecord(),i,str1);
            break;

          case IBPP::sdString:
          default:
            str1="";
            if(!res->IsNull(i+1)){
              res->Get(i+1,str1);
            }
            res->SetData(res->GetnRecord(),i,str1);
            break;
        };

      };
      res->SetnRecord(res->GetnRecord() + 1);
    }
    *result=(DB_RESULT)res;
  }
  else {
    retour=TRUE;
    *result=NULL;
    delete res;
  }
  if(tr1->Started() && autocommit==1){
    tr1->Commit();
  }
  return retour;
};

/*****************************************************************************

  The driver interface

*****************************************************************************/

DECLARE_DRIVER(_driver, "firebird");

/* internal function to quote a value stored as a string */

#if 0
static void quote(char *data, long len, DB_FORMAT_CALLBACK add)
{
	int i;
	unsigned char c;

	(*add)("'", 1);
	for (i = 0; i < len; i++)
	{
		c = (unsigned char)data[i];
		if (c == '\\')
			(*add)("\\\\", 2);
		else if (c == '\'')
			(*add)("''", 2);
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
#endif

/* internal function to quote a value stored as a blob */

static void quote_blob(char *data, long len, DB_FORMAT_CALLBACK add){
    //std::cout<<"quote blob"<<std::endl;
	int i;
	unsigned char c;

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
static int unquote(char *data, long len, DB_FORMAT_CALLBACK add)
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
#endif

/* internal function to unquote a value stored as a blob */

static int unquote_blob(char *data, int len, DB_FORMAT_CALLBACK add){
    //std::cout<<"unquote blob"<<std::endl;
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

/* Internal function to convert a database value into a Gambas variant value */

static GB_TYPE conv_type(int type){
  switch(type){
    /*case ft_Boolean:
    return GB_T_BOOLEAN;*/
    case IBPP::sdSmallint:
    case IBPP::sdInteger:
      return GB_T_INTEGER;

    case IBPP::sdLargeint:
      return GB_T_LONG;

    case IBPP::sdFloat:
    case IBPP::sdDouble:
      return GB_T_FLOAT;

    case IBPP::sdDate:
    case IBPP::sdTime:
    case IBPP::sdTimestamp:
      return GB_T_DATE;

    case IBPP::sdBlob:
      return DB_T_BLOB;

    case IBPP::sdString:
    default:
      return GB_T_STRING;
  }
};

/* Internal function to check database version number */

static int db_version(std::string str1){
  int dbversion=0;
  std::string mag(str1,4,1),med(str1,6,1),min(str1,8,1),s(str1,10,4);
  dbversion=atoi(mag.c_str())*1000000+atoi(med.c_str())*100000+atoi(min.c_str())*10000+atoi(s.c_str());
  return dbversion;
};

/*****************************************************************************

  get_quote()

  Returns the character used for quoting object names.

*****************************************************************************/

static const char *get_quote(void){
  return QUOTE_STRING;
};

/*****************************************************************************

  open_database()

  Connect to a database.

  <desc> points at a structure describing each connection parameter.

  This function must return a database handle, or NULL if the connection
  has failed.

  The name of the database can be NULL, meaning a default database.

*****************************************************************************/

static int open_database(DB_DESC *desc, DB_DATABASE *db){
  std::string nametmp=desc->name,usertmp=desc->user,passwordtmp=desc->password,hosttmp=desc->host;
  if(desc->port)
    hosttmp=hosttmp+"/"+desc->port;

  FBConnect *con=new FBConnect();
  std::transform(usertmp.begin(), usertmp.end(), usertmp.begin(), toupper);

  if (!IBPP::CheckVersion(IBPP::Version)){
    GB.Error("\nThis program got linked to an incompatible version of the library.\n"
        "Can't execute safely.\n");
    return TRUE;
  }
  if(!con->Connect(hosttmp, nametmp, usertmp, passwordtmp))
    return TRUE;
  db->version=db_version(con->GetVersion(hosttmp, usertmp, passwordtmp));
  if(!con->SetTransaction(IBPP::amWrite, IBPP::ilConcurrency, IBPP::lrWait))
    return TRUE;
    
	db->flags.no_table_type = TRUE;
	db->flags.no_serial = FALSE;
	db->flags.no_blob = FALSE;
	
	// Apparently idbp is always case unsensitive
	db->flags.no_case = TRUE;
	db->ignore_case = TRUE;
	
	db->handle=con;
	
	// BM: Firebird LIMIT syntax
	db->limit.keyword = "FIRST";
	db->limit.pos = DB_LIMIT_AT_BEGIN;
  return FALSE;
};

/*****************************************************************************

  close_database()

  Terminates the database connection.

  <handle> contains the database handle.

*****************************************************************************/

static void close_database(DB_DATABASE *db){
  FBConnect *con=(FBConnect *)db->handle;
  if(!con->Disconnect())
    GB.Error("No Database to Close !!!");
};

/*****************************************************************************

  format_blob()

  This function transforms a blob value into a string value that can
  be inserted into a SQL query.

  <blob> points to the DB_BLOB structure.
  <add> is a callback called to insert the string into the query.

*****************************************************************************/

static void format_blob(DB_BLOB *blob, DB_FORMAT_CALLBACK add)
{
    //std::cout<<"format blob"<<std::endl;
	quote_blob(blob->data, blob->length, add);
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
  char *s;
  int l;
  int i;
  GB_DATE_SERIAL *date;
  bool bc;
  switch (arg->type){
    case GB_T_BOOLEAN:
      if (VALUE((GB_BOOLEAN *)arg))
        add("TRUE", 4);
      else
        add("FALSE", 5);
      return TRUE;

    case GB_T_STRING:
    case GB_T_CSTRING:
      s = VALUE((GB_STRING *)arg).addr + VALUE((GB_STRING *)arg).start;
      l = VALUE((GB_STRING *)arg).len;
      if (arg->type == GB_T_STRING)
        add("'", 1);
      for (i = 0; i < l; i++, s++){
        add(s, 1);
        if (*s == '\'' || *s == '\\')
          add(s, 1);
      }
      if (arg->type == GB_T_STRING)
        add("'", 1);
      return TRUE;

    case GB_T_DATE:
      date = GB.SplitDate((GB_DATE *)arg);
      bc = date->year < 0;
      l = sprintf(_buffer, "'%04d-%02d-%02d %02d:%02d:%02d",abs(date->year), date->month, date->day,date->hour, date->min, date->sec);
      add(_buffer, l);
      if (date->msec){
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
  int retour=FALSE;
  FBConnect *con=(FBConnect *)db->handle;
  retour=con->Execute((char*)query,result,(char *)err);
  if (retour)
  	db->error = _last_error;
  return retour;
}

#define do_query(_db, _query, _result, _err) exec_query(_db, (char *)_query, (DB_RESULT *)(void *)_result, (char *)_err)


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
  FBResult *res = (FBResult *)result;
  if(res){
    *count = res->GetnRecord();
    info->nfield = res->Columns();
  }
  else{
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
  FBResult *res = (FBResult *)result;
  delete res;
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
  FBResult *res=(FBResult *)result;
  unsigned i;
  GB_VARIANT fantom;
  if(res->GetnRecord()>0){
    for (i=0; i < res->Columns(); i++){
      if(res->ColumnType(i+1)==IBPP::sdBlob){
				fantom.type = GB_T_VARIANT;
        fantom.value.type = GB_T_NULL;
	//res->GetData(pos,i).value._string.type = GB_T_NULL;
	//fantom.value._string.value=res->GetData(pos,i).value._string.value;
        GB.StoreVariant(&fantom, &buffer[i]);
      }
      else{
        GB.StoreVariant(&res->GetData(pos,i), &buffer[i]);
      }
    }
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
    //std::cout<<"read blob"<<std::endl;
  FBResult *res = (FBResult *)result;
  char *data;
  int len;

	data = res->GetData(pos,field).value.value._string;	//PQgetvalue(res, pos, field);
	len = strlen(data);

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
  FBResult *res=(FBResult *)result;
  // Attention position des colonnes a partir de 1 et pas 0
  return (char *) res->ColumnName(field+1);
}


/*****************************************************************************

  field_index()

  Return the index of a field in a result from its name.

  <result> is the result handle.
  <name> is the field name.
  <handle> is needed by this driver to enable table.field syntax

*****************************************************************************/

static int field_index(DB_RESULT Result, const char *name, DB_DATABASE *db)
{
  FBResult *res=(FBResult *)Result;
  // Attention position des Champs dans IBPP a partir de 1 et pas 0
  return res->ColumnNum(name)-1;
}


/*****************************************************************************

  field_type()

  Return the Gambas type of a field in a result from its index.

  <result> is the result handle.
  <field> is the field index.

*****************************************************************************/

static GB_TYPE field_type(DB_RESULT result, int field){
  FBResult *res=(FBResult *)result;
  // Attention position des Champs dans IBPP a partir de 1 et pas 0
  return conv_type(res->ColumnType(field+1));
}


/*****************************************************************************

  field_length()

  Return the length of a field in a result from its index.

  <result> is the result handle.
  <field> is the field index.

*****************************************************************************/

static int field_length(DB_RESULT result, int field){
  FBResult *res=(FBResult *)result;
  // Attention position des Champs dans IBPP a partir de 1 et pas 0
  return res->ColumnSize(field+1);
}


/*****************************************************************************

  begin_transaction()

  Begin a transaction.

  <handle> is the database handle.

  This function returns TRUE if the command has failed, and FALSE if
  everything was OK.

*****************************************************************************/

static int begin_transaction(DB_DATABASE *db){
  FBConnect *con = (FBConnect *)db->handle;
  if(!con->Started())
    con->Start();
  else
    return TRUE;
  return FALSE;
}


/*****************************************************************************

  commi_transaction()

  Commit a transaction.

  <handle> is the database handle.

  This function returns TRUE if the command has failed, and FALSE if
  everything was OK.

*****************************************************************************/

static int commit_transaction(DB_DATABASE *db){
  FBConnect *con = (FBConnect *)db->handle;
  if(con->Started())
    con->Commit();
  else
    return TRUE;
  return FALSE;
}


/*****************************************************************************

  rollback_transaction()

  Rolllback a transaction.

  <handle> is the database handle.

  This function returns TRUE if the command has failed, and FALSE if
  everything was OK.

*****************************************************************************/

static int rollback_transaction(DB_DATABASE *db){
  FBConnect *con = (FBConnect *)db->handle;
  if(con->Started())
    con->Rollback();
  else
    return TRUE;
  return FALSE;
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

static int table_init(DB_DATABASE *db, const char *table, DB_INFO *info){
  char qfield[SQLMAXLEN];
  FBResult *res;
  int i, n;
  DB_FIELD *f;
  snprintf(qfield,SQLMAXLEN-1,"select b.RDB$field_name,a.RDB$field_type,a.RDB$field_length from RDB$fields a,RDB$relation_fields b where a.RDB$field_name=b.RDB$field_source and b.RDB$relation_name=upper('%s') order by rdb$field_position",table);
  GB.NewString(&info->table, table, 0);
  if(do_query(db,qfield,&res,"Unable to get the table")){
    delete res;
    return TRUE;
  }
  info->nfield = n = res->GetnRecord();
  
  if (n == 0)
  {
    delete res;
    return TRUE;
  }
  
  GB.Alloc(POINTER(&info->field), sizeof(DB_FIELD) * n);
  for (i = 0; i < n; i++){
    f = &info->field[i];
    GB.NewString(&f->name, res->GetData(i,0).value.value._string, 0);
    f->type = conv_type(res->GetData(i,1).value.value._integer);
    f->length = 0;
    if (f->type == GB_T_STRING){
      f->length = res->GetData(i,2).value.value._integer;
      if (f->length < 0)
        f->length = 0;
      else
    f->length -= 4;
    }
  }
  delete res;
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

static int table_index(DB_DATABASE *db, const char *table, DB_INFO *info){
  char qindex[SQLMAXLEN];
  FBResult *res;
  int i, j, n;
  snprintf(qindex,SQLMAXLEN-1,"select * from rdb$index_segments where rdb$index_name in (SELECT rdb$index_name FROM RDB$RELATION_CONSTRAINTS WHERE rdb$relation_name=upper('%s') and RDB$CONSTRAINT_TYPE='PRIMARY KEY') order by rdb$field_position",table);
  if (do_query(db,qindex,&res,"Unable to get primary index: &1")){
    delete res;
    return TRUE;
  }
  n = info->nindex = res->GetnRecord();
  if (n <= 0){
    GB.Error("Table '&1' has no primary index", table);
    delete res;
    return TRUE;
  }
  GB.Alloc(POINTER(&info->index), sizeof(int) * n);
  for (i = 0; i < n; i++){
    for (j = 0; j < info->nfield; j++){
      if(strcmp(info->field[j].name, res->GetData(j,1).value.value._string) == 0){
        info->index[i] = j;
        break;
      }
    }
  }
  delete res;
  return FALSE;
}


/*****************************************************************************

  table_release()

  Free the info structure filled by table_init() and/or table_index()

  <handle> is the database handle.
  <info> points at the info structure.

*****************************************************************************/

static void table_release(DB_DATABASE *db, DB_INFO *info){
  /* All is done outside the driver */
}


/*****************************************************************************

  table_exist()

  Returns if a table exists

  <handle> is the database handle.
  <table> is the table name.

  This function returns TRUE if the table exists, and FALSE if not.

*****************************************************************************/

static int table_exist(DB_DATABASE *db, const char *table){
  static char query[SQLMAXLEN];
  int retour=FALSE;
  FBResult *res;
  snprintf(query,SQLMAXLEN-1,"select rdb$relation_name from rdb$relations where rdb$relation_name=upper('%s')",table);
  if (do_query(db,query,&res,"Unable to get the table")){
    delete res;
    return retour;
  }
  if (res->GetnRecord()>0)
    retour=TRUE;
  delete res;
  return retour;
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

static int table_list(DB_DATABASE *db, char ***tables){
  static char query[SQLMAXLEN];
  FBResult *res;
  unsigned i;
  int count;
  snprintf(query,SQLMAXLEN-1,"select rdb$relation_name from rdb$relations");
  if (do_query(db, query, &res, "Unable to get the table")){
    delete res;
    return -1;
  }
  if (tables){
    GB.NewArray(tables, sizeof(char *), res->GetnRecord());
    for (i = 0; i < res->GetnRecord(); i++)
      GB.NewString(&((*tables)[i]), res->GetData(i,0).value.value._string, 0);
  }
  count = res->GetnRecord();
  delete res;
  return count;
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

static int table_primary_key(DB_DATABASE *db, const char *table, char ***primary){
  static char query[SQLMAXLEN];
  FBResult *res;
  unsigned i;
  snprintf(query,SQLMAXLEN-1,"select * from rdb$index_segments where rdb$index_name in (SELECT rdb$index_name FROM RDB$RELATION_CONSTRAINTS WHERE rdb$relation_name=upper('%s') and RDB$CONSTRAINT_TYPE='PRIMARY KEY') order by rdb$field_position",table);
  if (do_query(db, query, &res, "Unable to get the table")){
    delete res;
    return TRUE;
  }
  GB.NewArray(primary, sizeof(char *), res->GetnRecord());
  for (i = 0; i < res->GetnRecord(); i++)
    GB.NewString(&((*primary)[i]), res->GetData(i,1).value.value._string, 0);
  delete res;
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

static int table_is_system(DB_DATABASE *db, const char *table){
  static char query[SQLMAXLEN];
  int retour=FALSE;
  FBResult *res;
  snprintf(query,SQLMAXLEN-1,"select rdb$system_flag from rdb$relations where rdb$relation_name=upper('%s')",table);
  if (do_query(db, query, &res, "Unable to get the table")){
    delete res;
    return FALSE;
  }
  if(res->GetData(0,0).value.value._integer == 1)
    retour=TRUE;
  delete res;
  return retour;
}


/*****************************************************************************

  table_delete()

  Deletes a table.

  <handle> is the database handle.
  <table> is the table name.

  This function returns TRUE if the command has failed, and FALSE if
  everything was OK.

*****************************************************************************/

static int table_delete(DB_DATABASE *db, const char *table){
  static char query[SQLMAXLEN];
  snprintf(query,SQLMAXLEN-1,"drop table %s",table);
  if (exec_query((DB_DATABASE *)db,(char *)query,NULL,"Unable to delete the table"))
    return TRUE;
  return FALSE;
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

static int table_create(DB_DATABASE *db, const char *table, DB_FIELD *fields, char **primary, const char *not_used){
  DB_FIELD *fp;
  static char query[SQLMAXLEN];
  int comma,retour=FALSE;
  const char *type;
  int i;
  DB.Query.Init();
  DB.Query.Add("CREATE TABLE ");
  DB.Query.Add(QUOTE_STRING);
  DB.Query.Add(table);
  DB.Query.Add(QUOTE_STRING);
  DB.Query.Add(" (");
  comma = FALSE;
  for (fp = fields; fp; fp = fp->next){
    if (comma)
      DB.Query.Add(", ");
    else
      comma = TRUE;
	  
	  DB.Query.Add(QUOTE_STRING);
    DB.Query.Add(fp->name);
  	DB.Query.Add(QUOTE_STRING);
    
    switch (fp->type){
      case GB_T_BOOLEAN:
      case GB_T_INTEGER: type = "INTEGER";
        break;
      case DB_T_SERIAL:
      case GB_T_LONG: type = "BIGINT";
        break;

      case DB_T_BLOB: type = "BLOB";
        break;

      case GB_T_FLOAT: type = "FLOAT";
        break;

      case GB_T_DATE: type = "TIMESTAMP";
        break;

      case GB_T_STRING:
        if (fp->length <= 0){
          type = "VARCHAR(DEFAULT_VARCHAR_LEN)";
        }
        else{
          sprintf(_buffer, "VARCHAR(%d)", fp->length);
          type = _buffer;
        }
        break;

        default: type = "VARCHAR(DEFAULT_VARCHAR_LEN)";
        break;
    }
    DB.Query.Add(" ");
    DB.Query.Add(type);
    if (fp->def.type != GB_T_NULL){
      DB.Query.Add(" DEFAULT ");
      DB.FormatVariant(&_driver, &fp->def, DB.Query.AddLength);
      DB.Query.Add(" NOT NULL ");
    }
    else if (DB.StringArray.Find(primary, fp->name) >= 0){
      DB.Query.Add(" NOT NULL ");
    }
  }
  if (primary){
    DB.Query.Add(", PRIMARY KEY (");
    for (i = 0; i < GB.Count(primary); i++){
      if (i > 0)
        DB.Query.Add(",");
      DB.Query.Add(primary[i]);
    }
    DB.Query.Add(")");
  }
  DB.Query.Add(" )");
  retour=exec_query((DB_DATABASE *)db,DB.Query.Get(),NULL,"Unable to create the table");
  for (fp = fields; fp; fp = fp->next){
    if(fp->type==DB_T_SERIAL){
	snprintf(query,SQLMAXLEN-1,"create generator gen_id_%s_%s",table,fp->name);
	retour=exec_query((DB_DATABASE *)db,(char *)query,NULL,"Unable to make generator");
	snprintf(query,SQLMAXLEN-1,"create trigger on_insert_%s_%s for %s active before insert position 0 as begin if(new.%s is NULL) then new.%s=gen_id(gen_id_%s_%s,1); end",table,fp->name,table,fp->name,fp->name,table,fp->name);
	retour=exec_query((DB_DATABASE *)db,(char *)query,NULL,"Unable to make trigger befor insert");
    }
  }
  return(retour);
}


/*****************************************************************************

  field_exist()

  Returns if a field exists in a given table

  <handle> is the database handle.
  <table> is the table name.
  <field> is the field name.

  This function returns TRUE if the field exists, and FALSE if not.

*****************************************************************************/

static int field_exist(DB_DATABASE *db, const char *table, const char *field){
  static char query[SQLMAXLEN];
  int retour=FALSE;
  FBResult *res;
  snprintf(query,SQLMAXLEN-1,"select rdb$field_name from rdb$relation_fields where rdb$relation_name=upper('%s') and rdb$field_name=upper('%s')",table,field);
  if (do_query(db, query, &res, "Unable to get the field")){
    delete res;
    return FALSE;
  }
  if(res->GetnRecord() > 0)
    retour=TRUE;
  delete res;
  return retour;
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

static int field_list(DB_DATABASE *db, const char *table, char ***fields){
  static char query[SQLMAXLEN];
  FBResult *res;
  unsigned i;
  int count;
  snprintf(query,SQLMAXLEN-1,"select rdb$field_name from rdb$relation_fields where rdb$relation_name=upper('%s')",table);
  if (do_query(db, query, &res, "Unable to get the field from the table")){
    delete res;
    return -1;
  }
  if (fields){
    GB.NewArray(fields, sizeof(char *), res->GetnRecord());
    for (i = 0; i < res->GetnRecord(); i++)
      GB.NewString(&((*fields)[i]), res->GetData(i,0).value.value._string, 0);
  }
  count = res->GetnRecord();
  delete res;
  return count;}


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

static int field_info(DB_DATABASE *db, const char *table, const char *field, DB_FIELD *info){
  FBResult *res;
  static char query[SQLMAXLEN];
  int type;
  std::string str1,str2;
  snprintf(query,SQLMAXLEN-1,"select b.RDB$field_name,a.RDB$field_type,b.rdb$null_flag,b.rdb$default_source,a.RDB$field_length from RDB$fields a,RDB$relation_fields b where a.RDB$field_name=b.RDB$field_source and b.RDB$relation_name=upper('%s') and b.rdb$field_name=upper('%s')",table,field);
  if (do_query(db, query, &res, "Unable to get the field from the table")){
    delete res;
    return TRUE;
  }
  if (res->GetnRecord()!=1){
    delete res;
    GB.Error("Unable to find field &1.&2", table, field);
    return TRUE;
  }
  if (strcmp(res->GetData(0,0).value.value._string, field)){
    delete res;
    GB.Error("Unable to find field &1.&2", table, field);
    return TRUE;
  }
  info->name = NULL;
  type = res->GetData(0,1).value.value._integer;
  info->type = conv_type(type);
  if (info->type == GB_T_STRING){
  info->length = res->GetData(0,4).value.value._integer;
  }
  info->def.type = GB_T_NULL;
  if(res->GetData(0,3).value.value._string){
    str1=res->GetData(0,3).value.value._string;
  if(str1!="")
    str2=str1.assign(str1,8,str1.length()-8);
  GB.FreeString(&res->GetData(0,3).value.value._string);
  res->SetData(0,3,str2);
  GB.StoreVariant(&res->GetData(0,3), &info->def);
  }
  delete res;
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

static int index_exist(DB_DATABASE *db, const char *table, const char *index){
  FBResult *res;
  static char query[SQLMAXLEN];
  int exist=FALSE;
  snprintf(query,SQLMAXLEN-1,"select rdb$index_name from rdb$indices where rdb$indices.rdb$relation_name=upper('%s') and rdb$indices.rdb$index_name=upper('%s')",table,index);
  if (do_query(db, query, &res, "Unable to get the field from the table")){
    delete res;
    return FALSE;
  }
  if(res->GetnRecord() > 0)
    exist=TRUE;
  delete res;
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

static int index_list(DB_DATABASE *db, const char *table, char ***indexes){
  FBResult *res;
  static char query[SQLMAXLEN];
  snprintf(query,SQLMAXLEN-1,"select rdb$index_name from rdb$indices where rdb$indices.rdb$relation_name=upper('%s')",table);
  unsigned i;
  int count=0;
  if (do_query(db, query, &res, "Unable to get the field from the table")){
    delete res;
    return TRUE;
  }
  if (indexes){
    GB.NewArray(indexes, sizeof(char *), res->GetnRecord());
    for (i = 0; i < res->GetnRecord(); i++)
      GB.NewString(&((*indexes)[i]), res->GetData(i,0).value.value._string, 0);
    count = res->GetnRecord();
  }
  delete res;
  return count;
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

static int index_info(DB_DATABASE *db, const char *table, const char *index, DB_INDEX *info){
  FBResult *res;
  static char query[SQLMAXLEN],query_field[SQLMAXLEN];
  unsigned i;
  snprintf(query,SQLMAXLEN-1,"select rdb$index_name,rdb$relation_name,rdb$unique_flag from rdb$indices where rdb$indices.rdb$relation_name=upper('%s') and rdb$indices.rdb$index_name=upper('%s')",table,index);
  snprintf(query_field,SQLMAXLEN-1,"select RDB$FIELD_NAME from rdb$index_segments where rdb$index_name=upper('%s')  order by rdb$field_position",index);
  if (do_query(db, query, &res, "Unable to get the field from the table")){
    delete res;
    return TRUE;
  }
  if (res->GetnRecord() != 1){
      GB.Error("Unable to find index &1.&2", table, index);
      delete res;
      return TRUE;
    }
  info->name = NULL;
  info->unique = res->GetData(0,2).value.value._integer;
  if(strncmp(res->GetData(0,0).value.value._string,"RDB$PRIMARY",11))
    info->primary = FALSE;
  else
    info->primary = TRUE;
  delete res;
  if (do_query(db, query_field, &res, "Unable to get the field from the table")){
    delete res;
    return TRUE;
  }
  DB.Query.Init();
  for (i = 0; i < res->GetnRecord(); i++){
    if (i > 0)
      DB.Query.Add(",");
    DB.Query.Add(res->GetData(i,0).value.value._string);
  }
  delete res;
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

static int index_delete(DB_DATABASE *db, const char *table, const char *index){
  static char query[SQLMAXLEN];
  snprintf(query,SQLMAXLEN-1,"drop index %s",index);
  return exec_query((DB_DATABASE *)db,(char *)query,NULL,"Unable to delete index");
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

static int index_create(DB_DATABASE *db, const char *table, const char *index, DB_INDEX *info){
  DB.Query.Init();
  DB.Query.Add("CREATE ");
  if (info->unique)
    DB.Query.Add("UNIQUE ");
  DB.Query.Add("INDEX ");
  DB.Query.Add(QUOTE_STRING);
  DB.Query.Add(index);
  DB.Query.Add(QUOTE_STRING);
  DB.Query.Add(" ON ");
  DB.Query.Add(table);
  DB.Query.Add(" ( ");
  DB.Query.Add(info->fields);
  DB.Query.Add(" )");
  return exec_query((DB_DATABASE *)db,(char *)DB.Query.Get(), NULL,"Unable to create index");
}


/*****************************************************************************

  database_exist()

  Returns if a database exists

  <handle> is any database handle.
  <name> is the database name.

  This function returns TRUE if the database exists, and FALSE if not.

*****************************************************************************/

static int database_exist(DB_DATABASE *db, const char *name){
  FBConnect *con = (FBConnect *)db->handle;
  std::string nametmp=name;
  //std::transform(nametmp.begin(), nametmp.end(), nametmp.begin(), toupper);
  if(nametmp==con->DataBase())
    return TRUE;
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

static int database_list(DB_DATABASE *db, char ***databases){
  return -1;
}


/*****************************************************************************

  database_is_system()

  Returns if a database is a system database.

  <handle> is any database handle.
  <name> is the database name.

  This function returns TRUE if the database is a system database, and
  FALSE if not.

*****************************************************************************/

static int database_is_system(DB_DATABASE *db, const char *name){
  //GB.Error("database is system not implemented in FBDatabase Driver");
  return FALSE;
}

/*****************************************************************************

  table_type()
  Not Valid in firebird

  <handle> is the database handle.
  <table> is the table name.
*****************************************************************************/
static char *table_type(DB_DATABASE *db, const char *table, const char *type){
  /*if (type)
  GB.Error("table type ist not implemented in FBDatabase Driver");*/
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

static int database_delete(DB_DATABASE *db, const char *name){
  FBConnect *con = (FBConnect *)db->handle;
  std::string nametmp=name;
  //std::transform(nametmp.begin(), nametmp.end(), nametmp.begin(), toupper);
  if(con->DataBase()==nametmp)
    if(con->DropDataBase())
      return FALSE;
  GB.Error("DataBase not Deleted !!! Database &1 not exist or permission not ok to Drop Database",name);
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

static int database_create(DB_DATABASE *db, const char *name){
  /*FBConnect *con = (FBConnect *)handle;
  std::string nametmp=name;
  if(con->DataBase()!=nametmp)
    con->Disconnect();
  con->DataBase(nametmp);
  if(con->Connect())
    return FALSE;
  GB.Error("Error Database not Created or permission not ok");*/
  return TRUE;
}


/*****************************************************************************

  user_exist()

  Returns if a user exists.

  <handle> is any database handle.
  <name> is the user name.

  This function returns TRUE if the user exists, and FALSE if not.

*****************************************************************************/

static int user_exist(DB_DATABASE *db, const char *name){
  FBConnect *con = (FBConnect *)db->handle;
  UsersL ul=con->UserList();
  std::string nametmp=name;
  std::transform(nametmp.begin(), nametmp.end(), nametmp.begin(), toupper);
  unsigned i;
  for(i=0;i<ul.size();i++)
    if(ul[i].username==nametmp)
      return TRUE;
  return FALSE;
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

static int user_list(DB_DATABASE *db, char ***users){
  FBConnect *con = (FBConnect *)db->handle;
  UsersL ul=con->UserList();
  unsigned i;
  int count=0;
  if (users){
    GB.NewArray(users, sizeof(char *), ul.size());
    for (i = 0; i < ul.size(); i++)
      GB.NewString(&((*users)[i]), ul[i].username.c_str(), 0);
    count = ul.size();
  }
  return ul.size();
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

static int user_info(DB_DATABASE *db, const char *name, DB_USER *info){
  /*FBConnect *con = (FBConnect *)handle;
  std::string nametmp=name;
  std::transform(nametmp.begin(), nametmp.end(), nametmp.begin(), toupper);
  if(con->UserInfo(nametmp,info))
  return FALSE;*/
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

static int user_delete(DB_DATABASE *db, const char *name){
  FBConnect *con = (FBConnect *)db->handle;
  std::string nametmp=name;
  std::transform(nametmp.begin(), nametmp.end(), nametmp.begin(), toupper);
  if(!con->RemoveUser(nametmp)){
    GB.Error("User &1 not existe or not permission to delete this user",name);
    return TRUE;
  };
  return FALSE;
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

static int user_create(DB_DATABASE *db, const char *name, DB_USER *info){
  FBConnect *con = (FBConnect *)db->handle;
  std::string nametmp=name;
  std::transform(nametmp.begin(), nametmp.end(), nametmp.begin(), toupper);
  if(!con->AddUser(nametmp,info->password)){
    GB.Error("user &1 not Created not permission to create this user !!!");
    return TRUE;
  };
  return FALSE;
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

static int user_set_password(DB_DATABASE *db, const char *name, const char *password){
  FBConnect *con = (FBConnect *)db->handle;
  std::string nametmp=name,passwordtmp=password;
  std::transform(nametmp.begin(), nametmp.end(), nametmp.begin(), toupper);
  if(!con->ModifyUser(nametmp,passwordtmp)){
    GB.Error("User &1 not modified or not permission to modify this user !!!");
    return TRUE;
  };
  return FALSE;
}

/*****************************************************************************

  The component entry and exit functions.

*****************************************************************************/

extern "C" {
  int EXPORT GB_INIT(void)
  {
    GB.GetInterface("gb.db", DB_INTERFACE_VERSION, &DB);
    DB.Register(&_driver);

    return FALSE;
  }

  void EXPORT GB_EXIT()
  {
  }
}
