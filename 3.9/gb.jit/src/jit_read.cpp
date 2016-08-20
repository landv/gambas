/***************************************************************************

  jit_read.c

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

#define __JIT_READ_C

#include "jit.h"
#include "gb_pcode.h"

#include <vector>
#include <algorithm>

static unsigned short* code;
static int size;
static int pos;
static int addr;
static uint stack_start;

static bool in_dup = false;

unsigned short* get_current_read_pos(){
	return code + pos;
}

static std::vector<Expression*> _stack;

void ref_stack(){
	for(size_t i=0, e=_stack.size(); i!=e; i++)
		_stack[i]->ref_on_stack();
}

static int stack_size(){
	return _stack.size();
}

static bool stack_empty(){
	return _stack.size() == stack_start;
}

static void push(Expression* expr){
	_stack.push_back(expr);
}

static Expression*& top(){
	return _stack.back();
}

static Expression* pop(){
	Expression* val = _stack.back();
	_stack.pop_back();
	return val;
}

static Expression** extract(int num){
	size_t s = _stack.size();
	size_t start = (int)s-num;
	Expression** it = &_stack[start];
	_stack.resize(start);
	return it;
}

static Expression** extract_reverse(int num){
	Expression** it = extract(num);
	std::reverse(it, it+num);
	return it;
}

std::vector<int> taken_addresses;

void mark_address_taken(int addr){
	taken_addresses.push_back(addr);
}

static void push_statement(Expression* expr){
	Statement* s = new Statement();
	s->addr = addr;
	s->address_taken = false;
	s->expr = expr;
	all_statements.push_back(s);
}

static Expression*& top_statement(){
	return all_statements.back()->expr;
}

static Expression* pop_statement(){
	Expression* ret = top_statement();
	delete all_statements.back();
	all_statements.pop_back();
	return ret;
}

static void push_constant(CLASS_DESC* desc){
	if (TYPE_is_string(desc->constant.type) && desc->constant.translate){
		char* addr = (char *)GB.Translate(desc->constant.value._string);
		push(new PushCStringExpression(addr, 0, strlen(addr)));
		return;
	}
	void* value = &desc->constant.value;
	switch(desc->constant.type){
		case T_BOOLEAN:
			push(new PushIntegerExpression(1, *(bool*)value)); return;
		case T_BYTE:
			push(new PushIntegerExpression(8, *(unsigned char*)value)); return;
		case T_SHORT:
			push(new PushIntegerExpression(16, *(short*)value)); return;
		case T_INTEGER:
			push(new PushIntegerExpression(32, *(int*)value)); return;
		case T_LONG:
			push(new PushIntegerExpression(64, *(int64_t*)value)); return;
		case T_SINGLE:
			push(new PushFloatExpression(32, *(float*)value)); return;
		case T_FLOAT:
			push(new PushFloatExpression(64, *(double*)value)); return;
		case T_CSTRING:
			push(new PushCStringExpression(*(char**)value, 0, strlen(*(char**)value))); return;
		default:
			assert(false && "Illegal constant type");
	}
}

static int get_subr_nargs(int digit, int extra){
	int nargs = extra;
	switch(digit){
		case 0x6D: //Timer
		case 0x6E: //Now
		case 0x75: //Error
		case 0x76: //Debug
			nargs = 0; break;
		
		case 0x43: //Len
		case 0x44: //Space
		case 0x46: //Trim
		case 0x47: //UCase
		case 0x48: //LCase
		case 0x49: //Chr
		case 0x53: //DConv
		case 0x54: //Abs
		case 0x55: //Int
		case 0x56: //Fix
		case 0x57: //Sgn
		case 0x58: //Math
		case 0x63: //IsAscii, IsLetter ...
		case 0x65: //IsBoolean ...
		case 0x66: //TypeOf
		case 0x67: //CBool ...
		case 0x6A: //Val
		case 0x6B: //Str
		case 0x6F: //Year
		case 0x79: //Close
		case 0x7B: //Line Input
		case 0x7F: //Flush
		case 0x80: //Lock
		case 0x81: //Input From, Output To, Error To
		case 0x85: //Kill
		case 0x86: //Mkdir, deprecated -> Even() & Odd()
		case 0x87: //Rmdir
		case 0x8E: //DFree
		case 0x90: //IsDir
		case 0x95: //Free
		case 0x98: //Sleep
		case 0x99: //VarPtr
		case 0x9B: //Tr
		case 0x9C: //Quote, Shell, Html
		case 0x9D: //Unquote
		case 0x9E: //MkInteger, ...
		case 0x9F: //Bool@, Byte@, ...
			nargs = 1; break;
		
		case 0x45: //String
		case 0x50: //Scan
		case 0x5D: //Min
		case 0x5E: //Max
		case 0x62: //Math2
		case 0x64: //Bit
		case 0x78: //Open
		case 0x7D: //Read
		case 0x88: //Move
		case 0x89: //Copy
			nargs = 2; break;
			
		case 0x8A: //Link, deprecated -> IsNan() & IsInf()
			nargs = extra == 0 ? 2 : 1; break;
		
		case 0x52: //Conv
		case 0x5F: //IIf
		case 0x73: //DateAdd, DateDiff
		case 0x7E: //Write
			nargs = 3; break;
		
		case 0x93: //Exec
			nargs = 4; break;
	}
	return nargs;
}

static void JIT_push_unknown(){
	const char* name = CP->load->unknown[code[pos+1]];
	pos += 2;
	bool defined;
	
	if (TYPE_is_pure_object(top()->type)){
		//These can now be on top of the stack:
		//T_NULL, a null object of type top()->type
		//A pure object (type > T_OBJECT) with type top()->type
		//When top()->type is a virtual class:
		// T_NULL can be returned (only by non-static methods at the moment)
		// An object represented as a virtual class top()->type is the virtual class,
		//  but the real type can be something else.
		// A T_CLASS of the class top()->type:
		//  happens when a native static method says it returns a virtual object
		//  But beware, a native static method might as well return a virtual object
		//  of type .SubCollection
		defined = true;
		CLASS* klass = (CLASS*)(void*)top()->type;
		
		JIT_load_class(klass);
		
		int index = CLASS_find_symbol(klass, name);
		
		bool is_unknown = false;
		
		if (index == NO_SYMBOL){
			if (klass->special[SPEC_UNKNOWN] == NO_SYMBOL)
				THROW(E_NSYMBOL, klass->name, name);
			
			if (klass->special[SPEC_PROPERTY] != NO_SYMBOL){
				push(new PushPureObjectUnknownExpression(pop(), name, code[pos-1]));
				return;
			}
			
			index = klass->special[SPEC_UNKNOWN];
			is_unknown = true;
		}
		
		CLASS_DESC* desc = klass->table[index].desc;
		
		switch (CLASS_DESC_get_type(desc)){
			case CD_CONSTANT:
				if (klass->is_virtual){
					//FIXME OK to extract the value and ignore actually evaluating, and what if we got a null value..???
					/*delete*/ pop();
					push_constant(desc);
					return;
				}
				push(new PushPureObjectConstantExpression(pop(), index));
				return;
			case CD_VARIABLE:
				/*int offset = desc->variable.offset;
				TYPE type = desc->variable.ctype;*/
				push(new PushPureObjectVariableExpression(pop(), index));
				return;
			
			case CD_STATIC_VARIABLE:
				THROW(E_STATIC, klass->name, name);
				//push(new PushPureObjectStaticVariableExpression(pop(), index));
				return;
				
			case CD_PROPERTY:
			case CD_PROPERTY_READ:
				if (klass->is_virtual)
					push(new PushVirtualPropertyExpression(pop(), index));
				else
					push(new PushPureObjectPropertyExpression(pop(), index));
				return;
				
			case CD_STRUCT_FIELD:
				push(new PushPureObjectStructFieldExpression(pop(), index));
				return;
				
			case CD_STATIC_PROPERTY:
			case CD_STATIC_PROPERTY_READ:
				//See System.User why this is a problem ...
				//Runtime check is now done.
				//if (!klass->is_virtual)
				//	THROW(E_STATIC, klass->name, name);
				//push(new PushVirtualClassStaticProperty(pop(), index));
				push(new PushPureObjectStaticPropertyExpression(pop(), index, name));
				return;
				
			case CD_METHOD:
				if (klass->is_virtual)
					push(new PushVirtualFunctionExpression(pop(), index, is_unknown ? name : NULL));
				else
					push(new PushPureObjectFunctionExpression(pop(), index, is_unknown ? name : NULL));
				return;
				
			case CD_STATIC_METHOD:
				if (klass->is_virtual)
					push(new PushVirtualStaticFunctionExpression(pop(), index, is_unknown ? name : NULL));
				else
					push(new PushPureObjectStaticFunctionExpression(pop(), index, is_unknown ? name : NULL));
				return;
				
			case CD_EXTERN:
				push(new PushExternExpression(klass, index, pop()));
				return;
			
			default: THROW(E_NSYMBOL, klass->name, name);
		}
		
	} else if (top()->type == T_OBJECT){
		defined = false;
		push(new PushUnknownExpression(pop(), code[pos-1], code + pos - 2));
	} else if (top()->type == T_CLASS){
		defined = true;
		
		PushClassExpression* pce = dyn_cast<PushClassExpression>(top());
		assert(pce);
		
		CLASS* klass = pce->klass;
		
		int index = CLASS_find_symbol(klass, name);
		
		if (index == NO_SYMBOL){
			if (klass->special[SPEC_UNKNOWN] == NO_SYMBOL)
				THROW(E_NSYMBOL, klass->name, name);
			
			if (klass->special[SPEC_PROPERTY] != NO_SYMBOL){
				pop();
				if (!klass->unknown_static || !klass->property_static){
					if (!klass->auto_create)
						THROW(E_DYNAMIC);
					push(new PushPureObjectUnknownExpression(new PushAutoCreateExpression(klass), name, code[pos-1]));
					return;
				}
				push(new PushStaticUnknownExpression(klass, name, code[pos-1]));
				return;
			}
			
			index = klass->special[SPEC_UNKNOWN];
			
			if (!klass->unknown_static){
				pop();
				if (!klass->auto_create)
					THROW(E_DYNAMIC);
				
				push(new PushPureObjectFunctionExpression(new PushAutoCreateExpression(klass), index, name));
				return;
			}
			
			push(new PushStaticFunctionExpression(pop(), index, name));
			return;
		}
		
		CLASS_DESC* desc = klass->table[index].desc;
		
		switch (CLASS_DESC_get_type(desc)){
			case CD_CONSTANT: {
				/*delete*/ pop();
				push_constant(desc);
				return;
			}
			case CD_VARIABLE:
				if (!klass->auto_create)
					THROW(E_DYNAMIC, klass->name, name);
				/*delete*/ pop();
				push(new PushPureObjectVariableExpression(new PushAutoCreateExpression(klass), index));
				return;
				
			case CD_STRUCT_FIELD:
				THROW(E_DYNAMIC, klass->name, name);
				
			case CD_STATIC_VARIABLE:
				/*delete*/ pop();
				push(new PushStaticVariableExpression(&desc->variable));
				return;
				
			case CD_PROPERTY:
			case CD_PROPERTY_READ:
				if (!klass->auto_create)
					THROW(E_DYNAMIC, klass->name, name);
				/*delete*/ pop();
				push(new PushPureObjectPropertyExpression(new PushAutoCreateExpression(klass), index));
				return;
				
			case CD_STATIC_PROPERTY:
			case CD_STATIC_PROPERTY_READ:
				push(new PushStaticPropertyExpression(pop(), index));
				return;
				
			case CD_METHOD:
				if (!klass->auto_create)
					THROW(E_DYNAMIC, klass->name, name);
				/*delete*/ pop();
				push(new PushPureObjectFunctionExpression(new PushAutoCreateExpression(klass), index, NULL));
				return;
				
			case CD_STATIC_METHOD:
				/*int kind = FUNCTION_PUBLIC;
				if (FUNCTION_is_native(&desc->method)){
					if (desc->method.subr)
						kind = FUNCTION_SUBR;
					else
						kind = FUNCTION_NATIVE;
				}*/
				push(new PushStaticFunctionExpression(pop(), index));
				return;
				
			case CD_EXTERN:
				pop();
				push(new PushExternExpression(klass, desc->ext.exec, NULL));
				return;
			
			default: THROW(E_NSYMBOL, klass->name, name);
			
		}
		
	} else if (top()->type == T_FUNCTION){
		assert(false && "Syntax error");
	} else if (top()->type == T_VARIANT){
		defined = false;
		push(new PushUnknownExpression(pop(), code[pos-1], code + pos - 2));
	} else if (top()->type == T_NULL) {
		THROW(E_NULL);
	} else {
		THROW(E_NOBJECT);
	}
}

static void JIT_pop_unknown(){
	const char* name = CP->load->unknown[code[pos+1]];
	pos += 2;
	
	if (TYPE_is_pure_object(top()->type)){
		//These can now be on top of the stack:
		//T_NULL, a null object of type top()->type
		//A pure object (type > T_OBJECT) with type top()->type
		//When top()->type is a virtual class:
		// T_NULL can be returned (only by non-static methods at the moment)
		// An object represented as a virtual class top()->type is the virtual class,
		//  but the real type can be something else.
		// A T_CLASS of the class top()->type:
		//  happens when a native static method says it returns a virtual object
		//  But beware, a native static method might as well return a virtual object
		//  of type .SubCollection
		// By the time of this writing, there are no write properties taking virtual classes.
		CLASS* klass = (CLASS*)(void*)top()->type;
		
		int index = CLASS_find_symbol(klass, name);
		
		if (index == NO_SYMBOL){
			if (klass->special[SPEC_UNKNOWN] == NO_SYMBOL)
				THROW(E_NSYMBOL, klass->name, name);
			
			if (klass->special[SPEC_PROPERTY] == NO_SYMBOL)
				THROW(E_NPROPERTY, klass->name, name);
			
			//Unknown property
			Expression* obj = pop();
			Expression* val = pop();
			push_statement(new PopUnknownPropertyUnknownExpression(obj, val, name));
			return;
		}
		
		CLASS_DESC* desc = klass->table[index].desc;
		
		Expression* obj = pop();
		Expression* val = pop();
			
		switch(CLASS_DESC_get_type(desc)){
			case CD_CONSTANT:
				THROW(E_NPROPERTY, klass->name, name);
			
			case CD_VARIABLE:
				push_statement(new PopPureObjectVariableExpression(obj, val, index));
				return;
			
			case CD_STRUCT_FIELD:
				if (desc->variable.ctype.id == TC_STRUCT || desc->variable.ctype.id == TC_ARRAY)
					THROW(E_NWRITE, klass->name, name);
				push_statement(new PopPureObjectStructFieldExpression(obj, val, index));
				return;
			
			case CD_STATIC_VARIABLE:
				THROW(E_STATIC, klass->name, name);
				//push_statement(new PopPureObjectStaticVariableExpression(obj, val, index));
				return;
			
			case CD_PROPERTY:
				if (klass->is_virtual)
					push_statement(new PopVirtualPropertyExpression(obj, val, index, name, false));
				else
					push_statement(new PopPureObjectPropertyExpression(obj, val, index));
				return;
			
			case CD_STATIC_PROPERTY:
				//Should only be used on virtual classes. Runtime check is added if not, throwing E_STATIC
				//THROW(E_STATIC, klass->name, name);
				push_statement(new PopVirtualPropertyExpression(obj, val, index, name, true));
				return;
			
			case CD_PROPERTY_READ:
			case CD_STATIC_PROPERTY_READ:
				THROW(E_NWRITE, klass->name, name);
			
			case CD_METHOD:
			case CD_STATIC_METHOD:
				THROW(E_NPROPERTY, klass->name, name);
			
			default: THROW(E_NSYMBOL, klass->name, name);
		}
	} else if (top()->type == T_OBJECT){
		Expression* obj = pop();
		Expression* val = pop();
		push_statement(new PopUnknownExpression(obj, val, code[pos-1], code + pos - 2));
	} else if (top()->type == T_CLASS){
		
		PushClassExpression* pce = dyn_cast<PushClassExpression>(top());
		assert(pce);
		
		CLASS* klass = pce->klass;
		
		int index = CLASS_find_symbol(klass, name);
		
		if (index == NO_SYMBOL){
			if (klass->special[SPEC_UNKNOWN] == NO_SYMBOL)
				THROW(E_NSYMBOL, klass->name, name);
			
			if (klass->special[SPEC_PROPERTY] == NO_SYMBOL)
				THROW(E_NPROPERTY, klass->name, name);
			
			//Unknown property
			Expression* obj = pop();
			Expression* val = pop();
			push_statement(new PopUnknownPropertyUnknownExpression(obj, val, name));
			return;
		}
		
		CLASS_DESC* desc = klass->table[index].desc;
		
		pop();
		Expression* val = pop();
		
		switch(CLASS_DESC_get_type(desc)){
			case CD_CONSTANT:
				THROW(E_NPROPERTY, klass->name, name);
			
			case CD_VARIABLE:
				if (!klass->auto_create)
					THROW(E_DYNAMIC, klass->name, name);
				//delete obj;
				push_statement(new PopPureObjectVariableExpression(new PushAutoCreateExpression(klass), val, index));
				return;
			
			case CD_STRUCT_FIELD:
				THROW(E_DYNAMIC, klass->name, name);
			
			case CD_STATIC_VARIABLE:
				//delete obj;
				if (desc->variable.ctype.id == TC_ARRAY || desc->variable.ctype.id == TC_STRUCT)
					THROW_ILLEGAL();
				push_statement(new PopStaticVariableExpression(desc->variable.type, (char*)desc->variable.klass->stat + desc->variable.offset, val));
				return;
				
			case CD_PROPERTY:
				if (!klass->auto_create)
					THROW(E_DYNAMIC, klass->name, name);
				//delete obj;
				push_statement(new PopPureObjectPropertyExpression(new PushAutoCreateExpression(klass), val, index));
				return;
			
			case CD_STATIC_PROPERTY:
				//delete obj;
				push_statement(new PopStaticPropertyExpression(klass, val, index));
				return;
			
			case CD_PROPERTY_READ:
			case CD_STATIC_PROPERTY_READ:
				THROW(E_NWRITE, klass->name, name);
			
			case CD_METHOD:
			case CD_STATIC_METHOD:
				THROW(E_NPROPERTY, klass->name, name);
			
			default: THROW(E_NSYMBOL, klass->name, name);
		}
	} else if (top()->type == T_FUNCTION){
		assert(false && "syntax error");
	} else if (top()->type == T_VARIANT){
		Expression* obj = pop();
		Expression* val = pop();
		push_statement(new PopUnknownExpression(obj, val, code[pos-1], code + pos - 2));
		return;
	} else if (top()->type == T_NULL) {
		THROW(E_NULL);
	} else {
		THROW(E_NOBJECT);
	}
}

static void check_swap(){
	if (!in_dup && !stack_empty()){
		/* Then this have happened:
		Push A
		Push B
		Pop A
		Pop B
		and we have just made the Pop A
		So we replace Push A, Push B, Pop A to a SwapExpression,
		that is taken as input for Pop B */
		push(new SwapExpression(pop(), pop_statement()));
	}
}

#define NEXT pos+=ncode; goto __NEXT;
#define CONT goto __NEXT;

static void JIT_read_statement(){
	while(pos < size){
		unsigned short op = code[pos];
		int ncode;
		switch (op & 0xFF00){
			case C_PUSH_UNKNOWN: case C_POP_UNKNOWN:
			case C_PUSH_INTEGER:
			case C_JUMP: case C_JUMP_IF_TRUE: case C_JUMP_IF_FALSE: case C_GOSUB:
			case C_NEXT: case C_JUMP_NEXT:
			case C_TRY:

				ncode = 2;
				break;

			case C_PUSH_LONG:

				ncode = 3;
				break;
				
			case C_BYREF:
				ncode = 2 + (op & 0xFF);
				break;

			default:

				if ((op & 0xFF00) == (C_PUSH_CONST | 0xF00))
					ncode = 2;
				else
					ncode = 1;
		}
		
		unsigned short digit = (op >> 12U);
		int value = op & 0xFFF;
		if (value >= 0x800) value |= 0xFFFFF000;
		
		switch (digit){
			case 0xF:
				push(new PushIntegerExpression(32, value));
				NEXT
			case 0xE: {
				VALUE val;
				unsigned short ind = op & 0xFFF;
				if ((op & 0xF00) == 0xF00)
					ind = code[++pos];
				#define LOCAL_gettext GB.Translate
				VALUE_class_constant_inline(CP, &val, ind);
				#undef LOCAL_gettext
				
				switch(val.type){
					case T_INTEGER:
						push(new PushIntegerExpression(32, val._integer.value));
						NEXT
					case T_LONG:
						push(new PushIntegerExpression(64, val._long.value));
						NEXT
					case T_SINGLE:
						push(new PushFloatExpression(32, val._single.value));
						NEXT
					case T_FLOAT:
						push(new PushFloatExpression(64, val._float.value));
						NEXT
					case T_CSTRING:
						push(new PushCStringExpression(val._string.addr, val._string.start, val._string.len));
						NEXT
					case T_POINTER:
						push(new PushNullPointerExpression());
						NEXT
				}
				assert(false && "Invalid constant type");
			}
			case 0xD:
				if (value & 0x800)
					push_statement(new PopStaticExpression(pop(), value & 0x7FF));
				else
					push_statement(new PopDynamicExpression(pop(), value & 0x7FF));
				check_swap();
				NEXT
			case 0xC:
				if (value & 0x800)
					push(new PushStaticExpression(value & 0x7FF));
				else
					push(new PushDynamicExpression(value & 0x7FF));
				NEXT
			case 0xB:
				if (value & 0x800)
					push(new PushFunctionExpression(value & 0x7FF));
				else
					push(new PushClassExpression(value & 0x7FF));
				NEXT
			case 0xA:
				push(new AddQuickExpression(pop(), value));
				NEXT
				
			default: {
				digit = op & 0xFF00;
				value = op & 0xFF;
				if (value >= 0x80) value |= 0xFFFFFF00;
				
				switch(digit){
					case C_PUSH_LOCAL:
						if (value >= FP->n_local && get_ctrl_type(value) == T_CLASS){
							//Control variable
							push(new PushClassExpression(get_ctrl_class(value)));
						} else {
							push(new PushLocalExpression(value));
						}
						NEXT
					case C_PUSH_PARAM:
						push(new PushParamExpression(value));
						NEXT
					case C_POP_LOCAL:
						push_statement(new PopLocalExpression(pop(), value));
						check_swap();
						NEXT
					case C_POP_PARAM:
						push_statement(new PopParamExpression(pop(), value));
						check_swap();
						NEXT
					case C_PUSH_UNKNOWN:
						JIT_push_unknown();
						CONT
					case C_POP_UNKNOWN:
						JIT_pop_unknown();
						check_swap();
						CONT
					case C_POP_CTRL:
						in_dup = false; //From Dup
						push_statement(new PopCtrlExpression(pop(), value));
						NEXT
					case C_POP_OPTIONAL:
						push_statement(new PopOptionalExpression(pop(), value));
						NEXT
					case C_PUSH_EXTERN:
						push(new PushExternExpression(CP, value, NULL));
						NEXT
					case C_PUSH_EVENT:
						if ((unsigned char)op == 0xFF){
							push(new PushEventExpression(0, CP->load->unknown[code[pos+1]]));
							pos++;
						} else {
							push(new PushEventExpression(value, NULL));
						}
						NEXT
					case C_PUSH_ARRAY:
						push(new PushArrayExpression(extract(value), value));
						NEXT
					case C_POP_ARRAY: {
						Expression** it = extract(value);
						Expression* val = pop();
						push_statement(new PopArrayExpression(it, value, val));
						check_swap();
						NEXT
					}
					case C_CALL: {
						Expression** it = extract(value);
						Expression* func = pop();
						CallExpression* ce = new CallExpression(func, value, it, code + pos);
						push(ce);
						if ((code[pos+1] & 0xFF00) == C_BYREF){
							int nbyref_codes = 1 + (code[pos+1] & 0xFF);
							uint64_t byref_mask = 0;
							for(int i=0; i<nbyref_codes; i++){
								byref_mask |= (uint64_t)code[pos+1+1+i] << i*4;
							}
							pos += 1+1+nbyref_codes;
							
							TYPE* signature = NULL;
							int npmax;
							
							if (ce->desc){
								signature = ce->desc->method.signature;
								npmax = ce->desc->method.npmax;
							} else if (ce->kind == FUNCTION_PRIVATE){
								FunctionExpression* fe = dynamic_cast<FunctionExpression*>(func);
								int index = fe->function_index;
								FUNCTION* fp = &CP->load->func[index];
								if (sizeof(TYPE) != sizeof(CLASS_PARAM)){
									//Something has changed...
									abort();
								}
								signature = (TYPE*)fp->param;
								npmax = fp->n_param;
							}
							
							for(int i=value; i --> 0;) if (1ULL<<i & byref_mask){
								if (signature && i >= npmax)
									THROW(E_BYREF);
								auto ex = new OnStackExpression();
								ex->type = signature ? signature[i] : -1;
								
								uint stack_start_save = stack_start;
								stack_start = stack_size();
								push(ex);
								JIT_read_statement();
								stack_start = stack_start_save;
								
								ce->byref_expressions.push_back(pop_statement());
							}
							CONT
						} else {
							NEXT
						}
					}
					case C_BYREF: {
						int nbyref_codes = 1 + (code[pos] & 0xFF);
						for(int i=0; i<nbyref_codes; i++){
							func_byref_mask |= (uint64_t)code[pos+1+i] << i*4;
						}
						NEXT
					}
					case C_PUSH_INTEGER:
						push(new PushIntegerExpression(32, (short)code[pos+1]));
						NEXT
					case C_PUSH_LONG:
						push(new PushIntegerExpression(32, code[pos+1] | (code[pos+2] << 16)));
						NEXT
					case C_PUSH_ME: {
						if (value & 2){ //Super
							if (OP)
								push(new PushSuperExpression());
							else
								push(new PushClassExpression(CP->parent));
						} else {
							if (OP)
								push(new PushMeExpression());
							else
								push(new PushClassExpression(CP));
						}
						NEXT
					}
					case C_PUSH_MISC:
						switch(value){
							case CPM_NULL: push(new PushNullExpression()); NEXT
							case CPM_VOID: push(new PushVoidExpression()); NEXT
							case CPM_FALSE: push(new PushIntegerExpression(1, false)); goto push_false_true;
							case CPM_TRUE: push(new PushIntegerExpression(1, true)); goto push_false_true;
							case CPM_LAST: push(new PushLastExpression()); NEXT
							case CPM_STRING: push(new PushCStringExpression(NULL, 0, 0)); NEXT
							case CPM_PINF: push(new PushFloatExpression(64, INFINITY)); NEXT
							case CPM_MINF: push(new PushFloatExpression(64, -INFINITY)); NEXT
							case CPM_COMPLEX: push(new PushComplexExpression(pop())); NEXT
						}
						assert(false && "Illegal Push Misc");
						push_false_true: {
							if ((code[pos+1] & 0xFF00) == C_JUMP){
								//Bytecode representation of And if, Or if is implemented in this strange way
								int addr = pos + 3 + code[pos+2];
								//There is a JUMP IF FALSE at addr
								int false_addr = addr + code[addr+1] + 2;
								push_statement(new JumpIfExpression(pop(), false_addr, addr + 2, false));
								pos += 3;
								CONT
							}
							NEXT
						}
					case C_ON: {
						int num_cases = value;
						std::vector<int> destinations;
						destinations.resize(num_cases);
						for(int i=0; i<num_cases; i++){
							destinations[i] = pos + 1 + i + (short)code[pos+i+1];
						}
						pos += 1 + num_cases;
						if ((code[pos] & 0xFF00) == C_JUMP){
							//Goto
							push_statement(new OnGotoExpression(pop(), std::move(destinations), pos + 2));
						} else {
							//GoSub
							ngosubs++;
							push_statement(new GosubExpression(code[pos] & 0xFF, pop(), std::move(destinations), pos + 2));
						}
						pos += 2;
						CONT
					}
					case C_JUMP:
						/*if (EC && code + pos + (short)code[pos+1] + 2 >= EC){
							push_statement(new EndTryExpression());
							push_statement(new JumpExpression(pos + (short)code[pos+1] + 2 + 1));
						} else {*/
							push_statement(new JumpExpression(pos + (short)code[pos+1] + 2));
						//}
						NEXT
					case C_JUMP_IF_TRUE: case C_JUMP_IF_FALSE:
						push_statement(new JumpIfExpression(pop(), pos + (short)code[pos+1] + 2, pos + 2, digit == C_JUMP_IF_TRUE));
						NEXT
					case C_JUMP_FIRST: {
						Expression* step = pop();
						Expression* to = pop();
						push_statement(new JumpFirstExpression(value, to, step, code[pos+3] & 0xFF, pos + 4, pos + code[pos+2] + 3));
						NEXT
					}
					case C_JUMP_NEXT:
						push_statement(new JumpNextExpression(code[pos-1] & 0xFF, code[pos+2] & 0xFF, pos + 3, pos + code[pos+1] + 2));
						pos += 3;
						CONT
					case C_GOSUB: {
						ngosubs++;
						push_statement(new GosubExpression(value, pos + (short)code[pos+1] + 2));
						NEXT
					}
					case C_FIRST: {
						Expression* obj = pop();
						push_statement(new JumpEnumFirstExpression(value, obj));
						NEXT
					}
					case C_NEXT:
						if (value) //Drop
							push_statement(new JumpEnumNextExpression((JumpEnumFirstExpression*)top_statement(), pos + 2, pos + code[pos+1] + 2, code + pos, true, NULL));
						else {
							OnStackExpression* ose = new OnStackExpression();
							push_statement(new JumpEnumNextExpression((JumpEnumFirstExpression*)top_statement(), pos + 2, pos + code[pos+1] + 2, code + pos, false, ose));
							push(ose);
							addr = pos+2;
						}
						NEXT
					case C_DROP:
						push_statement(new DropExpression(pop()));
						NEXT
					case C_DUP:
						in_dup = true; //Becomes false again after Pop Ctrl
						push(new DupExpression(top()));
						NEXT
					case C_NEW: {
						int nargs = value & 0x3F;
						Expression** it = extract(nargs);
						push(new NewExpression(it, nargs, value & CODE_NEW_EVENT));
						NEXT
					}
					case C_BREAK:
						//PC = code+pos;
						if (EXEC_profile)
							push_statement(new ProfileLineExpression(code + pos));
						else
							push_statement(new NopExpression(true/*JIF.F_DEBUG_get_current_position()*/));
						//PC = code;
						//push_statement(new NopExpression());
						NEXT
					case C_RETURN:
						push_statement(new ReturnExpression(value == 1 ? pop() : NULL, value));
						NEXT
					case C_QUIT:
						switch(value){
							case 0: default: push_statement(new QuitExpression(NULL)); NEXT
							case 1: push_statement(new NopExpression(false)); NEXT //FIXME breakpoint
							case 2: push_statement(new StopEventExpression()); NEXT
							case 3: push_statement(new QuitExpression(pop())); NEXT
						}
					case C_PUSH_CHAR:
						push(new PushCStringExpression((char*)&STRING_char_string[value * 2], 0, 1));
						NEXT
					case C_TRY:
						push_statement(new TryExpression(pos + 2, pos + code[pos+1] + 2));
						NEXT
					case C_END_TRY:
						push_statement(new EndTryExpression());
						NEXT
					case C_CATCH:
						if (code + pos < EC) // No finally
							push_statement(new ReturnExpression(NULL, 2));
						else // We have a finally somewhere above
							push_statement(new CatchExpression());
						NEXT
						
					case C_EQ: push(new EqExpression(extract(2))); NEXT
					case C_NE: push(new NotExpression(new EqExpression(extract(2)))); NEXT
					case C_GT: push(new LessExpression(extract_reverse(2))); NEXT
					case C_LE: push(new NotExpression(new LessExpression(extract_reverse(2)))); NEXT
					case C_LT: push(new LessExpression(extract(2))); NEXT
					case C_GE: push(new NotExpression(new LessExpression(extract(2)))); NEXT
					case C_NEAR: push(new NearExpression(extract(2))); NEXT
					case C_CASE: assert(false && "C_CASE is not used anymore?");
					#define op(_c, _n) case _c: push(new _n##Expression(extract(2))); NEXT
					op(C_ADD, Add)
					op(C_SUB, Sub)
					op(C_MUL, Mul)
					op(C_DIV, Div)
					
					op(C_QUO, Quo)
					op(C_REM, Rem)
					op(C_POW, Pow)
					op(C_AND, And)
					op(C_OR, Or)
					op(C_XOR, Xor)
					
					op(C_IS, Is)
					#undef op
					case C_NEG: push(new NegExpression(pop())); NEXT
					case C_NOT: push(new NotExpression(pop())); NEXT
					case C_CAT: push(new CatExpression(extract(value), value)); NEXT
					case C_FILE: push(new FileExpression(extract(value), value)); NEXT
					case C_LIKE: push(new LikeExpression(extract(2), value)); NEXT
					
					
					default: {
						digit >>= 8U;
						// 0x40 <= digit <= 0x9F
						int extra = value & 0x3F;
						int nargs = get_subr_nargs(digit, extra);
						switch(digit){
							case 0x67: //CBool ...
								JIT_conv(top(), extra);
								goto _next;
							case 0x6B: //Str
								if (TYPE_is_string(top()->type))
									goto _next;
								break;
						}
						push(new SubrExpression(digit, extract(nargs), nargs, extra));
						_next:
						NEXT
					}
				}
			}
		}
		__NEXT:
		if (stack_empty()) //Next code row
			return;
	}
}

void JIT_read(){
	code = FP->code;
	size = ((int*)code)[-1] / sizeof(short);
	if (FP->code[size-1] == 0)
		--size;
	
	pos = 0;
	
	TRY {
		if (EC != NULL){
			addr = -1;
			push_statement(new LargeTryExpression());
		}
		
		func_byref_mask = 0;
		
		stack_start = 0;
		while(pos < size){
			addr = pos;
			PC = code + pos;
			JIT_read_statement();
		}
		
		std::sort(taken_addresses.begin(), taken_addresses.end());
		for(size_t i=0, j=0, e1=taken_addresses.size(), e2=all_statements.size(); i!=e1 && j!=e2;){
			int a = taken_addresses[i];
			int b = all_statements[j]->addr;
			if (a == b){
				all_statements[j]->address_taken = true;
				i++, j++;
			} else if (a < b){
				i++;
			} else if (a > b){
				j++;
			}
		}
		taken_addresses.clear();
	}
	CATCH {
		//puts("Fel:");
		//ERROR_print_at(stderr, 1, 1);
		//abort();
		for(size_t i=0, e=all_statements.size(); i!=e; i++)
			delete all_statements[i];
		all_statements.clear();
		free_all_expressions();
		taken_addresses.clear();
		JIF.F_ERROR_propagate();
	}
	END_TRY
}
