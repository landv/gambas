/***************************************************************************

  gcontainer.cpp

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
#include <gtk/gtk.h>
#include "widgets.h"

#include <stdio.h>
#include <unistd.h>

#include "gapplication.h"
#include "gdesktop.h"
#include "gmainwindow.h"
#include "gsplitter.h"
#include "gcontainer.h"

static gControl* get_next_child_widget (gContainer *gtk_control, int *gtk_list, int gtk_count)
{
	gControl *ctrl;
	
	while ( (*gtk_list) < gtk_count )
	{
		ctrl = gtk_control->child(*gtk_list);
		//fprintf(stderr, "get_next_child_widget: %d: %p: %s\n", *gtk_list, ctrl, ctrl->name());
		(*gtk_list)++;
		
		if (!ctrl->border || !ctrl->widget || !ctrl->isVisible())
			continue;
		
		// Useless, as gTabStrip reimplements child() and childCount()
		//if (gtk_widget_get_parent(ctrl->border) != cont)
		//	continue;
		
		//fprintf(stderr, "get_next_child_widget: ==> %p\n", ctrl);
		return ctrl;
	}
	
	//fprintf(stderr, "get_next_child_widget: ==> NULL\n");
	return NULL; 
}

static void cb_arrange(gContainer *sender)
{
	if (sender->onArrange)
		(*(sender->onArrange))(sender);
}

static void resize_container(gControl *cont, int w, int h)
{
	if (w > 0 && h > 0)
		cont->resize(w, h);
}


#define WIDGET_TYPE gControl*
#define CONTAINER_TYPE gContainer*
#define ARRANGEMENT_TYPE gContainerArrangement*
#define IS_RIGHT_TO_LEFT() gDesktop::rightToLeft()
#define GET_WIDGET(_object) _object
#define GET_CONTAINER(_object) _object
#define GET_ARRANGEMENT(_object) (((gContainer*)_object)->getArrangement())
#define IS_EXPAND(_object) (((gControl*)_object)->expand())
#define IS_IGNORE(_object) (((gControl*)_object)->ignore())
#define IS_DESIGN(_object) (((gControl*)_object)->design())
#define IS_WIDGET_VISIBLE(_widget)  (((gControl*)_widget)->isVisible())

#define CAN_ARRANGE(_object) (IS_WIDGET_VISIBLE(GET_WIDGET(_object)) || (((gControl *)_object)->isTopLevel() && ((gMainWindow *)_object)->opened))

// BM: ClientX() & ClientY() are relative to the border.
// We need X & Y relative to the container widget.

#define GET_WIDGET_CONTENTS(_widget, _x, _y, _w, _h) _x=0; \
                                                     _y=0; \
                                                     _w=((gContainer*)_widget)->clientWidth(); \
                                                     _h=((gContainer*)_widget)->clientHeight()

#define GET_WIDGET_X(_widget)  (((gControl*)_widget)->left())
#define GET_WIDGET_Y(_widget)  (((gControl*)_widget)->top())
#define GET_WIDGET_W(_widget)  (((gControl*)_widget)->width())
#define GET_WIDGET_H(_widget)  (((gControl*)_widget)->height())
#define MOVE_WIDGET(_widget, _x, _y)  (((gControl*)_widget)->move( _x, _y))
#define RESIZE_WIDGET(_widget, _w, _h)  (((gControl*)_widget)->resize( _w, _h))
#define RESIZE_CONTAINER(_widget, _cont, _w, _h)  resize_container((gControl *)(_cont), _w, _h) 
#define MOVE_RESIZE_WIDGET(_widget, _x, _y, _w, _h) (((gControl*)_widget)->move( _x, _y, _w, _h))


#define INIT_CHECK_CHILDREN_LIST(_widget) \
	gContainer *gtk_control=(gContainer*)_widget; \
	int gtk_list=0; \
	int gtk_count = gtk_control->childCount(); \
	if (!gtk_count) return; 

#define RESET_CHILDREN_LIST() gtk_list=0;

#define GET_NEXT_CHILD_WIDGET() get_next_child_widget (gtk_control,&gtk_list,gtk_count)

#define GET_OBJECT_FROM_WIDGET(_widget) ((void*)_widget)

#define GET_OBJECT_NAME(_object) (((gControl *)_object)->name())

#define RAISE_ARRANGE_EVENT(_object) cb_arrange((gContainer *)_object);

#define FUNCTION_NAME arrangeContainer

#include "gb.form.arrangement.h"


int gContainer::_arrangement_level = 0;

void gContainer::performArrange()
{
	if (!gApplication::allEvents()) return;

	//fprintf(stderr, "gContainer::performArrange: %d: %s <<<\n", _arrangement_level, name());
	_arrangement_level++;	
	arrangeContainer((void*)this);
	_arrangement_level--;
	//fprintf(stderr, ">>> gContainer::performArrange: %d: %s\n", _arrangement_level, name());
}

void gContainer::initialize()
{
	ch_list = NULL;
	radiogroup = NULL;
	onArrange = NULL;
	_proxy = NULL;
	_client_w = 0;
	_client_h = 0;
	//onInsert = NULL;
	
	arrangement.mode=0;
	arrangement.spacing=0;
	arrangement.padding=0;
	arrangement.autoresize=false;
	arrangement.locked=false;
	arrangement.user=false;
}

gContainer::gContainer() 
{
	//g_print("gContainer() %d\n",this);
	initialize();
}

gContainer::gContainer(gContainer *parent) : gControl(parent)
{
	//g_print("gContainer(parent) %d par: %d\n",this,parent);
	initialize();
}

gContainer::~gContainer()
{
	GList *iter;
	
  // BM: Remove references to me
  
	iter = g_list_first(ch_list);
	while (iter)
	{
	  ((gControl*)iter->data)->removeParent();
		iter = iter->next;
	}
	
	if (radiogroup) { g_object_unref(G_OBJECT(radiogroup)); radiogroup=NULL; }
}

int gContainer::childCount()
{
	if (!ch_list) 
		return 0;
	else
		return g_list_length(ch_list);
}

gControl* gContainer::child(int index)
{
	GList *iter;
	
	if ( !ch_list ) return NULL;

	iter = g_list_nth(ch_list,index);
	if (!iter) return NULL;
	return (gControl*)iter->data;
}

int gContainer::arrange()
{
	return arrangement.mode;
}

void gContainer::setArrange(int vl)
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

int gContainer::padding()
{
	return arrangement.padding;
}

void gContainer::setPadding(int vl)
{
	if (vl >= 0 && vl <= 255) 
	{
		arrangement.padding=vl;
		performArrange();
	}
}

int gContainer::spacing()
{
	return arrangement.spacing;
}

void gContainer::setSpacing(int vl)
{
	if (vl >= 0 && vl <= 255) 
	{
		arrangement.spacing=vl;
		performArrange();
	}
}

bool gContainer::autoResize()
{
	return arrangement.autoresize;
}

void gContainer::setAutoResize(bool vl)
{
	arrangement.autoresize=vl;
	performArrange();
}

void gContainer::setUser(bool vl)
{
	arrangement.user = vl;
	performArrange();
}

int gContainer::clientX()
{
	gint xc, yc;
	GtkWidget *cont = getContainer();
	
	if (!cont->window || !border->window || cont == widget)
		return getFrameWidth();
	
// 	if (width() != widget->allocation.width || height() != widget->allocation.height)
// 	{
// 		updateGeometry();
// 		GtkAllocation a = { x(), y(), width(), height() };
// 		gtk_widget_size_allocate(widget, &a);
// 	}

	gtk_widget_translate_coordinates(cont, border, 0, 0, &xc, &yc);
	return xc; //+ getFrameWidth();
}

int gContainer::clientY()
{
	gint xc, yc;
	GtkWidget *cont = getContainer();
	
	if (!cont->window || !border->window || cont == widget)
		return getFrameWidth();
	
// 	if (width() != border->allocation.width || height() != border->allocation.height)
// 	{
// 		updateGeometry();
// 		GtkAllocation a = { x(), y(), width(), height() };
// 		gtk_widget_size_allocate(widget, &a);
// 	}

	gtk_widget_translate_coordinates(cont, border, 0, 0, &xc, &yc);
	
	/*if (getClass() == Type_gTabStrip && y < 16)
	fprintf(stderr, "clientY: cont = %p border = %p x = %d y = %d\n",
		cont, border, x, y);
	if (y & 0xFFFF0000)
		G_BREAKPOINT();*/

	return yc; // + getFrameWidth();
}

int gContainer::clientWidth()
{
	GtkWidget *cont = getContainer();
	
	if (_client_w > 0)
		return _client_w;
	
	if (cont != widget && cont->window)
	{
		if ((width() != widget->allocation.width || height() != widget->allocation.height)
		    && widget->allocation.width > 0 && widget->allocation.height > 0)
		{
			//g_debug("clientWidth: %s: %d", name(), width());
			GtkAllocation a = { x(), y(), width(), height() };
			//gt_disable_warnings(true);
			gtk_widget_size_allocate(widget, &a);
			//gt_disable_warnings(false);
		}
		//g_debug("ClientWidth: %s -> %d", this->name(), cont->allocation.width);
		if (cont->allocation.width > 0)
			return cont->allocation.width;
	}
	
	if (GTK_IS_SCROLLED_WINDOW(border))
		return (int)gtk_scrolled_window_get_hadjustment(GTK_SCROLLED_WINDOW(border))->page_size;
	
	return width() - getFrameWidth() - clientX();
}

int gContainer::clientHeight()
{
	GtkWidget *cont = getContainer();
	
	if (_client_h > 0)
		return _client_h;
	
	if (cont != widget && cont->window)
	{
		if ((width() != widget->allocation.width || height() != widget->allocation.height)
		    && widget->allocation.width > 0 && widget->allocation.height > 0)
		{
			//g_debug("clientHeight: %s: %d", name(), height());
			GtkAllocation a = { x(), y(), width(), height() };
			//gt_disable_warnings(true);
			gtk_widget_size_allocate(widget, &a);
			//gt_disable_warnings(false);
			//gtk_container_resize_children(GTK_CONTAINER(widget));
		}
		//g_debug("ClientHeight: %s -> %d", this->name(), cont->allocation.height);
		if (cont->allocation.height > 0)
			return cont->allocation.height;
	}
	
	if (GTK_IS_SCROLLED_WINDOW(border))
		return (int)gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(border))->page_size;
	
	return height() - getFrameWidth() - clientY();
}

void gContainer::insert(gControl *child)
{
	if (!gtk_widget_get_parent(child->border))
		gtk_layout_put(GTK_LAYOUT(getContainer()), child->border, 0, 0);
		
	ch_list = g_list_append(ch_list, child);
	
	//g_debug("gContainer::insert: visible = %d", isReallyVisible());
	performArrange();
}

void gContainer::remove(gControl *child)
{
	ch_list = g_list_remove(ch_list, child);
}


gControl *gContainer::find(int x, int y)
{
	GList *iter;
	gControl *child;
	
	if (!ch_list) 
		return NULL;
	
	iter = g_list_first(ch_list);
	while (iter)
	{
		child = (gControl *)iter->data;
		if (x >= child->left() && y >= child->top() && x < (child->left() + child->width()) && y < (child->top() + child->height()))
			return child;
		iter = iter->next;
	}
	
	return NULL;
}


void gContainer::setBackground(gColor color)
{
	GList *iter;
	gControl *child;
	
	gControl::setBackground(color);
	
	if (!ch_list) 
		return;
	
	iter = g_list_first(ch_list);
	while (iter)
	{
		child = (gControl *)iter->data;
		if (!child->bg_set)
			child->setBackground();
		iter = iter->next;
	}	
}

void gContainer::setForeground(gColor color)
{
	GList *iter;
	gControl *child;
	
	gControl::setForeground(color);
	
	if (!ch_list) 
		return;
	
	iter = g_list_first(ch_list);
	while (iter)
	{
		child = (gControl *)iter->data;
		if (!child->fg_set)
			child->setForeground();
		iter = iter->next;
	}	
}

GtkWidget *gContainer::getContainer()
{
	return widget;
}

gControl *gContainer::findFirstFocus()
{
	GList *iter;
	gControl *child;
	
	if (!ch_list)
		return NULL;
	
	iter = g_list_first(ch_list);
	while (iter)
	{
		child = (gControl *)iter->data;
		if (child->isContainer())
		{
			child = ((gContainer *)child)->findFirstFocus();
			if (child)
				return child;
		}
		else
		{
			if (GTK_WIDGET_CAN_FOCUS(child->widget) && !((child->getClass() == Type_gButton) && ((gButton *)child)->hasShortcut()))
				return child;
		}
		
		iter = iter->next;
	}
	
	return NULL;
}

void gContainer::resize(int w, int h)
{
	_client_w = 0;
	_client_h = 0;
	
	//if (w == bufW && h == bufH)
	//	return;
		
	gControl::resize(w, h);
	performArrange();
}

void gContainer::setVisible(bool vl)
{
	bool arr = vl && !isVisible();
	gControl::setVisible(vl);
	if (arr)
		performArrange();
}

