/***************************************************************************

  CConnection.c

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

#define __CCONNECTION_C

#include "main.h"

#include "CTable.h"
//#include "CView.h"
#include "CDatabase.h"
#include "CUser.h"
#include "CConnection.h"


/***************************************************************************

  Connection

***************************************************************************/

static CCONNECTION *_current = NULL;

static GB_SUBCOLLECTION_DESC _databases_desc =
{
  ".ConnectionDatabases",
  (void *)CDATABASE_get,
  (void *)CDATABASE_exist,
  (void *)CDATABASE_list,
  (void *)CDATABASE_release
};

static GB_SUBCOLLECTION_DESC _users_desc =
{
  ".ConnectionUsers",
  (void *)CUSER_get,
  (void *)CUSER_exist,
  (void *)CUSER_list,
  (void *)CUSER_release
};

static GB_SUBCOLLECTION_DESC _tables_desc =
{
  ".ConnectionTables",
  (void *)CTABLE_get,
  (void *)CTABLE_exist,
  (void *)CTABLE_list,
  (void *)CTABLE_release
};

/*static GB_SUBCOLLECTION_DESC _views_desc =
{
  ".ConnectionViews",
  (void *)CVIEW_get,
  (void *)CVIEW_exist,
  (void *)CVIEW_list
};*/



static void open_connection(CCONNECTION *_object)
{
  DB_Open(&THIS->desc, &THIS->driver, &THIS->db);
  THIS->limit = 0;
  THIS->trans = 0;
}

static bool check_opened(CCONNECTION *_object)
{
	if (!THIS->db.handle)
		open_connection(THIS);

  if (!THIS->db.handle)
  {
    GB.Error("Connection is not opened");
    return TRUE;
  }
  else
    return FALSE;
}

#define CHECK_OPEN() \
  if (check_opened(THIS)) \
    return;

static int get_current(CCONNECTION **current)
{
  if (*current == NULL)
  {
    if (_current == NULL)
    {
      GB.Error("No current connection");
      return TRUE;
    }
    *current = _current;
  }

  return FALSE;
}

#define CHECK_DB() \
  if (get_current((CCONNECTION **)(void *)&_object)) \
    return;

static void close_connection(CCONNECTION *_object)
{
  if (!THIS->db.handle)
    return;

  GB.Unref(POINTER(&THIS->databases));
  THIS->databases = NULL;
  GB.Unref(POINTER(&THIS->users));
  THIS->users = NULL;
  GB.Unref(POINTER(&THIS->tables));
  THIS->tables = NULL;

  THIS->driver->Close(&THIS->db);
  GB.FreeString(&THIS->db.charset);

  THIS->db.handle = NULL;
  THIS->driver = NULL;
}


// BEGIN_METHOD(CCONNECTION_new, GB_STRING url)
BEGIN_METHOD_VOID(CCONNECTION_new)

  /*int i, state, p;
  char c;*/

  THIS->db.handle = NULL;
  THIS->db.flags.ignore_case = FALSE; // Now case is sensitive by default!

  if (_current == NULL)
    _current = THIS;

#if 0
  if (!MISSING(url))
  {
    char *url = GB.ToZeroString(ARG(url));

    state = 0;
    p = 0;
    for (i = 0; i <= LENGTH(url); i++)
    {
      c = url[i];

      switch (state)
      {
        case 0: /* type */
          if (c == ':' || c == 0)
          {
            GB.NewString(&THIS->desc.type, &url[p], i - p);
            state++;
            p = i + 3; /* On saute '://' */
          }
          break;

        case 1: /* // */
        case 2:
          if (c && c != '/')
            goto BAD_URL;

          state++;
          break;

        case 3: /* host */
          if (c == ':' || c == '/' || c == 0)
          {
            if (i > p)
              GB.NewString(&THIS->desc.host, &url[p], i - p);
            p = i + 1;
            state++;
            if (c != ':')
              state++;
          }
          break;

        case 4: /* port */
          if (c == '/' || c == 0)
          {
            if (i == p)
              goto BAD_URL;

            GB.NewString(&THIS->desc.port, &url[p], i - p);
            p = i + 1;
            state++;
          }
          break;

        case 5: /* name */
          if (c == 0 && i > p)
            GB.NewString(&THIS->desc.name, &url[p], i - p);
          break;
      }
    }
  }

  return;

BAD_URL:

  GB.Error("Malformed URL");
  return;
#endif

END_METHOD


BEGIN_METHOD_VOID(CCONNECTION_free)

  close_connection(THIS);

  if (_current == THIS)
    _current = NULL;

  GB.StoreString(NULL, &THIS->desc.type);
  GB.StoreString(NULL, &THIS->desc.host);
  GB.StoreString(NULL, &THIS->desc.user);
  GB.StoreString(NULL, &THIS->desc.password);
  GB.StoreString(NULL, &THIS->desc.name);
  GB.StoreString(NULL, &THIS->desc.port);
  GB.StoreString(NULL, &THIS->db.charset);

END_METHOD


#define IMPLEMENT(_prop) \
BEGIN_PROPERTY(CCONNECTION_##_prop) \
\
  if (READ_PROPERTY) \
    GB.ReturnString(THIS->desc._prop); \
  else \
    GB.StoreString(PROP(GB_STRING), &THIS->desc._prop); \
\
END_PROPERTY

IMPLEMENT(type)
IMPLEMENT(host)
IMPLEMENT(user)
IMPLEMENT(password)
IMPLEMENT(name)
IMPLEMENT(port)


BEGIN_PROPERTY(CCONNECTION_version)

  CHECK_DB();
  CHECK_OPEN();

  GB.ReturnInteger(THIS->db.version);

END_PROPERTY


BEGIN_PROPERTY(CCONNECTION_opened)

  CHECK_DB();

	GB.ReturnBoolean(THIS->db.handle != NULL);

END_PROPERTY


BEGIN_PROPERTY(CCONNECTION_error)

  CHECK_DB();

	GB.ReturnInteger(THIS->db.error);

END_PROPERTY


BEGIN_METHOD_VOID(CCONNECTION_open)

  CHECK_DB();

  if (THIS->db.handle)
  {
    GB.Error("Connection already opened");
    return;
  }

  open_connection(THIS);

END_METHOD


BEGIN_METHOD_VOID(CCONNECTION_close)

  CHECK_DB();

  close_connection(THIS);

END_METHOD

#if 0
BEGIN_PROPERTY(CCONNECTION_ignore_case)

	CHECK_DB();
	CHECK_OPEN();
	
	if (READ_PROPERTY)
		GB.ReturnBoolean(THIS->db.ignore_case);
	else
	{
		if (THIS->db.flags.no_case)
		{
			if (THIS->db.flags.ignore_case)
				GB.Error("This database driver cannot be case sensitive");
			else
				GB.Error("This database driver is always case sensitive");
			return;
		}
		THIS->db.flags.ignore_case = VPROP(GB_BOOLEAN);
	}

END_PROPERTY
#endif

BEGIN_METHOD_VOID(CCONNECTION_begin)

  CHECK_DB();
  CHECK_OPEN();

	if (!THIS->db.flags.no_nest || THIS->trans == 0)
  	THIS->driver->Begin(&THIS->db);
  THIS->trans++;

END_METHOD


BEGIN_METHOD_VOID(CCONNECTION_commit)

  CHECK_DB();
  CHECK_OPEN();

	if (THIS->trans == 0)
		return;
  
  THIS->trans--;
	if (!THIS->db.flags.no_nest || THIS->trans == 0)
	  THIS->driver->Commit(&THIS->db);

END_METHOD


BEGIN_METHOD_VOID(CCONNECTION_rollback)

  CHECK_DB();
  CHECK_OPEN();

	if (THIS->trans == 0)
		return;
  
  THIS->trans--;
	if (!THIS->db.flags.no_nest || THIS->trans == 0)
	  THIS->driver->Rollback(&THIS->db);

END_METHOD


BEGIN_METHOD(CCONNECTION_limit, GB_INTEGER limit)

  CHECK_DB();
  CHECK_OPEN();

	THIS->limit = VARG(limit);
	GB.ReturnObject(THIS);

END_PROPERTY

static char *_make_query_buffer;
static char *_make_query_original;

static void make_query_get_param(int index, char **str, int *len)
{
	if (index == 1)
		*str = _make_query_buffer;
	else if (index == 2)
		*str = _make_query_original;

	*len = -1;
}

static char *make_query(CCONNECTION *_object, char *pattern, int len, int narg, GB_VALUE *arg)
{
	char *query;
	const char *keyword;
	char buffer[32];

	query = DB_MakeQuery(THIS->driver, pattern, len, narg, arg);

	if (query && THIS->limit > 0 && strncasecmp(query, "SELECT ", 7) == 0)
	{
		keyword = THIS->db.limit.keyword;
		if (!keyword)
			keyword = "LIMIT";

		snprintf(buffer, sizeof(buffer), "%s %d", keyword, THIS->limit);

		_make_query_buffer = buffer;
		_make_query_original = &query[7];

		switch (THIS->db.limit.pos)
		{
			case DB_LIMIT_AT_BEGIN:
				query = GB.SubstString("SELECT &1 &2", 0, make_query_get_param);
				break;

			case DB_LIMIT_AT_END:
			default:
				query = GB.SubstString("SELECT &2 &1", 0, make_query_get_param);
				break;
		}

		THIS->limit = 0;
	}

	return query;
}

BEGIN_METHOD(CCONNECTION_exec, GB_STRING query; GB_VALUE param[0])

  char *query;
  CRESULT *result;

  CHECK_DB();

  CHECK_OPEN();

  query = make_query(THIS, STRING(query), LENGTH(query), GB.NParam(), ARG(param[0]));
  if (!query)
    return;

  result = DB_MakeResult(THIS, RESULT_FIND, NULL, query);

  if (result)
    GB.ReturnObject(result);

END_METHOD


BEGIN_METHOD(CCONNECTION_create, GB_STRING table)

  CRESULT *result;
  char *table = GB.ToZeroString(ARG(table));

  CHECK_DB();

  CHECK_OPEN();

	if (!table || !*table)
	{
		GB.Error("Void table name");
		return;
	}

  result = DB_MakeResult(THIS, RESULT_CREATE, table, NULL);

  if (result)
    GB.ReturnObject(result);
	else
		GB.ReturnNull();

END_METHOD


static char *get_query(char *prefix, CCONNECTION *_object, char *table, int len_table, char *query, int len_query, GB_VALUE *arg)
{
	if (!len_table)
	{
		GB.Error("Void table name");
		return NULL;
	}

  q_init();

  q_add(prefix);
  q_add(" ");
  q_add(THIS->driver->GetQuote());
  q_add_length(table, len_table);
  q_add(THIS->driver->GetQuote());

  if (query && len_query > 0)
  {
  	q_add(" ");
  	if (strncmp(query, "WHERE ", 6))
    	q_add("WHERE ");
    q_add_length(query, len_query);
	}

  query = make_query(THIS, q_get(), q_length(), GB.NParam(), arg);

  return query;
}


BEGIN_METHOD(CCONNECTION_find, GB_STRING table; GB_STRING query; GB_VALUE param[0])

  char *query;
  CRESULT *result;

  CHECK_DB();
  CHECK_OPEN();

  query = get_query("SELECT * FROM", THIS, STRING(table), LENGTH(table),
    MISSING(query) ? NULL : STRING(query),
    MISSING(query) ? 0 : LENGTH(query),
    ARG(param[0]));

  if (!query)
    return;

  result = DB_MakeResult(THIS, RESULT_FIND, NULL, query);

  if (result)
    GB.ReturnObject(result);

END_METHOD


BEGIN_METHOD(CCONNECTION_delete, GB_STRING table; GB_STRING query; GB_VALUE param[0])

  char *query;

  CHECK_DB();
  CHECK_OPEN();

  query = get_query("DELETE FROM", THIS, STRING(table), LENGTH(table),
    MISSING(query) ? NULL : STRING(query),
    MISSING(query) ? 0 : LENGTH(query),
    ARG(param[0]));

  if (!query)
    return;

  DB_MakeResult(THIS, RESULT_DELETE, NULL, query);

END_METHOD


BEGIN_METHOD(CCONNECTION_edit, GB_STRING table; GB_STRING query; GB_VALUE param[0])

  char *query;
  CRESULT *result;
  /*char *table = GB.ToZeroString(ARG(table));*/

  CHECK_DB();

  CHECK_OPEN();

  query = get_query("SELECT * FROM", THIS, STRING(table), LENGTH(table),
    MISSING(query) ? NULL : STRING(query),
    MISSING(query) ? 0 : LENGTH(query),
    ARG(param[0]));

  if (!query)
    return;

  result = DB_MakeResult(THIS, RESULT_EDIT, GB.ToZeroString(ARG(table)), query);

  if (result)
    GB.ReturnObject(result);

END_METHOD


BEGIN_METHOD(CCONNECTION_quote, GB_STRING name)

  CHECK_DB();
  CHECK_OPEN();

  q_init();
  q_add(THIS->driver->GetQuote());
  q_add_length(STRING(name), LENGTH(name));
  q_add(THIS->driver->GetQuote());

  /* q_get() returns a gambas string */
  GB.ReturnString(q_get());

END_METHOD


BEGIN_METHOD(CCONNECTION_subst, GB_STRING query; GB_VALUE param[0])

  char *query;

  CHECK_DB();

  CHECK_OPEN();

  query = make_query(THIS, STRING(query), LENGTH(query), GB.NParam(), ARG(param[0]));

  if (!query)
    return;

  GB.ReturnString(query);

END_METHOD


BEGIN_PROPERTY(CCONNECTION_current)

  if (READ_PROPERTY)
    GB.ReturnObject(_current);
  else
    _current = (CCONNECTION *)VPROP(GB_OBJECT);

END_PROPERTY


BEGIN_PROPERTY(CCONNECTION_charset)

  CHECK_DB();
  CHECK_OPEN();
  if (THIS->db.charset)
    GB.ReturnString(THIS->db.charset);
  else
    GB.ReturnConstZeroString("ASCII");

END_PROPERTY


BEGIN_PROPERTY(CCONNECTION_databases)

  CHECK_DB();
  CHECK_OPEN();

  GB.SubCollection.New(&THIS->databases, &_databases_desc, THIS);
  GB.ReturnObject(THIS->databases);

END_PROPERTY


BEGIN_PROPERTY(CCONNECTION_users)

  CHECK_DB();
  CHECK_OPEN();

  GB.SubCollection.New(&THIS->users, &_users_desc, THIS);
  GB.ReturnObject(THIS->users);

END_PROPERTY


BEGIN_PROPERTY(CCONNECTION_tables)

  CHECK_DB();
  CHECK_OPEN();

  GB.SubCollection.New(&THIS->tables, &_tables_desc, THIS);
  GB.ReturnObject(THIS->tables);

END_PROPERTY


/*BEGIN_PROPERTY(CCONNECTION_views)

  CHECK_DB();
  CHECK_OPEN();

  GB.SubCollection.New(&THIS->views, &_views_desc, THIS);
  GB.ReturnObject(THIS->views);

END_PROPERTY*/


BEGIN_PROPERTY(CCONNECTION_debug)

  if (READ_PROPERTY)
    GB.ReturnBoolean(DB_IsDebug());
  else
    DB_SetDebug(VPROP(GB_BOOLEAN));

END_PROPERTY


GB_DESC CConnectionDesc[] =
{
  GB_DECLARE("Connection", sizeof(CCONNECTION)),

  GB_METHOD("_new", NULL, CCONNECTION_new, NULL), //"[(URL)s]"),
  GB_METHOD("_free", NULL, CCONNECTION_free, NULL),

  GB_PROPERTY("Type", "s", CCONNECTION_type),
  GB_PROPERTY("Host", "s", CCONNECTION_host),
  GB_PROPERTY("Login", "s", CCONNECTION_user),
  GB_PROPERTY("User", "s", CCONNECTION_user),
  GB_PROPERTY("Password", "s", CCONNECTION_password),
  GB_PROPERTY("Name", "s", CCONNECTION_name),
  GB_PROPERTY("Port", "s", CCONNECTION_port),
  GB_PROPERTY_READ("Charset", "s", CCONNECTION_charset),
  GB_PROPERTY_READ("Version", "i", CCONNECTION_version),
  GB_PROPERTY_READ("Opened", "b", CCONNECTION_opened),
  GB_PROPERTY_READ("Error", "i", CCONNECTION_error),
  //GB_PROPERTY("IgnoreCase", "b", CCONNECTION_ignore_case),

  GB_METHOD("Open", NULL, CCONNECTION_open, NULL),
  GB_METHOD("Close", NULL, CCONNECTION_close, NULL),

  GB_METHOD("Limit", "Connection", CCONNECTION_limit, "(Limit)i"),
  GB_METHOD("Exec", "Result", CCONNECTION_exec, "(Request)s(Arguments)."),
  GB_METHOD("Create", "Result", CCONNECTION_create, "(Table)s"),
  GB_METHOD("Find", "Result", CCONNECTION_find, "(Table)s[(Request)s(Arguments).]"),
  GB_METHOD("Edit", "Result", CCONNECTION_edit, "(Table)s[(Request)s(Arguments).]"),
  GB_METHOD("Delete", NULL, CCONNECTION_delete, "(Table)s[(Request)s(Arguments).]"),
  GB_METHOD("Subst", "s", CCONNECTION_subst, "(Format)s(Arguments)."),

  GB_METHOD("Begin", NULL, CCONNECTION_begin, NULL),
  GB_METHOD("Commit", NULL, CCONNECTION_commit, NULL),
  GB_METHOD("Rollback", NULL, CCONNECTION_rollback, NULL),

  GB_METHOD("Quote", "s", CCONNECTION_quote, "(Name)s"),

  GB_PROPERTY("Tables", ".ConnectionTables", CCONNECTION_tables),
  GB_PROPERTY("Databases", ".ConnectionDatabases", CCONNECTION_databases),
  GB_PROPERTY("Users", ".ConnectionUsers", CCONNECTION_users),
  //GB_PROPERTY("Views", ".ConnectionViews", CCONNECTION_views),

  GB_CONSTANT("_Properties", "s", "Type,Host,Login,Password,Name,Port"),

  GB_END_DECLARE
};


GB_DESC CDBDesc[] =
{
  GB_DECLARE("DB", 0), GB_VIRTUAL_CLASS(),

	GB_CONSTANT("Boolean", "i", GB_T_BOOLEAN),
	GB_CONSTANT("Integer", "i", GB_T_INTEGER),
	GB_CONSTANT("Long", "i", GB_T_LONG),
	GB_CONSTANT("Float", "i", GB_T_FLOAT),
	GB_CONSTANT("Date", "i", GB_T_DATE),
	GB_CONSTANT("String", "i", GB_T_STRING),
	GB_CONSTANT("Serial", "i", DB_T_SERIAL),
	GB_CONSTANT("Blob", "i", DB_T_BLOB),

  GB_STATIC_PROPERTY("Current", "Connection", CCONNECTION_current),

  GB_STATIC_METHOD("Open", NULL, CCONNECTION_open, NULL),
  GB_STATIC_METHOD("Close", NULL, CCONNECTION_close, NULL),

  GB_STATIC_PROPERTY_READ("Charset", "s", CCONNECTION_charset),
  GB_STATIC_PROPERTY_READ("Version", "i", CCONNECTION_version),
  GB_STATIC_PROPERTY_READ("Opened", "b", CCONNECTION_opened),
  GB_STATIC_PROPERTY_READ("Error", "i", CCONNECTION_error),
  //GB_STATIC_PROPERTY("IgnoreCase", "b", CCONNECTION_ignore_case),

  GB_STATIC_PROPERTY("Debug", "b", CCONNECTION_debug),

  GB_STATIC_METHOD("Limit", "Connection", CCONNECTION_limit, "(Limit)i"),
  GB_STATIC_METHOD("Exec", "Result", CCONNECTION_exec, "(Request)s(Arguments)."),
  GB_STATIC_METHOD("Create", "Result", CCONNECTION_create, "(Table)s"),
  GB_STATIC_METHOD("Find", "Result", CCONNECTION_find, "(Table)s[(Request)s(Arguments).]"),
  GB_STATIC_METHOD("Edit", "Result", CCONNECTION_edit, "(Table)s[(Request)s(Arguments).]"),
  GB_STATIC_METHOD("Delete", NULL, CCONNECTION_delete, "(Table)s[(Request)s(Arguments).]"),
  GB_STATIC_METHOD("Subst", "s", CCONNECTION_subst, "(Format)s(Arguments)."),

  GB_STATIC_METHOD("Begin", NULL, CCONNECTION_begin, NULL),
  GB_STATIC_METHOD("Commit", NULL, CCONNECTION_commit, NULL),
  GB_STATIC_METHOD("Rollback", NULL, CCONNECTION_rollback, NULL),

  GB_STATIC_METHOD("Quote", "s", CCONNECTION_quote, "(Name)s"),

  GB_STATIC_PROPERTY("Tables", ".ConnectionTables", CCONNECTION_tables),
  //GB_STATIC_PROPERTY("Views", ".ConnectionViews", CCONNECTION_views),
  GB_STATIC_PROPERTY("Databases", ".ConnectionDatabases", CCONNECTION_databases),
  GB_STATIC_PROPERTY("Users", ".ConnectionUsers", CCONNECTION_users),

  GB_END_DECLARE
};


