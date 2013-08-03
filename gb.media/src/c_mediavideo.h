/***************************************************************************

  c_mediavideo.h

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

#ifndef __C_MEDIAVIDEO_H
#define __C_MEDIAVIDEO_H

#include "main.h"
#include "c_media.h"

#include <gst/video/colorbalance.h>
#include <gst/video/colorbalancechannel.h>

#ifndef __C_MEDIAVIDEO_C

extern GB_DESC MediaVideoDesc[];

#else

#define THIS ((CMEDIAVIDEO *)_object)
#define ELEMENT ((GstElement *)THIS->elt)

#endif

typedef
	CMEDIACONTROL
		CMEDIAVIDEO;

#endif /* __C_MEDIAVIDEO_H */
