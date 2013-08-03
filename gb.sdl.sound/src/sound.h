/***************************************************************************

  sound.h

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

#ifndef __SOUND_H
#define __SOUND_H

#include "gambas.h"

#include "SDL.h"
#include "SDL_mixer.h"

#ifndef __SOUND_C
extern GB_DESC CSoundDesc[];
extern GB_DESC CMusicDesc[];
extern GB_DESC CChannelDesc[];
extern GB_DESC CChannelsDesc[];
#else

typedef
  struct {
    Mix_Music *music;
    int rate;
    Uint16 format;
    int channels;
    int buffers;
    }
  SOUND_INFO;

#endif

#define MAX_CHANNEL 64

typedef
  struct {
    GB_BASE ob;
    Mix_Chunk *chunk;
    }
  CSOUND;

typedef
  struct {
    GB_BASE ob;
    int channel;
    CSOUND *sound;
		char free;
    }
  CCHANNEL;

void SOUND_init(void);
void SOUND_exit(void);

#endif /* __SOUND_H */
