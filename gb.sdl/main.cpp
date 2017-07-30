/***************************************************************************

  main.cpp

  (c) 2006 Laurent Carlier <lordheavy@users.sourceforge.net>

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

#define __MAIN_CPP

#include "gambas.h"
#include "main.h"

#include <iostream>
#include "SDL_h.h"
#include "SDL_syswm.h"

#include "SDLapp.h"
#include "SDLcore.h"

#include "Cconst.h"
#include "Cdesktop.h"
#include "Cjoystick.h"
#include "Ckey.h"
#include "Cmouse.h"
#include "Cimage.h"
#include "Cdraw.h"
#include "Cwindow.h"
#include "Cfont.h"

GB_CLASS CLASS_Window;
GB_CLASS CLASS_Image;
GB_CLASS CLASS_Font;

class mySDLapp : public SDLapplication
{
public:
	mySDLapp(int &argc, char **argv):SDLapplication(argc, argv) {};
	virtual ~mySDLapp() {};

	virtual void ManageError(const char *myError);
};

mySDLapp *myApp = NULL;

static void my_main(int *argc, char **argv);
static int my_loop(void );
static void my_wait(int duration);

extern "C"
{
	GB_INTERFACE GB EXPORT;
	IMAGE_INTERFACE IMAGE EXPORT;

	GB_DESC *GB_CLASSES[] EXPORT =
	{
		CLine, CFill,
		CFont,
		CDesktop,
		CJoyInfos, CQueryJoys, CJoystick,
		CKey,
		CMouse, // CCursor,
		CImage,
		CDraw,
		CWindow,

		NULL
	};

	int EXPORT GB_INIT(void)
	{
		GB.GetInterface("gb.image", IMAGE_INTERFACE_VERSION, &IMAGE);
		IMAGE.SetDefaultFormat(GB_IMAGE_BGRA);

		GB.Hook(GB_HOOK_MAIN, (void *)my_main);
		GB.Hook(GB_HOOK_LOOP, (void *)my_loop);
		GB.Hook(GB_HOOK_WAIT, (void *)my_wait);

		CLASS_Window = GB.FindClass("Window");
		CLASS_Image = GB.FindClass("Image");
		CLASS_Font = GB.FindClass("Font");

		return -1;
	}

	void EXPORT GB_EXIT()
	{
		delete myApp;
	}

	void EXPORT GB_SIGNAL(int signal, void *param)
	{
		static bool wasFullscreen = false;

		if (!SDLcore::GetWindow())
			return;

		if ((signal == GB_SIGNAL_DEBUG_BREAK) || (signal == GB_SIGNAL_DEBUG_CONTINUE))
		{
			if (SDLcore::GetWindow()->IsFullScreen())
			{
				wasFullscreen = true;
				SDLcore::GetWindow()->SetFullScreen(false);
			}
		}

		if (signal == GB_SIGNAL_DEBUG_CONTINUE)
		{
			if (wasFullscreen)
				SDLcore::GetWindow()->SetFullScreen(true);
		}
	}
}

void mySDLapp::ManageError(const char *myError)
{
	GB.Error(COMP_ERR myError);
}

static void my_main(int *argc, char **argv)
{
	myApp = new mySDLapp(*argc, argv);
}

static int my_loop()
{
	while(myApp->HaveWindows())
	{
		myApp->ManageEvents();
		GB.Loop(10);
	}

	return 1;
}

static void my_wait(int duration)
{
	myApp->ManageEvents(duration == 0);
	GB.Loop(10);
}
