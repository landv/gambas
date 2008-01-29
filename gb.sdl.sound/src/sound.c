/***************************************************************************

  sound.c

  Sound routines

  (c) 2000-2007 Benoit Minisini <gambas@users.sourceforge.net>

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

#define __SOUND_C

#include <math.h>

#include "gb_common.h"

#include "sound.h"
#include "main.h"

static SOUND_INFO info = { 0 };

static CCHANNEL *channel_cache[MAX_CHANNEL] = { 0 };
static int channel_count;
/*static int count_sound = 0;*/

static double music_ref_time = 0;
static double music_ref_pos = 0;

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

static void free_channel_sound(CSOUND *sound)
{
/*  count_sound--;
  printf("Unref channel [%d] sound : %p\n", count_sound,sound);*/
  fflush(NULL);
  GB.Unref(POINTER(&sound));
}

static void channel_finished(int channel)
{
  CCHANNEL *ch = channel_cache[channel];

  if (!ch)
    return;

  /*printf("channel_finished: %p\n", ch->sound);*/
  fflush(NULL);

//  free_channel_sound(ch->sound);
  GB.Post(free_channel_sound, (intptr_t)ch->sound);
  ch->sound = NULL;
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

  Mix_QuerySpec(&info.rate, &info.format, &info.channels);

  channel_count = Mix_AllocateChannels(-1);

  Mix_ChannelFinished(channel_finished);

  return FALSE;
}

static int init = 0;

void SOUND_init(void)
{
  init++;
  if (init > 1)
    return;

  set_audio_properties();	//Fill audio structures with gambas properties
  start_sound_engine();		//Start the sound engine
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

void SOUND_exit(void)
{
  init--;
  if (init > 0)
    return;

  free_music();

  Mix_CloseAudio();
}


static void return_channel(int channel, CSOUND *sound)
{
  CCHANNEL *ob=0;

  if (channel < 0 || channel >= channel_count)
  {
		if (sound)
			GB.Unref(POINTER(&sound));

		GB.ReturnNull();
    return;
  }

  ob = channel_cache[channel];
  if (!ob)
  {
    GB.New(POINTER(&ob), GB.FindClass("Channel"), NULL, NULL);
    channel_cache[channel] = ob;
    ob->channel = channel;
    GB.Ref(ob);
  }

  if (sound)
    ob->sound = sound;

  GB.ReturnObject(ob);
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

  GB.ReleaseFile(&addr, len);

  if (!THIS->chunk)
    GB.Error(Mix_GetError());

END_METHOD


BEGIN_METHOD_VOID(CSOUND_free)

  Mix_FreeChunk(THIS->chunk);
  THIS->chunk = NULL;

END_METHOD


BEGIN_METHOD(CSOUND_play, GB_INTEGER loops)

  int loops = VARGOPT(loops, 0);
  int channel;

/*  count_sound++;
  printf("Ref sound [%d] play : %p\n", count_sound,THIS);*/
  fflush(NULL);
  GB.Ref(THIS);
  channel = Mix_PlayChannel(-1, THIS->chunk, loops);
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

  GB_METHOD("Play", "Channel", CSOUND_play, "[(Loops)i]"),

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
  }

END_PROPERTY


BEGIN_METHOD(CCHANNEL_play, GB_OBJECT sound; GB_INTEGER loops)

  CSOUND *sound;

  if (Mix_Paused(THIS->channel))
    Mix_Resume(THIS->channel);

  sound = VARGOPT(sound, NULL);
  if (!sound)
    return;

  /*printf("Ref %p\n", sound);*/
  fflush(NULL);
  GB.Ref(sound);
  Mix_PlayChannel(THIS->channel, sound->chunk, VARGOPT(loops, 0));
  THIS->sound = sound;

END_METHOD


BEGIN_METHOD_VOID(CCHANNEL_pause)

  Mix_Pause(THIS->channel);

END_METHOD


BEGIN_METHOD_VOID(CCHANNEL_stop)

  Mix_HaltChannel(THIS->channel);

END_METHOD


BEGIN_METHOD_VOID(CCHANNEL_exit)

  int i;
  CCHANNEL *ch;

  for (i = 0; i < MAX_CHANNEL; i++)
  {
    ch = channel_cache[i];
    if (!ch)
      continue;

    /*Mix_HaltChannel(ch->channel);*/

    if (ch->sound)
      free_channel_sound(ch->sound);

    /*if (ch->sound)
      GB.Unref((void **)&ch->sound);*/

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
  GB_METHOD("Play", NULL, CCHANNEL_play, "[(Sound)Sound;(Loops)i]"),
  GB_METHOD("Pause", NULL, CCHANNEL_pause, NULL),
  GB_METHOD("Stop", NULL, CCHANNEL_stop, NULL),

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

  if (Mix_PlayingMusic() && !Mix_PausedMusic())
  {
    GB.GetTime(&time, FALSE);
    return music_ref_pos + time - music_ref_time;
  }
  else
    return music_ref_pos;
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
  //BM Now do you now ? ;-)

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

  GB_END_DECLARE
};
