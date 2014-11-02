/***************************************************************************

  gbx_value.h

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

#ifndef __GBX_VALUE_H
#define __GBX_VALUE_H

#include <time.h>

#include "gbx_type.h"
#include "gbx_class.h"

#ifndef __DATE_DECLARED
#define __DATE_DECLARED
typedef
  struct {
    int date;
    int time;
    }
  DATE;
#endif

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
  VALUE_LONG;

typedef
  struct {
    TYPE type;
    char *value;
    }
  VALUE_POINTER;

typedef
  struct {
    TYPE type;
    float value;
    }
  VALUE_SINGLE;

typedef
  struct {
    TYPE type;
		#ifndef OS_64BITS
		int _padding;
		#endif
    double value;
    }
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
	FUNCTION_SUBR = 8
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
			DATE _date;
			char *_string;
			void *_pointer;
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
    VALUE_POINTER _pointer;
    VALUE_FUNCTION _function;
    VALUE_VARIANT _variant;
    VALUE_CLASS _class;
    VALUE_OBJECT _object;
    VALUE_VOID _void;
    }
  VALUE;

typedef
	void (*VALUE_CONVERT_FUNC)(VALUE *);
	
#define VALUE_copy(_dst, _src) \
	((_dst)->_void.type = (_src)->_void.type, \
	(_dst)->_void.ptype = (_src)->_void.ptype, \
	(_dst)->_void.value[0] = (_src)->_void.value[0], \
	(_dst)->_void.value[1] = (_src)->_void.value[1])

#define VALUE_is_equal(_v1, _v2) (*_v1 == *v2)
	
#define VALUE_is_object(val) (TYPE_is_object((val)->type))
#define VALUE_is_string(val) ((val)->type == T_STRING || (val)->type == T_CSTRING)
#define VALUE_is_number(val) ((val)->type >= T_BYTE && (val)->type <= T_FLOAT)

void VALUE_default(VALUE *value, TYPE type);

void VALUE_convert(VALUE *value, TYPE type);

void VALUE_convert_boolean(VALUE *value);
void VALUE_convert_integer(VALUE *value);
void VALUE_convert_float(VALUE *value);
void VALUE_convert_string(VALUE *value);
void VALUE_convert_variant(VALUE *value);

void VALUE_read(VALUE *value, void *addr, TYPE type);
void VALUE_write(VALUE *value, void *addr, TYPE type);

void VALUE_undo_variant(VALUE *value);

//void VALUE_put(VALUE *value, void *addr, TYPE type);

void VALUE_free(void *addr, TYPE type);
void VALUE_to_string(VALUE *value, char **addr, int *len);
void VALUE_from_string(VALUE *value, const char *addr, int len);

void VALUE_class_read(CLASS *class, VALUE *value, char *addr, CTYPE ctype, void *ref);
void VALUE_class_write(CLASS *class, VALUE *value, char *addr, CTYPE ctype);
void VALUE_class_constant(CLASS *class, VALUE *value, int ind);

bool VALUE_is_null(VALUE *val);

//void VALUE_get_string(VALUE *val, char **text, int *length);
#define VALUE_get_string(_value, _ptext, _plen) \
({ \
	*(_plen) = (_value)->_string.len; \
	if (*(_plen)) \
		*(_ptext) = (_value)->_string.start + (_value)->_string.addr; \
	else \
		*(_ptext) = NULL; \
})

void THROW_TYPE_INTEGER(TYPE type) NORETURN;
void THROW_TYPE_FLOAT(TYPE type) NORETURN;
void THROW_TYPE_STRING(TYPE type) NORETURN;

#define VALUE_conv(_value, _type) \
({ \
	if (UNLIKELY((_value)->type != (_type))) \
		VALUE_convert(_value, _type); \
})

#if 0

#define VALUE_conv_boolean(_value) \
({ \
	VALUE *v = _value; \
	if (UNLIKELY(v->type != T_BOOLEAN)) \
	{ \
		VALUE_convert_boolean(v); \
	} \
})

#define VALUE_conv_integer(_value) \
({ \
	VALUE *v = _value; \
	if (UNLIKELY(v->type != T_INTEGER)) \
	{ \
		if (TYPE_is_object(v->type)) \
			THROW_TYPE_INTEGER(v->type); \
		VALUE_convert_integer(v); \
	} \
})

#define VALUE_conv_float(_value) \
({ \
	VALUE *v = _value; \
	if (UNLIKELY(v->type != T_FLOAT)) \
	{ \
		if (TYPE_is_object(v->type)) \
			THROW_TYPE_FLOAT(v->type); \
		VALUE_convert_float(v); \
	} \
})

#define VALUE_conv_string(_value) \
({ \
	VALUE *v = _value; \
	if (UNLIKELY(v->type != T_STRING && v->type != T_CSTRING)) \
	{ \
		if (TYPE_is_object(v->type)) \
			THROW_TYPE_STRING(v->type); \
		VALUE_convert_string(v); \
	} \
})

#define VALUE_conv_variant(_value) \
({ \
	if (UNLIKELY((_value)->type != T_VARIANT)) \
		VALUE_convert_variant(_value); \
})

#define VALUE_conv_object(_value, _type) \
({ \
	if (UNLIKELY((_value)->type != (_type))) \
		VALUE_convert_object(_value, _type); \
})

#else

#define VALUE_conv_boolean(_value) \
({ \
	if (UNLIKELY((_value)->type != T_BOOLEAN)) \
		VALUE_convert_boolean(_value); \
})

#define VALUE_conv_float(_value) \
({ \
	if (UNLIKELY((_value)->type != T_FLOAT)) \
		VALUE_convert_float(_value); \
})

//#define VALUE_conv_boolean(_value) VALUE_conv(_value, T_BOOLEAN)
#define VALUE_conv_integer(_value) VALUE_conv(_value, T_INTEGER)
//#define VALUE_conv_float(_value) VALUE_conv(_value, T_FLOAT)
#define VALUE_conv_variant(_value) VALUE_conv(_value, T_VARIANT)
#define VALUE_conv_object(_value, _type) VALUE_conv(_value, _type)

#define VALUE_conv_string(_value) \
({ \
	if (UNLIKELY((_value)->type != T_STRING && (_value)->type != T_CSTRING)) \
		VALUE_conv(_value, T_STRING); \
})

#endif

#define VALUE_is_super(_value) (EXEC_super && EXEC_super == (_value)->_object.super)

#define VALUE_class_read_inline(_class, _value, _addr, _ctype, _ref) \
({ \
	static void *jump[17] = { \
		&&__VOID, &&__BOOLEAN, &&__BYTE, &&__SHORT, &&__INTEGER, &&__LONG, &&__SINGLE, &&__FLOAT, &&__DATE, \
		&&__STRING, &&__CSTRING, &&__POINTER, &&__VARIANT, &&__ARRAY, &&__STRUCT, &&__NULL, &&__OBJECT \
		}; \
	\
	for(;;) \
	{ \
		(_value)->type = (_ctype).id; \
		goto *jump[(_ctype).id]; \
		\
	__BOOLEAN: \
		(_value)->_boolean.value = -(*((unsigned char *)(_addr)) != 0); \
		break; \
		\
	__BYTE: \
		(_value)->_byte.value = *((unsigned char *)(_addr)); \
		break; \
		\
	__SHORT: \
		(_value)->_short.value = *((short *)(_addr)); \
		break; \
		\
	__INTEGER: \
		(_value)->_integer.value = *((int *)(_addr)); \
		break; \
		\
	__LONG: \
		(_value)->_long.value = *((int64_t *)(_addr)); \
		break; \
		\
	__SINGLE: \
		(_value)->_single.value = *((float *)(_addr)); \
		break; \
		\
	__FLOAT: \
		(_value)->_float.value = *((double *)(_addr)); \
		break; \
		\
	__DATE: \
		(_value)->_date.date = ((int *)(_addr))[0]; \
		(_value)->_date.time = ((int *)(_addr))[1]; \
		break; \
		\
	__STRING: \
		{ \
			char *str = *((char **)(_addr)); \
			\
			(_value)->type = T_STRING; \
			(_value)->_string.addr = str; \
			(_value)->_string.start = 0; \
			(_value)->_string.len = STRING_length(str); \
			\
			break; \
		} \
		\
	__CSTRING: \
		{ \
			char *str = *((char **)(_addr)); \
			\
			(_value)->type = T_CSTRING; \
			(_value)->_string.addr = str; \
			(_value)->_string.start = 0; \
			(_value)->_string.len = (str == NULL) ? 0 : strlen(str); \
			\
			break; \
		} \
		\
	__OBJECT: \
		(_value)->_object.object = *((void **)(_addr)); \
		(_value)->type = ((_ctype).value >= 0) ? (TYPE)(_class)->load->class_ref[(_ctype).value] : T_OBJECT; \
		break; \
		\
	__POINTER: \
		(_value)->_pointer.value = *((void **)(_addr)); \
		break; \
		\
	__VARIANT: \
		(_value)->_variant.type = T_VARIANT; \
		(_value)->_variant.vtype = ((VARIANT *)(_addr))->type; \
		if ((_value)->_variant.vtype == T_VOID) \
			(_value)->_variant.vtype = T_NULL; \
		VARIANT_copy_value(&(_value)->_variant, ((VARIANT *)(_addr))); \
		break; \
		\
	__ARRAY: \
		{ \
			void *object = CARRAY_create_static((_class), (_ref), (_class)->load->array[(_ctype).value], (_addr)); \
			(_value)->_object.class = OBJECT_class(object); \
			(_value)->_object.object = object; \
			break; \
		} \
		\
	__STRUCT: \
		{ \
			void *object = CSTRUCT_create_static((_ref), (_class)->load->class_ref[(_ctype).value], (_addr)); \
			(_value)->_object.class = OBJECT_class(object); \
			(_value)->_object.object = object; \
			break; \
		} \
		\
	__VOID: \
	__NULL: \
		THROW_ILLEGAL(); \
	} \
})

#define VALUE_class_constant_inline(_class, _value, _ind) \
({ \
	static void *jump[] = \
	{ \
		&&__ILLEGAL, &&__INTEGER, &&__INTEGER, &&__INTEGER, &&__INTEGER, &&__LONG, &&__SINGLE, &&__FLOAT, \
		&&__ILLEGAL, &&__STRING, &&__CSTRING, &&__POINTER, &&__ILLEGAL, &&__ILLEGAL, &&__ILLEGAL, &&__ILLEGAL \
	}; \
	\
	CLASS_CONST *cc; \
	\
	for(;;) \
	{ \
		cc = &(_class)->load->cst[_ind]; \
		goto *jump[cc->type]; \
		\
	__INTEGER: \
		\
		(_value)->type = T_INTEGER; \
		(_value)->_integer.value = cc->_integer.value; \
		break; \
		\
	__LONG: \
		\
		(_value)->type = T_LONG; \
		(_value)->_long.value = cc->_long.value; \
		break; \
		\
	__SINGLE: \
		\
		(_value)->type = T_SINGLE; \
		(_value)->_single.value = cc->_single.value; \
		break; \
		\
	__FLOAT: \
		\
		(_value)->type = T_FLOAT; \
		(_value)->_float.value = cc->_float.value; \
		break; \
		\
	__STRING: \
		\
		(_value)->type = T_CSTRING; \
		(_value)->_string.addr = (char *)cc->_string.addr; \
		(_value)->_string.start = 0; \
		(_value)->_string.len = cc->_string.len; \
		break; \
		\
	__CSTRING: \
		\
		(_value)->type = T_CSTRING; \
		(_value)->_string.addr = (char *)LOCAL_gettext(cc->_string.addr); \
		(_value)->_string.start = 0; \
		(_value)->_string.len = strlen((_value)->_string.addr); \
		break; \
		\
	__POINTER: \
		\
		(_value)->type = T_POINTER; \
		(_value)->_pointer.value = NULL; \
		break; \
		\
	__ILLEGAL: \
		\
		THROW_ILLEGAL(); \
	} \
})

#define VALUE_read_inline_type(_value, _addr, _ctype, _type, _label_noref, _label_ref) \
({ \
	static void *jump[17] = { \
		&&__VOID, &&__BOOLEAN, &&__BYTE, &&__SHORT, &&__INTEGER, &&__LONG, &&__SINGLE, &&__FLOAT, &&__DATE, \
		&&__STRING, &&__CSTRING, &&__POINTER, &&__VARIANT, &&__FUNCTION, &&__CLASS, &&__NULL, &&__OBJECT \
		}; \
	\
	for(;;) \
	{ \
		(_value)->type = (_type); \
		goto *jump[_ctype]; \
		\
	__BOOLEAN: \
		\
		(_value)->_boolean.value = (*((unsigned char *)(_addr)) != 0) ? (-1) : 0; \
		goto _label_noref; \
		\
	__BYTE: \
		\
		(_value)->_byte.value = *((unsigned char *)(_addr)); \
		goto _label_noref; \
		\
	__SHORT: \
		\
		(_value)->_short.value = *((short *)(_addr)); \
		goto _label_noref; \
		\
	__INTEGER: \
		\
		(_value)->_integer.value = *((int *)(_addr)); \
		goto _label_noref; \
		\
	__LONG: \
		\
		(_value)->_long.value = *((int64_t *)(_addr)); \
		goto _label_noref; \
		\
	__SINGLE: \
		\
		(_value)->_single.value = *((float *)(_addr)); \
		goto _label_noref; \
		\
	__FLOAT: \
		\
		(_value)->_float.value = *((double *)(_addr)); \
		goto _label_noref; \
		\
	__DATE: \
		\
		(_value)->_date.date = ((int *)(_addr))[0]; \
		(_value)->_date.time = ((int *)(_addr))[1]; \
		goto _label_noref; \
		\
	__STRING: \
		\
		{ \
			char *str = *((char **)(_addr)); \
			\
			(_value)->type = T_STRING; \
			(_value)->_string.addr = str; \
			(_value)->_string.start = 0; \
			(_value)->_string.len = STRING_length(str); \
			\
			goto _label_ref; \
		} \
		\
	__CSTRING: \
		\
		{ \
			char *str = *((char **)(_addr)); \
			\
			(_value)->type = T_CSTRING; \
			(_value)->_string.addr = str; \
			(_value)->_string.start = 0; \
			(_value)->_string.len = (str == NULL) ? 0 : strlen(str); \
			\
			goto _label_noref; \
		} \
		\
	__OBJECT: \
		\
		(_value)->_object.object = *((void **)(_addr)); \
		goto _label_ref; \
		\
	__POINTER: \
		\
		(_value)->_pointer.value = *((void **)(_addr)); \
		goto _label_noref; \
		break; \
		\
	__VARIANT: \
		\
		(_value)->_variant.type = T_VARIANT; \
		(_value)->_variant.vtype = ((VARIANT *)(_addr))->type; \
		\
		if ((_value)->_variant.vtype == T_VOID) \
			(_value)->_variant.vtype = T_NULL; \
		\
		VARIANT_copy_value(&(_value)->_variant, ((VARIANT *)(_addr))); \
		\
		goto _label_ref; \
		\
	__CLASS: \
		\
		(_value)->_class.class = *((void **)(_addr)); \
		(_value)->_class.super = NULL; \
		goto _label_noref; \
		\
	__VOID: \
	__FUNCTION: \
	__NULL: \
		THROW_ILLEGAL(); \
	} \
})

#endif
