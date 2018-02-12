/***************************************************************************

  cfaketrayicon.cpp

  (c) 2000-2017 Benoît Minisini <g4mba5@gmail.com>

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

#define __CFAKETRAYICON_CPP

#include "gambas.h"
#include "main.h"

#include "cfaketrayicon.h"


GB_DESC FakeTrayIconsDesc[] =
{
	GB_DECLARE_STATIC("TrayIcons"),

	GB_STATIC_METHOD("_next", "TrayIcon", NULL, NULL),
	GB_STATIC_METHOD("_get", "TrayIcon", NULL, "(Index)i"),
	GB_STATIC_PROPERTY_READ("Count", "i", NULL),
	GB_STATIC_METHOD("DeleteAll", NULL, NULL, NULL),

	GB_END_DECLARE
};


GB_DESC FakeTrayIconDesc[] =
{
	GB_DECLARE("TrayIcon", 0),

	//GB_STATIC_METHOD("_exit", NULL, NULL, NULL),

	GB_CONSTANT("Horizontal", "i", 0),
	GB_CONSTANT("Vertical", "i", 1),

	GB_METHOD("_new", NULL, NULL, NULL),
	GB_METHOD("_free", NULL, NULL, NULL),

	GB_METHOD("Show", NULL, NULL, NULL),
	GB_METHOD("Hide", NULL, NULL, NULL),
	GB_METHOD("Delete", NULL, NULL, NULL),

	GB_PROPERTY("Picture", "Picture", NULL),
	GB_PROPERTY("Icon", "Picture", NULL),
	GB_PROPERTY("Visible", "b", NULL),

	GB_PROPERTY("Text", "s", NULL),
	GB_PROPERTY("PopupMenu", "s", NULL),
	GB_PROPERTY("Tooltip", "s", NULL),
	GB_PROPERTY("Tag", "v", NULL),
	
	GB_EVENT("Click", NULL, NULL, NULL),
	GB_EVENT("MiddleClick", NULL, NULL, NULL),
	GB_EVENT("Scroll", NULL, "(Delta)f(Orientation)i", NULL),

	GB_CONSTANT("_IsControl", "b", TRUE),
	GB_CONSTANT("_Family", "s", "*"),
	GB_CONSTANT("_IsVirtual", "b", TRUE),
	GB_CONSTANT("_Group", "s", "Special"),
	GB_CONSTANT("_DefaultEvent", "s", "Click"),
	GB_CONSTANT("_Properties", "s", "Visible=False,Tag,Tooltip,Picture,PopupMenu{Menu}"),

	GB_END_DECLARE
};

