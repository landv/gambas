/***************************************************************************

  gbc_class.h

  (c) 2000-2012 Benoît Minisini <gambas@users.sourceforge.net>

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

#ifndef __GBC_CLASS_H
#define __GBC_CLASS_H

#include "gbc_type.h"
#include "gb_table.h"
#include "gbc_trans.h"

#define FUNC_INIT_STATIC    0
#define FUNC_INIT_DYNAMIC   1

typedef
	struct {
		TYPE type;
		int value;
		}
	PACKED
	CLASS_SYMBOL_INFO;

typedef
	struct {
		SYMBOL symbol;
		CLASS_SYMBOL_INFO global;
		CLASS_SYMBOL_INFO local;
		int class;
		int unknown;
		unsigned used : 1;
		unsigned _reserved : 31;
		}
	PACKED
	CLASS_SYMBOL;

typedef
	TRANS_PARAM PARAM;

typedef
	struct {
		TYPE type;
		int index;
		int pos;
		int size;
		}
	VARIABLE;

typedef
	struct {
		int num;
		int param;
		}
	VARIABLE_INIT;

typedef
	struct {
		TYPE type;
		int index;
		int value;
		int line;
		int64_t lvalue;
		}
	CONSTANT;

typedef
	struct {
		TYPE type;                     // Return value datatype
		int name;                      // Function name index in class symbol table

		char nparam;                   // Maximum number of arguments
		char npmin;                    // Minimum number of arguments
		char vararg;                   // If this function accepts extra arguments
		char _reserved;
		short nlocal;                  // Local variable count
		short nctrl;                   // Control structure variable count
		uint64_t byref;                // Byref mask
		PARAM *local;                  // Datatypes of local variables
		PARAM *param;                  // Datatypes of arguments

		PATTERN *start;                // Starts compilation from there
		int line;	                     // ...which is this line
		ushort *code;                  // Compile bytecode
		
		short *pos_line;               // Bytecode position of each code line
		
		ushort ncode;                  // Number of instructions
		ushort ncode_max;              // Size of the bytecode allocation

		short last_code;               // Last compiled bytecode position
		short last_code2;              // Last last compiled bytecode position
		short stack;                   // Needed stack
		short finally;                 // FINALLY position
		short catch;                   // CATCH position
		short _reserved2;
		}
	PACKED
	FUNCTION;

typedef
	struct {
		TYPE type;                     // Return value datatype
		int name;                      // Function name index in class symbol table
		PARAM *param;                  // Argument list
		short nparam;                  // Number of arguments
		short _reserved;
		}
	PACKED
	EVENT;

typedef
	struct {
		TYPE type;                     // Return value datatype
		int name;                      // Function name index in class symbol table
		PARAM *param;                  // Argument list
		short nparam;                  // Number of arguments
		short _reserved;
		int library;                   // Library name index
		int alias;                     // Real function name index
		}
	PACKED
	EXTFUNC;

typedef
	struct {
		TYPE type;                     // Property datatype
		int name;                      // Property name index
		int line;                      // The line where the property is declared
		int comment;                   // Property string description, added to datatype
		short read;                    // Read function
		short write;                   // Write function
		}
	PACKED
	PROPERTY;

typedef
	struct {
		TYPE type;                     // Array datatype
		int ndim;                      // Number of dimensions
		int dim[MAX_ARRAY_DIM];        // Dimensions bounds
		}
	CLASS_ARRAY;

typedef
	struct {
		int index;                     // Structure name
		int nfield;                    // Number of structure fields
		VARIABLE *field;               // Structure fields
		}
	CLASS_STRUCT;

typedef
	struct {
		int index;
		unsigned used : 1;
		unsigned exported : 1;
		unsigned structure : 1;
		unsigned _reserved : 29;
		}
	CLASS_REF;

typedef
	struct {
		TABLE *table;                  // symbol table
		TABLE *string;                 // strings table
		char *name;                    // class name
		short parent;                  // parent class
		unsigned exported : 1;         // class is exported
		unsigned autocreate : 1;       // class is auto-creatable
		unsigned optional : 1;         // class is optional
		unsigned nocreate : 1;         // class cannot be instantiated
		unsigned _reserved : 12;
		VARIABLE *stat;                // static variables
		VARIABLE *dyn;                 // dynamic variables
		CONSTANT *constant;            // constants
		CLASS_REF *class;              // classes
		int *unknown;                  // unknown symbols
		FUNCTION *function;            // functions
		int size_stat;                 // static variables total size
		int size_dyn;                  // dynamic variables total size
		EVENT *event;                  // events
		PROPERTY *prop;                // properties
		EXTFUNC *ext_func;             // extern functions
		CLASS_ARRAY *array;            // array definitions
		CLASS_STRUCT *structure;       // structs definitions
		char **names;                  // when some symbols must be created like object arrays
		}
	CLASS;


#define CLASS_get_symbol(class, ind) ((CLASS_SYMBOL *)TABLE_get_symbol((class)->table, ind))

#define FUNCTION_is_procedure(func)  (TYPE_get_id((func)->type) == T_VOID)
#define FUNCTION_is_static(func)     (TYPE_is_static((func)->type))

void CLASS_create(CLASS **result);
void CLASS_delete(CLASS **class);

CLASS_SYMBOL *CLASS_declare(CLASS *class, int index, bool global);

void CLASS_add_function(CLASS *class, TRANS_FUNC *decl);
void CLASS_add_event(CLASS *class, TRANS_EVENT *decl);
void CLASS_add_property(CLASS *class, TRANS_PROPERTY *prop);
void CLASS_add_extern(CLASS *class, TRANS_EXTERN *decl);
void CLASS_add_declaration(CLASS *class, TRANS_DECL *decl);
int CLASS_add_constant(CLASS *class, TRANS_DECL *decl);
int CLASS_add_class(CLASS *class, int index);
int CLASS_add_class_unused(CLASS *class, int index);
int CLASS_add_class_exported(CLASS *class, int index);
int CLASS_add_class_exported_unused(CLASS *class, int index);
bool CLASS_exist_class(CLASS *class, int index);
int CLASS_add_unknown(CLASS *class, int index);
int CLASS_add_array(CLASS *class, TRANS_ARRAY *array);

int CLASS_get_array_class(CLASS *class, int type, int value);

void FUNCTION_add_pos_line(void);
char *FUNCTION_get_fake_name(int func);

int CLASS_add_symbol(CLASS *class, const char *name);

void CLASS_sort_declaration(CLASS *class);
void CLASS_check_properties(CLASS *class);

/* gbc_dump.c */

void CLASS_dump(void);
void CLASS_export(void);

#endif
