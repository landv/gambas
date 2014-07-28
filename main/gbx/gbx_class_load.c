/***************************************************************************

	gbx_class_load.c

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
#include "gbx_c_array.h"
#include "gbx_string.h"
#include "gbx_object.h"
#include "gbx_variant.h"
#include "gbx_number.h"
#include "gbx_struct.h"
#include "gbx_jit.h"

#include "gambas.h"

#include "gbx_class.h"

//#define DEBUG DEBUG
//#define DEBUG_LOAD 1
//#define DEBUG_STRUCT 1

static bool _load_class_from_jit = FALSE;
static const char *_class_name;
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


static int align_pos(int pos, int size)
{
	switch (size)
	{
		case 1:
			return pos;
		case 2:
			return (pos + 1) & ~1;
		#ifndef OS_64BITS
		case 4:
		default:
			return (pos + 3) & ~3;
		#else
		case 4:
			return (pos + 3) & ~3;
		case 8:
		default:
			return (pos + 7) & ~7;
		#endif
	}
}


int CLASS_sizeof_ctype(CLASS *class, CTYPE ctype)
{
	size_t size;
	
	if (ctype.id == TC_ARRAY)
	{
		CLASS_ARRAY *array = class->load->array[ctype.value];
		size = CARRAY_get_static_size(class, array);
		return (size + 3) & ~3;
	}
	else if (ctype.id == TC_STRUCT)
	{
		return CSTRUCT_get_size(class->load->class_ref[ctype.value]);
	}
	else
		return TYPE_sizeof_memory(ctype.id);
}

TYPE CLASS_ctype_to_type(CLASS *class, CTYPE ctype)
{
	if (ctype.id == T_OBJECT && ctype.value >= 0)
		return (TYPE)(class->load->class_ref[ctype.value]);
	else if (ctype.id == TC_ARRAY)
		return (TYPE)CARRAY_get_array_class(class, class->load->array[ctype.value]->ctype);
	else if (ctype.id == TC_STRUCT)
		return (TYPE)(class->load->class_ref[ctype.value]);
	else
		return (TYPE)(ctype.id);	
}


static void conv_ctype(CTYPE *ctype)
{
	//if (ctype->id == TC_POINTER)
	//	ctype->id = T_POINTER;
}


static void conv_type(CLASS *class, void *ptype)
{
	CTYPE ctype = *(CTYPE *)ptype;

	if (_swap)
	{
		SWAP_int((int *)&ctype);
		SWAP_type(&ctype);
	}

	*((TYPE *)ptype) = CLASS_ctype_to_type(class, ctype);
}


static void conv_type_simple(CLASS *class, int *ptype)
{
	CTYPE ctype = *(CTYPE *)ptype;

	if (_swap)
		SWAP_int((int *)&ctype);

	*ptype = ctype.id;
}


static void check_version(CLASS *class, int loaded)
{
	if (loaded > GAMBAS_PCODE_VERSION)
		THROW_CLASS(class, "Bytecode too recent. Please upgrade Gambas.", "");
	if (loaded < GAMBAS_PCODE_VERSION_MIN)
		THROW_CLASS(class, "Bytecode too old. Please recompile the project.", "");
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
	fprintf(stderr, "Section #%d %s %08lX %d %d\n", NSection + 1, sec_name, (int)(current - (char *)Class->data), (int)size_one, (int)section_size);
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
			THROW(E_CLASS, _class_name, "Bad format in section: ", sec_name);
	
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
			ALLOC(&alloc, size_one_64 * size);
				
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
				*((int64_t *)pa) = *((int *)p);
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
}

#define RELOCATE(_ptr) (_ptr = (char *)&class->string[(int)(intptr_t)(_ptr)])

static void load_structure(CLASS *class, int *structure, int nfield)
{
	char *name;
	char *field;
	CLASS *sclass;
	int i, pos, size, len;
	CTYPE ctype;
	CLASS_DESC *desc;
	CLASS_VAR *var;
	GLOBAL_SYMBOL *global = NULL;
	
	name = (char *)(intptr_t)(*structure++);
	RELOCATE(name);
	#if DEBUG_STRUCT
	fprintf(stderr, "Loading structure %s\n", name);
	#endif
	
	if (class->global)
		sclass = CLASS_find_global(name);
	else
		sclass = CLASS_find(name);
	
	if (CLASS_is_loaded(sclass))
	{
		if (!sclass->is_struct)
			THROW_CLASS(class, "Class already exists: ", name);
		
		// Check compatibility with previous declaration
		
		if (sclass->load->n_dyn != nfield)
			goto __MISMATCH;
		
		desc = (CLASS_DESC *)sclass->data;
		
		for (i = 0; i < nfield; i++)
		{
			field = (char *)(intptr_t)(*structure++);
			RELOCATE(field);
			len = strlen(field);
			ctype = *((CTYPE *)structure);
			structure++;
			
			if (CLASS_ctype_to_type(class, ctype) != desc[i].variable.type)
				goto __MISMATCH;
			
			if (TABLE_compare_ignore_case_len(field, strlen(field), sclass->table[i].name, sclass->table[i].len))
				goto __MISMATCH;
		}
		
		// OK, they are the same!
		return;
	}
	
	sclass->swap = class->swap;
	sclass->component = class->component;
	sclass->debug = class->debug;

	ALLOC_ZERO(&sclass->load, sizeof(CLASS_LOAD));
	
	ALLOC(&var, sizeof(CLASS_VAR) * nfield);
	sclass->load->dyn = var;
	sclass->load->n_dyn = nfield;
	sclass->load->class_ref = class->load->class_ref;
	sclass->load->array = class->load->array;
	
	if (sclass->debug)
	{
		ALLOC(&global, sizeof(GLOBAL_SYMBOL) * nfield);
		sclass->load->global = global;
		sclass->load->n_global = nfield;
	}
	
	sclass->n_desc = nfield;
	ALLOC(&sclass->table, sizeof(CLASS_DESC_SYMBOL) * nfield);
	ALLOC(&desc, sizeof(CLASS_DESC) * nfield);
	sclass->data = (char *)desc;

	pos = 0; //sizeof(CSTRUCT);
	
	for (i = 0; i < nfield; i++)
	{
		field = (char *)(intptr_t)(*structure++);
		RELOCATE(field);
		len = strlen(field);
		ctype = *((CTYPE *)structure);
		structure++;
		
		size = CLASS_sizeof_ctype(class, ctype);
		pos = align_pos(pos, size);

		desc[i].variable.name = "f";
		desc[i].variable.type = CLASS_ctype_to_type(class, ctype);
		desc[i].variable.ctype = ctype;
		desc[i].variable.offset = pos; // This the position relative to the data, NOT the object!
		desc[i].variable.class = sclass;
		
		var[i].type = ctype;
		var[i].pos = pos;
		
		if (sclass->debug)
		{
			global[i].sym.name = field;
			global[i].sym.len = len;
			global[i].ctype = ctype;
			global[i].value = i;
		}
		
		#if DEBUG_STRUCT
		fprintf(stderr, "  %d: %s As %s (%d) pos = %d\n", i, field, TYPE_get_name(desc[i].variable.type), CLASS_sizeof_ctype(class, ctype), pos);
		#endif

		sclass->table[i].desc = &desc[i];
		sclass->table[i].name = field;
		sclass->table[i].len = len;

		pos += size; //sizeof_ctype(class, var->type);
	}
	
	#ifdef OS_64BITS
	size = align_pos(pos, 8);
	#else
	size = align_pos(pos, 4);
	#endif

	size += sizeof(CSTRUCT);

	#if DEBUG_STRUCT
	fprintf(stderr, "  --> size = %d\n", size);
	#endif
	
	CLASS_calc_info(sclass, 0, size, TRUE, 0);

	CLASS_sort(sclass);
	
	if (sclass->debug)
		sclass->load->sort = sclass->sort;
	
	CLASS_search_special(sclass);
	/*for (i = 0; i < MAX_SPEC; i++)
		sclass->special[i] = NO_SYMBOL;*/
	
	sclass->is_struct = TRUE;
	
	sclass->loaded = TRUE;
	sclass->ready = TRUE;
	return;
	
__MISMATCH:

	THROW_CLASS(class, "Structure is declared elsewhere differently: ", CLASS_get_name(sclass));
}


static void load_and_relocate(CLASS *class, int len_data, CLASS_DESC **pstart, int *pndesc, bool in_jit_compilation)
{
	char *section;
	CLASS_INFO *info;
	CLASS_HEADER *header;
	CLASS_DESC *start;
	CLASS_PARAM *local;
	CLASS_EVENT *event;
	CLASS_EXTERN *ext;
	CLASS_VAR *var;
	FUNCTION *func;
	FUNC_DEBUG *debug;
	int i, j, pos;
	int offset;
	short n_desc, n_class_ref, n_unknown, n_array, n_struct;
	CLASS_STRUCT *structure = NULL;
	int size;
	char *name;
	int len;
	bool have_jit_functions = FALSE;
	uchar flag;
	
	ALLOC_ZERO(&class->load, sizeof(CLASS_LOAD));

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
		
		fd = open("/tmp/gambas-bad-header.dump", O_CREAT | O_WRONLY, 0666);
		if (fd >= 0)
		{
			if (write(fd, class->data, len_data) != len_data)
				fprintf(stderr, "Cannot dump bad class file.\n");
			else
				fprintf(stderr, "Bad class file dumped at /tmp/gambas-bad-header.dump\n");
			close(fd);
		}
		
		THROW_CLASS(class, "Bad header", "");
	}
	
	check_version(class, header->version);

	class->debug = header->flag & CF_DEBUG;
	
	info = (CLASS_INFO *)get_section("info", &section, NULL, _s _s _i _i _s _s );
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

	// Structure descriptions
	
	if (info->nstruct)
	{
		ALLOC(&structure, sizeof(CLASS_STRUCT) * info->nstruct);
		for (i = 0; i < info->nstruct; i++)
		{
			structure[i].desc = (int *)get_section("structure", &section, &n_struct, _i);
			structure[i].nfield = (n_struct - 1) / 2;
		}
	}
	
	// Loading code

	for (i = 0; i < class->load->n_func; i++)
	{
		func = &class->load->func[i];
		func->code = (ushort *)get_section("code", &section, NULL, _s);

		flag = ((FUNCTION_FLAG *)func)->flag;

		func->fast = (flag & 1) != 0;
		func->optional = (func->npmin < func->n_param);
		func->use_is_missing = (flag & 2) != 0;

		if (func->fast)
		{
			func->fast = JIT_load();
			if (func->fast)
				have_jit_functions = TRUE;
		}

		if (func->use_is_missing)
		{
			func->stack_usage++;
			func->n_ctrl++;
		}

		func->_reserved = 0;
	}

	/* Creation flags */

	class->auto_create = (info->flag & CI_AUTOCREATE) != 0;
	class->no_create = (info->flag & CI_NOCREATE) != 0;
	//fprintf(stderr, "%s: info->flag = %d auto_create = %d no_create = %d\n", class->name, info->flag, class->auto_create, class->no_create);

	/* Debugging information */

	if (class->debug)
	{
		class->load->global = (GLOBAL_SYMBOL *)get_section("debug global", &section, &class->load->n_global, _p _i _c _i );
		class->load->sort = (ushort *)get_section("debug global sort", &section, NULL, _s);
		#ifdef OS_64BITS
		class->load->debug =
		#endif
		debug = (FUNC_DEBUG *)get_section("debug method", &section, NULL, _s _s _p _p _p _s _s );

		for (i = 0; i < class->load->n_func; i++)
		{
			func = &class->load->func[i];
			func->debug = &debug[i];
			func->debug->index = i;
		}

		for (i = 0; i < class->load->n_func; i++)
		{
			func = &class->load->func[i];
			func->debug->pos = (ushort *)get_section("debug line", &section, NULL, _s );
		}

		for (i = 0; i < class->load->n_func; i++)
		{
			func = &class->load->func[i];
			func->debug->local = (LOCAL_SYMBOL *)get_section("debug local", &section, &func->debug->n_local, _p _i _i );
		}
	}

	// Profile information
	
	if (EXEC_profile)
		ALLOC_ZERO(&class->load->prof, sizeof(uint) * (class->debug ? (1 + class->load->n_func) : 1));
	
	/* Source file path, ignored now! */

	if (class->debug)
		get_section("debug file name", &section, NULL, NULL);

	/* Strings */

	class->string = (char *)get_section("string", &section, NULL, NULL);

	/* Referenced classes */

	for (i = 0; i < n_class_ref; i++)
	{
		offset = (int)(intptr_t)class->load->class_ref[i];
		
		// The compiler does not know if an array class is global or not, we must check now.
		
		if (offset >= 0)
		{
			name = &class->string[offset];
			len = strlen(name);
			
			if (len >= 3 && name[len - 2] == '[')
			{
				do
				{
					len -= 2;
				}
				while (len >= 3 && name[len - 2] == '[');
			
				if (CLASS_look_global(name, len))
					offset = (- offset);
			}
		}
		
		if (offset >= 0)
			class->load->class_ref[i] = CLASS_find(&class->string[offset]);
		else if (offset < -1)
			class->load->class_ref[i] = CLASS_find_global(&class->string[-offset]);
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

			for (j = 0; j < func->n_local; j++)
				conv_ctype(&func->local[j].type);

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
		THROW_CLASS(class, "Unknown section", "");
	}


	/* Static array definition relocation */

	if (n_array > 0)
	{
		#ifdef OS_64BITS
		CLASS_ARRAY **array = class->load->array;
		n_array = *((int *)array) / sizeof(int);
		ALLOC(&class->load->array, sizeof(void *) * n_array);
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
			//conv_type(class, &class->load->array[i]->type);
		}
	}

	// Create structures (we may need the structure size to compute the variable sizes)
	
	if (info->nstruct)
	{
		for (i = 0; i < info->nstruct; i++)
			load_structure(class, structure[i].desc, structure[i].nfield);
		FREE(&structure);
	}

	// Computes and align the position of each static and dynamic variables.
	// Computes the total size needed accordingly.
	
	#ifdef DEBUG
	fprintf(stderr, "Compute variable position for %s\n", class->name);
	#endif
	
	pos = 0;
	for (i = 0; i < class->load->n_stat; i++)
	{
		var = &class->load->stat[i];
		conv_ctype(&var->type);
		size = CLASS_sizeof_ctype(class, var->type);
		pos = align_pos(pos, size);
		var->pos = pos;
		#ifdef DEBUG
		fprintf(stderr, "Static #%d: %d\n", i, var->pos);
		#endif
		pos += size;
	}
	#ifdef OS_64BITS
	info->s_static = align_pos(pos, 8);
	#else
	info->s_static = align_pos(pos, 4);
	#endif

	pos = 0;
	for (i = 0; i < class->load->n_dyn; i++)
	{
		var = &class->load->dyn[i];
		conv_ctype(&var->type);
		size = CLASS_sizeof_ctype(class, var->type);
		pos = align_pos(pos, size);
		var->pos = pos;
		#ifdef DEBUG
		fprintf(stderr, "Dynamic #%d: %d\n", i, var->pos);
		#endif
		pos += size; //sizeof_ctype(class, var->type);
	}
	#ifdef OS_64BITS
	info->s_dynamic = align_pos(pos, 8);
	#else
	info->s_dynamic = align_pos(pos, 4);
	#endif

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
		CLASS_inheritance(class, class->load->class_ref[info->parent], in_jit_compilation);
	}

	/* If there is no dynamic variable, then the class is not instanciable */
	//if (info->s_dynamic == 0)
	//  class->no_create = TRUE;
	
	/* Class size and offsets */

	CLASS_calc_info(class, class->n_event, info->s_dynamic, FALSE, info->s_static);
	
	*pstart = start;
	*pndesc = n_desc;
	
	/* JIT function pointers */
	
	if (have_jit_functions)
	{
		ALLOC_ZERO(&class->jit_functions, sizeof(void(*)(void)) * class->load->n_func);
		for(i = 0; i < class->load->n_func; i++)
		{
			class->jit_functions[i] = JIT_default_jit_function;
		}
	}
}


static void load_without_inits(CLASS *class, bool in_jit_compilation)
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
	CLASS_DESC *start;
	char kind;
	
	//size_t alloc = MEMORY_size;
	
	if (CLASS_is_loaded(class))
		return;

	if (class->error)
		THROW_CLASS(class, "Loading has already failed", "");

	if (CLASS_exiting)
		THROW_CLASS(class, "Program is exiting", "");

	class->error = TRUE;

	#if DEBUG_LOAD
		fprintf(stderr, "Loading class %s (%p)...\n", class->name, class);
	#endif

	#ifdef DEBUG
		Class = class;
		NSection = 0;
	#endif

	_class_name = class->name;

	if (class->in_load)
		THROW_CLASS(class, "Circular reference", "");

	if (!class->component)
	{
		if (CP)
			class->component = CP->component;
		else
			class->component = NULL;
	}

	#if DEBUG_COMP
	if (class->component)
		fprintf(stderr, "class %s -> component %s\n", class->name, class->component->name);
	else
		fprintf(stderr, "class %s -> no component\n", class->name);
	#endif

	save = COMPONENT_current;
	COMPONENT_current = class->component;
	#if DEBUG_LOAD
		fprintf(stderr, "COMPONENT_current = %s\n", COMPONENT_current ? COMPONENT_current->name : "NULL");
	#endif

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
			THROW_CLASS(class, "Unable to load class file", "");
		}
		END_TRY

		/*if (BUFFER_load_file(&class->data, FILE_get(name)))
			THROW(E_CLASS, _class_name, "Unable to load class file", "");*/
	}

	COMPONENT_current = save;
	#if DEBUG_LOAD
		fprintf(stderr, "COMPONENT_current = %s\n", COMPONENT_current ? COMPONENT_current->name : "NULL");
	#endif

	class->in_load = TRUE;

	class->init_dynamic = TRUE;

	load_and_relocate(class, len_data, &start, &n_desc, in_jit_compilation);

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
				//cc->_swap.val[0] = (int)(*((int64_t *)(void *)&cc->_string.addr));
				cc->_swap.val[0] = (int)(intptr_t)cc->_string.addr;
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
				if (cc->_string.len)
					cc->_string.addr += (intptr_t)class->string;
				break;
				
			case T_FLOAT: case T_SINGLE:
				cc->_string.addr += (intptr_t)class->string;
				if (NUMBER_from_string(NB_READ_FLOAT, cc->_string.addr, strlen(cc->_string.addr), &value))
					THROW_CLASS(class, "Bad constant", "");
				if (cc->type == T_SINGLE)
					cc->_single.value = (float)value._float.value;
				else
					cc->_float.value = value._float.value;
				break;
		}
	}

	/* Event description */

	CLASS_make_event(class, &first_event);

	if (class->free_event && class->n_event > first_event)
		memcpy(&class->event[first_event], class->load->event, (class->n_event - first_event) * sizeof(CLASS_EVENT));

	/* Class public description */

	for (i = 0; i < n_desc; i++)
	{
		desc = &start[i]; //class->table[i].desc;

		//desc->gambas.name = (char *)CLASS_DESC_get_type_name(desc);

		conv_type(class, &desc->gambas.type);
		
		kind = *CLASS_DESC_get_type_name(desc);

		if (!desc->gambas.val1 && index(CD_CALL_SOMETHING_LIST, kind) != NULL)
			fprintf(stderr, "load_without_inits: '%s.%s' gambas.val1 == 0\n", class->name, desc->gambas.name);
		
		switch (kind)
		{
			case CD_METHOD:
			case CD_STATIC_METHOD:

				func = &class->load->func[desc->gambas.val1];
				desc->method.exec = (void (*)())desc->gambas.val1;
				desc->method.npmin = func->npmin;
				desc->method.npmax = func->n_param;
				desc->method.npvar = func->vararg;
				desc->method.signature = (TYPE *)func->param;
				//desc->method.help = NULL;
				desc->method.native = FALSE;

				break;

			case CD_PROPERTY:
			case CD_STATIC_PROPERTY:
			case CD_PROPERTY_READ:
			case CD_STATIC_PROPERTY_READ:

				desc->property.read = (void (*)())desc->gambas.val1;
				desc->property.write = (void (*)())desc->gambas.val2;
				//if ((intptr_t)desc->property.write == -1L)
				//  desc->gambas.name = *desc->gambas.name == 'p' ? "r" : "R";
				desc->property.native = FALSE;

				break;

			case CD_VARIABLE:
			case CD_STATIC_VARIABLE:

				if (kind == CD_STATIC_VARIABLE)
					var = &class->load->stat[desc->gambas.val1];
				else
					var = &class->load->dyn[desc->gambas.val1];

				desc->variable.ctype = var->type;
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
				else if (desc->constant.type == T_SINGLE)
					desc->constant.value._single = cc->_single.value;
				else
				{
					desc->constant.type = T_CSTRING;
					desc->constant.value._string = cc->_string.addr;
					desc->constant.translate = (cc->type == T_CSTRING);
				}

				break;

			case CD_EVENT:

				//fprintf(stderr, "event %s.%s: %d %d\n", class->name, desc->event.name, first_event, (int)desc->event.index);
				
				event = &class->load->event[desc->event.index];
				if (class->parent)
					desc->event.index += class->parent->n_event;
				desc->event.npmin = event->n_param;
				desc->event.npmax = event->n_param;
				desc->event.signature = (TYPE *)event->param;
				//desc->event.help = NULL;
				//desc->event.index = first_event++;
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

				THROW_CLASS(class, "Bad description", "");
		}
	}

	/* Inheritance */

	CLASS_make_description(class, start, n_desc, &first);

	/* Transfer symbol kind into symbol name (which is stored in the symbol table now), like native classes */
	// Define event index
	
	for (i = 0; i < n_desc; i++)
	{
		desc = &start[i];
		desc->gambas.name = (char *)CLASS_DESC_get_type_name(desc);
		desc->method.class = class;
	}

	/* Sort the class description */

	CLASS_sort(class);

	/* Special methods */

	CLASS_search_special(class);

	/* Class is loaded... */

	class->in_load = FALSE;

	/* ...and usable ! */

	class->loaded = TRUE;
	class->error = FALSE;

	/* Init breakpoints */

	if (EXEC_debug)
		DEBUG.InitBreakpoints(class);
	
	//total += MEMORY_size - alloc;
	//printf("%s: %d  TOTAL = %d\n", class->name, MEMORY_size - alloc, total);
}

#if 0
void CLASS_load_without_init(CLASS *class)
{
	load_without_inits(class, FALSE);

	/* Call the static initializer */

	EXEC.native = FALSE;
	EXEC.class = class;
	EXEC.object = NULL;
	EXEC.nparam = 0;
	EXEC.index = FUNC_INIT_STATIC;
	//EXEC.func = &class->load->func[FUNC_INIT_STATIC];

	EXEC_function();
}
#endif

void CLASS_run_inits(CLASS *class)
{
	/* Call the static initializer */

	EXEC.native = FALSE;
	EXEC.class = class;
	EXEC.object = NULL;
	EXEC.nparam = 0;
	EXEC.index = FUNC_INIT_STATIC;
	//EXEC.func = &class->load->func[FUNC_INIT_STATIC];

	EXEC_function();
	
	/* _init */
	EXEC_public(class, NULL, "_init", 0);
}


void CLASS_load_real(CLASS *class)
{
	bool load_from_jit = _load_class_from_jit;
	char *name = class->name;
	int len = strlen(name);

	_load_class_from_jit = FALSE;
	
	if (!CLASS_is_loaded(class))
	{
		if (len >= 3 && name[len - 2] == '[' && name[len - 1] == ']' && !class->array_type)
		{
			CLASS_create_array_class(class);
			return;
		}
	}

	load_without_inits(class, load_from_jit);
	
	if (load_from_jit)
	{
		class->loaded = TRUE;
		class->ready = FALSE;
	}
	else
	{
		class->loaded = TRUE;
		class->ready = TRUE;
		
		CLASS_run_inits(class);
	}

	//EXEC_public(class, NULL, "_init", 0);
}

void CLASS_load_from_jit(CLASS *class)
{
	_load_class_from_jit = TRUE;
	CLASS_load_real(class);
}
