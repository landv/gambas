/***************************************************************************

  gbc_output.c

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

#define __GBC_OUTPUT_C

#include "gb_common.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
#include <time.h>

#include "config.h"

#include "gb_error.h"
#include "gb_alloc.h"
#include "gb_str.h"
#include "gb_file.h"
#include "gb_common_swap.h"
#include "gb_magic.h"
#include "gbc_chown.h"
#include "gbc_compile.h"
#include "gbc_form.h"
#include "gbc_output.h"


/*#define DEBUG*/
/*#define DEBUG_MORE*/

static CLASS *Class;

static int StringAddr;
static TABLE *StringTable;
static int NSection;
static int PosStartSection;
static int SizeSection;

static bool _swap;

static FILE *_file;

static int _pos;
static char _buffer[OUTPUT_BUFFER_SIZE + 8];
static const char * const _mbuffer = &_buffer[OUTPUT_BUFFER_SIZE];
static char *_pbuffer;

static OUTPUT_CHANGE *_change = NULL;

static void output_init(void)
{
	TABLE_create(&StringTable, sizeof(OUTPUT_SYMBOL), TF_NORMAL);
	StringAddr = 0;
	NSection = 0;
	_pos = 0;
	_pbuffer = _buffer;
	ARRAY_create(&_change);
}


static void output_exit(void)
{
	TABLE_delete(&StringTable);
	ARRAY_delete(&_change);
}


static int get_string(const char *string, int len)
{
	OUTPUT_SYMBOL *sym;
	int index;
	bool new;

	if (len < 0)
		len = strlen(string);

	new = !TABLE_add_symbol(StringTable, string, len, &index);
	sym = (OUTPUT_SYMBOL *)TABLE_get_symbol(StringTable, index);
	
	if (new)
	{
		sym->value = StringAddr;
		StringAddr += len + 1;
	}

	#ifdef DEBUG
	printf("'%.*s' -> %ld\n", len, string, sym->value);
	#endif

	return sym->value;
}

#define get_pos() (_pos)

static void flush_buffer(void)
{
	size_t len = _pbuffer - _buffer;
	
	if (len <= 0)
		return;
	
	if (fwrite(_buffer, sizeof(char), len, _file) != len)
		THROW("Write error");
		
	_pbuffer = _buffer;
}


static void write_byte(unsigned char val)
{
	#ifdef DEBUG_MORE
	printf("%ld : b %u 0x%X\n", get_pos(), val, val);
	#endif

	if (_pbuffer >= _mbuffer)
		flush_buffer();
		
	*_pbuffer++ = val;
	_pos++;

	/*if (fwrite(&val, sizeof(char), 1, _file) != 1)
		THROW("Write error");*/
}


static void write_short(ushort val)
{
	#ifdef DEBUG_MORE
	printf("%ld : i %u 0x%X\n", get_pos(), val, val);
	#endif

	if (_swap)
		SWAP_short((short *)&val);

	if (_pbuffer >= _mbuffer)
		flush_buffer();

	*((ushort *)_pbuffer) = val;
	_pbuffer += sizeof(val);
	_pos += sizeof(val);
}


static void write_int(uint val)
{
	#ifdef DEBUG_MORE
	printf("%ld : l %lu 0x%lX\n", get_pos(), val, val);
	#endif

	if (_swap)
		SWAP_int((int *)&val);

	if (_pbuffer >= _mbuffer)
		flush_buffer();

	*((uint *)_pbuffer) = val;
	_pbuffer += sizeof(val);
	_pos += sizeof(val);
}


static void write_int64(uint64_t val)
{
	#ifdef DEBUG_MORE
	printf("%ld : l %llu 0x%llX\n", get_pos(), val, val);
	#endif

	if (_swap)
		SWAP_int64((int64_t *)&val);

	if (_pbuffer >= _mbuffer)
		flush_buffer();
		
	*((uint64_t *)_pbuffer) = val;
	_pbuffer += sizeof(val);
	_pos += sizeof(val);
}

static void write_string(const char *str, int len)
{
	#ifdef DEBUG_MORE
	printf("%ld : s \"%.*s\"\n", get_pos(), len, str);
	#endif

	if (&_pbuffer[len] > _mbuffer)
		flush_buffer();
	
	if (&_pbuffer[len] <= _mbuffer)
	{
		memcpy(_pbuffer, str, len);
		_pbuffer += len;
		*_pbuffer++ = 0;
		_pos += len + 1;
		return;
	}

	if (fwrite(str, sizeof(char), len, _file) != len)
		THROW("Write error");
	
	_pos += len;  
	write_byte(0);
}


static void write_buffer(void *str, int len)
{
	#ifdef DEBUG_MORE
	printf("%ld : buffer %ld octets\n", get_pos(), len);
	#endif

	if (len == 0)
		return;

	if (&_pbuffer[len] > _mbuffer)
		flush_buffer();
	
	if (&_pbuffer[len] <= _mbuffer)
	{
		memcpy(_pbuffer, str, len);
		_pbuffer += len;
		_pos += len;
		return;
	}

	flush_buffer();
	
	if (fwrite(str, sizeof(char), len, _file) != len)
		THROW("Write error");
		
	_pos += len;
}


static void write_pad(void)
{
	while (get_pos() & 0x3)
		write_byte(0);
}


static void write_type(TYPE type)
{
	write_byte(type.t.flag);
	write_byte(type.t.id);
	write_short(type.t.value);
}


static void add_change(off_t pos, uint val)
{
	int prev = get_pos();
	char *ppos;
	OUTPUT_CHANGE *change;    
	
	ppos = &_buffer[pos - (prev - (_pbuffer - _buffer))];
	if (ppos >= _buffer && ppos < (_pbuffer - sizeof(uint)))
	{
		*((uint *)ppos) = val;
		return;
	}
	
	change = ARRAY_add(&_change);
	change->pos = pos;
	change->val = val;
}

static void begin_section(const char *name, int size)
{
	NSection++;
	#ifdef DEBUG
	printf("Section #%d : %s\n", NSection, name);
	#endif

	if (size)
	{
		PosStartSection = get_pos();
		write_int(0);
	}
	else
		PosStartSection = 0;

	SizeSection = size;
}


static void end_section(void)
{
	int len;

	if (PosStartSection)
	{
		write_pad();
		len = get_pos() - PosStartSection - sizeof(int);
		add_change(PosStartSection, len);

		#ifdef DEBUG
		printf("==> %ld %s\n", len / SizeSection, (SizeSection == 1) ? "bytes" : "elements");
		if (len % SizeSection)
			printf("*** remain %ld bytes\n", len % SizeSection);
		#endif
	}
}


static void output_header(void)
{
	begin_section("Header", 0);

	/* magic */
	write_int(OUTPUT_MAGIC);
	/* version */
	write_int(COMPILE_version);
	/* endianness */
	write_int(OUTPUT_ENDIAN);
	/* flag */
	if (JOB->debug)
		write_int(1);
	else
		write_int(0);

	end_section();
}


static void output_class(void)
{
	short flag;
	begin_section("Class", 1);

	// Parent class
	write_short(Class->parent);
	
	// Class flags
	flag = 0;
	if (Class->exported) flag |= 1;
	if (Class->autocreate) flag |= 2;
	if (Class->optional) flag |= 4;
	if (Class->nocreate) flag |= 8;
	write_short(flag);
	
	// Static size
	write_int(Class->size_stat);
	
	// Dynamic size
	write_int(Class->size_dyn);
	
	// Number of structures
	write_short(ARRAY_count(Class->structure));
	
	// reserved
	write_short(0);

	end_section();
}


static void output_desc(void)
{
	int i, n, nn = 0;
	CLASS_SYMBOL *csym;
	TYPE type;
	short out_type;

	n = TABLE_count(Class->table);

	begin_section("Description", 6 * sizeof(int));

	for (i = 0; i < n; i++)
	{
		csym = (CLASS_SYMBOL *)TABLE_get_symbol_sort(Class->table, i);
		//csym = (CLASS_SYMBOL *)TABLE_get_symbol(Class->table, csym->symbol.sort);

		type = csym->global.type;

		if (TYPE_is_public(type))
		{
			nn++;
			/* name */
			write_int(get_string(csym->symbol.name, csym->symbol.len));
			/* datatype */
			write_type(csym->global.type);

			switch (TYPE_get_kind(type))
			{
				case TK_VARIABLE:

					/* offset */
					write_int(csym->global.value);
					/* read */
					write_int(0);
					/* write */
					write_int(0);

					if (TYPE_is_static(type))
						out_type = CD_STATIC_VARIABLE_ID;
					else
						out_type = CD_VARIABLE_ID;

					break;

				case TK_PROPERTY:

					/* read */
					write_int(Class->prop[csym->global.value].read);
					/* write */
					write_int(Class->prop[csym->global.value].write);
					/* flag */
					write_int(0);

					if (TYPE_is_static(type))
						out_type = CD_STATIC_PROPERTY_ID;
					else
						out_type = CD_PROPERTY_ID;

					break;

				case TK_CONST:

					/* param */
					write_int(csym->global.value);
					/* read */
					write_int(0);
					/* write */
					write_int(0);

					out_type = CD_CONSTANT_ID;

					break;

				case TK_FUNCTION:

					/* exec */
					write_int(csym->global.value);
					/* signature */
					write_int(0);
					/* nparam */
					write_int(0);

					if (TYPE_is_static(type))
						out_type = CD_STATIC_METHOD_ID;
					else
						out_type = CD_METHOD_ID;

					break;

				case TK_EVENT:

					/* exec */
					write_int(csym->global.value);
					/* signature */
					write_int(0);
					/* nparam */
					write_int(0);

					out_type = CD_EVENT_ID;

					break;

				case TK_EXTERN:

					/* exec */
					write_int(csym->global.value);
					/* signature */
					write_int(0);
					/* nparam */
					write_int(0);

					out_type = CD_EXTERN_ID;

					break;

				default:

					ERROR_panic("output_desc: unknown symbol type");
					continue;
			}

			/* type de symbole */
			write_int(out_type);
		}
	}

	end_section();
}



static void output_constant(void)
{
	int i, n;
	CONSTANT *constant;
	SYMBOL *sym;
	//TABLE *table;

	n = ARRAY_count(Class->constant);

	begin_section("Constants", 3 * sizeof(int));

	for (i = 0; i < n; i++)
	{
		constant = &Class->constant[i];

		/* type */
		write_type(constant->type);
		/* value */
		switch (TYPE_get_id(constant->type))
		{
			case T_BOOLEAN: case T_BYTE: case T_SHORT: case T_INTEGER:

				write_int(constant->value);
				write_int(0);
				break;

			case T_LONG:

				write_int64(constant->lvalue);
				break;

			case T_SINGLE: case T_FLOAT:

				sym = TABLE_get_symbol(Class->table, constant->value);
				write_int(get_string(sym->name, sym->len));
				write_int(sym->len);
				break;

			case T_STRING: case T_CSTRING:
				
				if (constant->value == VOID_STRING)
				{
					write_int(0);
					write_int(0);
				}
				else
				{
					sym = TABLE_get_symbol(Class->string, constant->value);
					write_int(get_string(sym->name, sym->len));
					write_int(sym->len);
				}
				break;
		}
	}

	end_section();
}


static void output_class_ref(void)
{
	int i, n;
	SYMBOL *sym;
	CLASS_REF *ref;

	n = ARRAY_count(Class->class);

	begin_section("External classes", sizeof(int));

	for (i = 0; i < n; i++)
	{
		ref = &Class->class[i];
		sym = TABLE_get_symbol(Class->table, ref->index);
		if (ref->used)
		{
			if (ref->exported)
				write_int(-get_string(sym->name, sym->len));
			else
				write_int(get_string(sym->name, sym->len));
		}
		else
		{
			if (JOB->verbose)
				printf("Ignoring class %.*s\n", sym->len, sym->name);
			write_int(-1);
		}
	}

	end_section();
}


static void output_unknown_ref(void)
{
	int i, n;
	SYMBOL *sym;

	n = ARRAY_count(Class->unknown);

	begin_section("External symbols", sizeof(int));

	for (i = 0; i < n; i++)
	{
		sym = TABLE_get_symbol(Class->table, Class->unknown[i]);
		write_int(get_string(sym->name, sym->len));
	}

	end_section();
}


static void output_static(void)
{
	int i, n;
	VARIABLE *var;

	n = ARRAY_count(Class->stat);

	begin_section("Static variables", 2 * sizeof(int));

	for (i = 0; i < n; i++)
	{
		var = &Class->stat[i];

		/* type */
		write_type(var->type);
		/* addr */
		write_int(var->pos);
	}

	end_section();
}


static void output_dynamic(void)
{
	int i, n;
	VARIABLE *var;

	n = ARRAY_count(Class->dyn);

	begin_section("Dynamic variables", 2 * sizeof(int));

	for (i = 0; i < n; i++)
	{
		var = &Class->dyn[i];

		/* type */
		write_type(var->type);
		/* addr */
		write_int(var->pos);
	}

	end_section();
}


static void output_event(void)
{
	int i, n;
	EVENT *event;
	SYMBOL *sym;

	n = ARRAY_count(Class->event);

	begin_section("Events", 4 * sizeof(int));

	for (i = 0; i < n; i++)
	{
		event = &Class->event[i];

		/* type */
		write_type(event->type);
		/* n_param */
		write_short(event->nparam);
		/* reserved */
		write_short(0);
		/* desc_param */
		write_int(0);
		/* name */
		sym = TABLE_get_symbol(Class->table, event->name);
		write_int(get_string(sym->name, sym->len));
	}

	end_section();
}


static void output_extern(void)
{
	int i, n;
	EXTFUNC *ext;
	SYMBOL *sym;

	n = ARRAY_count(Class->ext_func);

	begin_section("Extern functions", 5 * sizeof(int));

	for (i = 0; i < n; i++)
	{
		ext = &Class->ext_func[i];

		/* type */
		write_type(ext->type);
		/* n_param */
		write_short(ext->nparam);
		/* vararg */
		write_byte(ext->vararg);
		/* reserved */
		write_byte(0);
		/* desc_param */
		write_int(0);
		/* name */
		/*sym = TABLE_get_symbol(Class->table, ext->name);
		write_int(get_string(sym->name, sym->len));*/
		/* alias name */
		if (ext->alias == NO_SYMBOL)
			sym = TABLE_get_symbol(Class->table, ext->name);
		else
			sym = TABLE_get_symbol(Class->string, ext->alias);
		write_int(get_string(sym->name, sym->len));
		/* library name */
		sym = TABLE_get_symbol(Class->string, ext->library);
		write_int(get_string(sym->name, sym->len));
	}

	end_section();
}


static void output_method(void)
{
	int i, n;
	FUNCTION *func;
	uchar flag;
	/*SYMBOL *sym;*/

	n = ARRAY_count(Class->function);

	begin_section("Methods", 8 * sizeof(int));

	for (i = 0; i < n; i++)
	{
		func = &Class->function[i];

		write_type(func->type);
		write_byte(func->nparam);
		write_byte(func->npmin);
		write_byte(func->vararg);

		flag = func->fast || Class->all_fast;
		if (func->use_is_missing)
			flag += 2;

		write_byte(flag);

		write_short(func->nlocal);
		write_short(func->nctrl);
		write_short(func->stack);

		/* gestion d'erreur */
		if (func->catch && func->finally)
			write_short(Min(func->finally, func->catch));
		else if (func->catch)
			write_short(func->catch);
		else
			write_short(func->finally);

		/* addr_code */
		write_int(func->ncode);
		/* desc_param */
		write_int(0);
		/* desc_local */
		write_int(0);
		/* debug_info */
		write_int(0);

	}

	end_section();
}


static void output_param_local(void)
{
	int i, j;
	FUNCTION *func;
	EVENT *event;
	EXTFUNC *ext;
	PARAM *param;

	begin_section("Parameters", sizeof(int));

	for (i = 0; i < ARRAY_count(Class->function); i++)
	{
		func = &Class->function[i];

		if (func->name != NO_SYMBOL)
		{
			/* Les param�res sont remis dans les variables locales !
			for (j = 0; j < func->nparam; j++)
			{
				param = &func->param[j];
				write_int(param->type);
			}
			*/

			for (j = 0; j < func->nlocal + func->nparam; j++)
			{
				param = &func->local[j];

				/* type */
				write_type(param->type);
			}
		}
	}

	for (i = 0; i < ARRAY_count(Class->event); i++)
	{
		event = &Class->event[i];

		for (j = 0; j < event->nparam; j++)
		{
			param = &event->param[j];

			/* type */
			write_type(param->type);
		}
	}

	for (i = 0; i < ARRAY_count(Class->ext_func); i++)
	{
		ext = &Class->ext_func[i];

		for (j = 0; j < ext->nparam; j++)
		{
			param = &ext->param[j];

			/* type */
			write_type(param->type);
		}
	}

	end_section();
}


static void output_array(void)
{
	int i, j, p;
	CLASS_ARRAY *array;

	begin_section("Arrays", sizeof(int));

	p = ARRAY_count(Class->array) * sizeof(int);

	for (i = 0; i < ARRAY_count(Class->array); i++)
	{
		array = &Class->array[i];
		write_int(p);
		p += sizeof(int) + array->ndim * sizeof(int);
	}

	for (i = 0; i < ARRAY_count(Class->array); i++)
	{
		array = &Class->array[i];

		write_type(array->type);

		for (j = 0; j < array->ndim; j++)
		{
			p = array->dim[j];
			if (j == (array->ndim - 1))
				p = (-p);

			write_int(p);
		}
	}

	end_section();
}


static void output_structure(void)
{
	int i, j;
	CLASS_STRUCT *structure;
	VARIABLE *field;
	SYMBOL *sym;

	for (i = 0; i < ARRAY_count(Class->structure); i++)
	{
		structure = &Class->structure[i];
		
		begin_section("Structure", sizeof(int));
		
		// Structure name
		sym = TABLE_get_symbol(Class->table, structure->index);
		write_int(get_string(sym->name, sym->len));
		
		for (j = 0; j < structure->nfield; j++)
		{
			field = &structure->field[j];
			
			// Field name
			sym = TABLE_get_symbol(Class->table, field->index);
			write_int(get_string(sym->name, sym->len));
			
			// Field datatype
			write_type(field->type);
		}
		
		end_section();
	}
}

static void output_code()
{
	int i, j;
	int n;
	FUNCTION *func;

	for (i = 0; i < ARRAY_count(Class->function); i++)
	{
		func = &Class->function[i];

		n = func->ncode;

		begin_section("Code", sizeof(short));
		if (_swap)
		{
			for (j = 0; j < n; j++)
				write_short(func->code[j]);
		}
		else
			write_buffer(func->code, n * sizeof(short));
		end_section();
	}
}


static void output_debug_global()
{
	int i, nn = 0;
	CLASS_SYMBOL *csym;
	TYPE type;

	begin_section("Global symbol table", 4 * sizeof(int));

	for (i = 0; i < TABLE_count(Class->table); i++)
	{
		csym = (CLASS_SYMBOL *)TABLE_get_symbol_sort(Class->table, i);
		//csym = (CLASS_SYMBOL *)TABLE_get_symbol(Class->table, csym->symbol.sort);

		type = csym->global.type;
		switch (TYPE_get_kind(type))
		{
			case TK_VARIABLE:
			case TK_FUNCTION:
			case TK_PROPERTY:
			case TK_EXTERN:
			case TK_CONST:

				nn++;
				/* name */
				write_int(get_string(csym->symbol.name, csym->symbol.len));
				/* len */
				write_int(csym->symbol.len);
				/* type */
				write_type(csym->global.type);
				/* value */
				write_int(csym->global.value);

				break;

			default:
				/* ignore */
				break;
		}
	}

	end_section();
	
	begin_section("Global symbol table sort", sizeof(ushort));
	
	for (i = 0; i < nn; i++)
		write_short(i);

	end_section();
}


static void output_debug_method()
{
	int i, j, n;
	SYMBOL *sym;
	OUTPUT_SYMBOL *osym;
	CLASS_SYMBOL *csym;
	PARAM *param;
	FUNCTION *func;
	TABLE *table;
	int index;

	begin_section("Debug method info", 5 * sizeof(int));

	for (i = 0; i < ARRAY_count(Class->function); i++)
	{
		func = &Class->function[i];

		if (func->pos_line != NULL && func->line < FORM_FIRST_LINE)
		{
			/* line */
			write_short(func->line);
			write_short(ARRAY_count(func->pos_line));
			/* pos_line */
			write_int(0);
			/* nom */
			sym = TABLE_get_symbol(Class->table, func->name);
			write_int(get_string(sym->name, sym->len));
			/* local symbols */
			write_int(0);
			/* n_local */
			write_short(0);
			/* reserved */
			write_short(0);
		}
		else
		{
			write_short(0);
			write_short(0);
			write_int(0);
			write_int(0);
			write_int(0);
			write_short(0);
			write_short(0);
		}
	}

	end_section();

	for (i = 0; i < ARRAY_count(Class->function); i++)
	{
		func = &Class->function[i];

		n = func->pos_line ? ARRAY_count(func->pos_line) : 0;

		begin_section("Debug method lines", sizeof(short));

		if (_swap)
		{
			for (j = 0; j < n; j++)
				write_short(func->pos_line[j]);
		}
		else
			write_buffer(func->pos_line, n * sizeof(short));

		end_section();
	}

	for (i = 0; i < ARRAY_count(Class->function); i++)
	{
		func = &Class->function[i];

		begin_section("Debug method local symbols", sizeof(int) * 3);

		if (func->name != NO_SYMBOL)
		{
			sym = (SYMBOL *)TABLE_get_symbol(Class->table, func->name);
			/*printf("%.*s()\n", sym->len, sym->name);*/

			TABLE_create(&table, sizeof(OUTPUT_SYMBOL), TF_IGNORE_CASE);

			for (j = 0; j < func->nlocal + func->nparam; j++)
			{
				param = &func->local[j];
				csym = (CLASS_SYMBOL *)TABLE_get_symbol(Class->table, param->index);

				TABLE_add_symbol(table, csym->symbol.name, csym->symbol.len, &index);
				osym = (OUTPUT_SYMBOL *)TABLE_get_symbol(table, index);
				osym->value = param->value;/*TYPE_long(param->type);*/
			}

			for (j = 0; j < TABLE_count(table); j++)
			{
				param = &func->local[j];
				osym = (OUTPUT_SYMBOL *)TABLE_get_symbol(table, j);

				/* name */
				write_int(get_string(osym->sym.name, osym->sym.len));
				/* len */
				write_int(osym->sym.len);
				/* value */
				write_int(osym->value);

				/*printf("%.*s %ld\n", osym->sym.len, osym->sym.name, osym->value);*/
			}
			
			// We actually do not use the table sort, so do not output it!
			
			TABLE_delete(&table);
		}

		end_section();
	}
}


static void output_debug_filename(void)
{
	char *path;
	int n;

	begin_section("Debug file name", 1);

	path = (char *)FILE_get_name(JOB->name);

	n = strlen(path);
	write_buffer(path, n);
	/*write_pad();*/

	end_section();
}


static void output_string(void)
{
	int i;
	SYMBOL *sym;

	begin_section("Strings", 1);

	for (i = 0; i < TABLE_count(StringTable); i++)
	{
		sym = TABLE_get_symbol(StringTable, i);
		write_string(sym->name, sym->len);
	}

	end_section();
}


char *OUTPUT_get_file(const char *file)
{
	char *output;
	char *p;
	//char *dir;
	char *name;

	//dir = STR_copy(FILE_get_dir(file));
	name = STR_copy(FILE_get_name(file));

	for (p = name; *p; p++)
	{
		if (*p == '.')
		{
			*p = 0;
			break;
		}

		*p = toupper(*p);
	}

	output = ".gambas";
	if (mkdir(output, 0777) == 0)
		FILE_set_owner(output, COMP_project);

	output = STR_copy(FILE_cat(output, name, NULL));

	//STR_free(dir);
	STR_free(name);

	return output;
}


char *OUTPUT_get_trans_file(const char *file)
{
	char *output;
	//char *dir;
	char *name;

	//dir = STR_copy(FILE_get_dir(file));
	name = STR_copy(FILE_get_name(file));

	output = ".lang"; //(char *)FILE_cat(dir, ".lang", NULL);
	if (mkdir(output, 0777) == 0)
		FILE_set_owner(output, COMP_project);

	output = (char *)FILE_cat(".lang", name, NULL);
	output = STR_copy(FILE_set_ext(output, "pot"));

	//STR_free(dir);
	STR_free(name);

	return output;
}

static void output_translation(void)
{
	FILE *file;
	int i, j, n;
	CONSTANT *constant;
	SYMBOL *sym;
	unsigned char c;
	time_t t;
	struct tm *now;

	/*printf("Generating %s\n", JOB->tname);*/

	if (!JOB->trans)
	{
		JOB->tname = OUTPUT_get_trans_file(JOB->name);
		FILE_unlink(JOB->tname);
		return;
	}
	
	file = fopen(JOB->tname, "w");
	if (!file)
		THROW("Cannot create file: &1", JOB->tname);

	fprintf(file, "# %s\n# Generated by the Gambas " GAMBAS_FULL_VERSION_STRING " compiler\n\n", JOB->name);

	fprintf(file,
		"#, fuzzy\n"
		"msgid \"\"\n"
		"msgstr \"\"\n"
		"\"Project-Id-Version: $(PACKAGE) $(VERSION)\\n\"\n");

	t = time(NULL);
	now = gmtime(&t);

	fprintf(file, "\"POT-Creation-Date: %04d-%02d-%02d %02d:%02d UTC\\n\"\n",
		now->tm_year + 1900, now->tm_mon + 1, now->tm_mday, now->tm_hour, now->tm_min);

	// "\"PO-Revision-Date: $(DATE)\\n\"\n" // YEAR-MO-DA HO:MI+ZONE
	fprintf(file,
		"\"MIME-Version: 1.0\\n\"\n"
		"\"Content-Type: text/plain; charset=UTF-8\\n\"\n"
		"\"Content-Transfer-Encoding: 8bit\\n\"\n\n");

	n = ARRAY_count(Class->constant);

	for (i = 0; i < n; i++)
	{
		constant = &Class->constant[i];
		if (TYPE_get_id(constant->type) != T_CSTRING)
			continue;

		sym = TABLE_get_symbol(Class->string, constant->value);
		if (sym->len == 0)
			continue;

		for (j = 0; j < sym->len; j++)
		{
			c = sym->name[j];
			if (c > ' ')
				break;
		}

		if (j >= sym->len)
			continue;

		if (constant->line < FORM_FIRST_LINE)
			fprintf(file, "#: %s:%d\n", FILE_get_name(JOB->name), constant->line);
		else
			fprintf(file, "#: %s:%d\n", FILE_get_name(JOB->form), constant->line - FORM_FIRST_LINE);

		fprintf(file, "msgid \"");

		for (j = 0; j < sym->len; j++)
		{
			c = sym->name[j];
			if (c == '\n')
				fprintf(file, "\\n");
			else if (c == '\t')
				fprintf(file, "\\t");
			else if (c == '\r')
				fprintf(file, "\\r");
			else if (c == '\\')
				fprintf(file, "\\\\");
			else if (c == '"')
				fprintf(file, "\\\"");
			else
				fputc(c, file);
		}

		fprintf(file, "\"\n");
		fprintf(file, "msgstr \"\"\n\n");

		sym->len = 0; /* Horrible hack for not writing the same string twice */
	}

	fclose(file);
	FILE_set_owner(JOB->tname, COMP_project);
}


static void output_finalize(void)
{
	int i;
	
	flush_buffer();
	
	for (i = 0; i < ARRAY_count(_change); i++)
	{
		fseek(_file, _change[i].pos, SEEK_SET);
		if (fwrite(&_change[i].val, sizeof(int), 1, _file) != 1)
			THROW("Write error");
	}
}


void OUTPUT_do(bool swap)
{
	const char *name;

	_swap = swap;

	output_init();

	// The first string is always the class name
	get_string(JOB->class->name, strlen(JOB->class->name));

	name = JOB->output;

	#ifdef DEBUG
	printf("Output to %s\n", name);
	#endif

	_file = fopen(name, "w");
	if (!_file)
		THROW("Cannot create file: &1", name);

	Class = JOB->class;

	#ifdef DEBUG
	printf("pos = %lu\n", get_pos());
	#endif

	output_header();
	output_class();
	output_desc();
	output_constant();
	output_class_ref();
	output_unknown_ref();
	output_static();
	output_dynamic();
	output_event();
	output_extern();
	output_method();
	output_param_local();
	output_array();
	output_structure();
	output_code();

	if (JOB->debug)
	{
		output_debug_global();
		output_debug_method();
		output_debug_filename();
	}

	output_string();

	output_finalize();
	
	fclose(_file);
	FILE_set_owner(JOB->output, COMP_project);

	output_exit();

	// Translations
	output_translation();
}
