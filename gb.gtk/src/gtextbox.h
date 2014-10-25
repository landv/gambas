/***************************************************************************

  gtextbox.h

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

#ifndef __GTEXTBOX_H
#define __GTEXTBOX_H

class gTextBox : public gControl
{
public:
	gTextBox(gContainer *parent, bool combo = false);
	~gTextBox();

//"Properties"
	int alignment();
	bool hasBorder();
	virtual int length();
	int maxLength();
	bool password();
	int position();
	virtual char *text();
	virtual bool isReadOnly();
	int selLength();
	int selStart();
	char* selText();
	bool isSelected();

	void setAlignment(int vl);
	void setBorder(bool vl);
	void setMaxLength(int len);
	void setPassword(bool vl);
	void setPosition(int pos);
	virtual void setReadOnly(bool vl);
	virtual void setText(const char *vl);
	void setSelText(char *txt, int len);

//"Methods"
	virtual void clear();
	void insert(char* txt,int len);
	void selClear();
	void select(int start,int len);
	void selectAll();
	bool hasEntry() const { return entry != 0; }

//"Signals"
	void (*onChange)(gTextBox *sender);
	void (*onActivate)(gTextBox *sender);

//"Private"
  virtual void updateCursor(GdkCursor *cursor);
  void initEntry();
  GtkWidget *entry;
	virtual int minimumHeight();

	unsigned _changed : 1;

#ifdef GTK3
	static GtkCssProvider *_style_provider;
#endif
};

#endif
