/***************************************************************************

  c_mediavideo.c

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

#define __C_MEDIAVIDEO_C

#include <linux/videodev2.h>

#include "c_mediavideo.h"

BEGIN_METHOD_VOID(MediaVideo_new)

END_METHOD

//-------------------------------------------------------------------------

#define IMPLEMENT_FLAG(_proc, _flag) \
BEGIN_PROPERTY(MediaVideo_##_proc) \
\
	GB.ReturnBoolean(MEDIA_get_flag(ELEMENT, "flags", _flag)); \
\
END_PROPERTY

IMPLEMENT_FLAG(HasCapture, V4L2_CAP_VIDEO_CAPTURE)
IMPLEMENT_FLAG(HasOutput, V4L2_CAP_VIDEO_OUTPUT)
IMPLEMENT_FLAG(HasOverlay, V4L2_CAP_VIDEO_OVERLAY)
IMPLEMENT_FLAG(HasVBICapture, V4L2_CAP_VBI_CAPTURE)
IMPLEMENT_FLAG(HasVBIOutput, V4L2_CAP_VBI_OUTPUT)
IMPLEMENT_FLAG(HasTuner, V4L2_CAP_TUNER)
IMPLEMENT_FLAG(HasAudio, V4L2_CAP_AUDIO)

GB_DESC MediaVideoDesc[] = 
{
	GB_DECLARE("MediaVideo", sizeof(CMEDIAVIDEO)),
	GB_INHERITS("MediaControl"),
	
	GB_METHOD("_new", NULL, MediaVideo_new, NULL),
	
	//GB_PROPERTY("Device", "s", MediaVideo_Device),
	//GB_PROPERTY_READ("Name", "s", MediaVideo_Name),
	
	GB_PROPERTY_READ("HasCapture", "b", MediaVideo_HasCapture),
	GB_PROPERTY_READ("HasOutput", "b", MediaVideo_HasOutput),
	GB_PROPERTY_READ("HasOverlay", "b", MediaVideo_HasOverlay),
	GB_PROPERTY_READ("HasVBICapture", "b", MediaVideo_HasVBICapture),
	GB_PROPERTY_READ("HasVBIOutput", "b", MediaVideo_HasVBIOutput),
	GB_PROPERTY_READ("HasTuner", "b", MediaVideo_HasTuner),
	GB_PROPERTY_READ("HasAudio", "b", MediaVideo_HasAudio),
	
	GB_END_DECLARE
};

