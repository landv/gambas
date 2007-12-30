/***************************************************************************

  SDLfont.h

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

#ifndef __SDLFONT_H
#define __SDLFONT_H

#include <X11/Xlib.h>
#include <X11/Xft/Xft.h>

#include <vector>
#include <string>

typedef std::vector<std::string> StringList;

class SDLfont
{
public:
	SDLfont() {};
	~SDLfont() {};

	static StringList GetFontList(void );
};

#endif /* _SDLFONT_H */
