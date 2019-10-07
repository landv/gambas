/***************************************************************************

  gb_common_buffer_temp.h

  (c) 2000-2017 Benoît Minisini <g4mba5@gmail.com>

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

#define __COMMON_BUFFER_C

#include "gb_common.h"
#include "gb_common_buffer.h"

char COMMON_buffer[COMMON_BUF_MAX];
int COMMON_pos;
int COMMON_len;

char *COMMON_start;
int COMMON_last;

void COMMON_buffer_init(const char *str, int len)
{
  COMMON_start = (char *)str;
  COMMON_len = len;
  COMMON_pos = 0;
  COMMON_last = (-1);
}


void COMMON_jump_space(void)
{
  int c;

  for(;;)
  {
    c = COMMON_look_char();
    if (c <= 0 || !isspace(c))
      break;
    COMMON_pos++;
  }
}



bool COMMON_has_string(const char *str, int len)
{
	if (COMMON_get_size_left() < len)
		return FALSE;
	
	return memcmp(&COMMON_start[COMMON_pos], str, len) == 0;
}
