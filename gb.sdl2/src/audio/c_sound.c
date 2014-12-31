/***************************************************************************

  c_sound.c

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

#define __C_SOUND_C

#include "c_channel.h"
#include "c_sound.h"

#define THIS ((CSOUND *)_object)

//-------------------------------------------------------------------------

BEGIN_PROPERTY(Sound_Frequency)

	if (READ_PROPERTY)
		GB.ReturnInteger(AUDIO_frequency);
	else
	{
		if (AUDIO_initialized)
			GB.Error("Read-only property. Audio has been initialized");
		else
			AUDIO_frequency = VPROP(GB_INTEGER);
	}

END_PROPERTY

BEGIN_PROPERTY(Sound_BufferSize)

	if (READ_PROPERTY)
		GB.ReturnInteger(AUDIO_buffer_size);
	else
	{
		if (AUDIO_initialized)
			GB.Error("Read-only property. Audio has been initialized");
		else
			AUDIO_buffer_size = VPROP(GB_INTEGER);
	}

END_PROPERTY

BEGIN_METHOD(Sound_Load, GB_STRING path)

	char *addr;
	int len;
	Mix_Chunk *chunk;
	CSOUND *sound;

	CHECK_AUDIO();

	if (GB.LoadFile(STRING(path), LENGTH(path), &addr, &len))
		return;

	chunk = Mix_LoadWAV_RW(SDL_RWFromMem(addr, len), TRUE);
	GB.ReleaseFile(addr, len);

	if (!chunk)
	{
		GB.Error(Mix_GetError());
		return;
	}

	sound = (CSOUND *)GB.New(CLASS_Sound, NULL, NULL);
	sound->chunk = chunk;
	GB.ReturnObject(sound);

END_METHOD

BEGIN_METHOD_VOID(Sound_free)

	Mix_FreeChunk(THIS->chunk);
	THIS->chunk = NULL;

END_METHOD

BEGIN_PROPERTY(Sound_Volume)

	if (READ_PROPERTY)
		GB.ReturnInteger(Mix_VolumeChunk(THIS->chunk, -1));
	else
	{
		int vol = VPROP(GB_INTEGER);
		if (vol < 0 || vol > MIX_MAX_VOLUME)
			GB.Error(GB_ERR_ARG);
		else
			Mix_VolumeChunk(THIS->chunk, vol);
	}

END_PROPERTY

BEGIN_METHOD(Sound_Play, GB_INTEGER loops; GB_FLOAT fadein)

	int loops = VARGOPT(loops, 0);
	int channel;

	GB.Ref(THIS);
	channel = CHANNEL_play_sound(-1, THIS, loops, MISSING(fadein) ? 0 : (int)(VARG(fadein) * 1000));
	CHANNEL_return(channel, THIS);

END_METHOD

//-------------------------------------------------------------------------

GB_DESC SoundDesc[] =
{
	GB_DECLARE("Sound", sizeof(CSOUND)), GB_NOT_CREATABLE(),

	GB_CONSTANT("MaxVolume", "i", MIX_MAX_VOLUME),

	GB_STATIC_PROPERTY("Frequency", "i", Sound_Frequency),
	GB_STATIC_PROPERTY("BufferSize", "i", Sound_BufferSize),

	GB_STATIC_METHOD("Load", "Sound", Sound_Load, "(Path)s"),

	GB_METHOD("_free", NULL, Sound_free, NULL),

	GB_PROPERTY("Volume", "i", Sound_Volume),

	GB_METHOD("Play", "Channel", Sound_Play, "[(Loops)i(FadeIn)f]"),

	GB_END_DECLARE
};
