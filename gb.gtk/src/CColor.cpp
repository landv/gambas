/***************************************************************************

  CColor.cpp

  (c) 2004-2006 - Daniel Campos Fernández <dcamposf@gmail.com>

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

#define __CCOLOR_CPP

#include <math.h>

#include "CColor.h"
#include "gdesktop.h"
#include "gcolor.h"

BEGIN_PROPERTY(CCOLOR_background)

	GB.ReturnInteger(gDesktop::bgColor());

END_PROPERTY

BEGIN_PROPERTY(CCOLOR_foreground)

	GB.ReturnInteger(gDesktop::fgColor());

END_PROPERTY

BEGIN_PROPERTY(CCOLOR_text_background)

	GB.ReturnInteger(gDesktop::textbgColor());

END_PROPERTY

BEGIN_PROPERTY(CCOLOR_text_foreground)

	GB.ReturnInteger(gDesktop::textfgColor());

END_PROPERTY

BEGIN_PROPERTY(CCOLOR_selected_background)

	GB.ReturnInteger(gDesktop::selbgColor());

END_PROPERTY

BEGIN_PROPERTY(CCOLOR_light_background)

	GB.ReturnInteger(gDesktop::lightbgColor());

END_PROPERTY

BEGIN_PROPERTY(CCOLOR_selected_foreground)

	GB.ReturnInteger(gDesktop::selfgColor());

END_PROPERTY

BEGIN_PROPERTY(CCOLOR_button_background)

	GB.ReturnInteger(gDesktop::buttonbgColor());

END_PROPERTY

BEGIN_PROPERTY(CCOLOR_button_foreground)

	GB.ReturnInteger(gDesktop::buttonfgColor());

END_PROPERTY

BEGIN_PROPERTY(CCOLOR_light_foreground)

	GB.ReturnInteger(gDesktop::lightfgColor());

END_PROPERTY

GB_DESC CColorDesc[] =
{
  GB_DECLARE("Color", 0), GB_VIRTUAL_CLASS(),

  GB_STATIC_PROPERTY("Background", "i", CCOLOR_background),
  GB_STATIC_PROPERTY("SelectedBackground", "i", CCOLOR_selected_background),
  GB_STATIC_PROPERTY("LightBackground", "i", CCOLOR_light_background),
  GB_STATIC_PROPERTY("TextBackground", "i", CCOLOR_text_background),
  GB_STATIC_PROPERTY("ButtonBackground", "i", CCOLOR_button_background),

  GB_STATIC_PROPERTY("Foreground", "i", CCOLOR_foreground),
  GB_STATIC_PROPERTY("SelectedForeground", "i", CCOLOR_selected_foreground),
  GB_STATIC_PROPERTY("LightForeground", "i", CCOLOR_light_foreground),
  GB_STATIC_PROPERTY("TextForeground", "i", CCOLOR_text_foreground),
  GB_STATIC_PROPERTY("ButtonForeground", "i", CCOLOR_button_foreground),

  GB_END_DECLARE
};


