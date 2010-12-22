/***************************************************************************

	CDrawingArea.cpp

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
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

***************************************************************************/

#define __CDRAWINGAREA_CPP

#include <stdio.h>

#include "main.h"
#include "gambas.h"
#include "widgets.h"
#include "CDraw.h"
#include "cpaint_impl.h"
#include "CDrawingArea.h"
#include "CWidget.h"
#include "CContainer.h"

DECLARE_EVENT(EVENT_draw);

/***************************************************************************

	DrawingArea

***************************************************************************/

static void Darea_Expose(gDrawingArea *sender,int x,int y,int w,int h) 
{
	CWIDGET *_object = GetObject(sender);
	
	//fprintf(stderr, "expose: %d %d %d %d\n", x, y, w, h);
	
	if (GB.CanRaise(THIS, EVENT_draw))
	{
		if (THIS->painted)
		{
			PAINT_begin(THIS);
			PAINT_clip(x, y, w, h);
			
			GB.Raise(THIS, EVENT_draw, 0);
			
			PAINT_end();
		}
		else
		{
			DRAW_begin(THIS);
			DRAW_get_current()->setClip(x, y, w, h);
			
			GB.Raise(THIS, EVENT_draw, 0);
			
			DRAW_end();
		}
	}
}


BEGIN_METHOD(CDRAWINGAREA_new, GB_OBJECT parent)

	InitControl(new gDrawingArea(CONTAINER(VARG(parent))), (CWIDGET*)THIS);
	WIDGET->onExpose = Darea_Expose;

END_METHOD

BEGIN_PROPERTY(CDRAWINGAREA_border)

	if (READ_PROPERTY) { GB.ReturnInteger(WIDGET->getBorder()); return; }
	WIDGET->setBorder(VPROP(GB_INTEGER));

END_PROPERTY

BEGIN_PROPERTY(CDRAWINGAREA_merge)

	fprintf(stderr, "warning: DrawingArea.Merge property has been deprecated\n");

END_PROPERTY


BEGIN_PROPERTY(CDRAWINGAREA_cached)

	if (READ_PROPERTY) { GB.ReturnBoolean(WIDGET->cached()); return; }
	WIDGET->setCached(VPROP(GB_BOOLEAN));

END_PROPERTY

BEGIN_PROPERTY(CDRAWINGAREA_focus)

	if (READ_PROPERTY) { GB.ReturnBoolean(WIDGET->canFocus()); return; }
	WIDGET->setCanFocus(VPROP(GB_BOOLEAN));

END_PROPERTY

BEGIN_METHOD_VOID(CDRAWINGAREA_clear)

	WIDGET->clear();

END_METHOD

BEGIN_PROPERTY(CDRAWINGAREA_painted)

	if (READ_PROPERTY)
		GB.ReturnBoolean(THIS->painted);
	else
		THIS->painted = VPROP(GB_BOOLEAN);

END_PROPERTY

BEGIN_PROPERTY(DrawingArea_NoBackground)

	if (READ_PROPERTY)
		GB.ReturnBoolean(WIDGET->hasNoBackground());
	else
		WIDGET->setNoBackground(VPROP(GB_BOOLEAN));

END_PROPERTY

GB_DESC CDrawingAreaDesc[] =
{
	GB_DECLARE("DrawingArea", sizeof(CDRAWINGAREA)), GB_INHERITS("Container"),

	GB_METHOD("_new", 0, CDRAWINGAREA_new, "(Parent)Container;"),

	GB_PROPERTY("Arrangement", "i", CCONTAINER_arrangement),
	GB_PROPERTY("AutoResize", "b", CCONTAINER_auto_resize),
	GB_PROPERTY("Padding", "i", CCONTAINER_padding),
	GB_PROPERTY("Spacing", "b", CCONTAINER_spacing),
	GB_PROPERTY("Margin", "b", CCONTAINER_margin),
	GB_PROPERTY("Indent", "b", CCONTAINER_indent),

	GB_PROPERTY("Cached", "b", CDRAWINGAREA_cached),
	GB_PROPERTY("Border", "i", CDRAWINGAREA_border),
	GB_PROPERTY("Merge","b",CDRAWINGAREA_merge),
	GB_PROPERTY("Focus","b",CDRAWINGAREA_focus),
	GB_PROPERTY("Painted", "b", CDRAWINGAREA_painted),
	GB_PROPERTY("NoBackground", "b", DrawingArea_NoBackground),

	GB_METHOD("Clear", 0, CDRAWINGAREA_clear, 0),

	GB_EVENT("Draw", 0, 0, &EVENT_draw),
	
	GB_INTERFACE("Draw", &DRAW_Interface),
	GB_INTERFACE("Paint", &PAINT_Interface),
	
	DRAWINGAREA_DESCRIPTION,

	GB_END_DECLARE
};


