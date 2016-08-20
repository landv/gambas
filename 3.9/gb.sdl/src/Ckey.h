/***************************************************************************

  Ckey.h

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

#ifndef __CKEY_H
#define __CKEY_H

#include "gambas.h"
#include "main.h"

#include "SDL.h"
#include "SDL_syswm.h"
//#include "X11/keysym.h"

typedef
  struct {
    int valid;
		int code;           // SDL virtual keysym
		int state;          // modifier state
    //int cancel;
		char text[4];       // UTF-8 text
    }
  CKEY_INFO;

#ifndef __CKEY_CPP
extern GB_DESC CKey[];
extern CKEY_INFO CKEY_info;
#endif /* __CKEY_CPP */

#endif /* __CKEY_H */

