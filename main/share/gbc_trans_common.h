/***************************************************************************

  gbc_trans_common.h

  Common declaration of code translation

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

#ifndef __GBC_TRANS_COMMMON_H
#define __GBC_TRANS_COMMMON_H

typedef
  struct {
    TYPE type;
    int index;
    int value;
    PATTERN *optional;
    }
  TRANS_PARAM;  /* must be the same as PARAM in class.h */

typedef
  PATTERN TRANS_TREE;

typedef
  struct {
    int type;
    int ival;
    int64_t lval;
    double dval;
    }
  TRANS_NUMBER;

typedef
  struct {
    TYPE type;
    int ndim;
    int dim[MAX_ARRAY_DIM];
    }
  TRANS_ARRAY;

typedef
  struct {
    int index;                /* index in symbol table */
    TYPE type;                /* data type */
    int value;                /* value */
    TRANS_ARRAY array;        /* array dimensions */
    PATTERN *init;            /* initialization code */
    int64_t lvalue;           /* The value of a LONG constant */
    unsigned is_new : 1;      /* if something must be instanciated */
    unsigned is_integer : 1;  /* if the constant is an integer */
		unsigned _reserved : 30;
    }
  TRANS_DECL;

typedef
  struct {
    int index;
    TYPE type;
    int nparam;
    TRANS_PARAM param[MAX_PARAM_FUNC];
    PATTERN *start;
    int line;
    unsigned vararg : 1;
		unsigned _reserved : 31;
    }
  TRANS_FUNC;

typedef
  struct {
    int index;
    TYPE type;
    int nparam;
    TRANS_PARAM param[MAX_PARAM_FUNC];
    }
  TRANS_EVENT;

typedef
  struct {
    int index;
    TYPE type;
    int line;
    int comment;
    bool read;
    bool _reserved[3];
    }
  TRANS_PROPERTY;

typedef
  struct {
    int index;
    TYPE type;
    int nparam;
    TRANS_PARAM param[MAX_PARAM_FUNC];
    int library;
    int alias;
    }
  TRANS_EXTERN;

typedef
  struct {
    int type;
    int value;
    int state;
    short *pos;
    short *pos_break;
    short *pos_continue;
    short local;
    short id;
    short loop_var;
		short _reserved;
    }
  TRANS_CTRL;

typedef
  struct {
    int index;
    int line;
    ushort pos;
    short ctrl_id;
    }
  TRANS_GOTO;

typedef
  struct {
    int index;
    ushort pos;
    short ctrl_id;
    }
  TRANS_LABEL;

typedef
  struct {
    RESERVED_ID id;
    void (*func)();
    }
  TRANS_STATEMENT;

#endif

