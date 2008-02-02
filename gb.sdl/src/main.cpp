/***************************************************************************

  main.cpp

  Gambas extension using SDL

  (c) 2006 Laurent Carlier <lordheavy@users.sourceforge.net>
           Benoît Minisini <gambas@users.sourceforge.net>

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

#define __MAIN_CPP

#include "gambas.h"
#include "main.h"

#include <iostream>
#include "SDL_h.h"
#include "SDL_syswm.h"

#include "SDLapp.h"
#include "SDLcore.h"

#include "timer.h"

#include "Cconst.h"
#include "Cwindow.h"
#include "Cimage.h"
#include "Cdraw.h"
#include "Ckey.h"
#include "Cmouse.h"
#include "Cdesktop.h"
#include "Ccolor.h"
#include "Cfont.h"

GB_CLASS CLASS_Window;
GB_CLASS CLASS_Image;

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
static int my_image(CIMAGE **pimage, GB_IMAGE_INFO *info);

extern "C"
{
	GB_INTERFACE GB EXPORT;

	GB_DESC *GB_CLASSES[] EXPORT =
	{
		CDesktop,
		CWindow,
		CImage,
		CDraw,
		CKey,
		CMouse, CCursor,
		CLine,
		CFill,
		CColor, CColorInfo,
		CFonts,

		NULL
	};

	int EXPORT GB_INIT(void)
	{

		GB.Hook(GB_HOOK_MAIN, (void *)my_main);
		GB.Hook(GB_HOOK_LOOP, (void *)my_loop);
		GB.Hook(GB_HOOK_IMAGE, (void *)my_image);

		CLASS_Window = GB.FindClass("Window");
		CLASS_Image = GB.FindClass("Image");

		return true;
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
#if 0
static void my_timer(GB_TIMER *timer, bool on)
{
	if (on)
		startTimer(timer);
	else
		stopTimer(timer);
}
#endif

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

static int my_image(CIMAGE **pimage, GB_IMAGE_INFO *info)
{
	CIMAGE *image = *pimage;

	if (!image)
	{
		SDLsurface *img = new SDLsurface();
		img->Create(info->width, info->height, 32); // format RGBA

		if (info->data)
			GB.Image.Convert(img->GetData(), GB_IMAGE_RGBA, info->data, info->format, info->width, info->height);

		GB.New(POINTER(&image), CLASS_Image, NULL, NULL);

		if (image->id)
			delete image->id;

		image->id = img;
		*pimage = image;
	}
	else
	{
		info->width = image->id->GetWidth();
		info->height = image->id->GetHeight();
		info->data = image->id->GetData();
		info->format = GB_IMAGE_RGBA;
	}

	return 0;
}


