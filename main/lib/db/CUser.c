/***************************************************************************

  CUser.c

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

#define __CUSER_C

#include "main.h"

#include "CUser.h"


static int valid_user(CUSER *_object)
{
  return !THIS->conn || !THIS->conn->db.handle;
}

static bool check_user(CCONNECTION *conn, const char *name, bool must_exist)
{
  bool exist = conn->driver->User.Exist(&conn->db, (char *)name);

  if (must_exist)
  {
    if (!exist)
    {
      GB.Error("Unknown user: &1", name);
      return TRUE;
    }
  }
  else
  {
    if (exist)
    {
      GB.Error("User already exists: &1", name);
      return TRUE;
    }
  }

  return FALSE;
}


void *CUSER_get(CCONNECTION *conn, const char *name)
{
  CUSER *_object;

  if (check_user(conn, name, TRUE))
    return NULL;

  GB.New(POINTER(&_object), GB.FindClass("DatabaseUser"), NULL, NULL);
  THIS->conn = conn;
  THIS->driver = conn->driver;
  GB.NewString(&THIS->name, name, 0);
  conn->driver->User.Info(&conn->db, THIS->name, &THIS->info);
  return THIS;
}


int CUSER_exist(CCONNECTION *conn, const char *name)
{
  return conn->driver->User.Exist(&conn->db, (char *)name);
}


void CUSER_list(CCONNECTION *conn, char ***list)
{
  conn->driver->User.List(&conn->db, list);
}


void CUSER_release(CCONNECTION *conn, void *_object)
{
	THIS->conn = NULL;
}


/***************************************************************************

  User

***************************************************************************/

BEGIN_METHOD_VOID(CUSER_free)

  if (!valid_user(THIS))
    GB.SubCollection.Remove(THIS->conn->users, THIS->name, 0);
  GB.FreeString(&THIS->name);
  GB.FreeString(&THIS->info.password);

END_METHOD


BEGIN_PROPERTY(CUSER_name)

  GB.ReturnString(THIS->name);

END_PROPERTY


BEGIN_METHOD_VOID(CUSER_delete)

  THIS->conn->driver->User.Delete(&THIS->conn->db, THIS->name);

END_METHOD


BEGIN_PROPERTY(CUSER_password)

  if (READ_PROPERTY)
    GB.ReturnString(THIS->info.password);
  else if (THIS->name)
  {
    GB.StoreString(PROP(GB_STRING), &THIS->info.password);
    THIS->driver->User.SetPassword(&THIS->conn->db, THIS->name, THIS->info.password);
  }

END_PROPERTY


BEGIN_PROPERTY(CUSER_administrator)

  GB.ReturnBoolean(THIS->info.admin);

END_PROPERTY


BEGIN_PROPERTY(CUSER_connection)

  GB.ReturnObject(THIS->conn);

END_PROPERTY


GB_DESC CUserDesc[] =
{
  GB_DECLARE("DatabaseUser", sizeof(CUSER)),
  GB_NOT_CREATABLE(),
  GB_HOOK_CHECK(valid_user),

  GB_METHOD("_free", NULL, CUSER_free, NULL),

  GB_METHOD("Delete", NULL, CUSER_delete, NULL),

  GB_PROPERTY_READ("Name", "s", CUSER_name),
  GB_PROPERTY_READ("Administrator", "b", CUSER_administrator),
  GB_PROPERTY("Password", "s", CUSER_password),
  GB_PROPERTY_READ("Connection", "Connection", CUSER_connection),

  GB_END_DECLARE
};



/***************************************************************************

  .ConnectionUsers

***************************************************************************/

#undef THIS
#define THIS ((GB_SUBCOLLECTION)_object)

BEGIN_METHOD(CUSER_add, GB_STRING name; GB_STRING password; GB_BOOLEAN admin)

  CCONNECTION *conn = GB.SubCollection.Container(THIS);
  char *name = GB.ToZeroString(ARG(name));
  DB_USER info;

  CLEAR(&info);

  if (DB_CheckNameWith(name, "user", "@%"))
    return;

  if (check_user(conn, name, FALSE))
    return;

  info.admin = VARGOPT(admin, FALSE);
  if (!MISSING(password))
    info.password = GB.ToZeroString(ARG(password));

  conn->driver->User.Create(&conn->db, name, &info);

END_METHOD


BEGIN_METHOD(CUSER_remove, GB_STRING name)

  CCONNECTION *conn = GB.SubCollection.Container(THIS);
  char *name = GB.ToZeroString(ARG(name));

  if (check_user(conn, name, TRUE))
    return;

  GB.SubCollection.Remove(THIS, STRING(name), LENGTH(name));
  conn->driver->User.Delete(&conn->db, name);

END_METHOD


GB_DESC CConnectionUsersDesc[] =
{
  GB_DECLARE(".ConnectionUsers", 0), GB_INHERITS(".SubCollection"),

  GB_METHOD("Add", NULL, CUSER_add, "(Name)s[(Password)s(Admin)b]"),
  GB_METHOD("Remove", NULL, CUSER_remove, "(Name)s"),

  GB_END_DECLARE
};


