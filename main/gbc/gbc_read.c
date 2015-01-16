/***************************************************************************

  gbc_read.c

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

#define __GBC_READ_C

#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <stdio.h>
#include <ctype.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <dirent.h>

#include "gb_common.h"
#include "gb_common_buffer.h"
#include "gb_common_case.h"
#include "gb_error.h"
#include "gb_table.h"
#include "gb_file.h"

#include "gbc_compile.h"
#include "gbc_class.h"
#include "gbc_preprocess.h"
#include "gbc_help.h"
#include "gbc_read.h"

//#define DEBUG
//#define BIG_COMMENT 1

static bool is_init = FALSE;
static COMPILE *comp;
static const char *source_ptr;
static int source_length;
static bool _begin_line = FALSE;
static bool _no_quote = FALSE;

static bool _prep = FALSE;
static int _prep_index;

static char ident_car[256];
static char first_car[256];
char READ_digit_car[256];
static char noop_car[256];
static char canres_car[256];

enum
{
	GOTO_BREAK, 
	GOTO_SPACE, 
	GOTO_COMMENT, 
	GOTO_STRING, 
	GOTO_IDENT, 
	GOTO_QUOTED_IDENT, 
	GOTO_NUMBER,
	GOTO_ERROR,
	GOTO_SHARP,
	GOTO_OTHER
};

static void READ_init(void)
{
	unsigned char i;
	
	JOB->line = 1;
	JOB->max_line = FORM_FIRST_LINE - 1;
	JOB->column = TRUE;
	
	if (!is_init)
	{
		for (i = 0; i < 255; i++)
		{
			ident_car[i] = (i != 0) && ((i >= 'A' && i <= 'Z') || (i >= 'a' && i <= 'z') || (i >= '0' && i <= '9') || strchr("$_?@", i));
			READ_digit_car[i] = (i >= '0' && i <= '9');
			noop_car[i] = ident_car[i] || READ_digit_car[i] || i <= ' ';
			canres_car[i] = (i != ':') && (i != '.') && (i != '!') && (i != '(');
			
			if (i == 0)
				first_car[i] = GOTO_BREAK;
			else if (i <= ' ')
				first_car[i] = GOTO_SPACE;
			else if (i == '\'')
				first_car[i] = GOTO_COMMENT;
			else if (i == '"')
				first_car[i] = GOTO_STRING;
			else if (i == '#')
				first_car[i] = GOTO_SHARP;
			else if ((i >= 'A' && i <= 'Z') || (i >= 'a' && i <= 'z') || i == '$' || i == '_')
				first_car[i] = GOTO_IDENT;
			else if (i == '{')
				first_car[i] = GOTO_QUOTED_IDENT;
			else if (i >= '0' && i <= '9')
				first_car[i] = GOTO_NUMBER;
			else if (i >= 127)
				first_car[i] = GOTO_ERROR;
			else
				first_car[i] = GOTO_OTHER;
		}
	
		is_init = TRUE;
	}
}


static void READ_exit(void)
{
	char *name = NULL;
	int len;
	int index;
	bool local;
	bool has_static;
	char c;

	local = FALSE;
	
	for(;;)
	{
		COMPILE_enum_class(&name, &len);
		if (!len)
			break;

		if (len == 1 && *name == '-')
		{
			local = TRUE;
		}
		else if (*name != '.')
		{
			has_static = FALSE;

			for(;;)
			{
				c = name[len - 1];
				if (c == '!')
				{
					has_static = TRUE;
					len--;
				}
				else if (c == '?')
					len--;
				else
					break;
			}

			if (TABLE_find_symbol(JOB->class->table, name, len, &index))
			{
				if (local)
					index = CLASS_add_class_unused(JOB->class, index);
				else
					index = CLASS_add_class_exported_unused(JOB->class, index);

				JOB->class->class[index].has_static = has_static;
			}
		}
	}

	if (JOB->verbose)
		printf("\n");
	
	JOB->column = FALSE;
}

static int get_utf8_length(const char *str, int len)
{
  int ulen = 0;
	int i;

  for (i = 0; i < len; i++)
  {
    if ((str[i] & 0xC0) != 0x80)
      ulen++;
  }

  return ulen;
}


int READ_get_column()
{
	const char *p = source_ptr;
	int col = 0;
	
	while (p > comp->source)
	{
		if (*p == '\n')
		{
			p++;
			break;
		}
		p--;
		col++;
	}
	
	return get_utf8_length(p, col + 1);
}


char *READ_get_pattern(PATTERN *pattern)
{
	int type = PATTERN_type(*pattern);
	int index = PATTERN_index(*pattern);
	const char *str;
	const char *before = _no_quote ? "" : "'";
	const char *after = _no_quote ? "" : "'";

	switch(type)
	{
		case RT_RESERVED:
			str = COMP_res_info[index].name; //TABLE_get_symbol_name(COMP_res_table, index);
			if (ispunct(*str))
				snprintf(COMMON_buffer, COMMON_BUF_MAX, "%s%s%s", before, str, after);
			else
				strcpy(COMMON_buffer, str);
			break;

		case RT_NUMBER:
		case RT_IDENTIFIER:
		case RT_CLASS:
			snprintf(COMMON_buffer, COMMON_BUF_MAX, "%s%s%s", before, TABLE_get_symbol_name(JOB->class->table, index), after);
			break;

		case RT_STRING:
		case RT_TSTRING:
			if (_no_quote)
				snprintf(COMMON_buffer, COMMON_BUF_MAX, "\"%s\"", TABLE_get_symbol_name(JOB->class->string, index));
			else
				strcpy(COMMON_buffer, "string");
			break;

		case RT_COMMAND:
			snprintf(COMMON_buffer, COMMON_BUF_MAX, "#%d", index);
			break;

		case RT_NEWLINE:
			strcpy(COMMON_buffer, "end of line");
			break;

		case RT_END:
			strcpy(COMMON_buffer, "end of file");
			break;

		case RT_SUBR:
			//snprintf(COMMON_buffer, COMMON_BUF_MAX, "%s%s%s", bafore, COMP_subr_info[index].name, after);
			strcpy(COMMON_buffer, COMP_subr_info[index].name);
			break;

		default:
			sprintf(COMMON_buffer, "%s?%02X.%02X.%d?%s", before, PATTERN_type(*pattern), PATTERN_flag(*pattern), (int)PATTERN_index(*pattern), after);
	}

	return COMMON_buffer;
}

void THROW_UNEXPECTED(PATTERN *pattern)
{
	switch (PATTERN_type(*pattern))
	{
		case RT_NEWLINE: case RT_END:
			THROW("Unexpected end of line");
		case RT_STRING: case RT_TSTRING:
			THROW("Unexpected string");
		default:
			THROW("Unexpected &1", READ_get_pattern(pattern));
	}
}

void READ_dump_pattern(PATTERN *pattern)
{
	int type = PATTERN_type(*pattern);
	int index = PATTERN_index(*pattern);

	/*pos = (int)(pattern - JOB->pattern);
	if (pos < 0 || pos >= JOB->pattern_count)
		return;
		
	printf("%d ", pos);*/

	if (PATTERN_flag(*pattern) & RT_FIRST)
		printf("!");
	else
		printf(" ");

	if (PATTERN_flag(*pattern) & RT_POINT)
		printf(".");
	else
		printf(" ");

	if (PATTERN_flag(*pattern) & RT_CLASS)
		printf("C");
	else
		printf(" ");

	printf(" ");

	_no_quote = TRUE;

	if (type == RT_RESERVED)
		printf("RESERVED     %s\n", READ_get_pattern(pattern));
	else if (type == RT_NUMBER)
		printf("NUMBER       %s\n", READ_get_pattern(pattern));
	else if (type == RT_IDENTIFIER)
		printf("IDENTIFIER   %s\n", READ_get_pattern(pattern));
	else if (type == RT_CLASS)
		printf("CLASS        %s\n", READ_get_pattern(pattern));
	else if (type == RT_STRING)
		printf("STRING       %s\n", READ_get_pattern(pattern));
	else if (type == RT_TSTRING)
		printf("TSTRING      %s\n", READ_get_pattern(pattern));
	else if (type == RT_NEWLINE)
		printf("NEWLINE      (%d)\n", index);
	else if (type == RT_END)
		printf("END\n");
	else if (type == RT_PARAM)
		printf("PARAM        %d\n", index);
	else if (type == RT_SUBR)
		printf("SUBR         %s\n", READ_get_pattern(pattern));
	else if (type == RT_COMMAND)
		printf("COMMAND      %d\n", index);
	else
		printf("?            %d\n", index);

	_no_quote = FALSE;
}


#if 0
static inline unsigned char get_char_offset(int offset)
{
	offset += source_ptr;

	if (offset >= source_length || offset < 0)
		return 0;
	else
		return (unsigned char)(comp->source[offset]);
}
#endif

#if 0
static unsigned char get_char(void)
{
	return get_char_offset(0);
}
#endif

#define get_char_offset(_offset) ((unsigned char)source_ptr[(_offset)])
#define get_char() ((unsigned char)(*source_ptr))


static unsigned char next_char(void)
{
	source_ptr++;
	return get_char();
}


#ifdef DEBUG

static void add_pattern(int type, int index)
{
	comp->pattern[comp->pattern_count++] = PATTERN_make(type, index);
	READ_dump_pattern(&comp->pattern[comp->pattern_count - 1]);
}

#define add_pattern_no_dump(_type, _index) comp->pattern[comp->pattern_count++] = PATTERN_make((_type), (_index));

#else

#define add_pattern(_type, _index) comp->pattern[comp->pattern_count++] = PATTERN_make((_type), (_index));
#define add_pattern_no_dump add_pattern

#endif

static PATTERN get_last_last_pattern()
{
	if (LIKELY(comp->pattern_count > 1))
		return comp->pattern[comp->pattern_count - 2];
	else
		return NULL_PATTERN;
}

#define get_last_pattern() (comp->pattern[comp->pattern_count - 1])

static void jump_to_next_prep(void)
{
	unsigned char car;
	const char *line_start;
	
	for (;;)
	{
		line_start = source_ptr;
		
		for(;;)
		{
			car = get_char();
			if (!car)
				return;
			if (car == '\n' || !car || !isspace(car))
				break;
			source_ptr++;
		}
		
		if (car == '#')
		{
			source_ptr = line_start;
			return;
		}
		
		for(;;)
		{
			car = get_char();
			if (!car)
				return;
			source_ptr++;
			if (car == '\n')
				break;
		}
		
		add_pattern(RT_NEWLINE, comp->line);
		comp->line++;
	}
}

static void add_newline()
{
	int action = PREP_CONTINUE;
	
	if (_prep)
	{
		int line = comp->line;
		
		add_pattern_no_dump(RT_NEWLINE, comp->line);
		action = PREP_analyze(&comp->pattern[_prep_index]);
		_prep = FALSE;
		
		comp->pattern_count = _prep_index;
		comp->line = line;
	}
	
	if (action == PREP_LINE)
		comp->line = PREP_next_line;
	
	// Void lines must act as void help comments
	if (comp->line > 0 && PATTERN_is_newline(get_last_pattern()))
		HELP_add_at_current_line("\n");
	
	add_pattern(RT_NEWLINE, comp->line);
	comp->line++;
	
	if (action == PREP_IGNORE)
		jump_to_next_prep();
}

static void add_end()
{
	add_pattern(RT_END, 0);
	comp->line++;
}


static bool add_number()
{
	unsigned char car;
	const char *start;
	int index;
	char sign;
	PATTERN last_pattern;
	bool has_digit;

	start = source_ptr;
	car = get_char();

	if (car == '-' || car == '+')
	{
		sign = car;
		car = next_char();
		
		if (car == 'I' || car == 'i')
		{
			car = next_char();
			if (car == 'N' || car == 'n')
			{
				car = next_char();
				if (car == 'F' || car == 'f')
				{
					car = next_char();
					add_pattern(RT_RESERVED, RESERVED_find_word(start, 4));
					return FALSE;
				}
			}
			
			goto NOT_A_NUMBER;
		}
	}
	else
		sign = 0;

	if (car == '&')
	{
		car = toupper(next_char());

		if (car == 'H')
			goto READ_HEXA;
		else if (car == 'X')
			goto READ_BINARY;
		else
		{
			source_ptr--;
			goto READ_HEXA;
		}
	}
	else if (car == '%')
		goto READ_BINARY;
	else if (isdigit(car))
		goto READ_NUMBER;
	else
		goto NOT_A_NUMBER;

READ_BINARY:

	has_digit = FALSE;
	for (;;)
	{
		car = next_char();
		if (UNLIKELY(car != '0' && car != '1'))
			break;
		has_digit = TRUE;
	}
	
	goto END_BINARY_HEXA;

READ_HEXA:

	has_digit = FALSE;
	for (;;)
	{
		car = next_char();
		if (UNLIKELY(!isxdigit(car)))
			break;
		has_digit = TRUE;
	}

END_BINARY_HEXA:

	if (!has_digit)
		goto NOT_A_NUMBER;

	if (car == '&')
		car = next_char();
	else if (first_car[car] == GOTO_IDENT)
		goto NOT_A_NUMBER;

	goto END;

READ_NUMBER:

	while (isdigit(car))
		car = next_char();

	if (car == '.')
	{
		do
		{
			car = next_char();
		}
		while (isdigit(car));
	}

	if (toupper(car) == 'E')
	{
		car = next_char();
		if (car == '+' || car == '-')
			car = next_char();

		while (isdigit(car))
			car = next_char();
	}
	else if (toupper(car) == 'I')
	{
		car = next_char();
	}

	goto END;

END:

	last_pattern = get_last_pattern();

	if (sign && !PATTERN_is_null(last_pattern) && (!PATTERN_is_reserved(last_pattern) || PATTERN_is(last_pattern, RS_RBRA) || PATTERN_is(last_pattern, RS_RSQR)))
	{
		add_pattern(RT_RESERVED, RESERVED_find_word(&sign, 1));
		TABLE_add_symbol(comp->class->table, start + 1, source_ptr - start - 1, &index);
		add_pattern(RT_NUMBER, index);
	}
	else
	{
		TABLE_add_symbol(comp->class->table, start, source_ptr - start, &index);
		add_pattern(RT_NUMBER, index);
	}
	
	return FALSE;
	
NOT_A_NUMBER:
	
	source_ptr = start;
	return TRUE;
}



static void add_identifier()
{
	unsigned char car;
	const char *start;
	int len;
	int index;
	int type;
	int flag;
	PATTERN last_pattern, last_last_pattern;
	bool not_first;
	bool can_be_reserved;
	bool last_identifier, last_type, last_class, last_pub;
	
	last_pattern = get_last_pattern();
	
	if (PATTERN_is_reserved(last_pattern))
	{
		flag = RES_get_ident_flag(PATTERN_index(last_pattern));
		if (flag & RSF_PREV)
		{
			last_last_pattern = get_last_last_pattern();
			if (PATTERN_is_reserved(last_last_pattern))
				flag = RES_get_ident_flag(PATTERN_index(last_last_pattern));
			else
				flag = 0;
		}
	}
	else
		flag = 0;

	type = RT_IDENTIFIER;

	start = source_ptr;
	len = 1;

	last_class = (flag & RSF_CLASS) != 0;
	last_type = (flag & RSF_AS) != 0;
	
	if (last_type)
	{
		for(;;)
		{
			source_ptr++;
			len++;
			car = get_char();
			if (ident_car[car])
				continue;
			if (car == '[')
			{
				car = get_char_offset(1);
				if (car == ']')
				{
					source_ptr++;
					len++;
					TABLE_add_symbol(comp->class->table, start, len - 2, &index);
					continue;
				}
			}
			
			len--;
			break;
		}
	}
	else
	{
		for(;;)
		{
			source_ptr++;
			car = get_char();
			if (!ident_car[car])
				break;
			len++;
		}
	}

	not_first = (flag & RSF_POINT) != 0;

	car = get_char();

	//can_be_reserved = !not_first && TABLE_find_symbol(COMP_res_table, &comp->source[start], len, NULL, &index);
	
	can_be_reserved = !not_first && !last_class;
	
	if (can_be_reserved)
	{
		index = RESERVED_find_word(start, len);
		can_be_reserved = (index >= 0);
	}
	
	if (can_be_reserved)
	{
		static void *jump[] = { 
			&&__OTHERS, &&__ME_NEW_LAST_SUPER, &&__CLASS, &&__STRUCT, &&__SUB_PROCEDURE_FUNCTION, &&__CONST_EXTERN, &&__ENUM, &&__READ, &&__DATATYPE 
		};
		
		last_identifier = (flag & RSF_IDENT) != 0;
		last_pub = (flag & RSF_PUB) != 0;

		goto *jump[RES_get_read_switch(index)];
		
		do
		{
		__ME_NEW_LAST_SUPER:
			can_be_reserved = !last_identifier;
			break;
			
		__CLASS:
			can_be_reserved = canres_car[car] && _begin_line;
			break;
			
		__STRUCT:
			can_be_reserved = canres_car[car] && (_begin_line || last_pub || PATTERN_is(last_pattern, RS_AS) || PATTERN_is(last_pattern, RS_END) || PATTERN_is(last_pattern, RS_NEW));
			break;
			
		__SUB_PROCEDURE_FUNCTION:
			can_be_reserved = canres_car[car] && (_begin_line || last_pub || PATTERN_is(last_pattern, RS_END));
			break;
		
		__CONST_EXTERN:
			can_be_reserved = canres_car[car] && (_begin_line || last_pub);
			break;
			
		__ENUM:
			can_be_reserved = canres_car[car] && (_begin_line || last_pub);
			break;
			
		__READ:
			can_be_reserved = canres_car[car] && (!last_identifier || PATTERN_is(last_pattern, RS_PROPERTY));
			break;
		
		__DATATYPE:
			if (car == '[' && get_char_offset(1) == ']')
			{
				len += 2;
				source_ptr += 2;
				can_be_reserved = FALSE;
			}
			else
			{
				if (last_type || PATTERN_is(last_pattern, RS_OPEN))
					can_be_reserved = TRUE;
				else
					can_be_reserved = FALSE;
			}
			break;
			
		__OTHERS:
			if (last_type || last_identifier || (PATTERN_is(last_pattern, RS_LBRA) && car == ')' && PATTERN_is_reserved(get_last_last_pattern())))
				can_be_reserved = FALSE;
			else
				can_be_reserved = canres_car[car];
			break;
		}
		while (0);
	}

	if (can_be_reserved)
	{
		type = RT_RESERVED;
		goto __ADD_PATTERN;
	}
	
	if ((flag == 0) && car != '.' && car != '!')
	{
		index = RESERVED_find_subr(start, len);
		if (index >= 0)
		{
			if (COMP_subr_info[index].min_param == 0 || car == '(')
			{
				type = RT_SUBR;
				goto __ADD_PATTERN;
			}
		}
	}

	if (last_type)
		type = RT_CLASS;

	if (flag & RSF_EVENT)
	{
		start--;
		len++;
		*((char *)start) = ':';
	}

	if (PATTERN_is(last_pattern, RS_EXCL))
	{
		TABLE_add_symbol(comp->class->string, start, len, &index);
		type = RT_STRING;
	}
	else
		TABLE_add_symbol(comp->class->table, start, len, &index);
	
__ADD_PATTERN:

	add_pattern(type, index);
}


static void add_quoted_identifier(void)
{
	unsigned char car;
	const char *start;
	int len;
	int index;
	int type;
	PATTERN last_pattern;
	
	last_pattern = get_last_pattern();

	type = RT_IDENTIFIER;

	start = source_ptr;
	len = 1;

	for(;;)
	{
		source_ptr++;
		car = get_char();
		if (!ident_car[car])
			break;
		len++;
	}

	if (get_char() == '}')
		source_ptr++;

	if (PATTERN_is(last_pattern, RS_EVENT) || PATTERN_is(last_pattern, RS_RAISE))
	{
		start--;
		len++;
		*((char *)start) = ':';
	}

	if (PATTERN_is(last_pattern, RS_EXCL))
	{
		TABLE_add_symbol(comp->class->string, start, len, &index);
		type = RT_STRING;
	}
	else
		TABLE_add_symbol(comp->class->table, start, len, &index);
	
	add_pattern(type, index);
}


static void add_operator()
{
	unsigned char car;
	const char *start;
	const char *end;
	int len;
	int op = NO_SYMBOL;
	int index;

	start = source_ptr;
	end = start;
	len = 1;

	for(;;)
	{
		source_ptr++;

		index = RESERVED_find_word(start, len);
		if (index >= 0)
		{
			op = index;
			end = source_ptr;
		}

		car = get_char();
		//if (!isascii(car) || !ispunct(car))
		if (noop_car[car])
			break;
		len++;
	}

	source_ptr = end;

	if (op < 0)
		THROW("Unknown operator");

	add_pattern(RT_RESERVED, op);
}


static int xdigit_val(unsigned char c)
{
	if (c >= '0' && c <= '9')
		return (c - '0');
	else if (c >= 'a' && c <= 'f')
		return (c - 'a' + 10);
	else if (c >= 'A' && c <= 'F')
		return (c - 'A' + 10);
	else
		return (-1);
}

static void add_string()
{
	unsigned char car;
	const char *start;
	int len;
	int index;
	int newline;
	bool jump;
	char *p;
	const char *end;
	int i;

	start = end = source_ptr;
	len = 0;
	newline = 0;
	jump = FALSE;
	p = (char *)source_ptr;

	for(;;)
	{
		source_ptr++;
		car = get_char();

		if (jump)
		{
			if (car == '\n')
				newline++;
			else if (car == '"')
				jump = FALSE;
			else if (!car || !isspace(car))
				break;
		}
		else
		{
			p++;
			len++;

			if (car == '\n')
				THROW("Non terminated string");

			if (car == '\\')
			{
				source_ptr++;
				car = get_char();

				if (car == 'n')
					*p = '\n';
				else if (car == 't')
					*p = '\t';
				else if (car == 'r')
					*p = '\r';
				else if (car == 'b')
					*p = '\b';
				else if (car == 'v')
					*p = '\v';
				else if (car == 'f')
					*p = '\f';
				else if (car == 'e')
					*p = '\x1B';
				else if (car == '0')
					*p = 0;
				else if (car == '\"' || car == '\'' || car == '\\')
					*p = car;
				else
				{
					if (car == 'x')
					{
						i = xdigit_val(get_char_offset(1));
						if (i >= 0)
						{
							car = i;
							i = xdigit_val(get_char_offset(2));
							if (i >= 0)
							{
								car = (car << 4) | (uchar)i;
								*p = car;
								source_ptr += 2;
								continue;
							}
						}
					}

					THROW("Bad character constant in string");
				}
			}
			else if (car == '"')
			{
				p--;
				len--;
				end = source_ptr;
				comp->line += newline;
				newline = 0;
				jump = TRUE;
			}
			else
				*p = car;
		}
	}

	if (len > 0)
	{
		TABLE_add_symbol(comp->class->string, start + 1, len, &index);
		add_pattern(RT_STRING, index);
	}
	else
		add_pattern(RT_STRING, VOID_STRING);

	source_ptr = end + 1;
	//for (i = 0; i < newline; i++)
	//	add_newline();
}


#if 0
static void add_command()
{
	unsigned char car;
	const char *start;
	int len;

	start = source_ptr;
	len = 0;

	for(;;)
	{
		source_ptr++;
		car = get_char();
		if (car == '\n' || !car)
			break;
		len++;
	}

	if (len)
	{
		//TABLE_add_symbol(comp->class->string, start + 1, len, NULL, &index);
		if (len == 7 && !strncasecmp(start + 1, "SECTION", 7))
			add_pattern(RT_COMMAND, RC_SECTION);
	}

	add_newline();
}
#endif

void READ_do(void)
{
	static void *jump_char[] =
	{
		&&__BREAK, 
		&&__SPACE, 
		&&__COMMENT, 
		&&__STRING, 
		&&__IDENT, 
		&&__QUOTED_IDENT,
		&&__NUMBER,
		&&__ERROR, 
		&&__SHARP,
		&&__OTHER
	};
	
	unsigned char car;

	comp = JOB;

	READ_init();
	PREP_init();
	
	//add_pattern(RT_NEWLINE, 0);

	source_ptr = comp->source;
	source_length = BUFFER_length(comp->source);
	_begin_line = TRUE;
	_prep = FALSE;

	//while (source_ptr < source_length)
	for(;;)
	{
		car = get_char();
		goto *jump_char[(int)first_car[car]];
		
	__ERROR:
		
		THROW("Syntax error");
			
	__SPACE:

		source_ptr++;
		if (car == '\n')
		{
			add_newline();
			_begin_line = TRUE;
		}
		continue;

	__COMMENT:

		source_ptr++;
		
		if (HELP_is_help_comment(source_ptr))
			HELP_add_at_current_line(source_ptr);
		
		car = get_char();
		while (car != '\n')
		{
			source_ptr++;
			car = get_char();
		}

		_begin_line = FALSE;
		continue;

	__STRING:
			
		add_string();
		_begin_line = FALSE;
		continue;

	__IDENT:
		
		add_identifier();
		_begin_line = FALSE;
		continue;

	__QUOTED_IDENT:
	
		source_ptr++;
		add_quoted_identifier();
		_begin_line = FALSE;
		continue;
		
	__NUMBER:
	
		add_number();
		_begin_line = FALSE;
		continue;
		
	__SHARP:
	
		if (_begin_line)
		{
			_prep = TRUE;
			_prep_index = comp->pattern_count;
			
			add_identifier();
			_begin_line = FALSE;
			continue;
		}
		else
			goto __OTHER;
	
	__OTHER:
	
#if BIG_COMMENT
		if (car == '/' && get_char_offset(1) == '*')
		{
			for(;;)
			{
				source_ptr++;
				car = get_char();
				if (car == 0)
					break;
				if (car == '*' && get_char_offset(1) == '/')
				{
					source_ptr += 2;
					break;
				}
				if (car == '\n')
					add_newline();
			}

			_begin_line = FALSE;
			continue;
		}
#endif
		
		if (add_number())
			add_operator();
		
		_begin_line = FALSE;
	}

__BREAK:

	// We add end markers to simplify the compiler job, when it needs to look 
	// at many patterns in one shot.
	
	JOB->max_line = JOB->line - 1;
	
	add_newline();
	add_end();
	add_end();
	add_end();
	add_end();

	PREP_exit();
	READ_exit();
}

