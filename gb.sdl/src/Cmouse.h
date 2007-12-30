/***************************************************************************

  Cmouse.h

  Gambas extension using SDL

  (c) 2006 Laurent Carlier <lordheavy@users.sourceforge.net>
           Benoît Minisini <gambas@users.sourceforge.net>

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

#ifndef __CMOUSE_H
#define __CMOUSE_H

#include "gambas.h"
#include "main.h"

#include "SDL.h"

typedef
  struct {
    bool valid;
    int x;
    int y;
    int relx;
    int rely;
    int state;
    }
  CMOUSE_INFO;

#ifndef __CMOUSE_CPP
extern GB_DESC CMouse[];
extern CMOUSE_INFO CMOUSE_info;
#endif /* __CMOUSE_CPP */

#endif /* __CMOUSE_H */

