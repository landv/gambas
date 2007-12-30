/***************************************************************************

  SDLfont.cpp

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

#include "SDLfont.h"
#include "SDLapp.h"

#include <iostream>
#include <string>
#include <ctype.h>

static StringList _FontList;

inline bool cmp_nocase(const std::string x, const std::string y)
{
	std::string a = x, b = y;
	transform(a.begin(), a.end(), a.begin(), tolower);
	transform(b.begin(), b.end(), b.begin(), tolower);
	return (b>a) ? 1 : 0;
}

StringList SDLfont::GetFontList(void )
{
	if (!_FontList.empty())
		return _FontList;

	Display *disp = SDLapp->X11appDisplay();
	XftFontSet *hSet = XftListFonts(disp, DefaultScreen(disp), NULL, XFT_FAMILY, NULL);

	// Get the fonts name
	for (int i=0; i<hSet->nfont; i++)
	{
		char *name[255];
		XftResult res = XftPatternGetString(hSet->fonts[i], XFT_FAMILY, 0, name);

		_FontList.push_back(name[0]);
	}

	std::sort(_FontList.begin(), _FontList.end(), cmp_nocase);

	XFree(hSet);

	return _FontList;

}

