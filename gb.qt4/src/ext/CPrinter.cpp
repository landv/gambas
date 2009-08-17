/***************************************************************************

  CPrinter.cpp

  (c) 2000-2009 Benoît Minisini <gambas@users.sourceforge.net>

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

#include <qapplication.h>
#include <qpainter.h>
#include <qprinter.h>
#include <q3paintdevicemetrics.h>

#include "CDraw.h"
#include "CPrinter.h"

QPrinter *CPRINTER_printer = NULL;

static PRINTER_SIZE _size[] =
{
  { "A0", QPrinter::A0 },
  { "A1", QPrinter::A1 },
  { "A2", QPrinter::A2 },
  { "A3", QPrinter::A3 },
  { "A4", QPrinter::A4 },
  { "A5", QPrinter::A5 },
  { "A6", QPrinter::A6 },
  { "A7", QPrinter::A7 },
  { "A8", QPrinter::A8 },
  { "A9", QPrinter::A9 },
  { "B0", QPrinter::B0 },
  { "B1", QPrinter::B1 },
  { "B2", QPrinter::B2 },
  { "B3", QPrinter::B3 },
  { "B4", QPrinter::B4 },
  { "B5", QPrinter::B5 },
  { "B6", QPrinter::B6 },
  { "B7", QPrinter::B7 },
  { "B8", QPrinter::B8 },
  { "B9", QPrinter::B9 },
  { "B10", QPrinter::B10 },
  { "C5E", QPrinter::C5E },
  { "DLE", QPrinter::DLE },
  { "Comm10E", QPrinter::Comm10E },
  { "Letter", QPrinter::Letter },
  { "Legal", QPrinter::Legal },
  { "Executive", QPrinter::Executive },
  { "Folio", QPrinter::Folio },
  { "Ledger", QPrinter::Ledger },
  { "Tabloid", QPrinter::Tabloid },
  { 0 }
};


/***************************************************************************

  Printer

***************************************************************************/

void CPRINTER_init(void)
{
  if (!PRINTER)
  {
    #if QT_VERSION >= 0x030100
    PRINTER = new QPrinter(QPrinter::HighResolution);
    PRINTER->setFullPage(true);
    PRINTER->setColorMode(QPrinter::Color);
    #else
    PRINTER = new QPrinter();
    PRINTER->setFullPage(true);
    #endif
  }
}


BEGIN_METHOD_VOID(CPRINTER_exit)

  if (PRINTER)
  {
    delete PRINTER;
    PRINTER = NULL;
  }

END_METHOD


BEGIN_METHOD_VOID(CPRINTER_new_page)

  CPRINTER_init();
  PRINTER->newPage();

END_METHOD


BEGIN_METHOD_VOID(CPRINTER_abort)

  CPRINTER_init();
  PRINTER->abort();

END_METHOD


BEGIN_METHOD_VOID(CPRINTER_setup)

  CPRINTER_init();
  GB.ReturnBoolean(!PRINTER->setup());

END_METHOD


BEGIN_PROPERTY(CPRINTER_size)

  PRINTER_SIZE *ps;

  CPRINTER_init();

  if (READ_PROPERTY)
  {
    for (ps = _size; ps->paper; ps++)
    {
      if (ps->value == PRINTER->pageSize())
      {
        GB.ReturnConstZeroString(ps->paper);
        return;
      }
    }
    GB.ReturnNull();
  }
  else
  {
    char *paper = GB.ToZeroString(PROP(GB_STRING));

    for (ps = _size; ps->paper; ps++)
    {
      if (strcasecmp(ps->paper, paper) == 0)
      {
        PRINTER->setPageSize(ps->value);
        break;
      }
    }
  }

END_PROPERTY


BEGIN_PROPERTY(CPRINTER_orientation)

  CPRINTER_init();

  if (READ_PROPERTY)
    GB.ReturnInteger(PRINTER->orientation());
  else
    PRINTER->setOrientation((QPrinter::Orientation)VPROP(GB_INTEGER));

END_PROPERTY


BEGIN_PROPERTY(CPRINTER_width)

  CPRINTER_init();

  Q3PaintDeviceMetrics pdm(PRINTER);

  GB.ReturnInteger(pdm.width());

END_PROPERTY


BEGIN_PROPERTY(CPRINTER_height)

  CPRINTER_init();

  Q3PaintDeviceMetrics pdm(PRINTER);

  GB.ReturnInteger(pdm.height());

END_PROPERTY


BEGIN_PROPERTY(CPRINTER_name)

  CPRINTER_init();

  if (READ_PROPERTY)
    GB.ReturnNewZeroString(TO_UTF8(PRINTER->printerName()));
  else
    PRINTER->setPrinterName(QSTRING_PROP());

END_PROPERTY


BEGIN_PROPERTY(CPRINTER_file)

  CPRINTER_init();

  if (READ_PROPERTY)
    GB.ReturnNewZeroString(TO_UTF8(PRINTER->outputFileName()));
  else
    PRINTER->setOutputFileName(QSTRING_PROP());

END_PROPERTY


BEGIN_PROPERTY(CPRINTER_resolution)

  CPRINTER_init();

  if (READ_PROPERTY)
    GB.ReturnInteger(PRINTER->resolution());
  else
    PRINTER->setResolution(VPROP(GB_INTEGER));

END_PROPERTY


BEGIN_PROPERTY(CPRINTER_color_mode)

  CPRINTER_init();

  if (READ_PROPERTY)
    GB.ReturnInteger(PRINTER->colorMode());
  else
    PRINTER->setColorMode((QPrinter::ColorMode)VPROP(GB_INTEGER));

END_PROPERTY


BEGIN_PROPERTY(CPRINTER_min_page)

  CPRINTER_init();

  if (READ_PROPERTY)
    GB.ReturnInteger(PRINTER->minPage());
  else
    PRINTER->setMinMax(VPROP(GB_INTEGER), PRINTER->maxPage());

END_PROPERTY


BEGIN_PROPERTY(CPRINTER_max_page)

  CPRINTER_init();

  if (READ_PROPERTY)
    GB.ReturnInteger(PRINTER->maxPage());
  else
    PRINTER->setMinMax(PRINTER->minPage(), VPROP(GB_INTEGER));

END_PROPERTY


BEGIN_PROPERTY(CPRINTER_from_page)

  CPRINTER_init();

  if (READ_PROPERTY)
    GB.ReturnInteger(PRINTER->fromPage());
  else
    PRINTER->setFromTo(VPROP(GB_INTEGER), PRINTER->toPage());

END_PROPERTY


BEGIN_PROPERTY(CPRINTER_to_page)

  CPRINTER_init();

  if (READ_PROPERTY)
    GB.ReturnInteger(PRINTER->toPage());
  else
    PRINTER->setFromTo(PRINTER->fromPage(), VPROP(GB_INTEGER));

END_PROPERTY


BEGIN_PROPERTY(CPRINTER_copies)

  CPRINTER_init();

  if (READ_PROPERTY)
    GB.ReturnInteger(PRINTER->numCopies());
  else
    PRINTER->setNumCopies(VPROP(GB_INTEGER));

END_PROPERTY



GB_DESC CPrinterDesc[] =
{
  GB_DECLARE("Printer", 0), GB_VIRTUAL_CLASS(),

  //GB_STATIC_METHOD("_init", NULL, CPRINTER_init, NULL),
  GB_STATIC_METHOD("_exit", NULL, CPRINTER_exit, NULL),

  GB_STATIC_METHOD("NewPage", NULL, CPRINTER_new_page, NULL),
  GB_STATIC_METHOD("Abort", NULL, CPRINTER_abort, NULL),
  GB_STATIC_METHOD("Setup", "b", CPRINTER_setup, NULL),

  GB_STATIC_PROPERTY("Name", "s", CPRINTER_name),
  GB_STATIC_PROPERTY("File", "s", CPRINTER_file),
  GB_STATIC_PROPERTY("Resolution", "i", CPRINTER_resolution),
  GB_STATIC_PROPERTY("ColorMode", "i", CPRINTER_color_mode),

  GB_STATIC_PROPERTY("Size", "s", CPRINTER_size),
  GB_STATIC_PROPERTY("Orientation", "i", CPRINTER_orientation),

  GB_STATIC_PROPERTY("MaxPage", "i", CPRINTER_max_page),
  GB_STATIC_PROPERTY("MinPage", "i", CPRINTER_min_page),
  GB_STATIC_PROPERTY("FromPage", "i", CPRINTER_from_page),
  GB_STATIC_PROPERTY("ToPage", "i", CPRINTER_to_page),
  GB_STATIC_PROPERTY("Copies", "i", CPRINTER_copies),

  GB_STATIC_PROPERTY_READ("Width", "i", CPRINTER_width),
  GB_STATIC_PROPERTY_READ("Height", "i", CPRINTER_height),

  GB_CONSTANT("Portrait", "i", QPrinter::Portrait),
  GB_CONSTANT("Landscape", "i", QPrinter::Landscape),

  GB_CONSTANT("Color", "i", QPrinter::Color),
  GB_CONSTANT("Black", "i", QPrinter::GrayScale),

	GB_INTERFACE("StaticDraw", &DRAW_Interface),

  GB_END_DECLARE
};


