/***************************************************************************

  class_desc.h

  Public class description

  (c) 2000-2007 Benoît Minisini <gambas@users.sourceforge.net>

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

#ifndef __GBX_CLASS_DESC_H
#define __GBX_CLASS_DESC_H

#include "gb_class_desc_common.h"

#define CD_PROPERTY               'p'
#define CD_PROPERTY_READ          'r'
#define CD_METHOD                 'm'
#define CD_CONSTANT               'C'
#define CD_EVENT                  ':'
#define CD_STATIC_PROPERTY        'P'
#define CD_STATIC_PROPERTY_READ   'R'
#define CD_STATIC_METHOD          'M'
#define CD_VARIABLE               'v'
#define CD_STATIC_VARIABLE        'V'
#define CD_EXTERN                 'X'

#define CD_STATIC_LIST    				"PRMVX"
#define CD_CALL_SOMETHING_LIST		"prmPRM"


typedef
  struct {
    char *name;
    TYPE type;                  /* property type */
    void (*read)();             /* read property */
    void (*write)();            /* write property */
    char native;                /* native property ? */
    char _reserved[3];
		#ifdef OS_64BITS
		int _reserved2;
		#endif
    struct _CLASS *class;
    }
  CLASS_DESC_PROPERTY;

typedef
  struct {
    char *name;
    TYPE type;                  /* variable type */
    int offset;   	            /* variable offset */
		#ifdef OS_64BITS
		int _reserved;
		#endif
		intptr_t _reserved2[2];
    struct _CLASS *class;
    }
  CLASS_DESC_VARIABLE;

typedef
  struct {
    char *name;
    TYPE type;                  /* type de la valeur de retour */
    void (*exec)();             /* m�hode */
    TYPE *signature;            /* signature */
    char npmin;                 /* nombre de param�res minimum */
    char npmax;                 /* nombre de param�res maximum dans la signature */
    char npvar;                 /* nombre d'arguments variables ? */
    char native;                /* native method */
		#ifdef OS_64BITS
		int _reserved;
		#endif
    struct _CLASS *class;
    }
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
		#ifdef OS_64BITS
		int _reserved2;
		#endif
    struct _CLASS *class;
    }
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
		#ifdef OS_64BITS
		int _reserved2;
		#endif
    struct _CLASS *class;
    }
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
		intptr_t _reserved;
    struct _CLASS *class;
    }
  CLASS_DESC_CONSTANT;

typedef
  struct {
    char *name;
    void (*func)();
    }
  CLASS_DESC_HOOK;

typedef
  struct {
    char *name;
    intptr_t type;
    intptr_t val1;
    intptr_t val2;
    intptr_t val3;
    intptr_t val4;
    }
  CLASS_DESC_GAMBAS;


typedef
  union {
    CLASS_DESC_PROPERTY property;
    CLASS_DESC_VARIABLE variable;
    CLASS_DESC_METHOD method;
    CLASS_DESC_CONSTANT constant;
    CLASS_DESC_EVENT event;
    CLASS_DESC_HOOK hook;
    CLASS_DESC_GAMBAS gambas;
    CLASS_DESC_EXTERN ext;
    }
  CLASS_DESC;

typedef
  struct {
    short sort;
    short len;
    char *name;
    CLASS_DESC *desc;
    }
  CLASS_DESC_SYMBOL;


#define CLASS_DESC_get_type(d) (*(d)->gambas.name)
#define CLASS_DESC_SELF (-1)

const char *CLASS_DESC_get_signature(CLASS_DESC *cd);
const char *CLASS_DESC_get_type_name(CLASS_DESC *desc);

#endif
