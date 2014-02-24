/***************************************************************************

  c_mediaplayer.h

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

#ifndef __C_MEDIAPLAYER_H
#define __C_MEDIAPLAYER_H

#include "main.h"
#include "c_media.h"

#include <gst/video/colorbalance.h>
#include <gst/video/colorbalancechannel.h>

#ifndef __C_MEDIAPLAYER_C

extern GB_DESC MediaPlayerDesc[];
extern GB_DESC MediaPlayerAudioDesc[];
extern GB_DESC MediaPlayerVideoDesc[];
extern GB_DESC MediaPlayerSubtitlesDesc[];
extern GB_DESC MediaPlayerBalanceDesc[];
extern GB_DESC MediaPlayerBalanceChannelDesc[];

#else

#define THIS ((CMEDIAPLAYER *)_object)
#define THIS_CONTROL (&(THIS->base.control))
#define ELEMENT ((GstPipeline *)(THIS_CONTROL->elt))
#define BALANCE (GST_COLOR_BALANCE(ELEMENT))

#endif

typedef
	struct {
		CMEDIAPIPELINE base;
		int channel;
	}
	CMEDIAPLAYER;

#endif /* __C_MEDIAPLAYER_H */
