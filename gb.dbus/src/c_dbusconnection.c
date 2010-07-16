/***************************************************************************

  c_dbusconnection.c

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
	conn = dbus_bus_get(type, &error);
	if (!conn)
		GB.Error("Cannot connect to the &1 bus", type == DBUS_BUS_SYSTEM ? "system" : "session");
	else
		dbus_connection_set_exit_on_disconnect(conn, FALSE);
	
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

BEGIN_METHOD_VOID(DBusConnection_exit)

	GB.Unref(POINTER(&_session));
	GB.Unref(POINTER(&_system));

END_METHOD

BEGIN_METHOD_VOID(DBusConnection_free)

	dbus_connection_unref(THIS->connection);

END_METHOD

BEGIN_METHOD(DBusConnection_Introspect, GB_STRING application; GB_STRING object)

	char *application = GB.ToZeroString(ARG(application));
	char *object;

	if (!MISSING(object))
		object = GB.ToZeroString(ARG(object));	
	else
		object = "/";
	
	GB.ReturnNewZeroString(DBUS_introspect(THIS->connection, application, object));

END_METHOD

BEGIN_METHOD(DBusConnection_CallMethod, GB_STRING application; GB_STRING object; GB_STRING interface; GB_STRING method; 
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

BEGIN_PROPERTY(DBusConnection_Applications)

	DBUS_call_method(THIS->connection, "org.freedesktop.DBus", "/", "org.freedesktop.DBus", "ListNames", "", "as", NULL);

END_PROPERTY


BEGIN_METHOD(DBusConnection_Register, GB_STRING name; GB_BOOLEAN unique)

	bool ret = DBUS_register(THIS->connection, GB.ToZeroString(ARG(name)), VARGOPT(unique, FALSE));
	
	GB.ReturnBoolean(ret);
	
END_METHOD


BEGIN_PROPERTY(DBusConnection_Name)

	GB.ReturnNewZeroString(dbus_bus_get_unique_name(THIS->connection));

END_PROPERTY

static void add_rule(char **match, const char *name, const char *rule, int len)
{
	if (len <= 0)
		len = strlen(rule);
	
	if (len == 1 && *rule == '*')
		return;
	
	if (*match)
		GB.AddString(match, ",", 1);
	
	GB.AddString(match, name, 0);
	GB.AddString(match, "='", 2);
	GB.AddString(match, rule, len);
	GB.AddString(match, "'", 1);
}

static bool handle_match(bool add, DBusConnection *connection, char *match)
{
	DBusError error;

	dbus_error_init(&error);
	
	if (add)
		dbus_bus_add_match(connection, match, &error);
	else
		dbus_bus_remove_match(connection, match, &error);
	
	GB.FreeString(&match);
	
	return dbus_error_is_set(&error);
}

BEGIN_METHOD(DBusConnection_AddMatch, GB_STRING type; GB_STRING object; GB_STRING member; GB_STRING interface; GB_STRING destination)

	char *match = NULL;
	
	add_rule(&match, "type", STRING(type), LENGTH(type));
	if (!MISSING(object)) add_rule(&match, "path", STRING(object), LENGTH(object));
	if (!MISSING(member)) add_rule(&match, "member", STRING(member), LENGTH(member));
	if (!MISSING(interface)) add_rule(&match, "interface", STRING(interface), LENGTH(interface));
	if (MISSING(destination))
		add_rule(&match, "destination", dbus_bus_get_unique_name(THIS->connection), 0);
	else
		add_rule(&match, "destination", STRING(destination), LENGTH(destination));
	
	GB.ReturnBoolean(handle_match(TRUE, THIS->connection, match));

END_METHOD


BEGIN_METHOD(DBusConnection_RemoveMatch, GB_STRING type; GB_STRING object; GB_STRING member; GB_STRING interface; GB_STRING destination)

	char *match = NULL;
	
	add_rule(&match, "type", STRING(type), LENGTH(type));
	if (!MISSING(object)) add_rule(&match, "path", STRING(object), LENGTH(object));
	if (!MISSING(member)) add_rule(&match, "member", STRING(member), LENGTH(member));
	if (!MISSING(interface)) add_rule(&match, "interface", STRING(interface), LENGTH(interface));
	if (MISSING(destination))
		add_rule(&match, "destination", dbus_bus_get_unique_name(THIS->connection), 0);
	else
		add_rule(&match, "destination", STRING(destination), LENGTH(destination));
	
	GB.ReturnBoolean(handle_match(FALSE, THIS->connection, match));

END_METHOD


GB_DESC CDBusConnectionDesc[] =
{
  GB_DECLARE("DBusConnection", sizeof(CDBUSCONNECTION)),

	GB_STATIC_METHOD("_exit", NULL, DBusConnection_exit, NULL),
	GB_METHOD("_free", NULL, DBusConnection_free, NULL),
	GB_METHOD("_Introspect", "s", DBusConnection_Introspect, "(Application)s[(Object)s]"),
	GB_METHOD("_CallMethod", "v", DBusConnection_CallMethod, "(Application)s(Object)s(Interface)s(Method)s(InputSignature)s(OutputSignature)s(Arguments)Array;"),
	GB_METHOD("_AddMatch", "b", DBusConnection_AddMatch, "(Type)s[(Object)s(Member)s(Interface)s(Destination)s]"),
	GB_METHOD("_RemoveMatch", "b", DBusConnection_RemoveMatch, "(Type)s[(Object)s(Member)s(Interface)s(Destination)s]"),
	GB_PROPERTY_READ("Applications", "String[]", DBusConnection_Applications),
	GB_METHOD("Register", "b", DBusConnection_Register, "(Name)s[(Unique)b]"),
	GB_PROPERTY_READ("Name", "s", DBusConnection_Name),

  GB_END_DECLARE
};

