/***************************************************************************

  gmessage.cpp

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

#include <glib.h>
#include <glib/gprintf.h>
#include "widgets.h"
#include "gdialog.h"
#include "gdesktop.h"
#include "gmainwindow.h"
#include "gapplication.h"
#include "gmessage.h"

static char *MESSAGE_title=NULL;

#define MESSAGE_ok ((char *)"OK")

static gColor DIALOG_color=0;
static char *DIALOG_path=NULL;
static char **DIALOG_paths=NULL;
static char *DIALOG_title=NULL;
static gFont *DIALOG_font=NULL;
static bool _dialog_show_hidden = false;

static gboolean cb_show(GtkWidget *widget, gpointer data)
{
	GdkGeometry geometry;
	geometry.min_width = gDesktop::scale() * 32;
	geometry.min_height = gDesktop::scale() * 6;

	gtk_window_set_geometry_hints(GTK_WINDOW(widget), widget, &geometry, (GdkWindowHints)(GDK_HINT_MIN_SIZE | GDK_HINT_POS));
	return false;
}


static int run_dialog(GtkDialog *window)
{
  gMainWindow *active;
	GtkWindowGroup *oldGroup;
  int ret;
  
	active = gDesktop::activeWindow();
	if (active)
    gtk_window_set_transient_for(GTK_WINDOW(window), GTK_WINDOW(active->border));
	
	//gApplication::setActiveControl(gApplication::activeControl(), false);
	//GB.CheckPost();
	
	gtk_window_present(GTK_WINDOW(window));
	oldGroup = gApplication::enterGroup();
	gApplication::_loopLevel++;
	(*gApplication::onEnterEventLoop)();
	ret = gtk_dialog_run(window);
	(*gApplication::onLeaveEventLoop)();
	gApplication::_loopLevel--;
	gApplication::exitGroup(oldGroup);
	return ret;
}


/******************************************************************************

gMessage

*******************************************************************************/

typedef struct
  {
    char *bt1;
    char *bt2;
    char *bt3;
  } 
dlg_btn;

static dlg_btn bt;

guint custom_dialog(const gchar *icon,GtkButtonsType btn,char *sg)
{
	GtkWidget *msg, *hrz, *label, *img;
	gint resp;
	char *buf=NULL;
	char *title;
	
  if (bt.bt1) { gMnemonic_correctText(bt.bt1, &buf); bt.bt1 = buf; }
  if (bt.bt2) { gMnemonic_correctText(bt.bt2, &buf); bt.bt2 = buf; }
  if (bt.bt3) { gMnemonic_correctText(bt.bt3, &buf); bt.bt3 = buf; }
	
	title = gMessage::title();
	
#ifdef GTK3
	msg = gtk_dialog_new_with_buttons(title, NULL,
					GTK_DIALOG_MODAL,
					bt.bt1, 1, bt.bt2, 2, bt.bt3, 3, (char *)NULL);
	img = gtk_image_new_from_icon_name(icon,GTK_ICON_SIZE_DIALOG);
#else
	msg = gtk_dialog_new_with_buttons(title, NULL,
					(GtkDialogFlags)(GTK_DIALOG_MODAL+GTK_DIALOG_NO_SEPARATOR),
					bt.bt1, 1, bt.bt2, 2, bt.bt3, 3, (char *)NULL);
	img = gtk_image_new_from_stock(icon,GTK_ICON_SIZE_DIALOG);
#endif

	label = gtk_label_new("");
	
	if (sg) 
		buf = gt_html_to_pango_string(sg, -1, true);
		
	if (buf)
	{
		gtk_label_set_markup(GTK_LABEL(label),buf);
		g_free(buf);
	}
	
	hrz = gtk_hbox_new(FALSE, gDesktop::scale());
  gtk_container_set_border_width(GTK_CONTAINER(hrz), gDesktop::scale() * 2);

	gtk_container_add (GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(msg))),hrz);
	
	gtk_container_add (GTK_CONTAINER(hrz),img);
	gtk_box_set_child_packing(GTK_BOX(hrz), img, false, false, 0, GTK_PACK_START);
  
  gtk_container_add (GTK_CONTAINER(hrz),label);
	//gtk_box_set_child_packing(GTK_BOX(hrz), label, true, false, 0, GTK_PACK_END);
	
	gtk_widget_show_all(hrz);
	
	gtk_widget_realize(msg);

	gdk_window_set_type_hint(gtk_widget_get_window(msg), GDK_WINDOW_TYPE_HINT_UTILITY);
	gtk_window_set_position(GTK_WINDOW(msg), GTK_WIN_POS_CENTER_ALWAYS);
	
	g_signal_connect(G_OBJECT(msg), "show", G_CALLBACK(cb_show), (gpointer)NULL);

	resp = run_dialog(GTK_DIALOG(msg));
	gtk_widget_destroy(msg);
	
	if (resp<0)
	{
		resp=1;
		if (bt.bt2) resp=2;
		if (bt.bt3) resp=3;
	}
	
	if (bt.bt1) g_free(bt.bt1);
	if (bt.bt2) g_free(bt.bt2);
	if (bt.bt3) g_free(bt.bt3);
	
	return resp;
}

int gMessage::showDelete(char *msg,char *btn1,char *btn2,char *btn3)
{
	bt.bt1=MESSAGE_ok;
	bt.bt2=NULL;
	bt.bt3=NULL;
	if (btn1) bt.bt1=btn1;
	if (btn2) bt.bt2=btn2;
	if (btn3) bt.bt3=btn3;
	return custom_dialog(
#ifdef GTK3
		"user-trash",
#else
		GTK_STOCK_DELETE,
#endif
		GTK_BUTTONS_OK,msg);
}

int gMessage::showError(char *msg,char *btn1,char *btn2,char *btn3)
{
	bt.bt1=MESSAGE_ok;
	bt.bt2=NULL;
	bt.bt3=NULL;
	if (btn1) bt.bt1=btn1;
	if (btn2) bt.bt2=btn2;
	if (btn3) bt.bt3=btn3;
	return custom_dialog(
#ifdef GTK3
		"dialog-error",
#else
		GTK_STOCK_DIALOG_ERROR,
#endif
		GTK_BUTTONS_OK,msg);
}

int gMessage::showInfo(char *msg,char *btn1)
{
	bt.bt1=MESSAGE_ok;
	bt.bt2=NULL;
	bt.bt3=NULL;
	if (btn1) bt.bt1=btn1;
	return custom_dialog(
#ifdef GTK3
		"dialog-information",
#else
		GTK_STOCK_DIALOG_INFO,
#endif
		GTK_BUTTONS_OK,msg);
}

int gMessage::showQuestion(char *msg,char *btn1,char *btn2,char *btn3)
{
	bt.bt1=MESSAGE_ok;
	bt.bt2=NULL;
	bt.bt3=NULL;
	if (btn1) bt.bt1=btn1;
	if (btn2) bt.bt2=btn2;
	if (btn3) bt.bt3=btn3;
	return custom_dialog(
#ifdef GTK3
		"dialog-question",
#else
		GTK_STOCK_DIALOG_QUESTION,
#endif
		GTK_BUTTONS_OK,msg);
}

int gMessage::showWarning(char *msg,char *btn1,char *btn2,char *btn3)
{
	bt.bt1=MESSAGE_ok;
	bt.bt2=NULL;
	bt.bt3=NULL;
	if (btn1) bt.bt1=btn1;
	if (btn2) bt.bt2=btn2;
	if (btn3) bt.bt3=btn3;
	return custom_dialog(
#ifdef GTK3
		"dialog-warning",
#else
		GTK_STOCK_DIALOG_WARNING,
#endif
		GTK_BUTTONS_OK,msg);
}

char *gMessage::title()
{
	return MESSAGE_title;
}

void gMessage::setTitle(char *title)
{
	if (MESSAGE_title)
	{
		g_free(MESSAGE_title);
		MESSAGE_title=NULL;
	}
	
	if (title && *title)
		MESSAGE_title = g_strdup(title);
}

void gMessage::exit()
{
  gMessage::setTitle(NULL);
}

/******************************************************************************

gDialog

*******************************************************************************/

GPtrArray *gDialog::_filter = NULL;

static void free_path(void)
{
	if (DIALOG_path)
	{
		g_free(DIALOG_path);
		DIALOG_path = NULL; 
	}
	
	if (DIALOG_paths)
	{
		int i = 0;
	
		while (DIALOG_paths[i])
		{
			g_free(DIALOG_paths[i]);
			i++;
		}
		g_free(DIALOG_paths);
		DIALOG_paths=NULL;
	}
}

static void set_filters(GtkFileChooser* ch)
{	
	char **filters;
	int nfilters;
	int i, p;
	GString *name;
	char *filter;
	char **patterns;
	GtkFileFilter *ft;
	GSList *lft;
	
	filters = gDialog::filter(&nfilters);
	if (!nfilters)
    return;
    
  nfilters--;
	
	for (i = 0; i < nfilters; i += 2)
  {
		filter = filters[i];
		
		ft = gtk_file_filter_new();
		
		name = g_string_new(filters[i + 1]);
		g_string_append_printf(name, " (%s)", filter);
		gtk_file_filter_set_name(ft, name->str);
		g_string_free(name, true);
		
    patterns = g_strsplit(filter, ";", 0);
    for (p = 0; patterns[p]; p++)
      gtk_file_filter_add_pattern(ft, patterns[p]);
      
    g_strfreev(patterns);
	
		gtk_file_chooser_add_filter(ch, ft);
	}
	
	lft = gtk_file_chooser_list_filters(ch);
	if (lft)
	{
		gtk_file_chooser_set_filter(ch, (GtkFileFilter *)lft->data);
		g_slist_free(lft);
	}
}
	
static bool run_file_dialog(GtkFileChooserDialog *msg)
{
	GSList *names,*iter;
	char *buf;
	long b=0;
	
	set_filters((GtkFileChooser*)msg);
	
	if (run_dialog(GTK_DIALOG(msg)) != GTK_RESPONSE_OK)
 	{
 		gtk_widget_destroy(GTK_WIDGET(msg));
		gDialog::setTitle(NULL);
		return true;
 	}
	
	free_path();
	
	names=gtk_file_chooser_get_filenames((GtkFileChooser*)msg);
	
	if (names)
	{
		buf=(char*)names->data;
		if (buf) {
			DIALOG_path=(char*)g_malloc( sizeof(char)*(strlen(buf)+1) );
			strcpy(DIALOG_path,buf);
		}
		
		b=0;
		DIALOG_paths=(char**)g_malloc(sizeof(char*)*(g_slist_length(names)+1) );
		DIALOG_paths[g_slist_length(names)]=NULL;
		iter=names;
		while(iter)
		{
			buf=(char*)iter->data;
			DIALOG_paths[b]=(char*)g_malloc( sizeof(char)*(strlen(buf)+1) );
			strcpy(DIALOG_paths[b++],buf);
			iter=iter->next;
		}
		
		g_slist_free(names);
	}
	
	gtk_widget_destroy(GTK_WIDGET(msg));
	gDialog::setTitle(NULL);
	return false;
}

void gDialog::exit()
{
	free_path();
	
	gDialog::setFilter(NULL, 0);
	gFont::assign(&DIALOG_font);
}

gFont* gDialog::font()
{
  return DIALOG_font;
}

void gDialog::setFont(gFont *ft)
{
  gFont::set(&DIALOG_font, ft->copy());
}

gColor gDialog::color()
{
	return DIALOG_color;
}
	
void gDialog::setColor(gColor col)
{
	DIALOG_color=col;
}

char* gDialog::title()
{
	return DIALOG_title;
}

void gDialog::setTitle(char *vl)
{
	if (DIALOG_title)
	{
		g_free(DIALOG_title);
		DIALOG_title=NULL;
	}
	
	if (vl && *vl)
		DIALOG_title = g_strdup(vl);
}

char *gDialog::path()
{
	return DIALOG_path;
}

char **gDialog::paths()
{
	return DIALOG_paths;
}

void gDialog::setPath(char *vl)
{
	if (DIALOG_path)
	{
		g_free(DIALOG_path);
		DIALOG_path=NULL;
	}
	
	if (!vl) return;
	
	DIALOG_path=(char*)g_malloc( sizeof(char)*(strlen(vl)+1) );
	strcpy(DIALOG_path,vl);
}

bool gDialog::showHidden()
{
	return _dialog_show_hidden;
}

void gDialog::setShowHidden(bool v)
{
	_dialog_show_hidden = v;
}

char** gDialog::filter(int *nfilter)
{
  if (!_filter)
  {
    *nfilter = 0;
    return NULL;
  }
  
  *nfilter = _filter->len;
  return (char **)(_filter->pdata);
}

void gDialog::setFilter(char** filter, int nfilter)
{
	int i;
	
	if (_filter)
	{
    for (i = 0; i < (int)_filter->len; i++)
      g_free(g_ptr_array_index(_filter, i));
      
    g_ptr_array_free(_filter, true);
    _filter = NULL;
  }
  	
  if (!filter)
    return;
	
	_filter = g_ptr_array_new();
  for (i = 0; i < nfilter; i++)
    g_ptr_array_add(_filter, (gpointer)g_strdup(filter[i]));
}

bool gDialog::openFile(bool multi)
{
	GtkFileChooserDialog *msg;

	msg = (GtkFileChooserDialog*)gtk_file_chooser_dialog_new(
		DIALOG_title ? DIALOG_title : GB.Translate("Open file"),
		NULL,
		GTK_FILE_CHOOSER_ACTION_OPEN,
#ifdef GTK3
		GB.Translate("Cancel"), GTK_RESPONSE_CANCEL, GB.Translate("Open"), GTK_RESPONSE_OK,
#else
		GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN, GTK_RESPONSE_OK,
#endif
		(void *)NULL);

	gtk_file_chooser_set_local_only((GtkFileChooser*)msg, true);
	gtk_file_chooser_set_select_multiple((GtkFileChooser*)msg, multi);
	gtk_widget_show(GTK_WIDGET(msg));
	gtk_file_chooser_unselect_all((GtkFileChooser*)msg);
	
	if (DIALOG_path)
	{
		if (g_file_test(DIALOG_path, G_FILE_TEST_IS_DIR))
			gtk_file_chooser_set_current_folder((GtkFileChooser*)msg, DIALOG_path);
		else
			gtk_file_chooser_select_filename((GtkFileChooser*)msg, DIALOG_path);
	}
	
	gtk_file_chooser_set_show_hidden((GtkFileChooser*)msg, _dialog_show_hidden);
	
	return run_file_dialog(msg);
}

bool gDialog::saveFile()
{
	GtkFileChooserDialog *msg;

	msg = (GtkFileChooserDialog*)gtk_file_chooser_dialog_new(
		DIALOG_title ? DIALOG_title : GB.Translate("Save file"),
		NULL,
		GTK_FILE_CHOOSER_ACTION_SAVE,
#ifdef GTK3
		GB.Translate("Cancel"), GTK_RESPONSE_CANCEL, GB.Translate("Save"), GTK_RESPONSE_OK,
#else
		GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_SAVE, GTK_RESPONSE_OK,
#endif
		(void *)NULL);
	
	gtk_file_chooser_set_do_overwrite_confirmation((GtkFileChooser*)msg, true);
	gtk_file_chooser_set_local_only((GtkFileChooser*)msg, true);
	gtk_file_chooser_set_select_multiple((GtkFileChooser*)msg, false);
	gtk_widget_show(GTK_WIDGET(msg));
	gtk_file_chooser_unselect_all((GtkFileChooser*)msg);
	
	if (DIALOG_path)
	{
		if (DIALOG_path[strlen(DIALOG_path) - 1] == '/' && g_file_test(DIALOG_path, G_FILE_TEST_IS_DIR))
			gtk_file_chooser_set_current_folder((GtkFileChooser*)msg, DIALOG_path);
		else
			gtk_file_chooser_select_filename((GtkFileChooser*)msg, DIALOG_path);
	}
		
	gtk_file_chooser_set_show_hidden((GtkFileChooser*)msg, _dialog_show_hidden);

	return run_file_dialog(msg);
}

bool gDialog::selectFolder()
{
	GtkFileChooserDialog *msg;

	msg = (GtkFileChooserDialog*)gtk_file_chooser_dialog_new(
		DIALOG_title ? DIALOG_title : GB.Translate("Select directory"),
		NULL,
		GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
#ifdef GTK3
		GB.Translate("Cancel"), GTK_RESPONSE_CANCEL, GB.Translate("Open"), GTK_RESPONSE_OK,
#else
		GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN, GTK_RESPONSE_OK,
#endif
		(void *)NULL);
	 
	gtk_file_chooser_set_local_only((GtkFileChooser*)msg, true);
	gtk_file_chooser_set_select_multiple((GtkFileChooser*)msg, false);
	gtk_widget_show(GTK_WIDGET(msg));
	gtk_file_chooser_unselect_all((GtkFileChooser*)msg);
	if (DIALOG_path)
	{
		if (g_file_test(DIALOG_path, G_FILE_TEST_IS_DIR))
			gtk_file_chooser_set_current_folder((GtkFileChooser*)msg, DIALOG_path);
		else
			gtk_file_chooser_select_filename((GtkFileChooser*)msg, DIALOG_path);
	}
	
	gtk_file_chooser_set_show_hidden((GtkFileChooser*)msg, _dialog_show_hidden);

	return run_file_dialog(msg);
}

#ifdef GTK3
bool gDialog::selectFont()
{
	GtkFontChooserDialog *dialog;
	PangoFontDescription *desc;
	gFont *font;

	dialog = (GtkFontChooserDialog *)gtk_font_chooser_dialog_new(DIALOG_title, NULL);

	if (DIALOG_font)
		gtk_font_chooser_set_font_desc(GTK_FONT_CHOOSER(dialog), pango_context_get_font_description(DIALOG_font->ct));

	if (run_dialog(GTK_DIALOG(dialog)) != GTK_RESPONSE_OK)
 	{
 		gtk_widget_destroy(GTK_WIDGET(dialog));
		gDialog::setTitle(NULL);
		return true;
 	}

	desc = gtk_font_chooser_get_font_desc(GTK_FONT_CHOOSER(dialog));

	gtk_widget_destroy(GTK_WIDGET(dialog));
	gDialog::setTitle(NULL);

	font = new gFont(desc);
	setFont(font);
	gFont::assign(&font);

	pango_font_description_free(desc);

	//printf("-> %s/%s/%s/%d\n", DIALOG_font->name(), DIALOG_font->bold() ? "BOLD" : "", DIALOG_font->italic() ? "ITALIC" : "", DIALOG_font->size());

	return false;
}
#else
bool gDialog::selectFont()
{
	GtkFontSelectionDialog *msg;
	PangoFontDescription *desc;
	char *buf;
	gFont *font;
		
	if (DIALOG_title)
		msg=(GtkFontSelectionDialog*)gtk_font_selection_dialog_new (DIALOG_title);
	else
		msg=(GtkFontSelectionDialog*)gtk_font_selection_dialog_new ("Select Font");


	if (DIALOG_font)
	{
		desc=pango_context_get_font_description(DIALOG_font->ct);
		buf=pango_font_description_to_string(desc);
		gtk_font_selection_dialog_set_font_name(msg,buf);
		g_free(buf);
	}
	
	if (run_dialog(GTK_DIALOG(msg)) != GTK_RESPONSE_OK)
 	{
 		gtk_widget_destroy(GTK_WIDGET(msg));
		gDialog::setTitle(NULL);
		return true;
 	}
	
	buf = gtk_font_selection_dialog_get_font_name(msg);
	desc = pango_font_description_from_string(buf);
	g_free(buf);
	
	gtk_widget_destroy(GTK_WIDGET(msg));
	gDialog::setTitle(NULL);
	
	font = new gFont(desc);
	setFont(font);
	gFont::assign(&font);
	
	pango_font_description_free(desc);
	
	//printf("-> %s/%s/%s/%d\n", DIALOG_font->name(), DIALOG_font->bold() ? "BOLD" : "", DIALOG_font->italic() ? "ITALIC" : "", DIALOG_font->size());
	
	return false;
}
#endif

#ifdef GTK3
bool gDialog::selectColor()
{
	GtkColorChooserDialog *dialog;
	GdkRGBA color;

	gt_color_to_frgba(DIALOG_color, &color.red, &color.green, &color.blue, &color.alpha);

	dialog = (GtkColorChooserDialog *)gtk_color_chooser_dialog_new(DIALOG_title, NULL);

	gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(dialog), &color);

	gtk_window_present(GTK_WINDOW(dialog));

	if (run_dialog(GTK_DIALOG(dialog)) != GTK_RESPONSE_OK)
 	{
 		gtk_widget_destroy(GTK_WIDGET(dialog));
		gDialog::setTitle(NULL);
		return true;
 	}

	gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(dialog), &color);
	DIALOG_color = gt_frgba_to_color(color.red, color.green, color.blue, color.alpha);

	gtk_widget_destroy(GTK_WIDGET(dialog));
	gDialog::setTitle(NULL);
	return false;
}
#else
bool gDialog::selectColor()
{
	GtkColorSelectionDialog *msg;
	GdkColor gcol;
	
	fill_gdk_color(&gcol, DIALOG_color);
	
	if (DIALOG_title)
		msg=(GtkColorSelectionDialog*)gtk_color_selection_dialog_new (DIALOG_title);
	else
		msg=(GtkColorSelectionDialog*)gtk_color_selection_dialog_new(GB.Translate("Select Color"));
    
	gtk_color_selection_set_current_color(GTK_COLOR_SELECTION(gtk_color_selection_dialog_get_color_selection(msg)), &gcol);
	
	gtk_window_present(GTK_WINDOW(msg));
	if (run_dialog(GTK_DIALOG(msg)) != GTK_RESPONSE_OK)
 	{
 		gtk_widget_destroy(GTK_WIDGET(msg));
		gDialog::setTitle(NULL);
		return true;
 	}
	
	gtk_color_selection_get_current_color(GTK_COLOR_SELECTION(gtk_color_selection_dialog_get_color_selection(msg)), &gcol);
	
	DIALOG_color = get_gdk_color(&gcol);
	
	gtk_widget_destroy(GTK_WIDGET(msg));
	gDialog::setTitle(NULL);
	return false;
}
#endif

