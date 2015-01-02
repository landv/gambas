/***************************************************************************

  gmenu.cpp

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
#include "gmainwindow.h"
#include "gapplication.h"
#include "gdesktop.h"
#include "gmenu.h"

typedef
	struct {
		int x;
		int y;
		}
	MenuPosition;

gMenu *gMenu::_current_popup = NULL;
int gMenu::_in_popup = 0;
int gMenu::_popup_count = 0;

static GList *menus = NULL;

static void mnu_destroy (GtkWidget *object, gMenu *data)
{
	if (data->stop_signal) 
		data->stop_signal = false; 
	else
		data->destroy();
}

static void mnu_activate(GtkMenuItem *menuitem, gMenu *data)
{
	if (data->child)
		return;

	if (data->radio())
		data->setRadio();
	else if (data->toggle())
		data->setChecked(!data->checked());

	if (data->onClick)
	{
		//fprintf(stderr, "mnu_activate: %s\n", data->name());
		data->onClick(data);
	}
}

static gboolean cb_map(GtkWidget *menu, gMenu *data)
{
	data->hideSeparators();
	if (data->onShow) (*data->onShow)(data);
	return false;
}

static gboolean cb_unmap(GtkWidget *menu, gMenu *data)
{
	if (data->onHide) (*data->onHide)(data);
	return false;
}

static int get_menu_pos(GtkWidget *menu)
{
	GList *children, *iter;
	int pos;
	
	if (!gtk_widget_get_parent(menu))
	{
		//g_debug("get_menu_pos: no parent for menu %p", menu);
		return -1;
	}
	
	children = gtk_container_get_children(GTK_CONTAINER(gtk_widget_get_parent(menu)));
  iter = g_list_first(children);
  
  for(pos = 0;; pos++) 
  {
    if (iter->data == (gpointer)menu) 
      break; 
    iter = g_list_next(iter);
  }
  
  g_list_free(children);

  return pos;
}

#ifdef GTK3
static gboolean cb_check_draw(GtkWidget *wid, cairo_t *cr, gMenu *menu)
{
	static GtkWidget *check_menu_item = NULL;
	static GtkWidget *radio_menu_item = NULL;

	int x, y, w, h;
	gint indicator_size = 0;
	GtkStateFlags state;
	GtkWidget *item;
	GtkStyleContext *style;

	GtkAllocation a;
	gtk_widget_get_allocation(wid, &a);
	x = 0; //a.x;
	y = 0; //a.y;
	w = a.width;
	h = a.height;

	if (menu->radio())
	{
		if (!radio_menu_item)
			radio_menu_item = gtk_radio_menu_item_new(NULL);
		item = radio_menu_item;
		style = gtk_widget_get_style_context(item);
		gtk_style_context_add_class(style, GTK_STYLE_CLASS_RADIO);
	}
	else
	{
		if (!check_menu_item)
			check_menu_item = gtk_check_menu_item_new();
		item = check_menu_item;
		style = gtk_widget_get_style_context(item);
		gtk_style_context_add_class(style, GTK_STYLE_CLASS_CHECK);
	}

	gtk_widget_style_get(item, "indicator-size", &indicator_size, (char *)NULL);
	indicator_size = MAX(16, indicator_size);

	x += (w - indicator_size) / 2;
	y += (h - indicator_size) / 2;
	w = indicator_size;
	h = indicator_size;

	state = gtk_widget_get_state_flags(wid);

	if (menu->checked())
		state = (GtkStateFlags)((int)state | GTK_STATE_FLAG_ACTIVE);

	gtk_widget_set_state_flags(item, state, true);

	style = gtk_widget_get_style_context(wid);
	gtk_style_context_save(style);
	gtk_style_context_set_state(style, state);

	if (menu->radio())
	{
    gtk_style_context_add_class(style, GTK_STYLE_CLASS_RADIO);
		//gtk_render_background(style, cr, x, y, w, h);
		gtk_render_option(style, cr, x, y, indicator_size, indicator_size);
	}
	else
	{
    gtk_style_context_add_class(style, GTK_STYLE_CLASS_CHECK);
		//gtk_render_background(style, cr, x, y, w, h);
		gtk_render_check(style, cr, x, y, indicator_size, indicator_size);
	}

	gtk_style_context_restore(style);
	return false;
}
#else
static gboolean cb_check_expose(GtkWidget *wid, GdkEventExpose *e, gMenu *menu)
{
	static GtkWidget *check_menu_item = NULL;
	static GtkWidget *radio_menu_item = NULL;
	GtkWidget *item;

	int x, y, w, h;
	gint indicator_size;
	GtkShadowType shadow;

	x = wid->allocation.x;
	y = wid->allocation.y;
	w = wid->allocation.width;
	h = wid->allocation.height;

	if (menu->radio())
	{
		if (!radio_menu_item)
			radio_menu_item = gtk_radio_menu_item_new(NULL);
		item = radio_menu_item;
	}
	else
	{
		if (!check_menu_item)
			check_menu_item = gtk_check_menu_item_new();
		item = check_menu_item;
	}

	gtk_widget_style_get(item, "indicator-size", &indicator_size, (char *)NULL);
	indicator_size = MAX(16, indicator_size);

	x += (w - indicator_size) / 2;
	y += (h - indicator_size) / 2;
	w = indicator_size;
	h = indicator_size;

	gtk_widget_set_state(item, (GtkStateType)GTK_WIDGET_STATE(wid));

	shadow = menu->checked() ? GTK_SHADOW_IN : GTK_SHADOW_OUT;

	if (menu->radio())
	{
		gtk_paint_option(wid->style, wid->window,
					(GtkStateType)GTK_WIDGET_STATE(wid), shadow,
					&e->area, radio_menu_item, "radiobutton",
					x, y, w, h);
	}
	else
	{

		gtk_paint_check(wid->style, wid->window,
					(GtkStateType)GTK_WIDGET_STATE(wid), shadow,
					&e->area, check_menu_item, "check",
					x, y, w, h);
	}
	
	return false;
}
#endif

void gMenu::update()
{
	GtkMenuShell *shell = NULL;
	gint pos;
	int size;
	
	if (_no_update)
		return;
	
	//g_debug("%p: START UPDATE (menu = %p child = %p parent = %p)", this, menu);
	
	if (_style != _oldstyle)
	{
		if (child)
		{
			g_object_ref(G_OBJECT(child));
			if (_style == MENU)
				gtk_menu_item_set_submenu(menu, NULL);
		}

		if (menu)
		{
			pos = get_menu_pos(GTK_WIDGET(menu));
			//shell = (GtkMenuShell*)GTK_WIDGET(menu)->parent;
			if (_style != NOTHING)
				stop_signal = true;
			gtk_widget_destroy(GTK_WIDGET(menu));
			//g_debug("%p: delete old menu/separator", this);
		}
		else
		{
			pos = -1;
			//shell = NULL;
		}
		
		if (_style != NOTHING)
		{
			if (_style == SEPARATOR)
			{
				menu = (GtkMenuItem *)gtk_separator_menu_item_new();
#ifdef GTK3
#else
				GtkRequisition req;
				gtk_widget_size_request(GTK_WIDGET(menu), &req);
				if (req.height > 5)
					gtk_widget_set_size_request(GTK_WIDGET(menu), -1, 5);
#endif
				//g_debug("%p: create new separator %p", this, menu);
			}
			else if (_style == MENU)
			{
				menu = (GtkMenuItem *)gtk_image_menu_item_new();
				//g_debug("%p: create new menu %p", this, menu);
				
				hbox = gtk_hbox_new(false, gDesktop::scale());
				//set_gdk_bg_color(hbox, 0xFF0000);
				gtk_container_add(GTK_CONTAINER(menu), GTK_WIDGET(hbox));
				
				label = gtk_label_new_with_mnemonic("");
				//gtk_label_set_justify(GTK_LABEL(lbl),GTK_JUSTIFY_LEFT);
				
				if (!top_level)
				{
					image = gtk_image_new();
					g_object_ref(image);
					
					aclbl = gtk_label_new("");
					gtk_misc_set_alignment(GTK_MISC(aclbl), 0, 0.5);
					gtk_size_group_add_widget(((gMenu*)pr)->sizeGroup, aclbl);
					
					check = gtk_image_new();
					g_object_ref(check);
					size = MAX(18, window()->font()->height());
					gtk_widget_set_size_request(check, size, size);
					ON_DRAW(check, this, cb_check_expose, cb_check_draw);
					//g_signal_connect_after(G_OBJECT(check), "expose-event", G_CALLBACK(cb_check_expose), (gpointer)this);
					
					//gtk_box_pack_start(GTK_BOX(hbox), image, false, false, 0);
					gtk_box_pack_start(GTK_BOX(hbox), label, false, false, 0);
					gtk_box_pack_end(GTK_BOX(hbox), aclbl, false, false, 0);				
				}
				else
				{
					aclbl = NULL;
					gtk_box_pack_start(GTK_BOX(hbox), label, false, false, 0);
				}
				
				if (child)
				{
					gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu), GTK_WIDGET(child));
					g_object_unref(G_OBJECT(child));
				}
					
				
				//set_gdk_fg_color(label, get_gdk_fg_color(GTK_WIDGET(shell)));
				//set_gdk_bg_color(label, get_gdk_bg_color(GTK_WIDGET(shell)));
			}
			
			gtk_widget_show_all(GTK_WIDGET(menu));
			
			if (top_level)
			{
				gMainWindow *win = (gMainWindow *)pr;
				
				//set_gdk_fg_color(GTK_WIDGET(menu), win->foreground());
				//set_gdk_bg_color(GTK_WIDGET(menu), win->background());	
				
				//gtk_menu_shell_append(GTK_MENU_SHELL(win->menuBar), GTK_WIDGET(menu));
				shell = GTK_MENU_SHELL(win->menuBar);
			}
			else
			{
				gMenu *parent = (gMenu *)pr;
				
				if (!parent->child)
				{
					parent->child = (GtkMenu*)gtk_menu_new();
					//g_debug("%p: creates a new child menu container in parent %p", this, parent->child);
					
					g_signal_connect(G_OBJECT(parent->child), "map", G_CALLBACK(cb_map), (gpointer)parent);
					g_signal_connect(G_OBJECT(parent->child), "unmap", G_CALLBACK(cb_unmap), (gpointer)parent);
					gtk_widget_show_all(GTK_WIDGET(parent->child));
					if (parent->style() == MENU)
						gtk_menu_item_set_submenu(GTK_MENU_ITEM(parent->menu), GTK_WIDGET(parent->child));
					
					parent->setColor();
					
					//gtk_menu_shell_append(GTK_MENU_SHELL(parent->child), GTK_WIDGET(menu));
					//g_debug("%p: add to new parent %p", this, parent->child);
				}
				shell = GTK_MENU_SHELL(parent->child);
			}
	
			if (shell)
			{
				if (pos < 0)
				{
					gtk_menu_shell_append(shell, GTK_WIDGET(menu));
					//g_debug("%p: append to parent %p", this, shell);
				}
				else
				{
					gtk_menu_shell_insert(shell, GTK_WIDGET(menu), pos);
					//g_debug("%p: insert into parent %p", this, shell);
				}
			}
			
			g_signal_connect(G_OBJECT(menu), "destroy", G_CALLBACK(mnu_destroy), (gpointer)this);
			g_signal_connect(G_OBJECT(menu), "activate", G_CALLBACK(mnu_activate), (gpointer)this);
		}
		
		_oldstyle = _style;
		updateVisible();
	}
	
	if (_style == MENU)
	{
		{
			char *buf;
			gMnemonic_correctText(_text, &buf);
			gtk_label_set_text_with_mnemonic(GTK_LABEL(label), buf);
			g_free(buf);
		}
		
		if (!top_level)
		{
			char *buf;
			
			if (_shortcut)
			{
				buf = g_strconcat("\t", _shortcut ,"  ",(void *)NULL);
				gtk_label_set_text(GTK_LABEL(aclbl), buf);
				g_free(buf);
			}
			else
				gtk_label_set_text(GTK_LABEL(aclbl), "\t");

			if (_checked || _radio || _toggle)
			{
				gtk_image_menu_item_set_image((GtkImageMenuItem *)menu, check);
				gtk_widget_show(check);
				gtk_widget_hide(image);
			}
			else
			{
				gtk_image_set_from_pixbuf(GTK_IMAGE(image), _picture ? _picture->getPixbuf() : NULL);
				gtk_image_menu_item_set_image((GtkImageMenuItem *)menu, image);
				gtk_widget_show(image);
				gtk_widget_hide(check);
			}
		}
		
		setColor();
		setFont();
	}

	//g_debug("%p: END UPDATE", this);	
}

void gMenu::initialize()
{
	//fprintf(stderr, "gMenu::gMenu: %p (%p)\n", this, pr);
	
	stop_signal = false;
	
	onFinish = NULL;
	onClick = NULL;
	onShow = NULL;
	onHide = NULL;
	
	hFree = NULL;
	child = NULL;
	image = NULL;
	label = NULL;
	aclbl = NULL;
	check = NULL;
	menu = NULL;
	top_level=false;
	
	_text = NULL;
	_shortcut = NULL;
	_checked = false;
	_picture = NULL;
	_name = NULL;
	_toggle = false;
	_radio = false;

	_style = NOTHING;
	_oldstyle = NOTHING;
	
	_no_update = false;
	_destroyed = false;
	_delete_later = false;
	_action = false;
	_visible = false;
	
	sizeGroup = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
	
	menus=g_list_append (menus,(gpointer)this);
}


static gboolean cb_menubar_changed(GtkWidget *widget, gMainWindow *data)
{
	data->configure();
	return false;
}

gMenu::gMenu(gMainWindow *par, bool hidden)
{
	pr = (gpointer)par;
  initialize();
	top_level=true;
	
	accel = par->accel;
	g_object_ref(accel);
	
	if (!par->menuBar)
	{
		par->menuBar = (GtkMenuBar*)gtk_menu_bar_new();
		g_signal_connect_after(G_OBJECT(par->menuBar), "map", G_CALLBACK(cb_menubar_changed), (gpointer)par);
		g_signal_connect(G_OBJECT(par->menuBar),"unmap", G_CALLBACK(cb_menubar_changed), (gpointer)par);
		par->embedMenuBar(par->border);
	}
	
	setText(NULL);
	setVisible(!hidden);
}

gMenu::gMenu(gMenu *par, bool hidden)
{
	pr = (gpointer)par;
	
	initialize();
	sizeGroup=gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
	
	if (!par) return;
	if (!par->menu) return;
	
	accel = par->accel;
	g_object_ref(accel);
	
	setText(NULL);
	setVisible(!hidden);
}

gMenu::~gMenu()
{
	GList *item;
	gMenu *mn;
	
	if (_destroyed)
		return;
		
	//fprintf(stderr, "gMenu::~gMenu: %p (%p) '%s'\n", this, pr, _text);
	
	_destroyed = true;
  
  // Remove references to me
  
	item = g_list_first(menus);
	while (item)
	{
		mn = (gMenu*)item->data;
		if (mn->pr == (void*)this)
		  mn->pr = NULL;
		item = g_list_next(item);
	}
	
	menus = g_list_remove(menus, (gpointer)this);
	
	_no_update = true;
	setText(NULL);
	setShortcut(NULL);
	setPicture(NULL);
	
	//if (_style != NOTHING)
	{
		if (aclbl && (!top_level) && pr)
			gtk_size_group_remove_widget(((gMenu*)pr)->sizeGroup, aclbl);
		
		if (child)
			gtk_widget_destroy(GTK_WIDGET(child));
	
		if (sizeGroup) 
			g_object_unref(G_OBJECT(sizeGroup));
		
		if (accel)
			g_object_unref(accel);	
	}
		
	_style = NOTHING;
	
	stop_signal = true;
	
	if (image)
		gtk_widget_destroy(GTK_WIDGET(image));
		
	if (check)
		gtk_widget_destroy(GTK_WIDGET(check));
		
	if (menu)
		gtk_widget_destroy(GTK_WIDGET(menu));
	
	if (_current_popup == this)
		_current_popup = NULL;
	
	if (onFinish) onFinish(this);
}

bool gMenu::enabled()
{
	return gtk_widget_is_sensitive(GTK_WIDGET(menu));
}

void gMenu::setEnabled(bool vl)
{
	gtk_widget_set_sensitive(GTK_WIDGET(menu),vl);
}


void gMenu::setText(const char *text)
{
	g_free(_text);
	if (text)
		_text = g_strdup(text);
	else
		_text = NULL;
		
	if (!_text || !*_text)
		_style = SEPARATOR;
	else
		_style = MENU;
	
	update();
}

bool gMenu::isVisible()
{
	if (!menu) return false;
	return _visible;	
}

void gMenu::updateVisible()
{
	bool vl = _visible;
	
	if (top_level && _style != MENU)
		vl = false;
	
	//fprintf(stderr, "gMenu::updateVisible: %s '%s' %d\n", name(), text(), vl);
	
	gtk_widget_set_visible(GTK_WIDGET(menu), vl);
	//g_object_set(G_OBJECT(menu),"visible",vl,(void *)NULL);
	
	if (top_level && pr)
		((gMainWindow *)pr)->checkMenuBar();
}

void gMenu::setVisible(bool vl)
{
	if (!menu) return;
	if (vl == _visible) return;
	
	_visible = vl;
	updateVisible();
}

void gMenu::setPicture(gPicture *pic)
{
	//fprintf(stderr, "gMenu::setPicture: %p\n", pic);
	gPicture::assign(&_picture, pic);
	update();
}

void gMenu::setChecked(bool vl)
{
	_checked = vl;
	update();	
}

void gMenu::setToggle(bool vl)
{
	_toggle = vl;
	update();
}

void gMenu::setRadio(bool vl)
{
	_radio = vl;
	update();
}

int gMenu::childCount()
{
	GList *item;
	gMenu *mn;
	int ct=0;
	
	if (!menus) return 0;
	
	item=g_list_first(menus);
	while (item)
	{
		mn=(gMenu*)item->data;
		if (mn->pr == (void*)this && !mn->_delete_later)
			ct++;
		item=g_list_next(item);
	}
	
	return ct;
}

gMenu* gMenu::childMenu(int pos)
{
	GList *item;
	gMenu *mn;
	int ct=0;
	
	if (!menus) return NULL;
	
	item=g_list_first(menus);
	while (item)
	{
		mn=(gMenu*)item->data;
		if (mn->pr == (void*)this && !mn->_delete_later)
		{
			if (ct==pos) return mn;
			ct++;
		}
		item=g_list_next(item);
	}
	
	return NULL;
}

void gMenu::destroy()
{
	if (!_destroyed && !_delete_later)
		delete this;
}

static void position_menu(GtkMenu *menu, gint *x, gint *y, gboolean *push_in, MenuPosition *pos)
{
	*x = pos->x;
	*y = pos->y;
	*push_in = false;
	delete pos;
}

void gMenu::doPopup(bool move, int x, int y)
{
	gMenu *save_current_popup;
	MenuPosition *pos = NULL;
	
	if (!child)
		return;
	
	if (move)
	{
		pos = new MenuPosition;
		pos->x = x;
		pos->y = y;
	}
	
	save_current_popup = _current_popup;
	_current_popup = this;
	
	_in_popup++;
	_popup_count++;
	
	gtk_menu_popup(child, NULL, NULL, move ? (GtkMenuPositionFunc)position_menu : NULL, (gpointer)pos, 0, gApplication::lastEventTime());
	
#if GTK_CHECK_VERSION(2, 20, 0)
	while (_current_popup && child && gtk_widget_get_mapped(GTK_WIDGET(child)))
#else
	while (_current_popup && child && GTK_WIDGET_MAPPED(child))
#endif
		MAIN_do_iteration(false);

	_current_popup = save_current_popup;

	_in_popup--;

	// flush the event loop so that the main window is reactivated before the click menu event is raised

	while (gtk_events_pending())
		MAIN_do_iteration(false);
}

void gMenu::popup(int x, int y)
{
	doPopup(true, x, y);
}

void gMenu::popup()
{
	doPopup(false);
}


int gMenu::winChildCount(gMainWindow *par)
{
	GList *item;
	gMenu *mn;
	int ct=0;
	
	if (!menus) return 0;
	
	item=g_list_first(menus);
	while (item)
	{
		mn=(gMenu*)item->data;
		if (mn->pr == (void*)par) ct++;
		item=g_list_next(item);
	}
	
	return ct;
}

gMenu* gMenu::winChildMenu(gMainWindow *par,int pos)
{
	GList *item;
	gMenu *mn;
	int ct=0;
	
	if (!menus) return NULL;
	
	item=g_list_first(menus);
	while (item)
	{
		mn=(gMenu*)item->data;
		if (mn->pr == (void*)par)
		{
			if (ct==pos) return mn;
			ct++;
		}
		item=g_list_next(item);
	}
	
	return NULL;
}

gMenu *gMenu::findFromName(gMainWindow *win, const char *name)
{
	int i;
	int count = winChildCount(win);
	gMenu *menu;
	
	for (i = 0; i < count; i++)
	{
		menu = winChildMenu(win, i);
		if (!strcasecmp(menu->name(), name))
			return menu;
	}
	
	return NULL;
}

void gMenu::setShortcut(char *shortcut)
{
	guint key;
	GdkModifierType mods;
	
	if (_shortcut)
	{
		gt_shortcut_parse(_shortcut, &key, &mods);
		if (key)
			gtk_widget_remove_accelerator(GTK_WIDGET(menu), accel, key, mods);
		g_free(_shortcut);
		_shortcut = NULL;
	}

	if (shortcut)
	{
		_shortcut = g_strdup(shortcut);
		gt_shortcut_parse(_shortcut, &key, &mods);
		if (key)
			gtk_widget_add_accelerator(GTK_WIDGET(menu),"activate",accel,key,mods,(GtkAccelFlags)0);
	}

	update();
}

gMainWindow *gMenu::window()
{
  if (!pr)
    return NULL;

  if (top_level)
    return (gMainWindow *)pr;
    
  return ((gMenu *)pr)->window();
}

void gMenu::setName(char *name)
{
	if (_name)
	{
		g_free(_name);
		_name = NULL;
	}
	
	if (name) 
		_name = g_strdup(name);
}


void gMenu::hideSeparators()
{
	gMenu *ch;
	gMenu *last_ch;
	bool is_sep;
	bool last_sep;
	GList *item;

	if (!child)
		return;
		
	last_sep = true;
	last_ch = 0;
	
	item = g_list_first(menus);
	while (item)
	{
		ch = (gMenu*)item->data;
		if (ch->pr == this)
		{
			is_sep = ch->style() == SEPARATOR;
			if (is_sep)
			{
				if (last_sep)
				{
					ch->hide();
				}
				else
				{
					ch->show();
					last_sep = true;
					last_ch = ch;
				}
			}
			else
			{
				if (ch->isVisible())
					last_sep = false;
			}
		}
		
		item = g_list_next(item);
	}
	
	if (last_sep && last_ch)
		last_ch->hide();
}

void gMenu::setFont()
{
	gMainWindow *win = window();
#ifdef GTK3
	if (label) gtk_widget_override_font(GTK_WIDGET(label), win->font()->desc());
	if (aclbl) gtk_widget_override_font(GTK_WIDGET(aclbl), win->font()->desc());
#else
	if (label) gtk_widget_modify_font(GTK_WIDGET(label), win->font()->desc());
	if (aclbl) gtk_widget_modify_font(GTK_WIDGET(aclbl), win->font()->desc());
#endif
}

void gMenu::setColor()
{
	gMainWindow *win = window();
	/*if (child) 
	{
		set_gdk_bg_color(GTK_WIDGET(child), win->background());
		set_gdk_fg_color(GTK_WIDGET(child), win->foreground());
	}*/
	if (label) set_gdk_fg_color(GTK_WIDGET(label), win->foreground());
	if (aclbl) set_gdk_fg_color(GTK_WIDGET(aclbl), win->foreground());
}

void gMenu::updateColor(gMainWindow *win)
{
	GList *item;
	gMenu *mn;

	if (!win->menuBar)
		return;
	
	set_gdk_bg_color(GTK_WIDGET(win->menuBar), win->background());

	if (!menus) 
		return;
	
	item = g_list_first(menus);
	while (item)
	{
		mn = (gMenu*)item->data;
		if (mn->pr == (void*)win)
			mn->setColor();
		item=g_list_next(item);
	}
}

void gMenu::updateFont(gMainWindow *win)
{
	GList *item;
	gMenu *mn;
	
	if (win->menuBar)
	{
		//fprintf(stderr, "set menu bar font\n");
#ifdef GTK3
		gtk_widget_override_font(GTK_WIDGET(win->menuBar), win->ownFont() ? win->font()->desc() : NULL);
#else
		gtk_widget_modify_font(GTK_WIDGET(win->menuBar), win->ownFont() ? win->font()->desc() : NULL);
#endif
	}
	
	if (!menus) 
		return;
	
	item = g_list_first(menus);
	while (item)
	{
		mn = (gMenu*)item->data;
		if (mn->pr == (void*)win)
			mn->setFont();
		item=g_list_next(item);
	}
}

void gMenu::setRadio()
{
	gMenu *child;
	GList *item;
	GList *start = NULL;

	if (top_level)
		return;

	item = g_list_first(menus);
	while (item)
	{
		child = (gMenu*)item->data;
		if (child->parent() == pr && !child->_delete_later)
		{
			if (child->radio())
			{
				if (!start)
					start = item;
				if (child == this)
					break;
			}
			else
				start = NULL;
		}

		item = g_list_next(item);
	}

	item = start;
	while (item)
	{
		child = (gMenu*)item->data;
		if (child->parent() == pr && !child->_delete_later)
		{
			if (!child->radio())
				break;

			child->setChecked(child == this);
		}

		item = g_list_next(item);
	}
}
