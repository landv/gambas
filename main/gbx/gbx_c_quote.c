/***************************************************************************

  gbx_c_quote.c

  (c) 2000-2007 Benoit Minisini <gambas@users.sourceforge.net>

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

#define __GBX_C_QUOTE_C

#include "gbx_info.h"

#ifndef GBX_INFO

#include "gb_common.h"
#include "gb_error.h"
#include "gambas.h"
#include "gbx_api.h"
#include "gbx_string.h"
#include "gbx_local.h"
#include "gbx_c_quote.h"


BEGIN_METHOD(CQUOTE_call, GB_STRING str)

	char *result = NULL;
	int i;
	unsigned char c;
	char buf[6];
	
	STRING_add(&result, "\"", 1);
	
	for (i = 0; i < LENGTH(str); i++)
	{
		c = STRING(str)[i];
		if (c == '\n')
			STRING_add(&result, "\\n", 2);
		else if (c == '\r')
			STRING_add(&result, "\\r", 2);
		else if (c == '\t')
			STRING_add(&result, "\\t", 2);
		else if (c == '"')
			STRING_add(&result, "\\\"", 2);
		else if (c == '\\')
			STRING_add(&result, "\\\\", 2);
		else if (c < ' ' || c > 126)
		{
			sprintf(buf, "\\x%02X", c);
			STRING_add(&result, buf, 4);
		}
		else
			STRING_add(&result, (char *)&c, 1);
	}
	
	STRING_add(&result, "\"", 1);
  STRING_extend_end(&result);
	
	GB_ReturnString(result);

END_METHOD


BEGIN_METHOD(CQUOTE_shell, GB_STRING str)

	char *str = STRING(str);
	int len = LENGTH(str);
	char *result = NULL;
	int i;
	unsigned char c;
	char buf[8];
	
	if (!LOCAL_is_UTF8)
	{
  	STRING_conv(&result, str, len, "UTF-8", LOCAL_encoding, FALSE);
  	str = result;
  	result = NULL;
		len = str ? strlen(str) : 0;
	}
	
	for (i = 0; i < len; i++)
	{
		c = str[i];
		if (c == '\n')
			STRING_add(&result, "$'\\n'", 5);
		else if (c == '\r')
			STRING_add(&result, "$'\\r'", 5);
		else if (c == '\t')
			STRING_add(&result, "$'\\t'", 5);
		else if (c < ' ') //|| (c > 126 && !LOCAL_is_UTF8))
		{
			sprintf(buf, "$'\\x%02X'", c);
			STRING_add(&result, buf, 7);
		}
		else if ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || index(".-/_~", c) || c > 126)
			STRING_add(&result, (char *)&c, 1);
		else
		{
			buf[0] = '\\';
			buf[1] = c;
			STRING_add(&result, buf, 2);
		}
	}
	
  STRING_extend_end(&result);
	
	GB_ReturnString(result);

END_METHOD


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

BEGIN_METHOD(CUNQUOTE_call, GB_STRING str)

	char *str = STRING(str);
	int len = LENGTH(str);
	char *result = NULL;
	int i;
	unsigned char c;
	
	if (len >= 2 && str[0] == '"' && str[len - 1] == '"')
	{
		str++;
		len -= 2;
	}

	for (i = 0; i < len; i++)
	{
		c = str[i];
		if (c == '\\')
		{
			i++;
			if (i >= len)
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
				if (i >= (len - 2))
					break;
					
				c = (read_hex_digit(str[i + 1]) << 4) + read_hex_digit(str[i + 2]);
				i += 2;
			}
		}
		
		STRING_add(&result, (char *)&c, 1);
	}

  STRING_extend_end(&result);
	GB_ReturnString(result);

END_METHOD


#endif


PUBLIC GB_DESC NATIVE_Quote[] =
{
  GB_DECLARE("Quote", 0), GB_VIRTUAL_CLASS(),

  GB_STATIC_METHOD("_call", "s", CQUOTE_call, "(String)s"),
  GB_STATIC_METHOD("Shell", "s", CQUOTE_shell, "(String)s"),
  
  GB_END_DECLARE
};

PUBLIC GB_DESC NATIVE_Unquote[] =
{
  GB_DECLARE("UnQuote", 0), GB_VIRTUAL_CLASS(),

  GB_STATIC_METHOD("_call", "s", CUNQUOTE_call, "(String)s"),
  
  GB_END_DECLARE
};

