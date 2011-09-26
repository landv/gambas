/***************************************************************************

	gbx_c_string.c

	(c) 2000-2011 Benoît Minisini <gambas@users.sourceforge.net>

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

#define __GBX_C_STRING_C

#include "gbx_info.h"

#ifndef GBX_INFO

#include "gb_common.h"
#include "gb_common_case.h"

#include <wctype.h>
#include <wchar.h>
#include <iconv.h>

#include "gb_error.h"
#include "gb_table.h"
#include "gbx_string.h"
#include "gbx_api.h"
#include "gbx_exec.h"
#include "gbx_subr.h"
#include "gbx_compare.h"
#include "gambas.h"

#include "gbx_c_string.h"

#define UNICODE_INVALID 0xFFFFFFFFU

static const char _char_length[256] =
{
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
	3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,4,4,4,4,4,4,4,4,5,5,5,5,6,6,1,1
};

/***************************************************************************/

#define utf8_get_char_length(_c) ((int)_char_length[(unsigned char)(_c)])

int STRING_get_utf8_char_length(unsigned char c)
{
	return utf8_get_char_length(c);
}

static int utf8_get_length(const char *str, int len)
{
	int ulen;
	int i;

	ulen = 0;

	for (i = 0; i < len; i++)
	{
		if ((str[i] & 0xC0) != 0x80)
			ulen++;
	}

	return ulen;
}

static uint utf8_to_unicode(const char *str, int len)
{
	uint unicode;
	
	switch (len)
	{
		case 2:
			unicode = (str[1] & 0x3F) + ((str[0] & 0x1F) << 6);
			if (unicode < 0x80)
				goto _INVALID;
			break;
			
		case 3:
			unicode = (str[2] & 0x3F) + ((str[1] & 0x3F) << 6) + ((str[0] & 0xF) << 12);
			if (unicode < 0x800)
				goto _INVALID;
			break;
		
		case 4:
			unicode = (str[3] & 0x3F) + ((str[2] & 0x3F) << 6) + ((str[1] & 0x3F) << 12) + ((str[0] & 0x7) << 18);
			if (unicode < 0x10000)
				goto _INVALID;
			break;
		
		case 5:
			unicode = (str[4] & 0x3F) + ((str[3] & 0x3F) << 6) + ((str[2] & 0x3F) << 12) + ((str[1] & 0x3F) << 18) + ((str[0] & 0x3) << 24);
			if (unicode < 0x200000)
				goto _INVALID;
			break;
		
		case 6:
			unicode = (str[5] & 0x3F) + ((str[4] & 0x3F) << 6) + ((str[3] & 0x3F) << 12) + ((str[2] & 0x3F) << 18) + ((str[1] & 0x3F) << 24) + ((str[0] & 0x1) << 30);
			if (unicode < 0x4000000)
				goto _INVALID;
			break;
			
		default:
			unicode = str[0];
			break;
	}
	
	return unicode;
	
_INVALID:

	return UNICODE_INVALID;
}

static void utf8_from_unicode(uint code, char *str)
{
	if (code < 0x80)
		str[0] = code;
	else if (code < 0x800)
	{
		str[0] = (code >> 6) | 0xC0;
		str[1] = (code & 0x3F) | 0x80;
	}
	else if (code < 0x10000)
	{
		str[0] = (code >> 12) | 0xE0;
		str[1] = ((code >> 6) & 0x3F) | 0x80;
		str[2] = (code & 0x3F) | 0x80;
	}
	else if (code < 0x200000)
	{
		str[0] = (code >> 18) | 0xF0;
		str[1] = ((code >> 12) & 0x3F) | 0x80;
		str[2] = ((code >> 6) & 0x3F) | 0x80;
		str[3] = (code & 0x3F) | 0x80;
	}
	else if (code < 0x4000000)
	{
		str[0] = (code >> 24) | 0xF8;
		str[1] = ((code >> 18) & 0x3F) | 0x80;
		str[2] = ((code >> 12) & 0x3F) | 0x80;
		str[3] = ((code >> 6) & 0x3F) | 0x80;
		str[4] = (code & 0x3F) | 0x80;
	}
	else if (code < 0x80000000)
	{
		str[0] = (code >> 31) | 0xFC;
		str[1] = ((code >> 24) & 0x3F)| 0x80;
		str[2] = ((code >> 18) & 0x3F) | 0x80;
		str[3] = ((code >> 12) & 0x3F) | 0x80;
		str[4] = ((code >> 6) & 0x3F) | 0x80;
		str[5] = (code & 0x3F) | 0x80;
	}
	else
		str[0] = 0;
}

/***************************************************************************/

char *STRING_utf8_current = NULL;
#define UTF8_POS_COUNT 256
static short _utf8_pos[UTF8_POS_COUNT];
static int _utf8_last_pos;

static int utf8_get_pos(const char *str, int len, int index)
{
	int i, pos;
	
	if (index <= 0)
		return 0;
	
	if (str != STRING_utf8_current)
	{
		STRING_utf8_current = (char *)str;
		_utf8_last_pos = 0;
		_utf8_pos[0] = 0;
	}
	
	if (index > len)
		index = len;
	
	if (index <= _utf8_last_pos)
		return _utf8_pos[index];
	
	pos = _utf8_pos[_utf8_last_pos];
	
	while (_utf8_last_pos < (UTF8_POS_COUNT - 1))
	{
		pos += utf8_get_char_length(str[pos]);
		_utf8_pos[++_utf8_last_pos] = pos;
		if (index == _utf8_last_pos)
			return pos;
	}
	
	for (i = UTF8_POS_COUNT - 1; i < index; i++)
		pos += utf8_get_char_length(str[pos]);
	
	return pos;
}


/***************************************************************************/

static int byte_to_index(const char *str, int len, int byte)
{
	if (byte <= 0)
		return 0;

	if (byte > len)
		byte = len;
	
	return utf8_get_length(str, byte);
}

static int index_to_byte(const char *str, int len, int index)
{
	if (index <= 0)
		return 0;

	return utf8_get_pos(str, len, index - 1) + 1;
}


BEGIN_METHOD(String_Pos, GB_STRING str; GB_INTEGER index)

	GB_ReturnInteger(index_to_byte(STRING(str), LENGTH(str), VARG(index)));

END_METHOD


BEGIN_METHOD(String_Len, GB_STRING str)

	GB_ReturnInteger(utf8_get_length(STRING(str), LENGTH(str)));

END_METHOD


BEGIN_METHOD(String_Index, GB_STRING str; GB_INTEGER pos)

	GB_ReturnInteger(byte_to_index(STRING(str), LENGTH(str), VARG(pos)));

END_METHOD


static void String_Mid(ushort code)
{
	char *str;
	int start, length;
	int len, ulen;
	bool null;

	SUBR_ENTER();

	null = SUBR_check_string(PARAM);

	VALUE_conv_integer(&PARAM[1]);
	start = PARAM[1]._integer.value - 1;

	if (start < 0)
		THROW(E_ARG);

	if (null)
		goto _SUBR_MID_FIN;
	
	str = PARAM->_string.addr + PARAM->_string.start;
	len = PARAM->_string.len;
	
	ulen = utf8_get_pos(str, len, start);
	if (ulen >= len)
	{
		VOID_STRING(PARAM);
		goto _SUBR_MID_FIN;
	}
	
	PARAM->_string.start += ulen;
	//str += ulen;
	//len -= ulen;
	
	if (NPARAM == 2)
	{
		ulen = len - ulen;
	}
	else
	{
		VALUE_conv_integer(&PARAM[2]);
		length = PARAM[2]._integer.value;

		if (length < 0)
			length += utf8_get_length(str, len) - start;
		
		if (length == 1)
			ulen = utf8_get_char_length(str[ulen]);
		else
			ulen = utf8_get_pos(str, len, start + length) - ulen;
	}
	
	if (ulen <= 0)
	{
		VOID_STRING(PARAM);
	}
	else
		PARAM->_string.len = ulen;

_SUBR_MID_FIN:

	SP -= NPARAM;
	SP++;
}


static void String_Left(ushort code)
{
	int val;
	char *str;
	int len, ulen;

	SUBR_ENTER();

	if (!SUBR_check_string(PARAM))
	{
		if (NPARAM == 1)
			val = 1;
		else
		{
			VALUE_conv_integer(&PARAM[1]);
			val = PARAM[1]._integer.value;
		}
	
		str = PARAM->_string.addr + PARAM->_string.start;
		len = PARAM->_string.len;
	
		if (val < 0)
			val += utf8_get_length(str, len);
		
		ulen = utf8_get_pos(str, len, val);
		PARAM->_string.len = ulen;
	}

	SP -= NPARAM;
	SP++;
}


static void String_Right(ushort code)
{
	int val;
	char *str;
	int len, ulen;

	SUBR_ENTER();

	if (!SUBR_check_string(PARAM))
	{
		if (NPARAM == 1)
			val = 1;
		else
		{
			VALUE_conv_integer(&PARAM[1]);
			val = PARAM[1]._integer.value;
		}
	
		str = PARAM->_string.addr + PARAM->_string.start;
		len = PARAM->_string.len;
	
		if (val < 0)
			val = (-val);
		else
			val = utf8_get_length(str, len) - val;
		
		ulen = utf8_get_pos(str, len, val);
		
		PARAM->_string.start += ulen;
		PARAM->_string.len -= ulen;
	}

	SP -= NPARAM;
	SP++;
}


bool STRING_convert_to_unicode(wchar_t **pwstr, int *pwlen, const char *str, int len)
{
	char *result;
	int wlen = utf8_get_length(str, len);
	int i, lc;
	wchar_t *wstr;
	
	result = STRING_new_temp(str, wlen * sizeof(wchar_t) + 3);
	wstr = (wchar_t *)result;
	
	for (i = 0; i < wlen; i++)
	{
		lc = utf8_get_char_length(*str);
		wstr[i] = (wchar_t)utf8_to_unicode(str, lc);
		if (wstr[i] == UNICODE_INVALID)
			return TRUE;
		str += lc;
	}
	
	wstr[wlen] = 0;
	*pwstr = wstr;
	*pwlen = wlen;
	return FALSE;
}

static bool convert_to_unicode(wchar_t **wstr, int *wlen, const char *str, int len, bool upper)
{
	wchar_t *wtemp;
	int i, l;
	
	if (len == 0)
	{
		*wstr = NULL;
		*wlen = 0;
		return FALSE;
	}
	
	if (STRING_convert_to_unicode(&wtemp, &l, str, len))
		return TRUE;
	
	if (upper)
	{
		for (i = 0; i < l; i++)
			wtemp[i] = towupper(wtemp[i]);
	}
	else
	{
		for (i = 0; i < l; i++)
			wtemp[i] = towlower(wtemp[i]);
	}
	
	*wstr = wtemp;
	*wlen = l;
	return FALSE;
}

static void convert_string(char *str, int len, bool upper)
{
	char *result;
	char *p, *pe;
	char c;
	int lc;
	wchar_t wc;
	
	if (len <= 0)
	{
		GB_ReturnNull();
		return;
	}
	
	result = STRING_new_temp(str, len);
	
	p = result;
	pe = &result[len];
	
	if (upper)
	{
		while (p < pe)
		{
			c = *p;
			lc = utf8_get_char_length(c);
			
			if (lc == 1)
			{
				*p = toupper(c);
				p++;
			}
			else
			{
				wc = (wchar_t)utf8_to_unicode(p, lc);
				wc = towupper(wc);
				// We suppose that the conversion does not change the number of bytes!
				utf8_from_unicode((uint)wc, p);
				//if (utf8_get_char_length(*p) != lc)
				//	fprintf(stderr, "convert_string: not the same number of bytes!\n");
				p += lc;
			}
		}
	}
	else
	{
		while (p < pe)
		{
			c = *p;
			lc = utf8_get_char_length(c);
			
			if (lc == 1)
			{
				*p = tolower(c);
				p++;
			}
			else
			{
				wc = (wchar_t)utf8_to_unicode(p, lc);
				wc = towlower(wc);
				// We suppose that the conversion does not change the number of bytes!
				utf8_from_unicode((uint)wc, p);
				//if (utf8_get_char_length(*p) != lc)
				//	fprintf(stderr, "convert_string: not the same number of bytes!\n");
				p += lc;
			}
		}
	}
	
	GB_ReturnString(result);
}


BEGIN_METHOD(String_Lower, GB_STRING str)

	convert_string(STRING(str), LENGTH(str), FALSE);

END_METHOD


BEGIN_METHOD(String_Upper, GB_STRING str)

	convert_string(STRING(str), LENGTH(str), TRUE);

END_METHOD


BEGIN_METHOD(String_Chr, GB_INTEGER code)

	char temp[8];

	utf8_from_unicode(VARG(code), temp);
	temp[utf8_get_char_length(temp[0])] = 0;
	GB_ReturnNewZeroString(temp);

END_METHOD


BEGIN_METHOD(String_Code, GB_STRING str; GB_INTEGER index)

	char *str;
	int len, index, pos, lc;

	index = VARGOPT(index, 1);
	if (index < 1)
	{
		GB_ReturnInteger(0);
		return;
	}
	
	str = STRING(str);
	len = LENGTH(str);
	pos = utf8_get_pos(str, len, index - 1);
	lc = utf8_get_char_length(str[pos]);
	
	GB_ReturnInteger(utf8_to_unicode(&str[pos], lc));
	
END_METHOD

static void string_search(const char *str, int len, const char *pattern, int lenp, int start, bool right, bool nocase)
{
	int pos;

	if (start)
		start = index_to_byte(str, len, start);

	if (!nocase)
	{
		pos = STRING_search(str, len, pattern, lenp, start, right, FALSE);
		pos = byte_to_index(str, len, pos);
	}
	else
	{
		wchar_t *wstr;
		int lstr;
		wchar_t *wpattern;
		int lpattern;
		
		if (convert_to_unicode(&wstr, &lstr, str, len, TRUE))
			goto __ERROR;
		
		if (convert_to_unicode(&wpattern, &lpattern, pattern, lenp, TRUE))
			goto __ERROR;
		
		pos = STRING_search((char *)wstr, lstr * sizeof(wchar_t), (char *)wpattern, lpattern * sizeof(wchar_t), start * sizeof(wchar_t), right, FALSE);
		if (pos)
			pos = (pos - 1) / sizeof(wchar_t) + 1;
	}
	
	GB_ReturnInteger(pos);
	return;
	
__ERROR:

	GB_ReturnInteger(0);
	return;
}

BEGIN_METHOD(String_Instr, GB_STRING str; GB_STRING pattern; GB_INTEGER start; GB_INTEGER mode)

	string_search(STRING(str), LENGTH(str), STRING(pattern), LENGTH(pattern), VARGOPT(start, 0), FALSE, VARGOPT(mode, GB_COMP_BINARY) == GB_COMP_NOCASE);

END_METHOD

BEGIN_METHOD(String_RInstr, GB_STRING str; GB_STRING pattern; GB_INTEGER start; GB_INTEGER mode)

	string_search(STRING(str), LENGTH(str), STRING(pattern), LENGTH(pattern), VARGOPT(start, 0), TRUE, VARGOPT(mode, GB_COMP_BINARY) == GB_COMP_NOCASE);

END_METHOD

BEGIN_METHOD(String_Comp, GB_STRING str1; GB_STRING str2; GB_INTEGER mode)

	int mode = VARGOPT(mode, GB_COMP_BINARY) | GB_COMP_LANG;
	bool nocase = (mode & GB_COMP_NOCASE) != 0;

	if (mode & GB_COMP_NATURAL)
		GB_ReturnInteger(COMPARE_string_natural(STRING(str1), LENGTH(str1), STRING(str2), LENGTH(str2), nocase));
	else
		GB_ReturnInteger(COMPARE_string_lang(STRING(str1), LENGTH(str1), STRING(str2), LENGTH(str2), nocase, TRUE));

END_METHOD

#define IS_VALID(_char)                        \
		((_char) < 0x110000 &&                     \
		(((_char) & 0xFFFFF800) != 0xD800) &&     \
		((_char) < 0xFDD0 || (_char) > 0xFDEF) &&  \
		((_char) & 0xFFFE) != 0xFFFE)

BEGIN_METHOD(String_IsValid, GB_STRING str)

	const uchar *str;
	int len, lc;
	uint unicode;
	bool valid = FALSE;
	int i;
	
	str = (const uchar *)STRING(str);
	len = LENGTH(str);
	
	while (len)
	{
		lc = utf8_get_char_length(*str);
		len -= lc;
		if (len < 0)
			goto _INVALID;
		
		//for (i = 0; i < len; i++)
		//	fprintf(stderr, "%02X ", str[i]);
		
		for (i = 1; i < lc; i++)
		{
			if ((str[i] & 0xC0) != 0x80)
				goto _INVALID;
		}
		
		unicode = utf8_to_unicode((char *)str, lc);
		if (unicode == UNICODE_INVALID)
			goto _INVALID;
		if (!IS_VALID(unicode))
			goto _INVALID;
		
		str += lc;
	}
	
	valid = TRUE;
	
_INVALID:

	GB_ReturnBoolean(valid);
	//fprintf(stderr, "\n");

END_METHOD

#endif

GB_DESC NATIVE_String[] =
{
	GB_DECLARE("String", 0),  GB_VIRTUAL_CLASS(),

	GB_STATIC_METHOD("Len", "i", String_Len, "(String)s"),

	GB_STATIC_FAST_METHOD("Mid", "s", String_Mid, "(String)s(Start)i[(Length)i]"),
	GB_STATIC_FAST_METHOD("Mid$", "s", String_Mid, "(String)s(Start)i[(Length)i]"),
	GB_STATIC_FAST_METHOD("Left", "s", String_Left, "(String)s[(Length)i]"),
	GB_STATIC_FAST_METHOD("Left$", "s", String_Left, "(String)s[(Length)i]"),
	GB_STATIC_FAST_METHOD("Right", "s", String_Right, "(String)s[(Length)i]"),
	GB_STATIC_FAST_METHOD("Right$", "s", String_Right, "(String)s[(Length)i]"),

	GB_STATIC_METHOD("Upper", "s", String_Upper, "(String)s"),
	GB_STATIC_METHOD("Upper$", "s", String_Upper, "(String)s"),
	GB_STATIC_METHOD("UCase", "s", String_Upper, "(String)s"),
	GB_STATIC_METHOD("UCase$", "s", String_Upper, "(String)s"),
	GB_STATIC_METHOD("Lower", "s", String_Lower, "(String)s"),
	GB_STATIC_METHOD("Lower$", "s", String_Lower, "(String)s"),
	GB_STATIC_METHOD("LCase", "s", String_Lower, "(String)s"),
	GB_STATIC_METHOD("LCase$", "s", String_Lower, "(String)s"),

	GB_STATIC_METHOD("InStr", "i", String_Instr, "(String)s(Pattern)s[(From)i(Mode)i]"),
	GB_STATIC_METHOD("RInStr", "i", String_RInstr, "(String)s(Pattern)s[(From)i(Mode)i]"),

	GB_STATIC_METHOD("Comp", "i", String_Comp, "(String)s(String2)s[(Mode)i]"),

	GB_STATIC_METHOD("Byte", "i", String_Pos, "(String)s(Index)i"),
	GB_STATIC_METHOD("Pos", "i", String_Pos, "(String)s(Index)i"),
	GB_STATIC_METHOD("Index", "i", String_Index, "(String)s(Byte)i"),

	GB_STATIC_METHOD("Chr", "s", String_Chr, "(Unicode)i"),
	GB_STATIC_METHOD("Chr$", "s", String_Chr, "(Unicode)i"),
	GB_STATIC_METHOD("Code", "i", String_Code, "(String)s[(Index)i]"),

	GB_STATIC_METHOD("IsValid", "b", String_IsValid, "(String)s"),

	GB_END_DECLARE
};
