/***************************************************************************

  gbc_reserved_make.c

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

#define __GBC_RESERVED_MAKE_C

#include <ctype.h>
#include <errno.h>

#include "gb_common.h"
#include "gb_reserved.h"
#include "gb_type_common.h"
#include "gb_pcode.h"
#include "gb_reserved_keyword.h"

#define HASH_SIZE 109

/*uint hash(const char *key, int len)
{
	int i;
	uint h = 0;
	for (i = 0; i < len; i++)
		h = (h << 4) + (h ^ (tolower(key[i])));
	
	return h % HASH_SIZE;
}*/

char buffer[1024];

char *read_line(FILE *f)
{
	return fgets(buffer, sizeof(buffer), f);
}

void read_write_until(FILE *in, FILE *out, char *until)
{
	char *line;
	
	for(;;)
	{
		line = read_line(in);
		if (!line)
			break;
		if (out) fputs(line, out);
		while (*line && ((unsigned char)*line <= ' '))
			line++;
		if (!strncmp(line, until, strlen(until)))
			break;
	}
}

int main(int argc, char **argv)
{
  COMP_INFO *info;
  SUBR_INFO *subr;
  int len;
  int i, p, n;
	uint h;
	char c;
	char last[16];
	char next[16];
	FILE *in, *out;

	in = fopen("../share/gb_reserved_temp.h", "r");
	if (!in)
	{
		fprintf(stderr, "unable to open '../share/gb_reserved_temp.h': %s\n", strerror(errno));
		exit(1);
	}
	
	out = fopen("../share/gb_reserved_temp.h.tmp", "w");
	
	read_write_until(in, out, "int RESERVED_find_word(const char *word, int len)");

	read_write_until(in, out, "static void *jump[] = {");
	
	for (h = 0; h < 12; h++)
	{
		if ((h % 8) == 0)
			fprintf(out, "\t\t");
		fprintf(out, "&&__%02d, ", h);
		if ((h % 8) == 7)
			fprintf(out, "\n");
	}
	fprintf(out, "\n\t};\n\n");

	//printf("  goto *jump[h %% %d];\n\n", HASH_SIZE);
	fprintf(out, "\tgoto *jump[len];\n\n");
	
	fprintf(out, "__00:\n__01:\n\treturn -1;\n");
	
	for (h = 2; h < 12; h++)
	{
		fprintf(out, "__%02d:\n", h);
		
		*last = 0;
	
		for(;;)
		{
			n = -1;
			
			*next = 0;
			
			for (info = &COMP_res_info[1], i = 1; info->name; info++, i++)
			{
				len = strlen(info->name);
				if (len != h)
					continue;
				
				if (strcmp(info->name, last) <= 0)
					continue;
				
				if (*next == 0 || strcmp(info->name, next) <= 0)
				{
					strcpy(next, info->name);
					n = i;
				}
			}
			
			if (n < 0)
				break;
			
			info = &COMP_res_info[n];
			len = strlen(info->name);

			strcpy(last, info->name);
			
			fprintf(out, "\tif (");
			for (p = 0; p < len; p++)
			{
				if (p)
					fprintf(out, " && ");
				c = info->name[p];
				if (isalpha(c))
					fprintf(out, "tolower(word[%d]) == '%c'", p, tolower(info->name[p]));
				else if (c == '\\')
					fprintf(out, "word[%d] == '\\\\'", p);
				else
					fprintf(out, "word[%d] == '%c'", p, info->name[p]);
			}
			/*printf("  if (!strncmp(\"");
			for (p = 0; p < len; p++) 
			{
				c = info->name[p];
				if (c == '\\')
					printf("\\\\");
				else
					putchar(tolower(info->name[p]));
			}
			printf("\", word, %d)", len);*/
			fprintf(out, ") return %d;\n", n);
		}
		
		fprintf(out, "\treturn -1;\n");
	}
	
	read_write_until(in, NULL, "}");
	read_write_until(in, NULL, "}");
	fprintf(out, "}\n");

	read_write_until(in, out, "static void *jump[] = {");
	
	for (h = 0; h < 12; h++)
	{
		if ((h % 8) == 0)
			fprintf(out, "\t\t");
		fprintf(out, "&&__%02d, ", h);
		if ((h % 8) == 7)
			fprintf(out, "\n");
	}
	fprintf(out, "\n\t};\n\n");

	fprintf(out, "\tgoto *jump[len];\n\n");
	
	fprintf(out, "__00:\n__01:\n\treturn -1;\n");

	for (h = 2; h < 12; h++)
	{
		fprintf(out, "__%02d:\n", h);
		
		*last = 0;
		
		for(;;)
		{
			n = -1;
			
			*next = 0;
			
			for (subr = &COMP_subr_info[0], i = 0; subr->name; subr++, i++)
			{
				len = strlen(subr->name);
				if (len != h)
					continue;
				
				if (strcmp(subr->name, last) <= 0)
					continue;
				
				if (*next == 0 || strcmp(subr->name, next) <= 0)
				{
					strcpy(next, subr->name);
					n = i;
				}
			}
			
			if (n < 0)
				break;
			
			subr = &COMP_subr_info[n];
			len = strlen(subr->name);

			strcpy(last, subr->name);
			
			fprintf(out, "\tif ("); //COMPARE_%d(\"%s\", word)) return %d;\n", len, len, info->name, i);
			for (p = 0; p < len; p++)
			{
				if (p)
					fprintf(out, " && ");
				c = subr->name[p];
				if (isalpha(c))
					fprintf(out, "tolower(word[%d]) == '%c'", p, tolower(subr->name[p]));
				else if (c == '\\')
					fprintf(out, "word[%d] == '\\\\'", p);
				else
					fprintf(out, "word[%d] == '%c'", p, subr->name[p]);
			}
			fprintf(out, ") return %d;\n", n);
		}
		
		fprintf(out, "\treturn -1;\n");
	}
	
	fprintf(out, "}\n");
	
	fclose(in);
	fclose(out);
	
	unlink("../share/gb_reserved_temp.h");
	rename("../share/gb_reserved_temp.h.tmp", "../share/gb_reserved_temp.h");
	
	return 0;
}