/***************************************************************************

  gbx_class_load.h

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

#ifndef __GBX_CLASS_LOAD_H
#define __GBX_CLASS_LOAD_H

#include "gb_common.h"

#ifdef OS_64BITS

typedef
	uint32_t ptr32_t;

typedef
  struct {
    ptr32_t name;
    int len;
    }
  PACKED
  SYMBOL_32;

typedef
  struct {
    CTYPE type;
    int pos;
    }
  PACKED
  CLASS_VAR_32;

typedef
	uint32_t TYPE_32;

typedef
  union {
    TYPE_32 type;
    struct { TYPE_32 type; double value; } PACKED _float;
    struct { TYPE_32 type; float value; } PACKED _single;
    struct { TYPE_32 type; int value; } PACKED _integer;
    struct { TYPE_32 type; int64_t value; } PACKED _long;
    struct { TYPE_32 type; ptr32_t addr; int len; } PACKED _string;
    struct { TYPE_32 type; int val[2]; } PACKED _swap;
    }
  PACKED
  CLASS_CONST_32;

typedef
	ptr32_t CLASS_REF_32;

typedef
	ptr32_t CLASS_UNKNOWN_32;

typedef
  struct {
    CTYPE type;
    }
  PACKED
  CLASS_LOCAL_32;

typedef
  struct {
    TYPE_32 type;
    }
  PACKED
  CLASS_PARAM_32;

typedef
  struct {
    SYMBOL_32 sym;
    int value;
    }
  LOCAL_SYMBOL_32;

typedef
  struct {
    unsigned short line;
    unsigned short nline;
    unsigned short *pos;
    ptr32_t name;
    ptr32_t local; // LOCAL_SYMBOL_32
    short n_local;
    unsigned short _reserved;
  }
  PACKED
  FUNC_DEBUG_32;

typedef
  struct {
    TYPE type;
    char n_param;
    char npmin;
    char vararg;
    char flag;
    short n_local;
    short n_ctrl;
    short stack_usage;
    short error;
    ptr32_t code; // unsigned short *
    ptr32_t param; // CLASS_PARAM
    ptr32_t local; // CLASS_LOCAL
    ptr32_t debug; // FUNC_DEBUG
    }
  PACKED
  FUNCTION_32;

typedef
  struct {
    TYPE_32 type;
    short n_param;
    short _reserved;
    ptr32_t param; // CLASS_PARAM
    ptr32_t name;
    }
  PACKED
  CLASS_EVENT_32;

typedef
  struct {
    TYPE_32 type;
    short n_param;
    unsigned loaded : 1;
    unsigned _reserved : 15;
    ptr32_t param; // CLASS_PARAM
    ptr32_t alias;
    ptr32_t library;
    }
  PACKED
  CLASS_EXTERN_32;

typedef
  struct {
    TYPE_32 type;
    int dim[0];
    }
  CLASS_ARRAY_32;

typedef
	CLASS_ARRAY_32 *CLASS_ARRAY_P_32;

struct _CLASS;

typedef
  struct {
    SYMBOL_32 sym;
    CTYPE ctype;
    int value;
    }
  GLOBAL_SYMBOL_32;

#if 0
typedef
  struct {
    ptr32_t name;
    TYPE_32 type;
    ptr32_t read;
    ptr32_t write;
    char native;
    char _reserved[3];
    ptr32_t class;
    }
  PACKED
  CLASS_DESC_PROPERTY_32;

typedef
  struct {
    ptr32_t name;
    TYPE_32 type;
    int offset;
    int _reserved;
    }
  PACKED
  CLASS_DESC_VARIABLE;

typedef
  struct {
    ptr32_t name;
    TYPE_32 type;
    void (*exec)();             /* m�hode */
    TYPE *signature;            /* signature */
    char npmin;                 /* nombre de param�res minimum */
    char npmax;                 /* nombre de param�res maximum dans la signature */
    char npvar;                 /* nombre d'arguments variables ? */
    char native;                /* native method */
    struct _CLASS *class;
    }
  PACKED
  CLASS_DESC_METHOD;

typedef
  struct {
    char *name;
    TYPE type;                  /* type de la valeur de retour */
    int *index;                /* num�o de l'��ement */
    TYPE *signature;            /* signature */
    char npmin;                 /* nombre de param�res minimum */
    char npmax;                 /* nombre de param�res maximum dans la signature */
    char npvar;                 /* nombre d'arguments variables ? */
    char _reserved;
    struct _CLASS *class;
    }
  PACKED
  CLASS_DESC_EVENT;

typedef
  struct {
    char *name;
    TYPE type;                  /* type de la valeur de retour */
    int exec;                  /* Index a ex�uter */
    TYPE *signature;            /* signature */
    char npmin;                 /* nombre de param�res minimum */
    char npmax;                 /* nombre de param�res maximum dans la signature */
    char npvar;                 /* nombre d'arguments variables ? */
    char _reserved;
    struct _CLASS *class;
    }
  PACKED
  CLASS_DESC_EXTERN;

typedef
  struct {
    char *name;
    TYPE type;                  /* type de constante */
    union {
      int _integer;
      double _float;
      char *_string;
      int64_t _long;
      void *_pointer;
      }
      value;
    }
  PACKED
  CLASS_DESC_CONSTANT;

typedef
  struct {
    char *name;
    void (*func)();
    }
  PACKED
  CLASS_DESC_HOOK;
#endif

typedef
  union {
    ptr32_t name;
    TYPE_32 type;
    ptr32_t val1;
    ptr32_t val2;
    ptr32_t val3;
    ptr32_t val4;
    }
  PACKED
  CLASS_DESC_GAMBAS_32;

typedef
  union {
    CLASS_DESC_GAMBAS_32 gambas;
    }
  PACKED
  CLASS_DESC_32;


typedef
  struct {
    CLASS_DESC_32 *desc;
    CLASS_CONST_32 *cst;
    CLASS_VAR_32 *stat;
    CLASS_VAR_32 *dyn;
    FUNCTION_32 *func;
    CLASS_EVENT_32 *event;
    CLASS_EXTERN_32 *ext;
    CLASS_ARRAY_32 **array;
    struct _CLASS **class_ref;
    char **unknown;
    GLOBAL_SYMBOL_32 *global;
    }
  PACKED
  CLASS_LOAD_32;

#endif

#endif
