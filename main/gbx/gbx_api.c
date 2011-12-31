/***************************************************************************

  gbx_api.c

  (c) 2000-2012 Benoît Minisini <gambas@users.sourceforge.net>

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

#define __GBX_API_C

#include "gb_common.h"
#include "gb_common_case.h"
#include "gb_common_buffer.h"
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
#include "gbx_regexp.h"
#include "gbx_c_array.h"
#include "gbx_c_timer.h"
#include "gbx_component.h"
#include "gbx_c_gambas.h"
#include "gbx_c_observer.h"
#include "gbx_debug.h"
#include "gbx_c_file.h"

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
  (void *)COMPONENT_exist,
  (void *)GB_CurrentComponent,
  (void *)COMPONENT_get_info,

  (void *)GB_Push,
  (void *)GB_GetFunction,
  (void *)GB_Call,
  (void *)GB_GetClassInterface,
  (void *)GB_GetProperty,
  (void *)GB_SetProperty,

  (void *)WATCH_one_loop,
  (void *)EVENT_post,
  (void *)EVENT_post2,
  (void *)CTIMER_every,
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
  (void *)GB_ReturnVariant,
  (void *)GB_ReturnConvVariant,
  (void *)GB_ReturnPtr,
  (void *)GB_ReturnSelf,

  (void *)GB_ReturnString,
  (void *)GB_ReturnVoidString,
  (void *)GB_ReturnConstString,
  (void *)GB_ReturnConstZeroString,
  (void *)GB_ReturnNewString,
  (void *)GB_ReturnNewZeroString,

  (void *)STRING_new,
  (void *)GB_NewZeroString,
  (void *)GB_TempString,
  (void *)GB_FreeString,
  (void *)STRING_extend,
  (void *)STRING_add,
  (void *)STRING_add_char,
  (void *)GB_StringLength,
  (void *)GB_ToZeroString,
  (void *)REGEXP_match,
  (void *)NUMBER_from_string,
  (void *)GB_NumberToString,
  (void *)LOCAL_gettext,
  
  (void *)STRING_subst,
  (void *)STRING_subst_add,
  (void *)STRING_make,
  (void *)GB_ConvString,
  (void *)STRING_conv_file_name,
  (void *)GB_RealFileName,

  (void *)GB_LoadFile,
  (void *)STREAM_unmap,
  (void *)GB_TempDir,
  (void *)GB_TempFile,
  (void *)GB_CopyFile,
  (void *)GB_BrowseProject,

  (void *)GB_Store,
  (void *)GB_StoreString,
  (void *)GB_StoreObject,
  (void *)GB_StoreVariant,
	(void *)VALUE_read,
	(void *)GB_BorrowValue,
	(void *)GB_ReleaseValue,

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
  (void *)GB_AppStartupClass,

  (void *)GB_SystemCharset,
  (void *)LOCAL_get_lang,
  (void *)GB_SystemDomainName,
  (void *)GB_IsRightToLeft,
  (void *)GB_SystemPath,

  (void *)GB_ArrayNew,
  (void *)GB_ArrayCount,
  (void *)GB_ArrayAdd,
  (void *)GB_ArrayGet,
  (void *)GB_ArrayType,

  (void *)GB_CollectionNew,
  (void *)GB_CollectionCount,
  (void *)GB_CollectionSet,
  (void *)GB_CollectionGet,
  (void *)GB_CollectionEnum,

  (void *)GB_HashTableNew,
  (void *)HASH_TABLE_delete,
  (void *)HASH_TABLE_size,
  (void *)GB_HashTableAdd,
  (void *)GB_HashTableRemove,
  (void *)GB_HashTableGet,
  (void *)GB_HashTableEnum,

	(void *)GB_StreamGet,
  (void *)GB_StreamSetBytesRead,
  (void *)GB_StreamSetSwapping,
  (void *)GB_StreamBlock,
  (void *)GB_StreamRead,
  (void *)GB_StreamWrite,

  (void *)STRING_start_len,
  (void *)STRING_end,
  (void *)STRING_make,
  
  (void *)DEBUG_get_current_position,

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
	(void *)DEBUG_set_value,
	(void *)CARRAY_get_value,
	(void *)GB_CollectionEnum,
	(void *)CLASS_get_next_sorted_symbol,
	(void *)DEBUG_get_object_access_type,
	(void *)DEBUG_find_class,
	(void *)CARRAY_get_array_bounds,
	NULL
};


bool GAMBAS_DoNotRaiseEvent = FALSE;
bool GAMBAS_StopEvent = FALSE;

static bool _event_stopped = FALSE;

#define CATCH_ERROR \
  bool ret = FALSE; \
  TRY
  
#define END_CATCH_ERROR \
  CATCH \
  { \
    ret = TRUE; \
    EXEC_set_native_error(TRUE); \
  } \
  END_TRY \
  return ret;

#define CATCH_ERROR_INT \
  int ret = 0; \
  TRY
  
#define END_CATCH_ERROR_INT \
  CATCH \
  { \
    ret = -1; \
    EXEC_set_native_error(TRUE); \
  } \
  END_TRY \
  return ret;

bool GB_GetInterface(const char *name, int version, void *iface)
{
	GB_LoadComponent(name);
	
  if (LIBRARY_get_interface_by_name(name, version, iface))
    ERROR_panic("Cannot find interface of library '%s'", name);

  return FALSE;
}


void *GB_Hook(int type, void *hook)
{
  void *old_hook;
  void **phook = (void **)(void *)&EXEC_Hook;

  if ((type < 0) || (type > GB_HOOK_MAX))
    return NULL;

  type--;
  old_hook = phook[type];
  if (hook)
  	phook[type] = hook;

  return old_hook;
}


bool GB_LoadComponent(const char *name)
{
  CATCH_ERROR
  {
    COMPONENT *comp = COMPONENT_create(name);
    COMPONENT_load(comp);
  }
  END_CATCH_ERROR
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


void GB_GetProperty(void *object, const char *property)
{
	static bool init = FALSE;
  static GB_FUNCTION func;
	
	if (!init)
	{
		GB_GetFunction(&func, (void *)CLASS_find("Object"), "GetProperty", NULL, NULL);
		init = TRUE;
	}
	
	GB_Push(2, GB_T_OBJECT, object, GB_T_STRING, property, strlen(property));
	GB_Call(&func, 2, FALSE);
}

void GB_SetProperty(void *object, const char *property, ...)
{
	static bool init = FALSE;
  static GB_FUNCTION func;
	
  va_list args;
	
	if (!init)
	{
		GB_GetFunction(&func, (void *)CLASS_find("Object"), "SetProperty", NULL, NULL);
		init = TRUE;
	}
	
	GB_Push(2, GB_T_OBJECT, object, GB_T_STRING, property, strlen(property));

  va_start(args, property);
  push(1, args);
  va_end(args);
	
	GB_Call(&func, 3, TRUE);
}


bool GB_CanRaise(void *object, int event_id)
{
  ushort *event_tab;
  int func_id;
  COBSERVER *obs;

  if (!object || !OBJECT_has_events(object))
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
	
	func_id--;

	if (OBJECT_is_class(observer))
		class = (CLASS *)observer; //OBJECT_class(object);	
	else
		class = OBJECT_class(observer);

	desc = &class->table[func_id].desc->method;

	old_last = EVENT_Last;
	EVENT_Last = object;
	//OBJECT_REF(object, "raise_event");
	
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

	EXEC_public_desc(class, observer, desc, nparam);

	if (RP->type == T_VOID)
		result = FALSE;
	else
		result = RP->_boolean.value != 0;

	if (GAMBAS_StopEvent)
		result = TRUE;
		
	GAMBAS_StopEvent = stop_event;
	
	//OBJECT_UNREF(object, "raise_event");
	EVENT_Last = old_last;

	EXEC_release_return_value();

	return result;
}

// If nparam < 0, the args are already on the stack

static void *_GB_Raise_unref = NULL;
static int _GB_Raise_nparam = 0;

static void error_GB_Raise()
{
	RELEASE_MANY(SP, _GB_Raise_nparam);
	OBJECT_UNREF(_GB_Raise_unref, "error_GB_Raise");
}

bool GB_Raise(void *object, int event_id, int nparam, ...)
{
  OBJECT *parent;
  int func_id;
  int result;
  va_list args;
  bool arg;
  COBSERVER *obs;
	void *save_GB_Raise_unref;
	int save_GB_Raise_nparam;

  if (GAMBAS_DoNotRaiseEvent)
    return FALSE;

  if (!OBJECT_is_valid(object) || !OBJECT_has_events(object))
    return FALSE;

	save_GB_Raise_unref = _GB_Raise_unref;
	save_GB_Raise_nparam = _GB_Raise_nparam;
	
	ON_ERROR(error_GB_Raise)
	{
		OBJECT_REF(object, "GB_Raise");
		_GB_Raise_unref = object;
	
		arg = nparam < 0;
		nparam = abs(nparam);
	
		if (!arg)
		{
			va_start(args, nparam);
			push(nparam, args);
			va_end(args);
		}
	
		_GB_Raise_nparam = nparam;
		
		result = FALSE;
	
		// Observers before
		
		LIST_for_each(obs, OBJECT_event(object)->observer)
		{
			parent = OBJECT_active_parent(obs);
			if (!parent)
				continue;
			if (obs->after)
				continue;
			//if (obs->object != object)
			//	continue;
				
			func_id = get_event_func_id(obs->event, event_id);
			if (!func_id)
				continue;
			
			if (!OBJECT_is_valid(parent))
			{
				OBJECT_detach(object);
				continue;
			}
			
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
					CLASS *class = OBJECT_class(object);
					printf("GB_Raise(%p, %d, %s)\n", object, event_id, class->event[event_id].name);
					printf("func_id = %d  parent = (%s %p)\n", func_id, parent->class->name, parent);
					if (OBJECT_is_locked(parent))
						printf("parent is locked!\n");
					fflush(NULL);
				#endif
			
				if (!OBJECT_is_valid(parent))
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
			//if (obs->object != object)
			//	continue;
				
			func_id = get_event_func_id(obs->event, event_id);
			if (!func_id)
				continue;
			
			if (!OBJECT_is_valid(parent))
			{
				OBJECT_detach(object);
				continue;
			}
			
			EXEC_dup(nparam);
			result = raise_event(parent, object, func_id, nparam);
			if (result)
				goto __RETURN;
		}
	
__RETURN:
	
		RELEASE_MANY(SP, nparam);
		OBJECT_UNREF(object, "GB_Raise");	
	}
	END_ERROR

	_GB_Raise_unref = save_GB_Raise_unref;
	_GB_Raise_nparam = save_GB_Raise_nparam;

	return result;
}


bool GB_GetFunction(GB_FUNCTION *_func, void *object, const char *name, const char *sign, const char *type)
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
	const char *err;

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
	{
		err = "Symbol not found";
    goto _NOT_FOUND;
	}

  desc = class->table[index].desc;
  if (CLASS_DESC_get_type(desc) != kind)
	{
		err = kind == CD_METHOD ? "Not a method" : "Not a static method";
    goto _NOT_FOUND;
	}

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
      err = "Parameters do not match";
      goto _NOT_FOUND;
    }
  }

	if (type)
	{
  	tret = TYPE_from_string(&type);
  	if (tret != desc->method.type)
  	{
  		if (tret == T_VOID)
  			err = "Must be a procedure";
			else if (desc->method.type == T_VOID)
  			err = "Must be a function";
			else
				err = "Return type does not match";

			goto _NOT_FOUND;
  	}
	}

  func->object = object;
  func->desc = &desc->method;

  if (!func->desc)
  	abort();

  return FALSE;

_NOT_FOUND:

  GB_Error("Unable to find method &1 in class &2. &3", name, class->name, err);
  func->object = NULL;
  func->desc = NULL;
  return TRUE;
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

	if (!func || !func->desc)
		GB_Error("Unknown function call");
		//TEMP.type = GB_T_NULL;
	else
	{
		stop_event = GAMBAS_StopEvent;
		EXEC_public_desc(func->desc->class, func->object, func->desc, nparam);
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


bool GB_Stopped(void)
{
	return _event_stopped;
}


int GB_NParam(void)
{
  return EXEC_unknown_nparam;
}


/*bool GB_IsProperty(void)
{
  return FALSE; //EXEC_unknown_property;
}*/


const char *GB_GetUnknown(void)
{
  return EXEC_unknown_name;
}


void GB_Error(const char *error, ...)
{
  va_list args;
  char *arg[4];
  int i;

  if (!error)
  {
    EXEC_set_native_error(FALSE);
    return;
  }

  va_start(args, error);

  for (i = 0; i < 4; i++)
    arg[i] = va_arg(args, char *);

  ERROR_define(error, arg);
  EXEC_set_native_error(TRUE);
}

#if DEBUG_REF

#include <execinfo.h>

static void print_stack_backtrace()
{
	void *trace[16];
	char **messages = (char **)NULL;
	int i, trace_size = 0;

	trace_size = backtrace(trace, 16);
	messages = backtrace_symbols(trace, trace_size);
	//printf("[bt] Execution path:\n");
	for (i=0; i<trace_size; ++i)
		fprintf(stderr, "[bt] %s\n", messages[i]);
}

#endif


void GB_Ref(void *object)
{
  #if TRACE_MEMORY
  CLASS *save = CP;
  CP = NULL;
  #endif

  if (object)
	{
		#if DEBUG_REF
		//print_stack_backtrace();
		#endif
    OBJECT_REF(object, "GB_Ref");
	}

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
	{
		#if DEBUG_REF
		//print_stack_backtrace();
		#endif
    OBJECT_UNREF(*object, "GB_Unref");
	}
	
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
		#if DEBUG_REF
		//print_stack_backtrace();
		#endif
    if (delete)
    {
      OBJECT_UNREF(*object, "GB_UnrefKeep");
    }
    else
    {
      OBJECT_UNREF_KEEP(*object, "GB_UnrefKeep");
    }
  }

  #if TRACE_MEMORY
  CP = save;
  #endif
}


void GB_Detach(void *object)
{
  if (object)
	{
		#if DEBUG_REF
		//print_stack_backtrace();
		#endif
    OBJECT_detach(object);
	}
}


void GB_Attach(void *object, void *parent, const char *name)
{
  if (object)
	{
		#if DEBUG_REF
		//print_stack_backtrace();
		#endif
    OBJECT_attach(object, parent, name);
	}
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


bool GB_NextEnum(void)
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


void GB_Return(GB_TYPE type, ...)
{
  static void *jump[16] = {
    &&__VOID, &&__BOOLEAN, &&__BYTE, &&__SHORT, &&__INTEGER, &&__LONG, &&__SINGLE, &&__FLOAT, &&__DATE,
    &&__STRING, &&__STRING, &&__POINTER, &&__VARIANT, &&__FUNCTION, &&__CLASS, &&__NULL
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

__POINTER:

  ret->_pointer.value = va_arg(args, void *);
  goto __CONV;

__CLASS:
  ret->_class.class = va_arg(args, CLASS *);
  goto __CONV;

__CONV:
__STRING:
__VOID:
__VARIANT:
__FUNCTION:
__NULL:
  return;
}


void GB_ReturnInteger(int val)
{
	TEMP.type = T_INTEGER;
  TEMP._integer.value = val;
}


void GB_ReturnLong(int64_t val)
{
	TEMP.type = T_LONG;
	TEMP._long.value = val;
}


void GB_ReturnPointer(void *val)
{
	TEMP.type = T_POINTER;
	TEMP._pointer.value = val;
}


void GB_ReturnSingle(float val)
{
	TEMP.type = T_SINGLE;
  TEMP._single.value = val;
}

void GB_ReturnFloat(double val)
{
	TEMP.type = T_FLOAT;
  TEMP._float.value = val;
}


void GB_ReturnDate(GB_DATE *date)
{
	TEMP.type = T_DATE;
	if (date)
	{
		TEMP._date.date = date->value.date;
		TEMP._date.time = date->value.time;
	}
	else
	{
		TEMP._date.date = 0;
		TEMP._date.time = 0;
	}
}


void GB_ReturnBoolean(int val)
{
	TEMP.type = T_BOOLEAN;
  TEMP._boolean.value = val ? -1 : 0;
}


void GB_ReturnObject(void *val)
{
  if (val == NULL)
    GB_ReturnNull();
  else
	{
		TEMP.type = T_OBJECT; //OBJECT_class(val);
		TEMP._object.object = val;
	}
}


void GB_ReturnVariant(GB_VARIANT_VALUE *val)
{
	TEMP._variant.type = T_VARIANT;
	
	if (!val)
	{
		TEMP._variant.vtype = T_NULL;
	}
	else
	{
  	//VALUE_read(&TEMP, value, type);
		TEMP._variant.vtype = val->type;
		if (TEMP._variant.vtype == T_VOID)
			TEMP._variant.vtype = T_NULL;

		VARIANT_copy_value(&TEMP._variant, (VARIANT *)val);
	}
}


void GB_ReturnConvVariant(void)
{
	VALUE_conv(&TEMP, T_VARIANT);
}


void GB_ReturnPtr(GB_TYPE type, void *value)
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

  str = STRING_new_temp(src->value.addr + src->value.start, src->value.len);

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
	if (len <= 0)
	{
		if (*src)
		{
			ERROR_warning("please use GB.ReturnNewZeroString() instead of GB.ReturnNewString()");
			len = strlen(src);
		}
		else
		{
			GB_ReturnVoidString();
			return;
		}
	}

  GB_ReturnString(STRING_new_temp(src, len));
}


void GB_ReturnNewZeroString(const char *src)
{
  GB_ReturnString(STRING_new_temp_zero(src));
}


void GB_ReturnNull(void)
{
  TEMP.type = T_NULL;
}

void GB_ReturnVoidString(void)
{
  STRING_void_value(&TEMP);
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


bool GB_Is(void *object, void *class)
{
  CLASS *ob_class;

  if (!object)
    return FALSE;

  ob_class = OBJECT_class(object);

  return ((ob_class == class) || CLASS_inherits(ob_class, class));
}


bool GB_LoadFile(const char *path, int lenp, char **addr, int *len)
{
  bool ret = FALSE;

  //fprintf(stderr, "GB_LoadFile: %.*s\n", lenp ? lenp : strlen(path), path);

  TRY
  {
    *addr = 0;
    *len = 0;

    STREAM_map(STRING_conv_file_name(path, lenp), addr, len);
  }
  CATCH
  {
    if (*addr)
      STREAM_unmap(*addr, *len);

    EXEC_set_native_error(TRUE);
    ret = TRUE;
  }
  END_TRY

  return ret;
}


bool GB_ExistFile(const char *path)
{
	return FILE_exist(path);
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
  STRING_unref(dst);
  if (src)
  {
    *dst = STRING_new(src->value.addr + src->value.start, src->value.len);
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
    OBJECT_REF(object, "GB_StoreObject");

  OBJECT_UNREF(*dst, "GB_StoreObject");
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

void GB_BorrowValue(GB_VALUE *value)
{
	BORROW((VALUE *)value);
}

void GB_ReleaseValue(GB_VALUE *value)
{
	RELEASE((VALUE *)value);
}

void GB_Watch(int fd, int flag, void *callback, intptr_t param)
{
  HOOK_DEFAULT(watch, WATCH_watch)(fd, flag, callback, param);
}


void *GB_New(void *class, const char *name, void *parent)
{
	void *object;
	
  if (name && !parent)
  {
    parent = OP;
    if (!parent)
      parent = CP;
  }

  if (!((CLASS *)class)->no_create)
    object = OBJECT_create(class, name, parent, 0);
  else
  {
    object = OBJECT_new(class, name, parent);
    OBJECT_UNREF_KEEP(object, "GB_New");
  }
  
  return object;
}


bool GB_CheckObject(void *object)
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

void *GB_AppStartupClass(void)
{
  return PROJECT_class;
}


void *GB_Eval(void *expr, void *func)
{
	bool err = EVAL_expression((EXPRESSION *)expr, (EVAL_FUNCTION)func);
  
	EXEC_set_native_error(err);
  
	if (err)
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


bool GB_Conv(GB_VALUE *arg, GB_TYPE type)
{
  CATCH_ERROR
  {
    VALUE_conv((VALUE *)arg, (GB_TYPE)type);
  }
  END_CATCH_ERROR
}


int GB_StringLength(const char *str)
{
  return STRING_length(str);
}


bool GB_NumberToString(int local, double value, const char *format, char **str, int *len)
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

bool GB_HashTableGet(GB_HASHTABLE hash, const char *key, int len, void **data)
{
  void **pdata;

  if (len <= 0)
    len = strlen(key);

  pdata = (void **)HASH_TABLE_lookup((HASH_TABLE *)hash, key, len);
  if (pdata)
  {
    *data = *pdata;
    return FALSE;
  }
  else
    return TRUE;
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
  if (count)
		ARRAY_add_data(pdata, count, TRUE);
}


int GB_CountArray(void *data)
{
  return ARRAY_count(data);
}


void *GB_Add(void *pdata)
{
  return ARRAY_add_void_size(pdata);
}

char *GB_NewZeroString(char *src)
{
	return STRING_new_zero(src);
}

char *GB_TempString(char *src, int len)
{
  return STRING_new_temp(src, len);
}

void GB_FreeString(char **str)
{
  STRING_unref(str);
  *str = NULL;
}

bool GB_ConvString(char **result, const char *str, int len, const char *src, const char *dst)
{
  CATCH_ERROR
  {
    STRING_conv(result, str, len, src, dst, TRUE);
  }
  END_CATCH_ERROR
}


char *GB_SystemCharset(void)
{
  return LOCAL_is_UTF8 ? "UTF-8" : LOCAL_encoding;
}

char *GB_SystemDomainName(void)
{
	char host[256], domain[256];
	
	if (gethostname(host, sizeof(host) - 1))
		return "";
		
	if (getdomainname(domain, sizeof(domain) - 1))
		return "";
	
	if ((strlen(host) + strlen(domain)) > COMMON_BUF_MAX)
		return "";
	
	*COMMON_buffer = 0;
	if (*domain && strcmp(domain, "(none)"))
	{
		strcpy(COMMON_buffer, domain);
		strcat(COMMON_buffer, ".");
	}
	strcat(COMMON_buffer, host);
	
	return COMMON_buffer;
}

bool GB_IsRightToLeft(void)
{
  return LOCAL_local.rtl;
}

char *GB_SystemPath(void)
{
	return PROJECT_exec_path;
}

/*void GB_StreamInit(GB_STREAM *stream, int fd)
{
  STREAM *s = (STREAM *)stream;

  s->type = &STREAM_direct;
  s->direct.fd = fd;
}*/

GB_STREAM *GB_StreamGet(void *object)
{
	return (GB_STREAM *)CSTREAM_stream(object);
}

void GB_StreamSetBytesRead(GB_STREAM *stream, int length)
{
	STREAM_eff_read = length;
}

void GB_StreamSetSwapping(GB_STREAM *stream, int swap)
{
	((STREAM *)stream)->common.swap = swap;
}

bool GB_StreamBlock(GB_STREAM *stream, int block)
{
	STREAM *st = (STREAM *)stream;
	bool old = STREAM_is_blocking(st);
	STREAM_blocking(st, block);
	return old;
}

int GB_StreamRead(GB_STREAM *stream, void *addr, int len)
{
	CATCH_ERROR_INT
	{
		if (len < 0)
			ret = STREAM_read_max((STREAM *)stream, addr, -len);
		else
		{
			STREAM_read((STREAM *)stream, addr, len);
			ret = STREAM_eff_read;
		}
	}
	END_CATCH_ERROR_INT
}

int GB_StreamWrite(GB_STREAM *stream, void *addr, int len)
{
	CATCH_ERROR_INT
	{
		STREAM_write((STREAM *)stream, addr, len);
		ret = len;
	}
	END_CATCH_ERROR_INT
}

int GB_tolower(int c)
{
  return tolower(c);
}

int GB_toupper(int c)
{
  return toupper(c);
}

char *GB_TempDir(void)
{
  return FILE_make_temp(NULL, NULL);
}

char *GB_TempFile(const char *pattern)
{
	int len;
	return FILE_make_temp(&len, pattern);
}

bool GB_CopyFile(const char *src, const char *dst)
{
	CATCH_ERROR
	{
		FILE_copy(src, dst);
	}
	END_CATCH_ERROR
}

#if 0
int GB_FindFile(const char *dir, int recursive, int follow, void (*found)(const char *))
{
	int ret = 0;
	char *pattern;
	int len_pattern;
	char *str;
	
	TRY
	{
		if (recursive)
			FILE_recursive_dir(dir, found, NULL, 0, follow);
		else
		{
			FILE_dir_first(dir, NULL, 0);
			while (!FILE_dir_next(&pattern, &len_pattern))
			{
				if (!LOCAL_is_UTF8)
				{
					if (STRING_conv(&str, pattern, len_pattern, LOCAL_encoding, SC_UTF8, FALSE))
						STRING_new(&str, pattern, len_pattern);
					else
						STRING_ref(str);
				}
				else
					STRING_new(&str, pattern, len_pattern);

				(*found)(str);
				STRING_unref(&str);
			}
		}
	}
	CATCH
	{
		ret = 1;
		GAMBAS_Error = TRUE;
	}
	END_TRY;
	
	return ret;
}

int GB_StatFile(const char *path, GB_FILE_STAT *info, int follow)
{
	int ret = 0;
	
	TRY
	{
		FILE_stat(path, (FILE_STAT *)info, follow);
	}
	CATCH
	{
		ret = 1;
		GAMBAS_Error = TRUE;
	}
	END_TRY;
	
	return ret;
}
#endif

char *GB_RealFileName(const char *name, int len)
{
  char *path = STRING_conv_file_name(name, len);
  char *real;
  char *temp;

  if (!STREAM_in_archive(path))
    return path;

  temp = FILE_make_temp(NULL, NULL);
  real = STRING_new_temp(NULL, strlen(temp) + strlen(path) + strlen("/data/"));
  snprintf(real, strlen(temp) + strlen(path) + strlen("/data/") + 1, "%s/data/%s", temp, path);

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

  return real;
}

static GB_BROWSE_CALLBACK _browse_project_func;

static void project_file_found(const char *path)
{
	FILE_STAT info;
	
	FILE_stat(path, &info, FALSE);
	(*_browse_project_func)(path, info.size);
}

void GB_BrowseProject(GB_BROWSE_CALLBACK func)
{
	ARCHIVE *arch = NULL;
	
	ARCHIVE_get_current(&arch);
	if (!arch || (arch == ARCHIVE_main && !EXEC_arch))
	{
		_browse_project_func = func;
		FILE_recursive_dir(PROJECT_path, project_file_found, NULL, 0, FALSE);
	}
	else
		ARCHIVE_browse(arch, func);
}


const char *GB_CurrentComponent()
{
  ARCHIVE *arch;

  ARCHIVE_get_current(&arch);
  return arch->name ? arch->name : "";
}


void *GB_DebugGetExec(void)
{
	return &EXEC_current;
}


bool GB_ExistClass(const char *name)
{
	return CLASS_look_global(name, strlen(name)) != NULL;
}


bool GB_ExistClassLocal(const char *name)
{
	return CLASS_look(name, strlen(name)) != NULL;
}

/*void GB_AddString(char **ptr, const char *src, int len)
{
	*ptr = STRING_add(*ptr, src, len);
}

void GB_ExtendString(char **ptr, int new_len)
{
	*ptr = STRING_extend(*ptr, new_len);
}*/
