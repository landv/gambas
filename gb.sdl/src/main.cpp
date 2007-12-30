/***************************************************************************

  main.cpp

  Gambas extension using SDL

  (c) 2006 Laurent Carlier <lordheavy@infonie.fr>
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

GB_CLASS CLASS_Window;
GB_CLASS CLASS_Image;

class mySDLapp : public SDLapplication
{
public:
	mySDLapp() {};
	virtual ~mySDLapp() {};

	virtual void ManageError(const char *myError);
};

mySDLapp *myApp = NULL;
static int my_loop(void );
static int my_image(CIMAGE **pimage, GB_IMAGE_INFO *info);

static void convert_image_data(void *dst, void *src, int w, int h, int format)
{
	int i;
	char *s;
	char *d;

	switch (format)
	{
		case GB_IMAGE_BGRA: case GB_IMAGE_BGRX: // BGRA to RGBA
			s = (char *)src;
			d = (char *)dst;
			for (i = 0; i < (w * h); i++)
			{
				d[0] = s[2];
				d[1] = s[1];
				d[2] = s[0];
				d[3] = s[3];
				s += 4;
				d += 4;
			}
			break;

		case GB_IMAGE_RGBA: case GB_IMAGE_RGBX:
			memcpy(dst, src, w * h * 4);
			break;

		case GB_IMAGE_ARGB: case GB_IMAGE_XRGB: // ARGB to RGBA
			s = (char *)src;
			d = (char *)dst;
			for (i = 0; i < (w * h); i++)
			{
				d[0] = s[3];
				d[1] = s[0];
				d[2] = s[1];
				d[3] = s[2];
				s += 4;
				d += 4;
			}
			break;

		case GB_IMAGE_BGR: // BGR to RGBA
			s = (char *)src;
			d = (char *)dst;
			for (i = 0; i < (w * h); i++)
			{
				d[0] = s[2];
				d[1] = s[1];
				d[2] = s[0];
				d[3] = 0xFF;
				s += 3;
				d += 4;
			}
			break;

		case GB_IMAGE_RGB: // RGB to RGBA
			s = (char *)src;
			d = (char *)dst;
			for (i = 0; i < (w * h); i++)
			{
				d[0] = s[0];
				d[1] = s[1];
				d[2] = s[2];
				d[3] = 0xFF;
				s += 3;
				d += 4;
			}
			break;
	}
}


static void my_timer(GB_TIMER *timer, bool on)
{
	if (on)
		startTimer(timer);
	else
		stopTimer(timer);
}

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
		CMouse,
		CLine,
		CFill,
		CColor, CColorInfo,

		NULL
	};

	int EXPORT GB_INIT(void)
	{
		myApp = new mySDLapp();

		GB.Hook(GB_HOOK_LOOP, (void *)my_loop);
		//GB.Hook(GB_HOOK_TIMER, (void *)my_timer);
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
	GB.Error(myError);
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
			convert_image_data(img->GetData(), info->data, info->width,
			info->height, info->format);

		GB.New((void **)&image, CLASS_Image, NULL, NULL);

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

