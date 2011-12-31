/***************************************************************************

  gcontainer.cpp

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

static void cb_before_arrange(gContainer *sender)
{
	if (sender->onBeforeArrange)
		(*(sender->onBeforeArrange))(sender);
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

#define GET_WIDGET_CONTENTS(_widget, _x, _y, _w, _h) _x=((gContainer*)_widget)->containerX(); \
                                                     _y=((gContainer*)_widget)->containerY(); \
                                                     _w=((gContainer*)_widget)->clientWidth(); \
                                                     _h=((gContainer*)_widget)->clientHeight()

#define GET_WIDGET_X(_widget)  (((gControl*)_widget)->left())
#define GET_WIDGET_Y(_widget)  (((gControl*)_widget)->top())
#define GET_WIDGET_W(_widget)  (((gControl*)_widget)->width())
#define GET_WIDGET_H(_widget)  (((gControl*)_widget)->height())
#define MOVE_WIDGET(_object, _widget, _x, _y)  (((gControl*)_widget)->move( _x, _y))
#define RESIZE_WIDGET(_object, _widget, _w, _h)  (((gControl*)_widget)->resize( _w, _h))
#define MOVE_RESIZE_WIDGET(_object, _widget, _x, _y, _w, _h) (((gControl*)_widget)->move( _x, _y, _w, _h))
#define RESIZE_CONTAINER(_widget, _cont, _w, _h)  resize_container((gControl *)(_cont), _w, _h) 

#define INIT_CHECK_CHILDREN_LIST(_widget) \
	gContainer *gtk_control=(gContainer*)_widget; \
	int gtk_list=0; \
	int gtk_count = gtk_control->childCount();

#define HAS_CHILDREN() (gtk_count != 0)

#define RESET_CHILDREN_LIST() gtk_list=0;

#define GET_NEXT_CHILD_WIDGET() get_next_child_widget (gtk_control,&gtk_list,gtk_count)

#define GET_OBJECT_FROM_WIDGET(_widget) ((void*)_widget)

#define GET_OBJECT_NAME(_object) (((gControl *)_object)->name())

#define RAISE_ARRANGE_EVENT(_object) cb_arrange((gContainer *)_object);
#define RAISE_BEFORE_ARRANGE_EVENT(_object) cb_before_arrange((gContainer *)_object);

#define DESKTOP_SCALE gDesktop::scale()

#define FUNCTION_NAME arrangeContainer

#include "gb.form.arrangement.h"


int gContainer::_arrangement_level = 0;

void gContainer::performArrange()
{
	if (!gApplication::allEvents()) return;

	//if (!CAN_ARRANGE(this))
	//	return;
	
	//fprintf(stderr, "gContainer::performArrange: %d: %s <<<\n", _arrangement_level, name());
	//_arrangement_level++;	
	arrangeContainer((void*)this);
	//_arrangement_level--;
	//fprintf(stderr, ">>> gContainer::performArrange: %d: %s\n", _arrangement_level, name());
}

void gContainer::initialize()
{
	ch_list = NULL;
	radiogroup = NULL;
	onArrange = NULL;
	onBeforeArrange = NULL;
	_proxyContainer = NULL;
	_client_x = -1;
	_client_y = -1;
	_client_w = 0;
	_client_h = 0;
	//onInsert = NULL;
	
	arrangement.mode = 0;
	arrangement.spacing = 0;
	arrangement.padding = 0;
	arrangement.autoresize = false;
	arrangement.locked = false;
	arrangement.user = false;
	arrangement.margin = false;
	arrangement.indent = 0;
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
			if (vl != arrangement.mode)
			{
				arrangement.mode = vl;
				performArrange();
			}
		default: 
			break;
	}
}

int gContainer::padding()
{
	return arrangement.padding;
}

void gContainer::setPadding(int vl)
{
	if (vl >= 0 && vl <= 255 && vl != arrangement.padding) 
	{
		arrangement.padding = vl;
		performArrange();
	}
}

bool gContainer::spacing()
{
	return arrangement.spacing;
}

void gContainer::setSpacing(bool vl)
{
	if (vl != arrangement.spacing)
	{
		arrangement.spacing = vl;
		performArrange();
	}
}

bool gContainer::margin()
{
	return arrangement.margin;
}

void gContainer::setMargin(bool vl)
{
	if (vl != arrangement.margin)
	{
		arrangement.margin = vl;
		performArrange();
	}
}

int gContainer::indent()
{
	return arrangement.indent;
}

void gContainer::setIndent(int vl)
{
	if (vl < 0)
		vl = 1;
	if (vl >= 0 && vl <= 7 && vl != arrangement.indent) 
	{
		arrangement.indent = vl;
		performArrange();
	}
}

bool gContainer::autoResize()
{
	return arrangement.autoresize;
}

void gContainer::setAutoResize(bool vl)
{
	if (vl != arrangement.autoresize)
	{
		arrangement.autoresize = vl;
		performArrange();
	}
}

void gContainer::setUser(bool vl)
{
	if (vl != arrangement.user)
	{
		arrangement.user = vl;
		performArrange();
	}
}

int gContainer::clientX()
{
	gint xc, yc;
	GtkWidget *cont = getContainer();
	
	if (_client_x >= 0)
		return _client_x;
	
	if (cont->window && border->window)
	{
		gtk_widget_translate_coordinates(cont, border, 0, 0, &xc, &yc);
		xc += containerX();
	}
	else
		xc = getFrameWidth();
		
	return xc;
}

int gContainer::containerX()
{
	GtkWidget *cont = getContainer();
	
	if (cont == widget && widget == frame)
		return getFrameWidth();
	else
		return 0;
}

int gContainer::clientY()
{
	gint xc, yc;
	GtkWidget *cont = getContainer();
	
	if (_client_y >= 0)
		return _client_y;
	
	if (cont->window && border->window)
	{
		gtk_widget_translate_coordinates(cont, border, 0, 0, &xc, &yc);
		yc += containerY();
	}
	else
		yc = getFrameWidth();
		
	return yc;
}

#if 0
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
	if (cont == widget)
		yc += getFrameWidth();
	
	return yc; // + getFrameWidth();
}
#endif

int gContainer::containerY()
{
	GtkWidget *cont = getContainer();
	
	if (cont == widget && widget == frame)
		return getFrameWidth();
	else
		return 0;
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
	
	if (_scroll)
		return (int)gtk_scrolled_window_get_hadjustment(_scroll)->page_size;
	
	return width() - getFrameWidth() * 2;
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
	
	if (_scroll)
		return (int)gtk_scrolled_window_get_vadjustment(_scroll)->page_size;
	
	return height() - getFrameWidth() * 2;
}

void gContainer::insert(gControl *child, bool realize)
{
	if (!gtk_widget_get_parent(child->border))
	{
		//gtk_layout_put(GTK_LAYOUT(getContainer()), child->border, 0, 0);
		gtk_container_add(GTK_CONTAINER(getContainer()), child->border);
	}
	child->bufX = child->bufY = 0;
		
	ch_list = g_list_append(ch_list, child);
	
	if (realize)
	{
		child->visible = true;
	}
    
	//g_debug("gContainer::insert: visible = %d", isReallyVisible());
	//fprintf(stderr, "insert %s into %s\n", child->name(), name());
	performArrange();
	//fprintf(stderr, "--> %d %d %d %d\n", child->x(), child->y(), child->width(), child->height());
	updateFocusChain();

	if (realize)
	{
    gtk_widget_realize(child->border);
		gtk_widget_show_all(child->border);
	}
    
	if (hasBackground()) child->setBackground();
	if (hasForeground()) child->setForeground();
  if (hasFont()) child->setFont(font());
}

void gContainer::remove(gControl *child)
{
	ch_list = g_list_remove(ch_list, child);
	updateFocusChain();
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
		if (!child->_bg_set)
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
		if (!child->_fg_set)
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
	if (w == bufW && h == bufH)
		return;
		
	_client_w = 0;
	_client_h = 0;
	
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

void gContainer::updateFocusChain()
{
	GList *chain = NULL;
	GList *iter = ch_list;
	gControl *child;
	
	//fprintf(stderr, "updateFocusChain()\n");
	
	iter = g_list_first(ch_list);
	while (iter)
	{
		child = (gControl*)(iter->data);
		//fprintf(stderr, "%s\n", child->name());	
	  chain = g_list_prepend(chain, child->border);
		iter = iter->next;
	}
	
	chain = g_list_reverse(chain);
	
	gtk_container_set_focus_chain(GTK_CONTAINER(widget), chain);
	
	g_list_free(chain);
}

void gContainer::setFont(gFont *ft)
{
	GList *iter;
	gControl *child;

	gControl::setFont(ft);

	if (!ch_list) 
		return;
	
	iter = g_list_first(ch_list);
	while (iter)
	{
		child = (gControl *)iter->data;
		if (!child->_font_set)
			child->setFont(child->font());
		iter = iter->next;
	}	
}

void gContainer::moveChild(gControl *child, int x, int y)
{
	GtkWidget *parent = getContainer();
	
	if (GTK_IS_LAYOUT(parent))
		gtk_layout_move(GTK_LAYOUT(parent), child->border, x, y);
	else
		gtk_fixed_move(GTK_FIXED(parent), child->border, x, y);
}

bool gContainer::hasBackground() const
{
	return _bg_set || (parent() && parent()->hasBackground());
}

bool gContainer::hasForeground() const
{
	return _fg_set || (parent() && parent()->hasForeground());
}

bool gContainer::hasFont() const
{
	return _font_set || (parent() && parent()->hasFont());
}
