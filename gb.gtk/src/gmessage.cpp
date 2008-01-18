/***************************************************************************

  gmessage.cpp

  (c) 2004-2006 - Daniel Campos Fernández <dcamposf@gmail.com>
  
  Gtkmae "GTK+ made easy" classes
  
  Realizado para la Junta de Extremadura. 
  Consejería de Educación Ciencia y Tecnología. 
  Proyecto gnuLinEx
  
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

#include <glib.h>
#include <glib/gprintf.h>
#include "widgets.h"
#include "widgets_private.h"
#include "gdialog.h"
#include "gdesktop.h"
#include "gmainwindow.h"
#include "gmessage.h"

static char *MESSAGE_title=NULL;

#define MESSAGE_ok ((char *)"OK")

static gColor DIALOG_color=0;
static char *DIALOG_path=NULL;
static char **DIALOG_paths=NULL;
static char *DIALOG_title=NULL;
static gFont *DIALOG_font=NULL;


int gDialog_run(GtkDialog *window)
{
  gMainWindow *active;
  
	active = gDesktop::activeWindow();
	if (active)
    gtk_window_set_transient_for(GTK_WINDOW(window), GTK_WINDOW(active->border));
	
	gtk_window_present(GTK_WINDOW(window));
	return gtk_dialog_run(window);
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
	GtkWidget *msg,*hrz,*label,*img;
	gint resp;
	char *buf=NULL;
	
  if (bt.bt1) { gMnemonic_correctText(bt.bt1, &buf); bt.bt1 = buf; }
  if (bt.bt2) { gMnemonic_correctText(bt.bt2, &buf); bt.bt2 = buf; }
  if (bt.bt3) { gMnemonic_correctText(bt.bt3, &buf); bt.bt3 = buf; }
	
	msg=gtk_dialog_new_with_buttons(GB.Application.Title(),NULL,
					(GtkDialogFlags)(GTK_DIALOG_MODAL+GTK_DIALOG_NO_SEPARATOR),
					bt.bt1,1,bt.bt2,2,bt.bt3,3,NULL);
					
	if (MESSAGE_title) gtk_window_set_title(GTK_WINDOW(msg),MESSAGE_title);
	
	img=gtk_image_new_from_stock(icon,GTK_ICON_SIZE_DIALOG);
	label = gtk_label_new ("");
	
	if (sg) 
		buf = gt_html_to_pango_string(sg, -1, true);
		
	if (buf)
	{
		gtk_label_set_markup(GTK_LABEL(label),buf);
		g_free(buf);
	}
	
	hrz=gtk_hbox_new(FALSE, 8);
  gtk_container_set_border_width(GTK_CONTAINER(hrz), 8);
  	
	gtk_container_add (GTK_CONTAINER(GTK_DIALOG(msg)->vbox),hrz);
	
	gtk_container_add (GTK_CONTAINER(hrz),img);
	gtk_box_set_child_packing(GTK_BOX(hrz), img, false, false, 0, GTK_PACK_START);
  
  gtk_container_add (GTK_CONTAINER(hrz),label);
	//gtk_box_set_child_packing(GTK_BOX(hrz), label, true, false, 0, GTK_PACK_END);
	
	gtk_widget_show_all(hrz);
	
	gtk_widget_realize(msg);
	gdk_window_set_type_hint(msg->window,GDK_WINDOW_TYPE_HINT_UTILITY);
	gtk_window_set_position(GTK_WINDOW(msg),GTK_WIN_POS_CENTER_ALWAYS);
	
	resp = gDialog_run(GTK_DIALOG(msg));
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
	return custom_dialog(GTK_STOCK_DELETE,GTK_BUTTONS_OK,msg);
}

int gMessage::showError(char *msg,char *btn1,char *btn2,char *btn3)
{
	bt.bt1=MESSAGE_ok;
	bt.bt2=NULL;
	bt.bt3=NULL;
	if (btn1) bt.bt1=btn1;
	if (btn2) bt.bt2=btn2;
	if (btn3) bt.bt3=btn3;
	return custom_dialog(GTK_STOCK_DIALOG_ERROR,GTK_BUTTONS_OK,msg);
}

int gMessage::showInfo(char *msg,char *btn1)
{
	bt.bt1=MESSAGE_ok;
	bt.bt2=NULL;
	bt.bt3=NULL;
	if (btn1) bt.bt1=btn1;
	return custom_dialog(GTK_STOCK_DIALOG_INFO,GTK_BUTTONS_OK,msg);
}

int gMessage::showQuestion(char *msg,char *btn1,char *btn2,char *btn3)
{
	bt.bt1=MESSAGE_ok;
	bt.bt2=NULL;
	bt.bt3=NULL;
	if (btn1) bt.bt1=btn1;
	if (btn2) bt.bt2=btn2;
	if (btn3) bt.bt3=btn3;
	return custom_dialog(GTK_STOCK_DIALOG_QUESTION,GTK_BUTTONS_OK,msg);
}

int gMessage::showWarning(char *msg,char *btn1,char *btn2,char *btn3)
{
	bt.bt1=MESSAGE_ok;
	bt.bt2=NULL;
	bt.bt3=NULL;
	if (btn1) bt.bt1=btn1;
	if (btn2) bt.bt2=btn2;
	if (btn3) bt.bt3=btn3;
	return custom_dialog(GTK_STOCK_DIALOG_WARNING,GTK_BUTTONS_OK,msg);
}

void gMessage::setTitle(char *title)
{
	if (MESSAGE_title)
	{
		g_free(MESSAGE_title);
		MESSAGE_title=NULL;
	}
	
	if (!title) return;
	
	MESSAGE_title=(char*)g_malloc( sizeof(char)*(strlen(title)+1) );
	strcpy(MESSAGE_title,title);
}

void gMessage::exit()
{
  gMessage::setTitle(NULL);
}

/******************************************************************************

gDialog

*******************************************************************************/

GPtrArray *gDialog::_filter = NULL;

static void gDialog_filters(GtkFileChooser* ch)
{	
	char **filters;
	int nfilters;
	int i, p;
	GString *name;
	char *filter;
	char **patterns;
	GtkFileFilter *ft;
	
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
}
	
#if 0
	for (bucle=0;bucle<DIALOG_nfilter;bucle++)
	{
		if (!strchr(DIALOG_filter[bucle],'('))
		{
			buf=NULL;
			nm=g_strstrip(DIALOG_filter[bucle]);
			data=NULL;
		}
		else
		{
			buf=g_strsplit(DIALOG_filter[bucle],"(",2);
			nm=g_strstrip(buf[0]);
			data=buf[1];
			if (strchr(data,')')) (strchr(data,')'))[0]=0;
		}	
		
		ft=gtk_file_filter_new();
		gtk_file_filter_set_name(ft,nm);
		
		if (buf)
		{
		
			if (strchr(data,';'))
			{
				b2=0;
				bdata=g_strsplit(data,";",0);
				while(bdata[b2])
				{
					bdata[b2]=g_strstrip(bdata[b2]);
					gtk_file_filter_add_pattern(ft,bdata[b2]);
					g_free(bdata[b2]);
					b2++;
				}
				g_free(bdata);
			}
			else
			{
				data=g_strstrip(data);
				gtk_file_filter_add_pattern(ft,data);
			}
		
			g_free(buf[0]);
			g_free(buf[1]);
			g_free(buf);
		}
		
		gtk_file_chooser_add_filter(ch,ft);
	}
#endif


static bool gDialog_runFile(GtkFileChooserDialog *msg)
{
	GSList *names,*iter;
	char *buf;
	long b=0;
	
	gDialog_filters((GtkFileChooser*)msg);
	gtk_window_present(GTK_WINDOW(msg));
	
	if (gDialog_run(GTK_DIALOG(msg)) != GTK_RESPONSE_OK)
  	{
  		gtk_widget_destroy(GTK_WIDGET(msg));
		gDialog::setTitle(NULL);
		return true;
  	}
	
	if (DIALOG_path) { g_free(DIALOG_path); DIALOG_path=NULL; }
	if (DIALOG_paths)
	{
		while (DIALOG_paths[b])
		{
			g_free(DIALOG_paths[b]);
			b++;
		}
		g_free(DIALOG_paths);
		DIALOG_paths=NULL;
	}
	
	
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
	long bucle=0;

	gDialog::setPath(NULL);
	if (DIALOG_paths)
	{
		while(DIALOG_paths[bucle]) g_free(DIALOG_paths[bucle++]);
		g_free(DIALOG_paths);
	}
	
	gDialog::setFilter(NULL, 0);
	gFont::assign(&DIALOG_font);
}

gFont* gDialog::font()
{
  return DIALOG_font;
}

void gDialog::setFont(gFont *ft)
{
  gFont::assign(&DIALOG_font, ft);
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
	
	if (!vl) return;
	
	DIALOG_title=(char*)g_malloc( sizeof(char)*(strlen(vl)+1) );
	strcpy(DIALOG_title,vl);
}

char* gDialog::path()
{
	return DIALOG_path;
}

char** gDialog::paths()
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
		DIALOG_title ? DIALOG_title : "Open",
		NULL,
		GTK_FILE_CHOOSER_ACTION_OPEN,
		GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
		GTK_STOCK_OPEN, GTK_RESPONSE_OK,
		(void *)NULL);

	gtk_file_chooser_set_local_only((GtkFileChooser*)msg,true);
	gtk_file_chooser_set_select_multiple((GtkFileChooser*)msg,multi);
	gtk_widget_show(GTK_WIDGET(msg));
	gtk_file_chooser_unselect_all((GtkFileChooser*)msg);
	if (DIALOG_path)
	{
		gtk_file_chooser_select_filename ((GtkFileChooser*)msg, DIALOG_path);
		if (g_file_test(DIALOG_path, G_FILE_TEST_IS_DIR))
			gtk_file_chooser_set_current_folder((GtkFileChooser*)msg, DIALOG_path);
	}
	
	return gDialog_runFile(msg);
}

bool gDialog::saveFile()
{
	GtkFileChooserDialog *msg;

	msg = (GtkFileChooserDialog*)gtk_file_chooser_dialog_new(
		DIALOG_title ? DIALOG_title : "Save",
		NULL,
		GTK_FILE_CHOOSER_ACTION_SAVE,
		GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
		GTK_STOCK_SAVE, GTK_RESPONSE_OK,
		(void *)NULL);
	 
	gtk_file_chooser_set_local_only((GtkFileChooser*)msg,true);
	gtk_file_chooser_set_select_multiple((GtkFileChooser*)msg,false);
	gtk_widget_show(GTK_WIDGET(msg));
	gtk_file_chooser_unselect_all((GtkFileChooser*)msg);
	if (DIALOG_path)
	{
		gtk_file_chooser_select_filename ((GtkFileChooser*)msg,DIALOG_path);
		gtk_file_chooser_set_current_name ((GtkFileChooser*)msg,DIALOG_path);
	}
		
	return gDialog_runFile(msg);
}

bool gDialog::selectFolder()
{
	GtkFileChooserDialog *msg;

	msg = (GtkFileChooserDialog*)gtk_file_chooser_dialog_new(
		DIALOG_title ? DIALOG_title : "Find directory",
		NULL,
		GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
		GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
		GTK_STOCK_OPEN, GTK_RESPONSE_OK, 
		(void *)NULL);
	 
	gtk_file_chooser_set_local_only((GtkFileChooser*)msg,true);
	gtk_file_chooser_set_select_multiple((GtkFileChooser*)msg,false);
	gtk_widget_show(GTK_WIDGET(msg));
	gtk_file_chooser_unselect_all((GtkFileChooser*)msg);
	if (DIALOG_path)
		gtk_file_chooser_select_filename ((GtkFileChooser*)msg,DIALOG_path);
	
	return gDialog_runFile(msg);

}
	
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
	
	gtk_window_present(GTK_WINDOW(msg));
	if (gtk_dialog_run(GTK_DIALOG(msg)) != GTK_RESPONSE_OK)
  	{
  		gtk_widget_destroy(GTK_WIDGET(msg));
		gDialog::setTitle(NULL);
		return true;
  	}
	
	buf = gtk_font_selection_dialog_get_font_name(msg);
	//printf("gDialog::selectFont: %s\n", buf);
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

bool gDialog::selectColor()
{
	GtkColorSelectionDialog *msg;
	GdkColor gcol;
	
	fill_gdk_color(&gcol, DIALOG_color);
	
	if (DIALOG_title)
		msg=(GtkColorSelectionDialog*)gtk_color_selection_dialog_new (DIALOG_title);
	else
		msg=(GtkColorSelectionDialog*)gtk_color_selection_dialog_new ("Select Color");
    
	gtk_color_selection_set_current_color((GtkColorSelection*)msg->colorsel,&gcol);
	
	gtk_window_present(GTK_WINDOW(msg));
	if (gtk_dialog_run(GTK_DIALOG(msg)) != GTK_RESPONSE_OK)
  	{
  		gtk_widget_destroy(GTK_WIDGET(msg));
		gDialog::setTitle(NULL);
		return true;
  	}
	
	gtk_color_selection_get_current_color((GtkColorSelection*)msg->colorsel,&gcol);
	
	DIALOG_color = get_gdk_color(&gcol);
	
	gtk_widget_destroy(GTK_WIDGET(msg));
	gDialog::setTitle(NULL);
	return false;
}

