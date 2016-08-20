/***************************************************************************

  font-parser.cpp

  (c) 2004-2006 - Daniel Campos Fernández <dcamposf@gmail.com>

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

#include <stdio.h>
#include <string.h>

#ifndef TRUE
#define TRUE -1
#endif

#ifndef FALSE
#define FALSE 0
#endif

static char  *gb_font_pointers[8];
static char  *gb_font_name;
static int   gb_font_strikeout;
static int   gb_font_underline;
static int   gb_font_italic;
static int   gb_font_bold;
static int   gb_font_size;
static int   gb_font_relative;

int gb_fontparser_italic()
{
	return gb_font_italic;
}

int gb_fontparser_bold()
{
	return gb_font_bold;
}

int gb_fontparser_underline()
{
	return gb_font_underline;
}

int gb_fontparser_strikeout()
{
	return gb_font_strikeout;
}

int gb_fontparser_relative()
{
	return gb_font_relative;
}

int gb_fontparser_size()
{
	return gb_font_size;
}

char* gb_fontparser_name()
{
	return gb_font_name;
}

void gb_font_trim()
{
	int bc,lenptr;
	char *ptr;

	for (bc=0;bc<8;bc++)
	{
		if (gb_font_pointers[bc]==NULL) return;
		ptr=gb_font_pointers[bc];
		while (ptr[0]==' ')
		{
			if (ptr[0]==0) break;
			if (ptr[0]==' ') gb_font_pointers[bc]++;
			ptr++;
		}
		lenptr=strlen(gb_font_pointers[bc])-1;
		ptr=gb_font_pointers[bc];
		while (lenptr>=0)
		{
			if (ptr[lenptr]==' ') { ptr[lenptr]=0; } else { lenptr=0; }
			lenptr--;
		}
	}


}

int gb_font_is_size(char *str)
{
	long bc,max;
	int rel=FALSE;
	int vl=0;
	int fact=1;

	if (!str) return TRUE;

	max=strlen(str);
	for(bc=0;bc<max;bc++)
	{
		switch (str[bc])
		{
			case '0': case '1': case '2': case '3': case '4': 
			case '5': case '6': case '7': case '8': case '9':
				vl*=10; vl+=(str[bc]-48); break;

			case '+':
				if (!bc) rel=TRUE;
				else return TRUE;
				break;

			case '-':
				if (!bc) { rel=TRUE; fact=-1; }
				else return TRUE;
				break;

			default: return TRUE;
		}
	}

	gb_font_size=vl;
	gb_font_relative=rel;
	if (gb_font_relative) gb_font_size*=fact;

	return FALSE;
}

void gb_fontparser_parse(char *str)
{
	long max,bc;
	long ptr=0;
	long curr=0;

	for (bc=0;bc<8;bc++) gb_font_pointers[bc]=NULL;
	gb_font_name=NULL;
	gb_font_strikeout=FALSE;
	gb_font_underline=FALSE;
	gb_font_italic=FALSE;
	gb_font_bold=FALSE;
    gb_font_relative=FALSE;
    gb_font_size=0;

	max=strlen(str);
	for (bc=0;bc<max;bc++)
		if (str[bc]==',') { str[bc]=0; gb_font_pointers[ptr++]=(str+curr); curr=bc+1; } 

	if (curr<(max-1)) gb_font_pointers[ptr]=(str+curr);

	gb_font_trim();

	for (bc=0; bc<8; bc++)
	{
		if ( gb_font_pointers[bc]==NULL	 ) break;
		
		if (!strcasecmp(gb_font_pointers[bc],"bold")) gb_font_bold=TRUE;
		else if (!strcasecmp(gb_font_pointers[bc],"italic")) gb_font_italic=TRUE;
		else if (!strcasecmp(gb_font_pointers[bc],"underline")) gb_font_underline=TRUE;
		else if (!strcasecmp(gb_font_pointers[bc],"strikeout")) gb_font_strikeout=TRUE;
		else
		{
			if (gb_font_is_size(gb_font_pointers[bc])) gb_font_name=gb_font_pointers[bc];		
		}
		
	}
}
