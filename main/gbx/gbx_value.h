/***************************************************************************

  gbx_value.h

  (c) 2000-2009 Benoît Minisini <gambas@users.sourceforge.net>

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
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

***************************************************************************/

#ifndef __GBX_VALUE_H
#define __GBX_VALUE_H

#include <time.h>

#include "gbx_type.h"
#include "gbx_class.h"

typedef
  struct {
    TYPE type;
    int value;
    }
  VALUE_BOOLEAN;

typedef
  struct {
    TYPE type;
    int value;
    }
  VALUE_BYTE;

typedef
  struct {
    TYPE type;
    int value;
    }
  VALUE_SHORT;

typedef
  struct {
    TYPE type;
    int value;
    }
  VALUE_INTEGER;

typedef
  struct {
    TYPE type;
		#ifndef OS_64BITS
		int _padding;
		#endif
    int64_t value;
    }
  PACKED
  VALUE_LONG;

typedef
  struct {
    TYPE type;
    intptr_t value;
    }
  PACKED
  VALUE_POINTER;

typedef
  struct {
    TYPE type;
		#ifndef OS_64BITS
		int _padding;
		#endif
    double value;
    }
  PACKED
  VALUE_SINGLE;

typedef
  struct {
    TYPE type;
		#ifndef OS_64BITS
		int _padding;
		#endif
    double value;
    }
  PACKED
  VALUE_FLOAT;

typedef
  struct {
    TYPE type;
    int date;  /* number of days */
    int time;  /* number of milliseconds */
    }
  VALUE_DATE;

typedef
  struct {
    TYPE type;
    char *addr;
    int start;
    int len;
    }
  VALUE_STRING;

typedef
  struct {
    TYPE type;
    CLASS *class;
    void *object;
    char kind;
    char defined;
    short index;
    }
  PACKED
  VALUE_FUNCTION;

enum
{
  FUNCTION_NULL = 0,
  FUNCTION_NATIVE = 1,
  FUNCTION_PRIVATE = 2,
  FUNCTION_PUBLIC = 3,
  FUNCTION_EVENT = 4,
  FUNCTION_EXTERN = 5,
  FUNCTION_UNKNOWN = 6,
  FUNCTION_CALL = 7,
};

typedef
  struct {
    TYPE type;
    TYPE ptype;
    intptr_t value[2];
    }
  VALUE_VOID;

typedef
  struct {
    TYPE type;
    TYPE vtype;
		union {
			char _boolean;
			unsigned char _byte;
			short _short;
			int _integer;
			int64_t _long;
			float _single;
			double _float;
			char *_string;
			void *_object;
			int64_t data;
			}
			value;
    }
  VALUE_VARIANT;

typedef
  struct {
    CLASS *class;
    void *object;
    void *super;
    }
  VALUE_OBJECT;

typedef
  struct {
    TYPE type;
    CLASS *class;
    void *super;
    }
  VALUE_CLASS;

typedef
  struct {
    TYPE type;
    CLASS *class;
    void *addr;
    short index;
    unsigned keep : 1;
		unsigned _reserved : 15;
    }
  VALUE_ARRAY;

typedef
  union value {
    TYPE type;
    VALUE_BOOLEAN _boolean;
    VALUE_BYTE _byte;
    VALUE_SHORT _short;
    VALUE_INTEGER _integer;
    VALUE_LONG _long;
    VALUE_SINGLE _single;
    VALUE_FLOAT _float;
    VALUE_DATE _date;
    VALUE_STRING _string;
    VALUE_FUNCTION _function;
    VALUE_VARIANT _variant;
    VALUE_CLASS _class;
    VALUE_OBJECT _object;
    VALUE_ARRAY _array;
    VALUE_VOID _void;
    VALUE_POINTER _pointer;
    }
  VALUE;

#define VALUE_copy(_dst, _src) \
	(_dst)->_void.type = (_src)->_void.type; \
	(_dst)->_void.ptype = (_src)->_void.ptype; \
	(_dst)->_void.value[0] = (_src)->_void.value[0]; \
	(_dst)->_void.value[1] = (_src)->_void.value[1];

#define VALUE_is_object(val) (TYPE_is_object((val)->type))
#define VALUE_is_string(val) ((val)->type == T_STRING || (val)->type == T_CSTRING)
#define VALUE_is_number(val) ((val)->type >= T_BYTE && (val)->type <= T_FLOAT)

void VALUE_default(VALUE *value, TYPE type);
void VALUE_convert(VALUE *value, TYPE type);
void VALUE_read(VALUE *value, void *addr, TYPE type);
void VALUE_write(VALUE *value, void *addr, TYPE type);

//void VALUE_put(VALUE *value, void *addr, TYPE type);

void VALUE_free(void *addr, TYPE type);
void VALUE_to_string(VALUE *value, char **addr, int *len);
void VALUE_from_string(VALUE *value, const char *addr, int len);

void VALUE_class_read(CLASS *class, VALUE *value, char *addr, CTYPE ctype);
void VALUE_class_write(CLASS *class, VALUE *value, char *addr, CTYPE ctype);
void VALUE_class_constant(CLASS *class, VALUE *value, int ind);

bool VALUE_is_null(VALUE *val);

void VALUE_get_string(VALUE *val, char **text, int *length);

#define VALUE_conv(_value, _type) \
{ \
	if ((_value)->type != (_type)) \
		VALUE_convert(_value, _type); \
}

#define VALUE_conv_string(_value) \
{ \
	if ((_value)->type != T_STRING && (_value)->type != T_CSTRING) \
		VALUE_convert(_value, T_STRING); \
}


#define VALUE_is_super(_value) (EXEC_super && EXEC_super == (_value)->_object.super)

#endif
