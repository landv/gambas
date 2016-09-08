/***************************************************************************

	gtextbox.cpp

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
#include "gkey.h"
#include "gtextbox.h"

#ifdef GTK3

#define MAX_ICONS 2

#if GTK_CHECK_VERSION(3,14,0)
	
struct _GtkEntryPrivate
{
  void         *icons[MAX_ICONS];

  GtkEntryBuffer        *buffer;
  GtkIMContext          *im_context;
  GtkWidget             *popup_menu;

  GdkWindow             *text_area;
};

#else

struct _GtkEntryPrivate
{
	void         *icons[MAX_ICONS];

	GtkEntryBuffer        *buffer;
	GtkIMContext          *im_context;
	GtkWidget             *popup_menu;

	GdkDevice             *device;

	GdkWindow             *text_area;
};

#endif

#endif

#ifdef GTK3
GtkCssProvider *gTextBox::_style_provider = NULL;
#endif

static gboolean raise_change(gTextBox *data)
{
	if (data->_changed)
	{
		data->emit(SIGNAL(data->onChange));
		data->_changed = false;
	}
	return FALSE;
}

static void cb_before_insert(GtkEditable *editable, gchar *new_text, gint new_text_length, gint *position, gTextBox *data)
{
	if (gKey::gotCommit())
	{
		gcb_im_commit(NULL, new_text, NULL);
		if (gKey::canceled())
			g_signal_stop_emission_by_name (G_OBJECT(editable), "insert-text");
		*position = gtk_editable_get_position(editable);
	}
}

static void cb_change_insert(GtkEditable *editable, gchar *new_text, gint new_text_length, gint *position, gTextBox *data)
{
	data->_changed = false;
	gtk_editable_set_position(editable, *position);
	data->emit(SIGNAL(data->onChange));
	*position = gtk_editable_get_position(editable);
}

static void cb_change_delete(GtkEditable *editable, gint start_pos, gint end_pos, gTextBox *data)
{
	if (!data->_changed)
	{
		data->_changed = true;
		g_timeout_add(0, (GSourceFunc)raise_change, data);
	}
}

static void cb_activate(GtkEntry *editable,gTextBox *data)
{
	data->emit(SIGNAL(data->onActivate));
}


gTextBox::gTextBox(gContainer *parent, bool combo) : gControl(parent)
{
#ifdef GTK3
	if (!_style_provider)
	{
		const char *css;

		_style_provider = gtk_css_provider_new();

		/*if (strcmp(gApplication::getStyleName(), "Clearlooks-Phenix") == 0)
			css = ".entry { border-width: 0; padding: 0; border-radius: 0; margin: 0; border-style: none; box-shadow: none; background-image: none; }";
		else*/
		css = "* { border: none; border-radius: 0; margin: 0; box-shadow: none; }";

		gtk_css_provider_load_from_data(_style_provider, css, -1, NULL);
	}

	g_object_ref(_style_provider);
#endif

	if (!combo)
	{
		g_typ=Type_gTextBox;
		
		have_cursor = true;
		_no_background = TRUE;
		
		entry = widget = gtk_entry_new();
		realize();
		setColorBase();
		initEntry();
	}
	
	_changed = false;
	_border = true;
	
	onChange = NULL;
	onActivate = NULL;
}

gTextBox::~gTextBox()
{
#ifdef GTK3
	g_object_unref(_style_provider);
#endif
}

/*static void cb_im_commit(GtkIMContext *context, const char *str, gpointer pointer)
{
	fprintf(stderr, "cb_im_commit: %s\n", str);
}*/

void gTextBox::initEntry()
{
	_has_input_method = entry != NULL;

	if (!entry)
		return;
	
	// This imitates the QT behaviour, where change signal is raised after the cursor has been moved.
	g_signal_connect(G_OBJECT(entry), "insert-text", G_CALLBACK(cb_before_insert), (gpointer)this);
	g_signal_connect_after(G_OBJECT(entry), "insert-text", G_CALLBACK(cb_change_insert), (gpointer)this);
	g_signal_connect_after(G_OBJECT(entry), "delete-text", G_CALLBACK(cb_change_delete), (gpointer)this);
	g_signal_connect(G_OBJECT(entry), "activate", G_CALLBACK(cb_activate), (gpointer)this);
	//g_signal_connect(getInputMethod(), "commit", G_CALLBACK(cb_im_commit), (gpointer)this);
}

char* gTextBox::text()
{
	return (char*)gtk_entry_get_text(GTK_ENTRY(entry));
}

void gTextBox::setText(const char *vl)
{
	if (!vl) vl = "";
	
	if (!entry || !strcmp(vl, text()))
		return;
		
	lock();
	gtk_entry_set_text(GTK_ENTRY(entry), vl);
	gtk_editable_set_position(GTK_EDITABLE(entry), -1);
	unlock();
	emit(SIGNAL(onChange));
}

bool gTextBox::password()
{
	if (entry)
		return !gtk_entry_get_visibility(GTK_ENTRY(entry));
	else
		return false;
}

void gTextBox::setPassword(bool vl)
{
	if (!entry)
		return;
		
	gtk_entry_set_visibility(GTK_ENTRY(entry),!vl);
	if (vl)
		gtk_entry_set_invisible_char(GTK_ENTRY(entry), (gunichar)0x25CF);
}

bool gTextBox::isReadOnly()
{
	return !gtk_editable_get_editable(GTK_EDITABLE(entry));
}

void gTextBox::setReadOnly(bool vl)
{
	gtk_editable_set_editable(GTK_EDITABLE(entry),!vl);
}

int gTextBox::position()
{
	if (entry)
		return gtk_editable_get_position(GTK_EDITABLE(entry));
	else
		return 0;
}

void gTextBox::setPosition(int pos)
{
	int len;
	
	if (!entry)
		return;
		
	len = length();
		
	if (pos < 0) 
		pos = 0;
	else if (pos > len)
		pos = -1;

	gtk_editable_set_position(GTK_EDITABLE(entry), pos);
}

bool gTextBox::hasBorder()
{
	if (entry)
		return _border; //gtk_entry_get_has_frame(GTK_ENTRY(entry));
	else
		return true;
}

void gTextBox::setBorder(bool vl)
{
	if (!entry)
		return;
	
	if (vl == hasBorder())
		return;
	
	_border = vl;
	
#ifdef GTK3
	GtkStyleContext *style = gtk_widget_get_style_context(entry);
	if (vl)
		gtk_style_context_remove_provider(style, GTK_STYLE_PROVIDER(_style_provider));
	else
		gtk_style_context_add_provider(style, GTK_STYLE_PROVIDER(_style_provider), GTK_STYLE_PROVIDER_PRIORITY_USER);

	//gtk_style_context_invalidate(style);
#else
	gtk_entry_set_has_frame(GTK_ENTRY(entry), vl);
	
	/*if (vl)
		gtk_entry_set_inner_border(GTK_ENTRY(entry), NULL);
	else
	{
		GtkBorder *border = gtk_border_new();
		border->left = border->right = 2;
		border->top = 1;
		gtk_entry_set_inner_border(GTK_ENTRY(entry), border);
		gtk_border_free(border);
	}*/
#endif
}

void gTextBox::insert(char *txt, int len)
{
	if (!entry || !len || !txt) return;
	
	lock();
	gtk_editable_delete_selection(GTK_EDITABLE(entry));
	unlock();
	int pos = position();
	gtk_editable_insert_text(GTK_EDITABLE(entry), txt, len, &pos);
}

int gTextBox::length()
{
	const gchar *buf;
	
	if (!entry)
		return 0;
	
	buf=gtk_entry_get_text(GTK_ENTRY(entry));
	if (!buf) return 0;
	
	return g_utf8_strlen(buf, -1);
}

int gTextBox::maxLength()
{
	if (entry)
		return gtk_entry_get_max_length(GTK_ENTRY(entry));
	else
		return 0;
}

void gTextBox::setMaxLength(int vl)
{
	if (!entry)
		return;
	if (vl<0) vl=0;
	if (vl>65536) vl=0;
	gtk_entry_set_max_length(GTK_ENTRY(entry), vl);
	
}

bool gTextBox::isSelected()
{
	if (entry)
		return gtk_editable_get_selection_bounds(GTK_EDITABLE(entry), NULL, NULL);
	else
		return false;
}

int gTextBox::selStart()
{
	int start;
	
	if (!entry)
		return -1;
	
	gtk_editable_get_selection_bounds(GTK_EDITABLE(entry),&start,NULL);
	return start;
}

int gTextBox::selLength()
{
	int start,end;

	if (!entry)
		return 0;

	gtk_editable_get_selection_bounds(GTK_EDITABLE(entry),&start,&end);
	return end - start;
}

char* gTextBox::selText()
{
	int start,end;

	if (!entry)
		return NULL;
	gtk_editable_get_selection_bounds(GTK_EDITABLE(entry),&start,&end);
	return gtk_editable_get_chars(GTK_EDITABLE(entry),start,end);
	
}

void gTextBox::setSelText(char *txt,int len)
{
	int start,end;

	if (!entry)
		return;

	gtk_editable_get_selection_bounds(GTK_EDITABLE(entry),&start,&end);	
	gtk_editable_delete_text(GTK_EDITABLE(entry),start,end);
	gtk_editable_insert_text(GTK_EDITABLE(entry),txt,len,&start);
	
}

void gTextBox::selClear()
{
	int start;

	if (!entry)
		return;

	gtk_editable_get_selection_bounds(GTK_EDITABLE(entry),&start,NULL);	
	gtk_editable_select_region(GTK_EDITABLE(entry),start,start);
}

void gTextBox::selectAll()
{
	if (entry)
		gtk_editable_select_region(GTK_EDITABLE(entry), 0, -1);
}

void gTextBox::select(int start,int len)
{
	if (!entry)
		return;
	if ( (len<=0) || (start<0) ) { selClear(); return; }
	gtk_editable_select_region(GTK_EDITABLE(entry),start,start+len);
}


int gTextBox::alignment()
{
	if (entry)
		return gt_to_alignment(gtk_entry_get_alignment(GTK_ENTRY(entry)));
	else
		return ALIGN_NORMAL;
}

void gTextBox::setAlignment(int al)
{
	if (!entry)
		return;
	gtk_entry_set_alignment(GTK_ENTRY(entry), gt_from_alignment(al));
}

void gTextBox::updateCursor(GdkCursor *cursor)
{
	GdkWindow *win;
	
	gControl::updateCursor(cursor);
	if (!entry)
		return;

#ifdef GTK3
	win = GTK_ENTRY(entry)->priv->text_area;
#else
	win = GTK_ENTRY(entry)->text_area;
#endif

	if (!win)
		return;
		
	if (cursor)
		gdk_window_set_cursor(win, cursor);
	else
	{
		cursor = gdk_cursor_new_for_display(gtk_widget_get_display(widget), GDK_XTERM);
		gdk_window_set_cursor(win, cursor);
#ifdef GTK3
		g_object_unref(cursor);
#else
		gdk_cursor_unref(cursor);
#endif
	}
}

void gTextBox::clear()
{
	setText("");
}


int gTextBox::minimumHeight()
{
	/*GtkRequisition req;
	
	gtk_widget_size_request(widget, &req);
	return req.height - 4;*/
	return font()->height() + hasBorder() ? 4 : 2;
}

GtkIMContext *gTextBox::getInputMethod()
{
#ifdef GTK3
	return entry ? GTK_ENTRY(entry)->priv->im_context : NULL;
#else
	return entry ? GTK_ENTRY(entry)->im_context : NULL;
#endif
}


void gTextBox::getCursorPos(int *x, int *y, int pos)
{
	int px, py;
	PangoLayout *layout;
	PangoRectangle rect;
	
	layout = gtk_entry_get_layout(GTK_ENTRY(entry));
	pos = gtk_entry_text_index_to_layout_index(GTK_ENTRY(entry), pos < 0 ? position() : pos);
	pango_layout_get_cursor_pos(layout, pos, &rect, NULL);
	
	gtk_entry_get_layout_offsets(GTK_ENTRY(entry), &px, &py);
	
#ifdef GTK3
	GdkRectangle r;
	gtk_entry_get_text_area(GTK_ENTRY(entry), &r);
	py = r.y;
#endif
	
	*x = px + PANGO_PIXELS(rect.x);
	*y = py + PANGO_PIXELS(rect.y + rect.height);
}

void gTextBox::setFocus()
{
	bool r = isReadOnly();
	
	if (!r)
		setReadOnly(true);
	gControl::setFocus();
	if (!r)
		setReadOnly(false);
}
