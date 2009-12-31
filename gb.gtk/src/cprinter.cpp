/***************************************************************************

  cprinter.cpp

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

#define __CPRINTER_CPP

#include "main.h"
#include "gambas.h"
#include "widgets.h"
#include "cpaint_impl.h"
#include "cprinter.h"

DECLARE_EVENT(EVENT_Begin);
DECLARE_EVENT(EVENT_End);
DECLARE_EVENT(EVENT_Paginate);
DECLARE_EVENT(EVENT_Draw);
DECLARE_EVENT(EVENT_Error);

static void cb_begin(gPrinter *printer)
{
	void *_object = printer->tag;
	THIS->current = 0;
	GB.Raise(THIS, EVENT_Begin, 0);
}

static void cb_end(gPrinter *printer)
{
	void *_object = printer->tag;
	THIS->current = 0;
	GB.Raise(THIS, EVENT_End, 0);
}

static void cb_paginate(gPrinter *printer)
{
	void *_object = printer->tag;

	if (GB.CanRaise(THIS, EVENT_Paginate))
		GB.Raise(THIS, EVENT_Paginate, 0);
	else
		printer->setPageCount(printer->pageCount());
}

static void cb_draw(gPrinter *printer, GtkPrintContext *context, int page)
{
	void *_object = printer->tag;
	THIS->current = page;
	THIS->context = context;
	PAINT_begin(THIS);
	GB.Raise(THIS, EVENT_Draw, 0);
	PAINT_end();
}

BEGIN_METHOD_VOID(Printer_new)

	THIS->printer = new gPrinter();
	PRINTER->tag = THIS;
	PRINTER->onBegin = cb_begin;
	PRINTER->onEnd = cb_end;
	PRINTER->onDraw = cb_draw;
	PRINTER->onPaginate = cb_paginate;

END_METHOD

BEGIN_METHOD_VOID(Printer_free)

	delete THIS->printer;

END_METHOD

BEGIN_METHOD(Printer_Run, GB_BOOLEAN configure)

	GB.ReturnBoolean(PRINTER->run(!VARGOPT(configure, FALSE)));

END_METHOD

BEGIN_METHOD_VOID(Printer_Cancel)

	PRINTER->cancel();

END_METHOD

BEGIN_PROPERTY(Printer_Count)

	if (READ_PROPERTY)
		GB.ReturnInteger(PRINTER->pageCount());
	else
		PRINTER->setPageCount(VPROP(GB_INTEGER));

END_PROPERTY

BEGIN_PROPERTY(Printer_Current)

	GB.ReturnInteger(THIS->current);

END_PROPERTY

GB_DESC PrinterDesc[] =
{
  GB_DECLARE("Printer", sizeof(CPRINTER)),

	GB_METHOD("_new", NULL, Printer_new, NULL),
	GB_METHOD("_free", NULL, Printer_free, NULL),
	
	GB_METHOD("Run", "b", Printer_Run, "[(DoNotConfigure)b]"),
	GB_METHOD("Cancel", NULL, Printer_Cancel, NULL),
	
	GB_PROPERTY("Count", "i", Printer_Count),
	GB_PROPERTY_READ("Page", "i", Printer_Current),
	
	GB_EVENT("Begin", NULL, NULL, &EVENT_Begin),
	GB_EVENT("End", NULL, NULL, &EVENT_End),
	GB_EVENT("Paginate", NULL, NULL, &EVENT_Paginate),
	GB_EVENT("Draw", NULL, NULL, &EVENT_Draw),
	GB_EVENT("Error", NULL, NULL, &EVENT_Error),

  GB_INTERFACE("Paint", &PAINT_Interface),

  GB_END_DECLARE
};

