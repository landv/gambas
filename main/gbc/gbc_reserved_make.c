/***************************************************************************

  gbc_reserved_make.c

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

#define __GBC_RESERVED_MAKE_C

#include <ctype.h>

#include "gb_common.h"
#include "gb_reserved.h"
#include "gb_type_common.h"
#include "gb_pcode.h"
#include "gb_reserved_keyword.h"

#define HASH_SIZE 109

uint hash(const char *key, int len)
{
	int i;
	uint h = 0;
	for (i = 0; i < len; i++)
		h = (h << 4) + (h ^ (tolower(key[i])));
	
	return h % HASH_SIZE;
}

int main(int argc, char **argv)
{
  COMP_INFO *info;
  SUBR_INFO *subr;
  int len;
  int i, p, l;
	uint h;
	char c;

	printf("//-----------------------------------------------\n");
	printf("  static void *jump[] = {\n");
	for (h = 0; h < 12; h++)
	{
		printf("    &&__%02d, ", h);
		if ((h % 8) == 7)
			printf("\n");
	}
	printf("\n  };\n\n");

	//printf("  goto *jump[h %% %d];\n\n", HASH_SIZE);
	printf("  goto *jump[len];\n\n");
	
	//for (h = 0; h < HASH_SIZE; h++)
	for (h = 0; h < 12; h++)
	{
		printf("__%02d:\n", h);
		
		for (l = 2; l < 11; l++)
		{
			for (info = &COMP_res_info[1], i = 1; info->name; info++, i++)
			{
				len = strlen(info->name);
				if (len != l)
					continue;
				//if (hash(info->name, len) != h)
				if (len != h)
					continue;
				//printf("  if (len == %d ", len); //COMPARE_%d(\"%s\", word)) return %d;\n", len, len, info->name, i);
				printf("  if (");
				for (p = 0; p < len; p++)
				{
					if (p)
						printf(" && ");
					c = info->name[p];
					if (isalpha(c))
						printf("tolower(word[%d]) == '%c'", p, tolower(info->name[p]));
					else if (c == '\\')
						printf("word[%d] == '\\\\'", p);
					else
						printf("word[%d] == '%c'", p, info->name[p]);
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
				printf(") return %d;\n", i);
			}
		}
		printf("  return -1;\n");
	}
	
	printf("//-----------------------------------------------\n");
	printf("  static void *jump[] = {\n");
	for (h = 0; h < 12; h++)
	{
		printf("    &&__%02d, ", h);
		if ((h % 8) == 7)
			printf("\n");
	}
	printf("\n  };\n\n");

	printf("  goto *jump[len];\n\n");
	
	for (h = 0; h < 12; h++)
	{
		printf("__%02d:\n", h);
		
		for (l = 1; l <= 11; l++)
		{
			for (subr = &COMP_subr_info[0], i = 0; subr->name; subr++, i++)
			{
				len = strlen(subr->name);
				if (len != l)
					continue;
				if (len != h)
					continue;
				printf("  if ("); //COMPARE_%d(\"%s\", word)) return %d;\n", len, len, info->name, i);
				for (p = 0; p < len; p++)
				{
					if (p)
						printf(" && ");
					c = subr->name[p];
					if (isalpha(c))
						printf("tolower(word[%d]) == '%c'", p, tolower(subr->name[p]));
					else if (c == '\\')
						printf("word[%d] == '\\\\'", p);
					else
						printf("word[%d] == '%c'", p, subr->name[p]);
				}
				printf(") return %d;\n", i);
			}
		}
		printf("  return -1;\n");
	}
	
	return 0;
}