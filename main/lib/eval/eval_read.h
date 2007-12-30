/***************************************************************************

  eval_read.h

  Lexical parser

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

#ifndef __EVAL_READ_H
#define __EVAL_READ_H

#include "gbc_read_common.h"

#ifndef __EVAL_READ_C
EXTERN const char *READ_source_ptr;
#endif

PUBLIC void EVAL_read(void);
PUBLIC char *READ_get_pattern(PATTERN *pattern);

#endif
