/***************************************************************************

  SDLfont.h

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

#ifndef __SDLFONT_H
#define __SDLFONT_H

#if 0
#include <X11/Xlib.h>
#include <X11/Xft/Xft.h>
#endif

#include "SDL_ttf.h"

#if 0
#include <vector>
#include <string>
#endif

#include "SDLsurface.h"
#if 0
typedef std::vector<std::string> StringList;

enum _fonttype {
	SDLTTF_font = 1,
	X_font};
#endif
class SDLfont
{
public:
	SDLfont(const char* fontfile = 0);
	~SDLfont();

//	static StringList GetFontList(void );
//	static void Init(void );
	
//	void SetFontName(char* name);
	void SetFontSize(int size);
	void SetFontBold(bool state);
	void SetFontItalic(bool state);
	void SetFontStrikeout(bool state);
	void SetFontUnderline(bool state);

	const char* GetFontName(void );
	int GetFontSize(void ) { return hfontsize; };
	int GetFontAscent(void );
	int GetFontDescent(void );

	bool IsFontFixed(void );
	bool IsFontBold(void );
	bool IsFontItalic(void );
	bool IsFontStrikeout(void );
	bool IsFontUnderline(void );
	bool IsFontScalable(void );

	void SizeText(const char* text, int len, int *width, int *height);
	SDLsurface* RenderText(const char *text, int len);

	int GetScale();
	static int GetDefaultFontSize();

private:
	void OpenFont(const char* file);
	
	SDLsurface *_last_surface;
	char *_last_text;

	int hfontsize;
	std::string hfontname;
	int hSDLfontstyle;

	/* SDL_TTF font */
	TTF_Font *hSDLfont;
};

#endif /* _SDLFONT_H */
