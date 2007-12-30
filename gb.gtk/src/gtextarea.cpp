/***************************************************************************

  gtextarea.cpp

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
#include "gapplication.h"
#include "gclipboard.h"
#include "gtextarea.h"

// Private structure took from GTK+ 2.10 code. Used for setting the text area cursor.
// **** May not work on future GTK+ versions ****

struct PrivateGtkTextWindow
{
  GtkTextWindowType type;
  GtkWidget *widget;
  GdkWindow *window;
  GdkWindow *bin_window;
  GtkRequisition requisition;
  GdkRectangle allocation;
};

// Private exported functions to get the size of the text
// **** May not work on future GTK+ versions ****

extern "C" {
void gtk_text_layout_set_screen_width(struct _GtkTextLayout *layout, gint width);
void gtk_text_layout_get_size(struct _GtkTextLayout  *layout, gint *width, gint *height);
void gtk_text_layout_invalidate (struct _GtkTextLayout *layout, const GtkTextIter *start, const GtkTextIter *end);
void gtk_text_layout_validate (struct _GtkTextLayout *layout, gint max_pixels);
}

static gboolean cb_motion_notify_event(GtkWidget *widget, GdkEventMotion *event, gTextArea *data)
{
	// Workaround GtkTextView internal cursor changes
	if (gApplication::isBusy())
		data->setMouse(data->mouse());
	return FALSE;
}

static void cb_changed(GtkTextBuffer *buf, gTextArea *data)
{
	data->emit(SIGNAL(data->onChange));
} 

static void cb_mark_set(GtkTextBuffer *buf, GtkTextIter *location, GtkTextMark *mark, gTextArea *data)
{
	if (mark == gtk_text_buffer_get_insert(buf))
		data->emit(SIGNAL(data->onCursor));
} 

gTextArea::gTextArea(gContainer *parent) : gControl(parent)
{
	GtkTextBuffer *buf;
	
	g_typ = Type_gTextArea;
	
	have_cursor = true;
	use_base = true;
	
	onChange = 0;
	onCursor = 0;

	border = gtk_scrolled_window_new (NULL,NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(border),GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);
	widget = gtk_text_view_new();
	
	realize();
	
	g_signal_connect_after(G_OBJECT(widget), "motion-notify-event", G_CALLBACK(cb_motion_notify_event), (gpointer)this);	
	buf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(widget));
	g_signal_connect_after(G_OBJECT(buf), "changed", G_CALLBACK(cb_changed), (gpointer)this);
	g_signal_connect_after(G_OBJECT(buf), "mark-set", G_CALLBACK(cb_mark_set), (gpointer)this);

	setBorder(true);
	setWrap(false);
}

bool gTextArea::hasBorder()
{
	if (gtk_scrolled_window_get_shadow_type(GTK_SCROLLED_WINDOW(border))==GTK_SHADOW_NONE) return false;
	return true;
}

void gTextArea::setBorder(bool vl)
{
	if (vl)
		gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(border),GTK_SHADOW_IN);
	else
		gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(border),GTK_SHADOW_NONE);
}

char* gTextArea::text()
{
	GtkTextBuffer *buf=gtk_text_view_get_buffer(GTK_TEXT_VIEW(widget));
	GtkTextIter start;
	GtkTextIter end;
	
	if (!buf) return NULL;
	
	gtk_text_buffer_get_bounds(buf,&start,&end);
	return gtk_text_buffer_get_text(buf,&start,&end,true);
}

void gTextArea::setText(char *txt)
{
	GtkTextBuffer *buf=gtk_text_view_get_buffer(GTK_TEXT_VIEW(widget));
	if (!txt) txt="";
	
	gtk_text_buffer_set_text(buf,(const gchar *)txt,-1);
}

bool gTextArea::readOnly()
{
	return !gtk_text_view_get_editable(GTK_TEXT_VIEW(widget));
}

void gTextArea::setReadOnly(bool vl)
{
	gtk_text_view_set_editable(GTK_TEXT_VIEW(widget),!vl);
	gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(widget),!vl);
}

int gTextArea::line()
{
	GtkTextBuffer *buf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(widget));
	GtkTextMark* mark = gtk_text_buffer_get_insert(buf);
	GtkTextIter iter;
	
	gtk_text_buffer_get_iter_at_mark(buf,&iter,mark);
	return gtk_text_iter_get_line(&iter);
}

void gTextArea::setLine(int vl)
{
	int col=column();
	GtkTextBuffer *buf=gtk_text_view_get_buffer(GTK_TEXT_VIEW(widget));
	GtkTextMark* mark=gtk_text_buffer_get_insert(buf);
	GtkTextIter iter;
	
	gtk_widget_grab_focus(widget);
	gtk_text_buffer_get_iter_at_mark(buf,&iter,mark);
	gtk_text_iter_set_line (&iter,vl);
	if (gtk_text_iter_get_chars_in_line (&iter)<=col) col=gtk_text_iter_get_chars_in_line (&iter)-1;
	gtk_text_iter_set_line_offset(&iter,col);
	gtk_text_buffer_place_cursor(buf,&iter);
	gtk_text_view_scroll_mark_onscreen(GTK_TEXT_VIEW(widget),mark);
}

int gTextArea::column()
{
	GtkTextBuffer *buf=gtk_text_view_get_buffer(GTK_TEXT_VIEW(widget));
	GtkTextMark* mark=gtk_text_buffer_get_insert(buf);
	GtkTextIter iter;
	
	gtk_text_buffer_get_iter_at_mark(buf,&iter,mark);
	return gtk_text_iter_get_line_offset(&iter);
}

void gTextArea::setColumn(int vl)
{
	GtkTextBuffer *buf=gtk_text_view_get_buffer(GTK_TEXT_VIEW(widget));
	GtkTextMark* mark=gtk_text_buffer_get_insert(buf);
	GtkTextIter iter;

	
	gtk_widget_grab_focus(widget);
	gtk_text_buffer_get_iter_at_mark(buf,&iter,mark);
	
	if (vl<0) 
	{
		vl=gtk_text_iter_get_chars_in_line (&iter)-1;
	}
	else
	{
		if (gtk_text_iter_get_chars_in_line (&iter)<=vl) 
			vl=gtk_text_iter_get_chars_in_line (&iter)-1;
	}
	
	gtk_text_iter_set_line_offset(&iter,vl);
	gtk_text_buffer_place_cursor(buf,&iter);
	gtk_text_view_scroll_mark_onscreen(GTK_TEXT_VIEW(widget),mark);
}

int gTextArea::position()
{
	GtkTextBuffer *buf=gtk_text_view_get_buffer(GTK_TEXT_VIEW(widget));
	GtkTextMark* mark=gtk_text_buffer_get_insert(buf);
	GtkTextIter iter;
	
	gtk_text_buffer_get_iter_at_mark(buf,&iter,mark);
	return gtk_text_iter_get_offset(&iter);
}

void gTextArea::setPosition(int vl)
{
	GtkTextBuffer *buf=gtk_text_view_get_buffer(GTK_TEXT_VIEW(widget));
	GtkTextMark* mark=gtk_text_buffer_get_insert(buf);
	GtkTextIter iter;

	
	gtk_widget_grab_focus(widget);
	gtk_text_buffer_get_iter_at_mark(buf,&iter,mark);
	
	if (vl<0) 
	{
		vl=gtk_text_iter_get_offset (&iter);
	}
	else
	{
		if (gtk_text_iter_get_offset (&iter)<vl) 
			vl=gtk_text_iter_get_offset (&iter);
	}
	
	gtk_text_iter_set_offset(&iter,vl);
	gtk_text_buffer_place_cursor(buf,&iter);
	gtk_text_view_scroll_mark_onscreen(GTK_TEXT_VIEW(widget),mark);
}

int gTextArea::length()
{
	GtkTextBuffer *buf=gtk_text_view_get_buffer(GTK_TEXT_VIEW(widget));
	GtkTextIter iter;
	
	gtk_text_buffer_get_end_iter(buf,&iter);
	return gtk_text_iter_get_offset(&iter);
}

bool gTextArea::wrap()
{
	if (gtk_text_view_get_wrap_mode(GTK_TEXT_VIEW(widget))==GTK_WRAP_NONE) return false;
	return true;
}

void gTextArea::setWrap(bool vl)
{
	if (vl)
		gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(widget),GTK_WRAP_WORD);
	else
		gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(widget),GTK_WRAP_NONE);
}

/**********************************************************************************

gTextArea methods

***********************************************************************************/

void gTextArea::copy()
{
	GtkTextBuffer *buf=gtk_text_view_get_buffer(GTK_TEXT_VIEW(widget));
	GtkClipboard* clip=gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
	
	gtk_text_buffer_copy_clipboard (buf,clip);
}

void gTextArea::cut()
{
	GtkTextBuffer *buf=gtk_text_view_get_buffer(GTK_TEXT_VIEW(widget));
	GtkClipboard* clip=gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
	
	gtk_text_buffer_cut_clipboard (buf,clip,true);
}

void gTextArea::ensureVisible()
{
	setPosition(position());
}

void gTextArea::paste()
{
	GtkTextBuffer *buf=gtk_text_view_get_buffer(GTK_TEXT_VIEW(widget));
	char *txt;

	if (gClipboard::getType() != gClipboard::Text) 
		return;
	
	txt = gClipboard::getText();
	if (txt)
	{
		gtk_text_buffer_insert_at_cursor(buf,(const gchar *)txt,-1);
		g_free(txt);
	}
	
}

void gTextArea::insert(char *txt)
{
	GtkTextBuffer *buf=gtk_text_view_get_buffer(GTK_TEXT_VIEW(widget));
	if (!txt) txt="";
	
	gtk_text_buffer_insert_at_cursor(buf,(const gchar *)txt,-1);
}

int gTextArea::toLine(int pos)
{
	GtkTextBuffer *buf=gtk_text_view_get_buffer(GTK_TEXT_VIEW(widget));
	GtkTextIter iter;

	gtk_text_buffer_get_start_iter(buf,&iter);
	if (pos<0) pos=0;
	if (pos>=length()) pos=length()-1;
	gtk_text_iter_set_offset(&iter,pos);
	return gtk_text_iter_get_line(&iter);
	
}

int gTextArea::toColumn(int pos)
{
	GtkTextBuffer *buf=gtk_text_view_get_buffer(GTK_TEXT_VIEW(widget));
	GtkTextIter iter;

	gtk_text_buffer_get_start_iter(buf,&iter);
	if (pos<0) pos=0;
	if (pos>=length()) pos=length()-1;
	gtk_text_iter_set_offset(&iter,pos);
	return gtk_text_iter_get_line_offset(&iter);
}

int gTextArea::toPosition(int line,int col)
{
	GtkTextBuffer *buf=gtk_text_view_get_buffer(GTK_TEXT_VIEW(widget));
	GtkTextIter iter;

	if (line<0) line=0;
	if (col<0) col=0;
	
	gtk_text_buffer_get_end_iter(buf,&iter);
	if (line>gtk_text_iter_get_line(&iter)) line=gtk_text_iter_get_line(&iter);
	gtk_text_iter_set_line(&iter,line);
	if (col>gtk_text_iter_get_line_offset(&iter)) col=gtk_text_iter_get_line_offset(&iter);
	gtk_text_iter_set_line_offset(&iter,col);
	return gtk_text_iter_get_offset(&iter);
} 


/**********************************************************************************

gTextArea selection

***********************************************************************************/

bool gTextArea::isSelected()
{
	GtkTextBuffer *buf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(widget));
	return gtk_text_buffer_get_selection_bounds(buf,NULL,NULL);
	//return gtk_text_buffer_get_has_selection(buf); // Only since 2.10
}

int gTextArea::selStart()
{
	GtkTextBuffer *buf=gtk_text_view_get_buffer(GTK_TEXT_VIEW(widget));
	GtkTextIter start,end;
	
	gtk_text_buffer_get_selection_bounds(buf,&start,&end);
	return gtk_text_iter_get_offset(&start);	
}

int gTextArea::selEnd()
{
	GtkTextBuffer *buf=gtk_text_view_get_buffer(GTK_TEXT_VIEW(widget));
	GtkTextIter start,end;
	
	gtk_text_buffer_get_selection_bounds(buf,&start,&end);
	return gtk_text_iter_get_offset(&end);
}

char* gTextArea::selText()
{
	GtkTextBuffer *buf=gtk_text_view_get_buffer(GTK_TEXT_VIEW(widget));
	GtkTextIter start,end;
	
	gtk_text_buffer_get_selection_bounds(buf,&start,&end);

	return gtk_text_buffer_get_text(buf,&start,&end,true);
}

void gTextArea::setSelText(char *vl)
{
	GtkTextBuffer *buf=gtk_text_view_get_buffer(GTK_TEXT_VIEW(widget));
	GtkTextIter start,end;
	
	if (!vl) vl="";
	
	if (gtk_text_buffer_get_selection_bounds(buf,&start,&end))
		gtk_text_buffer_delete(buf,&start,&end);
	
	gtk_text_buffer_insert(buf,&start,vl,-1);	
}

void gTextArea::selDelete()
{
	GtkTextBuffer *buf=gtk_text_view_get_buffer(GTK_TEXT_VIEW(widget));
	GtkTextIter start,end;
	
	if (gtk_text_buffer_get_selection_bounds(buf,&start,&end))
	{
		gtk_text_iter_set_offset(&end,gtk_text_iter_get_offset(&start));
		gtk_text_buffer_select_range(buf,&start,&end);
	}
}

void gTextArea::selSelect(int start,int length)
{
	GtkTextBuffer *buf=gtk_text_view_get_buffer(GTK_TEXT_VIEW(widget));
	GtkTextIter Start,End;
	
	gtk_text_buffer_get_end_iter(buf,&Start);
	if ( gtk_text_iter_get_offset(&Start)<start) start=gtk_text_iter_get_offset(&Start);
	
	if (start<0) { length-=start; start=0; }
	if ( (start+length)<0 ) length=(-1)*start;
	
	gtk_text_buffer_get_selection_bounds(buf,&Start,&End);
	gtk_text_iter_set_offset(&Start,start);
	gtk_text_iter_set_offset(&End,start+length);
	gtk_text_buffer_select_range(buf,&Start,&End);
	
}

void gTextArea::updateCursor(GdkCursor *cursor)
{
  GdkWindow *win = ((PrivateGtkTextWindow *)GTK_TEXT_VIEW(widget)->text_window)->bin_window;
  
  gControl::updateCursor(cursor);
  
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

void gTextArea::waitForLayout(int *tw, int *th)
{
	GtkTextBuffer *buf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(widget));
  GtkTextIter start;
  GtkTextIter end;
	gint w, h;
	
	gtk_text_layout_set_screen_width(GTK_TEXT_VIEW(widget)->layout, width());
	
  gtk_text_buffer_get_bounds (buf, &start, &end);
  gtk_text_layout_invalidate (GTK_TEXT_VIEW(widget)->layout, &start, &end);

	gtk_text_layout_validate(GTK_TEXT_VIEW(widget)->layout, 0x7FFFFFFF);
	gtk_text_layout_get_size(GTK_TEXT_VIEW(widget)->layout, &w, &h);
	
	*tw = w;
	*th = h;
}

int gTextArea::textWidth()
{
	int w, h;
	waitForLayout(&w, &h);
	return w;
}

int gTextArea::textHeight()
{
	int w, h;
	waitForLayout(&w, &h);
	return h;
}








