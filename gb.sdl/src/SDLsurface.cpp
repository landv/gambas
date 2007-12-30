/***************************************************************************

  SDLsurface.cpp

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

#include <iostream>

#include "SDLsurface.h"
#include "SDLcore.h"

#include "SDL_image.h"

// debugging
// #define DEBUGSURFACE

SDLsurface::SDLsurface()
{
	hSurfaceInfo = new SDL_INFO;
	hSurface = 0;
	hTexture = 0;
	hTextureWidth = 0;
	hTextureHeight = 0;
	hContext = NULL;
}

SDLsurface::~SDLsurface()
{
	if (hSurface)
		SDL_FreeSurface(hSurface);

	if (hTexture)
		glDeleteTextures(1, &hTexture);

	delete hSurfaceInfo;
}

void SDLsurface::Create(int Width, int Height, int Depth)
{
	SDL_Surface *pSurface = SDL_CreateRGBSurface(SDL_SWSURFACE, Width, Height, Depth,
#if SDL_BYTEORDER == SDL_LIL_ENDIAN /* OpenGL RGBA masks */
			0x000000FF, 
			0x0000FF00, 
			0x00FF0000, 
			0xFF000000
#else
			0xFF000000,
			0x00FF0000, 
			0x0000FF00, 
			0x000000FF
#endif
			);

	if (!pSurface)
		SDLcore::RaiseError(SDL_GetError());
	else
	{
		if (hSurface)
			SDL_FreeSurface(hSurface);

		hSurface = pSurface;
	}
	hTextureStatus = TEXTURE_TO_RELOAD;
}

void SDLsurface::LoadFromMem(char *addr, long len)
{
	SDL_Surface *surface = IMG_Load_RW(SDL_RWFromMem(addr, len), true);

	if (!surface)
		SDLcore::RaiseError(SDL_GetError());

	if (hSurface)
		SDL_FreeSurface(hSurface);

	hSurface = surface;
	hTextureStatus = TEXTURE_TO_RELOAD;

#ifdef DEBUGSURFACE
	if (hSurface->flags & SDL_SRCALPHA)
		std::cout << "SDLsurface::LoadFromMem : alpha layer detected" << std::endl;
#endif
}

int SDLsurface::GetWidth()
{
	if (hSurface)
		return (hSurface->w);
	else
		return (0);
}

int SDLsurface::GetHeight()
{
	if (hSurface)
		return (hSurface->h);
	else
		return (0);
}

int SDLsurface::GetDepth()
{
	if (hSurface)
		return (hSurface->format->BitsPerPixel);
	else
		return (0);
}

void* SDLsurface::GetData()
{
	if (hSurface)
		return (hSurface->pixels);
	else
		return (0);
}

void SDLsurface::SetAlphaBuffer(bool choice)
{
	if (!hSurface)
		return;

	Uint32 myflags = 0;

	if (choice)
	 	myflags = SDL_SRCALPHA;

	if ((SDL_SetAlpha(hSurface, myflags, SDL_ALPHA_OPAQUE))<0)
		SDLcore::RaiseError(SDL_GetError());
}

void SDLsurface::ConvertDepth(int depth)
{
	if (!hSurface)
		return;

	if (hSurface->format->BitsPerPixel == depth)
		return;

#ifdef DEBUGSURFACE
	std::cout << "SDLsurface::ConvertDepth : converting from " <<
		int(hSurface->format->BitsPerPixel) << " to " << depth << std::endl;
#endif

	SDL_Surface *tmpSurf = SDL_CreateRGBSurface(hSurface->flags, 1, 1, depth,
#if SDL_BYTEORDER == SDL_LIL_ENDIAN /* OpenGL RGBA masks */
			0x000000FF, 
			0x0000FF00, 
			0x00FF0000, 
			0xFF000000
#else
			0xFF000000,
			0x00FF0000, 
			0x0000FF00, 
			0x000000FF
#endif
			);


	if (!tmpSurf)
	{
		SDLcore::RaiseError(SDL_GetError());
		return;
	}

	SDL_Surface *surface = SDL_ConvertSurface(hSurface, tmpSurf->format, tmpSurf->flags);

	if (!surface)
	{
		SDLcore::RaiseError(SDL_GetError());
		return;
	}

	SDL_FreeSurface(tmpSurf);
	SDL_FreeSurface(hSurface);

	hSurface = surface;
	hTextureStatus = TEXTURE_TO_RELOAD;
}

void SDLsurface::Fill(Uint32 color)
{
	if (!hSurface)
		return;

	SDL_FillRect(hSurface, NULL, color);
	hTextureStatus = TEXTURE_TO_RELOAD;
}

void SDLsurface::Resize(int width, int height)
{
	if (!hSurface)
		return;

	Uint32 saved_flags = hSurface->flags & (SDL_SRCALPHA | SDL_RLEACCELOK);;
	Uint8  saved_alpha = hSurface->format->alpha;
 
	if ((saved_flags & SDL_SRCALPHA) == SDL_SRCALPHA)
		SDL_SetAlpha(hSurface, 0, 0);

	SDL_Surface *tmpSurf = SDL_CreateRGBSurface (SDL_SWSURFACE, width, height, 
		hSurface->format->BitsPerPixel, 0, 0, 0, 0);

	if (!tmpSurf)
	{
		SDLcore::RaiseError(SDL_GetError());
		return;
	}

	/* Copy the surface into the GL texture image */
	SDL_BlitSurface(hSurface, NULL, tmpSurf, NULL);

	/* Restore the alpha blending attributes */
	if ((saved_flags & SDL_SRCALPHA) == SDL_SRCALPHA )
		SDL_SetAlpha(tmpSurf, saved_flags, saved_alpha);

	SDL_FreeSurface(hSurface);
	hSurface = tmpSurf;
	hTextureStatus = TEXTURE_TO_RELOAD;
}
