/***************************************************************************

  gprinter.h

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

#ifndef __GPRINTER_H
#define __GPRINTER_H

class gPrinter
{
public:
	gPrinter();
	virtual ~gPrinter();
	void *tag;

	bool run(bool configure);
	void cancel();
	
	void setPageCount(int v);
	int pageCount() const { return _page_count; }
	bool isPageCountSet() const { return _page_count_set; }
	
// Signals

	void (*onBegin)(gPrinter *me);
	void (*onEnd)(gPrinter *me);
	void (*onDraw)(gPrinter *me, GtkPrintContext *context, int page);
	void (*onPaginate)(gPrinter *me);
	
private:
	GtkPrintOperation *_operation;
	GtkPrintSettings *_settings;
	GtkPageSetup *_page;
	int _page_count;
	bool _page_count_set;
};

#endif
