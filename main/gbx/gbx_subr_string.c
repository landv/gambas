/***************************************************************************

  gbx_subr_string.c

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
#include "gbx_c_array.h"
#include "gbx_local.h"
#include "gbx_compare.h"

void SUBR_cat(ushort code)
{
	int i;
	int len, len_cat;
	char *str, *ptr;

	SUBR_ENTER();

	len_cat = 0;

	for (i = 0; i < NPARAM; i++)
	{
		VALUE_conv_string(&PARAM[i]);
		/*BORROW(&PARAM[i]);*/
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

	/*printf("\n");*/

	SP -= NPARAM;
	SP->type = T_STRING;
	SP->_string.addr = str;
	SP->_string.start = 0;
	SP->_string.len = len_cat;
	SP++;
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
		STRING_void_value(RETURN);
	}
	else
	{
		STRING_new_temp_value(RETURN, NULL, len);
		memset(RETURN->_string.addr, ' ', len);
	}

	SUBR_LEAVE();
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
		return;

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
		STRING_void_value(RETURN);
	else
	{
		len = PARAM->_string.len;
		if (len > 0)
		{
			str = STRING_new_temp(&PARAM->_string.addr[PARAM->_string.start], PARAM->_string.len);
		
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
			
			RETURN->type = T_STRING;
			RETURN->_string.addr = str;
			RETURN->_string.start = 0;
			RETURN->_string.len = len;
		}
	}
	
	SUBR_LEAVE();
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
	static const void *jump[] = { &&__LIKE, &&__BEGINS, &&__ENDS, &&__RETURN };
	char *pattern;
	char *string;
	int len_pattern, len_string;
	bool ret = FALSE;

	SUBR_ENTER_PARAM(2);

	SUBR_get_string_len(&PARAM[0], &string, &len_string);
	SUBR_get_string_len(&PARAM[1], &pattern, &len_pattern);

	goto *jump[code & 0x3];
	
__LIKE:
	
	ret = REGEXP_match(pattern, len_pattern, string, len_string) ? -1 : 0;
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

__RETURN:
	
	RETURN->type = T_BOOLEAN;
	RETURN->_boolean.value = ret;

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
	int is, pos;
	bool nocase = FALSE;

	SUBR_ENTER();

	SUBR_get_string_len(&PARAM[0], &ps, &ls);
	SUBR_get_string_len(&PARAM[1], &pp, &lp);
	SUBR_get_string_len(&PARAM[2], &pr, &lr);
	if (NPARAM == 4)
		nocase = SUBR_get_integer(&PARAM[3]) == GB_COMP_NOCASE;

	if (lp == 0)
	{
		RELEASE(&PARAM[1]);
		RELEASE(&PARAM[2]);
		SP -= NPARAM;
		SP++;
		return;
	}

	STRING_start_len(ls);
		
	if (ls > 0 && lp > 0)
	{
		is = 0;
	
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
	}

	RETURN->type = T_STRING;
	RETURN->_string.addr = STRING_end_temp();
	RETURN->_string.start = 0;
	RETURN->_string.len = STRING_length(RETURN->_string.addr);

	SUBR_LEAVE();
}

void SUBR_split(ushort code)
{
	CARRAY *array;
	char *str;
	int lstr;
	char *sep = "";
	char *esc = "";
	bool no_void = FALSE;
	bool keep_esc = FALSE;

	SUBR_ENTER();
	
	VALUE_conv_string(PARAM);
	VALUE_get_string(PARAM, &str, &lstr);

	//SUBR_get_string_len(&PARAM[0], &str, &lstr);
	
	if (NPARAM >= 2)
	{
		sep = SUBR_get_string(&PARAM[1]);
		if (NPARAM >= 3)
		{
			esc = SUBR_get_string(&PARAM[2]);
			if (NPARAM >= 4)
			{
				no_void = SUBR_get_boolean(&PARAM[3]);
				if (NPARAM == 5)
					keep_esc = SUBR_get_boolean(&PARAM[4]);
			}
		}
	}

	array = OBJECT_create(CLASS_StringArray, NULL, NULL, 0);

	if (lstr)
	{
		if (*sep) STRING_ref(sep);
		if (*esc) STRING_ref(esc);

		CARRAY_split(array, str, lstr, sep, esc, no_void, keep_esc);

		if (*sep) STRING_unref(&sep);
		if (*esc) STRING_unref(&esc);
	}

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
		dst = "UTF-8";
	}
	else
	{
		src = "UTF-8";
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

void SUBR_is_chr(ushort code)
{
	static void *jump[] =
	{
		NULL, _is_ascii, _is_letter, _is_lower, _is_upper, _is_digit, _is_hexa, _is_space, _is_blank, _is_punct
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

void SUBR_quote(ushort code)
{
	static void *jump[4] = { &&__QUOTE, &&__SHELL, &&__HTML, &&__QUOTE };
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
	
	goto *jump[code & 0x3];
	
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
				STRING_make_char('n');
			else if (c == '\r')
				STRING_make_char('r');
			else if (c == '\t')
				STRING_make_char('t');
			else if (c == '"')
				STRING_make_char('"');
			else if (c == '\\')
				STRING_make_char('\\');
			else
			{
				snprintf(buf, sizeof(buf), "x%02X", c);
				STRING_make(buf, 3);
			}
		}
	}
	
	STRING_make_char('"');
	goto __END;
	
__SHELL:

	if (!LOCAL_is_UTF8)
	{
		char *conv;
  	STRING_conv(&conv, str, lstr, "UTF-8", LOCAL_encoding, FALSE);
  	str = conv;
		lstr = str ? strlen(str) : 0;
	}
	
	for (i = 0; i < lstr; i++)
	{
		c = str[i];
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
	}
	
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
		else
			STRING_make_char(c);
	}
	
	goto __END;

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

void SUBR_unquote(void)
{
	char *str;
	int lstr;
	int i;
	unsigned char c;
	
	SUBR_ENTER_PARAM(1);
	
	VALUE_conv_string(&PARAM[0]);
	
	str = PARAM->_string.addr + PARAM->_string.start;
	lstr = PARAM->_string.len;
	
	STRING_start_len(lstr);
	
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
	
	RETURN->type = T_STRING;
	RETURN->_string.addr = STRING_end_temp();
	RETURN->_string.start = 0;
	RETURN->_string.len = STRING_length(RETURN->_string.addr);
	
	SUBR_LEAVE();
}
