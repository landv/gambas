/***************************************************************************

  gbx_extern.c

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

#define __GBX_EXTERN_C

#include "config.h"
#include "gb_common.h"

#ifdef HAVE_FFI_COMPONENT

#include <ffi.h>

#include "gb_common_buffer.h"
#include "gb_table.h"
#include "gb_hash.h"
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
		void *code;
		int nparam;
		TYPE *sign;
		TYPE ret;
		EXTERN_CIF info;
	}
	EXTERN_CALLBACK;

typedef
	struct EXTERN_FUNC {
		struct EXTERN_FUNC *next;
		char *alias;
		void *call;
		EXTERN_CIF info;
	}
	EXTERN_FUNC;

static TABLE *_table = NULL;
//static EXTERN_CALLBACK *_callbacks = NULL;
static HASH_TABLE *_callbacks = NULL;
static EXTERN_FUNC *_functions = NULL;

static ffi_type *_to_ffi_type[17] = {
	&ffi_type_void, &ffi_type_sint32, &ffi_type_sint32, &ffi_type_sint32, 
	&ffi_type_sint32, &ffi_type_sint64, &ffi_type_float, &ffi_type_double, 
	&ffi_type_void,	&ffi_type_pointer, &ffi_type_pointer, &ffi_type_pointer, 
	&ffi_type_void, &ffi_type_void, &ffi_type_void, &ffi_type_pointer,
	&ffi_type_pointer
	};


static void prepare_cif(EXTERN_CIF *info, int nsign, TYPE *sign, TYPE ret, int nparam, VALUE *value)
{
	int i;
	TYPE t;

	info->types = NULL;
	
	if (nparam > 0)
	{
		ALLOC(&info->types, sizeof(ffi_type *) * nparam);
		
		for (i = 0; i < nparam; i++)
		{
			if (i < nsign)
				t = sign[i];
			else
				t = value[i].type;
			
			if (TYPE_is_object(t))
				t = T_OBJECT;
				
			info->types[i] = _to_ffi_type[t];
		}
	}
	
	if (TYPE_is_object(ret))
		t = T_OBJECT;
	else
		t = ret;
	
	info->rtype = _to_ffi_type[t];

	if (ffi_prep_cif(&info->cif, FFI_DEFAULT_ABI, nparam, info->rtype, info->types) != FFI_OK)
		THROW(E_EXTCB, "Unable to prepare function description");
}


static lt_dlhandle get_library(const char *name)
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
		{
			name = GB_RealFileName(name, strlen(name));
			#ifndef DONT_USE_LTDL
				esym->handle = lt_dlopenext(name);
			#else
				esym->handle = dlopen(name, RTLD_LAZY);
			#endif
		}
	
		if (esym->handle == NULL)
			THROW(E_EXTLIB, name, lt_dlerror());
			
		//fprintf(stderr, "%s loaded.\n", name);
	}
	
	return esym->handle;
}  

void *EXTERN_get_symbol(const char *library, const char *symbol)
{
	lt_dlhandle handle = get_library(library);
	return lt_dlsym(handle, symbol);
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

	ALLOC_ZERO(&func, sizeof(EXTERN_FUNC));
	func->next = _functions;
	func->alias = ext->alias;
	_functions = func;
	
	func->call = call;
	
	if (!ext->vararg)
		prepare_cif(&func->info, ext->n_param, (TYPE *)ext->param, ext->type, ext->n_param, NULL);
	
	//ext->library = (char *)handle;
	ext->alias = (char *)func;
	ext->loaded = TRUE;
	
	return func;
}

EXTERN_FUNC_INFO EXTERN_get_function_info(CLASS_EXTERN *ext)
{
	EXTERN_FUNC *func = get_function(ext);
	EXTERN_FUNC_INFO func_info = { func->call, func->alias };
	
	return func_info;
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
	static const int use_temp[17] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, sizeof(char *), sizeof(char *), 0, 0, 0, 0, 0, sizeof(void *) };
	static char temp[16 * sizeof(void *)];
	static void *null = 0;
		
	CLASS_EXTERN *ext = &EXEC.class->load->ext[EXEC.index];
	EXTERN_FUNC *func;
	EXTERN_CIF cif;
	bool vararg;
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
		if (!ext->vararg && nparam > ext->n_param)
			THROW(E_TMPARAM);
	}
	
	func = get_function(ext);
	sign = (TYPE *)ext->param;
	vararg = ext->vararg;
	value = &SP[-nparam];
	next_tmp = temp;

	for (i = 0; i < nparam; i++)
	{
		if (i < ext->n_param)
			VALUE_conv(&value[i], sign[i]);
		else
			VARIANT_undo(&value[i]);
	}
	
	if (vararg)
		prepare_cif(&cif, ext->n_param, (TYPE *)ext->param, ext->type, nparam, value);
	
	for (i = 0; i < nparam; i++, value++, sign++)
	{
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
		args[i] = &value->_single.value;
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
			CLASS *class;
			
			if (!ob)
				goto __NULL;
			
			class = OBJECT_class(ob);
			
			if (class == CLASS_Class && !CLASS_is_native((CLASS *)ob))
				addr = ((CLASS *)ob)->stat;
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
	
		THROW(E_UTYPE);
	}
	
	if (vararg)
	{
		ffi_call(&cif.cif, func->call, &rvalue, args);
		FREE(&cif.types);
	}
	else
		ffi_call(&func->info.cif, func->call, &rvalue, args);
	

	switch (ext->type)
	{
		case T_BOOLEAN:
		case T_BYTE:
		case T_SHORT:
		case T_INTEGER:
			GB_ReturnInteger(rvalue._integer);
			break;
		
		case T_LONG:
			GB_ReturnLong(rvalue._long);
			break;
		
		case T_SINGLE:
			GB_ReturnSingle(rvalue._single);
			break;
			
		case T_FLOAT:
			GB_ReturnFloat(rvalue._float);
			break;
			
		case T_STRING:
			GB_ReturnConstZeroString(rvalue._string);
			break;
			
		case T_POINTER:
			GB_ReturnPointer(rvalue._pointer);
			break;
			
		case T_VOID:
		default:
			
			if (TYPE_is_pure_object(ext->type))
			{
				CLASS *class = (CLASS *)(ext->type);
				if (CLASS_is_struct(class))
				{
					GB_ReturnObject(CSTRUCT_create_static(STRUCT_CONST, class, rvalue._pointer));
					break;
				}
			}
			
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
	EXTERN_CALLBACK *cb;
	HASH_ENUM iter;
	
	if (!_callbacks)
		return;
	
	CLEAR(&iter);
		
	for(;;)
	{
		cb = HASH_TABLE_next(_callbacks, &iter, FALSE);
		if (!cb)
			break;
		if (cb->exec.object)
		{
			OBJECT_UNREF(cb->exec.object);
			cb->exec.object = NULL;
		}
	}
}

void EXTERN_exit(void)
{
	int i;
	EXTERN_SYMBOL *esym;
	EXTERN_CALLBACK *cb;
	EXTERN_FUNC *func;
	HASH_ENUM iter;
	
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
	
	if (_callbacks)
	{
		CLEAR(&iter);
		
		for(;;)
		{
			cb = HASH_TABLE_next(_callbacks, &iter, FALSE);
			if (!cb)
				break;
			if (cb->exec.object)
				OBJECT_UNREF(cb->exec.object);
			FREE(&cb->info.types);
			ffi_closure_free(cb->closure);
		}
	
		HASH_TABLE_delete(&_callbacks);
	}
			
	while (_functions)
	{
		func = _functions;
		_functions = func->next;
		
		FREE(&func->info.types);
		FREE(&func);
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
		arg->_long.value = *((int64_t *)args[i]);
		continue;
	
	__SINGLE:
		arg->_single.value = *((float *)args[i]);
		continue;
	
	__FLOAT:
		arg->_float.value = *((double *)args[i]);
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

/*static void prepare_cif_from_native(EXTERN_CALLBACK *cb, CLASS_DESC_METHOD *desc)
{
	THROW(E_EXTCB, "Not implemented yet");
}*/

void *EXTERN_make_callback(VALUE_FUNCTION *value)
{
	EXEC_GLOBAL exec = { 0 };
	FUNCTION *func;
	EXTERN_CALLBACK *cb;	
	union {
		char key[sizeof(void *)];
		void *addr;
	}
	cb_key;
	
	if (value->kind == FUNCTION_EXTERN)
	{
		CLASS_EXTERN *ext = &value->class->load->ext[value->index];
		return get_function(ext)->call;
	}
	
	if (!_callbacks)
		HASH_TABLE_create(&_callbacks, sizeof(EXTERN_CALLBACK), HF_NORMAL);
	
	//ALLOC(&cb, sizeof(EXTERN_CALLBACK), "EXTERN_make_callback");
	
	// See gbx_exec_loop.c, at the _CALL label, to understand the following.
	
	if (value->kind == FUNCTION_PRIVATE)
	{
		exec.object = value->object;
		exec.class = value->class;
		exec.native = FALSE;
		exec.index = value->index;
	}
	else if (value->kind == FUNCTION_PUBLIC)
	{
		exec.object = value->object;
		exec.native = FALSE;
		exec.desc = &value->class->table[value->index].desc->method;
		exec.index = (int)(intptr_t)(exec.desc->exec);
		exec.class = exec.desc->class;
	}
	/*else if (value->kind == FUNCTION_NATIVE)
	{
		cb->exec.object = value->object;
		cb->exec.class = value->class;
		cb->exec.native = TRUE;
		cb->exec.index = value->index;
		cb->exec.desc = &value->class->table[value->index].desc->method;
		//cb->desc = &value->class->table[value->index].desc->method;
		
		prepare_cif_from_native(cb, cb->exec.desc);
	}*/
	else
		THROW(E_EXTCB, "Not supported");
	
	func = &exec.class->load->func[exec.index];
	cb_key.addr = func;
	
	cb = (EXTERN_CALLBACK *)HASH_TABLE_insert(_callbacks, cb_key.key, sizeof(void *));
	if (cb->code)
	{
		OBJECT_UNREF(value->object);
		return cb->code;
	}
	
	// Do not reference value->_function.object, as it has been already referenced
	// when put on the stack in exec_loop.c
	
	/*if (value->object)
	{
		fprintf(stderr, "EXTERN_make_callback: ref: %p\n", value->object);
		OBJECT_REF(value->object);
	}*/
	
	cb->exec = exec;
	prepare_cif_from_gambas(cb, func);
	
	cb->exec.nparam = cb->nparam;
	
	prepare_cif(&cb->info, cb->nparam, cb->sign, cb->ret, cb->nparam, NULL);
	
	cb->closure = ffi_closure_alloc(sizeof(ffi_closure), &cb->code);
	
	if (ffi_prep_closure_loc(cb->closure, &cb->info.cif, callback, cb, cb->code) != FFI_OK)
		THROW(E_EXTCB, "Unable to create closure");
	
	return cb->code;
}

#else /* HAVE_FFI_COMPONENT */

#include "gbx_value.h"
#include "gbx_extern.h"

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

void *EXTERN_get_symbol(const char *library, const char *symbol)
{
	return NULL;
}


EXTERN_FUNC_INFO EXTERN_get_function_info(CLASS_EXTERN *ext)
{
	EXTERN_FUNC_INFO func_info = { NULL, NULL };
	
	return func_info;
}
#endif

