/***************************************************************************

  SDLtexture.cpp

  (c) 2008 Laurent Carlier <lordheavy@users.sourceforge.net>

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

/* manage textures and context switching if needed */

#include "SDLsurface.h"
#include "SDLtexture.h"
#include "SDLcore.h"

#include <iostream>
#include <cstdio>
#include <cstring>

SDLtexture::SDLtexture(SDLsurface *surface)
{
	hSurface = surface;
	hTexinfo = new texinfo;
	hTexinfo->Index = 0;
	hRenderBuffer = 0;
}

SDLtexture::~SDLtexture()
{
	if (hTexinfo->Index)
		glDeleteTextures(1, &(hTexinfo->Index));

	if (hRenderBuffer)
		delete hRenderBuffer;
	
	delete hTexinfo;
}

void SDLtexture::init()
{
	FBOrender::Check();
}

void SDLtexture::Select()
{
	if (!FBOrender::Check())
		SDLcore::RaiseError("Unable to draw on the texture, FBO not supported!");
	
	/* Make sure our texture is properly loaded/synced */
	this->GetAsTexture(NULL);

	if (!hRenderBuffer)
		hRenderBuffer = new FBOrender();
	
	hRenderBuffer->Bind(hTexinfo->Index);

/*	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glViewport(0, 0, hSurface->GetWidth(), hSurface->GetHeight());*/
}

void SDLtexture::Unselect()
{
	FBOrender::Unbind();
}

void SDLtexture::Sync()
{
}

void SDLtexture::GetAsTexture(texinfo *tex)
{
	if (!hTexinfo->Index)
	{
		glGenTextures(1, &(hTexinfo->Index));
		hTexinfo->State = TEXTURE_TO_UPLOAD;
	}

	if (hTexinfo->State & TEXTURE_TO_UPLOAD)
	{
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, hTexinfo->Index);

		SDL_Surface *image;

		if (!GLEW_ARB_texture_non_power_of_two)
		{
			SDL_Surface *origin = hSurface->GetSdlSurface();

			/* Use the surface width and height expanded to powers of 2 */
			int w = this->GetPowerOfTwo(origin->w);
			int h = this->GetPowerOfTwo(origin->h);
			hTexinfo->Width = GLdouble(origin->w) / w;  /* Max X */
			hTexinfo->Height = GLdouble(origin->h) / h;  /* Max Y */

			image = SDL_CreateRGBSurface(SDL_SWSURFACE, w, h, 32,
//			#if SDL_BYTEORDER == SDL_LIL_ENDIAN /* OpenGL RGBA masks */
//				0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000
//			#else
//				0xFF000000, 0x00FF0000,	0x0000FF00, 0x000000FF
//			#endif
			0x0000FF00, 0x00FF0000, 0xFF000000, 0x000000FF
			);

			if (!image)
			{
				std::cerr << __FILE__ << ":" << __LINE__ << ": Failed to create SDL_Surface() !" << std::endl;
				return;
			}

			/* Save the alpha blending attributes */
			Uint32 saved_flags = origin->flags & (SDL_SRCALPHA | SDL_RLEACCELOK);
			Uint8 saved_alpha = origin->format->alpha;

			if ((saved_flags & SDL_SRCALPHA) == SDL_SRCALPHA)
				SDL_SetAlpha(origin, 0, 0);

			/* Copy the surface into the GL texture image */
			SDL_BlitSurface(origin, NULL, image, NULL);

			/* Restore the alpha blending attributes */
			if ((saved_flags & SDL_SRCALPHA) == SDL_SRCALPHA )
				SDL_SetAlpha(origin, saved_flags, saved_alpha);
		}
		else
		{
			hTexinfo->Width = 1.0;
			hTexinfo->Height = 1.0;
			image = hSurface->GetSdlSurface();
		}

//		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image->w, image->h, 0, GL_RGBA, 
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image->w, image->h, 0, GL_BGRA, 
			GL_UNSIGNED_BYTE, image->pixels);
		/* Use of CLAMP_TO_EDGE, without it give a black line around the texture
		with DRI radeon drivers */
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
// 		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
// 		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		if (!GLEW_ARB_texture_non_power_of_two)
			SDL_FreeSurface(image); /* No longer needed */

		hTexinfo->State = TEXTURE_OK;
		glBindTexture(GL_TEXTURE_2D, 0);
		glDisable(GL_TEXTURE_2D);
	}

	if (tex)
		std::memcpy(tex, hTexinfo, sizeof(texinfo));
}	

int SDLtexture::GetPowerOfTwo(int size)
{
	int value = 1;

	while ( value < size )
		value <<= 1;

	return value;
}

