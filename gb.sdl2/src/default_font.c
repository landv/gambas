/***************************************************************************

  default_font.c

  (c) 2014 Benoît Minisini <gambas@users.sourceforge.net>

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

#define __DEFAULT_FONT_C

#include "default_font.h"
#include "default_font_data.h"

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

#define utf8_get_char_length(_c) ((int)_char_length[(unsigned char)(_c)])

int UTF8_get_length(const char *sstr, int len)
{
	const uchar *str = (const uchar *)sstr;
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

static uint utf8_to_unicode(const char *sstr, int len)
{
	const uchar *str = (const uchar *)sstr;
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


void FONT_render_default(uint *dest, int size, const char *text, int len)
{
	static void *jump[] = { &&__0, &&__1, &&__2, &&__3, &&__4, &&__5, &&__6, &&__7, &&__8, &&__9, &&__A, &&__B, &&__C, &&__D, &&__E, &&__F };
	static void *jump2[] = { &&__00, &&__10, &&__20, &&__30, &&__40, &&__50, &&__60, &&__70, &&__80, &&__90, &&__A0, &&__B0, &&__C0, &&__D0, &&__E0, &&__F0 };

	int lc;
	const char *src;
	uint *p;
	int y;
	uchar line;
	uint code;
	const uint col = 0xFFFFFFFF;

	size *= DEFAULT_FONT_WIDTH;

	for(;;)
	{
		if (!*text)
			break;

		lc = utf8_get_char_length(*text);
		code = utf8_to_unicode(text, lc);
		text += lc;

		if (code >= 33 && code <= 126)
			src = _default_font_33_126 + DEFAULT_FONT_HEIGHT * (code - 33);
		else if (code >= 160 && code <= 687)
			src = _default_font_160_687 + DEFAULT_FONT_HEIGHT * (code - 160);
		else
			src = NULL;

		if (src)
		{
			p = dest;

			for (y = 0; y < DEFAULT_FONT_HEIGHT; y++)
			{
				line = *src++;
				if (!line)
				{
					p += size;
					continue;
				}

				goto *jump[line & 0xF];

				__1: p[0] = col; goto __0;
				__2: p[1] = col; goto __0;
				__3: p[0] = p[1] = col; goto __0;
				__4: p[2] = col; goto __0;
				__5: p[0] = p[2] = col; goto __0;
				__6: p[1] = p[2] = col; goto __0;
				__7: p[0] = p[1] = p[2] = col; goto __0;
				__8: p[3] = col; goto __0;
				__9: p[0] = p[3] = col; goto __0;
				__A: p[1] = p[3] = col; goto __0;
				__B: p[0] = p[1] = p[3] = col; goto __0;
				__C: p[2] = p[3] = col; goto __0;
				__D: p[0] = p[2] = p[3] = col; goto __0;
				__E: p[1] = p[2] = p[3] = col; goto __0;
				__F: p[0] = p[1] = p[2] = p[3] = col; goto __0;
				__0:

				goto *jump2[line >> 4];

				__10: p[4] = col; goto __00;
				__20: p[5] = col; goto __00;
				__30: p[4] = p[5] = col; goto __00;
				__40: p[6] = col; goto __00;
				__50: p[4] = p[6] = col; goto __00;
				__60: p[5] = p[6] = col; goto __00;
				__70: p[4] = p[5] = p[6] = col; goto __00;
				__80: p[7] = col; goto __00;
				__90: p[4] = p[7] = col; goto __00;
				__A0: p[5] = p[7] = col; goto __00;
				__B0: p[4] = p[5] = p[7] = col; goto __00;
				__C0: p[6] = p[7] = col; goto __00;
				__D0: p[4] = p[6] = p[7] = col; goto __00;
				__E0: p[5] = p[6] = p[7] = col; goto __00;
				__F0: p[4] = p[5] = p[6] = p[7] = col; goto __00;
				__00:

				p += size;
			}
		}

		dest += DEFAULT_FONT_WIDTH;
	}
}
