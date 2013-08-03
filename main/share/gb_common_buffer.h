/***************************************************************************

  gb_common_buffer.h

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

#ifndef __GB_COMMON_BUFFER_H
#define __GB_COMMON_BUFFER_H

#define COMMON_BUF_MAX 256

#ifndef __COMMON_BUFFER_C
EXTERN int COMMON_pos;
EXTERN char COMMON_buffer[];
#endif

void COMMON_init(void);

void COMMON_buffer_init(const char *str, int len);
int COMMON_get_char(void);
int COMMON_last_char(void);
int COMMON_look_char(void);
int COMMON_put_char(char c);
void COMMON_jump_space(void);
char *COMMON_get_current(void);
int COMMON_get_size_left(void);
#endif
