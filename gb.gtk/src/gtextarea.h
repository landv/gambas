/***************************************************************************

  gtextarea.h

  (c) 2000-2013 Benoît Minisini <gambas@users.sourceforge.net>

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

#ifndef __GTEXTAREA_H
#define __GTEXTAREA_H

class gTextAreaAction;

class gTextArea : public gControl
{
public:
	gTextArea(gContainer *parent);
	~gTextArea();

//"Properties"
	int column();
	int length();
	int line();
	int position();
	bool readOnly();
	char* text();
	bool wrap();
	bool isSelected();

	void setColumn(int vl);
	void setLine(int vl);
	void setPosition(int vl);
	void setReadOnly(bool vl);
	void setText(const char *txt, int len = -1);
	void setWrap(bool vl);
	
	//int textWidth();
	//int textHeight();
	
	int alignment() const;
	void setAlignment(int vl);

//"Methods"
	void copy();
	void cut();
	void ensureVisible();
	void paste();
	void insert(const char *txt);
	int toLine(int pos);
	int toColumn(int pos);
	int toPosition(int line,int col);

//"Selection properties"
	int selStart();
	int selEnd();
	char* selText();

	void setSelText(const char *vl);

//"Selection methods"
	void selDelete();
	void selSelect(int pos, int length);
	void selectAll() { selSelect(0, length()); }

	bool canUndo() const { return _undo_stack != 0; }
	bool canRedo() const { return _redo_stack != 0; }
	void begin() { _not_undoable_action++; }
	void end() { _not_undoable_action--; }
	void undo();
	void redo();
	void clear();
	
//"Signals"
	void (*onChange)(gTextArea *sender);
	void (*onCursor)(gTextArea *sender);

//"Private"
  virtual void updateCursor(GdkCursor *cursor);
#ifdef GTK3
	virtual void updateColor();
#endif
  //void waitForLayout(int *tw, int *th);
	void clearUndoStack();
	void clearRedoStack();
	
	gTextAreaAction *_undo_stack;
	gTextAreaAction *_redo_stack;
	int _not_undoable_action;
	bool _undo_in_progress;

private:
	GtkWidget *textview;
	GtkTextBuffer *_buffer;
	bool _align_normal;

	GtkTextIter *getIterAt(int pos = -1);
};

#endif
