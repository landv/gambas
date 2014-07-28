/***************************************************************************

  gbx_class.h

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

#ifndef __GBX_CLASS_H
#define __GBX_CLASS_H

#include "gb_error.h"
#include "gb_alloc.h"
#include "gbx_type.h"
#include "gb_table.h"
#include "gbx_class_desc.h"
#include "gbx_component.h"

#ifndef __GBX_CLASS_C
EXTERN bool CLASS_exiting;
#endif

typedef
	int CLASS_ID;

typedef
	struct {
		CTYPE type;
		int pos;
		}
	CLASS_VAR;

typedef
	struct _CLASS *CLASS_REF;

typedef
	char *CLASS_UNKNOWN;

typedef
	union {
		int type;
		struct { int type; double value; } PACKED _float;
		struct { int type; float value; } PACKED _single;
		struct { int type; int value; } PACKED _integer;
		struct { int type; int64_t value; } PACKED _long;
		struct { int type; char *addr; int len; } PACKED _string;
		struct { int type; int val[2]; } PACKED _swap;
		}
	PACKED
	CLASS_CONST;

typedef
	struct {
		CTYPE type;
		}
	CLASS_LOCAL;

typedef
	struct {
		TYPE type;
		}
	CLASS_PARAM;

typedef
	struct {
		SYMBOL sym;
		int value;
		}
	LOCAL_SYMBOL;

typedef
	struct {
		unsigned short line;
		unsigned short nline;
		unsigned short *pos;
		char *name;
		LOCAL_SYMBOL *local;
		short n_local;
		unsigned short index; // Function index in its class. Used by the profiler
	}
	PACKED
	FUNC_DEBUG;

typedef
	struct {
		TYPE type;
		char n_param;
		char npmin;
		char vararg;
		unsigned fast : 1;
		unsigned optional : 1;
		unsigned use_is_missing : 1;
		unsigned _reserved : 5;
		short n_local;
		short n_ctrl;
		short stack_usage;
		short error;
		unsigned short *code;
		CLASS_PARAM *param;
		CLASS_LOCAL *local;
		FUNC_DEBUG *debug;
		}
	PACKED
	FUNCTION;

typedef
	struct {
		TYPE type;
		char n_param;
		char npmin;
		char vararg;
		unsigned char flag;
		}
	PACKED
	FUNCTION_FLAG;

typedef
	struct {
		TYPE type;
		short n_param;
		short _reserved;
		CLASS_PARAM *param;
		char *name;
		}
	PACKED
	CLASS_EVENT;

typedef
	struct {
		TYPE type;
		short n_param;
		bool vararg;
		unsigned loaded : 1;
		unsigned _reserved : 7;
		CLASS_PARAM *param;
		char *alias;
		char *library;
		}
	PACKED
	CLASS_EXTERN;

typedef
	struct {
		uint magic;
		uint version;
		uint endian;
		uint flag;
		}
	PACKED
	CLASS_HEADER;

typedef
	struct {
		short parent;
		short flag;
		int s_static;
		int s_dynamic;
		short nstruct;
		short _reserved;
		}
	PACKED
	CLASS_INFO;

typedef
	struct {
		CTYPE ctype;
		int dim[0];
		}
	CLASS_ARRAY;

typedef
	CLASS_ARRAY *CLASS_ARRAY_P;

struct _CLASS;

typedef
	struct {
		SYMBOL sym;
		CTYPE ctype;
		int value;
		}
	GLOBAL_SYMBOL;
	
typedef
	struct {
		int nfield;
		int *desc;
	}
	CLASS_STRUCT;

typedef
	struct {
		short n_cst;
		short n_stat;
		short n_dyn;
		short n_func;

		CLASS_CONST *cst;
		CLASS_VAR *stat;
		CLASS_VAR *dyn;
		FUNCTION *func;
		CLASS_EVENT *event;
		CLASS_EXTERN *ext;
		CLASS_ARRAY **array;

		CLASS_REF *class_ref;
		char **unknown;

		GLOBAL_SYMBOL *global;
		ushort *sort;
		short n_global;
		short n_ext;
		uint *prof; // Shortcut indexes attributed during profiling. The first one is for the class, the other ones for the functions
		#ifdef OS_64BITS
		CLASS_DESC *desc;
		CLASS_PARAM *local;
		FUNC_DEBUG *debug;
		#endif
		}
	PACKED
	CLASS_LOAD;

enum {
	CO_EQUAL, CO_EQUALF, CO_EQUALO,
	CO_COMP, CO_COMPF, CO_COMPO,
	CO_ADD, CO_ADDF, CO_ADDO,
	CO_SUB, CO_SUBF, CO_SUBO,
	CO_MUL, CO_MULF, CO_MULO,
	CO_DIV, CO_DIVF, CO_DIVO,
	CO_POW, CO_POWF, CO_POWO,
	CO_NEG, CO_ABS, CO_FABS, CO_SGN,
	CO_STRENGTH
};

typedef
	struct _CLASS {                     // 32b 64b
		struct _CLASS *class;             //   4   8  Points at the 'Class' class !
		int ref;                          //   8  12  Reference count
		int count;                        //  12  16  Number of instanciated objects
		struct _CLASS *parent;            //  16  24  Inherited class
		char *name;                       //  20  32  Class name

		unsigned loaded : 1;              //          Class is loaded
		unsigned ready : 1;               //          Class is loaded and ready
		unsigned debug : 1;               //          Debugging information ?
		unsigned free_name : 1;           //          Must free the class name
		unsigned free_event : 1;          //          Must free class->event
		unsigned in_load : 1;             //          Class being loaded
		unsigned exit : 1;                //          Marker used by CLASS_exit
		unsigned auto_create : 1;         //          Automatically instanciated

		unsigned quick_array : 2;         //          Array accessor optimization type
		unsigned no_create : 1;           //          Cannot instanciate this class
		unsigned is_virtual : 1;          //          Virtual class (name beginning with a dot)
		unsigned swap : 1;                //          Class endianness was swapped
		unsigned enum_static : 1;         //          If class enumeration is static
		unsigned is_stream : 1;           //          If the class inherits stream
		unsigned global : 1;              //          If the class is in the global table

		unsigned is_native : 1;           //          If the class is native (i.e. written in C/C++)
		unsigned error : 1;               //          Loading or registering the class has failed
		unsigned is_observer : 1;         //          This is the Observer class
		unsigned is_struct : 1;           //          This class is a structure
		unsigned is_array : 1;            //          This class is an array
		unsigned is_array_of_struct : 1;  //          This class is an array of struct
		unsigned init_dynamic : 1;        //          If there is a special function to call at instanciation
		unsigned must_check : 1;          //          The class has a check function

		unsigned has_child : 1;           //          The class has an inherited child class
		unsigned unknown_static : 1;      //          If _unknown is static
		unsigned property_static : 1;     //          If _property is static
		unsigned has_convert : 1;         //          If the _convert interface is implemented
		unsigned has_operators : 1;       //          If the _operators interface is implemented
		unsigned _reserved : 3;           //  24  36

		short n_desc;                     //  26  38  number of descriptions
		short n_event;                    //  28  40  number of events

		CLASS_DESC_SYMBOL *table;         //  32  48  class description
		ushort *sort;                     //  36  56  table sort

		CLASS_EVENT *event;               //  40  64  event description

		int (*check)();                   //  44  72  method for checking that an object is valid

		char *data;                       //  48  80  class file data for loaded class 
		                                  //          or generated description for native class
		                                  //          or generated description for structures
		
		CLASS_LOAD *load;                 //  52  88  information about loaded class

		char *stat;                       //  56  96  static class data
		TYPE *signature;                  //  60 104  signatures address
		char *string;                     //  64 112  strings address

		uint size_stat;                   //  68 116  static class size
		uint size;                        //  72 120  dynamic class size
		uint off_event;                   //  76 124  offset of OBJECT_EVENT structure in the object
		#ifdef OS_64BITS
		uint _reserved2;                  //     128
		#endif

		short special[12];                // 100 152  special functions index (_new, _free, ...)

		TYPE array_type;                  // 104 160  datatype of the contents if this class is an array class of objects
		struct _CLASS *array_class;       // 108 168  array of class
		struct _CLASS *astruct_class;     // 112 176  array of struct class
		
		void *instance;                   // 116 184  automatically created instance
		void **operators;                 // 120 192  arithmetic interface
		bool (*convert)();                // 124 200  convert method
		
		COMPONENT *component;             // 128 208  The component the class belongs to
		
		void (**jit_functions)(void);     // 132 216  array of jit functions

		struct _CLASS *next;              // 136 224  next class
		}
	CLASS;
	
typedef
	struct {
		SYMBOL sym;
		CLASS *class;
		}
	PACKED
	CLASS_SYMBOL;

typedef
	enum {
		SPEC_NEW,
		SPEC_FREE,
		SPEC_GET,
		SPEC_PUT,
		SPEC_FIRST,
		SPEC_NEXT,
		SPEC_CALL,
		SPEC_UNKNOWN,
		SPEC_PROPERTY,
		SPEC_COMPARE,
		SPEC_ATTACH,
		MAX_SPEC = 11
		}
	CLASS_SPECIAL;

typedef
	enum {
		CQA_NONE = 0,
		CQA_ARRAY = 1,
		CQA_COLLECTION = 2
		}
	CLASS_QUICK_ARRAY;

typedef
	enum
	{
		CF_DEBUG = 1
	}
	CLASS_FLAG;

typedef
	enum
	{
		CI_EXPORTED = 1,
		CI_AUTOCREATE = 2,
		CI_OPTIONAL = 4,
		CI_NOCREATE = 8
	}
	CLASS_INFO_FLAG;

#ifndef __GBX_CLASS_INIT_C
EXTERN CLASS *CLASS_Class;
EXTERN CLASS *CLASS_Enum;
EXTERN CLASS *CLASS_Symbol;

EXTERN CLASS *CLASS_Array;
EXTERN CLASS *CLASS_Collection;
EXTERN CLASS *CLASS_Stream;
EXTERN CLASS *CLASS_File;
EXTERN CLASS *CLASS_Stat;
EXTERN CLASS *CLASS_AppArgs;
EXTERN CLASS *CLASS_AppEnv;
EXTERN CLASS *CLASS_Application;
EXTERN CLASS *CLASS_Process;
EXTERN CLASS *CLASS_Component;
EXTERN CLASS *CLASS_Observer;
EXTERN CLASS *CLASS_Timer;

EXTERN CLASS *CLASS_BooleanArray;
EXTERN CLASS *CLASS_ByteArray;
EXTERN CLASS *CLASS_ShortArray;
EXTERN CLASS *CLASS_IntegerArray;
EXTERN CLASS *CLASS_SingleArray;
EXTERN CLASS *CLASS_FloatArray;
EXTERN CLASS *CLASS_DateArray;
EXTERN CLASS *CLASS_StringArray;
EXTERN CLASS *CLASS_ObjectArray;
EXTERN CLASS *CLASS_VariantArray;
EXTERN CLASS *CLASS_LongArray;
EXTERN CLASS *CLASS_PointerArray;
#endif


#define DO_ERROR ((void (*)()) -1)

#define CLASS_is_native(_class) ((_class)->is_native)
#define CLASS_is_struct(_class) ((_class)->is_struct)
#define CLASS_is_array(_class) ((_class)->is_array)

#define FUNCTION_is_static(func) ((func)->type & TF_STATIC)
//#define FUNCTION_is_native(_desc) (((uintptr_t)(_desc)->exec >> 16) != 0)
#define FUNCTION_is_native(_desc) ((_desc)->native)

#define FUNC_INIT_STATIC   0
#define FUNC_INIT_DYNAMIC  1


/* class.c */

void CLASS_init(void);
void CLASS_clean_up(bool silent);
void CLASS_exit(void);
CLASS *CLASS_get_list(void);

CLASS *CLASS_get(const char *name);
int CLASS_count(void);

char *CLASS_get_name(CLASS *class);

CLASS_DESC_SYMBOL *CLASS_get_symbol(CLASS *class, const char *name);
CLASS_DESC *CLASS_get_symbol_desc(CLASS *class, const char *name);

short CLASS_get_symbol_index_kind(CLASS *class, const char *name, int kind, int kind2);
CLASS_DESC *CLASS_get_symbol_desc_kind(CLASS *class, const char *name, int kind, int kind2);
CLASS_DESC_METHOD *CLASS_get_special_desc(CLASS *class, int spec);

#define CLASS_get_desc(_class, _index) (((_class)->table[_index].desc))

CLASS_DESC_EVENT *CLASS_get_event_desc(CLASS *class, const char *name);

#define CLASS_find_symbol(_class, _name) \
	SYMBOL_find((_class)->table, (_class)->sort, (_class)->n_desc, sizeof(CLASS_DESC_SYMBOL), TF_IGNORE_CASE, (_name), strlen((_name)), NULL)

int CLASS_find_symbol_with_prefix(CLASS *class, const char *name, const char *prefix);

CLASS *CLASS_look(const char *name, int len);
CLASS *CLASS_find(const char *name);

TABLE *CLASS_get_table(void);

bool CLASS_inherits(CLASS *class, CLASS *parent);

CLASS *CLASS_replace_global(const char *name);
CLASS *CLASS_look_global(const char *name, int len);
CLASS *CLASS_find_global(const char *name);
CLASS *CLASS_check_global(CLASS *class);

void CLASS_ref(void *object);
bool CLASS_unref(void *object, bool can_free);
void CLASS_free(void *object);

int CLASS_get_inheritance(CLASS *class, CLASS **her);

void CLASS_do_nothing();
int CLASS_return_zero();

void CLASS_sort(CLASS *class);

void CLASS_inheritance(CLASS *class, CLASS *parent, bool in_jit_compilation);
void CLASS_make_description(CLASS *class, const CLASS_DESC *desc, int n_desc, int *first);
void CLASS_make_event(CLASS *class, int *first);
void CLASS_calc_info(CLASS *class, int n_event, int size_dynamic, bool all, int size_static);

void *CLASS_auto_create(CLASS *class, int nparam);

void CLASS_search_special(CLASS *class);

CLASS_DESC_SYMBOL *CLASS_get_next_sorted_symbol(CLASS *class, int *index);

int CLASS_can_be_used_like_an_array(CLASS *class);

void CLASS_create_array_class(CLASS *class);
CLASS *CLASS_get_array_class(CLASS *class);

void CLASS_create_array_of_struct_class(CLASS *class);
CLASS *CLASS_get_array_of_struct_class(CLASS *class);

int CLASS_sizeof(CLASS *class);

/* class_init.c */

void CLASS_init_native(void);

/* class_load.c */

TYPE CLASS_ctype_to_type(CLASS *class, CTYPE ctype);
int CLASS_sizeof_ctype(CLASS *class, CTYPE ctype);

//void CLASS_load_without_init(CLASS *class);
void CLASS_load_real(CLASS *class);
#define CLASS_load(_class) \
({ \
	if (!((_class)->ready)) \
		CLASS_load_real(_class); \
})
void CLASS_run_inits(CLASS *class);
void CLASS_load_from_jit(CLASS *class);

/* class_native.c */

CLASS *CLASS_register_class(GB_DESC *desc, CLASS *class);
CLASS *CLASS_register(GB_DESC *desc);

#define SET_IF_NULL(ptr, val)  if ((ptr) == NULL) (ptr) = (val)
/*
#define DO_NOTHING(ptr)  if ((ptr) == NULL) (ptr) = CLASS_do_nothing
#define RETURN_ZERO(ptr)  if ((ptr) == NULL) (ptr) = CLASS_return_zero
*/

#define CLASS_is_virtual(class)  (class->is_virtual)

#define CLASS_has_operator(_class, _op) (((void **)(_class)->operators)[_op] != NULL)
#define CLASS_get_operator_strength(_class) (((intptr_t *)(_class)->operators)[CO_STRENGTH])
#define CLASS_set_operator_strength(_class, _strength) (((intptr_t *)(_class)->operators)[CO_STRENGTH] = (_strength))

#define CLASS_is_loaded(_class) ((_class)->loaded)
#define CLASS_is_ready(_class) ((_class)->ready)

#endif /* _CLASS_H */
