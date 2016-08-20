/***************************************************************************

	CConnection.c

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

static SUBCOLLECTION_DESC _databases_desc =
{
	".Connection.Databases",
	(void *)CDATABASE_get,
	(void *)CDATABASE_exist,
	(void *)CDATABASE_list,
	(void *)CDATABASE_release
};

static SUBCOLLECTION_DESC _users_desc =
{
	".Connection.Users",
	(void *)CUSER_get,
	(void *)CUSER_exist,
	(void *)CUSER_list,
	(void *)CUSER_release
};

static SUBCOLLECTION_DESC _tables_desc =
{
	".Connection.Tables",
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
	if (DB_Open(&THIS->desc, &THIS->driver, &THIS->db))
		return;

	THIS->limit = 0;
	THIS->trans = 0;

	THIS->db.flags.system = !THIS->desc.name || THIS->driver->Database.IsSystem(&THIS->db, THIS->desc.name);
}

static bool check_opened(CCONNECTION *_object)
{
	DB_CurrentDatabase = &THIS->db;

	/*if (!THIS->db.handle)
		open_connection(THIS);*/

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


BEGIN_METHOD(CCONNECTION_new, GB_STRING url)

	char *url, *name, *p;

	THIS->db.handle = NULL;
	THIS->db.ignore_case = FALSE; // Now case is sensitive by default!
	THIS->db.timeout = 20; // Connection timeout is 20 seconds by default

	if (_current == NULL)
		_current = THIS;

	if (MISSING(url))
		return;

	url = GB.ToZeroString(ARG(url));

	p = index(url, ':');
	if (!p || p == url) goto __BAD_URL;
	*p++ = 0;
	if (p[0] != '/' || p[1] != '/') goto __BAD_URL;
	p += 2;

	THIS->desc.type = GB.NewZeroString(url);
	url = p;

	p = rindex(url, '/');
	if (!p || p == url) goto __BAD_URL;
	*p++ = 0;

	name = p;

	p = index(url, '@');
	if (p)
	{
		if (p == url)
			goto __BAD_URL;
		*p = 0;
		THIS->desc.user = GB.NewZeroString(url);
		url = p + 1;
	}

	p = index(url, ':');
	if (p)
	{
		*p = 0;
		THIS->desc.port = GB.NewZeroString(p + 1);
	}

	THIS->desc.host = GB.NewZeroString(url);
	THIS->desc.name = GB.NewZeroString(name);
	return;

__BAD_URL:

	GB.Error("Malformed URL");

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


BEGIN_PROPERTY(Connection_Timeout)

	if (READ_PROPERTY)
		GB.ReturnInteger(THIS->db.timeout);
	else
		THIS->db.timeout = VPROP(GB_INTEGER);

END_PROPERTY


BEGIN_PROPERTY(CCONNECTION_opened)

	CHECK_DB();

	GB.ReturnBoolean(THIS->db.handle != NULL);

END_PROPERTY


BEGIN_PROPERTY(CCONNECTION_error)

	CHECK_DB();

	GB.ReturnInteger(THIS->db.error);

END_PROPERTY


/*BEGIN_PROPERTY(Connection_Transaction)

	CHECK_DB();

	GB.ReturnInteger(THIS->trans);

END_PROPERTY*/


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
			if (THIS->db.ignore_case)
				GB.Error("This database driver cannot be case sensitive");
			else
				GB.Error("This database driver is always case sensitive");
			return;
		}
		THIS->db.ignore_case = VPROP(GB_BOOLEAN);
	}

END_PROPERTY
#endif

BEGIN_PROPERTY(CCONNECTION_ignore_charset)

	CHECK_DB();

	if (READ_PROPERTY)
		GB.ReturnBoolean(THIS->ignore_charset);
	else
		THIS->ignore_charset = VPROP(GB_BOOLEAN);

END_PROPERTY

BEGIN_PROPERTY(Connection_Collations)

	GB_ARRAY array;

	CHECK_DB();
	CHECK_OPEN();

	array = THIS->driver->GetCollations(&THIS->db);
	if (array)
		GB.ReturnObject(array);
	else
		GB.Error("Collations are not supported");

END_PROPERTY

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
		if (strncasecmp(query, "WHERE ", 6) && strncasecmp(query, "ORDER BY ", 9))
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

	/*if (check_table(THIS, table, TRUE))
		return;*/

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


BEGIN_METHOD(CCONNECTION_quote, GB_STRING name; GB_BOOLEAN is_table)

	char *name = STRING(name);
	int len = LENGTH(name);

	CHECK_DB();
	CHECK_OPEN();

	if (VARGOPT(is_table, FALSE)) // && THIS->db.flags.schema)
		GB.ReturnNewZeroString(DB_GetQuotedTable(THIS->driver, &THIS->db, GB.ToZeroString(ARG(name))));
	else
	{
		q_init();
		q_add(THIS->driver->GetQuote());
		q_add_length(name, len);
		q_add(THIS->driver->GetQuote());
		GB.ReturnString(q_get());
	}

END_METHOD


BEGIN_METHOD(CCONNECTION_format_blob, GB_STRING data)

	DB_BLOB blob;

	CHECK_DB();
	CHECK_OPEN();

	blob.data = STRING(data);
	blob.length = LENGTH(data);

	q_init();
	DB_CurrentDatabase = &THIS->db;
	(*THIS->driver->FormatBlob)(&blob, q_add_length);
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

	GB_SubCollectionNew(&THIS->databases, &_databases_desc, THIS);
	GB.ReturnObject(THIS->databases);

END_PROPERTY


BEGIN_PROPERTY(CCONNECTION_users)

	CHECK_DB();
	CHECK_OPEN();

	GB_SubCollectionNew(&THIS->users, &_users_desc, THIS);
	GB.ReturnObject(THIS->users);

END_PROPERTY


BEGIN_PROPERTY(CCONNECTION_tables)

	CHECK_DB();
	CHECK_OPEN();

	GB_SubCollectionNew(&THIS->tables, &_tables_desc, THIS);
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


BEGIN_PROPERTY(Connection_Handle)

	CHECK_DB();
	GB.ReturnPointer(THIS->db.handle);

END_PROPERTY


GB_DESC CConnectionDesc[] =
{
	GB_DECLARE("_Connection", sizeof(CCONNECTION)),

	GB_METHOD("_new", NULL, CCONNECTION_new, "[(DatabaseURL)s]"),
	GB_METHOD("_free", NULL, CCONNECTION_free, NULL),

	GB_PROPERTY("Type", "s", CCONNECTION_type),
	GB_PROPERTY("Host", "s", CCONNECTION_host),
	GB_PROPERTY("Login", "s", CCONNECTION_user),
	GB_PROPERTY("User", "s", CCONNECTION_user),
	GB_PROPERTY("Password", "s", CCONNECTION_password),
	GB_PROPERTY("Name", "s", CCONNECTION_name),
	GB_PROPERTY("Port", "s", CCONNECTION_port),
	GB_PROPERTY("Timeout", "i", Connection_Timeout),
	GB_PROPERTY_READ("Charset", "s", CCONNECTION_charset),
	GB_PROPERTY_READ("Version", "i", CCONNECTION_version),
	GB_PROPERTY_READ("Opened", "b", CCONNECTION_opened),
	GB_PROPERTY_READ("Error", "i", CCONNECTION_error),
	//GB_PROPERTY_READ("Transaction", "i", Connection_Transaction),
	GB_PROPERTY("IgnoreCharset", "b", CCONNECTION_ignore_charset),
	GB_PROPERTY_READ("Collations", "String[]", Connection_Collations),
	GB_STATIC_PROPERTY_READ("Handle", "p", Connection_Handle),

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

	GB_METHOD("Quote", "s", CCONNECTION_quote, "(Name)s[(Table)b]"),
	GB_METHOD("FormatBlob", "s", CCONNECTION_format_blob, "(Data)s"),

	GB_PROPERTY("Tables", ".Connection.Tables", CCONNECTION_tables),
	GB_PROPERTY("Databases", ".Connection.Databases", CCONNECTION_databases),
	GB_PROPERTY("Users", ".Connection.Users", CCONNECTION_users),
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
	//GB_STATIC_PROPERTY_READ("Transaction", "i", Connection_Transaction),
	GB_STATIC_PROPERTY("IgnoreCharset", "b", CCONNECTION_ignore_charset),
	GB_STATIC_PROPERTY_READ("Collations", "String[]", Connection_Collations),
	GB_STATIC_PROPERTY_READ("Handle", "p", Connection_Handle),

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

	GB_STATIC_METHOD("Quote", "s", CCONNECTION_quote, "(Name)s[(Table)b]"),
	GB_STATIC_METHOD("FormatBlob", "s", CCONNECTION_format_blob, "(Data)s"),

	GB_STATIC_PROPERTY("Tables", ".Connection.Tables", CCONNECTION_tables),
	//GB_STATIC_PROPERTY("Views", ".ConnectionViews", CCONNECTION_views),
	GB_STATIC_PROPERTY("Databases", ".Connection.Databases", CCONNECTION_databases),
	GB_STATIC_PROPERTY("Users", ".Connection.Users", CCONNECTION_users),

	GB_END_DECLARE
};


