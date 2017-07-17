/***************************************************************************

  jit_compile.c

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

#define __JIT_COMPILE_C

#include <bitset>
#include <algorithm>
#include "jit.h"

static std::vector<Expression*> all_expressions; //Save all expressions so they can be deleted
static std::vector<std::pair<TYPE, CLASS*> > ctrl_types; //second contains a class if first is T_CLASS
static std::vector<std::bitset<4> > used_ctrl_types; //0 = primitive type, 1 = string, 2 = object, 3 = variant
std::vector<Statement*> all_statements;
std::vector<CLASS*> classes_to_load;
int ngosubs;
uint64_t func_byref_mask;

void register_new_expression(Expression* expr){
	all_expressions.push_back(expr);
}

void free_all_expressions(){
	for(size_t i=0, e=all_expressions.size(); i!=e; i++){
		//printf("%d %p\n", i, all_expressions[i]);
		delete all_expressions[i];
	}
	all_expressions.clear();
}

CLASS* get_ctrl_class(int index){
	return ctrl_types[index - FP->n_local].second;
}
TYPE get_ctrl_type(int index){
	return ctrl_types[index - FP->n_local].first;
}

int special_ctrl_type(TYPE type){
	int t = 0;
	if (TYPE_is_string(type))
		t = 1;
	else if (TYPE_is_object(type))
		t = 2;
	else if (TYPE_is_variant(type))
		t = 3;
	return t;
}

void set_ctrl_type(TYPE type, int index, CLASS* second){
	ctrl_types[index - FP->n_local] = std::make_pair(type, second);
	used_ctrl_types[index - FP->n_local][special_ctrl_type(type)] = 1;
}

bool is_ctrl_type_used(int type, int index){
	return used_ctrl_types[index - FP->n_local][type];
}

void JIT_load_class(CLASS* klass){
	if (klass->ready)
		return;
	
	JIF.F_CLASS_load_from_jit(klass);
	if (!klass->is_array)
		classes_to_load.push_back(klass);
}

static void print_line()
{
	int i;
	
	for (i = 1; i < 10; i++)
		fputs("--------", stderr);
	fputc('\n', stderr);
}

void JIT_compile_and_execute(){
	if (MAIN_debug){
		print_line();
		fprintf(stderr, "gb.jit: beginning compiling %s.", CP->name);
		if (FP->debug)
			fprintf(stderr, "%s:\n", FP->debug->name);
		else
			fprintf(stderr, "%d:\n", EXEC.index);
		print_line();
		fputc('\n', stderr);
	}
	
	ctrl_types.resize(FP->n_ctrl);
	used_ctrl_types.resize(FP->n_ctrl);
	ngosubs = 0;
	size_t load_classes_size = classes_to_load.size();
	TRY {
		JIT_read();
	} CATCH {
		classes_to_load.resize(load_classes_size);
		JIF.F_ERROR_propagate();
	} END_TRY
	JIT_codegen();
	free_all_expressions();
	/*for(size_t i=0, e=all_statements.size(); i!=e; i++)
		delete all_statements[i];
	all_statements.clear();*/
	
	void (*fn)(void) = CP->jit_functions[EXEC.index];
	
	std::reverse(classes_to_load.begin() + load_classes_size, classes_to_load.end());
	
	while(classes_to_load.size() > load_classes_size){
		CLASS* klass = classes_to_load.back(); classes_to_load.pop_back();
		
		klass->loaded = true;
		klass->ready = true;
		
		JIF.F_CLASS_run_inits(klass);
	}
	
	(*fn)();
}
