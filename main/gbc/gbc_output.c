/***************************************************************************

  output.c

  The object file creator

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

#define __GBC_OUTPUT_C

#include "gb_common.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>

#include "config.h"

#include "gb_error.h"
#include "gb_alloc.h"
#include "gb_str.h"
#include "gb_file.h"
#include "gb_common_swap.h"
#include "gb_magic.h"

#include "gbc_compile.h"

#include "gbc_output.h"


/*#define DEBUG*/
/*#define DEBUG_MORE*/

static CLASS *Class;

static long StringAddr;
static TABLE *StringTable;
static int NSection;
static long PosStartSection;
static int SizeSection;

static bool _swap;

static FILE *_file;

static long _pos;
static char _buffer[OUTPUT_BUFFER_SIZE + 8];
static const char * const _mbuffer = &_buffer[OUTPUT_BUFFER_SIZE];
static char *_pbuffer;

static OUTPUT_CHANGE *_change;


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


static long get_string(const char *string, int len)
{
  OUTPUT_SYMBOL *sym;

  if (len < 0)
    len = strlen(string);

  if (!TABLE_add_symbol(StringTable, string, len, (SYMBOL **)(void *)&sym, NULL))
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


static void write_long(ulong val)
{
  #ifdef DEBUG_MORE
  printf("%ld : l %lu 0x%lX\n", get_pos(), val, val);
  #endif

  if (_swap)
  	SWAP_long((long *)&val);

  if (_pbuffer >= _mbuffer)
    flush_buffer();

  *((ulong *)_pbuffer) = val;
  _pbuffer += sizeof(val);
  _pos += sizeof(val);
}


static void write_long_long(unsigned long long val)
{
  #ifdef DEBUG_MORE
  printf("%ld : l %llu 0x%llX\n", get_pos(), val, val);
  #endif

  if (_swap)
  	SWAP_long_long((long long *)&val);

  if (_pbuffer >= _mbuffer)
    flush_buffer();
    
  *((unsigned long long *)_pbuffer) = val;
  _pbuffer += sizeof(val);
  _pos += sizeof(val);
}

#if 0
static void write_long_at(long pos, ulong val)
{
  long prev = get_pos();
  char *ppos;
  
  ppos = &_buffer[pos - (prev - (_pbuffer - _buffer))];
  if (ppos >= _buffer && ppos < (_pbuffer - sizeof(ulong)))
  {
    *((ulong *)ppos) = val;
    return;
  }

  flush_buffer();

  #ifdef DEBUG_MORE
  printf("@ ");
  #endif

  fseek(_file, pos, SEEK_SET);
  write_long(val);
  flush_buffer();
  
  fseek(_file, prev, SEEK_SET);
  
  _pos = prev;
}
#endif

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


static void write_buffer(void *str, long len)
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


static void add_change(off_t pos, ulong val)
{
  long prev = get_pos();
  char *ppos;
  OUTPUT_CHANGE *change;    
  
  ppos = &_buffer[pos - (prev - (_pbuffer - _buffer))];
  if (ppos >= _buffer && ppos < (_pbuffer - sizeof(ulong)))
  {
    *((ulong *)ppos) = val;
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
    write_long(0);
  }
  else
    PosStartSection = 0;

  SizeSection = size;
}


static void end_section(void)
{
  long len;

  if (PosStartSection)
  {
    write_pad();
    len = get_pos() - PosStartSection - sizeof(long);
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
  write_long(OUTPUT_MAGIC);
  /* version */
  write_long(GAMBAS_PCODE_VERSION);
  /* endianness */
  write_long(OUTPUT_ENDIAN);
  /* flag */
  if (JOB->debug)
    write_long(1);
  else
    write_long(0);

  end_section();
}


static void output_class(void)
{
  short flag;
  begin_section("Class", 1);

  /* classe parente */
  write_short(Class->parent);
  /* flags */
  flag = 0;
  if (Class->exported) flag |= 1;
  if (Class->autocreate) flag |= 2;
  if (Class->optional) flag |= 4;
  if (Class->nocreate) flag |= 8;
  write_short(flag);
  /* size_static */
  write_long(Class->size_stat);
  /* size_dynamic */
  write_long(Class->size_dyn);

  end_section();
}


static void output_desc(void)
{
  int i, n, nn = 0;
  CLASS_SYMBOL *csym;
  TYPE type;
  short out_type;

  n = TABLE_count(Class->table);

  begin_section("Description", 6 * sizeof(long));

  for (i = 0; i < n; i++)
  {
    csym = (CLASS_SYMBOL *)TABLE_get_symbol(Class->table, i);
    csym = (CLASS_SYMBOL *)TABLE_get_symbol(Class->table, csym->symbol.sort);

    type = csym->global.type;

    if (TYPE_is_public(type))
    {
      nn++;
      /* name */
      write_long(get_string(csym->symbol.name, csym->symbol.len));
			/* datatype */
			write_type(csym->global.type);

      switch (TYPE_get_kind(type))
      {
        case TK_VARIABLE:

          /* offset */
          write_long(csym->global.value);
          /* read */
          write_long(0);
          /* write */
          write_long(0);

          if (TYPE_is_static(type))
            out_type = CD_STATIC_VARIABLE_ID;
          else
            out_type = CD_VARIABLE_ID;

          break;

        case TK_PROPERTY:

          /* read */
          write_long(Class->prop[csym->global.value].read);
          /* write */
          write_long(Class->prop[csym->global.value].write);
          /* flag */
          write_long(0);

          if (TYPE_is_static(type))
            out_type = CD_STATIC_PROPERTY_ID;
          else
            out_type = CD_PROPERTY_ID;

          break;

        case TK_CONST:

          /* param */
          write_long(csym->global.value);
          /* read */
          write_long(0);
          /* write */
          write_long(0);

          out_type = CD_CONSTANT_ID;

          break;

        case TK_FUNCTION:

          /* exec */
          write_long(csym->global.value);
          /* signature */
          write_long(0);
          /* nparam */
          write_long(0);

          if (TYPE_is_static(type))
            out_type = CD_STATIC_METHOD_ID;
          else
            out_type = CD_METHOD_ID;

          break;

        case TK_EVENT:

          /* exec */
          write_long(csym->global.value);
          /* signature */
          write_long(0);
          /* nparam */
          write_long(0);

          out_type = CD_EVENT_ID;

          break;

        case TK_EXTERN:

          /* exec */
          write_long(csym->global.value);
          /* signature */
          write_long(0);
          /* nparam */
          write_long(0);

          out_type = CD_EXTERN_ID;

          break;

        default:

          ERROR_panic("output_desc: unknown symbol type");
          continue;
       }

      /* type de symbole */
      write_long(out_type);
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

  begin_section("Constants", 3 * sizeof(long));

  for (i = 0; i < n; i++)
  {
    constant = &Class->constant[i];

    /* type */
    write_type(constant->type);
    /* value */
    switch (TYPE_get_id(constant->type))
    {
      case T_BOOLEAN: case T_BYTE: case T_SHORT: case T_INTEGER:

        write_long(constant->value);
        write_long(0);
        break;

      case T_LONG:

        write_long_long(constant->lvalue);
        break;

      case T_SINGLE: case T_FLOAT:

        sym = TABLE_get_symbol(Class->table, constant->value);
        write_long(get_string(sym->name, sym->len));
        write_long(sym->len);
        break;

      case T_STRING: case T_CSTRING:

        sym = TABLE_get_symbol(Class->string, constant->value);
        write_long(get_string(sym->name, sym->len));
        write_long(sym->len);
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

  begin_section("External classes", sizeof(long));

  for (i = 0; i < n; i++)
  {
  	ref = &Class->class[i];
   	sym = TABLE_get_symbol(Class->table, ref->index);
  	if (ref->used)
  	{
	    write_long(get_string(sym->name, sym->len));
		}
		else
		{
			if (JOB->verbose)
				printf("Ignoring class %.*s\n", sym->len, sym->name);
    	write_long(-get_string(sym->name, sym->len));
		}
  }

  end_section();
}


static void output_unknown_ref(void)
{
  int i, n;
  SYMBOL *sym;

  n = ARRAY_count(Class->unknown);

  begin_section("External symbols", sizeof(long));

  for (i = 0; i < n; i++)
  {
    sym = TABLE_get_symbol(Class->table, Class->unknown[i]);
    write_long(get_string(sym->name, sym->len));
  }

  end_section();
}


static void output_static(void)
{
  int i, n;
  VARIABLE *var;

  n = ARRAY_count(Class->stat);

  begin_section("Static variables", 2 * sizeof(long));

  for (i = 0; i < n; i++)
  {
    var = &Class->stat[i];

    /* type */
    write_type(var->type);
    /* addr */
    write_long(var->pos);
  }

  end_section();
}


static void output_dynamic(void)
{
  int i, n;
  VARIABLE *var;

  n = ARRAY_count(Class->dyn);

  begin_section("Dynamic variables", 2 * sizeof(long));

  for (i = 0; i < n; i++)
  {
    var = &Class->dyn[i];

    /* type */
    write_type(var->type);
    /* addr */
    write_long(var->pos);
  }

  end_section();
}


static void output_event(void)
{
  int i, n;
  EVENT *event;
  SYMBOL *sym;

  n = ARRAY_count(Class->event);

  begin_section("Events", 4 * sizeof(long));

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
    write_long(0);
    /* name */
    sym = TABLE_get_symbol(Class->table, event->name);
    write_long(get_string(sym->name, sym->len));
  }

  end_section();
}


static void output_extern(void)
{
  int i, n;
  EXTFUNC *ext;
  SYMBOL *sym;

  n = ARRAY_count(Class->ext_func);

  begin_section("Extern functions", 5 * sizeof(long));

  for (i = 0; i < n; i++)
  {
    ext = &Class->ext_func[i];

    /* type */
    write_type(ext->type);
    /* n_param */
    write_short(ext->nparam);
    /* reserved */
    write_short(0);
    /* desc_param */
    write_long(0);
    /* name */
    /*sym = TABLE_get_symbol(Class->table, ext->name);
    write_long(get_string(sym->name, sym->len));*/
    /* alias name */
    if (ext->alias == NO_SYMBOL)
      sym = TABLE_get_symbol(Class->table, ext->name);
    else
      sym = TABLE_get_symbol(Class->string, ext->alias);
    write_long(get_string(sym->name, sym->len));
    /* library name */
    sym = TABLE_get_symbol(Class->string, ext->library);
    write_long(get_string(sym->name, sym->len));
  }

  end_section();
}


static void output_method(void)
{
  int i, n;
  FUNCTION *func;
  /*SYMBOL *sym;*/

  n = ARRAY_count(Class->function);

  begin_section("Methods", 8 * sizeof(long));

  for (i = 0; i < n; i++)
  {
    func = &Class->function[i];

    write_type(func->type);
    write_byte(func->nparam);
    write_byte(func->npmin);
    write_byte(func->vararg);
    write_byte(0);
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
    write_long(func->ncode);
    /* desc_param */
    write_long(0);
    /* desc_local */
    write_long(0);
    /* debug_info */
    write_long(0);

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

  begin_section("Parameters", sizeof(long));

  for (i = 0; i < ARRAY_count(Class->function); i++)
  {
    func = &Class->function[i];

    if (func->name != NO_SYMBOL)
    {
      /* Les param�res sont remis dans les variables locales !
      for (j = 0; j < func->nparam; j++)
      {
        param = &func->param[j];
        write_long(param->type);
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
  long i, j, p;
  CLASS_ARRAY *array;

  begin_section("Arrays", sizeof(long));

  p = ARRAY_count(Class->array) * sizeof(long);

  for (i = 0; i < ARRAY_count(Class->array); i++)
  {
    array = &Class->array[i];
    write_long(p);
    p += sizeof(long) + array->ndim * sizeof(long);
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

      write_long(p);
    }
  }

  end_section();
}


static void output_code()
{
  int i, j;
  long n;
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

  begin_section("Global symbol table", 4 * sizeof(long));

  for (i = 0; i < TABLE_count(Class->table); i++)
  {
    csym = (CLASS_SYMBOL *)TABLE_get_symbol(Class->table, i);
    csym = (CLASS_SYMBOL *)TABLE_get_symbol(Class->table, csym->symbol.sort);

    type = csym->global.type;
    switch (TYPE_get_kind(type))
    {
      case TK_VARIABLE:
      case TK_FUNCTION:
      case TK_PROPERTY:
      case TK_EXTERN:
      case TK_CONST:

        nn++;
        /* sort */
        write_short((short)nn);
        /* len */
        write_short(csym->symbol.len);
        /* name */
        write_long(get_string(csym->symbol.name, csym->symbol.len));
        /* type */
        write_type(csym->global.type);
        /* value */
        write_long(csym->global.value);

        break;

      default:
        /* ignore */
        break;
    }
  }

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

  begin_section("Debug method info", 5 * sizeof(long));

  for (i = 0; i < ARRAY_count(Class->function); i++)
  {
    func = &Class->function[i];

    if (func->pos_line != NULL)
    {
      /* line */
      write_short(func->line);
      write_short(ARRAY_count(func->pos_line));
      /* pos_line */
      write_long(0);
      /* nom */
      sym = TABLE_get_symbol(Class->table, func->name);
      write_long(get_string(sym->name, sym->len));
      /* local symbols */
      write_long(0);
      /* n_local */
      write_short(0);
      /* reserved */
      write_short(0);
    }
    else
    {
      write_short(0);
      write_short(0);
      write_long(0);
      write_long(0);
      write_long(0);
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

    begin_section("Debug method local symbols", sizeof(long) * 3);

    if (func->name != NO_SYMBOL)
    {
      sym = (SYMBOL *)TABLE_get_symbol(Class->table, func->name);
      /*printf("%.*s()\n", sym->len, sym->name);*/

      TABLE_create(&table, sizeof(OUTPUT_SYMBOL), TF_IGNORE_CASE);

      for (j = 0; j < func->nlocal + func->nparam; j++)
      {
        param = &func->local[j];
        csym = (CLASS_SYMBOL *)TABLE_get_symbol(Class->table, param->index);

        TABLE_add_symbol(table, csym->symbol.name, csym->symbol.len, (SYMBOL **)(void *)&osym, NULL);
        osym->value = param->value;/*TYPE_long(param->type);*/
      }

      for (j = 0; j < TABLE_count(table); j++)
      {
        param = &func->local[j];
        osym = (OUTPUT_SYMBOL *)TABLE_get_symbol(table, j);

        /* sort */
        write_short(osym->sym.sort);
        /* len */
        write_short(osym->sym.len);
        /* name */
        write_long(get_string(osym->sym.name, osym->sym.len));
        /* value */
        write_long(osym->value);

        /*printf("%.*s %ld\n", osym->sym.len, osym->sym.name, osym->value);*/
      }

      TABLE_delete(&table);
    }

    end_section();
  }
}


static void output_debug_filename(void)
{
  char path[MAX_PATH + 1];
  long n;

  begin_section("Debug file name", 1);

  if (JOB->name[0] == '/')
  {
    strcpy(path, JOB->name);
  }
  else
  {
    getcwd(path, MAX_PATH);
    n = strlen(path);

    if (path[n - 1] != '/')
      strcpy(&path[n], "/");

    strcat(&path[n], JOB->name);
  }

  n = strlen(path);
  write_buffer(path, n);
  /*write_pad();*/

  end_section();
}


static void output_string(void)
{
  long i;
  SYMBOL *sym;

  begin_section("Strings", 1);

  for (i = 0; i < TABLE_count(StringTable); i++)
  {
    sym = TABLE_get_symbol(StringTable, i);
    write_string(sym->name, sym->len);
  }

  end_section();
}


PUBLIC const char *OUTPUT_get_file(const char *file)
{
  const char *output;
  char *p;
  const char *dir;
  char *name;

  dir = STR_copy(FILE_get_dir(file));
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

  output = FILE_cat(dir, ".gambas", NULL);
  mkdir(output, 0777);

  output = STR_copy(FILE_cat(dir, ".gambas", name, NULL));

  STR_free(dir);
  STR_free(name);

  return output;
}


PUBLIC const char *OUTPUT_get_trans_file(const char *file)
{
  const char *output;
  const char *dir;
  const char *name;

  dir = STR_copy(FILE_get_dir(file));
  name = STR_copy(FILE_get_name(file));

  output = FILE_cat(dir, ".lang", NULL);
  mkdir(output, 0777);

  output = FILE_cat(dir, ".lang", name, NULL);
  output = STR_copy(FILE_set_ext(output, "pot"));

  STR_free(dir);
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

  /*printf("Generating %s\n", JOB->tname);*/

  file = fopen(JOB->tname, "w");
  if (!file)
    THROW("Cannot create file '&1'", JOB->tname);

  fprintf(file, "# %s\n# Generated by Gambas compiler\n\n", JOB->name);

  fprintf(file,
    "# FIRST AUTHOR <EMAIL@ADDRESS>, YEAR.\n"
    "#\n"
    "#, fuzzy\n"
    "msgid \"\"\n"
    "msgstr \"\"\n"
    "\"Project-Id-Version: PACKAGE VERSION\\n\""
    "\"POT-Creation-Date: 2002-11-01 04:27+0100\\n\"\n"
    "\"PO-Revision-Date: YEAR-MO-DA HO:MI+ZONE\\n\"\n"
    "\"Last-Translator: FULL NAME <EMAIL@ADDRESS>\\n\"\n"
    "\"Language-Team: LANGUAGE <LL@li.org>\\n\"\n"
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

    fprintf(file, "#: %s:%ld\n", FILE_get_name(JOB->name), constant->line);

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

    sym->len = 0; /* Immonde ruse pour ne pas �rire deux fois les m�es chaines */
  }

  fclose(file);
}


static void output_finalize(void)
{
  int i;
  
  flush_buffer();
  
  for (i = 0; i < ARRAY_count(_change); i++)
  {
    fseek(_file, _change[i].pos, SEEK_SET);
    if (fwrite(&_change[i].val, sizeof(long), 1, _file) != 1)
      THROW("Write error");
  }
}


PUBLIC void OUTPUT_do(bool swap)
{
  const char *name;

  _swap = swap;

  output_init();

  /* La premi�e cha�e est toujours le nom de la classe */
  get_string(JOB->class->name, strlen(JOB->class->name));

  name = JOB->output;

  #ifdef DEBUG
  printf("Output to %s\n", name);
  #endif

  _file = fopen(name, "w");
  if (!_file)
    THROW("Cannot create file '&1'", name);

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

  output_exit();

  /* Internationalisation */

  if (JOB->trans)
    output_translation();
}
