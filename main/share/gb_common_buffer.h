/***************************************************************************

  common_buffer.h

  Common useful routines for managing buffers

  Copyright (C) 2000 Benoît Minisini <gambas@freesurf.fr>

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

#ifndef __GB_COMMON_BUFFER_H
#define __GB_COMMON_BUFFER_H

#define COMMON_BUF_MAX 256

#ifndef __COMMON_BUFFER_C
EXTERN long COMMON_pos;
EXTERN char COMMON_buffer[];
#endif

PUBLIC void COMMON_init(void);

PUBLIC void COMMON_buffer_init(const char *str, long len);
PUBLIC int COMMON_get_char(void);
PUBLIC int COMMON_last_char(void);
PUBLIC int COMMON_look_char(void);
PUBLIC int COMMON_put_char(char c);
PUBLIC void COMMON_jump_space(void);
PUBLIC char *COMMON_get_current(void);
PUBLIC long COMMON_get_size_left(void);

#endif
