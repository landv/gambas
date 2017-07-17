/***************************************************************************

  jit.h

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

#ifndef __JIT_H
#define __JIT_H

#include <llvm/Config/llvm-config.h>

#if (LLVM_VERSION_MAJOR == 3 && LLVM_VERSION_MINOR >= 3)
	#include "llvm/IR/LLVMContext.h"
	#include "llvm/IR/Module.h"
	#include "llvm/IR/Constants.h"
	#include "llvm/IR/DerivedTypes.h"
	#include "llvm/IR/Intrinsics.h"
	#include "llvm/IR/Instructions.h"
#else
	#include "llvm/LLVMContext.h"
	#include "llvm/Module.h"
	#include "llvm/Constants.h"
	#include "llvm/DerivedTypes.h"
	#include "llvm/Intrinsics.h"
	#include "llvm/Instructions.h"
#endif

#if (LLVM_VERSION_MAJOR == 3 && LLVM_VERSION_MINOR >= 5)
	#include "llvm/IR/Verifier.h"
	#include "llvm/IR/CFG.h"
#else
	#include "llvm/Analysis/Verifier.h"
	#include "llvm/Support/CFG.h"
#endif

#include "llvm/ExecutionEngine/JIT.h"
#include "llvm/ExecutionEngine/Interpreter.h"
#include "llvm/ExecutionEngine/GenericValue.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/raw_ostream.h"

#if (LLVM_VERSION_MAJOR == 3 && LLVM_VERSION_MINOR >= 3)
	#include "llvm/IR/IRBuilder.h"
#elif (LLVM_VERSION_MAJOR == 3 && LLVM_VERSION_MINOR ==2)
	#include "llvm/IRBuilder.h"
#else
	#include "llvm/Support/IRBuilder.h"
#endif

#include "llvm/Support/DynamicLibrary.h"
#include "llvm/PassManager.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"

#include <typeinfo>
#include <vector>

extern "C" {
#define FALSE false
#define TRUE true

#include <unistd.h>
#include <sys/time.h>

#define class klass

#include "gb_common.h"
//#include "gb_error.h"
#include "gbx_type.h"

#include "gb_limit.h"
//#include "gbx_subr.h"
#include "gbx_stack.h"
//#include "gbx_exec.h"

#include "gbx_local.h"
#include "gbx_event.h"

#define throw _throw
#include "gbx_c_array.h"
#undef throw

#undef class
}

#include "main.h"

struct Expression;

void JIT_conv(Expression*& value, TYPE type, Expression* other = NULL);

void JIT_read(void);
void JIT_codegen(void);

void ref_stack(void);
void set_ctrl_type(TYPE type, int index, CLASS* second = NULL);
TYPE get_ctrl_type(int index);
CLASS* get_ctrl_class(int index);

int special_ctrl_type(TYPE type);
bool is_ctrl_type_used(int type, int index);
int special_ctrl_type(TYPE type);

struct Expression;
void register_new_expression(Expression*);
void free_all_expressions(void);

void mark_address_taken(int addr);

unsigned short* get_current_read_pos(void);

extern int ngosubs;
extern uint64_t func_byref_mask;

template <typename T>
static bool isa(Expression* expr){
	return typeid(*expr) == typeid(T);
}

template <typename T>
static T* dyn_cast(Expression* expr){
	if (typeid(*expr) == typeid(T))
		return (T*)expr;
	return NULL;
}

struct Expression {
	TYPE type;
	bool on_stack;
	bool stack_if_ref;
	bool no_ref_variant;
	
	void ref_on_stack(){
		stack_if_ref = true;
		if (type == T_STRING || type == T_OBJECT || type == T_VARIANT || TYPE_is_pure_object(type))
			on_stack = true;
	}
	void must_on_stack(){
		on_stack = true;
	}
	Expression(){
		type = T_VOID;
		on_stack = false;
		stack_if_ref = false;
		no_ref_variant = false;
		register_new_expression(this);
	}
	virtual void codegen(){
		puts(typeid(*this).name());
		assert(false && "Codegen not done yet for this type");
	}
	virtual llvm::Value* codegen_get_value(){
		puts(typeid(*this).name());
		assert(false && "codegen_get_value not done yet for this type");
	}
	virtual void codegen_on_stack(){
		puts(typeid(*this).name());
		assert(false && "codegen_on_stack not done yet for this type");
	}
	virtual ~Expression(){}
};

struct ConvExpression : Expression {
	Expression* expr;
	ConvExpression(Expression* expr, TYPE type) : expr(expr) {
		this->type = type;
	}
	llvm::Value* codegen_get_value();
	void codegen_on_stack(){ codegen_get_value(); }
};

enum FunctionType {
	PrivateFn,
	PureObjectFn,
	PureObjectStaticFn,
	VirtualObjectFn,
	VirtualObjectStaticFn,
	StaticFn,
	EventFn,
	
	ClassFn,
	ObjectAsFn
	//UnknownFn
};

struct FunctionExpression {
	llvm::Value* effective_class;
	CLASS* function_class;
	Expression* function_object;
	CLASS_DESC* function_desc;
	const char* function_unknown;
	char function_kind;
	char function_defined;
	short function_index;
	char function_expr_type; //Contains enum FunctionType
};

struct PushCStringExpression : Expression {
	char* addr;
	int start;
	int len;
	PushCStringExpression(char* a, int s, int l) : addr(a), start(s), len(l) {
		type = T_CSTRING;
	}
	
	llvm::Value* codegen_get_value();
	void codegen_on_stack(){ codegen_get_value(); }
};

//PushQuick, PushInteger
struct PushIntegerExpression : Expression {
	int bits;
	int64_t i;
	PushIntegerExpression(int bits, int64_t i) : bits(bits), i(i) {
		switch(bits){
			case 1: type = T_BOOLEAN; return;
			case 8: type = T_BYTE; return;
			case 16: type = T_SHORT; return;
			case 32: type = T_INTEGER; return;
			case 64: type = T_LONG; return;
		}
	}
	void codegen();
	llvm::Value* codegen_get_value();
	void codegen_on_stack();
};

struct AddQuickExpression : Expression {
	Expression* expr;
	int add;
	AddQuickExpression(Expression* val, int add);
	void codegen(){
		if (on_stack) codegen_on_stack();
	}
	llvm::Value* codegen_get_value();
	void codegen_on_stack(){
		codegen_get_value();
	}
};

struct PushFloatExpression : Expression {
	double val;
	PushFloatExpression(int bits, double val) : val(val) {
		type = bits == 32 ? T_SINGLE : T_FLOAT;
	}
	llvm::Value* codegen_get_value();
	void codegen_on_stack(){ codegen_get_value(); }
};

struct PushNullPointerExpression : Expression {
	PushNullPointerExpression() {
		type = T_POINTER;
	}
	llvm::Value* codegen_get_value();
	void codegen_on_stack(){ codegen_get_value(); }
};

struct PushVoidDateExpression : Expression {
	PushVoidDateExpression() {
		type = T_DATE;
	}
	llvm::Value* codegen_get_value();
	void codegen_on_stack(){ codegen_get_value(); }
};

struct PushNullExpression : Expression {
	PushNullExpression() {
		type = T_NULL;
	}
	llvm::Value* codegen_get_value();
	void codegen_on_stack(){ codegen_get_value(); }
};

struct PushVoidExpression : Expression {
	PushVoidExpression() {
		type = T_VOID;
	}
	llvm::Value* codegen_get_value();
	void codegen_on_stack(){ codegen_get_value(); }
};

struct PushLastExpression : Expression {
	PushLastExpression() {
		type = T_OBJECT;
	}
	llvm::Value* codegen_get_value();
	void codegen_on_stack(){ codegen_get_value(); }
};

struct PushComplexExpression : Expression {
	Expression* op;
	PushComplexExpression(Expression* op) : op(op) {
		JIT_conv(this->op, T_FLOAT);
		//FIXME
		abort();
	}
};

struct ReadVariableExpression : Expression {
	char* addr;
	CTYPE* ctype;
	CLASS* klass;
	llvm::Value* codegen_get_value();
	void codegen_on_stack(){ codegen_get_value(); }
};

static TYPE ctype_to_type(CTYPE* ctype, CLASS* klass = CP){
	TYPE type = ctype->id;
	if (ctype->id == TC_ARRAY){
		CTYPE* ctype2 = &klass->load->array[ctype->value]->ctype;
		type = (TYPE)(void*)JIF.F_CARRAY_get_array_class(klass, *ctype2);
	} else if (ctype->id == T_OBJECT || ctype->id == 14){
		if (ctype->id != T_OBJECT || ctype->value >= 0)
			type = (TYPE)(void*)klass->load->class_ref[ctype->value];
	}
	return type;
}

struct PushAutoCreateExpression : Expression {
	CLASS* klass;
	PushAutoCreateExpression(CLASS* klass) : klass(klass) {
		type = (TYPE)(void*)klass;
	}
	
	llvm::Value* codegen_get_value();
	void codegen_on_stack(){ codegen_get_value(); }
};

struct PushStaticExpression : ReadVariableExpression {
	PushStaticExpression(int index);
};

struct PopStaticExpression : Expression {
	Expression* val;
	char* addr;
	PopStaticExpression(Expression* val, int index);
	void codegen();
};

struct PushDynamicExpression : Expression {
	int index;
	int offset;
	CTYPE* ctype;
	PushDynamicExpression(int index);
	llvm::Value* codegen_get_value();
	void codegen_on_stack(){ codegen_get_value(); }
};

struct PopDynamicExpression : Expression {
	Expression* val;
	int index;
	int offset;
	PopDynamicExpression(Expression* val, int index);
	void codegen();
};

struct PushFunctionExpression : Expression, FunctionExpression {
	int index;
	PushFunctionExpression(int index);
	llvm::Value* codegen_get_value();
	void codegen_on_stack();
};

struct PushClassExpression : Expression {
	CLASS* klass;
	PushClassExpression(int index) {
		type = T_CLASS;
		klass = CP->load->class_ref[index];
		JIT_load_class(klass);
	}
	PushClassExpression(CLASS* klass) : klass(klass) {
		type = T_CLASS;
	}
	void codegen_on_stack();
};

struct PushLocalExpression : Expression {
	int index;
	PushLocalExpression(int index);
	llvm::Value* codegen_get_value();
	void codegen_on_stack();
};

struct PushParamExpression : Expression {
	int index;
	PushParamExpression(int index) : index(index) {
		type = FP->param[index + FP->n_param].type;
	}
	llvm::Value* codegen_get_value();
	void codegen_on_stack();
};

struct PopLocalExpression : Expression {
	Expression* val;
	int index;
	PopLocalExpression(Expression* val, int index);
	void codegen();
};

struct PopParamExpression : Expression {
	Expression* val;
	int index;
	PopParamExpression(Expression* val, int index);
	void codegen();
};

struct PopCtrlExpression : Expression {
	Expression* val;
	int index;
	PopCtrlExpression(Expression* val, int index);
	void codegen();
};

struct PopOptionalExpression : Expression {
	Expression* val;
	int index;
	bool is_default;
	PopOptionalExpression(Expression* val, int index);
	void codegen();
};

struct PushExternExpression : Expression {
	CLASS* klass;
	Expression* object_to_release;
	int index;
	bool must_variant_dispatch;
	PushExternExpression(CLASS* klass, int index, Expression* object_to_release)
	: klass(klass), object_to_release(object_to_release), index(index) {
		type = T_FUNCTION;
		must_variant_dispatch = false; //Might get changed later in CallExpression::CallExpression
	}
};

struct PushEventExpression : Expression, FunctionExpression {
	int index;
	PushEventExpression(int ind, const char* unknown_name);
};

struct PushArrayExpression : Expression {
	std::vector<Expression*> args;
	unsigned short* pc;
	bool can_quick;
	PushArrayExpression(Expression** it, int nargs);
	llvm::Value* codegen_get_value();
	void codegen_on_stack(){ codegen_get_value(); }
};

struct PopArrayExpression : Expression {
	std::vector<Expression*> args;
	Expression* val;
	unsigned short* pc;
	bool can_quick;
	PopArrayExpression(Expression** it, int nargs, Expression* val);
	void codegen();
};

struct PushSuperExpression : Expression {
	PushSuperExpression(){
		type = (TYPE)(void*)CP->parent;
		if (type == 0)
			THROW(E_PARENT);
	}
	llvm::Value* codegen_get_value();
	void codegen_on_stack(){ codegen_get_value(); }
};

struct PushMeExpression : Expression {
	PushMeExpression() {
		type = (TYPE)(void*)CP;
	}
	llvm::Value* codegen_get_value();
	void codegen_on_stack(){ codegen_get_value(); }
};

//Base class, not to be used directly
struct PushPureObjectExpression : Expression {
	Expression* obj;
	int index;
	PushPureObjectExpression(Expression* obj, int index) : obj(obj), index(index) {}
	CLASS* klass(){
		return (CLASS*)(void*)obj->type;
	}
	CLASS_DESC* desc(){
		CLASS* c = (CLASS*)(void*)obj->type;
		return c->table[index].desc;
	}
};

struct PushPureObjectConstantExpression : PushPureObjectExpression {
	PushPureObjectConstantExpression(Expression* obj, int index)
	: PushPureObjectExpression(obj, index){
		type = desc()->constant.type;
		ref_stack();
	}
	llvm::Value* codegen_get_value();
	void codegen_on_stack(){ codegen_get_value(); }
};

struct PushPureObjectVariableExpression : PushPureObjectExpression {
	PushPureObjectVariableExpression(Expression* obj, int index)
	: PushPureObjectExpression(obj, index){
		type = ctype_to_type(&desc()->variable.ctype, desc()->variable.klass);
		ref_stack();
		
	}
	llvm::Value* codegen_get_value();
	void codegen_on_stack(){ codegen_get_value(); }
};

/*struct PushPureObjectStaticVariableExpression : PushPureObjectExpression {
	PushPureObjectStaticVariableExpression(Expression* obj, int index)
	: PushPureObjectExpression(obj, index){
		type = ctype_to_type(&desc()->variable.ctype);
	}
};*/

struct PushPureObjectPropertyExpression : PushPureObjectExpression {
	PushPureObjectPropertyExpression(Expression* obj, int index)
	: PushPureObjectExpression(obj, index){
		type = desc()->property.type;
		ref_stack();
		obj->must_on_stack();
	}
	llvm::Value* codegen_private(bool get_value);
	llvm::Value* codegen_get_value(){ return codegen_private(true); }
	void codegen_on_stack(){ codegen_private(false); }
};

//The object expression must return null, but must still be executed
//This should only be used with virtual classes
//For example: Application.Env.Count
struct PushPureObjectStaticPropertyExpression : PushPureObjectExpression {
	const char* name;
	PushPureObjectStaticPropertyExpression(Expression* obj, int index, const char* name)
	: PushPureObjectExpression(obj, index), name(name) {
		type = desc()->property.type;
		ref_stack();
		obj->must_on_stack();
	}
	llvm::Value* codegen_private(bool get_value);
	llvm::Value* codegen_get_value(){ return codegen_private(true); }
	void codegen_on_stack(){ codegen_private(false); }
};

struct PushPureObjectStructFieldExpression : PushPureObjectExpression {
	PushPureObjectStructFieldExpression(Expression* obj, int index)
	: PushPureObjectExpression(obj, index){
		type = ctype_to_type(&desc()->variable.ctype, desc()->variable.klass);
	}
	llvm::Value* codegen_get_value();
	void codegen_on_stack(){ codegen_get_value(); }
};

struct PushPureObjectUnknownExpression : Expression {
	Expression* obj;
	const char* name;
	int name_id;
	PushPureObjectUnknownExpression(Expression* obj, const char* name, int name_id) : obj(obj), name(name), name_id(name_id) {
		type = T_VARIANT;
		ref_stack();
		obj->must_on_stack();
		on_stack = true;
	}
	llvm::Value* codegen_get_value();
	void codegen_on_stack();
};

struct PushPureObjectFunctionExpression : PushPureObjectExpression, FunctionExpression {
	PushPureObjectFunctionExpression(Expression* obj, int index, const char* unknown_name);
	llvm::Value* codegen_get_value();
	void codegen_on_stack(){ codegen_get_value(); }
};

struct PushPureObjectStaticFunctionExpression : PushPureObjectExpression, FunctionExpression {
	PushPureObjectStaticFunctionExpression(Expression* obj, int index, const char* unknown_name);
	llvm::Value* codegen_get_value();
	void codegen_on_stack(){ codegen_get_value(); }
};

//Virtual classes
struct PushVirtualPropertyExpression : PushPureObjectExpression {
	PushVirtualPropertyExpression(Expression* obj, int index)
	: PushPureObjectExpression(obj, index) {
		type = desc()->property.type;
		ref_stack();
		obj->must_on_stack();
		
		/*if (TYPE_is_pure_object(type))
			while (((CLASS*)(void*)type)->override)
				type = (TYPE)(((CLASS*)(void*)type)->override);*/
	}
	llvm::Value* codegen_private(bool get_value);
	llvm::Value* codegen_get_value(){ return codegen_private(true); }
	void codegen_on_stack(){ codegen_private(false); }
};

struct PushVirtualFunctionExpression : PushPureObjectExpression, FunctionExpression {
	PushVirtualFunctionExpression(Expression* obj, int index, const char* unknown_name);
	llvm::Value* codegen_get_value();
	void codegen_on_stack(){ codegen_get_value(); }
};

struct PushVirtualStaticFunctionExpression : PushPureObjectExpression, FunctionExpression {
	PushVirtualStaticFunctionExpression(Expression* obj, int index, const char* unknown_name);
	llvm::Value* codegen_get_value();
	void codegen_on_stack(){ codegen_get_value(); }
};

struct PushUnknownExpression : Expression {
	Expression* obj;
	unsigned short* pc;
	int name_id;
	PushUnknownExpression(Expression* obj, int name_id, unsigned short* pc) :
		obj(obj), pc(pc), name_id(name_id) {
		type = T_VARIANT;
		ref_stack();
		obj->must_on_stack();
		on_stack = true;
	}
	llvm::Value* codegen_get_value();
	void codegen_on_stack();
};

//This actually reads an arbitrary memory address into a value
struct PushStaticVariableExpression : ReadVariableExpression {
	PushStaticVariableExpression(CLASS_DESC_VARIABLE* var) {
		this->type = var->type;
		this->addr = (char*)var->klass->stat + var->offset;
		this->ctype = &var->ctype;
		this->klass = var->klass;
	}
};

struct PushStaticPropertyExpression : Expression {
	Expression* obj;
	int index;
	PushStaticPropertyExpression(Expression* obj, int index);
	llvm::Value* codegen_get_value();
	void codegen_on_stack(){ codegen_get_value(); }
};

struct PushStaticFunctionExpression : Expression, FunctionExpression {
	Expression* obj;
	int index;
	PushStaticFunctionExpression(Expression* obj, int index, const char* unknown_name = NULL);
	CLASS* klass(){
		return ((PushClassExpression*)obj)->klass;
		//return (CLASS*)(void*)obj->type;
	}
	CLASS_DESC* desc(){
		CLASS_DESC* desc = klass()->table[index].desc;
		return desc;
	}
	llvm::Value* codegen_get_value();
	void codegen_on_stack(){ codegen_get_value(); }
};

struct PushStaticUnknownExpression : Expression {
	CLASS* klass;
	const char* name;
	int name_id;
	PushStaticUnknownExpression(CLASS* klass, const char* name, int name_id) : klass(klass), name(name), name_id(name_id) {
		type = T_VARIANT;
		ref_stack();
		on_stack = true;
	}
	llvm::Value* codegen_get_value();
	void codegen_on_stack();
};

//Base class, not to be used directly
struct PopPureObjectExpression : Expression {
	Expression* obj;
	Expression* val;
	int index;
	PopPureObjectExpression(Expression* obj, Expression* val, int index)
	: obj(obj), val(val), index(index) {}
	CLASS_DESC* desc(){
		CLASS* c = (CLASS*)(void*)obj->type;
		return c->table[index].desc;
	}
};

struct PopPureObjectVariableExpression : PopPureObjectExpression {
	PopPureObjectVariableExpression(Expression* obj, Expression* val, int index)
	: PopPureObjectExpression(obj, val, index) {
		type = ctype_to_type(&desc()->variable.ctype, desc()->variable.klass);
		JIT_conv(this->val, type);
	}
	void codegen();
};

struct PopPureObjectStructFieldExpression : PopPureObjectExpression {
	PopPureObjectStructFieldExpression(Expression* obj, Expression* val, int index)
	: PopPureObjectExpression(obj, val, index) {
		if (desc()->variable.ctype.id == TC_ARRAY || desc()->variable.ctype.id == TC_STRUCT)
			THROW_ILLEGAL();
		type = ctype_to_type(&desc()->variable.ctype, desc()->variable.klass);
		JIT_conv(this->val, type);
		this->val->ref_on_stack();
	}
	void codegen();
};

/*struct PopPureObjectStaticVariableExpression : PopPureObjectExpression {
	PopPureObjectStaticVariableExpression(Expression* obj, Expression* val, int index)
	: PopPureObjectExpression(obj, val, index) {
		type = ctype_to_type(&desc()->variable.ctype);
		JIT_conv(val, type);
	}
};*/

struct PopVirtualPropertyExpression : PopPureObjectExpression {
	const char* name;
	bool is_static;
	PopVirtualPropertyExpression(Expression* obj, Expression* val, int index, const char* name, bool is_static);
	void codegen();
};

struct PopPureObjectPropertyExpression : PopPureObjectExpression {
	PopPureObjectPropertyExpression(Expression* obj, Expression* val, int index);
	void codegen();
};

//Actually writes the contents of a Value to an arbitrary address
struct PopStaticVariableExpression : Expression {
	char* addr;
	Expression* val;
	PopStaticVariableExpression(TYPE type, char* addr, Expression* val)
	: addr(addr), val(val) {
		this->type = type;
		JIT_conv(this->val, type);
	}
	void codegen();
};

struct PopStaticPropertyExpression : Expression {
	CLASS* klass;
	Expression* val;
	int index;
	PopStaticPropertyExpression(CLASS* klass, Expression* val, int index);
	void codegen();
};

struct PopUnknownPropertyUnknownExpression : Expression {
	Expression* obj;
	Expression* val;
	const char* name;
	PopUnknownPropertyUnknownExpression(Expression* obj, Expression* val, const char* name)
	: obj(obj), val(val), name(name) {
		//type = T_VARIANT;
		ref_stack();
		val->must_on_stack();
		obj->must_on_stack();
	}
	void codegen();
};

struct PopUnknownExpression : Expression {
	Expression* obj;
	Expression* val;
	int name_id;
	unsigned short* pc;
	PopUnknownExpression(Expression* obj, Expression* val, int name_id, unsigned short* pc)
	: obj(obj), val(val), name_id(name_id), pc(pc) {
		//type = T_VARIANT;
		ref_stack();
		val->must_on_stack();
		obj->must_on_stack();
	}
	void codegen();
};


/**
A Swap statement:
Push A
Push B
Pop A
Pop B
So we replace Push A, Push B, Pop A to a SwapExpression,
that is taken as input for Pop B
**/
struct SwapExpression : Expression {
	Expression* push_a_expr;
	Expression* pop_a_expr; //Of course this includes Push B
	SwapExpression(Expression* push_a, Expression* pop_a) : push_a_expr(push_a), pop_a_expr(pop_a) {
		type = push_a_expr->type;
		on_stack = push_a_expr->on_stack;
		stack_if_ref = push_a_expr->stack_if_ref;
		no_ref_variant = push_a_expr->no_ref_variant;
	}
	llvm::Value* codegen_get_value();
	void codegen_on_stack();
};

struct CallExpression : Expression {
	std::vector<Expression*> args;
	std::vector<Expression*> byref_expressions;
	Expression* func;
	CLASS* klass;
	CLASS_DESC* desc;
	unsigned short* pc;
	int index;
	char kind;
	bool variant_call;
	bool can_quick;
	CallExpression(Expression* function, int nargs, Expression** it, unsigned short* pc);
	llvm::Value* codegen_get_value();
	void codegen_on_stack();
};

struct OnGotoExpression : Expression {
	Expression* condition;
	std::vector<int> destinations;
	int default_addr;
	OnGotoExpression(Expression* condition, std::vector<int>&& destinations, int default_addr)
	: condition(condition), destinations(destinations), default_addr(default_addr) {
		for(size_t i=0, e=destinations.size(); i!=e; i++)
			mark_address_taken(destinations[i]);
		mark_address_taken(default_addr);
		JIT_conv(condition, T_INTEGER);
	}
	void codegen();
};

struct JumpExpression : Expression {
	int addr;
	JumpExpression(int addr) : addr(addr) {
		mark_address_taken(addr);
	}
	void codegen();
};

struct GosubExpression : Expression {
	int end_ctrl;
	
	//Normal use
	int gosubaddr;
	
	//On Gosub use
	Expression* condition;
	std::vector<int> destinations;
	int default_addr;
	
	GosubExpression(int end_ctrl, int gosubaddr) : end_ctrl(end_ctrl), gosubaddr(gosubaddr) {
		mark_address_taken(gosubaddr);
	}
	
	GosubExpression(int end_ctrl, Expression* condition, std::vector<int>&& destinations, int default_addr)
	: end_ctrl(end_ctrl), condition(condition), destinations(destinations), default_addr(default_addr) {
		for(size_t i=0, e=destinations.size(); i!=e; i++)
			mark_address_taken(destinations[i]);
		mark_address_taken(default_addr);
		JIT_conv(condition, T_INTEGER);
	}
	void codegen();
};

struct JumpIfExpression : Expression {
	Expression* val;
	int jump_addr;
	int next_addr;
	bool jump_if_true;
	JumpIfExpression(Expression* value, int jump_addr, int next_addr, bool t)
	: val(value), jump_addr(jump_addr), next_addr(next_addr), jump_if_true(t) {
		mark_address_taken(jump_addr);
		mark_address_taken(next_addr);
		JIT_conv(val, T_BOOLEAN);
	}
	void codegen();
};

struct JumpFirstExpression : Expression {
	Expression *to, *step;
	int ctrl_to, local_var;
	int body_addr, done_addr;
	JumpFirstExpression(int ctrl_to, Expression* to_expr, Expression* step_expr, int local_var, int body_addr, int done_addr);
	void codegen();
};

struct JumpNextExpression : Expression {
	int ctrl_to, local_var;
	int body_addr, done_addr;
	JumpNextExpression(int ctrl_to, int local_var, int body_addr, int done_addr)
	: ctrl_to(ctrl_to), local_var(local_var), body_addr(body_addr), done_addr(done_addr) {}
	void codegen();
};

struct JumpEnumFirstExpression : Expression {
	Expression* obj;
	llvm::Value* effective_class;
	llvm::Value* ob;
	int ctrl;
	JumpEnumFirstExpression(int ctrl, Expression* obj)
	: obj(obj), ctrl(ctrl) {
		set_ctrl_type(obj->type, ctrl);
	}
	void codegen();
};

//Workaround for return value for JumpEnumNext statement
//Also in use for ByRef
struct OnStackExpression : Expression {
	OnStackExpression(){
		on_stack = true;
	}
	llvm::Value* codegen_get_value();
	void codegen_on_stack(){}
};

struct JumpEnumNextExpression : Expression {
	JumpEnumFirstExpression* jfirst;
	OnStackExpression* retval;
	int cont_addr;
	int addr;
	unsigned short* pc;
	bool drop;
	bool defined;
	JumpEnumNextExpression(JumpEnumFirstExpression* jfirst, int cont_addr, int addr, unsigned short* pc, bool drop, OnStackExpression* retval);
	void codegen();
};

struct NopExpression : Expression {
	char* buf;
	bool test_stack;
	
	NopExpression(const char* str){
		int len = strlen(str);
		buf = (char*)malloc(len+1);
		memcpy(buf, str, len+1);
		test_stack = true;
	}
	
	NopExpression(bool test_stack) : test_stack(test_stack) {
		buf = (char*)"...";
	}
	
	void codegen();
	~NopExpression(){
		//free(buf);
	}
};

struct ProfileLineExpression : Expression {
	unsigned short* pc;
	ProfileLineExpression(unsigned short* pc) : pc(pc) {}
	void codegen();
};

struct DropExpression : Expression {
	Expression* expr;
	DropExpression(Expression* expr) : expr(expr) {}
	void codegen();
};

//Dup is only used in: With something = something_else ... End With
struct DupedValueExpression : Expression {
	llvm::Value* llvm_value;
	DupedValueExpression(TYPE t){
		type = t;
		llvm_value = NULL;
	}
	llvm::Value* codegen_get_value(){ return llvm_value; }
};

struct DupExpression : Expression {
	Expression* expr;
	DupedValueExpression* duped;
	DupExpression(Expression*& expression) : expr(expression) {
		expression = duped = new DupedValueExpression(type = expr->type);
	}
	llvm::Value* codegen_get_value();
	void codegen_on_stack();
};

struct NewExpression : Expression {
	std::vector<Expression*> args;
	unsigned short* pc;
	bool event;
	NewExpression(Expression** it, int nargs, bool event);
	llvm::Value* codegen_get_value();
	void codegen_on_stack();
};

struct ReturnExpression : Expression {
	Expression* retval;
	unsigned short* pc;
	int kind;
	ReturnExpression(Expression* expr, int kind);
	void codegen();
};

struct QuitExpression : Expression {
	Expression* quitval;
	QuitExpression(Expression* quitval);
	void codegen();
};

struct StopEventExpression : Expression {
	void codegen();
};

struct TryExpression : Expression {
	int addr1, addr2;
	TryExpression(int addr1, int addr2) : addr1(addr1), addr2(addr2) {
		mark_address_taken(addr1);
		mark_address_taken(addr2);
	}
	void codegen();
};

struct EndTryExpression : Expression {
	void codegen();
};

//Used as first statement in functions having a Catch/Finally
struct LargeTryExpression : Expression {
	int addr2;
	LargeTryExpression(){
		addr2 = (int)(EC - get_current_read_pos());
		mark_address_taken(0);
		mark_address_taken(addr2);
	}
	void codegen();
};
//Inserted where EC points to
struct LargeCatchExpression : Expression {
	void codegen();
};
//This is the Catch statement in the bytecode
struct CatchExpression : Expression {
	void codegen();
};

struct UnaryExpression : Expression {
	Expression* expr;
	UnaryExpression(Expression* expr) : expr(expr) {}
};

//Used by SubrExpression
struct CheckStringExpression : UnaryExpression {
	CheckStringExpression(Expression* expr) : UnaryExpression(expr) {
		type = T_STRING;
	}
	llvm::Value* codegen_get_value();
	void codegen_on_stack(){ codegen_get_value(); }
};
struct CheckIntegerExpression : UnaryExpression {
	CheckIntegerExpression(Expression* expr) : UnaryExpression(expr) {
		type = T_INTEGER;
	}
	llvm::Value* codegen_get_value();
	void codegen_on_stack(){ codegen_get_value(); }
};
struct CheckFloatExpression : UnaryExpression {
	CheckFloatExpression(Expression* expr) : UnaryExpression(expr) {
		type = T_FLOAT;
	}
	llvm::Value* codegen_get_value();
	void codegen_on_stack(){ codegen_get_value(); }
};
struct CheckPointerExpression : UnaryExpression {
	CheckPointerExpression(Expression* expr) : UnaryExpression(expr) {
		type = T_POINTER;
	}
	llvm::Value* codegen_get_value();
	void codegen_on_stack(){ codegen_get_value(); }
};


struct SubrExpression : Expression {
	std::vector<Expression*> args;
	int digit;
	int extra;
	unsigned short* pc;
	
	TYPE type2;
	
	SubrExpression(int digit, Expression** it, int nargs, int extra);
	llvm::Value* codegen_get_value();
	void codegen_on_stack();
};

struct BinOpExpression : Expression {
	Expression *left, *right;
	BinOpExpression(Expression** it) : left(it[0]), right(it[1]) {}
	std::pair<llvm::Value*, llvm::Value*> codegen_operands();
};

struct EqExpression : BinOpExpression {
	TYPE t;
	EqExpression(Expression** it);
	llvm::Value* codegen_get_value();
	void codegen_on_stack(){ codegen_get_value(); }
};

struct NotExpression : UnaryExpression {
	NotExpression(Expression* expr);
	llvm::Value* codegen_get_value();
	void codegen_on_stack(){ codegen_get_value(); }
};

struct LessExpression : BinOpExpression {
	TYPE t;
	LessExpression(Expression** it);
	llvm::Value* codegen_get_value();
	void codegen_on_stack(){ codegen_get_value(); }
};

struct NearExpression : BinOpExpression {
	NearExpression(Expression** it) : BinOpExpression(it) {
		type = T_BOOLEAN;
		JIT_conv(left, T_STRING);
		JIT_conv(right, T_STRING);
	}
	llvm::Value* codegen_get_value();
	void codegen_on_stack(){ codegen_get_value(); }
};

struct AddSubBaseExpression : BinOpExpression {
	AddSubBaseExpression(Expression** it);
};

struct AddExpression : AddSubBaseExpression {
	AddExpression(Expression** it) : AddSubBaseExpression(it) {}
	llvm::Value* codegen_get_value();
	void codegen_on_stack(){
		codegen_get_value();
	}
};

struct SubExpression : AddSubBaseExpression {
	SubExpression(Expression** it) : AddSubBaseExpression(it) {}
	llvm::Value* codegen_get_value();
	void codegen_on_stack(){
		codegen_get_value();
	}
};

struct MulExpression : BinOpExpression {
	MulExpression(Expression** it);
	llvm::Value* codegen_get_value();
	void codegen_on_stack(){
		codegen_get_value();
	}
};

struct DivExpression : BinOpExpression {
	DivExpression(Expression** it) : BinOpExpression(it) {
		type = T_FLOAT;
		ref_stack();
		JIT_conv(left, T_FLOAT);
		JIT_conv(right, T_FLOAT);
	}
	llvm::Value* codegen_get_value();
	void codegen_on_stack(){
		codegen_get_value();
	}
};

struct QuoRemBaseExpression : BinOpExpression {
	QuoRemBaseExpression(Expression** it);
};

struct QuoExpression : QuoRemBaseExpression {
	QuoExpression(Expression** it) : QuoRemBaseExpression(it) {}
	
	llvm::Value* codegen_get_value();
	void codegen_on_stack(){ codegen_get_value(); }
};

struct RemExpression : QuoRemBaseExpression {
	RemExpression(Expression** it) : QuoRemBaseExpression(it) {}
	
	llvm::Value* codegen_get_value();
	void codegen_on_stack(){ codegen_get_value(); }
};

struct PowExpression : BinOpExpression {
	PowExpression(Expression** it) : BinOpExpression(it) {
		JIT_conv(left, T_FLOAT);
		if (TYPE_is_integer(right->type)){
			JIT_conv(right, T_INTEGER);
		} else {
			JIT_conv(right, T_FLOAT);
		}
		type = T_FLOAT;
	}
	llvm::Value* codegen_get_value();
	void codegen_on_stack(){ codegen_get_value(); }
};

struct AndOrXorBaseExpression : BinOpExpression {
	AndOrXorBaseExpression(Expression** it);
};

struct AndExpression : AndOrXorBaseExpression {
	AndExpression(Expression** it) : AndOrXorBaseExpression(it) {}
	llvm::Value* codegen_get_value();
	void codegen_on_stack(){ codegen_get_value(); }
};

struct OrExpression : AndOrXorBaseExpression {
	OrExpression(Expression** it) : AndOrXorBaseExpression(it) {}
	llvm::Value* codegen_get_value();
	void codegen_on_stack(){ codegen_get_value(); }
};

struct XorExpression : AndOrXorBaseExpression {
	XorExpression(Expression** it) : AndOrXorBaseExpression(it) {}
	llvm::Value* codegen_get_value();
	void codegen_on_stack(){ codegen_get_value(); }
};

struct IsExpression : BinOpExpression {
	IsExpression(Expression** it) : BinOpExpression(it) {
		JIT_conv(left, T_OBJECT);
		type = T_BOOLEAN;
	}
	llvm::Value* codegen_get_value();
	void codegen_on_stack(){ codegen_get_value(); }
};

struct NegExpression : UnaryExpression {
	NegExpression(Expression* expr) : UnaryExpression(expr) {
		type = expr->type;
		if (type == T_VARIANT){
			ref_stack();
			no_ref_variant = true;
		} else if (!TYPE_is_number(type))
			THROW(E_TYPE, "Number", JIF.F_TYPE_get_name(type));
	}
	llvm::Value* codegen_get_value();
	void codegen_on_stack(){
		codegen_get_value();
	}
};

struct CatFileExpression : Expression {
	std::vector<Expression*> args;
	CatFileExpression(Expression** it, int nargs){
		args.resize(nargs);
		for(int i=0; i<nargs; i++){
			args[i] = it[i];
			JIT_conv(args[i], T_STRING);
			args[i]->must_on_stack();
		}
		type = T_STRING;
		on_stack = true;
	}
};

struct CatExpression : CatFileExpression {
	CatExpression(Expression** it, int nargs)
	: CatFileExpression(it, nargs) {}
	llvm::Value* codegen_get_value();
	void codegen_on_stack();
};

struct FileExpression : CatFileExpression {
	FileExpression(Expression** it, int nargs)
	: CatFileExpression(it, nargs) {}
	llvm::Value* codegen_get_value();
	void codegen_on_stack();
};

struct LikeExpression : BinOpExpression {
	int kind;
	LikeExpression(Expression** it, int kind)
	: BinOpExpression(it), kind(kind) {
		type = T_BOOLEAN;
		left->must_on_stack();
		right->must_on_stack();
		//The runtime converts to string, so ref_stack is kind of needed ..
		ref_stack();
	}
	llvm::Value* codegen_get_value();
	void codegen_on_stack();
};

struct Statement {
	int addr;
	bool address_taken;
	Expression* expr;
};

extern std::vector<Statement*> all_statements;

void mark_address_taken(int addr);

#endif /* __JIT_H */
