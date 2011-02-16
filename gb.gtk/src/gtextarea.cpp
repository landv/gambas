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
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

***************************************************************************/

#include "widgets.h"
#include "widgets_private.h"
#include "gapplication.h"
#include "gclipboard.h"
#include "gdesktop.h"
#include "gtextarea.h"

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

static gboolean cb_motion_notify_event(GtkWidget *widget, GdkEventMotion *event, gTextArea *data)
{
	// Workaround GtkTextView internal cursor changes
	if (gApplication::isBusy())
		data->setMouse(data->mouse());
	return FALSE;
}

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
		if (event->key.keyval == GDK_KEY_Z || event->key.keyval == GDK_KEY_z)
		{
			ctrl->undo();
			return true;
		}
		else if (event->key.keyval == GDK_KEY_Y || event->key.keyval == GDK_KEY_y)
		{
			ctrl->redo();
			return true;
		}
		else if (event->key.keyval == GDK_KEY_A || event->key.keyval == GDK_KEY_a)
		{
			ctrl->selectAll();
			return true;
		}
		else if (event->key.keyval == GDK_KEY_C || event->key.keyval == GDK_KEY_c)
		{
			ctrl->copy();
			return true;
		}
		else if (event->key.keyval == GDK_KEY_X || event->key.keyval == GDK_KEY_x)
		{
			ctrl->cut();
			return true;
		}
		else if (event->key.keyval == GDK_KEY_V || event->key.keyval == GDK_KEY_v)
		{
			ctrl->paste();
			return true;
		}
	}
	
	return false;
}

// TextArea

gTextArea::gTextArea(gContainer *parent) : gControl(parent)
{
	GtkTextBuffer *buf;
	
	g_typ = Type_gTextArea;
	_align_normal = false;
	
	have_cursor = true;
	use_base = true;
	_undo_stack = NULL;
	_redo_stack = NULL;
	_not_undoable_action = 0;
	_undo_in_progress = false;
	
	onChange = 0;
	onCursor = 0;

	textview = gtk_text_view_new();
	realizeScrolledWindow(textview);

	g_signal_connect_after(G_OBJECT(textview), "motion-notify-event", G_CALLBACK(cb_motion_notify_event), (gpointer)this);	
	g_signal_connect(G_OBJECT(textview), "key-press-event", G_CALLBACK(cb_keypress), (gpointer)this);
	
	buf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
	g_signal_connect_after(G_OBJECT(buf), "changed", G_CALLBACK(cb_changed), (gpointer)this);
	g_signal_connect_after(G_OBJECT(buf), "mark-set", G_CALLBACK(cb_mark_set), (gpointer)this);
	g_signal_connect(G_OBJECT(buf), "insert-text", G_CALLBACK(cb_insert_text), (gpointer)this);
	g_signal_connect(G_OBJECT(buf), "delete-range", G_CALLBACK(cb_delete_range), (gpointer)this);

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
	GtkTextBuffer *buf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
	GtkTextIter start;
	GtkTextIter end;
	char *text;
	
	if (!buf) return NULL;
	
	gtk_text_buffer_get_bounds(buf, &start, &end);
	
	text = gtk_text_buffer_get_text(buf, &start, &end, true);
	gt_free_later(text);
	return text;
}

void gTextArea::setText(const char *txt)
{
	GtkTextBuffer *buf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
	
	if (!txt) 
		txt = "";
	
	gtk_text_buffer_set_text(buf, (const gchar *)txt, -1);
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

int gTextArea::line()
{
	GtkTextBuffer *buf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
	GtkTextMark* mark = gtk_text_buffer_get_insert(buf);
	GtkTextIter iter;
	
	gtk_text_buffer_get_iter_at_mark(buf,&iter,mark);
	return gtk_text_iter_get_line(&iter);
}

void gTextArea::setLine(int vl)
{
	int col=column();
	GtkTextBuffer *buf=gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
	GtkTextMark* mark=gtk_text_buffer_get_insert(buf);
	GtkTextIter iter;
	
	gtk_widget_grab_focus(textview);
	gtk_text_buffer_get_iter_at_mark(buf,&iter,mark);
	gtk_text_iter_set_line (&iter,vl);
	if (gtk_text_iter_get_chars_in_line (&iter)<=col) col=gtk_text_iter_get_chars_in_line (&iter)-1;
	gtk_text_iter_set_line_offset(&iter,col);
	gtk_text_buffer_place_cursor(buf,&iter);
	gtk_text_view_scroll_mark_onscreen(GTK_TEXT_VIEW(textview),mark);
}

int gTextArea::column()
{
	GtkTextBuffer *buf=gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
	GtkTextMark* mark=gtk_text_buffer_get_insert(buf);
	GtkTextIter iter;
	
	gtk_text_buffer_get_iter_at_mark(buf,&iter,mark);
	return gtk_text_iter_get_line_offset(&iter);
}

void gTextArea::setColumn(int vl)
{
	GtkTextBuffer *buf=gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
	GtkTextMark* mark=gtk_text_buffer_get_insert(buf);
	GtkTextIter iter;

	
	gtk_widget_grab_focus(textview);
	gtk_text_buffer_get_iter_at_mark(buf,&iter,mark);
	
	if (vl<0) 
	{
		vl=gtk_text_iter_get_chars_in_line (&iter)-1;
	}
	else
	{
		if (gtk_text_iter_get_chars_in_line (&iter)<=vl) 
			vl=gtk_text_iter_get_chars_in_line (&iter)-1;
	}
	
	gtk_text_iter_set_line_offset(&iter,vl);
	gtk_text_buffer_place_cursor(buf,&iter);
	gtk_text_view_scroll_mark_onscreen(GTK_TEXT_VIEW(textview),mark);
}

int gTextArea::position()
{
	GtkTextBuffer *buf=gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
	GtkTextMark* mark=gtk_text_buffer_get_insert(buf);
	GtkTextIter iter;
	
	gtk_text_buffer_get_iter_at_mark(buf,&iter,mark);
	return gtk_text_iter_get_offset(&iter);
}

void gTextArea::setPosition(int vl)
{
	GtkTextBuffer *buf=gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
	GtkTextMark* mark=gtk_text_buffer_get_insert(buf);
	GtkTextIter iter;

	
	gtk_widget_grab_focus(textview);
	gtk_text_buffer_get_iter_at_mark(buf,&iter,mark);
	
	if (vl<0) 
	{
		vl=gtk_text_iter_get_offset (&iter);
	}
	else
	{
		if (gtk_text_iter_get_offset (&iter)<vl) 
			vl=gtk_text_iter_get_offset (&iter);
	}
	
	gtk_text_iter_set_offset(&iter,vl);
	gtk_text_buffer_place_cursor(buf,&iter);
	gtk_text_view_scroll_mark_onscreen(GTK_TEXT_VIEW(textview),mark);
}

int gTextArea::length()
{
	GtkTextBuffer *buf=gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
	GtkTextIter iter;
	
	gtk_text_buffer_get_end_iter(buf,&iter);
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
	GtkTextBuffer *buf=gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
	GtkClipboard* clip=gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
	
	gtk_text_buffer_copy_clipboard (buf,clip);
}

void gTextArea::cut()
{
	GtkTextBuffer *buf=gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
	GtkClipboard* clip=gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
	
	gtk_text_buffer_cut_clipboard (buf,clip,true);
}

void gTextArea::ensureVisible()
{
	setPosition(position());
}

void gTextArea::paste()
{
	GtkTextBuffer *buf=gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
	char *txt;

	if (gClipboard::getType() != gClipboard::Text) 
		return;
	
	txt = gClipboard::getText();
	if (txt)
		gtk_text_buffer_insert_at_cursor(buf,(const gchar *)txt,-1);
}

void gTextArea::insert(const char *txt)
{
	GtkTextBuffer *buf=gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
	if (!txt) txt="";
	
	gtk_text_buffer_insert_at_cursor(buf,(const gchar *)txt,-1);
}

int gTextArea::toLine(int pos)
{
	GtkTextBuffer *buf=gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
	GtkTextIter iter;

	gtk_text_buffer_get_start_iter(buf,&iter);
	if (pos<0) pos=0;
	if (pos>=length()) pos=length()-1;
	gtk_text_iter_set_offset(&iter,pos);
	return gtk_text_iter_get_line(&iter);
	
}

int gTextArea::toColumn(int pos)
{
	GtkTextBuffer *buf=gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
	GtkTextIter iter;

	gtk_text_buffer_get_start_iter(buf,&iter);
	if (pos<0) pos=0;
	if (pos>=length()) pos=length()-1;
	gtk_text_iter_set_offset(&iter,pos);
	return gtk_text_iter_get_line_offset(&iter);
}

int gTextArea::toPosition(int line,int col)
{
	GtkTextBuffer *buf=gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
	GtkTextIter iter;

	if (line<0) line=0;
	if (col<0) col=0;
	
	gtk_text_buffer_get_end_iter(buf,&iter);
	if (line>gtk_text_iter_get_line(&iter)) line=gtk_text_iter_get_line(&iter);
	gtk_text_iter_set_line(&iter,line);
	if (col>gtk_text_iter_get_line_offset(&iter)) col=gtk_text_iter_get_line_offset(&iter);
	gtk_text_iter_set_line_offset(&iter,col);
	return gtk_text_iter_get_offset(&iter);
} 


/**********************************************************************************

gTextArea selection

***********************************************************************************/

bool gTextArea::isSelected()
{
	GtkTextBuffer *buf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
	return gtk_text_buffer_get_selection_bounds(buf,NULL,NULL);
	//return gtk_text_buffer_get_has_selection(buf); // Only since 2.10
}

int gTextArea::selStart()
{
	GtkTextBuffer *buf=gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
	GtkTextIter start,end;
	
	gtk_text_buffer_get_selection_bounds(buf,&start,&end);
	return gtk_text_iter_get_offset(&start);	
}

int gTextArea::selEnd()
{
	GtkTextBuffer *buf=gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
	GtkTextIter start,end;
	
	gtk_text_buffer_get_selection_bounds(buf,&start,&end);
	return gtk_text_iter_get_offset(&end);
}

char *gTextArea::selText()
{
	GtkTextBuffer *buf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
	GtkTextIter start, end;
	char *text;
	
	gtk_text_buffer_get_selection_bounds(buf, &start, &end);

	text = gtk_text_buffer_get_text(buf, &start, &end, true);
	gt_free_later(text);
	return text;
}

void gTextArea::setSelText(const char *vl)
{
	GtkTextBuffer *buf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
	GtkTextIter start,end;
	
	if (!vl) vl="";
	
	if (gtk_text_buffer_get_selection_bounds(buf,&start,&end))
		gtk_text_buffer_delete(buf,&start,&end);
	
	gtk_text_buffer_insert(buf,&start,vl,-1);	
}

void gTextArea::selDelete()
{
	GtkTextBuffer *buf=gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
	GtkTextIter start,end;
	
	if (gtk_text_buffer_get_selection_bounds(buf,&start,&end))
	{
		gtk_text_iter_set_offset(&end,gtk_text_iter_get_offset(&start));
		gtk_text_buffer_select_range(buf,&start,&end);
	}
}

void gTextArea::selSelect(int start,int length)
{
	GtkTextBuffer *buf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
	GtkTextIter Start,End;
	
	gtk_text_buffer_get_end_iter(buf,&Start);
	if ( gtk_text_iter_get_offset(&Start)<start) start=gtk_text_iter_get_offset(&Start);
	
	if (start<0) { length-=start; start=0; }
	if ( (start+length)<0 ) length=(-1)*start;
	
	gtk_text_buffer_get_selection_bounds(buf,&Start,&End);
	gtk_text_iter_set_offset(&Start,start);
	gtk_text_iter_set_offset(&End,start+length);
	gtk_text_buffer_select_range(buf,&Start,&End);
	
}

void gTextArea::updateCursor(GdkCursor *cursor)
{
  GdkWindow *win = ((PrivateGtkTextWindow *)GTK_TEXT_VIEW(textview)->text_window)->bin_window;
  
  gControl::updateCursor(cursor);
  
  if (!win)
  	return;
  
  if (cursor)
    gdk_window_set_cursor(win, cursor);
  else
  {
    cursor = gdk_cursor_new_for_display(gtk_widget_get_display(textview), GDK_XTERM);
    gdk_window_set_cursor(win, cursor);
    gdk_cursor_unref(cursor);
  }
}

void gTextArea::waitForLayout(int *tw, int *th)
{
	GtkTextBuffer *buf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
  GtkTextIter start;
  GtkTextIter end;
	gint w, h;
	
	gtk_text_layout_set_screen_width(GTK_TEXT_VIEW(textview)->layout, width());
	
  gtk_text_buffer_get_bounds (buf, &start, &end);
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
}

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
	GtkTextBuffer *buf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
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
		gtk_text_buffer_get_iter_at_offset(buf, &start, action->start);
		gtk_text_buffer_get_iter_at_offset(buf, &stop, action->start + action->length);
		gtk_text_buffer_delete(buf, &start, &stop);
		gtk_text_buffer_place_cursor(buf, &start);
	}
	else if (action->type == ACTION_DELETE)
	{
		gtk_text_buffer_get_iter_at_offset(buf, &start, action->start);
		gtk_text_buffer_insert(buf, &start, action->text->str, action->text->len);
		gtk_text_buffer_get_iter_at_offset(buf, &stop, action->end);
		if (action->delete_key_used)
			gtk_text_buffer_place_cursor(buf, &start);
		else
			gtk_text_buffer_place_cursor(buf, &stop);
	}
	
	end();
	_undo_in_progress = false;
}

void gTextArea::redo()
{
	GtkTextBuffer *buf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
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
		gtk_text_buffer_get_iter_at_offset(buf, &start, action->start);
		gtk_text_buffer_insert(buf, &start, action->text->str, action->text->len);
		gtk_text_buffer_get_iter_at_offset(buf, &start, action->start + action->length);
		gtk_text_buffer_place_cursor(buf, &start);
	}
	else if (action->type == ACTION_DELETE)
	{
		gtk_text_buffer_get_iter_at_offset(buf, &start, action->start);
		gtk_text_buffer_get_iter_at_offset(buf, &stop, action->end);
		gtk_text_buffer_delete(buf, &start, &stop);
		gtk_text_buffer_place_cursor(buf, &start);
	}
	
	end();
	_undo_in_progress = false;
}

void gTextArea::clear()
{
	setText("");
	clearUndoStack();
	clearRedoStack();
}
