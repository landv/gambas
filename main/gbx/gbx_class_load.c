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


/*
static void CLASS_new(void **ptr, CLASS *class, char *name, OBJECT *parent)
{
  VALUE_FUNCTION exec;

  OBJECT_new(ptr, class, name, parent);

  exec.class = class;
  exec.object = *ptr;
  exec.function = FUNC_INIT_DYNAMIC;

  #if DEBUG_EVENT
  printf("CLASS_new (%s %p) parent (%s %p)\n", ((OBJECT *)*ptr)->class->name, *ptr, parent->class->name, parent);
  #endif

  EXEC_function(&exec, 0);
}
*/

#ifdef DEBUG
static CLASS *Class;
static int NSection;
#endif

#define RELOCATE(_ptr) (_ptr = (char *)&class->string[(int)(_ptr)])

static void SWAP_type(CTYPE *p)
{
	SWAP_short(&p->value);
}

static char *get_section(char *sec_name, char **section, int one, short *psize, char *swapcode)
{
  char *current = *section + sizeof(int);
  int section_size = *((int *)(*section));
  int i, n;
  char *p;
  char *ps;
  size_t ss;
  void (*func)();
  short size;

  if (_swap)
    SWAP_long(&section_size);

  #ifdef DEBUG
  NSection++;
  printf("Section #%d %s %08lX %d %d\n", NSection + 1, sec_name, (int)(current - (char *)Class->data), one, section_size);
  #endif

  /*
  if (section_size == 0)
    THROW(E_FORMAT);
  */

  *section += section_size + sizeof(int);

  if (section_size % one)
    THROW(E_CLASS, ClassName, "Bad format in ", sec_name);

  size = section_size / one;
  if (psize)
    *psize = size;

  if (_swap && swapcode)
  {
    p = current;

    /*if (one == sizeof(short))
    {
      for (i = 0; i < size; i++)
      {
        SWAP_short((short *)p);
        p += sizeof(short);
      }
    }
    else if (one == sizeof(long))
    {
      for (i = 0; i < size; i++)
      {
        SWAP_long((long *)p);
        p += sizeof(long);
      }
    }
    else*/
    {
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
          else if (*ps == 'l')
          {
            func = SWAP_long;
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
    }
  }

  return current;
}


static void conv_type(CLASS *class, void *ptype)
{
  CTYPE ctype = *(CTYPE *)ptype;
  TYPE type;

	if (_swap)
	{
		SWAP_long((int *)&ctype);
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


static void check_version(int loaded)
{
  int current = GAMBAS_PCODE_VERSION;

	// FIXME Remove this test in 2.0 final

  if ((GAMBAS_FULL_VERSION >> 16) == 0x0109)
  {
    if (loaded == current)
      return;
    
    fprintf(stderr, "WARNING: current bytecode version is ");
    fprintf(stderr, "%X.%X.%X ", current >> 24, (current >> 16) & 0xFF, current & 0xFFFF);
    fprintf(stderr, "and project bytecode version is ");
    fprintf(stderr, "%X.%X.%X.\n", loaded >> 24, (loaded >> 16) & 0xFF, loaded & 0xFFFF);
    return;
  }
  
  if (loaded > current)
    THROW(E_CLASS, ClassName, "Version too recent. Please upgrade Gambas.", "");
  if (loaded < current)
    THROW(E_CLASS, ClassName, "Version too old. Please recompile.", "");
}


PUBLIC void CLASS_load_without_init(CLASS *class)
{
  char *section;
  CLASS_HEADER *header;
  CLASS_INFO *info;
  int i, j;
  CLASS_LOCAL *local;
  FUNCTION *func;
  CLASS_DESC *desc;
  CLASS_DESC *start;
  short n_class_ref;
  short n_unknown;
  CLASS_CONST *cc;
  CLASS_VAR *var;
  int len;
  CLASS_EVENT *event;
  CLASS_EXTERN *ext;
  short n_array;
  VALUE value;
  int len_data;
  int first;
  short n_desc;
  int offset;
  int first_event;
  FUNC_DEBUG *debug;
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
    SWAP_long((int *)&header->magic);
    SWAP_long((int *)&header->version);
    SWAP_long((int *)&header->flag);
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

  /* Loading sections */

  #ifdef DEBUG
  Class = class;
  #endif

  info = (CLASS_INFO *)get_section("info section", &section, sizeof(CLASS_INFO), NULL, "s2l2");
  start = (CLASS_DESC *)get_section("description section", &section, sizeof(CLASS_DESC), &n_desc, "l*");
  class->load->cst = (CLASS_CONST *)get_section("constant section", &section, sizeof(CLASS_CONST), &class->load->n_cst, "l*");
  class->load->class_ref = (CLASS **)get_section("reference section", &section, sizeof(int), &n_class_ref, "l");
  class->load->unknown = (char **)get_section("unknown section", &section, sizeof(int), &n_unknown, "l");
  class->load->stat = (CLASS_VAR *)get_section("static section", &section, sizeof(CLASS_VAR), &class->load->n_stat, "tl");
  class->load->dyn = (CLASS_VAR *)get_section("dynamic section", &section, sizeof(CLASS_VAR), &class->load->n_dyn, "tl");
  class->load->event = (CLASS_EVENT *)get_section("event section", &section, sizeof(CLASS_EVENT), &class->n_event, "ls2l2");
  class->load->ext = (CLASS_EXTERN *)get_section("extern section", &section, sizeof(CLASS_EXTERN), &class->load->n_ext, "ls2l3");
  class->load->func = (FUNCTION *)get_section("function section", &section, sizeof(FUNCTION), &class->load->n_func, "lb4s4l4");
  local = (CLASS_LOCAL *)get_section("local section", &section, sizeof(CLASS_LOCAL), NULL, "l");
  class->load->array = (CLASS_ARRAY **)get_section("array section", &section, sizeof(int), &n_array, "l");

  /* Loading code */

  for (i = 0; i < class->load->n_func; i++)
  {
    func = &class->load->func[i];
    func->code = (ushort *)get_section("code section", &section, sizeof(ushort), NULL, "s");
  }

  /* Creation flags */

  class->auto_create = (info->flag & CI_AUTOCREATE) != 0;
  class->no_create = (info->flag & CI_NOCREATE) != 0;
  //fprintf(stderr, "%s: info->flag = %d auto_create = %d no_create = %d\n", class->name, info->flag, class->auto_create, class->no_create);

  /* Debugging information */

  if (class->debug)
  {
    class->load->global = (GLOBAL_SYMBOL *)
      get_section("global symbol section", &section, sizeof(GLOBAL_SYMBOL),
                  &class->load->n_global, "s2ltl");

    debug = (FUNC_DEBUG *)get_section("debug method section", &section, sizeof(FUNC_DEBUG), NULL, "s2l3s2");

    for (i = 0; i < class->load->n_func; i++)
    {
      func = &class->load->func[i];
      func->debug = &debug[i];
    }

    for (i = 0; i < class->load->n_func; i++)
    {
      func = &class->load->func[i];
      func->debug->pos = (ushort *)get_section("line method section", &section, sizeof(ushort), NULL, "s");
    }

    for (i = 0; i < class->load->n_func; i++)
    {
      func = &class->load->func[i];
      func->debug->local = (LOCAL_SYMBOL *)get_section("local symbol section", &section, sizeof(LOCAL_SYMBOL), &func->debug->n_local, "s2l2");
    }
  }

  /* Source file path */

  if (class->debug)
    class->path = (char *)get_section("source file name section", &section, sizeof(char), NULL, NULL);

  /* Strings */

  class->string = (char *)get_section("string section", &section, sizeof(char), NULL, NULL);

  /* Referenced classes */

  for (i = 0; i < n_class_ref; i++)
  {
  	offset = (int)class->load->class_ref[i];
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

			// As the 'local' section is a mix of CLASS_PARAM and CLASS_LOCAL,
			// we swap endianness there and not during get_section()

			if (_swap)
			{
	      for (j = 0; j < func->n_local; j++)
	      {
	      	SWAP_long((int *)&func->local[j].type);
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
    n_array = *((int *)class->load->array) / sizeof(int);

    for (i = 0; i < n_array; i++)
    {
      class->load->array[i] = (CLASS_ARRAY *)((char *)class->load->array + ((int *)class->load->array)[i]);
      conv_type(class, &(class->load->array[i]->type));
    }
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
  
  /* Checking callback, do nothing on classes written in Gambas */
  
  //SET_IF_NULL(class->check, (int (*)())CLASS_return_zero);

  /* Descriptions */

  CLASS_make_description(class, start, n_desc, &first);

  /* Class size and offsets */

  CLASS_calc_info(class, class->n_event, info->s_dynamic, FALSE, info->s_static);

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
    conv_type(class, &(cc->type));
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
  }

  /* Class public description */

  for (i = first; i < class->n_desc; i++)
  {
    desc = class->table[i].desc;

    switch (desc->gambas.val4)
    {
      case CD_PROPERTY_ID: desc->gambas.name = "p"; break;
      case CD_VARIABLE_ID: desc->gambas.name = "v"; break;
      case CD_METHOD_ID: desc->gambas.name = "m"; break;
      case CD_STATIC_PROPERTY_ID: desc->gambas.name = "P"; break;
      case CD_STATIC_VARIABLE_ID: desc->gambas.name = "V"; break;
      case CD_STATIC_METHOD_ID: desc->gambas.name = "M"; break;
      case CD_CONSTANT_ID: desc->gambas.name = "C"; break;
      case CD_EVENT_ID: desc->gambas.name = ":"; break;
      case CD_EXTERN_ID: desc->gambas.name = "X"; break;
    }

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
        if ((int)desc->property.write == -1)
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


PUBLIC void CLASS_load_real(CLASS *class)
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
