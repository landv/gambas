/***************************************************************************

  gbc_form_webpage.c

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

#define __GBC_FORM_WEBPAGE_C

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <ctype.h>

#include <sys/stat.h>
#include <sys/types.h>

#include <unistd.h>

#include "gb_common.h"
#include "gb_error.h"
#include "gb_limit.h"
#include "gb_file.h"
#include "gb_str.h"
#include "gbc_compile.h"
#include "gbc_chown.h"
#include "gbc_form.h"

/*#define DEBUG*/

enum { TYPE_CODE, TYPE_HTML, TYPE_COMMENT, TYPE_MARKUP, TYPE_ARG, TYPE_ROOT };

static const char *_start;

static void print_quoted_string(const char *str, int len)
{
	int i;
	char buf[8];
	uchar c;
	
	if (len == 0)
		return;
	
	FORM_print_char('"');
	
	for (i = 0; i < len; i++)
	{
		c = (uchar)str[i];
		//if (c >= ' ' && c <= 126 && c != '\\' && c != '"')
		if (c >= ' ' && c != '\\' && c != '"')
			FORM_print_char(c);
		else
		{
			FORM_print_char('\\');
			if (c == '\n')
				c = 'n';
			else if (c == '\r')
				c = 'r';
			else if (c == '\t')
				c = 't';
			else if (!(c == '"' || c == '\\'))
			{
				snprintf(buf, sizeof(buf), "x%02X", c);
				FORM_print_len(buf, 3);
				continue;
			}
			FORM_print_char(c);
		}
	}
	
	FORM_print_char('"');
}

static void print_lower_len(const char *str, int len)
{
	int i;
	
	for (i = 0; i < len; i++)
		FORM_print_char(tolower(str[i]));
}

static void flush_html(const char *end)
{
	if (_start == end)
		return;
	
	FORM_print_len("Print ", 6);
	if (end[-1] == '\n')
	{
		print_quoted_string(_start, end - _start - 1);
		FORM_print_char('\n');
	}
	else
	{
		print_quoted_string(_start, end - _start);
		FORM_print_len(";\n", 2);
	}
	
	_start = end;
}

static bool check_contents(const char *str, int len)
{
	if (len < 4)
		return FALSE;
	
	if (str[0] == '-' && str[1] == '-' && str[len - 1] == '-' && str[len - 2] == '-')
		return TRUE;
	else
		return FALSE;
}

static int print_code(const char *str, int len)
{
	char c;
	const char *start = str;
	
	//fprintf(stderr, "'%.*s'\n", len, str);
	
	while (len > 0)
	{
		c = *str++;
		len--;
		
		if (c == '"')
		{
			while (len > 0)
			{
				c = *str++;
				len--;
				if (c == '"')
					break;
				if (c == '\\' && *str)
				{
					str++;
					len--;
				}
			}
		}
		else if (c == '%' && *str == '>')
		{
			len = str - start - 1;
			FORM_print_len(start, len);
			return len + 2;
		}
	}
	
	THROW("&1: syntax error in form file", JOB->form);
}

static void print_markup(const char *str, int len)
{
	int l;
	char c;
	const char *p;
	bool comma;
	bool quote;
	bool end;
	
	if (len <= 0)
		return;
	
	p = str;
	while (len > 0)
	{
		c = *str++;
		len--;
		if (c == ' ')
			break;
	}
	
	l = str - p - (c == ' ');	
	
	if (*p == '/')
	{
		end = TRUE;
		p++;
		l--;
	}
	else
		end = FALSE;
	
	if (l <= 0)
		THROW(E_SYNTAX);
	
	FORM_print_len(p, l);
	
	if (end)
	{
		FORM_print(".__RenderEnd()\n");
		return;
	}
	
	FORM_print(".__Render(");
	
	if (len > 0)
	{
		FORM_print_char('[');
		
		comma = FALSE;
		
		while (len > 0)
		{
			while (len > 0 && *str == ' ')
			{
				str++;
				len--;
			}
			
			if (len == 0)
				break;
			
			p = str;
			while (len > 0)
			{
				c = *str++;
				len--;
				if (c == '=' || c == ' ')
					break;
			}
			
			if (comma)
				FORM_print(", ");
			
			FORM_print_char('"');
			l = str - p - (c == '=');
			print_lower_len(p, l);
			FORM_print("\": ");
			
			comma = TRUE;
			
			if (len <= 0 || c == ' ')
			{
				FORM_print("True");
				continue;
			}
		
			p = str;
			
			// attr=<% ... %>
			if (len >= 3 && *str == '<' && str[1] == '%' && str[2] == '=')
			{
				str += 3;
				len -= 3;
				FORM_print_char('(');
				l = print_code(str, len);
				FORM_print_char(')');
				str += l;
				len -= l;
				//fprintf(stderr, "-> '%.*s'\n", len, str);
				continue;
			}
			else if (*str == '"')
			{
				str++;
				len--;
				quote = TRUE;
			}
			else
			{
				quote = FALSE;
			}
			
			while (len > 0)
			{
				c = *str++;
				len--;
				if (quote)
				{
					if (len > 0 && c == '\\')
					{
						str++;
						len--;
					}
					else if (c == '"')
						break;
				}
				else
				{
					if (c == ' ')
						break;
				}
			}
			
			if (!quote && str == p)
				FORM_print("True");
			else
			{
				if (!quote)
					FORM_print_char('"');
				FORM_print_len(p, str - p);
				if (!quote)
					FORM_print_char('"');
			}
		}
			
		FORM_print_char(']');
	}
	
	FORM_print(")\n");
}

void FORM_webpage(char *source)
{
	char type = TYPE_CODE;
	char c;
	const char *p;
	int line;
	char buf[8];
	bool has_contents = FALSE;
	
	line = FORM_FIRST_LINE;
	
	FORM_print("Inherits WebPage\n\n");
	FORM_print("Public Sub _Render(Optional (_Arg) As Collection = New Collection)\n\n");
	
	p = source;
	
__PRINT:
	
	_start = p;
	for(;;)
	{
		c = *p++;
		if (!c)
		{
			flush_html(p - 1);
			goto __END;
		}
		
		if (c == '<')
		{
			if (*p == '%')
			{
				flush_html(p - 1);
				
				if (p[1] == '/' && p[2] == '%' && p[3] == '>')
				{
					p += 4;
					type = TYPE_ROOT;
				}
				else
				{
					p++;
					type = TYPE_CODE;
				}
				
				goto __CODE;
			}
			else if (*p == '<')
			{
				flush_html(p - 1);
				p++;
				type = TYPE_MARKUP;
				goto __CODE;
			}
		}
		
		if (c == '\n')
		{
			line++;
			if ((p - _start) >= 256)
				flush_html(p);
		}
	}
	
__CODE:

	FORM_print("#Line ");
	sprintf(buf, "%d", line);
	FORM_print(buf);
	FORM_print_char('\n');
	
	if (type == TYPE_CODE)
	{
		if (*p == '=')
		{
			type = TYPE_HTML;
			p++;
		}
		else if (*p == '!')
		{
			type = TYPE_ARG;
			p++;
		}
		else if (*p == '-' && p[1] == '-')
		{
			type = TYPE_COMMENT;
			p += 2;
		}
		else
			type = TYPE_CODE;
	}
	else if (type == TYPE_ROOT)
	{
		FORM_print("Print Html$(Application.Root);\n");
		goto __PRINT;
	}
	
	_start = p;
	
__END_STRING:
	
	for(;;)
	{
		c = *p++;
		if (!c)
			goto __ERROR;
		if (c == '"')
			goto __STRING;
		
		if (c == '\n')
		{
			line++;
			continue;
		}
		
		if (c == '%' && *p == '>' && type != TYPE_MARKUP)
		{
			switch (type)
			{
				case TYPE_CODE:
					FORM_print_len(_start, p - _start - 1);
					FORM_print_char('\n');
					break;
					
				case TYPE_HTML:
					
					if ((p - _start) == 1)
					{
						FORM_print("Print Html$(Application.Root);\n");
					}
					else
					{
						FORM_print("Print Html$(");
						FORM_print_len(_start, p - _start - 1);
						FORM_print(");\n");
					}
					break;
					
				case TYPE_ARG:
					FORM_print("Print _Arg!");
					print_lower_len(_start, p - _start - 1);
					FORM_print(";\n");
					break;
					
				case TYPE_COMMENT:
				default:
					break;
			}
			
			p++;
			if (*p == '\n')
			{
				line++;
				p++;
			}
			goto __PRINT;
		}
		else if (c == '>' && *p == '>' && type == TYPE_MARKUP)
		{
			if (check_contents(_start, p - _start - 1))
			{
				if (has_contents)
					THROW("Contents already declared");
				has_contents = TRUE;

				FORM_print("\nEnd\n\n");
				FORM_print("Public Sub _RenderEnd()\n\n");
			}
			else
			{
				print_markup(_start, p - _start - 1);
				FORM_print_char('\n');
			}
			
			p++;
			if (*p == '\n')
			{
				line++;
				p++;
			}
			goto __PRINT;
		}
	}

__STRING:

	for(;;)
	{
		c = *p++;
		if (!c)
			goto __ERROR;
		if (c == '"')
			goto __END_STRING;
		if (c == '\\' && *p)
			p++;
	}
	
__END:

	FORM_print("\nEnd\n\n");
	return;
	
__ERROR:

	THROW("&1: syntax error in form file", JOB->form);
}

