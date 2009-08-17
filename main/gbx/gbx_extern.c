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

#if HAVE_FFI_COMPONENT
#include <ffi.h>
#endif

#include "gb_common_buffer.h"
#include "gb_table.h"
#include "gbx_type.h"
#include "gbx_value.h"
#include "gbx_class.h"
#include "gbx_exec.h"
#include "gbx_api.h"

#include "gbx_extern.h"

typedef
  struct {
    SYMBOL sym;
    lt_dlhandle handle;
    }
  EXTERN_SYMBOL;  

static TABLE *_table = NULL;

#if HAVE_FFI_COMPONENT
static ffi_type *_to_ffi_type[17] = {
	&ffi_type_void, &ffi_type_sint32, &ffi_type_sint32, &ffi_type_sint32, 
	&ffi_type_sint32, &ffi_type_sint64, &ffi_type_float, &ffi_type_double, 
	&ffi_type_void,	&ffi_type_pointer, &ffi_type_pointer, &ffi_type_void, 
	&ffi_type_void, &ffi_type_void, &ffi_type_void, &ffi_type_pointer,
	&ffi_type_pointer
	};
#endif

static lt_dlhandle get_library(char *name)
{
  EXTERN_SYMBOL *esym;
  char *p;
  
  if (!_table)
    TABLE_create(&_table, sizeof(EXTERN_SYMBOL), TF_NORMAL);
    
  TABLE_add_symbol(_table, name, strlen(name), (SYMBOL **)(void *)&esym, NULL);
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
  
#if 0
static int put_arg(void *addr, VALUE *value)
{
  static void *jump[16] = {
    &&__VOID, &&__BOOLEAN, &&__BYTE, &&__SHORT, &&__INTEGER, &&__LONG, &&__SINGLE, &&__FLOAT, &&__DATE,
    &&__STRING, &&__STRING, &&__VARIANT, &&__ARRAY, &&__FUNCTION, &&__CLASS, &&__NULL
    };
    
  if (TYPE_is_object(value->type))
    goto __OBJECT;
  else
    goto *jump[value->type];

__BOOLEAN:

  *((int *)addr) = (value->_boolean.value != 0 ? 1 : 0);
  return 1;

__BYTE:

  *((int *)addr) = (unsigned char)(value->_byte.value);
  return 1;

__SHORT:

  *((int *)addr) = (short)(value->_short.value);
  return 1;

__INTEGER:

  *((int *)addr) = value->_integer.value;
  return 1;

__LONG:

  *((int64_t *)addr) = value->_long.value;
  return 2;

__SINGLE:

  *((float *)addr) = (float)value->_float.value;
  return 1;

__FLOAT:

  *((double *)addr) = value->_float.value;
  return 2;

__DATE:

  /* Inverser au cas o value ~= addr */

  ((int *)addr)[1] = value->_date.time;
  ((int *)addr)[0] = value->_date.date;
  return 2;

__STRING:

  *((char **)addr) = (char *)(value->_string.addr + value->_string.start);
  return 1;

__OBJECT:

  {
    void *ob = value->_object.object;
    CLASS *class = OBJECT_class(ob);
    
    if (!CLASS_is_native(class) && class == CLASS_Class)
      *((void **)addr) = class->stat;
    else
      *((void **)addr) = (char *)ob + sizeof(OBJECT);
    
    return 1;
  }

__NULL:
  *((void **)addr) = NULL;
  return 1;

__VARIANT:
__VOID:
__ARRAY:
__CLASS:
__FUNCTION:

  ERROR_panic("Bad type (%d) for EXTERN_call", value->type);
}
#endif



static void *get_function(CLASS_EXTERN *ext)
{
  lt_dlhandle handle;
  void *func;
  
  if (ext->loaded)
    return (void *)ext->alias;
    
  handle = get_library(ext->library);
  func = lt_dlsym(handle, ext->alias);
  
  if (func == NULL)
  {
    lt_dlclose(handle);
    THROW(E_EXTSYM, ext->library, ext->alias);
  }

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

#if HAVE_FFI_COMPONENT
void EXTERN_call(void)
{
  static const void *jump[17] = {
    &&__VOID, &&__BOOLEAN, &&__BYTE, &&__SHORT, &&__INTEGER, &&__LONG, &&__SINGLE, &&__FLOAT, &&__DATE,
    &&__STRING, &&__STRING, &&__VARIANT, &&__ARRAY, &&__FUNCTION, &&__CLASS, &&__NULL, &&__OBJECT
    };
  static const int use_temp[17] = { 0, 0, 0, 0, 0, 0, sizeof(float), 0, 0, sizeof(char *), sizeof(char *), 0, 0, 0, 0, 0, sizeof(void *) };
  static char temp[4 * sizeof(void *)];
  static void *null = 0;
    
  CLASS_EXTERN *ext = &EXEC.class->load->ext[EXEC.index];
  int nparam = EXEC.nparam;
  ffi_cif cif;
  ffi_type *types[nparam];
  ffi_type *rtype;
  void *args[nparam];
  TYPE *sign;
  VALUE *value;
  char *tmp = NULL;
  char *next_tmp;
  void *func;
  int i, t;
  union {
  	int _integer;
  	float _single;
  	double _float;
  	char * _string;
  	int64_t _long;
  	}
  	rvalue;

  if (nparam < ext->n_param)
    THROW(E_NEPARAM);
  if (nparam > ext->n_param)
    THROW(E_TMPARAM);
  
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
  	types[i] = _to_ffi_type[t];
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
			CLASS *class = OBJECT_class(ob);
			
			if (!CLASS_is_native(class) && class == CLASS_Class)
				*((void **)tmp) = class->stat;
			else
				*((void **)tmp) = (char *)ob + sizeof(OBJECT);
		}
		continue;
	
	__NULL:
		args[i] = &null;
		continue;
	
	__DATE:
	__VARIANT:
	__VOID:
	__ARRAY:
	__CLASS:
	__FUNCTION:
	
		ERROR_panic("Bad type (%d) for EXTERN_call", value->type);
  }
  
  rtype = _to_ffi_type[ext->type];
  
  func = get_function(ext);
  
	if (ffi_prep_cif(&cif, FFI_DEFAULT_ABI, nparam,	rtype, types) != FFI_OK)
		ERROR_panic("ffi_prep_cif has failed");
  
	ffi_call(&cif, func, &rvalue, args);

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
    
    default:
      //GB_ReturnNull();
      break;
  }	

  while (nparam > 0)
  {
    nparam--;
    POP();
  }

  POP(); /* extern function */
  
  /* from EXEC_native() */
    
  BORROW(&TEMP);

  if (EXEC.drop)
    RELEASE(&TEMP);
  else
  {
    VALUE_conv(&TEMP, ext->type);
    *SP = TEMP;
    SP++;
  }
}
#else
void EXTERN_call(void)
{
	THROW(E_ILLEGAL);
}
#endif

void EXTERN_exit(void)
{
  int i;
  EXTERN_SYMBOL *esym;
  
  if (!_table)
    return;
  
  for (i = 0; i < TABLE_count(_table); i++)
  {
    esym = (EXTERN_SYMBOL *)TABLE_get_symbol(_table, i);
    if (esym->handle)
      lt_dlclose(esym->handle);
  }
  
  TABLE_delete(&_table);
}
