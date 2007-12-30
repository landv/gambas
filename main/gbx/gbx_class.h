/***************************************************************************

  class.h

  Class loader

  (c) 2000-2006 Benoît Minisini <gambas@users.sourceforge.net>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 1, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

***************************************************************************/

#ifndef __GBX_CLASS_H
#define __GBX_CLASS_H

#include "gb_error.h"
#include "gb_alloc.h"
#include "gbx_type.h"
#include "gb_table.h"
#include "gbx_class_desc.h"
#include "gbx_component.h"


typedef
  long CLASS_ID;

/*
typedef
  struct {
    unsigned char flag;
    unsigned char id;
    short value;
    }
  PACKED
  CLASS_TYPE;
*/

typedef
  struct {
    CTYPE type;
    long pos;
    }
  PACKED
  CLASS_VAR;

typedef
  unsigned short CLASS_REF;

typedef
  union {
    TYPE type;
    struct { TYPE type; double value; } PACKED _float;
    struct { TYPE type; long value; } PACKED _integer;
    struct { TYPE type; long long value; } PACKED _long;
    struct { TYPE type; char *addr; long len; } PACKED _string;
    struct { TYPE type; long val[2]; } PACKED _swap;
    }
  PACKED
  CLASS_CONST;

typedef
  struct {
    CTYPE type;
    }
  PACKED
  CLASS_LOCAL;

typedef
  struct {
    TYPE type;
    }
  PACKED
  CLASS_PARAM;

typedef
  struct {
    SYMBOL sym;
    long value;
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
    unsigned short _reserved;
  }
  PACKED
  FUNC_DEBUG;

typedef
  struct {
    TYPE type;
    char n_param;
    char npmin;
    char vararg;
    char _reserved;
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
    unsigned loaded : 1;
    unsigned _reserved : 15;
    CLASS_PARAM *param;
    char *alias;
    char *library;
    }
  PACKED
  CLASS_EXTERN;

typedef
  struct {
    ulong magic;
    ulong version;
    ulong endian;
    ulong flag;
    }
  PACKED
  CLASS_HEADER;

typedef
  struct {
    short parent;
    short flag;
    long s_static;
    long s_dynamic;
    }
  PACKED
  CLASS_INFO;

typedef
  struct {
    TYPE type;
    long dim[0];
    }
  CLASS_ARRAY;

struct _CLASS;

typedef
  struct {
    SYMBOL sym;
    CTYPE ctype;
    long value;
    }
  GLOBAL_SYMBOL;

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

    struct _CLASS **class_ref;
    char **unknown;

    GLOBAL_SYMBOL *global;    /* symboles globaux */
    short n_global;
    short n_ext;
    }
  PACKED
  CLASS_LOAD;

enum
{
  CS_NULL = 0,
  CS_LOADED = 1,
  CS_READY = 2
};

typedef
  struct _CLASS {
    struct _CLASS *class;     /*   4  Points at the 'Class' class ! */
    long ref;                 /*   8  Reference count */
    long count;               /*  12  Number of instanciated objects */
    struct _CLASS *parent;    /*  16  Inherited class */
    COMPONENT *component;     /*  20  The component the class belongs to */
    char *name;               /*  24  Class name */
    char *path;               /*  28  Source file path */

    unsigned state : 2;       /*      Initialization state */
    unsigned debug : 1;       /*      Debugging information ? */
    unsigned free_name : 1;   /*      Must free the class name */
    unsigned free_event : 1;  /*      Must free class->event */
    unsigned in_load : 1;     /*      Class being loaded */
    unsigned exit : 1;        /*      Marker used by CLASS_exit */
    unsigned auto_create : 1; /*      Automatically instanciated */
    unsigned is_virtual : 1;  /*      Virtual class (name beginning with a dot) */
    unsigned mmapped : 1;     /*      mmap() was used to load the class */
    unsigned swap : 1;				/*      class endianness was swapped */
    unsigned enum_static : 1; /*      if class enumeration is static */
    unsigned _flag : 4;       /*  30  */

    short n_desc;             /*  32  number of descriptions */
    CLASS_DESC_SYMBOL *table; /*  36  class description */

    short n_event;            /*  38  number of events */
    short _reserved;          /*  40  */

    CLASS_EVENT *event;       /*  44  event description */

    void (*new)();            /*  48  class instanctiation method */
    void (*free)();           /*  52  class freeing method */
    int (*check)();           /*  56  method for checking that an object is valid */

    short special[16];        /*  88  special functions index (_new, _free, ...) */

    ulong size_stat;          /*  92  static class size */
    ulong size;               /*  96  dynamic class size */
    ulong off_event;          /* 100  offset of OBJECT_EVENT structure in the object */

    char *data;               /* 104  class file data for loaded class / generated description for native class */
    CLASS_LOAD *load;         /* 108  information about loaded class */

    char *stat;               /* 112  static class data */
    TYPE *signature;          /* 116  signatures address */
    char *string;             /* 120  strings address */

    void *instance;           /* 124  automatically created instance */
    struct _CLASS *next;      /* 128  next class */
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
    SPEC_COMPARE,
    SPEC_PRINT
    }
  CLASS_SPECIAL;

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
    CI_OPTIONAL = 4
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

EXTERN CLASS *CLASS_SubCollection;
#endif


#define DO_ERROR ((void (*)()) -1)

#define FUNCTION_is_static(func) ((func)->type & TF_STATIC)
#define CLASS_is_extern(class) ((class)->load == NULL)
#define CLASS_is_native CLASS_is_extern

#define FUNCTION_is_native(_desc) (((ulong)(_desc)->exec >> 16) != 0)

#define FUNC_INIT_STATIC   0
#define FUNC_INIT_DYNAMIC  1


/* class.c */

PUBLIC void CLASS_init(void);
PUBLIC void CLASS_exit(void);

PUBLIC CLASS *CLASS_get(const char *name);
PUBLIC long CLASS_count(void);

PUBLIC CLASS_DESC_SYMBOL *CLASS_get_symbol(CLASS *class, const char *name);
PUBLIC CLASS_DESC *CLASS_get_symbol_desc(CLASS *class, const char *name);

PUBLIC short CLASS_get_symbol_index_kind(CLASS *class, const char *name, int kind, int kind2);
PUBLIC CLASS_DESC *CLASS_get_symbol_desc_kind(CLASS *class, const char *name, int kind, int kind2);
PUBLIC CLASS_DESC_METHOD *CLASS_get_special_desc(CLASS *class, int spec);

#define CLASS_get_desc(_class, _index) (((_class)->table[_index].desc))

PUBLIC CLASS_DESC_EVENT *CLASS_get_event_desc(CLASS *class, const char *name);

PUBLIC long CLASS_find_symbol(CLASS *class, const char *name);
PUBLIC long CLASS_find_symbol_with_prefix(CLASS *class, const char *name, const char *prefix);

PUBLIC CLASS *CLASS_look(const char *name, int len);
PUBLIC CLASS *CLASS_find(const char *name);

PUBLIC TABLE *CLASS_get_table(void);

PUBLIC boolean CLASS_inherits(CLASS *class, CLASS *parent);

PUBLIC CLASS *CLASS_replace_global(const char *name);
PUBLIC CLASS *CLASS_look_global(const char *name, int len);
PUBLIC CLASS *CLASS_find_global(const char *name);
PUBLIC CLASS *CLASS_check_global(char *name);

PUBLIC void CLASS_ref(void *object);
PUBLIC void CLASS_unref(void **pobject, boolean can_free);
PUBLIC void CLASS_free(void **pobject);
PUBLIC void CLASS_release(CLASS *class, char *data);

PUBLIC int CLASS_get_inheritance(CLASS *class, CLASS **her);

PUBLIC void CLASS_do_nothing();
PUBLIC int CLASS_return_zero();

PUBLIC void CLASS_sort(CLASS *class);

PUBLIC void CLASS_inheritance(CLASS *class, CLASS *parent);
PUBLIC void CLASS_make_description(CLASS *class, CLASS_DESC *desc, long n_desc, long *first);
PUBLIC void CLASS_make_event(CLASS *class, int *first);
PUBLIC void CLASS_calc_info(CLASS *class, int n_event, int size_dynamic, boolean all, int size_static);

PUBLIC void *CLASS_auto_create(CLASS *class, int nparam);

PUBLIC void CLASS_search_special(CLASS *class);

PUBLIC CLASS_DESC_SYMBOL *CLASS_get_next_sorted_symbol(CLASS *class, int *index);

//PUBLIC void CLASS_create_array_class(CLASS *class);

/* class_init.c */

PUBLIC void CLASS_init_native(void);

/* class_load.c */

PUBLIC void CLASS_load_without_init(CLASS *class);
PUBLIC void CLASS_load_real(CLASS *class);
#define CLASS_load(_class) \
{ \
  if ((_class)->state != CS_READY) \
    CLASS_load_real(_class); \
}


/* class_native.c */

PUBLIC CLASS *CLASS_register_class(GB_DESC *desc, CLASS *class);
PUBLIC CLASS *CLASS_register(GB_DESC *desc);

#define SET_IF_NULL(ptr, val)  if ((ptr) == NULL) (ptr) = (val)
/*
#define DO_NOTHING(ptr)  if ((ptr) == NULL) (ptr) = CLASS_do_nothing
#define RETURN_ZERO(ptr)  if ((ptr) == NULL) (ptr) = CLASS_return_zero
*/

#define CLASS_is_virtual(class)  (class->is_virtual)

#endif /* _CLASS_H */
