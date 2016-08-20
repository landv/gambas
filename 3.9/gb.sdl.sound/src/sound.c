/***************************************************************************

  sound.c

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

#define __SOUND_C

#include <math.h>

#include "gb_common.h"

#include "sound.h"
#include "main.h"

static SOUND_INFO info = { 0 };

static CCHANNEL *channel_cache[MAX_CHANNEL] = { 0 };
static int channel_count;

static double music_ref_time = 0;
static double music_ref_pos = 0;

static int _ch_pipe[2];
static int _ch_playing = 0;
static int _init = 0;

#if 0
static void musicDone()
{
/* This is the function that we told SDL_Mixer to call when the music
   was finished. In our case, we're going to simply unload the music
   as though the player wanted it stopped.  In other applications, a
   different music file might be loaded and played. */

  /* BM: You cannot raise an event from a static class at the moment. */

  printf("The music stopped playing NOW\n");	//This should raise a gambas event!
}
#endif

static void set_audio_properties()
{
  /* We're going to be requesting certain things from our audio
     device, so we set them up beforehand */

  //Of course this should all come from gambas properties!
  info.rate = 44100;		//could be: 22050;
  info.format = MIX_DEFAULT_FORMAT; 	//16-bit stereo
  info.channels = 2;		//The only one that opens on my machine! SB_PCI128
  // BM: This is stereo. It does not matter.
  info.buffers = 4096;

}

static void free_channel(CCHANNEL *ch)
{
	if (!ch->sound)
		return;
	
	GB.Unref(POINTER(&ch->sound));
	ch->sound = NULL;
	ch->free = FALSE;

	_ch_playing--;
	if (_ch_playing == 0)
		GB.Watch(_ch_pipe[0], GB_WATCH_NONE, (void *)0, 0);
}

static void free_finished_channels(void)
{
	int i;
  CCHANNEL *ch;
	char foo;
	
	if (read(_ch_pipe[0], &foo, 1) != 1)
		return;
	
	for (i = 0; i < MAX_CHANNEL; i++)
	{
		ch = channel_cache[i];
		if (ch && ch->free)
			free_channel(ch);
	}
}

static void channel_finished(int channel)
{
  CCHANNEL *ch = channel_cache[channel];
	char foo = 0;

  if (!ch)
    return;

  /*printf("channel_finished: %p\n", ch->sound);*/

	// TODO: do not use GB.Post(), because we are not in the main thread. Write to a pipe and watch it in the main thread
	ch->free = write(_ch_pipe[1], &foo, 1) == 1;
}

static int play_channel(int channel, CSOUND *sound, int loops, int fadein)
{
	_ch_playing++;
	if (_ch_playing == 1)
		GB.Watch(_ch_pipe[0], GB_WATCH_READ , (void *)free_finished_channels,0);

	if (fadein > 0)
		return Mix_FadeInChannel(channel, sound->chunk, loops, fadein);
	else
		return Mix_PlayChannel(channel, sound->chunk, loops);
}

static bool start_sound_engine()
{
  /* This is where we open up our audio device.  Mix_OpenAudio takes
     as its parameters the audio format we'd /like/ to have. */
  if(Mix_OpenAudio(info.rate, info.format, info.channels, info.buffers))
  {
    GB.Error("Unable to open audio");
    return TRUE;
  }

	if (pipe(_ch_pipe))
	{
		GB.Error("Unable to initialize channel pipe");
		return TRUE;
	}
	
  Mix_QuerySpec(&info.rate, &info.format, &info.channels);
	//fprintf(stderr, "Mix_QuerySpec: %d %d %d\n", info.rate, info.format, info.channels);

  channel_count = Mix_AllocateChannels(-1);

  Mix_ChannelFinished(channel_finished);

	return FALSE;
}


static void free_music(void)
{
  if (!info.music)
    return;

  Mix_HaltMusic();
  Mix_RewindMusic();
  Mix_FreeMusic(info.music);
  info.music = NULL;
}


static void stop_sound_engine()
{
	if (_ch_playing)
		GB.Watch(_ch_pipe[0], GB_WATCH_NONE, (void *)0, 0);
	close(_ch_pipe[0]);
	close(_ch_pipe[1]);
  free_music();
  Mix_CloseAudio();
}

void SOUND_init(void)
{
  _init++;
  if (_init > 1)
    return;

  set_audio_properties();	//Fill audio structures with gambas properties
  start_sound_engine();		//Start the sound engine
}


void SOUND_exit(void)
{
  _init--;
  if (_init > 0)
    return;

	stop_sound_engine();
}


static void return_channel(int channel, CSOUND *sound)
{
  CCHANNEL *ch = NULL;

  if (channel < 0 || channel >= channel_count)
  {
		if (sound)
			GB.Unref(POINTER(&sound));

		GB.ReturnNull();
    return;
  }

  ch = channel_cache[channel];
  if (!ch)
  {
    ch = GB.New(GB.FindClass("Channel"), NULL, NULL);
    channel_cache[channel] = ch;
    ch->channel = channel;
    GB.Ref(ch);
  }

	free_channel(ch);
  if (sound)
    ch->sound = sound;

  GB.ReturnObject(ch);
}


static double volume_from_sdl(int vol)
{
  return log(1 + (M_E - 1) * (double)vol / MIX_MAX_VOLUME);
}

static int volume_to_sdl(double vol)
{
  return (exp(vol) - 1) / (M_E - 1) * MIX_MAX_VOLUME;
}

/***************************************************************************

  Sound

***************************************************************************/

#define THIS ((CSOUND *)_object)


BEGIN_METHOD(CSOUND_new, GB_STRING file)

  char *addr;
  int len;

  if (GB.LoadFile(STRING(file), LENGTH(file), &addr, &len))
    return;

  THIS->chunk = Mix_LoadWAV_RW(SDL_RWFromMem(addr, len), TRUE);

  GB.ReleaseFile(addr, len);

  if (!THIS->chunk)
    GB.Error(Mix_GetError());

END_METHOD


BEGIN_METHOD_VOID(CSOUND_free)

  Mix_FreeChunk(THIS->chunk);
  THIS->chunk = NULL;

END_METHOD


BEGIN_METHOD(CSOUND_play, GB_INTEGER loops; GB_FLOAT fadein)

  int loops = VARGOPT(loops, 0);
  int channel;

  GB.Ref(THIS);
  channel = play_channel(-1, THIS, loops, MISSING(fadein) ? 0 : (int)(VARG(fadein) * 1000));
  return_channel(channel, THIS);

END_METHOD


GB_DESC CSoundDesc[] =
{
  GB_DECLARE("Sound", sizeof(CSOUND)),

  //GB_STATIC_METHOD("_init", NULL, CSOUND_init, NULL),
  //GB_STATIC_METHOD("_exit", NULL, CSOUND_exit, NULL),

  GB_METHOD("_new", NULL, CSOUND_new, "(File)s"),
  GB_METHOD("_free", NULL, CSOUND_free, NULL),

  //GB_PROPERTY("Volume", "e", CSOUND_volume),

  GB_METHOD("Play", "Channel", CSOUND_play, "[(Loops)i(FadeIn)f]"),

  GB_END_DECLARE
};

/***************************************************************************

  Channel

***************************************************************************/

#undef THIS
#define THIS ((CCHANNEL *)_object)


BEGIN_METHOD(CCHANNEL_get, GB_INTEGER index)

  return_channel(VARG(index), NULL);

END_METHOD


BEGIN_PROPERTY(CCHANNEL_count)

  int nchan;

  if (READ_PROPERTY)
    GB.ReturnInteger(Mix_AllocateChannels(-1));
  else
  {
    nchan = VPROP(GB_INTEGER);
    if (nchan < 0)
      nchan = 0;
    else if (nchan >= MAX_CHANNEL)
      nchan = MAX_CHANNEL;

    Mix_AllocateChannels(nchan);
		channel_count = Mix_AllocateChannels(-1);
  }

END_PROPERTY


BEGIN_METHOD(CCHANNEL_play, GB_OBJECT sound; GB_INTEGER loops; GB_FLOAT fadein)

  CSOUND *sound;

  if (Mix_Paused(THIS->channel))
    Mix_Resume(THIS->channel);

  sound = VARGOPT(sound, NULL);
  if (!sound)
    return;

  /*printf("Ref %p\n", sound);*/
  GB.Ref(sound);
  THIS->sound = sound;
  play_channel(THIS->channel, sound, VARGOPT(loops, 0), MISSING(fadein) ? 0 : (int)(VARG(fadein) * 1000));

END_METHOD


BEGIN_METHOD_VOID(CCHANNEL_pause)

  Mix_Pause(THIS->channel);

END_METHOD


BEGIN_METHOD(CCHANNEL_stop, GB_FLOAT fadeout)

	if (MISSING(fadeout))
		Mix_HaltChannel(THIS->channel);
	else
		Mix_FadeOutChannel(THIS->channel, (int)(VARG(fadeout) * 1000));

END_METHOD


BEGIN_METHOD_VOID(CCHANNEL_exit)

  int i;
  CCHANNEL *ch;

  for (i = 0; i < MAX_CHANNEL; i++)
  {
    ch = channel_cache[i];
    if (!ch)
      continue;

		free_channel(ch);
    GB.Unref(POINTER(&ch));
  }

END_METHOD


BEGIN_PROPERTY(CCHANNEL_volume)

  int channel;

  channel = THIS ? THIS->channel : -1;

  if (READ_PROPERTY)
    GB.ReturnFloat(volume_from_sdl(Mix_Volume(channel, -1)));
  else
    Mix_Volume(channel, volume_to_sdl(VPROP(GB_FLOAT)));

END_PROPERTY



GB_DESC CChannelDesc[] =
{
  GB_DECLARE("Channel", sizeof(CCHANNEL)), GB_NOT_CREATABLE(),

  GB_STATIC_METHOD("_exit", NULL, CCHANNEL_exit, NULL),
  //GB_STATIC_PROPERTY("Volume", "e", CCHANNEL_volume),
  GB_METHOD("Play", NULL, CCHANNEL_play, "[(Sound)Sound;(Loops)i(FadeIn)f]"),
  GB_METHOD("Pause", NULL, CCHANNEL_pause, NULL),
  GB_METHOD("Stop", NULL, CCHANNEL_stop, "[(FadeOut)f]"),

  GB_PROPERTY("Volume", "f", CCHANNEL_volume),

  GB_END_DECLARE
};

GB_DESC CChannelsDesc[] =
{
  GB_DECLARE("Channels", 0), GB_NOT_CREATABLE(),

  GB_STATIC_METHOD("_get", "Channel", CCHANNEL_get, "(Index)i"),
  //GB_STATIC_METHOD("_next", "Channel", CCHANNEL_next, NULL),

  GB_STATIC_PROPERTY("Count", "i", CCHANNEL_count),
  GB_STATIC_PROPERTY("Volume", "f", CCHANNEL_volume),

  GB_END_DECLARE
};

/***************************************************************************

  Music

***************************************************************************/

static double get_music_pos(void)
{
  double time;

  if (Mix_PlayingMusic())
	{
		if (!Mix_PausedMusic())
		{
			GB.GetTime(&time, FALSE);
			return music_ref_pos + time - music_ref_time;
		}
		else
			return music_ref_pos;
	}
	else
		return 0;
}


BEGIN_METHOD(CMUSIC_load, GB_STRING file)

  free_music();

  /* Note that the music cannot be stored inside the project ! */

  info.music = Mix_LoadMUS(GB.RealFileName(STRING(file), LENGTH(file)));
  if (!info.music)
    GB.Error(Mix_GetError());

  music_ref_pos = 0;
  music_ref_time = 0;

END_METHOD


BEGIN_METHOD(CMUSIC_play, GB_INTEGER loops; GB_FLOAT fadein)

  double fadevalue=0;

  if (!info.music)
    return;

  GB.GetTime(&music_ref_time, FALSE);

  if (Mix_PausedMusic())
  {
    Mix_ResumeMusic();
    return;
  }

  /* We want to know when our music has stopped playing so we
  can free it up and set 'music' back to NULL.  SDL_Mixer
  provides us with a callback routine we can use to do
  exactly that */

  /*Mix_HookMusicFinished(musicDone);*/

  //The 'Looping' param should be optional in gambas, default=0. Don't know how?
  //BM Now do you know ? ;-)

  fadevalue = VARGOPT(fadein, 0) * 1000;
  // if fadevalue is too small -> music doesn't want to play !
  if (fadevalue<100)
  {
    fadevalue=0;
  }

  Mix_FadeInMusic(info.music, VARGOPT(loops, 1), fadevalue);

END_METHOD


BEGIN_METHOD_VOID(CMUSIC_pause)

  music_ref_pos = get_music_pos();
  Mix_PauseMusic();

END_METHOD


BEGIN_METHOD(CMUSIC_stop, GB_FLOAT fadeout)

  if (MISSING(fadeout))
    Mix_HaltMusic();
  else
    Mix_FadeOutMusic(VARG(fadeout)*1000);

  music_ref_pos = 0;

END_METHOD

BEGIN_PROPERTY(CMUSIC_pos)

  double pos;

  if (READ_PROPERTY)
  {
    GB.ReturnFloat(get_music_pos());
  }
  else
  {
    pos = VPROP(GB_FLOAT);
    Mix_RewindMusic();
    if (Mix_SetMusicPosition(pos) == 0)
      music_ref_pos = pos;
    else
      music_ref_pos = 0;
    GB.GetTime(&music_ref_time, FALSE);
  }

END_PROPERTY


BEGIN_PROPERTY(CMUSIC_volume)

  if (READ_PROPERTY)
    GB.ReturnFloat(volume_from_sdl(Mix_VolumeMusic(-1)));
  else
    Mix_VolumeMusic(volume_to_sdl(VPROP(GB_FLOAT)));

END_PROPERTY


BEGIN_PROPERTY(Music_State)

	if (Mix_PlayingMusic())
	{
		if (Mix_PausedMusic())
			GB.ReturnInteger(2);
		else
			GB.ReturnInteger(1);
	}
	else
		GB.ReturnInteger(0);

END_PROPERTY

GB_DESC CMusicDesc[] =
{
  GB_DECLARE("Music", 0),

  //GB_STATIC_METHOD("_init", NULL, CSOUND_init, NULL),
  //GB_STATIC_METHOD("_exit", NULL, CSOUND_exit, NULL),

  GB_STATIC_METHOD("Load", NULL, CMUSIC_load, "(File)s"),
  GB_STATIC_METHOD("Play", NULL, CMUSIC_play, "[(Loops)i(FadeIn)f]"),
  GB_STATIC_METHOD("Pause", NULL, CMUSIC_pause, NULL),
  GB_STATIC_METHOD("Stop", NULL, CMUSIC_stop, "[(FadeOut)f]"),

  GB_STATIC_PROPERTY("Volume", "f", CMUSIC_volume),
  GB_STATIC_PROPERTY("Pos", "f", CMUSIC_pos),

  GB_STATIC_PROPERTY_READ("State", "i", Music_State),

  GB_CONSTANT("Stopped", "i", 0),
  GB_CONSTANT("Playing", "i", 1),
  GB_CONSTANT("Paused", "i", 2),

  GB_END_DECLARE
};
