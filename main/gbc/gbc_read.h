/***************************************************************************

  gbc_read.h

  (c) 2000-2009 Benoît Minisini <gambas@users.sourceforge.net>

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
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

***************************************************************************/

#ifndef __GBC_READ_H
#define __GBC_READ_H

#include "gbc_read_common.h"

void READ_do(void);
void READ_dump_pattern(PATTERN *pattern);
char *READ_get_pattern(PATTERN *pattern);
void THROW_UNEXPECTED(PATTERN *pattern);

#endif
