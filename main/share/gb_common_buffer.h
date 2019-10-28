/***************************************************************************

  gb_common_buffer.h

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

#ifndef __GB_COMMON_BUFFER_H
#define __GB_COMMON_BUFFER_H

#define COMMON_BUF_MAX 512

#ifndef __COMMON_BUFFER_C
EXTERN int COMMON_pos;
EXTERN int COMMON_len;
EXTERN char COMMON_buffer[];
EXTERN char *COMMON_start;
EXTERN int COMMON_last;
#endif

void COMMON_init(void);

void COMMON_buffer_init(const char *str, int len);
void COMMON_jump_space(void);
bool COMMON_has_string(const char *str, int len);

#define COMMON_get_char(void) (COMMON_last = (COMMON_pos >= COMMON_len) ? (int)-1 : (int)(unsigned char)(COMMON_start[COMMON_pos++]))

#define COMMON_look_char() ((COMMON_pos >= COMMON_len) ? (int)-1 : (int)(unsigned char)(COMMON_start[COMMON_pos]))

#define COMMON_put_char(_c) \
({ \
  if (COMMON_pos >= COMMON_len) \
    (-1); \
	else \
	{ \
		COMMON_start[COMMON_pos++] = (_c); \
		0; \
	} \
})

#define COMMON_last_char() (COMMON_last)

#define COMMON_get_current() (&COMMON_start[COMMON_pos])

#define COMMON_get_size_left(void) (COMMON_len - COMMON_pos)

#endif
