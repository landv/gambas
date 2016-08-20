/***************************************************************************

  c_music.c

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

#define __C_MUSIC_C

#include "c_music.h"

static Mix_Music *_music = NULL;
static double _ref_time = 0;
static double _ref_pos = 0;
static int _volume = MIX_MAX_VOLUME;

//-------------------------------------------------------------------------

static double get_music_pos(void)
{
	double time;

	if (Mix_PlayingMusic())
	{
		if (!Mix_PausedMusic())
		{
			GB.GetTime(&time, FALSE);
			return _ref_pos + time - _ref_time;
		}
		else
			return _ref_pos;
	}
	else
		return 0;
}

void MUSIC_exit(void)
{
  if (!_music)
    return;

  Mix_HaltMusic();
  Mix_RewindMusic();
  Mix_FreeMusic(_music);
  _music = NULL;
}

static void update_volume(void)
{
	if (!Mix_PlayingMusic())
		return;

	Mix_VolumeMusic(_volume);
}

//-------------------------------------------------------------------------

BEGIN_METHOD(Music_Load, GB_STRING path)

	CHECK_AUDIO();

	MUSIC_exit();

	// Note that the music cannot be stored inside the project.

	_music = Mix_LoadMUS(GB.RealFileName(STRING(path), LENGTH(path)));
	if (!_music)
	{
		GB.Error(Mix_GetError());
		return;
	}

	_ref_pos = 0;
	_ref_time = 0;

END_METHOD

BEGIN_METHOD(Music_Play, GB_INTEGER loops; GB_FLOAT fadein)

	double fadevalue=0;

	CHECK_AUDIO();

	if (!_music)
		return;

	GB.GetTime(&_ref_time, FALSE);

	if (Mix_PausedMusic())
	{
		Mix_ResumeMusic();
		return;
	}

	fadevalue = VARGOPT(fadein, 0) * 1000;

	// if fadevalue is too small the music doesn't want to play.
	if (fadevalue < 100)
		fadevalue = 0;

	Mix_FadeInMusic(_music, VARGOPT(loops, 1), fadevalue);
	update_volume();

END_METHOD

BEGIN_METHOD_VOID(Music_Pause)

	CHECK_AUDIO();

	_ref_pos = get_music_pos();
	Mix_PauseMusic();

END_METHOD

BEGIN_METHOD(Music_Stop, GB_FLOAT fadeout)

	CHECK_AUDIO();

	if (MISSING(fadeout))
		Mix_HaltMusic();
	else
		Mix_FadeOutMusic(VARG(fadeout) * 1000);

	_ref_pos = 0;

END_METHOD

BEGIN_PROPERTY(Music_Pos)

	CHECK_AUDIO();

	if (READ_PROPERTY)
		GB.ReturnFloat(get_music_pos());
	else
	{
		double pos;

		if (!_music)
			return;

		if (Mix_GetMusicType(_music) == MUS_MOD)
		{
			GB.Error("Seeking is not supported on MOD files");
			return;
		}

		pos = VPROP(GB_FLOAT);
		Mix_RewindMusic();
		if (Mix_SetMusicPosition(pos) == 0)
			_ref_pos = pos;
		else
			_ref_pos = 0;
		GB.GetTime(&_ref_time, FALSE);
	}

END_PROPERTY

BEGIN_PROPERTY(Music_Volume)

	CHECK_AUDIO();

	if (READ_PROPERTY)
		GB.ReturnInteger(_volume);
	else
	{
		_volume = VPROP(GB_INTEGER);
		if (_volume < 0)
			_volume = 0;
		else if (_volume > MIX_MAX_VOLUME)
			_volume = MIX_MAX_VOLUME;

		update_volume();
	}

END_PROPERTY

BEGIN_PROPERTY(Music_State)

	CHECK_AUDIO();

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

BEGIN_PROPERTY(Music_SoundFontPath)

	if (READ_PROPERTY)
		GB.ReturnNewZeroString(Mix_GetSoundFonts());
	else
		Mix_SetSoundFonts(GB.ToZeroString(PROP(GB_STRING)));

END_PROPERTY

//-------------------------------------------------------------------------

GB_DESC MusicDesc[] =
{
	GB_DECLARE_STATIC("Music"),

	GB_STATIC_PROPERTY("SoundFontPath", "s", Music_SoundFontPath),

	GB_STATIC_METHOD("Load", NULL, Music_Load, "(File)s"),
	GB_STATIC_METHOD("Play", NULL, Music_Play, "[(Loops)i(FadeIn)f]"),
	GB_STATIC_METHOD("Pause", NULL, Music_Pause, NULL),
	GB_STATIC_METHOD("Stop", NULL, Music_Stop, "[(FadeOut)f]"),

	GB_STATIC_PROPERTY("Volume", "i", Music_Volume),
	GB_STATIC_PROPERTY("Pos", "f", Music_Pos),

	GB_STATIC_PROPERTY_READ("State", "i", Music_State),

	GB_CONSTANT("Stopped", "i", 0),
	GB_CONSTANT("Playing", "i", 1),
	GB_CONSTANT("Paused", "i", 2),

	GB_END_DECLARE
};
