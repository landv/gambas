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
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
	MA 02110-1301, USA.

***************************************************************************/

#include "SDLfont.h"
#include "SDLapp.h"
#include "gb_common.h"
#include "main.h"

//#include <algorithm>
//#include <iostream>
//#include <sstream>
#include <string>
#include <ctype.h>

#define DEFAULT_FONT_SIZE 20
#define DEFAULT_FONT "DejaVuSans.ttf"
/*
#ifndef TTF_STYLE_STRIKETHROUGH
#define TTF_STYLE_STRIKETHROUGH	0x08
#endif
*/
#if 0
typedef struct {
	std::string name;
	std::string realname;
	std::string foundry;
	std::string path;
	} fontdesc;
	
static std::vector<fontdesc> fontDB;
static StringList _FontList;

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
	return _FontList;
}

void SDLfont::Init()
{
	Display *disp = XOpenDisplay(NULL);
	int scr = XDefaultScreen(disp);
	int i;

	XftFontSet *FntnameSet = XftListFonts(disp, scr, 0,
		XFT_FAMILY, NULL);
	
	// Get the fonts name
	for (i = 0; i < FntnameSet->nfont; i++)	{
		char *name[255];
		char *foundry[255];
		fontdesc font;
		unsigned int j;
		
		XftResult res = XftPatternGetString(FntnameSet->fonts[i], XFT_FAMILY, 0, name);

		if (res!=XftResultMatch)
			continue;

		XftFontSet *Fntdetail = XftListFonts(disp, scr,
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
		
		XftFontSet *Fntdetail = XftListFonts(disp, scr,
				XFT_FAMILY, XftTypeString, fontDB[i].realname.c_str(),
				XFT_FOUNDRY, XftTypeString, fontDB[i].foundry.c_str(), 0,
				XFT_FILE, NULL);
		XftPatternGetString(Fntdetail->fonts[0], XFT_FILE, 0, res);
		fontDB[i].path = res[0];
		_FontList.push_back(fontDB[i].name);
		XFree(Fntdetail);
		i++;
	}
	
	XCloseDisplay(disp);
}
#endif
SDLfont::SDLfont(const char *fontfile)
{
	hfontsize = DEFAULT_FONT_SIZE;

	if (!fontfile)
	{
		hfontname = GB.System.Path();
		hfontname += "/share/gambas" GAMBAS_VERSION_STRING "/gb.sdl/" DEFAULT_FONT;
	}
	else
		hfontname = fontfile;
	
	hSDLfont = 0;

	OpenFont(hfontname.c_str());
}

SDLfont::~SDLfont()
{
	if (hSDLfont)
		TTF_CloseFont(hSDLfont);
}

void SDLfont::OpenFont(const char* file)
{
	if (hSDLfont)
		TTF_CloseFont(hSDLfont);

	hSDLfont = TTF_OpenFont(file, hfontsize);
	
	if (UNLIKELY(hSDLfont == NULL))
		SDLerror::RaiseError(TTF_GetError());
}
#if 0
void SDLfont::SetFontName(char* name)
{
	std::string font = name;
	int i=0;
	
	while (i<int(fontDB.size())) {
		if (!fontDB[i].name.compare(font)) {
			hfonttype = X_font;
			hfontsize = DEFAULT_FONT_SIZE;
			hfontindex = i;

			hfontname = fontDB[hfontindex].path;
			OpenFont(hfontname.c_str());
			return;
		}
		i++;
	}
	SDLerror::RaiseError("Font not found!");
}
#endif
const char* SDLfont::GetFontName(void )
{
	return hfontname.substr((hfontname.find_last_of("/"))+1).c_str();
}

void SDLfont::SetFontSize(int size)
{
	int style = TTF_GetFontStyle(hSDLfont);
	hfontsize = size;

	OpenFont(hfontname.c_str());
	TTF_SetFontStyle(hSDLfont, style);
}

void SDLfont::SetFontBold(bool state)
{
	if (state == (TTF_GetFontStyle(hSDLfont) & TTF_STYLE_BOLD))
		return;
	
	TTF_SetFontStyle(hSDLfont, (TTF_GetFontStyle(hSDLfont) ^ TTF_STYLE_BOLD));
}

bool SDLfont::IsFontBold(void )
{
	return (TTF_GetFontStyle(hSDLfont) & TTF_STYLE_BOLD);
}


void SDLfont::SetFontItalic(bool state)
{
	if (state == (TTF_GetFontStyle(hSDLfont) & TTF_STYLE_ITALIC))
		return;
	
	TTF_SetFontStyle(hSDLfont, (TTF_GetFontStyle(hSDLfont) ^ TTF_STYLE_ITALIC));
}

bool SDLfont::IsFontItalic(void )
{
	return (TTF_GetFontStyle(hSDLfont) & TTF_STYLE_ITALIC);
}

void SDLfont::SetFontStrikeout(bool state)
{
/*
	if (state == (TTF_GetFontStyle(hSDLfont) & TTF_STYLE_STRIKETHROUGH))
		return;
	
	TTF_SetFontStyle(hSDLfont, (TTF_GetFontStyle(hSDLfont) ^ TTF_STYLE_STRIKETHROUGH));
*/
}

bool SDLfont::IsFontStrikeout(void )
{
//	return (TTF_GetFontStyle(hSDLfont) & TTF_STYLE_STRIKETHROUGH);
	return false;
}

void SDLfont::SetFontUnderline(bool state)
{
	if (state == (TTF_GetFontStyle(hSDLfont) & TTF_STYLE_UNDERLINE))
		return;
	
	TTF_SetFontStyle(hSDLfont, (TTF_GetFontStyle(hSDLfont) ^ TTF_STYLE_UNDERLINE));
}

bool SDLfont::IsFontUnderline(void )
{
	return (TTF_GetFontStyle(hSDLfont) & TTF_STYLE_UNDERLINE);
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
	TTF_SizeUTF8(hSDLfont, text, width, height);
}

SDLsurface* SDLfont::RenderText(const char* text)
{
	SDL_Color fg = {0xFF, 0xFF, 0xFF};

	SDL_Surface *result = TTF_RenderUTF8_Blended(hSDLfont, text, fg);
	SDLsurface *surf = new SDLsurface(result);
	return (surf);
}