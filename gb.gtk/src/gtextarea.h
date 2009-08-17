/***************************************************************************

  gtextarea.h

  (c) 2000-2009 Benoît Minisini <gambas@users.sourceforge.net>

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
#ifndef __GTEXTAREA_H
#define __GTEXTAREA_H

class gTextArea : public gControl
{
public:
	gTextArea(gContainer *parent);

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
	void setText(const char *txt);
	void setWrap(bool vl);
	
	int textWidth();
	int textHeight();

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
	void selSelect(int start,int length);

//"Signals"
	void (*onChange)(gTextArea *sender);
	void (*onCursor)(gTextArea *sender);

//"Private"
  void updateCursor(GdkCursor *cursor);
  void waitForLayout(int *tw, int *th);

private:
	GtkWidget *textview;
};

#endif
