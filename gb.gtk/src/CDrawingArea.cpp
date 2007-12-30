/***************************************************************************

  CDrawingArea.cpp

  (c) 2004-2006 - Daniel Campos Fernández <dcamposf@gmail.com>
  
  GTK+ component
  
  Realizado para la Junta de Extremadura. 
  Consejería de Educación Ciencia y Tecnología. 
  Proyecto gnuLinEx
   
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 1, or (at your option)
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

#include "main.h"
#include "gambas.h"
#include "widgets.h"
#include "CDraw.h"
#include "CDrawingArea.h"
#include "CWidget.h"
#include "CContainer.h"

#include <stdio.h>

DECLARE_EVENT(EVENT_draw);

/***************************************************************************

  DrawingArea

***************************************************************************/

static void Darea_Expose(gDrawingArea *sender,int x,int y,int w,int h) 
{
	CWIDGET *_object = GetObject(sender);
	
	if (GB.CanRaise(THIS, EVENT_draw))
	{
		DRAW_begin(THIS);
		DRAW_get_current()->setClip(x, y, w, h);
		
		GB.Raise(THIS, EVENT_draw, 0);
		
		DRAW_end();
	}
}


BEGIN_METHOD(CDRAWINGAREA_new, GB_OBJECT parent)

	InitControl(new gDrawingArea(CONTAINER(VARG(parent))), (CWIDGET*)THIS);
	DRAWING->onExpose = Darea_Expose;

END_METHOD

BEGIN_PROPERTY(CDRAWINGAREA_border)

	if (READ_PROPERTY) { GB.ReturnInteger(DRAWING->getBorder()); return; }
	DRAWING->setBorder(VPROP(GB_INTEGER));

END_PROPERTY

/*********************************************************
 GTK+ manages the event compression internally,
 this function is placed here only for compatibility
 with gb.qt
**********************************************************/
BEGIN_PROPERTY(CDRAWINGAREA_merge)

	if (READ_PROPERTY) GB.ReturnBoolean(THIS->merge);
	else               THIS->merge=VPROP(GB_BOOLEAN);

END_PROPERTY


BEGIN_PROPERTY(CDRAWINGAREA_cached)

	if (READ_PROPERTY) { GB.ReturnBoolean(DRAWING->cached()); return; }
	DRAWING->setCached(VPROP(GB_BOOLEAN));

END_PROPERTY

BEGIN_PROPERTY(CDRAWINGAREA_focus)

	if (READ_PROPERTY) { GB.ReturnBoolean(DRAWING->canFocus()); return; }
	DRAWING->setCanFocus(VPROP(GB_BOOLEAN));

END_PROPERTY


BEGIN_PROPERTY(CDRAWINGAREA_track_mouse)

	if (READ_PROPERTY)
		GB.ReturnBoolean(DRAWING->isTracking());
	else
		DRAWING->setTracking(VPROP(GB_BOOLEAN));

END_PROPERTY



BEGIN_METHOD_VOID(CDRAWINGAREA_clear)

	DRAWING->clear();

END_METHOD




GB_DESC CDrawingAreaDesc[] =
{
  GB_DECLARE("DrawingArea", sizeof(CDRAWINGAREA)), GB_INHERITS("Container"),

  GB_METHOD("_new", NULL, CDRAWINGAREA_new, "(Parent)Container;"),

  GB_PROPERTY("Cached", "b", CDRAWINGAREA_cached),
  GB_PROPERTY("Border", "i", CDRAWINGAREA_border),
  GB_PROPERTY("Tracking", "b", CDRAWINGAREA_track_mouse),
  GB_PROPERTY("Merge","b",CDRAWINGAREA_merge),
  GB_PROPERTY("Focus","b",CDRAWINGAREA_focus),

  GB_METHOD("Clear", NULL, CDRAWINGAREA_clear, NULL),

  GB_EVENT("Draw", NULL, NULL, &EVENT_draw),
  
  GB_INTERFACE("Draw", &DRAW_Interface),
  
  DRAWINGAREA_DESCRIPTION,

  GB_END_DECLARE
};


