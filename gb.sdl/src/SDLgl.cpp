/***************************************************************************

  SDLgl.cpp

  Gambas extension using SDL

  (c) 2006-2008 Laurent Carlier <lordheavy@users.sourceforge.net>
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

#include "SDLgl.h"

#include <string>
#include <iostream>

std::string Extensions = "";

void GL::init()
{
	Extensions = reinterpret_cast<const char*> (glGetString(GL_EXTENSIONS));
}

bool GL::CheckExtension(const char *extension)
{
	if (Extensions.find(extension) != std::string::npos)
		return true;
	else
		return false;
}