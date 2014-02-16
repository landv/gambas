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
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
  MA 02110-1301, USA.

***************************************************************************/

#define __GPRINTER_CPP

#include <unistd.h>

#include "widgets.h"
#include "gdesktop.h"
#include "gmainwindow.h"
#include "gapplication.h"
#include "gb.form.print.h"
#include "gprinter.h"

//#define DEBUG_ME 1

static void cb_begin_cancel(GtkPrintOperation *operation, GtkPrintContext *context, gPrinter *printer)
{
	if (printer->_preview)
	{
		if (printer->onBegin)
			(*printer->onBegin)(printer, context);
		return;
	}
	
	#if DEBUG_ME
	fprintf(stderr, "cb_begin_cancel: %d\n", cairo_surface_get_type(cairo_get_target(gtk_print_context_get_cairo_context(context))));
	#endif
	printer->storeSettings();
	printer->cancel();
	printer->_configure_ok = true;
}

static void cb_begin(GtkPrintOperation *operation, GtkPrintContext *context, gPrinter *printer)
{
	#if DEBUG_ME
	fprintf(stderr, "cb_begin\n");
	#endif
	printer->defineSettings();
	//gtk_print_settings_to_file(gtk_print_operation_get_print_settings(operation), "/home/benoit/settings-begin-before.txt", NULL);
	if (printer->onBegin)
		(*printer->onBegin)(printer, context);
	//gtk_print_settings_to_file(gtk_print_operation_get_print_settings(operation), "/home/benoit/settings-begin-after.txt", NULL);
}

static void cb_end(GtkPrintOperation *operation, GtkPrintContext *context, gPrinter *printer)
{
	#if DEBUG_ME
	fprintf(stderr, "cb_end: %d\n", printer->_preview);
	#endif
	if (printer->_preview && printer->onEnd)
		(*printer->onEnd)(printer);
}

static gboolean cb_paginate(GtkPrintOperation *operation, GtkPrintContext *context, gPrinter *printer)
{
	#if DEBUG_ME
	fprintf(stderr, "cb_paginate\n");
	#endif
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
	#if DEBUG_ME
	fprintf(stderr, "cb_draw\n");
	#endif
	if (printer->onDraw)
		(*printer->onDraw)(printer, context, page);
}

static gboolean cb_preview(GtkPrintOperation *operation, GtkPrintOperationPreview *preview,GtkPrintContext *context, GtkWindow *parent, gPrinter *printer)
{
	#if DEBUG_ME
	fprintf(stderr, "cb_preview\n");
	#endif
	printer->_preview = true;
	return FALSE;
}


static gboolean find_default_printer(GtkPrinter *gtk_printer, gPrinter *printer)
{
	if (!printer->name())
		printer->setName(gtk_printer_get_name(gtk_printer));
	
	if (gtk_printer_is_default(gtk_printer))
	{
		#if DEBUG_ME
		fprintf(stderr, "find_default_printer: %s\n", gtk_printer_get_name(gtk_printer));
		#endif
	
		printer->setName(gtk_printer_get_name(gtk_printer));
		return TRUE;
	}
	
	return FALSE;
}

static gboolean find_file_printer(GtkPrinter *gtk_printer, gPrinter *printer)
{
	if (!strcmp(G_OBJECT_TYPE_NAME(gtk_printer_get_backend(gtk_printer)), "GtkPrintBackendFile"))
	{
		#if DEBUG_ME
		fprintf(stderr, "find_file_printer: %s\n", gtk_printer_get_name(gtk_printer));
		#endif

		printer->setName(gtk_printer_get_name(gtk_printer));
		return TRUE;
	}
		
	return FALSE;
}

gPrinter *gPrinter::_current = NULL;

gPrinter::gPrinter()
{
	_operation = NULL;
	_settings = gtk_print_settings_new();
	_page = gtk_page_setup_new();
	_page_count = 1;
	_page_count_set = false;
	
	gtk_enumerate_printers((GtkPrinterFunc)find_default_printer, this, NULL, TRUE);

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

void gPrinter::storeSettings()
{
	if (!_operation)
		return;
	
	g_object_unref(G_OBJECT(_settings));
	_settings = gtk_print_settings_copy(gtk_print_operation_get_print_settings(_operation));
	#if DEBUG_ME
	gtk_print_settings_to_file(_settings, "/home/benoit/settings.txt", NULL);
	gtk_page_setup_to_file(_page, "/home/benoit/page.txt", NULL);
	#endif
}

void gPrinter::defineSettings()
{
	if (!_operation)
		return;
	
	gtk_print_operation_set_print_settings(_operation, _settings);
	gtk_print_operation_set_default_page_setup(_operation, _page);
}

bool gPrinter::run(bool configure)
{
	GtkPrintOperation *operation;
  GtkPrintOperationResult res;
	GtkPrintOperationAction action;
	gMainWindow *active;
	GError *error;
	const char *file;
	
	#if DEBUG_ME
	fprintf(stderr, "gPrinter::run: %d\n", configure);
	fprintf(stderr, "orientation = %d\n", orientation());
	#endif

	operation = gtk_print_operation_new();
	_operation = operation;
	
	gtk_print_operation_set_embed_page_setup(operation, true);
	gtk_print_operation_set_n_pages(operation, _page_count);
	gtk_print_operation_set_use_full_page(operation, _use_full_page);
	gtk_print_operation_set_print_settings(operation, _settings);
	gtk_print_operation_set_default_page_setup(_operation, _page);
  
	if (configure)
	{
		_preview = false;
		_configure_ok = false;
		g_signal_connect(operation, "begin_print", G_CALLBACK(cb_begin_cancel), this);
		g_signal_connect(operation, "preview", G_CALLBACK(cb_preview), this);
		g_signal_connect(operation, "end_print", G_CALLBACK(cb_end), this);
		g_signal_connect(operation, "paginate", G_CALLBACK(cb_paginate), this);
		g_signal_connect(operation, "draw_page", G_CALLBACK(cb_draw), this);
	}
	else
	{
		_preview = true;
		g_signal_connect(operation, "begin_print", G_CALLBACK(cb_begin), this);
		g_signal_connect(operation, "end_print", G_CALLBACK(cb_end), this);
		g_signal_connect(operation, "paginate", G_CALLBACK(cb_paginate), this);
		g_signal_connect(operation, "draw_page", G_CALLBACK(cb_draw), this);
	}
	
	active = gDesktop::activeWindow();
	
	if (isVirtual())
	{
		_current = this;
		gApplication::_fix_printer_dialog = true;
	}
	else
		gApplication::_fix_printer_dialog = false;
	
	action = GTK_PRINT_OPERATION_ACTION_PRINT_DIALOG;
	if (!configure)
	{
		file = outputFileName();
		if (file) 
		{
			unlink(file);
			setOutputFileName(outputFileName());
			defineSettings();
		}
		// GTK+ bug: GTK_PRINT_OPERATION_ACTION_PRINT does not work with virtual printers.
		if (isVirtual())
		{
			gApplication::_close_next_window = true;
		}
		else
			action = GTK_PRINT_OPERATION_ACTION_PRINT;
	}
	
	//gtk_print_settings_to_file(gtk_print_operation_get_print_settings(operation), "/home/benoit/settings-before.txt", NULL);
	res = gtk_print_operation_run(operation, action, active ? GTK_WINDOW(active->border) : NULL, &error);
	_current = NULL;
	//gtk_print_settings_to_file(gtk_print_operation_get_print_settings(operation), "/home/benoit/settings-after.txt", NULL);

	#if DEBUG_ME
	fprintf(stderr, "_preview = %d\n", _preview);
	#endif

	if (_preview)
	{
		_preview = false;
		res = GTK_PRINT_OPERATION_RESULT_CANCEL;
	}
	else if (_configure_ok)
		res = GTK_PRINT_OPERATION_RESULT_APPLY;

	if (res == GTK_PRINT_OPERATION_RESULT_ERROR)
	{
		#if DEBUG_ME
		fprintf(stderr, "error: %s\n", error->message);
		#endif
		g_error_free(error);
	}
	else if (res == GTK_PRINT_OPERATION_RESULT_CANCEL)
	{
		#if DEBUG_ME
		fprintf(stderr, "cancel\n");
		#endif
	}
	else
	{
		#if DEBUG_ME
		fprintf(stderr, "ok\n");
		#endif
	}

	if (!configure)
		_page_count_set = false;
	
	if (configure && res == GTK_PRINT_OPERATION_RESULT_APPLY)
	{
		g_object_unref(G_OBJECT(_page));
		_page = gtk_page_setup_copy(gtk_print_operation_get_default_page_setup(operation));
	}
	
	g_object_unref(G_OBJECT(operation));
	_operation = NULL;
	
	#if DEBUG_ME
	fprintf(stderr, "orientation => %d\n", orientation());
	#endif
	
	return res != GTK_PRINT_OPERATION_RESULT_APPLY;
}

void gPrinter::cancel()
{
	if (!_operation)
		return;
	
	#if DEBUG_ME
	fprintf(stderr, "gPrinter::cancel\n");
	#endif
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
	switch (gtk_page_setup_get_orientation(_page))
	//switch (gtk_print_settings_get_orientation(_settings))
	{
		case GTK_PAGE_ORIENTATION_LANDSCAPE: return GB_PRINT_LANDSCAPE;
		case GTK_PAGE_ORIENTATION_REVERSE_PORTRAIT: return GB_PRINT_PORTRAIT;
		case GTK_PAGE_ORIENTATION_REVERSE_LANDSCAPE: return GB_PRINT_LANDSCAPE;
		case GTK_PAGE_ORIENTATION_PORTRAIT: default: return GB_PRINT_PORTRAIT;
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
	gtk_page_setup_set_orientation(_page, orient);
}

GtkPaperSize *gPrinter::getPaperSize()
{
	const char *name;
	
	switch(_paper_size)
	{
		case GB_PRINT_A3: name = GTK_PAPER_NAME_A3; break;
		case GB_PRINT_A4: name = GTK_PAPER_NAME_A4; break;
		case GB_PRINT_A5: name = GTK_PAPER_NAME_A5; break;
		case GB_PRINT_B5: name = GTK_PAPER_NAME_B5; break;
		case GB_PRINT_LETTER: name = GTK_PAPER_NAME_LETTER; break;
		case GB_PRINT_EXECUTIVE: name = GTK_PAPER_NAME_EXECUTIVE; break;
		case GB_PRINT_LEGAL: name = GTK_PAPER_NAME_LEGAL; break;
		default: name = GTK_PAPER_NAME_A4; _paper_size = GB_PRINT_A4;
	}
	
	return gtk_paper_size_new(name);
}

void gPrinter::setPaperModel(int v)
{
	GtkPaperSize *paper;
	
	_paper_size = v;
	paper = getPaperSize();
	gtk_print_settings_set_paper_size(_settings, paper);
	gtk_page_setup_set_paper_size(_page, paper);
	gtk_paper_size_free(paper);
}

void gPrinter::getPaperSize(double *width, double *height)
{
	GtkPaperSize *paper = gtk_page_setup_get_paper_size(_page);
	
	*width = gtk_paper_size_get_width(paper, GTK_UNIT_MM);
	*height = gtk_paper_size_get_height(paper, GTK_UNIT_MM);
	
	if (orientation() == GB_PRINT_LANDSCAPE)
	{
		double swap = *width;
		*width = *height;
		*height = swap;
	}
	
#if 0
	if (_paper_size == GB_PRINT_CUSTOM)
	{
		*width = gtk_print_settings_get_paper_width(_settings, GTK_UNIT_MM);
		*height = gtk_print_settings_get_paper_height(_settings, GTK_UNIT_MM);
		//*width = gtk_page_setup_get_paper_width(_page, GTK_UNIT_MM);
		//*height = gtk_page_setup_get_paper_height(_page, GTK_UNIT_MM);
		
		// orientation is taken into account
	}
	else
	{
		GtkPaperSize *paper = getPaperSize();
		*width = gtk_paper_size_get_width(paper, GTK_UNIT_MM);
		*height = gtk_paper_size_get_height(paper, GTK_UNIT_MM);
		gtk_paper_size_free(paper);
		
	}
#endif
}

void gPrinter::setPaperSize(double width, double height)
{
	GtkPaperSize *paper;
	
	_paper_size = GB_PRINT_CUSTOM;
	
	if (orientation() == GB_PRINT_LANDSCAPE)
	{
		double swap = width;
		width = height;
		height = swap;
	}
	
	paper = gtk_paper_size_new_custom("Custom", "Custom", width, height, GTK_UNIT_MM);
	gtk_page_setup_set_paper_size(_page, paper);
	gtk_print_settings_set_paper_size(_settings, paper);
	gtk_paper_size_free(paper);

	/*if (orientation() == GB_PRINT_LANDSCAPE)
	{
		double swap = width;
		width = height;
		height = swap;
	}*/
	
	//gtk_print_settings_set_paper_width(_settings, width, GTK_UNIT_MM);
	//gtk_print_settings_set_paper_height(_settings, height, GTK_UNIT_MM);
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
	
	if (gtk_print_settings_get_print_pages(_settings) == GTK_PRINT_PAGES_ALL)
	{
		*from = *to = -1;
		return;
	}
	
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

static char *unescape_uri(const char *uri)
{
	char *path;
	
	if (!uri)
		return NULL;
	
	if (strncmp(uri, "file://", 7))
		return NULL;
	
	path = g_uri_unescape_string(&uri[7], "/");
	gt_free_later(path);
	
	return path;
}

const char *gPrinter::outputFileName() const
{
	//fprintf(stderr, "outputFileName: %s\n", gtk_print_settings_get(_settings, GTK_PRINT_SETTINGS_OUTPUT_URI));
	return unescape_uri(gtk_print_settings_get(_settings, GTK_PRINT_SETTINGS_OUTPUT_URI));
}

void gPrinter::setOutputFileName(const char *file)
{
	char *escaped;
	char *uri = NULL; //[strlen(file) + 7];
	//const char *format;
	
	escaped = g_uri_escape_string(file, "/", true);
	g_stradd(&uri, "file://");
	g_stradd(&uri, escaped);
	g_free(escaped);
	
	/*if (g_str_has_suffix(uri, ".ps"))
		format = "ps";
	else if (g_str_has_suffix(uri, ".pdf"))
		format = "pdf";
	else if (g_str_has_suffix(uri, ".svg"))
		format = "svg";
	else
		format = NULL;*/

	gtk_enumerate_printers((GtkPrinterFunc)find_file_printer, this, NULL, TRUE);
	
	gtk_print_settings_set(_settings, GTK_PRINT_SETTINGS_OUTPUT_URI, uri);	
	g_free(uri);

	// It does not work!!!
	//if (format)
	//	gtk_print_settings_set(_settings, GTK_PRINT_SETTINGS_OUTPUT_FILE_FORMAT, format);
}


static bool _is_virtual;

static gboolean find_printer(GtkPrinter *gtk_printer, gPrinter *printer)
{
	if (strcmp(printer->name(), gtk_printer_get_name(gtk_printer)))
		return false;
	
	_is_virtual = gtk_printer_is_virtual(gtk_printer);
	return true;
}

bool gPrinter::isVirtual()
{
	_is_virtual = false;
	gtk_enumerate_printers((GtkPrinterFunc)find_printer, this, NULL, TRUE);
	return _is_virtual;
}

static int _dump_tree_radio_button;
static int _dump_tree_entry;

static void dump_tree(GtkWidget *wid, GtkPrintUnixDialog *dialog)
{
	//fprintf(stderr, "dump_tree: %s\n", G_OBJECT_TYPE_NAME(wid));
	if (GTK_IS_RADIO_BUTTON(wid))
	{
		//fprintf(stderr, "dump_tree: radio button: %s\n", gtk_button_get_label(GTK_BUTTON(wid)));
		_dump_tree_radio_button--;
		if (_dump_tree_radio_button == 0)
		{
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(wid), TRUE);
		}
	}
	else if (GTK_IS_ENTRY(wid))
	{
		//fprintf(stderr, "dump_tree: entry: %s\n", gtk_entry_get_text(GTK_ENTRY(wid)));
		_dump_tree_entry--;
		if (_dump_tree_entry == 0)
		{
			char *path;
			char *name;
			
			path = unescape_uri(gtk_print_settings_get(gPrinter::_current->_settings, GTK_PRINT_SETTINGS_OUTPUT_URI));
			//fprintf(stderr, "dump_tree: path = %s\n", path);
			if (path)
			{
				name = g_path_get_basename(path);
				gtk_entry_set_text(GTK_ENTRY(wid), name);
				g_free(name);
			}
		}
	}
	else if (GTK_IS_CONTAINER(wid))
		gtk_container_foreach(GTK_CONTAINER(wid), (GtkCallback)dump_tree, (gpointer)dialog);
}

void gPrinter::fixPrintDialog(GtkPrintUnixDialog *dialog)
{
	const char *output = gtk_print_settings_get(gPrinter::_current->_settings, GTK_PRINT_SETTINGS_OUTPUT_URI);
	
	_dump_tree_entry = 1;
	_dump_tree_radio_button = 0;

	if (output)
	{
		if (g_str_has_suffix(output, ".pdf"))
			_dump_tree_radio_button = 0;
		if (g_str_has_suffix(output, ".ps"))
			_dump_tree_radio_button = 2;
		else if (g_str_has_suffix(output, ".svg"))
			_dump_tree_radio_button = 3;
	}
	
	dump_tree(GTK_WIDGET(dialog), dialog);
}

static gboolean find_all_printers(GtkPrinter *gtk_printer, bool (*callback)(const char *, bool))
{
	if (strcmp(G_OBJECT_TYPE_NAME(gtk_printer_get_backend(gtk_printer)), "GtkPrintBackendFile"))
		return (*callback)(gtk_printer_get_name(gtk_printer), gtk_printer_is_default(gtk_printer));

	return FALSE;
}

void gPrinter::enumeratePrinters(bool (*callback)(const char *, bool))
{
	gtk_enumerate_printers((GtkPrinterFunc)find_all_printers, (gpointer)callback, NULL, TRUE);
}
