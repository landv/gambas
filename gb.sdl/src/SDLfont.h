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
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

***************************************************************************/

#ifndef __SDLFONT_H
#define __SDLFONT_H

#include <X11/Xlib.h>
#include <X11/Xft/Xft.h>

#include "SDL_ttf.h"

#include <vector>
#include <string>

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

	void SizeText(const char* text, int *width, int *height);
	SDLsurface* RenderText(const char* text);

private:
	void OpenFont(const char* file);
	
	int hfontsize;
	std::string hfontname;
	int hSDLfontstyle;

	/* SDL_TTF font */
	TTF_Font *hSDLfont;
};

#endif /* _SDLFONT_H */
