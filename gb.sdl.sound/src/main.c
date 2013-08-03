/***************************************************************************

  main.c

  (c) 2000-2013 Benoît Minisini <gambas@users.sourceforge.net>

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

#define __MAIN_C

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "sound.h"
#include "cdrom.h"
#include "main.h"

#include "SDL.h"

GB_INTERFACE GB EXPORT;

GB_DESC *GB_CLASSES[] EXPORT =
{
	CSoundDesc,
	CMusicDesc,
	CChannelsDesc,
	CChannelDesc,
	//CMusic,
	Cquerycdrom,
	Ctracks, Ctrack,
	Ccdrom,

	NULL
};


int EXPORT GB_INIT(void)
{
	Uint32 sysInit = SDL_WasInit(SDL_INIT_EVERYTHING);

	// if video is defined, sdl was init by gb.sdl component !
	if (sysInit & SDL_INIT_VIDEO)
	{
		if (SDL_InitSubSystem(SDL_INIT_AUDIO | SDL_INIT_CDROM)<0)
		{
			GB.Error(SDL_GetError());
			return 0;
		}
	}
	else
	{
		if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_CDROM | SDL_INIT_TIMER | SDL_INIT_NOPARACHUTE)<0)
		{
			GB.Error(SDL_GetError());
			return 0;
		}
	}

	SOUND_init();

	return -1;
}


void EXPORT GB_EXIT()
{
	Uint32 sysInit = SDL_WasInit(SDL_INIT_EVERYTHING);
	SOUND_exit();

	// if video is defined, gb.sdl component still not closed !
	if (sysInit & SDL_INIT_VIDEO)
		SDL_QuitSubSystem(SDL_INIT_AUDIO | SDL_INIT_CDROM);
	else
		SDL_Quit();	

}




