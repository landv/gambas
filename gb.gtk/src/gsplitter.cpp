/***************************************************************************

  gsplitter.cpp

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
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

void slt_notify(GObject *gobject,GParamSpec *arg1,gSplitter *data)
{
	if (data->onResize)
	{
		if ( !strcmp(arg1->name,"position") )
	 		data->onResize(data);
	}
}


gSplitter::gSplitter(gControl *parent,int type) : gContainer(parent)
{
	g_typ=Type_gSplitter;
	switch(type)
	{
		case 0:
			border=gtk_hpaned_new(); break;
		default:
			border=gtk_vpaned_new(); break;
	}
	
	onResize=NULL;
	widget=border;
	curr=GTK_PANED(border);
	connectParent();
	initSignals();

	g_signal_connect(G_OBJECT(curr),"notify",G_CALLBACK(slt_notify),(gpointer)this);
}

void gSplitter::addWidget(GtkWidget *w)
{	
	GtkWidget *tmp;
	
	if (!gtk_paned_get_child1(curr))
	{
		gtk_paned_add1(curr,w);
		return;
	}
	
	if (GTK_WIDGET_TYPE(widget)==GTK_TYPE_HPANED )
		tmp=gtk_hpaned_new();
	else
		tmp=gtk_vpaned_new();
		
	gtk_widget_show_all(tmp);
	gtk_paned_add2(curr,tmp);
	curr=GTK_PANED(tmp);
	gtk_paned_add1(curr,w);	
	g_signal_connect(G_OBJECT(curr),"notify",G_CALLBACK(slt_notify),(gpointer)this);
}

void gSplitter::setLayout(char *vl)
{
	char **split;
	char *sval;
	long val=0;
	long num;
	GtkPaned *iter=GTK_PANED(border);
	
	if (!vl) return;
	
	 split=g_strsplit((const char*)vl,",",-1);
	 if (!split) return;
	 
	 sval=split[val];
	 
	 while (sval)
	 {
	 	if (!iter) break;
		if (iter)
		{
			num=strtol(sval,NULL,10);
			if (errno!=EINVAL) gtk_paned_set_position(iter,num);
			
		}
		iter=(GtkPaned*)gtk_paned_get_child2(iter);
		val++;
		sval=split[val];
	 }
	 
	 val=0;
	 while (split[val])
	 {
	 	g_free(split[val++]);
	 }
	 
	 g_free(split);
}


char* gSplitter::layout()
{
	GtkPaned *iter;
	unsigned int vl,sum=0;
	char buf[7];
	GString *ret=g_string_new("");
	
	iter=GTK_PANED(border);
	if (!gtk_paned_get_child1(GTK_PANED(iter)) ) return g_string_free(ret,false);
	
	while (iter)
	{
		vl=(unsigned int)gtk_paned_get_position(iter);
		sum+=vl;
		iter=(GtkPaned*)gtk_paned_get_child2 (iter);
		if (iter)
		{
			g_string_append_printf(ret,"%u,",vl);
		}
	}
	
	if (GTK_WIDGET_TYPE(widget)==GTK_TYPE_HPANED )
		g_string_append_printf(ret,"%u",width()-sum);
	else
		g_string_append_printf(ret,"%u",height()-sum);
	return g_string_free(ret,false);
}

long gSplitter::childCount()
{
	GtkPaned *iter;
	long ret=0;
	
	if ( !gtk_paned_get_child1(GTK_PANED(border)) ) return 0;
	iter=GTK_PANED(border);
	
	while (iter)
	{
		ret++;
		iter=(GtkPaned*)gtk_paned_get_child2(iter);
	}
	
	return ret;
}

gControl* gSplitter::child(long index)
{
	GtkPaned *iter;
	long ret=0;
	GtkWidget *element=NULL;
	
	if (index<0) return NULL;
	
	if ( !gtk_paned_get_child1(GTK_PANED(border)) ) return NULL;
	iter=GTK_PANED(border);
	
	while (iter)
	{
		if (ret==index)
		{
			element=gtk_paned_get_child1(iter);
			break;
		}
		ret++;
		iter=(GtkPaned*)gtk_paned_get_child2(iter);
	}
	
	
	return gApplication::controlItem(element);
}


