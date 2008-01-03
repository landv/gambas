/***************************************************************************

  gbx_extern.c

  Extern calls in dynamic libraries

  (c) 2000-2007 Benoit Minisini <gambas@users.sourceforge.net>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 1, or (at your option)
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
#include "gb_common_buffer.h"
#include "gb_table.h"
#include "gbx_type.h"
#include "gbx_value.h"
#include "gbx_class.h"
#include "gbx_exec.h"
#include "gbx_api.h"

#include "gbx_extern.h"

/* Daniel Campos trick :-) */

typedef
  struct {
    int args[32];
  }
  ARGS;

typedef
  struct {
    SYMBOL sym;
    lt_dlhandle handle;
    }
  EXTERN_SYMBOL;  

static TABLE *_table = NULL;


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
      sprintf(COMMON_buffer, "%.*s." SHARED_LIBRARY_EXT ".%s", p - name, name, p + 1);
      
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

  *((long long *)addr) = value->_long.value;
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

#define CAST(_type, _func) (*((_type (*)())_func))

PUBLIC void EXTERN_call(void)
{
  CLASS_EXTERN *ext = &EXEC.class->load->ext[EXEC.index];
  int nparam = EXEC.nparam;
  int i, sz;
  VALUE *value;
  TYPE *sign;
  ARGS args;
  int *parg, *marg;
  void *func;
  
  if (nparam < ext->n_param)
    THROW(E_NEPARAM);
  if (nparam > ext->n_param)
    THROW(E_TMPARAM);
  
  if (nparam)
  {
    value = &SP[-nparam];
    sign = (TYPE *)ext->param;
    parg = (int *)&args;
    marg = parg + sizeof(ARGS) / sizeof(int);
  
    for (i = 0, sz = 0; i < nparam; i++, value++, sign++)
    {
      VALUE_conv(value, *sign);
      if (parg >= marg)
        THROW(E_TMPARAM);      
      parg += put_arg(parg, value);
    }
  }
  
  func = get_function(ext);
  
  switch (ext->type)
  {
    case T_BOOLEAN:
    case T_BYTE:
    case T_SHORT:
    case T_INTEGER:
      GB_ReturnInteger(CAST(int, func)(args));
      break;
    
    case T_LONG:
      GB_ReturnLong(CAST(long long, func)(args));
      break;
    
    case T_SINGLE:
      GB_ReturnFloat(CAST(float, func)(args));
      break;
      
    case T_FLOAT:
      GB_ReturnFloat(CAST(double, func)(args));
      break;
      
    case T_STRING:
      GB_ReturnConstString(CAST(char *, func)(args), 0);
      break;
    
    default:
      CAST(void, func)(args);
      GB_ReturnNull();
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


PUBLIC void EXTERN_exit(void)
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
