/***************************************************************************

  common_check.h

  Swapping routines for endianess management

  Copyright (c) 2000-2007 Benoit Minisini <gambas@freesurf.fr>

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

#ifndef __GB_COMMON_CHECK_H
#define __GB_COMMON_CHECK_H

#include <signal.h>
#include <setjmp.h>

#ifndef __GB_COMMON_CHECK_C
EXTERN sigjmp_buf CHECK_jump;
#endif

PUBLIC void CHECK_enter(void);
PUBLIC void CHECK_leave(void);
PUBLIC bool CHECK_got_error(void);
PUBLIC bool CHECK_address(void *ptr, long len);
PUBLIC bool CHECK_strlen(char *ptr, long *len);

#endif
