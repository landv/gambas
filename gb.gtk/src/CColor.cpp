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

BEGIN_PROPERTY(Color_Background)

	GB.ReturnInteger(gDesktop::bgColor());

END_PROPERTY

BEGIN_PROPERTY(Color_Foreground)

	GB.ReturnInteger(gDesktop::fgColor());

END_PROPERTY

BEGIN_PROPERTY(Color_TextBackground)

	GB.ReturnInteger(gDesktop::textbgColor());

END_PROPERTY

BEGIN_PROPERTY(Color_TextForeground)

	GB.ReturnInteger(gDesktop::textfgColor());

END_PROPERTY

BEGIN_PROPERTY(Color_SelectedBackground)

	GB.ReturnInteger(gDesktop::selbgColor());

END_PROPERTY

BEGIN_PROPERTY(Color_LightBackground)

	GB.ReturnInteger(gDesktop::lightbgColor());

END_PROPERTY

BEGIN_PROPERTY(Color_SelectedForeground)

	GB.ReturnInteger(gDesktop::selfgColor());

END_PROPERTY

BEGIN_PROPERTY(Color_ButtonBackground)

	GB.ReturnInteger(gDesktop::buttonbgColor());

END_PROPERTY

BEGIN_PROPERTY(Color_ButtonForeground)

	GB.ReturnInteger(gDesktop::buttonfgColor());

END_PROPERTY

BEGIN_PROPERTY(Color_LightForeground)

	GB.ReturnInteger(gDesktop::lightfgColor());

END_PROPERTY

BEGIN_PROPERTY(Color_TooltipBackground)

	GB.ReturnInteger(gDesktop::tooltipBackground());

END_PROPERTY

BEGIN_PROPERTY(Color_TooltipForeground)

	GB.ReturnInteger(gDesktop::tooltipForeground());

END_PROPERTY

BEGIN_PROPERTY(Color_LinkForeground)

  GB.ReturnInteger(gDesktop::linkForeground());

END_PROPERTY

BEGIN_PROPERTY(Color_VisitedForeground)

  GB.ReturnInteger(gDesktop::visitedForeground());

END_PROPERTY

GB_DESC CColorDesc[] =
{
  GB_DECLARE_STATIC("Color"),

  GB_STATIC_PROPERTY_READ("Background", "i", Color_Background),
  GB_STATIC_PROPERTY_READ("SelectedBackground", "i", Color_SelectedBackground),
  GB_STATIC_PROPERTY_READ("LightBackground", "i", Color_LightBackground),
  GB_STATIC_PROPERTY_READ("TextBackground", "i", Color_TextBackground),
  GB_STATIC_PROPERTY_READ("ButtonBackground", "i", Color_ButtonBackground),
  GB_STATIC_PROPERTY_READ("TooltipBackground", "i", Color_TooltipBackground),

  GB_STATIC_PROPERTY_READ("Foreground", "i", Color_Foreground),
  GB_STATIC_PROPERTY_READ("SelectedForeground", "i", Color_SelectedForeground),
  GB_STATIC_PROPERTY_READ("LightForeground", "i", Color_LightForeground),
  GB_STATIC_PROPERTY_READ("TextForeground", "i", Color_TextForeground),
  GB_STATIC_PROPERTY_READ("ButtonForeground", "i", Color_ButtonForeground),
  GB_STATIC_PROPERTY_READ("TooltipForeground", "i", Color_TooltipForeground),
  GB_STATIC_PROPERTY_READ("LinkForeground", "i", Color_LinkForeground),
  GB_STATIC_PROPERTY_READ("VisitedForeground", "i", Color_VisitedForeground),

  GB_END_DECLARE
};


