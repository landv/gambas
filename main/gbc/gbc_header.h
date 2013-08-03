/***************************************************************************

  gbc_header.h

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
  HF_VOID = 8,
	HF_NO_BYREF = 16
  };

enum {
  HS_ERROR 			= 0,
  HS_PUBLIC 		= 1 << 0,
  HS_STATIC 		= 1 << 1,
  HS_DYNAMIC 		= 1 << 2,
  HS_PROCEDURE 	= 1 << 3,
  HS_FUNCTION 	= 1 << 4,
  HS_PUT 				= 1 << 5,
  HS_UNKNOWN 		= 1 << 6,
  HS_PROPERTY		= 1 << 7,
  HS_NOPARAM 		= 1 << 8,
  HS_COMPARE 		= 1 << 9,
	HS_ATTACH 		= 1 << 10
  };

typedef
  struct {
    char *name;
    int flag;
    }
  HEADER_SPECIAL;


void HEADER_do(void);

#endif

