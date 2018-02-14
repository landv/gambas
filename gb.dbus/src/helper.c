/***************************************************************************

  helper.c

  (c) 2000-2017 Benoît Minisini <g4mba5@gmail.com>

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

#define __HELPER_C

#include "c_dbusvariant.h"
#include "c_dbusobserver.h"
#include "dbus_print_message.h"
#include "helper.h"

//#define DEBUG_ARG

typedef
	void (*RETRIEVE_CALLBACK)(GB_TYPE type, void *data, void *param);

typedef
	struct {
		GB_COLLECTION col;
		char *key;
	}
	COLLECTION_ADD;

bool DBUS_Debug = FALSE;

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
	GB.Post((GB_CALLBACK)check_message_now, (intptr_t)connection);
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
	{ "Collection", "a{sv}" },
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
	{ NULL }
};


static const char *to_dbus_type(GB_VALUE *arg)
{
	CONV_TYPE *p;
	GB_TYPE type, atype;
	int a;
	const char *dtype;
	char *result;
	
	switch(arg->type)
	{
		case GB_T_BOOLEAN: return DBUS_TYPE_BOOLEAN_AS_STRING;
		case GB_T_BYTE: return DBUS_TYPE_BYTE_AS_STRING;
		case GB_T_SHORT: return DBUS_TYPE_INT16_AS_STRING;
		case GB_T_INTEGER: return DBUS_TYPE_INT32_AS_STRING;
		case GB_T_LONG: return DBUS_TYPE_INT64_AS_STRING;
		case GB_T_POINTER: return DBUS_TYPE_INT64_AS_STRING;
		case GB_T_SINGLE: return DBUS_TYPE_DOUBLE_AS_STRING;
		case GB_T_FLOAT: return DBUS_TYPE_DOUBLE_AS_STRING;
		case GB_T_STRING: return DBUS_TYPE_STRING_AS_STRING;
		default: break;
	}

	if (arg->type >= GB_T_OBJECT)
	{
		//fprintf(stderr, "%s\n", GB.GetClassName((void *)arg->type));
		
		for (p = _conv_type; p->name; p++)
		{
			if (GB.FindClass(p->name) == arg->type)
				return p->dbus;
		}
				
		if (GB.Is(arg->_object.value, GB.FindClass("Array")))
		{
			type = arg->type;
			a = 0;
			
			for(;;)
			{
				atype = GB.GetArrayType(type);
				if (atype <= GB_T_OBJECT)
					break;
				type = atype;
				a++;
			}
			
			dtype = "v";
			for (p = _conv_type; p->name; p++)
			{
				if (GB.FindClass(p->name) == type)
				{
					dtype = p->dbus;
					break;
				}
			}
			
			result = NULL;
			while (a--)
				result = GB.AddChar(result, 'a');
			result = GB.AddString(result, dtype, strlen(dtype));
			GB.FreeStringLater(result);
			
			return result;
		}
		
		if (GB.Is(arg->_object.value, CLASS_DBusVariant))
			return "v";

		if (GB.Is(arg->_object.value, CLASS_DBusNull))
			return "s";

		return NULL;
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
			if (signature[1] == 's')
				return "Collection";
			else
				return NULL;
		
		
		case DBUS_TYPE_ARRAY: 
		{
			DBusSignatureIter siter_contents;
			char *type_contents;
			char *sign_contents;
			
			dbus_signature_iter_recurse(&siter, &siter_contents);
			sign_contents = dbus_signature_iter_get_signature(&siter_contents);
			type_contents = array_from_dbus_type(sign_contents);
			dbus_free(sign_contents);
			if (!type_contents)
				return NULL;
			
			if (type_contents != type)
				strcpy(type, type_contents);
			GB.GetArrayClass(GB.FindClass(type));
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
		
		case DBUS_TYPE_ARRAY:
		{
			DBusSignatureIter siter_contents;
			char *type;
			char *sign_contents;
			
			dbus_signature_iter_recurse(&siter, &siter_contents);
			sign_contents = dbus_signature_iter_get_signature(&siter_contents);
			type = array_from_dbus_type(sign_contents);
			dbus_free(sign_contents);
			if (type)
				return GB.FindClass(type);
			else
				return GB_T_VARIANT;
		}

		case DBUS_TYPE_STRUCT:
		{
			DBusSignatureIter siter_contents;
			char *atype;
			GB_TYPE type, type2;
			char *sign_contents;

			dbus_signature_iter_recurse(&siter, &siter_contents);
			sign_contents = dbus_signature_iter_get_signature(&siter_contents);
			atype = array_from_dbus_type(sign_contents);
			dbus_free(sign_contents);
			if (atype)
				type = GB.FindClass(atype);
			else
				return GB.FindClass("Variant[]");
	
			while (dbus_signature_iter_next(&siter_contents))
			{
				sign_contents = dbus_signature_iter_get_signature(&siter_contents);
				atype = array_from_dbus_type(sign_contents);
				dbus_free(sign_contents);
				if (atype)
					type2 = GB.FindClass(atype);
				else
					return GB.FindClass("Variant[]");
				
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
	char *sign;
	GB_VALUE rarg;
	GB_VALUE targ;
	
	if (arg->type == GB_T_VARIANT)
		GB.Conv(arg, arg->_variant.value.type);
		
	dbus_signature_iter_init(&siter, signature);
	type = dbus_signature_iter_get_current_type(&siter);
	
	sign = dbus_signature_iter_get_signature(&siter);
	gtype = from_dbus_type(sign);
	dbus_free(sign);
	
	if (gtype == GB_T_NULL)
	{
		GB.Error("Unsupported datatype for signature '&1'", sign);
		goto __UNSUPPORTED;
	}
	else if (gtype != GB_T_VARIANT)
	{
		// The contents of arg must not be modified, because it may have to be freed later by the caller
		
		if (arg->type >= GB_T_OBJECT)
		{
			if (GB.Is(arg->_object.value, CLASS_DBusNull))
			{
				targ.type = GB_T_NULL;
				GB.ReleaseValue(arg);
				arg = &targ;
			}
			else if (GB.Is(arg->_object.value, CLASS_DBusObject) && type == DBUS_TYPE_OBJECT_PATH)
			{
				GB_VALUE *val = GB.GetProperty(arg->_object.value, "_Path");
	
				if (val && (val->type == GB_T_STRING || val->type != GB_T_CSTRING))
				{
					targ = *val;
					targ.type = GB_T_CSTRING;
					// The value returned by GB.GetProperty() is not referenced, so it must not be freed.
					GB.BorrowValue(&targ);
					GB.ReleaseValue(arg);
					arg = &targ;
				}
			}
			else if (GB.Is(arg->_object.value, CLASS_DBusVariant))
			{
				CDBUSVARIANT *dbusvariant = (CDBUSVARIANT *)arg->_object.value;
				
				targ.type = GB_T_VARIANT;
				targ._variant.value = dbusvariant->value;
				GB.BorrowValue(&targ);
				GB.ReleaseValue(arg);
				arg = &targ;
			}
		}
			
		if (GB.Conv(arg, gtype))
		{
			//BREAKPOINT();
			GB.ReleaseValue(arg);
			GB.Error("Type mismatch");
			return TRUE;
		}
	}
	
	switch(type)
	{
		case DBUS_TYPE_BYTE:
		{
			unsigned char val;
			val = arg->_integer.value;
			dbus_message_iter_append_basic(iter, type, &val);
#ifdef DEBUG_ARG
			fprintf(stderr, "%d ", (int)val);
#endif
			break;
		}
		
		case DBUS_TYPE_BOOLEAN:
		{
			dbus_bool_t val;
			val = arg->_boolean.value ? 1 : 0;
			dbus_message_iter_append_basic(iter, type, &val);
#ifdef DEBUG_ARG
			fprintf(stderr, "%s ", val ? "true" : "false");
#endif
			break;
		}
			
		case DBUS_TYPE_INT16:
		case DBUS_TYPE_UINT16:
		{
			dbus_int16_t val;
			val = arg->_integer.value;
			dbus_message_iter_append_basic(iter, type, &val);
#ifdef DEBUG_ARG
			fprintf(stderr, "%d ", (int)val);
#endif
			break;
		}
			
		case DBUS_TYPE_INT32:
		case DBUS_TYPE_UINT32:
		{
			dbus_int32_t val;
			val = arg->_integer.value;
			dbus_message_iter_append_basic(iter, type, &val);
#ifdef DEBUG_ARG
			fprintf(stderr, "%d ", val);
#endif
			break;
		}
			
		case DBUS_TYPE_INT64:
		case DBUS_TYPE_UINT64:
		{
			dbus_int64_t val;
			val = arg->_long.value;
			dbus_message_iter_append_basic(iter, type, &val);
#ifdef DEBUG_ARG
			fprintf(stderr, "%" PRId64 " ", val);
#endif
			break;
		}
		
		case DBUS_TYPE_DOUBLE:
		{
			double val;
			val = arg->_float.value;
			dbus_message_iter_append_basic(iter, type, &val);
#ifdef DEBUG_ARG
			fprintf(stderr, "%g ", val);
#endif
			break;
		}

		case DBUS_TYPE_STRING:
		case DBUS_TYPE_OBJECT_PATH:
		case DBUS_TYPE_SIGNATURE:
		{
			const char *str;
			if (arg->type == GB_T_NULL)
				str = "";
			else
				str = GB.ToZeroString((GB_STRING *)arg);
			
			if (type == DBUS_TYPE_OBJECT_PATH && DBUS_validate_path(str, -1))
			{
				GB.Error("Invalid object path: &1", str);
				goto __ERROR;
			}
			
			dbus_message_iter_append_basic(iter, type, &str);
#ifdef DEBUG_ARG
			fprintf(stderr, "\"%s\" ", str);
#endif
			break;
		}
		
		case DBUS_TYPE_ARRAY:
		{
			DBusMessageIter citer;
			DBusMessageIter dict_entry_iter;
			int i;
			GB_VALUE value;
			const char *contents_signature = &signature[1];
			
			dbus_message_iter_open_container(iter, DBUS_TYPE_ARRAY, contents_signature, &citer);
			
			if (contents_signature[0] == '{' && contents_signature[1] == 's')
			{
				GB_COLLECTION col = (GB_COLLECTION)(arg->_object.value);
				char *key;
				int len;
				
#ifdef DEBUG_ARG
			fprintf(stderr, "{ ");
#endif
				if (col)
				{
					GB_COLLECTION_ITER iter;
					
					GB.Collection.Enum(col, &iter, NULL, NULL, NULL);
					for(;;)
					{
						if (GB.Collection.Enum(col, &iter, (GB_VARIANT *)&value, &key, &len))
							break;
						
						key = GB.TempString(key, len);
						dbus_message_iter_open_container(&citer, DBUS_TYPE_DICT_ENTRY, NULL, &dict_entry_iter);
						dbus_message_iter_append_basic(&dict_entry_iter, DBUS_TYPE_STRING, &key);
#ifdef DEBUG_ARG
			fprintf(stderr, "%s: ", key);
#endif
						
						GB.BorrowValue(&value);
						if (append_arg(&dict_entry_iter, &contents_signature[2], &value))
							goto __ERROR;
						
						dbus_message_iter_close_container(&citer, &dict_entry_iter);
					}
				}
#ifdef DEBUG_ARG
			fprintf(stderr, "} ");
#endif
			}
			else
			{
				GB_ARRAY array = (GB_ARRAY)(arg->_object.value);
				
#ifdef DEBUG_ARG
			fprintf(stderr, "[ ");
#endif
				if (array)
				{
					for (i = 0; i < GB.Array.Count(array); i++)
					{
						value.type = GB.Array.Type(array);
						GB.ReadValue(&value, GB.Array.Get(array, i), value.type);
						GB.BorrowValue(&value);
						if (append_arg(&citer, contents_signature, &value))
							goto __ERROR;
					}
				}
			}
			
			dbus_message_iter_close_container(iter, &citer);
#ifdef DEBUG_ARG
			fprintf(stderr, "] ");
#endif
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
#ifdef DEBUG_ARG
			fprintf(stderr, "( ");
#endif
			dbus_signature_iter_recurse(&siter, &siter_contents);
			
			if (array)
			{
				for (i = 0; i < GB.Array.Count(array); i++)
				{
					value.type = GB.Array.Type(array);
					GB.ReadValue(&value, GB.Array.Get(array, i), value.type);
					GB.BorrowValue(&value);
					sign = dbus_signature_iter_get_signature(&siter_contents);
					if (append_arg(&citer, sign, &value))
						goto __ERROR_SIGN;
					dbus_free(sign);
					dbus_signature_iter_next(&siter_contents);
				}
			}
			
			dbus_message_iter_close_container(iter, &citer);
#ifdef DEBUG_ARG
			fprintf(stderr, ") ");
#endif
			break;
		}

		case DBUS_TYPE_VARIANT:
		{
			DBusMessageIter citer;
			const char *contents_signature;
			
#ifdef DEBUG_ARG
			fprintf(stderr, "< ");
#endif
			if (arg->type >= GB_T_OBJECT && GB.Is(arg->_object.value, CLASS_DBusVariant))
			{
				CDBUSVARIANT *dbusvariant = (CDBUSVARIANT *)arg->_object.value;
				
				rarg.type = GB_T_VARIANT;
				rarg._variant.value = dbusvariant->value;
				
				contents_signature = CDBUSVARIANT_get_signature(dbusvariant);
				//fprintf(stderr, "#1: %s\n", contents_signature);
				GB.BorrowValue(&rarg);
				dbus_message_iter_open_container(iter, DBUS_TYPE_VARIANT, contents_signature, &citer);
				if (append_arg(&citer, contents_signature, &rarg))
					goto __ERROR;
				dbus_message_iter_close_container(iter, &citer);
			}
			else
			{
				contents_signature = to_dbus_type(arg);
				//fprintf(stderr, "#3: %s\n", contents_signature);
				if (!contents_signature)
				{
					GB.Error("Unsupported datatype in variant value");
					goto __UNSUPPORTED;
				}
				
				dbus_message_iter_open_container(iter, DBUS_TYPE_VARIANT, contents_signature, &citer);
				GB.BorrowValue(arg);
				if (append_arg(&citer, contents_signature, arg))
					goto __ERROR;
				dbus_message_iter_close_container(iter, &citer);
			}
#ifdef DEBUG_ARG
			fprintf(stderr, "> ");
#endif
			
			break;
		}
		
		default:
			GB.Error("Unsupported signature: &1", sign);
			goto __UNSUPPORTED;
	}
	
	GB.ReleaseValue(arg);
	return FALSE;
	
__UNSUPPORTED:
	//GB.Error("Unsupported datatype");
	goto __ERROR;

__ERROR_SIGN:
	dbus_free(sign);

__ERROR:
	GB.ReleaseValue(arg);
	return TRUE;
}

static void return_value_cb(GB_TYPE type, void *value, void *param)
{
	if (type == GB_T_STRING)
		type = GB_T_CSTRING;
	
	GB.ReturnPtr(type, value);
}

static void add_value_cb(GB_TYPE type, void *value, void *param)
{
	GB_ARRAY array = (GB_ARRAY)param;
	GB_VALUE val;
	
	if (type == GB_T_STRING)
		type = GB_T_CSTRING;
	
	GB.ReadValue(&val, value, type);
	GB.BorrowValue(&val);
	GB.Conv(&val, GB.Array.Type(array));
	GB.Store(GB.Array.Type(array), &val, GB.Array.Add(array));
	GB.ReleaseValue(&val);
}

static void add_collection_cb(GB_TYPE type, void *value, void *param)
{
	COLLECTION_ADD *add = (COLLECTION_ADD *)param;
	GB_VALUE val;
	
	if (type == GB_T_STRING)
		type = GB_T_CSTRING;
	
	GB.ReadValue(&val, value, type);
	GB.BorrowValue(&val);
	GB.Conv(&val, GB_T_VARIANT);
	//GB.Store(GB.Array.Type(array), &val, GB.Array.Add(array));
	GB.Collection.Set(add->col, add->key, strlen(add->key), (GB_VARIANT *)&val);
	GB.ReleaseValue(&val);	
}

static bool retrieve_arg(DBusMessageIter *iter, RETRIEVE_CALLBACK cb, void *param)
{
	char *signature = dbus_message_iter_get_signature(iter);
	GB_TYPE gtype = from_dbus_type(signature);
	int type = dbus_message_iter_get_arg_type(iter);
	
#ifdef DEBUG_ARG
	fprintf(stderr, "retrieve_arg: signature = '%s'", signature);
	if (gtype > GB_T_OBJECT) fprintf(stderr, " / %s", GB.GetClassName((void *)gtype));
	fprintf(stderr, "\n");
#endif
	
	dbus_free(signature);
	
	if (dbus_type_is_basic(type))
	{
#ifdef DEBUG_ARG
			fprintf(stderr, "-> basic type\n");
#endif
		char val[16];
		dbus_message_iter_get_basic(iter, (void *)val);
		(*cb)(gtype, val, param);
#ifdef DEBUG_ARG
			fprintf(stderr, "<- basic type\n");
#endif
		return FALSE;
	}
	
	switch(type)
	{
		case DBUS_TYPE_VARIANT:
		{
			DBusMessageIter iter_contents;

			#ifdef DEBUG_ARG
				fprintf(stderr, "-> variant\n");
			#endif
			dbus_message_iter_recurse(iter, &iter_contents);
			if (retrieve_arg(&iter_contents, cb, param))
				return TRUE;
			#ifdef DEBUG_ARG
				fprintf(stderr, "<- variant\n");
			#endif
			return FALSE;
		}
			
		case DBUS_TYPE_ARRAY:
		case DBUS_TYPE_STRUCT:
		{
			char *signature_contents;
			DBusMessageIter iter_contents;
			DBusMessageIter dict_entry_contents;

			dbus_message_iter_recurse(iter, &iter_contents);

			signature_contents = dbus_message_iter_get_signature(&iter_contents);
			
			if (signature_contents[0] == '{' && signature_contents[1] == 's')
			{
				GB_COLLECTION col;
				COLLECTION_ADD add;
				
				#ifdef DEBUG_ARG
					fprintf(stderr, "-> collection\n");
				#endif
					
				GB.Collection.New(POINTER(&col), FALSE);
				
				add.col = col;
				
				for(;;)
				{
					if (dbus_message_iter_get_arg_type(&iter_contents) == DBUS_TYPE_INVALID)
						break;
					
					dbus_message_iter_recurse(&iter_contents, &dict_entry_contents);
					
					// key
					
					dbus_message_iter_get_basic(&dict_entry_contents, &add.key);
					dbus_message_iter_next(&dict_entry_contents);
					
					// value
					
					if (dbus_message_iter_get_arg_type(&dict_entry_contents) != DBUS_TYPE_INVALID)
					{
						if (retrieve_arg(&dict_entry_contents, add_collection_cb, &add))
							return TRUE;
					}
					
					dbus_message_iter_next(&iter_contents);
				}
				
				(*cb)(gtype, &col, param);
				
				#ifdef DEBUG_ARG
					fprintf(stderr, "<- collection\n");
				#endif
				
				return FALSE;
			}
			else
			{
				GB_ARRAY array = GB.New(gtype, NULL, NULL);
				
				#ifdef DEBUG_ARG
					fprintf(stderr, (type == DBUS_TYPE_ARRAY) ? "-> array" : "-> struct");
					fprintf(stderr, " '%s' / %s\n", signature_contents, GB.GetClassName((void *)array));
				#endif
					
				dbus_free(signature_contents);
					
				while (dbus_message_iter_get_arg_type(&iter_contents) != DBUS_TYPE_INVALID)
				{
					if (retrieve_arg(&iter_contents, add_value_cb, array))
						return TRUE;
					dbus_message_iter_next(&iter_contents);
				}
				
				(*cb)(gtype, &array, param);

				#ifdef DEBUG_ARG
					fprintf(stderr, (type == DBUS_TYPE_ARRAY) ? "<- array\n" : "<- struct\n");
				#endif
				
				return FALSE;
			}
		}
	}
			
	GB.Error("Unsupported DBus datatype");
	return TRUE;
}

static bool define_arguments(DBusMessage *message, const char *signature, GB_ARRAY arguments)
{
	int nparam;
	int n;
	GB_TYPE type;
	GB_VALUE value;
	DBusMessageIter iter;
	DBusSignatureIter siter;
	char *sign;
	char *arg_sign;
	bool err = FALSE;
	
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
	
#ifdef DEBUG_ARG
	fprintf(stderr, "define_arguments: %s %d %ld\n", signature, nparam, type);
#endif
	
	if (!signature || !*signature)
	{
		if (nparam > 0) 
		{
			GB.Error("Too many arguments");
			return TRUE;
		}
		else
			return FALSE;
	}
	
	sign = GB.NewZeroString(signature);

	dbus_message_iter_init_append(message, &iter);
	
	dbus_signature_iter_init(&siter, sign);
	
	for (n = 0; n < nparam; n++)
	{
		value.type = type;
		GB.ReadValue(&value, GB.Array.Get(arguments, n), type);
		GB.BorrowValue(&value);

		arg_sign = dbus_signature_iter_get_signature(&siter);
#ifdef DEBUG_ARG
		fprintf(stderr, "[%d] %s: ", n, arg_sign);
#endif
		err = append_arg(&iter, arg_sign, &value);
#ifdef DEBUG_ARG
		fprintf(stderr, "\n");
#endif
		dbus_free(arg_sign);
		
		if (err) 
			goto __RETURN;
		
		if (!dbus_signature_iter_next(&siter))
		{
			if (n < (nparam - 1))
			{
				GB.Error("Too many arguments");
				err = TRUE;
			}
			goto __RETURN;
		}
	}
	
	GB.Error("Not enough arguments");
	err = TRUE;
	
__RETURN:

	GB.FreeString(&sign);
	return err;
}

bool DBUS_call_method(DBusConnection *connection, const char *application, const char *path, const char *interface, const char *method, 
											const char *signature_in, const char *signature_out, GB_ARRAY arguments)
{
	DBusMessage *message;
	//int n;
	DBusMessageIter iter;
	DBusMessage *reply;
	DBusError error;
	//DBusSignatureIter siter;
	bool ret;
	//GB_TYPE type;
	//int nparam;
	
	message = dbus_message_new_method_call(application, path, interface, method);
	if (!message)
	{
		GB.Error("Couldn't allocate D-Bus message");
		return TRUE;
	}
	
	ret = TRUE;
	
	dbus_message_set_auto_start(message, TRUE);

	if (define_arguments(message, signature_in, arguments))
		goto __RETURN;
	
	// Do not use asynchronous call, otherwise error message is lost.
	
	/*if (!signature_out || !*signature_out)
	{
		dbus_connection_send(connection, message, NULL);
		dbus_connection_flush(connection);
		reply = NULL;
		ret = FALSE;
	}*/
	
	dbus_error_init(&error);
	reply = dbus_connection_send_with_reply_and_block(connection, message, -1, &error);
	check_message(connection);

	if (dbus_error_is_set(&error))
	{
		GB.Error("&1: &2", error.name, error.message);
		if (reply)
			dbus_message_unref(reply);
		dbus_error_free(&error);
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


bool DBUS_send_signal(DBusConnection *connection, const char *path, const char *interface, const char *signal, const char *signature_in, GB_ARRAY arguments)
{
	DBusMessage *message;
	bool ret;
	
	message = dbus_message_new_signal(path, interface, signal);
	if (!message)
	{
		GB.Error("Couldn't allocate D-Bus message");
		return TRUE;
	}
	
	ret = TRUE;
	
	//dbus_message_set_auto_start(message, TRUE);

	if (define_arguments(message, signature_in, arguments))
		goto __RETURN;
	
	//dbus_error_init(&error);
	dbus_connection_send(connection, message, NULL);
	//dbus_connection_flush(connection);
	check_message(connection);
	ret = FALSE;

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
	//DBusError error;
	dbus_uint32_t serial = 0;

	reply = dbus_message_new_method_return(message);
	
	if (signature && *signature && arguments)
	{
		if (define_arguments(reply, signature, arguments))
			goto __RETURN;
	}
	
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
		full_type = GB.AddString(full_type, type, 0);
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
	reply = dbus_connection_send_with_reply_and_block(connection, message, -1, &error);
	check_message(connection);

	if (dbus_error_is_set(&error))
	{
		GB.Error("&1: &2", error.name, error.message);
		dbus_error_free(&error);
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
		
		// Beware: "" means "only me" in DBusObserver, but everything there!
		if (check_filter(obs->destination, dbus_message_get_destination(message)))
			continue;
		if (check_filter(obs->object, dbus_message_get_path(message)))
			continue;
		if (check_filter(obs->member, dbus_message_get_member(message)))
			continue;
		if (check_filter(obs->interface, dbus_message_get_interface(message)))
			continue;
		
		//print_message(message, FALSE);
		
		found = TRUE;
		obs->message = message;
		obs->reply = FALSE;
		DBUS_raise_observer(obs);
		obs->message = NULL;
		if (obs->reply)
			return DBUS_HANDLER_RESULT_HANDLED;
	}
	
  if (!found && DBUS_Debug)
	{
		fprintf(stderr, "gb.dbus: warning: unhandled message: ");
		print_message(message, FALSE);
	}
	
  return DBUS_HANDLER_RESULT_HANDLED;
}

static bool get_socket(DBusConnection *connection, int *socket)
{
	if (!dbus_connection_get_socket(connection, socket))
	{
		GB.Error("Unable to get DBus connection socket");
		return TRUE;
	}
	else
		return FALSE;
}

bool DBUS_watch(DBusConnection *connection, bool on)
{
	intptr_t count;
	int socket;
	static dbus_int32_t slot = -1;
	
	if (!dbus_connection_allocate_data_slot(&slot))
	{
		GB.Error("Unable to allocate DBusConnection data slot");
		return TRUE;
	}
		
	if (get_socket(connection, &socket))
		return TRUE;
	
	count = (intptr_t)dbus_connection_get_data(connection, slot);
	
	if (on)
	{
		if (count == 0)
		{
			count++;
			if (!dbus_connection_set_data(connection, slot, (void *)count, NULL))
			{
				GB.Error("Unable to increment watch count");
				return TRUE;
			}
			
			if (!dbus_connection_add_filter(connection, filter_func, NULL, NULL))
			{
				GB.Error("Unable to watch the DBus connection");
				return TRUE;
			}
		
			if (DBUS_Debug)
				fprintf(stderr, "gb.dbus: start watching connection\n");
			
			GB.Watch(socket, GB_WATCH_READ, (void *)handle_message, (intptr_t)connection);
			check_message(connection);
		}
	}
	else
	{
		count--;
		if (!dbus_connection_set_data(connection, slot, (void *)count, NULL))
		{
			GB.Error("Unable to decrement watch count");
			return TRUE;
		}
			
		if (count == 0)
		{
			if (DBUS_Debug)
				fprintf(stderr, "gb.dbus: stop watching connection\n");
			GB.Watch(socket, GB_WATCH_NONE, (void *)handle_message, (intptr_t)connection);
		}
	}
	
	return FALSE;
}

bool DBUS_register(DBusConnection *connection, const char *name, bool unique)
{
	DBusError error;
	int ret;

	dbus_error_init(&error);
	
	//fprintf(stderr, "DBUS_register: %s / unique = %d\n", name, unique);
	
	ret = dbus_bus_request_name(connection, name, unique ? DBUS_NAME_FLAG_DO_NOT_QUEUE : 0, &error);

	if (dbus_error_is_set(&error))
	{
		GB.Error("Unable to register application name: &1", error.message);
		dbus_error_free(&error);
		return TRUE;
	}

	if (unique && ret != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER)
		return TRUE;
	
	return DBUS_watch(connection, TRUE);
}

bool DBUS_unregister(DBusConnection *connection, const char *name)
{
	DBusError error;
	
	dbus_error_init(&error);
	
	dbus_bus_release_name(connection, name, &error);
	
	if (dbus_error_is_set(&error))
	{
		GB.Error("Unable to unregister application name: &1", error.message);
		dbus_error_free(&error);
		return TRUE;
	}

	return DBUS_watch(connection, FALSE);
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

GB_ARRAY DBUS_split_signature(const char *signature)
{
	GB_ARRAY result;
	DBusSignatureIter siter;
	char *sign;
	
	GB.Array.New(POINTER(&result), GB_T_STRING, 0);
	
	dbus_signature_iter_init(&siter, signature);
		
	for(;;)
	{
		sign = dbus_signature_iter_get_signature(&siter);
		*(char **)GB.Array.Add(result) = GB.NewZeroString(sign);
		dbus_free(sign);
			
		if (!dbus_signature_iter_next(&siter))
			break;
	}
	
	return result;
}
