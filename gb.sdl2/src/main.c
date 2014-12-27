/***************************************************************************

  main.c

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

#define __MAIN_C

// Lazyfoo!

#include "gambas.h"
#include "main.h"
#include "c_draw.h"
#include "c_window.h"

#include "gb_list_temp.h"

GB_INTERFACE GB EXPORT;
IMAGE_INTERFACE IMAGE EXPORT;

GB_CLASS CLASS_Window;

static void event_loop()
{
	SDL_Event event;
	
	while (SDL_PollEvent(&event))
	{
		switch(event.type)
		{
			case SDL_QUIT:
				break;
			case SDL_WINDOWEVENT:
				WINDOW_handle_event(&event.window);
				break;
		}
	}
}

static void my_main(int *argc, char **argv)
{
	CLASS_Window = GB.FindClass("Window");
}

static int my_loop()
{
	while (WINDOW_list != NULL)
	{
		event_loop();
		WINDOW_update();
	}

	return 1;
}

static void my_wait(int duration)
{
	event_loop();
}

GB_DESC *GB_CLASSES[] EXPORT =
{
	DrawDesc,
	WindowDesc,
	NULL
};

int EXPORT GB_INIT(void)
{
	GB.GetInterface("gb.image", IMAGE_INTERFACE_VERSION, &IMAGE);
	IMAGE.SetDefaultFormat(GB_IMAGE_BGRA);

	GB.Hook(GB_HOOK_MAIN, (void *)my_main);
	GB.Hook(GB_HOOK_LOOP, (void *)my_loop);
	GB.Hook(GB_HOOK_WAIT, (void *)my_wait);

	/*CLASS_Window = GB.FindClass("Window");
	CLASS_Image = GB.FindClass("Image");
	CLASS_Font = GB.FindClass("Font");*/
	
	if (SDL_Init(SDL_INIT_EVERYTHING))
	{
		fprintf(stderr, "gb.sdl2: error: unable to initialize the library: %s\n", SDL_GetError());
		abort();
	}

	return -1;
}

void EXPORT GB_EXIT()
{
	SDL_Quit();
}

void EXPORT GB_SIGNAL(int signal, void *param)
{
	//static bool wasFullscreen = false;

	//if (!SDLcore::GetWindow())
	//	return;

	if ((signal == GB_SIGNAL_DEBUG_BREAK) || (signal == GB_SIGNAL_DEBUG_CONTINUE))
	{
		/*if (SDLcore::GetWindow()->IsFullScreen())
		{
			wasFullscreen = true;
			SDLcore::GetWindow()->SetFullScreen(false);
		}*/
	}

	if (signal == GB_SIGNAL_DEBUG_CONTINUE)
	{
		/*if (wasFullscreen)
			SDLcore::GetWindow()->SetFullScreen(true);*/
	}
}

