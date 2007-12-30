/***************************************************************************

  gcontainer.cpp

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
#include <gtk/gtk.h>
#include "widgets.h"

#include <stdio.h>

gControl* get_next_child_widget (gContainer *gtk_control,long *gtk_list,long gtk_count);

#define WIDGET_TYPE gControl*
#define CONTAINER_TYPE gContainer*
#define ARRANGEMENT_TYPE gContainerArrangement*
#define IS_RIGHT_TO_LEFT() !gDesktop::rightToLeft()
#define GET_WIDGET(_object) _object
#define GET_CONTAINER(_object) _object
#define GET_ARRANGEMENT(_object) &((gContainer*)_object)->arrangement
#define IS_EXPAND(_object) ((gControl*)_object)->expand()
#define IS_DESIGN(_object) ((gControl*)_object)->design()
#define IS_WIDGET_VISIBLE(_widget)  (((gControl*)_widget)->visible())

#define GET_WIDGET_CONTENTS(_widget, _x, _y, _w, _h) _x=0; \
                                                     _y=0; \
                                                     _w=((gContainer*)_widget)->clientWidth(); \
                                                     _h=((gContainer*)_widget)->clientHeight()

#define GET_WIDGET_X(_widget)  ((gControl*)_widget)->left()
#define GET_WIDGET_Y(_widget)  ((gControl*)_widget)->top()
#define GET_WIDGET_W(_widget)  ((gControl*)_widget)->width()
#define GET_WIDGET_H(_widget)  ((gControl*)_widget)->height()
#define MOVE_WIDGET(_widget, _x, _y)  ((gControl*)_widget)->move( _x, _y)
#define RESIZE_WIDGET(_widget, _w, _h)  ((gControl*)_widget)->resize( _w, _h)
#define MOVE_RESIZE_WIDGET(_widget, _x, _y, _w, _h) ((gControl*)_widget)->moveResize( _x, _y, _w, _h)


#define INIT_CHECK_CHILDREN_LIST(_widget)   gContainer *gtk_control=(gContainer*)_widget; \
                                            long gtk_list=0; \
                                            long gtk_count; \
                                            gtk_count=gtk_control->childCount(); \
                                            if (!gtk_count) return; 

#define RESET_CHILDREN_LIST() gtk_list=0;

#define GET_NEXT_CHILD_WIDGET() get_next_child_widget (gtk_control,&gtk_list,gtk_count)


#define GET_OBJECT_FROM_WIDGET(_widget) ((void*)_widget)
#define FUNCTION_NAME arrangeContainer
#include "gb.form.arrangement.h"



gControl* get_next_child_widget (gContainer *gtk_control,long *gtk_list,long gtk_count)
{
	gControl *ret=NULL;
	
	while ( (*gtk_list)<gtk_count )
	{
		ret=gtk_control->child(*gtk_list);
		(*gtk_list)++;
		
		if ( (!ret->border) || (!ret->widget) )
		{
			ret=NULL;
		}
		else
		{
			if (ret->visible()==false) ret=NULL;
			else                       break;
		}
	}
	
	return ret; 
}

void gContainer::performArrange()
{
	long bucle,nchd;
	gControl *chd;

	if (!gApplication::allEvents()) return;

	arrangeContainer((void*)this);	 
	
	if ( getClass() == Type_gSplitter )
	{
		nchd=childCount();
		for (bucle=0;bucle<childCount();bucle++)
		{
			chd=child(bucle);
			chd->bufW=chd->border->allocation.width;
			chd->bufH=chd->border->allocation.height;
			if ( chd->getClass() & 0x0100 )
			((gContainer*)chd)->performArrange();
		}
	}
}



gContainer::gContainer() 
{
	g_print("gContainer() %d\n",this);
	ch_list=NULL;
	arrangement.mode=0;
	arrangement.spacing=0;
	arrangement.padding=0;
	arrangement.autoresize=false;
	arrangement.locked=false;
	arrangement.user=false;
	radiogroup=NULL;
}

gContainer::gContainer(gControl *parent) : gControl(parent)
{
	g_print("gContainer(parent) %d par: %d\n",this,parent);
	ch_list=NULL;
	arrangement.mode=0;
	arrangement.spacing=0;
	arrangement.padding=0;
	arrangement.locked=false;
	arrangement.autoresize=false;
	arrangement.user=false;
	radiogroup=NULL;
}

gContainer::~gContainer()
{
	if (radiogroup) { g_object_unref(G_OBJECT(radiogroup)); radiogroup=NULL; }
}

long gContainer::childCount()
{
	long bucle;
	long ct=0;
	GList *iter;
	
	if (!ch_list) return 0;
	
	iter=g_list_first(ch_list);
	while (iter)
	{
		ct++;
		iter=iter->next;
	}
	
	return ct;
	
	
}

gControl* gContainer::child(long index)
{
	GList *iter;
	long bucle;
	long ct=0;
	
	if ( !ch_list ) return NULL;

	iter=g_list_nth(ch_list,index);
	if (!iter) return NULL;
	return (gControl*)iter->data;
}

long gContainer::arrange()
{
	return arrangement.mode;
}

void gContainer::setArrange(long vl)
{
	switch(vl)
	{
		case ARRANGE_NONE:
		case ARRANGE_HORIZONTAL:
		case ARRANGE_VERTICAL:
		case ARRANGE_LEFT_RIGHT:
		case ARRANGE_TOP_BOTTOM:
		case ARRANGE_FILL: 
			arrangement.mode=vl;
			performArrange();
		default: break;
	}
}

long gContainer::padding()
{
	return arrangement.padding;
}

void gContainer::setPadding(long vl)
{
	if (vl>=0 && vl<=32768) 
	{
		arrangement.padding=vl;
		performArrange();
	}
}

long gContainer::spacing()
{
	return arrangement.spacing;
}

void gContainer::setSpacing(long vl)
{
	if (vl>=0 && vl<=32768) 
	{
		arrangement.spacing=vl;
		performArrange();
	}
}

bool gContainer::autoSize()
{
	return arrangement.autoresize;
}

void gContainer::setAutoSize(bool vl)
{
	arrangement.autoresize=vl;
	performArrange();
}

bool gContainer::user()
{
	return arrangement.user;
}

void gContainer::setUser(bool vl)
{
	arrangement.user=vl;
	performArrange();
}

long gContainer::clientX()
{
	GtkWidget *chd;
	gint x,y;
	
	if (getClass()==Type_gTabStrip)
	{
		chd=gtk_notebook_get_nth_page(GTK_NOTEBOOK(border),0);
		gtk_widget_translate_coordinates(chd,border,0,0,&x,&y);
		return x;
	}
	
	if (getClass()==Type_gFrame) return 2;
	
	return 0;
}

long gContainer::clientY()
{
	GtkWidget *chd;
	gint x,y;
	
	if (getClass()==Type_gTabStrip)
	{
		chd=gtk_notebook_get_nth_page(GTK_NOTEBOOK(border),0);
		gtk_widget_translate_coordinates(chd,border,0,0,&x,&y);
		return y;
	}
	
	if (getClass()==Type_gFrame) return 2;
	
	return 0;
}

long gContainer::clientWidth()
{
	GtkWidget *chd;
	gint x,y;
	
	if (getClass()==Type_gTabStrip)
	{
		chd=gtk_notebook_get_nth_page(GTK_NOTEBOOK(border),0);
		gtk_widget_translate_coordinates(chd,border,0,0,&x,&y);
		return width()-x;
	}
	
	if (getClass()==Type_gFrame)
		if (width()>=4) 
			return width()-4;
	
	return width();
}

long gContainer::clientHeight()
{
	GtkRequisition req;
	GtkWidget *chd;
	gint x,y;
	
	if (getClass()==Type_gMainWindow)
	{
		if ( ((gMainWindow*)this)->menuBar)
		{
			if ( GTK_WIDGET_VISIBLE( ((gMainWindow*)this)->menuBar ) )
			{
				gtk_widget_size_request( GTK_WIDGET(((gMainWindow*)this)->menuBar),&req);
				return height()-req.height;
			}
		}
	}
	
	if (getClass()==Type_gTabStrip)
	{
		chd=gtk_notebook_get_nth_page(GTK_NOTEBOOK(border),0);
		gtk_widget_translate_coordinates(chd,border,0,0,&x,&y);
		return height()-y;
	}
	
	if (getClass()==Type_gFrame)
		if (height()>=4) 
			return height()-4;
	
	
	return height();

}


