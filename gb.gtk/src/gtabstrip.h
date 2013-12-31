/***************************************************************************

  gtabstrip.h

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

#ifndef __GTABSTRIP_H
#define __GTABSTRIP_H

class gTabStripPage;

class gTabStrip : public gContainer
{
	friend class gTabStripPage;
	
public:
	gTabStrip(gContainer *parent);
	~gTabStrip();

//"Properties"
	int count() const { return _pages->len; }
	int index() const;
	int orientation() const;
	void setOrientation(int vl);
	bool tabEnabled(int ind) const;
	gPicture* tabPicture(int ind) const;
	bool tabVisible(int ind) const;
	char *tabText(int ind) const;
	int tabCount(int ind) const;
	gControl *tabChild(int ind, int n) const;
	int findIndex(gControl *child) const;
	
	bool setCount(int vl);
	void setIndex(int vl);
	void setTabPicture(int ind, gPicture *pic);
	void setTabEnabled(int ind, bool vl);
	void setTabText(int ind, char *txt);
	void setTabVisible(int ind, bool vl);
	bool removeTab(int ind);
	void setClosable(bool v);
	bool isClosable() const { return _button_pixbuf_normal != NULL; }

	virtual int childCount() const;
	virtual gControl *child(int index) const;

	virtual void setRealBackground(gColor color);
	virtual void setRealForeground(gColor color);
	virtual void updateFont();

	gFont *textFont();
	void setTextFont(gFont *ft);

	//"Events"
	void (*onClick)(gTabStrip *sender);
	void (*onClose)(gTabStrip *sender, int index);

//"Private"
	virtual GtkWidget *getContainer();
	int getRealIndex(GtkWidget *page) const;

	GdkPixbuf *_button_pixbuf_normal;
	GdkPixbuf *_button_pixbuf_disabled;
	
private:
	GPtrArray *_pages;
	gFont *_textFont;
	gTabStripPage *get(int ind) const;
	void destroyTab(int ind);
};

#endif
