/***************************************************************************

  main.h

  (c) 2014 Benoît Minisini <gambas@users.sourceforge.net>

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

#ifndef __MAIN_H
#define __MAIN_H

#include "gambas.h"
#include "gb_common.h"
#include "gb_list.h"

#include "SDL.h"
#include "SDL_mixer.h"

#ifndef __MAIN_C

extern GB_INTERFACE GB;

extern GB_CLASS CLASS_Sound;
extern GB_CLASS CLASS_Channel;

extern bool AUDIO_initialized;
extern int AUDIO_frequency;
extern int AUDIO_buffer_size;

#endif

bool AUDIO_init(void);

#define CHECK_AUDIO() if (!AUDIO_initialized && AUDIO_init()) return

#endif /* __MAIN_H */

