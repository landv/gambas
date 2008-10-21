/***************************************************************************

  gscrollview.cpp

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
#include "gscrollview.h"

static void cb_scroll(GtkAdjustment *Adj,gScrollView *data)
{
	if (data->onScroll) data->onScroll(data);
}

static void cb_inside_resize(GtkWidget *wid, GtkAllocation *a, gScrollView *data)
{
	data->performArrange();
}

gScrollView::gScrollView(gContainer *parent) : gContainer(parent)
{
	GtkAdjustment* Adj;

	g_typ=Type_gScrollView;
	
	_mw = _mh = 0;
	
	_scroll = gtk_scrolled_window_new(NULL,NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(_scroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  
  //gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(border), GTK_SHADOW_ETCHED_IN);	
	//gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(border), widget);
	//gtk_container_add(GTK_CONTAINER(border), widget);
	
  viewport = gtk_viewport_new(gtk_scrolled_window_get_hadjustment(GTK_SCROLLED_WINDOW(_scroll)),
                              gtk_scrolled_window_get_vadjustment (GTK_SCROLLED_WINDOW(_scroll)));
  
  gtk_viewport_set_shadow_type(GTK_VIEWPORT(viewport), GTK_SHADOW_NONE);
  gtk_container_add(GTK_CONTAINER(_scroll), viewport);
  
	border = _scroll; 
  widget = gtk_layout_new(0,0);
	realize(false);
	
	setBorder(true);
	
	Adj=gtk_scrolled_window_get_hadjustment(GTK_SCROLLED_WINDOW(_scroll));
	g_signal_connect(G_OBJECT(Adj),"value-changed",G_CALLBACK(cb_scroll),this);
	Adj=gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(_scroll));
	g_signal_connect(G_OBJECT(Adj),"value-changed",G_CALLBACK(cb_scroll),this);
	g_signal_connect(G_OBJECT(viewport), "size-allocate", G_CALLBACK(cb_inside_resize), (gpointer)this);
}


bool gScrollView::hasBorder()
{
	if (gtk_scrolled_window_get_shadow_type(GTK_SCROLLED_WINDOW(border))==GTK_SHADOW_NONE) return false;
	return true;
}

void gScrollView::setBorder(bool vl)
{
	if (vl)
		gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(border),GTK_SHADOW_IN);
	else
		gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(border),GTK_SHADOW_NONE);
}

void gScrollView::updateSize()
{
	int i, p;
	int ww, hh;
	gControl *ch;
	
	ww = hh = 0;
	
	for (i = 0; i < childCount(); i++)
	{
		ch = child(i);
		if (!ch->isVisible()) continue;
		
		p = ch->left() + ch->width();
		if (p > ww)
			ww = p;
			
		p = ch->top() + ch->height();
		if (p > hh)
			hh = p;
	}
	
	_mw = ww;
	ww = width() - getFrameWidth();
	if (_mw < ww)
		_mw = ww;
	_mh = hh;
	hh = height() - getFrameWidth();
	if (_mh < hh)
		_mh = hh;
	
	gtk_widget_set_size_request(widget, _mw, _mh);
}

void gScrollView::performArrange()
{
	gContainer::performArrange();
	updateSize();
}

void gScrollView::resize(int w, int h)
{
	if (w == bufW && h == bufH)
		return;
	
	gContainer::resize(w, h);
	
	if (bufW == 0 || bufH == 0)
		return;
	
	//updateSize();
	
	//performArrange();
	//parent()->performArrange();
}

void gScrollView::ensureVisible(int x, int y, int w, int h)
{
	if (!GTK_IS_SCROLLED_WINDOW(border))
		return;
	
	GtEnsureVisible arg;
	
	arg.clientWidth = clientWidth();
	arg.clientHeight = clientHeight();
	arg.scrollX = scrollX();
	arg.scrollY = scrollY();
	arg.scrollWidth = scrollWidth();
	arg.scrollHeight = scrollHeight();
	
	gt_ensure_visible(&arg, x, y, w, h);
	
	scroll(arg.scrollX, arg.scrollY);
}
