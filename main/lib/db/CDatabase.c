/***************************************************************************

  CDatabase.c

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

#define __CDATABASE_C

#include "main.h"

#include "CDatabase.h"


static int valid_database(CDATABASE *_object)
{
  return !THIS->conn || !THIS->conn->db.handle;
}

static bool check_database(CCONNECTION *conn, const char *name, bool must_exist)
{
  bool exist = conn->driver->Database.Exist(&conn->db, (char *)name);

  if (must_exist)
  {
    if (!exist)
    {
      GB.Error("Unknown database: &1", name);
      return TRUE;
    }
  }
  else
  {
    if (exist)
    {
      GB.Error("Database already exists: &1", name);
      return TRUE;
    }
  }

  return FALSE;
}


void *CDATABASE_get(CCONNECTION *conn, const char *name)
{
  CDATABASE *_object;

  if (check_database(conn, name, TRUE))
    return NULL;

  _object = GB.New(GB.FindClass("Database"), NULL, NULL);
  THIS->conn = conn;
  THIS->driver = conn->driver;
  THIS->name = GB.NewZeroString(name);
  return THIS;
}

int CDATABASE_exist(CCONNECTION *conn, const char *name)
{
  return conn->driver->Database.Exist(&conn->db, (char *)name);
}

void CDATABASE_list(CCONNECTION *conn, char ***list)
{
  conn->driver->Database.List(&conn->db, list);
}

void CDATABASE_release(CCONNECTION *conn, void *_object)
{
	THIS->conn = NULL;
}


/***************************************************************************

  Database

***************************************************************************/

BEGIN_METHOD_VOID(CDATABASE_free)

  if (!valid_database(THIS))
    GB_SubCollectionRemove(THIS->conn->databases, THIS->name, 0);
  GB.FreeString(&THIS->name);

END_METHOD


BEGIN_PROPERTY(CDATABASE_name)

  GB.ReturnString(THIS->name);

END_PROPERTY


BEGIN_PROPERTY(CDATABASE_system)

  GB.ReturnBoolean(THIS->driver->Database.IsSystem(&THIS->conn->db, THIS->name));

END_PROPERTY


BEGIN_METHOD_VOID(CDATABASE_delete)

  THIS->conn->driver->Database.Delete(&THIS->conn->db, THIS->name);

END_METHOD


BEGIN_PROPERTY(CDATABASE_connection)

  GB.ReturnObject(THIS->conn);

END_PROPERTY




GB_DESC CDatabaseDesc[] =
{
  GB_DECLARE("Database", sizeof(CDATABASE)),
  GB_NOT_CREATABLE(),
  GB_HOOK_CHECK(valid_database),

  GB_METHOD("_free", NULL, CDATABASE_free, NULL),

  GB_METHOD("Delete", NULL, CDATABASE_delete, NULL),

  GB_PROPERTY_READ("Name", "s", CDATABASE_name),
  GB_PROPERTY_READ("System", "b", CDATABASE_system),
  GB_PROPERTY_READ("Connection", "Connection", CDATABASE_connection),

  GB_END_DECLARE
};



/***************************************************************************

  .Connection.Databases

***************************************************************************/

#undef THIS
#define THIS ((CSUBCOLLECTION *)_object)

BEGIN_METHOD(CDATABASE_add, GB_STRING name)

  CCONNECTION *conn = GB_SubCollectionContainer(THIS);
  char *name = GB.ToZeroString(ARG(name));

  if (DB_CheckNameWith(name, "database", conn->db.db_name_char))
    return;

  if (check_database(conn, name, FALSE))
    return;

  conn->driver->Database.Create(&conn->db, name);

END_METHOD


BEGIN_METHOD(CDATABASE_remove, GB_STRING name)

  CCONNECTION *conn = GB_SubCollectionContainer(THIS);
  char *name = GB.ToZeroString(ARG(name));

  GB_SubCollectionRemove(THIS, STRING(name), LENGTH(name));
	
  if (check_database(conn, name, TRUE))
    return;

  conn->driver->Database.Delete(&conn->db, name);

END_METHOD



GB_DESC CConnectionDatabasesDesc[] =
{
  GB_DECLARE(".Connection.Databases", 0), GB_INHERITS(".SubCollection"),

  GB_METHOD("Add", NULL, CDATABASE_add, "(Name)s"),
  GB_METHOD("Remove", NULL, CDATABASE_remove, "(Name)s"),

  GB_END_DECLARE
};


