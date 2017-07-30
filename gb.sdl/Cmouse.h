/***************************************************************************

  Cmouse.h

  (c) 2006 Laurent Carlier <lordheavy@users.sourceforge.net>

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

#ifndef __CMOUSE_H
#define __CMOUSE_H

#include "gambas.h"
#include "main.h"

#include "SDL.h"
#include "SDLcursor.h"

typedef
  struct {
    bool valid;
    int x;
    int y;
    int relx;
    int rely;
    int state;
    SDLMod keymod;
    }
  CMOUSE_INFO;

#ifndef __CMOUSE_CPP
extern GB_DESC CMouse[];
//extern GB_DESC CCursor[];
extern CMOUSE_INFO CMOUSE_info;
#else

#define THIS ((CCURSOR *)_object)

#endif /* __CMOUSE_CPP */

typedef
  struct _CCURSOR {
    GB_BASE ob;
    int x;
    int y;
    SDLcursor *cursor;
    }
  CCURSOR;

#endif /* __CMOUSE_H */

