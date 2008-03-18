/***************************************************************************

  gbx_api.c

  Gambas API for external libraries

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

#define __GBX_API_C

#include "gb_common.h"
#include "gb_common_case.h"
#include "gb_error.h"
#include "gb_alloc.h"

#include <stdarg.h>

#include "gbx_class.h"
#include "gbx_exec.h"
#include "gbx_event.h"
#include "gbx_stack.h"
#include "gbx_stream.h"
#include "gbx_library.h"
#include "gbx_watch.h"
#include "gbx_project.h"
#include "gbx_eval.h"
#include "gbx_local.h"
#include "gb_hash.h"
#include "gb_file.h"
#include "gbx_number.h"
#include "gbx_object.h"
#include "gbx_string.h"
#include "gbx_date.h"
#include "gbx_print.h"
#include "gbx_regexp.h"
#include "gbx_c_array.h"
#include "gbx_c_timer.h"
#include "gbx_component.h"
#include "gbx_c_gambas.h"

#include "gambas.h"
#include "gbx_api.h"

typedef
  struct {
    OBJECT *object;
    CLASS_DESC_METHOD *desc;
    }
  GB_API_FUNCTION;

void *GAMBAS_Api[] =
{
  (void *)GB_VERSION,

  (void *)GB_GetInterface,

  (void *)GB_Hook,

  (void *)GB_LoadComponent,
  (void *)COMPONENT_find,
  (void *)GB_CurrentComponent,
  (void *)COMPONENT_get_info,

  (void *)GB_Push,
  (void *)GB_GetFunction,
  (void *)GB_Call,
  (void *)GB_GetClassInterface,

  (void *)WATCH_one_loop,
  (void *)EVENT_post,
  (void *)EVENT_post2,
  (void *)GB_Raise,
  (void *)EVENT_post_event,
  (void *)EVENT_check_post,
  (void *)GB_CanRaise,
  (void *)GB_GetEvent,
  (void *)GB_GetLastEventName,
  (void *)CTIMER_raise,
  (void *)GB_Stopped,
  (void *)COMPONENT_signal,

  (void *)GB_NParam,
  (void *)GB_Conv,
  (void *)GB_GetUnknown,
  (void *)GB_IsProperty,

  (void *)GB_Error,
  (void *)ERROR_propagate,

  (void *)GB_GetClass,
  (void *)GB_GetClassName,
  (void *)GB_ExistClass,
  (void *)CLASS_find_global,
  (void *)GB_ExistClassLocal,
  (void *)CLASS_find,
  (void *)GB_Is,
  (void *)GB_Ref,
  (void *)GB_Unref,
  (void *)GB_UnrefKeep,
  (void *)GB_Detach,
  (void *)GB_Attach,
  (void *)OBJECT_parent,
  (void *)GB_New,
  (void *)CLASS_auto_create,
  (void *)GB_CheckObject,

  (void *)GB_GetEnum,
  (void *)GB_StopEnum,
  (void *)GB_ListEnum,
  (void *)GB_NextEnum,
  (void *)GB_StopAllEnum,

  (void *)GB_Return,
  (void *)GB_ReturnInteger,
  (void *)GB_ReturnLong,
  (void *)GB_ReturnPointer,
  (void *)GB_ReturnBoolean,
  (void *)GB_ReturnDate,
  (void *)GB_ReturnObject,
  (void *)GB_ReturnNull,
  (void *)GB_ReturnFloat,
  (void *)GB_ReturnPtr,
  (void *)GB_ReturnSelf,

  (void *)GB_ReturnString,
  (void *)GB_ReturnConstString,
  (void *)GB_ReturnConstZeroString,
  (void *)GB_ReturnNewString,
  (void *)GB_ReturnNewZeroString,

  (void *)STRING_new,
  (void *)STRING_new_temp,
  (void *)GB_FreeString,
  (void *)STRING_extend,
  (void *)STRING_add,
  (void *)GB_StringLength,
  (void *)GB_ToZeroString,
  (void *)REGEXP_match,
  (void *)NUMBER_from_string,
  (void *)GB_NumberToString,
  (void *)LOCAL_gettext,
  
  (void *)STRING_subst,
  (void *)SUBST_add,
  (void *)GB_ConvString,
  (void *)STRING_conv_file_name,
  (void *)GB_RealFileName,

  (void *)GB_LoadFile,
  (void *)GB_ReleaseFile,
  (void *)FILE_exist,
  (void *)GB_GetTempDir,

  (void *)GB_Store,
  (void *)GB_StoreString,
  (void *)GB_StoreObject,
  (void *)GB_StoreVariant,

  (void *)DATE_split,
  (void *)DATE_make,
  (void *)DATE_from_time,
  (void *)DATE_timer,

  (void *)GB_Watch,

  (void *)GB_Eval,

  (void *)GB_Alloc,
  (void *)GB_Free,
  (void *)GB_Realloc,

  (void *)GB_NewArray,
  (void *)ARRAY_delete,
  (void *)GB_CountArray,
  (void *)GB_Add,
  (void *)ARRAY_insert_many,
  (void *)ARRAY_remove_many,

  (void *)GB_PrintData,
  (void *)PRINT_string,

  (void *)GB_SubCollectionNew,
  (void *)GB_SubCollectionAdd,
  (void *)GB_SubCollectionRemove,
  (void *)GB_SubCollectionGet,
  (void *)GB_SubCollectionContainer,

  (void *)GB_tolower,
  (void *)GB_toupper,
  (void *)strcasecmp,
  (void *)strncasecmp,

  (void *)GB_AppName,
  (void *)GB_AppTitle,
  (void *)GB_AppVersion,
  (void *)GB_AppPath,
  (void *)GB_AppStartup,

  (void *)GB_SystemCharset,
  (void *)LOCAL_get_lang,
  (void *)GB_IsRightToLeft,

  (void *)GB_ArrayNew,
  (void *)GB_ArrayCount,
  (void *)GB_ArrayAdd,
  (void *)GB_ArrayGet,

  (void *)GB_CollectionNew,
  (void *)GB_CollectionCount,
  (void *)GB_CollectionSet,
  (void *)GB_CollectionGet,

  (void *)GB_HashTableNew,
  (void *)HASH_TABLE_delete,
  (void *)HASH_TABLE_size,
  (void *)GB_HashTableAdd,
  (void *)GB_HashTableRemove,
  (void *)GB_HashTableGet,
  (void *)GB_HashTableEnum,

  (void *)GB_StreamInit,

  (void *)GB_ImageCreate,
  (void *)GB_ImageInfo,
  (void *)GB_ImageConvert,
  (void *)GB_PictureCreate,
  (void *)GB_PictureInfo,

  NULL
};

void *GAMBAS_DebugApi[] =
{
	(void *)GB_DebugGetExec,
	(void *)STACK_get_frame,
	(void *)ERROR_print_at,
	(void *)ERROR_save,
	(void *)ERROR_restore,
	(void *)VALUE_to_string,
	(void *)LOCAL_format_date,
	(void *)LOCAL_format_number,
	(void *)DEBUG_get_value,
	(void *)CARRAY_get_value,
	(void *)GB_CollectionEnum,
	(void *)CLASS_get_next_sorted_symbol,
	(void *)DEBUG_can_be_used_like_an_array,
	NULL
};


bool GAMBAS_Error = FALSE;
bool GAMBAS_DoNotRaiseEvent = FALSE;
bool GAMBAS_StopEvent = FALSE;

static bool _event_stopped = FALSE;

int GB_GetInterface(const char *name, int version, void *iface)
{
  if (LIBRARY_get_interface_by_name(name, version, iface))
    ERROR_panic("Cannot find interface of library '%s'", name);

  return FALSE;
}


void *GB_Hook(int type, void *hook)
{
  void *old_hook;
  void **phook = (void **)(void *)&EXEC_Hook;

  if ((type < GB_HOOK_MAIN) || (type > GB_HOOK_PICTURE))
    return NULL;

  type--;
  old_hook = phook[type];
  phook[type] = hook;

  return old_hook;
}


int GB_LoadComponent(const char *name)
{
  int ret = 0;

  TRY
  {
    COMPONENT *comp = COMPONENT_create(name);
    COMPONENT_load(comp);
  }
  CATCH
  {
    ret = 1;
    GAMBAS_Error = TRUE;
  }
  END_TRY

  return ret;
}


static void push(int nval, va_list args)
{
  TYPE type;

  STACK_check(nval);

  while (nval)
  {
    type = va_arg(args, int);
    SP->type = type;

    switch(type)
    {
      case T_INTEGER:
      case T_BOOLEAN:
        SP->_integer.value = va_arg(args, int);
        break;

      case T_LONG:
        SP->_long.value = va_arg(args, int64_t);
        break;

      case T_STRING:
        SP->type = T_CSTRING;
        SP->_string.addr = va_arg(args, char *);
        SP->_string.start = 0;
        SP->_string.len = va_arg(args, int);
        if (SP->_string.len <= 0 && SP->_string.addr)
          SP->_string.len = strlen(SP->_string.addr);
        break;

      case T_FLOAT:
        SP->_float.value = va_arg(args, double);
        break;

      case T_OBJECT:
        SP->_object.object = va_arg(args, void *);
        OBJECT_REF(SP->_object.object, "push");
        break;

      default:
        ERROR_panic("GB.Push: unknown datatype");
        break;
    }

    SP++;
    nval--;
  }
}


void GB_Push(int nval, ...)
{
  va_list args;

  va_start(args, nval);
  push(nval, args);
  va_end(args);
}


static void call_method(void *object, CLASS_DESC_METHOD *desc, int nparam)
{
  if (OBJECT_is_class(object))
  {
    EXEC.object = NULL;
    //EXEC.class = (CLASS *)object;
  }
  else
  {
    EXEC.object = object;
    //EXEC.class = OBJECT_class(object);
  }

  EXEC.class = desc->class;
  EXEC.nparam = nparam; /*desc->npmin;*/
  EXEC.drop = FALSE;

  if (FUNCTION_is_native(desc))
  {
    EXEC.native = TRUE;
    EXEC.use_stack = FALSE;
    EXEC.desc = desc;
    EXEC_native();
    SP--;
    *RP = *SP;
    SP->type = T_VOID;
  }
  else
  {
    EXEC.native = FALSE;
    EXEC.index = (int)(intptr_t)desc->exec;
    EXEC_function_keep();
  }

}



int GB_CanRaise(void *object, int event_id)
{
  ushort *event_tab;
  int func_id;
  COBSERVER *obs;

  if (object == NULL)
    return FALSE;

	LIST_for_each(obs, OBJECT_event(object)->observer)
	{
		if (OBJECT_active_parent(obs) && obs->event && obs->event[event_id])
	  	return TRUE;
	}
  
  if (!OBJECT_active_parent(object))
  	return FALSE;
  
  event_tab = OBJECT_event(object)->event;
  func_id = event_tab[event_id];

  return (func_id != 0);
}


static int get_event_func_id(ushort *event_tab, int event_id)
{
	int func_id;
	
	if (!event_tab)
		return 0;
		
	func_id = event_tab[event_id];
	if (!func_id)
		return 0;
		
	return func_id;
}

static bool raise_event(OBJECT *observer, void *object, int func_id, int nparam)
{
  bool stop_event;
	CLASS *class;
  CLASS_DESC_METHOD *desc;
  void *old_last;
  bool result;
	
	class = OBJECT_class(object);	
	func_id--;

	if (OBJECT_is_class(observer))
		desc = &(((CLASS *)observer)->table[func_id].desc->method);
	else
		desc = &(observer->class->table[func_id].desc->method);

	old_last = EVENT_Last;
	EVENT_Last = object;
	
// 	if (arg)
// 	{
// 		EXEC_dup(nparam);
// 	}
// 	else
// 	{
// 		va_start(args, nparam);
// 		push(nparam, args);
// 		va_end(args);
// 	}

	stop_event = GAMBAS_StopEvent;	
	GAMBAS_StopEvent = FALSE;

	call_method(observer, desc, nparam);

	if (RP->type == T_VOID)
		result = FALSE;
	else
		result = RP->_boolean.value != 0;

	if (GAMBAS_StopEvent)
		result = TRUE;
		
	GAMBAS_StopEvent = stop_event;
	EVENT_Last = old_last;

	EXEC_release_return_value();

	return result;
}

// If nparam < 0, the args are already on the stack

int GB_Raise(void *object, int event_id, int nparam, ...)
{
  OBJECT *parent;
  int func_id;
  int result;
  va_list args;
  bool arg;
  COBSERVER *obs;
  bool after = FALSE;

  /*MEMORY_check_ptr(object);*/

  if (GAMBAS_DoNotRaiseEvent)
    return FALSE;

  if (object == NULL)
    return FALSE;

	OBJECT_REF(object, "GB_Raise");

	arg = nparam < 0;
	nparam = abs(nparam);

	if (!arg)
	{
		va_start(args, nparam);
		push(nparam, args);
		va_end(args);
	}

	result = FALSE;

	// Observers before
	
	LIST_for_each(obs, OBJECT_event(object)->observer)
	{
		parent = OBJECT_active_parent(obs);
		if (!parent)
			continue;
		if (obs->after)
		{
			after = TRUE;
			continue;
		}
			
		func_id = get_event_func_id(obs->event, event_id);
		if (!func_id)
			continue;
		
		EXEC_dup(nparam);
		result = raise_event(parent, object, func_id, nparam);
		
		if (result)
			goto __RETURN;
	}
			
	// Parent

	parent = OBJECT_active_parent(object);
	if (parent)
	{	
		func_id = get_event_func_id(OBJECT_event(object)->event, event_id);
		if (func_id)
		{
			#if DEBUG_EVENT
				class = OBJECT_class(object);
				printf("GB_Raise(%p, %d, %s)\n", object, event_id, class->event[event_id].name);
				printf("func_id = %d  parent = (%s %p)\n", func_id, parent->class->name, parent);
				if (OBJECT_is_locked(parent))
					printf("parent is locked!\n");
				fflush(NULL);
			#endif
		
			if (parent->class->check && (*parent->class->check)(parent))
				OBJECT_detach(object);
			else
			{
				EXEC_dup(nparam);
				result = raise_event(parent, object, func_id, nparam);
				if (result)
					goto __RETURN;
			}
		}
	}
  	
	// Observers after
	
	LIST_for_each(obs, OBJECT_event(object)->observer)
	{
		parent = OBJECT_active_parent(obs);
		if (!parent)
			continue;
		if (!obs->after)
			continue;
			
		func_id = get_event_func_id(obs->event, event_id);
		if (!func_id)
			continue;
		
		EXEC_dup(nparam);
		result = raise_event(parent, object, func_id, nparam);
		if (result)
			goto __RETURN;
	}
	
__RETURN:
	
	RELEASE_MANY(SP, nparam);
	OBJECT_UNREF(&object, "GB_Raise");	
	return result;
}


int GB_GetFunction(GB_FUNCTION *_func, void *object, const char *name, const char *sign, const char *type)
{
  GB_API_FUNCTION *func = (GB_API_FUNCTION *)_func;
  char len_min, nparam, npvar;
  TYPE *tsign;
  TYPE tret;
  int index;
  CLASS *class;
  int kind;
  CLASS_DESC *desc;
  bool error;

  if (OBJECT_is_class(object))
  {
    class = (CLASS *)object;
    kind = CD_STATIC_METHOD;
  }
  else
  {
    class = OBJECT_class(object);
    kind = CD_METHOD;
  }

	CLASS_load(class);
	
  index = CLASS_find_symbol(class, name);
  if (index == NO_SYMBOL)
    goto _NOT_FOUND;

  desc = class->table[index].desc;
  if (CLASS_DESC_get_type(desc) != kind)
    goto _NOT_FOUND;

  if (sign)
  {
    TYPE_signature_length(sign, &len_min, &nparam, &npvar);
    tsign = NULL;

    if (nparam)
    {
      ALLOC(&tsign, nparam * sizeof(TYPE), "GB_GetFunction");
      tsign = TYPE_transform_signature(&tsign, sign, nparam);
    }

    error = TYPE_compare_signature(desc->method.signature, desc->method.npmax, tsign, nparam);

		if (nparam)
			FREE(&tsign, "GB_GetFunction");

    if (error)
    {
      GB_Error("Parameters do not match");
      goto _NOT_FOUND;
    }
  }

	if (type)
	{
  	tret = TYPE_from_string(&type);
  	if (tret != desc->method.type)
  	{
  		if (tret == T_VOID)
  			GB_Error("Must be a procedure");
			else if (desc->method.type == T_VOID)
  			GB_Error("Must be a function");
			else
				GB_Error("Return type does not match");

			goto _NOT_FOUND;
  	}
	}

  func->object = object;
  func->desc = &desc->method;

  if (!func->desc)
  	abort();

  return 0;

_NOT_FOUND:

  func->object = NULL;
  func->desc = NULL;
  return 1;
}

void *GB_GetClassInterface(void *_class, const char *_name)
{
  CLASS_DESC *desc;
	int index;
	CLASS *class = (CLASS *)_class;
	int len = strlen(_name);
	char name[len + 4];
	
	CLASS_load(class);
	
	strcpy(name, "_@");
	strcat(name, _name);

  index = CLASS_find_symbol(class, name);
  if (index == NO_SYMBOL)
    goto __NOT_FOUND;

  desc = class->table[index].desc;
  if (CLASS_DESC_get_type(desc) != CD_CONSTANT)
    goto __NOT_FOUND;

	if (desc->constant.type != T_POINTER)
		goto __NOT_FOUND;

	return desc->constant.value._pointer;

__NOT_FOUND:
	return NULL;
}

GB_VALUE *GB_Call(GB_FUNCTION *_func, int nparam, int release)
{
  GB_API_FUNCTION *func = (GB_API_FUNCTION *)_func;
  bool stop_event;

	if (!func->desc)
		GB_Error("Unknown function call");
	else
	{
		stop_event = GAMBAS_StopEvent;
		call_method(func->object, func->desc, nparam);
		_event_stopped = GAMBAS_StopEvent;
		GAMBAS_StopEvent = stop_event;
		
		if (release)
			EXEC_release_return_value();
		else
		{
			UNBORROW(RP);
			TEMP = *RP;
			RP->type = T_VOID;
		}
	}

  return (GB_VALUE *)&TEMP;
}


int GB_GetEvent(void *class, char *name)
{
  CLASS_DESC_EVENT *cd;

  cd = CLASS_get_event_desc((CLASS *)class, name);
  if (!cd)
    return (-1);
  else
    return *cd->index;
}


char *GB_GetLastEventName()
{
	return EVENT_Name;
}


int GB_Stopped(void)
{
	return _event_stopped;
}


int GB_NParam(void)
{
  return EXEC.nparvar;
}


int GB_IsProperty(void)
{
  return EXEC.property;
}


const char *GB_GetUnknown(void)
{
  return EXEC.unknown;
}


void GB_Error(const char *error, ...)
{
  va_list args;
  char *arg[8];
  int i;

  if (!error)
  {
    GAMBAS_Error = FALSE;
    return;
  }

  va_start(args, error);

  for (i = 0; i < 8; i++)
    arg[i] = va_arg(args, char *);

  ERROR_define(error, arg);

  GAMBAS_Error = TRUE;
}



void GB_Ref(void *object)
{
  #if TRACE_MEMORY
  CLASS *save = CP;
  CP = NULL;
  #endif

  if (object)
    OBJECT_REF(object, "GB_Ref");

  #if TRACE_MEMORY
  CP = save;
  #endif
}


void GB_Unref(void **object)
{
  #if TRACE_MEMORY
  CLASS *save = CP;
  CP = NULL;
  #endif

  if (*object)
    OBJECT_UNREF(object, "GB_Unref");

  #if TRACE_MEMORY
  CP = save;
  #endif
}


void GB_UnrefKeep(void **object, int delete)
{
  #if TRACE_MEMORY
  CLASS *save = CP;
  CP = NULL;
  #endif

  if (*object != NULL)
  {
    if (delete)
    {
      OBJECT_UNREF(object, "GB_UnrefKeep");
    }
    else
    {
      OBJECT_UNREF_KEEP(object, "GB_UnrefKeep");
    }
  }

  #if TRACE_MEMORY
  CP = save;
  #endif
}


void GB_Detach(void *object)
{
  if (object)
    OBJECT_detach(object);
}


void GB_Attach(void *object, void *parent, const char *name)
{
  if (object)
    OBJECT_attach(object, parent, name);
}


void GB_StopEnum(void)
{
  /* Do not forget than event if we stop the enumeration, the return value
     of _next will be converted
  */
  //VALUE_default(&TEMP, *GAMBAS_ReturnType);
  TEMP.type = T_VOID;
  EXEC_enum->stop = TRUE;
}


void *GB_GetEnum(void)
{
  return (void *)&EXEC_enum->data;
}


static void *_enum_object;

void GB_ListEnum(void *enum_object)
{
  EXEC_enum = NULL;
  _enum_object = enum_object;
}


int GB_NextEnum(void)
{
  for(;;)
  {
    EXEC_enum = CENUM_get_next(EXEC_enum);
    if (!EXEC_enum)
      return TRUE;
    if (EXEC_enum->enum_object == _enum_object)
      return FALSE;
  }
}

void GB_StopAllEnum(void *enum_object)
{
  GB_ListEnum(enum_object);
  while (!GB_NextEnum())
    GB_StopEnum();
}


void GB_Return(unsigned int type, ...)
{
  static void *jump[16] = {
    &&__VOID, &&__BOOLEAN, &&__BYTE, &&__SHORT, &&__INTEGER, &&__LONG, &&__SINGLE, &&__FLOAT, &&__DATE,
    &&__STRING, &&__STRING, &&__VARIANT, &&__ARRAY, &&__FUNCTION, &&__CLASS, &&__NULL
    };

  VALUE *ret = &TEMP;
  va_list args;

  va_start(args, type);

  ret->type = type;
  if (TYPE_is_object(type))
    goto __OBJECT;
  else
    goto *jump[type];

__BOOLEAN:

  ret->_integer.value = va_arg(args, int) ? (-1) : 0;
  goto __CONV;

__BYTE:

  ret->_integer.value = va_arg(args, int);
  goto __CONV;

__SHORT:

  ret->_integer.value = va_arg(args, int);
  goto __CONV;

__INTEGER:

  ret->_integer.value = va_arg(args, int);
  goto __CONV;

__LONG:

  ret->_long.value = va_arg(args, int64_t);
  goto __CONV;

__SINGLE:

  ret->_single.value = va_arg(args, double);
  goto __CONV;

__FLOAT:

  ret->_float.value = va_arg(args, double);
  goto __CONV;

__DATE:

  ret->_date.date = va_arg(args, int);
  ret->_date.time = va_arg(args, int);
  goto __CONV;

__OBJECT:

  ret->_object.object = va_arg(args, void *);
  goto __CONV;

__CLASS:
  ret->_class.class = va_arg(args, CLASS *);
  goto __CONV;

__CONV:
__STRING:
__VOID:
__VARIANT:
__ARRAY:
__FUNCTION:
__NULL:
  return;
}


void GB_ReturnInteger(int val)
{
  GB_Return(T_INTEGER, val);
}


void GB_ReturnLong(int64_t val)
{
  GB_Return(T_LONG, val);
}


void GB_ReturnPointer(void *val)
{
	GB_Return(T_POINTER, (intptr_t)val);
}


void GB_ReturnFloat(double val)
{
  GB_Return(T_FLOAT, val);
}


void GB_ReturnDate(GB_DATE *date)
{
  TEMP = *((VALUE *)date);
  TEMP.type = T_DATE;
}


void GB_ReturnBoolean(int val)
{
  GB_Return(T_BOOLEAN, val);
}


void GB_ReturnObject(void *val)
{
  if (val == NULL)
    GB_ReturnNull();
  else
    GB_Return(T_OBJECT, val);
}


void GB_ReturnPtr(unsigned int type, void *value)
{
  if (type == T_VOID)
    return;

	if (!value)
		VALUE_default(&TEMP, type);
	else
  	VALUE_read(&TEMP, value, type);
}


void GB_ReturnSelf(void *object)
{
  if (object)
    GB_ReturnObject(object);
  else
    GB_Return(T_CLASS, NULL);
}


char *GB_ToZeroString(GB_STRING *src)
{
  char *str;

  STRING_new_temp(&str, src->value.addr + src->value.start, src->value.len);

  if (str == NULL)
    return "";
  else
    return str;
}


void GB_ReturnString(char *str)
{
  TEMP.type = T_STRING;
  TEMP._string.addr = str;
  TEMP._string.start = 0;
  TEMP._string.len = STRING_length(str);

  if (TEMP._string.len == 0)
    TEMP._string.addr = 0;
}


void GB_ReturnConstString(const char *str, int len)
{
  TEMP.type = T_CSTRING;
  TEMP._string.addr = (char *)str;
  TEMP._string.start = 0;
  TEMP._string.len = len;

  if (TEMP._string.len == 0)
    TEMP._string.addr = 0;
}


void GB_ReturnConstZeroString(const char *str)
{
  int len;

  if (str)
    len = strlen(str);
  else
    len = 0;

  GB_ReturnConstString(str, len);
}


void GB_ReturnNewString(const char *src, int len)
{
  char *str;

  STRING_new_temp(&str, src, len);
  GB_ReturnString(str);
}


void GB_ReturnNewZeroString(const char *src)
{
  GB_ReturnNewString(src, 0);
}


void GB_ReturnNull(void)
{
  TEMP.type = T_NULL;
}


void *GB_GetClass(void *object)
{
  if (object)
    return OBJECT_class(object);
  else
    return EXEC.class;
}


char *GB_GetClassName(void *object)
{
  CLASS *class = GB_GetClass(object);
  return class->name;
}


int GB_Is(void *object, void *class)
{
  CLASS *ob_class;

  if (!object)
    return FALSE;

  ob_class = OBJECT_class(object);

  return ((ob_class == class) || CLASS_inherits(ob_class, class));
}


int GB_LoadFile(const char *path, int lenp, char **addr, int *len)
{
  int ret = 0;

  //fprintf(stderr, "GB_LoadFile: %.*s\n", lenp ? lenp : strlen(path), path);

  TRY
  {
    *addr = 0;

    STREAM_map(STRING_conv_file_name(path, lenp), addr, len);
  }
  CATCH
  {
    if (*addr)
      GB_ReleaseFile(addr, *len);

    GAMBAS_Error = TRUE;
    ret = 1;
  }
  END_TRY

  return ret;
}

void GB_ReleaseFile(char **addr, int len)
{
  //fprintf(stderr, "GB_ReleaseFile: ");
  if (ARCHIVE_check_addr(*addr))
  {
    //fprintf(stderr, "free\n");
    FREE(addr, "GB_ReleaseFile");
  }
  //else
    //fprintf(stderr, "unmap\n");

}


void GB_Store(GB_TYPE type, GB_VALUE *src, void *dst)
{
  if (src != NULL)
  {
    /* Ne marche que parce que value->type == type apr� un VALUE_read()
       Sinon il y'aurait des probl�es de r��ences - VALUE_write faisant
       appel �VALUE_conv() (ceci est un ancien commentaire)

       => src->type doit �re �al �type
    */
    VALUE_write((VALUE *)src, dst, type);
  }
  else
  {
    VALUE_free(dst, type);
  }
}


void GB_StoreString(GB_STRING *src, char **dst)
{
  char *str;

  STRING_unref(dst);
  if (src)
  {
    STRING_new(&str, src->value.addr + src->value.start, src->value.len);
    *dst = str;
  }
  else
    *dst = NULL;
}

void GB_StoreObject(GB_OBJECT *src, void **dst)
{
  void *object;

  if (src)
    object = src->value;
  else
    object = NULL;

  if (object)
    GB_Ref(object);

  GB_Unref(dst);
  *dst = object;
}

void GB_StoreVariant(GB_VARIANT *src, void *dst)
{
  /*GB_Store(GB_T_VARIANT, (GB_VALUE *)src, dst);*/
  if (src)
  {
    VALUE_write((VALUE *)src, dst, T_VARIANT);

    /*VARIANT_keep((VARIANT *)&src->value);
    VARIANT_free((VARIANT *)dst);
    *((VARIANT *)dst) = *((VARIANT *)&src->value);*/
  }
  else
  {
    VARIANT_free((VARIANT *)dst);
    ((VARIANT *)dst)->type = T_NULL;
  }
}



void GB_Watch(int fd, int flag, void *callback, int param)
{
  HOOK_DEFAULT(watch, WATCH_watch)(fd, flag, callback, param);
}


int GB_New(void **object, void *class, const char *name, void *parent)
{
  if (name && !parent)
  {
    parent = OP;
    if (!parent)
      parent = CP;
  }

  if (!((CLASS *)class)->no_create)
    OBJECT_create(object, class, name, parent, 0);
  else
  {
    OBJECT_new(object, class, name, parent);
    OBJECT_UNREF_KEEP(object, "GB_New");
  }

  return FALSE;
}


int GB_CheckObject(void *object)
{
  CLASS *class;
  
  if (object == NULL)
  {
    GB_Error((char *)E_NULL);
    return TRUE;
  }

  class = OBJECT_class(object);

  if (class->check && (*(class->check))(object))
  {
    GB_Error((char *)E_IOBJECT);
    return TRUE;
  }

  return FALSE;
}


const char *GB_AppName(void)
{
  return PROJECT_name;
}

const char *GB_AppPath(void)
{
  return PROJECT_path;
}

const char *GB_AppTitle(void)
{
  return LOCAL_gettext(PROJECT_title);
}

const char *GB_AppVersion(void)
{
  return PROJECT_version;
}

const char *GB_AppStartup(void)
{
  return PROJECT_startup;
}


void *GB_Eval(void *expr, void *func)
{
  GAMBAS_Error = EVAL_expression((EXPRESSION *)expr, (EVAL_FUNCTION)func);
  if (GAMBAS_Error)
  	return NULL;
	else
		return &TEMP;
}


void GB_Alloc(void **addr, int len)
{
  ALLOC(addr, len, "GB_Alloc");
}

void GB_Free(void **addr)
{
  FREE(addr, "GB_Free");
}

void GB_Realloc(void **addr, int len)
{
  REALLOC(addr, len, "GB_Realloc");
}


int GB_Conv(GB_VALUE *arg, GB_TYPE type)
{
  int ret = 0;

  TRY
  {
    VALUE_conv((VALUE *)arg, (GB_TYPE)type);
  }
  CATCH
  {
    ret = 1;
    GAMBAS_Error = TRUE;
  }
  END_TRY

  return ret;
}


int GB_StringLength(const char *str)
{
  return STRING_length(str);
}


int GB_NumberToString(int local, double value, const char *format, char **str, int *len)
{
  return
    LOCAL_format_number
    (
      value,
      format ? LF_USER : LF_GENERAL_NUMBER,
      format,
      format ? strlen(format) : 0,
      str, len, local != 0
    );
}


void GB_HashTableNew(GB_HASHTABLE *hash, int mode)
{
  HASH_TABLE_create((HASH_TABLE **)hash, sizeof(void *), mode);
}

void GB_HashTableAdd(GB_HASHTABLE hash, const char *key, int len, void *data)
{
  if (len <= 0)
    len = strlen(key);

  *((void **)HASH_TABLE_insert((HASH_TABLE *)hash, key, len)) = data;
}

void GB_HashTableRemove(GB_HASHTABLE hash, const char *key, int len)
{
  if (len <= 0)
    len = strlen(key);

  HASH_TABLE_remove((HASH_TABLE *)hash, key, len);
}

int GB_HashTableGet(GB_HASHTABLE hash, const char *key, int len, void **data)
{
  void **pdata;

  if (len <= 0)
    len = strlen(key);

  pdata = (void **)HASH_TABLE_lookup((HASH_TABLE *)hash, key, len);
  if (pdata)
  {
    *data = *pdata;
    return 0;
  }
  else
    return 1;
}


void GB_HashTableEnum(GB_HASHTABLE hash, GB_HASHTABLE_ENUM_FUNC func)
{
  HASH_ENUM iter;
  void **data;

  CLEAR(&iter);

  for(;;)
  {
    data = (void **)HASH_TABLE_next((HASH_TABLE *)hash, &iter);
    if (!data)
      break;

    (*func)(*data);
  }
}

void GB_NewArray(void *pdata, int size, int count)
{
  ARRAY_create_with_size(pdata, size, 16);
  ARRAY_add_data(pdata, count, TRUE);
}


int GB_CountArray(void *data)
{
  return ARRAY_count(data);
}


void *GB_Add(void *pdata)
{
  return ARRAY_add_void(pdata);
}


void GB_FreeString(char **str)
{
  STRING_unref(str);
  *str = NULL;
}

bool GB_ConvString(char **result, const char *str, int len, const char *src, const char *dst)
{
  bool err = FALSE;

  TRY
  {
    STRING_conv(result, str, len, src, dst, TRUE);
  }
  CATCH
  {
    err = TRUE;
    GAMBAS_Error = TRUE;
  }
  END_TRY

  return err;
}


char *GB_SystemCharset(void)
{
  return LOCAL_encoding;
}

int GB_IsRightToLeft(void)
{
  return LOCAL_local.rtl;
}


void GB_StreamInit(GB_STREAM *stream, int fd)
{
  STREAM *s = (STREAM *)stream;

  s->type = &STREAM_direct;
  s->direct.fd = fd;
}

int GB_tolower(int c)
{
  return tolower(c);
}

int GB_toupper(int c)
{
  return toupper(c);
}

char *GB_GetTempDir(void)
{
  return FILE_make_temp(NULL, NULL);
}

char *GB_RealFileName(const char *name, int len)
{
  char *path = STRING_conv_file_name(name, len);
  char *real;
  char *temp;

  if (!STREAM_in_archive(path))
    return path;

  temp = FILE_make_temp(NULL, NULL);
  STRING_new_temp(&real, NULL, strlen(temp) + strlen(path) + strlen("/data/"));
  snprintf(real, strlen(temp) + strlen(path) + strlen("/data/"), "%s/data/%s", temp, path);

  if (!FILE_exist(real))
  {
    TRY
    {
      FILE_make_path_dir(real);
      FILE_copy(path, real);
    }
    CATCH
    {
      real = path;
    }
    END_TRY
  }

  //fprintf(stderr, "GB.RealFileName: %s -> %s\n", path, real);

  return real;
}

void GB_PrintData(GB_TYPE type, void *addr)
{
  VALUE value;

  VALUE_read(&value, addr, (TYPE)type);
  PRINT_value(&value);
}


const char *GB_CurrentComponent()
{
  ARCHIVE *arch;

  ARCHIVE_get_current(&arch);
  return arch->name ? arch->name : "";
}

int GB_ImageCreate(GB_IMAGE *image, void *data, int width, int height, int format)
{
	GB_IMAGE_INFO info;

	if (!EXEC_Hook.image)
	{
		GB_Error("No image provider");
		return TRUE;
	}

	if (image)
		*image = NULL;

	info.width = width;
	info.height = height;
	info.format = format;
	info.data = data;

	return (*EXEC_Hook.image)(image, &info);
}

void GB_ImageInfo(GB_IMAGE image, GB_IMAGE_INFO *info)
{
	if (!EXEC_Hook.image)
	{
		GB_Error("No image provider");
		return;
	}

	if (image)
		(*EXEC_Hook.image)(&image, info);
}

int GB_PictureCreate(GB_PICTURE *picture, void *data, int width, int height, int format)
{
	GB_PICTURE_INFO info;

	if (!EXEC_Hook.picture)
	{
		GB_Error("No picture provider");
		return TRUE;
	}

	info.width = width;
	info.height = height;
	info.format = format;
	info.data = data;

	return (*EXEC_Hook.picture)(picture, &info);
}

void GB_PictureInfo(GB_PICTURE picture, GB_PICTURE_INFO *info)
{
	if (!EXEC_Hook.image)
	{
		GB_Error("No image provider");
		return;
	}

	if (picture)
		(*EXEC_Hook.picture)(&picture, info);
}

void *GB_DebugGetExec(void)
{
	return &EXEC_current;
}


int GB_ExistClass(const char *name)
{
	return CLASS_look_global(name, strlen(name)) != NULL;
}


int GB_ExistClassLocal(const char *name)
{
	return CLASS_look(name, strlen(name)) != NULL;
}


void GB_ImageConvert(void *dst, int dst_format, void *src, int src_format, int w, int h)
{
	register char *s = (char *)src;
	register char *d = (char *)dst;
	register char *dm = &d[w * h * 4];

  if (dst_format == GB_IMAGE_BGRA || dst_format == GB_IMAGE_BGRX)
  {
    switch (src_format)
    {
      case GB_IMAGE_BGRA: case GB_IMAGE_BGRX: 
        goto __0123;
  
      case GB_IMAGE_ARGB: case GB_IMAGE_XRGB:
        goto __3210;
  
      case GB_IMAGE_RGBA: case GB_IMAGE_RGBX:
        goto __2103;
  
      case GB_IMAGE_ABGR: case GB_IMAGE_XBGR:
        goto __1230;
        
      case GB_IMAGE_BGR:
        goto __012X;
  
      case GB_IMAGE_RGB:
        goto __210X;
    }
  }
  else if (dst_format == GB_IMAGE_RGBA || dst_format == GB_IMAGE_RGBX)
  {
    switch (src_format)
    {
      case GB_IMAGE_RGBA: case GB_IMAGE_RGBX:
        goto __0123;
  
      case GB_IMAGE_ABGR: case GB_IMAGE_XBGR:
        goto __3210;
  
      case GB_IMAGE_BGRA: case GB_IMAGE_BGRX:
        goto __2103;
  
      case GB_IMAGE_ARGB: case GB_IMAGE_XRGB:
        goto __1230;
  
      case GB_IMAGE_RGB:
        goto __012X;
      
      case GB_IMAGE_BGR:
        goto __210X;  
    }  
  }
  
__0123:         

  memcpy(dst, src, w * h * 4);
  return;

__3210:

  while (d < dm)
  {
    d[0] = s[3];
    d[1] = s[2];
    d[2] = s[1];
    d[3] = s[0];
    s += 4;
    d += 4;
  }
  return;

__2103:

  while (d < dm)
  {
    d[0] = s[2];
    d[1] = s[1];
    d[2] = s[0];
    d[3] = s[3];
    s += 4;
    d += 4;
  }
  return;
  
__1230:

  while (d < dm)
  {
    d[0] = s[1];
    d[1] = s[2];
    d[2] = s[3];
    d[3] = s[0];
    s += 4;
    d += 4;
  }
  return;

__012X:

  while (d < dm)
  {
    d[0] = s[0];
    d[1] = s[1];
    d[2] = s[2];
    d[3] = 0xFF;
    s += 3;
    d += 4;
  }
  return;

__210X:

  while (d < dm)
  {
    d[0] = s[2];
    d[1] = s[1];
    d[2] = s[0];
    d[3] = 0xFF;
    s += 3;
    d += 4;
  }
  return;
}
