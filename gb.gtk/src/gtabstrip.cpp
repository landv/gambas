/***************************************************************************

  gtabstrip.cpp

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
#include "gapplication.h"
#include "gmouse.h"
#include "gtabstrip.h"


static int gTabStrip_buttonRelease(GtkWidget *wid,GdkEventButton *e,gTabStrip *d)
{
	if (!gApplication::userEvents()) return false;

	if (d->onMouseEvent)
	{
		gMouse::validate();
		gMouse::setMouse((int)e->x, (int)e->y, 0, e->state);
		d->onMouseEvent(d,gEvent_MouseRelease);
		gMouse::invalidate();
	}
	
	return false;
}

static int gTabStrip_buttonPress(GtkWidget *wid,GdkEventButton *e,gTabStrip *d)
{
	if (!gApplication::userEvents()) return false;

	if (d->onMouseEvent)
	{
		gMouse::validate();
		gMouse::setMouse((int)e->x, (int)e->y, e->button, e->state);
		d->onMouseEvent(d,gEvent_MousePress);
		gMouse::invalidate();
	
		if (e->button==3) d->onMouseEvent(d,gEvent_MouseMenu);
	}
	return false;
}

static void cb_click(GtkNotebook *nb,GtkNotebookPage *pg,guint pnum,gTabStrip *data)
{
	data->performArrange();
	data->emit(SIGNAL(data->onClick));
}

static void cb_size_allocate(GtkWidget *wid, GtkAllocation *alloc, gTabStrip *data)
{
	if (wid == data->getContainer() && (alloc->width != data->_client_w || alloc->height != data->_client_h))
	{
		data->_client_w = alloc->width;
		data->_client_h = alloc->height;
		//g_debug("gTabStripPage: %s: cb_size_allocate: %d %d", data->name(), data->_client_w, data->_client_h);
		data->performArrange();
	}
}


/****************************************************************************
	
	gTabStripPage

****************************************************************************/

class gTabStripPage
{
public:
	gTabStripPage(gTabStrip *tab);
	~gTabStripPage();

	char *text();
	void setText(char *text);
	gPicture *picture() { return _picture; }
	void setPicture(gPicture *picture);
	bool isVisible() { return _visible; }
	void setVisible(bool v);
	bool enabled();
	void setEnabled(bool v);
	int count();
	gControl *child(int n);
	void updateColors();
	void updateFont();

	GtkWidget *fix;
	GtkWidget *widget;
	GtkWidget *label;
	GtkWidget *image;
	GtkWidget *hbox;
	
	gPicture *_picture;
	gTabStrip *parent;
	bool _visible;
	int index;
};

gTabStripPage::gTabStripPage(gTabStrip *tab)
{
	parent = tab;
	
	widget = gtk_layout_new(0,0);
	
	//fix = gtk_event_box_new(); 
	
	hbox = gtk_hbox_new(false, 4);
	fix = hbox;
	//gtk_box_set_spacing(GTK_BOX(hbox), 4);
	//gtk_container_add(GTK_CONTAINER(fix), hbox);
	
	image = gtk_image_new();
	//gtk_container_add(GTK_CONTAINER(hbox), image);
	gtk_box_pack_start(GTK_BOX(hbox), image, FALSE, FALSE, 0);
	
	label = gtk_label_new_with_mnemonic("");
	//gtk_container_add(GTK_CONTAINER(hbox), label);
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
	//gtk_label_set_ellipsize(GTK_LABEL(label), PANGO_ELLIPSIZE_END);
	
	updateColors();
	updateFont();
	
	g_signal_connect(G_OBJECT(widget),"button-press-event",G_CALLBACK(gTabStrip_buttonPress),(gpointer)parent);
	g_signal_connect(G_OBJECT(widget),"button-release-event",G_CALLBACK(gTabStrip_buttonRelease),(gpointer)parent);
	g_signal_connect(G_OBJECT(fix),"button-press-event",G_CALLBACK(gTabStrip_buttonPress),(gpointer)parent);
	g_signal_connect(G_OBJECT(fix),"button-release-event",G_CALLBACK(gTabStrip_buttonRelease),(gpointer)parent);
	g_signal_connect_after(G_OBJECT(widget), "size-allocate", G_CALLBACK(cb_size_allocate), (gpointer)parent);	
	
	g_object_ref(widget);
	g_object_ref(fix);
	
	_visible = false;
	_picture = NULL;
	
	if (parent->count())
		index = parent->get(parent->count() - 1)->index + 1;
	else
		index = 0;
	
	gtk_widget_hide(image);
	
	setVisible(true);
}

gTabStripPage::~gTabStripPage()
{
	setVisible(false);
	gPicture::assign(&_picture);
	g_object_unref(fix);
	g_object_unref(widget);
}

void gTabStripPage::updateColors()
{
	set_gdk_bg_color(widget, parent->realBackground());
	//set_gdk_bg_color(fix, parent->background());
	set_gdk_fg_color(label, parent->realForeground());
}

void gTabStripPage::updateFont()
{
	gFont *fnt = parent->font();
	gtk_widget_modify_font(widget, fnt ? fnt->desc() : NULL);
	gtk_widget_modify_font(label, fnt ? fnt->desc() : NULL);
}

void gTabStripPage::setText(char *text)
{
	char *buf;

	gMnemonic_correctText(text, &buf);
	gtk_label_set_text_with_mnemonic(GTK_LABEL(label), buf);
	g_free(buf);
}

char *gTabStripPage::text()
{
	char *buf;

	gMnemonic_returnText((char*)gtk_label_get_text(GTK_LABEL(label)), &buf);
	gt_free_later(buf);
	return buf;
}

void gTabStripPage::setPicture(gPicture *picture)
{
	GdkPixbuf *buf;
	
	gPicture::assign(&_picture, picture);
	
	buf = _picture ? _picture->getPixbuf() : NULL;
	
	if (!buf)
		gtk_widget_hide(image);
	else
	{
		gtk_image_set_from_pixbuf(GTK_IMAGE(image), buf);
		gtk_widget_show(image);
	}
}

bool gTabStripPage::enabled()
{
	return GTK_WIDGET_SENSITIVE(hbox);
}

void gTabStripPage::setEnabled(bool v)
{
	gtk_widget_set_sensitive(label, v);
	gtk_widget_set_sensitive(image, v);
	gtk_widget_set_sensitive(widget, v);
}

void gTabStripPage::setVisible(bool v)
{
	int ind, n;
	gTabStripPage *page;
	
	if (_visible == v)
		return;
		
	_visible = v;
	
	if (v)
	{
	  n = 0;
		for (ind = 0; ind < parent->count(); ind++)
		{
			page = parent->get(ind);
			if (index <= page->index)
				break;
			if (page->isVisible())
				n++;
		}
		gtk_notebook_insert_page(GTK_NOTEBOOK(parent->border), widget, fix, n);
		gtk_widget_realize(widget);
		gtk_widget_realize(fix);
		gtk_widget_show_all(widget);
		gtk_widget_show_all(fix);
		//gtk_container_resize_children(GTK_CONTAINER(gtk_widget_get_parent(widget)));
		gtk_container_resize_children(GTK_CONTAINER(gtk_widget_get_parent(gtk_widget_get_parent(widget))));
	}
	else
	{
		ind = gtk_notebook_page_num(GTK_NOTEBOOK(parent->border), widget);
		gtk_notebook_remove_page(GTK_NOTEBOOK(parent->border), ind);
	}
}

int gTabStripPage::count()
{
	int ct = 0;
	GList *iter;
	gControl *child;
	
	if (!parent->ch_list) return 0;
	
	iter = g_list_first(parent->ch_list);
	while (iter)
	{
		child = (gControl *)iter->data;
		if (gtk_widget_get_parent(child->border) == widget)
			ct++;
		iter=iter->next;
	}
	
	return ct;
}


gControl *gTabStripPage::child(int n)
{
	int ct = 0;
	GList *iter;
	gControl *child;
	
	if (!parent->ch_list) return NULL;
	
	iter = g_list_first(parent->ch_list);
	while (iter)
	{
		child = (gControl *)iter->data;
		if (gtk_widget_get_parent(child->border) == widget)
		{
			if (ct == n)
				return child;
			ct++;
		}
		iter=iter->next;
	}
	
	return NULL;
}


/****************************************************************************
	
	gTabStrip

****************************************************************************/

gTabStrip::gTabStrip(gContainer *parent) : gContainer(parent)
{
	g_typ=Type_gTabStrip;
	_pages = g_ptr_array_new();
	onClick=NULL;
	
	widget = gtk_notebook_new();
	gtk_notebook_set_scrollable(GTK_NOTEBOOK(widget), TRUE);
	gtk_drag_dest_unset(widget);
	
	realize();

	setCount(1);
	
	/*ebox=gtk_event_box_new();
	hbox=gtk_hbox_new(false,0);
	lbl=gtk_label_new_with_mnemonic("");
	gtk_container_add(GTK_CONTAINER(ebox),hbox);
	gtk_container_add(GTK_CONTAINER(hbox),lbl);
	gtk_widget_show_all(ebox);
	gtk_notebook_append_page(GTK_NOTEBOOK(border),widget,ebox);
	g_signal_connect(G_OBJECT(ebox),"button-press-event",G_CALLBACK(gTabStrip_buttonPress),(gpointer)this);
	g_signal_connect(G_OBJECT(ebox),"button-release-event",G_CALLBACK(gTabStrip_buttonRelease),(gpointer)this);
  */
  
	gtk_widget_add_events(widget,GDK_BUTTON_RELEASE_MASK);
	g_signal_connect_after(G_OBJECT(border), "switch-page", G_CALLBACK(cb_click), (gpointer)this);
}

gTabStrip::~gTabStrip()
{
	// Do not use setCount() or removeTab(), because they cannot remove non-empty tabs
	lock();
	while (count())
		destroyTab(count() - 1);
	unlock();
	g_ptr_array_free(_pages, TRUE);
}


int gTabStrip::getRealIndex(GtkWidget *page)
{
	int i;
	
	for (i = 0; i < count(); i++)
	{
		if (get(i)->widget == page)
			return i;
	}
	
	return -1;
}

int gTabStrip::index()
{
	GtkWidget *page = gtk_notebook_get_nth_page(GTK_NOTEBOOK(border), gtk_notebook_get_current_page(GTK_NOTEBOOK(border)));
	return getRealIndex(page);
}

void gTabStrip::setIndex(int vl)
{
	GtkWidget *page;
	
	if ( (vl<0) || (vl>=count()) || !get(vl)->isVisible() ) return;
	
	page = get(vl)->widget;
	gtk_notebook_set_current_page(GTK_NOTEBOOK(border), getRealIndex(page));
	//widget = get(vl)->widget;
}

int gTabStrip::orientation()
{
	switch ( gtk_notebook_get_tab_pos(GTK_NOTEBOOK(border)) )
	{
		case GTK_POS_TOP: return 0;
		default: return 1;
	}
}

void gTabStrip::destroyTab(int ind)
{
	delete (gTabStripPage *)g_ptr_array_index(_pages, ind);
	g_ptr_array_remove_index(_pages, ind);
}

bool gTabStrip::removeTab(int ind)
{
	if (get(ind)->count())
		return true;
		
	destroyTab(ind);
	return false;
}

bool gTabStrip::setCount(int vl)
{
	int i, ind;

	if (vl == count())
		return false;

	ind = index();

	lock();
	while (vl > count())
		g_ptr_array_add(_pages, (gpointer)new gTabStripPage(this));
	unlock();
	
	if (vl < count())
	{
		//GList *chd,*iter,*ok=NULL;
		/*chd=gtk_container_get_children(GTK_CONTAINER(border));
		iter=chd;
		while (iter)
		{
			if (gtk_notebook_page_num(GTK_NOTEBOOK(border),GTK_WIDGET(iter->data))>=vl)
			{
				ok=gtk_container_get_children(GTK_CONTAINER(iter->data));
				if (ok) break;
			}
			iter=g_list_next(iter);
		}
		g_list_free(chd);
		if (ok) return -1;*/
		
		for (i = vl; i < count(); i++)
		{
			if (get(i)->count())
				return true;
		}
		
		lock();
		while (vl < count())
			removeTab(count() - 1);
		unlock();
	}
	
	if (ind != index())
		emit(SIGNAL(onClick));
	
	return false;
}

void gTabStrip::setOrientation(int vl)
{
	switch (vl)
	{
		case 0:
			gtk_notebook_set_tab_pos(GTK_NOTEBOOK(border),GTK_POS_TOP); break;
		case 1:
			gtk_notebook_set_tab_pos(GTK_NOTEBOOK(border),GTK_POS_BOTTOM); break;
	}
}

gPicture* gTabStrip::tabPicture(int ind)
{
	if ( (ind<0) || (ind>=count()) ) 
		return NULL;
	else
		return get(ind)->picture();
}

void gTabStrip::setTabPicture(int ind,gPicture *pic)
{
	if ( (ind<0) || (ind>=count()) ) return;	
	get(ind)->setPicture(pic);
}

bool gTabStrip::tabEnabled(int ind)
{
	if ( (ind<0) || (ind>=count()) ) 
		return FALSE;
	else
		return get(ind)->enabled();
}

void gTabStrip::setTabEnabled(int ind, bool vl)
{
	if ( (ind<0) || (ind>=count()) ) return;	
	get(ind)->setEnabled(vl);
}

bool gTabStrip::tabVisible(int ind)
{
	if ( (ind<0) || (ind>=count()) ) 
		return FALSE;
	else
		return get(ind)->isVisible();
}

void gTabStrip::setTabVisible(int ind, bool vl)
{
	if ( (ind<0) || (ind>=count()) ) return;	
	get(ind)->setVisible(vl);
}

char* gTabStrip::tabText(int ind)
{
	if ( (ind<0) || (ind>=count()) ) 
		return NULL;
	else
		return get(ind)->text();
}

void gTabStrip::setTabText(int ind, char *txt)
{
	if ( (ind<0) || (ind>=count()) ) return;	
	get(ind)->setText(txt);
}

int gTabStrip::tabCount(int ind)
{
	if ( (ind<0) || (ind>=count()) ) 
		return 0;
	else
		return get(ind)->count();
}

gControl *gTabStrip::tabChild(int ind, int n)
{
	if ( (ind<0) || (ind>=count()) ) 
		return NULL;
	else
		return get(ind)->child(n);
}

int gTabStrip::childCount()
{
	return tabCount(index());
}

gControl *gTabStrip::child(int ind)
{
	return tabChild(index(), ind);
}

GtkWidget *gTabStrip::getContainer()
{
	gTabStripPage *page = get(index());
	
	if (page)
		return page->widget;
	else
		return widget;
}

void gTabStrip::setRealBackground(gColor color)
{
	gControl::setRealBackground(color);
	
	for (int i = 0; i < count(); i++)
		get(i)->updateColors();
}

void gTabStrip::setRealForeground(gColor color)
{
	gControl::setRealForeground(color);
	
	for (int i = 0; i < count(); i++)
		get(i)->updateColors();
}

gTabStripPage *gTabStrip::get(int ind)
{
	if (ind < 0 || ind >= count())
		return NULL;
	else 
		return (gTabStripPage *)g_ptr_array_index(_pages, ind);
}

void gTabStrip::setFont(gFont *font)
{
	int i;
	
	gControl::setFont(font);
	
	for (i = 0; i < count(); i++)
		get(i)->updateFont();	
}
