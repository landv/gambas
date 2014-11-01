/***************************************************************************

  jit_codegen.c

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

#define __JIT_CODEGEN_C

#include <array>
#include <set>
#include <map>
#include <queue>
#include <algorithm>

#include "jit.h"

#include "jit_runtime.h"
#include "jit_gambas_pass.h"

#define class klass
extern "C" {
#include "gbx_struct.h"
/*#include "gbx_api.h"
#include "gbx_regexp.h"
#include "gbx_math.h"

#define throw _throw
#include "gbx_compare.h"
#undef throw*/
}
#undef class

#ifdef OS_64BITS
#define TARGET_BITS 64
#else
#define TARGET_BITS 32
#endif

#define GOSUB_ON_STACK

#define llvmType(t) llvm::Type::t(llvm_context)
#define pointer_t(type) llvm::PointerType::get((type), 0)
#define charPP llvm::PointerType::get(llvmType(getInt8PtrTy), 0)
#define get_nullptr() llvm::ConstantPointerNull::get(llvmType(getInt8PtrTy))
#define get_voidstring() get_new_struct(string_type, getInteger(TARGET_BITS, T_CSTRING), get_nullptr(), getInteger(32, 0), getInteger(32, 0));

#define LONG_TYPE (TARGET_BITS == 64 ? llvmType(getInt64Ty) : llvmType(getInt32Ty))

#ifdef __CYGWIN__
#define __finite finite
#define __isnan __isnand
#define __isinf __isinfd
#endif

const size_t TYPE_sizeof_memory_tab[16] = { 0, 1, 1, 2, 4, 8, 4, 8, 8, sizeof(void *), sizeof(void *), sizeof(void *), sizeof(VARIANT), 0, 0, 0 };

///DEBUG
void print_type(llvm::Value* v);

static std::set<std::string> mappings;
//static std::map<void*, llvm::GlobalVariable*> variable_mappings;

static llvm::LLVMContext llvm_context;

static llvm::StructType* string_type;
static llvm::StructType* date_type;
static llvm::StructType* function_type;
static llvm::StructType* variant_type;
static llvm::StructType* object_type;
static llvm::StructType* OBJECT_type;
static llvm::StructType* value_type;
static llvm::StructType* value_types[17];
static llvm::StructType* two_longs_type;
static llvm::StructType* gosub_stack_node_type;

static llvm::Module* M;
static llvm::ExecutionEngine* EE = NULL;

static llvm::Function* llvm_function;
static llvm::BasicBlock* entry_block;
static llvm::IRBuilder<>* builder;
static llvm::Value* temp_value;
static llvm::Value* temp_value2;
static llvm::Value* temp_voidptr;
static llvm::Value* temp_int;
static llvm::Value* temp_double;
static llvm::Value* temp_date;
static llvm::Value* temp_2longs;
static llvm::Value* temp_errcontext1;
static llvm::Value* temp_errcontext2;
static llvm::Value* temp_got_error;
static llvm::Value* temp_got_error2;
//static llvm::Value* temp_gosub_stack;
//static llvm::Value* temp_num_gosubs_on_stack;
static llvm::Value* gp;
static llvm::Value* gosub_return_point;

static std::vector<llvm::BasicBlock*> return_points;
static std::vector<llvm::BasicBlock*> addr_blocks; //addr_blocks[i] contains the basic block at code pos i, if the address is taken
struct PendingBranch {
	llvm::BasicBlock* insert_point;
	llvm::Value* condition;  //i1, NULL if unconditional branch
	int true_addr;
	int false_addr; //Not used if unconditional branch
};
static std::vector<PendingBranch> pending_branches;
struct JumpTablePendingBranch {
	llvm::BasicBlock* insert_point;
	llvm::Value* condition; //int32
	std::vector<int>* destinations;
	int default_addr;
};
static std::vector<JumpTablePendingBranch> jump_table_pending_branches;
static std::vector<llvm::BasicBlock*> gosub_continue_points;
static std::vector<llvm::BasicBlock*> gosub_return_points;

static std::vector<llvm::Value*> current_ctrl_types;
static std::vector<std::array<llvm::Value*, 4> > ctrl_values;
static std::vector<llvm::Value*> locals;
static std::vector<llvm::Value*> params;
static llvm::Value* current_op;

static bool in_try = false;
static bool has_tries = false;
static std::vector<llvm::BasicBlock*> try_blocks;

static unsigned short* statement_pc;

static llvm::ConstantInt* getInteger(int bits, uint64_t value){
	return llvm::ConstantInt::get(llvm_context, llvm::APInt(bits, value, true));
}

template <typename _FloatType_>
static llvm::ConstantFP* getFloat(_FloatType_ value){
	return llvm::ConstantFP::get(llvm_context, llvm::APFloat(value));
}

static llvm::Type* TYPE_llvm(TYPE type){
	if (TYPE_is_object(type))
		return object_type;
	
	static llvm::Type* const arr[] = {
		llvmType(getVoidTy),
		llvmType(getInt1Ty),
		llvmType(getInt8Ty),
		llvmType(getInt16Ty),
		llvmType(getInt32Ty),
		llvmType(getInt64Ty),
		llvmType(getFloatTy),
		llvmType(getDoubleTy),
		date_type,
		string_type,
		string_type,
		llvmType(getInt8PtrTy),
		variant_type,
		NULL,
		llvmType(getInt8PtrTy),
		NULL
	};
	return arr[type];
}

static llvm::Type* type_from_char(char c){
	switch (c){
		case 'v': return llvmType(getVoidTy);
		case 'b': return llvmType(getInt1Ty);
		case 'c': return llvmType(getInt8Ty);
		case 'h': return llvmType(getInt16Ty);
		case 'i': return llvmType(getInt32Ty);
		case 'l': return llvmType(getInt64Ty);
		case 'f': return llvmType(getFloatTy);
		case 'd': return llvmType(getDoubleTy);
		case 'p': return llvmType(getInt8PtrTy);
		
		case 'j': return LONG_TYPE;
	}
	return 0; //Error
}

static std::vector<llvm::Type*> string_to_type_vector(const char* args){
	std::vector<llvm::Type*> llvm_args;
	for(int i=0, e=strlen(args); i!=e; i++)
		llvm_args.push_back(type_from_char(args[i]));
	return llvm_args;
}

static llvm::FunctionType* get_function_type(char ret, const char* args, bool vararg = false){
	std::vector<llvm::Type*> llvm_args = string_to_type_vector(args);
	return llvm::FunctionType::get(type_from_char(ret), llvm_args, vararg);
}

static llvm::Type* get_value_type(TYPE type){
	if (TYPE_is_object(type)) return value_types[T_OBJECT];
	return value_types[type];
}

static llvm::BasicBlock* create_bb(const char* name){
	return llvm::BasicBlock::Create(llvm_context, name, llvm_function);
}

extern "C" double __powidf2(double, int);

static void llvm_init(){
	llvm::InitializeNativeTarget();
	
	//string_type = llvm::StructType::create(llvm_context, string_to_type_vector("jpii"), "String");
	date_type = llvm::StructType::create(llvm_context, string_to_type_vector("ii"), "Date");
	function_type = llvm::StructType::create(llvm_context, string_to_type_vector("ppcch"), "Function");
	variant_type = llvm::StructType::create(llvm_context, string_to_type_vector("jl"), "Variant", true);
	object_type = llvm::StructType::create(llvm_context, string_to_type_vector("pp"), "Object");
	OBJECT_type = llvm::StructType::create(llvm_context, string_to_type_vector("pj"), "OBJECT");
	value_type = llvm::StructType::create(llvm_context, string_to_type_vector("jjjj"), "Value");
	two_longs_type = llvm::StructType::create(llvm_context, string_to_type_vector("jj"), "TwoLongs");
	
#if TARGET_BITS == 64
	static const char* const arr[17] = {"jjjj", "ji", "ji", "ji", "ji", "jl", "jf", "jd", "jii", "jpii", NULL, "jp", "jjl", "jppcch", "jp", "j", "ppp"};
#else
	static const char* const arr[17] = {"jjjj", "ji", "ji", "ji", "ji", "jil", "jf", "jid", "jii", "jpii", NULL, "jp", "jjl", "jppcch", "jp", "j", "ppp"};
#endif
	static const char* const names[17] = {"Void", "Boolean", "Byte", "Short", "Integer", "Long", "Single", "Float", "Date", "String", NULL, "Pointer", "ValueVariant", "ValueFunction", "Class", "Null", "ValueObject"};
	for(int i=0; i<=16; i++)
		if (arr[i])
			value_types[i] = llvm::StructType::create(llvm_context, string_to_type_vector(arr[i]), names[i]);
	value_types[T_CSTRING] = value_types[T_STRING];
	
	string_type = value_types[T_STRING];
	
	{
		std::vector<llvm::Type*> llvm_args;
		//llvm_args.push_back(llvmType(getInt8PtrTy)); //Stores the blockaddress. Is a ushort PC in gbx_stack.h but this should work due to alignment ;)
		llvm_args.push_back(llvmType(getInt16Ty)); //Stores an id of the return point, 0 == none
		llvm_args.push_back(pointer_t(value_type));
		gosub_stack_node_type = llvm::StructType::create(llvm_context, llvm_args, "GosubStackNode");
	}
	
	llvm::sys::DynamicLibrary::AddSymbol("__powidf2", (void*)__powidf2);
}

void register_global_symbol(llvm::StringRef name, llvm::GlobalValue* value, void* address){
	if (mappings.insert(name).second)
		EE->addGlobalMapping(value, address);
}

static llvm::Value* get_global(void* var, llvm::Type* llvm_type = llvmType(getInt8PtrTy)){
	/*char buf[32];
	sprintf(buf, "v_%p", var);
	std::string name = buf;
	if (var == &SP)
		name = "SP";
	else if (var == &BP)
		name = "BP";
	else if (var == &CP)
		name = "CP";
	auto it = variable_mappings.insert(std::make_pair(var, (llvm::GlobalVariable*)0));
	if (it.second){
		llvm::GlobalVariable* v = new llvm::GlobalVariable(*M, llvm_type, false, llvm::GlobalValue::ExternalLinkage, 0, name);
		it.first->second = v;
		EE->addGlobalMapping(v, var);
		return v;
	} else {
		return it.first->second;
	}*/
	return builder->CreateIntToPtr(getInteger(TARGET_BITS, (int64_t)(var)), pointer_t(llvm_type));
}

#define get_global_function(n, ret, args) get_global_function_real(#n, (void*)n, ret, args)
#define get_global_function_jif(n, ret, args) get_global_function_real(#n, (void*)JIF.F_##n, ret, args)
#define get_global_function_vararg(n, ret, args) get_global_function_real(#n, (void*)n, ret, args, true)
#define get_global_function_jif_vararg(n, ret, args) get_global_function_real(#n, (void*)JIF.F_##n, ret, args, true)
static llvm::Value* get_global_function_real(const char* name, void* func, char ret, const char* args, bool vararg = false){
	llvm::FunctionType* ft = get_function_type(ret, args, vararg);
	//llvm::sys::DynamicLibrary::AddSymbol
	//return get_global(func, ft);
	llvm::GlobalValue* val = (llvm::GlobalValue*)M->getOrInsertFunction(name, ft);
	register_global_symbol(name, val, func);
	return val;
}

static llvm::Value* read_global(void* var, llvm::Type* pt = llvmType(getInt8PtrTy)){
	//llvm::Value* addr = builder->CreateIntToPtr(getInteger(TARGET_BITS, (int64_t)(var)), pointer_t(pt));
	llvm::Value* addr = get_global(var, pt);
	llvm::Value* v = builder->CreateLoad(addr);
	return v;
}

static llvm::Value* create_gep(llvm::Value* addr, int bits1, int val1, int bits2, int val2){
	llvm::Value* arr[] = {getInteger(bits1, val1), getInteger(bits2, val2)};
	return builder->CreateGEP(addr, arr);
}

static llvm::Value* get_element_addr(llvm::Value* addr, int index){
	return create_gep(addr, TARGET_BITS, 0, 32, index);
}

static llvm::Value* load_element(llvm::Value* addr, int index){
	return builder->CreateLoad(get_element_addr(addr, index));
}

static void store_element(llvm::Value* addr, int index, llvm::Value* val){
	builder->CreateStore(val, get_element_addr(addr, index));
}

static llvm::Value* get_new_struct(llvm::StructType* st){
	return llvm::UndefValue::get(st);
}

static llvm::Value* get_new_struct(llvm::StructType* st, llvm::Value* v1){
	static const unsigned int arr[1] = {0};
	return builder->CreateInsertValue(llvm::UndefValue::get(st), v1, arr);
}

static llvm::Value* get_new_struct(llvm::StructType* st, llvm::Value* v1, llvm::Value* v2){
	static const unsigned int arr[2][1] = {{0}, {1}};
	llvm::Value* val = builder->CreateInsertValue(llvm::UndefValue::get(st), v1, arr[0]);
	return builder->CreateInsertValue(val, v2, arr[1]);
}

static llvm::Value* get_new_struct(llvm::StructType* st, llvm::Value* v1, llvm::Value* v2, llvm::Value* v3){
	static const unsigned int arr[3][1] = {{0}, {1}, {2}};
	llvm::Value* val = builder->CreateInsertValue(llvm::UndefValue::get(st), v1, arr[0]);
	val = builder->CreateInsertValue(val, v2, arr[1]);
	return builder->CreateInsertValue(val, v3, arr[2]);
}

static llvm::Value* get_new_struct(llvm::StructType* st, llvm::Value* v1, llvm::Value* v2, llvm::Value* v3, llvm::Value* v4){
	static const unsigned int arr[5][1] = {{0}, {1}, {2}, {3}, {4}};
	llvm::Value* val = builder->CreateInsertValue(llvm::UndefValue::get(st), v1, arr[0]);
	val = builder->CreateInsertValue(val, v2, arr[1]);
	val = builder->CreateInsertValue(val, v3, arr[2]);
	return builder->CreateInsertValue(val, v4, arr[3]);
}

static llvm::Value* get_new_struct(llvm::StructType* st, llvm::Value* v1, llvm::Value* v2, llvm::Value* v3, llvm::Value* v4, llvm::Value* v5){
	static const unsigned int arr[5][1] = {{0}, {1}, {2}, {3}, {4}};
	llvm::Value* val = builder->CreateInsertValue(llvm::UndefValue::get(st), v1, arr[0]);
	val = builder->CreateInsertValue(val, v2, arr[1]);
	val = builder->CreateInsertValue(val, v3, arr[2]);
	val = builder->CreateInsertValue(val, v4, arr[3]);
	return builder->CreateInsertValue(val, v5, arr[4]);
}

static llvm::Value* extract_value(llvm::Value* val, int index){
	unsigned int arr[] = {index};
	return builder->CreateExtractValue(val, arr);
}

static llvm::Value* insert_value(llvm::Value* container, llvm::Value* value, int index){
	unsigned int arr[] = {index};
	return builder->CreateInsertValue(container, value, arr);
}

static llvm::Value* read_value(llvm::Value* addr, TYPE type){
	addr = builder->CreateBitCast(addr, pointer_t(get_value_type(type)));
	switch(type){
		case T_VOID: return NULL;
		case T_BOOLEAN:
			return builder->CreateICmpNE(load_element(addr, 1), getInteger(32, 0));
		case T_BYTE:
		case T_SHORT:
			return builder->CreateTrunc(load_element(addr, 1), TYPE_llvm(type));
		case T_LONG:
		case T_FLOAT:
#if TARGET_BITS == 32
			return load_element(addr, 2);
#endif
		case T_INTEGER:
		case T_SINGLE:
		case T_POINTER:
		case T_CLASS:
			return load_element(addr, 1);
		case T_DATE:
			return get_new_struct(date_type, load_element(addr, 1), load_element(addr, 2));
		case T_STRING:
		case T_CSTRING: {
			//return builder->CreateLoad(addr);
			llvm::Value* t = load_element(addr, 0);
			llvm::Value* res = get_new_struct(string_type, t, load_element(addr, 1), load_element(addr, 2), load_element(addr, 3));
			
			/*llvm::BasicBlock *bb1 = create_bb("bb1"), *bb2 = create_bb("bb2");
			
			builder->CreateCondBr(builder->CreateICmpEQ(t, getInteger(TARGET_BITS, 12)), bb1, bb2);
			builder->SetInsertPoint(bb1);
			builder->CreateCall(get_global_function(abort, 'v', ""));
			builder->CreateUnreachable();
			builder->SetInsertPoint(bb2);*/
			return res;
			//FIXME remove above - now outcommented!
			
			/*gen_if_noreturn(builder->CreateICmpEQ(t, getInteger(TARGET_BITS, 12)), [&](){
				builder->CreateCall(get_global_function(abort, 'v', ""));
			});*/
		}
		//case T_CSTRING:
		//	return get_new_struct(string_type, getInteger(1, true), load_element(addr, 1), load_element(addr, 2), load_element(addr, 3));
		case T_VARIANT: {
			llvm::Value* not_null = get_new_struct(variant_type, load_element(addr, 1), load_element(addr, 2));
			llvm::Value* if_null = get_new_struct(variant_type, getInteger(TARGET_BITS, T_NULL));
			return builder->CreateSelect(builder->CreateICmpNE(load_element(addr, 0), getInteger(TARGET_BITS, T_NULL)), not_null, if_null);
		}
		case T_FUNCTION:
			return get_new_struct(function_type, load_element(addr, 1), load_element(addr, 2), load_element(addr, 3), load_element(addr, 4), load_element(addr, 5));
		case T_NULL:
			abort();
		default: {
			//FIXME
			//builder->CreateCall2(get_global_function_vararg(printf, 'v', "p"), get_global((void*)"laddar %p\n", llvmType(getInt8Ty)), load_element(addr, 0));
			llvm::Value* is_null = load_element(builder->CreateBitCast(addr, pointer_t(value_types[T_NULL])), 0);
			is_null = builder->CreateICmpEQ(is_null, getInteger(TARGET_BITS, T_NULL));
			//llvm::Value* nullstruct = insert_value(get_new_struct(object_type), get_nullptr(), 1);
			llvm::Value* nullstruct = get_new_struct(object_type, get_global((void*)type, llvmType(getInt8Ty)), get_nullptr());
			return builder->CreateSelect(is_null, nullstruct, get_new_struct(object_type, load_element(addr, 0), load_element(addr, 1)));
		}
	}
}

static void store_value(llvm::Value* addr, llvm::Value* val, TYPE type, bool store_type = true){
	addr = builder->CreateBitCast(addr, pointer_t(get_value_type(type)));
	if (store_type && type != T_STRING)
		store_element(addr, 0, TYPE_is_object(type) ? builder->CreateIntToPtr(getInteger(TARGET_BITS, (int64_t)(intptr_t)type), llvmType(getInt8PtrTy)) : getInteger(TARGET_BITS, type));
	switch(type){
		case T_BOOLEAN:
		case T_SHORT:
			store_element(addr, 1, builder->CreateSExt(val, llvmType(getInt32Ty))); break;
		case T_BYTE:
			store_element(addr, 1, builder->CreateZExt(val, llvmType(getInt32Ty))); break;
		case T_LONG:
		case T_FLOAT:
#if TARGET_BITS == 32
			store_element(addr, 2, val); break;
#endif
		case T_INTEGER:
		case T_SINGLE:
		case T_POINTER:
		case T_CLASS:
			store_element(addr, 1, val); break;
		case T_DATE:
		case T_VARIANT:
			store_element(addr, 1, extract_value(val, 0));
			store_element(addr, 2, extract_value(val, 1));
			break;
		case T_STRING:
		case T_CSTRING:
			//builder->CreateStore(val, addr);
			store_element(addr, 0, extract_value(val, 0));
			store_element(addr, 1, extract_value(val, 1));
			store_element(addr, 2, extract_value(val, 2));
			store_element(addr, 3, extract_value(val, 3));
		/*	store_element(addr, 0, builder->CreateSelect(extract_value(val, 0), getInteger(TARGET_BITS, T_CSTRING), getInteger(TARGET_BITS, T_STRING)));
			store_element(addr, 1, extract_value(val, 1));
			store_element(addr, 2, extract_value(val, 2));
			store_element(addr, 3, extract_value(val, 3));
			break;
		case T_CSTRING:
			store_element(addr, 1, extract_value(val, 1));
			store_element(addr, 2, extract_value(val, 2));
			store_element(addr, 3, extract_value(val, 3));*/
			break;
		case T_FUNCTION:
			store_element(addr, 1, extract_value(val, 0));
			store_element(addr, 2, extract_value(val, 1));
			store_element(addr, 3, extract_value(val, 2));
			store_element(addr, 4, extract_value(val, 3));
			store_element(addr, 5, extract_value(val, 4));
			break;
		case T_VOID:
		case T_NULL:
			break;
		default:
			store_element(addr, 0, extract_value(val, 0));
			store_element(addr, 1, extract_value(val, 1));
			break;
	}
}

/*static llvm::Value* array_read(llvm::Value* addr, TYPE type){
	switch(type){
		case T_VOID:
		case T_CSTRING:
		case T_NULL:
		case T_CLASS:
		case T_FUNCTION:
			abort();
		case T_BOOLEAN: return builder->CreateTrunc(builder->CreateLoad(builder->CreateBitCast(addr, llvmType(getInt8PtrTy))), llvmType(getInt1Ty));
		case T_BYTE: return builder->CreateLoad(builder->CreateBitCast(addr, llvmType(getInt8PtrTy)));
		case T_SHORT: return builder->CreateLoad(builder->CreateBitCast(addr, llvmType(getInt16PtrTy)));
		case T_INTEGER: return builder->CreateLoad(builder->CreateBitCast(addr, llvmType(getInt32PtrTy)));
		case T_LONG: return builder->CreateLoad(builder->CreateBitCast(addr, llvmType(getInt64PtrTy)));
		case T_SINGLE: return builder->CreateLoad(builder->CreateBitCast(addr, llvmType(getFloatPtrTy)));
		case T_FLOAT: return builder->CreateLoad(builder->CreateBitCast(addr, llvmType(getDoublePtrTy)));
		case T_DATE: return builder->CreateLoad(builder->CreateBitCast(addr, pointer_t(date_type)));
		case T_STRING: {
			addr = builder->CreateBitCast(addr, charPP);
			llvm::Value* len_addr = builder->CreateGEP(addr, getInteger(TARGET_BITS, -4));
			len_addr = builder->CreateBitCast(len_addr, llvmType(getInt32PtrTy));
			return get_new_struct(string_type, getInteger(TARGET_BITS, T_STRING), addr, getInteger(32, 0), builder->CreateLoad(len_addr));
		}
		case T_POINTER: return builder->CreateLoad(builder->CreateBitCast(addr, charPP));
		case T_VARIANT: {
			//FIXME can we skip the T_VOID -> T_NULL test?
			return builder->CreateLoad(builder->CreateBitCast(addr, pointer_t(variant_type)));
		}
		default: {
			return get_new_struct(object_type, getInteger(TARGET_BITS, type), builder->CreateLoad(builder->CreateBitCast(addr, charPP)));
		}
	}
}*/

static void store_default(llvm::Value* addr, TYPE type){
	switch(type){
		case T_VOID: return;
		case T_BOOLEAN: builder->CreateStore(getInteger(1, false), addr); return;
		case T_BYTE: builder->CreateStore(getInteger(8, 0), addr); return;
		case T_SHORT: builder->CreateStore(getInteger(16, 0), addr); return;
		case T_INTEGER: builder->CreateStore(getInteger(32, 0), addr); return;
		case T_LONG: builder->CreateStore(getInteger(64, 0), addr); return;
		case T_SINGLE: builder->CreateStore(getFloat(0.0f), addr); return;
		case T_FLOAT: builder->CreateStore(getFloat(0.0), addr); return;
		case T_DATE: builder->CreateStore(get_new_struct(date_type, getInteger(32, 0), getInteger(32, 0)), addr); return;
		case T_STRING:
		case T_CSTRING: builder->CreateStore(get_new_struct(string_type, getInteger(TARGET_BITS, T_CSTRING), get_nullptr(), getInteger(32, 0), getInteger(32, 0)), addr); return;
		case T_CLASS: //FIXME?
		case T_POINTER: builder->CreateStore(get_nullptr(), addr); return;
		case T_VARIANT: builder->CreateStore(get_new_struct(variant_type, getInteger(TARGET_BITS, T_NULL)/*, undef int64*/), addr); return;
		case T_FUNCTION:
		case T_NULL: abort();
		default:
			builder->CreateStore(get_new_struct(object_type, get_global((void*)type, llvmType(getInt8Ty)), get_nullptr()), addr); return;
	}
}

static llvm::Value* get_default(TYPE type){
	switch(type){
		case T_VOID: return NULL;
		case T_BOOLEAN: return getInteger(1, false);
		case T_BYTE: return getInteger(8, 0);
		case T_SHORT: return getInteger(16, 0);
		case T_INTEGER: return getInteger(32, 0);
		case T_LONG: return getInteger(64, 0);
		case T_SINGLE: return getFloat(0.0f);
		case T_FLOAT: return getFloat(0.0);
		case T_DATE: return get_new_struct(date_type, getInteger(32, 0), getInteger(32, 0));
		case T_STRING:
		case T_CSTRING: return get_new_struct(string_type, getInteger(TARGET_BITS, T_CSTRING), get_nullptr(), getInteger(32, 0), getInteger(32, 0));
		case T_CLASS: return NULL;
		case T_POINTER: return get_nullptr();
		case T_VARIANT: return get_new_struct(variant_type, getInteger(TARGET_BITS, T_NULL));
		case T_FUNCTION:
		case T_NULL: abort();
		default:
			return get_new_struct(object_type, get_global((void*)type, llvmType(getInt8Ty)), get_nullptr());
	}
}

template <typename T>
static void gen_if(llvm::Value* cond, T func, const char* if_name="if.then", const char* cont_name="if.cont"){
	llvm::BasicBlock* then_block = create_bb(if_name);
	
	llvm::BasicBlock* from_block = builder->GetInsertBlock();
	
	builder->SetInsertPoint(then_block);
	func();
	llvm::BasicBlock* cont_block = create_bb(cont_name);
	builder->CreateBr(cont_block);
	
	builder->SetInsertPoint(from_block);
	builder->CreateCondBr(cond, then_block, cont_block);
	
	builder->SetInsertPoint(cont_block);
}

template <typename T>
static llvm::PHINode* gen_if_phi(llvm::Value* else_value, llvm::Value* cond, T func, const char* if_name="if.then", const char* cont_name="if.cont"){
	llvm::BasicBlock* then_block = create_bb(if_name);
	
	llvm::BasicBlock* from_block = builder->GetInsertBlock();
	
	builder->SetInsertPoint(then_block);
	llvm::Value* ret = func();
	llvm::BasicBlock* then_block2 = builder->GetInsertBlock();
	llvm::BasicBlock* cont_block = create_bb(cont_name);
	builder->CreateBr(cont_block);
	
	builder->SetInsertPoint(from_block);
	builder->CreateCondBr(cond, then_block, cont_block);
	
	builder->SetInsertPoint(cont_block);
	
	llvm::PHINode* phi = builder->CreatePHI(ret->getType(), 2);
	phi->addIncoming(ret, then_block2);
	phi->addIncoming(else_value, from_block);
	return phi;
}

template <typename T>
static void gen_if_noreturn(llvm::Value* cond, T func, const char* if_name="if.then", const char* cont_name="if.cont"){
	llvm::BasicBlock* then_block = create_bb(if_name);
	
	llvm::BasicBlock* from_block = builder->GetInsertBlock();
	
	builder->SetInsertPoint(then_block);
	func();
	
	builder->SetInsertPoint(from_block);
	llvm::BasicBlock* cont_block = create_bb(cont_name);
	builder->CreateCondBr(cond, then_block, cont_block);
	
	builder->SetInsertPoint(cont_block);
}

template <typename T1, typename T2>
static void gen_if_else(llvm::Value* cond, T1 then_func, T2 else_func, const char* if_name="if.then", const char* else_name="if.else", const char* cont_name="if.cont"){
	
	llvm::BasicBlock* then_block = create_bb(if_name);
	
	llvm::BasicBlock* from_block = builder->GetInsertBlock();
	
	builder->SetInsertPoint(then_block);
	then_func();
	llvm::BasicBlock* then_block2 = builder->GetInsertBlock();
	
	llvm::BasicBlock* else_block = create_bb(else_name);
	builder->SetInsertPoint(else_block);
	else_func();
	llvm::BasicBlock* else_block2 = builder->GetInsertBlock();
	
	llvm::BasicBlock* cont_block = create_bb(cont_name);
	builder->SetInsertPoint(from_block);
	builder->CreateCondBr(cond, then_block, else_block);
	builder->SetInsertPoint(then_block2);
	builder->CreateBr(cont_block);
	builder->SetInsertPoint(else_block2);
	builder->CreateBr(cont_block);
	builder->SetInsertPoint(cont_block);
}

template <typename T1, typename T2>
static llvm::PHINode* gen_if_else_phi(llvm::Value* cond, T1 then_func, T2 else_func, const char* if_name="if.then", const char* else_name="if.else", const char* cont_name="if.cont"){
	
	llvm::BasicBlock* then_block = create_bb(if_name);
	
	llvm::BasicBlock* from_block = builder->GetInsertBlock();
	
	builder->SetInsertPoint(then_block);
	llvm::Value* ret1 = then_func();
	llvm::BasicBlock* then_block2 = builder->GetInsertBlock();
	
	llvm::BasicBlock* else_block = create_bb(else_name);
	builder->SetInsertPoint(else_block);
	llvm::Value* ret2 = else_func();
	llvm::BasicBlock* else_block2 = builder->GetInsertBlock();
	
	llvm::BasicBlock* cont_block = create_bb(cont_name);
	builder->SetInsertPoint(from_block);
	builder->CreateCondBr(cond, then_block, else_block);
	builder->SetInsertPoint(then_block2);
	builder->CreateBr(cont_block);
	builder->SetInsertPoint(else_block2);
	builder->CreateBr(cont_block);
	builder->SetInsertPoint(cont_block);
	
	llvm::PHINode* phi = builder->CreatePHI(ret1->getType(), 2);
	phi->addIncoming(ret1, then_block2);
	phi->addIncoming(ret2, else_block2);
	return phi;
}

template <typename T1>
static void gen_while(llvm::Value* initial_condition, T1 body_func){
	llvm::BasicBlock* body_block = create_bb("while_block");
	
	llvm::BasicBlock* from_block = builder->GetInsertBlock();
	
	builder->SetInsertPoint(body_block);
	llvm::Value* cont = body_func();
	llvm::BasicBlock* cont_block = create_bb("while_cont");
	builder->CreateCondBr(cont, body_block, cont_block);
	
	builder->SetInsertPoint(from_block);
	builder->CreateCondBr(initial_condition, body_block, cont_block);
	
	builder->SetInsertPoint(cont_block);
}

template <typename T1>
static llvm::Value* gen_and_if(llvm::Value* left, T1 right_func){
	llvm::BasicBlock* right_block = create_bb("and_if");
	
	llvm::BasicBlock* from_block = builder->GetInsertBlock();
	
	builder->SetInsertPoint(right_block);
	llvm::Value* right = right_func();
	llvm::BasicBlock* cont_block = create_bb("and_if_cont");
	builder->CreateBr(cont_block);
	
	builder->SetInsertPoint(from_block);
	builder->CreateCondBr(left, right_block, cont_block);
	
	builder->SetInsertPoint(cont_block);
	llvm::PHINode* phi = builder->CreatePHI(llvmType(getInt1Ty), 2);
	phi->addIncoming(getInteger(1, false), from_block);
	phi->addIncoming(right, right_block);
	return phi;
}

static llvm::Value* get_cstring_from_addr(llvm::Value* addr){
	llvm::Value* str;
	llvm::BasicBlock* B1 = builder->GetInsertBlock();
	llvm::BasicBlock* B2;
	
	gen_if(builder->CreateICmpNE(addr, get_nullptr()), [&](){
		llvm::Value* len = builder->CreateCall(get_global_function(strlen, 'j', "p"), addr);
		if (TARGET_BITS == 64)
			len = builder->CreateTrunc(len, llvmType(getInt32Ty));
		str = get_new_struct(string_type, getInteger(TARGET_BITS, T_CSTRING), addr, getInteger(32, 0), len);
		B2 = builder->GetInsertBlock();
	}, "cstring_strlen", "cstring_null_or_done_strlen");
	
	
	llvm::PHINode* phi = builder->CreatePHI(string_type, 2);
	phi->addIncoming(get_default(T_CSTRING), B1);
	phi->addIncoming(str, B2);
	return phi;
}

static llvm::Value* to_target_int(llvm::Value* integer32){
	if (TARGET_BITS == 64)
		return builder->CreateZExt(integer32, llvmType(getInt64Ty));
	return integer32;
}

static void init_locals(){
	int n_local = FP->n_local;
	int n_ctrl = FP->n_ctrl;
	int n_param = FP->n_param;
	
	locals.resize(n_local + n_ctrl);
	int i;
	for(i=0; i<n_local; i++){
		locals[i] = builder->CreateAlloca(TYPE_llvm(ctype_to_type(&FP->local[i].type)));
		builder->CreateStore(get_default(ctype_to_type(&FP->local[i].type)), locals[i]);
		//builder->CreateStore(read_value(builder->CreateGEP(read_global(&BP), getInteger(TARGET_BITS, i*sizeof(VALUE))), ctype_to_type(&FP->local[i].type)), locals[i]);
	}
	/*for(i; i < n_local + n_ctrl; i++){
		TYPE type = get_ctrl_type(i);
		if (type != T_VOID){
			locals[i] = builder->CreateAlloca(TYPE_llvm(type));
			store_default(locals[i], type);
		}
	}*/
	ctrl_values.resize(n_ctrl);
	current_ctrl_types.resize(n_ctrl);
	for(; i < n_local + n_ctrl; i++){
		set_ctrl_type(T_VOID, i);
		locals[i] = NULL;
		current_ctrl_types[i - n_local] = builder->CreateAlloca(llvmType(getInt32Ty));
		builder->CreateStore(getInteger(32, 0), current_ctrl_types[i - n_local]);
		ctrl_values[i - n_local][1] = builder->CreateAlloca(string_type);
		ctrl_values[i - n_local][2] = builder->CreateAlloca(object_type);
		ctrl_values[i - n_local][3] = builder->CreateAlloca(variant_type);
	}
	
	params.resize(n_param);
	for(i=0; i<n_param; i++){
		params[i] = builder->CreateAlloca(TYPE_llvm(FP->param[i].type));
		builder->CreateStore(read_value(builder->CreateGEP(read_global(&PP), getInteger(TARGET_BITS, (i-n_param)*sizeof(VALUE))), FP->param[i].type), params[i]);
	}
}

static llvm::Value* release_ctrl(int i);

static void finish_gosub_returns(){
	llvm::BasicBlock* bb = builder->GetInsertBlock();
	for(size_t i=0, e=gosub_return_points.size(); i!=e; i++){
		builder->SetInsertPoint(gosub_return_points[i]);
		/*llvm::Value* index = builder->CreateSub(builder->CreateLoad(temp_num_gosubs_on_stack), getInteger(32, 1));
		builder->CreateStore(index, temp_num_gosubs_on_stack);
		
		llvm::Value* indices[2] = {getInteger(TARGET_BITS, 0), to_target_int(index)};
		llvm::Value* addr = builder->CreateGEP(temp_gosub_stack, indices);*/
		
		//Cleanup current ctrl values
		for(int i=0; i<FP->n_ctrl; i++){
			llvm::Value* old_type = release_ctrl(i + FP->n_local);
			builder->CreateStore(getInteger(32, 0), current_ctrl_types[i]);
			gen_if(builder->CreateICmpNE(old_type, getInteger(32, 0)), [&](){
				llvm::Value* stack_addr = builder->CreateGEP(read_global(&BP), getInteger(TARGET_BITS, (i + FP->n_local)*sizeof(VALUE)));
				store_value(stack_addr, NULL, T_VOID);
			}, "old_ctrl_needs_to_be_cleaned1");
		}
		//And return
		/*llvm::Value* addr = builder->CreateLoad(gosub_return_point);
		llvm::IndirectBrInst* indbrinst = builder->CreateIndirectBr(addr, gosub_continue_points.size());
		
		for(size_t j=0, e=gosub_continue_points.size(); j!=e; j++){
			indbrinst->addDestination(gosub_continue_points[j]);
		}*/
		llvm::Value* return_id = builder->CreateLoad(gosub_return_point);
		llvm::SwitchInst* switchinst = builder->CreateSwitch(return_id, gosub_continue_points[0], gosub_continue_points.size()-1);
		
		for(size_t j=1, e=gosub_continue_points.size(); j!=e; j++){
			switchinst->addCase(getInteger(16, j+1), gosub_continue_points[j]);
		}
	}
	gosub_return_points.clear();
	builder->SetInsertPoint(bb);
}

static void create_return(){
	llvm::BasicBlock* BB = create_bb("return");
	builder->CreateBr(BB);
	
	for(size_t i=0, e=return_points.size(); i!=e; i++){
		builder->SetInsertPoint(return_points[i]);
		builder->CreateBr(BB);
	}
	return_points.clear();
	
	builder->SetInsertPoint(BB);
	
	if (func_byref_mask){
		int nparam = FP->n_param;
		for(int i=0; i<nparam; i++) if (1ULL<<i & func_byref_mask){
			TYPE type = FP->param[i].type;
			if (type != T_STRING && !TYPE_is_object(type)){
				llvm::Value* v = builder->CreateLoad(params[i]);
				llvm::Value* stack_addr = builder->CreateGEP(read_global(&PP), getInteger(TARGET_BITS, (i-nparam)*sizeof(VALUE)));
				store_value(stack_addr, v, type);
			}
		}
	}
	
	//static int counter = 0;
	
	//builder->CreateCall2(get_global_function_vararg(printf, 'v', "p"), get_global((void*)"leave %d\n", llvmType(getInt8Ty)), getInteger(32, counter++));
	
	builder->CreateCall(get_global_function_jif(EXEC_leave_keep, 'v', ""));
	//builder->CreateCall(get_global_function_vararg(printf, 'v', "p"), get_global((void*)"leave done\n", llvmType(getInt8Ty)));
	builder->CreateRetVoid();
}
static llvm::Value* read_sp();
static void codegen_statements(){
	addr_blocks.resize(all_statements.back()->addr + 1);
	
	for(size_t i=0, e=all_statements.size(); i!=e; i++){
		if (all_statements[i]->address_taken){
			llvm::BasicBlock* block = create_bb("block");
			builder->CreateBr(block);
			builder->SetInsertPoint(block);
			addr_blocks[all_statements[i]->addr] = block;
		}
		statement_pc = FP->code + all_statements[i]->addr;
		/*builder->CreateCall2(get_global_function_vararg(printf, 'v', "p"),
			get_global((void*)"before: %p\n", llvmType(getInt8Ty)), read_sp());*/
		all_statements[i]->expr->codegen();
		/*builder->CreateCall2(get_global_function_vararg(printf, 'v', "p"),
			get_global((void*)"after: %p\n", llvmType(getInt8Ty)), read_sp());*/
	}
}

static void insert_pending_branches(){
	for(size_t i=0, e=pending_branches.size(); i!=e; i++){
		PendingBranch& p = pending_branches[i];
		builder->SetInsertPoint(p.insert_point);
		if (p.condition){
			builder->CreateCondBr(p.condition, addr_blocks[p.true_addr], addr_blocks[p.false_addr]);
		} else {
			builder->CreateBr(addr_blocks[p.true_addr]);
		}
	}
	pending_branches.clear();
	for(size_t i=0, e=jump_table_pending_branches.size(); i!=e; i++){
		JumpTablePendingBranch& p = jump_table_pending_branches[i];
		builder->SetInsertPoint(p.insert_point);
		llvm::SwitchInst* sw = builder->CreateSwitch(p.condition, addr_blocks[p.default_addr], p.destinations->size());
		for(size_t j=0, ee=p.destinations->size(); j!=ee; j++)
			sw->addCase(getInteger(32, j), addr_blocks[(*p.destinations)[j]]);
	}
	jump_table_pending_branches.clear();
}

static void store_pc(unsigned short* pc){
	builder->CreateStore(getInteger(TARGET_BITS, (int64_t)(intptr_t)pc), get_global((void*)&PC, LONG_TYPE));
}

static void create_throw(int code){
	if (FP->debug)
		store_pc(statement_pc);
	builder->CreateCall(get_global_function_jif_vararg(THROW, 'v', "i"), getInteger(32, code));
	builder->CreateUnreachable();
}
static void create_throw(int code, const char* a, const char* b){
	if (FP->debug)
		store_pc(statement_pc);
	builder->CreateCall3(get_global_function_jif_vararg(THROW, 'v', "i"), getInteger(32, code), get_global((void*)a, llvmType(getInt8Ty)), get_global((void*)b, llvmType(getInt8Ty)));
	builder->CreateUnreachable();
}
static void create_throw(llvm::Value* code){
	if (FP->debug)
		store_pc(statement_pc);
	builder->CreateCall(get_global_function_jif_vararg(THROW, 'v', "i"), code);
	builder->CreateUnreachable();
}

static llvm::Value* get_value_on_top_addr(){
	//llvm::Value* sp = builder->CreateBitCast(read_global(&SP), pointer_t(value_type));
	llvm::Value* sp = read_global(&SP, pointer_t(value_type));
	return builder->CreateGEP(sp, getInteger(TARGET_BITS, -1));
}

static llvm::Value* get_top(TYPE type){
	return read_value(get_value_on_top_addr(), type);
}

static llvm::Value* read_sp(){
	return read_global(&SP, pointer_t(value_type));
}

static void store_sp(llvm::Value* sp){
	llvm::Value* sp_addr = get_global(&SP, pointer_t(value_type));
	builder->CreateStore(sp, sp_addr);
}

static void c_SP(int diff){
	if (diff == 0) return;
	llvm::Value* sp_addr = get_global(&SP, pointer_t(value_type));
	llvm::Value* sp = builder->CreateLoad(sp_addr);
	sp = builder->CreateGEP(sp, getInteger(TARGET_BITS, diff));
	builder->CreateStore(sp, sp_addr);
}

static void push_value(llvm::Value* val, TYPE type){
	//c_SP(1);
	//store_value(get_value_on_top_addr(), val, type);
	llvm::Value* top = get_global(&SP, pointer_t(value_type));
	llvm::Value* addr = builder->CreateLoad(top);
	store_value(addr, val, type);
	builder->CreateStore(builder->CreateGEP(addr, getInteger(TARGET_BITS, 1)), top);
}

static void set_top_value(llvm::Value* val, TYPE type, bool store_type = true){
	store_value(get_value_on_top_addr(), val, type, store_type);
}

static llvm::Value* ret_top_stack(TYPE type, bool save){
	llvm::Value* val = read_value(get_value_on_top_addr(), type);
	if (!save)
		c_SP(-1);
	return val;
}

static void unref_string_no_nullcheck(llvm::Value* ptr){
	llvm::Value* str_ptr = builder->CreateBitCast(ptr, pointer_t(llvmType(getInt32Ty)));
	llvm::Value* ref_addr = builder->CreateGEP(str_ptr, getInteger(TARGET_BITS, -2));
	llvm::Value* ref = builder->CreateLoad(ref_addr);
	/*gen_if_noreturn(builder->CreateICmpSLE(ref, getInteger(32, 0)), [&](){
		builder->CreateCall(get_global_function(abort, 'v', ""));
		builder->CreateUnreachable();
	});
	gen_if_noreturn(builder->CreateICmpUGE(ref, getInteger(32, 10000)), [&](){
		builder->CreateCall(get_global_function(abort, 'v', ""));
		builder->CreateUnreachable();
	});*/
	ref = builder->CreateSub(ref, getInteger(32, 1));
	builder->CreateStore(ref, ref_addr);
	llvm::Value* slt = builder->CreateICmpSLT(ref, getInteger(32, 1));
	if (llvm::Instruction* inst = llvm::dyn_cast<llvm::Instruction>(slt)){
		llvm::Value* arr[1] = {getInteger(32, 1)};
		inst->setMetadata("unref_slt", llvm::MDNode::get(llvm_context, arr));
	}
	gen_if(slt, [&](){
		llvm::Value* free_func = get_global_function_jif(STRING_free_real, 'v', "p");
		builder->CreateCall(free_func, ptr);
	}, "release_str", "release_done");
}

static void unref_string(llvm::Value* ptr){
	gen_if(builder->CreateICmpNE(ptr, get_nullptr()), [&](){
		unref_string_no_nullcheck(ptr);
	}, "str_not_null", "unref_done");
}

static void unref_object_no_nullcheck(llvm::Value* ptr){
	/*static int counter = 0;
	builder->CreateCall3(get_global_function_vararg(printf, 'v', "p"),
		get_global((void*)"unref #: %d, %p\n", llvmType(getInt8Ty)), getInteger(32, counter++), ptr);*/
		
	llvm::Value* object_ptr = builder->CreateBitCast(ptr, pointer_t(OBJECT_type));
	llvm::Value* ref_addr = get_element_addr(object_ptr, 1);
	llvm::Value* ref = builder->CreateLoad(ref_addr);
	ref = builder->CreateSub(ref, getInteger(TARGET_BITS, 1));
	builder->CreateStore(ref, ref_addr);
	/*builder->CreateCall3(get_global_function_vararg(printf, 'v', "p"),
		get_global((void*)"%p %ld -\n", llvmType(getInt8Ty)), ptr, ref);*/
	llvm::Value* slt = builder->CreateICmpSLT(ref, getInteger(TARGET_BITS, 1));
	if (llvm::Instruction* inst = llvm::dyn_cast<llvm::Instruction>(slt)){
		llvm::Value* arr[1] = {getInteger(32, 1)};
		inst->setMetadata("unref_slt", llvm::MDNode::get(llvm_context, arr));
	}
	gen_if(slt, [&](){
		llvm::Value* free_func = get_global_function_jif(CLASS_free, 'v', "p");
		builder->CreateCall(free_func, ptr);
	}, "release_obj", "release_done");
}

static void unref_object(llvm::Value* ptr){
	gen_if(builder->CreateICmpNE(ptr, get_nullptr()), [&](){
		unref_object_no_nullcheck(ptr);
	}, "obj_not_null", "unref_done");
}

static void codegen_printf(const char* str1, int tal){
	/*builder->CreateCall2(get_global_function_vararg(printf, 'v', "p"),
		get_global((void*)str1, llvmType(getInt8Ty)), getInteger(32, tal));*/
}
static void codegen_printf(const char* str1){
	/*builder->CreateCall(get_global_function_vararg(printf, 'v', "p"),
		get_global((void*)str1, llvmType(getInt8Ty)));*/
}
static void codegen_printf(const char* str1, llvm::Value* intval){
	/*builder->CreateCall2(get_global_function_vararg(printf, 'v', "p"),
		get_global((void*)str1, llvmType(getInt8Ty)), intval);*/
}

static void borrow_string_no_nullcheck(llvm::Value* ptr){
	llvm::Value* str_ptr = builder->CreateBitCast(ptr, pointer_t(llvmType(getInt32Ty)));
	llvm::Value* ref_addr = builder->CreateGEP(str_ptr, getInteger(TARGET_BITS, -2));
	llvm::Value* ref = builder->CreateLoad(ref_addr);
	/*gen_if_noreturn(builder->CreateICmpSLE(ref, getInteger(32, 0)), [&](){
		builder->CreateCall(get_global_function(abort, 'v', ""));
		builder->CreateUnreachable();
	});
	gen_if_noreturn(builder->CreateICmpUGT(ref, getInteger(32, 10000)), [&](){
		builder->CreateCall(get_global_function(abort, 'v', ""));
		builder->CreateUnreachable();
	});
	
	builder->CreateCall3(get_global_function_vararg(printf, 'v', "p"),
		get_global((void*)"%p %d\n", llvmType(getInt8Ty)), ptr, ref);*/
		
	ref = builder->CreateAdd(ref, getInteger(32, 1));
	builder->CreateStore(ref, ref_addr);
}

static void borrow_string(llvm::Value* ptr){
	gen_if(builder->CreateICmpNE(ptr, get_nullptr()), [&](){
		borrow_string_no_nullcheck(ptr);
	}, "str_not_null", "string_borrow_done");
}

static void borrow_object_no_nullcheck(llvm::Value* ptr){
	llvm::Value* object_ptr = builder->CreateBitCast(ptr, pointer_t(OBJECT_type));
	llvm::Value* ref_addr = get_element_addr(object_ptr, 1);
	llvm::Value* ref = builder->CreateLoad(ref_addr);
	ref = builder->CreateAdd(ref, getInteger(TARGET_BITS, 1));
	builder->CreateStore(ref, ref_addr);
	/*builder->CreateCall3(get_global_function_vararg(printf, 'v', "p"),
		get_global((void*)"%p %ld +\n", llvmType(getInt8Ty)), ptr, ref);*/
}

static void borrow_object(llvm::Value* ptr){
	gen_if(builder->CreateICmpNE(ptr, get_nullptr()), [&](){
		borrow_object_no_nullcheck(ptr);
	}, "obj_not_null", "borrow_object_done");
}

static void borrow_variant(llvm::Value* val){
	builder->CreateCall2(
		get_global_function(JR_borrow_variant, 'v', "jl"),
		extract_value(val, 0),
		extract_value(val, 1));
}

static void release(llvm::Value* val, TYPE type){
	if (TYPE_is_object(type))
		unref_object(extract_value(val, 1));
	else if (type == T_STRING){
		gen_if(builder->CreateICmpEQ(extract_value(val, 0), getInteger(TARGET_BITS, T_STRING)), [&](){
			unref_string(extract_value(val, 1));
		}, "release_T_STRING", "str_release_done");
	} else if (TYPE_is_variant(type))
		builder->CreateCall2(
			get_global_function(JR_release_variant, 'v', "jl"),
			extract_value(val, 0),
			extract_value(val, 1));
}

static void borrow(llvm::Value* val, TYPE type){
	if (TYPE_is_object(type))
		borrow_object(extract_value(val, 1));
	else if (type == T_STRING){
		gen_if(builder->CreateICmpEQ(extract_value(val, 0), getInteger(TARGET_BITS, T_STRING)), [&](){
			borrow_string(extract_value(val, 1));
		}, "borrow_T_STRING", "str_borrow_done");
	} else if (TYPE_is_variant(type))
		borrow_variant(val);
}

static void borrow_top(TYPE type){
	borrow(get_top(type), type);
}

static void release_top(Expression* expr){
	if (TYPE_is_object(expr->type)){
		unref_object(extract_value(expr->codegen_get_value(), 1));
	} else if (expr->type == T_STRING){
		release(expr->codegen_get_value(), T_STRING);
	} else if (TYPE_is_variant(expr->type)){
		expr->codegen();
		llvm::Value* free_variant_func = get_global_function_jif(EXEC_release, 'v', "jp");
		builder->CreateCall2(free_variant_func, getInteger(TARGET_BITS, T_VARIANT), get_value_on_top_addr());
	} else
		expr->codegen();
	c_SP(-1);
}

static void release_val(Expression* expr){
	if (TYPE_is_object(expr->type)){
		unref_object(extract_value(expr->codegen_get_value(), 1));
	} else if (TYPE_is_string(expr->type)){
		release(expr->codegen_get_value(), T_STRING);
	} else if (TYPE_is_variant(expr->type)){
		expr->on_stack = true;
		expr->codegen();
		llvm::Value* free_variant_func = get_global_function_jif(EXEC_release, 'v', "jp");
		builder->CreateCall2(free_variant_func, getInteger(TARGET_BITS, T_VARIANT), get_value_on_top_addr());
		c_SP(-1);
	} else
		expr->codegen();
}

///Convert a String to a pointer that can be put in an array or a Variant.
///If 'val' is a guaranteed to be T_CSTRING, set type to T_CSTRING, otherwise set it to T_STRING
static llvm::Value* string_for_array_or_variant(llvm::Value* val, TYPE type){
	llvm::Value* len = extract_value(val, 3);
	return gen_if_phi(get_nullptr(), builder->CreateICmpNE(len, getInteger(32, 0)), [&](){
		llvm::Value* ptr = extract_value(val, 1);
		llvm::Value* offset = extract_value(val, 2);
		if (type == T_STRING){
			llvm::Value* is_t_string = builder->CreateICmpEQ(extract_value(val, 0), getInteger(TARGET_BITS, T_STRING));
			llvm::Value* test1 = gen_and_if(builder->CreateAnd(
			is_t_string,
			builder->CreateICmpEQ(offset, getInteger(32, 0))), [&](){
				llvm::Value* len_addr = builder->CreateGEP(ptr, getInteger(TARGET_BITS, -4));
				len_addr = builder->CreateBitCast(len_addr, llvmType(getInt32PtrTy));
				return builder->CreateICmpEQ(builder->CreateLoad(len_addr), len);
			});
			return (llvm::Value*)gen_if_phi(ptr, builder->CreateXor(test1, getInteger(1, 1)), [&](){
				llvm::Value* newstr = builder->CreateCall2(get_global_function_jif(STRING_new, 'p', "pi"), builder->CreateGEP(ptr, to_target_int(offset)), len);
				newstr = builder->CreateCall(get_global_function_jif(STRING_free_later, 'p', "p"), newstr);
				gen_if(is_t_string, [&](){
					unref_string_no_nullcheck(ptr);
				});
				borrow_string_no_nullcheck(newstr);
				return newstr;
			});
		} else {
			llvm::Value* newstr = builder->CreateCall2(get_global_function_jif(STRING_new, 'p', "pi"), builder->CreateGEP(ptr, to_target_int(offset)), len);
			newstr = builder->CreateCall(get_global_function_jif(STRING_free_later, 'p', "p"), newstr);
			borrow_string_no_nullcheck(newstr);
			return newstr;
		}
	});
}

///Transfers the ownership from val to the variable in memory
///The old data in memory is not released
static void variable_write(llvm::Value* addr, llvm::Value* val, TYPE type){
	if (type != T_BOOLEAN && type != T_STRING && type != T_CSTRING && !TYPE_is_object(type))
		addr = builder->CreateBitCast(addr, pointer_t(TYPE_llvm(type)));
	/*if (type == T_BOOLEAN)
		char* */
	switch(type){
		case T_VOID:
		case T_NULL:
		case T_CLASS:
		case T_FUNCTION:
			abort();
		case T_BOOLEAN: builder->CreateStore(builder->CreateSExt(val, llvmType(getInt8Ty)), addr); break;
		case T_BYTE:
		case T_SHORT:
		case T_INTEGER:
		case T_LONG:
		case T_SINGLE:
		case T_FLOAT:
		case T_DATE:
		case T_POINTER:
		case T_VARIANT:
			builder->CreateStore(val, addr); break;
		case T_STRING:
		case T_CSTRING: {
			addr = builder->CreateBitCast(addr, charPP);
			builder->CreateStore(string_for_array_or_variant(val, type), addr);
			break;
		}
		default: {
			builder->CreateStore(extract_value(val, 1), builder->CreateBitCast(addr, charPP));
			break;
		}
	}
}

///Borrows the data
static llvm::Value* array_read(llvm::Value* addr, TYPE type){
	if (type != T_BOOLEAN && type != T_STRING && type != T_CSTRING && !TYPE_is_object(type))
		addr = builder->CreateBitCast(addr, pointer_t(TYPE_llvm(type)));
	switch(type){
		case T_VOID:
		case T_CSTRING:
		case T_NULL:
		case T_CLASS:
		case T_FUNCTION:
			abort();
		case T_BOOLEAN: return builder->CreateTrunc(builder->CreateLoad(addr), llvmType(getInt1Ty));
		case T_BYTE:
		case T_SHORT:
		case T_INTEGER:
		case T_LONG:
		case T_SINGLE:
		case T_FLOAT:
		case T_DATE:
		case T_POINTER:
			return builder->CreateLoad(addr);
		case T_VARIANT: {
			llvm::Value* ret = builder->CreateLoad(addr);
			borrow_variant(ret);
			return ret;
		}
		case T_STRING: {
			llvm::Value* ptr = builder->CreateLoad(builder->CreateBitCast(addr, charPP));
			return gen_if_phi(get_new_struct(string_type, getInteger(TARGET_BITS, T_CSTRING), get_nullptr(), getInteger(32, 0), getInteger(32, 0)),
				builder->CreateICmpNE(ptr, get_nullptr()), [&](){
				
				llvm::Value* len_addr = builder->CreateGEP(ptr, getInteger(TARGET_BITS, -4));
				llvm::Value* len = builder->CreateLoad(builder->CreateBitCast(len_addr, llvmType(getInt32PtrTy)));
				
				borrow_string_no_nullcheck(ptr);
				
				return get_new_struct(string_type, getInteger(TARGET_BITS, T_STRING), ptr, getInteger(32, 0), len);
			});
		}
		default: {
			llvm::Value* ret = builder->CreateLoad(builder->CreateBitCast(addr, charPP));
			borrow_object(ret);
			return get_new_struct(object_type, builder->CreateIntToPtr(getInteger(TARGET_BITS, type), llvmType(getInt8PtrTy)), ret);
		}
	}
}

void DropExpression::codegen(){
	/*if (expr->on_stack){
		release_top(expr);
	} else {
		release_val(expr);
	}*/
	if (CallExpression* ce = dyn_cast<CallExpression>(expr)){
		if (ce->variant_call){
			ce->codegen_on_stack();
			llvm::Value* ret = get_value_on_top_addr();
			gen_if(builder->CreateICmpNE(load_element(ret, 0), getInteger(TARGET_BITS, 0)), [&](){
				release(ret_top_stack(T_VARIANT, true), T_VARIANT);
			});
			c_SP(-1);
			return;
		}
	}
	release(expr->codegen_get_value(), expr->type);
	if (expr->on_stack)
		c_SP(-1);
}

llvm::Value* PushCStringExpression::codegen_get_value(){
	llvm::Value* ret = get_new_struct(string_type, getInteger(TARGET_BITS, T_CSTRING),
		builder->CreateIntToPtr(getInteger(TARGET_BITS, (int64_t)(void*)addr), llvmType(getInt8PtrTy)),
		getInteger(32, start), getInteger(32, len));
	if (on_stack)
		push_value(ret, type);
	return ret;
}

void PushIntegerExpression::codegen(){
	if (on_stack) codegen_on_stack();
}
llvm::Value* PushIntegerExpression::codegen_get_value(){
	llvm::Value* val = getInteger(bits, i);
	if (on_stack) codegen_on_stack();
	return val;
}
void PushIntegerExpression::codegen_on_stack(){
	push_value(getInteger(bits, i), type);
}

llvm::Value* PushFloatExpression::codegen_get_value(){
	llvm::Value* ret = type == T_SINGLE ? getFloat((float)val) : getFloat(val);
	if (on_stack)
		push_value(ret, type);
	return ret;
}

llvm::Value* PushNullPointerExpression::codegen_get_value(){
	llvm::Value* ret = get_nullptr();
	if (on_stack)
		push_value(ret, type);
	return ret;
}

llvm::Value* PushVoidDateExpression::codegen_get_value(){
	llvm::Value* ret = get_new_struct(date_type, getInteger(32, 0), getInteger(32, 0));
	if (on_stack)
		push_value(ret, T_DATE);
	return ret;
}

llvm::Value* PushNullExpression::codegen_get_value(){
	if (on_stack)
		push_value(NULL, T_NULL);
	return NULL;
}

llvm::Value* PushVoidExpression::codegen_get_value(){
	if (on_stack)
		push_value(NULL, T_VOID);
	return NULL;
}

llvm::Value* PushLastExpression::codegen_get_value(){
	llvm::Value* obj = read_global((void*)&EVENT_Last);
	borrow_object(obj);
	
	llvm::Value* ret = get_new_struct(object_type,
		builder->CreateIntToPtr(getInteger(TARGET_BITS, 16), llvmType(getInt8PtrTy)), obj);
	
	if (on_stack)
		push_value(ret, T_OBJECT);
	return ret;
}

static llvm::Value* read_variable(TYPE type, char* addr){
	llvm::Value* ret;
	if (type == T_BOOLEAN)
		ret = builder->CreateTrunc(read_global(addr, llvmType(getInt8Ty)), llvmType(getInt1Ty));
	else if (type < T_STRING || type == T_POINTER)
		ret = read_global(addr, TYPE_llvm(type));
	else if (type == T_STRING){
		llvm::Value* ad = read_global(addr);
		ret = gen_if_phi(get_new_struct(string_type, getInteger(TARGET_BITS, T_CSTRING), get_nullptr(), getInteger(32, 0), getInteger(32, 0)), builder->CreateICmpNE(ad, get_nullptr()), [&](){
			borrow_string_no_nullcheck(ad);
			
			llvm::Value* len_addr = builder->CreateGEP(ad, getInteger(TARGET_BITS, -4));
			len_addr = builder->CreateBitCast(len_addr, llvmType(getInt32PtrTy));
			llvm::Value* len = builder->CreateLoad(len_addr);
			
			return get_new_struct(string_type, getInteger(TARGET_BITS, T_STRING), ad, getInteger(32, 0), len);
		});
	} else if (type == T_CSTRING){
		llvm::Value* ad = read_global(addr);
		ret = get_cstring_from_addr(ad);
	} else if (TYPE_is_object(type)){
		llvm::Value* obj = read_global(addr);
		ret = get_new_struct(object_type, builder->CreateIntToPtr(getInteger(TARGET_BITS, type), llvmType(getInt8PtrTy)), obj);
		borrow_object(obj);
	} else if (type == T_VARIANT){ //FIXME
		ret = read_global(addr, variant_type);
		ret = gen_if_else_phi(builder->CreateICmpEQ(extract_value(ret, 0), getInteger(TARGET_BITS, T_VOID)), [&](){
			return get_new_struct(variant_type, getInteger(TARGET_BITS, T_NULL)/*, extract_value(ret, 1)*/);
		}, [&](){
			borrow_variant(ret);
			return ret;
		}, "Variant_T_VOID", "Variant_not_T_VOID", "Variant_T_VOID_done");
	} else abort();
	return ret;
}

static llvm::Value* codegen_tc_array(CLASS* klass, llvm::Value* ref, int array_load_index, llvm::Value* start_addr, TYPE type){
	llvm::Value* obj = builder->CreateCall4(get_global_function_jif(CARRAY_create_static, 'p', "pppp"),
		get_global((void*)klass, llvmType(getInt8Ty)),
		ref,
		get_global(klass->load->array[array_load_index], llvmType(getInt8Ty)),
		start_addr);
	
	borrow_object_no_nullcheck(obj);
	
	return get_new_struct(object_type, builder->CreateIntToPtr(getInteger(TARGET_BITS, type), llvmType(getInt8PtrTy)), obj);
}

llvm::Value* ReadVariableExpression::codegen_get_value(){
	llvm::Value* ret;
	if (ctype->id == TC_ARRAY){
		ret = codegen_tc_array(CP, get_global((void*)CP, llvmType(getInt8Ty)), ctype->value, get_global((void*)addr, llvmType(getInt8Ty)), type);
	} else if (ctype->id == TC_STRUCT){
		ret = builder->CreateCall3(get_global_function_jif(CSTRUCT_create_static, 'p', "ppp"),
			get_global((void*)klass, llvmType(getInt8Ty)),
			builder->CreateIntToPtr(getInteger(TARGET_BITS, type), llvmType(getInt8PtrTy)),
			get_global((void*)addr, llvmType(getInt8Ty)));
		borrow_object_no_nullcheck(ret);
		ret = get_new_struct(object_type, builder->CreateIntToPtr(getInteger(TARGET_BITS, type), llvmType(getInt8PtrTy)), ret);
	} else {
		ret = read_variable(type, addr);
	}
	if (on_stack)
		push_value(ret, type);
	return ret;
}


//FIXME this is almost same as array_read, only difference is in variant
static llvm::Value* read_variable(TYPE type, llvm::Value* addr){
	llvm::Value* ret;
	if (type == T_BOOLEAN)
		ret = builder->CreateTrunc(builder->CreateLoad(builder->CreateBitCast(addr, llvmType(getInt8PtrTy))), llvmType(getInt1Ty));
	else if (type < T_STRING || type == T_POINTER)
		ret = builder->CreateLoad(builder->CreateBitCast(addr, pointer_t(TYPE_llvm(type))));
	else if (type == T_STRING){
		llvm::Value* ad = builder->CreateLoad(builder->CreateBitCast(addr, charPP));
		ret = gen_if_phi(get_new_struct(string_type, getInteger(TARGET_BITS, T_CSTRING), get_nullptr(), getInteger(32, 0), getInteger(32, 0)), builder->CreateICmpNE(ad, get_nullptr()), [&](){
			borrow_string_no_nullcheck(ad);
			
			llvm::Value* len_addr = builder->CreateGEP(ad, getInteger(TARGET_BITS, -4));
			len_addr = builder->CreateBitCast(len_addr, llvmType(getInt32PtrTy));
			llvm::Value* len = builder->CreateLoad(len_addr);
			
			return get_new_struct(string_type, getInteger(TARGET_BITS, T_STRING), ad, getInteger(32, 0), len);
		});
	} else if (type == T_CSTRING){
		llvm::Value* ad = builder->CreateLoad(builder->CreateBitCast(addr, charPP));
		ret = get_cstring_from_addr(ad);
	} else if (TYPE_is_object(type)){
		llvm::Value* nobj = builder->CreateLoad(builder->CreateBitCast(addr, charPP));
		ret = get_new_struct(object_type, builder->CreateIntToPtr(getInteger(TARGET_BITS, type), llvmType(getInt8PtrTy)), nobj);
		borrow_object(nobj);
	} else if (type == T_VARIANT){
		ret = builder->CreateLoad(builder->CreateBitCast(addr, pointer_t(variant_type)));
		ret = gen_if_else_phi(builder->CreateICmpEQ(extract_value(ret, 0), getInteger(TARGET_BITS, T_VOID)), [&](){
			return get_new_struct(variant_type, getInteger(TARGET_BITS, T_NULL)/*, extract_value(ret, 1)*/);
		}, [&](){
			borrow_variant(ret);
			return ret;
		}, "Variant_T_VOID", "Variant_not_T_VOID", "Variant_T_VOID_done");
	} else abort();
	return ret;
	
}

///Reading an instance variable
static llvm::Value* read_variable_offset(TYPE type, llvm::Value* obj, llvm::Value* offset){
	return read_variable(type, builder->CreateGEP(obj, offset));
}

static void release_variable(TYPE type, llvm::Value* addr){
	if (type == T_STRING){
		unref_string(builder->CreateLoad(builder->CreateBitCast(addr, charPP)));
	} else if (TYPE_is_object(type)){
		unref_object(builder->CreateLoad(builder->CreateBitCast(addr, charPP)));
	} else if (type == T_VARIANT){
		release(builder->CreateLoad(builder->CreateBitCast(addr, pointer_t(variant_type))), T_VARIANT);
		/*builder->CreateCall2(get_global_function(JR_release_variant, 'v', "jl"),
			builder->CreateLoad(builder->CreateBitCast(addr, pointer_t(LONG_TYPE))),
			builder->CreateLoad(builder->CreateGEP(builder->CreateBitCast(addr, llvmType(getInt64PtrTy)), getInteger(TARGET_BITS, 1)))
		);*/
	}
}

static void write_variable_offset(TYPE type, llvm::Value* obj, int offset, llvm::Value* val){
	variable_write(builder->CreateGEP(obj, getInteger(TARGET_BITS, offset)), val, type);
}

static void make_nullcheck(llvm::Value* addr){
	gen_if_noreturn(builder->CreateICmpEQ(addr, get_nullptr()), [&](){
		create_throw(E_NULL);
	}, "is_null", "not_null");
}

template <typename T>
static void make_nullcheck(llvm::Value* addr, T func_if_error){
	gen_if_noreturn(builder->CreateICmpEQ(addr, get_nullptr()), [&](){
		func_if_error();
		create_throw(E_NULL);
	}, "is_null", "not_null");
}

static void make_double_nullcheck(llvm::Value* value){
	//FIXME: I think it is better to maybe make sure that an object on the stack actually
	//is of type object (nullpointer or not), instead of T_NULL.
	
	llvm::Value* valtype = builder->CreatePtrToInt(extract_value(value, 0), LONG_TYPE);
	
	gen_if_noreturn(builder->CreateICmpEQ(valtype, getInteger(TARGET_BITS, T_NULL)), [&](){
		create_throw(E_NULL);
	});
	
	llvm::Value* object_ptr = extract_value(value, 1);
	make_nullcheck(object_ptr);
}

template <typename T>
static void make_double_nullcheck(llvm::Value* value, T func_if_error){
	//FIXME: I think it is better to maybe make sure that an object on the stack actually
	//is of type object (nullpointer or not), instead of T_NULL.
	
	llvm::Value* valtype = builder->CreatePtrToInt(extract_value(value, 0), LONG_TYPE);
	
	gen_if_noreturn(builder->CreateICmpEQ(valtype, getInteger(TARGET_BITS, T_NULL)), [&](){
		func_if_error();
		create_throw(E_NULL);
	});
	
	llvm::Value* object_ptr = extract_value(value, 1);
	make_nullcheck(object_ptr, func_if_error);
}

static llvm::Value* get_class_desc_entry(llvm::Value* object_ptr, int index){
	llvm::Value* class_ptr = load_element(builder->CreateBitCast(object_ptr, pointer_t(OBJECT_type)), 0);
	llvm::Value* vtable_ptr = builder->CreateGEP(class_ptr, getInteger(TARGET_BITS, offsetof(CLASS, table)));
	llvm::Value* vtable = builder->CreateLoad(builder->CreateBitCast(vtable_ptr, charPP));
	
	llvm::Value* class_desc_ptr = builder->CreateGEP(vtable, getInteger(TARGET_BITS, index*sizeof(CLASS_DESC_SYMBOL)+offsetof(CLASS_DESC_SYMBOL, desc)));
	llvm::Value* class_desc = builder->CreateLoad(builder->CreateBitCast(class_desc_ptr, charPP));
	return class_desc;
}

static void create_check(CLASS* klass, llvm::Value* effective_class, llvm::Value* object){
	if (klass->must_check){
		//Assume klass->must_check only needs to be checked here, and not in the generated code
		int offset_check = offsetof(CLASS, check)/sizeof(void*); //From CLASS structure in gbx_class.h
		llvm::Value* check_func = builder->CreateLoad(builder->CreateGEP(builder->CreateBitCast(effective_class, charPP), getInteger(TARGET_BITS, offset_check)));
		check_func = builder->CreateBitCast(check_func, pointer_t(get_function_type('i', "p")));
		gen_if_noreturn(builder->CreateICmpNE(builder->CreateCall(check_func, object), getInteger(32, 0)), [&](){
			
			//static int counter = 0;
			
			//builder->CreateCall3(get_global_function_vararg(printf, 'v', "ppi"),
			//	get_global((void*)"Illegal: %p %d\n", llvmType(getInt8Ty)), object, getInteger(32, counter++));
				
			create_throw(E_IOBJECT);
		}, "illegal_object", "legal_object");
	}
}

//Same body as PopStaticExpression ...
void PopStaticVariableExpression::codegen(){
	llvm::Value* value = val->codegen_get_value();
	llvm::Value* var_addr = get_global((void*)addr, llvmType(getInt8Ty));
	release_variable(type, var_addr);
	variable_write(var_addr, value, type);
	c_SP(-val->on_stack);
}

void PushUnknownExpression::codegen_on_stack(){
	obj->codegen_on_stack();
	store_pc(pc);
	builder->CreateCall(get_global_function_jif(EXEC_push_unknown, 'v', "h"), getInteger(16, 0));
}

llvm::Value* PushUnknownExpression::codegen_get_value(){
	codegen_on_stack();
	return ret_top_stack(T_VARIANT, true);
}

llvm::Value* PushPureObjectVariableExpression::codegen_get_value(){
	if (isa<PushSuperExpression>(obj)){
		int offset = desc()->variable.offset;
		llvm::Value* ret = read_variable_offset(type, current_op, getInteger(TARGET_BITS, offset));
		if (on_stack)
			push_value(ret, type);
		return ret;
	}
	llvm::Value* object = obj->codegen_get_value();
	make_double_nullcheck(object);
	
	llvm::Value* object_ptr = extract_value(object, 1);
	
	//We have a non-native object, so dispatch is not needed for effective class for checking
	create_check(klass(), extract_value(object, 0), object_ptr);
	
	llvm::Value* desc_variable = get_class_desc_entry(object_ptr, index);
	llvm::Value* offset = to_target_int(builder->CreateLoad(builder->CreateBitCast(builder->CreateGEP(desc_variable, getInteger(TARGET_BITS, offsetof(CLASS_DESC_VARIABLE, offset))), llvmType(getInt32PtrTy))));
	
	llvm::Value* ret = read_variable_offset(type, object_ptr, offset);
	
	unref_object_no_nullcheck(object_ptr);
	
	c_SP(-obj->on_stack+on_stack);
	if (on_stack)
		set_top_value(ret, type);
	return ret;
}

void PopPureObjectVariableExpression::codegen(){
	llvm::Value* value = val->codegen_get_value();
	llvm::Value* object = obj->codegen_get_value();
	make_double_nullcheck(object, [&](){
		release(value, val->type);
	});
	
	llvm::Value* object_ptr = extract_value(object, 1);
	
	//We have a non-native object, so dispatch is not needed for effective class for checking
	create_check((CLASS*)(void*)obj->type, extract_value(object, 0), object_ptr);
	
	llvm::Value* desc_variable = get_class_desc_entry(object_ptr, index);
	llvm::Value* offset = to_target_int(builder->CreateLoad(builder->CreateBitCast(builder->CreateGEP(desc_variable, getInteger(TARGET_BITS, offsetof(CLASS_DESC_VARIABLE, offset))), llvmType(getInt32PtrTy))));
	
	llvm::Value* addr = builder->CreateGEP(object_ptr, offset);
	
	release_variable(type, addr);
	variable_write(addr, value, type);
	
	unref_object_no_nullcheck(object_ptr);
	
	c_SP(-val->on_stack-obj->on_stack);
}

llvm::Value* PushPureObjectConstantExpression::codegen_get_value(){
	llvm::Value* object = obj->codegen_get_value();
	if (obj->on_stack)
		c_SP(-1);
	
	llvm::Value* object_ptr = extract_value(object, 1);
	
	make_nullcheck(object_ptr);
	
	llvm::Value* desc_const = get_class_desc_entry(object_ptr, index);
	llvm::Value* ret;
	
	if (TYPE_is_string(type)){
		llvm::Value* translated = builder->CreateTrunc(builder->CreateLoad(builder->CreateGEP(desc_const, getInteger(TARGET_BITS, 2*sizeof(void*)+8))), llvmType(getInt1Ty));
		llvm::Value* straddr = builder->CreateGEP(desc_const, getInteger(TARGET_BITS, offsetof(CLASS_DESC_CONSTANT, value)));
		straddr = builder->CreateLoad(builder->CreateBitCast(straddr, llvmType(getInt8PtrTy)));
		straddr = gen_if_phi(straddr, translated, [&](){
			return builder->CreateCall(get_global_function(GB.Translate, 'p', "p"), straddr);
		});
		llvm::Value* len = builder->CreateCall(get_global_function(strlen, 'j', "p"), straddr);
		if (TARGET_BITS == 64)
			len = builder->CreateTrunc(len, llvmType(getInt32Ty));
		ret = get_new_struct(string_type, getInteger(TARGET_BITS, T_CSTRING), straddr, getInteger(32, 0), len);
	} else {
		llvm::Type* t = TYPE_llvm(type);
		llvm::Value* addr = builder->CreateGEP(desc_const, getInteger(TARGET_BITS, offsetof(CLASS_DESC_CONSTANT, value)));
		ret = builder->CreateLoad(builder->CreateBitCast(addr, pointer_t(t)));
	}
	
	unref_object_no_nullcheck(object_ptr);
	
	if (on_stack)
		push_value(ret, type);
	return ret;
}

llvm::Value* PushPureObjectPropertyExpression::codegen_private(bool get_value){
	obj->codegen_on_stack();
	
	llvm::Value* object = ret_top_stack(obj->type, true);
	
	bool super = isa<PushSuperExpression>(obj);
	
	if (!super)
		make_double_nullcheck(object);
	
	llvm::Value* object_ptr = extract_value(object, 1);
	
	llvm::Value* class_desc_property;
	llvm::Value* is_native;
	
	if (super){
		is_native = getInteger(1, klass()->is_native);
	} else {
		class_desc_property = get_class_desc_entry(object_ptr, index);
		if (!klass()->table[index].desc->property.native)
			is_native = getInteger(1, false); //Assume no children can be native either
		else
			is_native = builder->CreateTrunc(builder->CreateLoad(builder->CreateGEP(class_desc_property, getInteger(TARGET_BITS, 4*TARGET_BITS/8))), llvmType(getInt1Ty));
	}
	
	//FIXME dispatch:
	create_check(klass(), extract_value(object, 0), object_ptr);
	
	llvm::Value* ret = gen_if_else_phi(is_native, [&](){
		llvm::Value* read = super ? get_global((void*)this->desc()->property.read, llvmType(getInt8Ty)) : builder->CreateLoad(builder->CreateBitCast(builder->CreateGEP(class_desc_property, getInteger(TARGET_BITS, offsetof(CLASS_DESC_PROPERTY, read))), charPP));
		
		llvm::Value* got_error = builder->CreateCall4(get_global_function_jif(EXEC_call_native, 'c', "ppjp"),
			read,
			object_ptr,
			getInteger(TARGET_BITS, type),
			get_nullptr());
		
		gen_if_noreturn(builder->CreateICmpNE(got_error, getInteger(8, false)), [&](){
			builder->CreateCall(get_global_function_jif(ERROR_propagate, 'v', ""));
			builder->CreateUnreachable();
		});
		
		llvm::Value* ret = read_value(get_global(&TEMP, value_type), type);
		borrow(ret, type);
		return ret;
	}, [&](){
		llvm::Value* read = super ? (llvm::Value*)getInteger(32, (int)(intptr_t)this->desc()->property.read) : (llvm::Value*)builder->CreateLoad(builder->CreateBitCast(builder->CreateGEP(class_desc_property, getInteger(TARGET_BITS, offsetof(CLASS_DESC_PROPERTY, read))), llvmType(getInt32PtrTy)));
		llvm::Value* klass = super ? builder->CreateIntToPtr(getInteger(TARGET_BITS, obj->type), llvmType(getInt8PtrTy)) : builder->CreateLoad(builder->CreateBitCast(builder->CreateGEP(class_desc_property, getInteger(TARGET_BITS, offsetof(CLASS_DESC_PROPERTY, klass))), charPP));
		
		builder->CreateStore(klass, get_global((void*)&EXEC.klass));
		builder->CreateStore(object_ptr, get_global((void*)&EXEC.object));
		builder->CreateStore(getInteger(32, 0), get_global((void*)&EXEC.nparam, llvmType(getInt32Ty)));
		builder->CreateStore(read, get_global((void*)&EXEC.index, llvmType(getInt32Ty)));
		
		/*builder->CreateCall2(get_global_function_vararg(printf, 'v', "pi"),
			get_global((void*)"%d\n", llvmType(getInt8Ty)), getInteger(32, 1));*/
		
		builder->CreateCall(get_global_function_jif(EXEC_function_real, 'v', ""));
		
		llvm::Value* ret = read_value(get_global(RP, value_type), type);
		builder->CreateStore(getInteger(TARGET_BITS, T_VOID), get_global(RP, LONG_TYPE));
		return ret;
	}, "property_native", "property_non_native", "property_read_done");
	
	unref_object_no_nullcheck(object_ptr);
	
	if (get_value && !on_stack){
		c_SP(-1);
		return ret;
	} else {
		store_value(get_value_on_top_addr(), ret, type);
		return ret;
	}
}

void PopPureObjectPropertyExpression::codegen(){
	llvm::Value* v = val->codegen_get_value();
	obj->codegen_on_stack();
	
	llvm::Value* object = ret_top_stack(obj->type, true);
	
	bool super = isa<PushSuperExpression>(obj);
	
	if (!super)
		make_double_nullcheck(object);
	
	llvm::Value* object_ptr = extract_value(object, 1);
	
	llvm::Value* class_desc_property;
	llvm::Value* is_native;
	
	if (super){
		is_native = getInteger(1, ((CLASS*)(void*)obj->type)->is_native);
	} else {
		CLASS* klass = (CLASS*)(void*)obj->type;
		class_desc_property = get_class_desc_entry(object_ptr, index);
		if (!klass->table[index].desc->property.native)
			is_native = getInteger(1, false); //Assume no children can be native either
		else
			is_native = builder->CreateTrunc(builder->CreateLoad(builder->CreateGEP(class_desc_property, getInteger(TARGET_BITS, 4*TARGET_BITS/8))), llvmType(getInt1Ty));
	}
	
	//FIXME dispatch
	create_check((CLASS*)(void*)obj->type, extract_value(object, 0), object_ptr);
	
	gen_if_else(is_native, [&](){
		llvm::Value* write = super ? get_global((void*)this->desc()->property.write, llvmType(getInt8Ty)) : builder->CreateLoad(builder->CreateBitCast(builder->CreateGEP(class_desc_property, getInteger(TARGET_BITS, offsetof(CLASS_DESC_PROPERTY, write))), charPP));
		
		llvm::Value* val_ptr = builder->CreateBitCast(builder->CreateGEP(read_sp(), getInteger(TARGET_BITS, -2)), llvmType(getInt8PtrTy));
		
		llvm::Value* got_error = builder->CreateCall4(get_global_function_jif(EXEC_call_native, 'c', "ppjp"),
			write,
			object_ptr,
			getInteger(TARGET_BITS, 0),
			val_ptr);
		
		gen_if_noreturn(builder->CreateICmpNE(got_error, getInteger(8, false)), [&](){
			builder->CreateCall(get_global_function_jif(ERROR_propagate, 'v', ""));
			builder->CreateUnreachable();
		});
		release(v, val->type);
	}, [&](){
		push_value(v, val->type);
		llvm::Value* val_ptr = builder->CreateBitCast(builder->CreateGEP(read_sp(), getInteger(TARGET_BITS, -3)), pointer_t(LONG_TYPE));
		builder->CreateStore(getInteger(TARGET_BITS, T_VOID), val_ptr);
		
		llvm::Value* write = super ? (llvm::Value*)getInteger(32, (int)(intptr_t)this->desc()->property.write) : builder->CreateLoad(builder->CreateBitCast(builder->CreateGEP(class_desc_property, getInteger(TARGET_BITS, offsetof(CLASS_DESC_PROPERTY, write))), llvmType(getInt32PtrTy)));
		llvm::Value* klass = super ? builder->CreateIntToPtr(getInteger(TARGET_BITS, obj->type), llvmType(getInt8PtrTy)) : builder->CreateLoad(builder->CreateBitCast(builder->CreateGEP(class_desc_property, getInteger(TARGET_BITS, offsetof(CLASS_DESC_PROPERTY, klass))), charPP));
		
		builder->CreateStore(klass, get_global((void*)&EXEC.klass));
		builder->CreateStore(object_ptr, get_global((void*)&EXEC.object));
		builder->CreateStore(getInteger(32, 1), get_global((void*)&EXEC.nparam, llvmType(getInt32Ty)));
		builder->CreateStore(write, get_global((void*)&EXEC.index, llvmType(getInt32Ty)));
		
		/*builder->CreateCall5(get_global_function_vararg(printf, 'v', "p"),
			get_global((void*)"%d %p %p %d\n", llvmType(getInt8Ty)), getInteger(32, 2), klass, class_desc_property, getInteger(32, index));*/
		
		builder->CreateCall(get_global_function_jif(EXEC_function_real, 'v', ""));
		//I think the release of RP is not needed, it should already be T_VOID...
	}, "property_native", "property_non_native", "property_write_done");
	
	unref_object_no_nullcheck(object_ptr);
	c_SP(-2);
}

void PopVirtualPropertyExpression::codegen(){
	llvm::Value* v = val->codegen_get_value();
	llvm::Value* object = obj->codegen_get_value();
	llvm::Value* object_ptr = extract_value(object, 1);
	
	if (is_static){
		//Only allowed on virtual classes
		//Assume it returned a T_CLASS, or threw an error...
		
		gen_if_noreturn(builder->CreateICmpNE(load_element(get_value_on_top_addr(), 0), getInteger(TARGET_BITS, T_CLASS)), [&](){
			create_throw(E_STATIC, ((CLASS*)(void*)obj->type)->name, this->name);
		});
		
		object_ptr = get_nullptr();
	} else
		create_check((CLASS*)(void*)obj->type, get_global((void*)obj->type, llvmType(getInt8Ty)), object_ptr);
	
	llvm::Value* write = get_global((void*)desc()->property.write, llvmType(getInt8Ty));
	llvm::Value* val_ptr = builder->CreateBitCast(builder->CreateGEP(read_sp(), getInteger(TARGET_BITS, -2)), llvmType(getInt8PtrTy));
	
	llvm::Value* got_error = builder->CreateCall4(get_global_function_jif(EXEC_call_native, 'c', "ppjp"),
		write,
		object_ptr,
		getInteger(TARGET_BITS, 0),
		val_ptr);
	
	gen_if_noreturn(builder->CreateICmpNE(got_error, getInteger(8, false)), [&](){
		builder->CreateCall(get_global_function_jif(ERROR_propagate, 'v', ""));
		builder->CreateUnreachable();
	});
	
	release(v, val->type);
	if (!is_static)
		unref_object_no_nullcheck(object_ptr);
	c_SP(-2);
}

llvm::Value* PushVirtualPropertyExpression::codegen_private(bool get_value){
	obj->codegen_on_stack();
	
	llvm::Value* object = ret_top_stack(obj->type, true);
	
	//Might be T_NULL or T_CLASS or the virtual class:
	llvm::Value* valtype = builder->CreatePtrToInt(extract_value(object, 0), LONG_TYPE);
	
	gen_if_noreturn(builder->CreateICmpEQ(valtype, getInteger(TARGET_BITS, T_NULL)), [&](){
		create_throw(E_NULL);
	});
	
	//T_CLASS could be resolved at compile time, since only gb.db returns a (virtual) object
	//when invoking a static read property/static method...
	
	llvm::Value* object_ptr = builder->CreateSelect(
		builder->CreateICmpEQ(valtype, getInteger(TARGET_BITS, T_CLASS)),
		get_nullptr(),
		extract_value(object, 1));
	
	llvm::Value* got_error = builder->CreateCall4(get_global_function_jif(EXEC_call_native, 'c', "ppjp"),
		get_global((void*)desc()->property.read, llvmType(getInt8Ty)), //Assume no override by inheritance
		object_ptr,
		getInteger(TARGET_BITS, type),
		get_nullptr());
	
	gen_if_noreturn(builder->CreateICmpNE(got_error, getInteger(8, false)), [&](){
		builder->CreateCall(get_global_function_jif(ERROR_propagate, 'v', ""));
		builder->CreateUnreachable();
	});
	
	llvm::Value* ret = read_value(get_global(&TEMP, value_type), type);
	borrow(ret, type);
	
	unref_object(object_ptr);
	
	if (get_value && !on_stack){
		c_SP(-1);
		return ret;
	} else {
		store_value(get_value_on_top_addr(), ret, type);
		return ret;
	}
}

llvm::Value* PushPureObjectStaticPropertyExpression::codegen_private(bool get_value){
	obj->codegen_on_stack();
	
	//Assume it returned a T_CLASS, or threw an error...
	
	gen_if_noreturn(builder->CreateICmpNE(load_element(get_value_on_top_addr(), 0), getInteger(TARGET_BITS, T_CLASS)), [&](){
		create_throw(E_STATIC, this->klass()->name, this->name);
	});
	
	llvm::Value* got_error = builder->CreateCall4(get_global_function_jif(EXEC_call_native, 'c', "ppjp"),
		get_global((void*)desc()->property.read, llvmType(getInt8Ty)), //Assume no override by inheritance
		get_nullptr(),
		getInteger(TARGET_BITS, type),
		get_nullptr());
	
	gen_if_noreturn(builder->CreateICmpNE(got_error, getInteger(8, false)), [&](){
		builder->CreateCall(get_global_function_jif(ERROR_propagate, 'v', ""));
		builder->CreateUnreachable();
	});
	
	llvm::Value* ret = read_value(get_global(&TEMP, value_type), type);
	borrow(ret, type);
	
	if (get_value && !on_stack){
		c_SP(-1);
		return ret;
	} else {
		store_value(get_value_on_top_addr(), ret, type);
		return ret;
	}
}

llvm::Value* PushStaticPropertyExpression::codegen_get_value(){
	CLASS* c = ((PushClassExpression*)obj)->klass;
	CLASS_DESC* desc = c->table[index].desc;
	
	llvm::Value* ret;
	
	if (desc->property.native){
		llvm::Value* got_error = builder->CreateCall4(get_global_function_jif(EXEC_call_native, 'c', "ppjp"),
			get_global((void*)desc->property.read, llvmType(getInt8Ty)),
			get_nullptr(),
			getInteger(TARGET_BITS, type),
			get_nullptr());
		
		gen_if_noreturn(builder->CreateICmpNE(got_error, getInteger(8, false)), [&](){
			builder->CreateCall(get_global_function_jif(ERROR_propagate, 'v', ""));
			builder->CreateUnreachable();
		});
		
		ret = read_value(get_global(&TEMP, value_type), type);
		borrow(ret, type);
	} else {
		builder->CreateStore(get_global((void*)desc->property.klass, llvmType(getInt8Ty)), get_global((void*)&EXEC.klass));
		builder->CreateStore(get_nullptr(), get_global((void*)&EXEC.object));
		builder->CreateStore(getInteger(32, 0), get_global((void*)&EXEC.nparam, llvmType(getInt32Ty)));
		builder->CreateStore(getInteger(32, (int)(intptr_t)desc->property.read), get_global((void*)&EXEC.index, llvmType(getInt32Ty)));
		
		/*builder->CreateCall2(get_global_function_vararg(printf, 'v', "pi"),
			get_global((void*)"%d\n", llvmType(getInt8Ty)), getInteger(32, 3));*/
		
		builder->CreateCall(get_global_function_jif(EXEC_function_real, 'v', ""));
		
		ret = read_value(get_global(RP, value_type), type);
		builder->CreateStore(getInteger(TARGET_BITS, T_VOID), get_global(RP, LONG_TYPE));
	}
	
	if (on_stack)
		push_value(ret, type);
	return ret;
}

void PopStaticPropertyExpression::codegen(){
	llvm::Value* value = val->codegen_get_value();
	
	CLASS_DESC* desc = klass->table[index].desc;
	
	if (desc->property.native){
		llvm::Value* got_error = builder->CreateCall4(get_global_function_jif(EXEC_call_native, 'c', "ppjp"),
			get_global((void*)desc->property.write, llvmType(getInt8Ty)),
			get_nullptr(),
			getInteger(TARGET_BITS, type),
			builder->CreateBitCast(get_value_on_top_addr(), llvmType(getInt8PtrTy)));
		
		gen_if_noreturn(builder->CreateICmpNE(got_error, getInteger(8, false)), [&](){
			builder->CreateCall(get_global_function_jif(ERROR_propagate, 'v', ""));
			builder->CreateUnreachable();
		});
		
		release(value, type);
		c_SP(-val->on_stack);
	} else {
		builder->CreateStore(get_global((void*)desc->property.klass, llvmType(getInt8Ty)), get_global((void*)&EXEC.klass));
		builder->CreateStore(get_nullptr(), get_global((void*)&EXEC.object));
		builder->CreateStore(getInteger(32, 1), get_global((void*)&EXEC.nparam, llvmType(getInt32Ty)));
		builder->CreateStore(getInteger(32, (int)(intptr_t)desc->property.write), get_global((void*)&EXEC.index, llvmType(getInt32Ty)));
		
		/*builder->CreateCall2(get_global_function_vararg(printf, 'v', "pi"),
			get_global((void*)"%d\n", llvmType(getInt8Ty)), getInteger(32, 4));*/
		
		builder->CreateCall(get_global_function_jif(EXEC_function_real, 'v', ""));
	}
}

void PopUnknownPropertyUnknownExpression::codegen(){
	val->codegen_on_stack();
	
	llvm::Value* effective_class;
	llvm::Value* object;
	
	if (PushClassExpression* pce = dyn_cast<PushClassExpression>(obj)){
		effective_class = builder->CreateIntToPtr(getInteger(TARGET_BITS, (int64_t)(intptr_t)(void*)pce->klass), llvmType(getInt8PtrTy));
		object = get_nullptr();
		push_value(NULL, T_VOID);
	} else {
		llvm::Value* ob = obj->codegen_get_value();
		object = extract_value(ob, 1);
		CLASS* k = (CLASS*)(void*)obj->type;
		
		if (isa<PushSuperExpression>(obj)){
			effective_class = builder->CreateIntToPtr(getInteger(TARGET_BITS, (int64_t)(intptr_t)(void*)k), llvmType(getInt8PtrTy));
		} else if (!k->is_virtual){
			make_nullcheck(object);
			effective_class = load_element(builder->CreateBitCast(object, pointer_t(OBJECT_type)), 0);
		} else {
			effective_class = builder->CreateIntToPtr(getInteger(TARGET_BITS, (int64_t)(intptr_t)(void*)k), llvmType(getInt8PtrTy));
		}
		
		create_check(k, effective_class, object);
	}
	
	builder->CreateCall3(get_global_function(JR_pop_unknown_property_unknown, 'v', "ppj"),
		effective_class, object, getInteger(TARGET_BITS, (uint64_t)(void*)name));
}

void PopUnknownExpression::codegen(){
	val->codegen_on_stack();
	obj->codegen_on_stack();
	store_pc(pc);
	builder->CreateCall(get_global_function_jif(EXEC_pop_unknown, 'v', ""));
}

llvm::Value* PushDynamicExpression::codegen_get_value(){
	llvm::Value* ret;
	if (ctype->id == TC_ARRAY){
		/*llvm::Value* obj = builder->CreateCall4(get_global_function_jif(CARRAY_create_static, 'p', "pppp"),
			get_global((void*)CP, llvmType(getInt8Ty)),
			current_op,
			get_global(CP->load->array[ctype->value], llvmType(getInt8Ty)),
			builder->CreateGEP(current_op, getInteger(TARGET_BITS, offset)));
		
		borrow_object_no_nullcheck(obj);
		
		ret = get_new_struct(object_type, builder->CreateIntToPtr(getInteger(TARGET_BITS, type), llvmType(getInt8PtrTy)), obj);*/
		ret = codegen_tc_array(CP, current_op, ctype->value, builder->CreateGEP(current_op, getInteger(TARGET_BITS, offset)), type);
	} else if (ctype->id == TC_STRUCT){
		ret = builder->CreateCall3(get_global_function_jif(CSTRUCT_create_static, 'p', "ppp"),
			get_global((void*)CP, llvmType(getInt8Ty)),
			builder->CreateIntToPtr(getInteger(TARGET_BITS, type), llvmType(getInt8PtrTy)),
			builder->CreateGEP(current_op, getInteger(TARGET_BITS, offset)));
		borrow_object_no_nullcheck(ret);
		return get_new_struct(object_type, builder->CreateIntToPtr(getInteger(TARGET_BITS, type), llvmType(getInt8PtrTy)), ret);
	} else {
		ret = read_variable_offset(type, current_op, getInteger(TARGET_BITS, offset));
	}
	if (on_stack)
		push_value(ret, type);
	return ret;
}

void PopDynamicExpression::codegen(){
	llvm::Value* new_val = val->codegen_get_value();
	c_SP(-val->on_stack);
	release_variable(type, builder->CreateGEP(current_op, getInteger(TARGET_BITS, offset)));
	write_variable_offset(type, current_op, offset, new_val);
}

//Same body as PopStaticVariableExpression ...
void PopStaticExpression::codegen(){
	llvm::Value* new_val = val->codegen_get_value();
	llvm::Value* ad = get_global((void*)addr, llvmType(getInt8Ty));
	release_variable(type, ad);
	variable_write(ad, new_val, type);
	c_SP(-val->on_stack);
}

llvm::Value* PushFunctionExpression::codegen_get_value(){
	llvm::Value* func = insert_value(get_new_struct(function_type), current_op, 1);
	borrow_object(current_op);
	push_value(func, T_FUNCTION);
	return func;
}
void PushFunctionExpression::codegen_on_stack(){
	//llvm::Value* func = get_new_struct(function_type, read_global((void*)&CP), current_op, getInteger(8, FUNCTION_PRIVATE), getInteger(8, true), getInteger(16, index));
	llvm::Value* func = insert_value(get_new_struct(function_type), current_op, 1);
	borrow_object(current_op);
	push_value(func, T_FUNCTION);
}

void PushClassExpression::codegen_on_stack(){
	llvm::Value* sp = read_global((void*)&SP, pointer_t(value_types[T_CLASS]));
	store_element(sp, 0, getInteger(TARGET_BITS, T_CLASS));
	store_element(sp, 1, get_global((void*)klass, llvmType(getInt8Ty)));
	c_SP(1);
}

llvm::Value* AddQuickExpression::codegen_get_value(){
	if (type == T_VARIANT){
		expr->codegen_on_stack();
		builder->CreateCall(get_global_function(JR_aq_variant, 'v', "i"), getInteger(32, add));
		return ret_top_stack(T_VARIANT, on_stack);
	}
	llvm::Value* orig = expr->codegen_get_value();
	if (expr->on_stack) c_SP(-1);
	
	llvm::Value* new_val;
	switch(type){
		case T_BYTE: new_val = builder->CreateAdd(orig, getInteger(8, add)); break;
		case T_SHORT: new_val = builder->CreateAdd(orig, getInteger(16, add)); break;
		case T_INTEGER: new_val = builder->CreateAdd(orig, getInteger(32, add)); break;
		case T_LONG: new_val = builder->CreateAdd(orig, getInteger(64, add)); break;
		case T_SINGLE: new_val = builder->CreateFAdd(orig, getFloat((float)add)); break;
		case T_FLOAT: new_val = builder->CreateFAdd(orig, getFloat((double)add)); break;
		case T_POINTER: new_val = builder->CreateGEP(orig, getInteger(TARGET_BITS, add)); break;
		default: __builtin_unreachable();
	}
	if (on_stack)
		push_value(new_val, type);
	return new_val;
}

llvm::Value* PushMeExpression::codegen_get_value(){
	//llvm::Value* ret = get_new_struct(object_type, read_global((void*)CP), current_op);
	llvm::Value* ret = get_new_struct(object_type, get_global((void*)CP, llvmType(getInt8Ty)), current_op);
	borrow_object_no_nullcheck(current_op);
	if (on_stack)
		push_value(ret, type);
	return ret;
}

llvm::Value* PushSuperExpression::codegen_get_value(){
	llvm::Value* ret = get_new_struct(object_type, get_global((void*)type, llvmType(getInt8Ty)), current_op);
	borrow_object_no_nullcheck(current_op);
	if (on_stack)
		push_value(ret, type);
	return ret;
}

llvm::Value* PushLocalExpression::codegen_get_value(){
	llvm::Value* ret = builder->CreateLoad(locals[index]);
	if (on_stack)
		push_value(ret, type);
	if (no_ref_variant == false)
		borrow(ret, type);
	return ret;
}
void PushLocalExpression::codegen_on_stack(){
	llvm::Value* ret = builder->CreateLoad(locals[index]);
	push_value(ret, type);
	
	borrow(ret, type); //Is this better optimized?
	//borrow_top(type);
}

llvm::Value* PushParamExpression::codegen_get_value(){
	llvm::Value* ret = builder->CreateLoad(params[index + FP->n_param]);
	if (on_stack)
		push_value(ret, type);
	borrow(ret, type);
	return ret;
}
void PushParamExpression::codegen_on_stack(){
	llvm::Value* ret = builder->CreateLoad(params[index + FP->n_param]);
	push_value(ret, type);
	borrow(ret, type);
	//borrow_top(type);
}

void PopLocalExpression::codegen(){
	llvm::Value* v = val->codegen_get_value();
	if (val->on_stack) c_SP(-1);
	llvm::Value* old_val = builder->CreateLoad(locals[index]);
	release(old_val, type);
	builder->CreateStore(v, locals[index]);
	if (type == T_STRING || (type == T_VARIANT && !val->no_ref_variant) || TYPE_is_object(type)){
		if (TYPE_is_object(type)){
			//codegen_printf("%p\n", extract_value(v, 0));
		}
		llvm::Value* stack_addr = builder->CreateGEP(read_global(&BP), getInteger(TARGET_BITS, index*sizeof(VALUE)));
		store_value(stack_addr, v, type, false);
	} else if (type == T_VARIANT && val->no_ref_variant){
		llvm::Value* vtype = extract_value(old_val, 0);
		llvm::Value* is_str = builder->CreateICmpEQ(vtype, getInteger(TARGET_BITS, T_STRING));
		llvm::Value* is_obj = builder->CreateICmpUGE(vtype, getInteger(TARGET_BITS, T_OBJECT));
		gen_if(builder->CreateOr(is_str, is_obj), [&](){
			llvm::Value* stack_addr = builder->CreateGEP(read_global(&BP), getInteger(TARGET_BITS, index*sizeof(VALUE)));
			store_value(stack_addr, get_default(T_VARIANT), T_VARIANT, false);
		});
	}
}

void PopParamExpression::codegen(){
	llvm::Value* v = val->codegen_get_value();
	if (val->on_stack) c_SP(-1);
	llvm::Value* old_val = builder->CreateLoad(params[index + FP->n_param]);
	release(old_val, type);
	builder->CreateStore(v, params[index + FP->n_param]);
	if (type == T_STRING || (type == T_VARIANT && !val->no_ref_variant) || TYPE_is_object(type)){
		llvm::Value* stack_addr = builder->CreateGEP(read_global(&PP), getInteger(TARGET_BITS, index*sizeof(VALUE)));
		store_value(stack_addr, v, type, false);
	} else if (type == T_VARIANT && val->no_ref_variant){
		llvm::Value* vtype = extract_value(old_val, 0);
		llvm::Value* is_str = builder->CreateICmpEQ(vtype, getInteger(TARGET_BITS, T_STRING));
		llvm::Value* is_obj = builder->CreateICmpUGE(vtype, getInteger(TARGET_BITS, T_OBJECT));
		gen_if(builder->CreateOr(is_str, is_obj), [&](){
			llvm::Value* stack_addr = builder->CreateGEP(read_global(&PP), getInteger(TARGET_BITS, index*sizeof(VALUE)));
			store_value(stack_addr, get_default(T_VARIANT), T_VARIANT, false);
		});
	}
}

static llvm::AllocaInst* create_alloca_in_entry(llvm::Type* t){
	llvm::BasicBlock* bb = builder->GetInsertBlock();
	builder->SetInsertPoint(entry_block, entry_block->begin());
	llvm::AllocaInst* ret = builder->CreateAlloca(t);
	builder->SetInsertPoint(bb);
	return ret;
}

static llvm::AllocaInst* create_alloca_in_entry_init_default(llvm::Type* t, TYPE type){
	llvm::BasicBlock* bb = builder->GetInsertBlock();
	builder->SetInsertPoint(entry_block, entry_block->begin());
	llvm::AllocaInst* ret = builder->CreateAlloca(t);
	store_default(ret, type);
	builder->SetInsertPoint(bb);
	return ret;
}

static llvm::Value* release_ctrl(int index){
	llvm::Value* old_type = builder->CreateLoad(current_ctrl_types[index - FP->n_local]);
	if (is_ctrl_type_used(1, index)){
		gen_if(builder->CreateICmpEQ(old_type, getInteger(32, 1)), [&](){
			release(builder->CreateLoad(ctrl_values[index - FP->n_local][1]), T_STRING);
		}, "was_string_ctrl_before");
	}
	if (is_ctrl_type_used(2, index)){
		gen_if(builder->CreateICmpEQ(old_type, getInteger(32, 2)), [&](){
			release(builder->CreateLoad(ctrl_values[index - FP->n_local][2]), T_OBJECT);
		}, "was_object_ctrl_before");
	}
	if (is_ctrl_type_used(3, index)){
		gen_if(builder->CreateICmpEQ(old_type, getInteger(32, 3)), [&](){
			release(builder->CreateLoad(ctrl_values[index - FP->n_local][3]), T_VARIANT);
		}, "was_variant_ctrl_before");
	}
	return old_type;
}

static void set_ctrl(llvm::Value* v, TYPE type, int index){
	llvm::Value* old_type = release_ctrl(index);
	
	int type_next = special_ctrl_type(type);
	builder->CreateStore(getInteger(32, type_next), current_ctrl_types[index - FP->n_local]);
	set_ctrl_type(type, index);
	
	if (type_next == 0){
		if (locals[index] == NULL || locals[index]->getType() != TYPE_llvm(type))
			locals[index] = create_alloca_in_entry_init_default(TYPE_llvm(type), type);
	} else {
		locals[index] = ctrl_values[index - FP->n_local][type_next];
	}
	
	if (type == T_STRING || type == T_VARIANT || TYPE_is_object(type)){
		llvm::Value* stack_addr = builder->CreateGEP(read_global(&BP), getInteger(TARGET_BITS, index*sizeof(VALUE)));
		store_value(stack_addr, v, type);
	} else {
		gen_if(builder->CreateICmpNE(old_type, getInteger(32, 0)), [&](){
			llvm::Value* stack_addr = builder->CreateGEP(read_global(&BP), getInteger(TARGET_BITS, index*sizeof(VALUE)));
			store_value(stack_addr, v, type);
		}, "old_ctrl_needs_to_be_cleaned2");
	}
	
	if (type != T_NULL)
		builder->CreateStore(v, locals[index]);
#if 0
	TYPE old_ctrl_type = get_ctrl_type(index);
	set_ctrl_type(type, index);
	
	if (old_ctrl_type != T_VOID && old_ctrl_type != T_NULL)
		release(builder->CreateLoad(locals[index]), old_ctrl_type);
	if (type == T_NULL) return;
	if (old_ctrl_type != type){
		locals[index] = create_alloca_in_entry_init_default(TYPE_llvm(type), type);
	}
	builder->CreateStore(v, locals[index]);
	if (old_ctrl_type == T_STRING || old_ctrl_type == T_VARIANT || TYPE_is_object(old_ctrl_type) ||
		type == T_STRING || type == T_VARIANT || TYPE_is_object(type)){
		
		llvm::Value* stack_addr = builder->CreateGEP(read_global(&BP), getInteger(TARGET_BITS, index*sizeof(VALUE)));
		store_value(stack_addr, v, type);
	}
#endif
	/*builder->CreateStore(v, locals[index]);
	llvm::Value* stack_addr = builder->CreateGEP(read_global(&BP), getInteger(TARGET_BITS, index*sizeof(VALUE)));
	store_value(stack_addr, v, type);*/
}

static void codegen_pop_ctrl(llvm::Value* v, Expression* val, int index){
	TYPE type = val->type;
	
	llvm::Value* old_type = release_ctrl(index);
	
	int type_next = special_ctrl_type(type);
	builder->CreateStore(getInteger(32, type_next), current_ctrl_types[index - FP->n_local]);
	set_ctrl_type(type, index);
	
	if (type_next == 0){
		if (locals[index] == NULL || locals[index]->getType() != TYPE_llvm(type))
			locals[index] = create_alloca_in_entry_init_default(TYPE_llvm(type), type);
	} else {
		locals[index] = ctrl_values[index - FP->n_local][type_next];
	}
	
	if (type == T_STRING || (type == T_VARIANT && !val->no_ref_variant) || TYPE_is_object(type)){
		llvm::Value* stack_addr = builder->CreateGEP(read_global(&BP), getInteger(TARGET_BITS, index*sizeof(VALUE)));
		store_value(stack_addr, v, type);
	} else {
		gen_if(builder->CreateICmpNE(old_type, getInteger(32, 0)), [&](){
			llvm::Value* stack_addr = builder->CreateGEP(read_global(&BP), getInteger(TARGET_BITS, index*sizeof(VALUE)));
			store_value(stack_addr, v, type);
		}, "old_ctrl_needs_to_be_cleaned3");
	}
	
	if (val->on_stack) c_SP(-1);
	
	if (type != T_NULL)
		builder->CreateStore(v, locals[index]);
	
	/*
	//
	TYPE type = val->type;
	
	TYPE old_ctrl_type = get_ctrl_type(index);
	set_ctrl_type(type, index);
	
	if (val->on_stack) c_SP(-1);
	if (old_ctrl_type != T_VOID && old_ctrl_type != T_NULL)
		release(builder->CreateLoad(locals[index]), old_ctrl_type);
	if (type == T_NULL) return;
	if (old_ctrl_type != type){
		locals[index] = create_alloca_in_entry_init_default(TYPE_llvm(type), type);
	}
	builder->CreateStore(v, locals[index]);
	if (old_ctrl_type == T_STRING || old_ctrl_type == T_VARIANT || TYPE_is_object(old_ctrl_type) ||
		type == T_STRING || (type == T_VARIANT && !val->no_ref_variant) || TYPE_is_object(type)){
		
		llvm::Value* stack_addr = builder->CreateGEP(read_global(&BP), getInteger(TARGET_BITS, index*sizeof(VALUE)));
		store_value(stack_addr, v, type);
	}
	*/
}

static void codegen_pop_ctrl(Expression* val, int index){
	codegen_pop_ctrl(val->codegen_get_value(), val, index);
	return;
	
	/*TYPE type = val->type;
	
	TYPE old_ctrl_type = get_ctrl_type(index);
	set_ctrl_type(type, index);
	
	llvm::Value* v = val->codegen_get_value();
	if (val->on_stack) c_SP(-1);
	if (old_ctrl_type != T_VOID && old_ctrl_type != T_NULL)
		release(builder->CreateLoad(locals[index]), old_ctrl_type);
	if (type == T_NULL) return;
	if (old_ctrl_type != type){
		locals[index] = create_alloca_in_entry_init_default(TYPE_llvm(type), type);
	}
	builder->CreateStore(v, locals[index]);
	if (old_ctrl_type == T_STRING || old_ctrl_type == T_VARIANT || TYPE_is_object(old_ctrl_type) ||
		type == T_STRING || (type == T_VARIANT && !val->no_ref_variant) || TYPE_is_object(type)){
		
		llvm::Value* stack_addr = builder->CreateGEP(read_global(&BP), getInteger(TARGET_BITS, index*sizeof(VALUE)));
		store_value(stack_addr, v, type);
	}*/
}

//Same code as PopLocalExpression::codegen
void PopCtrlExpression::codegen(){
	if (!isa<PushClassExpression>(val))
		codegen_pop_ctrl(val, index);
}

void PopOptionalExpression::codegen(){
	llvm::Value* addr = builder->CreateBitCast(builder->CreateGEP(read_global(&PP), getInteger(TARGET_BITS, index*sizeof(VALUE))), pointer_t(LONG_TYPE));
	llvm::Value* t = builder->CreateLoad(addr);
	gen_if(builder->CreateICmpEQ(t, getInteger(TARGET_BITS, T_VOID)), [&](){
		if (is_default){
			store_default(params[FP->n_param + index], type);
		} else {
			llvm::Value* value = val->codegen_get_value();
			store_value(addr, value, type);
			builder->CreateStore(value, params[FP->n_param + index]);
			if (val->on_stack) c_SP(-1);
		}
	}, "not_passed", "passed_or_done");
}

llvm::Value* PushAutoCreateExpression::codegen_get_value(){
	llvm::Value* ret = builder->CreateCall2(get_global_function(GB.AutoCreate, 'p', "pi"),
		get_global((void*)klass, llvmType(getInt8Ty)), getInteger(32, 0));
	
	borrow_object_no_nullcheck(ret);
	
	ret = get_new_struct(object_type,
		get_global((void*)klass, llvmType(getInt8Ty)), ret);
	
	if (on_stack)
		push_value(ret, type);
	return ret;
}

void PushPureObjectUnknownExpression::codegen_on_stack(){
	llvm::Value* ob = obj->codegen_get_value();
	llvm::Value* object = extract_value(ob, 1);
	CLASS* k = (CLASS*)(void*)obj->type;
	llvm::Value* effective_class;
	
	if (isa<PushSuperExpression>(obj)){
		effective_class = builder->CreateIntToPtr(getInteger(TARGET_BITS, (int64_t)(intptr_t)(void*)k), llvmType(getInt8PtrTy));
	} else if (!k->is_virtual){
		make_nullcheck(object);
		effective_class = load_element(builder->CreateBitCast(object, pointer_t(OBJECT_type)), 0);
	} else {
		effective_class = builder->CreateIntToPtr(getInteger(TARGET_BITS, (int64_t)(intptr_t)(void*)k), llvmType(getInt8PtrTy));
	}
	
	create_check(k, effective_class, object);
	
	/*llvm::Value* func = insert_value(get_new_struct(function_type), object, 1);*/
	
	builder->CreateCall4(get_global_function(JR_push_unknown_property_unknown, 'v', "pipp"),
		builder->CreateIntToPtr(getInteger(TARGET_BITS, (int64_t)(void*)name), llvmType(getInt8PtrTy)),
		getInteger(32, name_id),
		effective_class,
		object);
}

llvm::Value* PushPureObjectUnknownExpression::codegen_get_value(){
	codegen_on_stack();
	return ret_top_stack(T_VARIANT, true);
}

void PushStaticUnknownExpression::codegen_on_stack(){
	builder->CreateCall4(get_global_function(JR_push_unknown_property_unknown, 'v', "pipp"),
		builder->CreateIntToPtr(getInteger(TARGET_BITS, (int64_t)(void*)name), llvmType(getInt8PtrTy)),
		getInteger(32, name_id),
		builder->CreateIntToPtr(getInteger(TARGET_BITS, (int64_t)(void*)klass), llvmType(getInt8PtrTy)),
		get_nullptr());
}

llvm::Value* PushStaticUnknownExpression::codegen_get_value(){
	codegen_on_stack();
	return ret_top_stack(T_VARIANT, true);
}

llvm::Value* PushPureObjectFunctionExpression::codegen_get_value(){
	llvm::Value* ob = obj->codegen_get_value();
	llvm::Value* object = extract_value(ob, 1);
	CLASS* k = klass();
	CLASS_DESC* d = desc();
	
	if (isa<PushSuperExpression>(obj)){
		effective_class = builder->CreateIntToPtr(getInteger(TARGET_BITS, (int64_t)(intptr_t)(void*)k), llvmType(getInt8PtrTy));
	} else if (!k->is_virtual){
		make_nullcheck(object);
		effective_class = load_element(builder->CreateBitCast(object, pointer_t(OBJECT_type)), 0);
	} else {
		//FIXME Doesn't PushVirtualFunctionExpression take care of this?
		//CLASS_load(k);
		effective_class = builder->CreateIntToPtr(getInteger(TARGET_BITS, (int64_t)(intptr_t)(void*)k), llvmType(getInt8PtrTy));
	}
	
	create_check(k, effective_class, object);
	
	llvm::Value* func = insert_value(get_new_struct(function_type), object, 1);
	
	if (!d->method.native){
		//Assume native classes don't inherit non-native ones :)
	} else {
		int offset_table = TARGET_BITS == 64 ? 40/8 : 28/4;
		int offset_desc_in_desc_symbol = TARGET_BITS == 64 ? 12 : 8;
		int offset_native_flag = TARGET_BITS == 64 ? 35 : 19;
		//table_addr = (char*)effective_class->table
		llvm::Value* table_addr = builder->CreateLoad(builder->CreateGEP(builder->CreateBitCast(effective_class, pointer_t(llvmType(getInt8PtrTy))), getInteger(TARGET_BITS, offset_table)));
		//desc_addr_addr = (char*)(table_addr + sizeof(CLASS_DESC_SYMBOL) * index + desc_offset)
		llvm::Value* desc_addr_addr = builder->CreateGEP(table_addr, getInteger(TARGET_BITS, sizeof(CLASS_DESC_SYMBOL) * index + offset_desc_in_desc_symbol));
		//desc_addr = *(char**)desc_addr_addr
		llvm::Value* desc_addr = builder->CreateLoad(builder->CreateBitCast(desc_addr_addr, pointer_t(llvmType(getInt8PtrTy))));
		//native_flag_addr = (char*)(desc_addr + offset_native_flag)
		llvm::Value* native_flag_addr = builder->CreateGEP(desc_addr, getInteger(TARGET_BITS, offset_native_flag));
		//native_subr_flag = *native_flag_addr
		llvm::Value* native_subr_flag = builder->CreateLoad(native_flag_addr);
		//native_flag = (bool)(native_subr_flag & 1)
		llvm::Value* native_flag = builder->CreateTrunc(native_subr_flag, llvmType(getInt1Ty));
		//subr_flag = (bool)(native_subr_flag & 2)
		//llvm::Value* subr_flag = builder->CreateICmpNE(builder->CreateAnd(native_subr_flag, getInteger(8, 2)), getInteger(8, 0));
		
		//llvm::Value* function_kind = builder->CreateSelect(native_flag, builder->CreateSelect(subr_flag, getInteger(8, FUNCTION_SUBR), getInteger(8, FUNCTION_NATIVE)), getInteger(8, FUNCTION_PUBLIC));
		llvm::Value* function_kind = builder->CreateSelect(native_flag, getInteger(8, FUNCTION_NATIVE), getInteger(8, FUNCTION_PUBLIC));
		///Disable subr because it is only meant to be used static
		func = insert_value(func, function_kind, 2);
	}
	if (obj->on_stack)
		c_SP(-1);
	if (on_stack)
		push_value(func, T_FUNCTION);
	return func;
}

llvm::Value* PushVirtualFunctionExpression::codegen_get_value(){
	llvm::Value* ob = obj->codegen_get_value();
	llvm::Value* object = extract_value(ob, 1);
	CLASS* k = klass();
	
	effective_class = get_global((void*)k, llvmType(getInt8Ty));
	
	create_check(k, effective_class, object);
	
	llvm::Value* func = insert_value(insert_value(get_new_struct(function_type), object, 1), getInteger(8, FUNCTION_NATIVE), 2);
	if (obj->on_stack)
		c_SP(-1);
	if (on_stack)
		push_value(func, T_FUNCTION);
	return func;
}

llvm::Value* PushVirtualStaticFunctionExpression::codegen_get_value(){
	llvm::Value* ob = obj->codegen_get_value();
	llvm::Value* object = extract_value(ob, 1);
	CLASS* k = klass();
	
	effective_class = get_global((void*)k, llvmType(getInt8Ty));
	
	llvm::Value* func = insert_value(insert_value(get_new_struct(function_type), object, 1), getInteger(8, FUNCTION_NATIVE), 2);
	if (obj->on_stack)
		c_SP(-1);
	if (on_stack)
		push_value(func, T_FUNCTION);
	return func;
}

//Almost the same code as above, but setting object to null ...
llvm::Value* PushPureObjectStaticFunctionExpression::codegen_get_value(){
	llvm::Value* ob = obj->codegen_get_value();
	llvm::Value* object = extract_value(ob, 1);
	CLASS* k = klass();
	CLASS_DESC* d = desc();
	
	if (isa<PushSuperExpression>(obj)){
		effective_class = builder->CreateIntToPtr(getInteger(TARGET_BITS, (int64_t)(intptr_t)(void*)k), llvmType(getInt8PtrTy));
	} else if (!k->is_virtual){
		make_nullcheck(object);
		effective_class = load_element(builder->CreateBitCast(object, pointer_t(OBJECT_type)), 0);
	} else {
		//CLASS_load(k);
		effective_class = builder->CreateIntToPtr(getInteger(TARGET_BITS, (int64_t)(intptr_t)(void*)k), llvmType(getInt8PtrTy));
	}
	
	create_check(k, effective_class, object);
	
	unref_object(object);
	object = get_nullptr();
	
	llvm::Value* func = insert_value(get_new_struct(function_type), object, 1);
	
	if (!d->method.native){
		//Assume native classes don't inherit non-native ones :)
	} else {
		int offset_table = TARGET_BITS == 64 ? 40/8 : 28/4;
		int offset_desc_in_desc_symbol = TARGET_BITS == 64 ? 12 : 8;
		int offset_native_flag = TARGET_BITS == 64 ? 35 : 19;
		//table_addr = (char*)effective_class->table
		llvm::Value* table_addr = builder->CreateLoad(builder->CreateGEP(builder->CreateBitCast(effective_class, pointer_t(llvmType(getInt8PtrTy))), getInteger(TARGET_BITS, offset_table)));
		//desc_addr_addr = (char*)(table_addr + sizeof(CLASS_DESC_SYMBOL) * index + desc_offset)
		llvm::Value* desc_addr_addr = builder->CreateGEP(table_addr, getInteger(TARGET_BITS, sizeof(CLASS_DESC_SYMBOL) * index + offset_desc_in_desc_symbol));
		//desc_addr = *(char**)desc_addr_addr
		llvm::Value* desc_addr = builder->CreateLoad(builder->CreateBitCast(desc_addr_addr, pointer_t(llvmType(getInt8PtrTy))));
		//native_flag_addr = (char*)(desc_addr + offset_native_flag)
		llvm::Value* native_flag_addr = builder->CreateGEP(desc_addr, getInteger(TARGET_BITS, offset_native_flag));
		//native_subr_flag = *native_flag_addr
		llvm::Value* native_subr_flag = builder->CreateLoad(native_flag_addr);
		//native_flag = (bool)(native_subr_flag & 1)
		llvm::Value* native_flag = builder->CreateTrunc(native_subr_flag, llvmType(getInt1Ty));
		//subr_flag = (bool)(native_subr_flag & 2)
		//llvm::Value* subr_flag = builder->CreateICmpNE(builder->CreateAnd(native_subr_flag, getInteger(8, 2)), getInteger(8, 0));
		
		//llvm::Value* function_kind = builder->CreateSelect(native_flag, builder->CreateSelect(subr_flag, getInteger(8, FUNCTION_SUBR), getInteger(8, FUNCTION_NATIVE)), getInteger(8, FUNCTION_PUBLIC));
		llvm::Value* function_kind = builder->CreateSelect(native_flag, getInteger(8, FUNCTION_NATIVE), getInteger(8, FUNCTION_PUBLIC));
		///Disable subr, because I think it will never be used this way anyway, else it is a (big) performance hit.
		func = insert_value(func, function_kind, 2);
	}
	if (obj->on_stack)
		c_SP(-1);
	if (on_stack)
		push_value(func, T_FUNCTION);
	return func;
}

llvm::Value* PushStaticFunctionExpression::codegen_get_value(){
	effective_class = builder->CreateIntToPtr(getInteger(TARGET_BITS, (int64_t)(intptr_t)(void*)klass()), llvmType(getInt8PtrTy));
	
	llvm::Value* ret = get_new_struct(function_type, effective_class, get_nullptr());
	
	if (on_stack)
		push_value(ret, T_FUNCTION);
	return ret;
}

llvm::Value* PushPureObjectStructFieldExpression::codegen_get_value(){
	llvm::Value* struct_obj = extract_value(obj->codegen_get_value(), 1);
	
	make_nullcheck(struct_obj);
	
	llvm::Value* ref_addr = builder->CreateBitCast(builder->CreateGEP(struct_obj, getInteger(TARGET_BITS, offsetof(CSTRUCT, ref))), charPP);
	
	llvm::Value* element_addr = gen_if_else_phi(builder->CreateICmpNE(builder->CreateLoad(ref_addr), get_nullptr()), [&](){
		llvm::Value* addr_addr = builder->CreateBitCast(builder->CreateGEP(struct_obj, getInteger(TARGET_BITS, offsetof(CSTATICSTRUCT, addr))), charPP);
		return builder->CreateGEP(builder->CreateLoad(addr_addr), getInteger(TARGET_BITS, this->desc()->variable.offset));
	}, [&](){
		return builder->CreateGEP(struct_obj, getInteger(TARGET_BITS, sizeof(CSTRUCT) + this->desc()->variable.offset));
	});
	
	llvm::Value* ret;
	
	int ctype_id = desc()->variable.ctype.id;
	if (ctype_id == TC_ARRAY){
		//Embedded array inside struct
		ret = codegen_tc_array(desc()->variable.klass, struct_obj, desc()->variable.ctype.value, element_addr, type);
	} else if (ctype_id == TC_STRUCT){
		//Embedded struct inside struct
		ret = builder->CreateCall3(get_global_function_jif(CSTRUCT_create_static, 'p', "ppp"),
			get_global((void*)desc()->variable.klass, llvmType(getInt8Ty)),
			builder->CreateIntToPtr(getInteger(TARGET_BITS, type), llvmType(getInt8PtrTy)),
			element_addr);
		borrow_object_no_nullcheck(ret);
		
		ret = get_new_struct(object_type, builder->CreateIntToPtr(getInteger(TARGET_BITS, type), llvmType(getInt8PtrTy)), ret);
	} else {
		ret = read_variable(type, element_addr);
	}
	
	unref_object_no_nullcheck(struct_obj);
	
	c_SP(-obj->on_stack);
	if (on_stack)
		push_value(ret, type);
	return ret;
}

void PopPureObjectStructFieldExpression::codegen(){
	llvm::Value* value = val->codegen_get_value();
	llvm::Value* struct_obj = extract_value(obj->codegen_get_value(), 1);
	
	make_nullcheck(struct_obj);
	
	llvm::Value* ref_addr = builder->CreateBitCast(builder->CreateGEP(struct_obj, getInteger(TARGET_BITS, offsetof(CSTRUCT, ref))), charPP);
	
	llvm::Value* element_addr = gen_if_else_phi(builder->CreateICmpNE(builder->CreateLoad(ref_addr), get_nullptr()), [&](){
		llvm::Value* addr_addr = builder->CreateBitCast(builder->CreateGEP(struct_obj, getInteger(TARGET_BITS, offsetof(CSTATICSTRUCT, addr))), charPP);
		return builder->CreateGEP(builder->CreateLoad(addr_addr), getInteger(TARGET_BITS, this->desc()->variable.offset));
	}, [&](){
		return builder->CreateGEP(struct_obj, getInteger(TARGET_BITS, sizeof(CSTRUCT) + this->desc()->variable.offset));
	});
	
	release_variable(type, element_addr);
	variable_write(element_addr, value, type);
	
	unref_object_no_nullcheck(struct_obj);
}

llvm::Value* SwapExpression::codegen_get_value(){
	llvm::Value* ret = push_a_expr->codegen_get_value();
	pop_a_expr->codegen();
	return ret;
}

void SwapExpression::codegen_on_stack(){
	push_a_expr->codegen_on_stack();
	pop_a_expr->codegen();
}

static llvm::Type* const extern_types[] = {
	llvmType(getVoidTy),
	llvmType(getInt8Ty),
	llvmType(getInt8Ty),
	llvmType(getInt16Ty),
	llvmType(getInt32Ty),
	llvmType(getInt64Ty),
	llvmType(getFloatTy),
	llvmType(getDoubleTy),
	NULL,
	llvmType(getInt8PtrTy),
	llvmType(getInt8PtrTy),
	llvmType(getInt8PtrTy),
	NULL,
	NULL,
	NULL,
	llvmType(getInt8PtrTy),
	llvmType(getInt8PtrTy)
};

llvm::Value* codegen_extern_manage_value(llvm::Value* val, TYPE type){
	if (type == T_BOOLEAN)
		val = builder->CreateZExt(val, llvmType(getInt8Ty));
	
	else if (TYPE_is_string(type))
		val = builder->CreateGEP(extract_value(val, 1), to_target_int(extract_value(val, 2)));
	
	else if (TYPE_is_object(type)){
		val = extract_value(val, 1);
		val = gen_if_phi(get_nullptr(), builder->CreateICmpNE(val, get_nullptr()), [&](){
			CLASS* object_class = (CLASS*)(void*)type;
				
			llvm::Value* normal = builder->CreateGEP(val, getInteger(TARGET_BITS, sizeof(OBJECT)));
			
			llvm::Value* OBJ = builder->CreateBitCast(val, pointer_t(OBJECT_type));
			llvm::Value* klass = load_element(OBJ, 0);
			
			auto get_bit_from_class = [](llvm::Value* obj, int offset_byte, int offset_bit){
				return builder->CreateTrunc(builder->CreateLShr(builder->CreateLoad(builder->CreateGEP(obj, getInteger(TARGET_BITS, offset_byte))), getInteger(8, offset_bit)), llvmType(getInt1Ty));
			};
			
			auto handle_class_object = [normal, &get_bit_from_class](llvm::Value* obj){
				const int offset_is_native = TARGET_BITS == 64 ? 34 : 22;
				const int bit_index_is_native = 2;
				
				llvm::Value* is_native = get_bit_from_class(obj, offset_is_native, bit_index_is_native);
				return gen_if_phi(normal, builder->CreateXor(is_native, getInteger(1, true)), [obj](){
					llvm::Value* stat = builder->CreateBitCast(builder->CreateGEP(obj, getInteger(TARGET_BITS, offsetof(CLASS, stat))), charPP);
					return builder->CreateLoad(stat);
				}, "not_native");
			};
			
			auto handle_struct_object = [](llvm::Value* obj){
				/*if (((CSTRUCT *)ob)->ref)
					addr = (char *)((CSTATICSTRUCT *)ob)->addr;
				else
					addr = (char *)ob + sizeof(CSTRUCT);*/
				llvm::Value* ref_addr = builder->CreateBitCast(builder->CreateGEP(obj, getInteger(TARGET_BITS, offsetof(CSTRUCT, ref))), charPP);
				llvm::Value* ref_not_null = builder->CreateICmpNE(builder->CreateLoad(ref_addr), get_nullptr());
				
				return gen_if_phi(builder->CreateGEP(obj, getInteger(TARGET_BITS, sizeof(CSTRUCT))), ref_not_null, [&](){
					llvm::Value* addr_addr = builder->CreateBitCast(builder->CreateGEP(obj, getInteger(TARGET_BITS, offsetof(CSTATICSTRUCT, addr))), charPP);
					return builder->CreateLoad(addr_addr);
				});
			};
			
			if (TYPE_is_pure_object(type) && object_class == (CLASS*)(void*)GB.FindClass("Class")){
				val = handle_class_object(val);
			} else if (TYPE_is_pure_object(type) && CLASS_is_array(object_class)){
				val = builder->CreateLoad(builder->CreateBitCast(builder->CreateGEP(val, getInteger(TARGET_BITS, offsetof(CARRAY, data))), charPP));
			} else if (TYPE_is_pure_object(type) && CLASS_is_struct(object_class)){
				val = handle_struct_object(val);
			} else if (TYPE_is_pure_object(type)){
				val = normal;
			} else {
				val = gen_if_else_phi(builder->CreateICmpEQ(klass, builder->CreateIntToPtr(getInteger(TARGET_BITS, GB.FindClass("Class")), llvmType(getInt8PtrTy))), [&](){
					return handle_class_object(val);
				}, [&](){
					const int offset_is_array = TARGET_BITS == 64 ? 34 : 22;
					const int bit_index_is_array = 6;
					
					return gen_if_else_phi(get_bit_from_class(klass, offset_is_array, bit_index_is_array), [&](){
						return builder->CreateLoad(builder->CreateBitCast(builder->CreateGEP(val, getInteger(TARGET_BITS, offsetof(CARRAY, data))), charPP));
					}, [&](){
						const int offset_is_struct = TARGET_BITS == 64 ? 34 : 22;
						const int bit_index_is_struct = 5;
						
						return gen_if_phi(normal, get_bit_from_class(klass, offset_is_struct, bit_index_is_struct), [&](){
							return handle_struct_object(val);
						}, "extern_arg_is_struct");
					}, "extern_arg_is_array");
				}, "extern_arg_is_class");
			}
			return val;
		}, "OBJ_not_null_for_extern");
	}
	return val;
}

static llvm::Value* codegen_extern_manage_return_value(llvm::Value* ret, TYPE type){
	if (type == T_BOOLEAN)
		ret = builder->CreateICmpNE(ret, getInteger(8, false));
	else if (TYPE_is_string(type)){
		ret = gen_if_phi(get_default(T_CSTRING), builder->CreateICmpNE(ret, get_nullptr()), [&](){
			return get_cstring_from_addr(ret);
		}, "extern_return_not_nullstring");
	} else if (TYPE_is_object(type)){
		if (TYPE_is_pure_object(type)){
			CLASS* class_struct = (CLASS*)(void*)type;
			if (CLASS_is_struct(class_struct)){
				ret = builder->CreateCall3(get_global_function_jif(CSTRUCT_create_static, 'p', "ppp"),
					get_global((void*)-1, llvmType(getInt8Ty)),
					get_global((void*)class_struct, llvmType(getInt8Ty)),
					ret);
			}
		}
		borrow_object(ret);
		ret = get_new_struct(object_type, builder->CreateIntToPtr(getInteger(TARGET_BITS, type), llvmType(getInt8PtrTy)), ret);
	}
	return ret;
}

static void func_extern_call_variant_vararg(void* return_value_addr, void* func_addr, int nargs, TYPE return_type);

static llvm::Value* codegen_raise_event(std::vector<Expression*>& args, int index, bool on_stack){
	for(size_t i=0, e=args.size(); i!=e; i++){
		args[i]->codegen_on_stack();
	}
	llvm::Value* event_stop = builder->CreateCall3(get_global_function_vararg(GB.Raise, 'c', "pii"),
		current_op, getInteger(32, index), getInteger(32, -args.size()));
	event_stop = builder->CreateTrunc(event_stop, llvmType(getInt1Ty));
	
	if (on_stack)
		push_value(event_stop, T_BOOLEAN);
	return event_stop;
}

llvm::Value* CallExpression::codegen_get_value(){
	if (func->type == T_CLASS && desc == NULL){
		//Cast
		llvm::Value* ret = args[0]->codegen_get_value();
		if (!args[0]->on_stack && on_stack){
			push_value(ret, type);
		} else if (args[0]->on_stack && !on_stack){
			c_SP(-1);
		}
		return ret;
	}
	if (FunctionExpression* fe = dynamic_cast<FunctionExpression*>(func)){
		if (fe->function_expr_type == EventFn){
			return codegen_raise_event(args, ((PushEventExpression*)func)->index, on_stack);
		}
	}
	if (PushExternExpression* ee = dynamic_cast<PushExternExpression*>(func)){
		llvm::Value* call_addr;
		
		if (ee->object_to_release){
			llvm::Value* obj = ee->object_to_release->codegen_get_value();
			c_SP(-ee->object_to_release->on_stack);
			call_addr = builder->CreateCall2(get_global_function(JR_extern_dispatch_object, 'p', "pi"), extract_value(obj, 1), getInteger(32, ee->index));
		}
		
		CLASS_EXTERN* ext;
		if (ee->object_to_release)
			ext = &ee->klass->load->ext[ee->klass->table[ee->index].desc->ext.exec]; //An extern method with the correct signature
		else 
			ext = &ee->klass->load->ext[ee->index];
		EXTERN_FUNC_INFO func = JIF.F_EXTERN_get_function_info(ext);
		
		llvm::Value* ret = NULL;
		
		if (!ee->must_variant_dispatch){
			std::vector<llvm::Type*> ft;
			std::vector<llvm::Value*> orig_args;
			std::vector<llvm::Value*> func_args;
			ft.resize(ext->n_param);
			orig_args.resize(args.size());
			func_args.resize(args.size());
			
			for(size_t i=0; i<args.size(); i++){
				if (i < (size_t)ext->n_param)
					ft[i] = extern_types[ext->param[i].type];
				
				llvm::Value* val = args[i]->codegen_get_value();
				orig_args[i] = val;
				
				val = codegen_extern_manage_value(val, args[i]->type);
				func_args[i] = val;
			}
			
			llvm::FunctionType* function_type = llvm::FunctionType::get(extern_types[type > T_OBJECT ? T_OBJECT : type], ft, true);
			
			std::string function_name = ext->library;
			function_name += '.';
			function_name += func.alias;
			
			llvm::Value* call_function;
			if (!ee->object_to_release){
				llvm::GlobalValue* glob_val = (llvm::GlobalValue*)M->getOrInsertFunction(function_name, function_type);
				register_global_symbol(function_name, glob_val, func.call);
				call_function = glob_val;
			} else {
				call_function = builder->CreateBitCast(call_addr, pointer_t(function_type));
			}
			
			ret = builder->CreateCall(call_function, func_args);
			
			ret = codegen_extern_manage_return_value(ret, type);
			
			for(size_t arg=args.size(); arg --> 0; ){
				release(orig_args[arg], args[arg]->type);
				if (args[arg]->on_stack)
					c_SP(-1);
			}
		} else {
			for(size_t i=0; i<args.size(); i++){
				args[i]->codegen_on_stack();
			}
			
			llvm::Value* return_value_addr = type == T_VOID ? get_nullptr() : (llvm::Value*)create_alloca_in_entry(TYPE_llvm(type));
			if (!ee->object_to_release)
				call_addr = builder->CreateIntToPtr(getInteger(TARGET_BITS, (uint64_t)func.call), llvmType(getInt8PtrTy));
			llvm::Value* args_size = getInteger(32, args.size());
			llvm::Value* return_type = getInteger(TARGET_BITS, type);
			
			builder->CreateCall4(get_global_function(func_extern_call_variant_vararg, 'v', "ppij"),
				builder->CreateBitCast(return_value_addr, llvmType(getInt8PtrTy)),
				call_addr,
				args_size,
				return_type);
			
			if (type != T_VOID)
				ret = builder->CreateLoad(return_value_addr);
		}
		
		if (on_stack)
			push_value(ret, type);
		
		return ret;
	}
	codegen_on_stack();
	return ret_top_stack(type, on_stack);
}
void CallExpression::codegen_on_stack(){
	FunctionExpression* fe = dynamic_cast<FunctionExpression*>(func);
	if (fe && fe->function_kind == FUNCTION_SUBR){
		//String.Mid, String.Left etc
		
		for(size_t i=0, e=args.size(); i!=e; i++){
			args[i]->codegen_on_stack();
		}
		
		void* f = (void*)klass->table[index].desc->method.exec;
		builder->CreateCall(
			builder->CreateBitCast(get_global(f, llvmType(getInt8Ty)), pointer_t(get_function_type('v', "h"))),
			getInteger(16, args.size()));
		
		return;
	}
	if (fe && fe->function_kind == FUNCTION_EVENT){
		codegen_raise_event(args, ((PushEventExpression*)func)->index, on_stack);
		return;
	}
	if (isa<PushExternExpression>(func)){
		codegen_get_value();
		return;
	}
	
	llvm::Value *func_value, *object;
	
	FunctionExpression fe_temp;
	
	if (func->type == T_CLASS){
		if (desc == NULL){
			//Cast
			llvm::Value* ret = args[0]->codegen_get_value();
			if (!args[0]->on_stack){
				push_value(ret, type);
			}
			return;
		}
		func_value = get_new_struct(function_type, get_global((void*)klass, llvmType(getInt8Ty)), get_nullptr());
		push_value(func_value, T_FUNCTION);
		object = get_nullptr();
		
		fe = &fe_temp;
		
		fe->function_class = klass;
		fe->function_expr_type = ClassFn;
		fe->function_kind = desc->method.native ? FUNCTION_NATIVE : FUNCTION_PUBLIC;
		fe->effective_class = get_global((void*)klass, llvmType(getInt8Ty));
		fe->function_unknown = NULL;
	} else if (fe != NULL){
		func_value = func->codegen_get_value();
		object = extract_value(func_value, 1);
	} else {
		//Variant call
		func->codegen_on_stack();
	}
	//llvm::Value* func_value_addr = builder->CreateBitCast(get_value_on_top_addr(), pointer_t(value_types[T_FUNCTION]));
	//llvm::Value* object = load_element(func_value_addr, 2);
	
	for(size_t i=0, e=args.size(); i!=e; i++){
		args[i]->codegen_on_stack();
	}
	
	//FIXME might only be needed for non-quick calls:
	builder->CreateStore(getInteger(TARGET_BITS, (int64_t)(intptr_t)pc), get_global((void*)&PC, LONG_TYPE));
	
	if (fe != NULL){
		builder->CreateStore(getInteger(8, args.size()), get_global((void*)&EXEC.nparam, llvmType(getInt8Ty)));
		if (fe->function_expr_type == PrivateFn){
			builder->CreateStore(getInteger(32, index), get_global((void*)&EXEC.index, llvmType(getInt32Ty)));
			builder->CreateStore(current_op, get_global((void*)&EXEC.object));
			builder->CreateStore(get_global((void*)klass, llvmType(getInt8Ty)), get_global((void*)&EXEC.klass));
			
			if (can_quick)
				builder->CreateCall(get_global_function_jif(EXEC_enter_quick, 'v', ""));
			else
				builder->CreateCall(get_global_function_jif(EXEC_enter, 'v', ""));
			if (klass->load->func[index].fast)
				builder->CreateCall(get_global_function(JR_EXEC_jit_execute_function, 'v', ""));
			else
				builder->CreateCall(get_global_function_jif(EXEC_function_loop, 'v', ""));
		} /*else if (fe->function_expr_type == UnknownFn){
			*pc |= CODE_CALL_VARIANT;
		} */else {
			if (fe->function_unknown){
				builder->CreateStore(getInteger(TARGET_BITS, (int64_t)(intptr_t)fe->function_unknown), get_global((void*)&EXEC_unknown_name, LONG_TYPE));
				*pc |= CODE_CALL_VARIANT;
			}
			
			if (fe->function_kind == FUNCTION_PUBLIC){
				if (can_quick)
					builder->CreateCall3(get_global_function(JR_exec_enter_quick, 'v', "ppi"), fe->effective_class, object, getInteger(32, index));
				else
					builder->CreateCall3(get_global_function(JR_exec_enter, 'v', "ppi"), fe->effective_class, object, getInteger(32, index));
			} else if (fe->function_kind == FUNCTION_NATIVE){
					builder->CreateStore(get_global((void*)&desc->method, llvmType(getInt8Ty)), get_global((void*)&EXEC.desc, llvmType(getInt8PtrTy)));
					builder->CreateStore(object, get_global((void*)&EXEC.object));
					builder->CreateStore(fe->effective_class, get_global((void*)&EXEC.klass));
				
				if (can_quick)
					builder->CreateCall(get_global_function_jif(EXEC_native_quick, 'v', ""));
				else {
					builder->CreateStore(getInteger(8, true), get_global((void*)&EXEC.use_stack, llvmType(getInt8Ty)));
					builder->CreateCall(get_global_function_jif(EXEC_native, 'v', ""));
				}
			} else if (fe->function_kind == -1){
				llvm::Value* kind = extract_value(func_value, 2);
				gen_if_else(builder->CreateICmpEQ(kind, getInteger(8, FUNCTION_PUBLIC)), [&](){
					//Public
					if (can_quick)
						builder->CreateCall3(get_global_function(JR_exec_enter_quick, 'v', "ppi"), fe->effective_class, object, getInteger(32, index));
					else
						builder->CreateCall3(get_global_function(JR_exec_enter, 'v', "ppi"), fe->effective_class, object, getInteger(32, index));
					//builder->CreateCall(get_global_function(EXEC_jit_execute_function, 'v', ""));
				}, [&](){
					//Native
					builder->CreateStore(get_global((void*)&desc->method, llvmType(getInt8Ty)), get_global((void*)&EXEC.desc));
					builder->CreateStore(object, get_global((void*)&EXEC.object));
					builder->CreateStore(fe->effective_class, get_global((void*)&EXEC.klass));
					
					if (can_quick)
						builder->CreateCall(get_global_function_jif(EXEC_native_quick, 'v', ""));
					else {
						builder->CreateStore(getInteger(8, true), get_global((void*)&EXEC.use_stack, llvmType(getInt8Ty)));
						builder->CreateCall(get_global_function_jif(EXEC_native, 'v', ""));
					}
				});
			} else {
				//FIXME hmm does this ever happen?
				abort();
			}
		}
	} else {
		//Variant call
		builder->CreateCall(get_global_function(JR_call, 'v', "i"), getInteger(32, args.size()));
	}
	if (!byref_expressions.empty()){
		gen_if_noreturn(builder->CreateICmpEQ(read_global((void*)&PC, llvmType(getInt16PtrTy)), get_global((void*)pc, llvmType(getInt16Ty))), [&](){
			create_throw(E_BYREF);
		});
		for(size_t i=0, e=byref_expressions.size(); i!=e; i++){
			byref_expressions[i]->codegen();
		}
	}
	
	return;
#if 0
	if (desc != NULL || kind == FUNCTION_PRIVATE){
		if (kind != -1){
			switch(kind){
				case FUNCTION_NULL: abort();
				case FUNCTION_NATIVE:
					builder->CreateStore(getInteger(8, true), get_global((void*)&EXEC.use_stack, llvmType(getInt8Ty)));
					//builder->CreateStore(getInteger(8, true), get_global((void*)&EXEC.native, llvmType(getInt8Ty)));
					builder->CreateStore(getInteger(32, index), get_global((void*)&EXEC.index, llvmType(getInt32Ty)));
					builder->CreateStore(get_global((void*)&desc->method, llvmType(getInt8Ty)), get_global((void*)&EXEC.desc, llvmType(getInt8PtrTy)));
					builder->CreateStore(object, get_global((void*)&EXEC.object));
					builder->CreateStore(get_global((void*)klass, llvmType(getInt8Ty)), get_global((void*)&EXEC.klass));
					
					
					if (can_quick)
						builder->CreateCall(get_global_function(EXEC_native_quick, 'v', ""));
					else 
						builder->CreateCall(get_global_function(EXEC_native, 'v', ""));
					break;
				
				case FUNCTION_PRIVATE:
					//builder->CreateStore(getInteger(8, false), get_global((void*)&EXEC.native, llvmType(getInt8Ty)));
					builder->CreateStore(getInteger(32, index), get_global((void*)&EXEC.index, llvmType(getInt32Ty)));
					builder->CreateStore(object, get_global((void*)&EXEC.object));
					builder->CreateStore(get_global((void*)klass, llvmType(getInt8Ty)), get_global((void*)&EXEC.klass));
					
					if (can_quick)
						builder->CreateCall(get_global_function(EXEC_enter_quick, 'v', ""));
					else
						builder->CreateCall(get_global_function(EXEC_enter, 'v', ""));
					builder->CreateCall(get_global_function(EXEC_jit_execute_function, 'v', ""));
					break;
				
				case FUNCTION_PUBLIC: {
					//builder->CreateStore(getInteger(8, false), get_global((void*)&EXEC.native, llvmType(getInt8Ty)));
					/*builder->CreateStore(getInteger(32, index), get_global((void*)&EXEC.index, llvmType(getInt32Ty)));
					builder->CreateStore(get_global((void*)&desc->method, llvmType(getInt8Ty)), get_global((void*)&EXEC.desc));
					builder->CreateStore(object, get_global((void*)&EXEC.object));
					builder->CreateStore(get_global((void*)klass, llvmType(getInt8Ty)), get_global((void*)&EXEC.klass));*/
					PushPureObjectExpression* ppoe = (PushPureObjectExpression*)func;
					
					if (can_quick)
						builder->CreateCall3(get_global_function(JR_exec_enter_quick, 'v', "ppi"), ppoe->effective_class, object, getInteger(32, index));
					else
						builder->CreateCall3(get_global_function(JR_exec_enter, 'v', "ppi"), ppoe->effective_class, object, getInteger(32, index));
					break;
				}
				case FUNCTION_EVENT:
					builder->CreateCall3(get_global_function_vararg(GB_Raise, 'i', "pii"),
						current_op, getInteger(32, index + (CP->parent ? CP->parent->n_event : 0)), getInteger(32, -args.size()));
					break;
				
				case FUNCTION_UNKNOWN:
				case FUNCTION_CALL:
				case FUNCTION_EXTERN:
				case FUNCTION_SUBR:
					abort();
			}
		} else {
			llvm::Value* kind = extract_value(func_value, 2);
			gen_if_else(builder->CreateICmpEQ(kind, getInteger(8, FUNCTION_SUBR)), [&](){
				//Subr
				//FIXME
			}, [&](){
				PushPureObjectExpression* ppoe = (PushPureObjectExpression*)func;
				
				
				gen_if_else(builder->CreateICmpEQ(kind, getInteger(8, FUNCTION_PUBLIC)), [&](){
					//Public
					if (can_quick)
						builder->CreateCall3(get_global_function(JR_exec_enter_quick, 'v', "ppi"), ppoe->effective_class, object, getInteger(32, index));
					else
						builder->CreateCall3(get_global_function(JR_exec_enter, 'v', "ppi"), ppoe->effective_class, object, getInteger(32, index));
					//builder->CreateCall(get_global_function(EXEC_jit_execute_function, 'v', ""));
				}, [&](){
					//Native
					builder->CreateStore(getInteger(32, index), get_global((void*)&EXEC.index, llvmType(getInt32Ty)));
					builder->CreateStore(get_global((void*)&desc->method, llvmType(getInt8Ty)), get_global((void*)&EXEC.desc));
					builder->CreateStore(object, get_global((void*)&EXEC.object));
					builder->CreateStore(ppoe->effective_class, get_global((void*)&EXEC.klass));
					
					builder->CreateStore(getInteger(8, true), get_global((void*)&EXEC.use_stack, llvmType(getInt8Ty)));
					if (can_quick)
						builder->CreateCall(get_global_function(EXEC_native_quick, 'v', ""));
					else 
						builder->CreateCall(get_global_function(EXEC_native, 'v', ""));
				});
			});
		}
	} else {
		//FIXME call variant/object runtime
	}
#endif
}

llvm::Value* DupExpression::codegen_get_value(){
	if (on_stack || expr->on_stack) on_stack = expr->on_stack = true;
	duped->llvm_value = expr->codegen_get_value();
	borrow(duped->llvm_value, type);
	return duped->llvm_value;
}

void DupExpression::codegen_on_stack(){
	on_stack = expr->on_stack = true;
	expr->codegen_on_stack();
	duped->llvm_value = ret_top_stack(type, true);
	borrow(duped->llvm_value, type);
}

void OnGotoExpression::codegen(){
	JumpTablePendingBranch p;
	p.condition = condition->codegen_get_value();
	if (condition->on_stack)
		c_SP(-1);
	p.insert_point = builder->GetInsertBlock(); //Must be after condition codegen!
	p.destinations = &destinations;
	p.default_addr = default_addr;
	jump_table_pending_branches.push_back(p);
	builder->SetInsertPoint(create_bb("dummy"));
}

void JumpExpression::codegen(){
	PendingBranch p;
	p.condition = NULL;
	p.insert_point = builder->GetInsertBlock();
	p.true_addr = addr;
	pending_branches.push_back(p);
	builder->SetInsertPoint(create_bb("dummy"));
}

void GosubExpression::codegen(){
	llvm::Value* cond = NULL;
	if (!destinations.empty()){
		cond = condition->codegen_get_value();
		if (condition->on_stack)
			c_SP(-1);
	}
	
	gen_if_else(!cond ? getInteger(1, true) : builder->CreateICmpULT(cond, getInteger(32, destinations.size())), [&](){
		/*llvm::Value* index = builder->CreateLoad(temp_num_gosubs_on_stack);
		llvm::Value* indices[2] = {getInteger(TARGET_BITS, 0), to_target_int(index)};
		builder->CreateStore(llvm::BlockAddress::get(contpoint), builder->CreateGEP(temp_gosub_stack, indices));
		
		index = builder->CreateAdd(index, getInteger(32, 1));
		builder->CreateStore(index, temp_num_gosubs_on_stack);*/

#ifndef GOSUB_ON_STACK
		llvm::Value* gosub_stack_node = builder->CreateBitCast(builder->CreateCall(get_global_function(GB.Add, 'p', "p"), get_global(&GP, llvmType(getInt8Ty))), pointer_t(gosub_stack_node_type));
		
		/*builder->CreateStore(builder->CreateLoad(gosub_return_point), create_gep(gosub_stack_node, TARGET_BITS, 0, 32, 0));
		builder->CreateStore(llvm::BlockAddress::get(contpoint), gosub_return_point);*/
		unsigned int gosub_return_id = gosub_continue_points.size() + 1;
		builder->CreateStore(builder->CreateLoad(gosub_return_point), create_gep(gosub_stack_node, TARGET_BITS, 0, 32, 0));
		builder->CreateStore(getInteger(16, gosub_return_id), gosub_return_point);
#else
		llvm::Value* stack_addr = builder->CreateLoad(gp);
		unsigned int gosub_return_id = gosub_continue_points.size() + 1;
		store_value(stack_addr, builder->CreateLoad(gosub_return_point), T_SHORT);
		builder->CreateStore(getInteger(16, gosub_return_id), gosub_return_point);
		
		int diff = 1 + end_ctrl - FP->n_local;
		llvm::Value* new_gp = builder->CreateGEP(stack_addr, getInteger(TARGET_BITS, diff));
		gen_if_noreturn(builder->CreateICmpUGE(
			builder->CreateGEP(stack_addr, getInteger(TARGET_BITS, 1 + FP->stack_usage - FP->n_local + 8)),
			builder->CreateIntToPtr(getInteger(TARGET_BITS, (int64_t)(void*)STACK_limit), pointer_t(value_type))), [&](){
			
			create_throw(E_STACK);
		});
		builder->CreateStore(new_gp, gp);
		//c_SP(diff);
		builder->CreateStore(new_gp, get_global(&SP, pointer_t(value_type)));
#endif
		
		
		if (FP->n_ctrl){
#ifndef GOSUB_ON_STACK
			llvm::Value* gp_ctrl_addr = create_gep(gosub_stack_node, TARGET_BITS, 0, 32, 1);
			
			builder->CreateCall2(get_global_function(GB.Alloc, 'v', "pj"), builder->CreateBitCast(gp_ctrl_addr, llvmType(getInt8PtrTy)), getInteger(TARGET_BITS, sizeof(VALUE) * FP->n_ctrl));
			
			llvm::Value* gp_ctrl = builder->CreateLoad(gp_ctrl_addr);
#else
			llvm::Value* gp_ctrl = builder->CreateGEP(stack_addr, getInteger(TARGET_BITS, 1));
#endif
			
			
			for(int i=FP->n_local; i<end_ctrl; i++){
				store_value(i == FP->n_local ? gp_ctrl : builder->CreateGEP(gp_ctrl, getInteger(TARGET_BITS, i - FP->n_local)), builder->CreateLoad(locals[i]), get_ctrl_type(i));
				int spec_type = special_ctrl_type(get_ctrl_type(i));
				if (spec_type != 0){
					llvm::Value* stack_addr = builder->CreateGEP(read_global(&BP), getInteger(TARGET_BITS, i*sizeof(VALUE)));
					store_value(stack_addr, NULL, T_VOID);
				}
				builder->CreateStore(getInteger(32, 0), current_ctrl_types[i - FP->n_local]);
			}
			for(int i=end_ctrl; i<FP->n_local+FP->n_ctrl; i++){
				store_value(builder->CreateGEP(gp_ctrl, getInteger(TARGET_BITS, i - FP->n_local)), NULL, T_VOID);
				
				llvm::Value* old_type = release_ctrl(i);
				builder->CreateStore(getInteger(32, 0), current_ctrl_types[i - FP->n_local]);
				gen_if(builder->CreateICmpNE(old_type, getInteger(32, 0)), [&](){
					llvm::Value* stack_addr = builder->CreateGEP(read_global(&BP), getInteger(TARGET_BITS, i*sizeof(VALUE)));
					store_value(stack_addr, NULL, T_VOID);
				}, "old_ctrl_needs_to_be_cleaned4");
			}
		}
	}, [&](){
		if (cond){
			PendingBranch p;
			p.condition = NULL;
			p.insert_point = builder->GetInsertBlock();
			p.true_addr = default_addr;
			pending_branches.push_back(p);
			builder->SetInsertPoint(create_bb("dummy"));
		}
	}, "gosub_should_run");
	
	if (!cond){
		PendingBranch p;
		p.condition = NULL;
		p.insert_point = builder->GetInsertBlock();
		p.true_addr = gosubaddr;
		pending_branches.push_back(p);
	} else {
		JumpTablePendingBranch p;
		p.condition = cond;
		p.insert_point = builder->GetInsertBlock();
		p.destinations = &destinations;
		p.default_addr = destinations.back();
		jump_table_pending_branches.push_back(p);
	}
	
	llvm::BasicBlock* contpoint = create_bb("gosub_continue_point");
	gosub_continue_points.push_back(contpoint);
	
	builder->SetInsertPoint(contpoint);
	
	//On return:
#ifndef GOSUB_ON_STACK
	llvm::Value* gp = read_global(&GP, pointer_t(gosub_stack_node_type));
	llvm::Value* gp_array = builder->CreateBitCast(gp, llvmType(getInt32PtrTy));
	llvm::Value* count_addr = builder->CreateGEP(gp_array, getInteger(TARGET_BITS, -4)); //((int*)GP)[-4] == ARRAY_count(GP)
	llvm::Value* index = builder->CreateAdd(builder->CreateLoad(count_addr), getInteger(32, -1));
	llvm::Value* gosub_stack_node = builder->CreateGEP(gp, to_target_int(index));
	builder->CreateStore(index, count_addr);
	
	llvm::Value* old_return_point = builder->CreateLoad(create_gep(gosub_stack_node, TARGET_BITS, 0, 32, 0));
	builder->CreateStore(old_return_point, gosub_return_point);
#else
	
	int diff = 1 + end_ctrl - FP->n_local;
	llvm::Value* new_gp = builder->CreateGEP(builder->CreateLoad(gp), getInteger(TARGET_BITS, -diff));
	builder->CreateStore(new_gp, gp);
	//c_SP(-diff)
	builder->CreateStore(new_gp, get_global(&SP, pointer_t(value_type)));
	
	llvm::Value* old_return_point = read_value(new_gp, T_SHORT);
	builder->CreateStore(old_return_point, gosub_return_point);
#endif
	
		
	if (FP->n_ctrl){
#ifndef GOSUB_ON_STACK
		llvm::Value* gp_ctrl_addr = create_gep(gosub_stack_node, TARGET_BITS, 0, 32, 1);
		llvm::Value* gp_ctrl = builder->CreateLoad(gp_ctrl_addr);
#else
		llvm::Value* gp_ctrl = builder->CreateGEP(new_gp, getInteger(TARGET_BITS, 1));
#endif
		
		for(int i=FP->n_local; i<end_ctrl; i++){
			llvm::Value* val = read_value(i == FP->n_local ? gp_ctrl : builder->CreateGEP(gp_ctrl, getInteger(TARGET_BITS, i - FP->n_local)), get_ctrl_type(i));
			builder->CreateStore(val, locals[i]);
			builder->CreateStore(getInteger(32, special_ctrl_type(get_ctrl_type(i))), current_ctrl_types[i - FP->n_local]);
			
			if (special_ctrl_type(get_ctrl_type(i)) != 0){
				llvm::Value* stack_addr = builder->CreateGEP(read_global(&BP), getInteger(TARGET_BITS, i*sizeof(VALUE)));
				store_value(stack_addr, val, get_ctrl_type(i));
			}
		}
		for(int i=end_ctrl; i<FP->n_local+FP->n_ctrl; i++){
			builder->CreateStore(getInteger(32, 0), current_ctrl_types[i - FP->n_local]);
		}

#ifndef GOSUB_ON_STACK
		builder->CreateCall(get_global_function(GB.Free, 'v', "p"), builder->CreateBitCast(gp_ctrl_addr, llvmType(getInt8PtrTy)));
#endif
	}
	
}

void JumpIfExpression::codegen(){
	PendingBranch p;
	p.condition = val->codegen_get_value();
	if (val->on_stack)
		c_SP(-1);
	p.insert_point = builder->GetInsertBlock(); //Must be after condition codegen!
	p.true_addr = jump_if_true ? jump_addr : next_addr;
	p.false_addr = jump_if_true ? next_addr : jump_addr;
	pending_branches.push_back(p);
	builder->SetInsertPoint(create_bb("dummy"));
}

void JumpFirstExpression::codegen(){
	llvm::Value* to_val = to->codegen_get_value();
	if (to->on_stack)
		c_SP(-1);
	llvm::Value* step_val = step->codegen_get_value();
	if (step->on_stack)
		c_SP(-1);
	
	//builder->CreateStore(to_val, locals[ctrl_to]);
	//builder->CreateStore(step_val, locals[ctrl_to+1]);
	set_ctrl(to_val, to->type, ctrl_to);
	set_ctrl(step_val, step->type, ctrl_to+1);
	llvm::Value* ival = builder->CreateLoad(locals[local_var]);
	
	//If step is 0, don't execute the loop
	llvm::Value* condition;
	if (step->type == T_SINGLE)
		condition = builder->CreateFCmpUEQ(step_val, getFloat(0.0f));
	else if (step->type == T_FLOAT)
		condition = builder->CreateFCmpUEQ(step_val, getFloat(0.0));
	else {
		static const int bits[] = {0, 1, 8, 16, 32, 64};
		condition = builder->CreateICmpEQ(step_val, getInteger(bits[step->type], 0));
	}
	gen_if_noreturn(condition, [&](){
		PendingBranch p;
		p.insert_point = builder->GetInsertBlock();
		p.condition = NULL;
		p.true_addr = done_addr;
		pending_branches.push_back(p);
	}, "step_is_zero", "step_not_zero");
	
	TYPE to_type = to->type;
	TYPE step_type = step->type;
	
	//Pre-test:
	bool is_float = to_type >= T_SINGLE;
	llvm::Value *is_positive, *cont_neg, *cont_pos;
	if (is_float){
		is_positive = builder->CreateFCmpUGE(step_val, to_type == T_SINGLE ? getFloat(0.0f) : getFloat(0.0));
		cont_neg = builder->CreateFCmpUGE(ival, to_val);
		cont_pos = builder->CreateFCmpULE(ival, to_val);
	} else {
		static const int bits[] = {0, 1, 8, 16, 32, 64};
		is_positive = builder->CreateICmpSGE(step_val, getInteger(bits[step_type], 0));
		if (to_type == T_BYTE){
			cont_neg = builder->CreateICmpUGE(ival, to_val);
			cont_pos = builder->CreateICmpULE(ival, to_val);
		} else {
			cont_neg = builder->CreateICmpSGE(ival, to_val);
			cont_pos = builder->CreateICmpSLE(ival, to_val);
		}
	}
	llvm::Value* cont = builder->CreateSelect(is_positive, cont_pos, cont_neg);
	
	PendingBranch p;
	p.insert_point = builder->GetInsertBlock();
	p.condition = cont;
	p.true_addr = body_addr;
	p.false_addr = done_addr;
	pending_branches.push_back(p);
	builder->SetInsertPoint(create_bb("dummy"));
}

void JumpNextExpression::codegen(){
	llvm::Value* to_val = builder->CreateLoad(locals[ctrl_to]);
	llvm::Value* step_val = builder->CreateLoad(locals[ctrl_to+1]);
	llvm::Value* ival = builder->CreateLoad(locals[local_var]);
	
	TYPE to_type = get_ctrl_type(ctrl_to);
	TYPE step_type = get_ctrl_type(ctrl_to+1);
	
	//Add step
	if (step_type != to_type){
		if (to_type == T_BYTE)
			ival = builder->CreateZExt(ival, llvmType(getInt32Ty));
		else
			ival = builder->CreateSExt(ival, llvmType(getInt32Ty));
		ival = builder->CreateAdd(ival, step_val, "", false, true /*nsw*/);
		ival = builder->CreateTrunc(ival, to_val->getType());
		builder->CreateStore(ival, locals[local_var]);
	} else {
		if (to_type == T_SINGLE || to_type == T_FLOAT)
			ival = builder->CreateFAdd(ival, step_val);
		else
			ival = builder->CreateAdd(ival, step_val, "", false, true /*nsw*/);
		builder->CreateStore(ival, locals[local_var]);
	}
	
	//Test if continue
	bool is_float = to_type >= T_SINGLE;
	llvm::Value *is_positive, *cont_neg, *cont_pos;
	if (is_float){
		is_positive = builder->CreateFCmpUGE(step_val, to_type == T_SINGLE ? getFloat(0.0f) : getFloat(0.0));
		cont_neg = builder->CreateFCmpUGE(ival, to_val);
		cont_pos = builder->CreateFCmpULE(ival, to_val);
	} else {
		static const int bits[] = {0, 1, 8, 16, 32, 64};
		is_positive = builder->CreateICmpSGE(step_val, getInteger(bits[step_type], 0));
		if (to_type == T_BYTE){
			cont_neg = builder->CreateICmpUGE(ival, to_val);
			cont_pos = builder->CreateICmpULE(ival, to_val);
		} else {
			cont_neg = builder->CreateICmpSGE(ival, to_val);
			cont_pos = builder->CreateICmpSLE(ival, to_val);
		}
	}
	llvm::Value* cont = builder->CreateSelect(is_positive, cont_pos, cont_neg);
	
	PendingBranch p;
	p.insert_point = builder->GetInsertBlock();
	p.condition = cont;
	p.true_addr = body_addr;
	p.false_addr = done_addr;
	pending_branches.push_back(p);
	builder->SetInsertPoint(create_bb("dummy"));
}

void JumpEnumFirstExpression::codegen(){
	//FIXME fix class enum
	if (!TYPE_is_pure_object(obj->type)){
		codegen_pop_ctrl(obj, ctrl);
		builder->CreateCall(get_global_function_jif(EXEC_enum_first, 'v', "h"), getInteger(16, ctrl));
		set_ctrl_type(T_OBJECT, ctrl+1);
	} else {
		llvm::Value* object = obj->codegen_get_value();
		ob = extract_value(object, 1);
		
		codegen_pop_ctrl(object, obj, ctrl);
		
		/* Instance variables:
		llvm::Value* effective_class;
		llvm::Value* obj;
		*/
		llvm::Value* enum_object;
		
		CLASS* klass = (CLASS*)(void*)obj->type;
		if (klass->is_virtual){
			effective_class = builder->CreateIntToPtr(getInteger(TARGET_BITS, obj->type), llvmType(getInt8PtrTy));
			//Ok, so what we should pass to CENUM_create is the pointer to the class (in case of a virtual class is enumerated), or
			//pointer to the object (in case of an instance of a virtual class is enumerated).
			//As val._class.class is the same memory as val._object.object, we don't need to do anything special here.
			enum_object = ob;
			//However, when running EXEC_special with SPEC_FIRST, we need to pass null as object (if virtual class), so we replace ob with null
			ob = builder->CreateSelect(builder->CreateICmpEQ(builder->CreatePtrToInt(extract_value(object, 0), llvmType(getInt32Ty)), getInteger(32, T_CLASS)), get_nullptr(), ob);
			
			//old: enum_object = builder->CreateSelect(builder->CreateICmpEQ(ob, get_nullptr()), effective_class, ob);
		} else {
			effective_class = extract_value(object, 0);
			make_nullcheck(ob);
			enum_object = ob;
		}
		
		llvm::Value* cenum_obj = builder->CreateCall(get_global_function_jif(CENUM_create, 'p', "p"), enum_object);
		
		//unref_object(extract_value(builder->CreateLoad(locals[ctrl+1]), 1));
		borrow_object_no_nullcheck(cenum_obj);
		
		llvm::Value* cenum_object = get_new_struct(object_type,
			get_global((void*)GB.FindClass("Enum"), llvmType(getInt8Ty)), cenum_obj);
		
		set_ctrl(cenum_object, T_OBJECT, ctrl+1);
		
		builder->CreateStore(read_global((void*)&EXEC_enum), temp_voidptr);
		builder->CreateStore(cenum_obj, get_global((void*)&EXEC_enum));
		builder->CreateCall5(get_global_function_jif(EXEC_special, 'c', "ippic"),
			getInteger(32, SPEC_FIRST),
			effective_class,
			ob,
			getInteger(32, 0),
			getInteger(8, true));
		builder->CreateStore(builder->CreateLoad(temp_voidptr), get_global((void*)&EXEC_enum));
	}
}

llvm::Value* OnStackExpression::codegen_get_value(){
	return get_top(type);
}

void JumpEnumNextExpression::codegen(){
	//FIXME class enum
	llvm::Value* stop;
	if (!TYPE_is_pure_object(jfirst->obj->type)){
		store_pc(pc);
		stop = builder->CreateICmpNE(builder->CreateCall(get_global_function_jif(EXEC_enum_next, 'c', "h"), getInteger(16, drop)), getInteger(8, false));
	} else {
		llvm::Value* cenum_obj = extract_value(builder->CreateLoad(locals[jfirst->ctrl+1]), 1);
		int stop_offset = sizeof(GB_BASE) + sizeof(LIST) + 5*sizeof(void*);
		stop = builder->CreateTrunc(builder->CreateLoad(builder->CreateGEP(cenum_obj, getInteger(TARGET_BITS, stop_offset))), llvmType(getInt1Ty));
		
		stop = gen_if_phi(stop, builder->CreateXor(stop, getInteger(1, 1)), [&](){
			builder->CreateStore(read_global((void*)&EXEC_enum), temp_voidptr);
			builder->CreateStore(cenum_obj, get_global((void*)&EXEC_enum));
			store_pc(pc); //Used by EXEC_native
			llvm::Value* err = builder->CreateCall5(get_global_function_jif(EXEC_special, 'c', "ippic"),
				getInteger(32, SPEC_NEXT),
				jfirst->effective_class,
				jfirst->ob,
				getInteger(32, 0),
				getInteger(8, false));
			builder->CreateStore(builder->CreateLoad(temp_voidptr), get_global((void*)&EXEC_enum));
			
			gen_if_noreturn(builder->CreateICmpNE(err, getInteger(8, false)), [&](){
				create_throw(E_ENUM);
			});
			
			llvm::Value* stop_after = builder->CreateTrunc(builder->CreateLoad(builder->CreateGEP(cenum_obj, getInteger(TARGET_BITS, stop_offset))), llvmType(getInt1Ty));
			gen_if(builder->CreateOr(getInteger(1, drop), stop_after), [&](){
				//When enumeration stops, T_VOID can be returned instead of the given type
				llvm::Value* t = get_top(type);
				llvm::Value* tt = load_element(get_value_on_top_addr(), 0);
				if (TYPE_is_object(type))
					tt = builder->CreatePtrToInt(tt, LONG_TYPE);
				gen_if(builder->CreateICmpNE(tt, getInteger(TARGET_BITS, T_VOID)), [&](){
					release(t, type);
				});
				c_SP(-1);
			});
			return stop_after;
		}, "enum_next_cont");
	}
	
	PendingBranch p;
	p.condition = stop;
	p.insert_point = builder->GetInsertBlock();
	p.true_addr = addr;
	p.false_addr = cont_addr;
	pending_branches.push_back(p);
	builder->SetInsertPoint(create_bb("dummy"));
}

void TryExpression::codegen(){
	in_try = true;
	has_tries = true;
	
	try_blocks.push_back(builder->GetInsertBlock());
	
	builder->CreateStore(read_sp(), get_global((void*)&EP, pointer_t(value_type)));
	//We only want it to be non-null, so let's put in 1:
	builder->CreateStore(get_global((void*)1, llvmType(getInt8Ty)), get_global((void*)&EC));
		
	llvm::Value* jmpbuf = builder->CreateCall(get_global_function(JR_try, 'p', "p"),
		create_gep(temp_errcontext1, TARGET_BITS, 0, TARGET_BITS, 0));
	
	llvm::Function* f = llvm::cast<llvm::Function>(get_global_function(_setjmp, 'i', "p"));
#if LLVM_VERSION_MAJOR > 3 || (LLVM_VERSION_MAJOR == 3 && LLVM_VERSION_MINOR == 2)
	f->addFnAttr(llvm::Attributes::ReturnsTwice);
#else
	f->addFnAttr(llvm::Attribute::ReturnsTwice);
#endif
	
	llvm::Value* setjmp_return = builder->CreateCall(f, jmpbuf);
	
	builder->CreateStore(builder->CreateTrunc(setjmp_return, llvmType(getInt8Ty)),
		create_gep(temp_errcontext1, TARGET_BITS, 0, TARGET_BITS, offsetof(ERROR_CONTEXT, ret)));
	
	llvm::Value* second_time = builder->CreateICmpNE(setjmp_return, getInteger(32, 0));
	
	gen_if_else(second_time, [&](){
		builder->CreateCall(get_global_function(JR_try_unwind, 'v', "p"), builder->CreateBitCast(builder->CreateLoad(gp), llvmType(getInt8PtrTy)));
		builder->CreateStore(getInteger(1, true), temp_got_error);
	}, [&](){
		builder->CreateStore(getInteger(1, false), temp_got_error);
	}, "Try_cleanup");
	
	PendingBranch p;
	p.condition = second_time;
	p.insert_point = builder->GetInsertBlock();
	p.true_addr = addr2;
	p.false_addr = addr1;
	pending_branches.push_back(p);
	builder->SetInsertPoint(create_bb("dummy"));
}

void EndTryExpression::codegen(){
	in_try = false;
	
	llvm::Value* call = builder->CreateCall(get_global_function(JR_end_try, 'v', "p"),
		create_gep(temp_errcontext1, TARGET_BITS, 0, TARGET_BITS, 0));
		
	if (llvm::Instruction* inst = llvm::dyn_cast<llvm::Instruction>(call)){
		llvm::Value* arr[1] = {getInteger(32, 1)};
		inst->setMetadata("end_try", llvm::MDNode::get(llvm_context, arr));
	}
		
	builder->CreateStore(get_nullptr(), get_global((void*)&EP));
	//We only need to store NULL (no catch) or something whatever else inside EC (there is a catch)
	llvm::Value* got_large_error = builder->CreateZExt(builder->CreateXor(builder->CreateLoad(temp_got_error2), getInteger(1, 1)), LONG_TYPE);
	builder->CreateStore(got_large_error, get_global((void*)&EC, LONG_TYPE));
}

void LargeTryExpression::codegen(){
	has_tries = true;
	
	builder->CreateStore(getInteger(1, false), temp_got_error2);
	
	llvm::Value* jmpbuf = builder->CreateCall(get_global_function(JR_try, 'p', "p"),
		create_gep(temp_errcontext2, TARGET_BITS, 0, TARGET_BITS, 0));
	
	llvm::Function* f = llvm::cast<llvm::Function>(get_global_function(_setjmp, 'i', "p"));
#if LLVM_VERSION_MAJOR > 3 || (LLVM_VERSION_MAJOR == 3 && LLVM_VERSION_MINOR == 2)
	f->addFnAttr(llvm::Attributes::ReturnsTwice);
#else
	f->addFnAttr(llvm::Attribute::ReturnsTwice);
#endif
	
	llvm::Value* setjmp_return = builder->CreateCall(f, jmpbuf);
	
	builder->CreateStore(builder->CreateTrunc(setjmp_return, llvmType(getInt8Ty)),
		create_gep(temp_errcontext2, TARGET_BITS, 0, TARGET_BITS, offsetof(ERROR_CONTEXT, ret)));
	
	llvm::Value* second_time = builder->CreateICmpNE(setjmp_return, getInteger(32, 0));
	
	gen_if(second_time, [&](){
		builder->CreateCall(get_global_function(JR_try_unwind, 'v', "p"), builder->CreateBitCast(builder->CreateLoad(gp), llvmType(getInt8PtrTy)));
		builder->CreateCall(get_global_function(JR_end_try, 'v', "p"),
			create_gep(temp_errcontext2, TARGET_BITS, 0, TARGET_BITS, 0));
		builder->CreateStore(get_nullptr(), get_global((void*)&EC));
		builder->CreateStore(getInteger(1, true), temp_got_error2);
	}, "Try_cleanup");
	
	PendingBranch p;
	p.condition = second_time;
	p.insert_point = builder->GetInsertBlock();
	p.true_addr = addr2;
	p.false_addr = 0;
	pending_branches.push_back(p);
	builder->SetInsertPoint(create_bb("dummy"));
}

void LargeCatchExpression::codegen(){
	//Also in ReturnExpression if returning before this
	builder->CreateCall(get_global_function(JR_end_try, 'v', "p"),
		create_gep(temp_errcontext2, TARGET_BITS, 0, TARGET_BITS, 0));
	//Differs from original interpreter, error thrown in a Finally block is not catched.
	builder->CreateStore(get_nullptr(), get_global((void*)&EC));
}

void CatchExpression::codegen(){
	llvm::Value* got_error = builder->CreateLoad(temp_got_error2);
	
	gen_if_noreturn(builder->CreateXor(got_error, getInteger(1, 1)), [&](){
		store_value(get_global((void*)RP), get_default(FP->type), FP->type);
		return_points.push_back(builder->GetInsertBlock());
	}, "do_not_catch", "do_catch");
}

llvm::Value* NewExpression::codegen_get_value(){
	codegen_on_stack();
	return ret_top_stack(type, true);
}
void NewExpression::codegen_on_stack(){
	for(size_t i=0, e=args.size(); i!=e; i++){
		args[i]->codegen_on_stack();
	}
	store_pc(pc);
	builder->CreateCall(get_global_function_jif(EXEC_new, 'v', ""));
}

std::pair<llvm::Value*, llvm::Value*> BinOpExpression::codegen_operands(){
	llvm::Value* l = left->codegen_get_value();
	if (left->on_stack)
		c_SP(-1);
	llvm::Value* r = right->codegen_get_value();
	if (right->on_stack)
		c_SP(-1);
	return std::make_pair(l, r);
}

llvm::Value* EqExpression::codegen_get_value(){
	llvm::Value* ret;
	if ((t >= T_BOOLEAN && t <= T_LONG) || t == T_POINTER){
		auto op = codegen_operands();
		ret = builder->CreateICmpEQ(op.first, op.second);
	} else if (t == T_DATE){
		auto op = codegen_operands();
		ret = builder->CreateAnd(
			builder->CreateICmpEQ(extract_value(op.first, 0), extract_value(op.second, 0)),
			builder->CreateICmpEQ(extract_value(op.first, 1), extract_value(op.second, 1))
		);
	} else if (t == T_NULL){
		//FIXME gör några unit tests på den här biten
		if (left->type == T_NULL && right->type == T_NULL){
			ret = getInteger(1, true);
		} else {
			Expression* testexpr = left->type == T_NULL ? right : left;
			llvm::Value* val = testexpr->codegen_get_value();
			switch(testexpr->type){
				case T_STRING:
				case T_CSTRING:
					ret = builder->CreateICmpEQ(extract_value(val, 3), getInteger(32, 0));
					if (testexpr->type == T_STRING)
						release(val, T_STRING);
					break;
				case T_DATE:
					ret = builder->CreateICmpEQ(builder->CreateOr(extract_value(val, 0), extract_value(val, 1)), getInteger(32, 0));
					break;
				case T_POINTER:
					ret = builder->CreateICmpEQ(val, get_nullptr());
					break;
				case T_VARIANT: {
					llvm::Value* vtype = extract_value(val, 0);
					ret = gen_if_phi(getInteger(1, true), builder->CreateICmpNE(vtype, getInteger(TARGET_BITS, T_NULL)), [&](){
						if (TARGET_BITS == 64){
							llvm::Value* date_str_ptr_obj = builder->CreateOr(
								builder->CreateAnd(
									builder->CreateICmpUGE(vtype, getInteger(TARGET_BITS, T_DATE)),
									builder->CreateICmpULE(vtype, getInteger(TARGET_BITS, T_POINTER))
								),
								builder->CreateICmpUGE(vtype, getInteger(TARGET_BITS, T_OBJECT))
							);
							llvm::Value* zero_data = builder->CreateICmpEQ(extract_value(val, 1), getInteger(64, 0));
							return builder->CreateSelect(date_str_ptr_obj, zero_data, getInteger(1, false));
						} else {
							llvm::Value* vdata = extract_value(val, 1);
							llvm::Value* ptr_is_null = builder->CreateICmpEQ(builder->CreateTrunc(vdata, llvmType(getInt32Ty)), getInteger(32, 0));
							return (llvm::Value*)gen_if_phi(ptr_is_null, builder->CreateICmpNE(vtype, getInteger(TARGET_BITS, T_STRING)), [&](){
								llvm::Value* zero_date = builder->CreateICmpEQ(vdata, getInteger(64, 0));
								return gen_if_phi(zero_date, builder->CreateICmpNE(vtype, getInteger(TARGET_BITS, T_DATE)), [&](){
									return builder->CreateSelect(
										builder->CreateOr(
											builder->CreateICmpEQ(vtype, getInteger(TARGET_BITS, T_POINTER)),
											builder->CreateICmpUGE(vtype, getInteger(TARGET_BITS, T_OBJECT))
										),
										ptr_is_null, getInteger(1, false)
									);
								});
							});
						}
					});
					gen_if(builder->CreateXor(ret, getInteger(1, 1)), [&](){
						release(val, T_VARIANT);
					});
					break;
				}
				default:
					if (TYPE_is_object(testexpr->type)){
						llvm::Value* object = extract_value(val, 1);
						ret = builder->CreateICmpEQ(object, get_nullptr());
						unref_object(object);
					} else {
						ret = getInteger(1, false);
					}
					break;
			}
			if (left->on_stack)
				c_SP(-1);
			if (right->on_stack)
				c_SP(-1);
		}
	} else if (t == T_STRING || t == T_CSTRING){
		auto op = codegen_operands();
		llvm::Value* len1 = extract_value(op.first, 3);
		llvm::Value* len2 = extract_value(op.second, 3);
		
		llvm::Value *ret1, *ret2;
		llvm::BasicBlock *BB1, *BB2;
		
		gen_if_else(builder->CreateICmpNE(len1, len2), [&](){
			ret1 = getInteger(1, false);
			BB1 = builder->GetInsertBlock();
		}, [&](){
			llvm::Value* addr1 = extract_value(op.first, 1);
			llvm::Value* addr2 = extract_value(op.second, 1);
			llvm::Value* offset1 = extract_value(op.first, 2);
			llvm::Value* offset2 = extract_value(op.second, 2);
			if (TARGET_BITS == 64){
				offset1 = builder->CreateZExt(offset1, llvmType(getInt64Ty));
				offset2 = builder->CreateZExt(offset2, llvmType(getInt64Ty));
				len1 = builder->CreateZExt(len1, llvmType(getInt64Ty));
			}
			addr1 = builder->CreateGEP(addr1, offset1);
			addr2 = builder->CreateGEP(addr2, offset2);
			ret2 = builder->CreateICmpEQ(builder->CreateCall3(get_global_function(memcmp, 'i', "ppj"), addr1, addr2, len1), getInteger(32, 0));
			BB2 = builder->GetInsertBlock();
		}, "strcomp_not_same_length", "strcomp_same_length", "strcomp_done");
		
		llvm::PHINode* phi = builder->CreatePHI(llvmType(getInt1Ty), 2);
		phi->addIncoming(ret1, BB1);
		phi->addIncoming(ret2, BB2);
		ret = phi;
		
		release(op.first, left->type);
		release(op.second, right->type);
	} else if (t == T_SINGLE || t == T_FLOAT){
		auto op = codegen_operands();
		ret = builder->CreateFCmpUEQ(op.first, op.second);
	} else if (t == T_OBJECT){
		llvm::Value* l = left->codegen_get_value();
		llvm::Value* r = right->codegen_get_value();
		llvm::Value* sp = read_sp();//builder->CreateBitCast(read_sp(), pointer_t(value_types[T_OBJECT]));
		llvm::Value* laddr = builder->CreateGEP(sp, getInteger(TARGET_BITS, -2));//create_gep(sp, TARGET_BITS, -2, 32, 1);
		llvm::Value* raddr = builder->CreateGEP(sp, getInteger(TARGET_BITS, -1));//create_gep(sp, TARGET_BITS, -1, 32, 1);
		laddr = create_gep(builder->CreateBitCast(laddr, pointer_t(value_types[T_OBJECT])), TARGET_BITS, 0, 32, 1);
		raddr = create_gep(builder->CreateBitCast(raddr, pointer_t(value_types[T_OBJECT])), TARGET_BITS, 0, 32, 1);
		
		llvm::Value* ret = builder->CreateCall2(get_global_function_jif(COMPARE_object, 'i', "pp"),
			builder->CreateBitCast(laddr, llvmType(getInt8PtrTy)),
			builder->CreateBitCast(raddr, llvmType(getInt8PtrTy)));
		ret = builder->CreateICmpEQ(ret, getInteger(32, 0));
		
		unref_object(extract_value(l, 1));
		unref_object(extract_value(r, 1));
		
		c_SP(-2);
		
		if (on_stack)
			push_value(ret, T_BOOLEAN);
		return ret;
	} else if (t == T_VARIANT){
		left->codegen_on_stack();
		right->codegen_on_stack();
		builder->CreateCall(get_global_function(JR_variant_equal, 'v', ""));
		return ret_top_stack(T_BOOLEAN, on_stack);
	}
	if (on_stack)
		push_value(ret, T_BOOLEAN);
	return ret;
}

static llvm::Value* is_empty_string(llvm::Value* str){
	//Null test of address should not be needed. Length check should be enough...
	llvm::Value* len = extract_value(str, 3);
	return builder->CreateICmpEQ(len, getInteger(32, 0));
}

llvm::Value* NotExpression::codegen_get_value(){
	if (expr->type == T_NULL){
		if (on_stack)
			push_value(getInteger(1, true), T_BOOLEAN);
		return getInteger(1, true);
	} else if (TYPE_is_variant(expr->type)){
		expr->codegen_on_stack();
		builder->CreateCall(get_global_function_jif(SUBR_not, 'v', "h"), getInteger(16, 0/*T_VARIANT*/));
		return ret_top_stack(T_VARIANT, true);
	} else {
		llvm::Value* op = expr->codegen_get_value();
		if (expr->on_stack)
			c_SP(-1);
		llvm::Value* ret;
		if (expr->type <= T_LONG){
			static const int bits[] = {0, 1, 8, 16, 32, 64};
			ret = builder->CreateXor(op, getInteger(bits[expr->type], -1));
		} else if (TYPE_is_string(expr->type)){
			ret = is_empty_string(op);
			release(op, expr->type);
		} else if (TYPE_is_object(expr->type)){
			ret = builder->CreateICmpEQ(extract_value(op, 1), get_nullptr());
			release(op, expr->type);
		}
		if (on_stack)
			push_value(ret, T_BOOLEAN);
		return ret;
	}
}

static llvm::Value* LessDate(llvm::Value* date_left, llvm::Value* date_right){
	llvm::Value* d1 = extract_value(date_left, 0);
	llvm::Value* d2 = extract_value(date_right, 0);
	llvm::Value* t1 = extract_value(date_left, 1);
	llvm::Value* t2 = extract_value(date_right, 1);
	
	llvm::BasicBlock *BB1, *BB2, *BB3, *BB4;
	llvm::Value* r3;
	
	gen_if_else(builder->CreateICmpSLT(d1, d2), [&](){
		BB1 = builder->GetInsertBlock(); //true
	}, [&](){
		gen_if_else(builder->CreateICmpSGT(d1, d2), [&](){
			BB2 = builder->GetInsertBlock(); //false
		}, [&](){
			r3 = builder->CreateICmpSLT(t1, t2);
			BB3 = builder->GetInsertBlock(); //r3
		});
		llvm::PHINode* phi = builder->CreatePHI(llvmType(getInt1Ty), 2);
		phi->addIncoming(getInteger(1, false), BB2);
		phi->addIncoming(r3, BB3);
		r3 = phi;
		BB4 = builder->GetInsertBlock();
	});
	
	llvm::PHINode* phi = builder->CreatePHI(llvmType(getInt1Ty), 2);
	phi->addIncoming(getInteger(1, true), BB1);
	phi->addIncoming(r3, BB4);
	return phi;
}

llvm::Value* LessExpression::codegen_get_value(){
	if (t == T_VARIANT){
		left->codegen_on_stack();
		right->codegen_on_stack();
		builder->CreateCall(get_global_function(JR_variant_compi_less_than, 'v', ""));
		return ret_top_stack(T_BOOLEAN, true);
	} else {
		auto op = codegen_operands();
		llvm::Value* ret;
		if (t <= T_LONG || t == T_POINTER){
			ret = builder->CreateICmpSLT(op.first, op.second);
		} else if (t == T_DATE){
			ret = LessDate(op.first, op.second);
		} else if (t == T_SINGLE || t == T_FLOAT){
			ret = builder->CreateFCmpULT(op.first, op.second);
		} /*else if (TYPE_is_object(t)){
			//FIXME the interpreter does not allow this ..
			abort();
		} */else if (TYPE_is_string(t)){
			llvm::Value* addr1 = extract_value(op.first, 1);
			llvm::Value* addr2 = extract_value(op.second, 1);
			llvm::Value* offset1 = extract_value(op.first, 2);
			llvm::Value* offset2 = extract_value(op.second, 2);
			llvm::Value* len1 = extract_value(op.first, 3);
			llvm::Value* len2 = extract_value(op.second, 3);
			if (TARGET_BITS == 64){
				offset1 = builder->CreateZExt(offset1, llvmType(getInt64Ty));
				offset2 = builder->CreateZExt(offset2, llvmType(getInt64Ty));
			}
			addr1 = builder->CreateGEP(addr1, offset1);
			addr2 = builder->CreateGEP(addr2, offset2);
			ret = builder->CreateICmpEQ(builder->CreateCall4(get_global_function_jif(STRING_compare, 'i', "pipi"), addr1, len1, addr2, len2), getInteger(32, -1));
			release(op.first, left->type);
			release(op.second, right->type);
		}
		if (on_stack)
			push_value(ret, T_BOOLEAN);
		
		return ret;
	}
}

llvm::Value* NearExpression::codegen_get_value(){
	auto op = codegen_operands();
	llvm::Value* ret;
	
	llvm::Value* len1 = extract_value(op.first, 3);
	llvm::Value* len2 = extract_value(op.second, 3);
	
	llvm::Value *ret1, *ret2;
	llvm::BasicBlock *BB1, *BB2;
	
	gen_if_else(builder->CreateICmpNE(len1, len2), [&](){
		ret1 = getInteger(1, false);
		BB1 = builder->GetInsertBlock();
	}, [&](){
		llvm::Value* addr1 = extract_value(op.first, 1);
		llvm::Value* addr2 = extract_value(op.second, 1);
		llvm::Value* offset1 = extract_value(op.first, 2);
		llvm::Value* offset2 = extract_value(op.second, 2);
		if (TARGET_BITS == 64){
			offset1 = builder->CreateZExt(offset1, llvmType(getInt64Ty));
			offset2 = builder->CreateZExt(offset2, llvmType(getInt64Ty));
		}
		addr1 = builder->CreateGEP(addr1, offset1);
		addr2 = builder->CreateGEP(addr2, offset2);
		ret2 = builder->CreateICmpNE(builder->CreateCall3(get_global_function_jif(STRING_equal_ignore_case_same, 'c', "ppi"), addr1, addr2, len1), getInteger(8, 0));
		BB2 = builder->GetInsertBlock();
	}, "strcomp_not_same_length", "strcomp_same_length", "strcomp_done");
	
	llvm::PHINode* phi = builder->CreatePHI(llvmType(getInt1Ty), 2);
	phi->addIncoming(ret1, BB1);
	phi->addIncoming(ret2, BB2);
	ret = phi;
	
	release(op.first, left->type);
	release(op.second, right->type);
	
	if (on_stack)
		push_value(ret, T_BOOLEAN);
	
	return ret;
}

llvm::Value* AddExpression::codegen_get_value(){
	if (type == T_VARIANT){
		left->codegen_on_stack();
		right->codegen_on_stack();
		builder->CreateCall(get_global_function(JR_add, 'v', "h"), getInteger(16, 0));
		return ret_top_stack(T_VARIANT, true);
	} else {
		auto op = codegen_operands();
		llvm::Value* ret;
		
		if (type == T_BOOLEAN){
			ret = builder->CreateOr(op.first, op.second);
		} else if (type <= T_LONG || type == T_POINTER){
			ret = builder->CreateAdd(op.first, op.second);
		} else /*if (type <= T_FLOAT)*/{
			ret = builder->CreateFAdd(op.first, op.second);
		}
		if (on_stack)
			push_value(ret, type);
		return ret;
	}
}

llvm::Value* SubExpression::codegen_get_value(){
	if (type == T_VARIANT){
		left->codegen_on_stack();
		right->codegen_on_stack();
		builder->CreateCall(get_global_function(JR_sub, 'v', "h"), getInteger(16, 0));
		return ret_top_stack(T_VARIANT, true);
	} else {
		auto op = codegen_operands();
		llvm::Value* ret;
		
		if (type == T_BOOLEAN){
			ret = builder->CreateXor(op.first, op.second);
		} else if (type <= T_LONG || type == T_POINTER){
			ret = builder->CreateSub(op.first, op.second);
		} else /*if (type <= T_FLOAT)*/{
			ret = builder->CreateFSub(op.first, op.second);
		}
		if (on_stack)
			push_value(ret, type);
		return ret;
	}
}

llvm::Value* MulExpression::codegen_get_value(){
	if (type == T_VARIANT){
		left->codegen_on_stack();
		right->codegen_on_stack();
		builder->CreateCall(get_global_function(JR_mul, 'v', "h"), getInteger(16, 0));
		return ret_top_stack(T_VARIANT, true);
	} else {
		auto op = codegen_operands();
		llvm::Value* ret;
		
		if (type == T_BOOLEAN){
			ret = builder->CreateAnd(op.first, op.second);
		} else if (type <= T_LONG || type == T_POINTER){
			ret = builder->CreateMul(op.first, op.second);
		} else /*if (type <= T_FLOAT)*/{
			ret = builder->CreateFMul(op.first, op.second);
		}
		if (on_stack)
			push_value(ret, type);
		return ret;
	}
}

llvm::Value* DivExpression::codegen_get_value(){
	auto op = codegen_operands();
	
	gen_if_noreturn(builder->CreateFCmpUEQ(op.second, getFloat(0.0)), [&](){
		create_throw(E_ZERO);
	}, "div_zero", "not_div_zero");
	
	llvm::Value* ret = builder->CreateFDiv(op.first, op.second);

	if (on_stack)
		push_value(ret, type);
	return ret;
}

llvm::Value* QuoExpression::codegen_get_value(){
	auto op = codegen_operands();
	llvm::Value* ret;
	if (type == T_BOOLEAN){
		gen_if_noreturn(builder->CreateICmpEQ(op.second, getInteger(1, false)), [&](){
			create_throw(E_ZERO);
		}, "div_zero", "not_div_zero");
		ret = op.first;
	} else {
		static const int bits[] = {0, 1, 8, 16, 32, 64};
		gen_if_noreturn(builder->CreateICmpEQ(op.second, getInteger(bits[type], 0)), [&](){
			create_throw(E_ZERO);
		}, "div_zero", "not_div_zero");
		if (type == T_BYTE)
			ret = builder->CreateUDiv(op.first, op.second);
		else
			ret = builder->CreateSDiv(op.first, op.second);
	}
	if (on_stack)
		push_value(ret, type);
	return ret;
}

llvm::Value* RemExpression::codegen_get_value(){
	auto op = codegen_operands();
	llvm::Value* ret;
	if (type == T_BOOLEAN){
		gen_if_noreturn(builder->CreateICmpEQ(op.second, getInteger(1, false)), [&](){
			create_throw(E_ZERO);
		}, "div_zero", "not_div_zero");
		ret = op.first;
	} else {
		static const int bits[] = {0, 1, 8, 16, 32, 64};
		gen_if_noreturn(builder->CreateICmpEQ(op.second, getInteger(bits[type], 0)), [&](){
			create_throw(E_ZERO);
		}, "div_zero", "not_div_zero");
		if (type == T_BYTE)
			ret = builder->CreateURem(op.first, op.second);
		else
			ret = builder->CreateSRem(op.first, op.second);
	}
	if (on_stack)
		push_value(ret, type);
	return ret;
}

llvm::Value* PowExpression::codegen_get_value(){
	auto op = codegen_operands();
	llvm::Value* func;
	if (right->type == T_INTEGER){
		llvm::Type* types[] = {llvmType(getDoubleTy)};
		func = llvm::Intrinsic::getDeclaration(M, llvm::Intrinsic::powi, types);
		//func = get_global_function(__powidf2, 'd', "di");
		//func = M->getOrInsertFunction("llvm.powi.f64", get_function_type('d', "di"));
	} else {
		func = M->getOrInsertFunction("llvm.pow.f64", get_function_type('d', "dd"));
	}
	llvm::Value* ret = builder->CreateCall2(func, op.first, op.second);
	if (on_stack)
		push_value(ret, type);
	return ret;
}

llvm::Value* AndExpression::codegen_get_value(){
	if (type == T_VARIANT){
		left->codegen_on_stack();
		right->codegen_on_stack();
		builder->CreateCall(get_global_function_jif(SUBR_and_, 'v', "h"), getInteger(16, C_AND));
		return ret_top_stack(T_VARIANT, on_stack);
	} else {
		auto op = codegen_operands();
		llvm::Value* ret = builder->CreateAnd(op.first, op.second);
		if (on_stack)
			push_value(ret, type);
		return ret;
	}
}

llvm::Value* OrExpression::codegen_get_value(){
	if (type == T_VARIANT){
		left->codegen_on_stack();
		right->codegen_on_stack();
		builder->CreateCall(get_global_function_jif(SUBR_and_, 'v', "h"), getInteger(16, C_OR));
		return ret_top_stack(T_VARIANT, on_stack);
	} else {
		auto op = codegen_operands();
		llvm::Value* ret = builder->CreateOr(op.first, op.second);
		if (on_stack)
			push_value(ret, type);
		return ret;
	}
}

llvm::Value* XorExpression::codegen_get_value(){
	if (type == T_VARIANT){
		left->codegen_on_stack();
		right->codegen_on_stack();
		builder->CreateCall(get_global_function_jif(SUBR_and_, 'v', "h"), getInteger(16, C_XOR));
		return ret_top_stack(T_VARIANT, on_stack);
	} else {
		auto op = codegen_operands();
		llvm::Value* ret = builder->CreateXor(op.first, op.second);
		if (on_stack)
			push_value(ret, type);
		return ret;
	}
}

llvm::Value* IsExpression::codegen_get_value(){
	llvm::Value* obj = left->codegen_get_value();
	if (left->on_stack)
		c_SP(-1);
	
	llvm::Value* addr = extract_value(obj, 1);
	
	llvm::BasicBlock *BB1, *BB2, *BB3, *BB4;
	llvm::Value* ret;
	
	BB1 = builder->GetInsertBlock(); //false
	gen_if(builder->CreateICmpNE(addr, get_nullptr()), [&](){
		llvm::Value* OBJ = builder->CreateBitCast(addr, pointer_t(OBJECT_type));
		llvm::Value* obj_klass = load_element(OBJ, 0);
		
		PushClassExpression* pce = dyn_cast<PushClassExpression>(right);
		assert(pce);
		gen_if_else(builder->CreateICmpEQ(builder->CreateIntToPtr(getInteger(TARGET_BITS, (intptr_t)(void*)pce->klass), llvmType(getInt8PtrTy)), obj_klass), [&](){
			BB2 = builder->GetInsertBlock(); //true
		}, [&](){
			ret = builder->CreateCall2(get_global_function_jif(CLASS_inherits, 'c', "pp"), obj_klass, builder->CreateIntToPtr(getInteger(TARGET_BITS, (intptr_t)(void*)pce->klass), llvmType(getInt8PtrTy)));
			ret = builder->CreateICmpNE(ret, getInteger(8, false));
			BB3 = builder->GetInsertBlock();
		});
		llvm::PHINode* phi = builder->CreatePHI(llvmType(getInt1Ty), 2);
		phi->addIncoming(getInteger(1, true), BB2);
		phi->addIncoming(ret, BB3);
		ret = phi;
		
		unref_object(addr);
		
		BB4 = builder->GetInsertBlock();
	});
	llvm::PHINode* phi = builder->CreatePHI(llvmType(getInt1Ty), 2);
	phi->addIncoming(getInteger(1, false), BB1);
	phi->addIncoming(ret, BB4);
	ret = phi;
	
	if (on_stack)
		push_value(ret, type);
	return ret;
}

llvm::Value* NegExpression::codegen_get_value(){
	if (type == T_VARIANT){
		llvm::Value* val = expr->codegen_get_value();
		c_SP(-expr->on_stack);
		llvm::Value* vtype = extract_value(val, 0);
		llvm::Value* data = extract_value(val, 1);
		
		llvm::BasicBlock* bb[8] = {
			create_bb("else"),
			create_bb("bool"),
			create_bb("byte"),
			create_bb("short"),
			create_bb("integer"),
			create_bb("long"),
			create_bb("single"),
			create_bb("float")
		};
		llvm::BasicBlock* done_block = create_bb("done_neg");
		
		llvm::SwitchInst* si = builder->CreateSwitch(vtype, bb[0], 7);
		for(int i=1; i<8; i++)
			si->addCase(getInteger(TARGET_BITS, i), bb[i]);
		
		llvm::Value* res[8];
		
		builder->SetInsertPoint(bb[1]); //bool
		res[1] = data;
		builder->CreateBr(done_block);
		
		builder->SetInsertPoint(bb[2]); //byte
		res[2] = builder->CreateZExt(
			builder->CreateSub(getInteger(8, 0), builder->CreateTrunc(data, llvmType(getInt8Ty))),
			llvmType(getInt64Ty)
		);
		builder->CreateBr(done_block);
		
		builder->SetInsertPoint(bb[3]); //short
		res[3] = builder->CreateSExt(
			builder->CreateSub(getInteger(16, 0), builder->CreateTrunc(data, llvmType(getInt16Ty))),
			llvmType(getInt64Ty)
		);
		builder->CreateBr(done_block);
		
		builder->SetInsertPoint(bb[4]); //integer
		res[4] = builder->CreateSExt(
			builder->CreateSub(getInteger(32, 0), builder->CreateTrunc(data, llvmType(getInt32Ty))),
			llvmType(getInt64Ty)
		);
		builder->CreateBr(done_block);
		
		builder->SetInsertPoint(bb[5]); //long
		res[5] = builder->CreateSub(getInteger(64, 0), data);
		builder->CreateBr(done_block);
		
		/* %1 = trunc i64 %data to i32
		%2 = bitcast i32 %1 to float
		%3 = fsub float -0.000000e+00, %2
		%4 = bitcast float %3 to i32
		%5 = zext i32 %4 to i64
		ret i64 %5 */
		builder->SetInsertPoint(bb[6]); //single
		res[6] = builder->CreateFSub(getFloat(0.0f), builder->CreateBitCast(builder->CreateTrunc(data, llvmType(getInt32Ty)), llvmType(getFloatTy)));
		res[6] = builder->CreateZExt(builder->CreateBitCast(res[6], llvmType(getInt32Ty)), llvmType(getInt64Ty));
		builder->CreateBr(done_block);
		
		builder->SetInsertPoint(bb[7]); //float
		res[7] = builder->CreateBitCast(builder->CreateFSub(getFloat(0.0), builder->CreateBitCast(data, llvmType(getDoubleTy))), llvmType(getInt64Ty));
		builder->CreateBr(done_block);
		
		builder->SetInsertPoint(bb[0]); //else
		release(val, T_VARIANT);
		create_throw(E_TYPE, "Number", "(unknown)");
		
		builder->SetInsertPoint(done_block);
		llvm::PHINode* phi = builder->CreatePHI(llvmType(getInt64Ty), 7, "variant_neg_result");
		for(int i=1; i<8; i++){
			phi->addIncoming(res[i], bb[i]);
		}
		
		llvm::Value* ret = get_new_struct(variant_type, vtype, phi);
		
		if (on_stack)
			push_value(ret, type);
		return ret;
	} else {
		llvm::Value* val = expr->codegen_get_value();
		if (expr->on_stack)
			c_SP(-1);
		llvm::Value* ret;
		if (type == T_BOOLEAN){
			ret = val;
		} else if (type <= T_LONG){
			static const int bits[] = {0, 1, 8, 16, 32, 64};
			ret = builder->CreateSub(getInteger(bits[type], 0), val);
		} else {
			ret = builder->CreateFSub(type == T_SINGLE ? getFloat(0.0f) : getFloat(0.0), val);
		}
		if (on_stack)
			push_value(ret, type);
		return ret;
	}
}

llvm::Value* CatExpression::codegen_get_value(){
	codegen_on_stack();
	return ret_top_stack(T_STRING, on_stack);
}
void CatExpression::codegen_on_stack(){
	for(size_t i=0, e=args.size(); i!=e; i++){
		args[i]->codegen_on_stack();
	}
	builder->CreateCall(get_global_function_jif(SUBR_cat, 'v', "h"), getInteger(16, args.size()));
}

llvm::Value* FileExpression::codegen_get_value(){
	codegen_on_stack();
	return ret_top_stack(T_STRING, on_stack);
}
void FileExpression::codegen_on_stack(){
	for(size_t i=0, e=args.size(); i!=e; i++){
		args[i]->codegen_on_stack();
	}
	builder->CreateCall(get_global_function_jif(SUBR_file, 'v', "h"), getInteger(16, args.size()));
}

llvm::Value* LikeExpression::codegen_get_value(){
	codegen_on_stack();
	return ret_top_stack(T_BOOLEAN, on_stack);
}
void LikeExpression::codegen_on_stack(){
	left->codegen_on_stack();
	right->codegen_on_stack();
	builder->CreateCall(get_global_function_jif(SUBR_like, 'v', "h"), getInteger(16, kind));
}

llvm::Value* CheckStringExpression::codegen_get_value(){
	llvm::Value* val = expr->codegen_get_value();
	llvm::Value* vtype = extract_value(val, 0);
	llvm::Value* vdata = extract_value(val, 1);
	
	llvm::Value* nullstring = get_new_struct(string_type, getInteger(TARGET_BITS, T_CSTRING), get_nullptr(), getInteger(32, 0), getInteger(32, 0));
	
	llvm::Value* ret = gen_if_phi(nullstring, builder->CreateICmpNE(vtype, getInteger(TARGET_BITS, T_NULL)), [&](){
		gen_if_noreturn(builder->CreateICmpNE(vtype, getInteger(TARGET_BITS, T_STRING)), [&](){
			//codegen_printf("okända typen: %ld\n", vtype);
			create_throw(E_TYPE, JIF.F_TYPE_get_name(T_STRING), "(unknown)");
		});
		llvm::Value* strptr = builder->CreateIntToPtr(vdata, llvmType(getInt8PtrTy));
		llvm::Value* len = builder->CreateLoad(builder->CreateGEP(builder->CreateBitCast(strptr, llvmType(getInt32PtrTy)), getInteger(TARGET_BITS, -1)));
		return get_new_struct(string_type, getInteger(TARGET_BITS, T_STRING), strptr, getInteger(32, 0), len);
	});
	
	c_SP(-expr->on_stack);
	
	if (on_stack)
		push_value(ret, type);
	return ret;
}
llvm::Value* CheckIntegerExpression::codegen_get_value(){
	llvm::Value* ret = expr->codegen_get_value();
	llvm::Value* vtype = extract_value(ret, 0);
	llvm::Value* data = extract_value(ret, 1);
	llvm::Value* not_ok = builder->CreateICmpUGT(vtype, getInteger(TARGET_BITS, T_INTEGER));
	
	c_SP(-expr->on_stack);
	
	gen_if_noreturn(not_ok, [&](){
		release(ret, T_VARIANT);
		create_throw(E_TYPE, JIF.F_TYPE_get_name(T_INTEGER), "(unknown)");
	});
	
	ret = builder->CreateTrunc(data, llvmType(getInt32Ty));
	if (on_stack)
		push_value(ret, T_INTEGER);
	return ret;
}
llvm::Value* CheckFloatExpression::codegen_get_value(){
	llvm::Value* ret = expr->codegen_get_value();
	llvm::Value* vtype = extract_value(ret, 0);
	llvm::Value* data = extract_value(ret, 1);
	llvm::Value* not_ok = builder->CreateICmpUGT(vtype, getInteger(TARGET_BITS, T_FLOAT));
	
	c_SP(-expr->on_stack);
	
	gen_if_noreturn(not_ok, [&](){
		release(ret, T_VARIANT);
		create_throw(E_TYPE, JIF.F_TYPE_get_name(T_INTEGER), "(unknown)");
	});
	
	llvm::Value* low32 = builder->CreateTrunc(data, llvmType(getInt32Ty));
	llvm::Value* float_from_int = builder->CreateSIToFP(low32, llvmType(getDoubleTy));
	
	ret = gen_if_phi(float_from_int, builder->CreateICmpSGT(vtype, getInteger(TARGET_BITS, T_INTEGER)), [&](){
		llvm::Value* float_data = builder->CreateBitCast(data, llvmType(getDoubleTy));
		llvm::Value* single_data = builder->CreateFPExt(builder->CreateBitCast(low32, llvmType(getFloatTy)), llvmType(getDoubleTy));
		llvm::Value* long_data = builder->CreateSIToFP(data, llvmType(getDoubleTy));
		return builder->CreateSelect(
			builder->CreateICmpEQ(vtype, getInteger(TARGET_BITS, T_FLOAT)),
				float_data,
				builder->CreateSelect(builder->CreateICmpEQ(vtype, getInteger(TARGET_BITS, T_SINGLE)), single_data, long_data));
	});
	
	if (on_stack)
		push_value(ret, T_FLOAT);
	return ret;
}
llvm::Value* CheckPointerExpression::codegen_get_value(){
	llvm::Value* ret = expr->codegen_get_value();
	llvm::Value* vtype = extract_value(ret, 0);
	llvm::Value* data = extract_value(ret, 1);
	llvm::Value* not_ok = builder->CreateICmpNE(vtype, getInteger(TARGET_BITS, T_POINTER));
	
	c_SP(-expr->on_stack);
	
	gen_if_noreturn(not_ok, [&](){
		release(ret, T_VARIANT);
		create_throw(E_TYPE, JIF.F_TYPE_get_name(T_POINTER), "(unknown)");
	});
	
	ret = builder->CreateIntToPtr(data, llvmType(getInt8PtrTy));
	if (on_stack)
		push_value(ret, T_POINTER);
	return ret;
}

static llvm::Value* gen_max(llvm::Value* val1, llvm::Value* val2){
	return builder->CreateSelect(builder->CreateICmpSLT(val1, val2), val2, val1);
}

static llvm::Value* gen_min(llvm::Value* val1, llvm::Value* val2){
	return builder->CreateSelect(builder->CreateICmpSLT(val1, val2), val1, val2);
}

static llvm::Value* gen_minmax(llvm::Value* val, llvm::Value* lo, llvm::Value* hi){
	llvm::BasicBlock* BB1 = builder->GetInsertBlock();
	llvm::BasicBlock* BB2 = create_bb("minmax1");
	llvm::BasicBlock* BB3 = create_bb("minmax2");
	llvm::Value* cmp = builder->CreateICmpSLT(val, lo);
	builder->CreateCondBr(cmp, BB3, BB2);
	
	builder->SetInsertPoint(BB2);
	llvm::Value* cmp2 = builder->CreateICmpSGT(val, hi);
	llvm::Value* r1 = builder->CreateSelect(cmp2, hi, val);
	builder->CreateBr(BB3);
	
	builder->SetInsertPoint(BB3);
	llvm::PHINode* phi = builder->CreatePHI(val->getType(), 2);
	phi->addIncoming(lo, BB1);
	phi->addIncoming(r1, BB2);
	
	return phi;
}

static llvm::Value* get_string(llvm::Value* str){
	llvm::Value* ptr = extract_value(str, 1);
	return builder->CreateGEP(ptr, to_target_int(extract_value(str, 2)));
}

static std::pair<llvm::Value*, llvm::Value*> get_string_len(llvm::Value* str){
	llvm::Value* ptr = extract_value(str, 1);
	ptr = builder->CreateGEP(ptr, to_target_int(extract_value(str, 2)));
	return std::pair<llvm::Value*, llvm::Value*>(ptr, extract_value(str, 3));
}

static llvm::Value* codegen_spec_method(CLASS_DESC* desc, int index, bool dispatch, bool can_quick, bool object_on_stack, CLASS* klass, llvm::Value* effective_class, llvm::Value* obj, int nargs, bool noret){
	llvm::Value* is_native;
	llvm::Value* desc_method;
	llvm::Value* desc_exec;
	llvm::Value* desc_class;
	if (dispatch){
		int offset_table = TARGET_BITS == 64 ? 40/8 : 28/4;
		int offset_desc_in_desc_symbol = TARGET_BITS == 64 ? 12 : 8;
		int offset_native_flag = TARGET_BITS == 64 ? 35 : 19;
		//table_addr = (char*)effective_class->table
		llvm::Value* table_addr = builder->CreateLoad(builder->CreateGEP(builder->CreateBitCast(effective_class, pointer_t(llvmType(getInt8PtrTy))), getInteger(TARGET_BITS, offset_table)));
		//desc_addr_addr = (char*)(table_addr + sizeof(CLASS_DESC_SYMBOL) * index + desc_offset)
		llvm::Value* desc_addr_addr = builder->CreateGEP(table_addr, getInteger(TARGET_BITS, sizeof(CLASS_DESC_SYMBOL) * index + offset_desc_in_desc_symbol));
		//desc_addr = *(char**)desc_addr_addr
		llvm::Value* desc_addr = builder->CreateLoad(builder->CreateBitCast(desc_addr_addr, pointer_t(llvmType(getInt8PtrTy))));
		//native_flag_addr = (char*)(desc_addr + offset_native_flag)
		llvm::Value* native_flag_addr = builder->CreateGEP(desc_addr, getInteger(TARGET_BITS, offset_native_flag));
		//native_subr_flag = *native_flag_addr
		llvm::Value* native_subr_flag = builder->CreateLoad(native_flag_addr);
		//native_flag = (bool)(native_subr_flag & 1)
		llvm::Value* native_flag = builder->CreateTrunc(native_subr_flag, llvmType(getInt1Ty));
		//subr_flag = (bool)(native_subr_flag & 2)
		//llvm::Value* subr_flag = builder->CreateICmpNE(builder->CreateAnd(native_subr_flag, getInteger(8, 2)), getInteger(8, 0));
		
		//llvm::Value* function_kind = builder->CreateSelect(native_flag, builder->CreateSelect(subr_flag, getInteger(8, FUNCTION_SUBR), getInteger(8, FUNCTION_NATIVE)), getInteger(8, FUNCTION_PUBLIC));
		///Disable subr, because I think it will never be used this way anyway, else it is a (big) performance hit.
		is_native = native_flag;
		desc_method = desc_addr;
		desc_exec = builder->CreateLoad(builder->CreateBitCast(builder->CreateGEP(desc_addr, getInteger(TARGET_BITS, offsetof(CLASS_DESC_METHOD, exec))), charPP));
		desc_class = builder->CreateLoad(builder->CreateBitCast(builder->CreateGEP(desc_addr, getInteger(TARGET_BITS, offsetof(CLASS_DESC_METHOD, klass))), charPP));
		
		if (!desc->method.native){
			//Assume native classes don't inherit non-native ones :)
			is_native = getInteger(1, false);
		}
	} else {
		is_native = getInteger(1, desc->method.native);
		desc_method = get_global((void*)&desc->method, llvmType(getInt8Ty));
		desc_exec = get_global((void*)desc->method.exec, llvmType(getInt8Ty));
		desc_class = get_global((void*)desc->method.klass, llvmType(getInt8Ty));
	}
	
	llvm::Value* dummy = getInteger(32, 0);
	
	llvm::Value* ret = gen_if_else_phi(is_native, [&](){
		if (can_quick){
			llvm::Value* got_error = builder->CreateCall4(get_global_function_jif(EXEC_call_native, 'c', "ppjp"),
				desc_exec,
				obj,
				getInteger(TARGET_BITS, desc->method.type),
				builder->CreateBitCast(builder->CreateGEP(read_sp(), getInteger(TARGET_BITS, -nargs)), llvmType(getInt8PtrTy)));
			
			gen_if_noreturn(builder->CreateICmpNE(got_error, getInteger(8, false)), [&](){
				builder->CreateCall(get_global_function_jif(ERROR_propagate, 'v', ""));
				builder->CreateUnreachable();
			});
			
			llvm::Value* ret = dummy;
			if (!noret){
				ret = read_value(get_global(&TEMP, value_type), desc->method.type);
				builder->CreateStore(getInteger(TARGET_BITS, T_VOID), get_global(&TEMP, LONG_TYPE));
				borrow(ret, desc->method.type);
			}
			
			if (nargs > 0){
				builder->CreateCall2(get_global_function_jif(RELEASE_many, 'v', "pi"),
					builder->CreateBitCast(read_sp(), llvmType(getInt8PtrTy)), getInteger(32, nargs));
				c_SP(-nargs);
			}
			unref_object(obj);
			if (object_on_stack)
				c_SP(-1);
			if (!noret)
				push_value(ret, desc->method.type);
			return ret;
		} else {
			builder->CreateStore(desc_class, get_global((void*)&EXEC.klass));
			builder->CreateStore(obj, get_global((void*)&EXEC.object));
			builder->CreateStore(getInteger(32, nargs), get_global((void*)&EXEC.nparam, llvmType(getInt32Ty)));
			
			builder->CreateStore(desc_method, get_global((void*)&EXEC.desc));
			builder->CreateStore(getInteger(8, false), get_global((void*)&EXEC.use_stack, llvmType(getInt8Ty)));
			
			builder->CreateCall(get_global_function_jif(EXEC_native, 'v' , ""));
			
			llvm::Value* ret = dummy;
			if (!noret)
				ret = read_value(get_value_on_top_addr(), desc->method.type);
			
			unref_object(obj);
			if (object_on_stack){
				c_SP(-1);
				if (!noret)
					store_value(get_value_on_top_addr(), ret, desc->method.type);
			}
			if (noret)
				c_SP(-1);
			return ret;
		}
	}, [&](){
		builder->CreateStore(desc_class, get_global((void*)&EXEC.klass));
		builder->CreateStore(obj, get_global((void*)&EXEC.object));
		builder->CreateStore(getInteger(32, nargs), get_global((void*)&EXEC.nparam, llvmType(getInt32Ty)));
		builder->CreateStore(builder->CreatePtrToInt(desc_exec, llvmType(getInt32Ty)), get_global((void*)&EXEC.index, llvmType(getInt32Ty)));
		
		/*builder->CreateCall2(get_global_function_vararg(printf, 'v', "pi"),
			get_global((void*)"%d\n", llvmType(getInt8Ty)), getInteger(32, 5));*/
		
		builder->CreateCall(get_global_function_jif(EXEC_function_real, 'v', ""));
		
		llvm::Value* ret = dummy;
		if (!noret){
			ret = read_value(get_global(RP, value_type), desc->method.type);
			builder->CreateStore(getInteger(TARGET_BITS, T_VOID), get_global(RP, LONG_TYPE));
		}
		unref_object(obj);
		if (object_on_stack)
			c_SP(-1);
		if (!noret)
			push_value(ret, desc->method.type);
		return ret;
	}, "spec_native", "spec_non_native", "spec_done");
	return ret;
}

struct InlineArrayGetAddrRet {
	llvm::Value* element_addr;
	CLASS* klass;
	CLASS* struct_class;
};
InlineArrayGetAddrRet inline_array_get_addr(std::vector<Expression*>& args, bool is_push_dynamic){
	CTYPE* ctype;
	CLASS* klass;
	llvm::Value* array_start;
	if (is_push_dynamic){
		ctype = &CP->load->dyn[((PushDynamicExpression*)args[0])->index].type;
		klass = CP;
		array_start = builder->CreateGEP(current_op, getInteger(TARGET_BITS, ((PushDynamicExpression*)args[0])->offset));
	} else {
		ctype = ((ReadVariableExpression*)args[0])->ctype;
		klass = ((ReadVariableExpression*)args[0])->klass;
		array_start = get_global((void*)((PushStaticExpression*)args[0])->addr, llvmType(getInt8Ty));
	}
	
	CLASS_ARRAY* ca = klass->load->array[ctype->value];
	int* dim = ca->dim;
	int ndim = args.size()-1;
	
	int dims[ndim];
	memcpy(dims, dim, ndim*sizeof(int));
	dims[ndim-1] = -dims[ndim-1];
	
	llvm::Value* index = getInteger(32, 0);
	
	for(int i=0; i<ndim; i++){
		llvm::Value* d = args[i+1]->codegen_get_value();
		gen_if_noreturn(builder->CreateICmpUGE(d, getInteger(32, dims[i])), [&](){
			create_throw(E_BOUND);
		});
		if (i != 0)
			index = builder->CreateMul(index, getInteger(32, dims[i]));
		index = builder->CreateAdd(index, d);
		
		c_SP(-args[i+1]->on_stack);
	}
	
	CLASS* struct_class = NULL;
	int element_size;
	if (ca->ctype.id == TC_STRUCT){
		//Array of structs
		struct_class = klass->load->class_ref[ca->ctype.value];
		element_size = struct_class->size - sizeof(CSTRUCT);
	} else {
		element_size = TYPE_sizeof_memory(ctype_to_type(&ca->ctype, klass));	
	}
	
	return {
		builder->CreateGEP(array_start, to_target_int(builder->CreateMul(index, getInteger(32, element_size)))),
		klass,
		struct_class
	};
}

llvm::Value* PushArrayExpression::codegen_get_value(){
	int nargs = args.size();
	
	CLASS* klass;
	bool is_push_class;
	
	bool is_push_dynamic = false;
	
	if ( ((is_push_dynamic = isa<PushDynamicExpression>(args[0])) && ((PushDynamicExpression*)args[0])->ctype->id == TC_ARRAY)
		||
		(dynamic_cast<ReadVariableExpression*>(args[0]) && ((ReadVariableExpression*)args[0])->ctype->id == TC_ARRAY) ){
		auto ret = inline_array_get_addr(args, is_push_dynamic);
		
		llvm::Value* val;
		
		if (ret.struct_class){
			//Array of structs
			llvm::Value* ref_obj;
			if (is_push_dynamic)
				ref_obj = current_op;
			else
				ref_obj = get_global((void*)((ReadVariableExpression*)args[0])->klass, llvmType(getInt8Ty));
			
			val = builder->CreateCall3(get_global_function_jif(CSTRUCT_create_static, 'p', "ppp"),
				ref_obj,
				get_global((void*)ret.struct_class, llvmType(getInt8Ty)),
				ret.element_addr);
				
			borrow_object_no_nullcheck(val);
			
			val = get_new_struct(object_type, get_global((void*)ret.struct_class, llvmType(getInt8Ty)), val);
		} else {
			val = array_read(ret.element_addr, type);
		}
		if (on_stack)
			push_value(val, type);
		return val;
	} else if (TYPE_is_pure_object(args[0]->type)){
		klass = (CLASS*)(void*)args[0]->type;
		
		if (klass->quick_array == CQA_ARRAY){
			llvm::Value* obj = extract_value(args[0]->codegen_get_value(), 1);
			llvm::Value* data;
			if (args.size() == 2){
				llvm::Value* index = args[1]->codegen_get_value();
				
				//We do not check that the dimensions are correct here ... :)
				int element_size = TYPE_sizeof_memory(type);
				int data_offset = offsetof(CARRAY, data);
				int count_offset = offsetof(CARRAY, count);
				
				make_nullcheck(obj);
				
				c_SP(-args[0]->on_stack-args[1]->on_stack);
				
				llvm::Value* count = builder->CreateLoad(builder->CreateBitCast(builder->CreateGEP(obj, getInteger(TARGET_BITS, count_offset)), llvmType(getInt32PtrTy)));
				data = builder->CreateLoad(builder->CreateBitCast(builder->CreateGEP(obj, getInteger(TARGET_BITS, data_offset)), charPP));
				
				gen_if_noreturn(builder->CreateICmpUGE(index, count), [&](){
					unref_object_no_nullcheck(obj);
					create_throw(E_BOUND);
				}, "array_get_out_of_bounds");
				
				data = builder->CreateGEP(data, to_target_int(builder->CreateMul(index, getInteger(32, element_size))));
			} else {
				args[1]->codegen_on_stack();
				llvm::Value* ind_addr = get_value_on_top_addr();
				for(int i=2; i<nargs; i++)
					args[i]->codegen_on_stack();
				
				make_nullcheck(obj);
				
				c_SP(-args[0]->on_stack-(nargs-1));
				
				data = builder->CreateCall3(get_global_function_jif(CARRAY_get_data_multi, 'p', "ppi"),
					obj, builder->CreateBitCast(ind_addr, llvmType(getInt8PtrTy)), getInteger(32, nargs-1));
				
				gen_if_noreturn(builder->CreateICmpEQ(data, get_nullptr()), [&](){
					unref_object_no_nullcheck(obj);
					
					builder->CreateCall(get_global_function_jif(ERROR_propagate, 'v', ""));
					builder->CreateUnreachable();
				});
			}
			data = array_read(data, type);
			
			unref_object_no_nullcheck(obj);
			
			if (on_stack)
				push_value(data, type);
			return data;
		} else if (klass->quick_array == CQA_COLLECTION){
			llvm::Value* obj = extract_value(args[0]->codegen_get_value(), 1);
			llvm::Value* key = args[1]->codegen_get_value();
			auto str = get_string_len(key);
			
			make_nullcheck(obj, [&](){
				if (!args[1]->on_stack)
					release(key, T_STRING);
			});
			
			llvm::Value* sp = builder->CreateGEP(read_sp(), getInteger(TARGET_BITS, -args[0]->on_stack-args[1]->on_stack));
			llvm::Value* sp_int8 = builder->CreateBitCast(sp, llvmType(getInt8PtrTy));
			
			/*llvm::Value* is_null =*/ builder->CreateCall4(get_global_function(GB.Collection.Get, 'c', "ppip"),
				obj, str.first, str.second, sp_int8);
			
			//type is always T_VARIANT, not T_NULL
			llvm::Value* ret = builder->CreateBitCast(sp, pointer_t(value_types[T_VARIANT]));
			ret = get_new_struct(variant_type, load_element(ret, 1), load_element(ret, 2));
			
			borrow_variant(ret);
			
			store_sp(builder->CreateGEP(sp, getInteger(TARGET_BITS, 1)));
			
			release(key, T_STRING);
			unref_object_no_nullcheck(obj);
			
			return ret;
		}
		klass = (CLASS*)(void*)args[0]->type;
		is_push_class = false;
		goto _SPEC_GET_METHOD;
	} else if (PushClassExpression* pce = dyn_cast<PushClassExpression>(args[0])){
		//llvm::Value* obj = extract_value(args[0]->codegen_get_value(), 1);
		klass = pce->klass;
		is_push_class = true;
		goto _SPEC_GET_METHOD;
	} else {
		for(size_t i=0, e=args.size(); i!=e; i++)
			args[i]->codegen_on_stack();
		builder->CreateCall(get_global_function_jif(EXEC_push_array, 'v', "h"), getInteger(16, *pc));
		return ret_top_stack(T_VARIANT, on_stack);
	}
	
	_SPEC_GET_METHOD: {
		int index = klass->special[SPEC_GET];
		CLASS_DESC* desc = CLASS_get_desc(klass, index);
		
		llvm::Value* obj = NULL;
		llvm::Value* effective_class = NULL;
		bool dispatch;
		
		if (!is_push_class){
			llvm::Value* object = args[0]->codegen_get_value();
			obj = extract_value(object, 1);
			
			if (!klass->is_virtual && !isa<PushSuperExpression>(args[0])){
				make_nullcheck(obj);
				
				effective_class = load_element(builder->CreateBitCast(obj, pointer_t(OBJECT_type)), 0);
				create_check(klass, effective_class, obj);
				dispatch = true;
			} else {
				effective_class = get_global((void*)klass, llvmType(getInt8Ty));
				dispatch = false;
			}
		} else {
			obj = get_nullptr();
			effective_class = get_global((void*)klass, llvmType(getInt8Ty));
			dispatch = false;
		}
		
		for(size_t i=1; i<args.size(); i++)
			args[i]->codegen_on_stack();
		
		return codegen_spec_method(desc, index, dispatch, can_quick, !is_push_class, klass, effective_class, obj, args.size()-1, false);
	}
}

void PopArrayExpression::codegen(){
	int nargs = args.size();
	
	CLASS* klass;
	bool is_push_class;
	bool is_push_dynamic = false;
	
	if ( ((is_push_dynamic = isa<PushDynamicExpression>(args[0])) && ((PushDynamicExpression*)args[0])->ctype->id == TC_ARRAY)
		||
		(dynamic_cast<ReadVariableExpression*>(args[0]) && ((ReadVariableExpression*)args[0])->ctype->id == TC_ARRAY) ){
		
		llvm::Value* addr = inline_array_get_addr(args, is_push_dynamic).element_addr;
		llvm::Value* new_val = val->codegen_get_value();
		
		release_variable(type, addr);
		variable_write(addr, new_val, type);
		
		c_SP(-val->on_stack); //FIXME what if destructor in release_variable throws an error?
		
		return;
	} else if (TYPE_is_pure_object(args[0]->type)){
		klass = (CLASS*)(void*)args[0]->type;
		
		if (klass->quick_array == CQA_ARRAY){
			llvm::Value* new_val = val->codegen_get_value();
			
			llvm::Value* obj = extract_value(args[0]->codegen_get_value(), 1);
			llvm::Value* data;
			if (args.size() == 2){
				llvm::Value* index = args[1]->codegen_get_value();
				
				//We do not check that the dimensions are correct here ... :)
				int element_size = TYPE_sizeof_memory(type);
				int data_offset = offsetof(CARRAY, data);
				int count_offset = offsetof(CARRAY, count);
				
				make_nullcheck(obj, [&](){
					release(new_val, val->type);
				});
				
				c_SP(-val->on_stack-args[0]->on_stack-args[1]->on_stack); //FIXME what if destructor in release_variable throws an error?
				
				llvm::Value* count = builder->CreateLoad(builder->CreateBitCast(builder->CreateGEP(obj, getInteger(TARGET_BITS, count_offset)), llvmType(getInt32PtrTy)));
				data = builder->CreateLoad(builder->CreateBitCast(builder->CreateGEP(obj, getInteger(TARGET_BITS, data_offset)), charPP));
				
				gen_if_noreturn(builder->CreateICmpUGE(index, count), [&](){
					release(new_val, val->type);
					unref_object_no_nullcheck(obj);
					create_throw(E_BOUND);
				}, "array_get_out_of_bounds");
				
				data = builder->CreateGEP(data, to_target_int(builder->CreateMul(index, getInteger(32, element_size))));
			} else {
				args[1]->codegen_on_stack();
				llvm::Value* ind_addr = get_value_on_top_addr();
				for(int i=2; i<nargs; i++)
					args[i]->codegen_on_stack();
				
				make_nullcheck(obj, [&](){
					if (!val->on_stack)
						release(new_val, val->type);
				});
				
				c_SP(-val->on_stack-args[0]->on_stack-(nargs-1));
				
				data = builder->CreateCall3(get_global_function_jif(CARRAY_get_data_multi, 'p', "ppi"),
					obj, builder->CreateBitCast(ind_addr, llvmType(getInt8PtrTy)), getInteger(32, nargs-1));
				
				gen_if_noreturn(builder->CreateICmpEQ(data, get_nullptr()), [&](){
					release(new_val, val->type);
					unref_object_no_nullcheck(obj);
					
					builder->CreateCall(get_global_function_jif(ERROR_propagate, 'v', ""));
					builder->CreateUnreachable();
				});
			}
			release_variable(type, data);
			variable_write(data, new_val, type);
			
			unref_object_no_nullcheck(obj);
			
			return;
		} else if (klass->quick_array == CQA_COLLECTION){
			llvm::Value* value = val->codegen_get_value();
			llvm::Value* object = extract_value(args[0]->codegen_get_value(), 1);
			llvm::Value* key = args[1]->codegen_get_value();
			
			c_SP(-1-args[0]->on_stack-args[1]->on_stack);
			
			make_nullcheck(object, [&](){
				release(key, T_STRING);
			});
			
			gen_if_noreturn(builder->CreateICmpEQ(extract_value(key, 3), getInteger(32, 0)), [&](){
				release(key, T_STRING);
				unref_object_no_nullcheck(object);
				create_throw(E_VKEY);
			});
			
			auto str = get_string_len(key);
			
			builder->CreateCall4(get_global_function(GB.Collection.Set, 'c', "ppip"),
				object, str.first, str.second, builder->CreateBitCast(read_sp(), llvmType(getInt8PtrTy)));
			
			release(value, T_VARIANT);
			release(key, T_STRING);
			unref_object_no_nullcheck(object);
			return;
		}
		klass = (CLASS*)(void*)args[0]->type;
		is_push_class = false;
		goto _SPEC_PUT_METHOD;
	} else if (PushClassExpression* pce = dyn_cast<PushClassExpression>(args[0])){
		klass = pce->klass;
		is_push_class = true;
		goto _SPEC_PUT_METHOD;
	} else {
		val->codegen_on_stack();
		for(size_t i=0, e=args.size(); i!=e; i++)
			args[i]->codegen_on_stack();
		builder->CreateCall(get_global_function_jif(EXEC_pop_array, 'v', "h"), getInteger(16, *pc));
		return;
	}
	
	_SPEC_PUT_METHOD: {
		int index = klass->special[SPEC_PUT];
		CLASS_DESC* desc = CLASS_get_desc(klass, index);
		
		llvm::Value* obj = NULL;
		llvm::Value* effective_class = NULL;
		bool dispatch;
		
		if (!is_push_class){
			llvm::Value* object = args[0]->codegen_get_value();
			obj = extract_value(object, 1);
			
			if (!klass->is_virtual && !isa<PushSuperExpression>(args[0])){
				make_nullcheck(obj);
				
				effective_class = load_element(builder->CreateBitCast(obj, pointer_t(OBJECT_type)), 0);
				create_check(klass, effective_class, obj);
				dispatch = true;
			} else {
				effective_class = get_global((void*)klass, llvmType(getInt8Ty));
				dispatch = false;
			}
		} else {
			obj = get_nullptr();
			effective_class = get_global((void*)klass, llvmType(getInt8Ty));
			dispatch = false;
		}
		
		val->codegen_on_stack();
		for(size_t i=1; i<args.size(); i++)
			args[i]->codegen_on_stack();
		
		codegen_spec_method(desc, index, dispatch, can_quick, !is_push_class, klass, effective_class, obj, args.size(), true);
	}
}

#include "jit_codegen_conv.h"

#define codegen_stack for(size_t i=0, e=args.size(); i!=e; i++){ args[i]->must_on_stack(); args[i]->codegen_on_stack(); } stack_diff -= args.size();
#define codegen_value for(size_t i=0, e=args.size(); i!=e; i++){ param[i] = args[i]->codegen_get_value(); stack_diff -= args[i]->on_stack; }

#define SUBR_CODE(func) on_stack = true; builder->CreateCall(get_global_function_jif(func, 'v', "h"), getInteger(16, (digit << 8) | extra)); break;
#define SUBR(func) on_stack = true; builder->CreateCall(get_global_function_jif(func, 'v', "")); break;

static llvm::Value* create_phi(llvm::Value* v1, llvm::BasicBlock* BB1, llvm::Value* v2, llvm::BasicBlock* BB2){
	llvm::PHINode* phi = builder->CreatePHI(v1->getType(), 2);
	phi->addIncoming(v1, BB1);
	phi->addIncoming(v2, BB2);
	return phi;
}

llvm::Value* SubrExpression::codegen_get_value(){
	int nargs = args.size();
	llvm::Value* param[nargs];
	
	int stack_diff = on_stack;
	/*for(int i=0; i<nargs; i++)
		stack_diff -= args[0]->on_stack;*/
	
	switch(digit){
		case 0x40: { //Left
			codegen_value
			llvm::Value* string_length = extract_value(param[0], 3);
			llvm::Value* val = nargs == 1 ? getInteger(32, 1) : param[1];
			llvm::Value* backwards = builder->CreateAdd(val, string_length);
			val = builder->CreateSelect(builder->CreateICmpSLT(val, getInteger(32, 0)), backwards, val);
			val = gen_minmax(val, getInteger(32, 0), string_length);
			param[0] = insert_value(param[0], val, 3);
			c_SP(-(args[0]->on_stack + (nargs == 2 && args[1]->on_stack)));
			if (on_stack)
				push_value(param[0], args[0]->type);
			return param[0];
		}
		case 0x42: { //Right
			codegen_value
			llvm::Value* string_offset = extract_value(param[0], 2);
			llvm::Value* string_length = extract_value(param[0], 3);
			llvm::Value* val = nargs == 1 ? getInteger(32, 1) : param[1];
			llvm::Value* backwards = builder->CreateAdd(val, string_length);
			val = builder->CreateSelect(builder->CreateICmpSLT(val, getInteger(32, 0)), backwards, val);
			llvm::Value* len = gen_minmax(val, getInteger(32, 0), string_length);
			val = builder->CreateSub(string_length, len);
			val = builder->CreateAdd(string_offset, val);
			param[0] = insert_value(insert_value(param[0], val, 2), len, 3);
			c_SP(-(args[0]->on_stack + (nargs == 2 && args[1]->on_stack)));
			if (on_stack)
				push_value(param[0], args[0]->type);
			return param[0];
		}
		case 0x41: { //Mid
			codegen_value
			llvm::Value* start = builder->CreateSub(param[1], getInteger(32, 1));
			gen_if_noreturn(builder->CreateICmpSLT(start, getInteger(32, 0)), [&](){
				create_throw(E_ARG);
			});
			llvm::Value* string_length = extract_value(param[0], 3);
			
			llvm::BasicBlock* BB1 = create_bb("Mid_return_empty_string");
			llvm::BasicBlock* BB2 = create_bb("cont1");
			llvm::BasicBlock* BB3 = create_bb("cont2");
			llvm::BasicBlock* BB4 = create_bb("cont3");
			
			builder->CreateCondBr(builder->CreateICmpSGE(start, string_length), BB1, BB2);
			builder->SetInsertPoint(BB2);
			
			llvm::Value* newlen = nargs == 2 ? string_length : param[2];
			
			llvm::Value* string_length_minus_start = builder->CreateSub(string_length, start);
			
			llvm::Value* newlen_if_negative = gen_max(getInteger(32, 0),
				builder->CreateAdd(string_length_minus_start, newlen));
			newlen = builder->CreateSelect(builder->CreateICmpSLT(newlen, getInteger(32, 0)),
				newlen_if_negative, newlen);
			
			newlen = gen_minmax(newlen, getInteger(32, 0), string_length_minus_start);
			
			builder->CreateCondBr(builder->CreateICmpEQ(newlen, getInteger(32, 0)), BB1, BB3);
			builder->SetInsertPoint(BB3);
			
			llvm::Value* ret1 = insert_value(insert_value(param[0], builder->CreateAdd(extract_value(param[0], 2), start), 2), newlen, 3);
			
			builder->CreateBr(BB4);
			builder->SetInsertPoint(BB1);
			release(param[0], args[0]->type);
			llvm::BasicBlock* BB5 = builder->GetInsertBlock();
			llvm::Value* ret2 = get_voidstring();
			builder->CreateBr(BB4);
			
			builder->SetInsertPoint(BB4);
			param[0] = create_phi(ret1, BB3, ret2, BB5);
			c_SP(-(args[0]->on_stack + (nargs == 2 && args[1]->on_stack) + (nargs == 3 && args[2]->on_stack)));
			if (on_stack)
				push_value(param[0], T_STRING);
			return param[0];
		}
		case 0x43: { //Len
			codegen_value
			llvm::Value* string_length = extract_value(param[0], 3);
			release(param[0], args[0]->type);
			c_SP(-args[0]->on_stack + on_stack);
			if (on_stack)
				set_top_value(string_length, T_INTEGER);
			return string_length;
		}
		case 0x44: { //Space
			codegen_value
			gen_if_noreturn(builder->CreateICmpSLT(param[0], getInteger(32, 0)), [&](){
				create_throw(E_ARG);
			});
			llvm::Value* ret = gen_if_else_phi(builder->CreateICmpEQ(param[0], getInteger(32, 0)), [&](){
				return get_voidstring();
			}, [&](){
				builder->CreateCall3(get_global_function_jif(STRING_new_temp_value, 'v', "ppi"),
					builder->CreateBitCast(temp_value, llvmType(getInt8PtrTy)), get_nullptr(), param[0]);
				
				llvm::Value* str = builder->CreateLoad(builder->CreateBitCast(temp_value, pointer_t(string_type)));
				if (TARGET_BITS == 64)
					param[0] = builder->CreateZExt(param[0], llvmType(getInt64Ty));
				
				llvm::Value* ptr = extract_value(str, 1);
				//llvm::Function* memset_func = llvm::Intrinsic::getDeclaration(M, llvm::Intrinsic::memset, string_to_type_vector("pj"));
				const char* memset_name = TARGET_BITS == 64 ? "llvm.memset.p0i8.i64" : "llvm.memset.p0i8.i32";
				llvm::Function* memset_func = llvm::cast<llvm::Function>(M->getOrInsertFunction(memset_name, get_function_type('v', "pcjib", false)));
				builder->CreateCall5(memset_func, ptr, getInteger(8, ' '), param[0], getInteger(32, 0), getInteger(1, false));
				borrow_string_no_nullcheck(ptr);
				return str;
			});
			c_SP(-args[0]->on_stack + on_stack);
			if (on_stack)
				set_top_value(ret, T_STRING);
			return ret;
		}
		case 0x45: //String
			codegen_stack
			SUBR(SUBR_string)
		
		case 0x46: { //Trim
			codegen_value
			bool left = extra == 0 || extra == 1;
			bool right = extra == 0 || extra == 2;
			llvm::Value* string_ptr = extract_value(param[0], 1);
			llvm::Value* string_offset = extract_value(param[0], 2);
			llvm::Value* string_length = extract_value(param[0], 3);
			
			string_ptr = builder->CreateGEP(string_ptr, to_target_int(string_offset));
			
			if (left){
				llvm::Value* enter_loop = gen_and_if(
					builder->CreateICmpSGT(string_length, getInteger(32, 0)),
					[&](){return builder->CreateICmpULE(builder->CreateLoad(string_ptr), getInteger(8, ' '));}
				);
				llvm::BasicBlock *from_block = builder->GetInsertBlock(), *loop_block;
				llvm::Value *ret_offset, *ret_length, *ret_ptr;
				
				gen_while(enter_loop, [&](){
					llvm::PHINode* phi_offset = builder->CreatePHI(llvmType(getInt32Ty), 2);
					llvm::PHINode* phi_length = builder->CreatePHI(llvmType(getInt32Ty), 2);
					llvm::PHINode* phi_ptr = builder->CreatePHI(llvmType(getInt8PtrTy), 2);
					ret_offset = builder->CreateAdd(phi_offset, getInteger(32, 1));
					ret_length = builder->CreateSub(phi_length, getInteger(32, 1));
					ret_ptr = builder->CreateGEP(phi_ptr, getInteger(TARGET_BITS, 1));
					
					llvm::Value* ret = gen_and_if(
						builder->CreateICmpSGT(ret_length, getInteger(32, 0)),
						[&](){return builder->CreateICmpULE(builder->CreateLoad(ret_ptr), getInteger(8, ' ')); }
					);
					loop_block = builder->GetInsertBlock();
					phi_offset->addIncoming(string_offset, from_block);
					phi_offset->addIncoming(ret_offset, loop_block);
					phi_length->addIncoming(string_length, from_block);
					phi_length->addIncoming(ret_length, loop_block);
					phi_ptr->addIncoming(string_ptr, from_block);
					phi_ptr->addIncoming(ret_ptr, loop_block);
					return ret;
				});
				string_ptr = create_phi(string_ptr, from_block, ret_ptr, loop_block);
				string_offset = create_phi(string_offset, from_block, ret_offset, loop_block);
				string_length = create_phi(string_length, from_block, ret_length, loop_block);
			}
			if (right){
				llvm::Value* enter_loop = gen_and_if(
					builder->CreateICmpSGT(string_length, getInteger(32, 0)),
					[&](){return builder->CreateICmpULE(
						builder->CreateLoad(
							builder->CreateGEP(
								string_ptr,
								to_target_int(builder->CreateSub(string_length, getInteger(32, 1)))
							)
						), getInteger(8, ' ')
					);}
				);
				llvm::BasicBlock *from_block = builder->GetInsertBlock(), *loop_block;
				llvm::Value* ret_length;
				gen_while(enter_loop, [&](){
					llvm::PHINode* phi_length = builder->CreatePHI(llvmType(getInt32Ty), 2);
					ret_length = builder->CreateSub(phi_length, getInteger(32, 1));
					
					llvm::Value* ret = gen_and_if(
						builder->CreateICmpSGT(ret_length, getInteger(32, 0)),
						[&](){return builder->CreateICmpULE(
							builder->CreateLoad(
								builder->CreateGEP(
									string_ptr,
									to_target_int(builder->CreateSub(ret_length, getInteger(32, 1)))
								)
							), getInteger(8, ' ')
						);}
					);
					loop_block = builder->GetInsertBlock();
					phi_length->addIncoming(string_length, from_block);
					phi_length->addIncoming(ret_length, loop_block);
					return ret;
				});
				string_length = create_phi(string_length, from_block, ret_length, loop_block);
			}
			llvm::Value* ret = insert_value(insert_value(param[0], string_offset, 2), string_length, 3);
			c_SP(-args[0]->on_stack + on_stack);
			if (on_stack)
				set_top_value(ret, args[0]->type);
			return ret;
		}
		case 0x47:
		case 0x48: { //UCase, LCase
			codegen_stack
			SUBR_CODE(SUBR_upper)
		}
		
		case 0x49: { //Chr
			codegen_value
			gen_if_noreturn(builder->CreateICmpUGT(param[0], getInteger(32, 255)), [&](){
				create_throw(E_ARG);
			}, "Chr_out_of_range", "Chr_ok");
			llvm::Value* arr = get_global((void*)STRING_char_string, llvmType(getInt8Ty));
			param[0] = builder->CreateMul(param[0], getInteger(32, 2));
			if (TARGET_BITS == 64)
				param[0] = builder->CreateZExt(param[0], llvmType(getInt64Ty));
			llvm::Value* retaddr = builder->CreateGEP(arr, param[0]);
			llvm::Value* ret = get_new_struct(string_type, getInteger(TARGET_BITS, T_CSTRING), retaddr, getInteger(32, 0), getInteger(32, 1));
			c_SP(-args[0]->on_stack + on_stack);
			if (on_stack)
				set_top_value(ret, T_CSTRING);
			return ret;
		}
		
		case 0x4A: { //Asc
			codegen_value
			llvm::Value* pos = nargs == 2 ? builder->CreateSub(param[1], getInteger(32, 1)) : getInteger(32, 0);
			llvm::Value* strptr = extract_value(param[0], 1);
			llvm::Value* ret = gen_if_else_phi(builder->CreateOr(builder->CreateICmpSLT(pos, getInteger(32, 0)), builder->CreateICmpSGE(pos, extract_value(param[0], 3))), [&](){
				return getInteger(32, 0);
			}, [&](){
				pos = builder->CreateAdd(pos, extract_value(param[0], 2));
				llvm::Value* target_type_pos = TARGET_BITS == 64 ? builder->CreateZExt(pos, llvmType(getInt64Ty)) : pos;
				llvm::Value* addr = builder->CreateGEP(strptr, target_type_pos);
				return builder->CreateZExt(builder->CreateLoad(addr), llvmType(getInt32Ty));
			}, "empty_string", "not_empty_string", "Asc_done");
			release(param[0], args[0]->type);
			c_SP(stack_diff);
			if (on_stack)
				set_top_value(ret, T_INTEGER);
			return ret;
		}
		
		case 0x4B: //InStr
		case 0x4C: //RInStr
			codegen_stack
			SUBR_CODE(SUBR_instr)
		
		case 0x4D: //Subst
			codegen_stack
			SUBR_CODE(SUBR_subst)
		
		case 0x4E: //Replace
			codegen_stack
			SUBR_CODE(SUBR_replace)
		
		case 0x4F: //Split
			codegen_stack
			SUBR_CODE(SUBR_split)
		
		case 0x50: { //Scan
			codegen_value
			auto str = get_string_len(param[0]);
			auto pat = get_string_len(param[1]);
			
			llvm::Value* arr = builder->CreateCall4(get_global_function_jif(OBJECT_create, 'p', "pppi"), get_global((void*)GB.FindClass("String[]"), llvmType(getInt8Ty)), get_nullptr(), get_nullptr(), getInteger(32, 0));
			
			gen_if(builder->CreateAnd(
				builder->CreateICmpNE(str.second, getInteger(32, 0)),
				builder->CreateICmpNE(pat.second, getInteger(32, 0))), [&](){
					builder->CreateCall5(get_global_function_jif(REGEXP_scan, 'c', "ppipi"), arr, pat.first, pat.second, str.first, str.second);
			}, "scan_ok_params", "scan_empty_params");
			
			llvm::Value* ret = get_new_struct(object_type, get_global((void*)GB.FindClass("String[]"), llvmType(getInt8Ty)), arr);
			borrow_object_no_nullcheck(arr);
			release(param[0], args[0]->type);
			release(param[1], args[1]->type);
			
			c_SP(-args[0]->on_stack-args[1]->on_stack+on_stack);
			if (on_stack)
				set_top_value(ret, (TYPE)(void*)GB.FindClass("String[]"));
			return ret;
		}
		
		case 0x51:  //Comp
			codegen_stack
			SUBR_CODE(SUBR_strcomp)
		
		case 0x52: { //Conv
			codegen_value
			auto str = get_string_len(param[0]);
			llvm::Value* src = get_string(param[1]);
			llvm::Value* dst = get_string(param[2]);
			
			llvm::Value* llvm_args[6] = {builder->CreateBitCast(temp_voidptr, llvmType(getInt8PtrTy)), str.first, str.second, src, dst, getInteger(8, false)};
			llvm::Value* conv_error = builder->CreateCall(get_global_function_jif(STRING_conv, 'i', "ppippc"), llvm_args);
			release(param[0], args[0]->type);
			release(param[1], args[1]->type);
			release(param[2], args[2]->type);
			c_SP(-args[0]->on_stack-args[1]->on_stack-args[2]->on_stack+on_stack);
			
			gen_if_noreturn(builder->CreateICmpNE(conv_error, getInteger(32, 0)), [&](){
				create_throw(conv_error);
			}, "iconv_error", "iconv_ok");
			
			llvm::Value* result = builder->CreateLoad(temp_voidptr);
			
			llvm::Value* ret = gen_if_else_phi(builder->CreateICmpEQ(result, get_nullptr()), [&](){
				return get_new_struct(string_type, getInteger(TARGET_BITS, T_CSTRING), get_nullptr(), getInteger(32, 0), getInteger(32, 0));
			}, [&](){
				borrow_string_no_nullcheck(result); //FIXME no nullcheck ok?
				llvm::Value* len = builder->CreateLoad(builder->CreateBitCast(builder->CreateGEP(result, getInteger(TARGET_BITS, -4)), llvmType(getInt32PtrTy)));
				return get_new_struct(string_type, getInteger(TARGET_BITS, T_STRING), result, getInteger(32, 0), len);
			});
			if (on_stack)
				set_top_value(ret, T_STRING);
			return ret;
		}
		
		case 0x53: { //DConv, SConv
			/*auto str = get_string_len(param[0]);
			llvm::Value* src = builder->CreateIntToPtr(getInteger(TARGET_BITS, -2), llvmType(getInt8PtrTy));
			llvm::Value* dst = get_global(;*/
			codegen_stack
			SUBR_CODE(SUBR_sconv)
		}
		
		case 0x54: //Abs
			if (type != T_VARIANT){
				codegen_value
				llvm::Value* ret;
				switch(type){
					#define ab(bits) ret = builder->CreateSelect(builder->CreateICmpSLT(param[0], getInteger(bits, 0)), builder->CreateSub(getInteger(bits, 0), param[0]), param[0]); break;
					#define fab(zero) ret = builder->CreateSelect(builder->CreateFCmpULT(param[0], getFloat(zero)), builder->CreateFSub(getFloat(zero), param[0]), param[0]); break;
					case T_BOOLEAN:
					case T_BYTE: ret = param[0]; break;
					case T_SHORT: ab(16)
					case T_INTEGER: ab(32)
					case T_LONG: ab(64)
					case T_SINGLE: fab(0.0f)
					case T_FLOAT: fab(0.0)
					#undef ab
					#undef fab
					default: abort();
				}
				c_SP(-args[0]->on_stack+on_stack);
				if (on_stack)
					set_top_value(ret, type, args[0]->on_stack ? false : true);
				return ret;
			} else {
				//store_pc(pc); Should not be needed...
				codegen_stack
				SUBR_CODE(SUBR_abs)
			}
		
		case 0x55: //Int
			if (type != T_VARIANT){
				codegen_value
				llvm::Value* ret = param[0];
				if (type == T_SINGLE)
					ret = builder->CreateCall(get_global_function(floorf, 'f', "f"), param[0]);
				else if (type == T_FLOAT)
					ret = builder->CreateCall(get_global_function(floor, 'd', "d"), param[0]);
				c_SP(-args[0]->on_stack+on_stack);
				if (on_stack)
					set_top_value(ret, type, args[0]->on_stack ? false : true);
				return ret;
			} else {
				codegen_stack
				SUBR_CODE(SUBR_int)
			}
		
		case 0x56: //Fix
			if (type != T_VARIANT){
				codegen_value
				llvm::Value* ret = param[0];
				if (type >= T_SINGLE){
					llvm::Value* zero = type == T_SINGLE ? getFloat(0.0f) : getFloat(0.0);
					llvm::Value* func1 = type == T_SINGLE ? get_global_function(floorf, 'f', "f") : get_global_function(floor, 'd', "d");
					llvm::Value* func2 = type == T_SINGLE ? get_global_function(fabsf, 'f', "f") : get_global_function(fabs, 'd', "d");
					ret = gen_if_else_phi(builder->CreateFCmpUGE(param[0], zero), [&](){
						return builder->CreateCall(func1, param[0]);
					}, [&](){
						return builder->CreateFSub(zero, builder->CreateCall(func1, builder->CreateCall(func2, param[0])));
					});
				}
				c_SP(-args[0]->on_stack+on_stack);
				if (on_stack)
					set_top_value(ret, type, args[0]->on_stack ? false : true);
				return ret;
			} else {
				codegen_stack
				SUBR_CODE(SUBR_fix)
			}
		
		case 0x57: //Sgn
			if (args[0]->type != T_VARIANT){
				llvm::Type* intTy = llvmType(getInt32Ty);
				static const int bits[] = {0, 1, 8, 16, 32, 64};
				static llvm::Value* const zero[] = {NULL, getInteger(1, 0), getInteger(8, 0), getInteger(16, 0), getInteger(32, 0), getInteger(64, 0), getFloat(0.0f), getFloat(0.0)};
				codegen_value
				llvm::Value* ret;
				TYPE t = args[0]->type;
				switch(t){
					case T_BOOLEAN: ret = builder->CreateSExt(param[0], intTy); break;
					case T_BYTE: ret = builder->CreateZExt(builder->CreateICmpNE(param[0], zero[t]), intTy); break;
					case T_SHORT:
					case T_INTEGER:
					case T_LONG: {
						llvm::Value* lobit = builder->CreateAShr(param[0], bits[t]-1);
						if (t < T_INTEGER) lobit = builder->CreateSExt(param[0], intTy);
						else if (t > T_INTEGER) lobit = builder->CreateTrunc(param[0], intTy);
						ret = builder->CreateSelect(builder->CreateICmpSGT(param[0], zero[t]), getInteger(32, 1), lobit);
						break;
					}
					case T_SINGLE:
					case T_FLOAT: {
						ret = builder->CreateSelect(builder->CreateFCmpOGT(param[0], zero[t]), getInteger(32, 1),
								builder->CreateSelect(builder->CreateFCmpOLT(param[0], zero[t]), getInteger(32, -1), zero[T_INTEGER]));
						break;
					}
					default: __builtin_unreachable();
				}
				c_SP(-args[0]->on_stack+on_stack);
				if (on_stack)
					set_top_value(ret, type, args[0]->on_stack ? false : true);
				return ret;
			} else {
				codegen_stack
				SUBR_CODE(SUBR_sgn)
			}
		
		case 0x58: { //Math
			codegen_value
			llvm::Value* ret;
			if (extra == 1){
				//frac
				llvm::Value* x = builder->CreateCall(get_global_function(fabs, 'd', "d"), param[0]);
				ret = builder->CreateFSub(x, builder->CreateCall(get_global_function(floor, 'd', "d"), x));
			} else if (extra == 11){
				//deg
				ret = builder->CreateFDiv(builder->CreateFMul(param[0], getFloat(180.0)), getFloat(M_PI));
			} else if (extra == 12){
				//rad
				ret = builder->CreateFDiv(builder->CreateFMul(param[0], getFloat(M_PI)), getFloat(180.0));
			} else {
				#define f(name) get_global_function(name, 'd', "d")
				llvm::Value* functions[28] = {
					NULL, NULL, f(log), f(exp), f(sqrt), f(sin), f(cos), f(tan), f(atan),
					f(asin), f(acos), NULL, NULL, f(log10), f(sinh), f(cosh), f(tanh), f(asinh),
					f(acosh), f(atanh), f(exp2), f(exp10), f(log2), f(cbrt), f(expm1), f(log1p),
					f(floor), f(ceil)
				};
				#undef f
				ret = builder->CreateCall(functions[extra], param[0]);
			}
			gen_if_noreturn(builder->CreateICmpEQ(builder->CreateCall(get_global_function(__finite, 'i', "d"), ret), getInteger(32, 0)), [&](){
				create_throw(E_MATH);
			});
			c_SP(-args[0]->on_stack+on_stack);
			if (on_stack)
				set_top_value(ret, type, args[0]->on_stack ? false : true);
			return ret;
		}
		
		case 0x59: { //Pi
			if (nargs == 0){
				if (on_stack)
					push_value(getFloat(M_PI), T_FLOAT);
				return getFloat(M_PI);
			} else {
				codegen_value
				llvm::Value* ret = builder->CreateFMul(getFloat(M_PI), param[0]);
				c_SP(-args[0]->on_stack+on_stack);
				if (on_stack)
					set_top_value(ret, T_FLOAT, args[0]->on_stack ? false : true);
				return ret;
			}
		}
		
		case 0x5A: { //Round
			codegen_value
			llvm::Value* val = nargs == 2 ? param[1] : getInteger(32, 0);
			llvm::Value* power = builder->CreateCall2(M->getOrInsertFunction("llvm.powi.f64", get_function_type('d', "di")), getFloat(10.0), val);
			
			llvm::Value* ret = builder->CreateFMul(
				builder->CreateCall(get_global_function(floor, 'd', "d"),
					builder->CreateFAdd(
						builder->CreateFDiv(param[0], power),
						getFloat(0.5)
					)
				), power
			);
			c_SP(-args[0]->on_stack-(nargs == 2 && args[1]->on_stack)+on_stack);
			if (on_stack)
				set_top_value(ret, T_FLOAT, args[0]->on_stack ? false : true);
			return ret;
		}
		
		case 0x5B: { //Randomize
			codegen_value
			llvm::Value* set = getInteger(8, nargs == 0 ? false : true);
			llvm::Value* seed = nargs == 0 ? getInteger(32, 0) : param[0];
			builder->CreateCall2(get_global_function_jif(randomize, 'v', "ci"), set, seed);
			c_SP(-(nargs && args[0]->on_stack)+on_stack);
			if (on_stack)
				set_top_value(NULL, T_VOID);
			return NULL;
		}
		
		case 0x5C: { //Rnd
			codegen_value
			llvm::Value *min, *max;
			int stackdiff = on_stack;
			if (nargs == 0){
				min = getFloat(0.0);
				max = getFloat(1.0);
			} else if (nargs == 1){
				min = getFloat(0.0);
				max = param[0];
				stackdiff -= args[0]->on_stack;
			} else if (nargs == 2){
				min = param[0];
				max = param[1];
				stackdiff -= args[0]->on_stack;
				stackdiff -= args[1]->on_stack;
			}
			llvm::Value* r = builder->CreateCall(get_global_function_jif(rnd, 'd', ""));
			llvm::Value* ret = builder->CreateFAdd(builder->CreateFMul(r, builder->CreateFSub(max, min)), min);
			c_SP(stackdiff);
			if (on_stack)
				set_top_value(ret, T_FLOAT, (nargs >= 1 && args[0]->on_stack) ? false : true);
			return ret;
		}
		
		case 0x5D: //Min
		case 0x5E: { //Max
			if (type != T_VARIANT){
				bool is_max = digit == 0x5E;
				codegen_value
				llvm::Value* ret;
				if (type == T_BOOLEAN || (type >= T_SHORT && type <= T_LONG))
					ret = (is_max ? gen_max : gen_min)(param[0], param[1]);
				else if (type == T_BYTE)
					ret = builder->CreateSelect(builder->CreateICmpULT(param[0], param[1]), is_max ? param[1] : param[0], is_max ? param[0] : param[1]);
				else if (type == T_FLOAT)
					ret = builder->CreateSelect(builder->CreateFCmpOLT(param[0], param[1]), is_max ? param[1] : param[0], is_max ? param[0] : param[1]);
				else if (type == T_DATE)
					ret = builder->CreateSelect(LessDate(param[0], param[1]), is_max ? param[0] : param[1], is_max ? param[1] : param[0]);
				c_SP(-args[0]->on_stack-args[1]->on_stack+on_stack);
				if (on_stack)
					set_top_value(ret, type, args[0]->on_stack ? false : true);
				return ret;
			} else {
				codegen_stack
				extra = digit << 8;
				SUBR_CODE(SUBR_min_max)
			}
		}
		
		case 0x5F: { //IIf
			//FIXME if a destructor throws an error, this does not work ...
			codegen_value
			llvm::Value* ret;
			if (args[1]->type == args[2]->type || (TYPE_is_string(args[1]->type) && TYPE_is_string(args[2]->type))){
				ret = builder->CreateSelect(param[0], param[1], param[2]);
				gen_if_else(param[0], [&](){
					release(param[2], args[2]->type);
				}, [&](){
					release(param[1], args[1]->type);
				}, "IIf_release_false_argument", "IIf_release_true_argument", "IIf_release_done");
				
				c_SP(-args[0]->on_stack-args[1]->on_stack-args[2]->on_stack+on_stack);
				if (on_stack)
					set_top_value(ret, type);
				return ret;
			} else {
				//Convert to variant
				
				c_SP(-args[0]->on_stack-args[1]->on_stack-args[2]->on_stack);
				
				args[1]->on_stack = false;
				args[2]->on_stack = false;
				
				ret = gen_if_else_phi(param[0], [&](){
					if (args[1]->type == type)
						return param[1];
					
					release(param[2], args[2]->type);
					
					return JIT_conv_to_variant(args[1], param[1], on_stack, NULL);
				}, [&](){
					if (args[2]->type == type)
						return param[2];
					
					release(param[1], args[1]->type);
					
					return JIT_conv_to_variant(args[2], param[2], on_stack, NULL);
				}, "IIf_then", "IIf_else");
				return ret;
			}
			break;
		}
		
		case 0x60: { //Choose
			llvm::Value* ret;
			if (nargs < 4){
				codegen_value
				ret = get_default(type);
			}
			if (nargs == 1){
				c_SP(-args[0]->on_stack+on_stack);
				if (on_stack)
					set_top_value(ret, type);
				return ret;
			} else if (nargs == 2){
				llvm::Value* cmp = builder->CreateICmpEQ(param[0], getInteger(32, 1));
				ret = builder->CreateSelect(cmp, param[1], ret);
				c_SP(-args[0]->on_stack-args[1]->on_stack+on_stack);
				if (on_stack)
					set_top_value(ret, type);
				gen_if(builder->CreateXor(cmp, getInteger(1, true)), [&](){
					release(param[1], args[1]->type);
				}, "release_1st_choose");
				return ret;
			} else if (nargs == 3){
				llvm::Value* index = builder->CreateSub(param[0], getInteger(32, 1));
				
				c_SP(-args[0]->on_stack-args[1]->on_stack-args[2]->on_stack);
				
				args[1]->on_stack = false;
				args[2]->on_stack = false;
				
				bool stack_already_set_in_conv = false;
				
				ret = gen_if_else_phi(builder->CreateICmpULT(index, getInteger(32, 2)), [&](){
					llvm::Value* r;
					if (args[1]->type == args[2]->type || (TYPE_is_string(args[1]->type) && TYPE_is_string(args[2]->type))){
						r = builder->CreateSelect(builder->CreateTrunc(index, llvmType(getInt1Ty)), param[2], param[1]);
						c_SP(on_stack);
						return r;
					} else {
						r = gen_if_else_phi(builder->CreateTrunc(index, llvmType(getInt1Ty)), [&](){
							return JIT_conv_to_variant(args[2], param[2], on_stack, NULL);
						}, [&](){
							return JIT_conv_to_variant(args[1], param[1], on_stack, NULL);
						});
						stack_already_set_in_conv = true;
						return r;
					}
				}, [&](){
					c_SP(on_stack);
					if (stack_already_set_in_conv && on_stack)
						set_top_value(ret, type);
					return ret;
				});
				
				if (!stack_already_set_in_conv && on_stack)
					set_top_value(ret, type);
				
				gen_if(builder->CreateICmpNE(param[0], getInteger(32, 1)), [&](){
					release(param[1], args[1]->type);
				}, "release_1st_choose");
				gen_if(builder->CreateICmpNE(param[0], getInteger(32, 2)), [&](){
					release(param[2], args[2]->type);
				}, "release_2st_choose");
				
				return ret;
			} else {
				codegen_stack
				SUBR_CODE(SUBR_choose)
			}
		}
		
		case 0x61: { //Array
			codegen_value
			if (nargs > 0){
				push_value(getInteger(32, nargs), T_INTEGER);
			}
			llvm::Value* arr = builder->CreateCall4(get_global_function_jif(OBJECT_create, 'p', "pppi"), get_global((void*)type, llvmType(getInt8Ty)), get_nullptr(), get_nullptr(), getInteger(32, nargs > 0 ? 1 : 0));
			borrow_object_no_nullcheck(arr);
			int element_size = TYPE_sizeof_memory(type2);
			int data_offset = offsetof(CARRAY, data);
			llvm::Value* data = builder->CreateLoad(builder->CreateBitCast(builder->CreateGEP(arr, getInteger(TARGET_BITS, data_offset)), charPP));
			for(int i=0; i<nargs; i++){
				llvm::Value* addr = builder->CreateGEP(data, getInteger(TARGET_BITS, i*element_size));
				variable_write(addr, param[i], args[i]->type);
			}
			llvm::Value* ret = get_new_struct(object_type, builder->CreateIntToPtr(getInteger(TARGET_BITS, type), llvmType(getInt8PtrTy)), arr);
			c_SP(stack_diff);
			if (on_stack)
				set_top_value(ret, type);
			return ret;
		}
		
		case 0x62: { //Math2
			codegen_value
			llvm::Value* ret;
			if (extra == 2){
				llvm::Value* temp = param[0];
				param[0] = param[1];
				param[1] = temp;
			}
			if (extra <= 2){
				//Atan2, Ang
				ret = builder->CreateCall2(get_global_function(atan2, 'd', "dd"), param[0], param[1]);
			} else {
				ret = builder->CreateCall(get_global_function(sqrt, 'd', "d"), builder->CreateFAdd(builder->CreateFMul(param[0], param[0]), builder->CreateFMul(param[1], param[1])));
			}
			gen_if_noreturn(builder->CreateICmpEQ(builder->CreateCall(get_global_function(__finite, 'i', "d"), ret), getInteger(32, 0)), [&](){
				create_throw(E_MATH);
			});
			c_SP(stack_diff);
			if (on_stack)
				set_top_value(ret, T_FLOAT);
			return ret;
		}
		
		case 0x63: { //IsAscii, IsLetter, ...
			codegen_value
			llvm::Value* orig_ptr = extract_value(param[0], 1);
			llvm::Value* ptr = builder->CreateGEP(orig_ptr, extract_value(param[0], 2));
			llvm::Value* endptr = builder->CreateGEP(ptr, to_target_int(extract_value(param[0], 3)));
			
			llvm::BasicBlock* from_block = builder->GetInsertBlock();
			llvm::BasicBlock* body_block = create_bb("IsChr...");
			llvm::BasicBlock* cont_block = create_bb("IsChr_done");
			builder->CreateCondBr(builder->CreateICmpNE(extract_value(param[0], 3), getInteger(32, 0)), body_block, cont_block);
			
			builder->SetInsertPoint(body_block);
			llvm::PHINode* current_ptr = builder->CreatePHI(llvmType(getInt8PtrTy), 2);
			llvm::Value* next = builder->CreateGEP(current_ptr, getInteger(TARGET_BITS, 1));
			current_ptr->addIncoming(ptr, from_block);
			current_ptr->addIncoming(next, body_block);
			llvm::Value* c = builder->CreateLoad(current_ptr);
			llvm::Value* res;
			switch(extra){
				case 1: //IsAscii:
					res = builder->CreateICmpEQ(builder->CreateAnd(c, getInteger(8, ~0x7F)), getInteger(8, 0));
					break;
				case 2: //IsLetter:
					res = builder->CreateOr(
						builder->CreateAnd(
							builder->CreateICmpSGE(c, getInteger(8, 'a')),
							builder->CreateICmpSLE(c, getInteger(8, 'z'))
						), builder->CreateAnd(
							builder->CreateICmpSGE(c, getInteger(8, 'A')),
							builder->CreateICmpSLE(c, getInteger(8, 'Z'))
						)
					); break;
				case 3: //IsLower:
					res = builder->CreateAnd(
						builder->CreateICmpSGE(c, getInteger(8, 'a')),
						builder->CreateICmpSLE(c, getInteger(8, 'z'))
					); break;
				case 4: //IsUpper:
					res = builder->CreateAnd(
						builder->CreateICmpSGE(c, getInteger(8, 'A')),
						builder->CreateICmpSLE(c, getInteger(8, 'Z'))
					); break;
				case 5: //IsDigit:
					res = builder->CreateAnd(
						builder->CreateICmpSGE(c, getInteger(8, '0')),
						builder->CreateICmpSLE(c, getInteger(8, '9'))
					); break;
				case 6: //IsHexa:
					res = builder->CreateOr(
						builder->CreateAnd(
							builder->CreateICmpSGE(c, getInteger(8, '0')),
							builder->CreateICmpSLE(c, getInteger(8, '9'))
						),
						builder->CreateOr(
							builder->CreateAnd(
								builder->CreateICmpSGE(c, getInteger(8, 'a')),
								builder->CreateICmpSLE(c, getInteger(8, 'f'))
							), builder->CreateAnd(
								builder->CreateICmpSGE(c, getInteger(8, 'A')),
								builder->CreateICmpSLE(c, getInteger(8, 'F'))
							)
						)
					); break;
				case 7: { //IsSpace:
					const char* conds = " \n\r\t\f\v";
					res = builder->CreateICmpEQ(c, getInteger(8, conds[0]));
					for(size_t i=1; i<strlen(conds); i++)
						res = builder->CreateOr(res, builder->CreateICmpEQ(c, getInteger(8, conds[i])));
					break;
				}
				case 8: //IsBlank:
					res = builder->CreateOr(builder->CreateICmpEQ(c, getInteger(8, 32)), builder->CreateICmpEQ(c, getInteger(8, '\t')));
					break;
				case 9: //IsPunct:
					res = builder->CreateAnd(
						builder->CreateICmpSGT(c, getInteger(8, 32)),
						builder->CreateXor(
							builder->CreateOr(
								builder->CreateOr(
									builder->CreateAnd(
										builder->CreateICmpSGE(c, getInteger(8, 'a')),
										builder->CreateICmpSLE(c, getInteger(8, 'z'))
									), builder->CreateAnd(
										builder->CreateICmpSGE(c, getInteger(8, 'A')),
										builder->CreateICmpSLE(c, getInteger(8, 'Z'))
									)
								),
								builder->CreateAnd(
									builder->CreateICmpSGE(c, getInteger(8, '0')),
									builder->CreateICmpSLE(c, getInteger(8, '9'))
								)
							),
							getInteger(1, 1)
						)
					);
					break;
			}
			llvm::Value* still_inside_string = builder->CreateICmpNE(next, endptr);
			builder->CreateCondBr(builder->CreateAnd(still_inside_string, res), body_block, cont_block);
			
			builder->SetInsertPoint(cont_block);
			res = create_phi(getInteger(1, false), from_block, res, body_block);
			release(param[0], args[0]->type);
			c_SP(stack_diff);
			if (on_stack)
				set_top_value(res, T_BOOLEAN);
			return res;
		}
		
		case 0x64: //Bit manipulation
			if (args[0]->type == T_VARIANT){
				codegen_stack
				SUBR_CODE(SUBR_bit)
			} else {
				codegen_value
				static const int bits[] = {0, 0, 8, 16, 32, 64};
				int nbits = bits[args[0]->type];
				gen_if_noreturn(builder->CreateICmpUGE(param[1], getInteger(32, nbits)), [&](){
					create_throw(E_ARG);
				}, "bit_out_of_range");
				llvm::Value* bt = nbits == 32 ? param[1] : (nbits == 64 ? builder->CreateZExt(param[1], llvmType(getInt64Ty)) : builder->CreateTrunc(param[1], param[0]->getType()));
				llvm::Value* res;
				switch(extra){
					case 1: //BClr
						res = builder->CreateAnd(param[0], builder->CreateXor(builder->CreateShl(getInteger(nbits, 1), bt), getInteger(nbits, -1)));
						break;
					case 2: //BSet
						res = builder->CreateOr(param[0], builder->CreateShl(getInteger(nbits, 1), bt));
						break;
					case 3: //BTst
						res = builder->CreateICmpNE(builder->CreateAnd(param[0], builder->CreateShl(getInteger(nbits, 1), bt)), getInteger(nbits, 0));
						break;
					case 4: //BChg:
						res = builder->CreateXor(param[0], builder->CreateShl(getInteger(nbits, 1), bt));
						break;
					case 5: //Asl:
						res = builder->CreateShl(param[0], bt);
						if (nbits != 8){
							llvm::Value* max_signed_int = getInteger(nbits, (1ULL << nbits)-1); //0x7F...
							llvm::Value* min_signed_int = getInteger(nbits, 1ULL << nbits); //0x80...
							res = builder->CreateOr(
								builder->CreateAnd(res, max_signed_int), builder->CreateAnd(param[0], min_signed_int)
							);
						}
						break;
					case 6: //Asr:
						res = builder->CreateAShr(param[0], bt);
						break;
					case 7: //Rol:
						res = builder->CreateOr(
							builder->CreateShl(param[0], bt), builder->CreateLShr(param[0], builder->CreateSub(getInteger(nbits, nbits), bt))
						);
						break;
					case 8: //Ror:
						res = builder->CreateOr(
							builder->CreateLShr(param[0], bt), builder->CreateShl(param[0], builder->CreateSub(getInteger(nbits, nbits), bt))
						);
						break;
					case 9: //Lsl:
						res = builder->CreateShl(param[0], bt);
						break;
					case 10: //Lsr:
						res = builder->CreateLShr(param[0], bt);
						break;
				}
				c_SP(stack_diff);
				if (on_stack)
					set_top_value(res, type, extra == 3 || !args[0]->on_stack);
				return res;
			}
		
		case 0x65: //IsBoolean ...
			codegen_stack
			SUBR_CODE(SUBR_is_type)
		
		case 0x66: { //TypeOf
			codegen_value
			llvm::Value* ret;
			if (extra){ //Sizeof
				ret = gen_if_phi(getInteger(32, TARGET_BITS/8), builder->CreateICmpULT(param[0], getInteger(32, 16)), [&](){
					llvm::Value* s = builder->CreateLoad(builder->CreateGEP(get_global((void*)TYPE_sizeof_memory_tab, LONG_TYPE), to_target_int(param[0])));
					if (TARGET_BITS == 64)
						s = builder->CreateTrunc(s, llvmType(getInt32Ty));
					return s;
				}, "SizeOf");
				c_SP(stack_diff);
				if (on_stack)
					set_top_value(ret, T_INTEGER, !args[0]->on_stack);
			} else { //TypeOf
				TYPE t = args[0]->type;
				if (t == T_VARIANT){
					ret = extract_value(param[0], 0);
					if (TARGET_BITS == 64)
						ret = builder->CreateTrunc(ret, llvmType(getInt32Ty));
					llvm::BasicBlock *BB0 = builder->GetInsertBlock(), *BB1 = create_bb("TypeOf1"), *BB2 = create_bb("TypeOf2"), *BB3 = create_bb("TypeOf3");
					
					builder->CreateCondBr(builder->CreateICmpEQ(ret, getInteger(32, T_CSTRING)), BB3, BB1);
					builder->SetInsertPoint(BB1);
					builder->CreateCondBr(builder->CreateICmpSGE(ret, getInteger(32, T_OBJECT)), BB2, BB3);
					builder->SetInsertPoint(BB2);
					builder->CreateBr(BB3);
					builder->SetInsertPoint(BB3);
					llvm::PHINode* phi = builder->CreatePHI(llvmType(getInt32Ty), 3);
					phi->addIncoming(getInteger(32, T_OBJECT), BB2);
					phi->addIncoming(ret, BB1);
					phi->addIncoming(getInteger(32, T_STRING), BB0);
					ret = phi;
				} else if (t == T_CSTRING){
					ret = getInteger(32, T_STRING);
				} else if (TYPE_is_object(t) && t != T_NULL){
					ret = getInteger(32, T_OBJECT);
				} else {
					ret = getInteger(32, t);
				}
				c_SP(stack_diff);
				if (on_stack)
					set_top_value(ret, T_INTEGER, args[0]->type != T_INTEGER || !args[0]->on_stack);
			}
			release(param[0], args[0]->type);
			return ret;
		}
		
		case 0x68: //Bin
		case 0x69: //Hex
			codegen_stack
			SUBR_CODE(SUBR_hex_bin)
		
		case 0x6A: //Val
			codegen_stack
			SUBR_CODE(SUBR_val)
		
		case 0x6B: { //Str
			args[0]->must_on_stack();
			codegen_value
			llvm::Value* stack_top = get_value_on_top_addr();
			builder->CreateCall3(get_global_function_jif(VALUE_to_string, 'v', "ppp"),
				builder->CreateBitCast(stack_top, llvmType(getInt8PtrTy)),
				builder->CreateBitCast(temp_voidptr, llvmType(getInt8PtrTy)),
				builder->CreateBitCast(temp_int, llvmType(getInt8PtrTy))
			);
			builder->CreateCall3(get_global_function_jif(STRING_new_temp_value, 'v', "ppi"),
				builder->CreateBitCast(on_stack ? stack_top : temp_value, llvmType(getInt8PtrTy)),
				builder->CreateLoad(temp_voidptr),
				builder->CreateLoad(temp_int)
			);
			llvm::Value* str = read_value(on_stack ? stack_top : temp_value, T_STRING);
			release(param[0], args[0]->type);
			borrow_string(extract_value(str, 1)); //Nullcheck needed, might be empty string = null pointer
			if (!on_stack)
				c_SP(-1);
			return str;
		}
		
		case 0x6C: //Format
			codegen_stack
			SUBR_CODE(SUBR_format)
		
		case 0x6D: { //Timer
			builder->CreateCall2(get_global_function_jif(DATE_timer, 'v', "pi"),
				builder->CreateBitCast(temp_double, llvmType(getInt8PtrTy)),
				getInteger(32, 1));
			llvm::Value* ret = builder->CreateLoad(temp_double);
			if (on_stack)
				push_value(ret, T_FLOAT);
			return ret;
		}
		
		case 0x6E: { //Now
			c_SP(on_stack);
			llvm::Value* addr = on_stack ? get_value_on_top_addr() : temp_value;
			builder->CreateCall(get_global_function_jif(DATE_now, 'v', "p"),
				builder->CreateBitCast(addr, llvmType(getInt8PtrTy)));
			return read_value(addr, T_DATE);
		}
		
		case 0x6F: //Year
			codegen_stack
			SUBR_CODE(SUBR_year)
		
		case 0x70: //Week
			codegen_stack
			SUBR_CODE(SUBR_week)
		
		case 0x71: //Date
			codegen_stack
			SUBR_CODE(SUBR_date)
		
		case 0x72: //Time
			codegen_stack
			SUBR_CODE(SUBR_time)
		
		case 0x73: //DateAdd, DateDiff
			if (extra == 0){ //DateAdd
				if (on_stack)
					args[0]->must_on_stack();
				codegen_value
				c_SP(stack_diff);
				llvm::Value* addr = args[0]->on_stack ? get_value_on_top_addr() : temp_value;
				if (!args[0]>on_stack)
					store_value(temp_value, param[0], T_DATE, false /*not needed*/);
				builder->CreateCall3(get_global_function_jif(DATE_add, 'v', "pii"),
					builder->CreateBitCast(addr, llvmType(getInt8PtrTy)),
					param[1], param[2]);
				return read_value(addr, T_DATE);
			} else { //DateDiff
				codegen_value
				c_SP(stack_diff);
				
				llvm::Value* addr1 = args[0]->on_stack ? get_value_on_top_addr() : temp_value2;
				llvm::Value* addr2 = args[1]->on_stack ? (args[0]->on_stack ? builder->CreateGEP(addr1, getInteger(TARGET_BITS, 1)) : get_value_on_top_addr()) : temp_value;
				
				if (!args[0]->on_stack)
					store_value(temp_value2, param[0], T_DATE, false);
				if (!args[1]->on_stack)
					store_value(temp_value, param[1], T_DATE, false);
				
				llvm::Value* ret = builder->CreateCall3(get_global_function_jif(DATE_diff, 'i', "ppi"),
					builder->CreateBitCast(addr2, llvmType(getInt8PtrTy)),
					builder->CreateBitCast(addr1, llvmType(getInt8PtrTy)),
					param[2]);
				if (on_stack)
					set_top_value(ret, T_INTEGER);
				return ret;
			}
		
		case 0x74: //Eval
			codegen_stack
			SUBR_CODE(SUBR_eval)
		
		case 0x75: { //Error
			//llvm::Value* ret = builder->CreateICmpNE(read_global((void*)&EXEC_got_error, llvmType(getInt8Ty)), getInteger(8, 0));
			llvm::Value* ret = builder->CreateLoad(temp_got_error);
			if (on_stack)
				push_value(ret, T_BOOLEAN);
			return ret;
		}
		
		case 0x76: //Debug
			store_pc(pc);
			SUBR(SUBR_debug)
		
		case 0x77: //Wait
			codegen_stack
			SUBR_CODE(SUBR_wait)
		
		case 0x78: //Open
			codegen_stack
			SUBR_CODE(SUBR_open)
		
		case 0x79: //Close
			codegen_stack
			SUBR_CODE(SUBR_close)
		
		case 0x7A: //Input
			codegen_stack
			SUBR_CODE(SUBR_input)
		
		case 0x7B: //Line Input
			codegen_stack
			SUBR(SUBR_linput)
		
		case 0x7C: //Print
			codegen_stack
			SUBR_CODE(SUBR_print)
		
		case 0x7D: //Read
			codegen_stack
			SUBR_CODE(SUBR_read)
		
		case 0x7E: //Write
			codegen_stack
			SUBR_CODE(SUBR_write)
		
		case 0x7F: //Flush
			codegen_stack
			SUBR(SUBR_flush)
		
		case 0x80: //Lock
			codegen_stack
			SUBR(SUBR_lock)
		
		case 0x81: //Input From, Output To, Error To
			codegen_stack
			SUBR_CODE(SUBR_inp_out)
		
		case 0x82: //Eof
			codegen_stack
			SUBR_CODE(SUBR_eof)
		
		case 0x83: //Lof
			codegen_stack
			SUBR_CODE(SUBR_lof)
		
		case 0x84: //Seek
			codegen_stack
			SUBR_CODE(SUBR_seek)
			
		case 0x86: //Mkdir, deprecated -> Even() & Odd()
			if (extra != 0){
				codegen_value
				llvm::Value* val = builder->CreateTrunc(param[0], llvmType(getInt1Ty));
				if (extra == 1) //Even
					val = builder->CreateXor(val, getInteger(1, 1));
				c_SP(stack_diff);
				if (on_stack)
					set_top_value(val, T_BOOLEAN);
				return val;
			}
			//Else fallthrough: deprecated Kill(1)
		
		case 0x85: //Kill
		case 0x87: //Rmdir, deprecated
			if (extra == 0) extra = digit - 0x85;
			codegen_stack
			SUBR_CODE(SUBR_kill)
			
		case 0x8A: //Link, deprecated -> IsNan() & IsInf
			if (extra != 0){
				codegen_value
				if (extra == 1){
					//IsNan
					llvm::Value* res = builder->CreateICmpNE(builder->CreateCall(get_global_function(__isnan, 'i', "d"), param[0]), getInteger(32, 0));
					c_SP(stack_diff);
					if (on_stack)
						set_top_value(res, T_BOOLEAN);
					return res;
				} else if (extra == 2){
					//IsInf
					llvm::Value* res = builder->CreateCall(get_global_function(__isinf, 'i', "d"), param[0]);
					c_SP(stack_diff);
					if (on_stack)
						set_top_value(res, T_INTEGER);
					return res;
				}
			}
			//Else fallthrough: deprecated Link
		
		case 0x88: //Move
		case 0x89: //Copy, deprecated
			if (extra == 0) extra = digit - 0x88;
			codegen_stack
			SUBR_CODE(SUBR_move)
		
		case 0x8B: //Exist
			codegen_stack
			SUBR_CODE(SUBR_exist)
		
		case 0x8C: //Access
			codegen_stack
			SUBR_CODE(SUBR_access)
		
		case 0x8D: //Stat
			codegen_stack
			SUBR_CODE(SUBR_stat)
		
		case 0x8E: //Dfree
			codegen_stack
			SUBR(SUBR_dfree)
		
		case 0x8F: //Temp
			codegen_stack
			SUBR_CODE(SUBR_temp)
		
		case 0x90: //IsDir
			codegen_stack
			SUBR_CODE(SUBR_isdir)
		
		case 0x91: //Dir
			codegen_stack
			SUBR_CODE(SUBR_dir)
		
		case 0x92: //RDir
			codegen_stack
			SUBR_CODE(SUBR_rdir)
		
		case 0x93: //Exec
			codegen_stack
			SUBR_CODE(SUBR_exec)
		
		case 0x94: //Alloc
			codegen_stack
			SUBR_CODE(SUBR_alloc)
		
		case 0x95: //Free
			codegen_stack
			SUBR(SUBR_free)
		
		case 0x96: //Realloc
			codegen_stack
			SUBR_CODE(SUBR_realloc)
		
		case 0x97: //StrPtr
			codegen_stack
			SUBR_CODE(SUBR_strptr)
		
		case 0x98: { //Sleep
			codegen_value
			store_element(temp_2longs, 0, builder->CreateFPToSI(param[0], LONG_TYPE));
			llvm::Value* fabs_ret = builder->CreateCall(get_global_function(fabs, 'd', "d"), param[0]);
			llvm::Value* floor_ret = builder->CreateCall(get_global_function(floor, 'd', "d"), fabs_ret);
			llvm::Value* nsec = builder->CreateFMul(builder->CreateFSub(fabs_ret, floor_ret), getFloat(1000000000.0));
			store_element(temp_2longs, 1, builder->CreateFPToSI(nsec, LONG_TYPE));
			
			llvm::BasicBlock *BB1 = create_bb("nanosleep_loop"), *BB2 = create_bb("nanosleep_cont");
			
			llvm::Value* ptr = builder->CreateBitCast(temp_2longs, llvmType(getInt8PtrTy));
			
			builder->CreateBr(BB1);
			builder->SetInsertPoint(BB1);
			llvm::Value* res = builder->CreateCall2(get_global_function(nanosleep, 'i', "pp"), ptr, ptr);
			builder->CreateCondBr(builder->CreateICmpSLT(res, getInteger(32, 0)), BB1, BB2);
			
			builder->SetInsertPoint(BB2);
			
			if (on_stack)
				push_value(NULL, T_VOID);
			return NULL;
		}
		
		case 0x99: { //VarPtr
			auto pie = (PushIntegerExpression*)args[0];
			
			llvm::Value* ret;
			
			ushort op = pie->i;
			int index = op & 0xFF;
			if ((op & 0xFF00) == C_PUSH_LOCAL){
				CLASS_LOCAL* var = &FP->local[index];
				TYPE t = ctype_to_type(&var->type);
				if (TYPE_is_string(t)){
					ret = builder->CreateGEP(load_element(locals[index], 1), to_target_int(load_element(locals[index], 2)));
				} else {
					ret = builder->CreateBitCast(locals[index], llvmType(getInt8PtrTy));
				}
			} else if ((op & 0xF800) == C_PUSH_DYNAMIC) {
				CLASS_VAR* var = &CP->load->dyn[op & 0x7FF];
				ret = builder->CreateGEP(current_op, getInteger(TARGET_BITS, var->pos));
			} else if ((op & 0xF800) == C_PUSH_STATIC) {
				CLASS_VAR* var = &CP->load->stat[op & 0x7FF];
				ret = builder->CreateGEP(get_global((void*)CP->stat, llvmType(getInt8Ty)), getInteger(TARGET_BITS, var->pos));
			}
			if (on_stack)
				push_value(ret, T_POINTER);
			return ret;
		}
		
		case 0x9A: //Collection
			codegen_stack
			SUBR_CODE(SUBR_collection)
		
		case 0x9B: //Tr
			codegen_stack
			SUBR(SUBR_tr)
		
		case 0x9C: //Quote, Shell, Html
			codegen_stack
			SUBR_CODE(SUBR_quote)
		
		case 0x9D: //Unquote
			codegen_stack
			SUBR_CODE(SUBR_unquote)
		
		case 0x9E: //MkInteger, ...
			codegen_value
			if (extra <= T_BYTE){
				llvm::Value* offset = builder->CreateShl(builder->CreateZExt(param[0], LONG_TYPE), getInteger(TARGET_BITS, 1));
				
				llvm::Value* ret = get_new_struct(string_type, getInteger(TARGET_BITS, T_CSTRING),
					builder->CreateGEP(builder->CreateIntToPtr(getInteger(TARGET_BITS, (int64_t)(void*)&STRING_char_string[0]), llvmType(getInt8PtrTy)), offset),
					getInteger(32, 0), getInteger(32, 1));
				
				c_SP(stack_diff);
				if (on_stack)
					set_top_value(ret, T_CSTRING);
				return ret;
			} else {
				static const int sizes[] = {0, 1, 1, 2, 4, 8, 4, 8, 8, 0, 0, TARGET_BITS/8};
				c_SP(stack_diff);
				llvm::Value* stack_top = get_value_on_top_addr();
				builder->CreateCall3(get_global_function_jif(STRING_new_temp_value, 'v', "ppi"),
					builder->CreateBitCast(stack_top, llvmType(getInt8PtrTy)),
					get_nullptr(),
					getInteger(32, sizes[extra]));
				
				llvm::Value* ret = read_value(stack_top, T_STRING);
				llvm::Value* ptr = extract_value(ret, 1);
				
				borrow_string_no_nullcheck(ptr);
				
				builder->CreateStore(param[0], builder->CreateBitCast(ptr, pointer_t(param[0]->getType())));
				return ret;
			}
			break;
		
		case 0x9F: //Ptr
			codegen_stack
			SUBR_CODE(SUBR_ptr)
		
		default: assert(false && "Subr not implemented yet");
	}
	return ret_top_stack(type, on_stack);
}
void SubrExpression::codegen_on_stack(){
	codegen_get_value();
}

void stack_corrupted_abort(){
	fprintf(stderr, "Stack became corrupted in a JIT function. Please make a bug report.\n");
	abort();
}

void NopExpression::codegen(){
	/*builder->CreateCall4(get_global_function_vararg(printf, 'v', "p"),
		get_global((void*)"Nu: %s %p %p\n", llvmType(getInt8Ty)),
		get_global((void*)buf, llvmType(getInt8Ty)),
		read_global((void*)&SP),
		read_global((void*)&BP));
	builder->CreateCall4(get_global_function_vararg(printf, 'v', "p"),
		get_global((void*)"Nu: %d %p %p\n", llvmType(getInt8Ty)),
		getInteger(32, FP->n_local + FP->n_ctrl),
		read_global((void*)&SP),
		read_global((void*)&BP));*/
	
	if (test_stack){
#ifndef GOSUB_ON_STACK
		llvm::Value* sp = read_global((void*)&SP);
		llvm::Value* bp = read_global((void*)&BP);
		bp = builder->CreateGEP(bp, getInteger(TARGET_BITS, sizeof(VALUE)*(FP->n_local+FP->n_ctrl)));
#else
		llvm::Value* sp = builder->CreateBitCast(read_global((void*)&SP), pointer_t(value_type));
		llvm::Value* bp = builder->CreateLoad(gp);
#endif
		gen_if_noreturn(builder->CreateICmpNE(bp, sp), [&](){
			builder->CreateCall(get_global_function(stack_corrupted_abort, 'v', ""));
			builder->CreateUnreachable();
		});
	}
}

void ProfileLineExpression::codegen(){
	gen_if(builder->CreateICmpNE(read_global((void*)&EXEC_profile_instr, llvmType(getInt8Ty)), getInteger(8, false)), [&](){
		builder->CreateCall3(get_global_function_jif(DEBUG_Profile_Add, 'v', "ppp"),
			get_global((void*)CP, llvmType(getInt8Ty)),
			get_global((void*)FP, llvmType(getInt8Ty)),
			get_global((void*)pc, llvmType(getInt8Ty)));
	});
}

void StopEventExpression::codegen(){
	builder->CreateStore(getInteger(8, true), get_global((void*)&GAMBAS_StopEvent, llvmType(getInt8Ty)));	
}

void ReturnExpression::codegen(){
	auto normal_return = [&](){
		llvm::Value* ret;
		if (retval){
			ret = retval->codegen_get_value();
			if (retval->on_stack)
				c_SP(-1);
		} else {
			ret = get_default(type);
		}
		store_value(get_global((void*)RP), ret, type);
		if (in_try)
			builder->CreateCall(get_global_function(JR_end_try, 'v', "p"),
				create_gep(temp_errcontext1, TARGET_BITS, 0, TARGET_BITS, 0));
		if (EC != NULL){
			gen_if(builder->CreateXor(builder->CreateLoad(temp_got_error2), getInteger(1, 1)), [&](){
				llvm::Value* call = builder->CreateCall(get_global_function(JR_end_try, 'v', "p"),
					create_gep(temp_errcontext2, TARGET_BITS, 0, TARGET_BITS, 0));
				
				if (llvm::Instruction* inst = llvm::dyn_cast<llvm::Instruction>(call)){
					llvm::Value* arr[1] = {getInteger(32, 1)};
					inst->setMetadata("large_end_try", llvm::MDNode::get(llvm_context, arr));
				}
			}, "return_in_large_try");
		}
		return_points.push_back(builder->GetInsertBlock());
	};
	
	if (ngosubs == 0 || kind > 0){
		normal_return();
	} else {
		gen_if_noreturn(builder->CreateICmpEQ(builder->CreateLoad(gosub_return_point), getInteger(16, 0)), [&](){
			normal_return();
		});
		//Else return in Gosub
		gosub_return_points.push_back(builder->GetInsertBlock());
	}
	builder->SetInsertPoint(create_bb("dummy"));
}

void QuitExpression::codegen(){
	if (quitval){
		llvm::Value* val = quitval->codegen_get_value();
		if (quitval->on_stack)
			c_SP(-1);
		
		builder->CreateStore(val, get_global((void*)&EXEC_quit_value, llvmType(getInt8Ty)));
	}
	
	builder->CreateCall(get_global_function_jif(EXEC_quit, 'v', ""));
	builder->CreateUnreachable();
	builder->SetInsertPoint(create_bb("dummy"));
}

static void debug_print_line(){
	for (int i = 1; i < 10; i++)
		fputs("--------", stderr);
	fputc('\n', stderr);
}

static void run_optimizations(){
	bool changed = true;
	while(changed){
		llvm::FunctionPassManager FPM(M);
		llvm::PassManager MPM;
		llvm::PassManagerBuilder PMB;
		PMB.OptLevel = 2;
		PMB.SizeLevel = 1;
		PMB.populateFunctionPassManager(FPM);
		PMB.populateModulePassManager(MPM);
		
		//FPM.add(createGambasPass());
		
		FPM.doInitialization();
		FPM.run(*llvm_function);
		FPM.doFinalization();
		MPM.run(*M);
	
		llvm::FunctionPass* pass = createGambasPass();
		changed = pass->runOnFunction(*llvm_function);
		delete pass;
	}
}

template <typename T>
static void do_search(llvm::BasicBlock* block, T func){
	std::queue<llvm::BasicBlock*> q;
	std::set<llvm::BasicBlock*> vis;
	
	q.push(block);
	vis.insert(block);
	
	while(!q.empty()){
		llvm::BasicBlock* BB = q.front(); q.pop();
		
		for(llvm::BasicBlock::iterator I = BB->begin(), E = BB->end(); I != E; ){
			if (func(I++))
				goto next_block_in_queue;
		}
		
		for(llvm::succ_iterator SI = succ_begin(BB), E = succ_end(BB); SI != E; ++SI){
			if (vis.count(*SI) == 0){
				q.push(*SI);
				vis.insert(*SI);
			}
		}
		next_block_in_queue:;
	}
}

static void fix_setjmp(llvm::BasicBlock* catch_block, llvm::BasicBlock* try_block, const char* end_try_string){
	std::set<llvm::AllocaInst*> vars;
	std::set<llvm::AllocaInst*> volatile_vars;
	std::vector<llvm::StoreInst*> stores;
	
	do_search(try_block, [&](llvm::Value* value){
		if (llvm::StoreInst* si = llvm::dyn_cast<llvm::StoreInst>(value)){
			if (llvm::AllocaInst* ai = llvm::dyn_cast<llvm::AllocaInst>(si->getPointerOperand())){
				vars.insert(ai);
				stores.push_back(si);
			}
		} else if (llvm::CallInst* ci = llvm::dyn_cast<llvm::CallInst>(value)){
			if (ci->hasMetadata() && ci->getMetadata(end_try_string))
				return true;
		}
		return false;
	});
	
	do_search(catch_block, [&](llvm::Value* value){
		if (llvm::LoadInst* li = llvm::dyn_cast<llvm::LoadInst>(value)){
			if (llvm::AllocaInst* ai = llvm::dyn_cast<llvm::AllocaInst>(li->getPointerOperand())){
				if (vars.count(ai) != 0){
					li->setVolatile(true);
					volatile_vars.insert(ai);
				}
			}
		}
		return false;
	});
	
	for(size_t i=0, e=stores.size(); i!=e; i++){
		llvm::StoreInst* si = stores[i];
		llvm::AllocaInst* ai = llvm::dyn_cast<llvm::AllocaInst>(si->getPointerOperand());
		if (volatile_vars.count(ai) != 0){
			si->setVolatile(true);
		}
	}
}

static void fix_setjmps(){
	if (!has_tries)
		return;
	
	if (EC != NULL){
		//There is a Catch/Finally in the function, so a setjmp is somewhere in the entry block
		//and the first branch is a conditional branch: true -> catch part, false -> try part
		
		llvm::BranchInst* br = llvm::dyn_cast<llvm::BranchInst>(entry_block->getTerminator());
		assert(br && br->isConditional());
		
		llvm::BasicBlock* catch_block = br->getSuccessor(0);
		llvm::BasicBlock* try_block = br->getSuccessor(1);
		
		fix_setjmp(catch_block, try_block, "large_end_try");
	}
	
	for(size_t i=0, e=try_blocks.size(); i!=e; i++){
		llvm::BasicBlock* BB = try_blocks[i];
		
		llvm::BranchInst* br = llvm::dyn_cast<llvm::BranchInst>(BB->getTerminator());
		assert(br && br->isConditional());
		
		llvm::BasicBlock* catch_block = br->getSuccessor(0);
		llvm::BasicBlock* try_block = br->getSuccessor(1);
		
		fix_setjmp(catch_block, try_block, "end_try");
	}
}


struct DynamicAllocatedString {
	char* data;
	int len;
	DynamicAllocatedString(const char* data, int len){
		this->data = new char[len];
		this->len = len;
		memcpy(this->data, data, len);
	}
	~DynamicAllocatedString(){
		delete[] data;
	}
};

static std::vector<DynamicAllocatedString> extern_signature_strings;
static std::map<llvm::StringRef, void(*)(void*, void*)> extern_signatures;

static void func_extern_call_variant_vararg(void* return_value_addr, void* func_addr, int nargs, TYPE return_type){
	std::map<llvm::StringRef, void(*)(void*, void*)>& signatures = extern_signatures;
	
	char signature_string[nargs+sizeof(TYPE)];
	
	*(TYPE*)&signature_string[nargs] = return_type;
	
	for(int i=0; i<nargs; i++){
		TYPE t = SP[-nargs+i].type;
		if (t == T_VARIANT){
			JIF.F_VALUE_undo_variant(&SP[-nargs+i]);
			t = SP[-nargs+i].type;
		}
		signature_string[i] = TYPE_is_pure_object(t) ? T_OBJECT : t;
	}
	
	auto it = signatures.find(llvm::StringRef(signature_string, nargs+sizeof(TYPE)));
	
	if (it == signatures.end()){
		M = new llvm::Module("jit_mod_vararg_variant_extern", llvm_context);
		if (TARGET_BITS == 64){
			M->setTargetTriple("x86_64-pc-linux-gnu");
			M->setDataLayout("e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64");
		} else {
			M->setTargetTriple("i386-unknown-linux-gnu");
			M->setDataLayout("e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:32:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32-S128");
		}
		EE->addModule(M);
		
		static char buf[256] = "extern_func_caller_";
		int eob = strlen("extern_func_caller_");
		for(int i=0; i<nargs; i++)
			buf[eob+i] = signature_string[i]+'A';
		buf[eob+nargs] = '\0';
		
		//func_addr, return_value_addr
		llvm_function = llvm::cast<llvm::Function>(M->getOrInsertFunction(buf, get_function_type('v', "pp", true)));
		llvm::Function::arg_iterator arg_it = llvm_function->arg_begin();
		llvm::Value* func_addr_arg = arg_it++;
		llvm::Value* return_value_addr_arg = arg_it;
		
		entry_block = create_bb("entry");
		builder = new llvm::IRBuilder<>(entry_block);
		
		std::vector<llvm::Type*> ft;
		std::vector<llvm::Value*> orig_args;
		std::vector<llvm::Value*> func_args;
		ft.resize(nargs);
		orig_args.resize(nargs);
		func_args.resize(nargs);
		
		llvm::Value* SP_base = builder->CreateGEP(read_sp(), getInteger(TARGET_BITS, -nargs));
		llvm::Value* SP_current = SP_base;
		
		for(int i=0; i<nargs; i++){
			ft[i] = extern_types[signature_string[i]];
			orig_args[i] = read_value(SP_current, signature_string[i]);
			func_args[i] = codegen_extern_manage_value(orig_args[i], SP[-nargs+i].type);
			
			if (i != nargs-1)
				SP_current = builder->CreateGEP(SP_current, getInteger(TARGET_BITS, 1));
		}
		
		//Call function
		llvm::Type* function_type = llvm::FunctionType::get(extern_types[return_type > T_OBJECT ? T_OBJECT : return_type], ft, true);
		llvm::Value* call_function = builder->CreateBitCast(func_addr_arg, pointer_t(function_type));
		
		llvm::Value* ret = builder->CreateCall(call_function, func_args);
		
		//Manage return value
		ret = codegen_extern_manage_return_value(ret, return_type);
		if (return_type != T_VOID)
			builder->CreateStore(ret, builder->CreateBitCast(return_value_addr_arg, pointer_t(TYPE_llvm(return_type))));
		
		//Release arguments
		for(int i=nargs; i --> 0; ){
			release(orig_args[i], signature_string[i]);
			c_SP(-1);
		}
		
		builder->CreateRetVoid();
		
		//M->dump();
		
		llvm::verifyModule(*M);
		
		llvm::FunctionPassManager FPM(M);
		llvm::PassManager MPM;
		llvm::PassManagerBuilder PMB;
		PMB.OptLevel = 2;
		PMB.SizeLevel = 1;
		PMB.populateFunctionPassManager(FPM);
		PMB.populateModulePassManager(MPM);
		
		FPM.doInitialization();
		FPM.run(*llvm_function);
		FPM.doFinalization();
		MPM.run(*M);
		
		//Print out the code after optimization
		if (MAIN_debug){
			debug_print_line();
			fprintf(stderr, "gb.jit: dumping vararg extern call function\n");
			debug_print_line();
			M->dump();
			debug_print_line();
			fputc('\n', stderr);
		}
		
		void (*fn)(void*, void*) = (void(*)(void*, void*))EE->getPointerToFunction(llvm_function);
		
		delete builder;
		
		llvm_function->deleteBody();
		mappings.clear();
		
		extern_signature_strings.emplace_back((char*)signature_string, nargs+sizeof(TYPE));
		signatures.insert(std::pair<llvm::StringRef, void(*)(void*, void*)>(llvm::StringRef(extern_signature_strings.back().data, nargs+sizeof(TYPE)), fn));
		(*fn)(func_addr, return_value_addr);
	} else {
		(*(it->second))(func_addr, return_value_addr);
	}
}

static void func_void(){
	RP->type = T_VOID;
	JIF.F_EXEC_leave_keep();
}
static void func_boolean(){
	RP->type = T_BOOLEAN;
	RP->_boolean.value = 0;
	JIF.F_EXEC_leave_keep();
}
static void func_byte(){
	RP->type = T_BYTE;
	RP->_byte.value = 0;
	JIF.F_EXEC_leave_keep();
}
static void func_short(){
	RP->type = T_SHORT;
	RP->_short.value = 0;
	JIF.F_EXEC_leave_keep();
}
static void func_integer(){
	RP->type = T_INTEGER;
	RP->_integer.value = 0;
	JIF.F_EXEC_leave_keep();
}
static void func_long(){
	RP->type = T_LONG;
	RP->_long.value = 0;
	JIF.F_EXEC_leave_keep();
}
static void func_single(){
	RP->type = T_SINGLE;
	RP->_single.value = 0.0f;
	JIF.F_EXEC_leave_keep();
}
static void func_float(){
	RP->type = T_FLOAT;
	RP->_float.value = 0.0;
	JIF.F_EXEC_leave_keep();
}
static void func_date(){
	RP->type = T_DATE;
	RP->_date.date = 0;
	RP->_date.time = 0;
	JIF.F_EXEC_leave_keep();
}
static void func_string(){
	RP->type = T_CSTRING;
	RP->_string.addr = NULL;
	RP->_string.start = 0;
	RP->_string.len = 0;
	JIF.F_EXEC_leave_keep();
}
static void func_pointer(){
	RP->type = T_POINTER;
	RP->_pointer.value = NULL;
	JIF.F_EXEC_leave_keep();
}
static void func_variant(){
	RP->type = T_VARIANT;
	RP->_variant.vtype = T_NULL;
	JIF.F_EXEC_leave_keep();
}
static void func_object(){
	RP->type = T_OBJECT;
	RP->_object.object = NULL;
	JIF.F_EXEC_leave_keep();
}

void JIT_codegen(){
	if (FP->type <= T_OBJECT && all_statements.size() == 1){
		if (auto re = dyn_cast<ReturnExpression>(all_statements.front()->expr)){
			if (re->retval == NULL){
				//Standard empty function
				
				delete all_statements.front();
				all_statements.clear();
				
				static void (* const funcs[17])(void) = {func_void, func_boolean, func_byte, func_short, func_integer,
					func_long, func_single, func_float, func_date, func_string, func_string,
					func_pointer, func_variant, NULL, NULL, NULL, func_object};
				CP->jit_functions[EXEC.index] = funcs[FP->type];
				
				return;
			}
		}
	}
	
	static bool inited = false;
	if (!inited){
		llvm_init();
		inited = true;
	}
	
	M = new llvm::Module("jit_mod", llvm_context);
	if (TARGET_BITS == 64){
		M->setTargetTriple("x86_64-pc-linux-gnu");
		M->setDataLayout("e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64");
	} else {
		M->setTargetTriple("i386-unknown-linux-gnu");
		M->setDataLayout("e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:32:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32-S128");
	}
	if (EE)
		EE->addModule(M);
	else
		EE = llvm::EngineBuilder(M)/*.setOptLevel(llvm::CodeGenOpt::Aggressive)*/.create();
	
	
	static int counter = 0;
	char buf[256];
	sprintf(buf, "func_%d_%s_%d", counter++, CP->name, EXEC.index);
	llvm_function = llvm::cast<llvm::Function>(M->getOrInsertFunction(buf, get_function_type('v', "")));
	
	entry_block = create_bb("entry");
	builder = new llvm::IRBuilder<>(entry_block);
	
	//builder->CreateCall(get_global_function((void*)&puts, 'v', "p"), get_global((void*)"hej", llvmType(getInt8Ty)));
	//builder->CreateCall2(get_global_function_vararg(printf, 'v', "p"), get_global((void*)"enter %d\n", llvmType(getInt8Ty)), getInteger(32, counter-1));
	
	current_op = OP == NULL ? get_nullptr() : read_global((void*)&OP);
	temp_value = builder->CreateAlloca(value_type);
	temp_value2 = builder->CreateAlloca(value_type);
	temp_voidptr = builder->CreateAlloca(llvmType(getInt8PtrTy));
	temp_int = builder->CreateAlloca(llvmType(getInt32Ty));
	temp_double = builder->CreateAlloca(llvmType(getDoubleTy));
	temp_date = builder->CreateAlloca(date_type);
	temp_2longs = builder->CreateAlloca(two_longs_type);
	temp_errcontext1 = builder->CreateAlloca(llvm::ArrayType::get(llvmType(getInt8Ty), sizeof(ERROR_CONTEXT)));
	temp_errcontext2 = builder->CreateAlloca(llvm::ArrayType::get(llvmType(getInt8Ty), sizeof(ERROR_CONTEXT)));
	temp_got_error = builder->CreateAlloca(llvmType(getInt1Ty));
	temp_got_error2 = builder->CreateAlloca(llvmType(getInt1Ty));
	if (ngosubs != 0){
		//temp_gosub_stack = builder->CreateAlloca(llvm::ArrayType::get(llvmType(getInt8PtrTy), ngosubs + 100));
		/*temp_num_gosubs_on_stack = builder->CreateAlloca(llvmType(getInt32Ty));
		builder->CreateStore(getInteger(32, 0), temp_num_gosubs_on_stack);*/
		
#ifndef GOSUB_ON_STACK
		builder->CreateCall3(get_global_function(GB.NewArray, 'v', "pii"),
			get_global(&GP, llvmType(getInt8Ty)), getInteger(32, sizeof(STACK_GOSUB)), getInteger(32, 0));
#endif
		
		/*gosub_return_point = builder->CreateAlloca(llvmType(getInt8PtrTy));
		builder->CreateStore(get_nullptr(), gosub_return_point);*/
		gosub_return_point = builder->CreateAlloca(llvmType(getInt16Ty));
		builder->CreateStore(getInteger(16, 0), gosub_return_point);
	}
	//For GoSubs and JR_try_unwind
	gp = builder->CreateAlloca(pointer_t(value_type));
		builder->CreateStore(builder->CreateGEP(read_global((void*)&BP, pointer_t(value_type)), getInteger(TARGET_BITS, FP->n_local + FP->n_ctrl)), gp);
	
	has_tries = false;
	try_blocks.clear();
	
	init_locals();
	
	codegen_statements();
	
	finish_gosub_returns();
	create_return();
	
	insert_pending_branches();
	//puts("Dump:");
	
	//Print out the code before optimization
	//M->dump();
	
	llvm::verifyModule(*M);
	
	for(size_t i=0, e=all_statements.size(); i!=e; i++)
		delete all_statements[i];
	all_statements.clear();
	
	fix_setjmps();
	
	run_optimizations();
	
	//Print out the code after optimization
	if (MAIN_debug){
		debug_print_line();
		fprintf(stderr, "gb.jit: dumping function %s.", CP->name);
		if (FP->debug)
			fprintf(stderr, "%s:\n", FP->debug->name);
		else
			fprintf(stderr, "%d:\n", EXEC.index);
		debug_print_line();
		M->dump();
		debug_print_line();
		fputc('\n', stderr);
	}
	
	void (*fn)(void) = (void(*)(void))EE->getPointerToFunction(llvm_function);
	
	delete builder;
	
	llvm_function->deleteBody();
	
	CP->jit_functions[EXEC.index] = fn;
	/*if (CP->jit_data == NULL){
		ALLOC_ZERO(&CP->jit_data, sizeof(llvm::ExecutionEngine*) * CP->load->n_func, "JIT_codegen");
	}
	((llvm::ExecutionEngine**)CP->jit_data)[EXEC.index] = EE;*/
	
	mappings.clear();
	//variable_mappings.clear();
}


static void JIT_cleanup(llvm::ExecutionEngine** EE, int n_func){
	/*for(int i=0; i<n_func; i++){
		if (EE[i]){
			EE[i]->freeMachineCodeForFunction(EE[i]->FindFunctionNamed("func"));
			delete EE[i];
		}
	}*/
}

void JIT_end(){
	delete EE;
	llvm::llvm_shutdown();
}

///DEBUG
void print_type(llvm::Value* v){
	v->getType()->print(llvm::outs());
	puts("");
}
void print_expr_type(Expression* expr){
	puts(typeid(*expr).name());
}
