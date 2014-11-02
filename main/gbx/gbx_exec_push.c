/***************************************************************************

  gbx_exec_push.c

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

#include "gb_common.h"
#include "gb_limit.h"
#include "gbx_exec.h"
#include "gb_pcode.h"
#include "gbx_api.h"

#include "gbx_string.h"
#include "gbx_c_array.h"
#include "gbx_c_collection.h"
#include "gbx_api.h"
#include "gbx_struct.h"
#include "gbx_local.h"

void EXEC_push_unknown(void)
{
	static void *jump[] = {
		&&_PUSH_GENERIC,               // 0
		&&_PUSH_CONSTANT,              // 1
		&&_PUSH_VARIABLE,              // 2
		&&_PUSH_STATIC_VARIABLE,       // 3
		&&_PUSH_PROPERTY,              // 4
		&&_PUSH_METHOD,                // 5
		&&_PUSH_STATIC_METHOD,         // 6
		&&_PUSH_VARIABLE_AUTO,         // 7
		&&_PUSH_PROPERTY_AUTO,         // 8
		&&_PUSH_METHOD_AUTO,           // 9
		&&_PUSH_EXTERN,                // 10
		&&_PUSH_STRUCT_FIELD,          // 11
		&&_PUSH_CONST_STRING,          // 12
		};

	const char *name;
	int index;
	CLASS_DESC *desc;
	CLASS *class;
	OBJECT *object;
	void *ref;
	char *addr;
	bool defined;
	VALUE *val;

	// EXEC_object can change *PC by calling EXEC_push_unknown() recursively
	// So don't store *PC anywhere

	defined = EXEC_object(&SP[-1], &class, &object);

	goto *jump[*PC & 0xF];


_PUSH_GENERIC:

	name = CP->load->unknown[PC[1]];

	// The first time we access a symbol, we must not be virtual to find it
	val = &SP[-1];
	if (defined && object && !VALUE_is_super(val))
		index = CLASS_find_symbol(val->_object.class, name);
	else
		index = CLASS_find_symbol(class, name);

	if (index == NO_SYMBOL)
	{
		//index = CLASS_find_symbol(class, name);

		if (class->special[SPEC_UNKNOWN] == NO_SYMBOL)
		{
			if (defined && object && !VALUE_is_super(val))
				class = val->_object.class;
			THROW(E_NSYMBOL, CLASS_get_name(class), name);
		}

		if (class->unknown_static && object)
			THROW(E_STATIC, CLASS_get_name(class), name);
		else if (!class->unknown_static && object == NULL)
		{
			if (!class->auto_create)
				THROW(E_DYNAMIC, CLASS_get_name(class), name);
			object = EXEC_auto_create(class, TRUE);
		}

		if (class->special[SPEC_PROPERTY] != NO_SYMBOL)
		{
			EXEC_unknown_name = name;
			if (!EXEC_special(SPEC_PROPERTY, class, class->property_static ? NULL : object, 0, FALSE))
			{
				VALUE_conv_boolean(&SP[-1]);
				SP--;
				if (SP->_boolean.value)
				{
					//SP--; Keep the object on the stack so that it is automatically freed if an error is raised
					EXEC_special(SPEC_UNKNOWN, class, class->unknown_static ? NULL : object, 0, FALSE);
					VALUE_conv_variant(&SP[-1]);
					SP--;
					SP[-1] = *SP;
					OBJECT_UNREF(object);
					goto _FIN;
				}
			}
		}

		goto _PUSH_UNKNOWN_METHOD;
	}
	
	desc = class->table[index].desc;

	switch (CLASS_DESC_get_type(desc))
	{
		case CD_CONSTANT:

			if (TYPE_is_string(desc->constant.type))
			{
				if (defined)
				{
					*PC |= 12;
					PC[1] = index;
				}
				
				goto _PUSH_CONST_STRING_2;
			}
			else
			{
				if (defined)
				{
					if ((PC[-1] & 0xF800) == C_PUSH_CLASS)
					{
						if (desc->constant.type == T_BOOLEAN)
						{
							PC[-1] = C_PUSH_MISC | (desc->constant.value._integer ? CPM_TRUE : CPM_FALSE);
							PC[0] = C_NOP;
							PC[1] = C_NOP;
							goto _PUSH_CONSTANT_2;
						}
						else if (desc->constant.type == T_BYTE || desc->constant.type == T_SHORT)
						{
							PC[-1] = C_PUSH_INTEGER;
							PC[0] = desc->constant.value._integer;
							PC[1] = (CODE_CONV << 8) | desc->constant.type;
							goto _PUSH_CONSTANT_2;
						}
						else if (desc->constant.type == T_INTEGER)
						{
							PC[-1] = C_PUSH_LONG;
							PC[0] = desc->constant.value._integer & 0xFFFF;
							PC[1] = desc->constant.value._integer >> 16;
							goto _PUSH_CONSTANT_2;
						}
					}
					
					*PC |= 1;
					
					PC[1] = index;
				}

				goto _PUSH_CONSTANT_2;
			}

		case CD_VARIABLE:

			if (object == NULL)
			{
				if (!class->auto_create)
					THROW(E_DYNAMIC, CLASS_get_name(class), name);
				object = EXEC_auto_create(class, TRUE);
				*PC |= 7;
			}
			else
			{
				if (defined) *PC |= 2;
			}

			if (defined)
				PC[1] = index;

			goto _PUSH_VARIABLE_2;

		case CD_STRUCT_FIELD:

			if (object == NULL)
				THROW(E_DYNAMIC, CLASS_get_name(class), name);
			
			if (defined) 
			{
				*PC |= 11;
				PC[1] = index;
			}

			goto _PUSH_STRUCT_FIELD_2;

		case CD_STATIC_VARIABLE:

			if (object)
				THROW(E_STATIC, CLASS_get_name(class), name);

			if (defined) *PC |= 3;

			if (defined)
				PC[1] = index;

			goto _PUSH_STATIC_VARIABLE_2;

		case CD_PROPERTY:
		case CD_PROPERTY_READ:

			if (object == NULL)
			{
				if (!class->auto_create)
					THROW(E_DYNAMIC, CLASS_get_name(class), name);
				object = EXEC_auto_create(class, TRUE);
				*PC |= 8;
			}
			else
			{
				if (defined) *PC |= 4;
			}

			if (defined)
				PC[1] = index;

			goto _PUSH_PROPERTY_2;

		case CD_STATIC_PROPERTY:
		case CD_STATIC_PROPERTY_READ:

			if (object)
				THROW(E_STATIC, CLASS_get_name(class), name);

			if (defined) *PC |= 4;

			if (defined)
				PC[1] = index;

			goto _PUSH_PROPERTY_2;

		case CD_METHOD:

			if (object == NULL)
			{
				if (!class->auto_create)
					THROW(E_DYNAMIC, CLASS_get_name(class), name);
				object = EXEC_auto_create(class, TRUE);
				*PC |= 9;
			}
			else
			{
				if (defined) *PC |= 5;
			}

			if (defined)
				PC[1] = index;

			goto _PUSH_METHOD_2;

		case CD_STATIC_METHOD:

			if (object)
			{
				OBJECT_UNREF(object);
				object = NULL;
			}

			if (defined) *PC |= 6;

			if (defined)
				PC[1] = index;

			goto _PUSH_METHOD_2;

		case CD_EXTERN:

			if (object)
			{
				OBJECT_UNREF(object);
				object = NULL;
			}

			if (defined) *PC |= 10;

			if (defined)
				PC[1] = index;

			goto _PUSH_EXTERN_2;

		default:

			THROW(E_NSYMBOL, CLASS_get_name(class), name);
	}

	
_PUSH_CONSTANT:

	desc = class->table[PC[1]].desc;

_PUSH_CONSTANT_2:

	VALUE_read(&SP[-1], (void *)&desc->constant.value, desc->constant.type);
	goto _FIN_DEFINED;


_PUSH_CONST_STRING:

	desc = class->table[PC[1]].desc;

_PUSH_CONST_STRING_2:

	SP--;
	SP->type = T_CSTRING;
	
	if (!desc->constant.value._string)
	{
		SP->_string.addr = NULL;
		SP->_string.len = 0;
	}
	else
	{
		if (desc->constant.translate)
			SP->_string.addr = (char *)LOCAL_gettext(desc->constant.value._string);
		else
			SP->_string.addr = (char *)desc->constant.value._string;
	
		SP->_string.len = strlen(SP->_string.addr);
	}
	
	SP->_string.start = 0;
	SP++;
	goto _FIN_DEFINED_NO_BORROW;


_PUSH_VARIABLE_AUTO:

	object = EXEC_auto_create(class, TRUE);

_PUSH_VARIABLE:

	desc = class->table[PC[1]].desc;

_PUSH_VARIABLE_2:

	addr = (char *)object + desc->variable.offset;
	ref = object;
	goto _READ_VARIABLE;


_PUSH_STATIC_VARIABLE:

	desc = class->table[PC[1]].desc;

_PUSH_STATIC_VARIABLE_2:

	addr = (char *)desc->variable.class->stat + desc->variable.offset;
	ref = desc->variable.class;
	goto _READ_VARIABLE;


_PUSH_STRUCT_FIELD:

	desc = class->table[PC[1]].desc;

_PUSH_STRUCT_FIELD_2:

	if (((CSTRUCT *)object)->ref)
		addr = (char *)((CSTATICSTRUCT *)object)->addr + desc->variable.offset;
	else
		addr = (char *)object + sizeof(CSTRUCT) + desc->variable.offset;
	
	ref = object;
	goto _READ_VARIABLE;


_READ_VARIABLE:

	VALUE_class_read(desc->variable.class, &SP[-1], (void *)addr, desc->variable.ctype, ref);
	goto _FIN_DEFINED;


_PUSH_PROPERTY_AUTO:

	object = EXEC_auto_create(class, TRUE);

_PUSH_PROPERTY:

	desc = class->table[PC[1]].desc;

_PUSH_PROPERTY_2:

	if (desc->property.native)
	{
		if (EXEC_call_native(desc->property.read, object, desc->property.type, NULL))
			PROPAGATE();

		//SP[-1] = TEMP;
		VALUE_copy(&SP[-1], &TEMP);
		goto _FIN_DEFINED;
	}
	else
	{
		EXEC.class = desc->property.class;
		EXEC.object = object;
		EXEC.nparam = 0;
		EXEC.native = FALSE;
		EXEC.index = (int)(intptr_t)desc->property.read;

		EXEC_function_keep();

		//SP[-1] = *RP;
		VALUE_copy(&SP[-1], RP);
		RP->type = T_VOID;
		goto _FIN_DEFINED_NO_BORROW;
	}


_PUSH_STATIC_METHOD:

	if (object)
	{
		OBJECT_UNREF(object);
		object = NULL;
	}

	goto _PUSH_METHOD;

_PUSH_METHOD_AUTO:

	object = EXEC_auto_create(class, TRUE);

_PUSH_METHOD:

	index = PC[1];
	desc = class->table[index].desc;

_PUSH_METHOD_2:

	//printf("PUSH_METHOD: %d %s\n", index, desc->method.name);

	SP--;
	SP->type = T_FUNCTION;
	SP->_function.class = class;
	SP->_function.object = object;
	/*SP->_function.function = (int)&desc->method;*/

	if (FUNCTION_is_native(&desc->method))
	{
		if (desc->method.subr)
			SP->_function.kind = FUNCTION_SUBR;
		else
			SP->_function.kind = FUNCTION_NATIVE;
	}
	else
		SP->_function.kind =  FUNCTION_PUBLIC;

	SP->_function.index = index;

	SP->_function.defined = defined;

	SP++;

	goto _FIN;


_PUSH_EXTERN:

	if (object)
	{
		OBJECT_UNREF(object);
		object = NULL;
	}

	index = PC[1];
	desc = class->table[index].desc;

_PUSH_EXTERN_2:

	//printf("PUSH_METHOD: %d %s\n", index, desc->method.name);

	SP--;
	SP->type = T_FUNCTION;
	SP->_function.class = class;
	SP->_function.object = object;
	SP->_function.kind = FUNCTION_EXTERN;
	SP->_function.index = (int)desc->ext.exec;
	SP->_function.defined = defined;
	SP++;

	goto _FIN;

_PUSH_UNKNOWN_METHOD:

	SP--;
	SP->type = T_FUNCTION;
	SP->_function.class = class;
	SP->_function.object = object;
	SP->_function.kind = FUNCTION_UNKNOWN;
	SP->_function.index = PC[1];
	SP->_function.defined = FALSE;
	SP++;

	//OBJECT_REF(&object);

	goto _FIN;

_FIN_DEFINED:

	BORROW(&SP[-1]);

_FIN_DEFINED_NO_BORROW:

	if (!defined)
		VALUE_conv_variant(&SP[-1]);

	// SP[-1] was the object and it has been erased. So we must unref it manually,
	// unless we are calling a static method (see above)
	OBJECT_UNREF(object);

_FIN:

	PC++;
}

#if 0
void EXEC_push_array(ushort code)
{
	static const void *jump[] = { &&__PUSH_GENERIC, &&__PUSH_QUICK_ARRAY, &&__PUSH_QUICK_COLLECTION, &&__PUSH_ARRAY };
	
	CLASS *class;
	OBJECT *object;
	GET_NPARAM(np);
	//int dim[MAX_ARRAY_DIM];
	int i;
	void *data;
	bool defined;
	VALUE *val;
	int fast;
	//ARRAY_DESC *desc;

	val = &SP[-np];
	np--;

	goto *jump[((unsigned char)code) >> 6];
	
__PUSH_GENERIC:

	defined = EXEC_object(val, &class, &object);
	
	fast = 3;

	if (defined)
	{
		if (class->quick_array == CQA_ARRAY)
			fast = 1;
		else if (class->quick_array == CQA_COLLECTION)
			fast = 2;
		else
		{
			// Check the symbol existance, but *not virtually*
			if (object && !VALUE_is_super(val))
			{
				CLASS *nvclass = val->_object.class;

				if (nvclass->special[SPEC_GET] == NO_SYMBOL)
					THROW(E_NARRAY, CLASS_get_name(nvclass));
			}
		}
	}

	*PC |= fast << 6;
	
	goto __PUSH_ARRAY_2;

/*__PUSH_STATIC_ARRAY:

	for (i = 1; i <= np; i++)
	{
		VALUE_conv_integer(&val[i];
		dim[i - 1] = val[i]._integer.value;
	}

	SP = val;

	desc = (ARRAY_DESC *)SP->_array.class->load->array[SP->_array.index];
	data = ARRAY_get_address(desc, SP->_array.addr, np, dim);

	VALUE_read(SP, data, CLASS_ctype_to_type(SP->_array.class, desc->type));

	PUSH();
	return;*/

__PUSH_QUICK_ARRAY:

	// Optimization test

	//EXEC_object_fast(val, &class, &object);
	EXEC_object_array(val, class, object);

	VALUE_conv_integer(&val[1]);

	if (np == 1)
	{
		data = CARRAY_get_data((CARRAY *)object, val[1]._integer.value);
	}
	else
	{
		for (i = 2; i <= np; i++)
			VALUE_conv_integer(&val[i]);

		data = CARRAY_get_data_multi((CARRAY *)object, (GB_INTEGER *)&val[1], np);
	}

	if (!data)
		PROPAGATE();

	//VALUE_read(val, data, ((CARRAY *)object)->type);
	if (TYPE_is_object(((CARRAY *)object)->type))
		fast = T_OBJECT;
	else
		fast = ((CARRAY *)object)->type;

	VALUE_read_inline_type(val, data, fast);

	goto __PUSH_QUICK_END;

__PUSH_QUICK_COLLECTION:

	EXEC_object_fast(val, &class, &object);
	
	VALUE_conv_string(&val[1]);
	//fprintf(stderr, "GB_CollectionGet: %p '%.*s'\n", val[1]._string.addr, val[1]._string.len, val[1]._string.addr + val[1]._string.start);
	GB_CollectionGet((GB_COLLECTION)object, val[1]._string.addr + val[1]._string.start, val[1]._string.len, (GB_VARIANT *)val);
	
	RELEASE_STRING(&val[1]);
	
__PUSH_QUICK_END:
	
	SP = val;
	PUSH();
	OBJECT_UNREF(object);
	return;

__PUSH_ARRAY:

	defined = EXEC_object(val, &class, &object);
	
__PUSH_ARRAY_2:

	if (EXEC_special(SPEC_GET, class, object, np, FALSE))
		THROW(E_NARRAY, CLASS_get_name(class));

	OBJECT_UNREF(object);
	SP--;
	//SP[-1] = SP[0];
	VALUE_copy(&SP[-1], &SP[0]);

	if (!defined)
		VALUE_conv_variant(&SP[-1]);
}
#endif

// JIT needs it
void EXEC_push_array(ushort code)
{
	CLASS *class;
	OBJECT *object;
	GET_NPARAM(np);
	bool defined;
	VALUE *val;

	val = &SP[-np];
	np--;
	defined = EXEC_object(val, &class, &object);

	if (EXEC_special(SPEC_GET, class, object, np, FALSE))
		THROW(E_NARRAY, CLASS_get_name(class));

	OBJECT_UNREF(object);
	SP--;
	//SP[-1] = SP[0];
	VALUE_copy(&SP[-1], &SP[0]);

	if (!defined)
		VALUE_conv_variant(&SP[-1]);
}

int EXEC_push_unknown_event(bool unknown)
{
	int index;
	CLASS_DESC *desc;
	const char *name;
	
	if (unknown)
	{
		name = CP->load->unknown[PC[1]];
		// The ':' is already in the name, thanks to the compiler.
		index = CLASS_find_symbol(CP, name);
		if (index == NO_SYMBOL)
			THROW(E_DYNAMIC, CLASS_get_name(CP), name);

		desc = CP->table[index].desc;
		if (CLASS_DESC_get_type(desc) != CD_EVENT)
			THROW(E_DYNAMIC, CLASS_get_name(CP), name);
		
		PC[1] = index;
		PC[0] &= ~1;
	}
	else
	{
		desc = CP->table[PC[1]].desc;
	}
	
	index = desc->event.index;
	
	//if (desc->event.class->parent)
	//	index += desc->event.class->parent->n_event;
	
	return index;
}

