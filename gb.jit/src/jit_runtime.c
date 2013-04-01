/***************************************************************************

  jit_runtime.c

  gb.jit component

  (c) 2012 Emil Lenngren <emil.lenngren [at] gmail.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 1, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
  MA 02110-1301, USA.

***************************************************************************/

#define __JIT_RUNTIME_C

#include "gb_common.h"
#include "gbx_type.h"
#include "gbx_string.h"
//#include "gbx_object.h"
//#include "gbx_exec.h"
#include "gbx_number.h"

#include "main.h"

#include "jit_runtime.h"

#define JR_STRING_unref(_p) \
({ \
  char **pptr = _p; \
  char *ptr = *pptr; \
  STRING *str; \
  if (LIKELY(ptr != NULL)) \
  { \
	  str = STRING_from_ptr(ptr); \
  	if ((--str->ref) <= 0) \
  	{ \
	  	JIF.F_STRING_free_real(ptr); \
    	*pptr = NULL; \
    } \
  } \
})

#define JR_OBJECT_unref(_object) \
{ \
	if (_object) \
	{ \
		if ((--((OBJECT *)(_object))->ref) <= 0) \
		{ \
			void *temp = (void *)(_object); \
			_object = NULL; \
			JIF.F_CLASS_free(temp); \
		} \
	} \
}

#define MANAGE_VARIANT(_func) \
({ \
	if (P1->type == T_VARIANT) \
		JIF.F_VALUE_undo_variant(P1); \
	if (P2->type == T_VARIANT) \
		JIF.F_VALUE_undo_variant(P2); \
	\
	if (TYPE_is_string(P1->type)) \
		JIF.F_VALUE_convert_float(P1); \
	\
	if (TYPE_is_string(P2->type)) \
		JIF.F_VALUE_convert_float(P2); \
	\
	if (TYPE_is_null(P1->type) || TYPE_is_null(P2->type)) \
		type = T_NULL; \
	else \
		type = Max(P1->type, P2->type); \
	\
	if (TYPE_is_number_date(type)) \
	{ \
		(_func)(code | type); \
		JIF.F_VALUE_convert_variant(P1); \
		return; \
	} \
})

#define MANAGE_VARIANT_POINTER(_func) \
({ \
	if (P1->type == T_VARIANT) \
		JIF.F_VALUE_undo_variant(P1); \
	if (P2->type == T_VARIANT) \
		JIF.F_VALUE_undo_variant(P2); \
	\
	if (TYPE_is_string(P1->type)) \
		JIF.F_VALUE_convert_float(P1); \
	\
	if (TYPE_is_string(P2->type)) \
		JIF.F_VALUE_convert_float(P2); \
	\
	if (TYPE_is_null(P1->type) || TYPE_is_null(P2->type)) \
		type = T_NULL; \
	else \
		type = Max(P1->type, P2->type); \
	\
	if (TYPE_is_number_date(type) || TYPE_is_pointer(type)) \
	{ \
		(_func)(code | type); \
		JIF.F_VALUE_convert_variant(P1); \
		return; \
	} \
})

static const char JR_EXEC_should_borrow[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 2, 2, 0, 0 };

#define JR_RELEASE(_value) \
do { \
	VALUE *_v = (_value); \
	TYPE type = _v->type; \
	if (TYPE_is_object(type)) \
	{ \
		JR_OBJECT_unref(_v->_object.object); \
	} \
	else if (JR_EXEC_should_borrow[type]) \
	{ \
		if (type == T_STRING) \
			JR_STRING_unref(&_v->_string.addr); \
		else \
			JIF.F_EXEC_release(type, _v); \
	} \
} while (0)

CLASS_DESC_METHOD *JR_CLASS_get_special_desc(CLASS *class, int spec)
{
	short index = class->special[spec];

	if (index == NO_SYMBOL)
		return NULL;
	else
		return &CLASS_get_desc(class, index)->method;
}

typedef
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
VARIANT_value;

void JR_release_variant(long vtype, char* val){
	if (vtype == T_STRING)
		JR_STRING_unref(&val);
	else if (TYPE_is_object(vtype))
		JR_OBJECT_unref(val);
}

void JR_borrow_variant(long vtype, char* val){
	if (vtype == T_STRING)
		STRING_ref(val);
	else if (TYPE_is_object(vtype))
		OBJECT_REF(val);
}

static double JR_date_to_float(DATE dateval){
	double date = dateval.date;
	double time = dateval.time;
	return date + time / 86400000.0;
}

static double JR_string_to_float(char* addr){
	VALUE val;
	int len = STRING_from_ptr(addr)->len;
	int res = JIF.F_NUMBER_from_string(NB_READ_FLOAT, addr, len, &val);
	JR_STRING_unref(&addr);
	if (res)
		THROW(E_TYPE, "Number", JIF.F_TYPE_get_name(T_STRING));
	return val._float.value;
}

void JR_aq_variant(int add){
	static void *_aq_jump[] = {
		NULL, &&__AQ_BOOLEAN, &&__AQ_BYTE, &&__AQ_SHORT, &&__AQ_INTEGER, &&__AQ_LONG, &&__AQ_SINGLE, &&__AQ_FLOAT, 
		&&__AQ_DATE, &&__AQ_STRING, &&__AQ_STRING, &&__AQ_POINTER
	};
	
		TYPE NO_WARNING(type);
		int NO_WARNING(value);
		VALUE * NO_WARNING(P1);
		void * NO_WARNING(jump_end);
	
		P1 = SP - 1;
	
		jump_end = &&__AQ_VARIANT_END;
		JIF.F_VALUE_undo_variant(P1);
	
		type = P1->type;
		value = add;
	
		if (LIKELY(type <= T_POINTER))
			goto *_aq_jump[type];
	
	__AQ_BOOLEAN:
		
		THROW(E_TYPE, "Number", JIF.F_TYPE_get_name(type));
	
	__AQ_BYTE:
		
		P1->_integer.value = (unsigned char)(P1->_integer.value + value);
		goto *jump_end;
	
	__AQ_SHORT:
	
		P1->_integer.value = (short)(P1->_integer.value + value);
		goto *jump_end;
	
	__AQ_INTEGER:
	
		P1->_integer.value += value;
		goto *jump_end;
	
	__AQ_LONG:
	
		P1->_long.value += (int64_t)value;
		goto *jump_end;
	
	__AQ_SINGLE:
	
		P1->_single.value += (float)value;
		goto *jump_end;
	
	__AQ_DATE:
	__AQ_STRING:
		
		JIF.F_VALUE_convert_float(P1);
	
	__AQ_FLOAT:
	
		P1->_float.value += (double)value;
		goto *jump_end;
	
	__AQ_POINTER:
	
		P1->_pointer.value += value;
		goto *jump_end;
	
	__AQ_VARIANT_END:
	
		JIF.F_VALUE_convert_variant(P1);
	
}

void JR_variant_equal(){
	static void *jump[17] = {
		&&__SC_VARIANT, &&__SC_BOOLEAN, &&__SC_BYTE, &&__SC_SHORT, &&__SC_INTEGER, &&__SC_LONG, &&__SC_SINGLE, &&__SC_FLOAT, &&__SC_DATE,
		&&__SC_STRING, &&__SC_STRING, &&__SC_POINTER, &&__SC_ERROR, &&__SC_ERROR, &&__SC_ERROR, &&__SC_NULL, &&__SC_OBJECT
		};

	char NO_WARNING(result);
	VALUE *NO_WARNING(P1);
	VALUE *NO_WARNING(P2);
	
	P1 = SP - 2;
	P2 = SP - 1;

	goto __SC_VARIANT;

__SC_BOOLEAN:
__SC_BYTE:
__SC_SHORT:
__SC_INTEGER:

	result = P1->_integer.value == P2->_integer.value;
	goto __SC_END;
	
__SC_LONG:

	if (P1->type != T_LONG)
		JIF.F_VALUE_convert(P1, T_LONG);
	if (P2->type != T_LONG)
		JIF.F_VALUE_convert(P2, T_LONG);

	result = P1->_long.value == P2->_long.value;
	goto __SC_END;

__SC_DATE:

	if (P1->type != T_DATE)
		JIF.F_VALUE_convert(P1, T_DATE);
	if (P2->type != T_DATE)
		JIF.F_VALUE_convert(P2, T_DATE);

	result = JIF.F_DATE_comp((DATE *)&P1->_date.date, (DATE *)&P2->_date.date) == 0;
	goto __SC_END;

__SC_NULL:

	if (P2->type == T_NULL)
	{
		result = JIF.F_VALUE_is_null(P1);
		goto __SC_END_RELEASE;
	}
	else if (P1->type == T_NULL)
	{
		result = JIF.F_VALUE_is_null(P2);
		goto __SC_END_RELEASE;
	}

__SC_STRING:

	if (!TYPE_is_string(P1->type))
		JIF.F_VALUE_convert_string(P1);
	if (!TYPE_is_string(P2->type))
		JIF.F_VALUE_convert_string(P2);

	if (P1->_string.len != P2->_string.len)
		result = 0;
	else
		//result = STRING_equal_same(P1->_string.addr + P1->_string.start, P2->_string.addr + P2->_string.start, P1->_string.len);
		result = memcmp(P1->_string.addr + P1->_string.start, P2->_string.addr + P2->_string.start, P1->_string.len) == 0;
	
	if (P1->type == T_STRING)
		JR_STRING_unref(&P1->_string.addr);
	if (P2->type == T_STRING)
		JR_STRING_unref(&P2->_string.addr);
	goto __SC_END;

__SC_SINGLE:

	if (P1->type != T_SINGLE)
		JIF.F_VALUE_convert(P1, T_SINGLE);
	if (P2->type != T_SINGLE)
		JIF.F_VALUE_convert(P2, T_SINGLE);

	result = P1->_single.value == P2->_single.value;
	goto __SC_END;

__SC_FLOAT:

	if (P1->type != T_FLOAT)
		JIF.F_VALUE_convert_float(P1);
	if (P2->type != T_FLOAT)
		JIF.F_VALUE_convert_float(P2);

	result = P1->_float.value == P2->_float.value;
	goto __SC_END;

__SC_POINTER:

	if (P1->type != T_POINTER)
		JIF.F_VALUE_convert(P1, T_POINTER);
	if (P2->type != T_POINTER)
		JIF.F_VALUE_convert(P2, T_POINTER);

	result = P1->_pointer.value == P2->_pointer.value;
	goto __SC_END;

__SC_OBJECT:

	result = JIF.F_OBJECT_comp_value(P1, P2) == 0;
	//RELEASE_OBJECT(P1);
	//RELEASE_OBJECT(P2);
	goto __SC_END_RELEASE;

__SC_VARIANT:

	{
		TYPE type;
	
		if (TYPE_is_variant(P1->type))
		{
			JIF.F_VALUE_undo_variant(P1);
		}

		if (TYPE_is_variant(P2->type))
		{
			JIF.F_VALUE_undo_variant(P2);
		}

		type = Max(P1->type, P2->type);

		if (TYPE_is_object_null(P1->type) && TYPE_is_object_null(P2->type))
			type = T_OBJECT;
		else if (TYPE_is_object(type))
			THROW(E_TYPE, "Object", JIF.F_TYPE_get_name(Min(P1->type, P2->type)));
		else if (TYPE_is_void(type))
			THROW(E_NRETURN);

		goto *jump[type];
	}

__SC_ERROR:

	THROW(E_TYPE, "Something comparable", JIF.F_TYPE_get_name(T_VARIANT));

__SC_END_RELEASE:

	JR_RELEASE(P1);
	JR_RELEASE(P2);

__SC_END:

	P1->type = T_BOOLEAN;
	SP--;
	P1->_boolean.value = -result;
}

void JR_variant_compi_less_than(void)
{
	static void *jump[17] = {
		&&__VARIANT, &&__BOOLEAN, &&__BYTE, &&__SHORT, &&__INTEGER, &&__LONG, &&__SINGLE, &&__FLOAT, &&__DATE,
		&&__STRING, &&__STRING, &&__POINTER, &&__ERROR, &&__ERROR, &&__ERROR, &&__NULL, &&__OBJECT
		};

	char NO_WARNING(result);
	VALUE *P1;
	VALUE *P2;
	TYPE type;

	P1 = SP - 2;
	P2 = P1 + 1;

	type = T_VARIANT;
	goto __VARIANT;

__BOOLEAN:
__BYTE:
__SHORT:
__INTEGER:

	result = P1->_integer.value < P2->_integer.value ? -1 : 0;
	goto __END;
	
__LONG:

	JIF.F_VALUE_convert(P1, T_LONG);
	JIF.F_VALUE_convert(P2, T_LONG);

	result = P1->_long.value < P2->_long.value ? -1 : 0;
	goto __END;

__DATE:

	JIF.F_VALUE_convert(P1, T_DATE);
	JIF.F_VALUE_convert(P2, T_DATE);

	result = JIF.F_DATE_comp((DATE *)&P1->_date.date, (DATE *)&P2->_date.date) < 0 ? -1 : 0;
	goto __END;

__NULL:
__STRING:

	JIF.F_VALUE_convert_string(P1);
	JIF.F_VALUE_convert_string(P2);

	result = JIF.F_STRING_compare(P1->_string.addr + P1->_string.start, P1->_string.len, P2->_string.addr + P2->_string.start, P2->_string.len) < 0 ? -1 : 0;
	
	if (P1->type == T_STRING)
		JR_STRING_unref(&P1->_string.addr);
	if (P2->type == T_STRING)
		JR_STRING_unref(&P2->_string.addr);
	goto __END;

__SINGLE:

	JIF.F_VALUE_convert(P1, T_SINGLE);
	JIF.F_VALUE_convert(P2, T_SINGLE);

	result = P1->_single.value < P2->_single.value ? -1 : 0;
	goto __END;

__FLOAT:

	JIF.F_VALUE_convert_float(P1);
	JIF.F_VALUE_convert_float(P2);

	result = P1->_float.value < P2->_float.value ? -1 : 0;
	goto __END;

__POINTER:

	JIF.F_VALUE_convert(P1, T_POINTER);
	JIF.F_VALUE_convert(P2, T_POINTER);

	result = P1->_pointer.value < P2->_pointer.value ? -1 : 0;
	goto __END;

__OBJECT:

	result = JIF.F_OBJECT_comp_value(P1, P2) < 0 ? -1 : 0;
	//RELEASE_OBJECT(P1);
	//RELEASE_OBJECT(P2);
	goto __END_RELEASE;

__VARIANT:

	{
		bool variant = FALSE;
	
		if (TYPE_is_variant(P1->type))
		{
			JIF.F_VALUE_undo_variant(P1);
			variant = TRUE;
		}

		if (TYPE_is_variant(P2->type))
		{
			JIF.F_VALUE_undo_variant(P2);
			variant = TRUE;
		}

		type = Max(P1->type, P2->type);

		if (type == T_NULL || TYPE_is_string(type))
		{
			TYPE typem = Min(P1->type, P2->type);
			if (!TYPE_is_string(typem))
				THROW(E_TYPE, JIF.F_TYPE_get_name(typem), JIF.F_TYPE_get_name(type));
		}
		else if (TYPE_is_object(type))
			goto __ERROR;

		goto *jump[type];
	}

__ERROR:

	THROW(E_TYPE, "Number, Date or String", JIF.F_TYPE_get_name(type));

__END_RELEASE:

	JR_RELEASE(P1);
	JR_RELEASE(P2);

__END:

	P1->type = T_BOOLEAN;
	P1->_boolean.value = result;
	SP--;
	return;
}
void JR_add(ushort code)
{
	static void *jump[] = {
		&&__VARIANT, &&__BOOLEAN, &&__BYTE, &&__SHORT, &&__INTEGER, &&__LONG, &&__SINGLE, &&__FLOAT, &&__DATE, NULL, NULL, &&__POINTER
		};

	TYPE type;
	VALUE *P1, *P2;

	P1 = SP - 2;
	P2 = P1 + 1;

	type = code & 0x0F;
	goto *jump[type];

__BOOLEAN:
	
	P1->type = T_BOOLEAN;
	P1->_integer.value = P1->_integer.value | P2->_integer.value; goto __END;

__BYTE:
	
	P1->type = T_BYTE;
	P1->_integer.value = (unsigned char)(P1->_integer.value + P2->_integer.value); goto __END;

__SHORT:
	
	P1->type = T_SHORT;
	P1->_integer.value = (short)(P1->_integer.value + P2->_integer.value); goto __END;

__INTEGER:

	P1->type = T_INTEGER;
	P1->_integer.value += P2->_integer.value; goto __END;

__LONG:

	JIF.F_VALUE_convert(P1, T_LONG);
	JIF.F_VALUE_convert(P2, T_LONG);

	P1->_long.value += P2->_long.value; goto __END;

__SINGLE:

	JIF.F_VALUE_convert(P1, T_SINGLE);
	JIF.F_VALUE_convert(P2, T_SINGLE);

	P1->_single.value += P2->_single.value; goto __END;

__DATE:
__FLOAT:

	JIF.F_VALUE_convert_float(P1);
	JIF.F_VALUE_convert_float(P2);

	P1->_float.value += P2->_float.value; 
	//fprintf(stderr, "+: %.24g\n", P1->_float.value);
	goto __END;
	
__POINTER:

	JIF.F_VALUE_convert(P1, T_POINTER);
	JIF.F_VALUE_convert(P2, T_POINTER);

	P1->_pointer.value += (intptr_t)P2->_pointer.value; goto __END;

__VARIANT:

	MANAGE_VARIANT_POINTER(JR_add);
	goto __ERROR;

__ERROR:

	THROW(E_TYPE, "Number", JIF.F_TYPE_get_name(type));

__END:

	SP--;
}

void JR_sub(ushort code)
{
	static void *jump[] = {
		&&__VARIANT, &&__BOOLEAN, &&__BYTE, &&__SHORT, &&__INTEGER, &&__LONG, &&__SINGLE, &&__FLOAT, &&__DATE, NULL, NULL, &&__POINTER
		};

	TYPE type;
	VALUE *P1, *P2;

	P1 = SP - 2;
	P2 = P1 + 1;

	type = code & 0x0F;
	goto *jump[type];

__BOOLEAN:
	
	P1->type = T_BOOLEAN;
	P1->_integer.value = P1->_integer.value ^ P2->_integer.value; goto __END;

__BYTE:
	
	P1->type = T_BYTE;
	P1->_integer.value = (unsigned char)(P1->_integer.value - P2->_integer.value); goto __END;

__SHORT:
	
	P1->type = T_SHORT;
	P1->_integer.value = (short)(P1->_integer.value - P2->_integer.value); goto __END;

__INTEGER:

	P1->type = T_INTEGER;
	P1->_integer.value -= P2->_integer.value; goto __END;

__LONG:

	JIF.F_VALUE_convert(P1, T_LONG);
	JIF.F_VALUE_convert(P2, T_LONG);

	P1->_long.value -= P2->_long.value; goto __END;

__SINGLE:

	JIF.F_VALUE_convert(P1, T_SINGLE);
	JIF.F_VALUE_convert(P2, T_SINGLE);

	P1->_single.value -= P2->_single.value; goto __END;

__DATE:
__FLOAT:

	JIF.F_VALUE_convert_float(P1);
	JIF.F_VALUE_convert_float(P2);

	P1->_float.value -= P2->_float.value; goto __END;

__POINTER:

	JIF.F_VALUE_convert(P1, T_POINTER);
	JIF.F_VALUE_convert(P2, T_POINTER);

	P1->_pointer.value -= (intptr_t)P2->_pointer.value; goto __END;

__VARIANT:

	MANAGE_VARIANT_POINTER(JR_sub);
	goto __ERROR;

__ERROR:

	THROW(E_TYPE, "Number", JIF.F_TYPE_get_name(type));

__END:

	SP--;
}

void JR_mul(ushort code)
{
	static void *jump[] = {
		&&__VARIANT, &&__BOOLEAN, &&__BYTE, &&__SHORT, &&__INTEGER, &&__LONG, &&__SINGLE, &&__FLOAT, &&__ERROR
		};

	TYPE type;
	VALUE *P1, *P2;

	P1 = SP - 2;
	P2 = P1 + 1;

	type = code & 0x0F;
	goto *jump[type];

__BOOLEAN:
	
	P1->type = T_BOOLEAN;
	P1->_integer.value = P1->_integer.value & P2->_integer.value; goto __END;

__BYTE:
	
	P1->type = T_BYTE;
	P1->_integer.value = (unsigned char)(P1->_integer.value * P2->_integer.value); goto __END;

__SHORT:
	
	P1->type = T_SHORT;
	P1->_integer.value = (short)(P1->_integer.value * P2->_integer.value); goto __END;

__INTEGER:

	P1->type = T_INTEGER;
	P1->_integer.value *= P2->_integer.value; goto __END;

__LONG:

	JIF.F_VALUE_convert(P1, T_LONG);
	JIF.F_VALUE_convert(P2, T_LONG);

	P1->_long.value *= P2->_long.value; goto __END;

__SINGLE:

	JIF.F_VALUE_convert(P1, T_SINGLE);
	JIF.F_VALUE_convert(P2, T_SINGLE);

	P1->_single.value *= P2->_single.value; goto __END;

__FLOAT:

	JIF.F_VALUE_convert_float(P1);
	JIF.F_VALUE_convert_float(P2);

	P1->_float.value *= P2->_float.value;
	//fprintf(stderr, "*: %.24g\n", P1->_float.value);
	goto __END;

__VARIANT:

	MANAGE_VARIANT(JR_mul);
	goto __ERROR;

__ERROR:

	THROW(E_TYPE, "Number", JIF.F_TYPE_get_name(type));

__END:

	SP--;
}

void JR_push_unknown_property_unknown(const char *name, int name_id, CLASS *class, void *object){
	bool call_static = object == NULL;
	EXEC_unknown_name = name;
	JIF.F_EXEC_special(SPEC_PROPERTY, class, class->property_static ? NULL : object, 0, FALSE);
	if (class->unknown_static){
		JR_OBJECT_unref(object);
		object = NULL;
	}
	if (SP[-1]._boolean.value){
		SP--;
		EXEC_unknown_name = name;
		JIF.F_EXEC_special(SPEC_UNKNOWN, class, object, 0, FALSE);
		JIF.F_VALUE_convert_variant(&SP[-1]);
		if (!call_static){
			SP[-2]._variant = SP[-1]._variant;
			SP--;
		}
		JR_OBJECT_unref(object);
	} else {
		SP -= call_static ? 1 : 2;
		SP->type = T_FUNCTION;
		SP->_function.class = class;
		SP->_function.object = object;
		SP->_function.kind = FUNCTION_UNKNOWN;
		SP->_function.index = name_id;
		SP->_function.defined = FALSE;
		SP++;
	}
}

void JR_pop_unknown_property_unknown(CLASS *class, void *object, const char *name){
	EXEC_unknown_name = name;
	JIF.F_EXEC_special(SPEC_PROPERTY, class, object, 0, FALSE);
	SP--;
	if (!SP->_boolean.value)
		THROW(E_NPROPERTY, class->name, name);
		
	EXEC_unknown_name = name;

	*SP = SP[-2];
	SP[-2].type = T_VOID;
	SP++;

	JIF.F_EXEC_special(SPEC_UNKNOWN, class, object, 1, TRUE);
	JR_OBJECT_unref(object);
	
	SP -= 2;
}

void JR_call(int nparam){
	static const void *call_jump[] = 
	{
		&&__CALL_NULL, &&__CALL_NATIVE, &&__CALL_PRIVATE, &&__CALL_PUBLIC,
		NULL, &&__CALL_EXTERN, &&__CALL_UNKNOWN, &&__CALL_CALL, 
		NULL
	};

	VALUE * NO_WARNING(val);

	int ind = nparam;
	val = &SP[-(ind + 1)];

	if (UNLIKELY(!TYPE_is_function(val->type)))
	{
		bool defined = FALSE;
		if (TYPE_is_variant(val->type))
			EXEC.class = JIF.F_EXEC_object_variant(val, (OBJECT **)&EXEC.object);
		else
			JIF.F_EXEC_object_other(val, &EXEC.class, (OBJECT **)&EXEC.object);
		
		val->type = T_FUNCTION;
		val->_function.kind = FUNCTION_CALL;
		val->_function.defined = defined;
		val->_function.class = EXEC.class;
		val->_function.object = EXEC.object;
		//goto _CALL;
	}
	else
	{
		EXEC.class = val->_function.class;
		EXEC.object = val->_function.object;
	}

	EXEC.nparam = ind;
	EXEC.use_stack = TRUE;

	if (UNLIKELY(!val->_function.defined))
		*PC |= CODE_CALL_VARIANT;

	goto *call_jump[(int)val->_function.kind];

__CALL_NULL:

	while (ind > 0)
	{
		SP--;
		JR_RELEASE(SP);
		ind--;
	}

	SP--;
	JR_RELEASE(SP);

	//if (!PCODE_is_void(code))
	{
		/*VALUE_default(SP, (TYPE)(val->_function.function));*/
		SP->type = T_NULL;
		SP++;
	}

	return;

__CALL_NATIVE:

	EXEC.native = TRUE;
	EXEC.index = val->_function.index;
	EXEC.desc = &EXEC.class->table[EXEC.index].desc->method;
	//EXEC.use_stack = TRUE;

	goto __EXEC_NATIVE;

__CALL_PRIVATE:

	EXEC.native = FALSE;
	EXEC.index = val->_function.index;

	goto __EXEC_ENTER;

__CALL_PUBLIC:

	EXEC.native = FALSE;
	EXEC.desc = &EXEC.class->table[val->_function.index].desc->method;
	EXEC.index = (int)(intptr_t)(EXEC.desc->exec);
	EXEC.class = EXEC.desc->class;

	goto __EXEC_ENTER;

__EXEC_ENTER:

	JIF.F_EXEC_enter();
	if (EXEC.class->load->func[EXEC.index].fast)
		JR_EXEC_jit_execute_function();
	else
		JIF.F_EXEC_function_loop();
	return;

__EXEC_NATIVE:

	JIF.F_EXEC_native();
	return;

__CALL_UNKNOWN:

	EXEC_unknown_name = CP->load->unknown[val->_function.index];
	EXEC.desc = JR_CLASS_get_special_desc(EXEC.class, SPEC_UNKNOWN);
	//EXEC.use_stack = TRUE;
	goto __CALL_SPEC;

__CALL_CALL:

	EXEC.desc = JR_CLASS_get_special_desc(EXEC.class, SPEC_CALL);
	if (UNLIKELY(!EXEC.desc && !EXEC.object && EXEC.nparam == 1 && !EXEC.class->is_virtual))
	{
		SP[-2] = SP[-1];
		SP--;
		JIF.F_VALUE_convert(SP - 1, (TYPE)EXEC.class);
		return;
	}
	
	goto __CALL_SPEC;

__CALL_SPEC:

	if (UNLIKELY(!EXEC.desc))
		THROW(E_NFUNC);

	EXEC.native = FUNCTION_is_native(EXEC.desc);

	if (EXEC.native)
	{
		JIF.F_EXEC_native();
		return;
	}
	else
	{
		EXEC.index = (int)(intptr_t)(EXEC.desc->exec);
		EXEC.class = EXEC.desc->class;
		JIF.F_EXEC_enter();
		if (EXEC.class->load->func[EXEC.index].fast)
			JR_EXEC_jit_execute_function();
		else
			JIF.F_EXEC_function_loop();
		return;
	}

__CALL_EXTERN:

	EXEC.index = val->_function.index;
	JIF.F_EXTERN_call();
	return;
}

OBJECT* JR_object_cast(OBJECT* object, CLASS* target_class){
	CLASS* class = object->class;
	if ((class == target_class) || JIF.F_CLASS_inherits(class, target_class))
		return object;
	
	if (class->has_convert){
		OBJECT* conv = ((OBJECT *(*)())(class->convert))(object, target_class);
		if (conv){
			OBJECT_REF(conv);
			JR_OBJECT_unref(object);
			return conv;
		}
	}
	
	JR_OBJECT_unref(object);
	
	THROW(E_TYPE, JIF.F_TYPE_get_name((TYPE)(void*)target_class), JIF.F_TYPE_get_name((TYPE)(void*)class));
}

void* JR_extern_dispatch_object(OBJECT* object, int index){
	if (object == NULL)
		THROW(E_NULL);
	CLASS* class = object->class;
	JR_OBJECT_unref(object);
	return JIF.F_EXTERN_get_function_info(&class->load->ext[class->table[index].desc->ext.exec]).call;
}

void JR_exec_enter_quick(CLASS* klass, void* object, int index){
	CLASS_DESC_METHOD* desc = &klass->table[index].desc->method;
	EXEC.desc = desc;
	EXEC.index = (int)(intptr_t)(desc->exec);
	EXEC.class = EXEC.desc->class;
	EXEC.object = object;
	JIF.F_EXEC_enter_quick();
	if (FP->fast)
		JR_EXEC_jit_execute_function();
	else
		JIF.F_EXEC_function_loop();
}
void JR_exec_enter(CLASS* klass, void* object, int index){
	CLASS_DESC_METHOD* desc = &klass->table[index].desc->method;
	EXEC.desc = desc;
	EXEC.index = (int)(intptr_t)(desc->exec);
	EXEC.class = EXEC.desc->class;
	EXEC.object = object;
	JIF.F_EXEC_enter();
	if (FP->fast)
		JR_EXEC_jit_execute_function();
	else
		JIF.F_EXEC_function_loop();
}

void JR_EXEC_jit_execute_function(){
	(*CP->jit_functions[EXEC.index])();
}

/*void JR_call_native_or_public(CLASS* klass, void* object, int index, int nparam){
	CLASS_DESC_METHOD* desc = &klass->table[index].desc->method;
	if (desc->native){
		if (desc->subr){
		} else {
			EXEC.native = TRUE;
			EXEC.index = index;
			EXEC.desc = desc;
			EXEC.class = klass;
			EXEC.object = object;
			EXEC.nparam = nparam;
		}
	} else {
		EXEC.desc = desc;
		EXEC.index = (int)(intptr_t)(desc->exec);
		EXEC.class = EXEC.desc->class;
		EXEC.object = object;
		EXEC_enter();
		EXEC_jit_execute_function();
	}
}*/
void* JR_try(ERROR_CONTEXT* err){
	ERROR_enter(err);
	return err->env;
}
void JR_end_try(ERROR_CONTEXT* err){
	ERROR_leave(err);
}

//FIXME Can this happen: Try native_call -> do non_native call -> non_native throw, directly back to first try?
//FIXME FIXME exec_function_keep måste ju sätta upp en try catch .. väl ..
void JR_try_unwind(VALUE* stack_start){
	JIF.F_ERROR_set_last(EP != NULL ? FALSE : TRUE);
	
	JIF.F_ERROR_lock();
	while(EC == NULL){
		JIF.F_EXEC_leave_drop();
	}
	while(SP > stack_start){
		SP--;
		JR_RELEASE(SP);
	}
	JIF.F_ERROR_unlock();
	
	EP = NULL;
}
