/***************************************************************************

  gtextbox.cpp

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

#include "widgets.h"
#include "widgets_private.h"
#include "gtextbox.h"

static void cb_change_insert(GtkEditable *editable, gchar *new_text, gint new_text_length, gint *position, gTextBox *data)
{
	gtk_editable_set_position(editable, *position);
	data->emit(SIGNAL(data->onChange));
	*position = gtk_editable_get_position(editable);
}

static void cb_change_delete(GtkEditable *editable, gint start_pos, gint end_pos, gTextBox *data)
{
	data->emit(SIGNAL(data->onChange));	
}

static void cb_activate(GtkEntry *editable,gTextBox *data)
{
	data->emit(SIGNAL(data->onActivate));
}


gTextBox::gTextBox(gContainer *parent, bool combo) : gControl(parent)
{
	if (!combo)
	{
		g_typ=Type_gTextBox;
		
		have_cursor = true;
		use_base = true;
		
		entry = widget = gtk_entry_new();	
		realize();
		initEntry();
	}
		
	onChange = NULL;
	onActivate = NULL;
}

void gTextBox::initEntry()
{
	if (!entry)
		return;
	
	// This imitates the QT behaviour, where change signal is raised after the cursor has been moved.
	g_signal_connect_after(G_OBJECT(entry), "insert-text", G_CALLBACK(cb_change_insert), (gpointer)this);
	g_signal_connect_after(G_OBJECT(entry), "delete-text", G_CALLBACK(cb_change_delete), (gpointer)this);
	g_signal_connect(G_OBJECT(entry), "activate", G_CALLBACK(cb_activate), (gpointer)this);
}

char* gTextBox::text()
{
	return (char*)gtk_entry_get_text(GTK_ENTRY(entry));
}

void gTextBox::setText(const char *vl)
{
	if (!entry || !strcmp(vl, text()))
		return;
		
	lock();
	gtk_entry_set_text(GTK_ENTRY(entry), vl);
	gtk_editable_set_position(GTK_EDITABLE(entry), -1);
	unlock();
	emit(SIGNAL(onChange));
}

bool gTextBox::password()
{
	if (entry)
		return !gtk_entry_get_visibility(GTK_ENTRY(entry));
	else
		return false;
}

void gTextBox::setPassword(bool vl)
{
	if (!entry)
		return;
		
	gtk_entry_set_visibility(GTK_ENTRY(entry),!vl);
	if (vl)
		gtk_entry_set_invisible_char(GTK_ENTRY(entry), (gunichar)0x25CF);
}

bool gTextBox::isReadOnly()
{
	return !gtk_editable_get_editable(GTK_EDITABLE(entry));
}

void gTextBox::setReadOnly(bool vl)
{
	gtk_editable_set_editable(GTK_EDITABLE(entry),!vl);
}

int gTextBox::position()
{
	if (entry)
		return gtk_editable_get_position(GTK_EDITABLE(entry));
	else
		return 0;
}

void gTextBox::setPosition(int pos)
{
	int len;
	
	if (!entry)
		return;
		
	len = length();
		
	if (pos < 0) 
		pos = 0;
	else if (pos >= len)
		pos = -1;
		
	gtk_editable_set_position(GTK_EDITABLE(entry), pos);
}

bool gTextBox::hasBorder()
{
	if (entry)
		return gtk_entry_get_has_frame(GTK_ENTRY(entry));
	else
		return true;
}

void gTextBox::setBorder(bool vl)
{
	if (!entry)
		return;
	gtk_entry_set_has_frame(GTK_ENTRY(entry), vl);
}

void gTextBox::insert(char *txt, int len)
{
	if (!entry || !len || !txt) return;
	
	int pos = position();
	gtk_editable_insert_text(GTK_EDITABLE(entry), txt, len, &pos);
}

int gTextBox::length()
{
	const gchar *buf;
	
	if (!entry)
		return 0;
	
	buf=gtk_entry_get_text(GTK_ENTRY(entry));
	if (!buf) return 0;
	
	return g_utf8_strlen(buf, -1);
}

int gTextBox::maxLength()
{
	if (entry)
		return gtk_entry_get_max_length(GTK_ENTRY(entry));
	else
		return 0;
}

void gTextBox::setMaxLength(int vl)
{
	if (!entry)
		return;
	if (vl<0) vl=0;
	if (vl>65536) vl=0;
	gtk_entry_set_max_length(GTK_ENTRY(entry), vl);
	
}

bool gTextBox::isSelected()
{
	if (entry)
		return gtk_editable_get_selection_bounds(GTK_EDITABLE(entry), NULL, NULL);
	else
		return false;
}

int gTextBox::selStart()
{
	int start;
	
	if (!entry)
		return -1;
	
	gtk_editable_get_selection_bounds(GTK_EDITABLE(entry),&start,NULL);
	return start;
}

int gTextBox::selLength()
{
	int start,end;

	if (!entry)
		return 0;

	gtk_editable_get_selection_bounds(GTK_EDITABLE(entry),&start,&end);
	return start - end;
}

char* gTextBox::selText()
{
	int start,end;

	if (!entry)
		return NULL;
	gtk_editable_get_selection_bounds(GTK_EDITABLE(entry),&start,&end);
	return gtk_editable_get_chars(GTK_EDITABLE(entry),start,end);
	
}

void gTextBox::setSelText(char *txt,int len)
{
	int start,end;

	if (!entry)
		return;

	gtk_editable_get_selection_bounds(GTK_EDITABLE(entry),&start,&end);	
	gtk_editable_delete_text(GTK_EDITABLE(entry),start,end);
	gtk_editable_insert_text(GTK_EDITABLE(entry),txt,len,&start);
	
}

void gTextBox::selClear()
{
	int start;

	if (!entry)
		return;

	gtk_editable_get_selection_bounds(GTK_EDITABLE(entry),&start,NULL);	
	gtk_editable_select_region(GTK_EDITABLE(entry),start,start);
}

void gTextBox::selectAll()
{
	if (entry)
		gtk_editable_select_region(GTK_EDITABLE(entry), 0, -1);
}

void gTextBox::select(int start,int len)
{
	if (!entry)
		return;
	if ( (len<=0) || (start<0) ) { selClear(); return; }
	gtk_editable_select_region(GTK_EDITABLE(entry),start,start+len);
}


int gTextBox::alignment()
{
	if (entry)
		return gt_to_alignment(gtk_entry_get_alignment(GTK_ENTRY(entry)));
	else
		return ALIGN_NORMAL;
}

void gTextBox::setAlignment(int al)
{
	if (!entry)
		return;
	gtk_entry_set_alignment(GTK_ENTRY(entry), gt_from_alignment(al));
}

void gTextBox::updateCursor(GdkCursor *cursor)
{
  GdkWindow *win;
  
  gControl::updateCursor(cursor);
  if (!entry)
  	return;
  	
  win = GTK_ENTRY(entry)->text_area;
  if (!win)
  	return;
  	
  if (cursor)
    gdk_window_set_cursor(win, cursor);
  else
  {
    cursor = gdk_cursor_new_for_display(gtk_widget_get_display(widget), GDK_XTERM);
    gdk_window_set_cursor(win, cursor);
    gdk_cursor_unref(cursor);
  }
}

void gTextBox::clear()
{
	setText("");
}


int gTextBox::minimumHeight()
{
	GtkRequisition req;
	
	gtk_widget_size_request(widget, &req);
	return req.height - 4;
}
