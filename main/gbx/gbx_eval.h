/***************************************************************************

  eval.h

  Expression evaluator

  (c) 2000-2005 Beno�t Minisini <gambas@users.sourceforge.net>

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


#ifndef __GBX_EVAL_H
#define __GBX_EVAL_H

#define PATTERN void

#include "gb_table.h"
#include "gbx_class.h"
#include "gb_reserved.h"
#include "gbx_c_collection.h"

#include "gbx_expression.h"
#include "../lib/eval/gb.eval.h"

#ifndef __GBX_EVAL_C
EXTERN bool EVAL_debug;
#endif

PUBLIC bool EVAL_expression(EXPRESSION *expr, EVAL_FUNCTION func);

#endif
