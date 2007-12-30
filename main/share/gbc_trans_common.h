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
    long index;
    PATTERN *optional;
    long value;
    }
  TRANS_PARAM;  /* must be the same as PARAM in class.h */

typedef
  PATTERN TRANS_TREE;

typedef
  struct {
    long type;
    long ival;
    long long lval;
    double dval;
    }
  TRANS_NUMBER;

typedef
  struct {
    TYPE type;
    long ndim;
    long dim[MAX_ARRAY_DIM];
    }
  TRANS_ARRAY;

typedef
  struct {
    long index;               		/* index in symbol table */
    TYPE type;                		/* data type */
    long value;               		/* value */
    TRANS_ARRAY array;        		/* array dimensions */
    PATTERN *init;            		/* initialization code */
    long long lvalue;         		/* The value of a LONG constant */
    unsigned is_new : 1;      		/* if something must be instanciated */
    unsigned is_integer : 1;  		/* if the constant is an integer */
    }
  PACKED
  TRANS_DECL;

typedef
  struct {
    long index;
    TYPE type;
    long nparam;
    TRANS_PARAM param[MAX_PARAM_FUNC];
    PATTERN *start;
    long line;
    unsigned vararg : 1;
    }
  PACKED
  TRANS_FUNC;

typedef
  struct {
    long index;
    TYPE type;
    long nparam;
    TRANS_PARAM param[MAX_PARAM_FUNC];
    }
  PACKED
  TRANS_EVENT;

typedef
  struct {
    long index;
    TYPE type;
    long line;
    long comment;
    bool read;
    bool _reserved[3];
    }
  PACKED
  TRANS_PROPERTY;

typedef
  struct {
    long index;
    TYPE type;
    long nparam;
    TRANS_PARAM param[MAX_PARAM_FUNC];
    long library;
    long alias;
    }
  PACKED
  TRANS_EXTERN;

typedef
  struct {
    long type;
    long value;
    long state;
    short *pos;
    short *pos_break;
    short *pos_continue;
    short local;
    short id;
    short loop_var;
    }
  PACKED
  TRANS_CTRL;

typedef
  struct {
    long index;
    long line;
    ushort pos;
    short ctrl_id;
    }
  PACKED
  TRANS_GOTO;

typedef
  struct {
    long index;
    ushort pos;
    short ctrl_id;
    }
  PACKED
  TRANS_LABEL;

typedef
  struct {
    RESERVED_ID id;
    void (*func)();
    }
  TRANS_STATEMENT;

#endif

