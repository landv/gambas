/***************************************************************************

  gbutton.cpp

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
#include "widgets_private.h"
#include "gapplication.h"
#include "gmainwindow.h"
#include "gbutton.h"

#include <unistd.h>

static void cb_click(GtkButton *object, gButton *data)
{
	if (data->disable)
	{
		data->disable=false;
		return;
	}
		
	if (!gApplication::userEvents()) return;

	data->unsetOtherRadioButtons();

	if (data->type == gButton::Tool)
	{
		if (!data->isToggle())
		{
			data->disable = true;
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data->widget), false);
		}
	}

	data->emit(SIGNAL(data->onClick));
}

static void cb_click_radio(GtkButton *object,gControl *data)
{
	if (!gApplication::userEvents()) return;

	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(object)))
		if (((gButton*)data)->onClick) ((gButton*)data)->onClick((gControl*)data);
	return;
}

static void cb_click_check(GtkButton *object, gButton *data)
{
	if (data->isTristate() && !data->locked())
	{
		data->lock();
		if (data->inconsistent())
		{
			data->setInconsistent(false);
			data->setValue(false);
		}
		else if (!data->value())
			data->setInconsistent(true);
		data->unlock();
	}

	data->emit(SIGNAL(data->onClick));
}


static bool button_expose(GtkWidget *wid,GdkEventExpose *e,gButton *data)
{
	GdkGC        *gc;
	GdkPixbuf    *img;
	GdkRectangle rpix={0,0,0,0};
	GdkRectangle rect;
	GtkCellRendererState state;
	gint         py,px;
	bool         rtl,bcenter=false;
	gint dx, dy;
	GdkWindow *win;

		
	rtl = gtk_widget_get_default_direction() == GTK_TEXT_DIR_RTL;

	rect = wid->allocation;
	win = wid->window;
	
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

	//g_debug("button_expose: %d %d %d %d", e->area.x, e->area.y, e->area.width, e->area.height);
	//g_debug("rect: %d %d %d %d", rect.x, rect.y, rect.width, rect.height);

	if (data->rendpix)
	{
		if (GTK_WIDGET_STATE(data->widget)==GTK_STATE_INSENSITIVE) 
		{
		  if (!data->rendinc)
		    data->rendinc = gt_pixbuf_create_disabled(data->rendpix);
		  img = data->rendinc;
    }
		else
		  img = data->rendpix;

		rpix.width = gdk_pixbuf_get_width(img);
		rpix.height = gdk_pixbuf_get_height(img);
		
		py = (rect.height - rpix.height)/2;
		
		gc=gdk_gc_new(win);
		gdk_gc_set_clip_origin(gc,0,0);
		gdk_gc_set_clip_rectangle(gc,&e->area);

		bcenter = !(data->text()) || !(*data->text());
		
		if (bcenter) 
		{	
			//fprintf(stderr, "draw pixbuf: %d %d\n", rect.x + (px-rpix.width)/2, rect.y + py);
			gdk_draw_pixbuf(GDK_DRAWABLE(win),gc,img,0,0,rect.x + (px-rpix.width)/2, rect.y + py,
                                        -1,-1,GDK_RGB_DITHER_MAX,0,0);
			g_object_unref(gc);
			return false;
		}

		if (rtl)
			gdk_draw_pixbuf(GDK_DRAWABLE(win),gc,img,0,0,rect.x + rect.width - 6, rect.y + py,
                                        -1,-1,GDK_RGB_DITHER_MAX,0,0);
		else
			gdk_draw_pixbuf(GDK_DRAWABLE(win),gc,img,0,0,rect.x + 6, rect.y + py,
                                        -1,-1,GDK_RGB_DITHER_MAX,0,0);

		g_object_unref(G_OBJECT(gc));
		
		rect.width -= rpix.width;
		rect.x += rpix.width;
	}
	
	gt_set_cell_renderer_text_from_font((GtkCellRendererText *)data->rendtxt, data->font());
	g_object_set(G_OBJECT(data->rendtxt),"sensitive",true, (void *)NULL);
	
	switch(GTK_WIDGET_STATE(data->widget))
	{
		//case GTK_STATE_NORMAL:
		//case GTK_STATE_ACTIVE: state=GTK_CELL_RENDERER_PRELIT; break;
		//case GTK_STATE_PRELIGHT: state=GTK_CELL_RENDERER_PRELIT; break;
		case GTK_STATE_SELECTED: 
			state=GTK_CELL_RENDERER_SELECTED; 
			break;
		
		case GTK_STATE_INSENSITIVE: 
			state=GTK_CELL_RENDERER_INSENSITIVE; 
			g_object_set(G_OBJECT(data->rendtxt),"sensitive",false, (void *)NULL); 
			break;
			
		default:
			state=(GtkCellRendererState)0; 
			break;
	}
	
	
	/*rect.width-=12;
	rect.x+=6;
	if (rtl)
	{
		rect.width=px-rect.x-6;
		rect.x=6;
	}*/
	
	if (rect.width >= 1 && rect.height >= 1)
	{
		gtk_cell_renderer_set_fixed_size(data->rendtxt,rect.width,rect.height);
		gtk_cell_renderer_render(data->rendtxt,win,wid,&rect,&rect,&rect,state);
	}
	
	return FALSE;
}

gButton::gButton(gContainer *par, Type typ) : gControl(par)
{
	gContainer *ct;
	bool invisible;

	g_typ = Type_gButton;
	
	_toggle = false;
	_radio = false;
	_animated = false;
	_stretch = true;
	_tristate = false;
	_autoresize = false;
	scaled = false;
	disable = false;
	bufText = NULL;
	rendtxt = NULL;
	label = NULL;
	rendpix = NULL;
	rendinc = NULL;
	pic = NULL;
	shortcut = 0;
	
	//border=gtk_event_box_new();
	
	invisible = false;
	
	switch(typ)
	{
		case Toggle:
			rendtxt = gtk_cell_renderer_text_new();
			border = gtk_toggle_button_new();
			break;
			
		case Check:
			border = gtk_check_button_new();
			break;
			
		case Radio:
			ct = parent();
			if (!ct->radiogroup) 
			{
				ct->radiogroup = gtk_radio_button_new(NULL);
				g_object_ref(ct->radiogroup);
				border = gtk_radio_button_new_from_widget(GTK_RADIO_BUTTON(ct->radiogroup));
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(border),true);
			}
			else 
			{
				border = gtk_radio_button_new_from_widget(GTK_RADIO_BUTTON(ct->radiogroup));
			}
			break;
		
		case Tool:
			rendtxt = gtk_cell_renderer_text_new();
			border = gtk_toggle_button_new();
			gtk_button_set_focus_on_click(GTK_BUTTON(border), false);
			invisible = true;
			break;
		
		default:
			border=gtk_button_new();
			rendtxt=gtk_cell_renderer_text_new();
			typ = Button;
			break;
	}

  widget = border;
  border = gtk_event_box_new();
	if (invisible)
		gtk_event_box_set_visible_window(GTK_EVENT_BOX(border), false);
																
  type = typ;

	if (rendtxt) 
	{
		g_object_set(G_OBJECT(rendtxt),"xalign",0.5,(void *)NULL);
		g_object_set(G_OBJECT(rendtxt),"yalign",0.5,(void *)NULL);

		g_signal_connect_after(G_OBJECT(widget),"expose-event",G_CALLBACK(button_expose),(gpointer)this);
	}
	else
	{	
		label = gtk_label_new_with_mnemonic("");
		gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_CENTER);
		gtk_container_add (GTK_CONTAINER(widget), GTK_WIDGET(label));
	}
	
	realize();
	
	gtk_widget_add_events(widget, GDK_POINTER_MOTION_MASK);
	onClick=NULL;
	
	if (type == Radio)
		g_signal_connect(G_OBJECT(widget),"clicked",G_CALLBACK(cb_click_radio),(gpointer)this);
	else if (type == Check)
		g_signal_connect(G_OBJECT(widget), "clicked", G_CALLBACK(cb_click_check), (gpointer)this);	
	else
		g_signal_connect(G_OBJECT(widget),"clicked",G_CALLBACK(cb_click),(gpointer)this);	
	
	
	if (type == Tool) 
    setBorder(false);
}

gButton::~gButton()
{
	setDefault(false);
	setCancel(false);
  setPicture(0);
  g_free(bufText);  
}

void gButton::setInconsistent(bool vl)
{
	if (type != Check) return;

	gtk_toggle_button_set_inconsistent (GTK_TOGGLE_BUTTON(widget),vl);
}

bool gButton::inconsistent()
{
	gboolean vl=false;

	if (type != Check) return false;
	g_object_get (G_OBJECT(widget),"inconsistent",&vl,(void *)NULL);
	return vl;	
}

#if 0
void gButton::setFont(gFont *ft)
{
	PangoFontDescription *desc=pango_context_get_font_description(ft->ct);
	GtkBox *box;
	GtkLabel *lbl;
	GList *chd;

  gControl::setFont(ft);

	return; /* TODO */
	
	font_change=true;
	if (type==4) { gtk_widget_modify_font(widget,desc); return; }

	if (rendtxt)
	{
	}
	else
	{
		box=(GtkBox*)gtk_bin_get_child(GTK_BIN(widget));
		chd=gtk_container_get_children(GTK_CONTAINER(box));
		chd=chd->next;
		lbl=GTK_LABEL(chd->data);
		gtk_widget_modify_font(GTK_WIDGET(lbl),desc);
		g_list_free(chd);
	}
}
#endif

void gButton::setFont(gFont *ft)
{
	gControl::setFont(ft);
	
	if (label)
		gtk_widget_modify_font(label, fnt ? fnt->desc() : NULL);
	
	updateSize();
}

bool gButton::enabled()
{
	return GTK_WIDGET_SENSITIVE(widget);
}

void gButton::setEnabled(bool vl)
{
	gtk_widget_set_sensitive(widget, vl);
}

#if 0
void gButton::setBackground(long color)
{
	GtkWidget *wid;

	wid=(type == Check) ? border : widget;
	set_gdk_bg_color(wid,color);
	if (!border->window) gtk_widget_realize(border);
	gdk_window_process_updates(border->window,true);
}

long gButton::background()
{
	if (type == Check) get_gdk_bg_color(border);
	return get_gdk_bg_color(widget);
}

void gButton::setForeground(long color)
{
	GdkColor col;
	GtkWidget *lbl;

	set_gdk_fg_color(widget,color);	
	
	if (rendtxt)
	{
		fill_gdk_color(&col,color);
		g_object_set(G_OBJECT(rendtxt),"foreground-set",TRUE,NULL);
		g_object_set(G_OBJECT(rendtxt),"foreground-gdk",&col,NULL);
	}
	else
	{
		lbl=gtk_bin_get_child(GTK_BIN(widget));
		set_gdk_fg_color(lbl,color);
	}
	
	if (!border->window) gtk_widget_realize(border);
	gdk_window_process_updates(border->window,true);
}

long gButton::foreground()
{
	return get_gdk_fg_color(widget);
}
#endif

const char* gButton::text()
{
	//if (type == Tool) return this->toolTip();
	return bufText;
}

void gButton::setText(const char *st)
{
	GtkLabel *lbl=NULL;
	GtkAccelGroup *accel;
	char *buf;
	
	accel = window()->accel;
		
	if (bufText) 
	{
		if (shortcut)
			gtk_widget_remove_accelerator(widget, accel, (guint)shortcut, GDK_MOD1_MASK);
		g_free(bufText);
	}

 	bufText = g_strdup(st);
	updateSize();
	resize();
	
	if (!rendtxt)
	{
		lbl=(GtkLabel*)gtk_bin_get_child(GTK_BIN(widget));
		if (!st) st="";
		if (!strlen(st))
		{ 
			gtk_label_set_text(lbl,"");
			g_object_set(G_OBJECT(lbl),"visible",false,(void *)NULL);
			return;
		}
		gMnemonic_correctText((char*)st,&buf);
		gtk_label_set_text_with_mnemonic(lbl,buf);
		g_object_set(G_OBJECT(lbl),"visible",true,(void *)NULL);
		g_free(buf);
		return;
		
	}
	
	shortcut = (int)gMnemonic_correctMarkup((char*)st,&buf);
	
	if (shortcut) 
		gtk_widget_add_accelerator(widget, "clicked", accel, (guint)shortcut, GDK_MOD1_MASK, (GtkAccelFlags)0);
	
	g_object_set(G_OBJECT(rendtxt),"markup",buf,(void *)NULL);
	
	g_free(buf);
	
	refresh();
}


gPicture* gButton::picture()
{
	if ( (type == Check) || (type == Radio) ) 
    return NULL;
	
	return pic;	
}

void gButton::setPicture(gPicture *npic)
{
	GdkPixbuf *new_rendpix = NULL;
	
	if ((type == Check) || (type == Radio)) return;
	
	gPicture::assign(&pic, npic);
	
	if (pic)
	{
   	new_rendpix = pic->getPixbuf();
    if (new_rendpix)
    	g_object_ref(new_rendpix);
  }
  
	if (rendpix) { g_object_unref(G_OBJECT(rendpix)); rendpix = NULL; }
	if (rendinc) { g_object_unref(G_OBJECT(rendinc)); rendinc = NULL; }
	
	rendpix = new_rendpix;
	
	updateSize();
	refresh();
}

bool gButton::getBorder()
{
	switch(gtk_button_get_relief(GTK_BUTTON(widget)))
	{
		case GTK_RELIEF_NORMAL:
		case GTK_RELIEF_HALF:
			return true;
		default: 
			return false;
	}
}

void gButton::setBorder(bool vl)
{
	if (vl)
		gtk_button_set_relief (GTK_BUTTON(widget),GTK_RELIEF_NORMAL);
	else
		gtk_button_set_relief (GTK_BUTTON(widget),GTK_RELIEF_NONE);
}

bool gButton::isDefault()
{
	gMainWindow *win = window();	
	return win ? win->_default == this : false;
}

void gButton::setDefault(bool vl)
{
	gMainWindow *win = window();
	
	if (type != Button || !win)
		return;
	
	if (vl)
	{
		win->_default = this;
		GTK_WIDGET_SET_FLAGS (widget, GTK_CAN_DEFAULT);
		//gtk_widget_grab_default (widget);
	}
	else
	{
		GTK_WIDGET_UNSET_FLAGS(widget, GTK_CAN_DEFAULT);
		if (win->_default == this)
			win->_default = NULL;
	}
}

bool gButton::isCancel()
{
	gMainWindow *win = window();	
	return win ? win->_cancel == this : false;
}

void gButton::setCancel(bool vl)
{
	gMainWindow *win = window();
	
	if (type != Button || !win) 
    return;
	
	if (vl)
		win->_cancel = this;
	else if (win->_cancel == this)
		win->_cancel = NULL;
}

bool gButton::value()
{
  if (type == Button)
    return false;
  else
		return gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(widget));
}

void gButton::setValue(bool vl)
{
  if (type == Button)
  {
    if (vl) gtk_button_clicked(GTK_BUTTON(widget));
  }
  else
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(widget),vl);
}

void gButton::setToggle(bool vl)
{
	if (type != Tool) return;
	_toggle = vl;
}

bool gButton::isToggle()
{
	return type == Toggle || type == Check || type == Radio || _toggle;
}

void gButton::animateClick(bool on)
{
	if (type != Button) return;
	
	if (!on && !_animated)
	{
		gtk_widget_set_state(widget, GTK_STATE_ACTIVE);
		_animated = true;
	}
	else if (on && _animated)
	{
		_animated = false;
		gtk_widget_set_state(widget, GTK_STATE_NORMAL);
		gtk_button_clicked(GTK_BUTTON(widget));
	}
}

int gButton::minimumHeight()
{
	int mh = 0;
	
	if (bufText && *bufText)
	{
		if (type == Button || type == Toggle || type == Tool)
			mh = font()->height() + 8;
		else
			mh = font()->height() + 2;
	}
	
	if (pic && (pic->height() > mh))
		mh = pic->height();
	
	return mh;
}

void gButton::setRadio(bool vl)
{
	_radio = vl;
	if (value())
		unsetOtherRadioButtons();
}

bool gButton::isRadio()
{
	return type == Radio || _radio;
}

void gButton::unsetOtherRadioButtons()
{
	gContainer *pr = parent();
	gControl *child;
	gButton *button;
	int i;
	
	if (type == Radio || type == Button || !isRadio() || !isToggle())
		return;
	
	for (i = 0; i < pr->childCount(); i++)
	{
		child = pr->child(i);
		if (child->getClass() != getClass())
			continue;
			
		button = (gButton *)child;
		
		if (button == this)
		{
			if (!value())
			{
				button->disable = true;
				button->setValue(true);
			}
		}
		else if (button->type == type && button->isRadio() && button->isToggle() && button->value())
		{
			button->disable = true;
			button->setValue(false);
		}
	}
}

bool gButton::hasShortcut()
{
	return isDefault() || isCancel() || shortcut;
}


void gButton::setStretch(bool vl)
{
	_stretch = vl;
}

void gButton::setRealForeground(gColor color)
{
	gControl::setRealForeground(color);

	if (rendtxt)
	{
		if (color == COLOR_DEFAULT)
		{
			g_object_set(G_OBJECT(rendtxt),
				"foreground-set", FALSE,
				(void *)NULL);
		}
		else
		{
			GdkColor col;
			fill_gdk_color(&col, color);
			g_object_set(G_OBJECT(rendtxt),
				"foreground-set", TRUE,
				"foreground-gdk", &col,
				(void *)NULL);
		}
	}
	
	if (label)
		set_gdk_fg_color(label, color);
}

void gButton::setTristate(bool vl)
{
	_tristate = vl;
	if (!_tristate)
		setInconsistent(false);
}

void gButton::setAutoResize(bool vl)
{
	_autoresize = vl;
	updateSize();
}

void gButton::updateSize()
{
	GtkRequisition req;
	int mw, mh;
	
	if (!_autoresize)
		return;
	
	mh = minimumHeight();
	mw = 0;
	
	if (bufText && *bufText)
	{
		int m = font()->width(bufText, strlen(bufText));
	
		if (type == Check || type == Radio)
		{
			g_signal_emit_by_name(border, "size-request",	&req);
			m += req.width;
			if (req.height > m)
				m = req.height;
		}
		
		mw += m;
	}
	
	if (pic)
	{
		if (mw) mw += 8;
		mw += pic->width();
	}
	
	mw += 16;
	
	if (mh < height())
		mh = height();

	resize(mw, mh);
}
