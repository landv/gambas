/***************************************************************************

  gbx_c_string.c

  (c) 2000-2006 Benoît Minisini <gambas@users.sourceforge.net>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 1, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

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
#include "gambas.h"

#include "gbx_c_string.h"


static const char *_str;
static long _len;
static long _pos;
static long _clen;

static int get_char_length(const char *s)
{
  int n = 1;
  unsigned char c = *((unsigned char *)s);

  if (c & 0x80)
  {
    for (;;)
    {
      c <<= 1;
      if (!(c & 0x80))
        break;
      n++;
    }
  }

  return n;
}


static void init_conv(const char *str, long len)
{
  _str = str;
  _len = len;
  _pos = 0;
  _clen = -1;
}

static long get_next_pos(void)
{
  if (_pos >= _len)
    return 0;

  _pos += get_char_length(&_str[_pos]);
  return _pos;
}


static long get_pos(long index)
{
	int i;

  for (i = 1; i < index; i++)
    get_next_pos();

	return _pos;
}

static long get_length(void)
{
  long len;
  long i;

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

static long byte_to_index(const char *str, long len, long byte)
{
  if (byte < 1)
  	return 0;

  if (byte > len)
    byte = len;

  init_conv(str, byte);

  return get_length();
}

static long index_to_byte(const char *str, long len, long index)
{
	if (index <= 0)
		return 0;

  init_conv(str, len);
  return get_pos(index) + 1;
}


BEGIN_METHOD(string_pos, GB_STRING str; GB_INTEGER index)

  GB_ReturnInteger(index_to_byte(STRING(str), LENGTH(str), VARG(index)));

END_METHOD


BEGIN_METHOD(string_len, GB_STRING str)

  init_conv(STRING(str), LENGTH(str));

  GB_ReturnInteger(get_length());

END_METHOD


BEGIN_METHOD(string_index, GB_STRING str; GB_INTEGER pos)

  GB_ReturnInteger(byte_to_index(STRING(str), LENGTH(str), VARG(pos)));

END_METHOD


static void get_substring(long start, long len)
{
  long i;
  long pos;

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

  GB_ReturnNewString(_str + pos, _pos - pos);
}


BEGIN_METHOD(string_mid, GB_STRING str; GB_INTEGER start; GB_INTEGER len)

  long start = VARG(start);
  long len = VARGOPT(len, LENGTH(str));

  if (start < 1)
  {
    GB_Error((char *)E_ARG);
    return;
  }

  init_conv(STRING(str), LENGTH(str));
  get_substring(start - 1, len);

END_METHOD


BEGIN_METHOD(string_left, GB_STRING str; GB_INTEGER len)

  long len = VARGOPT(len, 1);

  init_conv(STRING(str), LENGTH(str));

  get_substring(0, len);

END_METHOD


BEGIN_METHOD(string_right, GB_STRING str; GB_INTEGER len)

  long len = VARGOPT(len, 1);

  init_conv(STRING(str), LENGTH(str));

  if (len < 0)
    get_substring((-len), LENGTH(str));
  else
    get_substring(get_length() - len, len);

END_METHOD


static void convert_string(char *str, long len, bool upper)
{
  char *charset;
  char *temp = NULL;
  int i, l;
  wchar_t *wtemp;

  if (len > 0)
  {
    charset = EXEC_big_endian ? "UCS-4BE" : "UCS-4LE";

    STRING_conv(&temp, str, len, "UTF-8", charset, TRUE);

    l = STRING_length(temp) / sizeof(wchar_t);
    wtemp = (wchar_t *)temp;

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

    STRING_conv(&temp, temp, l * sizeof(wchar_t), charset, "UTF-8", TRUE);
  }

  GB_ReturnString(temp);
}


static int compare_string(char *s1, long l1, char *s2, long l2, bool nocase)
{
  char *charset;
  wchar_t *t1 = NULL;
  wchar_t *t2 = NULL;
  int i, len , diff;
  wchar_t wc1, wc2;
  
	if (l1 == 0)
	{
		if (l2 == 0)
			return 0;
		else
			return (-1);
	}
	else if (l2 == 0)
		return 1;

	charset = EXEC_big_endian ? "UCS-4BE" : "UCS-4LE";

	if (STRING_conv((char **)&t1, s1, l1, "UTF-8", charset, FALSE)
		  || STRING_conv((char **)&t2, s2, l2, "UTF-8", charset, FALSE))
	{
		return nocase ? TABLE_compare_ignore_case(s1, l1, s2, l2) : TABLE_compare(s1, l1, s2, l2);
	}
	
	l1 = STRING_length((char *)t1) / sizeof(wchar_t);
	l2 = STRING_length((char *)t2) / sizeof(wchar_t);
	
	len = Min(l1, l2);
	
	if (nocase)
	{
		for (i = 0; i < len; i++)
		{
			wc1 = towlower(t1[i]);
			wc2 = towlower(t2[i]);
			if (wc1 > wc2) return 1;
			if (wc1 < wc2) return -1;
		}
	}
	else
	{
		for (i = 0; i < len; i++)
		{
			wc1 = t1[i];
			wc2 = t2[i];
			if (wc1 > wc2) return 1;
			if (wc1 < wc2) return -1;
		}
	}

  diff =  l1 - l2;
  return (diff < 0) ? (-1) : (diff > 0) ? 1 : 0;
}


BEGIN_METHOD(string_lower, GB_STRING str)

  convert_string(STRING(str), LENGTH(str), FALSE);

END_METHOD


BEGIN_METHOD(string_upper, GB_STRING str)

  convert_string(STRING(str), LENGTH(str), TRUE);

END_METHOD


BEGIN_METHOD(string_chr, GB_INTEGER code)

	const char *charset = EXEC_big_endian ? "UCS-4BE" : "UCS-4LE";
	char *temp;

	STRING_conv(&temp, (char *)(&VARG(code)), sizeof(wchar_t), charset, "UTF-8", TRUE);
	GB_ReturnString(temp);

END_METHOD

BEGIN_METHOD(string_code, GB_STRING str; GB_INTEGER index)

	long index, pos, npos;
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
	GB_ReturnInteger(*((long *)temp));

END_METHOD

static void string_search(const char *str, long len, const char *pattern, long lenp, long start, bool right)
{
	long pos;

	if (start)
		start = index_to_byte(str, len, start);

  pos = STRING_search(str, len, pattern, lenp, start, right);
	GB_ReturnInteger(byte_to_index(str, len, pos));
}

BEGIN_METHOD(string_instr, GB_STRING str; GB_STRING pattern; GB_INTEGER start)

	string_search(STRING(str), LENGTH(str), STRING(pattern), LENGTH(pattern), VARGOPT(start, 0), FALSE);

END_METHOD

BEGIN_METHOD(string_rinstr, GB_STRING str; GB_STRING pattern; GB_INTEGER start)

	string_search(STRING(str), LENGTH(str), STRING(pattern), LENGTH(pattern), VARGOPT(start, 0), TRUE);

END_METHOD

BEGIN_METHOD(string_comp, GB_STRING str1; GB_STRING str2; GB_INTEGER mode)

	bool mode = VARGOPT(mode, GB_COMP_BINARY) != GB_COMP_BINARY;

	GB_ReturnInteger(compare_string(STRING(str1), LENGTH(str1), STRING(str2), LENGTH(str2), mode));

END_METHOD

#endif

PUBLIC GB_DESC NATIVE_String[] =
{
  GB_DECLARE("String", 0),  GB_VIRTUAL_CLASS(),

  GB_STATIC_METHOD("Len", "i", string_len, "(String)s"),

  GB_STATIC_METHOD("Mid", "s", string_mid, "(String)s(Start)i[(Length)i]"),
  GB_STATIC_METHOD("Mid$", "s", string_mid, "(String)s(Start)i[(Length)i]"),
  GB_STATIC_METHOD("Left", "s", string_left, "(String)s[(Length)i]"),
  GB_STATIC_METHOD("Left$", "s", string_left, "(String)s[(Length)i]"),
  GB_STATIC_METHOD("Right", "s", string_right, "(String)s[(Length)i]"),
  GB_STATIC_METHOD("Right$", "s", string_right, "(String)s[(Length)i]"),

  GB_STATIC_METHOD("Upper", "s", string_upper, "(String)s"),
  GB_STATIC_METHOD("Upper$", "s", string_upper, "(String)s"),
  GB_STATIC_METHOD("UCase", "s", string_upper, "(String)s"),
  GB_STATIC_METHOD("UCase$", "s", string_upper, "(String)s"),
  GB_STATIC_METHOD("Lower", "s", string_lower, "(String)s"),
  GB_STATIC_METHOD("Lower$", "s", string_lower, "(String)s"),
  GB_STATIC_METHOD("LCase", "s", string_lower, "(String)s"),
  GB_STATIC_METHOD("LCase$", "s", string_lower, "(String)s"),

  GB_STATIC_METHOD("InStr", "i", string_instr, "(String)s(Pattern)s[(From)i]"),
  GB_STATIC_METHOD("RInStr", "i", string_rinstr, "(String)s(Pattern)s[(From)i]"),

  GB_STATIC_METHOD("Comp", "i", string_comp, "(String)s(String2)s[(Mode)i]"),

  GB_STATIC_METHOD("Byte", "i", string_pos, "(String)s(Index)i"),
  GB_STATIC_METHOD("Pos", "i", string_pos, "(String)s(Index)i"),
  GB_STATIC_METHOD("Index", "i", string_index, "(String)s(Byte)i"),

  GB_STATIC_METHOD("Chr", "s", string_chr, "(Unicode)i"),
  GB_STATIC_METHOD("Chr$", "s", string_chr, "(Unicode)i"),
  GB_STATIC_METHOD("Code", "i", string_code, "(String)s[(Index)i]"),

  GB_END_DECLARE
};
