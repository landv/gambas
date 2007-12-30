/***************************************************************************

  CResultField.c

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

#define __CRESULTFIELD_C

#include <stdlib.h>

#include "main.h"

#include "CResultField.h"


static int valid_result_field(CRESULTFIELD *_object)
{
  return !THIS->result || !THIS->result->conn || !THIS->result->conn->db.handle;
}


int CRESULTFIELD_find(CRESULT *result, const char *name, bool error)
{
  long index;
  char *p;

	if (!name || !*name)
		return -1;

  index = strtol(name, &p, 10);
  if (*name && *p == 0)
  {
    if (index < 0 || index >= result->info.nfield)
    {
    	if (error)
      	GB.Error("Bad field index");
      index = -1;
    }
  }
  else
  {
    if (result->handle)
      index = result->driver->Result.Field.Index(result->handle, (char *)name, &result->conn->db);
    else
    {
      for (index = 0; index < result->info.nfield; index++)
      {
        if (strcmp(name, result->info.field[index].name) == 0)
          break;
      }
    }

    if (index < 0 || index >= result->info.nfield)
    {
    	if (error)
      	GB.Error("Unknown field: &1", name);
      index = -1;
    }
  }

  return index;
}


static CRESULTFIELD *make_result_field(CRESULT *result, long index)
{
  CRESULTFIELD *_object;

  GB.New(POINTER(&_object), GB.FindClass("ResultField"), NULL, NULL);
  THIS->result = result;
  THIS->driver = result->conn->driver;
  THIS->index = index;

  return _object;
}


void *CRESULTFIELD_get(CRESULT *result, const char *name)
{
  long index;

  if ((long)name & 0xFFFF0000)
    index = CRESULTFIELD_find(result, name, TRUE);
  else
    index = (long)name;

  if (index < 0)
    return NULL;
  else
    return make_result_field(result, index);
}


int CRESULTFIELD_exist(CRESULT *result, const char *name)
{
  return CRESULTFIELD_find(result, name, FALSE) >= 0;
}

char *CRESULTFIELD_key(CRESULT *result, long index)
{
  if (result->handle)
    return result->driver->Result.Field.Name(result->handle, index);
  else
    return result->info.field[index].name;
}

void CRESULTFIELD_release(CRESULT *result, void *_object)
{
	THIS->result = NULL;
}




/***************************************************************************

  ResultField

***************************************************************************/

BEGIN_METHOD_VOID(CRESULTFIELD_free)

	if (!valid_result_field(THIS))
  	GB.SubCollection.Remove(THIS->result->fields, CRESULTFIELD_key(THIS->result, THIS->index), 0);

END_METHOD



BEGIN_PROPERTY(CRESULTFIELD_name)

  GB.ReturnNewZeroString(CRESULTFIELD_key(THIS->result, THIS->index));

END_PROPERTY


BEGIN_PROPERTY(CRESULTFIELD_type)

  CRESULT *result = THIS->result;

  if (result->handle)
    GB.ReturnInteger(result->driver->Result.Field.Type(result->handle, THIS->index));
  else
    GB.ReturnInteger(result->info.field[THIS->index].type);

END_PROPERTY


BEGIN_PROPERTY(CRESULTFIELD_length)

  CRESULT *result = THIS->result;

  if (result->handle)
    GB.ReturnInteger(result->driver->Result.Field.Length(result->handle, THIS->index));
  else
    GB.ReturnInteger(result->info.field[THIS->index].length);

END_PROPERTY


/*BEGIN_PROPERTY(CRESULTFIELD_default)

  GB.Error("No default value");

END_PROPERTY*/

BEGIN_PROPERTY(CRESULTFIELD_result)

  GB.ReturnObject(THIS->result);

END_PROPERTY



GB_DESC CResultFieldDesc[] =
{
  GB_DECLARE("ResultField", sizeof(CRESULTFIELD)),
  GB_NOT_CREATABLE(),
  GB_HOOK_CHECK(valid_result_field),

  GB_METHOD("_free", NULL, CRESULTFIELD_free, NULL),

  GB_PROPERTY_READ("Name", "s", CRESULTFIELD_name),
  GB_PROPERTY_READ("Type", "i", CRESULTFIELD_type),
  GB_PROPERTY_READ("Length", "i", CRESULTFIELD_length),
  //GB_PROPERTY_READ("Default", "v", CRESULTFIELD_default),

  GB_PROPERTY_READ("Result", "Result", CRESULTFIELD_result),

  GB_END_DECLARE
};



/***************************************************************************

  .ResultFields

***************************************************************************/

#undef THIS
#define THIS ((GB_SUBCOLLECTION)_object)

BEGIN_PROPERTY(CRESULTFIELD_count)

  CRESULT *result = GB.SubCollection.Container(THIS);
  GB.ReturnInteger(result->info.nfield);

END_PROPERTY


BEGIN_METHOD_VOID(CRESULTFIELD_next)

  CRESULT *result = GB.SubCollection.Container(THIS);
  long *index = (long *)GB.GetEnum();
  CRESULTFIELD *rf;

  if (*index >= result->info.nfield)
    GB.StopEnum();
  else
  {
    rf = GB.SubCollection.Get(THIS, CRESULTFIELD_key(result, *index), 0);
    (*index)++;
    GB.ReturnObject(rf);
  }

END_METHOD



GB_DESC CResultFieldsDesc[] =
{
  GB_DECLARE(".ResultFields", 0), GB_INHERITS(".SubCollection"),

  GB_PROPERTY_READ("Count", "i", CRESULTFIELD_count),
  //GB_PROPERTY_READ("Length", "i", CRESULTFIELD_count),
  GB_METHOD("_next", "ResultField", CRESULTFIELD_next, NULL),

  GB_END_DECLARE
};



