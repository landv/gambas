/***************************************************************************

  eval.h

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

#ifndef __EVAL_H
#define __EVAL_H

#include "gb_table.h"
#include "gb_reserved.h"
#include "gb_error.h"
#include "eval_read.h"
/*#include "CCollection.h"*/
#include "main.h"

#include "../../gbx/gbx_expression.h"
#include "gb.eval.h"

#ifndef __EVAL_C
EXTERN EXPRESSION *EVAL;
EXTERN EXPRESSION EVAL_read_expr;
#endif


void EVAL_init(void);
void EVAL_exit(void);

void EVAL_new(EXPRESSION **expr, char *src, int len);
void EVAL_free(EXPRESSION **expr);
bool EVAL_compile(EXPRESSION *expr, bool assign);

GB_VALUE *EVAL_expression(EXPRESSION *expr, EVAL_FUNCTION get_value);
void EVAL_clear(EXPRESSION *expr, bool keep_error);

int EVAL_add_constant(CLASS_CONST *cst);
int EVAL_add_class(char *name);
int EVAL_add_unknown(char *name);
int EVAL_add_variable(int index);

void EVAL_start(EXPRESSION *expr);

bool EVAL_get_assignment_symbol(EXPRESSION *expr, const char **name, int *len);

#endif
