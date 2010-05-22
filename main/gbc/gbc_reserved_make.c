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

uint hash(const char *key, int len)
{
	int i;
	uint h = 1;
	for (i = 0; i < len; i++)
		h = (h << 4) + (h ^ (key[i] & 0x1F));
	
	return h % 73;
}

int main(int argc, char **argv)
{
  COMP_INFO *info;
  //SUBR_INFO *subr;
  int len;
  int i, p;
	uint h;
	char c;

	printf("  static void *jump[] = {\n");
	for (h = 0; h <= 72; h++)
	{
		printf("    &&__%02d, ", h);
		if ((h % 8) == 7)
			printf("\n");
	}
	printf("\n  };\n\n");

	printf("  goto *jump[hash(word, len)];\n\n");
	
	for (h = 0; h <= 72; h++)
	{
		printf("__%02d:\n", h);
		
		for (info = &COMP_res_info[1], i = 1; info->name; info++, i++)
		{
			len = strlen(info->name);
			if (len <= 1)
				continue;
			if (hash(info->name, len) != h)
				continue;
			printf("  if (len == %d ", len); //COMPARE_%d(\"%s\", word)) return %d;\n", len, len, info->name, i);
			for (p = 0; p < len; p++)
			{
				c = info->name[p];
				if (isalpha(c))
					printf("&& tolower(word[%d]) == '%c' ", p, tolower(info->name[p]));
				else if (c == '\\')
					printf("&& word[%d] == '\\\\' ", p);
				else
					printf("&& word[%d] == '%c' ", p, info->name[p]);
			}
			printf(") return %d;\n", i);
		}
		printf("  return -1;\n");
	}
	
	return 0;
}