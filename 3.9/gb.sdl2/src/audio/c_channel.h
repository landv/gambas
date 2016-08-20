/***************************************************************************

  c_channel.h

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

#ifndef __C_CHANNEL_H
#define __C_CHANNEL_H

#include "main.h"
#include "c_sound.h"

typedef
	struct {
		GB_BASE ob;
		int channel;
		CSOUND *sound;
		uchar distance;
		ushort angle;
		unsigned reverse : 1;
		unsigned free : 1;
		}
	CCHANNEL;

#ifndef __C_CHANNEL_C
extern GB_DESC ChannelDesc[];
extern GB_DESC ChannelsDesc[];
#endif

#define MAX_CHANNEL 64

bool CHANNEL_init(void);
void CHANNEL_exit(void);
int CHANNEL_play_sound(int channel, CSOUND *sound, int loops, int fadein);
void CHANNEL_return(int channel, CSOUND *sound);

#endif /* __C_CHANNEL_H */

