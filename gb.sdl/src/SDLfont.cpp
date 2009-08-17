/***************************************************************************

  SDLfont.cpp

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
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

***************************************************************************/

#include "SDLfont.h"
#include "SDLapp.h"

#include <algorithm>
#include <iostream>
#include <string>
#include <ctype.h>

static StringList _FontList;

#define DEFAULT_FONT_SIZE 20
#define DEFAULT_DPI 72 /* Default DPI size in SDL_TTF */

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

            if (res!=XftResultMatch)
            {
                  XFree(hSet);
                  SDLerror::RaiseError("Failed to list system fonts !");
                  return _FontList;
            }

		_FontList.push_back(name[0]);
	}

	std::sort(_FontList.begin(), _FontList.end(), cmp_nocase);

	XFree(hSet);

	return _FontList;
}

SDLfont::SDLfont()
{
	hfonttype = X_font;
}

SDLfont::SDLfont(char *fontfile)
{
	hfonttype = SDLTTF_font;
	hfontname = fontfile;
	hfontsize = DEFAULT_FONT_SIZE;

	hSDLfont = TTF_OpenFont(fontfile, hfontsize);

	if (!hSDLfont)
		SDLerror::RaiseError(TTF_GetError());

}

SDLfont::~SDLfont()
{
	if ((hfonttype == SDLTTF_font) && hSDLfont)
		TTF_CloseFont(hSDLfont);
}

void SDLfont::SetFontName(char* name)
{
}

const char* SDLfont::GetFontName(void )
{
	if (hfonttype == SDLTTF_font)
	{
		std::string name; 
		name = hfontname.substr((hfontname.find_last_of("/"))+1);
		return name.c_str();
	}
	else 
		return hfontname.c_str();
}

void SDLfont::SetFontSize(int size)
{
	hfontsize = size;

	if (hfonttype == SDLTTF_font)
	{
		if (hSDLfont)
			TTF_CloseFont(hSDLfont);

		hSDLfont = TTF_OpenFont(hfontname.c_str(), hfontsize);

		if (!hSDLfont)
			SDLerror::RaiseError(TTF_GetError());
	}
}

void SDLfont::SetFontUnderline(bool state)
{
	if (hfonttype == SDLTTF_font)
	{
	}
}

bool SDLfont::IsFontUnderlined(void )
{
	if (hfonttype == SDLTTF_font)
	{
	}

	return false;
}

int SDLfont::GetFontAscent(void )
{
	if (hfonttype == SDLTTF_font)
		return TTF_FontAscent(hSDLfont);

	return 0;
}

int SDLfont::GetFontDescent(void )
{
	if (hfonttype == SDLTTF_font)
		return TTF_FontDescent(hSDLfont);

	return 0;
}

bool SDLfont::IsFontFixed(void )
{
	if (hfonttype == SDLTTF_font)
		return (TTF_FontFaceIsFixedWidth(hSDLfont));

	return false;
}

SDLsurface* SDLfont::RenderText(const char* text)
{
	SDLsurface* surf;

	if (hfonttype == SDLTTF_font)
	{
		 SDL_Color fg = {(hForeColor >> 24) & 0xFF, (hForeColor >> 16) & 0xFF, (hForeColor >> 8) & 0xFF};
		 SDL_Surface *result = TTF_RenderUTF8_Blended(hSDLfont, text, fg);
		 surf = new SDLsurface(result);
	}

	return surf;
}
