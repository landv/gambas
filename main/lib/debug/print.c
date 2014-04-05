/***************************************************************************

  print.c

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

#define __PRINT_C

// Do not include gbx_debug.h
#define __GBX_DEBUG_H

#include "gb_common.h"

#include <unistd.h>
#include <signal.h>
#include <fcntl.h>

#include "gb_error.h"
#include "gbx_type.h"
#include "gb_limit.h"
#include "gbx_class.h"
#include "gbx_object.h"
#include "gbx_local.h"
#include "gbx_variant.h"

#include "gambas.h"

#include "print.h"


static FILE *_where;
static int _level;

static void print_value(VALUE *value);

static void print_string(const char *s, int len)
{
	int i;
	uchar c;

	fputc('"', _where);

	for (i = 0; i < len; i++)
	{
		if (i > (DEBUG_OUTPUT_MAX_SIZE - 8))
		{
			fprintf(_where, "...");
			break;
		}

		c = s[i];

		if (c < 32)
		{
			if (c == 10)
				fprintf(_where, "\\n");
			else if (c == 13)
				fprintf(_where, "\\r");
			else if (c == 9)
				fprintf(_where, "\\t");
			else
				fprintf(_where, "\\x%02X", c);
		}
		else if (c == '\"')
		{
			fprintf(_where, "\\\"");
		}
		else
		{
			fputc(c, _where);
		}
	}

	fputc('"', _where);
}

#if 0
static void new_line(void)
{
	int i;

	fputc('\t', _where);
	for (i = 1; i < _level; i++)
		fprintf(_where, "    ");
}
#endif

static void print_object(void *object)
{
	//int i;
	//char *key;
	//VALUE value;
	//long len;

	fprintf(_where, "(%s %p)", OBJECT_class(object)->name, object);

/*
	if (GB.Is(object, GB.FindClass("Collection")))
	{
		//if (_level > 1)
		//	new_line();

		//fprintf(_where, "%s", OBJECT_class(object)->name);
		GB_DEBUG.EnumCollection(object, NULL, NULL, NULL);
		for (i = 0; i < 8; i++)
		{
			if (GB_DEBUG.EnumCollection(object, (GB_VARIANT *)&value, &key, &len))
				break;
			new_line();
			fprintf(_where, "[");
			print_string(key, len);
			fprintf(_where, "] = ");
			print_value(&value);
		}
		if (GB.Collection.Count(object) > 8)
		{
			new_line();
			fprintf(_where, "... #%ld", GB.Collection.Count(object));
		}
	}
	else if (GB.Is(object, GB.FindClass("Array")))
	{
		//if (_level > 1)
		//	new_line();

		//fprintf(_where, "%s", OBJECT_class(object)->name);
		for (i = 0; i < GB.Array.Count(object); i++)
		{
			if (i >= 8 && i < (GB.Array.Count(object) - 1))
			{
				new_line();
				fprintf(_where, "...");
				i = GB.Array.Count(object) - 1;
			}
			new_line();
			GB_DEBUG.GetArrayValue(object, i, (GB_VALUE *)&value);
			fprintf(_where, "[%d] = ", i);
			print_value(&value);
		}
	}
*/
}

static void print_value(VALUE *value)
{
  static void *jump[16] = {
    &&__VOID, &&__BOOLEAN, &&__BYTE, &&__SHORT, &&__INTEGER, &&__LONG, &&__SINGLE, &&__FLOAT, &&__DATE,
    &&__STRING, &&__STRING, &&__POINTER, &&__VARIANT, &&__FUNCTION, &&__CLASS, &&__NULL
    };

  VALUE conv;
  char *addr;
  int len;

  //*more = FALSE;

  if (_level >= 4)
  {
  	fprintf(_where, "...");
  	return;
	}

	_level++;

__CONV:

  if (TYPE_is_object(value->type))
    goto __OBJECT;
  else
    goto *jump[value->type];

__NULL:

	fprintf(_where, "NULL");
  goto __RETURN;

__BOOLEAN:

	fprintf(_where, value->_boolean.value ? "TRUE" : "FALSE");
  goto __RETURN;

__BYTE:
__SHORT:
__INTEGER:

  fprintf(_where, "%d", value->_integer.value);
  goto __RETURN;

__LONG:

  fprintf(_where, "%" PRId64, value->_long.value);
  goto __RETURN;

__DATE:

  GB_DEBUG.FormatDate(GB.SplitDate((GB_DATE *)value), LF_STANDARD, NULL, 0, &addr, &len);
  goto __PRINT;

__SINGLE:

  GB_DEBUG.FormatNumber(value->_single.value, LF_SHORT_NUMBER, NULL, 0, &addr, &len, TRUE);
  goto __PRINT;
	
__FLOAT:

  GB_DEBUG.FormatNumber(value->_float.value, LF_STANDARD, NULL, 0, &addr, &len, TRUE);
	
__PRINT:
	
  fprintf(_where, "%.*s", (int)len, addr);
  goto __RETURN;

__STRING:

	print_string(value->_string.addr + value->_string.start, value->_string.len);
  goto __RETURN;

__OBJECT:

  if (!value->_object.object)
    goto __NULL;

  //*more = !CLASS_is_native(OBJECT_class(value->_object.object));

	print_object(value->_object.object);
  goto __RETURN;

__VARIANT:

  conv = *value;
  value = &conv;
  GB.Conv((GB_VALUE *)value, (GB_TYPE)value->_variant.vtype);
  //VARIANT_undo(value);
  goto __CONV;

__VOID:

	fprintf(_where, "VOID");
  goto __RETURN;

__CLASS:

  {
    CLASS *class = value->_class.class;
    //*more = (!CLASS_is_native(class) && class->load->n_stat > 0);

    fprintf(_where, "(%s)", class->name);
    goto __RETURN;
  }

__POINTER:

  fprintf(_where, "(%p)", value->_pointer.value);
  goto __RETURN;

__FUNCTION:

  fprintf(_where, "FUNCTION");
  goto __RETURN;

__RETURN:
	_level--;
}


void PRINT_value(FILE *where, VALUE *value, bool format)
{
  char *pval;
  int lpval;

  if (format)
  {
  	_where = where;
  	_level = 0;
    print_value(value);
    //fputc('\n', _where);
	}
  else
  {
    GB_DEBUG.ToString((GB_VALUE *)value, &pval, &lpval);
    fwrite(pval, sizeof(char), lpval, where);
	}
}

void PRINT_symbol(FILE *where, const char *sym, int len)
{
	GB_VALUE value;
	
	_where = where;
	
	if (GB_DEBUG.GetValue(sym, len, (GB_VARIANT *)(void *)&value))
	{
		fprintf(_where, "Unknown symbol");
		return;
	}
	
	print_value((VALUE *)&value);
}

static void print_key(char *key, int len)
{
	fprintf(_where, " ");
	print_string(key, len);
}

void PRINT_object(FILE *where, VALUE *value)
{
	VALUE conv;
	void *object;
	int index;
	int count = 0;
	CLASS *class, *real_class;
	CLASS_DESC_SYMBOL *cd;
	char *key;
	int len;
	bool static_class;
	int access;
	int *dim;
	
	_where = where;
	
	if (value->type == T_VARIANT)
	{
	  conv = *value;
  	value = &conv;
  	GB.Conv((GB_VALUE *)value, (GB_TYPE)value->_variant.vtype);
	}
	
	if (value->type < T_OBJECT && value->type != T_CLASS)
	{
		//fprintf(_where, "\n");
		return;
	}
		
	real_class = NULL;
	
	if (value->type == T_CLASS)
	{
		object = value->_class.class;
		class = (CLASS *)object;
		static_class = TRUE;
		real_class = NULL;
	}
	else
	{
		object = value->_object.object;
		real_class = (CLASS *)GB.GetClass(object);
		if (value->type == T_OBJECT)
			class = real_class;
		else
			class = value->_object.class;
			
		if (real_class == class)
			real_class = NULL;
			
		/*if (value->type != T_OBJECT && value->_object.class->is_virtual)
		{
		}
		else
			class = (CLASS *)GB.GetClass(object);*/
		//fprintf(stderr, "PRINT_object: %s %s\n", class->name, value->_object.class->name);
		static_class = FALSE;
	}
		
	if (!object)
		return;
		
	fprintf(_where, "%s ", class->name);
	if (real_class) fprintf(_where, "%s", real_class->name);
	fputc(' ', _where);
	
	access = GB_DEBUG.GetObjectAccessType(object, class, &count);
	
	if (access == GB_DEBUG_ACCESS_COLLECTION)
	{
		fprintf(_where, "C: [%d]", count);
		
		GB_DEBUG.EnumKeys(object, print_key);
		fprintf(_where, " ");
	}
	else if (GB.Is(object, GB.FindClass("Array")))
	{
		dim = GB_DEBUG.GetArrayBounds(object);
		if (!dim)
			fprintf(_where, "A: [%d] ", count);
		else
		{
			fprintf(_where, "A: [");
			for(;;)
			{
				len = *dim++;
				if (len > 0)
					fprintf(_where, "%d,", len);
				else
				{
					fprintf(_where, "%d", -len);
					break;
				}
			}
			fprintf(_where, "] ");
		}
	}
	else if (access == GB_DEBUG_ACCESS_ARRAY)
	{
		fprintf(_where, "A: [%d] ", count);
	}
	
	if (!class->is_virtual && real_class)
		class = real_class;
	
	fprintf(_where, "S:");
	
	index = 0;
	
	for(;;)
	{
		cd = (CLASS_DESC_SYMBOL *)GB_DEBUG.GetNextSortedSymbol(class, &index);
		if (!cd)
			break;
		key = cd->name;
		if (cd->len == 0 || (cd->len == 1 && key[0] == '.'))
			continue;
	
	  switch(CLASS_DESC_get_type(cd->desc))
		{
			case CD_STATIC_VARIABLE:
			case CD_STATIC_PROPERTY:
			case CD_STATIC_PROPERTY_READ:
			case CD_CONSTANT:
				fprintf(_where, " %.*s", cd->len, key);
				break;
		}
	}
	
	if (count > 0 && static_class)
		fprintf(_where, " [%d]", count);
	
	fprintf(_where, " D:");
	
	if (!static_class)
	{	
		index = 0;
		
		for(;;)
		{
			cd = (CLASS_DESC_SYMBOL *)GB_DEBUG.GetNextSortedSymbol(class, &index);
			if (!cd)
				break;
			key = cd->name;
			if (cd->len == 0 || (cd->len == 1 && key[0] == '.'))
				continue;
		
			switch(CLASS_DESC_get_type(cd->desc))
			{
				case CD_VARIABLE:
				case CD_PROPERTY:
				case CD_PROPERTY_READ:
				case CD_STRUCT_FIELD:
					fprintf(_where, " %.*s", cd->len, key);
					break;
			}
		}
		
		if (count > 0)
			fprintf(_where, " [%d]", count);
	}
	
	//fprintf(_where, "\n");
}
