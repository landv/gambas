/***************************************************************************

  expression.h

  Compiled expression

  (c) 2000-2005 Benoît Minisini <gambas@users.sourceforge.net>

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


#ifndef __GBX_EXPRESSION_H
#define __GBX_EXPRESSION_H

#include "gb_table.h"
#include "gbx_class.h"

typedef
  struct {
    char *source;
    long len;
    PATTERN *pattern;
    PATTERN *current;
    PATTERN *tree;
    CLASS exec_class;
    CLASS_LOAD class_load;
    FUNCTION func;
    CLASS_CONST *cst;
    ushort *code;
    TABLE *table;
    TABLE *string;
    /*TABLE *variable;*/
    CLASS **class;
    char **unknown;
    long *var;
    short nvar;
    short last_code;
    long stack_usage;
    void *op;
    unsigned analyze : 1;
    unsigned _reserved : 31;
    }
  EXPRESSION;

typedef
  struct {
    SYMBOL sym;
    long local;
    }
  EVAL_SYMBOL;
  
#endif
