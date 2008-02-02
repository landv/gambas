/***************************************************************************

  debug.c

  debug routines

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

#include "gbx_debug.h"


DEBUG_INTERFACE DEBUG;
DEBUG_INFO *DEBUG_info = NULL;

static boolean calc_line_from_position(CLASS *class, FUNCTION *func, PCODE *addr, ushort *line)
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

  DEBUG_info = DEBUG.Init((GB_DEBUG_INTERFACE *)(void *)GAMBAS_DebugApi, EXEC_fifo);

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
  fprintf(stderr, "%s: ", DEBUG_get_current_position());
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
        }
        else
        {
          var = &DEBUG_info->cp->load->stat[gp->value];
          addr = (char *)DEBUG_info->cp->stat + var->pos;
        }

        VALUE_class_read(DEBUG_info->cp, &value, addr, var->type);
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
		}
		goto __FOUND;
	}

  return TRUE;

__FOUND:

  /*printf("%.*s =", (int)len, sym);
  print_value(&value);*/

  BORROW(&value);
  if (value.type == T_ARRAY)
    value._array.keep = TRUE;
  else
    VALUE_conv(&value, T_VARIANT);
  UNBORROW(&value);

  *((VALUE *)ret) = value;
  return FALSE;
}

int DEBUG_can_be_used_like_an_array(void *object, CLASS *class)
{
	CLASS_DESC *desc;
  char type;
  int index;

	//fprintf(stderr, "DEBUG_can_be_used_like_an_array: %p %s ?\n", object, class->name);

	if (!object)
		return 0;
	
	if (class == CLASS_Class)
	{
		class = (CLASS *)object;
		object = NULL;
		CLASS_load(class);
	}

  index = class->special[SPEC_GET];
  if (index == NO_SYMBOL)
  {
  	//fprintf(stderr, "No _get method\n");
    return 0;
  }
	desc = class->table[index].desc;
	if (desc->method.npmin != 1 || desc->method.npmax != 1 || *desc->method.signature != T_INTEGER)
	{
  	//fprintf(stderr, "No _get(Arg AS Integer) method\n");
		return 0;
	}
	
	index = CLASS_find_symbol(class, "Count");
	if (index == NO_SYMBOL)
	{
  	//fprintf(stderr, "No Count symbol\n");		
		return 0;
	}
		
	desc = class->table[index].desc;
	type = CLASS_DESC_get_type(desc);
	if (type != 'r' && type != 'R')
	{
  	//fprintf(stderr, "Count is not a read-only property\n");		
		return 0;
	}
	
	
  if (desc->property.native)
  {
    if (EXEC_call_native(desc->property.read, object, desc->property.type, 0))
    {
	  	//fprintf(stderr, "Count has failed\n");
    	return 0;
    }
  }
  else
  {
    EXEC.class = desc->property.class;
    EXEC.object = object;
    EXEC.drop = FALSE;
    EXEC.nparam = 0;
    EXEC.native = FALSE;
    EXEC.index = (int)(intptr_t)desc->property.read;
    //EXEC.func = &class->load->func[(int)desc->property.read];

    EXEC_function_keep();

    TEMP = *RP;
    UNBORROW(RP);
    RP->type = T_VOID;
  }

	VALUE_conv(&TEMP, GB_T_INTEGER);
	return TEMP._integer.value;
}



DEBUG_BACKTRACE *DEBUG_backtrace()
{
	DEBUG_BACKTRACE *result;
  int i, n;
  STACK_CONTEXT *context;

	n = 1;
  for (i = 0; i < (STACK_frame_count - 1); i++)
  {
    context = &STACK_frame[i];
    if (!context->pc)
    	continue;
    n++;
  }
	
	ARRAY_create(&result);
	ARRAY_add_many(&result, n);

	result[0].pc = PC;
	result[0].cp = CP;
	result[0].fp = FP;

	n = 0;
  for (i = 0; i < (STACK_frame_count - 1); i++)
  {
    context = &STACK_frame[i];
    if (!context->pc)
    	continue;
    n++;
  	result[n].pc = context->pc;
  	result[n].cp = context->cp;
  	result[n].fp = context->fp;
  }
  
  return result;
}

DEBUG_BACKTRACE *DEBUG_copy_backtrace(DEBUG_BACKTRACE *bt)
{
	DEBUG_BACKTRACE *result;
	int i;
	int n = ARRAY_count(bt);
	
	ARRAY_create(&result);
	ARRAY_add_many(&result, n);
	for (i = 0; i < n; i++)
		result[i] = bt[i];
		
	return result;
}


GB_ARRAY DEBUG_get_string_array_from_backtrace(DEBUG_BACKTRACE *bt)
{
	GB_ARRAY array;
	int i, n;
	
	n = ARRAY_count(bt);
	GB_ArrayNew(&array, GB_T_STRING, n);
	for (i = 0; i < n; i++)
		STRING_new((char **)GB_ArrayGet(array, i), DEBUG_get_position(bt[i].cp, bt[i].fp, bt[i].pc), 0);

	return array;
}
