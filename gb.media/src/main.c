/***************************************************************************

  main.c

  gb.media component

  (c) 2000-2013 Benoît Minisini <gambas@users.sourceforge.net>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 1, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
  MA 02110-1301, USA.

***************************************************************************/

#define __MAIN_C

#include "main.h"
#include "c_media.h"
#include "c_mediaplayer.h"

GB_INTERFACE GB EXPORT;
IMAGE_INTERFACE IMAGE EXPORT;

GB_DESC *GB_CLASSES[] EXPORT =
{
	//MediaSignalArgumentsDesc,
	MediaTagListDesc,
	MediaLinkDesc,
	MediaControlDesc,
	MediaFilterDesc,
	MediaContainerChildrenDesc,
	MediaContainerDesc,
	MediaPipelineDesc,
	MediaDesc,
	MediaPlayerAudioDesc,
	MediaPlayerVideoDesc,
	MediaPlayerSubtitlesDesc,
	MediaPlayerBalanceChannelDesc,
	MediaPlayerBalanceDesc,
	MediaPlayerDesc,
  NULL
};

int MAIN_get_x11_handle(void *control)
{
	int (*get_handle)(void *) = NULL;
	
	if (!get_handle)
	{
		GB.Component.GetInfo("GET_HANDLE", (void **)&get_handle);
		if (!get_handle)
		{
			GB.Error("Unable to get window handle");
			return 0;
		}
	}
	
	return (*get_handle)(control);
}

static void *_old_hook_main;

static void hook_main(int *argc, char ***argv)
{
	gst_init(argc, argv);
	CALL_HOOK_MAIN(_old_hook_main, argc, argv);
}

int EXPORT GB_INIT()
{
	_old_hook_main = GB.Hook(GB_HOOK_MAIN, (void *)hook_main);
	GB.GetInterface("gb.image", IMAGE_INTERFACE_VERSION, &IMAGE);
	return 0;
}

void EXPORT GB_EXIT()
{
}

