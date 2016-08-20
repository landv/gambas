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
#include <string>
#include <ctype.h>

#include "default_font.h"

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

#define UNICODE_INVALID 0xFFFFFFFFU

static const char _char_length[256] =
{
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
	3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,4,4,4,4,4,4,4,4,5,5,5,5,6,6,1,1
};

#define utf8_get_char_length(_c) ((int)_char_length[(unsigned char)(_c)])

static int utf8_get_length(const char *sstr, int len)
{
	const uchar *str = (const uchar *)sstr;
	int ulen;
	int i;

	ulen = 0;

	for (i = 0; i < len; i++)
	{
		if ((str[i] & 0xC0) != 0x80)
			ulen++;
	}

	return ulen;
}

static uint utf8_to_unicode(const char *sstr, int len)
{
	const uchar *str = (const uchar *)sstr;
	uint unicode;

	switch (len)
	{
		case 2:
			unicode = (str[1] & 0x3F) + ((str[0] & 0x1F) << 6);
			if (unicode < 0x80)
				goto _INVALID;
			break;

		case 3:
			unicode = (str[2] & 0x3F) + ((str[1] & 0x3F) << 6) + ((str[0] & 0xF) << 12);
			if (unicode < 0x800)
				goto _INVALID;
			break;

		case 4:
			unicode = (str[3] & 0x3F) + ((str[2] & 0x3F) << 6) + ((str[1] & 0x3F) << 12) + ((str[0] & 0x7) << 18);
			if (unicode < 0x10000)
				goto _INVALID;
			break;

		case 5:
			unicode = (str[4] & 0x3F) + ((str[3] & 0x3F) << 6) + ((str[2] & 0x3F) << 12) + ((str[1] & 0x3F) << 18) + ((str[0] & 0x3) << 24);
			if (unicode < 0x200000)
				goto _INVALID;
			break;

		case 6:
			unicode = (str[5] & 0x3F) + ((str[4] & 0x3F) << 6) + ((str[3] & 0x3F) << 12) + ((str[2] & 0x3F) << 18) + ((str[1] & 0x3F) << 24) + ((str[0] & 0x1) << 30);
			if (unicode < 0x4000000)
				goto _INVALID;
			break;

		default:
			unicode = str[0];
			break;
	}

	return unicode;

_INVALID:

	return UNICODE_INVALID;
}


static void render_default_font(uint *dest, int size, const char *text, int len)
{
	static void *jump[] = { &&__0, &&__1, &&__2, &&__3, &&__4, &&__5, &&__6, &&__7, &&__8, &&__9, &&__A, &&__B, &&__C, &&__D, &&__E, &&__F };
	static void *jump2[] = { &&__00, &&__10, &&__20, &&__30, &&__40, &&__50, &&__60, &&__70, &&__80, &&__90, &&__A0, &&__B0, &&__C0, &&__D0, &&__E0, &&__F0 };

	int lc;
	const char *src;
	uint *p;
	int y;
	uchar line;
	uint code;
	uint col = 0xFFFFFFFF;

	size *= DEFAULT_FONT_WIDTH;

	for(;;)
	{
		if (!*text)
			break;

		lc = utf8_get_char_length(*text);
		code = utf8_to_unicode(text, lc);
		text += lc;

		if (code >= 33 && code <= 126)
			src = _default_font_33_126 + DEFAULT_FONT_HEIGHT * (code - 33);
		else if (code >= 160 && code <= 687)
			src = _default_font_160_687 + DEFAULT_FONT_HEIGHT * (code - 160);
		else
			src = NULL;

		if (src)
		{
			p = dest;

			for (y = 0; y < DEFAULT_FONT_HEIGHT; y++)
			{
				line = *src++;
				if (!line)
				{
					p += size;
					continue;
				}

				goto *jump[line & 0xF];

				__1: p[0] = col; goto __0;
				__2: p[1] = col; goto __0;
				__3: p[0] = p[1] = col; goto __0;
				__4: p[2] = col; goto __0;
				__5: p[0] = p[2] = col; goto __0;
				__6: p[1] = p[2] = col; goto __0;
				__7: p[0] = p[1] = p[2] = col; goto __0;
				__8: p[3] = col; goto __0;
				__9: p[0] = p[3] = col; goto __0;
				__A: p[1] = p[3] = col; goto __0;
				__B: p[0] = p[1] = p[3] = col; goto __0;
				__C: p[2] = p[3] = col; goto __0;
				__D: p[0] = p[2] = p[3] = col; goto __0;
				__E: p[1] = p[2] = p[3] = col; goto __0;
				__F: p[0] = p[1] = p[2] = p[3] = col; goto __0;
				__0:

				goto *jump2[line >> 4];

				__10: p[4] = col; goto __00;
				__20: p[5] = col; goto __00;
				__30: p[4] = p[5] = col; goto __00;
				__40: p[6] = col; goto __00;
				__50: p[4] = p[6] = col; goto __00;
				__60: p[5] = p[6] = col; goto __00;
				__70: p[4] = p[5] = p[6] = col; goto __00;
				__80: p[7] = col; goto __00;
				__90: p[4] = p[7] = col; goto __00;
				__A0: p[5] = p[7] = col; goto __00;
				__B0: p[4] = p[5] = p[7] = col; goto __00;
				__C0: p[6] = p[7] = col; goto __00;
				__D0: p[4] = p[6] = p[7] = col; goto __00;
				__E0: p[5] = p[6] = p[7] = col; goto __00;
				__F0: p[4] = p[5] = p[6] = p[7] = col; goto __00;
				__00:

				p += size;
			}
		}

		dest += DEFAULT_FONT_WIDTH;
	}
}


//-------------------------------------------------------------------------

SDLfont::SDLfont(const char *fontfile)
{
	hfontsize = DEFAULT_FONT_HEIGHT;
	hSDLfont = 0;
	_last_surface = 0;
	_last_text = 0;

	/*if (!fontfile)
	{
		hfontname = GB.System.Path();
		hfontname += "/share/gambas" GAMBAS_VERSION_STRING "/gb.sdl/" DEFAULT_FONT;
	}
	else
		hfontname = fontfile;*/

	if (fontfile)
	{
		hfontname = fontfile;
		OpenFont(hfontname.c_str());
	}
}

SDLfont::~SDLfont()
{
	GB.FreeString(&_last_text);
	if (_last_surface)
		_last_surface->Unref();
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
	if (!hSDLfont)
		return "";
	else
		return hfontname.substr((hfontname.find_last_of("/"))+1).c_str();
}

void SDLfont::SetFontSize(int size)
{
	hfontsize = size;

	if (!hSDLfont)
		return;

	int style = TTF_GetFontStyle(hSDLfont);

	OpenFont(hfontname.c_str());
	TTF_SetFontStyle(hSDLfont, style);
}

int SDLfont::GetScale()
{
	if (hSDLfont)
		return 1;

	if (hfontsize <= DEFAULT_FONT_HEIGHT)
		return 1;

	return hfontsize / DEFAULT_FONT_HEIGHT;
}

void SDLfont::SetFontBold(bool state)
{
	if (!hSDLfont)
		return;

	if (state == (TTF_GetFontStyle(hSDLfont) & TTF_STYLE_BOLD))
		return;
	
	TTF_SetFontStyle(hSDLfont, (TTF_GetFontStyle(hSDLfont) ^ TTF_STYLE_BOLD));
}

bool SDLfont::IsFontBold(void )
{
	if (!hSDLfont)
		return false;
	else
		return (TTF_GetFontStyle(hSDLfont) & TTF_STYLE_BOLD);
}


void SDLfont::SetFontItalic(bool state)
{
	if (!hSDLfont)
		return;

	if (state == (TTF_GetFontStyle(hSDLfont) & TTF_STYLE_ITALIC))
		return;
	
	TTF_SetFontStyle(hSDLfont, (TTF_GetFontStyle(hSDLfont) ^ TTF_STYLE_ITALIC));
}

bool SDLfont::IsFontItalic(void )
{
	if (!hSDLfont)
		return false;
	else
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
	if (!hSDLfont)
		return;

	if (state == (TTF_GetFontStyle(hSDLfont) & TTF_STYLE_UNDERLINE))
		return;
	
	TTF_SetFontStyle(hSDLfont, (TTF_GetFontStyle(hSDLfont) ^ TTF_STYLE_UNDERLINE));
}

bool SDLfont::IsFontUnderline(void )
{
	if (!hSDLfont)
		return false;
	else
		return (TTF_GetFontStyle(hSDLfont) & TTF_STYLE_UNDERLINE);
}

bool SDLfont::IsFontScalable(void )
{
	if (!hSDLfont)
		return false;
	else
		return true;
}

int SDLfont::GetFontAscent(void )
{
	if (hSDLfont)
		return TTF_FontAscent(hSDLfont);
	else
		return DEFAULT_FONT_ASCENT * GetScale();
}

int SDLfont::GetFontDescent(void )
{
	if (hSDLfont)
		return TTF_FontDescent(hSDLfont);
	else
		return DEFAULT_FONT_DESCENT * GetScale();
}

bool SDLfont::IsFontFixed(void )
{
	if (hSDLfont)
		return (TTF_FontFaceIsFixedWidth(hSDLfont));
	else
		return true;
}

void SDLfont::SizeText(const char *text, int len, int *width, int *height)
{
	if (len == 0)
	{
		*width = 0;
		*height = GetFontAscent() + GetFontDescent();
		return;
	}

	if (hSDLfont)
	{
		TTF_SizeUTF8(hSDLfont, GB.TempString(text, len), width, height);
		return;
	}

	*width = utf8_get_length(text, len) * DEFAULT_FONT_WIDTH * GetScale();
	*height = DEFAULT_FONT_HEIGHT * GetScale();
}

SDLsurface* SDLfont::RenderText(const char *text, int len)
{
	SDL_Surface *result;

	if (len <= 0 || len >= 1024)
		return NULL;

	if (_last_surface)
	{
		if (len == GB.StringLength(_last_text) && strncmp(text, _last_text, len) == 0)
		{
			_last_surface->Ref();
			return _last_surface;
		}
	}

	if (hSDLfont)
	{
		SDL_Color fg = {0xFF, 0xFF, 0xFF};

		result = TTF_RenderUTF8_Blended(hSDLfont, GB.TempString(text, len), fg);
	}
	else
	{
		int size = utf8_get_length(text, len);

		result = SDL_CreateRGBSurface(SDL_SWSURFACE, size * DEFAULT_FONT_WIDTH, DEFAULT_FONT_HEIGHT, 32,
		#if SDL_BYTEORDER == SDL_BIG_ENDIAN
				0x0000FF00, 0x00FF0000, 0xFF000000, 0x000000FF);
		#else
				0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
		#endif

		if (SDL_MUSTLOCK(result))
			SDL_LockSurface(result);
		render_default_font((uint *)result->pixels, size, text, len);
		if (SDL_MUSTLOCK(result))
			SDL_UnlockSurface(result);
	}

	GB.FreeString(&_last_text);
	_last_text = GB.NewString(text, len);

	if (_last_surface)
		_last_surface->Unref();
	_last_surface = new SDLsurface(result);
	_last_surface->Ref();

	return _last_surface;
}

int SDLfont::GetDefaultFontSize()
{
	return DEFAULT_FONT_HEIGHT;
}
