/***************************************************************************

  gtools.cpp

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
#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>
/*******************************************************************

Conversion between GDK and long type colors

********************************************************************/
void fill_gdk_color(GdkColor *gcol,long color)
{
	GdkColormap *cmap;
	
	cmap=gdk_colormap_get_system();
	gcol->red=0xFF + ((color & 0xFF0000)>>8);
	gcol->green=0xFF + (color & 0x00FF00);
	gcol->blue=0xFF + ((color & 0x0000FF)<<8);
	gdk_color_alloc(cmap,gcol);
}

long get_gdk_color(GdkColor *gcol)
{
	long color=0;

	color = (gcol->red & 0xFF00)<<8;
	color |= gcol->green & 0xFF00;
	color |= (gcol->blue & 0xFF00)>>8;
	
	return color;
}

long get_gdk_fg_color(GtkWidget *wid)
{
	GtkStyle* st;

	st=gtk_widget_get_style(wid);	
	return get_gdk_color(&st->fg[GTK_STATE_NORMAL]);
}

void set_gdk_fg_color(GtkWidget *wid,long color)
{
	GdkColor gcol;

	if (get_gdk_fg_color(wid)==color) return;
	
	fill_gdk_color(&gcol,color);
	gtk_widget_modify_fg (wid,GTK_STATE_NORMAL,&gcol);	
}

long get_gdk_bg_color(GtkWidget *wid)
{
	GtkStyle* st;

	st=gtk_widget_get_style(wid);	
	return get_gdk_color(&st->bg[GTK_STATE_NORMAL]);
}

void set_gdk_bg_color(GtkWidget *wid,long color)
{
	GdkColor gcol;

	if (get_gdk_bg_color(wid)==color) return;
	
	fill_gdk_color(&gcol,color);
	gtk_widget_modify_bg (wid,GTK_STATE_NORMAL,&gcol);
	gtk_widget_modify_bg (wid,GTK_STATE_ACTIVE,&gcol);
	gtk_widget_modify_bg (wid,GTK_STATE_PRELIGHT,&gcol);
	gtk_widget_modify_bg (wid,GTK_STATE_SELECTED,&gcol);
}

long get_gdk_text_color(GtkWidget *wid)
{
	GtkStyle* st;

	st=gtk_widget_get_style(wid);	
	return get_gdk_color(&st->text[GTK_STATE_NORMAL]);
}

void set_gdk_text_color(GtkWidget *wid,long color)
{
	GdkColor gcol;

	if (get_gdk_text_color(wid)==color) return;
	
	fill_gdk_color(&gcol,color);
	gtk_widget_modify_text (wid,GTK_STATE_NORMAL,&gcol);	
}

long get_gdk_base_color(GtkWidget *wid)
{
	GtkStyle* st;

	st=gtk_widget_get_style(wid);
	return get_gdk_color(&st->base[GTK_STATE_NORMAL]);
}

void set_gdk_base_color(GtkWidget *wid,long color)
{
	GdkColor gcol;
	
	if (get_gdk_base_color(wid)==color) return;
	
	fill_gdk_color(&gcol,color);
	gtk_widget_modify_base (wid,GTK_STATE_NORMAL,&gcol);	
}
/*******************************************************************

 Border style in gWidgets that use a Frame to show it
 
 *******************************************************************/
int Frame_getBorder(GtkFrame *fr)
{
	switch (gtk_frame_get_shadow_type(fr))
	{
		case GTK_SHADOW_NONE: return BORDER_NONE;
		case GTK_SHADOW_ETCHED_OUT: return BORDER_PLAIN;
		case GTK_SHADOW_IN: return BORDER_SUNKEN;
		case GTK_SHADOW_OUT: return BORDER_RAISED;
		case GTK_SHADOW_ETCHED_IN: return BORDER_ETCHED;
	}
	
	return BORDER_NONE;
}

void Frame_setBorder(GtkFrame *fr,int vl)
{
	switch(vl)
	{
		case BORDER_NONE:
			gtk_frame_set_shadow_type(fr,GTK_SHADOW_NONE);
			break;
		case BORDER_PLAIN:
			gtk_frame_set_shadow_type(fr,GTK_SHADOW_ETCHED_OUT);
			break;
		case BORDER_SUNKEN:
			gtk_frame_set_shadow_type(fr,GTK_SHADOW_IN);
			break;
		case BORDER_RAISED:
			gtk_frame_set_shadow_type(fr,GTK_SHADOW_OUT);
			break;
		case BORDER_ETCHED:
			gtk_frame_set_shadow_type(fr,GTK_SHADOW_ETCHED_IN);
			break;
		default: return;
	}
}

/*************************************************************

 Takes a snapshot of a GdkWindow
 
 *************************************************************/
gPicture *Grab_gdkWindow(GdkWindow *win)
{
	GdkPixbuf *buf;
	GdkColormap* cmap;
	gPicture *ret;
	gint x,y;
	
	gdk_window_get_geometry(win,0,0,&x,&y,0);
    if ( (x<1) || (y<1) ) return NULL;
	cmap=gdk_colormap_get_system();
	buf=gdk_pixbuf_get_from_drawable(NULL,win,cmap,0,0,0,0,x,y);
	ret=gPicture::fromPixbuf(buf);
	g_object_unref(G_OBJECT(buf));
	g_object_unref(G_OBJECT(cmap));
	return ret;
}

/*************************************************************

 GtkLabel with accelerators
 
 *************************************************************/
void gMnemonic_correctText(char *st,char **buf)
{
	long bucle,b2;
	long len=strlen(st);
	long len_in=len;
	
	for (bucle=0;bucle<len_in;bucle++)
	{
		if (st[bucle]=='&')
		{
			if (bucle<(len_in-1)) 
				if (st[bucle+1]=='&')
					len--;
				
		}
		else if (st[bucle]=='_') len++;
	}
	
	*buf=(char*)g_malloc(sizeof(char)*(len+1));
	b2=0;
	
	for (bucle=0;bucle<len_in;bucle++)
	{
		if (st[bucle]=='&')
		{
			if (bucle<(len_in-1))
			{
				if (st[bucle+1]=='&')
				{
					(*buf)[b2++]='&';
					bucle++;
				}
				else
				{
					(*buf)[b2++]='_';
				}
			}
			else
			{
				(*buf)[b2]=' ';
				b2++;
			}
		}
		else if (st[bucle]=='_')
		{
			(*buf)[b2++]='_';
			(*buf)[b2++]='_';
		}
		else
		{
			(*buf)[b2++]=st[bucle];
		}	
		(*buf)[b2]=0;
	}
}

guint gMnemonic_correctMarkup(char *st,char **buf)
{
	long bucle,b2;
	long len=strlen(st);
	long len_in=len;
	guint retval=0;	

	for (bucle=0;bucle<len_in;bucle++)
	{
		if (st[bucle]=='&')
		{
			if (bucle<(len_in-1)) 
			{
				if (st[bucle+1]=='&') len+-3;
				else                  len+=6;
			}
			else
			{
				len+=4;
			}
				
		}
		else if (st[bucle]=='<') 
		{
			len+=3;
		}
		else if (st[bucle]=='>')
		{
			len+=3;
		}
	}
	
	*buf=(char*)g_malloc(sizeof(char)*(len+1));
	(*buf[0])=0;
	b2=0;
	
	for (bucle=0;bucle<len_in;bucle++)
	{
		if (st[bucle]=='&')
		{
			if (bucle<(len_in-1))
			{
				if (st[bucle+1]=='&')
				{
					(*buf)[b2++]='&';
					(*buf)[b2++]='a';
					(*buf)[b2++]='m';
					(*buf)[b2++]='p';
					(*buf)[b2++]=';';
					bucle++;
				}
				else
				{
					bucle++;
					retval=(guint)st[bucle];
					(*buf)[b2++]='<';
					(*buf)[b2++]='u';
					(*buf)[b2++]='>';
					(*buf)[b2++]=st[bucle];
					(*buf)[b2++]='<';
					(*buf)[b2++]='/';
					(*buf)[b2++]='u';
					(*buf)[b2++]='>';
				}
			}
			else
			{
				(*buf)[b2++]='&';
				(*buf)[b2++]='a';
				(*buf)[b2++]='m';
				(*buf)[b2++]='p';
				(*buf)[b2++]=';';
			}
		}
		else if (st[bucle]=='<')
		{
			(*buf)[b2++]='&';
			(*buf)[b2++]='l';
			(*buf)[b2++]='t';
			(*buf)[b2++]=';';
		}
		else if (st[bucle]=='>')
		{
			(*buf)[b2++]='&';
			(*buf)[b2++]='g';
			(*buf)[b2++]='t';
			(*buf)[b2++]=';';
		}
		else
		{
			(*buf)[b2++]=st[bucle];
		}	
		(*buf)[b2]=0;
	}

	return retval;
}

void gMnemonic_returnText(char *st,char **buf)
{
		long bucle,b2;
	long len=strlen(st);
	long len_in=len;
	
	for (bucle=0;bucle<len_in;bucle++)
	{
		if (st[bucle]=='_')
		{
			if (bucle<(len_in-1)) 
				if (st[bucle+1]=='_')
					len--;
				
		}
		else if (st[bucle]=='&') len++;
	}
	
	*buf=(char*)g_malloc(sizeof(char)*(len+1));
	b2=0;
	
	for (bucle=0;bucle<len_in;bucle++)
	{
		if (st[bucle]=='_')
		{
			if (bucle<(len_in-1))
			{
				if (st[bucle+1]=='_')
				{
					(*buf)[b2++]='&';
					bucle++;
				}
				else
				{
					(*buf)[b2++]='_';
				}
			}
			else
			{
				(*buf)[b2]=' ';
				b2++;
			}
		}
		else if (st[bucle]=='&')
		{
			(*buf)[b2++]='&';
			(*buf)[b2++]='&';
		}
		else
		{
			(*buf)[b2++]=st[bucle];
		}	
		(*buf)[b2]=0;
	}
}
