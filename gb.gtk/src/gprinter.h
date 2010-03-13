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

	bool configure() { return run(true); }
	bool print() { return run(false); }
	void cancel();
	
	void setPageCount(int v);
	int pageCount() const { return _page_count; }
	bool isPageCountSet() const { return _page_count_set; }
	
	int orientation() const;
	void setOrientation(int v);
	
	int paperModel() const { return _paper_size; }
	void setPaperModel(int v);
	
	void getPaperSize(double *width, double *height) const;
	void setPaperSize(double width, double height);
	
	bool collateCopies() const;
	void setCollateCopies(bool v);
	
	bool reverserOrder() const;
	void setReverseOrder(bool v);
	
	int duplex() const;
	void setDuplex(int v);
	
	bool useColor() const;
	void setUseColor(bool v);
	
	int numCopies() const;
	void setNumCopies(int v);
	
	int resolution() const;
	void setResolution(int v);
	
	void getPrintPages(int *from, int *to) const;
	void setPrintPages(int from, int to);
	
	bool useFullPage() const { return _use_full_page; }
	void setUseFullPage(bool v);
	
	const char *name() const;
	void setName(const char *name);
	
	const char *outputFileName() const;
	void setOutputFileName(const char *file);
	
// Signals

	void (*onBegin)(gPrinter *me, GtkPrintContext *context);
	void (*onEnd)(gPrinter *me);
	void (*onDraw)(gPrinter *me, GtkPrintContext *context, int page);
	void (*onPaginate)(gPrinter *me);
	
private:
	bool run(bool configure);
	
	GtkPrintOperation *_operation;
	GtkPrintSettings *_settings;
	GtkPageSetup *_page;
	int _page_count;
	bool _page_count_set;
	int _paper_size;
	bool _use_full_page;
};

#endif
