/***************************************************************************

  gb.eval.h

  The evaluator plug-in

  (c) 2000-2007 Benoit Minisini <gambas@freesurf.fr>

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

#ifndef __GB_EVAL_H
#define __GB_EVAL_H

#include "gambas.h"

/*#define DEBUG*/

typedef
  bool (*EVAL_FUNCTION)(const char *, long, GB_VARIANT *);

typedef
  void *EVAL_EXPRESSION;

//typedef
//  long EVAL_PATTERN;

typedef
  struct {
    unsigned state : 4;
    unsigned len : 12;
  }
  EVAL_COLOR;

typedef
  struct
  {
    char *str;
    EVAL_COLOR *color;
    int len;
    int proc;
  }
  EVAL_ANALYZE;

#define EVAL_NORMAL       0
#define EVAL_USE_CONTEXT  1

enum {
  EVAL_TYPE_END = 0,
  EVAL_TYPE_NEWLINE = 1,
  EVAL_TYPE_RESERVED = 2,
  EVAL_TYPE_IDENTIFIER = 3,
  EVAL_TYPE_NUMBER = 4,
  EVAL_TYPE_STRING = 5,
  EVAL_TYPE_TSTRING = 6,
  EVAL_TYPE_PARAM = 7,
  EVAL_TYPE_SUBR = 8,
  EVAL_TYPE_CLASS = 9,
  EVAL_TYPE_COMMENT = 10,
  EVAL_TYPE_OPERATOR = 11,
  EVAL_TYPE_DATATYPE = 12,
  EVAL_TYPE_ERROR = 13
  };

typedef
  struct {
    long version;
    //EVAL_PATTERN *(*Read)(const char *src, long len);
    void (*Analyze)(const char *src, long len, EVAL_ANALYZE *result);
    //const char *(*GetSymbol)(EVAL_PATTERN pattern);
    //int (*GetType)(EVAL_PATTERN pattern);
    //long (*IsMeLast)(EVAL_PATTERN pattern);
    void (*New)(EVAL_EXPRESSION *expr, const char *src, long len);
    bool (*Compile)(EVAL_EXPRESSION expr);
    GB_VALUE *(*Run)(EVAL_EXPRESSION expr, EVAL_FUNCTION func);
    void (*Free)(EVAL_EXPRESSION *expr);
    }
  EVAL_INTERFACE;

#define EVAL_INTERFACE_VERSION 2

#endif

