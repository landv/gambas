/***************************************************************************

  gtabstrip.cpp

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
#include "gmouse.h"
#include "gdesktop.h"
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
		data->_client_x = alloc->x;
		data->_client_y = alloc->y;
		data->_client_w = alloc->width;
		data->_client_h = alloc->height;
		data->performArrange();
	}
}

static bool cb_button_expose(GtkWidget *wid, GdkEventExpose *e, gTabStrip *data)
{
	GdkGC        *gc;
	GdkPixbuf    *img;
	GdkRectangle rpix={0,0,0,0};
	GdkRectangle rect={0,0,0,0};
	gint         py,px;
	bool         rtl,bcenter=false;
	gint dx, dy;

	rtl = gtk_widget_get_default_direction() == GTK_TEXT_DIR_RTL;

	rect = wid->allocation;
	px = rect.width;

	if (GTK_WIDGET_STATE(data->widget) == GTK_STATE_ACTIVE)
	{
	  gtk_widget_style_get (wid,
				"child-displacement-x", &dx,
				"child-displacement-y", &dy,
				(void *)NULL);
		rect.x += dx;
		rect.y += dy;
	}

	if (GTK_WIDGET_STATE(data->widget)==GTK_STATE_INSENSITIVE) 
	{
		if (!data->_button_pixbuf_disabled)
		{
			data->_button_pixbuf_disabled = gt_pixbuf_create_disabled(data->_button_pixbuf_normal);
			g_object_ref(G_OBJECT(data->_button_pixbuf_disabled));
		}
		img = data->_button_pixbuf_disabled;
	}
	else
		img = data->_button_pixbuf_normal;

	rpix.width = gdk_pixbuf_get_width(img);
	rpix.height = gdk_pixbuf_get_height(img);
	
	py = (rect.height - rpix.height)/2;
	
	gc = gdk_gc_new(wid->window);
	gdk_gc_set_clip_origin(gc,0,0);
	gdk_gc_set_clip_rectangle(gc,&e->area);

	bcenter = true; //!(data->text()) || !(*data->text());
	
	if (bcenter) 
	{	
		gdk_draw_pixbuf(GDK_DRAWABLE(wid->window),gc,img,0,0,rect.x + (px-rpix.width)/2, rect.y + py,
																			-1,-1,GDK_RGB_DITHER_MAX,0,0);
		g_object_unref(gc);
		return false;
	}

	if (rtl)
		gdk_draw_pixbuf(GDK_DRAWABLE(wid->window),gc,img,0,0,rect.x + rect.width - 6, rect.y + py,
																			-1,-1,GDK_RGB_DITHER_MAX,0,0);
	else
		gdk_draw_pixbuf(GDK_DRAWABLE(wid->window),gc,img,0,0,rect.x + 6, rect.y + py,
																			-1,-1,GDK_RGB_DITHER_MAX,0,0);

	g_object_unref(G_OBJECT(gc));
	
	//rect.width -= rpix.width;
	//rect.x += rpix.width;
	
	return FALSE;
}

static void cb_button_clicked(GtkWidget *wid, gTabStrip *data)
{
	if (data->onClose)
		(*data->onClose)(data, data->getRealIndex((GtkWidget *)g_object_get_data(G_OBJECT(wid), "gambas-tab-page")));
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
	void updateButton();

	GtkWidget *fix;
	GtkWidget *widget;
	GtkWidget *label;
	GtkWidget *image;
	GtkWidget *hbox;
	GtkWidget *_button;
	
	gPicture *_picture;
	gTabStrip *parent;
	bool _visible;
	int index;
};

gTabStripPage::gTabStripPage(gTabStrip *tab)
{
	char text[16];

	parent = tab;
	
	widget = gtk_fixed_new();
	
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
	
	_button = NULL;
	updateButton();
	
	sprintf(text, "Tab %d", index);
	setText(text);
	
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
	set_gdk_fg_color(label, parent->realForeground());
}

void gTabStripPage::updateFont()
{
	gFont *fnt = parent->textFont();
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
		gtk_notebook_insert_page(GTK_NOTEBOOK(parent->widget), widget, fix, n);
		//gtk_notebook_set_tab_reorderable(GTK_NOTEBOOK(parent->widget), widget, TRUE);
		gtk_widget_realize(widget);
		gtk_widget_realize(fix);
		gtk_widget_show_all(widget);
		gtk_widget_show_all(fix);
		//gtk_container_resize_children(GTK_CONTAINER(gtk_widget_get_parent(widget)));
		//gtk_container_resize_children(GTK_CONTAINER(gtk_widget_get_parent(gtk_widget_get_parent(widget))));
	}
	else
	{
		ind = gtk_notebook_page_num(GTK_NOTEBOOK(parent->widget), widget);
		gtk_notebook_remove_page(GTK_NOTEBOOK(parent->widget), ind);
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


void gTabStripPage::updateButton()
{
	bool v = parent->isClosable();
	
	if (v && !_button)
	{
		_button = gtk_button_new();
		gtk_button_set_relief(GTK_BUTTON(_button), GTK_RELIEF_NONE);
		g_signal_connect_after(G_OBJECT(_button), "expose-event", G_CALLBACK(cb_button_expose), (gpointer)parent);
		g_signal_connect(G_OBJECT(_button), "clicked", G_CALLBACK(cb_button_clicked), (gpointer)parent);
		g_object_set_data(G_OBJECT(_button), "gambas-tab-page", (void *)widget);
		
		gtk_widget_show(_button);
		
		gtk_box_pack_start(GTK_BOX(hbox), _button, FALSE, FALSE, 0);
	}
	else if (!v && _button)
	{
		gtk_widget_destroy(_button);
		_button = NULL;
	}
	
	if (_button)
	{
		gtk_widget_set_size_request(_button, 20, 20);
	}
}


/****************************************************************************
	
	gTabStrip

****************************************************************************/

gTabStrip::gTabStrip(gContainer *parent) : gContainer(parent)
{
	g_typ = Type_gTabStrip;
	_pages = g_ptr_array_new();
	_textFont = NULL;
	_button_pixbuf_normal = NULL;
	_button_pixbuf_disabled = NULL;
	
	onClick = NULL;
	onClose = NULL;
	
	border = gtk_event_box_new();
	//gtk_event_box_set_visible_window(GTK_EVENT_BOX(border), false);
	
	widget = gtk_notebook_new();
	gtk_notebook_set_scrollable(GTK_NOTEBOOK(widget), TRUE);
	gtk_drag_dest_unset(widget);
	
	realize();

	setCount(1);
  
	gtk_widget_add_events(widget,GDK_BUTTON_RELEASE_MASK);
	g_signal_connect_after(G_OBJECT(widget), "switch-page", G_CALLBACK(cb_click), (gpointer)this);
}

gTabStrip::~gTabStrip()
{
	// Do not use setCount() or removeTab(), because they cannot remove non-empty tabs
	lock();
	while (count())
		destroyTab(count() - 1);
	unlock();
	gFont::assign(&_textFont);
	setClosable(false);
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
	GtkWidget *page = gtk_notebook_get_nth_page(GTK_NOTEBOOK(widget), gtk_notebook_get_current_page(GTK_NOTEBOOK(widget)));
	return getRealIndex(page);
}

void gTabStrip::setIndex(int vl)
{
	GtkWidget *page;
	
	if ( (vl<0) || (vl>=count()) || !get(vl)->isVisible() ) return;
	
	page = get(vl)->widget;
	gtk_notebook_set_current_page(GTK_NOTEBOOK(widget), getRealIndex(page));
	//widget = get(vl)->widget;
}

int gTabStrip::orientation()
{
	return gtk_notebook_get_tab_pos(GTK_NOTEBOOK(widget));
}

void gTabStrip::destroyTab(int ind)
{
	delete (gTabStripPage *)g_ptr_array_index(_pages, ind);
	g_ptr_array_remove_index(_pages, ind);
}

bool gTabStrip::removeTab(int ind)
{
	gTabStripPage *page = get(ind);
	
	if (!page || page->count())
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

	if (vl > count())
	{
		lock();
		while (vl > count())
			g_ptr_array_add(_pages, (gpointer)new gTabStripPage(this));
		setIndex(count() - 1);
		unlock();
	}
	
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
    gtk_notebook_set_tab_pos(GTK_NOTEBOOK(widget),GtkPositionType(vl));
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

void gTabStrip::updateFont()
{
	int i;
	
	for (i = 0; i < count(); i++)
		get(i)->updateFont();	
}

void gTabStrip::setFont(gFont *font)
{
	gControl::setFont(font);
	updateFont();
}

gFont *gTabStrip::textFont()
{
	return _textFont;
}

void gTabStrip::setTextFont(gFont *font)
{
	gFont::assign(&_textFont, font);
	updateFont();
}

void gTabStrip::setClosable(bool v)
{
	int i;
	
	if (v == isClosable())
		return;
	
	if (_button_pixbuf_normal)
	{
		g_object_unref(G_OBJECT(_button_pixbuf_normal));
		_button_pixbuf_normal = NULL;
	}
	if (_button_pixbuf_disabled)
	{
		g_object_unref(G_OBJECT(_button_pixbuf_disabled));
		_button_pixbuf_disabled = NULL;
	}
		
	if (v)
	{
		_button_pixbuf_normal = gtk_icon_theme_load_icon(gtk_icon_theme_get_default(), GTK_STOCK_CLOSE, 16, GTK_ICON_LOOKUP_USE_BUILTIN, NULL);
		g_object_ref(G_OBJECT(_button_pixbuf_normal));
	}
	
	for (i = 0; i < count(); i++)
		get(i)->updateButton();
}


