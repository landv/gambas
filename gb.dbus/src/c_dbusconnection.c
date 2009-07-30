/***************************************************************************

  cdbus.c

  gb.dbus component

  (c) 2009 Benoît Minisini <gambas@users.sourceforge.net>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 1, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

***************************************************************************/

#define __C_DBUSCONNECTION_C

#include "c_dbusconnection.h"

static CDBUSCONNECTION *_system = NULL;
static CDBUSCONNECTION *_session = NULL;

static DBusConnection *get_bus(DBusBusType type)
{
	DBusConnection *conn;
  DBusError error;
	
	dbus_error_init (&error);
	conn = dbus_bus_get (type, &error);
	
	if (!conn)
		GB.Error("Cannot connect to the &1 bus", type == DBUS_BUS_SYSTEM ? "system" : "session");
	
	return conn;
}

CDBUSCONNECTION *CDBUSCONNECTION_get(DBusBusType type)
{
	DBusConnection *conn;
	
	if (type == DBUS_BUS_SYSTEM)
	{
		if (!_system)
		{
			conn = get_bus(type);
			if (conn)
			{
				GB.New(POINTER(&_system), GB.FindClass("DBusConnection"), NULL, NULL);
				GB.Ref(_system);
				_system->connection = conn;
			}
		}
		return _system;
	}
	else if (type == DBUS_BUS_SESSION)
	{
		if (!_session)
		{
			conn = get_bus(type);
			if (conn)
			{
				GB.New(POINTER(&_session), GB.FindClass("DBusConnection"), NULL, NULL);
				GB.Ref(_session);
				_session->connection = conn;
			}
		}
		return _session;
	}
	else
		return NULL;
}

static bool call_method(const char *path, const char *interface, const char *method, GB_VALUE *args, int nparam)
{
	DBusMessage *message;
	
	message = dbus_message_new_method_call (NULL,	path, interface, method);
  if (!message)
	{
		GB.Error("Couldn't allocate D-Bus message");
		return TRUE;
	}
	
	dbus_message_set_auto_start (message, TRUE);
	
	

	return FALSE;
}

BEGIN_METHOD_VOID(dbusconnection_exit)

	GB.Unref(POINTER(&_session));
	GB.Unref(POINTER(&_system));

END_METHOD

BEGIN_METHOD_VOID(dbusconnection_free)

	GB.FreeString(&THIS->path);
	GB.FreeString(&THIS->interface);
	dbus_connection_unref(THIS->connection);

END_METHOD

BEGIN_METHOD(dbusconnection_get, GB_STRING path; GB_STRING interface)

	GB.FreeString(&THIS->path);
	GB.FreeString(&THIS->interface);
	GB.NewString(&THIS->path, STRING(path), LENGTH(path));
	if (!MISSING(interface))
		GB.NewString(&THIS->interface, STRING(interface), LENGTH(interface));

END_METHOD

BEGIN_METHOD(dbusconnectionpath_unknown, GB_VALUE param[0])

	if (GB.IsProperty())
	{
		GB.Error("Unknown property");
		return;
	}
	
  call_method(THIS->path, THIS->interface, GB.GetUnknown(), ARG(param[0]), GB.NParam());

END_METHOD

GB_DESC CDBusObjectDesc[] =
{
  GB_DECLARE(".DBusObject", 0), GB_VIRTUAL_CLASS(),

	GB_METHOD("_unknown", "v", dbusconnectionpath_unknown, "."),

  GB_END_DECLARE
};

GB_DESC CDBusConnectionDesc[] =
{
  GB_DECLARE("DBusConnection", sizeof(CDBUSCONNECTION)),

	GB_STATIC_METHOD("_exit", NULL, dbusconnection_exit, NULL),
	GB_METHOD("_free", NULL, dbusconnection_free, NULL),
	GB_METHOD("_get", ".DBusObject", dbusconnection_get, "(Path)s[(Interface)s]"),

  GB_END_DECLARE
};

