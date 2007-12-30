/***************************************************************************

  timer.cpp

  The SDL component

  (c) 2006 Laurent Carlier <lordheavy@infonie.fr>
           Benoit Minisini <gambas@users.sourceforge.net>

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

#define __TIMER_C

#include "gambas.h"
#include "main.h"

#include "SDL.h"

static Uint32 myTimer(Uint32 _interval, void *timer)
{
	GB.RaiseTimer((GB_TIMER *) timer);
	return _interval;
}

/**************************************************************************

  Timer

***************************************************************************/

void startTimer (GB_TIMER *timer)
{
	timer->id = (long ) SDL_AddTimer((timer->delay/10)*10, myTimer, timer);
}

void stopTimer (GB_TIMER *timer)
{
	if (!timer->id)
		return;
		
	SDL_RemoveTimer((SDL_TimerID ) timer->id);
	timer->id = 0;
}
