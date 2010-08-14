/***************************************************************************

  gbx_debug.c

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

#define __GBX_DEBUG_C

#include "gb_common.h"
#include "gb_common_buffer.h"
#include "gb_common_case.h"
#include <stdarg.h>

#include "gb_buffer.h"
#include "gb_array.h"
#include "gb_error.h"
#include "gbx_exec.h"
#include "gbx_api.h"
#include "gbx_class.h"
#include "gbx_c_array.h"
#include "gbx_project.h"

#include "gbx_debug.h"


DEBUG_INTERFACE DEBUG;
DEBUG_INFO *DEBUG_info = NULL;

static bool calc_line_from_position(CLASS *class, FUNCTION *func, PCODE *addr, ushort *line)
{
  int i;
  ushort pos = addr - func->code;
  ushort *post;

  if (func->debug)
  {
    post =  func->debug->pos;
    for (i = 0; i < (func->debug->nline - 1); i++)
    {
      if (pos >= post[i] && pos < post[i + 1])
      {
        *line = i + func->debug->line;
        return FALSE;
      }
    }

    /*printf("pos = %d addr=%p func->code=%p\n", pos, addr, func->code);*/
  }

  return TRUE;
}

const char *DEBUG_get_position(CLASS *cp, FUNCTION *fp, PCODE *pc)
{
#if DEBUG_MEMORY
  static char buffer[256];
#endif
  ushort line = 0;

	if (!cp || !pc)
		return "?";
		
  if (fp != NULL && fp->debug)
    calc_line_from_position(cp, fp, pc, &line);

#if DEBUG_MEMORY
  snprintf(buffer, sizeof(buffer), "%s.%s.%d",
    cp ? cp->name : "?",
    (fp && fp->debug) ? fp->debug->name : "?",
    line);

  return buffer;
#else
  snprintf(COMMON_buffer, COMMON_BUF_MAX, "%.64s.%.64s.%d",
    cp ? cp->name : "?",
    (fp && fp->debug) ? fp->debug->name : "?",
    line);

  return COMMON_buffer;
#endif
}


const char *DEBUG_get_current_position(void)
{
  return DEBUG_get_position(CP, FP, PC);
}


void DEBUG_init(void)
{
  if (!EXEC_debug)
  	return;

  COMPONENT_load(COMPONENT_create("gb.debug"));
  LIBRARY_get_interface_by_name("gb.debug", DEBUG_INTERFACE_VERSION, &DEBUG);

  DEBUG_info = DEBUG.Init((GB_DEBUG_INTERFACE *)(void *)GAMBAS_DebugApi, EXEC_fifo, EXEC_fifo_name);

  if (!DEBUG_info)
  	ERROR_panic("Cannot initializing debug mode");
}


void DEBUG_exit(void)
{
	if (!EXEC_debug)
		return;

	DEBUG.Exit();
}


void DEBUG_where(void)
{
	//static bool breakpoint = FALSE;
	const char *where = DEBUG_get_current_position();
	/*if (!breakpoint && !strcmp(where, "FForm._new.97"))
	{
		breakpoint = TRUE;
		BREAKPOINT();
	}*/
  fprintf(stderr, "%s: ", where);
}


bool DEBUG_get_value(const char *sym, int len, GB_VARIANT *ret)
{
  int i;
  VALUE value;
  LOCAL_SYMBOL *lp;
  GLOBAL_SYMBOL *gp;
  CLASS_VAR *var;
  char *addr;
  CLASS *class;
	void *ref;

  if (DEBUG_info->fp)
  {
    for (i = 0; i < DEBUG_info->fp->debug->n_local; i++)
    {
      lp = &DEBUG_info->fp->debug->local[i];
      if (len == lp->sym.len && strncasecmp(sym, lp->sym.name, len) == 0)
      {
        value = DEBUG_info->bp[lp->value];
        goto __FOUND;
      }
    }
  }

  if (DEBUG_info->cp)
  {
    for (i = 0; i < DEBUG_info->cp->load->n_global; i++)
    {
      gp = &DEBUG_info->cp->load->global[i];
      if (len != gp->sym.len || strncasecmp(sym, gp->sym.name, len) != 0)
        continue;

      if (CTYPE_get_kind(gp->ctype) == TK_VARIABLE)
      {
        if (!CTYPE_is_static(gp->ctype) && DEBUG_info->op)
        {
          var = &DEBUG_info->cp->load->dyn[gp->value];
          addr = (char *)DEBUG_info->op + var->pos;
					ref = DEBUG_info->op;
        }
        else
        {
          var = &DEBUG_info->cp->load->stat[gp->value];
          addr = (char *)DEBUG_info->cp->stat + var->pos;
					ref = DEBUG_info->cp;
        }

        VALUE_class_read(DEBUG_info->cp, &value, addr, var->type, ref);
        goto __FOUND;
      }
      else if (CTYPE_get_kind(gp->ctype) == TK_CONST)
      {
        VALUE_class_constant(DEBUG_info->cp, &value, gp->value);
        goto __FOUND;
      }
    }
  }

	//class = CLASS_look_global(sym, len);
	class = CLASS_look(sym, len);
	if (class)
	{
		if (class->auto_create && class->instance)
		{
			value._object.class = class;
			value._object.object = class->instance;
			value._object.super = NULL;
		}
		else
		{
			value.type = T_CLASS;
			value._class.class = class;
			value._class.super = NULL;
		}
		goto __FOUND;
	}

  return TRUE;

__FOUND:

  /*printf("%.*s =", (int)len, sym);
  print_value(&value);*/

  BORROW(&value);
  /*if (value.type == T_ARRAY)
    value._array.keep = TRUE;
  else*/
    VALUE_conv_variant(&value);
  UNBORROW(&value);

  *((VALUE *)ret) = value;
  return FALSE;
}

int DEBUG_set_value(const char *sym, int len, VALUE *value)
{
  int i;
  LOCAL_SYMBOL *lp;
  GLOBAL_SYMBOL *gp;
  CLASS_VAR *var;
  char *addr;
	VALUE *where;
	bool ret = GB_DEBUG_SET_OK;

  TRY
  {
		if (DEBUG_info->fp)
		{
			for (i = 0; i < DEBUG_info->fp->debug->n_local; i++)
			{
				lp = &DEBUG_info->fp->debug->local[i];
				if (len == lp->sym.len && strncasecmp(sym, lp->sym.name, len) == 0)
				{
					where = &DEBUG_info->bp[lp->value];
					VALUE_conv(value, where->type);
					RELEASE(where);
					*where = *value;
					BORROW(where);
					goto __FOUND;
				}
			}
		}

		if (DEBUG_info->cp)
		{
			for (i = 0; i < DEBUG_info->cp->load->n_global; i++)
			{
				gp = &DEBUG_info->cp->load->global[i];
				if (len != gp->sym.len || strncasecmp(sym, gp->sym.name, len) != 0)
					continue;

				if (CTYPE_get_kind(gp->ctype) == TK_VARIABLE)
				{
					if (!CTYPE_is_static(gp->ctype) && DEBUG_info->op)
					{
						var = &DEBUG_info->cp->load->dyn[gp->value];
						addr = (char *)DEBUG_info->op + var->pos;
					}
					else
					{
						var = &DEBUG_info->cp->load->stat[gp->value];
						addr = (char *)DEBUG_info->cp->stat + var->pos;
					}

					VALUE_class_write(DEBUG_info->cp, value, addr, var->type);
					goto __FOUND;
				}
			}
		}

		ret = GB_DEBUG_SET_READ_ONLY;
		
__FOUND:

		0;
	}
  CATCH
  {
    ret = GB_DEBUG_SET_ERROR;
    GAMBAS_Error = TRUE;
  }
  END_TRY

  return ret;
}

int DEBUG_get_object_access_type(void *object, CLASS *class, int *count)
{
	CLASS_DESC *desc;
  char type;
  int index;
	int access = GB_DEBUG_ACCESS_NORMAL;

	//fprintf(stderr, "DEBUG_can_be_used_like_an_array: %p %s ?\n", object, class->name);

	if (!object)
		goto __NORMAL;
	
	if (class == CLASS_Class || OBJECT_is_class(object))
	{
		class = (CLASS *)object;
		object = NULL;
		CLASS_load(class);
	}

  index = class->special[SPEC_GET];
  if (index == NO_SYMBOL)
  {
  	//fprintf(stderr, "No _get method\n");
		goto __NORMAL;
  }

	desc = class->table[index].desc;
	if (desc->method.npmin != 1 || desc->method.npmax != 1)
	{
  	//fprintf(stderr, "No _get(Arg AS Integer) method\n");
		goto __NORMAL;
	}

	if (*desc->method.signature == T_INTEGER)
		access = GB_DEBUG_ACCESS_ARRAY;
	//else if (*desc->method.signature == T_STRING)
	//	access = GB_DEBUG_ACCESS_COLLECTION;
	else
		goto __NORMAL;
	
	index = CLASS_find_symbol(class, "Count");
	if (index == NO_SYMBOL)
	{
  	//fprintf(stderr, "No Count symbol\n");		
		goto __NORMAL;
	}
		
	desc = class->table[index].desc;
	type = CLASS_DESC_get_type(desc);
	
	// The two only possible cases:
	// A static read-only property, and object == NULL
	// or a dynamic read-only property, and object != NULL
	
	if (!((type == 'r' && object) || (type == 'R' && !object)))
		goto __NORMAL;
	
  if (desc->property.native)
  {
    if (EXEC_call_native(desc->property.read, object, desc->property.type, 0))
    {
	  	//fprintf(stderr, "Count has failed\n");
			goto __NORMAL;
    }
  }
  else
  {
    EXEC.class = desc->property.class;
    EXEC.object = object;
    EXEC.nparam = 0;
    EXEC.native = FALSE;
    EXEC.index = (int)(intptr_t)desc->property.read;
    //EXEC.func = &class->load->func[(int)desc->property.read];

    EXEC_function_keep();

    TEMP = *RP;
    UNBORROW(RP);
    RP->type = T_VOID;
  }

	VALUE_conv_integer(&TEMP);
	*count = TEMP._integer.value;
	return access;

__NORMAL:
	return GB_DEBUG_ACCESS_NORMAL;
}


void DEBUG_print_backtrace(ERROR_INFO *err)
{
	int i, n;
	STACK_CONTEXT *sc = (STACK_CONTEXT *)(STACK_base + STACK_size) - err->bt_count;
	
	fprintf(stderr, "0: %s\n", DEBUG_get_position(err->cp, err->fp, err->pc));
	for (i = 0, n = 0; i < err->bt_count; i++)
	{
		//fprintf(stderr, "%d: %s\n", i, DEBUG_get_position(bt[i].cp, bt[i].fp, bt[i].pc));
		if (!sc[i].pc)
			continue;
		n++;
		fprintf(stderr, "%d: %s\n", n, DEBUG_get_position(sc[i].cp, sc[i].fp, sc[i].pc));
	}
}


GB_ARRAY DEBUG_get_string_array_from_backtrace(ERROR_INFO *err)
{
	GB_ARRAY array;
	int i, n;
	STACK_CONTEXT *sc = (STACK_CONTEXT *)(STACK_base + STACK_size) - err->bt_count;
	
	for (i = 0, n = 1; i < err->bt_count; i++)
	{
		if (!sc[i].pc)
			continue;
		n++;
	}

	GB_ArrayNew(&array, GB_T_STRING, n);
	*((char **)GB_ArrayGet(array, 0)) = STRING_new_zero(DEBUG_get_position(err->cp, err->fp, err->pc));
	for (i = 0, n = 1; i < err->bt_count; i++)
	{
		if (!sc[i].pc)
			continue;
		*((char **)GB_ArrayGet(array, n)) = STRING_new_zero(DEBUG_get_position(sc[i].cp, sc[i].fp, sc[i].pc));
		n++;
	}

	return array;
}

GB_CLASS DEBUG_find_class(const char *name)
{
	CLASS *class;
	CLASS *save = CP;
	
	// As the startup class is automatically exported, this is the only way for the debugger to find it.
	if (PROJECT_class && !strcmp(name, PROJECT_class->name))
		return (GB_CLASS)PROJECT_class;
	
	CP = NULL;
	class = CLASS_find(name);
	CP = save;
	
	return (GB_CLASS)class;
}

