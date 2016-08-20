/***************************************************************************

  c_channel.c

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

#define __C_CHANNEL_C

#include "c_channel.h"

#define THIS ((CCHANNEL *)_object)

DECLARE_EVENT(EVENT_Finish);

static CCHANNEL *_cache[MAX_CHANNEL] = { 0 };
static int _count = 0;

static int _pipe[2];
static int _pipe_usage = 0;

//-------------------------------------------------------------------------

static void free_channel(CCHANNEL *ch)
{
	if (!ch->sound)
		return;

	GB.Unref(POINTER(&ch->sound));
	ch->sound = NULL;
	ch->free = FALSE;

	_pipe_usage--;
	if (_pipe_usage == 0)
	{
		//fprintf(stderr, "stop watch\n");
		GB.Watch(_pipe[0], GB_WATCH_NONE, NULL, 0);
	}
}

static void free_finished_channel(void)
{
	//int i;
  CCHANNEL *ch;
	char channel;

	if (read(_pipe[0], &channel, 1) != 1)
		return;

	ch = _cache[(int)channel];
	if (ch)
	{
		if (ch->free)
			free_channel(ch);
		//fprintf(stderr, "raise finish %d\n", (int)channel);
		GB.Raise(ch, EVENT_Finish, 0);
	}

	/*for (i = 0; i < MAX_CHANNEL; i++)
	{
		ch = channel_cache[i];
		if (ch && ch->free)
			free_channel(ch);
	}*/
}

static void channel_finished_cb(int channel)
{
	CCHANNEL *ch = _cache[channel];
	char buf = (char)channel;

	if (!ch)
		return;

	ch->free = (write(_pipe[1], &buf, 1) == 1);
	//fprintf(stderr, "finish %d (%d)\n", channel, ch->free);
}

static int find_free_channel(void)
{
	int i;

	for (i = 0; i < MAX_CHANNEL; i++)
	{
		if (!_cache[i])
			return i;
	}

	return -1;
}

void CHANNEL_return(int channel, CSOUND *sound)
{
	CCHANNEL *ch = NULL;

	if (channel < 0 || channel >= _count)
	{
		if (sound)
			GB.Unref(POINTER(&sound));

		GB.ReturnNull();
		return;
	}

	CHECK_AUDIO();

	ch = _cache[channel];
	if (!ch)
	{
		ch = GB.New(CLASS_Channel, NULL, NULL);
		_cache[channel] = ch;
		ch->channel = channel;
		GB.Ref(ch);
	}

	//free_channel(ch);
	if (sound)
	{
		GB.Unref(POINTER(&ch->sound));
		ch->sound = sound;
	}

	GB.ReturnObject(ch);
}

bool CHANNEL_init(void)
{
	if (pipe(_pipe))
	{
		GB.Error("Unable to initialize channel pipe");
		return TRUE;
	}

	_count = Mix_AllocateChannels(-1);

	Mix_ChannelFinished(channel_finished_cb);
	return FALSE;
}

int CHANNEL_play_sound(int channel, CSOUND *sound, int loops, int fadein)
{
	_pipe_usage++;
	if (_pipe_usage == 1)
	{
		//fprintf(stderr, "watch pipe\n");
		GB.Watch(_pipe[0], GB_WATCH_READ, (void *)free_finished_channel, 0);
	}

	if (fadein > 0)
		return Mix_FadeInChannel(channel, sound->chunk, loops, fadein);
	else
		return Mix_PlayChannel(channel, sound->chunk, loops);
}

void CHANNEL_exit()
{
	int i;
	CCHANNEL *ch;

	Mix_HaltChannel(-1);

	for (i = 0; i < MAX_CHANNEL; i++)
	{
		ch = _cache[i];
		if (!ch)
			continue;

		free_channel(ch);
		GB.Unref(POINTER(&ch));
	}

	if (_pipe_usage)
	{
		GB.Watch(_pipe[0], GB_WATCH_NONE, NULL, 0);
		_pipe_usage = 0;
	}

	close(_pipe[0]);
	close(_pipe[1]);
}

static void update_channel_effect(CCHANNEL *_object)
{
	if (Mix_SetPosition(THIS->channel, THIS->angle, THIS->distance) == 0)
		GB.Error("Unable to set effect: &1", Mix_GetError());
}

//-------------------------------------------------------------------------

BEGIN_METHOD(Channels_get, GB_INTEGER index)

	CHANNEL_return(VARG(index), NULL);

END_METHOD

BEGIN_PROPERTY(Channels_Count)

	int nchan;

	CHECK_AUDIO();

	if (READ_PROPERTY)
		GB.ReturnInteger(Mix_AllocateChannels(-1));
	else
	{
		nchan = VPROP(GB_INTEGER);
		if (nchan < 0 || nchan > MAX_CHANNEL)
		{
			GB.Error(GB_ERR_ARG);
			return;
		}

		Mix_AllocateChannels(nchan);
		_count = Mix_AllocateChannels(-1);
	}

END_PROPERTY

BEGIN_PROPERTY(Channels_Volume)

	CHECK_AUDIO();

	if (READ_PROPERTY)
		GB.ReturnInteger(Mix_Volume(-1, -1));
	else
		Mix_Volume(-1, VPROP(GB_INTEGER));

END_PROPERTY

//-------------------------------------------------------------------------

BEGIN_METHOD_VOID(Channel_new)

	int channel = find_free_channel();

	if (channel < 0)
	{
		GB.Error("No more channel available");
		return;
	}

	THIS->channel = channel;
	_cache[channel] = THIS;
	GB.Ref(THIS);

END_METHOD

BEGIN_METHOD(Channel_Play, GB_OBJECT sound; GB_INTEGER loops; GB_FLOAT fadein)

	CSOUND *sound;

	if (Mix_Paused(THIS->channel))
		Mix_Resume(THIS->channel);

	sound = VARGOPT(sound, NULL);
	if (!sound)
		return;

	while (THIS->sound)
	{
		Mix_HaltChannel(THIS->channel);
		GB.Loop(10);
	}

	GB.Ref(sound);
	THIS->sound = sound;
	CHANNEL_play_sound(THIS->channel, sound, VARGOPT(loops, 0), (int)(VARGOPT(fadein, 0) * 1000));

END_METHOD


BEGIN_METHOD_VOID(Channel_Pause)

	Mix_Pause(THIS->channel);

END_METHOD


BEGIN_METHOD(Channel_Stop, GB_FLOAT fadeout)

	if (MISSING(fadeout))
		Mix_HaltChannel(THIS->channel);
	else
		Mix_FadeOutChannel(THIS->channel, (int)(VARG(fadeout) * 1000));

END_METHOD

BEGIN_PROPERTY(Channel_Volume)

	int channel = THIS->channel;

	if (READ_PROPERTY)
		GB.ReturnInteger(Mix_Volume(channel, -1));
	else
		Mix_Volume(channel, VPROP(GB_INTEGER));

END_PROPERTY

BEGIN_PROPERTY(Channel_Distance)

	if (READ_PROPERTY)
		GB.ReturnInteger(THIS->distance);
	else
	{
		int d = VPROP(GB_INTEGER);

		if (d < 0 || d > 255)
		{
			GB.Error(GB_ERR_ARG);
			return;
		}

		THIS->distance = d;
		update_channel_effect(THIS);
	}

END_PROPERTY

BEGIN_PROPERTY(Channel_Angle)

	if (READ_PROPERTY)
		GB.ReturnInteger(THIS->angle);
	else
	{
		THIS->angle = VPROP(GB_INTEGER);
		update_channel_effect(THIS);
	}

END_PROPERTY

BEGIN_PROPERTY(Channel_Reverse)

	if (READ_PROPERTY)
		GB.ReturnBoolean(THIS->reverse);
	else
	{
		bool v = VPROP(GB_BOOLEAN);
		if (Mix_SetReverseStereo(THIS->channel, v))
			THIS->reverse = v;
		else
			GB.Error(Mix_GetError());
	}

END_PROPERTY

BEGIN_PROPERTY(Channel_Index)

	GB.ReturnInteger(THIS->channel);

END_PROPERTY

//-------------------------------------------------------------------------

GB_DESC ChannelsDesc[] =
{
	GB_DECLARE_STATIC("Channels"),

	GB_STATIC_METHOD("_get", "Channel", Channels_get, "(Index)i"),
	//GB_STATIC_METHOD("_next", "Channel", CCHANNEL_next, NULL),

	GB_STATIC_PROPERTY("Count", "i", Channels_Count),
	GB_STATIC_PROPERTY("Volume", "i", Channels_Volume),

	GB_END_DECLARE
};

GB_DESC ChannelDesc[] =
{
	GB_DECLARE("Channel", sizeof(CCHANNEL)),

	GB_METHOD("_new", NULL, Channel_new, NULL),

	//GB_STATIC_METHOD("_exit", NULL, Channel_exit, NULL),
	GB_METHOD("Play", NULL, Channel_Play, "[(Sound)Sound;(Loops)i(FadeIn)f]"),
	GB_METHOD("Pause", NULL, Channel_Pause, NULL),
	GB_METHOD("Stop", NULL, Channel_Stop, "[(FadeOut)f]"),

	GB_PROPERTY_READ("Index", "i", Channel_Index),
	GB_PROPERTY("Volume", "i", Channel_Volume),
	GB_PROPERTY("Distance", "i", Channel_Distance),
	GB_PROPERTY("Angle", "i", Channel_Angle),
	GB_PROPERTY("Reverse", "b", Channel_Reverse),

	GB_EVENT("Finish", NULL, NULL, &EVENT_Finish),

	GB_END_DECLARE
};

