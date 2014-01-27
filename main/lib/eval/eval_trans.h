/***************************************************************************

  eval_trans.h

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

#ifndef __EVAL_TRANS_H
#define __EVAL_TRANS_H

#include "gb_type_common.h"
#include "gb_reserved.h"
#include "eval_read.h"
#include "gb_limit.h"
#include "../../gbx/gbx_class.h"

#include "gbc_trans_common.h"

/*
enum {
  TT_NOTHING = 0,
  TT_DO_NOT_CHECK_AS = 1,
  TT_CAN_SQUARE = 2,
  TT_CAN_ARRAY = 4,
  TT_CAN_NEW = 8
  };
*/

/*
void TRANS_reset(void);
boolean TRANS_newline(void);
boolean TRANS_type(int flag, TRANS_DECL *result);
boolean TRANS_check_declaration(void);
void TRANS_get_constant_value(TRANS_DECL *decl, PATTERN value);

void TRANS_want(int reserved);
boolean TRANS_is(int reserved);
void TRANS_ignore(int reserved);
*/

bool TRANS_get_number(int index, TRANS_NUMBER *result);

/* eval_trans_expr.c */

void TRANS_expression(void);
bool TRANS_affectation(void);
void TRANS_operation(short op, short nparam, PATTERN previous);

/* eval_trans_tree.c */

#define RS_UNARY (-1)

void TRANS_tree(void);
/*boolean TRANS_is_statement(TRANS_TREE *tree);*/

#endif

