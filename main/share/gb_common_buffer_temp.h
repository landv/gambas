/***************************************************************************

  gb_common_buffer_temp.h

  common buffer useful routines

  (c) 2000-2005 Beno�t Minisini <gambas@users.sourceforge.net>

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

#define __COMMON_BUFFER_C

#include "gb_common.h"

PUBLIC char COMMON_buffer[256];
PUBLIC int COMMON_pos;

static char *common_buffer;
static int common_len;
static int common_last;

PUBLIC void COMMON_buffer_init(char *str, int len)
{
  common_buffer = str;
  common_len = len;
  COMMON_pos = 0;
  common_last = (-1);
}


PUBLIC int COMMON_look_char(void)
{
  if (COMMON_pos >= common_len)
    return (-1);

  return (unsigned char)(common_buffer[COMMON_pos]);
}


PUBLIC int COMMON_get_char(void)
{
  if (COMMON_pos >= common_len)
    common_last = (-1);
  else
    common_last = (unsigned char)(common_buffer[COMMON_pos++]);

  return common_last;
}


PUBLIC int COMMON_last_char(void)
{
  return common_last;
}


PUBLIC int COMMON_put_char(char c)
{
  if (COMMON_pos >= common_len)
    return (-1);

  common_buffer[COMMON_pos++] = c;
  return 0;
}


PUBLIC void COMMON_jump_space(void)
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


PUBLIC char *COMMON_get_current(void)
{
  return &common_buffer[COMMON_pos];
}


PUBLIC int COMMON_get_size_left(void)
{
  return common_len - COMMON_pos;
}


