/***************************************************************************

  main.c

  The database component

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
#include <ctype.h>

#include "gb_common.h"

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
static bool _debug = FALSE;
static char *_try_another = NULL;


int DB_CheckNameWith(const char *name, const char *msg, const char *more)
{
  unsigned char c;
  const char *p = name;

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


void DB_LowerString(char *s)
{
  register char c;

  for(;;)
  {
    c = *s;
    if (!c)
      return;
    *s++ = tolower(c);
  }
}


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
    GB.NewString(&str, array[i], 0);
    *((char **)GB.Array.Get(garray, i)) = str;
  }

  return garray;
}

int DB_FindStringArray(char **array, char *elt)
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


void DB_TryAnother(char *driver)
{
	_try_another = driver;
}

static DB_DRIVER *DB_GetDriver(char *type)
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

  GB.LoadComponent(comp);

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
  char *type = desc->type;

	CLEAR(db);

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
  long l;
  int i;

  if (arg->type == GB_T_VARIANT)
    GB.Conv(arg, ((GB_VARIANT *)arg)->value.type);

	if (arg->type == (GB_TYPE)CLASS_Blob)
	{
		(*driver->FormatBlob)((DB_BLOB *)(((GB_OBJECT *)arg)->value), add);
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

        l = sprintf(buffer, "%ld", VALUE((GB_INTEGER *)arg));
        add(buffer, l);
        return;

      case GB_T_LONG:

        l = sprintf(buffer, "%lld", VALUE((GB_LONG *)arg));
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

      case GB_T_NULL:

        add("NULL", 4);
        return;

      default:
      	fprintf(stderr, "gb.db: DB_Format: unsupported datatype: %ld\n", arg->type);
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
        val->value.addr = arg->_string.value;
        val->value.start = 0;
        if (arg->type == GB_T_STRING)
          val->value.len = GB.StringLength(arg->_string.value);
        else
          val->value.len = strlen(arg->_string.value);
      }
      break;

    default:
      memcpy(&value, arg, sizeof(GB_VARIANT_VALUE));
      break;
  }

  DB_Format(driver, &value, add);
}


static int query_narg;
static GB_VALUE *query_arg;
static DB_DRIVER *query_driver;

static void get_param(int index, char **str, long *len)
{
  if (index < 1 || index > query_narg)
    return;

  DB_Format(query_driver, &query_arg[index - 1], (DB_FORMAT_CALLBACK)GB.SubstAdd);
}

char *DB_MakeQuery(DB_DRIVER *driver, char *pattern, long len, int narg, GB_VALUE *arg)
{
  char *query;

  query_narg = narg;
  query_arg = arg;
  query_driver = driver;

	if (narg == 0)
		GB.TempString(&query, pattern, len);
	else
  	query = GB.SubstString(pattern, len, get_param);

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
}

void q_add(char *str)
{
  if (str)
    GB.AddString(&_query, str, 0);
}

void q_add_length(char *str, long len)
{
  if (str)
    GB.AddString(&_query, str, len);
}

char *q_get(void)
{
  return _query;
}

char *q_steal(void)
{
  char *query = _query;
  _query = NULL;
  return query;
}

long q_length(void)
{
  return GB.StringLength(_query);
}

void DB_SetDebug(int debug)
{
  _debug = debug;
}

int DB_IsDebug(void)
{
  return _debug;
}


GB_DESC *GB_CLASSES [] EXPORT =
{
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

  (void *)q_init,
  (void *)q_add,
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

