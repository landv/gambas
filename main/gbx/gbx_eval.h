/***************************************************************************

  gbx_eval.h

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

bool EVAL_expression(EXPRESSION *expr, EVAL_FUNCTION func);

#endif
