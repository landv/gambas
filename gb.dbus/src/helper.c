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
#include "helper.h"

typedef
	void (*RETRIEVE_CALLBACK)(GB_TYPE type, void *data, void *param);

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

static char *to_dbus_type(GB_TYPE type)
{
	switch(type)
	{
		case GB_T_BYTE: return DBUS_TYPE_BYTE_AS_STRING;
		case GB_T_BOOLEAN: return DBUS_TYPE_BOOLEAN_AS_STRING;
		case GB_T_SHORT: return DBUS_TYPE_INT16_AS_STRING;
		case GB_T_INTEGER: return DBUS_TYPE_INT32_AS_STRING;
		case GB_T_LONG: return DBUS_TYPE_INT64_AS_STRING;
		case GB_T_SINGLE: return DBUS_TYPE_DOUBLE_AS_STRING;
		case GB_T_FLOAT: return DBUS_TYPE_DOUBLE_AS_STRING;
		case GB_T_STRING: return DBUS_TYPE_STRING_AS_STRING;
		default: return NULL;
	}
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
				contents_signature = to_dbus_type(arg->type);
			
			if (contents_signature)
			{
				dbus_message_iter_open_container(iter, DBUS_TYPE_VARIANT, contents_signature, &citer);
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
	GB_VALUE value;
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
	
	dbus_message_iter_init_append(message, &iter);
	
	dbus_signature_iter_init(&siter, signature_in);
	for(n = 0; n < nparam; n++)
	{
		value.type = type;
		GB.ReadValue(&value, GB.Array.Get(arguments, n), type);
		if (append_arg(&iter, dbus_signature_iter_get_signature(&siter), &value))
			goto __RETURN;
		dbus_signature_iter_next(&siter);
	}

	dbus_error_init(&error);
	reply = dbus_connection_send_with_reply_and_block(connection, message, -1, &error);

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
				goto __RETURN;
		}
		while(dbus_message_iter_next(&iter));
		
		GB.ReturnObject(result);
		ret = FALSE;
	}
	
	dbus_message_unref(reply);
	
__RETURN:
	
	dbus_message_unref(message);
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

	if (dbus_error_is_set (&error))
	{
		GB.Error("&1: &2", error.name, error.message);
		goto __RETURN;
	}

	if (!reply)
		goto __RETURN;
	
	/*const char *sender;
	const char *destination;
	int message_type;

	message_type = dbus_message_get_type (reply);
	sender = dbus_message_get_sender (reply);
	destination = dbus_message_get_destination (reply);
	
	printf ("%s sender=%s -> dest=%s",
		type_to_name (message_type),
		sender ? sender : "(null sender)",
		destination ? destination : "(null destination)");
	
	switch (message_type)
	{
		case DBUS_MESSAGE_TYPE_METHOD_CALL:
		case DBUS_MESSAGE_TYPE_SIGNAL:
			printf (" serial=%u path=%s; interface=%s; member=%s\n",
										dbus_message_get_serial (message),
				dbus_message_get_path (message),
				dbus_message_get_interface (message),
				dbus_message_get_member (message));
			break;
				
		case DBUS_MESSAGE_TYPE_METHOD_RETURN:
			printf (" reply_serial=%u\n",
						dbus_message_get_reply_serial (message));
			break;

		case DBUS_MESSAGE_TYPE_ERROR:
			printf (" error_name=%s reply_serial=%u\n",
				dbus_message_get_error_name (message),
						dbus_message_get_reply_serial (message));
			break;

		default:
			printf ("\n");
			break;
	}*/

	dbus_message_iter_init(reply, &iter);
	type = dbus_message_iter_get_arg_type(&iter);
	if (type == DBUS_TYPE_STRING)
		dbus_message_iter_get_basic(&iter, &signature);
	
	dbus_message_unref (reply);

__RETURN:

	dbus_message_unref(message);
	return signature;
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
