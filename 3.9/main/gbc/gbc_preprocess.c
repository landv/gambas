/***************************************************************************

  gbc_preprocess.c

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

#define __GBC_PREPROCESS_C

#include "gb_common.h"
#include "gb_common_case.h"
#include "gb_error.h"
#include "gb_table.h"

#include "gbc_compile.h"
#include "gbc_read.h"
#include "gbc_trans.h"
#include "gbc_preprocess.h"

typedef
	struct {
		bool ignore;
		int ignore_level;
	}
	PREP_STATE;

#define MAX_LEVEL 16

int PREP_next_line;
	
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

static bool compare_value(const char *value)
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
		THROW("String expected");
	
	sym = TABLE_get_symbol(JOB->class->string, PATTERN_index(*_current));
	_current++;
	
	if (value)
	{
		if (strlen(value) != sym->len)
			diff = 1;
		else
			diff = strncasecmp(value, sym->name, sym->len);
		
		switch (PATTERN_index(op))
		{
			case RS_EQUAL: return diff == 0;
			case RS_NE: return diff != 0;
			default: THROW("Equality or inequality operator expected");
		}
	}
	else
	{
		if (sym->len < 1 || sym->len >= sizeof(version))
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
}

#define compare_symbol(_symbol, _name, _len) ((strlen(_symbol) == _len) && !strncasecmp((_symbol), (_name), (_len)))

static int get_symbol(const char *name, int len)
{
	if (compare_symbol("system", name, len))
		return compare_value(SYSTEM);

	if (compare_symbol("arch", name, len) || compare_symbol("architecture", name, len))
		return compare_value(ARCHITECTURE);

	if (compare_symbol("version", name, len) || compare_symbol("gambas", name, len))
		return compare_value(NULL);
	
	/*if (compare_symbol("debug", name, len))
		return JOB->debug;*/
	
	/*if (compare_symbol("true", name, len))
		return TRUE;

	if (compare_symbol("false", name, len))
		return FALSE;*/

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
	else if (is_current(RS_FALSE))
		return 0;
	else if (is_current(RS_TRUE))
		return 1;
	else if (is_current(RS_DEBUG))
		return JOB->debug;
	else if (is_current(RS_EXEC))
		return JOB->exec;
	
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

int PREP_analyze(PATTERN *line)
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
		//bool else_if = FALSE;
		
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
					
					//else_if = TRUE;
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
	else if (PATTERN_is(*line, RS_P_LINE))
	{
		TRANS_NUMBER result;
		
		line++;
		if (!PATTERN_is_number(*line))
			THROW_UNEXPECTED(line);
		
		TRANS_get_number(PATTERN_index(*line), &result);
		if (result.type != T_INTEGER)
			THROW_UNEXPECTED(line);
		
		PREP_next_line = result.ival - 1;
		return PREP_LINE;
	}
	else if (PATTERN_is(*line, RS_P_CONST))
	{
		// TODO
	}
	else
		THROW(E_SYNTAX);
	
	return _ignore ? PREP_IGNORE : PREP_CONTINUE;
}
