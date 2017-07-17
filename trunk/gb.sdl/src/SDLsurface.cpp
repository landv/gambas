/***************************************************************************

  SDLsurface.cpp

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

#include <iostream>

#include "SDLsurface.h"
#include "SDLcore.h"
#include "SDLtexture.h"

// debugging
// #define DEBUGSURFACE

SDLsurface::SDLsurface()
{
	hTexture = new SDLtexture(this);
	hSurface = 0;
	ref = 1;
}

SDLsurface::SDLsurface(char *data, int width, int height)
{
	hTexture = new SDLtexture(this);
	ref = 1;

	hSurface = SDL_CreateRGBSurfaceFrom((void *)data, width, height, 32, width * sizeof(int), 
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
		0x0000FF00, 0x00FF0000, 0xFF000000, 0x000000FF);
#else
		0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
#endif

	if (!hSurface)
		SDLcore::RaiseError(SDL_GetError());
	else
		hTexture->ToLoad();
}

SDLsurface::SDLsurface(const SDLsurface& surf)
{
#define cpySurf surf.hSurface

	ref = 1;
	hTexture = new SDLtexture(this);
	hSurface = 0;

	Create(cpySurf->w, cpySurf->h, cpySurf->format->BitsPerPixel);

	if (!hSurface->w || !hSurface->h)
		return;  // no surface to copy

	// now we are copying the surface
	Uint32 saved_flags;
	Uint8  saved_alpha;

	saved_flags = cpySurf->flags & (SDL_SRCALPHA | SDL_RLEACCELOK);
	saved_alpha = cpySurf->format->alpha;

	if ((saved_flags & SDL_SRCALPHA) == SDL_SRCALPHA)
		SDL_SetAlpha(cpySurf, 0, 0);

	/* Copy into the new surface */
	surf.hTexture->Sync();
	SDL_BlitSurface(cpySurf, NULL, hSurface, NULL);

	/* Restore the alpha blending attributes & set same attributes to new surface */
	if ((saved_flags & SDL_SRCALPHA) == SDL_SRCALPHA )
	{
		SDL_SetAlpha(cpySurf, saved_flags, saved_alpha);
		SDL_SetAlpha(hSurface, saved_flags, saved_alpha);
	}

#undef cpySurf
}

SDLsurface::SDLsurface(SDL_Surface* surf)
{
	ref = 1;
	hTexture = new SDLtexture(this);
	hSurface = surf;
	hTexture->ToLoad();
}

SDLsurface::SDLsurface(int Width, int Height)
{
	ref = 1;
	hTexture = new SDLtexture(this);
	hSurface = SDL_CreateRGBSurface(SDL_SWSURFACE, Width, Height, 32,
		0x0000FF00, 0x00FF0000, 0xFF000000, 0x000000FF);

	if (!hSurface)
		SDLcore::RaiseError(SDL_GetError());
	
	hTexture->ToLoad();
}

SDLsurface::~SDLsurface()
{
	if (hSurface)
		SDL_FreeSurface(hSurface);

	if (hTexture)
		delete hTexture;
}

void SDLsurface::Create(int Width, int Height, int Depth)
{
	SDL_Surface *pSurface = SDL_CreateRGBSurface(SDL_SWSURFACE, Width, Height, Depth,
		0x0000FF00, 0x00FF0000, 0xFF000000, 0x000000FF);

	if (!pSurface)
		SDLcore::RaiseError(SDL_GetError());
	else
	{
		if (hSurface)
			SDL_FreeSurface(hSurface);

		hSurface = pSurface;
	}
	hTexture->ToLoad();
}
/*
void SDLsurface::LoadFromMem(char *addr, long len)
{
	SDL_Surface *surface = IMG_Load_RW(SDL_RWFromMem(addr, len), true);

	if (!surface)
		SDLcore::RaiseError(SDL_GetError());

	if (hSurface)
		SDL_FreeSurface(hSurface);

	hSurface = surface;
	hTexture->ToLoad();

#ifdef DEBUGSURFACE
	if (hSurface->flags & SDL_SRCALPHA)
		std::cout << "SDLsurface::LoadFromMem : alpha layer detected" << std::endl;
#endif
}
*/
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
		0x0000FF00, 0x00FF0000, 0xFF000000, 0x000000FF);


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
	hTexture->ToLoad();
}

void SDLsurface::Fill(Uint32 color)
{
	if (!hSurface)
		return;

	SDL_FillRect(hSurface, NULL, color);
	hTexture->ToLoad();
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
		hSurface->format->BitsPerPixel, 0x0000FF00, 0x00FF0000, 0xFF000000, 0x000000FF);

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
	hTexture->ToLoad();
}
