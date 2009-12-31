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
#include "gprinter.h"

static void cb_begin(GtkPrintOperation *operation, GtkPrintContext *context, gPrinter *printer)
{
	if (printer->onBegin)
		(*printer->onBegin)(printer);
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
  GtkPrintOperationResult res;
	gMainWindow *active;
	GError *error;
	
  _operation = gtk_print_operation_new();
	gtk_print_operation_set_n_pages(_operation, _page_count);
  
	gtk_print_operation_set_print_settings(_operation, _settings);
	gtk_print_operation_set_default_page_setup(_operation, _page);
  
	g_signal_connect(_operation, "begin_print", G_CALLBACK(cb_begin), this);
	g_signal_connect(_operation, "end_print", G_CALLBACK(cb_end), this);
  g_signal_connect(_operation, "paginate", G_CALLBACK(cb_paginate), this);
  g_signal_connect(_operation, "draw_page", G_CALLBACK(cb_draw), this);
	
	active = gDesktop::activeWindow();
	
  res = gtk_print_operation_run(_operation, configure ? GTK_PRINT_OPERATION_ACTION_PRINT_DIALOG : GTK_PRINT_OPERATION_ACTION_PRINT,
		active ? GTK_WINDOW(active->border) : NULL, &error);
	
	if (res == GTK_PRINT_OPERATION_RESULT_ERROR)
	{
		g_error_free(error);
	}
	else if (res == GTK_PRINT_OPERATION_RESULT_APPLY)
	{
		g_object_unref(G_OBJECT(_settings));
		_settings = GTK_PRINT_SETTINGS(g_object_ref(gtk_print_operation_get_print_settings(_operation)));
	}

	g_object_unref(G_OBJECT(_operation));
	_page_count_set = false;
	
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


