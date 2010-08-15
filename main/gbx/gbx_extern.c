/***************************************************************************

	gbx_extern.c

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

#define __GBX_EXTERN_C

#include "config.h"
#include "gb_common.h"

#ifdef HAVE_FFI_COMPONENT

#include <ffi.h>

#include "gb_common_buffer.h"
#include "gb_table.h"
#include "gbx_type.h"
#include "gbx_value.h"
#include "gbx_class_desc.h"
#include "gbx_class.h"
#include "gbx_exec.h"
#include "gbx_api.h"
#include "gbx_c_array.h"
#include "gbx_struct.h"
#include "gbx_extern.h"

typedef
	struct {
		SYMBOL sym;
		lt_dlhandle handle;
		}
	EXTERN_SYMBOL;  

typedef
	struct {
		ffi_cif cif;
		ffi_type **types;
		ffi_type *rtype;
	}
	EXTERN_CIF;
	
typedef
	struct EXTERN_CALLBACK {
		struct EXTERN_CALLBACK *next;
		EXEC_GLOBAL exec;
		void *closure;
		int nparam;
		TYPE *sign;
		TYPE ret;
		EXTERN_CIF info;
	}
	EXTERN_CALLBACK;
	
typedef
	struct EXTERN_FUNC {
		struct EXTERN_FUNC *next;
		void *call;
		EXTERN_CIF info;
	}
	EXTERN_FUNC;

static TABLE *_table = NULL;
static EXTERN_CALLBACK *_callbacks = NULL;
static EXTERN_FUNC *_functions = NULL;

static ffi_type *_to_ffi_type[17] = {
	&ffi_type_void, &ffi_type_sint32, &ffi_type_sint32, &ffi_type_sint32, 
	&ffi_type_sint32, &ffi_type_sint64, &ffi_type_float, &ffi_type_double, 
	&ffi_type_void,	&ffi_type_pointer, &ffi_type_pointer, &ffi_type_pointer, 
	&ffi_type_void, &ffi_type_void, &ffi_type_void, &ffi_type_pointer,
	&ffi_type_pointer
	};


static void prepare_cif(EXTERN_CIF *info, int nparam, TYPE *sign, TYPE ret)
{
	int i;
	TYPE t;

	info->types = NULL;
	
	if (nparam > 0)
	{
		ALLOC(&info->types, sizeof(ffi_type *) * nparam, "prepare_cif");
		
		for (i = 0; i < nparam; i++)
		{
			if (TYPE_is_object(sign[i]))
				t = T_OBJECT;
			else
				t = (int)sign[i];
				
			info->types[i] = _to_ffi_type[t];
		}
	}
	
	if (TYPE_is_object(ret))
		t = T_OBJECT;
	else
		t = (int)ret;
	
	info->rtype = _to_ffi_type[t];

	if (ffi_prep_cif(&info->cif, FFI_DEFAULT_ABI, nparam, info->rtype, info->types) != FFI_OK)
		THROW(E_EXTCB, "Unable to prepare function description");
}


static lt_dlhandle get_library(char *name)
{
	EXTERN_SYMBOL *esym;
	char *p;
	int index;
	
	if (!_table)
		TABLE_create(&_table, sizeof(EXTERN_SYMBOL), TF_NORMAL);
		
	TABLE_add_symbol(_table, name, strlen(name), &index);
	esym = (EXTERN_SYMBOL *)TABLE_get_symbol(_table, index);
	if (!esym->handle)
	{
		/* !!! Must add the suffix !!! */
	
		p = strrchr(name, ':');
		if (!p)
			sprintf(COMMON_buffer, "%s." SHARED_LIBRARY_EXT, name);
		else
			sprintf(COMMON_buffer, "%.*s." SHARED_LIBRARY_EXT ".%s", (int)(p - name), name, p + 1);
			
		name = COMMON_buffer;
		
		#ifndef DONT_USE_LTDL
			/* no more available in libltld ?
			lt_dlopen_flag = RTLD_LAZY;
			*/
			esym->handle = lt_dlopenext(name);
		#else
			esym->handle = dlopen(name, RTLD_LAZY);
		#endif
	
		if (esym->handle == NULL)
			THROW(E_EXTLIB, name, lt_dlerror());
			
		//fprintf(stderr, "%s loaded.\n", name);
	}
	
	return esym->handle;
}  


static EXTERN_FUNC *get_function(CLASS_EXTERN *ext)
{
	EXTERN_FUNC *func;
	void *call;
	lt_dlhandle handle;
	
	if (ext->loaded)
		return (EXTERN_FUNC *)ext->alias;
	
	handle = get_library(ext->library);
	call = lt_dlsym(handle, ext->alias);
	
	if (call == NULL)
	{
		lt_dlclose(handle);
		THROW(E_EXTSYM, ext->library, ext->alias);
	}

	ALLOC(&func, sizeof(EXTERN_FUNC), "get_function");
	func->next = _functions;
	_functions = func;
	
	func->call = call;
	prepare_cif(&func->info, ext->n_param, (TYPE *)ext->param, ext->type);
	
	//ext->library = (char *)handle;
	ext->alias = (char *)func;
	ext->loaded = TRUE;
	
	return func;
}


/*
	EXEC.class : the class
	EXEC.index : the extern function index
	EXEC.nparam : the number of parameters to the call
	EXEC.drop : if the return value should be dropped.
*/

void EXTERN_call(void)
{
	static const void *jump[17] = {
		&&__VOID, &&__BOOLEAN, &&__BYTE, &&__SHORT, &&__INTEGER, &&__LONG, &&__SINGLE, &&__FLOAT, &&__DATE,
		&&__STRING, &&__STRING, &&__POINTER, &&__VARIANT, &&__FUNCTION, &&__CLASS, &&__NULL, &&__OBJECT
		};
	static const int use_temp[17] = { 0, 0, 0, 0, 0, 0, sizeof(float), 0, 0, sizeof(char *), sizeof(char *), 0, 0, 0, 0, 0, sizeof(void *) };
	static char temp[4 * sizeof(void *)];
	static void *null = 0;
		
	CLASS_EXTERN *ext = &EXEC.class->load->ext[EXEC.index];
	EXTERN_FUNC *func;
	int nparam = EXEC.nparam;
	void *args[nparam];
	TYPE *sign;
	VALUE *value;
	char *tmp = NULL;
	char *next_tmp;
	int i, t;
	union {
		int _integer;
		float _single;
		double _float;
		char *_string;
		int64_t _long;
		void *_pointer;
		}
		rvalue;

	if (!ext->loaded)
	{
		if (nparam < ext->n_param)
			THROW(E_NEPARAM);
		if (nparam > ext->n_param)
			THROW(E_TMPARAM);
	}
	
	func = get_function(ext);
	
	sign = (TYPE *)ext->param;
	value = &SP[-nparam];
	next_tmp = temp;
	
	for (i = 0; i < nparam; i++, value++, sign++)
	{
		VALUE_conv(value, *sign);
		
		if (TYPE_is_object(value->type))
			t = T_OBJECT;
		else
			t = (int)value->type;
			
		if (use_temp[t])
		{
			tmp = next_tmp;
			if ((tmp + use_temp[t]) > &temp[sizeof(temp)])
				THROW(E_TMPARAM);
			args[i] = tmp;
			next_tmp = tmp + use_temp[t];
		}
		goto *jump[t];

	__BOOLEAN:
	__BYTE:
	__SHORT:
	__INTEGER:
		args[i] = &value->_integer.value;
		continue;
	
	__LONG:
		args[i] = &value->_long.value;
		continue;
	
	__SINGLE:
		*((float *)tmp) = (float)value->_float.value;
		continue;
	
	__FLOAT:
		args[i] = &value->_float.value;
		continue;
	
	__STRING:
		*((char **)tmp) = (char *)(value->_string.addr + value->_string.start);
		continue;
	
	__OBJECT:	
		{
			void *ob = value->_object.object;
			void *addr;
			CLASS *class = OBJECT_class(ob);
			
			if (!CLASS_is_native(class) && class == CLASS_Class)
				addr = class->stat;
			else if (CLASS_is_array(class))
				addr = ((CARRAY *)ob)->data;
			else if (CLASS_is_struct(class))
			{
				if (((CSTRUCT *)ob)->ref)
					addr = (char *)((CSTATICSTRUCT *)ob)->addr;
				else
					addr = (char *)ob + sizeof(CSTRUCT);
			}
			else
				addr = (char *)ob + sizeof(OBJECT);
			
			*((void **)tmp) = addr;
		}
		continue;
	
	__POINTER:
		args[i] = &value->_pointer.value;
		continue;
	
	__NULL:
		args[i] = &null;
		continue;
	
	__DATE:
	__VARIANT:
	__VOID:
	__CLASS:
	__FUNCTION:
	
		ERROR_panic("Bad type (%d) for EXTERN_call", value->type);
	}
	
	ffi_call(&func->info.cif, func->call, &rvalue, args);

	switch (ext->type)
	{
		case T_BOOLEAN:
		case T_BYTE:
		case T_SHORT:
		case T_INTEGER:
			//GB_ReturnInteger(*(int *)POINTER(rvalue));
			GB_ReturnInteger(rvalue._integer);
			break;
		
		case T_LONG:
			//GB_ReturnLong(*(int64_t *)POINTER(rvalue));
			GB_ReturnLong(rvalue._long);
			break;
		
		case T_SINGLE:
			//GB_ReturnFloat(*(float *)POINTER(rvalue));
			GB_ReturnFloat(rvalue._single);
			break;
			
		case T_FLOAT:
			//GB_ReturnFloat(*(double *)POINTER(rvalue));
			GB_ReturnFloat(rvalue._float);
			break;
			
		case T_STRING:
			//GB_ReturnConstString(*(char **)POINTER(rvalue), 0);
			GB_ReturnConstString(rvalue._string, 0);
			break;
			
		case T_POINTER:
			GB_ReturnPointer(rvalue._pointer);
			break;
			
		case T_VOID:
		default:
			TEMP.type = T_VOID;
			break;
	}	

	while (nparam)
	{
		nparam--;
		POP();
	}

	POP(); /* extern function */
	
	/* from EXEC_native() */
		
	BORROW(&TEMP);

	VALUE_conv(&TEMP, ext->type);
	*SP = TEMP;
	SP++;
}

void EXTERN_release(void)
{
	EXTERN_CALLBACK *cb = _callbacks;
	
	while (cb)
	{
		if (cb->exec.object)
			OBJECT_UNREF(cb->exec.object, "EXTERN_exit");
		cb = cb->next;
	}
}

void EXTERN_exit(void)
{
	int i;
	EXTERN_SYMBOL *esym;
	EXTERN_CALLBACK *cb;
	EXTERN_FUNC *func;
	
	if (_table)
	{
		for (i = 0; i < TABLE_count(_table); i++)
		{
			esym = (EXTERN_SYMBOL *)TABLE_get_symbol(_table, i);
			if (esym->handle)
				lt_dlclose(esym->handle);
		}
		
		TABLE_delete(&_table);
	}
	
	while (_callbacks)
	{
		cb = _callbacks;
		_callbacks = cb->next;
		
		if (cb->exec.object)
			OBJECT_UNREF(cb->exec.object, "EXTERN_exit");
		FREE(&cb->info.types, "EXTERN_exit");
		FREE(&cb, "EXTERN_exit");
	}
	
	while (_functions)
	{
		func = _functions;
		_functions = func->next;
		
		FREE(&func->info.types, "EXTERN_exit");
		FREE(&func, "EXTERN_exit");
	}
}

static void callback(ffi_cif *cif, void *result, void **args, void *user_data)
{
	static const void *jump[17] = {
		&&__VOID, &&__BOOLEAN, &&__BYTE, &&__SHORT, &&__INTEGER, &&__LONG, &&__SINGLE, &&__FLOAT, &&__DATE,
		&&__STRING, &&__STRING, &&__POINTER, &&__VARIANT, &&__FUNCTION, &&__CLASS, &&__NULL, &&__OBJECT
		};

	EXTERN_CALLBACK *cb = (EXTERN_CALLBACK *)user_data;
	//VALUE_FUNCTION *value = &cb->func;
	int i;
	VALUE *arg;
	
	STACK_check(cb->nparam);
	
	for (i = 0; i < cb->nparam; i++)
	{
		arg = SP++;
		arg->type = cb->sign[i];
		
		if (TYPE_is_object(cb->sign[i]))
			goto __OBJECT;
		else
			goto *jump[cb->sign[i]];
		
	__BOOLEAN:
		arg->_integer.value = *((char *)args[i]) ? -1 : 0;
		continue;
		
	__BYTE:
		arg->_integer.value = *((char *)args[i]);
		continue;
	
	__SHORT:
		arg->_integer.value = *((short *)args[i]);
		continue;

	__INTEGER:
		arg->_integer.value = *((int *)args[i]);
		continue;
	
	__LONG:
		arg->_integer.value = *((int64_t *)args[i]);
		continue;
	
	__SINGLE:
		arg->_integer.value = *((float *)args[i]);
		continue;
	
	__FLOAT:
		arg->_integer.value = *((double *)args[i]);
		continue;
	
	__STRING:
		arg->type = T_CSTRING;
		arg->_string.addr = *((char **)args[i]);
		arg->_string.start = 0;
		arg->_string.len = arg->_string.addr ? strlen(arg->_string.addr) : 0;
		continue;
	
	__OBJECT:	
		arg->_object.object = *((void **)args[i]);
		continue;
	
	__POINTER:
		arg->_pointer.value = *((void **)args[i]);
		continue;
	
	__NULL:
	__DATE:
	__VARIANT:
	__VOID:
	__CLASS:
	__FUNCTION:
		arg->type = T_NULL;
	}

	EXEC = cb->exec;
	
	if (!EXEC.native)
	{
		EXEC_function_keep();
		
		// Do that later, within a TRY/CATCH: VALUE_conv(RP, cb->ret);
		
		switch (cb->ret)
		{
			case T_BOOLEAN:
			case T_BYTE:
			case T_SHORT:
			case T_INTEGER:
				*((ffi_arg *)result) = RP->_integer.value;
				break;
			
			case T_LONG:
				*((int64_t *)result) = RP->_long.value;
				break;
			
			case T_SINGLE:
				*((float *)result) = RP->_single.value;
				break;
				
			case T_FLOAT:
				*((double *)result) = RP->_float.value;
				break;
				
			case T_STRING:
				if (!RP->_string.len)
					*((char **)result) = NULL;
				else
					*((char **)result) = RP->_string.addr + RP->_string.start;
				break;
				
			case T_OBJECT:
				*((void **)result) = RP->_object.object;
				break;
				
			case T_POINTER:
				*((void **)result) = RP->_pointer.value;
				break;
				
			default:
				break;
		}	
		
		EXEC_release_return_value();
	}
}


static void prepare_cif_from_gambas(EXTERN_CALLBACK *cb, FUNCTION *func)
{
	if (func->npmin != func->n_param || func->vararg)
		THROW(E_EXTCB, "The function must take a fixed number of arguments");
	
	cb->nparam = func->npmin;
	cb->sign = (TYPE *)func->param;
	cb->ret = func->type;
}

static void prepare_cif_from_native(EXTERN_CALLBACK *cb, CLASS_DESC_METHOD *desc)
{
	THROW(E_EXTCB, "Not implemented yet");
}

void *EXTERN_make_callback(VALUE_FUNCTION *value)
{
	EXTERN_CALLBACK *cb;
	void *code;
	
	if (value->kind == FUNCTION_EXTERN)
	{
		CLASS_EXTERN *ext = &value->class->load->ext[value->index];
		return get_function(ext);
	}
	
	ALLOC(&cb, sizeof(EXTERN_CALLBACK), "EXTERN_make_callback");
	
	cb->next = _callbacks;
	_callbacks = cb;
	
	if (value->object)
		OBJECT_REF(value->object, "EXTERN_make_callback");
	
	// See gbx_exec_loop.c, at the _CALL label, to understand the following.
	
	if (value->kind == FUNCTION_PRIVATE)
	{
		cb->exec.object = value->object;
		cb->exec.class = value->class;
		cb->exec.native = FALSE;
		cb->exec.index = value->index;
		
		prepare_cif_from_gambas(cb, &cb->exec.class->load->func[cb->exec.index]);
	}
	else if (value->kind == FUNCTION_PUBLIC)
	{
		cb->exec.object = value->object;
		cb->exec.native = FALSE;
		cb->exec.desc = &value->class->table[value->index].desc->method;
		cb->exec.index = (int)(intptr_t)(cb->exec.desc->exec);
		cb->exec.class = cb->exec.desc->class;
		prepare_cif_from_gambas(cb, &cb->exec.class->load->func[cb->exec.index]);
	}
	else if (value->kind == FUNCTION_NATIVE)
	{
		cb->exec.object = value->object;
		cb->exec.class = value->class;
		cb->exec.native = TRUE;
		cb->exec.index = value->index;
		cb->exec.desc = &value->class->table[value->index].desc->method;
		//cb->desc = &value->class->table[value->index].desc->method;
		prepare_cif_from_native(cb, cb->exec.desc);
	}
	else
		THROW(E_EXTCB, "Function must be public or extern");
		
	cb->exec.nparam = cb->nparam;
	
	prepare_cif(&cb->info, cb->nparam, cb->sign, cb->ret);
	
	cb->closure = ffi_closure_alloc(sizeof(ffi_closure), &code);
	
	if (ffi_prep_closure_loc(cb->closure, &cb->info.cif, callback, cb, code) != FFI_OK)
		THROW(E_EXTCB, "Unable to create closure");
	
	return code;
}

#else /* HAVE_FFI_COMPONENT */

#include "gbx_value.h"

void EXTERN_call(void)
{
	THROW_ILLEGAL();
}

void EXTERN_release(void)
{
}

void EXTERN_exit(void)
{
}

void *EXTERN_make_callback(VALUE_FUNCTION *value)
{
	return NULL;
}

#endif

