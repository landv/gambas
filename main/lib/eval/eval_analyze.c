/***************************************************************************

	eval_analyze.c

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

#define __EVAL_ANALYZE_C

#include "gambas.h"
#include "gb_common.h"
#include "eval_analyze.h"

#include "CSystem.h"
/*#define DEBUG*/

static char _analyze_buffer[256];
static int _analyze_buffer_pos;

static EVAL_COLOR colors[EVAL_MAX_COLOR];
static int colors_len;

static int get_type(PATTERN *pattern)
{
	int type = PATTERN_type(*pattern);
	int index = PATTERN_index(*pattern);

	if (type == EVAL_TYPE_RESERVED)
	{
		if (index >= RS_COLON)
		{
			if (!((index == RS_AND || index == RS_OR) && PATTERN_is(pattern[1], RS_IF)))
				type = EVAL_TYPE_OPERATOR;
		}
		else if (RES_is_type(index))
			type = EVAL_TYPE_DATATYPE;
		else if (index == RS_WITH && pattern > EVAL->pattern)
		{
			index = PATTERN_index(pattern[-1]);
			if (index == RS_BEGINS || index == RS_ENDS)
				type = EVAL_TYPE_OPERATOR;
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
	static char keyword[16];
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
		case EVAL_TYPE_ERROR:
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
	if (*len > EVAL_COLOR_MAX_LEN)
		*len = EVAL_COLOR_MAX_LEN;
}


static void add_data(int state, int len)
{
	EVAL_COLOR *color;

	if (colors_len >= EVAL_MAX_COLOR || len == 0)
		return;

	//printf("[%d] %d %d\n", colors_len, state, len);

	color = &colors[colors_len];
	color->state = state;
	color->len = len;
	color->alternate = FALSE;
	colors_len++;
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
		flush_result(result);
	
	if (len > sizeof(_analyze_buffer))
		result->str = GB.AddString(result->str, str, len);
	else
	{
		memcpy(&_analyze_buffer[_analyze_buffer_pos], str, len);
		_analyze_buffer_pos += len;
	}
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
	bool space_before, space_after;
	int len, i;
	bool preprocessor;

	pattern = EVAL->pattern;
	colors_len = 0;
	nspace = 0;
	preprocessor = FALSE;

	if (EVAL->len <= 0)
		return;

	if (!EVAL->comment)
	{
		nspace = get_indent(&empty);
		add_data(EVAL_TYPE_END, nspace);
	}

	if (empty)
		return;

	if (!pattern)
		return;

	init_result();
	
	if (nspace)
		add_result(result, EVAL->source, nspace);

	type = EVAL->comment ? EVAL_TYPE_COMMENT : EVAL_TYPE_END;
	next_type = EVAL_TYPE_END;
	space_after = FALSE;

	for(;;)
	{
		old_type = next_type;
		type = get_type(pattern);
		next_type = type;
		get_symbol(*pattern, &symbol, &len);

		space_before = space_after;
		space_after = FALSE;

		if (type == EVAL_TYPE_END)
			break;

		//if (in_quote && (type == EVAL_TYPE_RESERVED || type == EVAL_TYPE_DATATYPE || type == EVAL_TYPE_SUBR))
		//	type = EVAL_TYPE_IDENTIFIER;

		switch(type)
		{
			case EVAL_TYPE_RESERVED:
				//state = Keyword;
				//if (old_type != EVAL_TYPE_OPERATOR)
				//me = is_me_last(*pattern);
				
				if (is_me_last_kind(*pattern))
				{
					if (old_type != EVAL_TYPE_OPERATOR)
						space_before = TRUE;
					next_type = EVAL_TYPE_IDENTIFIER;
				}
				else if (is_optional_kind(*pattern))
				{
					if (old_type != EVAL_TYPE_OPERATOR)
						space_before = TRUE;
				}
				else
					space_before = TRUE;
				
				/*if (!is_me_last_kind(*pattern))
					space_before = TRUE;
				else
				{
					if (*pattern != RS_OPTIONAL)
					if (old_type != EVAL_TYPE_OPERATOR)
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

			case EVAL_TYPE_DATATYPE:
				//state = Datatype;
				if (PATTERN_is(pattern[-1], RS_OPEN))
					type = EVAL_TYPE_RESERVED;

				if (old_type != EVAL_TYPE_OPERATOR)
					space_before = TRUE;
				
				break;

			case EVAL_TYPE_IDENTIFIER:
			case EVAL_TYPE_CLASS:
				//state = Symbol;
				if (old_type != EVAL_TYPE_OPERATOR)
					space_before = TRUE;
				break;

			case EVAL_TYPE_NUMBER:
				//state = Number;
				if (old_type != EVAL_TYPE_OPERATOR)
					space_before = TRUE;
				break;

			case EVAL_TYPE_STRING:
				//state = String;
				if (old_type != EVAL_TYPE_OPERATOR)
					space_before = TRUE;
				break;

			case EVAL_TYPE_SUBR:
				//state = Subr;
				if (old_type != EVAL_TYPE_OPERATOR)
					space_before = TRUE;
				break;

			case EVAL_TYPE_COMMENT:
				//state = Commentary;
				space_before = *symbol != ' ';
				i = get_symbol_indent(symbol, len);
				if (i <= (len - 2) && symbol[i + 1] == '\'')
					type = EVAL_TYPE_HELP;
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
							type = EVAL_TYPE_HELP;
					}
				}
				break;

			case EVAL_TYPE_OPERATOR:

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
					if (old_type != EVAL_TYPE_OPERATOR)
						space_before = TRUE;
					space_after = FALSE;
					//in_quote = *symbol == '{';
					
					if (!preprocessor && *symbol == '#' && old_type == EVAL_TYPE_END)
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
				else if (PATTERN_is(*pattern, RS_NOT) || *symbol == '-')
				{
					if (old_type == EVAL_TYPE_OPERATOR && (PATTERN_is(pattern[-1], RS_LBRA) || PATTERN_is(pattern[-1],RS_LSQR)))
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

				if (old_type == EVAL_TYPE_RESERVED)
					space_before = TRUE;

				break;

			case EVAL_TYPE_ERROR:
				space_before = TRUE;
				break;
		}

		if (space_before && old_type != EVAL_TYPE_END)
		{
			add_result_char(result, ' ');
			add_data(preprocessor ? EVAL_TYPE_PREPROCESSOR : EVAL_TYPE_END, 1);
		}

		if (type == EVAL_TYPE_STRING)
			add_result_char(result, '"');

		if (len)
		{
			if (EVAL->rewrite && type == EVAL_TYPE_CLASS)
			{
				add_result_char(result, toupper(symbol[0]));
				if (len > 1) add_result(result, &symbol[1], len - 1);
			}
			else
				add_result(result, symbol, len);
			//printf("add: %.*s\n", len, symbol);
			len = get_utf8_length(symbol, len);
		}

		if (type == EVAL_TYPE_STRING)
		{
			add_result_char(result, '"');
			len += 2;
		}

		if (preprocessor && type != EVAL_TYPE_COMMENT && type != EVAL_TYPE_HELP)
			add_data(EVAL_TYPE_PREPROCESSOR, len);
		else
			add_data(type, len);
		//printf("add_data: %.d (%d)\n", type, len);

		pattern++;
	}

	flush_result(result);

	result->color = colors;
	result->len = colors_len;

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
		TABLE_add_symbol(EVAL->string, READ_source_ptr, len, &index);
		add_pattern(EVAL_TYPE_ERROR, index);
	}
	
	add_pattern(EVAL_TYPE_END, 0);
	//get_symbol(PATTERN_make(EVAL_TYPE_ERROR, index), &sym, &len);
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
		EVAL->comment = state == EVAL_TYPE_COMMENT;
		
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
		result->state = EVAL->comment ? EVAL_TYPE_COMMENT : EVAL_TYPE_END;

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



