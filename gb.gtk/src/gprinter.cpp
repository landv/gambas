/***************************************************************************

  gprinter.cpp

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

#define __GPRINTER_CPP

#include "widgets.h"
#include "gdesktop.h"
#include "gmainwindow.h"
#include "gb.form.print.h"
#include "gprinter.h"

static void cb_begin_cancel(GtkPrintOperation *operation, GtkPrintContext *context, gPrinter *printer)
{
	printer->cancel();
}

static void cb_begin(GtkPrintOperation *operation, GtkPrintContext *context, gPrinter *printer)
{
	if (printer->onBegin)
		(*printer->onBegin)(printer, context);
}

static void cb_end(GtkPrintOperation *operation, GtkPrintContext *context, gPrinter *printer)
{
	if (printer->onEnd)
		(*printer->onEnd)(printer);
}

static bool cb_paginate(GtkPrintOperation *operation, GtkPrintContext *context, gPrinter *printer)
{
	if (printer->onPaginate)
	{
		(*printer->onPaginate)(printer);
		return printer->isPageCountSet();
	}
	else
		return TRUE;
}

static void cb_draw(GtkPrintOperation *operation, GtkPrintContext *context, int page, gPrinter *printer)
{
	if (printer->onDraw)
		(*printer->onDraw)(printer, context, page);
}

gPrinter::gPrinter()
{
	_operation = NULL;
	_settings = gtk_print_settings_new();
	_page = gtk_page_setup_new();
	_page_count = 1;
	_page_count_set = false;
	
	setPaperModel(GB_PRINT_A4);
	setUseFullPage(false);
	
	onBegin = NULL;
	onEnd = NULL;
	onDraw = NULL;
	onPaginate = NULL;
}

gPrinter::~gPrinter()
{
	g_object_unref(G_OBJECT(_settings));
	g_object_unref(G_OBJECT(_page));
}

bool gPrinter::run(bool configure)
{
	GtkPrintOperation *operation;
  GtkPrintOperationResult res;
	gMainWindow *active;
	GError *error;
	
  operation = gtk_print_operation_new();
	if (!configure)
		_operation = operation;
	
	gtk_print_operation_set_n_pages(operation, _page_count);
	gtk_print_operation_set_use_full_page(operation, _use_full_page);
  
	gtk_print_operation_set_print_settings(operation, _settings);
	//gtk_print_operation_set_default_page_setup(_operation, _page);
  
	if (configure)
	{
		g_signal_connect(operation, "begin_print", G_CALLBACK(cb_begin_cancel), this);
	}
	else
	{
		g_signal_connect(operation, "begin_print", G_CALLBACK(cb_begin), this);
		g_signal_connect(operation, "end_print", G_CALLBACK(cb_end), this);
		g_signal_connect(operation, "paginate", G_CALLBACK(cb_paginate), this);
		g_signal_connect(operation, "draw_page", G_CALLBACK(cb_draw), this);
	}
	
	active = gDesktop::activeWindow();
	
  res = gtk_print_operation_run(operation, 
		configure ? GTK_PRINT_OPERATION_ACTION_PRINT_DIALOG : GTK_PRINT_OPERATION_ACTION_PRINT,	
		active ? GTK_WINDOW(active->border) : NULL, &error);
	
	if (res == GTK_PRINT_OPERATION_RESULT_ERROR)
	{
		g_error_free(error);
	}
	else if (res == GTK_PRINT_OPERATION_RESULT_APPLY)
	{
		g_object_unref(G_OBJECT(_settings));
		_settings = GTK_PRINT_SETTINGS(g_object_ref(gtk_print_operation_get_print_settings(operation)));
	}

	g_object_unref(G_OBJECT(operation));
	
	if (!configure)
	{
		_operation = NULL;
		_page_count_set = false;
	}
	//else
	//	gtk_print_settings_to_file(_settings, "/home/benoit/settings.txt", NULL);
	
	return res != GTK_PRINT_OPERATION_RESULT_APPLY;
}

void gPrinter::cancel()
{
	if (!_operation)
		return;
	
	gtk_print_operation_cancel(_operation);
}

void gPrinter::setPageCount(int v)
{
	if (v < 1 || v > 32767)
		return;
	
	_page_count = v;
	_page_count_set = true;
	if (_operation)
		gtk_print_operation_set_n_pages(_operation, _page_count);
}

int gPrinter::orientation() const
{
	switch(gtk_print_settings_get_orientation(_settings))
	{
		case GTK_PAGE_ORIENTATION_PORTRAIT: return GB_PRINT_PORTRAIT;
		case GTK_PAGE_ORIENTATION_LANDSCAPE: return GB_PRINT_LANDSCAPE;
		case GTK_PAGE_ORIENTATION_REVERSE_PORTRAIT: return GB_PRINT_PORTRAIT;
		case GTK_PAGE_ORIENTATION_REVERSE_LANDSCAPE: return GB_PRINT_LANDSCAPE;
	}
}

void gPrinter::setOrientation(int v)
{
	GtkPageOrientation orient;
	
	switch(v)
	{
		case GB_PRINT_LANDSCAPE: orient = GTK_PAGE_ORIENTATION_LANDSCAPE; break;
		//case GB_PRINT_REVERSE_PORTRAIT: orient = GTK_PAGE_ORIENTATION_REVERSE_PORTRAIT; break;
		//case GB_PRINT_REVERSE_LANDSCAPE: orient = GTK_PAGE_ORIENTATION_REVERSE_LANDSCAPE; break;	
		case GB_PRINT_PORTRAIT: default: orient = GTK_PAGE_ORIENTATION_PORTRAIT; break;
	}
	
	gtk_print_settings_set_orientation(_settings, orient);
}

void gPrinter::setPaperModel(int v)
{
	GtkPaperSize *paper;
	const char *name;
	
	switch(v)
	{
		case GB_PRINT_A3: name = GTK_PAPER_NAME_A3; break;
		case GB_PRINT_A4: name = GTK_PAPER_NAME_A4; break;
		case GB_PRINT_A5: name = GTK_PAPER_NAME_A5; break;
		case GB_PRINT_B5: name = GTK_PAPER_NAME_B5; break;
		case GB_PRINT_LETTER: name = GTK_PAPER_NAME_LETTER; break;
		case GB_PRINT_EXECUTIVE: name = GTK_PAPER_NAME_EXECUTIVE; break;
		case GB_PRINT_LEGAL: name = GTK_PAPER_NAME_LEGAL; break;
		default: name = GTK_PAPER_NAME_A4; v = GB_PRINT_A4;
	}
	
	_paper_size = v;
	
	paper = gtk_paper_size_new(name);
	gtk_print_settings_set_paper_size(_settings, paper);
	gtk_paper_size_free(paper);
}

void gPrinter::getPaperSize(double *width, double *height) const
{
	*width = gtk_print_settings_get_paper_width(_settings, GTK_UNIT_MM);
	*height = gtk_print_settings_get_paper_height(_settings, GTK_UNIT_MM);
}

void gPrinter::setPaperSize(double width, double height)
{
	_paper_size = GB_PRINT_CUSTOM;
	gtk_print_settings_set_paper_width(_settings, width, GTK_UNIT_MM);
	gtk_print_settings_set_paper_height(_settings, height, GTK_UNIT_MM);
}

bool gPrinter::collateCopies() const
{
	return gtk_print_settings_get_collate(_settings);
}

void gPrinter::setCollateCopies(bool v)
{
	gtk_print_settings_set_collate(_settings, v);
}

bool gPrinter::reverserOrder() const
{
	return gtk_print_settings_get_reverse(_settings);
}

void gPrinter::setReverseOrder(bool v)
{
	gtk_print_settings_set_reverse(_settings, v);
}

int gPrinter::duplex() const
{
	switch (gtk_print_settings_get_duplex(_settings))
	{
		case GTK_PRINT_DUPLEX_SIMPLEX: return GB_PRINT_SIMPLEX;
		case GTK_PRINT_DUPLEX_HORIZONTAL: return GB_PRINT_DUPLEX_HORIZONTAL;
		case GTK_PRINT_DUPLEX_VERTICAL: return GB_PRINT_DUPLEX_VERTICAL;
		default: return GB_PRINT_SIMPLEX;
	}
}

void gPrinter::setDuplex(int v)
{
	GtkPrintDuplex duplex;
	
	switch(v)
	{
		case GB_PRINT_SIMPLEX: duplex = GTK_PRINT_DUPLEX_SIMPLEX; break;
		case GB_PRINT_DUPLEX_HORIZONTAL: duplex = GTK_PRINT_DUPLEX_HORIZONTAL; break;
		case GB_PRINT_DUPLEX_VERTICAL: duplex = GTK_PRINT_DUPLEX_VERTICAL; break;
		default:  duplex = GTK_PRINT_DUPLEX_SIMPLEX; break;
	}
	
	gtk_print_settings_set_duplex(_settings, duplex);
}
	
bool gPrinter::useColor() const
{
	return gtk_print_settings_get_use_color(_settings);
}

void gPrinter::setUseColor(bool v)
{
	gtk_print_settings_set_use_color(_settings, v);
}
	
int gPrinter::numCopies() const
{
	return gtk_print_settings_get_n_copies(_settings);
}

void gPrinter::setNumCopies(int v)
{
	gtk_print_settings_set_n_copies(_settings, v);
}
	
int gPrinter::resolution() const
{
	return gtk_print_settings_get_resolution(_settings);
}

void gPrinter::setResolution(int v)
{
	gtk_print_settings_set_resolution(_settings, v);
}
	
void gPrinter::getPrintPages(int *from, int *to) const
{
	GtkPageRange *range;
	int nrange;
	
	range = gtk_print_settings_get_page_ranges(_settings, &nrange);
	
	if (nrange <= 0)
		*from = *to = -1;
	else
	{
		*from = range->start;
		*to = range->end;
		g_free(range);
	}
}

void gPrinter::setPrintPages(int from, int to)
{
	GtkPageRange range = { from, to };
	
	gtk_print_settings_set_page_ranges(_settings, &range, 1);
	if (from < 0)
		gtk_print_settings_set_print_pages(_settings, GTK_PRINT_PAGES_ALL);
	else
		gtk_print_settings_set_print_pages(_settings, GTK_PRINT_PAGES_RANGES);
}

void gPrinter::setUseFullPage(bool v)
{
	_use_full_page = v;
	if (_operation)
		gtk_print_operation_set_use_full_page(_operation, v);
}

const char *gPrinter::name() const
{
	return gtk_print_settings_get_printer(_settings);
}

void gPrinter::setName(const char *name)
{
	gtk_print_settings_set_printer(_settings, name);
}

const char *gPrinter::outputFileName() const
{
	const char *uri;
	
	uri = gtk_print_settings_get(_settings, "output-uri");
	
	if (!uri)
		return NULL;
	
	if (strncmp(uri, "file://", 7))
		return NULL;
	
	return &uri[7];
}

void gPrinter::setOutputFileName(const char *file)
{
	char uri[strlen(file) + 7];
	const char *format;
	
	strcpy(uri, "file://");
	strcat(uri, file);
	
	gtk_print_settings_set(_settings, GTK_PRINT_SETTINGS_OUTPUT_URI, uri);	

	if (g_str_has_suffix(uri, ".ps"))
		format = "ps";
	else if (g_str_has_suffix(uri, ".pdf"))
		format = "pdf";
	else if (g_str_has_suffix(uri, ".svg"))
		format = "svg";
	else
		format = NULL;

	// It does not work!!!
	// if (format)
	//	 gtk_print_settings_set(_settings, GTK_PRINT_SETTINGS_OUTPUT_FILE_FORMAT, format);
}
