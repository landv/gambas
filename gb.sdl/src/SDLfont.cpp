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

static StringList _FontList;
Display *SDLfont::display = 0;
int SDLfont::screen = 0;
XftColor SDLfont::background, SDLfont::foreground;

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
	if (LIKELY(_FontList.empty() == 0))
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

void SDLfont::Init()
{
	XRenderColor backg, foreg;
	display = SDLapp->X11appDisplay();
	screen = DefaultScreen(display);
	
	// full white
	foreg.red = foreg.green = foreg.blue = backg.alpha = 0xFFFF;
	//transparent black
	backg.red = backg.green = backg.blue = foreg.alpha = 0x0000;
	
	if (!XftColorAllocValue(display , DefaultVisual(display, screen),
		DefaultColormap(display, screen), &foreg, &foreground))
		std::cerr << "error XftColorAllocValue() foreground" << std::endl;
	
	if (!XftColorAllocValue(display , DefaultVisual(display, screen),
		DefaultColormap(display, screen), &backg, &background))
		std::cerr << "error XftColorAllocValue() background" << std::endl;

}

void SDLfont::Exit()
{
	XftColorFree(display, DefaultVisual(display, screen), DefaultColormap(display, screen), &foreground);
	XftColorFree(display, DefaultVisual(display, screen), DefaultColormap(display, screen), &background);
}

SDLfont::SDLfont()
{
	hfonttype = X_font;
	char fnt[512];
	
	snprintf(fnt, 512, "Arial,helvetica-%d:medium:slant=roman:dpi=%d",
		DEFAULT_FONT_SIZE, DEFAULT_DPI);
		
	hXfont = XftFontOpenName(display, screen, fnt);
//	XftNameUnparse(hXfont->pattern, fnt, 512);
//	std::cout << fnt << std::endl;

	if (UNLIKELY(hXfont == NULL))
		SDLerror::RaiseError("Failed to open default font!");
}

SDLfont::SDLfont(char *fontfile)
{
	hfonttype = SDLTTF_font;
	hfontname = fontfile;
	hfontsize = DEFAULT_FONT_SIZE;

	hSDLfont = TTF_OpenFont(fontfile, hfontsize);

	if (UNLIKELY(hSDLfont == NULL))
		SDLerror::RaiseError(TTF_GetError());

}

SDLfont::~SDLfont()
{
	if ((hfonttype == SDLTTF_font) && hSDLfont)
		TTF_CloseFont(hSDLfont);

	if ((hfonttype == X_font) && hXfont)
		XftFontClose(display, hXfont);
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

		if (UNLIKELY(hSDLfont == NULL))
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

bool SDLfont::IsFontScalable(void )
{
	if (hfonttype == SDLTTF_font)
		return true;
	else
	{
		Bool scalable;
		
		XftPatternGetBool(hXfont->pattern,
				XFT_SCALABLE, 0, &scalable);
		return (scalable);
	}		
}

int SDLfont::GetFontAscent(void )
{
	if (hfonttype == SDLTTF_font)
		return TTF_FontAscent(hSDLfont);
	else 
		return (hXfont->ascent);
}

int SDLfont::GetFontDescent(void )
{
	if (hfonttype == SDLTTF_font)
		return TTF_FontDescent(hSDLfont);
	else 
		return (hXfont->descent);
}

bool SDLfont::IsFontFixed(void )
{
	if (hfonttype == SDLTTF_font)
		return (TTF_FontFaceIsFixedWidth(hSDLfont));
	else
	{
		int spacing;
		
		XftPatternGetInteger(hXfont->pattern,
				XFT_SPACING, 0, &spacing);
		return (spacing >= XFT_MONO);
	}		
}

void SDLfont::SizeText(const char *text, int *width, int *height)
{
	if (hfonttype == SDLTTF_font)
		TTF_SizeText(hSDLfont, text, width, height);
	else
	{
		XGlyphInfo glyphinfo;
		
		XftTextExtentsUtf8(display , hXfont, (XftChar8* )text, strlen(text), &glyphinfo);
		*width = int(glyphinfo.width);
		*height = int(glyphinfo.height);
	}		
}

SDLsurface* SDLfont::RenderText(const char* text)
{
	if (hfonttype == SDLTTF_font)
	{
		SDL_Color fg = {0xFF, 0xFF, 0xFF};

		SDL_Surface *result = TTF_RenderUTF8_Blended(hSDLfont, text, fg);
		SDLsurface *surf = new SDLsurface(result);
		return (surf);
	}
	else
	{
		XGlyphInfo glyphinfo;

		SDLapp->LockX11();
		XftTextExtentsUtf8(display , hXfont, (XftChar8* )text, strlen(text), &glyphinfo);
		// Create the pixmap to draw on.  
		Pixmap pixmap = XCreatePixmap(display, DefaultRootWindow(display), glyphinfo.width,
					glyphinfo.height, DefaultDepth(display, screen));
					
		// And the Xft wrapper around it.  
		XftDraw *draw = XftDrawCreate(display, pixmap, DefaultVisual(display, screen),
					DefaultColormap(display, screen));  
		XftDrawRect(draw, &background, 0, 0, glyphinfo.width, glyphinfo.height);
		XftDrawString8(draw, &foreground, hXfont, 0, glyphinfo.height-1 , (XftChar8 *)text, strlen(text));
		XImage *img = XGetImage(display, pixmap, 0, 0, glyphinfo.width, glyphinfo.height, AllPlanes, ZPixmap);
		SDLsurface *surf = new SDLsurface(glyphinfo.width, glyphinfo.height);
		Uint8 *data = (Uint8 *) surf->GetData();
		
		for (int y = 0; y < glyphinfo.height; y++)
		{
			for (int x = 0; x < glyphinfo.width; x++)
			{
				Uint32 pixel = (Uint32 )XGetPixel(img, x, y);
				((Uint32* )surf->GetData())[x + (y * glyphinfo.width)] = XGetPixel(img, x, y);
				/* Create alpha value depending of pixel value (lowest R,G,B value)*/
				data[3] = (pixel & 0xFF) & ((pixel >> 8) & 0xFF) &
						((pixel >> 16) & 0xFF);
				data += 4;
			}
		}
		// clean up
		XDestroyImage(img); 
		XftDrawDestroy(draw);  
		XFreePixmap(display, pixmap);  
		SDLapp->UnlockX11();
		return (surf);
	}
}