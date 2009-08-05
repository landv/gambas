/***************************************************************************

  c_dbusconnection.c

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

#include "helper.h"
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

BEGIN_METHOD_VOID(dbusconnection_exit)

	GB.Unref(POINTER(&_session));
	GB.Unref(POINTER(&_system));

END_METHOD

BEGIN_METHOD_VOID(dbusconnection_free)

	dbus_connection_unref(THIS->connection);

END_METHOD

BEGIN_METHOD(dbusconnection_Introspect, GB_STRING application; GB_STRING object)

	char *application = GB.ToZeroString(ARG(application));
	char *object;

	if (!MISSING(object))
		object = GB.ToZeroString(ARG(object));	
	else
		object = "/";
	
	GB.ReturnNewZeroString(DBUS_introspect(THIS->connection, application, object));

END_METHOD

BEGIN_METHOD(dbusconnection_CallMethod, GB_STRING application; GB_STRING object; GB_STRING interface; GB_STRING method; 
             GB_STRING input_signature; GB_STRING output_signature; GB_OBJECT arguments)

	char *application = GB.ToZeroString(ARG(application));
	char *object = GB.ToZeroString(ARG(object));
	char *interface = GB.ToZeroString(ARG(interface));
	char *method = GB.ToZeroString(ARG(method));
	char *input_signature = GB.ToZeroString(ARG(input_signature));
	char *output_signature = GB.ToZeroString(ARG(output_signature));
	
	if (DBUS_validate_path(object, LENGTH(object)))
	{
		GB.Error("Invalid object path");
		return;
	}
	
	if (!*interface)
		interface = NULL;
	else if (DBUS_validate_interface(interface, LENGTH(interface)))
	{
		GB.Error("Invalid interface name");
		return;
	}
	
	if (DBUS_validate_method(method, LENGTH(method)))
	{
		GB.Error("Invalid method name");
		return;
	}
	
	DBUS_call_method(THIS->connection, application, object, interface, method, input_signature, output_signature, VARG(arguments));

END_METHOD

GB_DESC CDBusConnectionDesc[] =
{
  GB_DECLARE("DBusConnection", sizeof(CDBUSCONNECTION)),

	GB_STATIC_METHOD("_exit", NULL, dbusconnection_exit, NULL),
	GB_METHOD("_free", NULL, dbusconnection_free, NULL),
	GB_METHOD("_Introspect", "s", dbusconnection_Introspect, "(Application)s[(Object)s]"),
	GB_METHOD("_CallMethod", "v", dbusconnection_CallMethod, "(Application)s(Object)s(Interface)s(Method)s(InputSignature)s(OutputSignature)s(Arguments)Array;"),

  GB_END_DECLARE
};

