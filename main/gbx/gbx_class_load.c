/***************************************************************************

  class_load.c

  Gambas class loader

  (c) 2000-2007 Benoit Minisini <gambas@users.sourceforge.net>

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

#define __GBX_CLASS_LOAD_C

#include "gb_common.h"
#include "gb_common_case.h"
#include "gb_common_buffer.h"
#include "gb_common_swap.h"
#include "gb_alloc.h"
#include "gb_error.h"
#include "gb_limit.h"

#include <ctype.h>

#include "gb_buffer.h"
#include "gb_file.h"
#include "gbx_type.h"
#include "gbx_exec.h"
#include "gbx_debug.h"
#include "gb_magic.h"
#include "gbx_stream.h"

#include "gbx_string.h"
#include "gbx_object.h"
#include "gbx_variant.h"
#include "gbx_number.h"

#include "gambas.h"

#include "gbx_class.h"

//#define DEBUG
//#define DEBUG_LOAD 1

//long total = 0;

static const char *ClassName;
static bool _swap;

#define _b "\x1"
#define _s "\x2"
#define _i "\x3"
#define _p "\x4"
#define _c "\x5"
#define _t "\x6"

#ifdef DEBUG
static CLASS *Class;
static int NSection;
#endif

static void SWAP_type(CTYPE *p)
{
	SWAP_short(&p->value);
}

static char *get_section(char *sec_name, char **section, short *pcount, const char *desc)
{
	static void *jump_swap[] = { &&__SWAP_END, &&__SWAP_BYTE, &&__SWAP_SHORT, &&__SWAP_INT, &&__SWAP_POINTER, &&__SWAP_CTYPE, &&__SWAP_TYPE };
	static size_t sizeof_32[] = { 0, 1, 2, 4, 4, 4, 4 };
	#ifdef OS_64BITS
	static void *jump_trans[] = { &&__TRANS_END, &&__TRANS_BYTE, &&__TRANS_SHORT, &&__TRANS_INT, &&__TRANS_POINTER, &&__TRANS_CTYPE, &&__TRANS_TYPE };
	static size_t sizeof_64[] = { 0, 1, 2, 4, 8, 4, 8 };
	#endif

  char *current = *section + sizeof(int);
  int section_size = *((int *)(*section));
  int i;
  char *p;
  const char *pdesc;
  short size;
  size_t size_one = 0;
  #ifdef OS_64BITS
  size_t size_one_64 = 0;
  char *alloc = NULL;
  char *pa = NULL;
  #endif

  if (_swap)
    SWAP_int(&section_size);

  #ifdef DEBUG
  NSection++;
  printf("Section #%d %s %08lX %d %d\n", NSection + 1, sec_name, (int)(current - (char *)Class->data), one, section_size);
  #endif

  *section += section_size + sizeof(int);

	if (desc)
	{
		pdesc = desc;
		while (*pdesc)
		{
			size_one += sizeof_32[(int)*pdesc];
			#ifdef OS_64BITS
			size_one_64 += sizeof_64[(int)*pdesc];
			#endif
			pdesc++;
		}
  
		if (section_size % size_one)
			THROW(E_CLASS, ClassName, "Bad format in section: ", sec_name);
	
		size = section_size / size_one;
		if (pcount) *pcount = size;
		if (!size)
			return NULL;

		if (_swap)
		{
			for (i = 0; i < size; i++)
			{
				p = current + i * size_one;
				pdesc = desc;
				
			__SWAP_NEXT:
				goto *jump_swap[(int)(*pdesc++)];
			
			__SWAP_BYTE:
				p++;
				goto __SWAP_NEXT;
			
			__SWAP_SHORT:
				SWAP_short((short *)p);
				p += sizeof(short);
				goto __SWAP_NEXT;
				
			__SWAP_INT:
			__SWAP_POINTER:
				SWAP_int((int *)p);
				p += sizeof(int);
				goto __SWAP_NEXT;
				
			__SWAP_CTYPE:
			__SWAP_TYPE:
				SWAP_type((CTYPE *)p);
				p += sizeof(CTYPE);
				goto __SWAP_NEXT;
				
			__SWAP_END:
				continue;
			}
		}
	
		#ifdef OS_64BITS
		
		if (size_one_64 != size_one)
		{
			ALLOC(POINTER(&alloc), size_one_64 * size, "get_section");
				
			for (i = 0; i < size; i++)
			{
				p = current + i * size_one;
				pa = alloc + i * size_one_64;
				pdesc = desc;
				
			__TRANS_NEXT:
				goto *jump_trans[(int)(*pdesc++)];
			
			__TRANS_BYTE:
				*pa++ = *p++;
				goto __TRANS_NEXT;
			
			__TRANS_SHORT:
				*((short *)pa) = *((short *)p);
				pa += sizeof(short);
				p += sizeof(short);
				goto __TRANS_NEXT;
				
			__TRANS_INT:
			__TRANS_CTYPE:
				*((int *)pa) = *((int *)p);
				pa += sizeof(int);
				p += sizeof(int);
				goto __TRANS_NEXT;			
			
			__TRANS_TYPE:
				*((int *)pa) = *((int *)p);
				pa += sizeof(int);
				p += sizeof(int);
				*((int *)pa) = 0;
				pa += sizeof(int);
				goto __TRANS_NEXT;			
			
			__TRANS_POINTER:
				*((uint64_t *)pa) = *((uint *)p);
				pa += sizeof(int64_t);
				p += sizeof(int);
				goto __TRANS_NEXT;			
				
			__TRANS_END:
				continue;
			}
			
			return alloc;
		}
	
		#endif
	
	}
	
	return current;
	
		#if 0
		for (i = 0; i < size; i++)
		{
			p = current + i * one;

			ps = swapcode;
			while (*ps)
			{
				if (*ps == 's')
				{
					func = SWAP_short;
					ss = sizeof(short);
				}
				else if (*ps == 'i')
				{
					func = SWAP_int;
					ss = sizeof(int);
				}
				else if (*ps == 't')
				{
					func = SWAP_type;
					ss = sizeof(int);
				}
				else
				{
					func = NULL;
					ss = 1;
				}

				ps++;

				if (isdigit(*ps))
				{
					n = *ps - '0';
					ps++;
				}
				else if (*ps == '*')
				{
					n = one / ss;
					ps++;
				}
				else
					n = 1;

				if (func)
				{
					while (n)
					{
						(*func)(p);
						p += ss;
						n--;
					}
				}
				else
					p += n;
			}
    }
    #endif

}

TYPE CLASS_ctype_to_type(CLASS *class, CTYPE ctype)
{
  if (ctype.id == T_OBJECT && ctype.value >= 0)
    return (TYPE)(class->load->class_ref[ctype.value]);
  else if (ctype.id == T_ARRAY)
    ERROR_panic("conv_type: bad type");
  else
    return (TYPE)(ctype.id);	
}

static void conv_type(CLASS *class, void *ptype)
{
  CTYPE ctype = *(CTYPE *)ptype;
  TYPE type;

	if (_swap)
	{
		SWAP_int((int *)&ctype);
		SWAP_type(&ctype);
	}

  if (ctype.id == T_OBJECT && ctype.value >= 0)
    type = (TYPE)(class->load->class_ref[ctype.value]);
  else if (ctype.id == T_ARRAY)
    ERROR_panic("conv_type: bad type");
  else
    type = (TYPE)(ctype.id);

  *((TYPE *)ptype) = type;
}

static void conv_type_simple(CLASS *class, int *ptype)
{
  CTYPE ctype = *(CTYPE *)ptype;

	if (_swap)
		SWAP_int((int *)&ctype);

  *ptype = ctype.id;
}


static void check_version(int loaded)
{
	static bool warning = FALSE;
	
  int current = GAMBAS_PCODE_VERSION;

  if (GAMBAS_FULL_VERSION > 0x01090000 && GAMBAS_FULL_VERSION <= 0x0200FFFF)
  {
    if (loaded == current)
      return;
    
    if ((COMPONENT_current && COMPONENT_current->warning) || warning)
    	return;
    
    fprintf(stderr, "WARNING: ");
    if (COMPONENT_current)
    	fprintf(stderr, "%s: ", COMPONENT_current->name);
    fprintf(stderr, "current bytecode version is ");
    fprintf(stderr, "%X.%X.%X ", current >> 24, (current >> 16) & 0xFF, current & 0xFFFF);
    fprintf(stderr, "and project bytecode version is ");
    fprintf(stderr, "%X.%X.%X. You should recompile your project.\n", loaded >> 24, (loaded >> 16) & 0xFF, loaded & 0xFFFF);
    
    if (COMPONENT_current)
    	COMPONENT_current->warning = TRUE;
    else
    	warning = TRUE;
    	
    return;
  }
  
  if (loaded > current)
    THROW(E_CLASS, ClassName, "Version too recent. Please upgrade Gambas.", "");
  if (loaded < current)
    THROW(E_CLASS, ClassName, "Version too old. Please recompile the project.", "");
}

#define RELOCATE(_ptr) (_ptr = (char *)&class->string[(int)(intptr_t)(_ptr)])


static int sizeof_ctype(CLASS *class, CTYPE ctype)
{
	size_t size;
	
	if (ctype.id != T_ARRAY)
		return TYPE_sizeof_memory(ctype.id);
		
	size = ARRAY_get_size((ARRAY_DESC *)class->load->array[ctype.value]);
  return (size + 3) & ~3;
}

static void load_and_relocate(CLASS *class, int len_data, int *pndesc, int *pfirst)
{
  char *section;
	CLASS_INFO *info;
	CLASS_HEADER *header;
	CLASS_DESC *start;
  CLASS_PARAM *local;
  CLASS_EVENT *event;
  CLASS_EXTERN *ext;
  FUNCTION *func;
  FUNC_DEBUG *debug;
  int i, j;
  int offset;
  short n_desc, n_class_ref, n_unknown, n_array;
	
  ALLOC_ZERO(&class->load, sizeof(CLASS_LOAD), "CLASS_load");

  /* header */

  section = class->data;

  header = (CLASS_HEADER *)section;
  section += sizeof(CLASS_HEADER);

  class->swap = header->endian != OUTPUT_ENDIAN;
  _swap = class->swap;
  if (_swap)
    fprintf(stderr, "Swapping class %s\n", class->name);

  if (_swap)
  {
    SWAP_int((int *)&header->magic);
    SWAP_int((int *)&header->version);
    SWAP_int((int *)&header->flag);
  }

  if (header->magic != OUTPUT_MAGIC)
  {
  	int fd;
  	
  	fprintf(stderr, "Dumping class file at /tmp/gambas-bad-header.out: %d bytes\n", len_data);
  	fd = open("/tmp/gambas-bad-header.out", O_CREAT | O_WRONLY, 0666);
  	write(fd, class->data, len_data);
  	close(fd);
    THROW(E_CLASS, ClassName, "Bad header", "");
	}
	
  check_version(header->version);

  class->debug = header->flag & CF_DEBUG;
  
  info = (CLASS_INFO *)get_section("info", &section, NULL, _s _s _i _i );
  #ifdef OS_64BITS
  class->load->desc =
  #endif
  start = (CLASS_DESC *)get_section("description", &section, &n_desc, _p _t _p _p _p _p );
  class->load->cst = (CLASS_CONST *)get_section("constant", &section, &class->load->n_cst, _i _p _i ); // A special process is needed later
  class->load->class_ref = (CLASS **)get_section("reference", &section, &n_class_ref, _p );
  class->load->unknown = (char **)get_section("unknown", &section, &n_unknown, _p );
  class->load->stat = (CLASS_VAR *)get_section("static", &section, &class->load->n_stat, _c _i );
  class->load->dyn = (CLASS_VAR *)get_section("dynamic", &section, &class->load->n_dyn, _c _i );
  class->load->event = (CLASS_EVENT *)get_section("event", &section, &class->n_event, _t _s _s _p _p );
  class->load->ext = (CLASS_EXTERN *)get_section("extern", &section, &class->load->n_ext, _t _s _s _p _p _p );
  class->load->func = (FUNCTION *)get_section("function", &section, &class->load->n_func, _t _b _b _b _b _s _s _s _s _p _p _p _p );
  #ifdef OS_64BITS
  class->load->local =
  #endif
  local = (CLASS_PARAM *)get_section("local", &section, NULL, _t);
  class->load->array = (CLASS_ARRAY **)get_section("array", &section, &n_array, _i);  // A special process is needed later

  /* Loading code */

  for (i = 0; i < class->load->n_func; i++)
  {
    func = &class->load->func[i];
    func->code = (ushort *)get_section("code", &section, NULL, _s);
  }

  /* Creation flags */

  class->auto_create = (info->flag & CI_AUTOCREATE) != 0;
  class->no_create = (info->flag & CI_NOCREATE) != 0;
  //fprintf(stderr, "%s: info->flag = %d auto_create = %d no_create = %d\n", class->name, info->flag, class->auto_create, class->no_create);

  /* Debugging information */

  if (class->debug)
  {
    class->load->global = (GLOBAL_SYMBOL *)get_section("debug global", &section, &class->load->n_global, _s _s _p _c _i );
    #ifdef OS_64BITS
    class->load->debug =
    #endif
    debug = (FUNC_DEBUG *)get_section("debug method", &section, NULL, _s _s _p _p _p _s _s );

    for (i = 0; i < class->load->n_func; i++)
    {
      func = &class->load->func[i];
      func->debug = &debug[i];
    }

    for (i = 0; i < class->load->n_func; i++)
    {
      func = &class->load->func[i];
      func->debug->pos = (ushort *)get_section("debug line", &section, NULL, _s );
    }

    for (i = 0; i < class->load->n_func; i++)
    {
      func = &class->load->func[i];
      func->debug->local = (LOCAL_SYMBOL *)get_section("debug local", &section, &func->debug->n_local, _s _s _p _i );
    }
  }

  /* Source file path */

  if (class->debug)
    class->path = (char *)get_section("debug file name", &section, NULL, NULL);

  /* Strings */

  class->string = (char *)get_section("string", &section, NULL, NULL);

  /* Referenced classes */

  for (i = 0; i < n_class_ref; i++)
  {
  	offset = (int)(intptr_t)class->load->class_ref[i];
  	if (offset >= 0)
    	class->load->class_ref[i] = CLASS_find(&class->string[offset]);
		else
			class->load->class_ref[i] = 0; //0x31415926; //CLASS_find(&class->string[-offset]);
	}

  /* Datatype conversion */

  for (i = 0; i < class->load->n_func; i++)
  {
    func = &class->load->func[i];
    conv_type(class, &func->type);

    if (func->n_param > 0)
    {
      func->param = (CLASS_PARAM *)local;

      for (j = 0; j < func->n_param; j++)
        conv_type(class, &func->param[j].type);

      local += func->n_param;
    }

    if (func->n_local > 0)
    {
      func->local = (CLASS_LOCAL *)local;
      
      #ifdef OS_64BITS
      // The local variable descriptions are CTYPE that are 32 bits only.
      // We must transform a 64 bits integer array into a 32 bits integer array.
      for (j = 0; j < func->n_local; j++)
      {
      	func->local[j] = func->local[j * 2];
      }
      #endif

			// As the 'local' section is a mix of CLASS_PARAM and CLASS_LOCAL,
			// we swap endianness there and not during get_section()

			if (_swap)
			{
	      for (j = 0; j < func->n_local; j++)
	      {
	      	SWAP_int((int *)&func->local[j].type);
	      	SWAP_type(&func->local[j].type);
				}
      }

      local += func->n_local;
    }
  }

  /* Events information */

  for (i = 0; i < class->n_event; i++)
  {
    event = &class->load->event[i];

    conv_type(class, &event->type);

    if (event->n_param > 0)
    {
      event->param = (CLASS_PARAM *)local;

      for (j = 0; j < event->n_param; j++)
        conv_type(class, &event->param[j].type);

      local += event->n_param;
    }
  }

  /* Extern calls information */

  for (i = 0; i < class->load->n_ext; i++)
  {
    ext = &class->load->ext[i];

    conv_type(class, &ext->type);

    if (ext->n_param > 0)
    {
      ext->param = (CLASS_PARAM *)local;

      for (j = 0; j < ext->n_param; j++)
        conv_type(class, &ext->param[j].type);

      local += ext->n_param;
    }
  }

  /* End of file reached ? */

  if (section != &class->data[len_data])
  {
    /*printf("%d\n", &class->load[BUFFER_length(class->load)] - section);*/
    THROW(E_CLASS, ClassName, "Unknown section", "");
  }


  /* Static array definition relocation */

  if (n_array > 0)
  {
  	#ifdef OS_64BITS
  	CLASS_ARRAY **array = class->load->array;
    n_array = *((int *)array) / sizeof(int);
  	ALLOC(&class->load->array, sizeof(void *) * n_array, "CLASS_load");
  	#else
    n_array = *((int *)class->load->array) / sizeof(int);
  	#endif

    for (i = 0; i < n_array; i++)
    {
    	#ifdef OS_64BITS
      class->load->array[i] = (CLASS_ARRAY *)((char *)array + ((int *)array)[i]);
    	#else
      class->load->array[i] = (CLASS_ARRAY *)((char *)class->load->array + ((int *)class->load->array)[i]);
      #endif
    }
  }

  /* Computes variable position again */
  
	{
		int pos;
		CLASS_VAR *var;
		
		pos = 0;
		for (i = 0; i < class->load->n_stat; i++)
		{
			var = &class->load->stat[i];
			var->pos = pos;
			pos += sizeof_ctype(class, var->type);
		}
		info->s_static = pos;
  
		pos = 0;
		for (i = 0; i < class->load->n_dyn; i++)
		{
			var = &class->load->dyn[i];
			var->pos = pos;
			pos += sizeof_ctype(class, var->type);
		}
		info->s_dynamic = pos;
	}


  /* String relocation */

  for (i = 0; i < n_desc; i++)
    RELOCATE(start[i].gambas.name);

  for (i = 0; i < n_unknown; i++)
    RELOCATE(class->load->unknown[i]);

  for (i = 0; i < class->n_event; i++)
    RELOCATE(class->load->event[i].name);

  for (i = 0; i < class->load->n_ext; i++)
  {
    RELOCATE(class->load->ext[i].library);
    RELOCATE(class->load->ext[i].alias);
  }

  if (class->debug)
  {
    for (i = 0; i < class->load->n_global; i++)
    {
      RELOCATE(class->load->global[i].sym.name);
      /*conv_type(class, &(class->load->global[i].type));*/
    }

    for (i = 0; i < class->load->n_func; i++)
    {
      func = &class->load->func[i];
      RELOCATE(func->debug->name);

      for (j = 0; j < func->debug->n_local; j++)
        RELOCATE(func->debug->local[j].sym.name);
    }
  }
  
  /* Inheritance */

  if (info->parent >= 0)
  {
    //printf("%s inherits %s\n", class->name, (class->load->class_ref[info->parent])->name);
    CLASS_inheritance(class, class->load->class_ref[info->parent]);
  }

  /* If there is no dynamic variable, then the class is not instanciable */
  //if (info->s_dynamic == 0)
  //  class->no_create = TRUE;
  
  /* Descriptions */

  CLASS_make_description(class, start, n_desc, pfirst);

  /* Class size and offsets */

  CLASS_calc_info(class, class->n_event, info->s_dynamic, FALSE, info->s_static);

  *pndesc = n_desc;
}

void CLASS_load_without_init(CLASS *class)
{
  int i;
  FUNCTION *func;
  CLASS_DESC *desc;
  CLASS_CONST *cc;
  CLASS_VAR *var;
  int len;
  CLASS_EVENT *event;
  CLASS_EXTERN *ext;
  VALUE value;
  int len_data;
  int n_desc;
  int offset;
  int first;
  int first_event;
  COMPONENT *save;

  //size_t alloc = MEMORY_size;

  if (class->state >= CS_LOADED)
    return;

  #if DEBUG_LOAD
    printf("Loading class %s (%p)...\n", class->name, class);
  #endif

  #ifdef DEBUG
    Class = class;
    NSection = 0;
  #endif

  ClassName = class->name;

  if (class->in_load)
    THROW(E_CLASS, ClassName, "Circular reference", "");

  if (COMPONENT_current)
    class->component = COMPONENT_current;
  else if (CP)
    class->component = CP->component;
  else
    class->component = NULL;

  save = COMPONENT_current;
  COMPONENT_current = class->component;

  len = strlen(class->name);

  {
    char name[len + 9];
    char *p;

    strcpy(name, ".gambas/");
    p = &name[8];

    for (i = 0; i < len; i++)
      *p++ = toupper(class->name[i]);
    *p = 0;

    TRY
    {
      //class->mmapped = !STREAM_map(name, &class->data, &len_data);
      STREAM_load(name, &class->data, &len_data);
    }
    CATCH
    {
      THROW(E_CLASS, class->name, "Unable to load class file", "");
    }
    END_TRY

    /*if (BUFFER_load_file(&class->data, FILE_get(name)))
      THROW(E_CLASS, ClassName, "Unable to load class file", "");*/
  }

  COMPONENT_current = save;

  class->in_load = TRUE;

  #if DEBUG_COMP
  if (class->component)
    fprintf(stderr, "class %s -> component %s\n", class->name, class->component->name);
  else
    fprintf(stderr, "class %s -> no component\n", class->name);
  #endif

	load_and_relocate(class, len_data, &n_desc, &first);

  /* Information on static and dynamic variables */

  if (class->parent)
    offset = class->parent->off_event;
  else
    offset = sizeof(OBJECT);

  for (i = 0; i < class->load->n_dyn; i++)
  {
    var = &class->load->dyn[i];
    var->pos += offset;
  }

  /* Constant conversion & relocation */

  for (i = 0; i < class->load->n_cst; i++)
  {
    cc = &class->load->cst[i];
    conv_type_simple(class, &(cc->type));
    
    switch (cc->type)
    {
    	case T_BOOLEAN: case T_BYTE: case T_SHORT: case T_INTEGER:
				#ifdef OS_64BITS
				// Special process for integer constants
				cc->_integer.value = (int)(intptr_t)cc->_string.addr;
				#endif
    		break;
    	
    	case T_LONG:
				#ifdef OS_64BITS
				// Special process for long constants: the first 32 bits part of the LONG constant
				// has been extended to 64 bits
				cc->_swap.val[0] = (int)(*((int64_t *)(void *)&cc->_string.addr));
				cc->_swap.val[1] = cc->_string.len;
				#endif
				/*
				The two 32 bits parts of the LONG value have been already swapped independently.
				So we just have to swap the two parts again.
				*/
				if (_swap)
				{
					int val;
	
					val = cc->_swap.val[0];
					cc->_swap.val[0] = cc->_swap.val[1];
					cc->_swap.val[1] = val;
				}
				break;
				
			case T_STRING: case T_CSTRING:
	      cc->_string.addr += (intptr_t)class->string;
	      break;
	      
	    case T_FLOAT: case T_SINGLE:
	      cc->_string.addr += (intptr_t)class->string;
				if (NUMBER_from_string(NB_READ_FLOAT, cc->_string.addr, strlen(cc->_string.addr), &value))
					THROW(E_CLASS, ClassName, "Bad constant", "");
	      cc->_float.value = value._float.value;
				break;
    }
    
    #if 0
    if (!TYPE_is_integer_long(cc->type))
      cc->_string.addr += (int)class->string;

    if (cc->type == T_FLOAT || cc->type == T_SINGLE)
    {
      if (NUMBER_from_string(NB_READ_FLOAT, cc->_string.addr, strlen(cc->_string.addr), &value))
        THROW(E_CLASS, ClassName, "Bad constant", "");

      cc->_float.value = value._float.value;
    }
    else if (cc->type == T_LONG)
    {
      /*
      Each long value has been already swapped independently.
      In the case of a LONG constant, we just have to swap the two 32 bits parts of the
      constant value.
      */
      if (_swap)
      {
        int val;

        val = cc->_swap.val[0];
        cc->_swap.val[0] = cc->_swap.val[1];
        cc->_swap.val[1] = val;
      }
    }
    #ifdef OS_64BITS
    // Special process for integer constants
    else if (cc->type <= T_INTEGER)
    {
    	cc->_integer.value = (int)cc->_string.addr;
    }
    #endif
    #endif
  }

  /* Class public description */

  for (i = first; i < class->n_desc; i++)
  {
    desc = class->table[i].desc;

		desc->gambas.name = CLASS_DESC_get_type_name(desc);

    conv_type(class, &desc->gambas.type);

    switch (CLASS_DESC_get_type(desc))
    {
      case CD_METHOD:
      case CD_STATIC_METHOD:

        func = &class->load->func[desc->gambas.val1];
        desc->method.exec = (void (*)())desc->gambas.val1;
        desc->method.npmin = func->npmin;
        desc->method.npmax = func->n_param;
        desc->method.signature = (TYPE *)func->param;
        //desc->method.help = NULL;
        desc->method.native = FALSE;

        break;

      case CD_PROPERTY:
      case CD_STATIC_PROPERTY:

        desc->property.read = (void (*)())desc->gambas.val1;
        desc->property.write = (void (*)())desc->gambas.val2;
        if ((intptr_t)desc->property.write == -1L)
          desc->gambas.name = *desc->gambas.name == 'p' ? "r" : "R";
        desc->property.native = FALSE;

        break;

      case CD_VARIABLE:
      case CD_STATIC_VARIABLE:

        if (CLASS_DESC_get_type(desc) == CD_STATIC_VARIABLE)
          var = &class->load->stat[desc->gambas.val1];
        else
          var = &class->load->dyn[desc->gambas.val1];

        desc->variable.offset = var->pos;

        break;

      case CD_CONSTANT:

        cc = &class->load->cst[desc->gambas.val1];

        if (TYPE_is_integer(desc->constant.type))
          desc->constant.value._integer = cc->_integer.value;
        else if (desc->constant.type == T_FLOAT)
          desc->constant.value._float = cc->_float.value;
        else if (desc->constant.type == T_LONG)
          desc->constant.value._long = cc->_long.value;
        else
        {
          desc->constant.type = T_CSTRING;
          desc->constant.value._string = cc->_string.addr;
        }

        break;

      case CD_EVENT:

        event = &class->load->event[desc->gambas.val1];
        desc->event.npmin = event->n_param;
        desc->event.npmax = event->n_param;
        desc->event.signature = (TYPE *)event->param;
        //desc->event.help = NULL;
        break;

      case CD_EXTERN:

        ext = &class->load->ext[desc->gambas.val1];
        desc->ext.npmin = ext->n_param;
        desc->ext.npmax = ext->n_param;
        desc->ext.npmin = ext->n_param;
        desc->ext.signature = (TYPE *)ext->param;
        //desc->event.help = NULL;
        break;

      default:

        THROW(E_CLASS, ClassName, "Bad description", "");
    }

    desc->method.class = class;
  }

  /* Event description */

  CLASS_make_event(class, &first_event);

  if (class->free_event && class->n_event > first_event)
    memcpy(&class->event[first_event], class->load->event, (class->n_event - first_event) * sizeof(CLASS_EVENT));

  /* Sort the class description */

  CLASS_sort(class);

  /* Special methods */

  CLASS_search_special(class);

  /* Class is loaded... */

  class->in_load = FALSE;

  /* ...and usable ! */

  class->state = CS_LOADED;

	/* Init breakpoints */

	if (EXEC_debug)
		DEBUG.InitBreakpoints(class);

  /* Call the static initializer */

  EXEC.native = FALSE;
  EXEC.class = class;
  EXEC.object = NULL;
  EXEC.nparam = 0;
  EXEC.index = FUNC_INIT_STATIC;
  //EXEC.func = &class->load->func[FUNC_INIT_STATIC];

  EXEC_function();

  //total += MEMORY_size - alloc;
  //printf("%s: %d  TOTAL = %d\n", class->name, MEMORY_size - alloc, total);
}


void CLASS_load_real(CLASS *class)
{
#if 0
	char *name = class->name;
	int len = strlen(name);

	if (len >= 3 && name[len - 2] == '[' && name[len - 1] == ']')
	{
		CLASS_create_array_class(class);
		return;
	}
#endif

  CLASS_load_without_init(class);
  class->state = CS_READY;

  EXEC_public(class, NULL, "_init", 0);
}
