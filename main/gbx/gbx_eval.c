/***************************************************************************

  eval.c

  Expression evaluator

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

#define __GBX_EVAL_C

#include "gb_common.h"

#include "gb_array.h"
#include "gbx_string.h"
#include "gbx_class.h"
#include "gbx_exec.h"

#include "gbx_eval.h"

#include "gbx_c_collection.h"
#include "gbx_api.h"

bool EVAL_debug = FALSE;

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

#if 0
static bool EVAL_exec()
{
	bool debug = EXEC_debug;
	
  STACK_push_frame(&EXEC_current);

	EXEC_debug = FALSE;
	
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
	EXEC_debug = debug;  
  return (ERROR_info.code != 0);
}
#endif

static void EVAL_exec()
{
	// We need to push a void frame, because EXEC_leave looks at *PC to know if a return value is expected
	STACK_push_frame(&EXEC_current);

	PC = NULL;

	TRY
	{
		EVAL_enter();
	}
	CATCH
	{
		STACK_pop_frame(&EXEC_current);
		PROPAGATE();	
	}
	END_TRY

	EXEC_function_loop();

	TEMP = *RP;
	UNBORROW(&TEMP);
	
	#if 0
	if (PC != NULL)
	{
		do
		{
			TRY
			{
				EXEC_loop();
				TEMP = *RP;
				UNBORROW(&TEMP);
				retry = FALSE;
			}
			CATCH
			{
				// QUIT was called
				if (ERROR_info.code == E_ABORT)
				{
					#if DEBUG_ERROR
					printf("#0 QUIT\n");
					#endif
					ERROR_lock();
					while (PC != NULL)
						EXEC_leave(TRUE);
					ERROR_unlock();

					//STACK_pop_frame(&EXEC_current);
					PROPAGATE();
				}
				// We are in a TRY
				else if (EP != NULL)
				{
					#if DEBUG_ERROR
					printf("#1 EP = %d  SP = %d\n", EP - (VALUE *)STACK_base, SP - (VALUE *)STACK_base);
					#endif

					while (SP > EP)
						POP();

					PC = EC;
					EP = NULL;
					retry = TRUE;
					/* On va directement sur le END TRY */
				}
				// There is a CATCH in the function
				else if (EC != NULL)
				{
					#if DEBUG_ERROR
					printf("#2 EC = %p\n", EC);
					#endif

					PC = EC;
					EC = NULL;
					retry = TRUE;
				}
				// There is no event handler in the function
				else
				{
					#if DEBUG_ERROR
					printf("#3\n");
					#endif

					/*if (EXEC_debug && !STACK_has_error_handler())
					{
						if (TP && TC)
						{
							ERROR_lock();
							while (BP > TP)
							{
								EXEC_leave(TRUE);
								if (!PC)
									STACK_pop_frame(&EXEC_current);
							}
							while (SP > TP)
								POP();
							PC = TC;
							ERROR_unlock();
						}

						DEBUG.Main(TRUE);
						retry = TRUE;
					}
					else*/
					{
						ERROR_lock();
						while (PC != NULL && EC == NULL)
							EXEC_leave(TRUE);
						ERROR_unlock();

						if (PC == NULL)
						{
							/*printf("try to propagate\n");*/
							STACK_pop_frame(&EXEC_current);
							PROPAGATE();

							/*ERROR_print();
							exit(1);*/
							/*retry = FALSE;*/
						}

						if (EP != NULL)
						{
							#if DEBUG_ERROR
							printf("#1 EP = %d  SP = %d\n", EP - (VALUE *)STACK_base, SP - (VALUE *)STACK_base);
							#endif

							ERROR_lock();
							while (SP > EP)
								POP();
							ERROR_unlock();

							EP = NULL;
							/* On va directement sur le END TRY */
						}

						PC = EC;
						EC = NULL;

						retry = TRUE;
					}
				}

				while (SP < EXEC_super)
					EXEC_super = ((VALUE *)EXEC_super)->_object.super;
			}
			END_TRY

			#if DEBUG_ERROR
			if (retry)
				printf("retry %p\n", PC);
			#endif
		}
		while (retry);
	}

	STACK_pop_frame(&EXEC_current);
	#endif
}

bool EVAL_expression(EXPRESSION *expr, EVAL_FUNCTION func)
{
  int i;
  EVAL_SYMBOL *sym;
  bool debug;
  bool error;
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
      return TRUE;
    }

    BORROW(SP);

    SP++;
  }

	debug = EXEC_debug;
	EXEC_debug = FALSE;
	error = FALSE;

	TRY
	{
  	EVAL_exec();
  }
  CATCH
  {
  	error = TRUE;
  }
  END_TRY
  
  EXEC_debug = debug;
  return error;
}



