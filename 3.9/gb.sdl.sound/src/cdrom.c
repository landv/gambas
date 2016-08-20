/***************************************************************************

  cdrom.c

  (c) 2004,2005 Laurent Carlier <lordheavy@users.sourceforge.net>

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

#define __CDROM_C

#include "config.h"

#include "main.h"
#include "cdrom.h"

/* CDROM volume control */
#ifdef OS_LINUX
#include <sys/ioctl.h>
#include <fcntl.h>
#include <linux/cdrom.h>
#endif

#ifdef OS_FREEBSD /* Is this the good headers ? */
#include <sys/cdio.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#endif

#include <string.h>
#include "SDL.h"


/***************************************************************************

  CDROMS - detect available CDROMs

***************************************************************************/

BEGIN_METHOD(CDROMS_get, GB_INTEGER index)

  int numdrives = SDL_CDNumDrives();

  if (numdrives==0)
  {
    GB.Error("no CDROM found !");
    return;
  }

  if (VARG(index)>numdrives)
  {
    GB.Error("CDROM &1 not available !", VARG(index));
    return;
  }

  GB.ReturnConstZeroString(SDL_CDName(VARG(index)-1));

END_METHOD

BEGIN_PROPERTY(CDROMS_count)

  GB.ReturnInteger(SDL_CDNumDrives());

END_METHOD

/***************************************************************************

  Track - managing a Track

***************************************************************************/

BEGIN_METHOD(TRACK_play, GB_INTEGER start; GB_INTEGER length)

  int track  = THIS->index;
  int start  = VARGOPT(start,0);
  int length = VARGOPT(length,0);
  
  if (CDROM->track[track-1].type!=SDL_AUDIO_TRACK)
    return;
  
  if ((track>CDROM->numtracks) || !track)
    return;

  if (start>((CDROM->track[track-1].length)/CD_FPS))
    return;

  if ((start+length)>((CDROM->track[track-1].length)/CD_FPS))
    length = 0;
    
  if (length==0)
  {
    if (SDL_CDPlayTracks(CDROM, track-1, start*CD_FPS, 1, 0)==-1)
      GB.Error(SDL_GetError());
  }
  else
  {
    if (SDL_CDPlayTracks(CDROM, track-1, start*CD_FPS, 0, length*CD_FPS)==-1)
      GB.Error(SDL_GetError());
  }
  
END_METHOD

BEGIN_PROPERTY(TRACK_length)

  if ((THIS->index>CDROM->numtracks) || (CDROM->track[(THIS->index)-1].type!=SDL_AUDIO_TRACK))
    GB.ReturnInteger(0);
  else
    GB.ReturnInteger((CDROM->track[(THIS->index)-1].length)/CD_FPS);

END_PROPERTY

BEGIN_PROPERTY(TRACK_position)

  if (CDROM->cur_track!=((THIS->index)-1))
    GB.ReturnInteger(0);
  else
    GB.ReturnInteger((CDROM->cur_frame)/CD_FPS);

END_PROPERTY

BEGIN_PROPERTY(TRACK_playable)

  if (THIS->index>CDROM->numtracks)
    GB.ReturnBoolean(0);

  if (CDROM->track[(THIS->index)-1].type==SDL_AUDIO_TRACK)
    GB.ReturnBoolean(1);
  else
    GB.ReturnBoolean(0);

END_PROPERTY

/***************************************************************************

  Tracks - managing the Tracks

***************************************************************************/

BEGIN_METHOD(TRACKS_get, GB_INTEGER index)

  CDstatus status = SDL_CDStatus(CDROM);
  long index = VARG(index);

  if (!CD_INDRIVE(status))
  {
    GB.Error("CDROM not available !");
    return;
  }

  THIS->index = index;
  RETURN_SELF();

END_METHOD

BEGIN_PROPERTY(TRACKS_count)

  SDL_CDStatus(CDROM);
  GB.ReturnInteger(CDROM->numtracks);

END_PROPERTY

BEGIN_PROPERTY(TRACKS_current)

  CDstatus status = SDL_CDStatus(CDROM);

  if (!CD_INDRIVE(status))
    GB.ReturnInteger(0);
  else
    GB.ReturnInteger(CDROM->cur_track+1);

END_PROPERTY

/***************************************************************************

  CDrom - use a detected CDROM

***************************************************************************/

BEGIN_METHOD(CDROM_new, GB_INTEGER index)

  int numdrives = SDL_CDNumDrives();
  int index = VARGOPT(index, 0);

  if (numdrives==0)
  {
    GB.Error("no CDROM found !");
    return;
  }

  if (!MISSING(index))
  {
    CDROM = SDL_CDOpen(index);
    THIS->id = index;

    if (!CDROM)
      GB.Error(SDL_GetError());

    return;
  }
  else
  {
    CDROM = SDL_CDOpen(0);
    THIS->id = 0;

    if (!CDROM)
      GB.Error(SDL_GetError());

    return;
  }

END_METHOD

BEGIN_METHOD_VOID(CDROM_free)

  SDL_CDStop(CDROM);
  SDL_CDClose(CDROM);

END_METHOD

BEGIN_METHOD_VOID(CDROM_eject)

  if (SDL_CDEject(CDROM)==-1)
    GB.Error(SDL_GetError());

END_METHOD

BEGIN_METHOD_VOID(CDROM_stop)

  if (SDL_CDStop(CDROM)==-1)
    GB.Error(SDL_GetError());

END_METHOD

BEGIN_METHOD_VOID(CDROM_pause)

  if (SDL_CDPause(CDROM)==-1)
    GB.Error(SDL_GetError());

END_METHOD

BEGIN_METHOD_VOID(CDROM_resume)

  if (SDL_CDResume(CDROM)==-1)
    GB.Error(SDL_GetError());

END_METHOD

BEGIN_METHOD(CDROM_play, GB_INTEGER start; GB_INTEGER tracks)

  CDstatus status = SDL_CDStatus(CDROM);
  int start = VARGOPT(start,1);
  int tracks = VARGOPT(tracks,0);

  if (status == CD_ERROR)
    GB.Error(SDL_GetError());

  if (!CD_INDRIVE(status))
    return;

  if (start>CDROM->numtracks)
    return;

  if ((start+(tracks-1))>CDROM->numtracks)
    tracks = (CDROM->numtracks)-start;

  if (SDL_CDPlayTracks(CDROM, start-1, 0, tracks, 0)==-1)
    GB.Error(SDL_GetError());

END_METHOD

BEGIN_PROPERTY(CDROM_ready)

  CDstatus status = SDL_CDStatus(CDROM);

  GB.ReturnBoolean(CD_INDRIVE(status));

END_PROPERTY

BEGIN_PROPERTY(CDROM_paused)

  CDstatus status = SDL_CDStatus(CDROM);

  if (status == CD_ERROR)
    GB.Error(SDL_GetError());

  GB.ReturnBoolean(status == CD_PAUSED);

END_PROPERTY

BEGIN_PROPERTY(CDROM_playing)

  CDstatus status = SDL_CDStatus(CDROM);

  GB.ReturnBoolean(status == CD_PLAYING);

END_PROPERTY

BEGIN_PROPERTY(CDROM_stopped)

  CDstatus status = SDL_CDStatus(CDROM);

  GB.ReturnBoolean(status == CD_STOPPED);

END_PROPERTY

BEGIN_PROPERTY(CDROM_length)

  CDstatus status = SDL_CDStatus(CDROM);
  int i, length = 0;

  if (!CD_INDRIVE(status))
  {
    GB.ReturnInteger(0);
  }
  else
  {
    for (i=0;i<((CDROM->numtracks)-1);i++)
    {
      if (CDROM->track[i].type==SDL_AUDIO_TRACK)
        length = length + (CDROM->track[i].length);
    }

    GB.ReturnInteger(length/CD_FPS);
  }

END_PROPERTY

BEGIN_PROPERTY(CDROM_position)

  CDstatus status = SDL_CDStatus(CDROM);
  int i, length =0;

  if (!CD_INDRIVE(status))
    GB.ReturnInteger(0);
  else
  {
    for (i=0;i<(CDROM->cur_track);i++)
    {
      if (CDROM->track[i].type==SDL_AUDIO_TRACK)
        length = length + (CDROM->track[i].length);
    }

    GB.ReturnInteger(((CDROM->cur_frame)+length)/CD_FPS);
  }

END_PROPERTY

BEGIN_PROPERTY(CDROM_volume)

#ifdef OS_LINUX

  struct cdrom_volctrl vol;

  if (READ_PROPERTY)
  {
    ioctl(CDROM->id, CDROMVOLREAD, (void *) &vol);
    GB.ReturnInteger(vol.channel0);
  }
  else
  {
    int volume = VPROP(GB_INTEGER);

    if (volume<0)
      volume = 0;

    if (volume>255)
      volume = 255;

    vol.channel0 = volume;
    vol.channel1 = volume;
    ioctl(CDROM->id, CDROMVOLCTRL,(void *) &vol);
  }

#endif

#ifdef OS_FREEBSD

  struct ioc_vol vol;

  if (READ_PROPERTY)
  {
    ioctl(CDROM->id, CDIOCGETVOL, &vol);
    GB.ReturnInteger(vol.vol[0]);
  }
  else
  {
    int volume = VPROP(GB_INTEGER);

    if (volume <0)
      volume = 0;
    if (volume>255)
      volume = 255;

    vol.vol[0] = volume;
    vol.vol[1] = volume;
    vol.vol[2] = 0;
    vol.vol[3] = 0;
    ioctl(CDROM->id, CDIOCSETVOL, &vol);
  }

#endif

#if !defined(OS_LINUX) && !defined(OS_FREEBSD)

  if (!READ_PROPERTY)
    GB.ReturnInteger(0);

#endif

END_PROPERTY

/**************************************************************************/

GB_DESC Cquerycdrom[] =
{
  GB_DECLARE("CDRoms", 0), GB_VIRTUAL_CLASS(),

  GB_STATIC_METHOD("_get", "s", CDROMS_get, "(Index)i"),
  GB_STATIC_PROPERTY_READ("Count", "i",  CDROMS_count),

  GB_END_DECLARE
};

GB_DESC Ctrack[] =
{
  GB_DECLARE(".Track",0), GB_VIRTUAL_CLASS(),

  GB_METHOD("Play",   NULL, TRACK_play,   "[(Start)i(Length)i]"),

  GB_PROPERTY_READ("Length",   "i", TRACK_length),
  GB_PROPERTY_READ("Position", "i", TRACK_position),
  GB_PROPERTY_READ("Playable", "b", TRACK_playable),

  GB_END_DECLARE
};

GB_DESC Ctracks[] =
{
  GB_DECLARE(".Tracks",0), GB_VIRTUAL_CLASS(),

  GB_METHOD("_get", ".Track", TRACKS_get, "(Index)i"),

  GB_PROPERTY_READ("Count",    "i", TRACKS_count),
  GB_PROPERTY_READ("Current",  "i", TRACKS_current),

  GB_END_DECLARE
};

GB_DESC Ccdrom[] =
{
  GB_DECLARE("CDRom", sizeof(CCDROM)),

  GB_METHOD("_new",  NULL, CDROM_new,  "[(Index)i]"),
  GB_METHOD("_free", NULL, CDROM_free, NULL),

  /* cdrom commands */
  GB_METHOD("Eject",  NULL, CDROM_eject,  NULL),
  GB_METHOD("Stop",   NULL, CDROM_stop,   NULL),
  GB_METHOD("Pause",  NULL, CDROM_pause,  NULL),
  GB_METHOD("Resume", NULL, CDROM_resume, NULL),
  GB_METHOD("Play",   NULL, CDROM_play,   "[(StartTrack)i(NbrTracks)i]"),

  /* cdrom status */
  GB_PROPERTY_READ("Ready",    "b", CDROM_ready),
  GB_PROPERTY_READ("Paused",   "b", CDROM_paused),
  GB_PROPERTY_READ("Playing",  "b", CDROM_playing),
  GB_PROPERTY_READ("Stopped",  "b", CDROM_stopped),
  GB_PROPERTY_READ("Length",   "i", CDROM_length),
  GB_PROPERTY_READ("Position", "i", CDROM_position),

  GB_PROPERTY("Volume", "i", CDROM_volume),

  /* Track management */
  GB_PROPERTY_SELF("Tracks", ".Tracks"),

  GB_END_DECLARE
};
