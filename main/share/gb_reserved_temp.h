/***************************************************************************

  gb_reserved_temp.h

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

#include "gb_common.h"
#include "gb_common_case.h"
#include "gb_pcode.h"
#include "gb_type_common.h"
#include "gb_reserved.h"

/* If this file is modified, don't forget to update GAMBAS_PCODE_VERSION in acinclude.m4 if needed */

#include "gb_reserved_keyword.h"

int SUBR_VarPtr;
int SUBR_Mid;
int SUBR_MidS;

static uchar _operator_table[256] = { 0 };

static int get_index(const char *subr_name)
{
	return RESERVED_find_subr(subr_name, strlen(subr_name));
}

void RESERVED_init(void)
{
	COMP_INFO *info;
	SUBR_INFO *subr;
	int len;
	int i;
	
	/* Reserved words symbol table */
	
	//TABLE_create(&COMP_res_table, 0, TF_IGNORE_CASE);
	for (info = &COMP_res_info[0], i = 0; info->name; info++, i++)
	{
		len = strlen(info->name);
		if (len == 1)
			_operator_table[(uint)*info->name] = i;
		
		//TABLE_add_symbol(COMP_res_table, info->name, len, &index);
	}
	
	/* Subroutines table */

	//TABLE_create(&COMP_subr_table, 0, TF_IGNORE_CASE);
	for (subr = &COMP_subr_info[0]; subr->name; subr++)
	{
		if (subr->max_param == 0)
			subr->max_param = subr->min_param;

		//TABLE_add_symbol(COMP_subr_table, subr->name, strlen(subr->name), &index);
	}

	SUBR_VarPtr = get_index("VarPtr");
	SUBR_Mid = get_index("Mid");
	SUBR_MidS = get_index("Mid$");
}


void RESERVED_exit(void)
{
	//TABLE_delete(&COMP_res_table);
	//TABLE_delete(&COMP_subr_table);
}


SUBR_INFO *SUBR_get(const char *subr_name)
{
	int index = get_index(subr_name);

	if (index == NO_SYMBOL)
		return NULL;
	else
		return &COMP_subr_info[index];
}


SUBR_INFO *SUBR_get_from_opcode(ushort opcode, ushort optype)
{
	SUBR_INFO *si;

	for (si = COMP_subr_info; si->name; si++)
	{
		if (si->opcode == opcode)
		{
			if (si->min_param != si->max_param)
				return si;
			else if (si->optype == optype || si->optype == 0)
				return si;
		}
	}

	/*ERROR_panic("SUBR_get_from_opcode: SUBR not found !");*/
	return NULL;
}


int RESERVED_find_word(const char *word, int len)
{
	int ind;
	
	if (len == 1)
	{
		ind = _operator_table[(uint)*word];
		if (ind)
			return ind;
		else
			return -1;
	}
	
	// No symbol longer than 10 characters in the table
	
	if (len > 10)
		return -1;
	
	// Now find it

	static void *jump[] = {
		&&__00, &&__01, &&__02, &&__03, &&__04, &&__05, &&__06, &&__07, 
		&&__08, &&__09, &&__10, &&__11, 
	};

	goto *jump[len];

__00:
__01:
	return -1;
__02:
	if (word[0] == '%' && word[1] == '=') return 166;
	if (word[0] == '&' && word[1] == '/') return 137;
	if (word[0] == '&' && word[1] == '=') return 167;
	if (word[0] == '*' && word[1] == '=') return 163;
	if (word[0] == '+' && word[1] == '=') return 161;
	if (word[0] == '-' && word[1] == '=') return 162;
	if (word[0] == '/' && word[1] == '=') return 164;
	if (word[0] == '<' && word[1] == '=') return 141;
	if (word[0] == '<' && word[1] == '>') return 142;
	if (word[0] == '=' && word[1] == '=') return 126;
	if (word[0] == '>' && word[1] == '=') return 140;
	if (tolower(word[0]) == 'a' && tolower(word[1]) == 's') return 25;
	if (tolower(word[0]) == 'd' && tolower(word[1]) == 'o') return 34;
	if (tolower(word[0]) == 'i' && tolower(word[1]) == 'f') return 40;
	if (tolower(word[0]) == 'i' && tolower(word[1]) == 'n') return 69;
	if (tolower(word[0]) == 'i' && tolower(word[1]) == 's') return 153;
	if (tolower(word[0]) == 'm' && tolower(word[1]) == 'e') return 57;
	if (tolower(word[0]) == 'o' && tolower(word[1]) == 'f') return 26;
	if (tolower(word[0]) == 'o' && tolower(word[1]) == 'r') return 146;
	if (tolower(word[0]) == 't' && tolower(word[1]) == 'o') return 46;
	if (word[0] == '\\' && word[1] == '=') return 165;
	if (word[0] == '^' && word[1] == '=') return 169;
	return -1;
__03:
	if (word[0] == '#' && tolower(word[1]) == 'i' && tolower(word[2]) == 'f') return 111;
	if (word[0] == '&' && word[1] == '/' && word[2] == '=') return 168;
	if (word[0] == '.' && word[1] == '.' && word[2] == '.') return 119;
	if (tolower(word[0]) == 'a' && tolower(word[1]) == 'n' && tolower(word[2]) == 'd') return 145;
	if (tolower(word[0]) == 'd' && tolower(word[1]) == 'e' && tolower(word[2]) == 'c') return 98;
	if (tolower(word[0]) == 'd' && tolower(word[1]) == 'i' && tolower(word[2]) == 'm') return 27;
	if (tolower(word[0]) == 'd' && tolower(word[1]) == 'i' && tolower(word[2]) == 'v') return 150;
	if (tolower(word[0]) == 'e' && tolower(word[1]) == 'n' && tolower(word[2]) == 'd') return 44;
	if (tolower(word[0]) == 'f' && tolower(word[1]) == 'o' && tolower(word[2]) == 'r') return 45;
	if (tolower(word[0]) == 'i' && tolower(word[1]) == 'n' && tolower(word[2]) == 'c') return 97;
	if (tolower(word[0]) == 'l' && tolower(word[1]) == 'e' && tolower(word[2]) == 't') return 77;
	if (tolower(word[0]) == 'm' && tolower(word[1]) == 'o' && tolower(word[2]) == 'd') return 152;
	if (tolower(word[0]) == 'n' && tolower(word[1]) == 'e' && tolower(word[2]) == 'w') return 28;
	if (tolower(word[0]) == 'n' && tolower(word[1]) == 'o' && tolower(word[2]) == 't') return 147;
	if (tolower(word[0]) == 's' && tolower(word[1]) == 'u' && tolower(word[2]) == 'b') return 30;
	if (tolower(word[0]) == 't' && tolower(word[1]) == 'r' && tolower(word[2]) == 'y') return 59;
	if (tolower(word[0]) == 'x' && tolower(word[1]) == 'o' && tolower(word[2]) == 'r') return 148;
	return -1;
__04:
	if (tolower(word[0]) == 'b' && tolower(word[1]) == 'y' && tolower(word[2]) == 't' && tolower(word[3]) == 'e') return 2;
	if (tolower(word[0]) == 'c' && tolower(word[1]) == 'a' && tolower(word[2]) == 's' && tolower(word[3]) == 'e') return 52;
	if (tolower(word[0]) == 'c' && tolower(word[1]) == 'o' && tolower(word[2]) == 'p' && tolower(word[3]) == 'y') return 96;
	if (tolower(word[0]) == 'd' && tolower(word[1]) == 'a' && tolower(word[2]) == 't' && tolower(word[3]) == 'e') return 3;
	if (tolower(word[0]) == 'e' && tolower(word[1]) == 'a' && tolower(word[2]) == 'c' && tolower(word[3]) == 'h') return 68;
	if (tolower(word[0]) == 'e' && tolower(word[1]) == 'l' && tolower(word[2]) == 's' && tolower(word[3]) == 'e') return 42;
	if (tolower(word[0]) == 'e' && tolower(word[1]) == 'n' && tolower(word[2]) == 'd' && tolower(word[3]) == 's') return 159;
	if (tolower(word[0]) == 'e' && tolower(word[1]) == 'n' && tolower(word[2]) == 'u' && tolower(word[3]) == 'm') return 76;
	if (tolower(word[0]) == 'e' && tolower(word[1]) == 'x' && tolower(word[2]) == 'e' && tolower(word[3]) == 'c') return 90;
	if (tolower(word[0]) == 'e' && tolower(word[1]) == 'x' && tolower(word[2]) == 'i' && tolower(word[3]) == 't') return 53;
	if (tolower(word[0]) == 'f' && tolower(word[1]) == 'r' && tolower(word[2]) == 'o' && tolower(word[3]) == 'm') return 48;
	if (tolower(word[0]) == 'g' && tolower(word[1]) == 'o' && tolower(word[2]) == 't' && tolower(word[3]) == 'o') return 56;
	if (tolower(word[0]) == 'k' && tolower(word[1]) == 'i' && tolower(word[2]) == 'l' && tolower(word[3]) == 'l') return 94;
	if (tolower(word[0]) == 'l' && tolower(word[1]) == 'a' && tolower(word[2]) == 's' && tolower(word[3]) == 't') return 58;
	if (tolower(word[0]) == 'l' && tolower(word[1]) == 'i' && tolower(word[2]) == 'k' && tolower(word[3]) == 'e') return 155;
	if (tolower(word[0]) == 'l' && tolower(word[1]) == 'i' && tolower(word[2]) == 'n' && tolower(word[3]) == 'e') return 88;
	if (tolower(word[0]) == 'l' && tolower(word[1]) == 'i' && tolower(word[2]) == 'n' && tolower(word[3]) == 'k') return 102;
	if (tolower(word[0]) == 'l' && tolower(word[1]) == 'o' && tolower(word[2]) == 'c' && tolower(word[3]) == 'k') return 103;
	if (tolower(word[0]) == 'l' && tolower(word[1]) == 'o' && tolower(word[2]) == 'n' && tolower(word[3]) == 'g') return 7;
	if (tolower(word[0]) == 'l' && tolower(word[1]) == 'o' && tolower(word[2]) == 'o' && tolower(word[3]) == 'p') return 35;
	if (tolower(word[0]) == 'm' && tolower(word[1]) == 'o' && tolower(word[2]) == 'v' && tolower(word[3]) == 'e') return 95;
	if (tolower(word[0]) == 'n' && tolower(word[1]) == 'e' && tolower(word[2]) == 'x' && tolower(word[3]) == 't') return 50;
	if (tolower(word[0]) == 'n' && tolower(word[1]) == 'u' && tolower(word[2]) == 'l' && tolower(word[3]) == 'l') return 66;
	if (tolower(word[0]) == 'o' && tolower(word[1]) == 'p' && tolower(word[2]) == 'e' && tolower(word[3]) == 'n') return 82;
	if (tolower(word[0]) == 'p' && tolower(word[1]) == 'i' && tolower(word[2]) == 'p' && tolower(word[3]) == 'e') return 107;
	if (tolower(word[0]) == 'q' && tolower(word[1]) == 'u' && tolower(word[2]) == 'i' && tolower(word[3]) == 't') return 72;
	if (tolower(word[0]) == 'r' && tolower(word[1]) == 'e' && tolower(word[2]) == 'a' && tolower(word[3]) == 'd') return 80;
	if (tolower(word[0]) == 's' && tolower(word[1]) == 'e' && tolower(word[2]) == 'e' && tolower(word[3]) == 'k') return 84;
	if (tolower(word[0]) == 's' && tolower(word[1]) == 't' && tolower(word[2]) == 'e' && tolower(word[3]) == 'p') return 49;
	if (tolower(word[0]) == 's' && tolower(word[1]) == 't' && tolower(word[2]) == 'o' && tolower(word[3]) == 'p') return 71;
	if (tolower(word[0]) == 's' && tolower(word[1]) == 'w' && tolower(word[2]) == 'a' && tolower(word[3]) == 'p') return 65;
	if (tolower(word[0]) == 't' && tolower(word[1]) == 'h' && tolower(word[2]) == 'e' && tolower(word[3]) == 'n') return 41;
	if (tolower(word[0]) == 't' && tolower(word[1]) == 'r' && tolower(word[2]) == 'u' && tolower(word[3]) == 'e') return 63;
	if (tolower(word[0]) == 'w' && tolower(word[1]) == 'a' && tolower(word[2]) == 'i' && tolower(word[3]) == 't') return 92;
	if (tolower(word[0]) == 'w' && tolower(word[1]) == 'e' && tolower(word[2]) == 'n' && tolower(word[3]) == 'd') return 39;
	if (tolower(word[0]) == 'w' && tolower(word[1]) == 'i' && tolower(word[2]) == 't' && tolower(word[3]) == 'h') return 62;
	return -1;
__05:
	if (word[0] == '#' && tolower(word[1]) == 'e' && tolower(word[2]) == 'l' && tolower(word[3]) == 's' && tolower(word[4]) == 'e') return 112;
	if (word[0] == '#' && tolower(word[1]) == 'l' && tolower(word[2]) == 'i' && tolower(word[3]) == 'n' && tolower(word[4]) == 'e') return 115;
	if (tolower(word[0]) == 'b' && tolower(word[1]) == 'r' && tolower(word[2]) == 'e' && tolower(word[3]) == 'a' && tolower(word[4]) == 'k') return 54;
	if (tolower(word[0]) == 'b' && tolower(word[1]) == 'y' && tolower(word[2]) == 'r' && tolower(word[3]) == 'e' && tolower(word[4]) == 'f') return 109;
	if (tolower(word[0]) == 'c' && tolower(word[1]) == 'a' && tolower(word[2]) == 't' && tolower(word[3]) == 'c' && tolower(word[4]) == 'h') return 61;
	if (tolower(word[0]) == 'c' && tolower(word[1]) == 'l' && tolower(word[2]) == 'a' && tolower(word[3]) == 's' && tolower(word[4]) == 's') return 13;
	if (tolower(word[0]) == 'c' && tolower(word[1]) == 'l' && tolower(word[2]) == 'o' && tolower(word[3]) == 's' && tolower(word[4]) == 'e') return 83;
	if (tolower(word[0]) == 'c' && tolower(word[1]) == 'o' && tolower(word[2]) == 'n' && tolower(word[3]) == 's' && tolower(word[4]) == 't') return 16;
	if (tolower(word[0]) == 'd' && tolower(word[1]) == 'e' && tolower(word[2]) == 'b' && tolower(word[3]) == 'u' && tolower(word[4]) == 'g') return 106;
	if (tolower(word[0]) == 'e' && tolower(word[1]) == 'n' && tolower(word[2]) == 'd' && tolower(word[3]) == 'i' && tolower(word[4]) == 'f') return 43;
	if (tolower(word[0]) == 'e' && tolower(word[1]) == 'r' && tolower(word[2]) == 'r' && tolower(word[3]) == 'o' && tolower(word[4]) == 'r') return 74;
	if (tolower(word[0]) == 'e' && tolower(word[1]) == 'v' && tolower(word[2]) == 'e' && tolower(word[3]) == 'n' && tolower(word[4]) == 't') return 21;
	if (tolower(word[0]) == 'f' && tolower(word[1]) == 'a' && tolower(word[2]) == 'l' && tolower(word[3]) == 's' && tolower(word[4]) == 'e') return 64;
	if (tolower(word[0]) == 'f' && tolower(word[1]) == 'l' && tolower(word[2]) == 'o' && tolower(word[3]) == 'a' && tolower(word[4]) == 't') return 5;
	if (tolower(word[0]) == 'f' && tolower(word[1]) == 'l' && tolower(word[2]) == 'u' && tolower(word[3]) == 's' && tolower(word[4]) == 'h') return 89;
	if (tolower(word[0]) == 'i' && tolower(word[1]) == 'n' && tolower(word[2]) == 'p' && tolower(word[3]) == 'u' && tolower(word[4]) == 't') return 79;
	if (tolower(word[0]) == 'm' && tolower(word[1]) == 'k' && tolower(word[2]) == 'd' && tolower(word[3]) == 'i' && tolower(word[4]) == 'r') return 99;
	if (tolower(word[0]) == 'p' && tolower(word[1]) == 'r' && tolower(word[2]) == 'i' && tolower(word[3]) == 'n' && tolower(word[4]) == 't') return 78;
	if (tolower(word[0]) == 'r' && tolower(word[1]) == 'a' && tolower(word[2]) == 'i' && tolower(word[3]) == 's' && tolower(word[4]) == 'e') return 73;
	if (tolower(word[0]) == 'r' && tolower(word[1]) == 'm' && tolower(word[2]) == 'd' && tolower(word[3]) == 'i' && tolower(word[4]) == 'r') return 100;
	if (tolower(word[0]) == 's' && tolower(word[1]) == 'h' && tolower(word[2]) == 'e' && tolower(word[3]) == 'l' && tolower(word[4]) == 'l') return 91;
	if (tolower(word[0]) == 's' && tolower(word[1]) == 'h' && tolower(word[2]) == 'o' && tolower(word[3]) == 'r' && tolower(word[4]) == 't') return 8;
	if (tolower(word[0]) == 's' && tolower(word[1]) == 'l' && tolower(word[2]) == 'e' && tolower(word[3]) == 'e' && tolower(word[4]) == 'p') return 93;
	if (tolower(word[0]) == 's' && tolower(word[1]) == 'u' && tolower(word[2]) == 'p' && tolower(word[3]) == 'e' && tolower(word[4]) == 'r') return 75;
	if (tolower(word[0]) == 'u' && tolower(word[1]) == 'n' && tolower(word[2]) == 't' && tolower(word[3]) == 'i' && tolower(word[4]) == 'l') return 37;
	if (tolower(word[0]) == 'w' && tolower(word[1]) == 'a' && tolower(word[2]) == 't' && tolower(word[3]) == 'c' && tolower(word[4]) == 'h') return 101;
	if (tolower(word[0]) == 'w' && tolower(word[1]) == 'h' && tolower(word[2]) == 'i' && tolower(word[3]) == 'l' && tolower(word[4]) == 'e') return 36;
	if (tolower(word[0]) == 'w' && tolower(word[1]) == 'r' && tolower(word[2]) == 'i' && tolower(word[3]) == 't' && tolower(word[4]) == 'e') return 81;
	return -1;
__06:
	if (word[0] == '#' && tolower(word[1]) == 'c' && tolower(word[2]) == 'o' && tolower(word[3]) == 'n' && tolower(word[4]) == 's' && tolower(word[5]) == 't') return 114;
	if (word[0] == '#' && tolower(word[1]) == 'e' && tolower(word[2]) == 'n' && tolower(word[3]) == 'd' && tolower(word[4]) == 'i' && tolower(word[5]) == 'f') return 113;
	if (tolower(word[0]) == 'a' && tolower(word[1]) == 'p' && tolower(word[2]) == 'p' && tolower(word[3]) == 'e' && tolower(word[4]) == 'n' && tolower(word[5]) == 'd') return 85;
	if (tolower(word[0]) == 'b' && tolower(word[1]) == 'e' && tolower(word[2]) == 'g' && tolower(word[3]) == 'i' && tolower(word[4]) == 'n' && tolower(word[5]) == 's') return 157;
	if (tolower(word[0]) == 'b' && tolower(word[1]) == 'i' && tolower(word[2]) == 'n' && tolower(word[3]) == 'a' && tolower(word[4]) == 'r' && tolower(word[5]) == 'y') return 87;
	if (tolower(word[0]) == 'c' && tolower(word[1]) == 'r' && tolower(word[2]) == 'e' && tolower(word[3]) == 'a' && tolower(word[4]) == 't' && tolower(word[5]) == 'e') return 86;
	if (tolower(word[0]) == 'd' && tolower(word[1]) == 'o' && tolower(word[2]) == 'w' && tolower(word[3]) == 'n' && tolower(word[4]) == 't' && tolower(word[5]) == 'o') return 47;
	if (tolower(word[0]) == 'e' && tolower(word[1]) == 'x' && tolower(word[2]) == 'p' && tolower(word[3]) == 'o' && tolower(word[4]) == 'r' && tolower(word[5]) == 't') return 24;
	if (tolower(word[0]) == 'e' && tolower(word[1]) == 'x' && tolower(word[2]) == 't' && tolower(word[3]) == 'e' && tolower(word[4]) == 'r' && tolower(word[5]) == 'n') return 67;
	if (tolower(word[0]) == 'm' && tolower(word[1]) == 'e' && tolower(word[2]) == 'm' && tolower(word[3]) == 'o' && tolower(word[4]) == 'r' && tolower(word[5]) == 'y') return 110;
	if (tolower(word[0]) == 'o' && tolower(word[1]) == 'b' && tolower(word[2]) == 'j' && tolower(word[3]) == 'e' && tolower(word[4]) == 'c' && tolower(word[5]) == 't') return 11;
	if (tolower(word[0]) == 'o' && tolower(word[1]) == 'u' && tolower(word[2]) == 't' && tolower(word[3]) == 'p' && tolower(word[4]) == 'u' && tolower(word[5]) == 't') return 33;
	if (tolower(word[0]) == 'p' && tolower(word[1]) == 'u' && tolower(word[2]) == 'b' && tolower(word[3]) == 'l' && tolower(word[4]) == 'i' && tolower(word[5]) == 'c') return 18;
	if (tolower(word[0]) == 'r' && tolower(word[1]) == 'e' && tolower(word[2]) == 'p' && tolower(word[3]) == 'e' && tolower(word[4]) == 'a' && tolower(word[5]) == 't') return 38;
	if (tolower(word[0]) == 'r' && tolower(word[1]) == 'e' && tolower(word[2]) == 't' && tolower(word[3]) == 'u' && tolower(word[4]) == 'r' && tolower(word[5]) == 'n') return 31;
	if (tolower(word[0]) == 's' && tolower(word[1]) == 'e' && tolower(word[2]) == 'l' && tolower(word[3]) == 'e' && tolower(word[4]) == 'c' && tolower(word[5]) == 't') return 51;
	if (tolower(word[0]) == 's' && tolower(word[1]) == 'i' && tolower(word[2]) == 'n' && tolower(word[3]) == 'g' && tolower(word[4]) == 'l' && tolower(word[5]) == 'e') return 4;
	if (tolower(word[0]) == 's' && tolower(word[1]) == 't' && tolower(word[2]) == 'a' && tolower(word[3]) == 't' && tolower(word[4]) == 'i' && tolower(word[5]) == 'c') return 19;
	if (tolower(word[0]) == 's' && tolower(word[1]) == 't' && tolower(word[2]) == 'r' && tolower(word[3]) == 'i' && tolower(word[4]) == 'n' && tolower(word[5]) == 'g') return 9;
	if (tolower(word[0]) == 's' && tolower(word[1]) == 't' && tolower(word[2]) == 'r' && tolower(word[3]) == 'u' && tolower(word[4]) == 'c' && tolower(word[5]) == 't') return 15;
	if (tolower(word[0]) == 'u' && tolower(word[1]) == 'n' && tolower(word[2]) == 'l' && tolower(word[3]) == 'o' && tolower(word[4]) == 'c' && tolower(word[5]) == 'k') return 104;
	return -1;
__07:
	if (tolower(word[0]) == 'b' && tolower(word[1]) == 'o' && tolower(word[2]) == 'o' && tolower(word[3]) == 'l' && tolower(word[4]) == 'e' && tolower(word[5]) == 'a' && tolower(word[6]) == 'n') return 1;
	if (tolower(word[0]) == 'd' && tolower(word[1]) == 'e' && tolower(word[2]) == 'f' && tolower(word[3]) == 'a' && tolower(word[4]) == 'u' && tolower(word[5]) == 'l' && tolower(word[6]) == 't') return 70;
	if (tolower(word[0]) == 'f' && tolower(word[1]) == 'i' && tolower(word[2]) == 'n' && tolower(word[3]) == 'a' && tolower(word[4]) == 'l' && tolower(word[5]) == 'l' && tolower(word[6]) == 'y') return 60;
	if (tolower(word[0]) == 'i' && tolower(word[1]) == 'n' && tolower(word[2]) == 't' && tolower(word[3]) == 'e' && tolower(word[4]) == 'g' && tolower(word[5]) == 'e' && tolower(word[6]) == 'r') return 6;
	if (tolower(word[0]) == 'l' && tolower(word[1]) == 'i' && tolower(word[2]) == 'b' && tolower(word[3]) == 'r' && tolower(word[4]) == 'a' && tolower(word[5]) == 'r' && tolower(word[6]) == 'y') return 105;
	if (tolower(word[0]) == 'p' && tolower(word[1]) == 'o' && tolower(word[2]) == 'i' && tolower(word[3]) == 'n' && tolower(word[4]) == 't' && tolower(word[5]) == 'e' && tolower(word[6]) == 'r') return 12;
	if (tolower(word[0]) == 'p' && tolower(word[1]) == 'r' && tolower(word[2]) == 'i' && tolower(word[3]) == 'v' && tolower(word[4]) == 'a' && tolower(word[5]) == 't' && tolower(word[6]) == 'e') return 17;
	if (tolower(word[0]) == 'v' && tolower(word[1]) == 'a' && tolower(word[2]) == 'r' && tolower(word[3]) == 'i' && tolower(word[4]) == 'a' && tolower(word[5]) == 'n' && tolower(word[6]) == 't') return 10;
	return -1;
__08:
	if (tolower(word[0]) == 'c' && tolower(word[1]) == 'o' && tolower(word[2]) == 'n' && tolower(word[3]) == 't' && tolower(word[4]) == 'i' && tolower(word[5]) == 'n' && tolower(word[6]) == 'u' && tolower(word[7]) == 'e') return 55;
	if (tolower(word[0]) == 'f' && tolower(word[1]) == 'u' && tolower(word[2]) == 'n' && tolower(word[3]) == 'c' && tolower(word[4]) == 't' && tolower(word[5]) == 'i' && tolower(word[6]) == 'o' && tolower(word[7]) == 'n') return 14;
	if (tolower(word[0]) == 'i' && tolower(word[1]) == 'n' && tolower(word[2]) == 'h' && tolower(word[3]) == 'e' && tolower(word[4]) == 'r' && tolower(word[5]) == 'i' && tolower(word[6]) == 't' && tolower(word[7]) == 's') return 22;
	if (tolower(word[0]) == 'o' && tolower(word[1]) == 'p' && tolower(word[2]) == 't' && tolower(word[3]) == 'i' && tolower(word[4]) == 'o' && tolower(word[5]) == 'n' && tolower(word[6]) == 'a' && tolower(word[7]) == 'l') return 32;
	if (tolower(word[0]) == 'p' && tolower(word[1]) == 'r' && tolower(word[2]) == 'o' && tolower(word[3]) == 'p' && tolower(word[4]) == 'e' && tolower(word[5]) == 'r' && tolower(word[6]) == 't' && tolower(word[7]) == 'y') return 20;
	return -1;
__09:
	if (tolower(word[0]) == 'p' && tolower(word[1]) == 'r' && tolower(word[2]) == 'o' && tolower(word[3]) == 'c' && tolower(word[4]) == 'e' && tolower(word[5]) == 'd' && tolower(word[6]) == 'u' && tolower(word[7]) == 'r' && tolower(word[8]) == 'e') return 29;
	if (tolower(word[0]) == 'r' && tolower(word[1]) == 'a' && tolower(word[2]) == 'n' && tolower(word[3]) == 'd' && tolower(word[4]) == 'o' && tolower(word[5]) == 'm' && tolower(word[6]) == 'i' && tolower(word[7]) == 'z' && tolower(word[8]) == 'e') return 108;
	return -1;
__10:
	if (tolower(word[0]) == 'i' && tolower(word[1]) == 'm' && tolower(word[2]) == 'p' && tolower(word[3]) == 'l' && tolower(word[4]) == 'e' && tolower(word[5]) == 'm' && tolower(word[6]) == 'e' && tolower(word[7]) == 'n' && tolower(word[8]) == 't' && tolower(word[9]) == 's') return 23;
	return -1;
__11:
	return -1;
}

int RESERVED_find_subr(const char *word, int len)
{
	// No symbol longer than 11 characters in the table
	
	if (len > 11)
		return -1;
	
	// Now find it

	static void *jump[] = {
		&&__00, &&__01, &&__02, &&__03, &&__04, &&__05, &&__06, &&__07, 
		&&__08, &&__09, &&__10, &&__11, 
	};

	goto *jump[len];

__00:
__01:
	return -1;
__02:
	if (tolower(word[0]) == 'i' && tolower(word[1]) == 'f') return 86;
	if (tolower(word[0]) == 'p' && tolower(word[1]) == 'i') return 80;
	if (tolower(word[0]) == 't' && tolower(word[1]) == 'r') return 212;
	return -1;
__03:
	if (tolower(word[0]) == 'a' && tolower(word[1]) == 'b' && tolower(word[2]) == 's') return 43;
	if (tolower(word[0]) == 'a' && tolower(word[1]) == 'c' && tolower(word[2]) == 's') return 58;
	if (tolower(word[0]) == 'a' && tolower(word[1]) == 'n' && tolower(word[2]) == 'g') return 92;
	if (tolower(word[0]) == 'a' && tolower(word[1]) == 's' && tolower(word[2]) == 'c') return 27;
	if (tolower(word[0]) == 'a' && tolower(word[1]) == 's' && tolower(word[2]) == 'l') return 111;
	if (tolower(word[0]) == 'a' && tolower(word[1]) == 's' && tolower(word[2]) == 'n') return 56;
	if (tolower(word[0]) == 'a' && tolower(word[1]) == 's' && tolower(word[2]) == 'r') return 113;
	if (tolower(word[0]) == 'a' && tolower(word[1]) == 't' && tolower(word[2]) == 'n') return 54;
	if (tolower(word[0]) == 'b' && tolower(word[1]) == 'i' && tolower(word[2]) == 'n') return 142;
	if (tolower(word[0]) == 'c' && tolower(word[1]) == 'b' && tolower(word[2]) == 'r') return 75;
	if (tolower(word[0]) == 'c' && tolower(word[1]) == 'h' && tolower(word[2]) == 'r') return 26;
	if (tolower(word[0]) == 'c' && tolower(word[1]) == 'o' && tolower(word[2]) == 's') return 52;
	if (tolower(word[0]) == 'd' && tolower(word[1]) == 'a' && tolower(word[2]) == 'y') return 154;
	if (tolower(word[0]) == 'd' && tolower(word[1]) == 'e' && tolower(word[2]) == 'g') return 60;
	if (tolower(word[0]) == 'd' && tolower(word[1]) == 'i' && tolower(word[2]) == 'r') return 200;
	if (tolower(word[0]) == 'e' && tolower(word[1]) == 'o' && tolower(word[2]) == 'f') return 184;
	if (tolower(word[0]) == 'e' && tolower(word[1]) == 'x' && tolower(word[2]) == 'p') return 49;
	if (tolower(word[0]) == 'f' && tolower(word[1]) == 'i' && tolower(word[2]) == 'x') return 45;
	if (tolower(word[0]) == 'h' && tolower(word[1]) == 'e' && tolower(word[2]) == 'x') return 144;
	if (tolower(word[0]) == 'h' && tolower(word[1]) == 'y' && tolower(word[2]) == 'p') return 93;
	if (tolower(word[0]) == 'i' && tolower(word[1]) == 'i' && tolower(word[2]) == 'f') return 87;
	if (tolower(word[0]) == 'i' && tolower(word[1]) == 'n' && tolower(word[2]) == 't') return 44;
	if (tolower(word[0]) == 'l' && tolower(word[1]) == 'e' && tolower(word[2]) == 'n') return 6;
	if (tolower(word[0]) == 'l' && tolower(word[1]) == 'o' && tolower(word[2]) == 'f') return 185;
	if (tolower(word[0]) == 'l' && tolower(word[1]) == 'o' && tolower(word[2]) == 'g') return 48;
	if (tolower(word[0]) == 'l' && tolower(word[1]) == 's' && tolower(word[2]) == 'l') return 116;
	if (tolower(word[0]) == 'l' && tolower(word[1]) == 's' && tolower(word[2]) == 'r') return 117;
	if (tolower(word[0]) == 'm' && tolower(word[1]) == 'a' && tolower(word[2]) == 'g') return 94;
	if (tolower(word[0]) == 'm' && tolower(word[1]) == 'a' && tolower(word[2]) == 'x') return 85;
	if (tolower(word[0]) == 'm' && tolower(word[1]) == 'i' && tolower(word[2]) == 'd') return 3;
	if (tolower(word[0]) == 'm' && tolower(word[1]) == 'i' && tolower(word[2]) == 'n') return 84;
	if (tolower(word[0]) == 'n' && tolower(word[1]) == 'o' && tolower(word[2]) == 'w') return 151;
	if (tolower(word[0]) == 'r' && tolower(word[1]) == 'a' && tolower(word[2]) == 'd') return 61;
	if (tolower(word[0]) == 'r' && tolower(word[1]) == 'n' && tolower(word[2]) == 'd') return 83;
	if (tolower(word[0]) == 'r' && tolower(word[1]) == 'o' && tolower(word[2]) == 'l') return 114;
	if (tolower(word[0]) == 'r' && tolower(word[1]) == 'o' && tolower(word[2]) == 'r') return 115;
	if (tolower(word[0]) == 's' && tolower(word[1]) == 'g' && tolower(word[2]) == 'n') return 46;
	if (tolower(word[0]) == 's' && tolower(word[1]) == 'h' && tolower(word[2]) == 'l') return 110;
	if (tolower(word[0]) == 's' && tolower(word[1]) == 'h' && tolower(word[2]) == 'r') return 112;
	if (tolower(word[0]) == 's' && tolower(word[1]) == 'i' && tolower(word[2]) == 'n') return 51;
	if (tolower(word[0]) == 's' && tolower(word[1]) == 'q' && tolower(word[2]) == 'r') return 50;
	if (tolower(word[0]) == 's' && tolower(word[1]) == 't' && tolower(word[2]) == 'r') return 147;
	if (tolower(word[0]) == 't' && tolower(word[1]) == 'a' && tolower(word[2]) == 'n') return 53;
	if (tolower(word[0]) == 't' && tolower(word[1]) == 'r' && word[2] == '$') return 213;
	if (tolower(word[0]) == 'v' && tolower(word[1]) == 'a' && tolower(word[2]) == 'l') return 145;
	return -1;
__04:
	if (tolower(word[0]) == 'a' && tolower(word[1]) == 'c' && tolower(word[2]) == 'o' && tolower(word[3]) == 's') return 59;
	if (tolower(word[0]) == 'a' && tolower(word[1]) == 's' && tolower(word[2]) == 'i' && tolower(word[3]) == 'n') return 57;
	if (tolower(word[0]) == 'a' && tolower(word[1]) == 't' && tolower(word[2]) == 'a' && tolower(word[3]) == 'n') return 55;
	if (tolower(word[0]) == 'a' && tolower(word[1]) == 'c' && tolower(word[2]) == 's' && tolower(word[3]) == 'h') return 68;
	if (tolower(word[0]) == 'a' && tolower(word[1]) == 's' && tolower(word[2]) == 'n' && tolower(word[3]) == 'h') return 66;
	if (tolower(word[0]) == 'a' && tolower(word[1]) == 't' && tolower(word[2]) == 'n' && word[3] == '2') return 91;
	if (tolower(word[0]) == 'a' && tolower(word[1]) == 't' && tolower(word[2]) == 'n' && tolower(word[3]) == 'h') return 70;
	if (tolower(word[0]) == 'b' && tolower(word[1]) == 'c' && tolower(word[2]) == 'h' && tolower(word[3]) == 'g') return 109;
	if (tolower(word[0]) == 'b' && tolower(word[1]) == 'c' && tolower(word[2]) == 'l' && tolower(word[3]) == 'r') return 106;
	if (tolower(word[0]) == 'b' && tolower(word[1]) == 's' && tolower(word[2]) == 'e' && tolower(word[3]) == 't') return 107;
	if (tolower(word[0]) == 'b' && tolower(word[1]) == 't' && tolower(word[2]) == 's' && tolower(word[3]) == 't') return 108;
	if (tolower(word[0]) == 'b' && tolower(word[1]) == 'i' && tolower(word[2]) == 'n' && word[3] == '$') return 141;
	if (tolower(word[0]) == 'c' && tolower(word[1]) == 'i' && tolower(word[2]) == 'n' && tolower(word[3]) == 't') return 131;
	if (tolower(word[0]) == 'c' && tolower(word[1]) == 's' && tolower(word[2]) == 't' && tolower(word[3]) == 'r') return 137;
	if (tolower(word[0]) == 'c' && tolower(word[1]) == 'e' && tolower(word[2]) == 'i' && tolower(word[3]) == 'l') return 79;
	if (tolower(word[0]) == 'c' && tolower(word[1]) == 'h' && tolower(word[2]) == 'r' && word[3] == '$') return 25;
	if (tolower(word[0]) == 'c' && tolower(word[1]) == 'o' && tolower(word[2]) == 'm' && tolower(word[3]) == 'p') return 36;
	if (tolower(word[0]) == 'c' && tolower(word[1]) == 'o' && tolower(word[2]) == 'n' && tolower(word[3]) == 'v') return 37;
	if (tolower(word[0]) == 'c' && tolower(word[1]) == 'o' && tolower(word[2]) == 's' && tolower(word[3]) == 'h') return 64;
	if (tolower(word[0]) == 'd' && tolower(word[1]) == 'a' && tolower(word[2]) == 't' && tolower(word[3]) == 'e') return 160;
	if (tolower(word[0]) == 'e' && tolower(word[1]) == 'v' && tolower(word[2]) == 'a' && tolower(word[3]) == 'l') return 164;
	if (tolower(word[0]) == 'e' && tolower(word[1]) == 'x' && tolower(word[2]) == 'p' && word[3] == '2') return 72;
	if (tolower(word[0]) == 'e' && tolower(word[1]) == 'x' && tolower(word[2]) == 'p' && tolower(word[3]) == 'm') return 76;
	if (tolower(word[0]) == 'f' && tolower(word[1]) == 'r' && tolower(word[2]) == 'a' && tolower(word[3]) == 'c') return 47;
	if (tolower(word[0]) == 'f' && tolower(word[1]) == 'r' && tolower(word[2]) == 'e' && tolower(word[3]) == 'e') return 205;
	if (tolower(word[0]) == 'h' && tolower(word[1]) == 'e' && tolower(word[2]) == 'x' && word[3] == '$') return 143;
	if (tolower(word[0]) == 'h' && tolower(word[1]) == 'o' && tolower(word[2]) == 'u' && tolower(word[3]) == 'r') return 155;
	if (tolower(word[0]) == 'h' && tolower(word[1]) == 't' && tolower(word[2]) == 'm' && tolower(word[3]) == 'l') return 218;
	if (tolower(word[0]) == 'i' && tolower(word[1]) == 'n' && tolower(word[2]) == 't' && word[3] == '@') return 226;
	if (tolower(word[0]) == 'l' && tolower(word[1]) == 'e' && tolower(word[2]) == 'f' && tolower(word[3]) == 't') return 1;
	if (tolower(word[0]) == 'l' && tolower(word[1]) == 'o' && tolower(word[2]) == 'g' && word[3] == '2') return 74;
	if (tolower(word[0]) == 'l' && tolower(word[1]) == 'o' && tolower(word[2]) == 'g' && tolower(word[3]) == 'p') return 77;
	if (tolower(word[0]) == 'm' && tolower(word[1]) == 'i' && tolower(word[2]) == 'd' && word[3] == '$') return 2;
	if (tolower(word[0]) == 'r' && tolower(word[1]) == 'd' && tolower(word[2]) == 'i' && tolower(word[3]) == 'r') return 201;
	if (tolower(word[0]) == 's' && tolower(word[1]) == 'c' && tolower(word[2]) == 'a' && tolower(word[3]) == 'n') return 35;
	if (tolower(word[0]) == 's' && tolower(word[1]) == 'e' && tolower(word[2]) == 'e' && tolower(word[3]) == 'k') return 186;
	if (tolower(word[0]) == 's' && tolower(word[1]) == 'i' && tolower(word[2]) == 'n' && tolower(word[3]) == 'h') return 63;
	if (tolower(word[0]) == 's' && tolower(word[1]) == 't' && tolower(word[2]) == 'a' && tolower(word[3]) == 't') return 195;
	if (tolower(word[0]) == 's' && tolower(word[1]) == 't' && tolower(word[2]) == 'r' && word[3] == '$') return 146;
	if (tolower(word[0]) == 's' && tolower(word[1]) == 't' && tolower(word[2]) == 'r' && word[3] == '@') return 207;
	if (tolower(word[0]) == 't' && tolower(word[1]) == 'a' && tolower(word[2]) == 'n' && tolower(word[3]) == 'h') return 65;
	if (tolower(word[0]) == 't' && tolower(word[1]) == 'e' && tolower(word[2]) == 'm' && tolower(word[3]) == 'p') return 197;
	if (tolower(word[0]) == 't' && tolower(word[1]) == 'i' && tolower(word[2]) == 'm' && tolower(word[3]) == 'e') return 161;
	if (tolower(word[0]) == 't' && tolower(word[1]) == 'r' && tolower(word[2]) == 'i' && tolower(word[3]) == 'm') return 12;
	if (tolower(word[0]) == 'w' && tolower(word[1]) == 'e' && tolower(word[2]) == 'e' && tolower(word[3]) == 'k') return 159;
	if (tolower(word[0]) == 'y' && tolower(word[1]) == 'e' && tolower(word[2]) == 'a' && tolower(word[3]) == 'r') return 152;
	return -1;
__05:
	if (word[0] == '.' && tolower(word[1]) == 'c' && tolower(word[2]) == 'o' && tolower(word[3]) == 'p' && tolower(word[4]) == 'y') return 191;
	if (word[0] == '.' && tolower(word[1]) == 'e' && tolower(word[2]) == 'x' && tolower(word[3]) == 'e' && tolower(word[4]) == 'c') return 202;
	if (word[0] == '.' && tolower(word[1]) == 'k' && tolower(word[2]) == 'i' && tolower(word[3]) == 'l' && tolower(word[4]) == 'l') return 187;
	if (word[0] == '.' && tolower(word[1]) == 'l' && tolower(word[2]) == 'i' && tolower(word[3]) == 'n' && tolower(word[4]) == 'k') return 192;
	if (word[0] == '.' && tolower(word[1]) == 'l' && tolower(word[2]) == 'o' && tolower(word[3]) == 'c' && tolower(word[4]) == 'k') return 179;
	if (word[0] == '.' && tolower(word[1]) == 'm' && tolower(word[2]) == 'o' && tolower(word[3]) == 'v' && tolower(word[4]) == 'e') return 190;
	if (word[0] == '.' && tolower(word[1]) == 'o' && tolower(word[2]) == 'p' && tolower(word[3]) == 'e' && tolower(word[4]) == 'n') return 168;
	if (word[0] == '.' && tolower(word[1]) == 'r' && tolower(word[2]) == 'e' && tolower(word[3]) == 'a' && tolower(word[4]) == 'd') return 174;
	if (word[0] == '.' && tolower(word[1]) == 'w' && tolower(word[2]) == 'a' && tolower(word[3]) == 'i' && tolower(word[4]) == 't') return 167;
	if (tolower(word[0]) == 'a' && tolower(word[1]) == 'c' && tolower(word[2]) == 'o' && tolower(word[3]) == 's' && tolower(word[4]) == 'h') return 69;
	if (tolower(word[0]) == 'a' && tolower(word[1]) == 's' && tolower(word[2]) == 'i' && tolower(word[3]) == 'n' && tolower(word[4]) == 'h') return 67;
	if (tolower(word[0]) == 'a' && tolower(word[1]) == 't' && tolower(word[2]) == 'a' && tolower(word[3]) == 'n' && word[4] == '2') return 90;
	if (tolower(word[0]) == 'a' && tolower(word[1]) == 't' && tolower(word[2]) == 'a' && tolower(word[3]) == 'n' && tolower(word[4]) == 'h') return 71;
	if (tolower(word[0]) == 'a' && tolower(word[1]) == 'l' && tolower(word[2]) == 'l' && tolower(word[3]) == 'o' && tolower(word[4]) == 'c') return 204;
	if (tolower(word[0]) == 'b' && tolower(word[1]) == 'o' && tolower(word[2]) == 'o' && tolower(word[3]) == 'l' && word[4] == '@') return 222;
	if (tolower(word[0]) == 'b' && tolower(word[1]) == 'y' && tolower(word[2]) == 't' && tolower(word[3]) == 'e' && word[4] == '@') return 224;
	if (tolower(word[0]) == 'c' && tolower(word[1]) == 'b' && tolower(word[2]) == 'o' && tolower(word[3]) == 'o' && tolower(word[4]) == 'l') return 127;
	if (tolower(word[0]) == 'c' && tolower(word[1]) == 'b' && tolower(word[2]) == 'y' && tolower(word[3]) == 't' && tolower(word[4]) == 'e') return 129;
	if (tolower(word[0]) == 'c' && tolower(word[1]) == 'd' && tolower(word[2]) == 'a' && tolower(word[3]) == 't' && tolower(word[4]) == 'e') return 136;
	if (tolower(word[0]) == 'c' && tolower(word[1]) == 'l' && tolower(word[2]) == 'o' && tolower(word[3]) == 'n' && tolower(word[4]) == 'g') return 133;
	if (tolower(word[0]) == 'c' && tolower(word[1]) == 'o' && tolower(word[2]) == 'n' && tolower(word[3]) == 'v' && word[4] == '$') return 38;
	if (tolower(word[0]) == 'd' && tolower(word[1]) == 'c' && tolower(word[2]) == 'o' && tolower(word[3]) == 'n' && tolower(word[4]) == 'v') return 41;
	if (tolower(word[0]) == 'd' && tolower(word[1]) == 'a' && tolower(word[2]) == 't' && tolower(word[3]) == 'e' && word[4] == '@') return 231;
	if (tolower(word[0]) == 'd' && tolower(word[1]) == 'f' && tolower(word[2]) == 'r' && tolower(word[3]) == 'e' && tolower(word[4]) == 'e') return 196;
	if (tolower(word[0]) == 'e' && tolower(word[1]) == 'x' && tolower(word[2]) == 'i' && tolower(word[3]) == 's' && tolower(word[4]) == 't') return 193;
	if (tolower(word[0]) == 'e' && tolower(word[1]) == 'x' && tolower(word[2]) == 'p' && word[3] == '1' && word[4] == '0') return 73;
	if (tolower(word[0]) == 'f' && tolower(word[1]) == 'l' && tolower(word[2]) == 'o' && tolower(word[3]) == 'o' && tolower(word[4]) == 'r') return 78;
	if (tolower(word[0]) == 'h' && tolower(word[1]) == 't' && tolower(word[2]) == 'm' && tolower(word[3]) == 'l' && word[4] == '$') return 219;
	if (tolower(word[0]) == 'i' && tolower(word[1]) == 'n' && tolower(word[2]) == 's' && tolower(word[3]) == 't' && tolower(word[4]) == 'r') return 28;
	if (tolower(word[0]) == 'i' && tolower(word[1]) == 's' && tolower(word[2]) == 'd' && tolower(word[3]) == 'i' && tolower(word[4]) == 'r') return 199;
	if (tolower(word[0]) == 'l' && tolower(word[1]) == 'c' && tolower(word[2]) == 'a' && tolower(word[3]) == 's' && tolower(word[4]) == 'e') return 24;
	if (tolower(word[0]) == 'l' && tolower(word[1]) == 't' && tolower(word[2]) == 'r' && tolower(word[3]) == 'i' && tolower(word[4]) == 'm') return 14;
	if (tolower(word[0]) == 'l' && tolower(word[1]) == 'e' && tolower(word[2]) == 'f' && tolower(word[3]) == 't' && word[4] == '$') return 0;
	if (tolower(word[0]) == 'l' && tolower(word[1]) == 'o' && tolower(word[2]) == 'g' && word[3] == '1' && word[4] == '0') return 62;
	if (tolower(word[0]) == 'l' && tolower(word[1]) == 'o' && tolower(word[2]) == 'n' && tolower(word[3]) == 'g' && word[4] == '@') return 228;
	if (tolower(word[0]) == 'l' && tolower(word[1]) == 'o' && tolower(word[2]) == 'w' && tolower(word[3]) == 'e' && tolower(word[4]) == 'r') return 22;
	if (tolower(word[0]) == 'm' && tolower(word[1]) == 'o' && tolower(word[2]) == 'n' && tolower(word[3]) == 't' && tolower(word[4]) == 'h') return 153;
	if (tolower(word[0]) == 'q' && tolower(word[1]) == 'u' && tolower(word[2]) == 'o' && tolower(word[3]) == 't' && tolower(word[4]) == 'e') return 214;
	if (tolower(word[0]) == 'r' && tolower(word[1]) == 't' && tolower(word[2]) == 'r' && tolower(word[3]) == 'i' && tolower(word[4]) == 'm') return 16;
	if (tolower(word[0]) == 'r' && tolower(word[1]) == 'i' && tolower(word[2]) == 'g' && tolower(word[3]) == 'h' && tolower(word[4]) == 't') return 5;
	if (tolower(word[0]) == 'r' && tolower(word[1]) == 'o' && tolower(word[2]) == 'u' && tolower(word[3]) == 'n' && tolower(word[4]) == 'd') return 81;
	if (tolower(word[0]) == 's' && tolower(word[1]) == 'c' && tolower(word[2]) == 'o' && tolower(word[3]) == 'n' && tolower(word[4]) == 'v') return 39;
	if (tolower(word[0]) == 's' && tolower(word[1]) == 'h' && tolower(word[2]) == 'e' && tolower(word[3]) == 'l' && tolower(word[4]) == 'l') return 216;
	if (tolower(word[0]) == 's' && tolower(word[1]) == 'p' && tolower(word[2]) == 'a' && tolower(word[3]) == 'c' && tolower(word[4]) == 'e') return 8;
	if (tolower(word[0]) == 's' && tolower(word[1]) == 'p' && tolower(word[2]) == 'l' && tolower(word[3]) == 'i' && tolower(word[4]) == 't') return 34;
	if (tolower(word[0]) == 's' && tolower(word[1]) == 'u' && tolower(word[2]) == 'b' && tolower(word[3]) == 's' && tolower(word[4]) == 't') return 31;
	if (tolower(word[0]) == 't' && tolower(word[1]) == 'e' && tolower(word[2]) == 'm' && tolower(word[3]) == 'p' && word[4] == '$') return 198;
	if (tolower(word[0]) == 't' && tolower(word[1]) == 'i' && tolower(word[2]) == 'm' && tolower(word[3]) == 'e' && tolower(word[4]) == 'r') return 150;
	if (tolower(word[0]) == 't' && tolower(word[1]) == 'r' && tolower(word[2]) == 'i' && tolower(word[3]) == 'm' && word[4] == '$') return 11;
	if (tolower(word[0]) == 'u' && tolower(word[1]) == 'c' && tolower(word[2]) == 'a' && tolower(word[3]) == 's' && tolower(word[4]) == 'e') return 20;
	if (tolower(word[0]) == 'u' && tolower(word[1]) == 'p' && tolower(word[2]) == 'p' && tolower(word[3]) == 'e' && tolower(word[4]) == 'r') return 18;
	return -1;
__06:
	if (word[0] == '.' && tolower(word[1]) == 'a' && tolower(word[2]) == 'r' && tolower(word[3]) == 'r' && tolower(word[4]) == 'a' && tolower(word[5]) == 'y') return 89;
	if (word[0] == '.' && tolower(word[1]) == 'c' && tolower(word[2]) == 'l' && tolower(word[3]) == 'o' && tolower(word[4]) == 's' && tolower(word[5]) == 'e') return 170;
	if (word[0] == '.' && tolower(word[1]) == 'd' && tolower(word[2]) == 'e' && tolower(word[3]) == 'b' && tolower(word[4]) == 'u' && tolower(word[5]) == 'g') return 166;
	if (word[0] == '.' && tolower(word[1]) == 'e' && tolower(word[2]) == 'r' && tolower(word[3]) == 'r' && tolower(word[4]) == 'o' && tolower(word[5]) == 'r') return 165;
	if (word[0] == '.' && tolower(word[1]) == 'f' && tolower(word[2]) == 'l' && tolower(word[3]) == 'u' && tolower(word[4]) == 's' && tolower(word[5]) == 'h') return 178;
	if (word[0] == '.' && tolower(word[1]) == 'i' && tolower(word[2]) == 'n' && tolower(word[3]) == 'p' && tolower(word[4]) == 'u' && tolower(word[5]) == 't') return 171;
	if (word[0] == '.' && tolower(word[1]) == 'm' && tolower(word[2]) == 'k' && tolower(word[3]) == 'd' && tolower(word[4]) == 'i' && tolower(word[5]) == 'r') return 188;
	if (word[0] == '.' && tolower(word[1]) == 'p' && tolower(word[2]) == 'r' && tolower(word[3]) == 'i' && tolower(word[4]) == 'n' && tolower(word[5]) == 't') return 173;
	if (word[0] == '.' && tolower(word[1]) == 'r' && tolower(word[2]) == 'm' && tolower(word[3]) == 'd' && tolower(word[4]) == 'i' && tolower(word[5]) == 'r') return 189;
	if (word[0] == '.' && tolower(word[1]) == 's' && tolower(word[2]) == 'h' && tolower(word[3]) == 'e' && tolower(word[4]) == 'l' && tolower(word[5]) == 'l') return 203;
	if (word[0] == '.' && tolower(word[1]) == 's' && tolower(word[2]) == 'l' && tolower(word[3]) == 'e' && tolower(word[4]) == 'e' && tolower(word[5]) == 'p') return 209;
	if (word[0] == '.' && tolower(word[1]) == 'w' && tolower(word[2]) == 'r' && tolower(word[3]) == 'i' && tolower(word[4]) == 't' && tolower(word[5]) == 'e') return 176;
	if (tolower(word[0]) == 'a' && tolower(word[1]) == 'c' && tolower(word[2]) == 'c' && tolower(word[3]) == 'e' && tolower(word[4]) == 's' && tolower(word[5]) == 's') return 194;
	if (tolower(word[0]) == 'c' && tolower(word[1]) == 'f' && tolower(word[2]) == 'l' && tolower(word[3]) == 'o' && tolower(word[4]) == 'a' && tolower(word[5]) == 't') return 135;
	if (tolower(word[0]) == 'c' && tolower(word[1]) == 's' && tolower(word[2]) == 'h' && tolower(word[3]) == 'o' && tolower(word[4]) == 'r' && tolower(word[5]) == 't') return 130;
	if (tolower(word[0]) == 'c' && tolower(word[1]) == 'h' && tolower(word[2]) == 'o' && tolower(word[3]) == 'o' && tolower(word[4]) == 's' && tolower(word[5]) == 'e') return 88;
	if (tolower(word[0]) == 'd' && tolower(word[1]) == 'c' && tolower(word[2]) == 'o' && tolower(word[3]) == 'n' && tolower(word[4]) == 'v' && word[5] == '$') return 42;
	if (tolower(word[0]) == 'f' && tolower(word[1]) == 'l' && tolower(word[2]) == 'o' && tolower(word[3]) == 'a' && tolower(word[4]) == 't' && word[5] == '@') return 230;
	if (tolower(word[0]) == 'f' && tolower(word[1]) == 'o' && tolower(word[2]) == 'r' && tolower(word[3]) == 'm' && tolower(word[4]) == 'a' && tolower(word[5]) == 't') return 149;
	if (tolower(word[0]) == 'i' && tolower(word[1]) == 's' && tolower(word[2]) == 'd' && tolower(word[3]) == 'a' && tolower(word[4]) == 't' && tolower(word[5]) == 'e') return 122;
	if (tolower(word[0]) == 'i' && tolower(word[1]) == 's' && tolower(word[2]) == 'h' && tolower(word[3]) == 'e' && tolower(word[4]) == 'x' && tolower(word[5]) == 'a') return 102;
	if (tolower(word[0]) == 'i' && tolower(word[1]) == 's' && tolower(word[2]) == 'l' && tolower(word[3]) == 'o' && tolower(word[4]) == 'n' && tolower(word[5]) == 'g') return 120;
	if (tolower(word[0]) == 'i' && tolower(word[1]) == 's' && tolower(word[2]) == 'n' && tolower(word[3]) == 'u' && tolower(word[4]) == 'l' && tolower(word[5]) == 'l') return 124;
	if (tolower(word[0]) == 'l' && tolower(word[1]) == 'c' && tolower(word[2]) == 'a' && tolower(word[3]) == 's' && tolower(word[4]) == 'e' && word[5] == '$') return 23;
	if (tolower(word[0]) == 'l' && tolower(word[1]) == 't' && tolower(word[2]) == 'r' && tolower(word[3]) == 'i' && tolower(word[4]) == 'm' && word[5] == '$') return 13;
	if (tolower(word[0]) == 'l' && tolower(word[1]) == 'o' && tolower(word[2]) == 'w' && tolower(word[3]) == 'e' && tolower(word[4]) == 'r' && word[5] == '$') return 21;
	if (tolower(word[0]) == 'm' && tolower(word[1]) == 'i' && tolower(word[2]) == 'n' && tolower(word[3]) == 'u' && tolower(word[4]) == 't' && tolower(word[5]) == 'e') return 156;
	if (tolower(word[0]) == 'q' && tolower(word[1]) == 'u' && tolower(word[2]) == 'o' && tolower(word[3]) == 't' && tolower(word[4]) == 'e' && word[5] == '$') return 215;
	if (tolower(word[0]) == 'r' && tolower(word[1]) == 'i' && tolower(word[2]) == 'n' && tolower(word[3]) == 's' && tolower(word[4]) == 't' && tolower(word[5]) == 'r') return 29;
	if (tolower(word[0]) == 'r' && tolower(word[1]) == 't' && tolower(word[2]) == 'r' && tolower(word[3]) == 'i' && tolower(word[4]) == 'm' && word[5] == '$') return 15;
	if (tolower(word[0]) == 'r' && tolower(word[1]) == 'i' && tolower(word[2]) == 'g' && tolower(word[3]) == 'h' && tolower(word[4]) == 't' && word[5] == '$') return 4;
	if (tolower(word[0]) == 's' && tolower(word[1]) == 'c' && tolower(word[2]) == 'o' && tolower(word[3]) == 'n' && tolower(word[4]) == 'v' && word[5] == '$') return 40;
	if (tolower(word[0]) == 's' && tolower(word[1]) == 'e' && tolower(word[2]) == 'c' && tolower(word[3]) == 'o' && tolower(word[4]) == 'n' && tolower(word[5]) == 'd') return 157;
	if (tolower(word[0]) == 's' && tolower(word[1]) == 'h' && tolower(word[2]) == 'e' && tolower(word[3]) == 'l' && tolower(word[4]) == 'l' && word[5] == '$') return 217;
	if (tolower(word[0]) == 's' && tolower(word[1]) == 'h' && tolower(word[2]) == 'o' && tolower(word[3]) == 'r' && tolower(word[4]) == 't' && word[5] == '@') return 225;
	if (tolower(word[0]) == 's' && tolower(word[1]) == 'i' && tolower(word[2]) == 'z' && tolower(word[3]) == 'e' && tolower(word[4]) == 'o' && tolower(word[5]) == 'f') return 126;
	if (tolower(word[0]) == 's' && tolower(word[1]) == 'p' && tolower(word[2]) == 'a' && tolower(word[3]) == 'c' && tolower(word[4]) == 'e' && word[5] == '$') return 7;
	if (tolower(word[0]) == 's' && tolower(word[1]) == 't' && tolower(word[2]) == 'r' && tolower(word[3]) == 'i' && tolower(word[4]) == 'n' && tolower(word[5]) == 'g') return 10;
	if (tolower(word[0]) == 's' && tolower(word[1]) == 'u' && tolower(word[2]) == 'b' && tolower(word[3]) == 's' && tolower(word[4]) == 't' && word[5] == '$') return 30;
	if (tolower(word[0]) == 't' && tolower(word[1]) == 'y' && tolower(word[2]) == 'p' && tolower(word[3]) == 'e' && tolower(word[4]) == 'o' && tolower(word[5]) == 'f') return 125;
	if (tolower(word[0]) == 'u' && tolower(word[1]) == 'c' && tolower(word[2]) == 'a' && tolower(word[3]) == 's' && tolower(word[4]) == 'e' && word[5] == '$') return 19;
	if (tolower(word[0]) == 'u' && tolower(word[1]) == 'p' && tolower(word[2]) == 'p' && tolower(word[3]) == 'e' && tolower(word[4]) == 'r' && word[5] == '$') return 17;
	if (tolower(word[0]) == 'v' && tolower(word[1]) == 'a' && tolower(word[2]) == 'r' && tolower(word[3]) == 'p' && tolower(word[4]) == 't' && tolower(word[5]) == 'r') return 210;
	return -1;
__07:
	if (word[0] == '.' && tolower(word[1]) == 'u' && tolower(word[2]) == 'n' && tolower(word[3]) == 'l' && tolower(word[4]) == 'o' && tolower(word[5]) == 'c' && tolower(word[6]) == 'k') return 180;
	if (tolower(word[0]) == 'c' && tolower(word[1]) == 's' && tolower(word[2]) == 'i' && tolower(word[3]) == 'n' && tolower(word[4]) == 'g' && tolower(word[5]) == 'l' && tolower(word[6]) == 'e') return 134;
	if (tolower(word[0]) == 'c' && tolower(word[1]) == 's' && tolower(word[2]) == 't' && tolower(word[3]) == 'r' && tolower(word[4]) == 'i' && tolower(word[5]) == 'n' && tolower(word[6]) == 'g') return 138;
	if (tolower(word[0]) == 'd' && tolower(word[1]) == 'a' && tolower(word[2]) == 't' && tolower(word[3]) == 'e' && tolower(word[4]) == 'a' && tolower(word[5]) == 'd' && tolower(word[6]) == 'd') return 162;
	if (tolower(word[0]) == 'f' && tolower(word[1]) == 'o' && tolower(word[2]) == 'r' && tolower(word[3]) == 'm' && tolower(word[4]) == 'a' && tolower(word[5]) == 't' && word[6] == '$') return 148;
	if (tolower(word[0]) == 'i' && tolower(word[1]) == 's' && tolower(word[2]) == 'a' && tolower(word[3]) == 's' && tolower(word[4]) == 'c' && tolower(word[5]) == 'i' && tolower(word[6]) == 'i') return 95;
	if (tolower(word[0]) == 'i' && tolower(word[1]) == 's' && tolower(word[2]) == 'b' && tolower(word[3]) == 'l' && tolower(word[4]) == 'a' && tolower(word[5]) == 'n' && tolower(word[6]) == 'k') return 104;
	if (tolower(word[0]) == 'i' && tolower(word[1]) == 's' && tolower(word[2]) == 'd' && tolower(word[3]) == 'i' && tolower(word[4]) == 'g' && tolower(word[5]) == 'i' && tolower(word[6]) == 't') return 101;
	if (tolower(word[0]) == 'i' && tolower(word[1]) == 's' && tolower(word[2]) == 'f' && tolower(word[3]) == 'l' && tolower(word[4]) == 'o' && tolower(word[5]) == 'a' && tolower(word[6]) == 't') return 121;
	if (tolower(word[0]) == 'i' && tolower(word[1]) == 's' && tolower(word[2]) == 'l' && tolower(word[3]) == 'c' && tolower(word[4]) == 'a' && tolower(word[5]) == 's' && tolower(word[6]) == 'e') return 97;
	if (tolower(word[0]) == 'i' && tolower(word[1]) == 's' && tolower(word[2]) == 'l' && tolower(word[3]) == 'o' && tolower(word[4]) == 'w' && tolower(word[5]) == 'e' && tolower(word[6]) == 'r') return 98;
	if (tolower(word[0]) == 'i' && tolower(word[1]) == 's' && tolower(word[2]) == 'p' && tolower(word[3]) == 'u' && tolower(word[4]) == 'n' && tolower(word[5]) == 'c' && tolower(word[6]) == 't') return 105;
	if (tolower(word[0]) == 'i' && tolower(word[1]) == 's' && tolower(word[2]) == 's' && tolower(word[3]) == 'p' && tolower(word[4]) == 'a' && tolower(word[5]) == 'c' && tolower(word[6]) == 'e') return 103;
	if (tolower(word[0]) == 'i' && tolower(word[1]) == 's' && tolower(word[2]) == 'u' && tolower(word[3]) == 'c' && tolower(word[4]) == 'a' && tolower(word[5]) == 's' && tolower(word[6]) == 'e') return 99;
	if (tolower(word[0]) == 'i' && tolower(word[1]) == 's' && tolower(word[2]) == 'u' && tolower(word[3]) == 'p' && tolower(word[4]) == 'p' && tolower(word[5]) == 'e' && tolower(word[6]) == 'r') return 100;
	if (tolower(word[0]) == 'r' && tolower(word[1]) == 'e' && tolower(word[2]) == 'a' && tolower(word[3]) == 'l' && tolower(word[4]) == 'l' && tolower(word[5]) == 'o' && tolower(word[6]) == 'c') return 206;
	if (tolower(word[0]) == 'r' && tolower(word[1]) == 'e' && tolower(word[2]) == 'p' && tolower(word[3]) == 'l' && tolower(word[4]) == 'a' && tolower(word[5]) == 'c' && tolower(word[6]) == 'e') return 33;
	if (tolower(word[0]) == 's' && tolower(word[1]) == 'i' && tolower(word[2]) == 'n' && tolower(word[3]) == 'g' && tolower(word[4]) == 'l' && tolower(word[5]) == 'e' && word[6] == '@') return 229;
	if (tolower(word[0]) == 's' && tolower(word[1]) == 't' && tolower(word[2]) == 'r' && tolower(word[3]) == 'i' && tolower(word[4]) == 'n' && tolower(word[5]) == 'g' && word[6] == '$') return 9;
	if (tolower(word[0]) == 's' && tolower(word[1]) == 't' && tolower(word[2]) == 'r' && tolower(word[3]) == 'i' && tolower(word[4]) == 'n' && tolower(word[5]) == 'g' && word[6] == '@') return 208;
	if (tolower(word[0]) == 'u' && tolower(word[1]) == 'n' && tolower(word[2]) == 'q' && tolower(word[3]) == 'u' && tolower(word[4]) == 'o' && tolower(word[5]) == 't' && tolower(word[6]) == 'e') return 220;
	if (tolower(word[0]) == 'w' && tolower(word[1]) == 'e' && tolower(word[2]) == 'e' && tolower(word[3]) == 'k' && tolower(word[4]) == 'd' && tolower(word[5]) == 'a' && tolower(word[6]) == 'y') return 158;
	return -1;
__08:
	if (word[0] == '.' && tolower(word[1]) == 'e' && tolower(word[2]) == 'r' && tolower(word[3]) == 'r' && tolower(word[4]) == 'o' && tolower(word[5]) == 'r' && tolower(word[6]) == 't' && tolower(word[7]) == 'o') return 183;
	if (tolower(word[0]) == 'b' && tolower(word[1]) == 'o' && tolower(word[2]) == 'o' && tolower(word[3]) == 'l' && tolower(word[4]) == 'e' && tolower(word[5]) == 'a' && tolower(word[6]) == 'n' && word[7] == '@') return 223;
	if (tolower(word[0]) == 'c' && tolower(word[1]) == 'b' && tolower(word[2]) == 'o' && tolower(word[3]) == 'o' && tolower(word[4]) == 'l' && tolower(word[5]) == 'e' && tolower(word[6]) == 'a' && tolower(word[7]) == 'n') return 128;
	if (tolower(word[0]) == 'c' && tolower(word[1]) == 'i' && tolower(word[2]) == 'n' && tolower(word[3]) == 't' && tolower(word[4]) == 'e' && tolower(word[5]) == 'g' && tolower(word[6]) == 'e' && tolower(word[7]) == 'r') return 132;
	if (tolower(word[0]) == 'c' && tolower(word[1]) == 'p' && tolower(word[2]) == 'o' && tolower(word[3]) == 'i' && tolower(word[4]) == 'n' && tolower(word[5]) == 't' && tolower(word[6]) == 'e' && tolower(word[7]) == 'r') return 139;
	if (tolower(word[0]) == 'c' && tolower(word[1]) == 'v' && tolower(word[2]) == 'a' && tolower(word[3]) == 'r' && tolower(word[4]) == 'i' && tolower(word[5]) == 'a' && tolower(word[6]) == 'n' && tolower(word[7]) == 't') return 140;
	if (tolower(word[0]) == 'd' && tolower(word[1]) == 'a' && tolower(word[2]) == 't' && tolower(word[3]) == 'e' && tolower(word[4]) == 'd' && tolower(word[5]) == 'i' && tolower(word[6]) == 'f' && tolower(word[7]) == 'f') return 163;
	if (tolower(word[0]) == 'i' && tolower(word[1]) == 'n' && tolower(word[2]) == 't' && tolower(word[3]) == 'e' && tolower(word[4]) == 'g' && tolower(word[5]) == 'e' && tolower(word[6]) == 'r' && word[7] == '@') return 227;
	if (tolower(word[0]) == 'i' && tolower(word[1]) == 's' && tolower(word[2]) == 'l' && tolower(word[3]) == 'e' && tolower(word[4]) == 't' && tolower(word[5]) == 't' && tolower(word[6]) == 'e' && tolower(word[7]) == 'r') return 96;
	if (tolower(word[0]) == 'i' && tolower(word[1]) == 's' && tolower(word[2]) == 'n' && tolower(word[3]) == 'u' && tolower(word[4]) == 'm' && tolower(word[5]) == 'b' && tolower(word[6]) == 'e' && tolower(word[7]) == 'r') return 123;
	if (tolower(word[0]) == 'p' && tolower(word[1]) == 'o' && tolower(word[2]) == 'i' && tolower(word[3]) == 'n' && tolower(word[4]) == 't' && tolower(word[5]) == 'e' && tolower(word[6]) == 'r' && word[7] == '@') return 232;
	if (tolower(word[0]) == 'r' && tolower(word[1]) == 'e' && tolower(word[2]) == 'p' && tolower(word[3]) == 'l' && tolower(word[4]) == 'a' && tolower(word[5]) == 'c' && tolower(word[6]) == 'e' && word[7] == '$') return 32;
	if (tolower(word[0]) == 'u' && tolower(word[1]) == 'n' && tolower(word[2]) == 'q' && tolower(word[3]) == 'u' && tolower(word[4]) == 'o' && tolower(word[5]) == 't' && tolower(word[6]) == 'e' && word[7] == '$') return 221;
	return -1;
__09:
	if (word[0] == '.' && tolower(word[1]) == 'o' && tolower(word[2]) == 'u' && tolower(word[3]) == 't' && tolower(word[4]) == 'p' && tolower(word[5]) == 'u' && tolower(word[6]) == 't' && tolower(word[7]) == 't' && tolower(word[8]) == 'o') return 182;
	if (tolower(word[0]) == 'i' && tolower(word[1]) == 's' && tolower(word[2]) == 'b' && tolower(word[3]) == 'o' && tolower(word[4]) == 'o' && tolower(word[5]) == 'l' && tolower(word[6]) == 'e' && tolower(word[7]) == 'a' && tolower(word[8]) == 'n') return 118;
	if (tolower(word[0]) == 'i' && tolower(word[1]) == 's' && tolower(word[2]) == 'i' && tolower(word[3]) == 'n' && tolower(word[4]) == 't' && tolower(word[5]) == 'e' && tolower(word[6]) == 'g' && tolower(word[7]) == 'e' && tolower(word[8]) == 'r') return 119;
	return -1;
__10:
	if (word[0] == '.' && tolower(word[1]) == 'i' && tolower(word[2]) == 'n' && tolower(word[3]) == 'p' && tolower(word[4]) == 'u' && tolower(word[5]) == 't' && tolower(word[6]) == 'f' && tolower(word[7]) == 'r' && tolower(word[8]) == 'o' && tolower(word[9]) == 'm') return 181;
	if (word[0] == '.' && tolower(word[1]) == 'l' && tolower(word[2]) == 'i' && tolower(word[3]) == 'n' && tolower(word[4]) == 'e' && tolower(word[5]) == 'i' && tolower(word[6]) == 'n' && tolower(word[7]) == 'p' && tolower(word[8]) == 'u' && tolower(word[9]) == 't') return 172;
	if (word[0] == '.' && tolower(word[1]) == 'r' && tolower(word[2]) == 'a' && tolower(word[3]) == 'n' && tolower(word[4]) == 'd' && tolower(word[5]) == 'o' && tolower(word[6]) == 'm' && tolower(word[7]) == 'i' && tolower(word[8]) == 'z' && tolower(word[9]) == 'e') return 82;
	if (word[0] == '.' && tolower(word[1]) == 'r' && tolower(word[2]) == 'e' && tolower(word[3]) == 'a' && tolower(word[4]) == 'd' && tolower(word[5]) == 'b' && tolower(word[6]) == 'y' && tolower(word[7]) == 't' && tolower(word[8]) == 'e' && tolower(word[9]) == 's') return 175;
	return -1;
__11:
	if (word[0] == '.' && tolower(word[1]) == 'c' && tolower(word[2]) == 'o' && tolower(word[3]) == 'l' && tolower(word[4]) == 'l' && tolower(word[5]) == 'e' && tolower(word[6]) == 'c' && tolower(word[7]) == 't' && tolower(word[8]) == 'i' && tolower(word[9]) == 'o' && tolower(word[10]) == 'n') return 211;
	if (word[0] == '.' && tolower(word[1]) == 'o' && tolower(word[2]) == 'p' && tolower(word[3]) == 'e' && tolower(word[4]) == 'n' && tolower(word[5]) == 'm' && tolower(word[6]) == 'e' && tolower(word[7]) == 'm' && tolower(word[8]) == 'o' && tolower(word[9]) == 'r' && tolower(word[10]) == 'y') return 169;
	if (word[0] == '.' && tolower(word[1]) == 'w' && tolower(word[2]) == 'r' && tolower(word[3]) == 'i' && tolower(word[4]) == 't' && tolower(word[5]) == 'e' && tolower(word[6]) == 'b' && tolower(word[7]) == 'y' && tolower(word[8]) == 't' && tolower(word[9]) == 'e' && tolower(word[10]) == 's') return 177;
	return -1;
}
