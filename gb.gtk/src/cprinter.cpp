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
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
  MA 02110-1301, USA.

***************************************************************************/

#define __CPRINTER_CPP

#include "main.h"
#include "gambas.h"
#include "widgets.h"
#include "cpaint_impl.h"
#include "gb.form.print.h"
#include "cprinter.h"

DECLARE_EVENT(EVENT_Begin);
DECLARE_EVENT(EVENT_End);
DECLARE_EVENT(EVENT_Paginate);
DECLARE_EVENT(EVENT_Draw);

static void cb_begin(gPrinter *printer, GtkPrintContext *context)
{
	void *_object = printer->tag;
	THIS->current = 0;
	THIS->context = context;
	PAINT_begin(THIS);
	GB.Raise(THIS, EVENT_Begin, 0);
	PAINT_end();
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
	THIS->current = page + 1;
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

BEGIN_METHOD_VOID(Printer_Configure)

	GB.ReturnBoolean(PRINTER->configure());

END_METHOD

BEGIN_METHOD_VOID(Printer_Print)

	GB.ReturnBoolean(PRINTER->print());

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

BEGIN_PROPERTY(Printer_Name)

	if (READ_PROPERTY)
		GB.ReturnNewZeroString(PRINTER->name());
	else
		PRINTER->setName(GB.ToZeroString(PROP(GB_STRING)));

END_PROPERTY

BEGIN_PROPERTY(Printer_Orientation)

	if (READ_PROPERTY)
		GB.ReturnInteger(PRINTER->orientation());
	else
		PRINTER->setOrientation(VPROP(GB_INTEGER));

END_PROPERTY

BEGIN_PROPERTY(Printer_Paper)

	if (READ_PROPERTY)
		GB.ReturnInteger(PRINTER->paperModel());
	else
		PRINTER->setPaperModel(VPROP(GB_INTEGER));

END_PROPERTY

BEGIN_PROPERTY(Printer_PaperWidth)

	double w, h;
	
	PRINTER->getPaperSize(&w, &h);

	if (READ_PROPERTY)
		GB.ReturnFloat(w);
	else
		PRINTER->setPaperSize(VPROP(GB_FLOAT), h);

END_PROPERTY

BEGIN_PROPERTY(Printer_PaperHeight)

	double w, h;
	
	PRINTER->getPaperSize(&w, &h);

	if (READ_PROPERTY)
		GB.ReturnFloat(h);
	else
		PRINTER->setPaperSize(w, VPROP(GB_FLOAT));

END_PROPERTY

BEGIN_PROPERTY(Printer_CollateCopies)

	if (READ_PROPERTY)
		GB.ReturnBoolean(PRINTER->collateCopies());
	else
		PRINTER->setCollateCopies(VPROP(GB_BOOLEAN));

END_PROPERTY

BEGIN_PROPERTY(Printer_ReverseOrder)

	if (READ_PROPERTY)
		GB.ReturnBoolean(PRINTER->reverserOrder());
	else
		PRINTER->setReverseOrder(VPROP(GB_BOOLEAN));

END_PROPERTY

BEGIN_PROPERTY(Printer_Duplex)

	if (READ_PROPERTY)
		GB.ReturnInteger(PRINTER->duplex());
	else
		PRINTER->setDuplex(VPROP(GB_INTEGER));

END_PROPERTY

BEGIN_PROPERTY(Printer_GrayScale)

	if (READ_PROPERTY)
		GB.ReturnBoolean(!PRINTER->useColor());
	else
		PRINTER->setUseColor(!VPROP(GB_BOOLEAN));

END_PROPERTY

BEGIN_PROPERTY(Printer_NumCopies)

	if (READ_PROPERTY)
		GB.ReturnInteger(PRINTER->numCopies());
	else
		PRINTER->setNumCopies(VPROP(GB_INTEGER));

END_PROPERTY

BEGIN_PROPERTY(Printer_Resolution)

	if (READ_PROPERTY)
		GB.ReturnInteger(PRINTER->resolution());
	else
		PRINTER->setResolution(VPROP(GB_INTEGER));

END_PROPERTY

BEGIN_PROPERTY(Printer_FirstPage)

	int from, to;
	
	PRINTER->getPrintPages(&from, &to);

	if (READ_PROPERTY)
		GB.ReturnInteger(from + 1);
	else
		PRINTER->setPrintPages(VPROP(GB_INTEGER) - 1, to);

END_PROPERTY

BEGIN_PROPERTY(Printer_LastPage)

	int from, to;
	
	PRINTER->getPrintPages(&from, &to);

	if (READ_PROPERTY)
		GB.ReturnInteger(to + 1);
	else
		PRINTER->setPrintPages(from, VPROP(GB_INTEGER) - 1);

END_PROPERTY

BEGIN_PROPERTY(Printer_FullPage)

	if (READ_PROPERTY)
		GB.ReturnBoolean(PRINTER->useFullPage());
	else
		PRINTER->setUseFullPage(VPROP(GB_BOOLEAN));

END_PROPERTY

BEGIN_PROPERTY(Printer_OutputFile)

	if (READ_PROPERTY)
		GB.ReturnNewZeroString(PRINTER->outputFileName());
	else
		PRINTER->setOutputFileName(GB.FileName(PSTRING(), PLENGTH()));

END_PROPERTY

static bool find_default_printer(const char *name, bool def)
{
	if (def)
	{
		GB.ReturnNewZeroString(name);
		return true;
	}
	else
		return false;
}

BEGIN_PROPERTY(Printer_Default)

	GB.ReturnNull();
	gPrinter::enumeratePrinters(find_default_printer);

END_PROPERTY

static GB_ARRAY _list = NULL;

static bool add_printer(const char *name, bool def)
{
	*((char **)GB.Array.Add(_list)) = GB.NewZeroString(name);
	return FALSE;
}

BEGIN_PROPERTY(Printer_List)

	GB_ARRAY array;

	GB.Array.New(&array, GB_T_STRING, 0);
	_list = array;
	gPrinter::enumeratePrinters(add_printer);
	_list = NULL;
	GB.ReturnObject(array);

END_PROPERTY



GB_DESC PrinterDesc[] =
{
  GB_DECLARE("Printer", sizeof(CPRINTER)),

  GB_STATIC_PROPERTY_READ("Default", "s", Printer_Default),
  GB_STATIC_PROPERTY_READ("List", "String[]", Printer_List),

	GB_CONSTANT("Portrait", "i", GB_PRINT_PORTRAIT),
	GB_CONSTANT("Landscape", "i", GB_PRINT_LANDSCAPE),
	//GB_CONSTANT("ReversePortrait", "i", GB_PRINT_REVERSE_PORTRAIT),
	//GB_CONSTANT("ReverseLandscape", "i", GB_PRINT_REVERSE_LANDSCAPE),

	GB_CONSTANT("Custom", "i", GB_PRINT_CUSTOM),
	GB_CONSTANT("A3", "i", GB_PRINT_A3),
	GB_CONSTANT("A4", "i", GB_PRINT_A4),
	GB_CONSTANT("A5", "i", GB_PRINT_A5),
	GB_CONSTANT("B5", "i", GB_PRINT_B5),
	GB_CONSTANT("Letter", "i", GB_PRINT_LETTER),
	GB_CONSTANT("Executive", "i", GB_PRINT_EXECUTIVE),
	GB_CONSTANT("Legal", "i", GB_PRINT_LEGAL),

	GB_CONSTANT("Simplex", "i", GB_PRINT_SIMPLEX),
	GB_CONSTANT("Horizontal", "i", GB_PRINT_DUPLEX_HORIZONTAL),
	GB_CONSTANT("Vertical", "i", GB_PRINT_DUPLEX_VERTICAL),

	GB_METHOD("_new", NULL, Printer_new, NULL),
	GB_METHOD("_free", NULL, Printer_free, NULL),
	
	GB_METHOD("Configure", "b", Printer_Configure, NULL),
	GB_METHOD("Print", "b", Printer_Print, NULL),
	GB_METHOD("Cancel", NULL, Printer_Cancel, NULL),
	
	GB_PROPERTY("Count", "i", Printer_Count),
	GB_PROPERTY_READ("Page", "i", Printer_Current),
	
	GB_PROPERTY("Name", "s", Printer_Name),
	GB_PROPERTY("Orientation", "i", Printer_Orientation),
	GB_PROPERTY("Paper", "i", Printer_Paper),
	GB_PROPERTY("PaperWidth", "f", Printer_PaperWidth),
	GB_PROPERTY("PaperHeight", "f", Printer_PaperHeight),
	GB_PROPERTY("CollateCopies", "b", Printer_CollateCopies),
	GB_PROPERTY("ReverseOrder", "b", Printer_ReverseOrder),
	GB_PROPERTY("Duplex", "i", Printer_Duplex),
	GB_PROPERTY("GrayScale", "b", Printer_GrayScale),
	GB_PROPERTY("NumCopies", "i", Printer_NumCopies),
	GB_PROPERTY("Resolution", "i", Printer_Resolution),
	GB_PROPERTY("FirstPage", "i", Printer_FirstPage),
	GB_PROPERTY("LastPage", "i", Printer_LastPage),
	GB_PROPERTY("FullPage", "b", Printer_FullPage),
	GB_PROPERTY("OutputFile", "s", Printer_OutputFile),
	
	GB_EVENT("Begin", NULL, NULL, &EVENT_Begin),
	GB_EVENT("End", NULL, NULL, &EVENT_End),
	GB_EVENT("Paginate", NULL, NULL, &EVENT_Paginate),
	GB_EVENT("Draw", NULL, NULL, &EVENT_Draw),

  GB_INTERFACE("Paint", &PAINT_Interface),

	PRINTER_DESCRIPTION,

  GB_END_DECLARE
};

