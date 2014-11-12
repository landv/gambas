/***************************************************************************

  gbx_subr_string.c

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

#include "gb_common.h"
#include "gb_common_buffer.h"
#include "gb_common_case.h"

#include <ctype.h>
#include <regex.h>

#include "gb_pcode.h"
#include "gbx_value.h"
#include "gbx_subr.h"
#include "gbx_regexp.h"
#include "gbx_class.h"
#include "gbx_string.h"
#include "gbx_split.h"
#include "gbx_c_array.h"
#include "gbx_local.h"
#include "gbx_compare.h"
#include "gb.pcre.h"

//static int _count = 0;

static PCRE_INTERFACE PCRE;

static void init_pcre()
{
	static bool init = FALSE;
	
	if (init)
		return;
		
	COMPONENT_load(COMPONENT_create("gb.pcre"));
	LIBRARY_get_interface_by_name("gb.pcre", PCRE_INTERFACE_VERSION, &PCRE);
	init = TRUE;
}

//---------------------------------------------------------------------------

void SUBR_cat(ushort code)
{
	SUBR_ENTER();

	if (NPARAM == 2)
	{
		int len, len2;
		char *str;
		
		VALUE_conv_string(&PARAM[0]);
		len = PARAM[0]._string.len ;
		VALUE_conv_string(&PARAM[1]);
		len2 = PARAM[1]._string.len;
		
		#if 0
		if (EXEC_string_add)
		{
			EXEC_string_add = FALSE;
			
			str = PARAM[0]._string.addr;
			
			if (0 && PARAM[0].type == T_STRING && PARAM[0]._string.start == 0 && STRING_length(str) == len)
			{
				if (str && !STRING_extend_will_realloc(str, len + len2))
				{
					//_count++;
					//fprintf(stderr, "[%d] &= optimization: str = %p (%d) param2 = %p (%d)\n", _count, str, len, PARAM[1]._string.addr + PARAM[1]._string.start, len2);
					str = STRING_add(str, PARAM[1]._string.addr + PARAM[1]._string.start, len2);
					/*if (str != PARAM[0]._string.addr)
					{
						//fprintf(stderr, "--> %p !\n", str);
						BREAKPOINT();
					}*/
					RELEASE_STRING(&PARAM[1]);

					SP -= 2;
					//SP->type = T_STRING;
					//SP->_string.addr = str;
					//SP->_string.start = 0;
					SP->_string.len += len2;
					SP++;
					return;
				}
			}
			/*else
			{
				if (PARAM[0].type != T_STRING)
					fprintf(stderr, "PARAM[0].type == %ld\n", PARAM[0].type);
				else if (PARAM[0]._string.start)
					fprintf(stderr, "PARAM[0]._string.start == %d\n", PARAM[0]._string.start);
				else if (STRING_length(str) != len)
					fprintf(stderr, "len == %d / %d\n", len, STRING_length(str));
			}*/
		}
		#endif
		
		str = STRING_new(NULL, len + len2);

		//fprintf(stderr, "normal: str = %p p0 = %p p1 = %p\n", str, PARAM[0]._string.addr + PARAM[0]._string.start, PARAM[1]._string.addr + PARAM[1]._string.start);
		
		memcpy(str, PARAM[0]._string.addr + PARAM[0]._string.start, len);
		memcpy(&str[len], PARAM[1]._string.addr + PARAM[1]._string.start, len2);
		
		RELEASE_STRING(&PARAM[0]);
		RELEASE_STRING(&PARAM[1]);

		SP -= 2;
		SP->type = T_STRING;
		SP->_string.addr = str;
		SP->_string.start = 0;
		SP->_string.len = len + len2;
		SP++;
	}
	else
	{
		int i;
		int len, len_cat;
		char *str, *ptr;

		len_cat = 0;

		for (i = 0; i < NPARAM; i++)
		{
			VALUE_conv_string(&PARAM[i]);
			len_cat += PARAM[i]._string.len;
		}

		str = STRING_new(NULL, len_cat);
		ptr = str;

		i = NPARAM;
		while (i--)
		{
			len = PARAM->_string.len;
			
			if (len)
			{
				memcpy(ptr, PARAM->_string.addr + PARAM->_string.start, len);
				ptr += len;
			}
			
			RELEASE_STRING(PARAM);
			PARAM++;
		}

		SP -= NPARAM;
		SP->type = T_STRING;
		SP->_string.addr = str;
		SP->_string.start = 0;
		SP->_string.len = len_cat;
		SP++;
	}
}


void SUBR_file(ushort code)
{
	int i;
	int length;
	char *addr;
	int len;
	char *str, *ptr;
	bool slash;

	SUBR_ENTER();

	length = 0;
	slash = FALSE;

	for (i = 0; i < NPARAM; i++)
	{
		/*VALUE_conv(&PARAM[i], T_STRING);*/
		SUBR_get_string_len(&PARAM[i], &addr, &len);

		if (len > 0)
		{
			if (length > 0)
			{
				if (!slash && addr[0] != '/')
					length++;
				else if (slash && addr[0] == '/')
					length--;
			}

			slash = addr[len - 1] == '/';

			length += len;
		}

	}

	str = STRING_new(NULL, length);
	ptr = str;
	slash = FALSE;

	i = NPARAM;
	while (i--)
	{
		if (PARAM->type != T_NULL)
		{
			VALUE_get_string(PARAM, &addr, &len);
			
			if (len > 0)
			{
				if (ptr > str)
				{
					if (!slash && *addr != '/')
						*ptr++ = '/';
					else if (slash && *addr == '/')
						ptr--;
				}
				
				slash = addr[len - 1] == '/';
				
				memcpy(ptr, addr, len);
				ptr += len;
			}

			RELEASE_STRING(PARAM);
		}
		
		PARAM++;
	}

	SP -= NPARAM;
	SP->type = T_STRING;
	SP->_string.addr = str;
	SP->_string.start = 0;
	SP->_string.len = length;
	SP++;
}



void SUBR_space(void)
{
	int len;

	SUBR_ENTER_PARAM(1);

	SUBR_check_integer(PARAM);
	len = PARAM->_integer.value;

	if (len < 0)
		THROW(E_ARG);

	if (len == 0)
	{
		STRING_void_value(&SP[-1]);
	}
	else
	{
		char *str = STRING_new(NULL, len);
		memset(str, ' ', len);
		SP--;
		SP->type = T_STRING;
		SP->_string.addr = str;
		SP->_string.start = 0;
		SP->_string.len = len;
		SP++;
	}

	//SUBR_LEAVE();
}



void SUBR_string(void)
{
	int i;
	char *d;
	char *s;
	int ld, ls;

	SUBR_ENTER_PARAM(2);

	SUBR_check_integer(PARAM);
	SUBR_get_string_len(&PARAM[1], &s, &ls);

	ld = PARAM->_integer.value * ls;
	if (ld < 0)
		THROW(E_ARG);

	if (ld == 0)
	{
		STRING_void_value(RETURN);
	}
	else
	{
		STRING_new_temp_value(RETURN, NULL, ld);
		d = RETURN->_string.addr;

		for (i = 0; i < PARAM->_integer.value; i++)
		{
			memcpy(d, s, ls);
			d += ls;
		}

		*d = 0;
	}

	SUBR_LEAVE();
}


void SUBR_trim(ushort code)
{
	unsigned char *str;
	bool left, right;

	SUBR_GET_PARAM(1);

	if (SUBR_check_string(PARAM))
	{
		VOID_STRING(PARAM);
		return;
	}

	code &= 0x1F;
	left = (code == 0 || code == 1);
	right = (code == 0 || code == 2);

	if (PARAM->_string.len > 0)
	{
		str = (uchar *)&PARAM->_string.addr[PARAM->_string.start];

		if (left)
		{
			while (PARAM->_string.len > 0 && *str <= ' ')
			{
				PARAM->_string.start++;
				PARAM->_string.len--;
				str++;
			}
		}

		if (right)
		{
			while (PARAM->_string.len > 0 && str[PARAM->_string.len - 1] <= ' ')
			{
				PARAM->_string.len--;
			}
		}
	}
}

void SUBR_upper(ushort code)
{
	char *str;
	int len, i;
	
	SUBR_ENTER_PARAM(1);
	
	if (SUBR_check_string(PARAM))
	{
		VOID_STRING(&SP[-1]);
	}
	else
	{
		len = PARAM->_string.len;
		if (len > 0)
		{
			str = STRING_new(&PARAM->_string.addr[PARAM->_string.start], PARAM->_string.len);
		
			if (code & 0x3F)
			{
				for (i = 0; i < len; i++)
					str[i] = tolower(str[i]);
			}
			else
			{
				for (i = 0; i < len; i++)
					str[i] = toupper(str[i]);
			}
			
			SP--;
			RELEASE_STRING(SP);
			SP->type = T_STRING;
			SP->_string.addr = str;
			SP->_string.start = 0;
			SP->_string.len = len;
			SP++;
		}
	}
}

void SUBR_lower(void)
{
	SUBR_upper(1);
}

void SUBR_chr(void)
{
	int car;

	SUBR_GET_PARAM(1);

	VALUE_conv_integer(PARAM);

	car = PARAM->_integer.value;
	if (car < 0 || car > 255)
		THROW(E_ARG);

	STRING_char_value(PARAM, car);
}

void SUBR_asc(ushort code)
{
	int pos = 0;

	SUBR_ENTER();

	if (!SUBR_check_string(PARAM))
	{
		pos = 1;
		if (NPARAM == 2)
		{
			SUBR_check_integer(&PARAM[1]);
			pos = PARAM[1]._integer.value;
		}

		if (pos < 1 || pos > PARAM->_string.len)
			pos = 0;
		else
			pos = (unsigned char)PARAM->_string.addr[PARAM->_string.start + pos - 1];
	}

	RETURN->type = T_INTEGER;
	RETURN->_integer.value = pos;

	SUBR_LEAVE();
}

void SUBR_instr(ushort code)
{
	bool right, nocase = FALSE;
	int is, pos;
	char *ps, *pp;
	int ls, lp;

	SUBR_ENTER();

	/* Knuth Morris Pratt one day maybe ? */

	pos = 0;

	if (SUBR_check_string(PARAM))
		goto __FOUND;

	if (SUBR_check_string(&PARAM[1]))
		goto __FOUND;

	lp = PARAM[1]._string.len;
	ls = PARAM->_string.len;

	right = ((code >> 8) == CODE_RINSTR);

	if (lp > ls) goto __FOUND;

	is = 0;

	if (NPARAM >= 3)
		is = SUBR_get_integer(&PARAM[2]);
	
	if (NPARAM == 4)
		nocase = SUBR_get_integer(&PARAM[3]) == GB_COMP_NOCASE;

	ps = PARAM->_string.addr + PARAM->_string.start;
	pp = PARAM[1]._string.addr + PARAM[1]._string.start;

	pos = STRING_search(ps, ls, pp, lp, is, right, nocase);

__FOUND:

	RELEASE_STRING(PARAM);
	RELEASE_STRING(&PARAM[1]);

	SP -= NPARAM;
	SP->type = T_INTEGER;
	SP->_integer.value = pos;
	SP++;
}

void SUBR_like(ushort code)
{
	static const void *jump[] = { &&__LIKE, &&__BEGINS, &&__ENDS, &&__MATCH };
	char *pattern;
	char *string;
	int len_pattern, len_string;
	bool ret = FALSE;

	SUBR_ENTER_PARAM(2);

	SUBR_get_string_len(&PARAM[0], &string, &len_string);
	SUBR_get_string_len(&PARAM[1], &pattern, &len_pattern);

	goto *jump[code & 0x3];
	
__LIKE:
	
	ret = REGEXP_match(pattern, len_pattern, string, len_string);
	goto __RETURN;

__BEGINS:
	
	if (len_pattern == 0)
		ret = TRUE;
	else if (len_pattern <= len_string)
		ret = STRING_equal_same(string, pattern, len_pattern);
	goto __RETURN;

__ENDS:
	
	if (len_pattern == 0)
		ret = TRUE;
	else if (len_pattern <= len_string)
		ret = STRING_equal_same(string + len_string - len_pattern, pattern, len_pattern);
	goto __RETURN;
	
__MATCH:

	init_pcre();
	ret = PCRE.Match(string, len_string, pattern, len_pattern, 0, 0);
	goto __RETURN;

__RETURN:
	
	RETURN->type = T_BOOLEAN;
	RETURN->_boolean.value = -(ret ^ !!(code & 0x4));

	SUBR_LEAVE();
}

static int subst_nparam;
static VALUE *subst_param;

static void get_subst(int np, char **str, int *len)
{
	if (np > 0 && np < subst_nparam)
		VALUE_get_string(&subst_param[np], str, len);
	else
	{
		*str = NULL;
		*len = 0;
	}
}

void SUBR_subst(ushort code)
{
	char *string;
	int len;
	int np;

	SUBR_ENTER();

	SUBR_get_string_len(&PARAM[0], &string, &len);

	for (np = 1; np < NPARAM; np++)
		VALUE_conv_string(&PARAM[np]);

	subst_param = PARAM;
	subst_nparam = NPARAM;

	string = STRING_subst(string, len, get_subst);

	/*for (np = 0; np < NPARAM; np++)
		RELEASE_STRING(&PARAM[np]);*/

	RETURN->type = T_STRING;
	RETURN->_string.addr = (char *)string;
	RETURN->_string.start = 0;
	RETURN->_string.len = STRING_length(string);

	SUBR_LEAVE();
}

void SUBR_replace(ushort code)
{
	char *ps;
	char *pp;
	char *pr;
	int ls, lp, lr;
	int pos;
	bool nocase = FALSE;

	SUBR_ENTER();

	SUBR_get_string_len(&PARAM[0], &ps, &ls);
	SUBR_get_string_len(&PARAM[1], &pp, &lp);
	SUBR_get_string_len(&PARAM[2], &pr, &lr);
	if (NPARAM == 4)
		nocase = SUBR_get_integer(&PARAM[3]) == GB_COMP_NOCASE;

	if (lp == 0 || ls == 0)
	{
		RELEASE(&PARAM[1]);
		RELEASE(&PARAM[2]);
		SP -= NPARAM;
		SP++;
		return;
	}

	if (lp == 1 && lr == 1)
	{
		char cp = *pp;
		char cr = *pr;
		
		ps = STRING_new_temp(ps, ls);
		
		for (pos = 0; pos < ls; pos++)
		{
			if (ps[pos] == cp)
				ps[pos] = cr;
		}
		
		RETURN->_string.addr = ps;
		RETURN->_string.len = ls;
	}
	else
	{
		STRING_start_len(ls);
	
		for(;;)
		{
			pos = STRING_search(ps, ls, pp, lp, 1, FALSE, nocase);
			if (pos == 0)
				break;

			pos--;

			if (pos > 0)
				STRING_make(ps, pos);

			STRING_make(pr, lr);

			pos += lp;

			ps += pos;
			ls -= pos;

			if (ls <= 0)
				break;
		}
		
		STRING_make(ps, ls);
		RETURN->_string.addr = STRING_end_temp();
		RETURN->_string.len = STRING_length(RETURN->_string.addr);
	}

	RETURN->type = T_STRING;
	RETURN->_string.start = 0;

	SUBR_LEAVE();
}

void SUBR_split(ushort code)
{
	CARRAY *array;
	char *str;
	int lstr;
	char *sep = NULL;
	int lsep = 0;
	char *esc = NULL;
	int lesc = 0;
	bool no_void = FALSE;
	bool keep_esc = FALSE;

	SUBR_ENTER();
	
	VALUE_conv_string(PARAM);
	VALUE_get_string(PARAM, &str, &lstr);

	if (NPARAM >= 2)
	{
		SUBR_get_string_len(&PARAM[1], &sep, &lsep);
		if (NPARAM >= 3)
		{
			SUBR_get_string_len(&PARAM[2], &esc, &lesc);
			if (NPARAM >= 4)
			{
				no_void = SUBR_get_boolean(&PARAM[3]);
				if (NPARAM == 5)
					keep_esc = SUBR_get_boolean(&PARAM[4]);
			}
		}
	}

	array = STRING_split(str, lstr, sep, lsep, esc, lesc, no_void, keep_esc);

	RETURN->_object.class = CLASS_StringArray;
	RETURN->_object.object = array;

	SUBR_LEAVE();
}

void SUBR_scan(void)
{
	CARRAY *array;
	char *str;
	int len_str;
	char *pat;
	int len_pat;

	SUBR_ENTER_PARAM(2);

	SUBR_get_string_len(&PARAM[0], &str, &len_str);
	SUBR_get_string_len(&PARAM[1], &pat, &len_pat);

	array = OBJECT_create(CLASS_StringArray, NULL, NULL, 0);

	if (len_str && len_pat)
		REGEXP_scan(array, pat, len_pat, str, len_str);

	RETURN->_object.class = CLASS_StringArray;
	RETURN->_object.object = array;

	SUBR_LEAVE();
}

void SUBR_iconv(void)
{
	char *str;
	const char *src;
	const char *dst;
	char *result;
	int len;

	SUBR_ENTER_PARAM(3);

	str = SUBR_get_string(&PARAM[0]);
	len = PARAM[0]._string.len;

	src = SUBR_get_string(&PARAM[1]);
	dst = SUBR_get_string(&PARAM[2]);

	STRING_conv(&result, str, len, src, dst, TRUE);

	if (!result)
		RETURN->type = T_NULL;
	else
	{
		RETURN->type = T_STRING;
		RETURN->_string.addr = result;
		RETURN->_string.start = 0;
		RETURN->_string.len = STRING_length(result);
	}

	SUBR_LEAVE();
}

void SUBR_sconv(ushort code)
{
	char *str;
	const char *src;
	const char *dst;
	char *result;
	int len;

	SUBR_ENTER_PARAM(1);

	if (LOCAL_is_UTF8)
		return;

	str = SUBR_get_string(&PARAM[0]);
	len = PARAM[0]._string.len;

	if (code & 0xF)
	{
		src = LOCAL_encoding;
		dst = SC_UTF8;
	}
	else
	{
		src = SC_UTF8;
		dst = LOCAL_encoding;
	}

	STRING_conv(&result, str, len, src, dst, TRUE);

	if (!result)
		RETURN->type = T_NULL;
	else
	{
		RETURN->type = T_STRING;
		RETURN->_string.addr = result;
		RETURN->_string.start = 0;
		RETURN->_string.len = STRING_length(result);
	}

	SUBR_LEAVE();
}

static int _is_ascii(int c)
{
	return (c & ~0x7F) == 0;
}

static int _is_letter(int c)
{
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

static int _is_lower(int c)
{
	return (c >= 'a' && c <= 'z');
}

static int _is_upper(int c)
{
	return (c >= 'A' && c <= 'Z');
}

static int _is_digit(int c)
{
	return (c >= '0' && c <= '9');
}

static int _is_hexa(int c)
{
	return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

static int _is_space(int c)
{
	return strchr(" \n\r\t\f\v", c) != NULL;
}

static int _is_blank(int c)
{
	return (c == 32 || c == '\t');
}

static int _is_punct(int c)
{
	return ((c > 32) && (c < 128) && !(_is_letter(c) || _is_digit(c)));
}

static int _is_alnum(int c)
{
	return _is_letter(c) || _is_digit(c);
}

void SUBR_is_chr(ushort code)
{
	static void *jump[] =
	{
		NULL, _is_ascii, _is_letter, _is_lower, _is_upper, _is_digit, _is_hexa, _is_space, _is_blank, _is_punct, _is_alnum
	};

	char *addr;
	int len;
	int i;
	int (*func)(int);

	SUBR_ENTER_PARAM(1);

	VALUE_conv_string(PARAM);

	//SUBR_get_string_len(PARAM, &addr, &len);
	VALUE_get_string(PARAM, &addr, &len);

	func = jump[code & 0x3F];

	i = len;
	while(i)
	{
		if (!(*func)(*addr++))
			break;
		i--;
	}

	RELEASE_STRING(PARAM);
	SP--;
	SP->type = T_BOOLEAN;
	SP->_boolean.value = (len > 0 && i == 0) ? -1 : 0;
	SP++;
}

void SUBR_tr(void)
{
	char *str;
	
	SUBR_ENTER_PARAM(1);
	
	VALUE_conv_string(&PARAM[0]);
	
	if (SUBR_check_string(PARAM))
		STRING_void_value(RETURN);
	else
	{
		str = STRING_new_temp(&PARAM->_string.addr[PARAM->_string.start], PARAM->_string.len);
		
		RETURN->type = T_CSTRING;
		RETURN->_string.addr = (char *)LOCAL_gettext(str);
		RETURN->_string.start = 0;
		RETURN->_string.len = strlen(RETURN->_string.addr);
	}
	
	SUBR_LEAVE();
}

static void make_hex_char(uchar c)
{
	static const char hex_digit[] = "0213456789ABCDEF";

	STRING_make_char(hex_digit[c >> 4]);
	STRING_make_char(hex_digit[c & 7]);
}

void SUBR_quote(ushort code)
{
	static void *jump[8] = { &&__QUOTE, &&__SHELL, &&__HTML, &&__BASE64, &&__URL , &&__ILLEGAL, &&__ILLEGAL, &&__ILLEGAL };
	char *str;
	int lstr;
	int i;
	unsigned char c;
	char buf[8];
	
	SUBR_ENTER_PARAM(1);

	VALUE_conv_string(&PARAM[0]);
	
	str = PARAM->_string.addr + PARAM->_string.start;
	lstr = PARAM->_string.len;
	
	STRING_start_len(lstr);
	
	goto *jump[code & 0x7];
	
__QUOTE:
	
	STRING_make_char('"');
	
	for (i = 0; i < lstr; i++)
	{
		c = str[i];
		//if (c >= ' ' && c <= 126 && c != '\\' && c != '"')
		if (c >= ' ' && c != '\\' && c != '"')
			STRING_make_char(c);
		else
		{
			STRING_make_char('\\');
			if (c == '\n')
				c = 'n';
			else if (c == '\r')
				c = 'r';
			else if (c == '\t')
				c = 't';
			else if (!(c == '"' || c == '\\'))
			{
				snprintf(buf, sizeof(buf), "x%02X", c);
				STRING_make(buf, 3);
				continue;
			}
			STRING_make_char(c);
		}
	}
	
	STRING_make_char('"');
	goto __END;
	
__SHELL:

	if (!LOCAL_is_UTF8)
	{
		char *conv;
  	STRING_conv(&conv, str, lstr, SC_UTF8, LOCAL_encoding, FALSE);
  	str = conv;
		lstr = str ? strlen(str) : 0;
	}
	
	// TODO: The following works with bash, but not with dash!
	
	STRING_make_char('\'');

	for (i = 0; i < lstr; i++)
	{
		c = str[i];
		if (c == '\'')
			STRING_make_char(c);
		STRING_make_char(c);
		/*
		if (c == '\n')
			STRING_make("$'\\n'", 5);
		else if (c == '\r')
			STRING_make("$'\\r'", 5);
		else if (c == '\t')
			STRING_make("$'\\t'", 5);
		else if (c < ' ') //|| (c > 126 && !LOCAL_is_UTF8))
		{
			snprintf(buf, sizeof(buf), "$'\\x%02X'", c);
			STRING_make(buf, 7);
		}
		else if ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || index(".-/_~", c) || c > 126)
			STRING_make_char(c);
		else
		{
			STRING_make_char('\\');
			STRING_make_char(c);
		}
		*/
	}
	
	STRING_make_char('\'');

	goto __END;

__HTML:

	for (i = 0; i < lstr; i++)
	{
		c = str[i];
		if (c == '&')
			STRING_make("&amp;", 5);
		else if (c == '<')
			STRING_make("&lt;", 4);
		else if (c == '>')
			STRING_make("&gt;", 4);
		else if (c == '"')
			STRING_make("&quot;", 6);
		else if (c == '\'')
			STRING_make("&#x27;", 6);
		else if (c == 0xC2 && (uchar)str[i + 1] == 0xA0)
		{
			STRING_make("&nbsp;", 6);
			i++;
		}
		else
			STRING_make_char(c);
	}
	
	goto __END;
	
__BASE64:
	{
		static const char base64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
		uchar *in;
		char *out = buf;
		
		for (i = 0; i < (lstr - 2); i += 3)
		{
			in = (uchar *)&str[i];
			out[0] = base64[in[0] >> 2];
			out[1] = base64[((in[0] & 0x03) << 4) | ((in[1] & 0xF0) >> 4)];
			out[2] = base64[((in[1] & 0x0F) << 2) | ((in[2] & 0xC0) >> 6)];
			out[3] = base64[in[2] & 0x3F];
			STRING_make(out, 4);
		}

		if (i < lstr)
		{
			in = (uchar *)&str[i];
			lstr -= i;
			out[0] = base64[in[0] >> 2];
			out[1] = base64[((in[0] & 0x03) << 4) | ((in[1] & 0xF0) >> 4)];
			out[2] = (lstr > 1 ? base64[((in[1] & 0x0F) << 2) | ((in[2] & 0xC0) >> 6) ] : '=');
			out[3] = (lstr > 2 ? base64[in[2] & 0x3F] : '=');
			STRING_make(out, 4);
		}
	}

	goto __END;

__URL:

	// Warning! '/' is not encoded, so that the function is more pratical, by supposing that no file url will have '/' in its name.

	for (i = 0; i < lstr; i++)
	{
		c = str[i];
		if (c == ' ')
			STRING_make_char('+');
		else if ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || index("-._~,$!/", c))
			STRING_make_char(c);
		else
		{
			STRING_make_char('%');
			make_hex_char(c);
		}
	}

	goto __END;

#if 0
__JAVASCRIPT:
	{
		STRING_make_char('\'');

		for (i = 0; i < lstr; i++)
		{
			c = str[i];
			if (c >= ' ' && c <= 126 && c != '\\' && c != '\'')
				STRING_make_char(c);
			else
			{
				STRING_make_char('\\');
				if (c == '\n')
					c = 'n';
				else if (c == '\r')
					c = 'r';
				else if (c == '\t')
					c = 't';
				else if (!(c == '\'' || c == '\\'))
				{
					snprintf(buf, sizeof(buf), "x%02X", c);
					STRING_make(buf, 3);
					continue;
				}
				STRING_make_char(c);
			}
		}

		STRING_make_char('\'');
	}
	
	goto __END;

#endif

__ILLEGAL:

	THROW_ILLEGAL();

__END:

	RETURN->type = T_STRING;
	RETURN->_string.addr = STRING_end_temp();
	RETURN->_string.start = 0;
	RETURN->_string.len = STRING_length(RETURN->_string.addr);

	SUBR_LEAVE();
}

static int read_hex_digit(unsigned char c)
{
	if (c >= '0' && c <= '9')
		return (c - '0');
	else if (c >= 'A' && c <= 'F')
		return (c - 'A' + 10);
	else if (c >= 'a' && c <= 'f')
		return (c - 'a' + 10);
	else
		return 0;
}

void SUBR_unquote(ushort code)
{
	static void *jump[4] = { &&__UNQUOTE, &&__FROM_BASE64, &&__FROM_URL, &&__ILLEGAL };
	
	char *str;
	int lstr;
	int i;
	unsigned char c;
	
	SUBR_ENTER_PARAM(1);

	VALUE_conv_string(&PARAM[0]);
	
	str = PARAM->_string.addr + PARAM->_string.start;
	lstr = PARAM->_string.len;
	
	STRING_start_len(lstr);
	
	goto *jump[code & 0x3];

__UNQUOTE:
	
	if (lstr >= 2 && str[0] == '"' && str[lstr - 1] == '"')
	{
		str++;
		lstr -= 2;
	}

	for (i = 0; i < lstr; i++)
	{
		c = str[i];
		if (c == '\\')
		{
			i++;
			if (i >= lstr)
				break;
			c = str[i];
			
			if (c == 'n')
				c = '\n';
			else if (c == 't')
				c = '\t';
			else if (c == 'r')
				c = '\r';
			else if (c == 'x')
			{
				if (i >= (lstr - 2))
					break;
					
				c = (read_hex_digit(str[i + 1]) << 4) + read_hex_digit(str[i + 2]);
				i += 2;
			}
		}
		
		STRING_make_char(c);
	}
	
	goto __END;
	
__FROM_BASE64:

	{
		char buf[4];
		unsigned char n = 0;
		
		for (i = 0; i < lstr; i++)
		{
			c = str[i];
			if (c >= 'A' && c <= 'Z')
				c = c - 'A';
			else if (c >= 'a' && c <= 'z')
				c = c - 'a' + 26;
			else if (c >= '0' && c <= '9')
				c = c - '0' + 52;
			else if (c == '+')
				c = 62;
			else if (c == '/')
				c = 63;
			else if (c == '=')
				break;
			else
				continue;
			
			switch (n & 3)
			{
				case 0: buf[0] = c << 2; break;
				case 1: buf[0] |= c >> 4; buf[1] = c << 4; break;
				case 2: buf[1] |= c >> 2; buf[2] = c << 6; break;
				case 3: buf[2] |= c; STRING_make(buf, 3); break;
			}
			n++;
		}

		if ((n & 3) > 1)
			STRING_make(buf, (n & 3) - 1);
	}
	
	goto __END;

__FROM_URL:

	for (i = 0; i < lstr; i++)
	{
		c = str[i];
		if (c == '+')
			c = ' ';
		else if (c == '%')
		{
			if (i >= (lstr - 2))
				break;

			c = (read_hex_digit(str[i + 1]) << 4) + read_hex_digit(str[i + 2]);
			i += 2;
		}

		STRING_make_char(c);
	}

	goto __END;

__ILLEGAL:

	THROW_ILLEGAL();
	
__END:
	
	RETURN->type = T_STRING;
	RETURN->_string.addr = STRING_end_temp();
	RETURN->_string.start = 0;
	RETURN->_string.len = STRING_length(RETURN->_string.addr);
	
	SUBR_LEAVE();
}

void SUBR_swap(ushort code)
{
	char *src, *dst;
	int len, i, j;

	if (!(code & 0xFF))
	{
		SUBR_move(1);
		return;
	}
	
	SUBR_ENTER();
	
	if (NPARAM == 2 && (SUBR_get_integer(&PARAM[1]) == GB_BIG_ENDIAN) == EXEC_big_endian)
	{
		SP--;
		return;
	}

	if (SUBR_check_string(PARAM))
		STRING_void_value(RETURN);
	else
	{
		len = PARAM->_string.len;
		if (len > 0)
		{
			src = PARAM->_string.addr + PARAM->_string.start;
			dst = STRING_new_temp(NULL, PARAM->_string.len);

			for (i = 0, j = len - 1; i < len; i++,j--)
				dst[i] = src[j];

			RETURN->type = T_STRING;
			RETURN->_string.addr = dst;
			RETURN->_string.start = 0;
			RETURN->_string.len = len;
		}
	}
	
	SUBR_LEAVE();
}


