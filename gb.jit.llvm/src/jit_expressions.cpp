/***************************************************************************

  jit_expressions.c

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

#define __JIT_EXPRESSIONS_C

#include "jit.h"
#include "jit_runtime.h"

static void ref_variant(Expression* expr){
	if (expr->no_ref_variant) return;
	ref_stack();
}

static void check_string(Expression*& expr){
	TYPE t = expr->type;
	if (!TYPE_is_string(t) && t != T_NULL && t != T_VARIANT)
		THROW(E_TYPE, JIF.F_TYPE_get_name(T_STRING), JIF.F_TYPE_get_name(t));
	if (TYPE_is_string(t))
		return;
	if (t == T_NULL){
		//abort();
		//FIXME is this good?
		assert(isa<PushNullExpression>(expr));
		//delete expr;
		expr = new PushCStringExpression(NULL, 0, 0);
		return;
	}
	if (t == T_VARIANT){
		ref_variant(expr);
		expr->must_on_stack();
	}
	expr = new CheckStringExpression(expr);
}

static void check_integer(Expression*& expr){
	TYPE t = expr->type;
	if (!TYPE_is_integer(t) && t != T_VARIANT)
		THROW(E_TYPE, JIF.F_TYPE_get_name(T_INTEGER), JIF.F_TYPE_get_name(t));
	if (t == T_VARIANT){
		ref_variant(expr);
		expr->must_on_stack();
		expr = new CheckIntegerExpression(expr);
	}
}

static void check_float(Expression*& expr){
	TYPE t = expr->type;
	if (!TYPE_is_number(t) && t != T_VARIANT)
		THROW(E_TYPE, JIF.F_TYPE_get_name(T_FLOAT), JIF.F_TYPE_get_name(t));
	if (t == T_VARIANT){
		ref_variant(expr);
		expr->must_on_stack();
		expr = new CheckFloatExpression(expr);
	} else
		JIT_conv(expr, T_FLOAT);
}

static void check_pointer(Expression*& expr){
	TYPE t = expr->type;
	if (t != T_POINTER && t != T_VARIANT)
		THROW(E_TYPE, "Pointer", JIF.F_TYPE_get_name(t));
	if (t == T_VARIANT){
		ref_variant(expr);
		expr->must_on_stack();
		expr = new CheckPointerExpression(expr);
	}
}

static TYPE check_good_type(TYPE t1, TYPE t2){
	if (t1 == T_CSTRING) t1 = T_STRING;
	if (t2 == T_CSTRING) t2 = T_STRING;
	
	if (t1 == t2){
		
	} else if (t1 == T_NULL){
		if (t2 <= T_FLOAT)
			return T_VARIANT;
		t1 = t2;
	} else if (t1 <= T_FLOAT && t2 <= T_FLOAT){
		if (t2 > t1)
			t1 = t2;
	} else if (t2 == T_NULL){
		if (t1 <= T_FLOAT)
			return T_VARIANT;
	} else if (TYPE_is_object(t1) && TYPE_is_object(t2)){
		return T_OBJECT;
	} else
		return T_VARIANT;
	
	if (t1 == T_VOID)
		THROW(E_NRETURN);
	else if (t1 > T_VARIANT && t1 < T_OBJECT)
		THROW(E_TYPE, "Standard type", JIF.F_TYPE_get_name(t1));
	
	return t1;
}

AddQuickExpression::AddQuickExpression(Expression* val, int add) : expr(val), add(add) {
	no_ref_variant = true;
	
	if (TYPE_is_string(expr->type) || expr->type == T_DATE)
		JIT_conv(expr, T_FLOAT);
	
	if (expr->type <= T_BOOLEAN || expr->type > T_VARIANT)
		THROW(E_TYPE, "Number", JIF.F_TYPE_get_name(expr->type));
	
	type = expr->type;
	
	if (type == T_VARIANT)
		expr->must_on_stack();
}
PushStaticExpression::PushStaticExpression(int index) {
	CLASS_VAR* var = &CP->load->stat[index];
	ctype = &var->type;
	klass = CP;
	type = ctype_to_type(ctype);
	addr = (char*)CP->stat + var->pos;
}
PopStaticExpression::PopStaticExpression(Expression* val, int index) : val(val) {
	CLASS_VAR* var = &CP->load->stat[index];
	CTYPE* ctype = &var->type;
	if (ctype->id == TC_ARRAY || ctype->id == TC_STRUCT)
		THROW_ILLEGAL();
	type = ctype_to_type(ctype);
	addr = (char*)CP->stat + var->pos;
	JIT_conv(this->val, type);
}
PushDynamicExpression::PushDynamicExpression(int index) : index(index) {
	CLASS_VAR* var = &CP->load->dyn[index];
	ctype = &var->type;
	type = ctype_to_type(ctype);
	offset = var->pos;
}
PopDynamicExpression::PopDynamicExpression(Expression* val, int index) : val(val), index(index) {
	CLASS_VAR* var = &CP->load->dyn[index];
	CTYPE* ctype = &var->type;
	if (ctype->id == TC_ARRAY || ctype->id == TC_STRUCT)
		THROW_ILLEGAL();
	type = ctype_to_type(ctype);
	offset = var->pos;
	JIT_conv(this->val, type);
}
PushFunctionExpression::PushFunctionExpression(int index) : index(index) {
	type = T_FUNCTION;
	
	function_class = CP;
	function_object = NULL; //Will be set later
	function_kind = FUNCTION_PRIVATE;
	function_defined = true;
	function_unknown = NULL;
	function_index = index;
	function_expr_type = PrivateFn;
}
PushLocalExpression::PushLocalExpression(int index) : index(index) {
	if (index >= FP->n_local){
		type = get_ctrl_type(index);
	} else {
		CLASS_LOCAL* var = &FP->local[index];
		CTYPE* ctype = &var->type;
		type = ctype_to_type(ctype);
	}
}
PopLocalExpression::PopLocalExpression(Expression* val, int index) : val(val), index(index) {
	CLASS_LOCAL* var = &FP->local[index];
	CTYPE* ctype = &var->type;
	type = ctype_to_type(ctype);
	JIT_conv(this->val, type);
}
PopParamExpression::PopParamExpression(Expression* val, int index) : val(val), index(index) {
	type = FP->param[index + FP->n_param].type;
	JIT_conv(this->val, type);
}
PopCtrlExpression::PopCtrlExpression(Expression* val, int index) : val(val), index(index) {
	CLASS* klass = NULL;
	if (auto pce = dyn_cast<PushClassExpression>(val)){
		klass = pce->klass;
	}
	set_ctrl_type(val->type, index, klass);
	type = val->type;
}
PopOptionalExpression::PopOptionalExpression(Expression* val, int index) : val(val), index(index) {
	type = FP->param[FP->n_param + index].type;
	if (val->type == T_VOID){
		is_default = true;
	} else {
		is_default = false;
		JIT_conv(this->val, type);
	}
}
PushEventExpression::PushEventExpression(int ind, const char* unknown_name) : index(ind) {
	//type = CP->load->event[index].type;
	type = T_FUNCTION;
	if (unknown_name){
		index = CLASS_find_symbol(CP, unknown_name);
		if (index == NO_SYMBOL)
			THROW(E_DYNAMIC, CP->name, unknown_name);
		
		CLASS_DESC* desc = CP->table[index].desc;
		if (CLASS_DESC_get_type(desc) != CD_EVENT)
			THROW(E_DYNAMIC, CP->name, unknown_name);
		
		index = desc->event.index;
	} else if (CP->parent){
		index += CP->parent->n_event;
	}
	
	function_kind = FUNCTION_EVENT;
	function_expr_type = EventFn;
}
PushArrayExpression::PushArrayExpression(Expression** it, int nargs){
	args.resize(nargs);
	for(int i=0; i<nargs; i++){
		args[i] = it[i];
	}
	CLASS* klass = NULL;
	bool is_push_class;
	
	bool is_push_dynamic = false;
	
	if ( ((is_push_dynamic = isa<PushDynamicExpression>(args[0])) && ((PushDynamicExpression*)args[0])->ctype->id == TC_ARRAY)
		||
		(dynamic_cast<ReadVariableExpression*>(args[0]) && ((ReadVariableExpression*)args[0])->ctype->id == TC_ARRAY) ){
		CTYPE* ctype;
		CLASS* klass;
		if (is_push_dynamic){
			ctype = &CP->load->dyn[((PushDynamicExpression*)args[0])->index].type;
			klass = CP;
		} else {
			ctype = ((ReadVariableExpression*)args[0])->ctype;
			klass = ((ReadVariableExpression*)args[0])->klass;
		}
		
		CLASS_ARRAY* ca = klass->load->array[ctype->value];
		int* dim = ca->dim;
		
		int ndim = 1;
		while(*dim++ > 0)
			ndim++;
		
		if (nargs-1 != ndim)
			THROW(E_NDIM);
		
		for(int i=0; i<ndim; i++){
			JIT_conv(args[i+1], T_INTEGER);
		}
		
		type = ctype_to_type(&ca->ctype, klass);
	} else if (TYPE_is_pure_object(args[0]->type)){
		klass = (CLASS*)(void*)args[0]->type;
		
		if (klass->quick_array == CQA_ARRAY){
			ref_stack();
			type = klass->array_type;
			for(int i=1; i<nargs; i++)
				JIT_conv(args[i], T_INTEGER);
			if (nargs > 2)
				for(int i=1; i<nargs; i++)
					args[i]->must_on_stack();
		} else if (klass->quick_array == CQA_COLLECTION){
			if (nargs > 2)
				THROW(E_TMPARAM);
			JIT_conv(args[1], T_STRING);
			on_stack = true;
			type = T_VARIANT;
		} else {
			klass = (CLASS*)(void*)args[0]->type;
			is_push_class = false;
			goto _SPEC_GET_METHOD;
		}
	} else if (PushClassExpression* pce = dyn_cast<PushClassExpression>(args[0])){
		klass = pce->klass;
		is_push_class = true;
		goto _SPEC_GET_METHOD;
	} else if (args[0]->type == T_OBJECT || args[0]->type == T_VARIANT){
		pc = get_current_read_pos();
		*pc |= 3 << 6;
		type = T_VARIANT;
		on_stack = true;
		for(int i=0; i<nargs; i++){
			args[i]->must_on_stack();
		}
	} else {
		THROW(E_NOBJECT);
	}
	
	return;
	
	_SPEC_GET_METHOD: {
		int index = klass->special[SPEC_GET];
		
		if (index == NO_SYMBOL)
			THROW(E_NARRAY, klass->name);
		
		ref_stack();
		
		CLASS_DESC* desc = CLASS_get_desc(klass, index);
		
		type = desc->method.type;
		
		if (is_push_class){
			if (CLASS_DESC_get_type(desc) != CD_STATIC_METHOD){
				if (!klass->auto_create)
					THROW(E_NOBJECT);
				//delete args[0];
				args[0] = new PushAutoCreateExpression(klass);
			}
		} else {
			if (CLASS_DESC_get_type(desc) == CD_STATIC_METHOD && !klass->is_virtual)
				THROW(E_NARRAY);
		}
		
		nargs--;
		
		if (nargs < desc->method.npmin)
			THROW(E_NEPARAM);
		if (nargs > desc->method.npmax && !desc->method.npvar)
			THROW(E_TMPARAM);
		
		can_quick = !(desc->method.npmin < desc->method.npmax || nargs != desc->method.npmin || desc->method.npvar);
		
		if (can_quick){
			for(int i=0; i<nargs; i++){
				JIT_conv(args[i+1], desc->method.signature[i]);
			}
		}
		on_stack = true;
		for(uint i=0; i<args.size(); i++)
			args[i]->must_on_stack();
	}
}
PopArrayExpression::PopArrayExpression(Expression** it, int nargs, Expression* val) : val(val) {
	args.resize(nargs);
	for(int i=0; i<nargs; i++){
		args[i] = it[i];
	}
	CLASS* klass = NULL;
	bool is_push_class;
	
	bool is_push_dynamic = false;
	if ( ((is_push_dynamic = isa<PushDynamicExpression>(args[0])) && ((PushDynamicExpression*)args[0])->ctype->id == TC_ARRAY)
		||
		(dynamic_cast<ReadVariableExpression*>(args[0]) && ((ReadVariableExpression*)args[0])->ctype->id == TC_ARRAY) ){
		CTYPE* ctype;
		CLASS* klass;
		if (is_push_dynamic){
			ctype = &CP->load->dyn[((PushDynamicExpression*)args[0])->index].type;
			klass = CP;
		} else {
			ctype = ((ReadVariableExpression*)args[0])->ctype;
			klass = ((ReadVariableExpression*)args[0])->klass;
		}
		
		CLASS_ARRAY* ca = klass->load->array[ctype->value];
		int* dim = ca->dim;
		
		int ndim = 1;
		while(*dim++ > 0)
			ndim++;
		
		if (nargs-1 != ndim)
			THROW(E_NDIM);
		
		for(int i=0; i<ndim; i++){
			JIT_conv(args[i+1], T_INTEGER);
		}
		
		if (ca->ctype.id == TC_STRUCT)
			THROW_ILLEGAL();
		
		type = ctype_to_type(&ca->ctype, klass);
		
		JIT_conv(this->val, type);
	} else if (TYPE_is_pure_object(args[0]->type)){
		klass = (CLASS*)(void*)args[0]->type;
		
		if (klass->quick_array == CQA_ARRAY){
			type = klass->array_type;
			JIT_conv(this->val, type);
			ref_stack();
			for(int i=1; i<nargs; i++)
				JIT_conv(args[i], T_INTEGER);
			if (nargs > 2)
				for(int i=1; i<nargs; i++)
					args[i]->must_on_stack();
		} else if (klass->quick_array == CQA_COLLECTION){
			if (nargs > 2)
				THROW(E_TMPARAM);
			JIT_conv(this->val, T_VARIANT);
			JIT_conv(args[1], T_STRING);
			this->val->must_on_stack();
		} else {
			klass = (CLASS*)(void*)args[0]->type;
			is_push_class = false;
			goto _SPEC_PUT_METHOD;
		}
	} else if (PushClassExpression* pce = dyn_cast<PushClassExpression>(args[0])){
		klass = pce->klass;
		is_push_class = true;
		goto _SPEC_PUT_METHOD;
	} else if (args[0]->type == T_OBJECT || args[0]->type == T_VARIANT){
		pc = get_current_read_pos();
		*pc |= 3 << 6;
		type = T_VARIANT;
		on_stack = true;
		this->val->must_on_stack();
		for(int i=0; i<nargs; i++){
			args[i]->must_on_stack();
		}
	} else {
		THROW(E_NOBJECT);
	}
	
	return;
	
	_SPEC_PUT_METHOD: {
		int index = klass->special[SPEC_PUT];
		
		if (index == NO_SYMBOL)
			THROW(E_NARRAY, klass->name);
		
		ref_stack();
		
		CLASS_DESC* desc = CLASS_get_desc(klass, index);
		
		type = desc->method.type;
		
		if (is_push_class){
			if (CLASS_DESC_get_type(desc) != CD_STATIC_METHOD){
				if (!klass->auto_create)
					THROW(E_NOBJECT);
				//delete args[0];
				args[0] = new PushAutoCreateExpression(klass);
			}
		} else {
			if (CLASS_DESC_get_type(desc) == CD_STATIC_METHOD && !klass->is_virtual)
				THROW(E_NARRAY);
		}
		
		//nargs contains the number of parameters, since object and value are swapped
		
		if (nargs < desc->method.npmin)
			THROW(E_NEPARAM);
		if (nargs > desc->method.npmax && !desc->method.npvar)
			THROW(E_TMPARAM);
		
		can_quick = !(desc->method.npmin < desc->method.npmax || nargs != desc->method.npmin || desc->method.npvar);
		
		if (can_quick){
			JIT_conv(this->val, desc->method.signature[0]);
			for(int i=1; i<nargs; i++){
				JIT_conv(args[i], desc->method.signature[i]);
			}
		}
		on_stack = true;
		for(int i=0; i<nargs; i++)
			args[i]->must_on_stack();
		this->val->must_on_stack();
	}
}
/*PushPureObjectUnknownFunctionExpression::PushPureObjectUnknownFunctionExpression(Expression* obj) : obj(obj){
	CLASS* klass = (CLASS*)(void*)obj->type;
	
	type = T_FUNCTION;
	if (klass->must_check){
		ref_stack();
		obj->must_on_stack();
	}
	
	function_class = klass;
	function_object = obj;
	function_defined = true;
	function_expr_type = UnknownFn;
}*/
PushPureObjectFunctionExpression::PushPureObjectFunctionExpression(Expression* obj, int index, const char* unknown_name)
: PushPureObjectExpression(obj, index){
	type = T_FUNCTION;
	if (klass()->must_check){
		ref_stack();
		obj->must_on_stack();
	}
	
	function_class = klass();
	function_object = obj;
	function_desc = desc();
	function_kind = desc()->method.native ? -1 : FUNCTION_PUBLIC;
	function_defined = !unknown_name;
	function_unknown = unknown_name;
	function_index = index;
	function_expr_type = PureObjectFn;
}
PushPureObjectStaticFunctionExpression::PushPureObjectStaticFunctionExpression(Expression* obj, int index, const char* unknown_name)
: PushPureObjectExpression(obj, index){
	type = T_FUNCTION;
	
	function_class = klass();
	function_object = obj;
	function_desc = desc();
	function_kind = desc()->method.native ? -1 : FUNCTION_PUBLIC;
	function_defined = !unknown_name;
	function_unknown = unknown_name;
	function_index = index;
	function_expr_type = PureObjectStaticFn;
}
PushVirtualFunctionExpression::PushVirtualFunctionExpression(Expression* obj, int index, const char* unknown_name)
: PushPureObjectExpression(obj, index) {
	type = T_FUNCTION;
	
	function_class = klass();
	function_object = obj;
	function_desc = desc();
	function_kind = FUNCTION_NATIVE;
	function_defined = !unknown_name;
	function_unknown = unknown_name;
	function_index = index;
	function_expr_type = VirtualObjectFn;
}
PushVirtualStaticFunctionExpression::PushVirtualStaticFunctionExpression(Expression* obj, int index, const char* unknown_name)
: PushPureObjectExpression(obj, index) {
	//type = desc()->method.type;
	type = T_FUNCTION;
	
	function_class = klass();
	function_object = obj;
	function_desc = desc();
	function_kind = FUNCTION_NATIVE;
	function_defined = !unknown_name;
	function_unknown = unknown_name;
	function_index = index;
	function_expr_type = VirtualObjectStaticFn;
}
PushStaticPropertyExpression::PushStaticPropertyExpression(Expression* obj, int index)
: obj(obj), index(index) {
	CLASS* c = ((PushClassExpression*)obj)->klass;
	CLASS_DESC* desc = c->table[index].desc;
	type = desc->property.type;
	ref_stack();
	obj->must_on_stack();
}
PushStaticFunctionExpression::PushStaticFunctionExpression(Expression* obj, int index, const char* unknown_name)
: obj(obj), index(index) {
	//type = desc()->method.type;
	type = T_FUNCTION;
	ref_stack();
	obj->must_on_stack();
	
	function_class = klass();
	function_object = obj;
	function_desc = desc();
	function_kind = function_desc->method.native ? (function_desc->method.subr ? FUNCTION_SUBR : FUNCTION_NATIVE) : FUNCTION_PUBLIC;
	function_defined = true;
	function_unknown = unknown_name;
	function_index = index;
	function_expr_type = StaticFn;
}
PopVirtualPropertyExpression::PopVirtualPropertyExpression(Expression* obj, Expression* val, int index, const char* name, bool is_static)
: PopPureObjectExpression(obj, val, index), name(name), is_static(is_static) {
	type = desc()->property.type;
	ref_stack();
	obj->must_on_stack();
	this->val->must_on_stack();
	JIT_conv(this->val, type);
}
PopPureObjectPropertyExpression::PopPureObjectPropertyExpression(Expression* obj, Expression* val, int index)
: PopPureObjectExpression(obj, val, index) {
	type = desc()->property.type;
	ref_stack();
	val->must_on_stack();
	obj->must_on_stack();
	JIT_conv(this->val, type);
}
PopStaticPropertyExpression::PopStaticPropertyExpression(CLASS* klass, Expression* val, int index)
: klass(klass), val(val), index(index) {
	CLASS_DESC* desc = klass->table[index].desc;
	type = desc->property.type;
	ref_stack();
	JIT_conv(this->val, type);
	this->val->must_on_stack();
}
CallExpression::CallExpression(Expression* function, int nargs, Expression** it, unsigned short* pc) : func(function), pc(pc) {
	on_stack = true; //Gets replaced by false if EventFn or Extern
	desc = NULL;
	variant_call = false;
	ref_stack();
	args.resize(nargs);
	for(int i=0; i<nargs; i++){
		args[i] = it[i];
	}
	
	if (PushExternExpression* ee = dynamic_cast<PushExternExpression*>(func)){
		CLASS_EXTERN* ext;
		if (ee->object_to_release)
			ext = &ee->klass->load->ext[ee->klass->table[ee->index].desc->ext.exec]; //An extern method with the correct signature
		else 
			ext = &ee->klass->load->ext[ee->index];
		
		if (nargs < ext->n_param)
			THROW(E_NEPARAM);
		if (!ext->vararg && nargs > ext->n_param)
			THROW(E_TMPARAM);
		
		for(int i=0; i<ext->n_param; i++){
			TYPE t = ext->param[i].type;
			
			if (t == T_VOID || t == T_DATE || t == T_VARIANT || t == T_CLASS || t == T_FUNCTION)
				THROW(E_UTYPE);
			
			JIT_conv(args[i], t);
			
			if (args[i]->type == T_STRING || TYPE_is_object(t))
				args[i]->must_on_stack();
		}
		
		for(int i=ext->n_param; i<nargs; i++){
			TYPE t = args[i]->type;
			
			if (t == T_VOID || t == T_DATE || t == T_CLASS || t == T_FUNCTION)
				THROW(E_UTYPE);
			
			if (t == T_VARIANT)
				ee->must_variant_dispatch = true;
			
			if (args[i]->type == T_STRING || TYPE_is_object(t))
				args[i]->must_on_stack();
		}
		
		if (ee->must_variant_dispatch)
			for(int i=0; i<nargs; i++)
				args[i]->must_on_stack();
		
		type = ext->type;
		on_stack = false;
		return;
	}
	
	if (TYPE_is_pure_object(func->type)){
		JIT_load_class((CLASS*)(void*)func->type);
		CLASS_DESC_METHOD* method_desc = JR_CLASS_get_special_desc((CLASS*)(void*)func->type, SPEC_CALL);
		if (method_desc == NULL)
			THROW(E_NFUNC);
		//Can be virtual static... See Draw.Clip._call
		//Atm no virtual non-static methods are written
		
		//Note: func can be Super
		
		
		desc = (CLASS_DESC*)(void*)method_desc; //Works, since CLASS_DESC is a union containing CLASS_DESC_METHOD
		klass = method_desc->klass;
		index = klass->special[SPEC_CALL];
		
		switch(CLASS_DESC_get_type(desc)){
			case CD_METHOD:
				if (klass->is_virtual)
					func = new PushVirtualFunctionExpression(func, index, NULL);
				else
					func = new PushPureObjectFunctionExpression(func, index, NULL);
				break;
			case CD_STATIC_METHOD:
				if (klass->is_virtual)
					func = new PushVirtualStaticFunctionExpression(func, index, NULL);
				else
					func = new PushPureObjectStaticFunctionExpression(func, index, NULL);
				break;
			default:
				THROW(E_NFUNC);
		}
	}
	func->must_on_stack();
	
	if (FunctionExpression* fe = dynamic_cast<FunctionExpression*>(func)){
		switch(fe->function_expr_type){
			case PrivateFn: {
				index = fe->function_index;
				kind = FUNCTION_PRIVATE;
				klass = CP;
				
				FUNCTION* fp = &CP->load->func[index];
				if (nargs < fp->npmin)
					THROW(E_NEPARAM);
				if (nargs > fp->n_param && !fp->vararg)
					THROW(E_TMPARAM);
				
				can_quick = !(fp->npmin < fp->n_param || nargs != fp->npmin || fp->vararg);
				if (can_quick){
					for(int i=0; i<nargs; i++)
						JIT_conv(args[i], fp->param[i].type);
				}
				type = fp->type;
				goto _SET_ON_STACK;
				return;
			}
			case EventFn: {
				on_stack = false;
				type = T_BOOLEAN;
				goto _SET_ON_STACK;
				return;
			}
			/*case UnknownFn: {
				type = T_VARIANT;
				goto _SET_ON_STACK;
			}*/
		}
		desc = fe->function_desc;
		index = fe->function_index;
		klass = fe->function_class;
		type = fe->function_unknown ? T_VARIANT : desc->method.type;
		
		goto _NORMAL_METHOD;
	}
	
	if (PushClassExpression* ce = dyn_cast<PushClassExpression>(func)){
		int special_call_function_index = ce->klass->special[SPEC_CALL];
		if (special_call_function_index == NO_SYMBOL){
			//Then it is a cast: Dim obj2 As SomeClass = SomeClass(obj1);
			if (nargs != 1)
				THROW(E_NFUNC);
			
			desc = NULL;
			type = (TYPE)(void*)ce->klass;
			JIT_conv(args[0], (TYPE)(void*)ce->klass);
			return;
		}
		CLASS_DESC_METHOD* method_desc = &CLASS_get_desc(ce->klass, special_call_function_index)->method;
		desc = (CLASS_DESC*)(void*)method_desc; //Works, since CLASS_DESC is a union containing CLASS_DESC_METHOD
		index = special_call_function_index;
		klass = method_desc->klass;
		type = method_desc->type;
		
		goto _NORMAL_METHOD;
	}
	
	if (func->type != T_OBJECT && func->type != T_VARIANT)
		THROW(E_NFUNC);
	else {
		type = T_VARIANT;
		can_quick = false;
		variant_call = true;
		goto _SET_ON_STACK;
	}
	
	return; //Line unreachable
	
	_NORMAL_METHOD:
	if (nargs < desc->method.npmin)
		THROW(E_NEPARAM);
	if (nargs > desc->method.npmax && !desc->method.npvar)
		THROW(E_TMPARAM);
	
	can_quick = !(desc->method.npmin < desc->method.npmax || nargs != desc->method.npmin || desc->method.npvar);
	
	if (can_quick){
		for(int i=0; i<nargs; i++){
			JIT_conv(args[i], desc->method.signature[i]);
		}
	}
	
	_SET_ON_STACK:
	for(int i=0; i<nargs; i++){
		args[i]->must_on_stack();
	}
}
JumpFirstExpression::JumpFirstExpression(int ctrl_to, Expression* to_expr, Expression* step_expr, int local_var, int body_addr, int done_addr)
: to(to_expr), step(step_expr), ctrl_to(ctrl_to), local_var(local_var), body_addr(body_addr), done_addr(done_addr) {
	TYPE type = FP->local[local_var].type.id;
	
	if (type < T_BYTE || type > T_FLOAT)
		THROW(E_TYPE, "Number", JIF.F_TYPE_get_name(type));
	
	if (TYPE_is_integer(type))
		JIT_conv(step, T_INTEGER);
	else
		JIT_conv(step, type);
	
	JIT_conv(to, type);
	
	set_ctrl_type(type, ctrl_to);
	set_ctrl_type(step->type, ctrl_to+1);
	
	mark_address_taken(body_addr);
	mark_address_taken(done_addr);
}
JumpEnumNextExpression::JumpEnumNextExpression(JumpEnumFirstExpression* jfirst, int cont_addr, int addr, unsigned short* pc, bool drop, OnStackExpression* retval)
: jfirst(jfirst), retval(retval), cont_addr(cont_addr), addr(addr), pc(pc), drop(drop) {
	mark_address_taken(cont_addr);
	mark_address_taken(addr);
	TYPE obj_type = get_ctrl_type(jfirst->ctrl);
	
	CLASS* klass;
	
	if (obj_type == T_VARIANT || obj_type == T_OBJECT){
		defined = false;
		type = T_VARIANT;
	} else if (obj_type == T_CLASS) {
		defined = true;
		PushClassExpression* pce = dyn_cast<PushClassExpression>(jfirst->obj);
		assert(pce);
		klass = pce->klass;
		
		/*if (klass->auto_create && !klass->enum_static){
			delete pce;
			jfirst->obj = PushAutoCreate(klass);
			obj_type = (CLASS*)(void*)klass;
		}*/
	} else if (TYPE_is_pure_object(obj_type)){
		defined = true;
		klass = (CLASS*)(void*)obj_type;
	} else {
		THROW(E_NOBJECT);
	}
	
	if (defined){
		short index = klass->special[SPEC_NEXT];
		if (index == NO_SYMBOL)
			THROW(E_ENUM);
		
		CLASS_DESC* desc = CLASS_get_desc(klass, index);
		type = desc->method.type;
	}
	
	if (!drop)
		retval->type = type;
	
	set_ctrl_type(T_OBJECT, jfirst->ctrl+1);
}
NewExpression::NewExpression(Expression** it, int nargs, bool event) : event(event) {
	pc = get_current_read_pos();
	must_on_stack();
	args.resize(nargs);
	for(int i=0; i<nargs; i++){
		args[i] = it[i];
		args[i]->must_on_stack();
	}
	
	if (PushClassExpression* pce = dyn_cast<PushClassExpression>(args[0])){
		/*if (pce->klass->override)
			pce->klass = pce->klass->override;*/
		type = (TYPE)(void*)pce->klass;
	} else {
		type = T_OBJECT;
	}
}
ReturnExpression::ReturnExpression(Expression* expr, int kind) : retval(expr), kind(kind) {
	pc = get_current_read_pos();
	type = FP->type;
	if (retval){
		/*if (TYPE_is_pure_object(type) && ((CLASS*)type)->override)
			type = (TYPE)(((CLASS*)type)->override);*/
		JIT_conv(retval, type);
	}
}
QuitExpression::QuitExpression(Expression* quitval) : quitval(quitval) {
	if (quitval)
		JIT_conv(this->quitval, T_BYTE);
}
SubrExpression::SubrExpression(int digit, Expression** it, int nargs, int extra) : digit(digit), extra(extra) {
	args.resize(nargs);
	for(int i=0; i<nargs; i++){
		args[i] = it[i];
		//args[i]->must_on_stack();
	}
	
	switch(digit){
		case 0x41: //Mid
			if (nargs == 3)
				JIT_conv(args[2], T_INTEGER);
			//Fallthrough
			ref_stack();
		case 0x40: //Left
		case 0x42: //Right
			if (nargs != 1)
				JIT_conv(args[1], T_INTEGER);
			/*type = args[0]->type; //Want T_STRING or T_CSTRING
			if (type == T_VARIANT || type = T_NULL)
				type = T_STRING;
			if (!TYPE_is_string(type))
				THROW(E_TYPE, JIF.F_TYPE_get_name(T_STRING), JIF.F_TYPE_get_name(type));
			if (type == T_VARIANT)
				ref_stack();*/
			check_string(args[0]);
			type = args[0]->type == T_CSTRING ? T_CSTRING : T_STRING;
			break;
			
		case 0x43: //Len
			check_string(args[0]);
			type = T_INTEGER;
			break;
			
		case 0x45: //String
			check_string(args[1]);
			//Fallthrough
		case 0x44: //Space
			check_integer(args[0]);
			ref_stack();
			type = T_STRING;
			break;
			
		case 0x4A: //Asc
			check_string(args[0]);
			if (nargs == 2)
				check_integer(args[1]);
			type = T_INTEGER;
			break;
			
		case 0x46: //Trim
			check_string(args[0]);
			type = args[0]->type;
			break;
			
		case 0x48: //LCase
			extra = 1;
		case 0x47: //UCase
			check_string(args[0]);
			type = T_STRING;
			break;
			
		case 0x49: //Chr
			ref_stack();
			JIT_conv(args[0], T_INTEGER);
			type = T_CSTRING;
			break;
		
		case 0x4B: //Instr
		case 0x4C: //RInStr
			type = T_INTEGER;
			check_string(args[0]);
			check_string(args[1]);
			if (nargs >= 3)
				check_integer(args[2]);
			if (nargs == 4)
				check_integer(args[3]);
			break;
		
		case 0x4D: //Subst
			type = T_STRING;
			check_string(args[0]);
			for(int i=1; i<nargs; i++)
				JIT_conv(args[i], T_STRING);
			break;
		
		case 0x4E: //Replace
			type = T_STRING;
			check_string(args[0]);
			check_string(args[1]);
			check_string(args[2]);
			if (nargs == 4)
				check_integer(args[3]);
			break;
		
		case 0x4F: //Split
			type = (TYPE)(void*)GB.FindClass("String[]");
			JIT_conv(args[0], T_STRING);
			if (nargs >= 2){
				check_string(args[1]);
				if (nargs >= 3){
					check_string(args[2]);
					if (nargs >= 4){
						JIT_conv(args[3], T_BOOLEAN);
						if (nargs == 5)
							JIT_conv(args[4], T_BOOLEAN);
					}
				}
			}
			break;
		
		case 0x50: //Scan
			type = (TYPE)(void*)GB.FindClass("String[]");
			check_string(args[0]);
			check_string(args[1]);
			break;
		
		case 0x51: //Comp
			type = T_INTEGER;
			JIT_conv(args[0], T_STRING);
			JIT_conv(args[1], T_STRING);
			if (nargs == 2)
				check_integer(args[2]);
			break;
		
		case 0x52: //Conv
			ref_stack();
			type = T_STRING;
			check_string(args[0]);
			check_string(args[1]);
			check_string(args[2]);
			break;
		
		case 0x53: //DConv
			ref_stack();
			type = T_STRING;
			check_string(args[0]);
			break;
		
		case 0x54: //Abs
		case 0x55: //Int
		case 0x56: //Fix
			pc = get_current_read_pos();
			type = args[0]->type;
			if (!TYPE_is_number(type) && type != T_VARIANT)
				THROW(E_TYPE, "Number", JIF.F_TYPE_get_name(type));
			if (type == T_VARIANT){
				args[0]->must_on_stack();
				no_ref_variant = true;
				on_stack = true;
			}
			break;
		
		case 0x57: //Sgn
			type = T_INTEGER;
			if (!TYPE_is_number(args[0]->type) && args[0]->type != T_VARIANT)
				THROW(E_TYPE, "Number", JIF.F_TYPE_get_name(args[0]->type));
			if (args[0]->type == T_VARIANT){
				args[0]->must_on_stack();
				no_ref_variant = true;
				on_stack = true;
			}
			break;
		
		case 0x58: //Math
			ref_stack();
			type = T_FLOAT;
			JIT_conv(args[0], T_FLOAT);
			break;
		
		case 0x59: //Pi
			type = T_FLOAT;
			if (nargs == 1)
				JIT_conv(args[0], T_FLOAT);
			break;
		
		case 0x5A: //Round
			type = T_FLOAT;
			JIT_conv(args[0], T_FLOAT);
			if (nargs == 2){
				check_integer(args[1]);
				JIT_conv(args[1], T_INTEGER);
			}
			break;
		
		case 0x5B: //Randomize
			type = T_VOID;
			if (nargs != 0){
				check_integer(args[0]);
				JIT_conv(args[0], T_INTEGER);
			}
			break;
		
		case 0x5C: //Rnd
			type = T_FLOAT;
			if (nargs >= 1)
				JIT_conv(args[0], T_FLOAT);
			if (nargs == 2)
				JIT_conv(args[1], T_FLOAT);
			break;
		
		case 0x5D: //Min
		case 0x5E: { //Max
			TYPE t1 = args[0]->type, t2 = args[1]->type;
			
			if (!TYPE_is_number_date(t1) && t1 != T_VARIANT)
				THROW(E_TYPE, "Number or date", JIF.F_TYPE_get_name(t1));
			if (!TYPE_is_number_date(t2) && t2 != T_VARIANT)
				THROW(E_TYPE, "Number or date", JIF.F_TYPE_get_name(t2));
			
			type = Max(t1, t2);
			if (type != T_VARIANT){
				if (type == T_SINGLE)
					type = T_FLOAT;
				if (t1 != type) JIT_conv(args[0], type);
				else if (t2 != type) JIT_conv(args[1], type);
			} else {
				/*if (t1 == T_VARIANT && !args[0]->no_ref_variant)
					ref_variant(args[0]);
				if (t2 == T_VARIANT && !args[1]->no_ref_variant)
					ref_variant(args[1]);*/
				ref_stack();
				if (t1 == T_VARIANT)
					args[0]->must_on_stack();
				if (t2 == T_VARIANT)
					args[1]->must_on_stack();
				no_ref_variant = true;
			}
			break;
		}
		
		case 0x5F: //IIf
			if (args[0]->type == T_VOID || args[1]->type == T_VOID || args[2]->type == T_VOID)
				THROW(E_NRETURN);
			type = check_good_type(args[1]->type, args[2]->type);
			JIT_conv(args[0], T_BOOLEAN);
			if (type == T_VARIANT && (args[1]->type != T_VARIANT || args[2]->type != T_VARIANT)){
				/*ref_stack();
				args[1]->ref_on_stack();
				on_stack = true;*/
			} else {
				JIT_conv(args[1], type);
				JIT_conv(args[2], type);
			}
			break;
		
		case 0x60: //Choose
			JIT_conv(args[0], T_INTEGER);
			type = args[1]->type;
			extra = 1; //All same type
			for(int i=1; i<nargs; i++){
				if (args[i]->type == T_VOID)
					THROW(E_NRETURN);
				if (args[i]->type != type && !(TYPE_is_string(type) && TYPE_is_string(args[i]->type)))
					extra = 0; //Different types
			}
			if (extra == 0 || nargs > 3)
				type = T_VARIANT;
			break;
		
		case 0x61: { //Array
			for(int i=0; i<nargs; i++) if (args[i]->type == T_VOID)
				THROW(E_NRETURN);
			type2 = args[0]->type;
			if (type2 > T_VARIANT && type2 < T_OBJECT)
				THROW(E_TYPE, "Standard type", JIF.F_TYPE_get_name(type2));
			for(int i=1; i<nargs && type != T_VARIANT; i++){
				type2 = check_good_type(type2, args[i]->type);
			}
			if (type2 == T_CSTRING)
				type2 = T_STRING;
			for(int i=0; i<nargs; i++)
				JIT_conv(args[i], type2);
			
			CTYPE ctype;
			ctype.id = T_NULL;
			type = (TYPE)(void*)JIF.F_CARRAY_get_array_class((CLASS*)(void*)type2, ctype);
			//ref_stack();
			break;
		}
		
		case 0x62: //Math2
			type = T_FLOAT;
			JIT_conv(args[0], T_FLOAT);
			JIT_conv(args[1], T_FLOAT);
			ref_stack();
			break;
		
		case 0x63: //IsAscii, IsLetter ...
			type = T_BOOLEAN;
			JIT_conv(args[0], T_STRING);
			break;
		
		case 0x64: //Bit
			type = args[0]->type;
			if (type != T_VARIANT && (type <= T_BOOLEAN || type > T_LONG))
				THROW(E_TYPE, "Number", JIF.F_TYPE_get_name(type));
			if (type == T_VARIANT && (extra & 0x1F) != 3)
				no_ref_variant = true;
			ref_stack();
			JIT_conv(args[1], T_INTEGER);
			
			if ((extra & 0x1F) == 3) //BTst
				type = T_BOOLEAN;
			break;
		
		case 0x65: //IsBoolean ...
			type = T_BOOLEAN;
			if (extra != T_NULL)
				check_string(args[0]);
			break;
		
		case 0x66: //TypeOf
			type = T_INTEGER;
			if (extra){
				check_integer(args[0]);
				JIT_conv(args[0], T_INTEGER);
			}
			break;
		
		/*case 0x67: //CBool ...
			JIT_conv(args[0], extra);
			type = extra;
			break;
		*/ //Done in jit_read.cpp
		
		case 0x68: //Bin
		case 0x69: //Hex
			type = T_STRING;
			JIT_conv(args[0], T_LONG);
			if (nargs == 2){
				ref_stack();
				JIT_conv(args[1], T_INTEGER);
			}
			break;
		
		case 0x6A: //Val
			check_string(args[0]);
			type = T_VARIANT;
			no_ref_variant = true;
			break;
		
		case 0x6B: //Str
			type = T_STRING;
			if (args[0]->type == T_VOID)
				THROW(E_NRETURN);
			break;
		
		case 0x6C: //Format
			type = T_STRING;
			ref_stack();
			break;
		
		case 0x6D: //Timer
			type = T_FLOAT;
			break;
		
		case 0x6E: //Now
			type = T_DATE;
			break;
		
		case 0x6F: //Year
			JIT_conv(args[0], T_DATE);
			type = T_INTEGER;
			break;
		
		case 0x70: //Week
			if (nargs >= 1){
				JIT_conv(args[0], T_DATE);
				if (nargs >= 2){
					check_integer(args[1]);
					if (nargs == 3)
						JIT_conv(args[2], T_BOOLEAN);
				}
			}
			type = T_INTEGER;
			break;
		
		case 0x71: //Date
		case 0x72: //Time
			if (nargs == 1)
				JIT_conv(args[0], T_DATE);
			else if (nargs >= 3){
				for(int i=0; i<nargs; i++)
					JIT_conv(args[i], T_INTEGER);
			} else if (nargs == 2)
				THROW(E_NEPARAM);
			type = T_DATE;
			break;
		
		case 0x73: //DateAdd, DateDiff
			ref_stack();
			JIT_conv(args[0], T_DATE);
			if (extra == 0){ //DateAdd
				check_integer(args[1]);
				JIT_conv(args[1], T_INTEGER);
				type = T_DATE;
			} else if (extra == 1){ //DateDiff
				JIT_conv(args[1], T_DATE);
				type = T_INTEGER;
			}
			check_integer(args[2]);
			JIT_conv(args[2], T_INTEGER);
			break;
		
		case 0x74: //Eval
			check_string(args[0]);
			if (nargs == 2)
				JIT_conv(args[1], (TYPE)(void*)GB.FindClass("Collection"));
			type = T_VARIANT;
			ref_stack();
			break;
		
		case 0x75: //Error
			type = T_BOOLEAN;
			break;
		
		case 0x76: //Debug
			pc = get_current_read_pos();
			type = T_VOID;
			break;
			
		case 0x77: //Wait
			if (nargs != 0)
				check_float(args[0]);
			type = T_VOID;
			break;
		
		case 0x78: //Open
			check_integer(args[1]);
			if (extra && args[0]->type != T_POINTER)
				THROW(E_TYPE, "Pointer", JIF.F_TYPE_get_name(args[0]->type));
			else if (extra == 0)
				check_string(args[0]);
			type = (TYPE)(void*)GB.FindClass("File");
			//ref_stack(); Nothing else can be on the stack ;)
			break;
		
		case 0x79: //Close
			//ref_stack();
			type = T_VOID;
			break;
		
		case 0x7A: //Input
			//ref_stack();
			type = -1; //Unknown type
			break;
		
		case 0x7B: //Line Input
			//ref_stack();
			type = T_STRING;
			break;
		
		case 0x7C: //Print
			type = T_VOID;
			if (nargs < 1)
				THROW(E_NEPARAM);
			for(int i=1; i<nargs; i++){
				//FIXME
				if (args[i]->type == T_FUNCTION)
					assert(false && "Jit compiler cannot call Print with a Function as argument yet.");
				if (args[i]->type == T_CLASS)
					assert(false && "Jit compiler cannot call Print with a Class as argument yet.");
			}
			break;
		
		case 0x7D: //Read
			if (extra){
				//ReadBytes
				JIT_conv(args[1], T_INTEGER);
				type = T_STRING;
			} else {
				//Read
				if (args[1]->type == T_INTEGER){
					auto int_expr = dyn_cast<PushIntegerExpression>(args[1]);
					assert(int_expr != NULL);
					type = int_expr->i;
				} else if (args[1]->type == T_CLASS){
					auto class_expr = dyn_cast<PushClassExpression>(args[1]);
					assert(class_expr != NULL);
					type = (TYPE)(void*)class_expr->klass;
				} else
					THROW_ILLEGAL();
			}
			break;
		
		case 0x7E: //Write
			if (extra){
				//WriteBytes
				JIT_conv(args[2], T_INTEGER);
				if (args[1]->type != T_POINTER)
					check_string(args[1]);
			} else {
				//Write
				TYPE type;
				if (args[2]->type == T_INTEGER){
					auto int_expr = dyn_cast<PushIntegerExpression>(args[2]);
					assert(int_expr != NULL);
					type = int_expr->i;
				} else if (args[2]->type == T_CLASS){
					auto class_expr = dyn_cast<PushClassExpression>(args[2]);
					assert(class_expr != NULL);
					type = (TYPE)(void*)class_expr->klass;
				} else
					THROW_ILLEGAL();
				JIT_conv(args[1], type);
			}
			type = T_VOID;
			break;
		
		case 0x7F: //Flush
			type = T_VOID;
			break;
		
		case 0x80: //Lock
			if (extra){
				//Unlock
				type = T_VOID;
			} else {
				//Lock
				check_string(args[0]);
				type = (TYPE)(void*)GB.FindClass("File");
			}
			break;
		
		case 0x81: //Input From, Output To, Error To
			//extra contains 0, 1 or 2
			type = T_VOID;
			break;
		
		case 0x82: //Eof
			ref_stack();
			type = T_BOOLEAN;
			break;
		
		case 0x83: //Lof
			ref_stack();
			type = T_LONG;
			break;
		
		case 0x84: //Seek
			ref_stack();
			if (nargs >= 2){
				JIT_conv(args[1], T_LONG);
				if (nargs == 3){
					JIT_conv(args[2], T_INTEGER);
				}
				type = T_VOID;
			} else
				type = T_LONG;
			break;
		
		case 0x85: //Kill
		case 0x87: //Rmdir, deprecated
			//New syntax: Kill(0) = Kill, Kill(1) = Mkdir, Kill(2) = Rmdir
			//extra = 0, 1 or 2
			check_string(args[0]);
			type = T_VOID;
			break;
			
		case 0x86: //Mkdir, deprecated -> Even() & Odd()
			if (extra == 0){
				check_string(args[0]);
				type = T_VOID;
			} else if (extra == 1 || extra == 2){
				JIT_conv(args[0], T_LONG);
				type = T_BOOLEAN;
			} else {
				THROW_ILLEGAL();
			}
			break;
		
		case 0x88: //Move
		case 0x89: //Copy, deprecated
			//New syntax: func = [Move, Copy, Link, Chmod, Chown, Chgrp][extra]
			check_string(args[0]);
			check_string(args[1]);
			type = T_VOID;
			break;
			
		case 0x8A: //Link, deprecated -> IsNan() & IsInf()
			if (extra == 0){
				check_string(args[0]);
				check_string(args[1]);
				type = T_VOID;
			} else if (extra == 1){
				JIT_conv(args[0], T_FLOAT);
				type = T_BOOLEAN;
			} else if (extra == 2){
				JIT_conv(args[0], T_FLOAT);
				type = T_INTEGER;
			} else {
				THROW_ILLEGAL();
			}
			break;
		
		case 0x8B: //Exist
			check_string(args[0]);
			if (nargs == 2)
				JIT_conv(args[1], T_BOOLEAN);
			type = T_BOOLEAN;
			break;
		
		case 0x8C: //Access
			check_string(args[0]);
			if (nargs != 1)
				JIT_conv(args[1], T_INTEGER);
			type = T_BOOLEAN;
			break;
		
		case 0x8D: //Stat
			ref_stack();
			check_string(args[0]);
			if (nargs == 2)
				JIT_conv(args[1], T_BOOLEAN);
			type = (TYPE)(void*)GB.FindClass("Stat");
			break;
		
		case 0x8E: //Dfree
			check_string(args[0]);
			type = T_LONG;
			break;
		
		case 0x8F: //Temp
			if (nargs != 0)
				check_string(args[0]);
			type = T_STRING;
			break;
		
		case 0x90: //IsDir
			check_string(args[0]);
			type = T_BOOLEAN;
			break;
		
		case 0x91: //Dir
			ref_stack();
			check_string(args[0]);
			if (nargs >= 2){
				check_string(args[1]);
				if (nargs == 3)
					check_integer(args[2]);
			}
			type = (TYPE)(void*)GB.FindClass("String[]");
			break;
		
		case 0x92: //RDir
			ref_stack();
			check_string(args[0]);
			if (nargs >= 2){
				check_string(args[1]);
				if (nargs >= 3){
					check_integer(args[2]);
					if (nargs == 4)
						JIT_conv(args[3], T_BOOLEAN);
				}
			}
			type = (TYPE)(void*)GB.FindClass("String[]");
			break;
		
		case 0x93: { //Exec
			if (extra){
				//Shell
				check_string(args[0]);
			} else {
				JIT_conv(args[0], (TYPE)(void*)GB.FindClass("String[]"));
			}
			auto int_expr = dyn_cast<PushIntegerExpression>(args[2]);
			assert(int_expr != NULL);
			check_string(args[3]);
			if (int_expr->i & 8){ // 8 = TS_EXEC_STRING = PM_STRING
				type = T_STRING;
			} else {
				type = (TYPE)(void*)GB.FindClass("Process");
			}
			break;
		}
		
		case 0x94: //Alloc
			ref_stack();
			if (nargs == 2)
				check_integer(args[1]);
			if (!TYPE_is_string(args[0]->type))
				check_integer(args[0]);
			type = T_POINTER;
			break;
		
		case 0x95: //Free
			check_pointer(args[0]);
			type = T_VOID;
			break;
		
		case 0x96: //Realloc
			ref_stack();
			if (nargs == 3)
				check_integer(args[2]);
			check_integer(args[1]);
			check_pointer(args[0]);
			type = T_POINTER;
			break;
		
		case 0x97: //StrPtr
			ref_stack();
			check_pointer(args[0]);
			if (nargs != 1)
				check_integer(args[1]);
			type = T_CSTRING;
			break;
		
		case 0x98: //Sleep
			check_float(args[0]);
			type = T_VOID;
			break;
		
		case 0x99: { //VarPtr
			type = T_POINTER;
			//FIXME check numeric, pointer or string
			auto pie = dyn_cast<PushIntegerExpression>(args[0]);
			assert(pie);
			ushort op = pie->i;
			if ((op & 0xFF00) == C_PUSH_LOCAL){
				CLASS_LOCAL* var = &FP->local[op & 0xFF];
				TYPE t = ctype_to_type(&var->type);
				if (t == T_VOID || t > T_POINTER)
					THROW(E_TYPE, "Number", JIF.F_TYPE_get_name(t));
			} else if ((op & 0xF800) == C_PUSH_DYNAMIC){
				if (OP == NULL)
					THROW_ILLEGAL();
			} else if ((op & 0xF800) == C_PUSH_STATIC){
			} else
				THROW_ILLEGAL();
			break;
		}
		
		case 0x9A: //Collection
			ref_stack();
			type = (TYPE)(void*)GB.FindClass("Collection");
			break;
		
		case 0x9B: //Tr
			JIT_conv(args[0], T_STRING);
			type = T_CSTRING;
			break;
		
		case 0x9C: //Quote, Shell, Html
		case 0x9D: //Unquote
			JIT_conv(args[0], T_STRING);
			type = T_STRING;
			break;
		
		case 0x9E: //MkInteger, ...
			if (extra == 0 || (extra > T_DATE && extra < T_POINTER) || extra > T_POINTER)
				THROW_ILLEGAL();
			JIT_conv(args[0], extra);
			type = extra <= T_BYTE ? T_CSTRING : T_STRING;
			if (extra > T_BYTE)
				on_stack = true;
			break;
		
		case 0x9F: //Ptr
			ref_stack();
			type = args[0]->type;
			if (type != T_VARIANT && !TYPE_is_string(type) && type != T_POINTER)
				THROW(E_TYPE, "Pointer", JIF.F_TYPE_get_name(type));
			type = extra;
			break;
	}
}
EqExpression::EqExpression(Expression** it) : BinOpExpression(it) {
	type = T_BOOLEAN;
	
	if (left->type == T_VOID || right->type == T_VOID)
		THROW(E_NRETURN);
	
	if (left->type == T_NULL || right->type == T_NULL){
		t = T_NULL;
		return;
	}
	
	if (left->type == T_VARIANT || right->type == T_VARIANT){
		ref_stack();
		left->must_on_stack();
		right->must_on_stack();
		t = T_VARIANT;
		return;
	}
	
	t = Max(left->type, right->type);
	if (TYPE_is_object(left->type) && TYPE_is_object(right->type)){
		t = T_OBJECT;
		left->must_on_stack();
		right->must_on_stack();
	}
	else if (TYPE_is_object(t))
		THROW(E_TYPE, "Object", JIF.F_TYPE_get_name(Min(left->type, right->type)));
	
	if (!TYPE_is_object(t)){
		JIT_conv(left, t);
		JIT_conv(right, t);
	}
}
NotExpression::NotExpression(Expression* expr) : UnaryExpression(expr) {
	type = expr->type;
	if (type >= T_BOOLEAN && type <= T_LONG) return;
	if (type == T_VARIANT){
		ref_stack(); //FIXME might not always be needed
		expr->must_on_stack();
		on_stack = true;
		no_ref_variant = true;
		return;
	}
	if (TYPE_is_object(type) || type == T_NULL || TYPE_is_string(type))
		type = T_BOOLEAN;
	else
		THROW(E_TYPE, "Number, String or Object", JIF.F_TYPE_get_name(type));
}
LessExpression::LessExpression(Expression** it) : BinOpExpression(it) {
	type = T_BOOLEAN;
	
	if (left->type == T_VOID || right->type == T_VOID)
		THROW(E_NRETURN);
	
	if (left->type == T_VARIANT || right->type == T_VARIANT){
		ref_stack(); //FIXME might not always be needed
		left->must_on_stack();
		right->must_on_stack();
		must_on_stack();
		t = T_VARIANT;
		return;
	}
	
	t = Max(left->type, right->type);
	if (t == T_NULL || TYPE_is_string(t)){
		TYPE typem = Min(left->type, right->type);
		if (!TYPE_is_string(typem))
			THROW(E_TYPE, JIF.F_TYPE_get_name(typem), JIF.F_TYPE_get_name(t));
	} else if (TYPE_is_object(t))
		THROW(E_TYPE, "Number, Date or String", JIF.F_TYPE_get_name(type));
	
	if (t == T_BYTE && Min(left->type, right->type) == T_BOOLEAN){
		//To get the expected result:
		JIT_conv(left, T_INTEGER);
		JIT_conv(right, T_INTEGER);
	} else {
		JIT_conv(left, t);
		JIT_conv(right, t);
	}
}
AddSubBaseExpression::AddSubBaseExpression(Expression** it) : BinOpExpression(it) {
	type = Max(left->type, right->type);
	
	if (left->type == T_VOID || right->type == T_VOID)
		THROW(E_NRETURN);
	
	if (left->type == T_VARIANT || right->type == T_VARIANT){
		ref_stack(); //FIXME might not always be needed
		left->must_on_stack();
		right->must_on_stack();
		type = T_VARIANT;
		no_ref_variant = true;
		on_stack = true;
		return;
	}
	
	if (TYPE_is_number_date(type) || TYPE_is_pointer(type)){
		if (type == T_DATE){
			JIT_conv(left, T_FLOAT);
			JIT_conv(right, T_FLOAT);
		} else {
			JIT_conv(left, type);
			JIT_conv(right, type);
		}
	}
	
	if (TYPE_is_string(left->type)){
		JIT_conv(left, T_FLOAT);
	}
	
	if (TYPE_is_string(right->type)){
		JIT_conv(right, T_FLOAT);
	}
	
	if (left->type == T_NULL || right->type == T_NULL)
		THROW(E_TYPE, "Number", JIF.F_TYPE_get_name(T_NULL));
	
	type = Max(left->type, right->type);
	
	if (TYPE_is_number_date(type) || TYPE_is_pointer(type)){
		if (type == T_DATE){
			JIT_conv(left, T_FLOAT);
			JIT_conv(right, T_FLOAT);
		} else {
			JIT_conv(left, type);
			JIT_conv(right, type);
		}
	} else
		THROW(E_TYPE, "Number", JIF.F_TYPE_get_name(type));
}
MulExpression::MulExpression(Expression** it) : BinOpExpression(it) {
	type = Max(left->type, right->type);
	
	if (left->type == T_VOID || right->type == T_VOID)
		THROW(E_NRETURN);
	
	if (left->type == T_VARIANT || right->type == T_VARIANT){
		ref_stack(); //FIXME might not always be needed
		left->must_on_stack();
		right->must_on_stack();
		type = T_VARIANT;
		no_ref_variant = true;
		on_stack = true;
		return;
	}
	
	if (TYPE_is_number_date(type)){
		JIT_conv(left, type);
		JIT_conv(right, type);
		return;
	}
	
	if (TYPE_is_string(left->type)){
		JIT_conv(left, T_FLOAT);
	}
	
	if (TYPE_is_string(right->type)){
		JIT_conv(right, T_FLOAT, left);
	}
	
	if (left->type == T_NULL || right->type == T_NULL)
		THROW(E_TYPE, "Number", JIF.F_TYPE_get_name(T_NULL));
	
	type = Max(left->type, right->type);
	
	if (TYPE_is_number_date(type)){
		JIT_conv(left, type);
		JIT_conv(right, type, left);
	} else
		THROW(E_TYPE, "Number", JIF.F_TYPE_get_name(type));
}
QuoRemBaseExpression::QuoRemBaseExpression(Expression** it) : BinOpExpression(it) {
	type = Max(left->type, right->type);
	
	if (left->type == T_VOID || right->type == T_VOID)
		THROW(E_NRETURN);
	
	ref_stack(); //Division by zero possible
	
	if (left->type == T_VARIANT || right->type == T_VARIANT){
		/*left->must_on_stack();
		right->must_on_stack();
		type = T_VARIANT;
		no_ref_variant = true;*/
		//FIXME is this good/ok to use Long here?
		type = T_LONG;
		JIT_conv(left, T_LONG);
		JIT_conv(right, T_LONG);
		return;
	}
	
	if (left->type == T_NULL || right->type == T_NULL)
		THROW(E_TYPE, "Number", JIF.F_TYPE_get_name(T_NULL));
	
	if (TYPE_is_integer_long(type)){
		JIT_conv(left, type);
		JIT_conv(right, type, left);
	} else {
		THROW(E_TYPE, "Number", JIF.F_TYPE_get_name(type));
	}
}
AndOrXorBaseExpression::AndOrXorBaseExpression(Expression** it) : BinOpExpression(it) {
	if (left->type == T_VOID || right->type == T_VOID)
		THROW(E_NRETURN);
	
	if (TYPE_is_variant(left->type) || TYPE_is_variant(right->type)){
		ref_stack();
		left->must_on_stack();
		right->must_on_stack();
		type = T_VARIANT;
		no_ref_variant = true;
		on_stack = true;
		return;
	}
	
	if (TYPE_is_string(left->type))
		JIT_conv(left, T_BOOLEAN);
	if (TYPE_is_string(right->type))
		JIT_conv(right, T_BOOLEAN);
	
	type = Max(left->type, right->type);
	
	if (TYPE_is_null(left->type) || TYPE_is_null(right->type))
		THROW(E_TYPE, "Number", JIF.F_TYPE_get_name(T_NULL));
	
	if (TYPE_is_number_date(type)){
		JIT_conv(left, type);
		JIT_conv(right, type);
	} else
		THROW(E_TYPE, "Number", JIF.F_TYPE_get_name(type));
}
