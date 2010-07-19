/***************************************************************************

  helper.c

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

#define __HELPER_C

#include "c_dbusvariant.h"
#include "c_dbusobserver.h"
#include "dbus_print_message.h"
#include "helper.h"

typedef
	void (*RETRIEVE_CALLBACK)(GB_TYPE type, void *data, void *param);

static void handle_message(int fd, int type, DBusConnection *connection)
{
	//fprintf(stdout, "handle_message\n");
	do
	{
		dbus_connection_read_write_dispatch(connection, -1);
	}
	while (dbus_connection_get_dispatch_status(connection) == DBUS_DISPATCH_DATA_REMAINS);
}

static void check_message_now(DBusConnection *connection)
{
	if (dbus_connection_get_dispatch_status(connection) == DBUS_DISPATCH_DATA_REMAINS)
		handle_message(-1, 0, connection);
}

static void check_message(DBusConnection *connection)
{
	GB.Post((GB_POST_FUNC)check_message_now, (intptr_t)connection);
}


/***************************************************************************

Method signatures have the following syntax:

<ARG-1><ARG-2>...<ARG-n>

where <ARG-i> is the DBus signature of the i-th input argument. All 
arguments are in order.

The signature of one argument can be a simple type:

y         BYTE              Byte
b         BOOLEAN           Boolean
n         INT16             Short
q         UINT16            Short
i         INT32             Integer
u         UINT32            Integer
x         INT64             Long
t         UINT64            Long
d         DOUBLE            Float
s         STRING            String
o         OBJECT_PATH       String
g         SIGNATURE         String
v         VARIANT           Variant
	
Or a compound type (T is any datatype):
	
aT        ARRAY             Array
(TT...)   STRUCT            Variant[]
{TT}      DICT_ENTRY        Collection

***************************************************************************/
typedef
	struct {
		const char *name;
		const char *dbus;
	}
	CONV_TYPE;
	
CONV_TYPE _conv_type[] = 
{
	{ "DBusObject", "o" },
	{ "Collection", "{sv}" },
	{ "Boolean[]", "ab" },
	{ "Byte[]", "ay" },
	{ "Short[]", "an" },
	{ "Integer[]", "ai" },
	{ "Long[]", "ax" },
	{ "Single[]", "ad" },
	{ "Float[]", "ad" },
	{ "Date[]", "ad" },
	{ "Pointer[]", "ax" },
	{ "String[]", "as" },
	{ "Variant[]", "av" },
	{ "DBusObject[]", "ao" },
	{ "Collection[]", "a{sv}" },
	{ NULL }
};


const char *DBUS_to_dbus_type(GB_TYPE type)
{
	CONV_TYPE *p;
	
	switch(type)
	{
		case GB_T_BYTE: return DBUS_TYPE_BYTE_AS_STRING;
		case GB_T_BOOLEAN: return DBUS_TYPE_BOOLEAN_AS_STRING;
		case GB_T_SHORT: return DBUS_TYPE_INT16_AS_STRING;
		case GB_T_INTEGER: return DBUS_TYPE_INT32_AS_STRING;
		case GB_T_LONG: return DBUS_TYPE_INT64_AS_STRING;
		case GB_T_POINTER: return DBUS_TYPE_INT64_AS_STRING;
		case GB_T_SINGLE: return DBUS_TYPE_DOUBLE_AS_STRING;
		case GB_T_FLOAT: return DBUS_TYPE_DOUBLE_AS_STRING;
		case GB_T_STRING: return DBUS_TYPE_STRING_AS_STRING;
		default: break;
	}
	
	for (p = _conv_type; p->name; p++)
	{
		if (GB.FindClass(p->name) == type)
			return p->dbus;
	}
	
	return "v";
}

static char *array_from_dbus_type(const char *signature)
{
	static char type[DBUS_MAXIMUM_SIGNATURE_LENGTH + 1];
	DBusSignatureIter siter;
	
	dbus_signature_iter_init(&siter, signature);
	
	switch (dbus_signature_iter_get_current_type(&siter))
	{
		case DBUS_TYPE_BYTE: return "Byte[]";
		case DBUS_TYPE_BOOLEAN: return "Boolean[]";
		case DBUS_TYPE_INT16: case DBUS_TYPE_UINT16: return "Short[]";
		case DBUS_TYPE_INT32: case DBUS_TYPE_UINT32: return "Integer[]";
		case DBUS_TYPE_INT64: case DBUS_TYPE_UINT64: return "Long[]";
		case DBUS_TYPE_DOUBLE: return "Float[]";
		case DBUS_TYPE_STRING:
		case DBUS_TYPE_OBJECT_PATH:
		case DBUS_TYPE_SIGNATURE: return "String[]";
		case DBUS_TYPE_VARIANT: return "Variant[]";
		
		case DBUS_TYPE_DICT_ENTRY: 
			return "Collection[]";
		
		case DBUS_TYPE_ARRAY: 
		{
			DBusSignatureIter siter_contents;
			char *type_contents;
			
			dbus_signature_iter_recurse(&siter, &siter_contents);
			type_contents = array_from_dbus_type(dbus_signature_iter_get_signature(&siter_contents));
			if (type_contents != type)
				strcpy(type, type_contents);
			strcat(type, "[]");
			return type;
		}
			
		default: 
			return "Variant[]";
	}
}

static GB_TYPE from_dbus_type(const char *signature)
{
	DBusSignatureIter siter;

	dbus_signature_iter_init(&siter, signature);
	
	switch (dbus_signature_iter_get_current_type(&siter))
	{
		case DBUS_TYPE_BYTE: return GB_T_BYTE;
		case DBUS_TYPE_BOOLEAN: return GB_T_BOOLEAN;
		case DBUS_TYPE_INT16: case DBUS_TYPE_UINT16: return GB_T_SHORT;
		case DBUS_TYPE_INT32: case DBUS_TYPE_UINT32: return GB_T_INTEGER; 
		case DBUS_TYPE_INT64: case DBUS_TYPE_UINT64: return GB_T_LONG;
		case DBUS_TYPE_DOUBLE: return GB_T_FLOAT; 
		case DBUS_TYPE_STRING:
		case DBUS_TYPE_OBJECT_PATH:
		case DBUS_TYPE_SIGNATURE: return GB_T_STRING; 
		
		case DBUS_TYPE_DICT_ENTRY: return GB.FindClass("Collection");
		
		case DBUS_TYPE_ARRAY:
		{
			DBusSignatureIter siter_contents;
			char *type;
			
			dbus_signature_iter_recurse(&siter, &siter_contents);
			type = array_from_dbus_type(dbus_signature_iter_get_signature(&siter_contents));
			return GB.FindClass(type);
		}

		case DBUS_TYPE_STRUCT:
		{
			DBusSignatureIter siter_contents;
			GB_TYPE type, type2;

			dbus_signature_iter_recurse(&siter, &siter_contents);
			type = GB.FindClass(array_from_dbus_type(dbus_signature_iter_get_signature(&siter_contents)));
	
			while (dbus_signature_iter_next(&siter_contents))
			{
				type2 = GB.FindClass(array_from_dbus_type(dbus_signature_iter_get_signature(&siter_contents)));
				if (type != type2)
					return GB.FindClass("Variant[]");
			}
	
			return type;
		}

		case DBUS_TYPE_VARIANT:
		default: return GB_T_VARIANT; 
	}
}

static bool append_arg(DBusMessageIter *iter, const char *signature, GB_VALUE *arg)
{
	DBusSignatureIter siter;
	int type;
	GB_TYPE gtype;
	
	if (arg->type == GB_T_VARIANT)
		GB.Conv(arg, arg->_variant.value.type);
		
	dbus_signature_iter_init(&siter, signature);
	type = dbus_signature_iter_get_current_type(&siter);
	gtype = from_dbus_type(dbus_signature_iter_get_signature(&siter));
	
	if (gtype == GB_T_NULL)
	{
		goto __UNSUPPORTED;
	}
	else if (gtype != GB_T_VARIANT)
	{
		if (GB.Conv(arg, gtype))
			return TRUE;
	}
	
	GB.ReleaseValue(arg);

	switch(type)
	{
		case DBUS_TYPE_BYTE:
		{
			unsigned char val;
			val = arg->_integer.value;
			dbus_message_iter_append_basic(iter, type, &val);
			break;
		}
		
		case DBUS_TYPE_BOOLEAN:
		{
			dbus_bool_t val;
			val = arg->_boolean.value ? 1 : 0;
			dbus_message_iter_append_basic(iter, type, &val);
			break;
		}
			
		case DBUS_TYPE_INT16:
		case DBUS_TYPE_UINT16:
		{
			dbus_int16_t val;
			val = arg->_integer.value;
			dbus_message_iter_append_basic(iter, type, &val);
			break;
		}
			
		case DBUS_TYPE_INT32:
		case DBUS_TYPE_UINT32:
		{
			dbus_int32_t val;
			val = arg->_integer.value;
			dbus_message_iter_append_basic(iter, type, &val);
			break;
		}
			
		case DBUS_TYPE_INT64:
		case DBUS_TYPE_UINT64:
		{
			dbus_int64_t val;
			val = arg->_long.value;
			dbus_message_iter_append_basic(iter, type, &val);
			break;
		}
		
		case DBUS_TYPE_DOUBLE:
		{
			double val;
			val = arg->_float.value;
			dbus_message_iter_append_basic(iter, type, &val);
			break;
		}

		case DBUS_TYPE_STRING:
		case DBUS_TYPE_OBJECT_PATH:
		case DBUS_TYPE_SIGNATURE:
		{
			const char *str = GB.ToZeroString((GB_STRING *)arg);
			dbus_message_iter_append_basic(iter, type, &str);
			break;
		}
		
		case DBUS_TYPE_ARRAY:
		{
			GB_ARRAY array;
			DBusMessageIter citer;
			int i;
			GB_VALUE value;
			const char *contents_signature = &signature[1];
			
			array = (GB_ARRAY)(arg->_object.value);
			
			dbus_message_iter_open_container(iter, DBUS_TYPE_VARIANT, contents_signature, &citer);
			
			value.type = GB.Array.Type(array);
			for (i = 0; i < GB.Array.Count(array); i++)
			{
				GB.ReadValue(&value, GB.Array.Get(array, i), value.type);
				GB.BorrowValue(&value);
				if (append_arg(&citer, contents_signature, &value))
					return TRUE;
			}
			
			dbus_message_iter_close_container(iter, &citer);
			break;
		}
		
		case DBUS_TYPE_STRUCT:
		{
			GB_ARRAY array;
			DBusMessageIter citer;
			DBusSignatureIter siter_contents;
			int i;
			GB_VALUE value;
			
			array = (GB_ARRAY)(arg->_object.value);
			
			dbus_message_iter_open_container(iter, DBUS_TYPE_STRUCT, NULL, &citer);
			dbus_signature_iter_recurse(&siter, &siter_contents);
			
			value.type = GB.Array.Type(array);
			for (i = 0; i < GB.Array.Count(array); i++)
			{
				GB.ReadValue(&value, GB.Array.Get(array, i), value.type);
				GB.BorrowValue(&value);
				if (append_arg(&citer, dbus_signature_iter_get_signature(&siter_contents), &value))
					return TRUE;
				dbus_signature_iter_next(&siter_contents);
			}
			
			dbus_message_iter_close_container(iter, &citer);
			break;
		}

		case DBUS_TYPE_VARIANT:
		{
			DBusMessageIter citer;
			const char *contents_signature;
			GB_VALUE rarg;
			
			if (arg->type == CLASS_DBusVariant)
			{
				CDBUSVARIANT *dbusvariant = (CDBUSVARIANT *)arg->_object.value;
				
				rarg.type = GB_T_VARIANT;
				rarg._variant.value = dbusvariant->value;
				arg = &rarg;
				
				contents_signature = dbusvariant->signature;
			}
			else
				contents_signature = DBUS_to_dbus_type(arg->type);
			
			if (contents_signature)
			{
				dbus_message_iter_open_container(iter, DBUS_TYPE_VARIANT, contents_signature, &citer);
				GB.BorrowValue(arg);
				if (append_arg(&citer, contents_signature, arg))
					return TRUE;
				dbus_message_iter_close_container(iter, &citer);
				break;
			}
		}
		
		default:
			goto __UNSUPPORTED;
	}
	
	return FALSE;
	
__UNSUPPORTED:
	GB.Error("Unsupported datatype");
	return TRUE;

}

static void return_value_cb(GB_TYPE type, void *value, void *param)
{
	GB.ReturnPtr(type, value);
}

static void add_value_cb(GB_TYPE type, void *value, void *param)
{
	GB_ARRAY array = (GB_ARRAY)param;
	GB_VALUE val;
	
	if (type == GB_T_STRING)
		type = GB_T_CSTRING;
	
	GB.ReadValue(&val, value, type);
	GB.Conv(&val, GB.Array.Type(array));
	GB.Store(GB.Array.Type(array), &val, GB.Array.Add(array));
}

static bool retrieve_arg(DBusMessageIter *iter, RETRIEVE_CALLBACK cb, void *param)
{
	char *signature = dbus_message_iter_get_signature(iter);
	GB_TYPE gtype = from_dbus_type(signature);
	int type = dbus_message_iter_get_arg_type(iter);
	
	if (dbus_type_is_basic(type))
	{
		char val[16];
		dbus_message_iter_get_basic (iter, (void *)val);
		(*cb)(gtype, val, param);
		return FALSE;
	}
	
	switch(type)
	{
		case DBUS_TYPE_VARIANT:
		{
			DBusMessageIter iter_contents;

			dbus_message_iter_recurse(iter, &iter_contents);
			return retrieve_arg(&iter_contents, cb, param);
		}
			
		case DBUS_TYPE_ARRAY:
		case DBUS_TYPE_STRUCT:
		{
			GB_ARRAY array;
			const char *signature_contents;
			DBusMessageIter iter_contents;

			dbus_message_iter_recurse(iter, &iter_contents);
			signature_contents = dbus_message_iter_get_signature(&iter_contents);
			
			GB.Array.New(POINTER(&array), from_dbus_type(signature_contents), 0);
			
			while (dbus_message_iter_get_arg_type(&iter_contents) != DBUS_TYPE_INVALID)
			{
				if (retrieve_arg(&iter_contents, add_value_cb, array))
					return TRUE;
				dbus_message_iter_next(&iter_contents);
			}
			
			(*cb)(gtype, &array, param);
			return FALSE;
		}
			
		case DBUS_TYPE_DICT_ENTRY:
		default:
			GB.Error("Unsupported DBus datatype");
			return TRUE;
	}
}

static bool define_arguments(DBusMessage *message, const char *signature, GB_ARRAY arguments)
{
	int nparam;
	int n;
	GB_TYPE type;
	GB_VALUE value;
	DBusMessageIter iter;
	DBusSignatureIter siter;
	
	if (arguments)
	{
		nparam = GB.Array.Count(arguments);
		type = GB.Array.Type(arguments);
	}
	else
	{
		nparam = 0;
		type = GB_T_NULL;
	}
	
	if (signature && *signature)
	{
		dbus_message_iter_init_append(message, &iter);
		
		dbus_signature_iter_init(&siter, signature);
		
		for (n = 0; n < nparam; n++)
		{
			value.type = type;
			GB.ReadValue(&value, GB.Array.Get(arguments, n), type);
			GB.BorrowValue(&value);
			if (append_arg(&iter, dbus_signature_iter_get_signature(&siter), &value))
				return TRUE;
			if (!dbus_signature_iter_next(&siter))
			{
				if (n < (nparam - 1))
				{
					GB.Error("Too many arguments");
					return TRUE;
				}
				return FALSE;
			}
		}
	}
	
	GB.Error("Not enough arguments");
	return TRUE;
}

bool DBUS_call_method(DBusConnection *connection, const char *application, const char *path, const char *interface, const char *method, 
											const char *signature_in, const char *signature_out, GB_ARRAY arguments)
{
	DBusMessage *message;
	int n;
	DBusMessageIter iter;
	DBusMessage *reply;
	DBusError error;
	DBusSignatureIter siter;
	bool ret;
	GB_TYPE type;
	int nparam;
	
	message = dbus_message_new_method_call(application, path, interface, method);
	if (!message)
	{
		GB.Error("Couldn't allocate D-Bus message");
		return TRUE;
	}
	
	ret = TRUE;
	
	dbus_message_set_auto_start (message, TRUE);

	if (define_arguments(message, signature_in, arguments))
		goto __RETURN;
	
	dbus_error_init(&error);
	reply = dbus_connection_send_with_reply_and_block(connection, message, -1, &error);
	check_message(connection);

	if (dbus_error_is_set(&error))
	{
		GB.Error("&1: &2", error.name, error.message);
		goto __RETURN;
	}

	if (!reply)
		goto __RETURN;
	
	dbus_message_iter_init(reply, &iter);
	
	if (dbus_message_iter_get_arg_type(&iter) == DBUS_TYPE_INVALID)
	{
		GB.ReturnNull();
		ret = FALSE;
	}
	else if (!dbus_message_iter_has_next(&iter))
	{
		ret = retrieve_arg(&iter, return_value_cb, NULL);
	}
	else
	{
		GB_ARRAY result;
		GB.Array.New(POINTER(&result), GB_T_VARIANT, 0);
		
		do
		{
			if (retrieve_arg(&iter, add_value_cb, result))
			{
				GB.Unref(&result);
				goto __RETURN;
			}
		}
		while (dbus_message_iter_next(&iter));
		
		GB.ReturnObject(result);
		ret = FALSE;
	}
	
	dbus_message_unref(reply);
	
__RETURN:
	
	dbus_message_unref(message);
	return ret;
}

bool DBUS_retrieve_message_arguments(DBusMessage *message)
{
	DBusMessageIter iter;

	dbus_message_iter_init(message, &iter);
	
	if (dbus_message_iter_get_arg_type(&iter) == DBUS_TYPE_INVALID)
	{
		GB.ReturnNull();
		return FALSE;
	}
	else
	{
		GB_ARRAY result;
		GB.Array.New(POINTER(&result), GB_T_VARIANT, 0);
		
		do
		{
			if (retrieve_arg(&iter, add_value_cb, result))
			{
				GB.Unref(&result);
				return TRUE;
			}
		}
		while(dbus_message_iter_next(&iter));
		
		GB.ReturnObject(result);
		return FALSE;
	}
}

bool DBUS_reply(DBusConnection *connection, DBusMessage *message, const char *signature, GB_ARRAY arguments)
{
	bool ret = TRUE;
	DBusMessage *reply;
	DBusError error;
	dbus_uint32_t serial = 0;

	reply = dbus_message_new_method_return(message);
	
	if (define_arguments(reply, signature, arguments))
		goto __RETURN;
	
	if (!dbus_connection_send(connection, reply, &serial)) 
	{
		GB.Error("Cannot send reply");
		goto __RETURN;
	}
	
	dbus_connection_flush(connection);

	check_message(connection);
	
	ret = FALSE;
	
__RETURN:
	
	dbus_message_unref(reply);
	return ret;
}

bool DBUS_error(DBusConnection *connection, DBusMessage *message, const char *type, const char *error)
{
	bool ret = TRUE;
	DBusMessage *reply;
	dbus_uint32_t serial = 0;
	char *full_type;

	if (!error) error = "";
	
	if (!type)
		full_type = DBUS_ERROR_FAILED;
	else
	{
		full_type = GB.NewZeroString("org.freedesktop.org.DBus.Error.");
		GB.AddString(&full_type, type, 0);
	}
	
	reply = dbus_message_new_error(message, full_type, error);
	
	if (!dbus_connection_send(connection, reply, &serial)) 
	{
		GB.Error("Cannot send error");
		goto __RETURN;
	}
	
	dbus_connection_flush(connection);

	check_message(connection);
	
	ret = FALSE;
	
__RETURN:
	
	dbus_message_unref(reply);
	return ret;
}

/*static const char *type_to_name (int message_type)
{
	switch (message_type)
		{
		case DBUS_MESSAGE_TYPE_SIGNAL:
			return "signal";
		case DBUS_MESSAGE_TYPE_METHOD_CALL:
			return "method call";
		case DBUS_MESSAGE_TYPE_METHOD_RETURN:
			return "method return";
		case DBUS_MESSAGE_TYPE_ERROR:
			return "error";
		default:
			return "(unknown message type)";
		}
}*/

char *DBUS_introspect(DBusConnection *connection, const char *application, const char *path)
{
	DBusMessage *message;
	DBusMessage *reply;
	DBusError error;
	DBusMessageIter iter;
	int type;
	char *signature = NULL;
	
	message = dbus_message_new_method_call(application, path, "org.freedesktop.DBus.Introspectable", "Introspect");
	if (!message)
	{
		GB.Error("Couldn't allocate D-Bus message");
		return NULL;
	}
	
	dbus_message_set_auto_start(message, TRUE);

	dbus_error_init(&error);
	reply = dbus_connection_send_with_reply_and_block (connection, message, -1, &error);
	check_message(connection);

	if (dbus_error_is_set(&error))
	{
		GB.Error("&1: &2", error.name, error.message);
		goto __RETURN;
	}

	if (!reply)
		goto __RETURN;
	
	dbus_message_iter_init(reply, &iter);
	type = dbus_message_iter_get_arg_type(&iter);
	if (type == DBUS_TYPE_STRING)
		dbus_message_iter_get_basic(&iter, &signature);
	
	dbus_message_unref (reply);

__RETURN:

	dbus_message_unref(message);
	return signature;
}

static bool check_filter(char *rule, const char *value)
{
	if (!rule || !*rule || (rule[0] == '*' && rule[1] == 0))
		return FALSE;
	
	return (strcasecmp(rule, value) != 0);
}

static DBusHandlerResult filter_func(DBusConnection *connection, DBusMessage *message, void *user_data)
{
	CDBUSOBSERVER *obs;
	bool found = FALSE;
	
	for (obs = DBUS_observers; obs; obs = obs->next)
	{
		if (obs->type != dbus_message_get_type(message))
			continue;
		
		// Beware: "" means "only me" in DBusObserver, but everthing there!
		if (check_filter(obs->destination, dbus_message_get_destination(message)))
			continue;
		if (check_filter(obs->object, dbus_message_get_path(message)))
			continue;
		if (check_filter(obs->member, dbus_message_get_member(message)))
			continue;
		if (check_filter(obs->interface, dbus_message_get_interface(message)))
			continue;
		
		found = TRUE;
		obs->message = message;
		obs->reply = FALSE;
		DBUS_raise_observer(obs);
		obs->message = NULL;
		if (obs->reply)
			return DBUS_HANDLER_RESULT_HANDLED;
	}
	
  if (!found)
	{
		fprintf(stderr, "warning: unhandled message: ");
		print_message(message, FALSE);
	}
	
  return DBUS_HANDLER_RESULT_HANDLED;
}

bool DBUS_register(DBusConnection *connection, const char *name, bool unique)
{
	DBusError error;
	int ret, socket;

	if (!dbus_connection_get_socket(connection, &socket) || !dbus_connection_add_filter(connection, filter_func, NULL, NULL))
	{
		GB.Error("Unable to watch the DBus connection");
		return TRUE;
	}
		
	dbus_error_init(&error);
	
	ret = dbus_bus_request_name(connection, name, unique ? DBUS_NAME_FLAG_DO_NOT_QUEUE : 0, &error);

	if (dbus_error_is_set(&error))
	{
		GB.Error("Unable to register application");
		return TRUE;
	}

	if (unique && ret != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER)
		return TRUE;
	
	/*if (add_match(connection, "type='signal'")) goto ERROR_MATCH;
	if (add_match(connection, "type='method_call'")) goto ERROR_MATCH;
	//if (add_match(connection, "type='method_return'") goto ERROR_MATCH;
	if (add_match(connection, "type='error'")) goto ERROR_MATCH;*/

	GB.Watch(socket, GB_WATCH_READ, (void *)handle_message, (intptr_t)connection);
	
	check_message(connection);
		
	return FALSE;
	
}

/***************************************************************************

Validation routines taken directly from the D-Bus source code.

***************************************************************************/


/**
* Determine wether the given character is valid as the first character
* in a name.
*/
#define VALID_INITIAL_NAME_CHARACTER(c)         \
	( ((c) >= 'A' && (c) <= 'Z') ||               \
		((c) >= 'a' && (c) <= 'z') ||               \
		((c) == '_') )

/**
* Determine wether the given character is valid as a second or later
* character in a name
*/
#define VALID_NAME_CHARACTER(c)                 \
	( ((c) >= '0' && (c) <= '9') ||               \
		((c) >= 'A' && (c) <= 'Z') ||               \
		((c) >= 'a' && (c) <= 'z') ||               \
		((c) == '_') )

bool DBUS_validate_path(const char *path, int len)
{
	const unsigned char *s;
	const unsigned char *end;
	const unsigned char *last_slash;

	if (len <= 0)
		len = strlen(path);
	
	s = (const unsigned char *)path;
	end = s + (uint)len;

	if (*s != '/')
		return TRUE;
	last_slash = s;
	++s;

	while (s != end)
	{
		if (*s == '/')
		{
			if ((s - last_slash) < 2)
				return TRUE; /* no empty path components allowed */

			last_slash = s;
		}
		else
		{
			if (!VALID_NAME_CHARACTER (*s))
				return TRUE;
		}

		++s;
	}

	if ((end - last_slash) < 2 && len > 1)
		return TRUE; /* trailing slash not allowed unless the string is "/" */

	return FALSE;
}

bool DBUS_validate_interface (const char *interface, int len)
{
	const unsigned char *s;
	const unsigned char *end;
	const unsigned char *last_dot;

	if (!interface)
		return FALSE;
	
	if (len <= 0)
		len = strlen(interface);

	if (len > DBUS_MAXIMUM_NAME_LENGTH || len == 0)
		return TRUE;

	last_dot = NULL;
	s = (const unsigned char *)interface;
	end = s + (uint)len;

	/* check special cases of first char so it doesn't have to be done
		* in the loop. Note we know len > 0
		*/
	if (*s == '.') /* disallow starting with a . */
		return TRUE;
	else if (!VALID_INITIAL_NAME_CHARACTER (*s))
		return TRUE;
	else
		++s;

	while (s != end)
	{
		if (*s == '.')
		{
			if ((s + 1) == end)
				return TRUE;
			else if (!VALID_INITIAL_NAME_CHARACTER (*(s + 1)))
				return TRUE;
			last_dot = s;
			++s; /* we just validated the next char, so skip two */
		}
		else if (!VALID_NAME_CHARACTER (*s))
		{
			return TRUE;
		}

		++s;
	}

	if (last_dot == NULL)
		return TRUE;

	return FALSE;
}

bool DBUS_validate_method(const char *method, int len)
{
	const unsigned char *s;
	const unsigned char *end;

	if (len <= 0)
		len = strlen(method);

	if (len > DBUS_MAXIMUM_NAME_LENGTH || len == 0)
		return TRUE;

	s = (const unsigned char *)method;
	end = s + (uint)len;

	/* check special cases of first char so it doesn't have to be done
		* in the loop. Note we know len > 0
		*/

	if (!VALID_INITIAL_NAME_CHARACTER (*s))
		return TRUE;
	else
		++s;

	while (s != end)
	{
		if (!VALID_NAME_CHARACTER (*s))
		{
			return TRUE;
		}

		++s;
	}

	return FALSE;
}

