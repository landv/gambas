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
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

***************************************************************************/

#include "widgets.h"
#include "widgets_private.h"
#include "gmainwindow.h"
#include "gmenu.h"

typedef
	struct {
		int x;
		int y;
		}
	MenuPosition;

static GList *menus=NULL;

static void mnu_destroy (GtkWidget *object,gMenu *data)
{
	if (data->stop_signal) 
		data->stop_signal = false; 
	else
		delete data;
}

static void mnu_activate(GtkMenuItem *menuitem,gMenu *data)
{
	if (data->onClick) data->onClick(data);
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
  GtkMenuShell *par;
	GList *iter;
	int pos;
	
	if (!menu->parent)
	{
		//g_debug("get_menu_pos: no parent for menu %p", menu);
		return -1;
	}
	
  par = (GtkMenuShell*)(menu->parent);
  iter = g_list_first(par->children);
  
  for(pos = 0;; pos++) 
  {
    if (iter->data == (gpointer)menu) 
      break; 
    iter = g_list_next(iter);
  }
  
  return pos;
}

static gboolean cb_check_expose(GtkWidget *wid, GdkEventExpose *e, gMenu *menu)
{
	int x, y, w, h;
	
	if (menu->checked())
	{
		x = wid->allocation.x;
		y = wid->allocation.y;
		w = wid->allocation.width;
		h = wid->allocation.height;
		if (h > w)
		{
			y += (h - w) / 2;
			h = w;
		}
		else if (w > h)
		{
			x += (w - h) / 2;
			w = h;
		}
		
		gtk_paint_check(wid->style, wid->window, (GtkStateType)GTK_WIDGET_STATE(wid), GTK_SHADOW_IN, 
			&e->area, NULL, NULL, 
			x + 1, y + 1, w - 2, h - 2); 
	}
	
	return false;
}

void gMenu::update()
{
	GtkWidget *hbox;
	GtkMenuShell *shell = NULL;
	gint pos;
	GtkRequisition req;
	
	if (_no_update)
		return;
	
	//g_debug("%p: START UPDATE (menu = %p child = %p parent = %p)", this, menu);
	
	if (_style != _oldstyle)
	{
		if (child)
		{
			g_object_ref(G_OBJECT(child));
			if (_style == MENU)
				gtk_menu_item_remove_submenu(menu);
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
				gtk_widget_size_request(GTK_WIDGET(menu), &req);
				if (req.height > 5)
					gtk_widget_set_size_request(GTK_WIDGET(menu), -1, 5);
				//g_debug("%p: create new separator %p", this, menu);
			}
			else if (_style == MENU)
			{
				menu = (GtkMenuItem *)gtk_menu_item_new();
				//g_debug("%p: create new menu %p", this, menu);
				
				hbox = gtk_hbox_new(false, 4);
				gtk_container_add(GTK_CONTAINER(menu), GTK_WIDGET(hbox));
				
				label = gtk_label_new_with_mnemonic("");
				//gtk_label_set_justify(GTK_LABEL(lbl),GTK_JUSTIFY_LEFT);
				
				if (!top_level)
				{
					image = gtk_image_new();
					
					aclbl = gtk_label_new("");
					gtk_misc_set_alignment(GTK_MISC(aclbl), 0, 0.5);
					gtk_size_group_add_widget(((gMenu*)pr)->sizeGroup, aclbl);
					
					check = gtk_image_new();
					gtk_widget_set_size_request(check, 16, 16);
					g_signal_connect_after(G_OBJECT(check), "expose-event", G_CALLBACK(cb_check_expose), (gpointer)this);
				
					gtk_box_pack_start(GTK_BOX(hbox), check, false, false, 0);
					gtk_box_pack_start(GTK_BOX(hbox), image, false, false, 0);
					gtk_box_pack_start(GTK_BOX(hbox), label, false, false, 0);
					gtk_box_pack_end(GTK_BOX(hbox), aclbl, false, false, 0);				
				}
				else
				{
					check = NULL;
					image = NULL;
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
				
				set_gdk_fg_color(GTK_WIDGET(menu), win->foreground());
				set_gdk_bg_color(GTK_WIDGET(menu), win->background());	
				
				//gtk_menu_shell_append(GTK_MENU_SHELL(win->menuBar), GTK_WIDGET(menu));
				shell = GTK_MENU_SHELL(win->menuBar);
			}
			else
			{
				gMenu *parent = (gMenu *)pr;
				
				if (!parent->child)
				{
					parent->child = (GtkMenu*)gtk_menu_new();
					parent->setFont();
					//g_debug("%p: creates a new child menu container in parent %p", this, parent->child);
					
					g_signal_connect(G_OBJECT(parent->child), "map", G_CALLBACK(cb_map), (gpointer)parent);
					g_signal_connect(G_OBJECT(parent->child), "unmap", G_CALLBACK(cb_unmap), (gpointer)parent);
					gtk_widget_show_all(GTK_WIDGET(parent->child));
					if (parent->style() == MENU)
						gtk_menu_item_set_submenu(GTK_MENU_ITEM(parent->menu), GTK_WIDGET(parent->child));
					
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
				buf = g_strconcat("    ", _shortcut ,"  ",(void *)NULL);
				gtk_label_set_text(GTK_LABEL(aclbl), buf);
				g_free(buf);
			}
			else
				gtk_label_set_text(GTK_LABEL(aclbl), "      ");
		}
		
		if (!top_level)
		{
			GdkPixbuf *buf;
			buf = _picture ? _picture->getPixbuf() : 0;
			gtk_image_set_from_pixbuf(GTK_IMAGE(image), buf);
			
			if (buf)
				gtk_widget_show(image);
			else
				gtk_widget_hide(image);
				
			if (_checked || !buf)
				gtk_widget_show(check);
			else
				gtk_widget_hide(check);			
		}
		else
		{
			//gtk_widget_hide(image);
			//gtk_widget_hide(check);			
		}
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
	
	_style = NOTHING;
	_oldstyle = NOTHING;
	
	_no_update = false;
	_destroyed = false;
	_action = false;
	_visible = false;
	
	sizeGroup = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
	
	menus=g_list_append (menus,(gpointer)this);
}


gMenu::gMenu(gMainWindow *par,bool hidden)
{
	pr = (gpointer)par;
  initialize();
	top_level=true;
	
	accel = par->accel;
	g_object_ref(accel);
	
	if (!par->menuBar)
	{
		par->menuBar=(GtkMenuBar*)gtk_menu_bar_new();
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
		
	if (menu)
		gtk_widget_destroy(GTK_WIDGET(menu));
	
	if (onFinish) onFinish(this);
	
	menus = g_list_remove(menus, (gpointer)this);
}

bool gMenu::enabled()
{
	return GTK_WIDGET_SENSITIVE(GTK_WIDGET(menu));
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
	
	g_object_set(G_OBJECT(menu),"visible",vl,(void *)NULL);
	
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
		if (mn->pr == (void*)this) ct++;
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
		if (mn->pr == (void*)this)
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
	if (!_destroyed)
		delete this;
}

void gMenu::popup()
{
	guint32 time = gtk_get_current_event_time();
	
	if (!child)
		return;
		
	gtk_menu_popup(child, NULL, NULL, NULL, NULL, 0, time);
}

static void position_menu(GtkMenu *menu, gint *x, gint *y, gboolean *push_in, MenuPosition *pos)
{
	*x = pos->x;
	*y = pos->y;
	*push_in = false;
	delete pos;
}

void gMenu::popup(int x, int y)
{
	guint32 time = gtk_get_current_event_time();
	MenuPosition *pos;
	
	if (!child)
		return;
	
	pos = new MenuPosition;
	pos->x = x;
	pos->y = y;
	
	gtk_menu_popup(child, NULL, NULL, (GtkMenuPositionFunc)position_menu, (gpointer)pos, 0, time);
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
	if (_name) g_free(_name);
	_name = NULL;
	if (name) _name = g_strdup(name);
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
	if (child)
	{
		gMainWindow *win = window();
		//fprintf(stderr, "set menu child font\n");
		gtk_widget_modify_font(GTK_WIDGET(child), win->ownFont() ? win->font()->desc() : NULL);
	}
}


void gMenu::updateFont(gMainWindow *win)
{
	GList *item;
	gMenu *mn;

	if (win->menuBar)
	{
		//fprintf(stderr, "set menu bar font\n");
		gtk_widget_modify_font(GTK_WIDGET(win->menuBar), win->ownFont() ? win->font()->desc() : NULL);	
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
