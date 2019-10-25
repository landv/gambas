/***************************************************************************

	eval_analyze.c

	(c) 2000-2017 Benoît Minisini <g4mba5@gmail.com>

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

#define __EVAL_ANALYZE_C

#include "gambas.h"
#include "gb_common.h"
#include "gb_array.h"
#include "eval_analyze.h"

#include "c_system.h"
/*#define DEBUG*/

static const uchar _utf8_char_length[256] =
{
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
	3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,4,4,4,4,4,4,4,4,5,5,5,5,6,6,1,1
};

static char _analyze_buffer[256];
static int _analyze_buffer_pos;

#define COLOR_BUFFER_SIZE 256
static EVAL_COLOR _colors[COLOR_BUFFER_SIZE];
static int _colors_len = 0;
static EVAL_COLOR *_color_buffer = NULL;

#define NEXT_UTF8_CHAR(_p) (_p += _utf8_char_length[(uchar)*(_p)])

static int get_type(PATTERN *pattern)
{
	int type = PATTERN_type(*pattern);
	int index = PATTERN_index(*pattern);

	if (type == RT_RESERVED)
	{
		if (index >= RS_COLON)
		{
			if (!((index == RS_AND || index == RS_OR) && PATTERN_is(pattern[1], RS_IF)))
				type = RT_OPERATOR;
		}
		else if (RES_is_type(index))
			type = RT_DATATYPE;
		else if (index == RS_WITH && pattern > EVAL->pattern)
		{
			index = PATTERN_index(pattern[-1]);
			if (index == RS_BEGINS || index == RS_ENDS)
				type = RT_OPERATOR;
		}
	}

	return type;
}

static int is_me_last_kind(PATTERN pattern)
{
	return PATTERN_is(pattern, RS_ME)
				|| PATTERN_is(pattern, RS_SUPER)
				|| PATTERN_is(pattern, RS_LAST)
				|| PATTERN_is(pattern, RS_TRUE)
				|| PATTERN_is(pattern, RS_FALSE)
				|| PATTERN_is(pattern, RS_PINF)
				|| PATTERN_is(pattern, RS_MINF)
				|| PATTERN_is(pattern, RS_NULL);
}

static int is_optional_kind(PATTERN pattern)
{
	return PATTERN_is(pattern, RS_OPTIONAL)
				|| PATTERN_is(pattern, RS_BYREF);
}

static void get_symbol(PATTERN pattern, const char **symbol, int *len)
{
	static char keyword[32];
	int i;
	SYMBOL *sym;
	int type = PATTERN_type(pattern);
	int index = PATTERN_index(pattern);

	switch(type)
	{
		case RT_RESERVED:
			*symbol = COMP_res_info[index].name;
			*len = strlen(*symbol);
			if (!EVAL->rewrite)
			{
				memcpy(keyword, *symbol, *len);
				for (i = 0; i < *len; i++)
					keyword[i] = toupper(keyword[i]);
				*symbol = keyword;
			}
			return;
			
		case RT_NUMBER:
		case RT_IDENTIFIER:
			sym = TABLE_get_symbol(EVAL->table, index);
			break;
			
		case RT_CLASS:
			sym = TABLE_get_symbol(EVAL->table, index);
			break;
			
		case RT_STRING:
		case RT_TSTRING:
		case RT_COMMENT:
		case RT_ERROR:
			sym = TABLE_get_symbol(EVAL->string, index);
			break;
			
		case RT_SUBR:
			*symbol = COMP_subr_info[index].name;
			*len = strlen(*symbol);
			return;
			
		default:
			*symbol = NULL;
			*len = 0;
			return;
	}

	*symbol = sym->name;
	*len = sym->len;
	/*if (*len > EVAL_COLOR_MAX_LEN)
		*len = EVAL_COLOR_MAX_LEN;*/
}


static void add_data(int state, int len)
{
	EVAL_COLOR *color;

	while (len > EVAL_COLOR_MAX_LEN)
	{
		add_data(state, EVAL_COLOR_MAX_LEN);
		len -= EVAL_COLOR_MAX_LEN;
	}
	
	if (len == 0)
		return;
	
	if (_colors_len >= COLOR_BUFFER_SIZE)
	{
		if (!_color_buffer)
			ARRAY_create_inc(&_color_buffer, COLOR_BUFFER_SIZE);
		
		color = ARRAY_add_many(&_color_buffer, COLOR_BUFFER_SIZE);
		memcpy(color, _colors, sizeof(EVAL_COLOR) * COLOR_BUFFER_SIZE);
		_colors_len = 0;
	}
	
	color = &_colors[_colors_len];
	color->state = state;
	color->len = len;
	color->alternate = FALSE;
	_colors_len++;
}

static void add_data_merge(int state, int len)
{
	if (_colors_len > 0 && _colors[_colors_len - 1].state == state && (_colors[_colors_len - 1].len + len) <= EVAL_COLOR_MAX_LEN)
	  _colors[_colors_len - 1].len += len;
	else
		add_data(state, len);
}

static void flush_colors(EVAL_ANALYZE *result)
{
	EVAL_COLOR *color;
	
	if (_color_buffer)
	{
		if (_colors_len)
		{
			color = ARRAY_add_many(&_color_buffer, _colors_len);
			memcpy(color, _colors, sizeof(EVAL_COLOR) * _colors_len);
		}
		
		result->color = _color_buffer;
		result->len = ARRAY_count(_color_buffer);
	}
	else
	{
		result->color = _colors;
		result->len = _colors_len;
	}
}

static int is_proc(void)
{
	PATTERN pattern;
	int i;

	if (!EVAL->pattern)
		return FALSE;

	for (i = 0;; i++)
	{
		pattern = EVAL->pattern[i];
		if (PATTERN_is_end(pattern))
			return FALSE;

		if (PATTERN_is(pattern, RS_PRIVATE) || PATTERN_is(pattern, RS_PUBLIC) || PATTERN_is(pattern, RS_STATIC) || PATTERN_is(pattern, RS_FAST))
			continue;

		return (PATTERN_is(pattern, RS_SUB) || PATTERN_is(pattern, RS_PROCEDURE) || PATTERN_is(pattern, RS_FUNCTION));
	}
}

static int get_indent(bool *empty)
{
	int i;
	unsigned char c;

	*empty = TRUE;

	for (i = 0; i < (int)EVAL->len; i++)
	{
		c = EVAL->source[i];
		if (c > ' ')
		{
			*empty = FALSE;
			break;
		}
	}

	return i;
}

static int get_symbol_indent(const char *symbol, int len)
{
	int i;
	
	for (i = 0; i < len; i++)
	{
		if (!(symbol[i] > 0 && symbol[i] < 33))
			return i;
	}
	
	return len;
}

static bool symbol_starts_with(const char *symbol, int len, int from, const char *comp)
{
	int l = strlen(comp);
	return (from < (len - l) && strncmp(&symbol[from], comp, l) == 0);
}


static int get_utf8_length(const char *s, int l)
{
	int len;
	int i;

	for (i = 0, len = 0; i < l; i++)
	{
		if ((s[i] & 0xC0) != 0x80)
			len++;
	}

	return len;
}

static void init_result()
{
	_analyze_buffer_pos = 0;
}

static void flush_result(EVAL_ANALYZE *result)
{
	if (_analyze_buffer_pos > 0)
	{
		result->str = GB.AddString(result->str, _analyze_buffer, _analyze_buffer_pos);
		_analyze_buffer_pos = 0;
	}
}

static void add_result(EVAL_ANALYZE *result, const char *str, int len)
{
	if ((_analyze_buffer_pos + len) > sizeof(_analyze_buffer))
	{
		flush_result(result);
		if (len >= sizeof(_analyze_buffer))
		{
			result->str = GB.AddString(result->str, str, len);
			return;
		}
	}
	
	memcpy(&_analyze_buffer[_analyze_buffer_pos], str, len);
	_analyze_buffer_pos += len;
}

static void add_result_char(EVAL_ANALYZE *result, char c)
{
	if ((_analyze_buffer_pos + 1) > sizeof(_analyze_buffer))
		flush_result(result);
	
	_analyze_buffer[_analyze_buffer_pos++] = c;
}

static void analyze(EVAL_ANALYZE *result)
{
	PATTERN *pattern;
	int nspace;
	bool empty = FALSE;
	int type, old_type, next_type;
	const char *symbol;
	const char *p;
	bool space_before, space_after;
	int len, i, l;
	bool preprocessor;

	_colors_len = 0;
	EVAL_analyze_exit();
	
	pattern = EVAL->pattern;
	nspace = 0;
	preprocessor = FALSE;

	if (EVAL->len <= 0)
		return;

	if (!EVAL->comment)
	{
		nspace = get_indent(&empty);
		add_data(RT_END, nspace);
	}

	if (empty)
		return;

	if (!pattern)
		return;

	init_result();
	
	if (nspace)
		add_result(result, EVAL->source, nspace);

	type = EVAL->comment ? RT_COMMENT : RT_END;
	next_type = RT_END;
	old_type = RT_END;
	space_after = FALSE;

	for(;;)
	{
		old_type = next_type;
		type = get_type(pattern);
		next_type = type;
		get_symbol(*pattern, &symbol, &len);

		space_before = space_after;
		space_after = FALSE;

		if (type == RT_END)
			break;

		//if (in_quote && (type == RT_RESERVED || type == RT_DATATYPE || type == RT_SUBR))
		//	type = RT_IDENTIFIER;

		switch(type)
		{
			case RT_RESERVED:
				//state = Keyword;
				//if (old_type != RT_OPERATOR)
				//me = is_me_last(*pattern);
				
				if (is_me_last_kind(*pattern))
				{
					if (old_type != RT_OPERATOR)
						space_before = TRUE;
					next_type = RT_IDENTIFIER;
				}
				else if (is_optional_kind(*pattern))
				{
					if (old_type != RT_OPERATOR)
						space_before = TRUE;
				}
				else
					space_before = TRUE;
				
				/*if (!is_me_last_kind(*pattern))
					space_before = TRUE;
				else
				{
					if (*pattern != RS_OPTIONAL)
					if (old_type != RT_OPERATOR)
						space_before = TRUE;
				}*/
				
				if (preprocessor && PATTERN_is(pattern[-1], RS_SHARP))
					space_before = FALSE;

				/*if (PATTERN_index(*pattern) >= RS_COLON)
				{
					int i;

					for (i = 0; i < len; i++)
						usym[i] = GB.toupper(symbol[i]);
					usym[len] = 0;

					symbol = usym;
				}*/

				break;

			case RT_DATATYPE:
				//state = Datatype;
				if (PATTERN_is(pattern[-1], RS_OPEN))
					type = RT_RESERVED;

				if (old_type != RT_OPERATOR)
					space_before = TRUE;
				
				break;

			case RT_IDENTIFIER:
			case RT_CLASS:
				//state = Symbol;
				if (old_type != RT_OPERATOR)
					space_before = TRUE;
				break;

			case RT_NUMBER:
				//state = Number;
				if (old_type != RT_OPERATOR)
					space_before = TRUE;
				break;

			case RT_STRING:
				//state = String;
				if (old_type != RT_OPERATOR)
					space_before = TRUE;
				break;

			case RT_SUBR:
				//state = Subr;
				if (old_type != RT_OPERATOR)
					space_before = TRUE;
				break;

			case RT_COMMENT:
				//state = Commentary;
				space_before = *symbol != ' ';
				i = get_symbol_indent(symbol, len);
				if (i <= (len - 2) && symbol[i + 1] == '\'')
					type = RT_HELP;
				else
				{
					while (i < len && (uchar)symbol[i] == '\'')
						i++;
					
					while (i < len && (uchar)symbol[i] <= ' ')
						i++;
					
					if (i < len)
					{
						if (symbol_starts_with(symbol, len, i, "NOTE:")
								|| symbol_starts_with(symbol, len, i, "TODO:")
								|| symbol_starts_with(symbol, len, i, "FIXME:"))
							type = RT_HELP;
					}
				}
				break;

			case RT_OPERATOR:

				if (index("([)]@", *symbol))
				{
					space_after = FALSE;
				}
				else if (index(":;,", *symbol))
				{
					space_before = FALSE;
					space_after = TRUE;
				}
				else if (index("#{", *symbol))
				{
					if (old_type != RT_OPERATOR)
						space_before = TRUE;
					space_after = FALSE;
					//in_quote = *symbol == '{';
					
					if (!preprocessor && *symbol == '#' && old_type == RT_END)
						preprocessor = TRUE;
				}
				else if (index("}", *symbol))
				{
					space_before = FALSE;
					space_after = FALSE;
					//in_quote = FALSE;
				}
				else if (index(".!", *symbol)) //symbol[0] == '.' && symbol[1] == 0)
				{
					//space_before = FALSE;
					space_after = FALSE;
				}
				else if (*symbol == '-' && len == 1)
				{
					if (old_type == RT_OPERATOR && (PATTERN_is(pattern[-1], RS_LBRA) || PATTERN_is(pattern[-1],RS_LSQR)))
						space_before = FALSE;
					else
						space_before = TRUE;
					
					if (old_type == RT_RESERVED || old_type == RT_DATATYPE)
						space_after = FALSE;
					else if (old_type == RT_OPERATOR)
					{
						get_symbol(pattern[-1], &symbol, &len);
						if (index(")]}", *symbol))
							space_after = TRUE;
						else
							space_after = FALSE;
						get_symbol(*pattern, &symbol, &len);
					}
					else
						space_after = TRUE;
				}
				else if (PATTERN_is(*pattern, RS_NOT))
				{
					if (old_type == RT_OPERATOR && (PATTERN_is(pattern[-1], RS_LBRA) || PATTERN_is(pattern[-1],RS_LSQR)))
						space_before = FALSE;
					else
						space_before = TRUE;

					space_after = TRUE;
				}
				else
				{
					space_before = TRUE;
					space_after = TRUE;
				}

				if (old_type == RT_RESERVED)
					space_before = TRUE;

				break;

			case RT_ERROR:
				space_before = TRUE;
				break;
		}

		if (space_before && old_type != RT_END)
		{
			add_result_char(result, ' ');
			add_data(preprocessor ? RT_PREPROCESSOR : RT_END, 1);
		}

		if (type == RT_STRING)
			add_result_char(result, '"');

		if (len)
		{
			if (EVAL->rewrite && type == RT_CLASS)
			{
				add_result_char(result, toupper(symbol[0]));
				if (len > 1) add_result(result, &symbol[1], len - 1);
			}
			else
				add_result(result, symbol, len);
			//printf("add: %.*s\n", len, symbol);
			len = get_utf8_length(symbol, len);
		}

		if (type == RT_STRING)
		{
			add_result_char(result, '"');
			len += 2;
		}

		if (EVAL->rewrite)
		{
			if (type == RT_STRING)
			{
				add_data(RT_STRING, 1);
				len -= 2;
				for (i = 0, p = symbol; i < len; i++)
				{
					if (*p == '\\')
					{
						i++;
						NEXT_UTF8_CHAR(p);
						
						add_data_merge(RT_ESCAPE, 1);
						if (i < len)
						{
							if (*p == 'x' && i < (len - 2) && isxdigit(p[1]) && isxdigit(p[2]))
							{
								l = 3;
								i += 2;
							}
							else
								l = 1;
							add_data_merge(RT_ESCAPE, l);
							
							while (l--)
								NEXT_UTF8_CHAR(p);
						}
					}
					else
					{
						NEXT_UTF8_CHAR(p);
						add_data_merge(RT_STRING, 1);
					}
				}
				add_data_merge(RT_STRING, 1);
				goto __NEXT_PATTERN;
			}
			else if (type == RT_IDENTIFIER)
			{
				if (PATTERN_is(pattern[1], RS_COLON))
				{
					add_result_char(result, ':');
					add_data(RT_LABEL, len + 1);
					space_after = TRUE;
					pattern ++;
					goto __NEXT_PATTERN;
				}
				else if (old_type == RT_RESERVED && (PATTERN_is(pattern[-1], RS_GOTO) || PATTERN_is(pattern[-1], RS_GOSUB)))
				{
					type = RT_LABEL;
				}
			}
			else if (type == RT_RESERVED)
			{
				if (PATTERN_is(*pattern, RS_NULL)
						|| PATTERN_is(*pattern, RS_TRUE)
						|| PATTERN_is(*pattern, RS_FALSE)
						|| PATTERN_is(*pattern, RS_PINF)
						|| PATTERN_is(*pattern, RS_MINF))
				{
					type = RT_CONSTANT;
				}
			}
		}
		
		if (preprocessor && type != RT_COMMENT && type != RT_HELP)
			add_data(RT_PREPROCESSOR, len);
		else
			add_data(type, len);
		//printf("add_data: %.d (%d)\n", type, len);

	__NEXT_PATTERN:
		pattern++;
	}

	flush_result(result);
	flush_colors(result);

	//fprintf(stderr, "analyze: %d %s\n", strlen(result->str), result->str);
}


#define add_pattern(_type, _index) EVAL->pattern[EVAL->pattern_count++] = PATTERN_make((_type), (_index));

static void add_end_pattern(void)
{
	int index;
	int len;

	len = EVAL->len - (READ_source_ptr - EVAL->source);
	if (len > 0)
	{
		index = TABLE_add_symbol(EVAL->string, READ_source_ptr, len);
		add_pattern(RT_ERROR, index);
	}
	
	add_pattern(RT_END, 0);
	//get_symbol(PATTERN_make(RT_ERROR, index), &sym, &len);
}


PUBLIC void EVAL_analyze(const char *src, int len, int state, EVAL_ANALYZE *result, bool rewrite)
{
	int nspace = 0;

	#ifdef DEBUG
	printf("EVAL: %*.s\n", expr->len, expr->source);
	#endif

	CLEAR(result);

	while (len > 0 && src[len - 1] == ' ')
	{
		len--;
		nspace++;
	}

	result->len = 0;
	result->str = NULL;

	if (len > 0)
	{
		EVAL = &EVAL_read_expr;

		EVAL_clear(EVAL, FALSE);
		
		EVAL->source = GB.NewString(src, len);
		EVAL->source = GB.AddString(EVAL->source, "\0\0", 2);
		EVAL->len = len;
		
		EVAL->analyze = TRUE;
		EVAL->rewrite = rewrite;
		EVAL->comment = state == RT_COMMENT;
		
		//fprintf(stderr, "EVAL_analyze: [%d] %.*s\n", EVAL->comment, len, src);

		EVAL_start(EVAL);

		TRY
		{
			EVAL_read();
		}
		CATCH
		{
			add_end_pattern();
		}
		END_TRY

		analyze(result);
		result->proc = is_proc();
		result->state = EVAL->comment ? RT_COMMENT : RT_END;

		//fprintf(stderr, "--> [%d]\n", EVAL->comment);
		
		GB.FreeString(&EVAL->source);
	}
	else
	{
		result->proc = FALSE;
	}

	while (nspace > 0)
	{
		result->str = GB.AddString(result->str, "        ", nspace > 8 ? 8 : nspace);
		nspace -= 8;
	}
}


void EVAL_analyze_exit(void)
{
	if (_color_buffer)
		ARRAY_delete(&_color_buffer);
}
