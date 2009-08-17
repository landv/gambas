/***************************************************************************

  gsplitter.cpp

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
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

***************************************************************************/

#include "widgets.h"
#include "widgets_private.h"
#include "gapplication.h"
#include "gsplitter.h"

// static gboolean notify_later(gSplitter *data)
// {
// 	data->performArrange();
// 	data->emit(SIGNAL(data->onResize));
// 	data->_notify = false;
// 	return false;
// }

static void cb_notify(GtkPaned *paned, GParamSpec *arg1, gSplitter *data)
{
	if (!strcmp(arg1->name, "position")) // && !data->_notify)
	{
		//data->performArrange();
		data->emit(SIGNAL(data->onResize));
		//data->_notify = true;
		//g_timeout_add(0, (GSourceFunc)notify_later, data);
	}
}

static void cb_size_allocate(GtkPaned *widget, GtkAllocation *allocation, gSplitter *data)
{
	//g_debug("gSplitter: cb_size_allocate");
	//data->performArrange();
	data->updateChild(gtk_paned_get_child1(widget));
}

static void cb_child_visibility(GtkWidget *widget, gSplitter *data)
{
	data->updateVisibility();
}
	
GtkPaned *gSplitter::next(GtkPaned *iter)
{
	GtkWidget *child;
	
	for(;;)
	{
		if (!iter)
			iter = GTK_PANED(border);
		else
			iter = (GtkPaned*)gtk_paned_get_child2(iter);
		
		if (!iter)
			return NULL;
			
		child = gtk_paned_get_child1(iter);
		if (child) // && gApplication::controlItem(child)->isVisible())
			return iter;
	}	
}

void gSplitter::updateVisibility()
{
	int i, n;
	GtkPaned *iter;
	
	// Hide internal GTK_PANED 
	
	n = childCount() - 1;
	while (n >= 0)
	{
		//fprintf(stderr, "child(%d)->isVisible() = %d\n", n, child(n)->isVisible());
		if (child(n)->isVisible())
			break;
		n--;
	}
	
  //fprintf(stderr, "updateVisibility: last visible is child #%d\n", n);
  
	for (i = 0; i <= n; i++)
	{
		iter = GTK_PANED(gtk_widget_get_parent(child(i)->border));
		gtk_widget_show(GTK_WIDGET(iter));
	}
	
	n++;
	
	if (n < childCount())
	{
		iter = GTK_PANED(gtk_widget_get_parent(child(n)->border));
		gtk_widget_hide(GTK_WIDGET(iter));
	}
	
	if (n >= (childCount() - 1))
		refresh(); // Workaround frame drawing bug when the last one is shown or hidden
	emit(SIGNAL(onResize));
}

static int get_child_width(GtkPaned *iter)
{
	GtkWidget *child = gtk_paned_get_child1(iter);
	if (gApplication::controlItem(child)->isVisible())
		return gtk_paned_get_position(iter);
	else
		return 0;
}

gSplitter::gSplitter(gContainer *parent, bool vert) : gContainer(parent)
{
	g_typ=Type_gSplitter;
	vertical = vert;
	
	border = vertical ? gtk_vpaned_new() : gtk_hpaned_new();
	curr = GTK_PANED(border);
	widget = border;
	
	onResize = NULL;
	_notify = false;
	
	realize();
	
	g_signal_connect_after(G_OBJECT(curr), "notify", G_CALLBACK(cb_notify), (gpointer)this);
	g_signal_connect_after(G_OBJECT(curr), "size-allocate", G_CALLBACK(cb_size_allocate), (gpointer)this);
}

void gSplitter::insert(gControl *child)
{	
	GtkWidget *w = child->border;
	GtkWidget *tmp;
	
	lock();
	
	if (!gtk_paned_get_child1(curr))
	{
		gtk_paned_pack1(curr, w, TRUE, TRUE);
	}
	else
	{	
		if (!vertical)
			tmp=gtk_hpaned_new();
		else
			tmp=gtk_vpaned_new();
			
		//gtk_widget_show_all(tmp);
		gtk_paned_pack2(curr, tmp, TRUE, TRUE);
		curr=GTK_PANED(tmp);
		gtk_paned_pack1(curr, w, TRUE, TRUE);	
		//g_signal_connect_after(G_OBJECT(curr),"notify",G_CALLBACK(slt_notify),(gpointer)this);
  	g_signal_connect_after(G_OBJECT(curr), "size-allocate", G_CALLBACK(cb_size_allocate), (gpointer)this);
		g_signal_connect_after(G_OBJECT(curr), "notify", G_CALLBACK(cb_notify), (gpointer)this);
	}
	
	g_signal_connect_after(G_OBJECT(w), "show", G_CALLBACK(cb_child_visibility), (gpointer)this);
	g_signal_connect_after(G_OBJECT(w), "hide", G_CALLBACK(cb_child_visibility), (gpointer)this);
	
	unlock();
	
	gContainer::insert(child);
	//updateVisibility();
	
	//emit(SIGNAL(onResize));
}

void gSplitter::remove(gControl *child)
{
	gContainer::remove(child);

	g_signal_handlers_disconnect_matched(G_OBJECT(child->border), G_SIGNAL_MATCH_DATA, 0, (GQuark)0, NULL, NULL, (gpointer)this);
}

int gSplitter::handleCount()
{
	int n, nh = -1;
	
	for (n = 0; n < childCount(); n++)
		if (child(n)->isVisible())
			nh++;
			
	return nh;
}

void gSplitter::setLayout(char *vl)
{
	char **split;
	char *sval;
	int i;
	long num;
	double factor;
	GtkPaned *iter;
	int shandle;
	
	if (!vl || !*vl) return;
	
	//fprintf(stderr, "setLayout: %s\n", vl);
  
  gtk_widget_style_get (border,
			"handle-size", &shandle,
			(void *)NULL);
	
	split=g_strsplit((const char*)vl, ",", -1);
	if (!split) return;
	 
	factor = 0;
	for (i = 0;; i++)
	{
		if (i >= childCount())
			break;
		if (child(i)->isVisible())
		{
			sval = split[i];
			if (!sval)
				break;
			errno = 0;
			num = strtol(sval, NULL, 10);
			if (errno || num < 1)
				num = 0;
			factor += num;
		}
	}
	
	if (factor == 0)
		return;
	
	factor = ((vertical ? height() : width()) - handleCount() * shandle) / factor;

	lock();
	
	iter = next(NULL);
	
	for (i = 0;; i++)
	{
	 	if (!iter) break;
		if (i >= childCount())
			break;
		
		if (child(i)->isVisible())
		{
			sval = split[i];
			if (!sval)
				break;
				
			errno = 0;
			num = strtol(sval,NULL,10);
			if (errno || num < 1)
				num = 0;
			gtk_paned_set_position(iter, (gint)(num * factor + 0.5));
		}
		
		iter = next(iter);
	}
	
	g_strfreev(split);
	
	unlock();
	
	emit(SIGNAL(onResize));
	
	//fprintf(stderr, "setLayout: layout = %s\n", layout());
}


char* gSplitter::layout()
{
	GtkPaned *iter;
	int vl, sum, nhandle, shandle;
	GString *ret = g_string_new("");
	char *l;
	
  gtk_widget_style_get (border,
			"handle-size", &shandle,
			(void *)NULL);
	
	iter = next(NULL);
	if (iter)
	{
		sum = 0;
		nhandle = 0;
		for(;;)
		{
			vl = get_child_width(iter); //gtk_paned_get_position(iter);
			iter = next(iter);
			if (!iter)
				break;
			if (vl)
				nhandle++;
			sum += vl;
			g_string_append_printf(ret, "%d,", vl);
		}
		
		if (childCount() > 1 && child(childCount() - 1)->isVisible())
			g_string_append_printf(ret, "%d", (vertical ? height() : width()) - sum - nhandle * shandle);
		else	
			g_string_append(ret, "0");
	}
		
	l = g_string_free(ret, false);
	gt_free_later(l);
	
	//fprintf(stderr, "layout: %s\n", l);
	
	if (!*l)
		return NULL;
	else
		return l;
}

#if 0
int gSplitter::childCount()
{
	GtkPaned *iter;
	int ret=0;
	
	if ( !gtk_paned_get_child1(GTK_PANED(border)) ) return 0;
	iter=GTK_PANED(border);
	
	while (iter)
	{
		ret++;
		iter=(GtkPaned*)gtk_paned_get_child2(iter);
	}
	
	return ret;
}

gControl* gSplitter::child(int index)
{
	GtkPaned *iter;
	int ret=0;
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
#endif

void gSplitter::updateChild(GtkWidget *w)
{
	int bucle, nchd;
	gControl *chd;

	nchd = childCount();
	for (bucle = 0; bucle < nchd; bucle++)
	{
		chd=child(bucle);
		
		if (w && chd->border != w)
			continue;
		
		chd->bufX = chd->border->allocation.x;
		chd->bufY = chd->border->allocation.y;
		chd->bufW = chd->border->allocation.width;
		chd->bufH = chd->border->allocation.height;
		chd->_dirty_pos = false;
		chd->_dirty_size = false;
		//chd->resize(chd->border->allocation.width, chd->border->allocation.height);
		//gApplication::setDirty();
		//g_debug("gSplitter::updateChild: %s -> (%d %d %d %d)", chd->name(), chd->x(), chd->y(), chd->width(), chd->height());
		if (chd->isContainer())
			((gContainer*)chd)->performArrange();
	}
}

void gSplitter::performArrange()
{
	updateChild();
	updateVisibility();
}

void gSplitter::resize(int w, int h)
{
	if (w == width() && h == height())
		return;
	
	//l = layout();	
	gContainer::resize(w, h);
	//setLayout(l);
}


