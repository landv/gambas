/***************************************************************************

  gtextarea.cpp

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
#include "gclipboard.h"
#include "gdesktop.h"
#include "gtextarea.h"

#ifdef GTK3
#else

// Private structure took from GTK+ 2.10 code. Used for setting the text area cursor.
// **** May not work on future GTK+ versions ****

struct PrivateGtkTextWindow
{
  GtkTextWindowType type;
  GtkWidget *widget;
  GdkWindow *window;
  GdkWindow *bin_window;
  GtkRequisition requisition;
  GdkRectangle allocation;
};

// Private exported functions to get the size of the text
// **** May not work on future GTK+ versions ****

extern "C" {
void gtk_text_layout_set_screen_width(struct _GtkTextLayout *layout, gint width);
void gtk_text_layout_get_size(struct _GtkTextLayout  *layout, gint *width, gint *height);
void gtk_text_layout_invalidate (struct _GtkTextLayout *layout, const GtkTextIter *start, const GtkTextIter *end);
void gtk_text_layout_validate (struct _GtkTextLayout *layout, gint max_pixels);
}
#endif

// Undo/Redo actions

enum { ACTION_VOID, ACTION_INSERT, ACTION_DELETE };

class gTextAreaAction
{
public:
	gTextAreaAction *prev;
	gTextAreaAction *next;
	GString *text;
	int length;
	int start;
	int end;
	unsigned mergeable : 1;
	unsigned delete_key_used : 1;
	unsigned type : 2;
	
	gTextAreaAction();
	~gTextAreaAction();
	static gTextAreaAction *insertAction(GtkTextBuffer *buffer, char *text, int length, GtkTextIter *where);
	static gTextAreaAction *deleteAction(GtkTextBuffer *buffer, GtkTextIter *start, GtkTextIter *end);
	bool canBeMerged(gTextAreaAction *prev);
	void addText(char *text, int length);
};

gTextAreaAction::gTextAreaAction()
{
	prev = next = NULL;
	text = NULL;
	length = start = end = 0;
	mergeable = delete_key_used = false;
	type = ACTION_VOID;
}

gTextAreaAction::~gTextAreaAction()
{
	if (text)
		g_string_free(text, TRUE);
}

gTextAreaAction *gTextAreaAction::insertAction(GtkTextBuffer *buffer, char *text, int len, GtkTextIter *where)
{
	gTextAreaAction *action = new gTextAreaAction;
	
	action->type = ACTION_INSERT;
	action->start = gtk_text_iter_get_offset(where);
	action->text = g_string_new_len(text, len);
	action->length = g_utf8_strlen(text, len);
	
	action->mergeable = (len == 1) && *text != '\r' && *text != '\n' && *text != ' ';
	
	return action;
}

gTextAreaAction *gTextAreaAction::deleteAction(GtkTextBuffer *buffer, GtkTextIter *start, GtkTextIter *end)
{
	GtkTextIter insert_iter;
	int insert_offset;
	GtkTextMark *mark;
	gTextAreaAction *action = new gTextAreaAction;
	char *text;
	
	action->type = ACTION_DELETE;
	text = gtk_text_buffer_get_text(buffer, start, end, FALSE);
	action->text = g_string_new(text);
	action->length = g_utf8_strlen(action->text->str, action->text->len);
	g_free(text);
	action->start = gtk_text_iter_get_offset(start);
	action->end = gtk_text_iter_get_offset(end);
	
	mark = gtk_text_buffer_get_insert(buffer);
	gtk_text_buffer_get_iter_at_mark(buffer, &insert_iter, mark);
	insert_offset = gtk_text_iter_get_offset(&insert_iter);
	
	action->delete_key_used = insert_offset < action->start;
	
	action->mergeable = (action->length == 1) && *(action->text->str) != '\r' && *(action->text->str) != '\n' && *(action->text->str) != ' ';
	
	return action;
}

bool gTextAreaAction::canBeMerged(gTextAreaAction *prev)
{
	if (!prev || prev->type != type)
		return false;
	
	if (!mergeable || !prev->mergeable)
		return false;
	
	if (type == ACTION_INSERT)
	{
		if (start != (prev->start + prev->length))
			return false;
		if (isspace(*text->str) != isspace(*prev->text->str))
			return false;
	}
	else if (type == ACTION_DELETE)
	{
		if (prev->delete_key_used != delete_key_used)
			return false;
		if (prev->start != start && prev->start != end)
			return false;
		if (isspace(*text->str) != isspace(*prev->text->str))
			return false;
	}
	else
		return false;
	
	return true;
}

void gTextAreaAction::addText(char *add, int len)
{
	g_string_append_len(text, add, len);
	length += g_utf8_strlen(add, len);
}


// Callbacks

/*static gboolean cb_motion_notify_event(GtkWidget *widget, GdkEventMotion *event, gTextArea *data)
{
	// Workaround GtkTextView internal cursor changes
	if (gApplication::isBusy())
		data->setMouse(data->mouse());
	return FALSE;
}*/

static void cb_changed(GtkTextBuffer *buf, gTextArea *data)
{
	data->emit(SIGNAL(data->onChange));
} 

static void cb_mark_set(GtkTextBuffer *buf, GtkTextIter *location, GtkTextMark *mark, gTextArea *data)
{
	if (mark == gtk_text_buffer_get_insert(buf))
		data->emit(SIGNAL(data->onCursor));
} 

static void cb_insert_text(GtkTextBuffer *buf, GtkTextIter *location, gchar *text, gint len, gTextArea *ctrl)
{
	gTextAreaAction *action, *prev;
	
	if (!ctrl->_undo_in_progress)
		ctrl->clearRedoStack();
	
	if (ctrl->_not_undoable_action)
		return;
	
	action = gTextAreaAction::insertAction(buf, text, len, location);
	
	prev = ctrl->_undo_stack;
	if (action->canBeMerged(prev))
	{
		prev->addText(action->text->str, action->length);
		//fprintf(stderr, "insert merge: %d '%.*s'\n", prev->start, prev->text->len, prev->text->str);
		delete action;
	}
	else
	{
		//fprintf(stderr, "insert: %d '%.*s'\n", action->start, action->text->len, action->text->str);
		action->next = ctrl->_undo_stack;
		if (ctrl->_undo_stack)
			ctrl->_undo_stack->prev = action;
		ctrl->_undo_stack = action;
	}
}

static void cb_delete_range(GtkTextBuffer *buf, GtkTextIter *start, GtkTextIter *end, gTextArea *ctrl)
{
	gTextAreaAction *action, *prev;
	
	if (!ctrl->_undo_in_progress)
		ctrl->clearRedoStack();
	
	if (ctrl->_not_undoable_action)
		return;
	
	action = gTextAreaAction::deleteAction(buf, start, end);
	
	prev = ctrl->_undo_stack;
	if (action->canBeMerged(prev))
	{
		if (prev->start == action->start)
		{
			prev->addText(action->text->str, action->length);
			prev->end += action->end - action->start;
		}
		else
		{
			GString *text = prev->text;
			prev->text = action->text;
			action->text = NULL;
			prev->addText(text->str, text->len);
			g_string_free(text, TRUE);
			prev->start = action->start;
		}
		//fprintf(stderr, "delete merge: %d %d '%.*s'\n", prev->start, prev->end, prev->text->len, prev->text->str);
		delete action;
	}
	else
	{
		//fprintf(stderr, "delete: %d %d '%.*s'\n", action->start, action->end, action->text->len, action->text->str);
		action->next = ctrl->_undo_stack;
		if (ctrl->_undo_stack)
			ctrl->_undo_stack->prev = action;
		ctrl->_undo_stack = action;
	}
}

static gboolean cb_keypress(GtkWidget *widget, GdkEvent *event, gTextArea *ctrl)
{
	if (event->key.state & GDK_CONTROL_MASK)
	{
		int key = gdk_keyval_to_unicode(gdk_keyval_to_upper(event->key.keyval));
		
		if (key == 'Z')
		{
			ctrl->undo();
			return true;
		}
		else if (key == 'Y')
		{
			ctrl->redo();
			return true;
		}
		else if (key == 'A')
		{
			ctrl->selectAll();
			return true;
		}
		else if (key == 'C')
		{
			ctrl->copy();
			return true;
		}
		else if (key == 'X')
		{
			ctrl->cut();
			ctrl->ensureVisible();
			return true;
		}
		else if (key == 'V')
		{
			ctrl->paste();
			ctrl->ensureVisible();
			return true;
		}
	}
	
	return false;
}

// TextArea

gTextArea::gTextArea(gContainer *parent) : gControl(parent)
{
	g_typ = Type_gTextArea;
	_align_normal = false;
	
	have_cursor = true;
	_undo_stack = NULL;
	_redo_stack = NULL;
	_not_undoable_action = 0;
	_undo_in_progress = false;
	
	onChange = 0;
	onCursor = 0;

	textview = gtk_text_view_new();
	realizeScrolledWindow(textview);

	setColorBase();

	//g_signal_connect_after(G_OBJECT(textview), "motion-notify-event", G_CALLBACK(cb_motion_notify_event), (gpointer)this);
	g_signal_connect(G_OBJECT(textview), "key-press-event", G_CALLBACK(cb_keypress), (gpointer)this);
	
	_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
	g_signal_connect_after(G_OBJECT(_buffer), "changed", G_CALLBACK(cb_changed), (gpointer)this);
	g_signal_connect_after(G_OBJECT(_buffer), "mark-set", G_CALLBACK(cb_mark_set), (gpointer)this);
	g_signal_connect(G_OBJECT(_buffer), "insert-text", G_CALLBACK(cb_insert_text), (gpointer)this);
	g_signal_connect(G_OBJECT(_buffer), "delete-range", G_CALLBACK(cb_delete_range), (gpointer)this);

	/*gtk_text_view_set_left_margin(GTK_TEXT_VIEW(textview), 2);
	gtk_text_view_set_right_margin(GTK_TEXT_VIEW(textview), 2);*/

	setBorder(true);
	setWrap(false);
}

void gTextArea::clearUndoStack()
{
	gTextAreaAction *action;
	
	while (_undo_stack)
	{
		action = _undo_stack;
		_undo_stack = _undo_stack->next;
		delete action;
	}
}

void gTextArea::clearRedoStack()
{
	gTextAreaAction *action;
	
	while (_redo_stack)
	{
		action = _redo_stack;
		_redo_stack = _redo_stack->next;
		delete action;
	}
}

gTextArea::~gTextArea()
{
	clearRedoStack();
	clearUndoStack();
}

char *gTextArea::text()
{
	GtkTextIter start;
	GtkTextIter end;
	char *text;
	
	gtk_text_buffer_get_bounds(_buffer, &start, &end);
	
	text = gtk_text_buffer_get_text(_buffer, &start, &end, true);
	gt_free_later(text);
	return text;
}

void gTextArea::setText(const char *txt, int len)
{
	if (!txt) 
	{
		txt = "";
		len = 0;
	}
	
	begin();
	gtk_text_buffer_set_text(_buffer, (const gchar *)txt, len);
	end();
}

bool gTextArea::readOnly()
{
	return !gtk_text_view_get_editable(GTK_TEXT_VIEW(textview));
}

void gTextArea::setReadOnly(bool vl)
{
	gtk_text_view_set_editable(GTK_TEXT_VIEW(textview), !vl);
	gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(textview), !vl);
}

GtkTextIter *gTextArea::getIterAt(int pos)
{
	static GtkTextIter iter;
	
	if (pos < 0)
		gtk_text_buffer_get_iter_at_mark(_buffer, &iter, gtk_text_buffer_get_insert(_buffer));
	else
		gtk_text_buffer_get_iter_at_offset(_buffer, &iter, pos);
		
	return &iter;
}

int gTextArea::line()
{
	return gtk_text_iter_get_line(getIterAt());
}

void gTextArea::setLine(int vl)
{
	int col = column();
	GtkTextIter *iter = getIterAt();
	
	if (vl < 0)
	{
		setPosition(0);
		return;
	}
	else if (vl >= gtk_text_buffer_get_line_count(_buffer))
	{
		setPosition(length());
		return;
	}
	
	gtk_text_iter_set_line(iter, vl);
	if (gtk_text_iter_get_chars_in_line(iter) <= col)
		col = gtk_text_iter_get_chars_in_line(iter) - 1;
	gtk_text_iter_set_line_offset(iter, col);
	gtk_text_buffer_place_cursor(_buffer, iter);
	ensureVisible();
}

int gTextArea::column()
{
	return gtk_text_iter_get_line_offset(getIterAt());
}

void gTextArea::setColumn(int vl)
{
	GtkTextIter *iter = getIterAt();

	if (vl < 0) 
	{
		vl = gtk_text_iter_get_chars_in_line(iter)-1;
	}
	else
	{
		if (gtk_text_iter_get_chars_in_line (iter) <= vl) 
			vl = gtk_text_iter_get_chars_in_line(iter) - 1;
	}
	
	gtk_text_iter_set_line_offset(iter,vl);
	gtk_text_buffer_place_cursor(_buffer, iter);
	ensureVisible();
}

int gTextArea::position()
{
	return gtk_text_iter_get_offset(getIterAt());
}

void gTextArea::setPosition(int vl)
{
	GtkTextIter *iter = getIterAt();
	
	gtk_text_iter_set_offset(iter, vl);
	gtk_text_buffer_place_cursor(_buffer, iter);
	ensureVisible();
}

int gTextArea::length()
{
	GtkTextIter iter;
	
	gtk_text_buffer_get_end_iter(_buffer, &iter);	
	return gtk_text_iter_get_offset(&iter);
}

bool gTextArea::wrap()
{
	return (gtk_text_view_get_wrap_mode(GTK_TEXT_VIEW(textview)) != GTK_WRAP_NONE);
}

void gTextArea::setWrap(bool vl)
{
	if (vl)
		gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(textview), GTK_WRAP_WORD_CHAR);
	else
		gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(textview), GTK_WRAP_NONE);
}

/**********************************************************************************

gTextArea methods

***********************************************************************************/

void gTextArea::copy()
{
	gtk_text_buffer_copy_clipboard(_buffer, gtk_clipboard_get(GDK_SELECTION_CLIPBOARD));
}

void gTextArea::cut()
{
	gtk_text_buffer_cut_clipboard(_buffer, gtk_clipboard_get(GDK_SELECTION_CLIPBOARD), true);
}

void gTextArea::ensureVisible()
{
	gtk_text_view_scroll_mark_onscreen(GTK_TEXT_VIEW(textview), gtk_text_buffer_get_insert(_buffer));
}

void gTextArea::paste()
{
	char *txt;
	int len;

	if (gClipboard::getType() != gClipboard::Text) 
		return;
	
	txt = gClipboard::getText(&len, NULL);
	if (txt)
		gtk_text_buffer_insert_at_cursor(_buffer, (const gchar *)txt, len);
}

void gTextArea::insert(const char *txt)
{
	if (!txt || !*txt)
		return;
	
	gtk_text_buffer_insert_at_cursor(_buffer, (const gchar *)txt, -1);
}

int gTextArea::toLine(int pos)
{
	if (pos < 0)
		pos=0;
	else if (pos >= length())
		pos = length() - 1;

	return gtk_text_iter_get_line(getIterAt(pos));
}

int gTextArea::toColumn(int pos)
{
	if (pos < 0)
		pos=0;
	else if (pos >= length())
		pos = length() - 1;

	return gtk_text_iter_get_line_offset(getIterAt(pos));
}

int gTextArea::toPosition(int line,int col)
{
	GtkTextIter iter;

	if (line < 0) line=0;
	if (col < 0) col=0;
	
	gtk_text_buffer_get_end_iter(_buffer, &iter);
	if (line > gtk_text_iter_get_line(&iter))
		line = gtk_text_iter_get_line(&iter);
	gtk_text_iter_set_line(&iter, line);
	
	if (col > gtk_text_iter_get_line_offset(&iter)) 
		col = gtk_text_iter_get_line_offset(&iter);
	
	gtk_text_iter_set_line_offset(&iter, col);
	
	return gtk_text_iter_get_offset(&iter);
} 


/**********************************************************************************

gTextArea selection

***********************************************************************************/

bool gTextArea::isSelected()
{
	return gtk_text_buffer_get_selection_bounds(_buffer, NULL, NULL);
	//return gtk_text_buffer_get_has_selection(buf); // Only since 2.10
}

int gTextArea::selStart()
{
	GtkTextIter start, end;
	
	gtk_text_buffer_get_selection_bounds(_buffer, &start, &end);
	return gtk_text_iter_get_offset(&start);	
}

int gTextArea::selEnd()
{
	GtkTextIter start, end;
	
	gtk_text_buffer_get_selection_bounds(_buffer, &start, &end);
	return gtk_text_iter_get_offset(&end);
}

char *gTextArea::selText()
{
	GtkTextIter start, end;
	char *text;
	
	gtk_text_buffer_get_selection_bounds(_buffer, &start, &end);
	text = gtk_text_buffer_get_text(_buffer, &start, &end, true);
	gt_free_later(text);
	return text;
}

void gTextArea::setSelText(const char *vl)
{
	GtkTextIter start, end;
	
	if (!vl) 
		vl = "";
	
	if (gtk_text_buffer_get_selection_bounds(_buffer, &start, &end))
		gtk_text_buffer_delete(_buffer, &start, &end);
	
	gtk_text_buffer_insert(_buffer, &start, vl, -1);	
}

void gTextArea::selDelete()
{
	GtkTextIter start, end;
	
	if (gtk_text_buffer_get_selection_bounds(_buffer, &start, &end))
	{
		gtk_text_iter_set_offset(&end, gtk_text_iter_get_offset(&start));
		gtk_text_buffer_select_range(_buffer, &start, &end);
	}
}

void gTextArea::selSelect(int pos, int length)
{
	GtkTextIter start, end;
	
	gtk_text_buffer_get_end_iter(_buffer, &start);
	
	if (gtk_text_iter_get_offset(&start) < pos)
		pos = gtk_text_iter_get_offset(&start);
	
	if (pos < 0) 
	{
		length -= pos;
		pos = 0;
	}
	
	if ((pos + length) < 0)
		length = (-pos);
	
	gtk_text_buffer_get_selection_bounds(_buffer, &start, &end);
	gtk_text_iter_set_offset(&start, pos);
	gtk_text_iter_set_offset(&end, pos + length);
	gtk_text_buffer_select_range(_buffer, &start, &end);
}

void gTextArea::updateCursor(GdkCursor *cursor)
{
  GdkWindow *win;

#ifdef GTK3
	win = gtk_text_view_get_window(GTK_TEXT_VIEW(textview), GTK_TEXT_WINDOW_TEXT);
#else
	win = ((PrivateGtkTextWindow *)GTK_TEXT_VIEW(textview)->text_window)->bin_window;
#endif
  
  gControl::updateCursor(cursor);
  
  if (!win)
  	return;
  
  if (cursor)
    gdk_window_set_cursor(win, cursor);
  else
  {
    cursor = gdk_cursor_new_for_display(gtk_widget_get_display(textview), GDK_XTERM);
    gdk_window_set_cursor(win, cursor);
#ifdef GTK3
    g_object_unref(cursor);
#else
    gdk_cursor_unref(cursor);
#endif
  }
}

/*void gTextArea::waitForLayout(int *tw, int *th)
{
  GtkTextIter start;
  GtkTextIter end;
	gint w, h;
	
	gtk_text_layout_set_screen_width(GTK_TEXT_VIEW(textview)->layout, width());
	
  gtk_text_buffer_get_bounds (_buffer, &start, &end);
  gtk_text_layout_invalidate (GTK_TEXT_VIEW(textview)->layout, &start, &end);
	gtk_text_layout_validate(GTK_TEXT_VIEW(textview)->layout, 0x7FFFFFFF);
	gtk_text_layout_get_size(GTK_TEXT_VIEW(textview)->layout, &w, &h);
	
	*tw = w;
	*th = h;
}

int gTextArea::textWidth()
{
	int w, h;
	waitForLayout(&w, &h);
	return w;
}

int gTextArea::textHeight()
{
	int w, h;
	waitForLayout(&w, &h);
	return h;
}*/

int gTextArea::alignment() const
{
	if (_align_normal)
		return ALIGN_NORMAL;
	
	switch(gtk_text_view_get_justification(GTK_TEXT_VIEW(textview)))
	{
		case GTK_JUSTIFY_RIGHT: return ALIGN_RIGHT;
		case GTK_JUSTIFY_CENTER: return ALIGN_CENTER;
		case GTK_JUSTIFY_LEFT: default: return ALIGN_LEFT;
	}
}

void gTextArea::setAlignment(int vl)
{
	GtkJustification align;
	
	_align_normal = false;
	
	switch(vl & 0xF)
	{
		case ALIGN_LEFT: align = GTK_JUSTIFY_LEFT; break;
		case ALIGN_RIGHT: align = GTK_JUSTIFY_RIGHT; break;
		case ALIGN_CENTER: align = GTK_JUSTIFY_CENTER; break;
		case ALIGN_NORMAL: default: align = gDesktop::rightToLeft() ? GTK_JUSTIFY_RIGHT: GTK_JUSTIFY_LEFT; _align_normal = true; break;
	}

	gtk_text_view_set_justification(GTK_TEXT_VIEW(textview), align);
}

void gTextArea::undo()
{
	gTextAreaAction *action;
	GtkTextIter start, stop;
	
	if (!_undo_stack)
		return;
	
	begin();
	_undo_in_progress = true;
	
	action = _undo_stack;
	_undo_stack = _undo_stack->next;
	
	action->prev = NULL;
	action->next = _redo_stack;
	if (_redo_stack)
		_redo_stack->prev = action;
	_redo_stack = action;
	
	if (action->type == ACTION_INSERT)
	{
		gtk_text_buffer_get_iter_at_offset(_buffer, &start, action->start);
		gtk_text_buffer_get_iter_at_offset(_buffer, &stop, action->start + action->length);
		gtk_text_buffer_delete(_buffer, &start, &stop);
		gtk_text_buffer_place_cursor(_buffer, &start);
	}
	else if (action->type == ACTION_DELETE)
	{
		gtk_text_buffer_get_iter_at_offset(_buffer, &start, action->start);
		gtk_text_buffer_insert(_buffer, &start, action->text->str, action->text->len);
		gtk_text_buffer_get_iter_at_offset(_buffer, &stop, action->end);
		if (action->delete_key_used)
			gtk_text_buffer_place_cursor(_buffer, &start);
		else
			gtk_text_buffer_place_cursor(_buffer, &stop);
	}
	
	end();
	ensureVisible();
	_undo_in_progress = false;
}

void gTextArea::redo()
{
	gTextAreaAction *action;
	GtkTextIter start, stop;

	if (!_redo_stack)
		return;
	
	begin();
	_undo_in_progress = true;
	
	action = _redo_stack;
	_redo_stack = _redo_stack->next;
	
	action->prev = NULL;
	action->next = _undo_stack;
	if (_undo_stack)
		_undo_stack->prev = action;
	_undo_stack = action;
	
	if (action->type == ACTION_INSERT)
	{
		gtk_text_buffer_get_iter_at_offset(_buffer, &start, action->start);
		gtk_text_buffer_insert(_buffer, &start, action->text->str, action->text->len);
		gtk_text_buffer_get_iter_at_offset(_buffer, &start, action->start + action->length);
		gtk_text_buffer_place_cursor(_buffer, &start);
	}
	else if (action->type == ACTION_DELETE)
	{
		gtk_text_buffer_get_iter_at_offset(_buffer, &start, action->start);
		gtk_text_buffer_get_iter_at_offset(_buffer, &stop, action->end);
		gtk_text_buffer_delete(_buffer, &start, &stop);
		gtk_text_buffer_place_cursor(_buffer, &start);
	}
	
	end();
	ensureVisible();
	_undo_in_progress = false;
}

void gTextArea::clear()
{
	begin();
	setText("");
	clearUndoStack();
	clearRedoStack();
	end();
}

#ifdef GTK3
void gTextArea::updateColor()
{
	gt_widget_set_color(textview, FALSE, background(), _bg_name, &_bg_default);
}
#endif

