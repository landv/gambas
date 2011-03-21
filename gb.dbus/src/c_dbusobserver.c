/***************************************************************************

  c_dbusobserver.c

  (c) 2000-2011 Benoît Minisini <gambas@users.sourceforge.net>

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

#define __C_DBUSOBSERVER_C

#include "helper.h"
#include "c_dbusconnection.h"
#include "c_dbusobserver.h"

DECLARE_EVENT(EVENT_MESSAGE);

CDBUSOBSERVER *DBUS_observers = NULL;

void DBUS_raise_observer(CDBUSOBSERVER *_object)
{
	GB.Raise(THIS, EVENT_MESSAGE, 0);
}

static void add_rule(char **match, const char *name, const char *rule)
{
	if (!rule)
		return;
	
	if (rule[0] == '*' && rule[1] == 0)
		return;
	
	if (*match)
		GB.AddString(match, ",", 1);
	
	GB.AddString(match, name, 0);
	GB.AddString(match, "='", 2);
	GB.AddString(match, rule, 0);
	GB.AddString(match, "'", 1);
}

static void set_filter(char **property, const char *str, int len)
{
	if (!str)
		return;
	
	if (len <= 0)
		len = strlen(str);
	
	if (len == 0) // || (len == 1 && *str == '*'))
		return;
	
	*property = GB.NewString(str, len);
}

static void update_match(CDBUSOBSERVER *_object, bool noerr)
{
	static char *type[] = { "method_call", "method_return", "signal", "error" };
	char *match = NULL;
	DBusError error;

	if (THIS->type >= 0 && THIS->type <= 3)
		add_rule(&match, "type", type[THIS->type]);
	
	add_rule(&match, "path", THIS->object);
	add_rule(&match, "member", THIS->member);
	add_rule(&match, "interface", THIS->interface);
	
	if (THIS->destination && *(THIS->destination))
		add_rule(&match, "destination", THIS->destination);
	else
		add_rule(&match, "destination", dbus_bus_get_unique_name(THIS->connection));

	dbus_error_init(&error);
	
	if (THIS->enabled)
	{
		dbus_bus_add_match(THIS->connection, match, &error);
		if (dbus_error_is_set(&error))
		{
			if (!noerr) GB.Error("Cannot enable observer");
			THIS->enabled = FALSE;
		}
	}
	else
	{
		dbus_bus_remove_match(THIS->connection, match, &error);
		if (dbus_error_is_set(&error))
		{
			if (!noerr) GB.Error("Cannot disable observer");
			THIS->enabled = TRUE;
		}
	}
	
	dbus_bus_flush_connection(THIS->connection);
	
	GB.FreeString(&match);
}

BEGIN_METHOD(DBusObserver_new, GB_OBJECT connection; GB_INTEGER type; GB_STRING object; GB_STRING member; GB_STRING interface; GB_STRING destination)

	CDBUSCONNECTION *connection = VARG(connection);
	
	if (GB.CheckObject(connection))
		return;
	
	THIS->connection = connection->connection;

	THIS->type = VARG(type);
	if (!MISSING(object)) set_filter(&THIS->object, STRING(object), LENGTH(object));
	if (!MISSING(member)) set_filter(&THIS->member, STRING(member), LENGTH(member));
	if (!MISSING(interface)) set_filter(&THIS->interface, STRING(interface), LENGTH(interface));
	if (!MISSING(destination)) set_filter(&THIS->destination, STRING(destination), LENGTH(destination));
	
	THIS->next = DBUS_observers;
	
	if (DBUS_observers)
		DBUS_observers->prev = THIS;
	
	DBUS_observers = THIS;
	
END_METHOD

BEGIN_METHOD_VOID(DBusObserver_free)

	if (THIS->enabled)
	{
		THIS->enabled = FALSE;
		update_match(THIS, TRUE);
	}

	if (THIS == DBUS_observers)
		DBUS_observers = THIS->next;

	if (THIS->prev)
		THIS->prev->next = THIS->next;
	
	if (THIS->next)
		THIS->next->prev = THIS->prev;
	
	GB.FreeString(&THIS->object);
	GB.FreeString(&THIS->member);
	GB.FreeString(&THIS->interface);
	GB.FreeString(&THIS->destination);
	
END_METHOD

BEGIN_PROPERTY(DBusObserver_Enabled)

	if (READ_PROPERTY)
		GB.ReturnBoolean(THIS->enabled);
	else
	{
		bool e = VPROP(GB_BOOLEAN);
		if (e == THIS->enabled)
			return;
		THIS->enabled = e;
		update_match(THIS, FALSE);
	}

END_PROPERTY


BEGIN_PROPERTY(DBusObserver_Message)

	if (THIS->message)
		RETURN_SELF();
	else
		GB.ReturnNull();

END_PROPERTY


BEGIN_PROPERTY(DBusObserverMessage_Type)

	GB.ReturnInteger(dbus_message_get_type(THIS->message));

END_PROPERTY

BEGIN_PROPERTY(DBusObserverMessage_Serial)

	GB.ReturnInteger(dbus_message_get_serial(THIS->message));

END_PROPERTY

BEGIN_PROPERTY(DBusObserverMessage_Sender)

	GB.ReturnConstZeroString(dbus_message_get_sender(THIS->message));

END_PROPERTY

BEGIN_PROPERTY(DBusObserverMessage_Destination)

	GB.ReturnConstZeroString(dbus_message_get_destination(THIS->message));

END_PROPERTY

BEGIN_PROPERTY(DBusObserverMessage_Object)

	GB.ReturnConstZeroString(dbus_message_get_path(THIS->message));

END_PROPERTY

BEGIN_PROPERTY(DBusObserverMessage_Interface)

	GB.ReturnConstZeroString(dbus_message_get_interface(THIS->message));

END_PROPERTY

BEGIN_PROPERTY(DBusObserverMessage_Member)

	GB.ReturnConstZeroString(dbus_message_get_member(THIS->message));

END_PROPERTY

BEGIN_PROPERTY(DBusObserverMessage_Arguments)

	DBUS_retrieve_message_arguments(THIS->message);

END_PROPERTY

BEGIN_METHOD(DBusObserver_Reply, GB_STRING signature; GB_OBJECT args)

	if (THIS->message)
	{
		if (!DBUS_reply(THIS->connection, THIS->message, GB.ToZeroString(ARG(signature)), VARG(args)))
			THIS->reply = TRUE;
	}

END_METHOD

BEGIN_METHOD(DBusObserver_Error, GB_STRING type; GB_STRING error)

	if (THIS->message)
	{
		if (!DBUS_error(THIS->connection, THIS->message, MISSING(error) ? NULL : GB.ToZeroString(ARG(error)), MISSING(type) ? NULL : GB.ToZeroString(ARG(type))))
			THIS->reply = TRUE;
	}

END_METHOD

GB_DESC CDBusObserverMessageDesc[] =
{
  GB_DECLARE(".DBusObserverMessage", 0), GB_VIRTUAL_CLASS(),

	GB_PROPERTY_READ("Type", "i", DBusObserverMessage_Type),
	GB_PROPERTY_READ("Serial", "i", DBusObserverMessage_Serial),
	GB_PROPERTY_READ("Sender", "s", DBusObserverMessage_Sender),
	GB_PROPERTY_READ("Destination", "s", DBusObserverMessage_Destination),
	GB_PROPERTY_READ("Object", "s", DBusObserverMessage_Object),
	GB_PROPERTY_READ("Interface", "s", DBusObserverMessage_Interface),
	GB_PROPERTY_READ("Member", "s", DBusObserverMessage_Member),
	GB_PROPERTY_READ("Arguments", "Variant[]", DBusObserverMessage_Arguments),
	
  GB_END_DECLARE
};


GB_DESC CDBusObserverDesc[] =
{
  GB_DECLARE("DBusObserver", sizeof(CDBUSOBSERVER)),

	GB_METHOD("_new", NULL, DBusObserver_new, "(Connection)DBusConnection;(Type)i[(Object)s(Member)s(Interface)s(Destination)s]"),
	GB_METHOD("_free", NULL, DBusObserver_free, NULL),
	
	//GB_PROPERTY("Tag", "v", DBusObserver_Tag),
	GB_PROPERTY("Enabled", "b", DBusObserver_Enabled),
	GB_PROPERTY_READ("Message", ".DBusObserverMessage", DBusObserver_Message),
	GB_METHOD("Reply", NULL, DBusObserver_Reply, "[(Signature)s(Arguments)Array;]"),
	GB_METHOD("Error", NULL, DBusObserver_Error, "[(Error)s(Type)s]"),
	
	GB_EVENT("Message", NULL, NULL, &EVENT_MESSAGE),

  GB_END_DECLARE
};

