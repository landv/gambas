/***************************************************************************

	CColor.cpp

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

#define __CCOLOR_CPP

#include <qapplication.h>
#include <qcolor.h>
#include <qpalette.h>

#include "gambas.h"

#include "CWidget.h"
#include "CColor.h"

static uint get_light_foreground()
{
	return IMAGE.MergeColor(qApp->palette().color(QPalette::Window).rgb() & 0xFFFFFF, qApp->palette().color(QPalette::WindowText).rgb() & 0xFFFFFF, 0.3);
}

QColor CCOLOR_light_foreground()
{
	return TO_QCOLOR(get_light_foreground());
}

QColor CCOLOR_make(GB_COLOR color)
{
	int b = color & 0xFF;
	int g = (color >> 8) & 0xFF;
	int r = (color >> 16) & 0xFF;
	int a = (color >> 24) ^ 0xFF;
	
	return QColor(r, g, b, a);
}

static void return_color(QPalette::ColorRole role)
{
	GB.ReturnInteger(QApplication::palette().color(role).rgb() & 0xFFFFFF);
}

BEGIN_PROPERTY(Color_Background)

	return_color(QPalette::Window);

END_PROPERTY

BEGIN_PROPERTY(Color_Foreground)

	return_color(QPalette::WindowText);

END_PROPERTY

BEGIN_PROPERTY(Color_TextBackground)

	return_color(QPalette::Base);

END_PROPERTY

BEGIN_PROPERTY(Color_TextForeground)

	return_color(QPalette::Text);

END_PROPERTY

BEGIN_PROPERTY(Color_SelectedBackground)

	return_color(QPalette::Highlight);

END_PROPERTY

BEGIN_PROPERTY(Color_SelectedForeground)

	return_color(QPalette::HighlightedText);

END_PROPERTY

BEGIN_PROPERTY(Color_ButtonBackground)

	return_color(QPalette::Button);

END_PROPERTY

BEGIN_PROPERTY(Color_ButtonForeground)

	return_color(QPalette::ButtonText);

END_PROPERTY

BEGIN_PROPERTY(Color_LightBackground)

	uint col = IMAGE.MergeColor(qApp->palette().color(QPalette::Base).rgb() & 0xFFFFFF, qApp->palette().color(QPalette::Highlight).rgb() & 0xFFFFFF, 0.5);
	GB.ReturnInteger(col);

END_PROPERTY

BEGIN_PROPERTY(Color_LightForeground)

	GB.ReturnInteger(get_light_foreground());

END_PROPERTY

BEGIN_PROPERTY(Color_TooltipBackground)

	return_color(QPalette::ToolTipBase);

END_PROPERTY

static int get_luminance(QColor col)
{
	return (int)(0.299 * col.red() + 0.587 * col.green() + 0.114 * col.blue());
}

BEGIN_PROPERTY(Color_TooltipForeground)

	QColor bg = qApp->palette().color(QPalette::ToolTipBase);
	QColor fg = qApp->palette().color(QPalette::ToolTipText);
	int lbg = get_luminance(bg);
	int lfg = get_luminance(fg);
	
	if (abs(lbg - lfg) <= 64)
		fg.setHsv(fg.hue(), fg.saturation(), 255 - fg.value());
		
	GB.ReturnInteger(fg.rgb() & 0xFFFFFF);

END_PROPERTY

BEGIN_PROPERTY(Color_LinkForeground)

	return_color(QPalette::Link);

END_PROPERTY

BEGIN_PROPERTY(Color_VisitedForeground)

	return_color(QPalette::LinkVisited);

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
	GB_STATIC_PROPERTY_READ("LightForeground", "i", Color_LightForeground),
	GB_STATIC_PROPERTY_READ("SelectedForeground", "i", Color_SelectedForeground),
	GB_STATIC_PROPERTY_READ("TextForeground", "i", Color_TextForeground),
	GB_STATIC_PROPERTY_READ("ButtonForeground", "i", Color_ButtonForeground),
	GB_STATIC_PROPERTY_READ("TooltipForeground", "i", Color_TooltipForeground),
	GB_STATIC_PROPERTY_READ("LinkForeground", "i", Color_LinkForeground),
	GB_STATIC_PROPERTY_READ("VisitedForeground", "i", Color_VisitedForeground),

	GB_END_DECLARE
};


