/***************************************************************************

  gbc_help.c

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

#define __GBC_HELP_C

#include "gb_common.h"
#include "gb_error.h"
#include "gb_file.h"

#include "gbc_compile.h"
#include "gbc_help.h"

//#define DEBUG_ME 1

static bool get_help_at(int i, const char **help, int *len)
{
	const char *p;
	int n = 0;
	
	if (i < 0 || i >= ARRAY_count(JOB->help))
		return TRUE;

	p = JOB->help[i];
	if (!p)
		return TRUE;
	
	if (HELP_is_void_line(p))
	{
		*help = p;
		n = 0;
	}
	else
	{
		while (*p == '\'')
			p++;
		if (*p == ' ')
			p++;
		*help = p;
		while (*p++ != '\n')
			n++;
	}

	*len = n;
	return FALSE;
}

void HELP_add_at_current_line(const char *help)
{
	int count = ARRAY_count(JOB->help);
	int new_max;
	
	if (!count)
	{
		if (HELP_is_void_line(help))
			return;
		
		ARRAY_create_inc(&JOB->help, 256);
		
		JOB->help_first_line = JOB->line;
	}
	else
	{
		new_max = JOB->line - JOB->help_first_line;
		
		if (new_max < count) // Already added!
			return;
		
		if (new_max > count)
			ARRAY_add_many_void(&JOB->help, new_max - count);
	}

	*ARRAY_add(&JOB->help) = help;
	
	#if DEBUG_ME
	int len;
	get_help_at(JOB->line - JOB->help_first_line, &help, &len);
	fprintf(stderr, "add help comment: %d %p %d: %.*s\n", JOB->line, help, ARRAY_count(JOB->help), len, help);
	#endif
}

void HELP_search_and_print(FILE *file, int line)
{
	const char *help;
	int len, i, ii;
	
	if (!JOB->help)
		return;
	
	#if DEBUG_ME
	fprintf(stderr, "HELP_search_and_print: %s: %d\n", FILE_get_name(JOB->name), line);
	#endif
	
	i = line - JOB->help_first_line;
	
	if (i < 0)
		return;
	
	if (JOB->help[i])
	{
		if (!get_help_at(i, &help, &len))
		{
			#if DEBUG_ME
			fprintf(stderr, "[%d] '' %.*s\n", line, len, help);
			#endif
			fprintf(file, "'%.*s\n", len, help);
			JOB->help[i] = NULL;
		}
	}
	else
	{
		ii = i;
		i--;
		while (i >= 0)
		{
			if (!JOB->help[i] || HELP_is_for_class(JOB->help[i]))
				break;
			i--;
		}
		
		for (i++; i < ii; i++)
		{
			if (HELP_is_void_line(JOB->help[i]))
				continue;
			
			if (get_help_at(i, &help, &len))
				continue;
			
			#if DEBUG_ME
			fprintf(stderr, "{%d} ''%.*s\n", line + i - ii, len, help);
			#endif
			fprintf(file, "'%.*s\n", len, help);
		}
	}
}

void HELP_search_and_print_for_class(FILE *file)
{
	int i;
	const char *help;
	int len;
	
	if (!JOB->help)
		return;
	
	for (i = 0; i < ARRAY_count(JOB->help); i++)
	{
		if (!JOB->help[i] || !HELP_is_for_class(JOB->help[i]))
			break;
		
		if (get_help_at(i, &help, &len))
			break;
		
		#if DEBUG_ME
		fprintf(stderr, "[%d] '''%.*s\n", JOB->help_first_line + i, len, help);
		#endif
		fprintf(file, "'%.*s\n", len, help);
	}
}

