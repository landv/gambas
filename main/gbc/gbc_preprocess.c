/***************************************************************************

	gbc_preprocess.c

	(c) 2000-2009 Benoît Minisini <gambas@users.sourceforge.net>

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
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

***************************************************************************/

#define __GBC_PREPROCESS_C

#include "gb_common.h"
#include "gb_common_case.h"
#include "gb_error.h"
#include "gb_table.h"

#include "gbc_compile.h"
#include "gbc_read.h"
#include "gbc_preprocess.h"

typedef
	struct {
		bool ignore;
		int ignore_level;
	}
	PREP_STATE;

#define MAX_LEVEL 16
	
static PREP_STATE _stack[MAX_LEVEL];

static int _level;
static bool _ignore;
static int _ignore_level;
static PATTERN *_current;


static int get_expression(void);

static bool is_current(int res)
{
	if (PATTERN_is(*_current, res))
	{
		_current++;
		return TRUE;
	}
	else
		return FALSE;
}

static bool compare_version(void)
{
	PATTERN op;
	SYMBOL *sym;
	char version[8];
	int n, major, minor;
	int diff;
	
	op = *_current++;
	if (!PATTERN_is_reserved(op))
		THROW("Missing operator");
	
	if (!PATTERN_is_string(*_current))
		THROW("Version string expected");
	
	sym = TABLE_get_symbol(JOB->class->string, PATTERN_index(*_current));
	_current++;
	
	if (sym->len < 1 || sym->len > 8)
		THROW("Bad version string");
	
	memcpy(version, sym->name, sym->len);
	version[sym->len] = 0;
	n = sscanf(version, "%d.%d", &major, &minor);
	if (n == 0)
		THROW("Bad version string");
	else if (n == 1)
		minor = 0;
	
	diff = GAMBAS_VERSION - major;
	if (!diff) 
		diff = GAMBAS_MINOR_VERSION - minor;
	
	switch (PATTERN_index(op))
	{
		case RS_EQUAL: return diff == 0;
		case RS_NE: return diff != 0;
		case RS_GT: return diff > 0;
		case RS_LT: return diff < 0;
		case RS_GE: return diff >= 0;
		case RS_LE: return diff <= 0;
		default: THROW("Comparison operator expected");
	}
}

#define compare_symbol(_symbol, _len_symbol, _name, _len) (((_len_symbol) == (_len)) && !strncasecmp((_symbol), (_name), (_len)))

static int get_symbol(const char *name, int len)
{
	if (compare_symbol("os_linux", 8, name, len))
	{
		#if OS_LINUX
	    return TRUE;
		#else
			return FALSE;
		#endif
	}

	if (compare_symbol("os_bsd", 6, name, len))
	{
		#if OS_BSD
	    return TRUE;
		#else
			return FALSE;
		#endif
	}

	if (compare_symbol("os_freebsd", 10, name, len))
	{
		#if OS_FREEBSD
	    return TRUE;
		#else
			return FALSE;
		#endif
	}

	if (compare_symbol("os_netbsd", 9, name, len))
	{
		#if OS_NETBSD
	    return TRUE;
		#else
			return FALSE;
		#endif
	}

	if (compare_symbol("os_macosx", 9, name, len))
	{
		#if OS_MACOSX
	    return TRUE;
		#else
			return FALSE;
		#endif
	}

	if (compare_symbol("arch_x86", 8, name, len))
	{
		#if ARCH_X86
	    return TRUE;
		#else
			return FALSE;
		#endif
	}

	if (compare_symbol("arch_x86_64", 11, name, len))
	{
		#if ARCH_X86_64
	    return TRUE;
		#else
			return FALSE;
		#endif
	}

	if (compare_symbol("arch_ppc", 8, name, len))
	{
		#if ARCH_PPC
	    return TRUE;
		#else
			return FALSE;
		#endif
	}

	if (compare_symbol("arch_arm", 8, name, len))
	{
		#if ARCH_ARM
	    return TRUE;
		#else
			return FALSE;
		#endif
	}

	if (compare_symbol("version", 7, name, len) || compare_symbol("gambas", 6, name, len) )
		return compare_version();

	return FALSE;
}

static int get_value(void)
{
	int value;
	SYMBOL *sym;
	
	if (is_current(RS_LBRA))
	{
		value = get_expression();
		if (!is_current(RS_RBRA))
			THROW("Missing right brace");
		return value;
	}
	
	if (is_current(RS_NOT))
		return !get_value();
	
	if (PATTERN_is_identifier(*_current))
	{
		sym = TABLE_get_symbol(JOB->class->table, PATTERN_index(*_current));
		_current++;
		value = get_symbol(sym->name, sym->len);
		return value;
	}
	
	return 0;
}

static int get_expression(void)
{
	int value;
	
	for(;;)
	{
		value = get_value();
	
		if (is_current(RS_AND))
		{
			if (!value)
				return FALSE;
			
			continue;
		}
		
		if (is_current(RS_OR))
		{
			if (value)
				return TRUE;
			
			continue;
		}
		
		if (PATTERN_is_newline(*_current) || PATTERN_is(*_current, RS_RBRA))
			return value;
		
		THROW(E_SYNTAX);
	}
}

void PREP_init(void)
{
	_level = 0;
	_ignore = FALSE;
	_current = NULL;
}

void PREP_exit(void)
{
	if (_level)
		THROW("Missing #Endif");
}

bool PREP_analyze(PATTERN *line)
{
	bool test;
	
	if (PATTERN_is(*line, RS_P_IF))
	{
		if (_level >= MAX_LEVEL)
			THROW("Too many imbricated #If...#Endif");
		
		_stack[_level].ignore = _ignore;
		_stack[_level].ignore_level = _ignore_level;
		
		line++;
		_level++;
		
		if (!_ignore)
		{
			_current = line;
			if (!get_expression())
			{
				_ignore = TRUE;
				_ignore_level = _level;
			}
		}
	}
	else if (PATTERN_is(*line, RS_P_ELSE))
	{
		bool else_if = FALSE;
		
		if (!_level)
			THROW_UNEXPECTED(line);
		
		test = TRUE;
		line++;
		
		if (!_ignore)
		{
			_ignore = TRUE;
			_ignore_level = _level - 1; 
		}
		else
		{
			if (_level == _ignore_level)
			{
				if (PATTERN_is(*line, RS_IF))
				{
					line++;
					
					_current = line;
					test = get_expression();
					
					else_if = TRUE;
				}
			}
			
			if (_level == _ignore_level)
				_ignore = !test;
		}
	}
	else if (PATTERN_is(*line, RS_P_ENDIF))
	{
		if (!_level)
			THROW_UNEXPECTED(line);
		
		_level--;
		_ignore = _stack[_level].ignore;
		_ignore_level = _stack[_level].ignore_level;
	}
	else
		THROW(E_SYNTAX);
	
	return _ignore;
}
