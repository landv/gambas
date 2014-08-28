/***************************************************************************

  jit_codegen_conv.h

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

#ifndef __JIT_CODEGEN_CONV_H
#define __JIT_CODEGEN_CONV_H

extern "C" {
#include "gbx_number.h"
#include "gb_common_buffer.h"
}

#ifdef __CYGWIN__
#define __finite finite
#endif

llvm::Value* JIT_conv_to_variant(Expression* value, llvm::Value* val, bool on_stack, bool* no_ref_variant){
	llvm::Value* ret;
	if (TYPE_is_string(value->type)){
		ret = string_for_array_or_variant(val, value->type);
		ret = get_new_struct(variant_type, getInteger(TARGET_BITS, T_STRING), builder->CreatePtrToInt(ret, llvmType(getInt64Ty)));
	} else {
		if (value->type < T_OBJECT && no_ref_variant)
			*no_ref_variant = true;
		
		llvm::Value* data;
		llvm::Type* t64 = llvmType(getInt64Ty);
		
		if (value->type < T_OBJECT)
			ret = get_new_struct(variant_type, getInteger(TARGET_BITS, value->type));
		else
			ret = get_new_struct(variant_type, builder->CreatePtrToInt(extract_value(val, 0), LONG_TYPE));
		
		switch(value->type){
			case T_BYTE:
				data = builder->CreateZExt(val, t64);
				break;
			case T_BOOLEAN:
			case T_SHORT:
			case T_INTEGER:
				data = builder->CreateSExt(val, t64);
				break;
			case T_LONG:
				data = val;
				break;
			case T_SINGLE:
				data = builder->CreateBitCast(val, llvmType(getInt32Ty));
				data = builder->CreateZExt(data, t64);
				break;
			case T_FLOAT:
				data = builder->CreateBitCast(val, t64);
				break;
			case T_DATE:
				data = builder->CreateShl(builder->CreateZExt(extract_value(val, 1), t64), getInteger(64, 32));
				data = builder->CreateOr(data, builder->CreateZExt(extract_value(val, 0), t64));
				break;
			case T_POINTER:
				data = builder->CreatePtrToInt(val, t64);
				break;
			case T_CLASS:
				assert(dynamic_cast<PushClassExpression*>(value));
				data = getInteger(64, (uint64_t)(void*)((PushClassExpression*)value)->klass);
				val = builder->CreateIntToPtr(data, llvmType(getInt8PtrTy));
				break;
			case T_NULL:
				break;
			default:
				data = builder->CreatePtrToInt(extract_value(val, 1), t64);
				break;
		}
		if (value->type != T_NULL)
			ret = insert_value(ret, data, 1);
		
		if (on_stack){ //FIXME is this code really good/correct? stack seems strange
			c_SP(-value->on_stack+1);
			llvm::Value* addr = builder->CreateBitCast(get_value_on_top_addr(), pointer_t(LONG_TYPE));
			
			builder->CreateStore(getInteger(TARGET_BITS, T_VARIANT), addr);
			
			addr = builder->CreateGEP(addr, getInteger(TARGET_BITS, 1));
			if (value->type < T_OBJECT)
				builder->CreateStore(getInteger(TARGET_BITS, value->type), addr);
			else
				builder->CreateStore(builder->CreatePtrToInt(extract_value(val, 0), LONG_TYPE), addr);
			
			if (value->type != T_NULL){
				addr = builder->CreateGEP(addr, getInteger(TARGET_BITS, 1));
				if (value->type == T_BYTE)
					builder->CreateStore(builder->CreateZExt(val, llvmType(getInt32Ty)), builder->CreateBitCast(addr, llvmType(getInt32PtrTy)));
				else if (value->type < T_INTEGER)
					builder->CreateStore(builder->CreateSExt(val, llvmType(getInt32Ty)), builder->CreateBitCast(addr, llvmType(getInt32PtrTy)));
				else if (value->type < T_OBJECT)
					builder->CreateStore(val, builder->CreateBitCast(addr, pointer_t(TYPE_llvm(value->type))));
				else
					builder->CreateStore(extract_value(val, 1), builder->CreateBitCast(addr, charPP));
			}
			return ret;
		}
	}
	c_SP(-value->on_stack+on_stack);
	if (on_stack)
		set_top_value(ret, T_VARIANT);
	return ret;
}

llvm::Value* ConvExpression::codegen_get_value()
{
	Expression* value = expr;
	static const void *jump[16][16] =
	{
	/*  ,------->  void       b          c          h          i          l          g          f          d          cs         s          p          v          func       class      n         */
	//  |
	/* void   */ { &&__OK,    &&__NR,    &&__NR,    &&__NR,    &&__NR,    &&__NR,    &&__NR,    &&__NR,    &&__NR,    &&__NR,    &&__NR,    &&__NR,    &&__NR,    &&__NR,    &&__NR,    &&__NR,    },
	/* b      */ { &&__N,     &&__OK,    &&__b2c,   &&__b2h,   &&__b2i,   &&__b2l,   &&__b2g,   &&__b2f,   &&__N,     &&__b2s,   &&__b2s,   &&__N,     &&__2v,    &&__N,     &&__N,     &&__N,     },
	/* c      */ { &&__N,     NULL,      &&__OK,    &&__c2h,   &&__c2i,   &&__c2l,   &&__c2g,   &&__c2f,   &&__c2d,   &&__c2s,   &&__c2s,   &&__N,     &&__2v,    &&__N,     &&__N,     &&__N,     },
	/* h      */ { &&__N,     NULL,      &&__h2c,   &&__OK,    &&__h2i,   &&__h2l,   &&__h2g,   &&__h2f,   &&__h2d,   &&__h2s,   &&__h2s,   &&__N,     &&__2v,    &&__N,     &&__N,     &&__N,     },
	/* i      */ { &&__N,     NULL,      &&__i2c,   &&__i2h,   &&__OK,    &&__i2l,   &&__i2g,   &&__i2f,   &&__i2d,   &&__i2s,   &&__i2s,   &&__i2p,   &&__2v,    &&__N,     &&__N,     &&__N,     },
	/* l      */ { &&__N,     NULL,      &&__l2c,   &&__l2h,   &&__l2i,   &&__OK,    &&__l2g,   &&__l2f,   &&__l2d,   &&__l2s,   &&__l2s,   &&__l2p,   &&__2v,    &&__N,     &&__N,     &&__N,     },
	/* g      */ { &&__N,     NULL,      &&__g2c,   &&__g2h,   &&__g2i,   &&__g2l,   &&__OK,    &&__g2f,   &&__g2d,   &&__g2s,   &&__g2s,   &&__N,     &&__2v,    &&__N,     &&__N,     &&__N,     },
	/* f      */ { &&__N,     NULL,      &&__f2c,   &&__f2h,   &&__f2i,   &&__f2l,   &&__f2g,   &&__OK,    &&__f2d,   &&__f2s,   &&__f2s,   &&__N,     &&__2v,    &&__N,     &&__N,     &&__N,     },
	/* d      */ { &&__N,     &&__d2b,   &&__d2c,   &&__d2h,   &&__d2i,   &&__d2l,   &&__d2g,   &&__d2f,   &&__OK,    &&__d2s,   &&__d2s,   &&__N,     &&__2v,    &&__N,     &&__N,     &&__N,     },
	/* cs     */ { &&__N,     &&__s2b,   &&__s2c,   &&__s2h,   &&__s2i,   &&__s2l,   &&__s2g,   &&__s2f,   &&__s2d,   &&__OK,    &&__OK,    &&__N,     &&__s2v,   &&__N,     &&__N,     &&__N,     },
	/* s      */ { &&__N,     &&__s2b,   &&__s2c,   &&__s2h,   &&__s2i,   &&__s2l,   &&__s2g,   &&__s2f,   &&__s2d,   &&__OK,    &&__OK,    &&__N,     &&__s2v,   &&__N,     &&__N,     &&__N,     },
	/* p      */ { &&__N,     &&__N,     &&__N,     &&__N,     &&__p2i,   &&__p2l,   &&__N,     &&__N,     &&__N,     &&__N,     &&__N,     &&__OK,    &&__2v,    &&__N,     &&__N,     &&__N,     },
	/* v      */ { &&__N,     &&__v2,    &&__v2,    &&__v2,    &&__v2,    &&__v2,    &&__v2,    &&__v2,    &&__v2,    &&__v2,    &&__v2,    &&__v2,    &&__OK,    &&__N,     &&__v2,    &&__v2,    },
	/* func   */ { &&__N,     &&__func,  &&__func,  &&__func,  &&__func,  &&__func,  &&__func,  &&__func,  &&__func,  &&__func,  &&__func,  &&__F2p,   &&__func,  &&__OK,    &&__N,     &&__func,  },
	/* class  */ { &&__N,     &&__N,     &&__N,     &&__N,     &&__N,     &&__N,     &&__N,     &&__N,     &&__N,     &&__N,     &&__N,     &&__N,     &&__2v,    &&__N,     &&__OK,    &&__N,     },
	/* null   */ { &&__N,     NULL,      &&__N,     &&__N,     &&__N,     &&__N,     &&__N,     &&__N,     NULL,      NULL,      NULL,      &&__N,     &&__2v,    &&__N,     &&__N,     &&__OK,    },
	};

	llvm::Value* val = NULL;
	llvm::Value* ret;

	if (value->type == (TYPE)-1)
		goto __UNKNOWN;
	else if ((type | value->type) >> 4)
		goto __OBJECT;
	else {
		if (value->type != T_FUNCTION && value->type != T_CLASS)
			val = value->codegen_get_value();
		goto *jump[value->type][type];
	}


__d2b:
	ret = builder->CreateICmpNE(builder->CreateOr(extract_value(val, 0), extract_value(val, 1)), getInteger(32, 0));
	goto __DONE;

__b2c:
__b2h:
__b2i:
__h2i:
__b2l:
__h2l:
__i2l:

	ret = builder->CreateSExt(val, TYPE_llvm(type));
	goto __DONE;

__h2c:
__i2c:
__l2c:
__i2h:
__l2h:
__l2i:

	ret = builder->CreateTrunc(val, TYPE_llvm(type));
	goto __DONE;

__g2c:
__f2c:

	ret = builder->CreateFPToUI(val, llvmType(getInt8Ty));
	goto __DONE;

__c2h:
__c2i:
__c2l:

	ret = builder->CreateZExt(val, TYPE_llvm(type));
	goto __DONE;

__g2h:
__f2h:
__g2i:
__f2i:
__g2l:
__f2l:

	ret = builder->CreateFPToSI(val, TYPE_llvm(type));
	goto __DONE;
	
__p2i:
__p2l:

	ret = builder->CreatePtrToInt(val, TYPE_llvm(type));
	goto __DONE;

__c2g:
__c2f:

	ret = builder->CreateUIToFP(val, TYPE_llvm(type));
	goto __DONE;

__b2g:
__h2g:
__i2g:
__b2f:
__h2f:
__i2f:
__l2f:

	ret = builder->CreateSIToFP(val, TYPE_llvm(type));
	goto __DONE;

__l2g:

	ret = builder->CreateSIToFP(val, TYPE_llvm(type));
	goto __test_overflow;


__f2g:

	ret = builder->CreateFPTrunc(val, llvmType(getFloatTy));
	goto __test_overflow;
	
__test_overflow:
{
	llvm::Value* ret64 = type == T_SINGLE ? builder->CreateFPExt(ret, llvmType(getDoubleTy)) : ret;
	llvm::Value* res = builder->CreateCall(get_global_function(__finite, 'i', "d"), ret64);
	
	gen_if_noreturn(builder->CreateICmpEQ(res, getInteger(32, 0)), [&](){
		create_throw(E_OVERFLOW);
	}, "test_overflow");
	
	goto __DONE;
}

__g2f:

	ret = builder->CreateFPExt(val, llvmType(getDoubleTy));
	goto __DONE;

__c2d:
__h2d:
__i2d:

	ret = val;
	if (value->type != T_INTEGER){
		if (value->type == T_BYTE)
			ret = builder->CreateZExt(ret, llvmType(getInt32Ty));
		else
			ret = builder->CreateSExt(ret, llvmType(getInt32Ty));
	}
	ret = gen_max(getInteger(32, 0), ret);
	ret = get_new_struct(date_type, ret, getInteger(32, 0));
	goto __DONE;

__l2d:

	ret = builder->CreateSelect(
		builder->CreateICmpSLT(val, getInteger(64, 0)),
		getInteger(32, 0),
		builder->CreateSelect(
			builder->CreateICmpSGT(val, getInteger(64, INT_MAX)),
			getInteger(32, INT_MAX),
			builder->CreateTrunc(val, llvmType(getInt32Ty))
		)
	);
	ret = get_new_struct(date_type, ret, getInteger(32, 0));
	goto __DONE;

__g2d:
{
	llvm::Value* ival = builder->CreateCall(get_global_function(floorf, 'f', "f"), val);
	llvm::Value* timepart = builder->CreateFPToSI(builder->CreateFAdd(builder->CreateFMul(builder->CreateFSub(val, ival), getFloat(86400000.0f)), getFloat(0.5f)), llvmType(getInt32Ty));
	ret = get_new_struct(date_type, builder->CreateFPToSI(ival, llvmType(getInt32Ty)), timepart);
	goto __DONE;
}

__f2d:
{
	llvm::Value* ival = builder->CreateCall(get_global_function(floor, 'd', "d"), val);
	llvm::Value* timepart = builder->CreateFPToSI(builder->CreateFAdd(builder->CreateFMul(builder->CreateFSub(val, ival), getFloat(86400000.0)), getFloat(0.5)), llvmType(getInt32Ty));
	ret = get_new_struct(date_type, builder->CreateFPToSI(ival, llvmType(getInt32Ty)), timepart);
	goto __DONE;
}

__d2c:
__d2h:
__d2i:
__d2l:

	ret = extract_value(val, 0);
	if (type < T_INTEGER)
		ret = builder->CreateTrunc(ret, TYPE_llvm(type));
	else if (type == T_LONG)
		ret = builder->CreateSExt(ret, llvmType(getInt64Ty));
	goto __DONE;

__d2g:
{
	llvm::Value* datepart = builder->CreateSIToFP(extract_value(val, 0), llvmType(getFloatTy));
	llvm::Value* timepart = builder->CreateSIToFP(extract_value(val, 1), llvmType(getFloatTy));
	
	ret = builder->CreateFAdd(datepart, builder->CreateFDiv(timepart, getFloat(86400000.0f)));
	goto __DONE;
}

__d2f:
{
	llvm::Value* datepart = builder->CreateSIToFP(extract_value(val, 0), llvmType(getDoubleTy));
	llvm::Value* timepart = builder->CreateSIToFP(extract_value(val, 1), llvmType(getDoubleTy));
	
	ret = builder->CreateFAdd(datepart, builder->CreateFDiv(timepart, getFloat(86400000.0)));
	goto __DONE;
}

__b2s:

	ret = builder->CreateSelect(val,
		get_new_struct(string_type, getInteger(TARGET_BITS, T_CSTRING), get_global((void*)"T", llvmType(getInt8Ty)), getInteger(32, 0), getInteger(32, 1)),
		get_new_struct(string_type, getInteger(TARGET_BITS, T_CSTRING), get_nullptr(), getInteger(32, 0), getInteger(32, 0))
	);
	goto __DONE;

__c2s:
__h2s:
__i2s:
__l2s:
{
	llvm::Value* num;
	if (value->type == T_BYTE)
		num = builder->CreateZExt(val, llvmType(getInt64Ty));
	else if (value->type == T_LONG)
		num = val;
	else
		num = builder->CreateSExt(val, llvmType(getInt64Ty));
	
	c_SP(-value->on_stack+on_stack);
	
	llvm::Value* addr = on_stack ? get_value_on_top_addr() : temp_value;
	
	builder->CreateCall4(get_global_function_jif(NUMBER_int_to_string, 'v', "liip"),
		num, getInteger(32, 0), getInteger(32, 10), builder->CreateBitCast(addr, llvmType(getInt8PtrTy)));
	
	ret = read_value(addr, T_STRING);
	borrow(ret, T_STRING);
	return ret;
}

__g2s:
__f2s:
{
	c_SP(-value->on_stack+on_stack);
	
	if (value->type == T_SINGLE)
		val = builder->CreateFPExt(val, llvmType(getDoubleTy));
	
	llvm::Value* args[] = {
		val, getInteger(32, LF_GENERAL_NUMBER), get_nullptr(), getInteger(32, 0),
		builder->CreateBitCast(temp_voidptr, llvmType(getInt8PtrTy)),
		builder->CreateBitCast(temp_int, llvmType(getInt8PtrTy)), getInteger(8, 0)
	};
	/*llvm::Value* got_error =*/ builder->CreateCall(get_global_function_jif(LOCAL_format_number, 'c', "dipippc"), args);
	
	llvm::Value* addr = on_stack ? get_value_on_top_addr() : temp_value;
	
	builder->CreateCall3(get_global_function_jif(STRING_new_temp_value, 'v', "ppi"),
		builder->CreateBitCast(addr, llvmType(getInt8PtrTy)),
		builder->CreateLoad(temp_voidptr),
		builder->CreateLoad(temp_int)
	);
	
	ret = read_value(addr, T_STRING);
	borrow(ret, T_STRING);
	return ret;
}

__d2s:
{
	llvm::Value* src_addr = value->on_stack ? get_value_on_top_addr() : temp_value;
	if (!value->on_stack)
		store_value(src_addr, val, T_DATE, false);
	
	c_SP(-value->on_stack+on_stack);
	llvm::Value* addr = on_stack ? get_value_on_top_addr() : temp_value;
	
	static char buffer[128];
	
	llvm::Value* len = builder->CreateCall2(get_global_function_jif(DATE_to_string, 'i', "pp"),
		get_global((void*)buffer, llvmType(getInt8Ty)),
		builder->CreateBitCast(src_addr, llvmType(getInt8PtrTy))
	);
	builder->CreateCall3(get_global_function_jif(STRING_new_temp_value, 'v', "ppi"),
		builder->CreateBitCast(addr, llvmType(getInt8PtrTy)),
		get_global((void*)buffer, llvmType(getInt8Ty)),
		len
	);
	
	ret = read_value(addr, T_STRING);
	borrow(ret, T_STRING);
	return ret;
}

__s2b:

	ret = builder->CreateICmpNE(extract_value(val, 3), getInteger(32, 0));
	release(val, value->type);
	goto __DONE;

__s2c:
__s2h:
__s2i:
__s2l:
{
	auto str = get_string_len(val);
	
	c_SP(-value->on_stack+on_stack);
	llvm::Value* addr = (on_stack && (type == T_INTEGER || type == T_LONG)) ? get_value_on_top_addr() : temp_value;
	
	llvm::Value* got_error = builder->CreateCall4(get_global_function_jif(NUMBER_from_string, 'c', "ipip"),
		getInteger(32, type == T_LONG ? NB_READ_LONG : NB_READ_INTEGER), str.first, str.second,
		builder->CreateBitCast(addr, llvmType(getInt8PtrTy))
	);
	
	release(val, value->type);
	
	gen_if_noreturn(builder->CreateICmpNE(got_error, getInteger(8, 0)), [&](){
		create_throw(E_TYPE, JIF.F_TYPE_get_name(type), JIF.F_TYPE_get_name(value->type));
	});
	
	llvm::Value* intval = read_value(addr, type == T_LONG ? T_LONG : T_INTEGER);
	
	if (type < T_INTEGER)
		intval = builder->CreateTrunc(intval, TYPE_llvm(type));
	
	if (on_stack && addr == temp_value)
		set_top_value(intval, type);
	
	return intval;
}

__s2g:
__s2f:
{
	auto str = get_string_len(val);
	
	c_SP(-value->on_stack+on_stack);
	llvm::Value* addr = (on_stack && type == T_FLOAT) ? get_value_on_top_addr() : temp_value;
	
	llvm::Value* got_error = builder->CreateCall4(get_global_function_jif(NUMBER_from_string, 'c', "ipip"),
		getInteger(32, NB_READ_FLOAT), str.first, str.second,
		builder->CreateBitCast(addr, llvmType(getInt8PtrTy))
	);
	
	release(val, value->type);
	
	gen_if_noreturn(builder->CreateICmpNE(got_error, getInteger(8, 0)), [&](){
		create_throw(E_TYPE, JIF.F_TYPE_get_name(type), JIF.F_TYPE_get_name(value->type));
	});
	
	llvm::Value* ret = read_value(addr, T_FLOAT);
	
	if (type == T_SINGLE)
		ret = builder->CreateFPTrunc(ret, llvmType(getFloatTy));
	
	if (on_stack && addr == temp_value)
		set_top_value(ret, type);
	
	return ret;
}

__s2d:
{
	auto str = get_string_len(val);
	
	c_SP(-value->on_stack+on_stack);
	llvm::Value* addr = on_stack ? get_value_on_top_addr() : temp_value;
	
	llvm::Value* got_error = builder->CreateCall4(get_global_function_jif(DATE_from_string, 'c', "pipc"),
		str.first, str.second,
		builder->CreateBitCast(addr, llvmType(getInt8PtrTy)),
		getInteger(8, false)
	);
	
	release(val, value->type);
	
	gen_if_noreturn(builder->CreateICmpNE(got_error, getInteger(8, 0)), [&](){
		create_throw(E_TYPE, JIF.F_TYPE_get_name(type), JIF.F_TYPE_get_name(value->type));
	});
	
	return read_value(addr, T_DATE);
}

	/*addr = value->type == T_STRING ? value->_string.addr : NULL;

	if (DATE_from_string(value->_string.addr + value->_string.start, value->_string.len, value, FALSE))
		goto __N;

	STRING_unref(&addr);
	return;*/

__UNKNOWN:
	value->on_stack = true;
	value->codegen_on_stack();
__v2:

	builder->CreateCall2(get_global_function_jif(VALUE_convert, 'v', "pj"),
		builder->CreateBitCast(get_value_on_top_addr(), llvmType(getInt8PtrTy)),
		getInteger(TARGET_BITS, type));
	return ret_top_stack(type, on_stack);

	/*VALUE_undo_variant(value);
	goto __CONV;*/

__s2v:
__2v:
	return JIT_conv_to_variant(value, val, on_stack, &no_ref_variant);

	/* VALUE_put ne fonctionne pas avec T_STRING ! */
	/*if (value->type != T_NULL)
		VALUE_put(value, &value->_variant.value, value->type);

	value->_variant.vtype = value->type;
	value->type = T_VARIANT;
	return;*/

__func:

	//if (unknown_function(value))
	//	goto __CONV;
	//else
	goto __N;

__i2p:
__l2p:

	ret = builder->CreateIntToPtr(val, llvmType(getInt8PtrTy));
	goto __DONE;
	
__F2p:

	//FIXME
	abort();
	/*value->_pointer.value = EXTERN_make_callback(&value->_function);
	value->type = T_POINTER;
	return;*/

__OBJECT:
{
	if (!TYPE_is_object(type)){
		val = value->codegen_get_value();
		if (type == T_BOOLEAN){
			llvm::Value* obj = extract_value(val, 1);
			ret = builder->CreateICmpNE(obj, get_nullptr());
			unref_object(obj);
			goto __DONE;
		}
		//type == T_VARIANT
		goto __2v;
	}
	
	if (!TYPE_is_object(value->type)){
		if (value->type == T_NULL){
			value->on_stack = false;
			ret = get_new_struct(object_type, get_global((void*)type, llvmType(getInt8Ty)), get_nullptr());
			goto __DONE;
		}
		
		if (value->type == T_VARIANT){
			//Variant to Object
			val = value->codegen_get_value();
			goto __v2;
		}
		
		if (value->type == T_CLASS){
			llvm::Value* object = get_global((void*)((PushClassExpression*)value)->klass, llvmType(getInt8Ty));
			
			val = get_new_struct(object_type,
				get_global((void*)GB.FindClass("Class"), llvmType(getInt8Ty)),
				object);
			
			borrow_object_no_nullcheck(object);
			
			//To simplify code below
			value->on_stack = false;
			value->type = (TYPE)(void*)GB.FindClass("Class");
		}
	}
	
	if (val == NULL)
		val = value->codegen_get_value();
	
	/*llvm::Value* klass = extract_value(val, 0);*/
	llvm::Value* object = extract_value(val, 1);
	llvm::Value* to_class = get_global((void*)type, llvmType(getInt8Ty));
		
	if (type == T_OBJECT){
		ret = get_new_struct(object_type, builder->CreateIntToPtr(getInteger(TARGET_BITS, T_OBJECT), llvmType(getInt8PtrTy)), object);
		goto __DONE;
	}
	
	c_SP(-value->on_stack);
	
	ret = gen_if_else_phi(builder->CreateICmpEQ(object, get_nullptr()), [&](){
		//OK
		return get_new_struct(object_type, to_class, object);
	}, [&](){
		if (value->type == T_OBJECT){
			//klass = load_element(builder->CreateBitCast(klass, pointer_t(OBJECT_TYPE)), 0);
			
			//return get_new_struct(object_type, to_class, builder->CreateCall3(get_global_function(JR_conv_from_T_OBJECT, 'p', "pp"), object, to_class));
		}
		
		/*//Check if klass is virtual
		llvm::Value* class_flags = builder->CreateBitCast(builder->CreateGEP(klass, getInteger(TARGET_BITS, TARGET_BITS == 64 ? 32 : 20)), llvmType(getInt32Ty));
		llvm::Value* is_virtual = builder->CreateTrunc(builder->CreateLShr(class_flags, getInteger(32, 10)), llvmType(getInt1Ty));
		
		gen_if_noreturn(is_virtual, [&](){
			create_throw(E_VIRTUAL);
		}, "virtual_class");*/
		
		if (value->type != T_OBJECT){
			if (JIF.F_CLASS_inherits((CLASS*)(void*)value->type, (CLASS*)(void*)type)){
				//This cast is always possible
				return get_new_struct(object_type, to_class, object);
			}
			
			/*llvm::Value* first_invalid_test = builder->CreateICmpEQ(builder->CreateCall2(get_global_function(CLASS_inherits, 'c', "pp"), klass, to_class), getInteger(8, false));
			
			llvm::Value* invalid_cast = gen_and_if(builder->CreateICmpNE(klass, to_class), [&](){
				return builder->CreateICmpEQ(builder->CreateCall2(get_global_function(CLASS_inherits, 'c', "pp"), klass, to_class), getInteger(8, false));
			});*/
		}
		return get_new_struct(object_type, to_class, builder->CreateCall2(get_global_function(JR_object_cast, 'p', "pp"), object, to_class));
		
	});
	
	if (on_stack)
		push_value(ret, type);
	return ret;
}
#if 0
	if (!TYPE_is_object(type))
	{
		if (type == T_BOOLEAN)
		{
			test = (value->_object.object != NULL);
			OBJECT_UNREF(value->_object.object, "VALUE_convert");
			value->_boolean.value = -test;
			value->type = T_BOOLEAN;
			return;
		}

		if (type == T_VARIANT)
			goto __2v;
		
		goto __N;
	}

	if (!TYPE_is_object(value->type))
	{
		if (value->type == T_NULL)
		{
			OBJECT_null(value, (CLASS *)type); /* marche aussi pour type = T_OBJECT */
			goto __TYPE;
		}

		if (value->type == T_VARIANT)
			goto __v2;

		if (value->type == T_FUNCTION)
			goto __func;

		if (value->type == T_CLASS)
		{
			klass = value->_class.klass;
			
			if (CLASS_is_virtual(klass))
				THROW(E_VIRTUAL);
			
			CLASS_load(klass);

			if (klass->auto_create)
				value->_object.object = CLASS_auto_create(klass, 0);
			else
				value->_object.object = klass;

			OBJECT_REF(value->_object.object, "VALUE_convert");
			value->type = T_OBJECT;
			/* on continue... */
		}
		else
			goto __N;
	}

	if (value->_object.object == NULL)
		goto __TYPE;

	if (value->type == T_OBJECT)
	{
		/*if (value->_object.object == NULL)
			goto __TYPE;*/

		klass = OBJECT_class(value->_object.object);
		/* on continue */
	}
	else
		klass = value->_object.klass;

	if (CLASS_is_virtual(klass))
		THROW(E_VIRTUAL);

	if (type == T_OBJECT)
		goto __TYPE;
#endif

/*
	if ((klass == (CLASS *)type) || CLASS_inherits(klass, (CLASS *)type))
		goto __TYPE;

	if (value->type != T_OBJECT && value->_object.object)
	{
		klass = OBJECT_class(value->_object.object);
		value->type = T_OBJECT;
		goto __RETRY;
	}

	if (klass->special[SPEC_CONVERT] != NO_SYMBOL)
	{
		void *conv = ((void *(*)())(CLASS_get_desc(klass, klass->special[SPEC_CONVERT])->constant.value._pointer))(value->_object.object, type);
		if (conv)
		{
			OBJECT_REF(conv, "VALUE_conv");
			OBJECT_UNREF(value->_object.object, "VALUE_conv");
			value->_object.object = conv;
			goto __TYPE;
		}
	}

	THROW(E_TYPE, TYPE_get_name(type), TYPE_get_name((TYPE)klass));*/

__DONE:

	c_SP(-value->on_stack+on_stack);
	if (on_stack)
		set_top_value(ret, type);
	return ret;

__OK:

	abort();
	return val;

__N:

	THROW(E_TYPE, JIF.F_TYPE_get_name(type), JIF.F_TYPE_get_name(value->type));

__NR:

	THROW(E_NRETURN);
}

#endif /* __JIT_CODEGEN_CONV_H */
