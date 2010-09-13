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
#include "gb_common.h"

#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>
#include <ctype.h>

typedef struct {
	std::string name;
	std::string realname;
	std::string foundry;
	std::string path;
	} fontdesc;
	
static std::vector<fontdesc> fontDB;

static StringList _FontList;
Display *SDLfont::display = 0;
int SDLfont::screen = 0;

#define DEFAULT_FONT_SIZE 20
#define DEFAULT_DPI 72 /* Default DPI size in SDL_TTF */

inline bool cmp_db_nocase(const fontdesc x, const fontdesc y)
{
	std::string a = x.name, b = y.name;
	transform(a.begin(), a.end(), a.begin(), tolower);
	transform(b.begin(), b.end(), b.begin(), tolower);
	return (b>a) ? 1 : 0;
}

StringList SDLfont::GetFontList(void )
{
	if (LIKELY(_FontList.empty() == 0))
		return _FontList;

	Display *disp = SDLapp->X11appDisplay();
	XftFontSet *FntnameSet = XftListFonts(disp, DefaultScreen(disp), NULL,
		XFT_FAMILY, /*XFT_FILE, XFT_FOUNDRY,*/ NULL);
	int i;
	
	// Get the fonts name
	for (i = 0; i < FntnameSet->nfont; i++)	{
		char *name[255];
		char *foundry[255];
		fontdesc font;
		unsigned int j;
		
		XftResult res = XftPatternGetString(FntnameSet->fonts[i], XFT_FAMILY, 0, name);

		if (res!=XftResultMatch)
			continue;

		XftFontSet *Fntdetail = XftListFonts(disp, DefaultScreen(disp),
			XFT_FAMILY, XftTypeString, name[0], 0,
			XFT_FOUNDRY, NULL);
		j = Fntdetail->nfont;
		
		if (j>1) {
			while (j) {
				XftPatternGetString(Fntdetail->fonts[j-1], XFT_FOUNDRY, 0, foundry);
				font.name = font.realname = name[0];
				font.foundry = foundry[0];
				font.name = font.name + " [" +font.foundry+ "]";
				fontDB.push_back(font);
				j--;
			}
		}
		else {
			font.name = font.realname = name[0];
			XftPatternGetString(Fntdetail->fonts[0], XFT_FOUNDRY, 0, foundry);
			font.foundry = foundry[0];
			fontDB.push_back(font);
		}
		XFree(Fntdetail);
	}

	std::sort(fontDB.begin(), fontDB.end(), cmp_db_nocase);
	XFree(FntnameSet);

	std::string font = "";
	i = 0;
	while (i<int(fontDB.size())) {
		if (!fontDB[i].name.compare(font)) {
			fontDB.erase(fontDB.begin() + i);
			i++;
			continue;
		}
		font = fontDB[i++].name;
	}
	
	i = 0;
	while (i<int(fontDB.size())) {
		char *res[255];
		
		XftFontSet *Fntdetail = XftListFonts(disp, DefaultScreen(disp),
				XFT_FAMILY, XftTypeString, fontDB[i].realname.c_str(),
				XFT_FOUNDRY, XftTypeString, fontDB[i].foundry.c_str(), 0,
				XFT_FILE, NULL);
		XftPatternGetString(Fntdetail->fonts[0], XFT_FILE, 0, res);
		fontDB[i].path = res[0];
		_FontList.push_back(fontDB[i].name);
		XFree(Fntdetail);
		i++;
	}

	return _FontList;
}

void SDLfont::Init()
{
	display = SDLapp->X11appDisplay();
	screen = DefaultScreen(display);
}

void SDLfont::Exit()
{
}

SDLfont::SDLfont()
{
	hfonttype = X_font;
	hfontsize = DEFAULT_FONT_SIZE;
	hfontindex = 0;
	
	SDLfont::GetFontList();
	hfontname = fontDB[hfontindex].path;
	hSDLfont = TTF_OpenFont(hfontname.c_str(), hfontsize);

	if (UNLIKELY(hSDLfont == NULL))
		SDLerror::RaiseError(TTF_GetError());
}

SDLfont::SDLfont(char *fontfile)
{
	hfonttype = SDLTTF_font;
	hfontsize = DEFAULT_FONT_SIZE;
	hfontname = fontfile;

	hSDLfont = TTF_OpenFont(fontfile, hfontsize);

	if (UNLIKELY(hSDLfont == NULL))
		SDLerror::RaiseError(TTF_GetError());
}

SDLfont::~SDLfont()
{
	if (hSDLfont)
		TTF_CloseFont(hSDLfont);
}

void SDLfont::SetFontName(char* name)
{
}

const char* SDLfont::GetFontName(void )
{
	if (hfonttype == SDLTTF_font) {
		std::string name; 
		name = hfontname.substr((hfontname.find_last_of("/"))+1);
		return name.c_str();
	}
	else
		return fontDB[hfontindex].name.c_str();
}

void SDLfont::SetFontSize(int size)
{
	hfontsize = size;

	if (hSDLfont)
		TTF_CloseFont(hSDLfont);

	hSDLfont = TTF_OpenFont(hfontname.c_str(), hfontsize);

	if (UNLIKELY(hSDLfont == NULL))
		SDLerror::RaiseError(TTF_GetError());
}

void SDLfont::SetFontUnderline(bool state)
{
}

bool SDLfont::IsFontUnderlined(void )
{
	return false;
}

bool SDLfont::IsFontScalable(void )
{
	return true;
}

int SDLfont::GetFontAscent(void )
{
	return TTF_FontAscent(hSDLfont);
}

int SDLfont::GetFontDescent(void )
{
	return TTF_FontDescent(hSDLfont);
}

bool SDLfont::IsFontFixed(void )
{
	return (TTF_FontFaceIsFixedWidth(hSDLfont));
}

void SDLfont::SizeText(const char *text, int *width, int *height)
{
	TTF_SizeText(hSDLfont, text, width, height);
}

SDLsurface* SDLfont::RenderText(const char* text)
{
	SDL_Color fg = {0xFF, 0xFF, 0xFF};

	SDL_Surface *result = TTF_RenderUTF8_Blended(hSDLfont, text, fg);
	SDLsurface *surf = new SDLsurface(result);
	return (surf);
}