/***************************************************************************

  CDraw.cpp

  (c) 2000-2007 Benoit Minisini <gambas@users.sourceforge.net>

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

#define __CDRAW_CPP

#ifdef OS_SOLARIS
/* Make math.h define M_PI and a few other things */
#define __EXTENSIONS__
/* Get definition for finite() */
#include <ieeefp.h>
#endif
#include <math.h>

#include <qpainter.h>
#include <qpen.h>
#include <qbrush.h>
#include <qapplication.h>
#include <qpaintdevicemetrics.h>
#include <qpicture.h>
#include <qpixmap.h>
#include <qbitmap.h>

#include "CDrawing.h"
#include "CPrinter.h"
#include "CDraw.h"

GB_DRAW_DESC DRAW_Interface;
DRAW_INTERFACE DRAW;

#define EXTRA(d) ((QT_DRAW_EXTRA *)(&(d->extra)))
#define DP(d) (EXTRA(d)->p)
#define DPM(d) (EXTRA(d)->pm)

static bool _init = FALSE;

static GB_CLASS CLASS_Printer, CLASS_Drawing;

static void init()
{
	if (_init)
		return;
		
	GB.GetInterface("gb.draw", DRAW_INTERFACE_VERSION, &DRAW);
	CLASS_Printer = GB.FindClass("Printer");
	CLASS_Drawing = GB.FindClass("Drawing");
	_init = TRUE;
}

static void init_drawing(GB_DRAW *d, QPainter *p, int w, int h, int dpi = 0)
{
  EXTRA(d)->p = p;
  EXTRA(d)->pm = 0;
  EXTRA(d)->mask = 0;
  d->width = w;
  d->height = h;
  
	if (dpi)
		d->resolution = dpi;
	else
		#ifdef NO_X_WINDOW
			d->resolution = 72;
		#else
			d->resolution = QPaintDevice::x11AppDpiY();
		#endif
}

static int begin(GB_DRAW *d)
{
	void *device = d->device;
	
	init();

  if ((GB_CLASS)device == CLASS_Printer)
  {
    CPRINTER_init();
    QPrinter *printer = CPRINTER_printer;
		QPaintDeviceMetrics pdm(printer);
    init_drawing(d, new QPainter(printer), pdm.width(), pdm.height(), printer->resolution());
  }
  else if (GB.Is(device, CLASS_Drawing))
  {
    CDRAWING *drawing = (CDRAWING *)device;
    init_drawing(d, new QPainter(drawing->picture), drawing->picture->boundingRect().width(), drawing->picture->boundingRect().height());
  }
  
  return FALSE;
}

void DRAW_init()
{
	DRAW_Interface = *((GB_DRAW_DESC *)QT.GetDrawInterface());
	DRAW_Interface.Begin = begin;	
}

BEGIN_METHOD(CDRAW_drawing, GB_OBJECT drawing; GB_INTEGER x; GB_INTEGER y; GB_INTEGER w; GB_INTEGER h; GB_INTEGER sx; GB_INTEGER sy; GB_INTEGER sw; GB_INTEGER sh)

  static bool warn = false;
  GB_DRAW *d;
  int x, y, w, h;
  int sx, sy, sw, sh;
  CDRAWING *drawing = (CDRAWING *)VARG(drawing);
  QPicture *pic;

  d = DRAW.GetCurrent();
  if (!d)
  	return;
  
  if (GB.CheckObject(drawing))
    return;

  x = VARGOPT(x, 0);
  y = VARGOPT(y, 0);
  w = VARGOPT(w, -1);
  h = VARGOPT(h, -1);

  sx = VARGOPT(sx, 0);
  sy = VARGOPT(sy, 0);
  sw = VARGOPT(sw, -1);
  sh = VARGOPT(sh, -1);

  pic = drawing->picture;

  DP(d)->save();
  if ((sw > 0) && (sh > 0))
    DP(d)->setClipRect(x + sx, y + sy, sw, sh);
  DP(d)->translate(x, y);
  if ((w > 0) && (h > 0))
  	DP(d)->scale((double)w / pic->boundingRect().width(), (double)h / pic->boundingRect().height());
  DP(d)->drawPicture(0, 0, *pic);
  DP(d)->restore();

  if (DPM(d))
  {
		DPM(d)->save();
    if ((sw > 0) && (sh > 0))
      DPM(d)->setClipRect(x + sx, y + sy, sw, sh);
		DPM(d)->translate(x, y);
		if ((w > 0) && (h > 0))
			DPM(d)->scale((double)w / pic->boundingRect().width(), (double)h / pic->boundingRect().height());
		DPM(d)->drawPicture(0, 0, *pic);
		DPM(d)->restore();
    if (!warn)
    {
      qDebug("WARNING: Draw.Drawing() on transparent devices partially implemented.");
      warn = true;
    }
  }

END_METHOD

GB_DESC CDrawDesc[] =
{
  GB_DECLARE("Draw", 0), GB_VIRTUAL_CLASS(),

  GB_STATIC_METHOD("Drawing", NULL, CDRAW_drawing, "(Drawing)Drawing;(X)i(Y)i[(Width)i(Height)i(SrcX)i(SrcY)i(SrcWidth)i(SrcHeight)i]"),

  GB_END_DECLARE
};
