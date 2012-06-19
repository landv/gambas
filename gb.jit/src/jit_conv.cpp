/***************************************************************************

  jit_conv.c

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

#define __JIT_CONV_C

#include "jit.h"

static const int type_to_bits[] = {0, 1, 8, 16, 32, 64, 32, 64};

void JIT_conv(Expression*& value, TYPE type, Expression* other){
	static const void *jump[16][16] =
	{
	/*  ,------->  void       b          c          h          i          l          g          f          d          cs         s          p          v          func       class      n         */
	//  |
	/* void   */ { &&__OK,    &&__NR,    &&__NR,    &&__NR,    &&__NR,    &&__NR,    &&__NR,    &&__NR,    &&__NR,    &&__NR,    &&__NR,    &&__NR,    &&__NR,    &&__NR,    &&__NR,    &&__NR,    },
	/* b      */ { &&__N,     &&__OK,    &&__TYPE,  &&__TYPE,  &&__TYPE,  &&__TYPE,  &&__TYPE,  &&__TYPE,  &&__N,     &&__TYPE,  &&__TYPE,  &&__N,     &&__TYPE,  &&__N,     &&__N,     &&__N,     },
	/* c      */ { &&__N,     &&__c2b,   &&__OK,    &&__TYPE,  &&__TYPE,  &&__TYPE,  &&__TYPE,  &&__TYPE,  &&__TYPE,  &&__TYPE,  &&__TYPE,  &&__N,     &&__TYPE,  &&__N,     &&__N,     &&__N,     },
	/* h      */ { &&__N,     &&__h2b,   &&__TYPE,  &&__OK,    &&__TYPE,  &&__TYPE,  &&__TYPE,  &&__TYPE,  &&__TYPE,  &&__TYPE,  &&__TYPE,  &&__N,     &&__TYPE,  &&__N,     &&__N,     &&__N,     },
	/* i      */ { &&__N,     &&__i2b,   &&__TYPE,  &&__TYPE,  &&__OK,    &&__TYPE,  &&__TYPE,  &&__TYPE,  &&__TYPE,  &&__TYPE,  &&__TYPE,  &&__TYPE,  &&__TYPE,  &&__N,     &&__N,     &&__N,     },
	/* l      */ { &&__N,     &&__l2b,   &&__TYPE,  &&__TYPE,  &&__TYPE,  &&__OK,    &&__TYPE2, &&__TYPE,  &&__TYPE,  &&__TYPE,  &&__TYPE,  &&__TYPE,  &&__TYPE,  &&__N,     &&__N,     &&__N,     },
	/* g      */ { &&__N,     &&__g2b,   &&__TYPE,  &&__TYPE,  &&__TYPE,  &&__TYPE,  &&__OK,    &&__TYPE,  &&__TYPE,  &&__TYPE,  &&__TYPE,  &&__N,     &&__TYPE,  &&__N,     &&__N,     &&__N,     },
	/* f      */ { &&__N,     &&__f2b,   &&__TYPE,  &&__TYPE,  &&__TYPE,  &&__TYPE,  &&__TYPE2, &&__OK,    &&__TYPE,  &&__TYPE,  &&__TYPE,  &&__N,     &&__TYPE,  &&__N,     &&__N,     &&__N,     },
	/* d      */ { &&__N,     &&__TYPE,  &&__TYPE,  &&__TYPE,  &&__TYPE,  &&__TYPE,  &&__TYPE,  &&__TYPE,  &&__OK,    &&__TYPE,  &&__TYPE,  &&__N,     &&__TYPE,  &&__N,     &&__N,     &&__N,     },
	/* cs     */ { &&__N,     &&__TYPE,  &&__TYPE2, &&__TYPE2, &&__TYPE2, &&__TYPE2, &&__TYPE2, &&__TYPE2, &&__TYPE2, &&__OK,    &&__OK,    &&__N,     &&__TYPE,  &&__N,     &&__N,     &&__N,     },
	/* s      */ { &&__N,     &&__TYPE,  &&__TYPE2, &&__TYPE2, &&__TYPE2, &&__TYPE2, &&__TYPE2, &&__TYPE2, &&__TYPE2, &&__OK,    &&__OK,    &&__N,     &&__TYPE,  &&__N,     &&__N,     &&__N,     },
	/* p      */ { &&__N,     &&__N,     &&__N,     &&__N,     &&__TYPE,  &&__TYPE,  &&__N,     &&__N,     &&__N,     &&__N,     &&__N,     &&__OK,    &&__TYPE,  &&__N,     &&__N,     &&__N,     },
	/* v      */ { &&__N,     &&__v2,    &&__v2,    &&__v2,    &&__v2,    &&__v2,    &&__v2,    &&__v2,    &&__v2,    &&__v2,    &&__v2,    &&__v2,    &&__OK,    &&__N,     &&__v2,    &&__v2,    },
	/* func   */ { &&__N,     &&__func,  &&__func,  &&__func,  &&__func,  &&__func,  &&__func,  &&__func,  &&__func,  &&__func,  &&__func,  &&__F2p,  &&__func, &&__OK,    &&__N,     &&__func,  },
	/* class  */ { &&__N,     &&__N,     &&__N,     &&__N,     &&__N,     &&__N,     &&__N,     &&__N,     &&__N,     &&__N,     &&__N,     &&__N,     &&__TYPE,  &&__N,     &&__OK,    &&__N,     },
	/* null   */ { &&__N,     &&__n2b,   &&__N,     &&__N,     &&__N,     &&__N,     &&__N,     &&__N,     &&__n2d,   &&__n2s,   &&__n2s,   &&__N,     &&__TYPE,  &&__N,     &&__N,     &&__OK,    },
	};
	
	if (type == T_CSTRING)
		type = T_STRING;

	if (value->type == (TYPE)-1) //Unknown, so always do a VALUE_convert
		goto __TYPE2;
	else if ((type | value->type) >> 4)
		goto __OBJECT;
	else
		goto *jump[value->type][type];

__c2b:
__h2b:
__i2b:
__l2b: {

	bool on_stack_save = value->on_stack;
	bool ref_stack_save = value->stack_if_ref;
	Expression* arr[] = {value, new PushIntegerExpression(type_to_bits[value->type], 0)};
	value = new NotExpression(new EqExpression(arr));
	value->on_stack = on_stack_save;
	value->stack_if_ref = ref_stack_save;
	return;
}

__g2b:
__f2b: {

	bool on_stack_save = value->on_stack;
	bool ref_stack_save = value->stack_if_ref;
	Expression* arr[] = {value, new PushFloatExpression(type_to_bits[value->type], 0.0)};
	value = new NotExpression(new EqExpression(arr));
	value->on_stack = on_stack_save;
	value->stack_if_ref = ref_stack_save;
	return;	
}

__n2b: {

	bool on_stack_save = value->on_stack;
	bool ref_stack_save = value->stack_if_ref;
	//delete value;
	value = new PushIntegerExpression(1, false);
	value->on_stack = on_stack_save;
	value->stack_if_ref = ref_stack_save;
	return;
}

__n2d: {

	bool on_stack_save = value->on_stack;
	bool ref_stack_save = value->stack_if_ref;
	//delete value;
	value = new PushVoidDateExpression();
	value->on_stack = on_stack_save;
	value->stack_if_ref = ref_stack_save;
	return;
}

__n2s: {

	bool on_stack_save = value->on_stack;
	bool ref_stack_save = value->stack_if_ref;
	//delete value;
	value = new PushCStringExpression(NULL, 0, 0);
	value->on_stack = on_stack_save;
	value->stack_if_ref = ref_stack_save;
	return;
}

__F2p: {
	if (PushFunctionExpression* pfe = dynamic_cast<PushFunctionExpression*>(value)){
		bool on_stack_save = value->on_stack;
		VALUE_FUNCTION v;
		v.type = T_FUNCTION;
		v.klass = CP;
		v.object = OP;
		v.kind = FUNCTION_PRIVATE;
		v.index = pfe->index;
		v.defined = true;
		if (OP)
			((OBJECT*)OP)->ref++;
		value = new PushIntegerExpression(8*sizeof(void*), (int64_t)(void*)JIF.F_EXTERN_make_callback(&v));
		JIT_conv(value, T_POINTER);
		value->on_stack = on_stack_save;
		return;
	}
	assert(false && "Not implemented yet!");
}

__func:

	//if (unknown_function(value))
	//	goto __CONV;
	//else
	goto __N;

__v2:

	ref_stack();
	value->must_on_stack();
	value = new ConvExpression(value, type);
	value->on_stack = true;
	return;

__OBJECT:

	if (TYPE_is_pure_object(type))
		JIT_load_class((CLASS*)(void*)type);
	
	if (TYPE_is_pure_object(value->type))
		JIT_load_class((CLASS*)(void*)value->type);
	
	if (!TYPE_is_object(type))
	{
		if (type == T_BOOLEAN)
		{
			goto __TYPE;
		}

		if (type == T_VARIANT)
			goto __TYPE;
		
		goto __N;
	}

	if (!TYPE_is_object(value->type))
	{
		if (value->type == T_NULL)
		{
			goto __TYPE;
		}

		if (value->type == T_VARIANT)
			goto __TYPE2;

		if (value->type == T_FUNCTION)
			goto __func;

		if (value->type == T_CLASS)
		{
			//Virtual, Auto create or 'Class' object
			PushClassExpression* pce = dyn_cast<PushClassExpression>(value);
			assert(pce);
			
			CLASS* klass = pce->klass;
			
			if (CLASS_is_virtual(klass))
				THROW(E_VIRTUAL);
			
			if (klass->auto_create){
				Expression* tmp = new PushAutoCreateExpression(klass);
				tmp->on_stack = value->on_stack;
				tmp->stack_if_ref = value->stack_if_ref;
				//delete value;
				value = tmp;
			} else {
				//Get the 'Class' object from the object
				if (type != T_OBJECT && type != GB.FindClass("Class"))
					goto __TYPE2; //This only works if there is a SPEC_CONVERT
				goto __TYPE;
			}
		}
		else
			goto __N;
	}
	
	if (type == value->type)
		return;
	
	if (value->type != T_OBJECT)
		if (((CLASS*)(void*)value->type)->is_virtual)
			THROW(E_VIRTUAL);
	
	/*if (value->type != T_OBJECT && type != T_OBJECT){
		if (!CLASS_inherits((CLASS*)(void*)value->type, (CLASS*)(void*)type)
			&& !CLASS_inherits((CLASS*)(void*)type, (CLASS*)(void*)value->type)){
			goto __N;
		}
	} Does not work for e.g. Integer[] -> Float[] ...*/

	if (type == T_OBJECT)
		goto __TYPE;
	else
		goto __TYPE2;

	

__TYPE: {

	bool on_stack = value->on_stack;
	bool stack_if_ref = value->stack_if_ref;
	//value->on_stack = false;
	//value->stack_if_ref = false;
	value = new ConvExpression(value, type);
	value->on_stack = on_stack;
	value->stack_if_ref = stack_if_ref;
	return;
}

__TYPE2: {

	if (other)
		other->must_on_stack();
	ref_stack();
	bool on_stack = value->on_stack;
	bool stack_if_ref = value->stack_if_ref;
	value->on_stack = true;
	value = new ConvExpression(value, type);
	value->on_stack = on_stack;
	value->stack_if_ref = stack_if_ref;
	
	
}

__OK:

	return;

__N:

	THROW(E_TYPE, JIF.F_TYPE_get_name(type), JIF.F_TYPE_get_name(value->type));

__NR:

	THROW(E_NRETURN);
}
