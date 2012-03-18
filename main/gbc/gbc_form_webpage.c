/***************************************************************************

  gbc_form_webpage.c

  (c) 2000-2012 Benoît Minisini <gambas@users.sourceforge.net>

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

//static char *_source;
static const char *_start;

static void print_quoted_string(const char *str, int len)
{
	int i;
	char buf[8];
	char c;
	
	if (len == 0)
		return;
	
	FORM_print_char('"');
	
	for (i = 0; i < len; i++)
	{
		c = str[i];
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

void FORM_webpage(char *source)
{
	bool html;
	char c;
	const char *p;
	int line;
	char buf[8];
	
	line = FORM_FIRST_LINE;
	
	FORM_print("Inherits WebPage\n\n");
	FORM_print("Public Sub Render()\n\n");
	
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
		
		if (c == '<' && *p == '%')
		{
			flush_html(p - 1);
			p++;
			goto __CODE;
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
				
	if (*p == '=')
	{
		html = TRUE;
		p++;
	}
	else
		html = FALSE;
	
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
		
		if (c == '%' && *p == '>')
		{
			if (html)
			{
				FORM_print("Print Html$(");
				FORM_print_len(_start, p - _start - 1);
				FORM_print(");\n");
			}
			else
			{
				FORM_print_len(_start, p - _start - 1);
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

