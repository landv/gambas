/***************************************************************************

  eval.h

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

#ifndef __EVAL_H
#define __EVAL_H

#include "gb_table.h"
#include "gb_reserved.h"
#include "eval_read.h"
/*#include "CCollection.h"*/
#include "main.h"

#include "../../gbx/gbx_expression.h"
#include "gb.eval.h"

#ifndef __EVAL_C
EXTERN EXPRESSION *EVAL;
EXTERN EXPRESSION EVAL_read_expr;
#endif


PUBLIC void EVAL_init(void);
PUBLIC void EVAL_exit(void);

PUBLIC void EVAL_new(EXPRESSION **expr, char *src, long len);
PUBLIC void EVAL_free(EXPRESSION **expr);
PUBLIC bool EVAL_compile(EXPRESSION *expr);

PUBLIC GB_VALUE *EVAL_expression(EXPRESSION *expr, EVAL_FUNCTION get_value);
PUBLIC void EVAL_clear(EXPRESSION *expr);

PUBLIC long EVAL_add_constant(CLASS_CONST *cst);
PUBLIC long EVAL_add_class(char *name);
PUBLIC long EVAL_add_unknown(char *name);
PUBLIC long EVAL_add_variable(long index);

PUBLIC void EVAL_start(EXPRESSION *expr);

#endif
