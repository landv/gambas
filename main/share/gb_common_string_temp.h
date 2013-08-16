/***************************************************************************

  gb_common_string_temp.h

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

#include "gb_common_case.h"

bool STRING_equal_same(const char *str1, const char *str2, int len)
{
	static const void *jump[8] = { &&__LEN_0, &&__LEN_1, &&__LEN_2, &&__LEN_3, &&__LEN_4, &&__LEN_5, &&__LEN_6, &&__LEN_7 };
	
	//return len == 0 || memcmp(str1, str2, len) == 0;
	
	#if defined(ARCH_X86_64) || defined(ARCH_X86)
	
	for(;;)
	{
		if (len < 8) break;
	
		if (*((int64_t *)str1) != *((int64_t *)str2))
			return FALSE;
		str1 += 8;
		str2 += 8;
		len -= 8;

		if (len < 8) break;
	
		if (*((int64_t *)str1) != *((int64_t *)str2))
			return FALSE;
		str1 += 8;
		str2 += 8;
		len -= 8;
		
		if (len < 8) break;
	
		if (*((int64_t *)str1) != *((int64_t *)str2))
			return FALSE;
		str1 += 8;
		str2 += 8;
		len -= 8;
		
		if (len < 8) break;
	
		if (*((int64_t *)str1) != *((int64_t *)str2))
			return FALSE;
		str1 += 8;
		str2 += 8;
		len -= 8;
	}
	
	#else
	
	while (len >= 8)
	{
		if (str1[0] != str2[0])
			return FALSE;
		if (str1[1] != str2[1])
			return FALSE;
		if (str1[2] != str2[2])
			return FALSE;
		if (str1[3] != str2[3])
			return FALSE;
		if (str1[4] != str2[4])
			return FALSE;
		if (str1[5] != str2[5])
			return FALSE;
		if (str1[6] != str2[6])
			return FALSE;
		if (str1[7] != str2[7])
			return FALSE;
		str1 += 8;
		str2 += 8;
		len -= 8;
	}
	
	#endif
	
	goto *jump[len];
	
__LEN_7: if (str1[6] != str2[6]) return FALSE;
__LEN_6: if (str1[5] != str2[5]) return FALSE;
__LEN_5: if (str1[4] != str2[4]) return FALSE;
__LEN_4: if (str1[3] != str2[3]) return FALSE;
__LEN_3: if (str1[2] != str2[2]) return FALSE;
__LEN_2: if (str1[1] != str2[1]) return FALSE;
__LEN_1: if (str1[0] != str2[0]) return FALSE;
__LEN_0: return TRUE;
}

bool STRING_equal_ignore_case_same(const char *str1, const char *str2, int len)
{
	static const void *jump[8] = { &&__LEN_0, &&__LEN_1, &&__LEN_2, &&__LEN_3, &&__LEN_4, &&__LEN_5, &&__LEN_6, &&__LEN_7 };
	
	#if defined(ARCH_X86_64) || defined(ARCH_X86)
	while (len >= 8)
	{
		if (*((int64_t *)str1) != *((int64_t *)str2))
		{
			if (toupper(str1[0]) != toupper(str2[0])) return FALSE;
			if (toupper(str1[1]) != toupper(str2[1])) return FALSE;
			if (toupper(str1[2]) != toupper(str2[2])) return FALSE;
			if (toupper(str1[3]) != toupper(str2[3])) return FALSE;
			if (toupper(str1[4]) != toupper(str2[4])) return FALSE;
			if (toupper(str1[5]) != toupper(str2[5])) return FALSE;
			if (toupper(str1[6]) != toupper(str2[6])) return FALSE;
			if (toupper(str1[7]) != toupper(str2[7])) return FALSE;
		}
		
		str1 += 8;
		str2 += 8;
		len -= 8;
	}
	#else
	while (len >= 8)
	{
		if (str1[0] != str2[0] && toupper(str1[0]) != toupper(str2[0])) return FALSE;
		if (str1[1] != str2[1] && toupper(str1[1]) != toupper(str2[1])) return FALSE;
		if (str1[2] != str2[2] && toupper(str1[2]) != toupper(str2[2])) return FALSE;
		if (str1[3] != str2[3] && toupper(str1[3]) != toupper(str2[3])) return FALSE;
		if (str1[4] != str2[4] && toupper(str1[4]) != toupper(str2[4])) return FALSE;
		if (str1[5] != str2[5] && toupper(str1[5]) != toupper(str2[5])) return FALSE;
		if (str1[6] != str2[6] && toupper(str1[6]) != toupper(str2[6])) return FALSE;
		if (str1[7] != str2[7] && toupper(str1[7]) != toupper(str2[7])) return FALSE;
		str1 += 8;
		str2 += 8;
		len -= 8;
	}
	#endif
	
	goto *jump[len];
	
__LEN_7: if (str1[6] != str2[6] && toupper(str1[6]) != toupper(str2[6])) return FALSE;
__LEN_6: if (str1[5] != str2[5] && toupper(str1[5]) != toupper(str2[5])) return FALSE;
__LEN_5: if (str1[4] != str2[4] && toupper(str1[4]) != toupper(str2[4])) return FALSE;
__LEN_4: if (str1[3] != str2[3] && toupper(str1[3]) != toupper(str2[3])) return FALSE;
__LEN_3: if (str1[2] != str2[2] && toupper(str1[2]) != toupper(str2[2])) return FALSE;
__LEN_2: if (str1[1] != str2[1] && toupper(str1[1]) != toupper(str2[1])) return FALSE;
__LEN_1: if (str1[0] != str2[0] && toupper(str1[0]) != toupper(str2[0])) return FALSE;
__LEN_0: return TRUE;
}

int STRING_compare(const char *str1, int len1, const char *str2, int len2)
{
	uint i;
	int len = len1 < len2 ? len1 : len2;
	int diff;
	register unsigned char c1, c2;

	for (i = 0; i < len; i++)
	{
		c1 = str1[i];
		c2 = str2[i];
		if (c1 > c2) return 1;
		if (c1 < c2) return -1;
	}

	diff = len1 - len2;
	return (diff < 0) ? (-1) : (diff > 0) ? 1 : 0;
}


int STRING_compare_ignore_case(const char *str1, int len1, const char *str2, int len2)
{
	uint i;
	int len = len1 < len2 ? len1 : len2;
	int diff;
	register unsigned char c1, c2;

	for (i = 0; i < len; i++)
	{
		c1 = tolower(str1[i]);
		c2 = tolower(str2[i]);
		if (c1 > c2) return 1;
		if (c1 < c2) return -1;
	}

	diff = len1 - len2;
	return (diff < 0) ? (-1) : (diff > 0) ? 1 : 0;
}
