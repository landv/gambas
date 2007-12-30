/***************************************************************************

  eval.c

  Expression evaluator

  (c) 2000-2006 Benoît Minisini <gambas@users.sourceforge.net>

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

#define __GBX_EVAL_C

#include "gb_common.h"

#include "gb_array.h"
#include "gbx_string.h"
#include "gbx_class.h"
#include "gbx_exec.h"

#include "gbx_eval.h"

#include "gbx_c_collection.h"
#include "gbx_api.h"

PUBLIC bool EVAL_debug = FALSE;

static EXPRESSION *EVAL;

static void EVAL_enter()
{
  STACK_push_frame(&EXEC_current);

  STACK_check(EVAL->func.stack_usage);

  BP = SP;
  PP = SP;
  FP = &EVAL->func;
  PC = EVAL->func.code;
  
  OP = NULL;
  CP = &EVAL->exec_class;
  //AP = ARCH_from_class(CP);

  EP = NULL;
  EC = NULL;

  RP->type = T_VOID;
}


static bool EVAL_exec()
{
  STACK_push_frame(&EXEC_current);

  PC = NULL;

  EVAL_enter();
  
  ERROR_clear();

  TRY
  {
    EXEC_loop();
    TEMP = *RP;
    /*RET.type = T_VOID;*/
    UNBORROW(&TEMP);
    STACK_pop_frame(&EXEC_current);
  }
  CATCH
  {
    STACK_pop_frame(&EXEC_current);
    STACK_pop_frame(&EXEC_current);
  }
  END_TRY

  //AP = ARCH_from_class(CP);
  
  return (ERROR_info.code != 0);
}


PUBLIC bool EVAL_expression(EXPRESSION *expr, EVAL_FUNCTION func)
{
  int i;
  EVAL_SYMBOL *sym;
  /*HASH_TABLE *hash_table;
  char *name;
  CCOL_ENUM enum_state;*/

  EVAL = expr;

  #ifdef DEBUG
  printf("EVAL: %s\n", EVAL->source);
  #endif

  STACK_check(EVAL->nvar);

  for (i = 0; i < EVAL->nvar; i++)
  {
    SP->type = T_VARIANT;
    SP->_variant.vtype = T_NULL;

    sym = (EVAL_SYMBOL *)TABLE_get_symbol(EVAL->table, EVAL->var[EVAL->nvar - i - 1]);
    if ((*func)(sym->sym.name, sym->sym.len, (GB_VARIANT *)SP))
    {
      GB_Error("Unknown symbol");
      /* ?? La pile est elle bien lib�� */
      return TRUE;
    }

    /*VALUE_read(SP, &value, T_VARIANT);*/
    BORROW(SP);

    SP++;
  }

  /*EXEC_function(&exec, EVAL->nvar);*/

  return EVAL_exec();
}



