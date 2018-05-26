#include <stdint.h>
#include <stddef.h>
#include <sys/types.h>

typedef
	unsigned char uchar;
	
typedef
  char bool;

#ifndef PACKED
#define PACKED __attribute__((packed))
#endif

/* Gambas datatypes identifiers */

#define GB_T_VOID         0
#define GB_T_BOOLEAN      1
#define GB_T_BYTE         2
#define GB_T_SHORT        3
#define GB_T_INTEGER      4
#define GB_T_LONG         5
#define GB_T_SINGLE       6
#define GB_T_FLOAT        7
#define GB_T_DATE         8
#define GB_T_STRING       9
#define GB_T_CSTRING      10
#define GB_T_POINTER      11
#define GB_T_VARIANT      12
#define GB_T_NULL         15
#define GB_T_OBJECT       16


/* This type represents a Gambas datatype identifier */

typedef
	uintptr_t GB_TYPE;


/* This opaque type represents a Gambas class identifier */

typedef
	GB_TYPE GB_CLASS;


/* This structure represents the base of every Gambas object.
   It must be placed in the beginning of all object structure defined
   in a component.
*/

typedef
	struct {
		GB_CLASS klass;
		intptr_t ref;
		}
	GB_BASE;


/* Gambas STRING datatype definition */

typedef
  struct {
    GB_TYPE type;
    struct {
      char *addr;
      int start;
      int len;
      } value;
    #if __WORDSIZE == 64
		intptr_t _reserved;
		#endif
    }
  GB_STRING;


/* Gambas INTEGER datatype definition */

typedef
  struct {
    GB_TYPE type;
    int value;
    #if __WORDSIZE == 64
    int _pad;
    #endif
		intptr_t _reserved[2];
    }
  GB_INTEGER;


/* Gambas LONG datatype definition */

typedef
  struct {
    GB_TYPE type;
		#if __WORDSIZE == 32
		int _pad;
		#endif
    int64_t value;
    #if __WORDSIZE == 64
    intptr_t _reserved[2];
    #endif
    }
  GB_LONG;


/* Gambas POINTER datatype definition */

typedef
  struct {
    GB_TYPE type;
    intptr_t value;
    intptr_t _reserved[2];
    }
  GB_POINTER;


/* Gambas BOOLEAN datatype definition */

typedef
  struct {
    GB_TYPE type;
    int value;
    #if __WORDSIZE == 64
    int _pad;
    #endif
		intptr_t _reserved[2];
    }
  GB_BOOLEAN;


/* Gambas SINGLE datatype definition */

typedef
  struct {
    GB_TYPE type;
    float value;
    #if __WORDSIZE == 64
    float _pad;
    #endif
		intptr_t _reserved[2];
    }
  GB_SINGLE;


/* Gambas FLOAT datatype definition */

typedef
  struct {
    GB_TYPE type;
    #if __WORDSIZE == 32
		int _pad;
		#endif
    double value;
    #if __WORDSIZE == 64
    intptr_t _reserved[2];
    #endif
    }
  GB_FLOAT;


/* Gambas DATE datatype definition */

typedef
  struct {
    int date;
    int time;
    }
  GB_DATE_VALUE;

typedef
  struct {
    GB_TYPE type;
		GB_DATE_VALUE value;
    #if __WORDSIZE == 64
    intptr_t _reserved[2];
    #else
    int _reserved;
    #endif
    }
  GB_DATE;


/* Gambas OBJECT datatype definition */

typedef
  struct {
    GB_TYPE type;
    void *value;
    intptr_t _reserved[2];
    }
  GB_OBJECT;


/* Gambas VARIANT datatype definition */

typedef
  struct {
    GB_TYPE type;
		union {
			char _boolean;
			unsigned char _byte;
			short _short;
			int _integer;
			int64_t _long;
			float _single;
			double _float;
			GB_DATE_VALUE _date;
			char *_string;
			intptr_t _pointer;
			void *_object;
			int64_t data;
			}
			value;
		}
	PACKED
  GB_VARIANT_VALUE;

typedef
  struct {
    GB_TYPE type;
		GB_VARIANT_VALUE value;
		#if __WORDSIZE == 64
		int64_t _pad;
		#endif
    }
  GB_VARIANT;


/* Gambas common value definition */

typedef
	union {
		GB_TYPE type;
		GB_BOOLEAN _boolean;
		GB_INTEGER _integer;
		GB_LONG _long;
		GB_SINGLE _single;
		GB_FLOAT _float;
		GB_DATE _date;
		GB_STRING _string;
		GB_POINTER _pointer;
		GB_OBJECT _object;
		GB_VARIANT _variant;
		}
	GB_VALUE;

typedef
  GB_VALUE VALUE;

typedef
  ushort PCODE;

typedef
	struct _stack_context {
		struct _stack_context *next;
		VALUE *bp;        // local variables
		VALUE *pp;        // local parameters
		void *cp;        // current class
		char *op;         // current object
		VALUE *ep;        // error pointer
		void *fp;     // current function
		PCODE *pc;        // instruction
		PCODE *ec;        // instruction if error
		PCODE *et;        // TRY save
		VALUE *gp;        // GOSUB stack pointer
		}
	STACK_CONTEXT;

typedef
	struct {
    VALUE **sp;
    PCODE **pc;
    void **EXEC_subr_table;
    const char *STRING_char_string;
  }
  JIT_INTERFACE;
  
JIT_INTERFACE *JIT_PTR;
#define JIT (*JIT_PTR)

#define SP (*(JIT.sp))
#define PC (*(JIT.pc))

#define PUSH_i(_val) (SP->type = GB_T_INTEGER, SP->_integer.value = (_val), SP++)
#define PUSH_s(_val) (*SP++ = (GB_VALUE)(_val))
#define PUSH_t(_val) (*SP++ = (GB_VALUE)(_val))

#define GET_CHAR(_char) ({ \
  GB_STRING temp; \
  temp.type = GB_T_CSTRING; \
  temp.value.addr = (char *)&JIT.STRING_char_string[(_char) * 2]; \
  temp.value.start = 0; \
  temp.value.len = 1; \
  temp; })
  
typedef
	void (*EXEC_FUNC)();

typedef
	void (*EXEC_FUNC_CODE)(ushort);
  
#define CALL_SUBR(_code) (PC = &_code, (*(EXEC_FUNC)JIT.EXEC_subr_table[_code >> 8])())
#define CALL_SUBR_CODE(_code) (PC = &_code, (*(EXEC_FUNC_CODE)JIT.EXEC_subr_table[_code >> 8])(_code))

