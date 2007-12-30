/***************************************************************************

  gtextarea.cpp

  (c) 2004-2005 - Daniel Campos Fernández <danielcampos@netcourrier.com>
  
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
#include <gtk/gtk.h>

gTextArea::gTextArea(gControl *parent) : gControl(parent)
{
	g_typ=Type_gTextArea;
	border=gtk_scrolled_window_new (NULL,NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(border),GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(border),GTK_SHADOW_IN);
	widget=gtk_text_view_new();
	gtk_container_add(GTK_CONTAINER(border),widget);
	connectParent();
	initSignals();
	setWrap(false);
}

long gTextArea::backGround()
{
	return get_gdk_base_color(widget);
}

void gTextArea::setBackGround(long color)
{
	set_gdk_base_color(widget,color);	
	if (!border->window) gtk_widget_realize(border);
	gdk_window_process_updates(border->window,true);
}

long gTextArea::foreGround()
{
	return get_gdk_text_color(widget);
}

void gTextArea::setForeGround(long color)
{	
	set_gdk_text_color(widget,color);
	if (!border->window) gtk_widget_realize(border);
	gdk_window_process_updates(border->window,true);
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

long gTextArea::line()
{
	GtkTextBuffer *buf=gtk_text_view_get_buffer(GTK_TEXT_VIEW(widget));
	GtkTextMark* mark=gtk_text_buffer_get_insert(buf);
	GtkTextIter iter;
	
	gtk_text_buffer_get_iter_at_mark(buf,&iter,mark);
	return gtk_text_iter_get_line(&iter);
}

void gTextArea::setLine(long vl)
{
	long col=column();
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

long gTextArea::column()
{
	GtkTextBuffer *buf=gtk_text_view_get_buffer(GTK_TEXT_VIEW(widget));
	GtkTextMark* mark=gtk_text_buffer_get_insert(buf);
	GtkTextIter iter;
	
	gtk_text_buffer_get_iter_at_mark(buf,&iter,mark);
	return gtk_text_iter_get_line_offset(&iter);
}

void gTextArea::setColumn(long vl)
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

long gTextArea::position()
{
	GtkTextBuffer *buf=gtk_text_view_get_buffer(GTK_TEXT_VIEW(widget));
	GtkTextMark* mark=gtk_text_buffer_get_insert(buf);
	GtkTextIter iter;
	
	gtk_text_buffer_get_iter_at_mark(buf,&iter,mark);
	return gtk_text_iter_get_offset(&iter);
}

void gTextArea::setPosition(long vl)
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

long gTextArea::length()
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

	if (clipBoard_Type()!=1) return;
	txt=clipBoard_getText();
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

long gTextArea::toLine(long pos)
{
	GtkTextBuffer *buf=gtk_text_view_get_buffer(GTK_TEXT_VIEW(widget));
	GtkTextIter iter;

	gtk_text_buffer_get_start_iter(buf,&iter);
	if (pos<0) pos=0;
	if (pos>=length()) pos=length()-1;
	gtk_text_iter_set_offset(&iter,pos);
	return gtk_text_iter_get_line(&iter);
	
}

long gTextArea::toColumn(long pos)
{
	GtkTextBuffer *buf=gtk_text_view_get_buffer(GTK_TEXT_VIEW(widget));
	GtkTextIter iter;

	gtk_text_buffer_get_start_iter(buf,&iter);
	if (pos<0) pos=0;
	if (pos>=length()) pos=length()-1;
	gtk_text_iter_set_offset(&iter,pos);
	return gtk_text_iter_get_line_offset(&iter);
}

long gTextArea::toPosition(long line,long col)
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


long gTextArea::scrollBar()
{
	GtkPolicyType h,v;
	long ret=3;
	
	gtk_scrolled_window_get_policy(GTK_SCROLLED_WINDOW(border),&h,&v);
	if (h==GTK_POLICY_NEVER) ret--;
	if (v==GTK_POLICY_NEVER) ret-=2;
	
	return ret;
}

void gTextArea::setScrollBar(long vl)
{
	GtkScrolledWindow *sc=GTK_SCROLLED_WINDOW(border);
	switch(vl)
	{
		case 0:
			gtk_scrolled_window_set_policy(sc,GTK_POLICY_NEVER,GTK_POLICY_NEVER);
			break;
		case 1:
			gtk_scrolled_window_set_policy(sc,GTK_POLICY_AUTOMATIC,GTK_POLICY_NEVER);
			break;
		case 2:
			gtk_scrolled_window_set_policy(sc,GTK_POLICY_NEVER,GTK_POLICY_AUTOMATIC);
			break;
		case 3:
			gtk_scrolled_window_set_policy(sc,GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);
			break;
	}
}
/**********************************************************************************

gTextArea selection

***********************************************************************************/

long gTextArea::selStart()
{
	GtkTextBuffer *buf=gtk_text_view_get_buffer(GTK_TEXT_VIEW(widget));
	GtkTextIter start,end;
	
	gtk_text_buffer_get_selection_bounds(buf,&start,&end);
	return gtk_text_iter_get_offset(&start);	
}

long gTextArea::selEnd()
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

void gTextArea::selSelect(long start,long length)
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








