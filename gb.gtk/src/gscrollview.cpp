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
#include <gtk/gtk.h>

void gSV_scroll(GtkAdjustment *Adj,gScrollView *data)
{
	if (data->onScroll) data->onScroll(data);
}

gScrollView::gScrollView(gControl *parent) : gContainer(parent)
{
	GtkAdjustment* Adj;

	g_typ=Type_gScrollView;
	
	border=gtk_scrolled_window_new(NULL,NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(border),GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);
	widget=gtk_layout_new(0,0);
	gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(border),widget);
	
	connectParent();
	initSignals();
	
	Adj=gtk_scrolled_window_get_hadjustment(GTK_SCROLLED_WINDOW(border));
	g_signal_connect(G_OBJECT(Adj),"value-changed",G_CALLBACK(gSV_scroll),this);
	Adj=gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(border));
	g_signal_connect(G_OBJECT(Adj),"value-changed",G_CALLBACK(gSV_scroll),this);

}

long gScrollView::foreGround()
{
	return get_gdk_fg_color(widget);
}

void gScrollView::setForeGround(long color)
{	
	set_gdk_fg_color(widget,color);
	if (!border->window) gtk_widget_realize(border);
	gdk_window_process_updates(border->window,true);
}

long gScrollView::backGround()
{
	return get_gdk_bg_color(border);
}

void gScrollView::setBackGround(long color)
{	
	GtkWidget *lbl;
	
	set_gdk_bg_color(border,color);
	set_gdk_bg_color(widget,color);
	if (!border->window) gtk_widget_realize(border);
	gdk_window_process_updates(border->window,true);
}

long gScrollView::scrollBar()
{
	GtkPolicyType h,v;
	long ret=3;
	
	gtk_scrolled_window_get_policy(GTK_SCROLLED_WINDOW(border),&h,&v);
	if (h==GTK_POLICY_NEVER) ret--;
	if (v==GTK_POLICY_NEVER) ret-=2;
	
	return ret;
}

void gScrollView::setScrollBar(long vl)
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

int gScrollView::getBorder()
{
	switch (gtk_scrolled_window_get_shadow_type(GTK_SCROLLED_WINDOW(border)))
	{
		case GTK_SHADOW_NONE: return BORDER_NONE;
		case GTK_SHADOW_ETCHED_OUT: return BORDER_PLAIN;
		case GTK_SHADOW_IN: return BORDER_SUNKEN;
		case GTK_SHADOW_OUT: return BORDER_RAISED;
		case GTK_SHADOW_ETCHED_IN: return BORDER_ETCHED;
	}
	
	return BORDER_NONE;
}

void gScrollView::setBorder(int vl)
{
	GtkScrolledWindow *wr=GTK_SCROLLED_WINDOW(border);
	
	switch(vl)
	{
		case BORDER_NONE:
			gtk_scrolled_window_set_shadow_type(wr,GTK_SHADOW_NONE);
			break;
		case BORDER_PLAIN:
			gtk_scrolled_window_set_shadow_type(wr,GTK_SHADOW_ETCHED_OUT);
			break;
		case BORDER_SUNKEN:
			gtk_scrolled_window_set_shadow_type(wr,GTK_SHADOW_IN);
			break;
		case BORDER_RAISED:
			gtk_scrolled_window_set_shadow_type(wr,GTK_SHADOW_OUT);
			break;
		case BORDER_ETCHED:
			gtk_scrolled_window_set_shadow_type(wr,GTK_SHADOW_ETCHED_IN);
			break;
		default: return;
	}
}

long gScrollView::width()
{
	return bufW;
}

long gScrollView::height()
{
	return bufH;
}

void gScrollView::setWidth(long w)
{
	resize(w,height());
}

void gScrollView::setHeight(long h)
{
	resize(width(),h);
}

void gScrollView::move(long x,long y)
{
	GtkLayout *fx;

	if ( (x==this->bufX) && (y==this->bufY) ) return;

	this->bufX=x;
	this->bufY=y;
	
	fx=GTK_LAYOUT(pr->widget);
	gtk_layout_move(fx,border,x,y);
	((gContainer*)parent())->performArrange();
}

void gScrollView::performArrange()
{
	long bucle;
	long minX=0,maxX=0,minY=0,maxY=0;
	

	gContainer::performArrange();
	for (bucle=0;bucle<childCount();bucle++)
	{
		if (!child(bucle)->visible()) continue;
		if (child(bucle)->left()<0)
			if (child(bucle)->left()<minX)
				minX=child(bucle)->left();
				
		if ( (child(bucle)->width()+child(bucle)->left())>maxX )
			maxX=child(bucle)->width()+child(bucle)->left();
			
		if (child(bucle)->top()<0)
			if (child(bucle)->top()<minY)
				minY=child(bucle)->top();
				
		if ( (child(bucle)->height()+child(bucle)->top())>maxY )
			maxY=child(bucle)->height()+child(bucle)->top();
	}
	
	gtk_widget_set_size_request(widget,maxX-minX,maxY-minY);
}

void gScrollView::resize(long w,long h)
{
	GList *chd;
	long bucle;
	long minX=0,maxX=0,minY=0,maxY=0;
	
	if (w<1) w=1;
	if (h<1) h=1;	

	this->bufW=w;
	this->bufH=h;
	
	for (bucle=0;bucle<childCount();bucle++)
	{
		if (!child(bucle)->visible()) continue;
		if (child(bucle)->left()<0)
			if (child(bucle)->left()<minX)
				minX=child(bucle)->left();
				
		if ( (child(bucle)->width()+child(bucle)->left())>maxX )
			maxX=child(bucle)->width()+child(bucle)->left();
			
		if (child(bucle)->top()<0)
			if (child(bucle)->top()<minY)
				minY=child(bucle)->top();
				
		if ( (child(bucle)->height()+child(bucle)->top())>maxY )
			maxY=child(bucle)->height()+child(bucle)->top();
	}
	
	gtk_widget_set_size_request(border,w,h);
	gtk_widget_set_size_request(widget,maxX-minX,maxY-minY);
	performArrange();
	((gContainer*)parent())->performArrange();
}

long gScrollView::scrollX()
{
	GtkAdjustment* Adj;
	
	Adj=gtk_scrolled_window_get_hadjustment(GTK_SCROLLED_WINDOW(border));
	return (long)Adj->value;
}

void gScrollView::setScrollX(long vl)
{
	GtkAdjustment* Adj;
	
	Adj=gtk_scrolled_window_get_hadjustment(GTK_SCROLLED_WINDOW(border));
	if (vl<0) vl=0;
	if (vl>(long)(Adj->upper-Adj->page_size)) vl=(long)(Adj->upper-Adj->page_size);
	gtk_adjustment_set_value(Adj,(gdouble)vl);
}

long gScrollView::scrollY()
{
	GtkAdjustment* Adj;
	
	Adj=gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(border));
	return (long)Adj->value;
}

void gScrollView::setScrollY(long vl)
{
	GtkAdjustment* Adj;
	
	Adj=gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(border));
	if (vl<0) vl=0;
	if (vl>(long)(Adj->upper-Adj->page_size)) vl=(long)(Adj->upper-Adj->page_size);
	gtk_adjustment_set_value(Adj,(gdouble)vl);
}

void gScrollView::scroll(long x,long y)
{
	setScrollX(x);
	setScrollY(y);
}

long gScrollView::clientX()
{
	return 2;
}

long gScrollView::clientY()
{
	return 2;
}

long gScrollView::clientWidth()
{
	GtkAdjustment* Adj;
	
	Adj=gtk_scrolled_window_get_hadjustment(GTK_SCROLLED_WINDOW(border));
	return (long)Adj->page_size;
}

long gScrollView::clientHeight()
{
	GtkAdjustment* Adj;
	
	Adj=gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(border));
	return (long)Adj->page_size;
}

void gScrollView::ensureVisible(long x,long y,long w,long h)
{
	GtkAdjustment* Adj;
	GtkWidget *lbl=gtk_button_new_with_label("JE");
	long extX,extY;
	long px,py;
	
	Adj=gtk_scrolled_window_get_hadjustment(GTK_SCROLLED_WINDOW(border));
	extX=Adj->value+clientWidth();
	px=Adj->value;
	
	Adj=gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(border));
	extY=Adj->value+clientHeight();
	py=Adj->value;
	
	if ( (x+w)>extX ) px+= (w+x-extX);
	if ( (y+h)>extY ) py+= (h+y-extY);
	
	scroll(px,py);
}
