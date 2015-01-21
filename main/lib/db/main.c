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
#include <ctype.h>

#include "gb_common.h"

#include "c_subcollection.h"
#include "CConnection.h"
#include "CDatabase.h"
#include "CUser.h"
#include "CTable.h"
#include "CField.h"
#include "CIndex.h"
#include "CResult.h"
#include "CResultField.h"

#include "sqlite.h"
#include "main.h"


GB_INTERFACE GB EXPORT;


static DB_DRIVER *_drivers[MAX_DRIVER];
static int _drivers_count = 0;
static char *_query = NULL;

#define TEMP_MAX 64
static char _temp[TEMP_MAX];
static int _temp_len;

static bool _debug = FALSE;
static const char *_try_another = NULL;

DB_DATABASE *DB_CurrentDatabase = NULL;

int DB_CheckNameWith(const char *name, const char *msg, const char *more)
{
	unsigned char c;
	const char *p = name;

	if (!name || !*name)
	{
		GB.Error("Void &1 name", msg);
		return TRUE;
	}

	while ((c = *p++))
	{
		if ((c >= 'A' && c <='Z') || (c >= 'a' && c <='z') || (c >= '0' && c <= '9') || c == '_')
			continue;

		if (more && index(more, c))
			continue;

		GB.Error("Bad &1 name: &2", msg, name);
		return TRUE;
	}

	return FALSE;
}


// void DB_LowerString(char *s)
// {
//   register char c;
// 
//   for(;;)
//   {
//     c = *s;
//     if (!c)
//       return;
//     *s++ = tolower(c);
//   }
// }

void DB_FreeStringArray(char ***parray)
{
	int i;
	char **array = *parray;

	if (!*parray)
		return;

	for (i = 0; i < GB.Count(array); i++)
		GB.FreeString(&array[i]);

	GB.FreeArray(parray);
}

GB_ARRAY DB_StringArrayToGambasArray(char **array)
{
	GB_ARRAY garray;
	int i, n;
	char *str;

	n = GB.Count(array);

	GB.Array.New(&garray, GB_T_STRING, n);

	for (i = 0; i < n; i++)
	{
		str = GB.NewZeroString(array[i]);
		*((char **)GB.Array.Get(garray, i)) = str;
	}

	return garray;
}

int DB_FindStringArray(char **array, const char *elt)
{
	int i;

	for (i = 0; i < GB.Count(array); i++)
	{
		if (!strcasecmp(elt, array[i]))
			return i;
	}

	return -1;
}


static void DB_Register(DB_DRIVER *driver)
{
	if (_drivers_count >= MAX_DRIVER)
		return;

	_drivers[_drivers_count] = driver;
	_drivers_count++;
}


void DB_TryAnother(const char *driver)
{
	_try_another = driver;
}

static DB_DRIVER *DB_GetDriver(const char *type)
{
	int i;
	char comp[type ? strlen(type) + 8 : 1];

	if (!type)
	{
		GB.Error("Driver name missing");
		return NULL;
	}

	strcpy(comp, "gb.db.");
	strcat(comp, type);

	GB.Component.Load(comp);
	GB.Error(NULL); // reset the error flag;
	
	for (i = 0; i < _drivers_count; i++)
	{
		if (strcasecmp(_drivers[i]->name, type) == 0)
			return _drivers[i];
	}

	GB.Error("Cannot find driver for database: &1", type);
	return NULL;
}


bool DB_Open(DB_DESC *desc, DB_DRIVER **driver, DB_DATABASE *db)
{
	DB_DRIVER *d;
	int res;
	int timeout;
	const char *type = desc->type;

	timeout = db->timeout;
	CLEAR(db);
	db->timeout = timeout;

	for(;;)
	{
		d = DB_GetDriver(type);
		if (!d)
			return TRUE;

		*driver = d;

		_try_another = NULL;
		res = (*d->Open)(desc, db);
		if (!res)
			return FALSE;

		if (!_try_another)
			return TRUE;

		type = _try_another;
	}
}

void DB_Format(DB_DRIVER *driver, GB_VALUE *arg, DB_FORMAT_CALLBACK add)
{
	static char buffer[32];

	char *s;
	int l;
	int i;

	if (arg->type == GB_T_VARIANT)
		GB.Conv(arg, ((GB_VARIANT *)arg)->value.type);

	if (arg->type == (GB_TYPE)CLASS_Blob)
	{
		(*driver->FormatBlob)((DB_BLOB *)(((GB_OBJECT *)arg)->value), add);
		return;
	}

	if ((arg->type == GB_T_DATE && arg->_date.value.date == 0 && arg->_date.value.time == 0)
			|| (arg->type == GB_T_STRING && arg->_string.value.len == 0)
			|| (arg->type == GB_T_NULL))
	{
		add("NULL", 4);
		return;
	}
	
	if (!(*driver->Format)(arg, add))
	{
		switch (arg->type)
		{
			case GB_T_BOOLEAN:

				if (VALUE((GB_BOOLEAN *)arg))
					add("TRUE", 4);
				else
					add("FALSE", 5);

				return;

			case GB_T_BYTE:
			case GB_T_SHORT:
			case GB_T_INTEGER:

				l = sprintf(buffer, "%d", VALUE((GB_INTEGER *)arg));
				add(buffer, l);
				return;

			case GB_T_LONG:

				l = sprintf(buffer, "%" PRId64, VALUE((GB_LONG *)arg));
				add(buffer, l);
				return;

			case GB_T_FLOAT:
				
				GB.NumberToString(FALSE, VALUE((GB_FLOAT *)arg), NULL, &s, &l);
				add(s, l);

				return;

			case GB_T_STRING:
			case GB_T_CSTRING:

				s = VALUE((GB_STRING *)arg).addr + VALUE((GB_STRING *)arg).start;
				l = VALUE((GB_STRING *)arg).len;

				add("'", 1);

				for (i = 0; i < l; i++, s++)
				{
					add(s, 1);
					if (*s == '\'' || *s == '\\')
						add(s, 1);
				}

				add("'", 1);
				return;

			default:
				fprintf(stderr, "gb.db: DB_Format: unsupported datatype: %d\n", (int)arg->type);
				return;
		}
	}
}


void DB_FormatVariant(DB_DRIVER *driver, GB_VARIANT_VALUE *arg, DB_FORMAT_CALLBACK add)
{
	GB_VALUE value;

	value.type = arg->type;

	switch(arg->type)
	{
		case GB_T_NULL:
			break;

		case GB_T_STRING:
		case GB_T_CSTRING:
			{
				GB_STRING *val = (GB_STRING *)(void *)&value;
				val->value.addr = arg->value._string;
				val->value.start = 0;
				if (arg->type == GB_T_STRING)
					val->value.len = GB.StringLength(arg->value._string);
				else
					val->value.len = strlen(arg->value._string);
			}
			break;

		default:
			value.type = GB_T_VARIANT;
			value._variant.value = *arg;
			GB.Conv(&value, arg->type);
			break;
	}

	DB_Format(driver, &value, add);
}


static int query_narg;
static GB_VALUE *query_arg;
static DB_DRIVER *query_driver;

static void mq_add_param(int index)
{
	if (index < 1 || index > query_narg)
		return;

	DB_Format(query_driver, &query_arg[index - 1], (DB_FORMAT_CALLBACK)GB.SubstAddCallback);
}

char *DB_MakeQuery(DB_DRIVER *driver, const char *pattern, int len, int narg, GB_VALUE *arg)
{
	char *query;

	query_narg = narg;
	query_arg = arg;
	query_driver = driver;

	if (narg == 0)
		query = GB.TempString(pattern, len);
	else
		query = GB.SubstStringAdd(pattern, len, mq_add_param);

	if (!query || *query == 0)
	{
		GB.Error("Void query");
		return NULL;
	}
	else
		return query;
}


void q_init(void)
{
	GB.FreeString(&_query);
	_query = NULL;
	_temp_len = 0;
}

static void q_dump_temp(void)
{
	if (!_temp_len)
		return;
		
	_query = GB.AddString(_query, _temp, _temp_len);
	_temp_len = 0;
}

void q_add_length(const char *str, int len)
{
	if (!str)
		return;
	
	if ((_temp_len + len) > TEMP_MAX)
		q_dump_temp();
	
	if (len > TEMP_MAX)
		_query = GB.AddString(_query, str, len);
	else
	{
		memcpy(&_temp[_temp_len], str, len);
		_temp_len += len;
	}
}

void q_add(const char *str)
{
	if (str)
		q_add_length(str, strlen(str));
}

void q_add_lower(const char *str)
{
	int i, len;
	char *lstr;

	if (!str)
		return;

	len = strlen(str);

	if (len <= 0)
		return;

	lstr = GB.TempString(str, len);
	for (i = 0; i < len; i++)
		lstr[i] = GB.ToLower(lstr[i]);

	q_add_length(lstr, len);
}

char *q_get(void)
{
	q_dump_temp();
	return _query;
}

char *q_steal(void)
{
	char *s;
	q_dump_temp();
	s = _query;
	_query = NULL;
	return s;
}

int q_length(void)
{
	return GB.StringLength(_query) + _temp_len;
}

void DB_SetDebug(int debug)
{
	_debug = debug;
}

int DB_IsDebug(void)
{
	return _debug;
}

static char *_quote;
DB_SUBST_CALLBACK _quote_cb;

static void ss_get_param(int index, char **str, int *len)
{
	(*_quote_cb)(index, str, len, _quote[index]);
}

char *DB_SubstString(const char *pattern, int len_pattern, DB_SUBST_CALLBACK add)
{
	int i;
	unsigned char last_c, c;
	int n;
	char quote[20] = {0};
	
	c = 0;
	
	len_pattern--;
	for (i = 0; i < len_pattern; i++)
	{
		last_c = c;
		c = pattern[i];
		
		if (c == '&')
		{
			c = pattern[++i];
			if (c == '&')
				continue;
			if (isdigit(c))
			{
				n = c - '0';
				c = pattern[++i];
				if (isdigit(c))
				{
					n = n * 10 + c - '0';
					i++;
				}
				quote[n] = last_c;
			}
		} 
	}
	
	_quote_cb = add;
	_quote = quote;
	
	return GB.SubstString(pattern, len_pattern, ss_get_param);
}

char *DB_QuoteString(const char *str, int len, char quote)
{
	char *res, *p, c;
	int len_res;
	int i;
	
	len_res = len;
	for (i = 0; i < len; i++)
	{
		if (str[i] == quote)
			len_res++;
	}
	
	res = GB.TempString(NULL, len_res);
	
	p = res;
	for (i = 0; i < len; i++)
	{
		c = str[i];
		*p++ = c;
		if (c == quote || c == '\\')
			*p++ = c;
	}
	*p = 0;
	
	return res;
}

char *DB_UnquoteString(const char *str, int len, char quote)
{
	char *res, *p, c;
	int len_res;
	int i;
	
	if (len >= 2 && str[0] == quote && str[len - 1] == quote)
	{
		str++;
		len -= 2;
	}
	
	if (!len)
		return "";
	
	len_res = len;
	for (i = 0; i < (len - 1); i++)
	{
		if ((str[i] == quote && str[i + 1] == quote) || str[i] == '\\')
		{
			len_res--;
			i++;
		}
	}
	
	res = GB.TempString(NULL, len_res);
	
	p = res;
	for (i = 0; i < len; i++)
	{
		c = str[i];
		if (c == quote && (i + 1) < len && str[i + 1] == quote)
			i++;
		else if (c == '\\' && (i + 1) < len)
		{
			c = str[i + 1];
			i++;
		}

		*p++ = c;
	}
	*p = 0;
	
	return res;
}

DB_DATABASE *DB_GetCurrent()
{
	return DB_CurrentDatabase;
}

char *DB_GetQuotedTable(DB_DRIVER *driver, DB_DATABASE *db, const char *table)
{
	int len;
	char *point = NULL;
	char *res;
	const char *quote;
	
	if (!table || !*table)
		return "";
	
	len = strlen(table);
	if (db->flags.schema)
		point = index(table, '.');
	
	quote = (*driver->GetQuote)();
	
	if (!point)
	{
		res = GB.TempString(NULL, len + 2);
		sprintf(res, "%s%s%s", quote, table, quote);
	}
	else
	{
		res = GB.TempString(NULL, len + 2);
		sprintf(res, "%.*s.%s%s%s", (int)(point - table), table, quote, point + 1, quote);
	}
	
	return res;
}


GB_DESC *GB_CLASSES [] EXPORT =
{
	SubCollectionDesc,
	CIndexDesc,
	CFieldDesc,
	CTableFieldsDesc,
	CTableIndexesDesc,
	CTableDesc,
	CUserDesc,
	CDatabaseDesc,
	CConnectionUsersDesc,
	CConnectionDatabasesDesc,
	CConnectionTablesDesc,
	CConnectionDesc,
	CDBDesc,
	CBlobDesc,
	CResultFieldDesc,
	CResultFieldsDesc,
	CResultDesc,
	NULL
};

void *GB_DB_1[] EXPORT = {

	(void *)1,

	(void *)DB_Register,
	(void *)DB_Format,
	(void *)DB_FormatVariant,
	(void *)DB_IsDebug,
	(void *)DB_TryAnother,
	(void *)DB_SubstString,
	(void *)DB_QuoteString,
	(void *)DB_UnquoteString,
	(void *)DB_GetCurrent,

	(void *)q_init,
	(void *)q_add,
	(void *)q_add_lower,
	(void *)q_add_length,
	(void *)q_get,
	(void *)q_steal,
	(void *)q_length,

	(void *)DB_FindStringArray,

	NULL
};


int EXPORT GB_INIT(void)
{
	DB_Register(&DB_sqlite_pseudo_driver);
	return 0;
}


void EXPORT GB_EXIT()
{
	GB.FreeString(&_query);
}

