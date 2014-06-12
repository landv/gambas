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
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
  MA 02110-1301, USA.

***************************************************************************/

#include "widgets.h"
#include "gapplication.h"
#include "gmouse.h"
#include "gdesktop.h"
#include "gtabstrip.h"

static const struct {
  guint  	 width;
  guint  	 height;
  guint  	 bytes_per_pixel; /* 2:RGB16, 3:RGB, 4:RGBA */
  guint8 	 pixel_data[16 * 16 * 4 + 1];
} _close_button = {
  16, 16, 4,
  "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
  "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
  "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
  "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
  "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
  "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0_a]\206`d_\344bd`x\0\0\0"
  "\0\0\0\0\0\0\0\0\0\0\0\0\0__]v`d_\344ac_\204\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
  "\0\0\0\0\0\0\0\0\0\0bd`\335\261\264\254\377}\200z\356`b^\200\0\0\0\0\0\0"
  "\0\0`b\\}z}w\355\260\263\253\377ce`\331\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
  "\0\0\0\0\0\0\0_a]v}\200z\356\272\275\265\377\201\203}\356`b^\200`b\\}z}w"
  "\355\261\264\254\377|~y\355ad_n\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
  "\0\0\0\0\0\0\0ac]~\203\205\200\356\300\304\274\377\206\211\203\356\200\202"
  "}\355\261\264\254\377|~y\356`d^x\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
  "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0bd`\177\212\215\210\356\301\304\274\377\271"
  "\274\264\377\201\204}\356ac_y\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
  "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0bd`}\213\215\210\355\303\306\276\377"
  "\272\276\266\377\177\201|\355`c\\w\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
  "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0bd^}\204\206\201\355\307\313\303\377\212"
  "\215\206\355\203\205\200\355\261\264\254\377z|v\355`c\\w\0\0\0\0\0\0\0\0"
  "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0aa]v\200\202~\355\302\306\276\377"
  "\205\207\202\356bd^\200`b^}{}w\355\261\264\254\377z|v\355`c`o\0\0\0\0\0\0"
  "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0dfa\342\271\274\264\377\203\205\200\355"
  "`d^\200\0\0\0\0\0\0\0\0`b\\}|~y\354\261\264\254\377bd^\340\0\0\0\0\0\0\0"
  "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0ab_\231cfa\341`b^z\0\0\0\0\0\0\0\0\0\0"
  "\0\0\0\0\0\0^`\\wcfa\335`b^\222\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
  "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
  "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
  "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
  "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
  "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
  "\0\0\0",
};



cairo_surface_t *gTabStrip::_button_normal = NULL;
cairo_surface_t *gTabStrip::_button_disabled = NULL;

static void cb_click(GtkNotebook *nb, GtkWidget *pg, guint pnum, gTabStrip *data)
{
	data->updateFont();
	data->performArrange();
	data->emit(SIGNAL(data->onClick));
}

static void cb_size_allocate(GtkWidget *wid, GtkAllocation *alloc, gTabStrip *data)
{
	if (wid == data->getContainer() && (alloc->width != data->_client_w || alloc->height != data->_client_h))
	{
		int tx, ty, px, py;

		if (data->getScreenPos(&tx, &ty))
			return;

		gdk_window_get_origin(gtk_widget_get_window(wid), &px, &py);
		//fprintf(stderr, "alloc: tab = %d %d page = %d %d alloc = %d %d\n", tx, ty, px, py, alloc->x, alloc->y);

		data->_client_x = px - tx + alloc->x;
		data->_client_y = py - ty + alloc->y;
		data->_client_w = alloc->width;
		data->_client_h = alloc->height;
		//fprintf(stderr, "alloc: %s: %d %d %d %d\n", data->name(), data->_client_x, data->_client_y, alloc->width, alloc->height);
		data->performArrange();
	}
}

#ifdef GTK3
static gboolean cb_button_draw(GtkWidget *wid, cairo_t *cr, gTabStrip *data)
#else
static gboolean cb_button_expose(GtkWidget *wid, GdkEventExpose *e, gTabStrip *data)
#endif
{
#ifndef GTK3
	cairo_t *cr;
#endif
	cairo_surface_t *img;
	GdkRectangle rpix = {0,0,0,0};
	GdkRectangle rect = {0,0};
	int py, px;
	int dx, dy;

#ifndef GTK3
	GTK_BUTTON(wid)->relief = GTK_RELIEF_NORMAL;
#endif

	gtk_widget_get_allocation(wid, &rect);
#ifdef GTK3
	rect.x = rect.y = 0;
#endif
	px = rect.width;

#ifdef GTK3
	if (gtk_widget_get_state_flags(data->widget) & GTK_STATE_FLAG_ACTIVE)
#else
	if (GTK_WIDGET_STATE(data->widget) == GTK_STATE_ACTIVE)
#endif
	{
	  gtk_widget_style_get (wid,
				"child-displacement-x", &dx,
				"child-displacement-y", &dy,
				(void *)NULL);
		rect.x += dx;
		rect.y += dy;
	}

#ifdef GTK3
	if (gtk_widget_get_state_flags(data->widget) & GTK_STATE_FLAG_INSENSITIVE)
#else
	if (GTK_WIDGET_STATE(data->widget) == GTK_STATE_INSENSITIVE)
#endif
		img = data->_button_disabled;
	else
		img = data->_button_normal;

	rpix.width = cairo_image_surface_get_width(img);
	rpix.height = cairo_image_surface_get_height(img);

	py = (rect.height - rpix.height) / 2;

	rect.x += (px - rpix.width) / 2;
	rect.y += py;

#ifndef GTK3
	cr = gdk_cairo_create(wid->window);
#endif

	cairo_set_source_surface(cr, img, rect.x, rect.y);
	cairo_paint(cr);

#ifndef GTK3
	cairo_destroy(cr);
#endif

	return false;
}

static void cb_button_clicked(GtkWidget *wid, gTabStrip *data)
{
	if (data->onClose)
		(*data->onClose)(data, data->getRealIndex((GtkWidget *)g_object_get_data(G_OBJECT(wid), "gambas-tab-page")));
}

#ifdef GTK3
static void cb_scroll(GtkWidget *wid, GdkEvent *event, gTabStrip *data)
{
	int dir = event->scroll.direction;
	int page;

	if (dir == GDK_SCROLL_SMOOTH)
		return;

	page = gtk_notebook_get_current_page(GTK_NOTEBOOK(data->widget));

	switch (dir)
	{
		case GDK_SCROLL_UP:
		case GDK_SCROLL_LEFT:
			page--;
			if (page >= 0)
				gtk_notebook_set_current_page(GTK_NOTEBOOK(data->widget), page);
			break;
		case GDK_SCROLL_RIGHT:
		case GDK_SCROLL_DOWN: default:
			page++;
			if (page < gtk_notebook_get_n_pages(GTK_NOTEBOOK(data->widget)))
				gtk_notebook_set_current_page(GTK_NOTEBOOK(data->widget), page);
			break;
	}
}
#endif

/****************************************************************************
	
	gTabStripPage

****************************************************************************/

class gTabStripPage
{
public:
	gTabStripPage(gTabStrip *tab);
	~gTabStripPage();

	char *text() const;
	void setText(char *text);
	gPicture *picture() const { return _picture; }
	void setPicture(gPicture *picture);
	bool isVisible() const { return _visible; }
	void setVisible(bool v);
	bool enabled() const;
	void setEnabled(bool v);
	/*int count() const;
	gControl *child(int n) const;*/
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
#ifdef GTK3
	gt_widget_set_color(widget, FALSE, parent->realBackground());
#else
	set_gdk_bg_color(widget, parent->realBackground());
	set_gdk_fg_color(label, parent->realForeground());
#endif
}

void gTabStripPage::updateFont()
{
	PangoFontDescription *desc = NULL;
	gFont *fnt;
	
	fnt = parent->textFont();
	if (!fnt)
		fnt = parent->font();
	
	if (fnt)
		desc = fnt->desc();

#ifdef GTK3
	gtk_widget_override_font(widget, desc);
	gtk_widget_override_font(label, desc);
#else
	gtk_widget_modify_font(widget, desc);
	gtk_widget_modify_font(label, desc);
#endif
}

void gTabStripPage::setText(char *text)
{
	char *buf;

	gMnemonic_correctText(text, &buf);
	gtk_label_set_text_with_mnemonic(GTK_LABEL(label), buf);
	g_free(buf);
}

char *gTabStripPage::text() const
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

bool gTabStripPage::enabled() const
{
#if GTK_CHECK_VERSION(2, 18, 0)
	return gtk_widget_get_sensitive(hbox);
#else
	return GTK_WIDGET_SENSITIVE(hbox);
#endif
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

#ifndef GTK3
static gboolean cb_button_fix(GtkWidget *wid, GdkEvent *e, gTabStripPage *data)
{
	GTK_BUTTON(wid)->relief = GTK_RELIEF_NONE;
	return false;
}
#endif

void gTabStripPage::updateButton()
{
	bool v = parent->isClosable();
	
	if (v && !_button)
	{
		_button = gtk_button_new();
		gtk_button_set_focus_on_click(GTK_BUTTON(_button), false);
#ifdef GTK3
		gtk_button_set_relief(GTK_BUTTON(_button), GTK_RELIEF_NONE);
#else
		g_signal_connect(G_OBJECT(_button), "expose-event", G_CALLBACK(cb_button_fix), (gpointer)this);
#endif
		//g_signal_connect_after(G_OBJECT(_button), "expose-event", G_CALLBACK(cb_button_expose), (gpointer)parent);
		ON_DRAW(_button, parent, cb_button_expose, cb_button_draw);
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
	_closable = false;
	
	onClick = NULL;
	onClose = NULL;
	
	//border = gtk_event_box_new();
	//gtk_event_box_set_visible_window(GTK_EVENT_BOX(border), false);
	
	border = widget = gtk_notebook_new();
	gtk_notebook_set_scrollable(GTK_NOTEBOOK(widget), TRUE);
	gtk_drag_dest_unset(widget);
	
	realize();

	gtk_widget_add_events(border, GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK
		| GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK
		| GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE_MASK | GDK_SCROLL_MASK);
	
	setCount(1);
  
	g_signal_connect_after(G_OBJECT(widget), "switch-page", G_CALLBACK(cb_click), (gpointer)this);
#ifdef GTK3
	g_signal_connect(G_OBJECT(widget), "scroll-event", G_CALLBACK(cb_scroll), (gpointer)this);
#endif
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


int gTabStrip::getRealIndex(GtkWidget *page) const
{
	int i;
	
	for (i = 0; i < count(); i++)
	{
		if (get(i)->widget == page)
			return i;
	}
	
	return -1;
}

int gTabStrip::index() const
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

int gTabStrip::orientation() const
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
	
	if (!page || tabCount(ind))
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
			if (tabCount(i))
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

gPicture* gTabStrip::tabPicture(int ind) const
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

bool gTabStrip::tabEnabled(int ind) const
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

bool gTabStrip::tabVisible(int ind) const
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

char* gTabStrip::tabText(int ind) const
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

int gTabStrip::tabCount(int ind) const
{
	int i;
	gControl *ch;
	int ct = 0;
	
	if ((ind < 0) || (ind >= count())) 
		return 0;
	
	for (i = 0; i < gContainer::childCount(); i++)
	{
		ch = gContainer::child(i);
		if (gtk_widget_get_parent(ch->border) == get(ind)->widget)
			ct++;
	}
	
	return ct;
}

gControl *gTabStrip::tabChild(int ind, int n) const
{
	int i;
	gControl *ch;
	int ct = 0;
	
	if ((ind < 0) || (ind >= count()))
		return NULL;
	
	for (i = 0; i < gContainer::childCount(); i++)
	{
		ch = gContainer::child(i);
		if (gtk_widget_get_parent(ch->border) == get(ind)->widget)
		{
			if (ct == n)
				return ch;
			ct++;
		}
	}
	
	return NULL;
}

int gTabStrip::childCount() const
{
	return tabCount(index());
}

gControl *gTabStrip::child(int ind) const
{
	return tabChild(index(), ind);
}

GtkWidget *gTabStrip::getContainer()
{
	gTabStripPage *page = get(index());
	
	if (page)
		return page->widget;
	else
		return NULL;
}

#ifdef GTK3
void gTabStrip::updateColor()
{
	//fprintf(stderr, "%s: updateColors\n", name());
	gt_widget_set_color(border, false, realBackground());
	gt_widget_set_color(widget, false, realBackground());

	for (int i = 0; i < count(); i++)
		get(i)->updateColors();
}
#else
void gTabStrip::setRealBackground(gColor color)
{
	gControl::setRealBackground(color);

	for (int i = 0; i < count(); i++)
		get(i)->updateColors();
}
#endif

void gTabStrip::setRealForeground(gColor color)
{
	gControl::setRealForeground(color);
	
	for (int i = 0; i < count(); i++)
		get(i)->updateColors();
}

gTabStripPage *gTabStrip::get(int ind) const
{
	if (ind < 0 || ind >= count())
		return NULL;
	else 
		return (gTabStripPage *)g_ptr_array_index(_pages, ind);
}

void gTabStrip::updateFont()
{
	int i;
	
	gContainer::updateFont();

	for (i = 0; i < count(); i++)
		get(i)->updateFont();	
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
	
	if (v == _closable)
		return;
	
	_closable = v;

	if (v)
	{
		if (!_button_normal)
		{
			GdkPixbuf *normal, *disabled;

			normal = gtk_icon_theme_load_icon(gtk_icon_theme_get_default(), "window-close", 16, GTK_ICON_LOOKUP_USE_BUILTIN, NULL);
			if (!normal)
			{
				normal = gdk_pixbuf_new_from_data(_close_button.pixel_data, GDK_COLORSPACE_RGB, TRUE, 8,
																					_close_button.width, _close_button.height,
																					_close_button.width * sizeof(int), NULL, NULL);
			}

			disabled = gt_pixbuf_create_disabled(normal);

			_button_normal = gt_cairo_create_surface_from_pixbuf(normal);
			_button_disabled = gt_cairo_create_surface_from_pixbuf(disabled);

			g_object_unref(normal);
			g_object_unref(disabled);

		}
	}
	
	for (i = 0; i < count(); i++)
		get(i)->updateButton();

}

int gTabStrip::findIndex(gControl *child) const
{
	int i;
	GtkWidget *page;
	
	page = gtk_widget_get_parent(child->border); 
	
	for (i = 0; i < count(); i++)
	{
		if (get(i)->widget == page)
			return i;
	}
	
	return -1;
}
