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

#include <wctype.h>
#include <wchar.h>
#include <iconv.h>

#include "gb_error.h"
#include "gb_table.h"
#include "gbx_string.h"
#include "gbx_api.h"
#include "gbx_exec.h"
#include "gbx_compare.h"
#include "gambas.h"

#include "gbx_c_string.h"


static const char *_str;
static int _len;
static int _pos;
static int _clen;

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

int STRING_get_utf8_char_length(unsigned char c)
{
	return _char_length[c];
}

static void init_conv(const char *str, int len)
{
  _str = str;
  _len = len;
  _pos = 0;
  _clen = -1;
}

#define get_char_length() (_char_length[(unsigned char)_str[_pos]])

static int get_next_pos(void)
{
  if (_pos >= _len)
    return 0;

  //_pos += get_char_length(&_str[_pos]);
  _pos += get_char_length();

  return _pos;
}


static int get_pos(int index)
{
	int i;

  for (i = 1; i < index; i++)
    get_next_pos();

	return _pos;
}

static int get_length(void)
{
  int len;
  int i;

  if (_clen >= 0)
    return _clen;

  len = 0;

  for (i = 0; i < _len; i++)
  {
    if ((_str[i] & 0xC0) != 0x80)
      len++;
  }

  _clen = len;

  return len;
}

static int byte_to_index(const char *str, int len, int byte)
{
  if (byte < 1)
  	return 0;

  if (byte > len)
    byte = len;

  init_conv(str, byte);

  return get_length();
}

static int index_to_byte(const char *str, int len, int index)
{
	if (index <= 0)
		return 0;

  init_conv(str, len);
  return get_pos(index) + 1;
}


BEGIN_METHOD(String_Pos, GB_STRING str; GB_INTEGER index)

  GB_ReturnInteger(index_to_byte(STRING(str), LENGTH(str), VARG(index)));

END_METHOD


BEGIN_METHOD(String_Len, GB_STRING str)

  init_conv(STRING(str), LENGTH(str));

  GB_ReturnInteger(get_length());

END_METHOD


BEGIN_METHOD(String_Index, GB_STRING str; GB_INTEGER pos)

  GB_ReturnInteger(byte_to_index(STRING(str), LENGTH(str), VARG(pos)));

END_METHOD


static void get_substring(int start, int len)
{
  int i;
  int pos;

  if (len < 0)
    len += get_length();

  if (len <= 0)
  {
    GB_ReturnNull();
    return;
  }

  for (i = 0; i < start; i++)
  {
    if (get_next_pos() <= 0)
    {
      GB_ReturnNull();
      return;
    }
  }

  pos = _pos;

  for (i = 0; i < len; i++)
  {
    if (get_next_pos() <= 0)
      break;
  }

	if (_pos > _len)
		_pos = _len;
	
	if (_pos > pos)
		GB_ReturnNewString(_str + pos, _pos - pos);
	else
		GB_ReturnNull();
}


BEGIN_METHOD(String_Mid, GB_STRING str; GB_INTEGER start; GB_INTEGER len)

  int start = VARG(start);
  int len = VARGOPT(len, LENGTH(str));

  if (start < 1)
  {
    GB_Error((char *)E_ARG);
    return;
  }

  init_conv(STRING(str), LENGTH(str));
  get_substring(start - 1, len);

END_METHOD


BEGIN_METHOD(String_Left, GB_STRING str; GB_INTEGER len)

  int len = VARGOPT(len, 1);

  init_conv(STRING(str), LENGTH(str));

  get_substring(0, len);

END_METHOD


BEGIN_METHOD(String_Right, GB_STRING str; GB_INTEGER len)

  int len = VARGOPT(len, 1);

  init_conv(STRING(str), LENGTH(str));

  if (len < 0)
    get_substring((-len), LENGTH(str));
  else
    get_substring(get_length() - len, len);

END_METHOD


static bool convert_to_unicode(wchar_t **wstr, int *wlen, const char *str, int len, bool upper)
{
	char *temp;
	wchar_t *wtemp;
	int i, l;
	
	if (len == 0)
	{
		*wstr = NULL;
		*wlen = 0;
		return FALSE;
	}
	
	if (STRING_conv(&temp, str, len, "UTF-8", SC_UNICODE, FALSE))
		return TRUE;
	
	wtemp = (wchar_t *)temp;
	l = wcslen(wtemp);
	
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
	char *temp = NULL;
  wchar_t *wtemp;
	int ltemp;

  if (len > 0)
  {
		if (convert_to_unicode(&wtemp, &ltemp, str, len, upper))
			goto __ERROR;
		
    if (STRING_conv(&temp, (char *)wtemp, ltemp * sizeof(wchar_t), SC_UNICODE, "UTF-8", FALSE))
    	goto __ERROR;
  }

  GB_ReturnString(temp);
  return;
  
__ERROR:

	if (len > 0)
		GB_ReturnNewString(str, len);
	else
		GB_ReturnNull();
}


BEGIN_METHOD(String_Lower, GB_STRING str)

  convert_string(STRING(str), LENGTH(str), FALSE);

END_METHOD


BEGIN_METHOD(String_Upper, GB_STRING str)

  convert_string(STRING(str), LENGTH(str), TRUE);

END_METHOD


BEGIN_METHOD(String_Chr, GB_INTEGER code)

	const char *charset = EXEC_big_endian ? "UCS-4BE" : "UCS-4LE";
	char *temp;

	STRING_conv(&temp, (char *)(&VARG(code)), sizeof(wchar_t), charset, "UTF-8", TRUE);
	GB_ReturnString(temp);

END_METHOD

BEGIN_METHOD(String_Code, GB_STRING str; GB_INTEGER index)

	int index, pos, npos;
	const char *charset = EXEC_big_endian ? "UCS-4BE" : "UCS-4LE";
	char *temp;

	index = VARGOPT(index, 1);
	if (index < 1)
	{
		GB_ReturnInteger(0);
		return;
	}
	//{
	//	GB_Error((char *)E_ARG);
	//	return;
	//}

  init_conv(STRING(str), LENGTH(str));
  pos = get_pos(index);
  npos = get_next_pos();
  if (npos == 0)
  {
  	GB_ReturnInteger(0);
  	return;
	}
	
	STRING_conv(&temp, STRING(str) + pos, npos - pos, "UTF-8", charset, TRUE);
	GB_ReturnInteger(*((int *)temp));

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

	int len;
	uint unicode;
	bool valid = FALSE;
	int i;
	const uchar *str;
	
  init_conv(STRING(str), LENGTH(str));
	while (_str[_pos])
	{
		len = get_char_length();
		if ((_pos + len) > _len)
			goto _INVALID;
		
		str = (const uchar *)&_str[_pos];
		
		//for (i = 0; i < len; i++)
		//	fprintf(stderr, "%02X ", str[i]);
		
		for (i = 1; i < len; i++)
		{
			if ((str[i] & 0xC0) != 0x80)
				goto _INVALID;
		}
		
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
		
		if (!IS_VALID(unicode))
			goto _INVALID;
		
		get_next_pos();
		//fprintf(stderr, " . ");
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

  GB_STATIC_METHOD("Mid", "s", String_Mid, "(String)s(Start)i[(Length)i]"),
  GB_STATIC_METHOD("Mid$", "s", String_Mid, "(String)s(Start)i[(Length)i]"),
  GB_STATIC_METHOD("Left", "s", String_Left, "(String)s[(Length)i]"),
  GB_STATIC_METHOD("Left$", "s", String_Left, "(String)s[(Length)i]"),
  GB_STATIC_METHOD("Right", "s", String_Right, "(String)s[(Length)i]"),
  GB_STATIC_METHOD("Right$", "s", String_Right, "(String)s[(Length)i]"),

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
