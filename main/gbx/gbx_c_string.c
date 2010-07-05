/***************************************************************************

  gbx_c_string.c

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

static int get_next_pos(void)
{
  if (_pos >= _len)
    return 0;

  //_pos += get_char_length(&_str[_pos]);
  _pos += _char_length[(unsigned char)_str[_pos]];

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


BEGIN_METHOD(string_mid, GB_STRING str; GB_INTEGER start; GB_INTEGER len)

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


BEGIN_METHOD(string_left, GB_STRING str; GB_INTEGER len)

  int len = VARGOPT(len, 1);

  init_conv(STRING(str), LENGTH(str));

  get_substring(0, len);

END_METHOD


BEGIN_METHOD(string_right, GB_STRING str; GB_INTEGER len)

  int len = VARGOPT(len, 1);

  init_conv(STRING(str), LENGTH(str));

  if (len < 0)
    get_substring((-len), LENGTH(str));
  else
    get_substring(get_length() - len, len);

END_METHOD


static void convert_string(char *str, int len, bool upper)
{
  char *temp = NULL;
  int i, l;
  wchar_t *wtemp;

  if (len > 0)
  {
    if (STRING_conv(&temp, str, len, "UTF-8", SC_UNICODE, FALSE))
    	goto __ERROR;

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

    if (STRING_conv(&temp, temp, l * sizeof(wchar_t), SC_UNICODE, "UTF-8", FALSE))
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

static void string_search(const char *str, int len, const char *pattern, int lenp, int start, bool right)
{
	int pos;

	if (start)
		start = index_to_byte(str, len, start);

  pos = STRING_search(str, len, pattern, lenp, start, right, FALSE);
	GB_ReturnInteger(byte_to_index(str, len, pos));
}

BEGIN_METHOD(string_instr, GB_STRING str; GB_STRING pattern; GB_INTEGER start)

	string_search(STRING(str), LENGTH(str), STRING(pattern), LENGTH(pattern), VARGOPT(start, 0), FALSE);

END_METHOD

BEGIN_METHOD(string_rinstr, GB_STRING str; GB_STRING pattern; GB_INTEGER start)

	string_search(STRING(str), LENGTH(str), STRING(pattern), LENGTH(pattern), VARGOPT(start, 0), TRUE);

END_METHOD

BEGIN_METHOD(string_comp, GB_STRING str1; GB_STRING str2; GB_INTEGER mode)

	int mode = VARGOPT(mode, GB_COMP_BINARY) | GB_COMP_LANG;
	bool nocase = (mode & GB_COMP_NOCASE) != 0;

	if (mode & GB_COMP_NATURAL)
		GB_ReturnInteger(COMPARE_string_natural(STRING(str1), LENGTH(str1), STRING(str2), LENGTH(str2), nocase));
	else
		GB_ReturnInteger(COMPARE_string_lang(STRING(str1), LENGTH(str1), STRING(str2), LENGTH(str2), nocase, TRUE));

END_METHOD

#endif

GB_DESC NATIVE_String[] =
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
