/***************************************************************************

  header.h

  Analyzing class description

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

#ifndef __GBC_HEADER_H
#define __GBC_HEADER_H

#include "gbc_type.h"
#include "gb_reserved.h"
#include "gbc_read.h"
#include "gb_limit.h"
#include "gbc_trans.h"


enum {
  HF_NORMAL = 0,
  HF_NO_3PTS = 1,
  HF_EVENT = 2,
  HF_NO_OPT = 4,
  HF_VOID = 8
  };

enum {
  HS_ERROR = 0,
  HS_PUBLIC = 1,
  HS_STATIC = 2,
  HS_DYNAMIC = 4,
  HS_PROCEDURE = 8,
  HS_FUNCTION = 16,
  HS_PUT = 32,
  HS_UNKNOWN = 64,
  HS_NOPARAM = 128,
  HS_COMPARE = 256,
  };

typedef
  struct {
    char *name;
    int flag;
    }
  HEADER_SPECIAL;


PUBLIC void HEADER_do(void);

#endif

