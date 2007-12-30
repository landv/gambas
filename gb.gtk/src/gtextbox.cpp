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

void tBox_Change(GtkEditable *editable,gTextBox *data)
{
	if (data->onChange) data->onChange(data);
}

void tBox_Activate(GtkEntry *editable,gTextBox *data)
{
	if (data->onActivate) data->onActivate(data);
}


gTextBox::gTextBox(gControl *parent) : gControl(parent)
{
	g_typ=Type_gTextBox;
	widget=gtk_entry_new();	
	border=widget;
	connectParent();
	initSignals();
	
	onChange=NULL;
	onActivate=NULL;
	g_signal_connect(G_OBJECT(widget),"changed",G_CALLBACK(tBox_Change),(gpointer)this);
	g_signal_connect(G_OBJECT(widget),"activate",G_CALLBACK(tBox_Activate),(gpointer)this);
}

long gTextBox::backGround()
{
	return get_gdk_base_color(widget);
}

void gTextBox::setBackGround(long color)
{
	set_gdk_base_color(widget,color);	
	if (!border->window) gtk_widget_realize(border);
	gdk_window_process_updates(border->window,true);
}

long gTextBox::foreGround()
{
	return get_gdk_text_color(widget);
}

void gTextBox::setForeGround(long color)
{	
	set_gdk_text_color(widget,color);
	if (!border->window) gtk_widget_realize(border);
	gdk_window_process_updates(border->window,true);
}

char* gTextBox::text()
{
	return (char*)gtk_entry_get_text(GTK_ENTRY(widget));
}

void gTextBox::setText(char *vl)
{
	gtk_entry_set_text(GTK_ENTRY(widget),vl);
}

bool gTextBox::password()
{
	return !gtk_entry_get_visibility(GTK_ENTRY(widget));
}

void gTextBox::setPassword(bool vl)
{
	gtk_entry_set_visibility(GTK_ENTRY(widget),!vl);
}

bool gTextBox::readOnly()
{
	return !gtk_editable_get_editable(GTK_EDITABLE(widget));
}

void gTextBox::setReadOnly(bool vl)
{
	gtk_editable_set_editable(GTK_EDITABLE(widget),!vl);
}

int gTextBox::position()
{
	return gtk_editable_get_position(GTK_EDITABLE(widget));
}

void gTextBox::setPosition(int pos)
{
	if (pos<0) pos=0;
	gtk_editable_set_position(GTK_EDITABLE(widget),pos);
}

bool gTextBox::hasBorder()
{
	return gtk_entry_get_has_frame(GTK_ENTRY(widget));
}

void gTextBox::setBorder(bool vl)
{
	gtk_entry_set_has_frame(GTK_ENTRY(widget),vl);
}

void gTextBox::insert(char *txt,int len)
{
	if ( (!len) || (!txt) ) return;
	
	int pos=position();
	gtk_editable_insert_text(GTK_EDITABLE(widget),txt,len,&pos);
	
}

long gTextBox::length()
{
	const gchar *buf;
	long len;
	
	buf=gtk_entry_get_text(GTK_ENTRY(widget));
	if (!buf) return 0;
	
	len=g_utf8_strlen(buf,-1);
	//g_free((void*)buf);
	return len;
}

int gTextBox::maxLength()
{
	return gtk_entry_get_max_length(GTK_ENTRY(widget));
}

void gTextBox::setMaxLength(int vl)
{
	if (vl<0) vl=0;
	if (vl>65536) vl=0;
	gtk_entry_set_max_length(GTK_ENTRY(widget),vl);
	
}

int gTextBox::selStart()
{
	int start;

	gtk_editable_get_selection_bounds(GTK_EDITABLE(widget),&start,NULL);
	
	return start;
}

int gTextBox::selLength()
{
	int start,end;

	gtk_editable_get_selection_bounds(GTK_EDITABLE(widget),&start,&end);
	
	return start-end;
}

char* gTextBox::selText()
{
	int start,end;

	gtk_editable_get_selection_bounds(GTK_EDITABLE(widget),&start,&end);
	return gtk_editable_get_chars(GTK_EDITABLE(widget),start,end);
	
}

void gTextBox::setSelText(char *txt,int len)
{
	int start,end;

	gtk_editable_get_selection_bounds(GTK_EDITABLE(widget),&start,&end);	
	gtk_editable_delete_text(GTK_EDITABLE(widget),start,end);
	gtk_editable_insert_text(GTK_EDITABLE(widget),txt,len,&start);
	
}

void gTextBox::selClear()
{
	int start;

	gtk_editable_get_selection_bounds(GTK_EDITABLE(widget),&start,NULL);	
	gtk_editable_select_region(GTK_EDITABLE(widget),start,start);
}

void gTextBox::selectAll()
{
	gtk_editable_select_region(GTK_EDITABLE(widget),0,65536);
}

void gTextBox::select(int start,int len)
{
	if ( (len<=0) || (start<0) ) { selClear(); return; }
	gtk_editable_select_region(GTK_EDITABLE(widget),start,start+len);
}


int gTextBox::alignment()
{
	gfloat x;
	int retval=0;
	
	x=gtk_entry_get_alignment (GTK_ENTRY(widget));

	if (!x) retval=alignLeft;
	else if (x<=0.5) retval=alignCenter;
	else retval=alignLeft;
	
	
	return retval;
}

void gTextBox::setAlignment(int al)
{
	gfloat x;
		
	switch (al)
	{
		case alignBottom:		x=0.5; 	break;
		case alignBottomLeft:	x=0; 	break;
		case alignBottomNormal:	x=0; 	break;
		case alignBottomRight:	x=1; 	break;
		case alignCenter:		x=0.5; 	break;
		case alignLeft:			x=0; 	break;
		case alignNormal:		x=0; 	break;
		case alignRight:		x=1; 	break;
		case alignTop:			x=0.5; 	break;
		case alignTopLeft:		x=0; 	break;
		case alignTopNormal:	x=0; 	break;
		case alignTopRight:		x=1; 	break;
		default: return;
	}
	
	gtk_entry_set_alignment (GTK_ENTRY(widget),x);
}





