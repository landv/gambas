/***************************************************************************

  Cdesktop.cpp

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

#define __CDESKTOP_CPP

#include "Cdesktop.h"
#include "SDLapp.h"

BEGIN_PROPERTY(CDESKTOP_width)

	GB.ReturnInteger(SDLapp->DesktopWidth());

END_PROPERTY

BEGIN_PROPERTY(CDESKTOP_height)

	GB.ReturnInteger(SDLapp->DesktopHeight());

END_PROPERTY

GB_DESC CDesktop[] =
{
  GB_DECLARE("Desktop", 0), GB_VIRTUAL_CLASS(),

  GB_STATIC_PROPERTY_READ("W", "i", CDESKTOP_width),
  GB_STATIC_PROPERTY_READ("H", "i", CDESKTOP_height),
  GB_STATIC_PROPERTY_READ("Width", "i", CDESKTOP_width),
  GB_STATIC_PROPERTY_READ("Height", "i", CDESKTOP_height),

  GB_END_DECLARE
};
