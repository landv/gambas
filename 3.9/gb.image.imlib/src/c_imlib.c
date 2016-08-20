/***************************************************************************

  c_imlib.c

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

#define __C_IMLIB_C

#include "c_image.h"
#include "c_imlib.h"

#if 0
typedef
	struct IMLIB_DRAW {
		struct IMLIB_DRAW *previous;
		CIMAGE *device;
		Imlib_Image image;
		}
	IMLIB_DRAW;

IMLIB_DRAW *_current = NULL;

#define THIS _current
#define IMAGE (_current->image)

static bool check_device()
{
	if (!_current)
	{
		GB.Error("No current device");
		return TRUE;
	}
	else
		return FALSE;
}

#define CHECK_DEVICE() \
	if (check_device()) \
		return; \
	imlib_context_set_image(THIS->image);

/**** Cairo ****************************************************************/

BEGIN_METHOD(Imlib_Begin, GB_OBJECT device)

	void *device = VARG(device);
	IMLIB_DRAW *draw;

	if (GB.CheckObject(device))
		return;

	GB.Alloc(POINTER(&draw), sizeof(IMLIB_DRAW));
	draw->previous = _current;
	
	if (GB.Is(device, GB.FindClass("Image")))
	{
		draw->image = IMAGE_check(device);
	}
	else
	{
		GB.Free(POINTER(&draw));
		GB.Error("Bad device");
		return;
	}
	
	draw->device = device;
	GB.Ref(device);
	
	_current = draw;

END_METHOD

static void end_current()
{
	IMLIB_DRAW *draw = _current;

	if (!_current)
		return;
		
	_current = draw->previous;
	
	GB.Unref(POINTER(&draw->device));
	GB.Free(POINTER(&draw));
}

BEGIN_METHOD_VOID(Imlib_End)

	if (check_device())
		return;
		
	end_current();

END_METHOD

BEGIN_METHOD_VOID(Imlib_exit)

	while	(_current)
		end_current();

END_METHOD
#endif

GB_DESC ImlibDesc[] =
{
  GB_DECLARE("Imlib", 0), GB_VIRTUAL_CLASS(),

	//GB_STATIC_PROPERTY("Image", "Image", Imlib_Image),
	//GB_STATIC_PROPERTY("AntiAlias", "b", Imlib_AntiAlias),

  GB_END_DECLARE
};

